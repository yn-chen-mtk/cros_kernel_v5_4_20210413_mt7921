// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek xHCI Host Controller Driver
 *
 * Copyright (c) 2015 MediaTek Inc.
 * Author:
 *  Chunfeng Yun <chunfeng.yun@mediatek.com>
 */

#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>

#include "xhci.h"
#include "xhci-mtk.h"

/* ip_pw_ctrl0 register */
#define CTRL0_IP_SW_RST	BIT(0)

/* ip_pw_ctrl1 register */
#define CTRL1_IP_HOST_PDN	BIT(0)

/* ip_pw_ctrl2 register */
#define CTRL2_IP_DEV_PDN	BIT(0)

/* ip_pw_sts1 register */
#define STS1_IP_SLEEP_STS	BIT(30)
#define STS1_U3_MAC_RST	BIT(16)
#define STS1_XHCI_RST		BIT(11)
#define STS1_SYS125_RST	BIT(10)
#define STS1_REF_RST		BIT(8)
#define STS1_SYSPLL_STABLE	BIT(0)

/* ip_xhci_cap register */
#define CAP_U3_PORT_NUM(p)	((p) & 0xff)
#define CAP_U2_PORT_NUM(p)	(((p) >> 8) & 0xff)

/* u3_ctrl_p register */
#define CTRL_U3_PORT_HOST_SEL	BIT(2)
#define CTRL_U3_PORT_PDN	BIT(1)
#define CTRL_U3_PORT_DIS	BIT(0)

/* u2_ctrl_p register */
#define CTRL_U2_PORT_HOST_SEL	BIT(2)
#define CTRL_U2_PORT_PDN	BIT(1)
#define CTRL_U2_PORT_DIS	BIT(0)

/* u2_phy_pll register */
#define CTRL_U2_FORCE_PLL_STB	BIT(28)

/* usb remote wakeup registers in syscon */

/* mt8173 etc */
#define PERI_WK_CTRL1	0x4
#define WC1_IS_C(x)	(((x) & 0xf) << 26)  /* cycle debounce */
#define WC1_IS_EN	BIT(25)
#define WC1_IS_P	BIT(6)  /* polarity for ip sleep */

/* mt8183 */
#define PERI_WK_CTRL0	0x0
#define WC0_IS_C(x)	((u32)(((x) & 0xf) << 28))  /* cycle debounce */
#define WC0_IS_P	BIT(12)	/* polarity */
#define WC0_IS_EN	BIT(6)

/* mt8192 */
#define WC0_SSUSB0_CDEN		BIT(6)
#define WC0_IS_SPM_EN		BIT(1)

/* mt2712 etc */
#define PERI_SSUSB_SPM_CTRL	0x0
#define SSC_IP_SLEEP_EN	BIT(4)
#define SSC_SPM_INT_EN		BIT(1)

enum ssusb_uwk_vers {
	SSUSB_UWK_V1 = 1,
	SSUSB_UWK_V2,
	SSUSB_UWK_V1_1 = 101,	/* specific revision 1.01 */
	SSUSB_UWK_V1_2,		/* specific revision 1.2 */
};

int xhci_mtk_runtime_ready;

static int xhci_mtk_runtime_resume(struct device *dev);

static void xhci_mtk_seal_work(struct work_struct *work)
{
	struct xhci_hcd_mtk *mtk =
			container_of(work, struct xhci_hcd_mtk, seal.work);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);

	xhci_dbg(xhci, "spm unseals xHCI controller %i\n", mtk->seal_status);
	if (mtk->seal_status == SEAL_SUSPENDED) {
		mtk->seal_status = SEAL_RESUMING;
		pm_runtime_mark_last_busy(mtk->dev);
		pm_runtime_put_sync_autosuspend(mtk->dev);
	} else {
		/*
		 * FIXME: Sometimes seal_status will keep it on SEAL_RESUMING staus not to expect
		 * since pm_runtime_put_sync_autosuspend doesn't invoke runtime_resume of callback.
		 * and to survey why not to invoke runtime_resume callback named
		 * xhci_mtk_runtime_resume in time in short feature, Herely provide one solution
		 * that make sure seal_status to match h/w state machine,and MTK xHCI clocks
		 * on as soon as unseal event received.
		 */
		if (mtk->seal_status == SEAL_RESUMING)
			xhci_mtk_runtime_resume(mtk->dev);
		else
			xhci_warn(xhci,
				"Ignore seal wakeup source disordered in xHCI controller\n");
	}
}

static irqreturn_t xhci_mtk_seal_irq(int irq, void *data)
{
	struct xhci_hcd_mtk *mtk = data;
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);

	xhci_dbg(xhci, "seal irq ISR invoked\n");

	schedule_delayed_work(&mtk->seal, 0);

	return IRQ_HANDLED;
}

static void xhci_mtk_seal_wakeup_enable(struct xhci_hcd_mtk *mtk, bool enable)
{
	struct irq_desc *desc;
	struct device *dev = mtk->dev;

	if (mtk && mtk->seal_irq) {
		desc = irq_to_desc(mtk->seal_irq);
		if (enable) {
			desc->irq_data.chip->irq_ack(&desc->irq_data);
			enable_irq(mtk->seal_irq);
			dev_dbg(dev, "%s: enable_irq %i\n",
				 __func__, mtk->seal_irq);
		} else {
			disable_irq(mtk->seal_irq);
			dev_dbg(dev, "%s: disable_irq %i\n",
				 __func__, mtk->seal_irq);
		}
	}
}

static int xhci_mtk_host_enable(struct xhci_hcd_mtk *mtk)
{
	struct mu3c_ippc_regs __iomem *ippc = mtk->ippc_regs;
	u32 value, check_val;
	int u3_ports_disabed = 0;
	int ret;
	int i;

	if (!mtk->has_ippc)
		return 0;

	/* power on host ip */
	value = readl(&ippc->ip_pw_ctr1);
	value &= ~CTRL1_IP_HOST_PDN;
	writel(value, &ippc->ip_pw_ctr1);

	/* power on and enable u3 ports except skipped ones */
	for (i = 0; i < mtk->num_u3_ports; i++) {
		if ((0x1 << i) & mtk->u3p_dis_msk) {
			u3_ports_disabed++;
			continue;
		}

		value = readl(&ippc->u3_ctrl_p[i]);
		value &= ~(CTRL_U3_PORT_PDN | CTRL_U3_PORT_DIS);
		value |= CTRL_U3_PORT_HOST_SEL;
		writel(value, &ippc->u3_ctrl_p[i]);
	}

	/* power on and enable all u2 ports */
	for (i = 0; i < mtk->num_u2_ports; i++) {
		value = readl(&ippc->u2_ctrl_p[i]);
		value &= ~(CTRL_U2_PORT_PDN | CTRL_U2_PORT_DIS);
		value |= CTRL_U2_PORT_HOST_SEL;
		writel(value, &ippc->u2_ctrl_p[i]);
	}

	/*
	 * wait for clocks to be stable, and clock domains reset to
	 * be inactive after power on and enable ports
	 */
	check_val = STS1_SYSPLL_STABLE | STS1_REF_RST |
			STS1_SYS125_RST | STS1_XHCI_RST;

	if (mtk->num_u3_ports > u3_ports_disabed)
		check_val |= STS1_U3_MAC_RST;

	ret = readl_poll_timeout(&ippc->ip_pw_sts1, value,
			  (check_val == (value & check_val)), 100, 20000);
	if (ret) {
		dev_err(mtk->dev, "clocks are not stable (0x%x)\n", value);
		return ret;
	}

	return 0;
}

static int xhci_mtk_host_disable(struct xhci_hcd_mtk *mtk)
{
	struct mu3c_ippc_regs __iomem *ippc = mtk->ippc_regs;
	u32 value;
	int ret;
	int i;

	if (!mtk->has_ippc)
		return 0;

	/* power down u3 ports except skipped ones */
	for (i = 0; i < mtk->num_u3_ports; i++) {
		if ((0x1 << i) & mtk->u3p_dis_msk)
			continue;

		value = readl(&ippc->u3_ctrl_p[i]);
		value |= CTRL_U3_PORT_PDN;
		writel(value, &ippc->u3_ctrl_p[i]);
	}

	/* power down all u2 ports */
	for (i = 0; i < mtk->num_u2_ports; i++) {
		value = readl(&ippc->u2_ctrl_p[i]);
		value |= CTRL_U2_PORT_PDN;
		writel(value, &ippc->u2_ctrl_p[i]);
	}

	/* power down host ip */
	value = readl(&ippc->ip_pw_ctr1);
	value |= CTRL1_IP_HOST_PDN;
	writel(value, &ippc->ip_pw_ctr1);

	/* wait for host ip to sleep */
	ret = readl_poll_timeout(&ippc->ip_pw_sts1, value,
			  (value & STS1_IP_SLEEP_STS), 100, 100000);
	if (ret) {
		dev_err(mtk->dev, "ip sleep failed!!!\n");
		return ret;
	} else {
		usleep_range(200, 250);
	}

	return 0;
}

static int xhci_mtk_ssusb_config(struct xhci_hcd_mtk *mtk)
{
	struct mu3c_ippc_regs __iomem *ippc = mtk->ippc_regs;
	u32 value;

	if (!mtk->has_ippc)
		return 0;

	/* reset whole ip */
	value = readl(&ippc->ip_pw_ctr0);
	value |= CTRL0_IP_SW_RST;
	writel(value, &ippc->ip_pw_ctr0);
	udelay(1);
	value = readl(&ippc->ip_pw_ctr0);
	value &= ~CTRL0_IP_SW_RST;
	writel(value, &ippc->ip_pw_ctr0);

	/*
	 * device ip is default power-on in fact
	 * power down device ip, otherwise ip-sleep will fail
	 */
	value = readl(&ippc->ip_pw_ctr2);
	value |= CTRL2_IP_DEV_PDN;
	writel(value, &ippc->ip_pw_ctr2);

	value = readl(&ippc->ip_xhci_cap);
	mtk->num_u3_ports = CAP_U3_PORT_NUM(value);
	mtk->num_u2_ports = CAP_U2_PORT_NUM(value);
	dev_dbg(mtk->dev, "%s u2p:%d, u3p:%d\n", __func__,
			mtk->num_u2_ports, mtk->num_u3_ports);

	return xhci_mtk_host_enable(mtk);
}

static int xhci_mtk_clks_get(struct xhci_hcd_mtk *mtk)
{
	struct device *dev = mtk->dev;

	mtk->sys_clk = devm_clk_get(dev, "sys_ck");
	if (IS_ERR(mtk->sys_clk)) {
		dev_err(dev, "fail to get sys_ck\n");
		return PTR_ERR(mtk->sys_clk);
	}

	mtk->xhci_clk = devm_clk_get_optional(dev, "xhci_ck");
	if (IS_ERR(mtk->xhci_clk))
		return PTR_ERR(mtk->xhci_clk);

	mtk->ref_clk = devm_clk_get_optional(dev, "ref_ck");
	if (IS_ERR(mtk->ref_clk))
		return PTR_ERR(mtk->ref_clk);

	mtk->mcu_clk = devm_clk_get_optional(dev, "mcu_ck");
	if (IS_ERR(mtk->mcu_clk))
		return PTR_ERR(mtk->mcu_clk);

	mtk->dma_clk = devm_clk_get_optional(dev, "dma_ck");
	return PTR_ERR_OR_ZERO(mtk->dma_clk);
}

static int xhci_mtk_clks_enable(struct xhci_hcd_mtk *mtk)
{
	int ret;

	ret = clk_prepare_enable(mtk->ref_clk);
	if (ret) {
		dev_err(mtk->dev, "failed to enable ref_clk\n");
		goto ref_clk_err;
	}

	ret = clk_prepare_enable(mtk->sys_clk);
	if (ret) {
		dev_err(mtk->dev, "failed to enable sys_clk\n");
		goto sys_clk_err;
	}

	ret = clk_prepare_enable(mtk->xhci_clk);
	if (ret) {
		dev_err(mtk->dev, "failed to enable xhci_clk\n");
		goto xhci_clk_err;
	}

	ret = clk_prepare_enable(mtk->mcu_clk);
	if (ret) {
		dev_err(mtk->dev, "failed to enable mcu_clk\n");
		goto mcu_clk_err;
	}

	ret = clk_prepare_enable(mtk->dma_clk);
	if (ret) {
		dev_err(mtk->dev, "failed to enable dma_clk\n");
		goto dma_clk_err;
	}

	return 0;

dma_clk_err:
	clk_disable_unprepare(mtk->mcu_clk);
mcu_clk_err:
	clk_disable_unprepare(mtk->xhci_clk);
xhci_clk_err:
	clk_disable_unprepare(mtk->sys_clk);
sys_clk_err:
	clk_disable_unprepare(mtk->ref_clk);
ref_clk_err:
	return ret;
}

static void xhci_mtk_clks_disable(struct xhci_hcd_mtk *mtk)
{
	clk_disable_unprepare(mtk->dma_clk);
	clk_disable_unprepare(mtk->mcu_clk);
	clk_disable_unprepare(mtk->xhci_clk);
	clk_disable_unprepare(mtk->sys_clk);
	clk_disable_unprepare(mtk->ref_clk);
}

/* only clocks can be turn off for ip-sleep wakeup mode */
static void usb_wakeup_ip_sleep_set(struct xhci_hcd_mtk *mtk, bool enable)
{
	u32 reg, msk, val;

	switch (mtk->uwk_vers) {
	case SSUSB_UWK_V1:
		reg = mtk->uwk_reg_base + PERI_WK_CTRL1;
		msk = WC1_IS_EN | WC1_IS_C(0xf) | WC1_IS_P;
		val = enable ? (WC1_IS_EN | WC1_IS_C(0x8)) : 0;
		break;
	case SSUSB_UWK_V1_1:
		reg = mtk->uwk_reg_base + PERI_WK_CTRL0;
		msk = WC0_IS_EN | WC0_IS_C(0xf) | WC0_IS_P;
		val = enable ? (WC0_IS_EN | WC0_IS_C(0x8)) : 0;
		break;
	case SSUSB_UWK_V1_2:
		reg = mtk->uwk_reg_base + PERI_WK_CTRL0;
		msk = WC0_SSUSB0_CDEN | WC0_IS_SPM_EN;
		val = enable ? msk : 0;
		break;
	case SSUSB_UWK_V2:
		reg = mtk->uwk_reg_base + PERI_SSUSB_SPM_CTRL;
		msk = SSC_IP_SLEEP_EN | SSC_SPM_INT_EN;
		val = enable ? msk : 0;
		break;
	default:
		return;
	}
	regmap_update_bits(mtk->uwk, reg, msk, val);
}

static int usb_wakeup_of_property_parse(struct xhci_hcd_mtk *mtk,
				struct device_node *dn)
{
	struct of_phandle_args args;
	int ret;

	/* Wakeup function is optional */
	mtk->uwk_en = of_property_read_bool(dn, "wakeup-source");
	if (!mtk->uwk_en)
		return 0;

	ret = of_parse_phandle_with_fixed_args(dn,
				"mediatek,syscon-wakeup", 2, 0, &args);
	if (ret)
		return ret;

	mtk->uwk_reg_base = args.args[0];
	mtk->uwk_vers = args.args[1];
	mtk->uwk = syscon_node_to_regmap(args.np);
	of_node_put(args.np);
	dev_info(mtk->dev, "uwk - reg:0x%x, version:%d\n",
			mtk->uwk_reg_base, mtk->uwk_vers);

	return PTR_ERR_OR_ZERO(mtk->uwk);
}

static void usb_wakeup_set(struct xhci_hcd_mtk *mtk, bool enable)
{
	if (mtk->uwk_en)
		usb_wakeup_ip_sleep_set(mtk, enable);
}

static int xhci_mtk_ldos_enable(struct xhci_hcd_mtk *mtk)
{
	int ret;

	ret = regulator_enable(mtk->vbus);
	if (ret) {
		dev_err(mtk->dev, "failed to enable vbus\n");
		return ret;
	}

	ret = regulator_enable(mtk->vusb33);
	if (ret) {
		dev_err(mtk->dev, "failed to enable vusb33\n");
		regulator_disable(mtk->vbus);
		return ret;
	}
	return 0;
}

static void xhci_mtk_ldos_disable(struct xhci_hcd_mtk *mtk)
{
	regulator_disable(mtk->vbus);
	regulator_disable(mtk->vusb33);
}

static void xhci_mtk_quirks(struct device *dev, struct xhci_hcd *xhci)
{
	struct usb_hcd *hcd = xhci_to_hcd(xhci);
	struct xhci_hcd_mtk *mtk = hcd_to_mtk(hcd);

	/*
	 * As of now platform drivers don't provide MSI support so we ensure
	 * here that the generic code does not try to make a pci_dev from our
	 * dev struct in order to setup MSI
	 */
	xhci->quirks |= XHCI_PLAT;
	xhci->quirks |= XHCI_MTK_HOST;
	/*
	 * MTK host controller gives a spurious successful event after a
	 * short transfer. Ignore it.
	 */
	xhci->quirks |= XHCI_SPURIOUS_SUCCESS;
	if (mtk->lpm_support)
		xhci->quirks |= XHCI_LPM_SUPPORT;
	if (mtk->u2_lpm_disable)
		xhci->quirks |= XHCI_HW_LPM_DISABLE;

	/*
	 * MTK xHCI 0.96: PSA is 1 by default even if doesn't support stream,
	 * and it's 3 when support it.
	 */
	if (xhci->hci_version < 0x100 && HCC_MAX_PSA(xhci->hcc_params) == 4)
		xhci->quirks |= XHCI_BROKEN_STREAMS;
}

/* called during probe() after chip reset completes */
static int xhci_mtk_setup(struct usb_hcd *hcd)
{
	struct xhci_hcd_mtk *mtk = hcd_to_mtk(hcd);
	int ret;

	if (usb_hcd_is_primary_hcd(hcd)) {
		ret = xhci_mtk_ssusb_config(mtk);
		if (ret)
			return ret;
	}

	ret = xhci_gen_setup(hcd, xhci_mtk_quirks);
	if (ret)
		return ret;

	if (usb_hcd_is_primary_hcd(hcd)) {
		ret = xhci_mtk_sch_init(mtk);
		if (ret)
			return ret;
	}

	return ret;
}

static const struct xhci_driver_overrides xhci_mtk_overrides __initconst = {
	.reset = xhci_mtk_setup,
	.add_endpoint = xhci_mtk_add_ep,
	.drop_endpoint = xhci_mtk_drop_ep,
	.check_bandwidth = xhci_mtk_check_bandwidth,
	.reset_bandwidth = xhci_mtk_reset_bandwidth,
};

static struct hc_driver __read_mostly xhci_mtk_hc_driver;

static int xhci_mtk_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct xhci_hcd_mtk *mtk;
	const struct hc_driver *driver;
	struct xhci_hcd *xhci;
	struct resource *res;
	struct usb_hcd *hcd;
	int ret = -ENODEV;
	int irq;

	if (usb_disabled())
		return -ENODEV;

	driver = &xhci_mtk_hc_driver;
	mtk = devm_kzalloc(dev, sizeof(*mtk), GFP_KERNEL);
	if (!mtk)
		return -ENOMEM;

	mtk->dev = dev;
	mtk->vbus = devm_regulator_get(dev, "vbus");
	if (IS_ERR(mtk->vbus)) {
		dev_err(dev, "fail to get vbus\n");
		return PTR_ERR(mtk->vbus);
	}

	mtk->vusb33 = devm_regulator_get(dev, "vusb33");
	if (IS_ERR(mtk->vusb33)) {
		dev_err(dev, "fail to get vusb33\n");
		return PTR_ERR(mtk->vusb33);
	}

	ret = xhci_mtk_clks_get(mtk);
	if (ret)
		return ret;

	mtk->lpm_support = of_property_read_bool(node, "usb3-lpm-capable");
	mtk->u2_lpm_disable = of_property_read_bool(node, "usb2-lpm-disable");
	/* optional property, ignore the error if it does not exist */
	of_property_read_u32(node, "mediatek,u3p-dis-msk",
			     &mtk->u3p_dis_msk);

	ret = usb_wakeup_of_property_parse(mtk, node);
	if (ret) {
		dev_err(dev, "failed to parse uwk property\n");
		return ret;
	}

	pm_runtime_set_active(dev);
	pm_runtime_use_autosuspend(dev);
	pm_runtime_set_autosuspend_delay(dev,
				CONFIG_USB_AUTOSUSPEND_DELAY * 1000);
	pm_runtime_enable(dev);

	ret = xhci_mtk_ldos_enable(mtk);
	if (ret)
		goto disable_pm;

	ret = xhci_mtk_clks_enable(mtk);
	if (ret)
		goto disable_ldos;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		ret = irq;
		goto disable_clk;
	}
	dev_dbg(dev, "irq %i\n", irq);

	mtk->seal_irq = platform_get_irq_optional(pdev, 1);
	if (mtk->seal_irq < 0) {
		ret = mtk->seal_irq;
		goto disable_clk;
	}
	dev_dbg(dev, "seal_irq %i\n", mtk->seal_irq);

	hcd = usb_create_hcd(driver, dev, dev_name(dev));
	if (!hcd) {
		ret = -ENOMEM;
		goto disable_clk;
	}

	/*
	 * USB 2.0 roothub is stored in the platform_device.
	 * Swap it with mtk HCD.
	 */
	mtk->hcd = platform_get_drvdata(pdev);
	platform_set_drvdata(pdev, mtk);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "mac");
	hcd->regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(hcd->regs)) {
		ret = PTR_ERR(hcd->regs);
		goto put_usb2_hcd;
	}
	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ippc");
	if (res) {	/* ippc register is optional */
		mtk->ippc_regs = devm_ioremap_resource(dev, res);
		if (IS_ERR(mtk->ippc_regs)) {
			ret = PTR_ERR(mtk->ippc_regs);
			goto put_usb2_hcd;
		}
		mtk->has_ippc = true;
	} else {
		mtk->has_ippc = false;
	}

	device_init_wakeup(dev, true);

	xhci = hcd_to_xhci(hcd);
	xhci->main_hcd = hcd;

	/*
	 * imod_interval is the interrupt moderation value in nanoseconds.
	 * The increment interval is 8 times as much as that defined in
	 * the xHCI spec on MTK's controller.
	 */
	xhci->imod_interval = 5000;
	device_property_read_u32(dev, "imod-interval-ns", &xhci->imod_interval);

	xhci->shared_hcd = usb_create_shared_hcd(driver, dev,
			dev_name(dev), hcd);
	if (!xhci->shared_hcd) {
		ret = -ENOMEM;
		goto disable_device_wakeup;
	}

	ret = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (ret)
		goto put_usb3_hcd;

	if (HCC_MAX_PSA(xhci->hcc_params) >= 4 &&
	    !(xhci->quirks & XHCI_BROKEN_STREAMS))
		xhci->shared_hcd->can_do_streams = 1;

	ret = usb_add_hcd(xhci->shared_hcd, irq, IRQF_SHARED);
	if (ret)
		goto dealloc_usb2_hcd;

	INIT_DELAYED_WORK(&mtk->seal, xhci_mtk_seal_work);
	snprintf(mtk->seal_descr, sizeof(mtk->seal_descr), "seal%s:usb%d",
		 hcd->driver->description, hcd->self.busnum);
	ret = devm_request_irq(dev, mtk->seal_irq, &xhci_mtk_seal_irq,
			  IRQF_TRIGGER_FALLING,	mtk->seal_descr, mtk);
	if (ret != 0) {
		dev_err(dev, "seal request interrupt %d failed\n",
			mtk->seal_irq);
		goto dealloc_usb2_hcd;
	}
	xhci_mtk_seal_wakeup_enable(mtk, false);

	device_enable_async_suspend(dev);
	xhci_mtk_runtime_ready = 1;

	pm_runtime_mark_last_busy(dev);
	pm_runtime_put_autosuspend(dev);

	dev_dbg(dev, "%s: xhci_mtk_runtime_ready %i",
		 __func__, xhci_mtk_runtime_ready);

	return 0;

dealloc_usb2_hcd:
	usb_remove_hcd(hcd);

put_usb3_hcd:
	xhci_mtk_sch_exit(mtk);
	usb_put_hcd(xhci->shared_hcd);

disable_device_wakeup:
	device_init_wakeup(dev, false);

put_usb2_hcd:
	usb_put_hcd(hcd);

disable_clk:
	xhci_mtk_clks_disable(mtk);

disable_ldos:
	xhci_mtk_ldos_disable(mtk);

disable_pm:
	pm_runtime_put_sync_autosuspend(dev);
	pm_runtime_disable(dev);
	return ret;
}

static int xhci_mtk_remove(struct platform_device *dev)
{
	struct xhci_hcd_mtk *mtk = platform_get_drvdata(dev);
	struct usb_hcd	*hcd = mtk->hcd;
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	struct usb_hcd  *shared_hcd = xhci->shared_hcd;

	pm_runtime_put_noidle(&dev->dev);
	pm_runtime_disable(&dev->dev);

	xhci_mtk_runtime_ready = 0;
	usb_remove_hcd(shared_hcd);
	xhci->shared_hcd = NULL;
	device_init_wakeup(&dev->dev, false);

	usb_remove_hcd(hcd);
	usb_put_hcd(shared_hcd);
	usb_put_hcd(hcd);
	xhci_mtk_sch_exit(mtk);
	xhci_mtk_clks_disable(mtk);
	xhci_mtk_ldos_disable(mtk);

	return 0;
}

/*
 * if ip sleep fails, and all clocks are disabled, access register will hang
 * AHB bus, so stop polling roothubs to avoid regs access on bus suspend.
 * and no need to check whether ip sleep failed or not; this will cause SPM
 * to wake up system immediately after system suspend complete if ip sleep
 * fails, it is what we wanted.
 */
static int __maybe_unused xhci_mtk_suspend(struct device *dev)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);

	xhci_dbg(xhci, "%s: stop port polling\n", __func__);
	clear_bit(HCD_FLAG_POLL_RH, &hcd->flags);
	del_timer_sync(&hcd->rh_timer);
	clear_bit(HCD_FLAG_POLL_RH, &xhci->shared_hcd->flags);
	del_timer_sync(&xhci->shared_hcd->rh_timer);

	xhci_mtk_host_disable(mtk);
	xhci_mtk_clks_disable(mtk);
	usb_wakeup_set(mtk, true);

	return 0;
}

static int __maybe_unused xhci_mtk_resume(struct device *dev)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);

	usb_wakeup_set(mtk, false);
	xhci_mtk_clks_enable(mtk);
	xhci_mtk_host_enable(mtk);

	xhci_dbg(xhci, "%s: restart port polling\n", __func__);
	set_bit(HCD_FLAG_POLL_RH, &xhci->shared_hcd->flags);
	usb_hcd_poll_rh_status(xhci->shared_hcd);
	set_bit(HCD_FLAG_POLL_RH, &hcd->flags);
	usb_hcd_poll_rh_status(hcd);
	return 0;
}

static int __maybe_unused xhci_mtk_bus_status(struct device *dev)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd;
	struct xhci_hcd *xhci;
	struct xhci_hub *usb2_rhub;
	struct xhci_hub *usb3_rhub;
	struct xhci_bus_state *bus_state;
	struct xhci_port *port;
	u32	usb2_suspended_ports = -1;
	u32	usb3_suspended_ports = -1;
	u16 status;
	int num_ports;
	int ret = 0;
	int i;

	if (!mtk->hcd)
		return -ESHUTDOWN;

	hcd = mtk->hcd;
	xhci = hcd_to_xhci(hcd);
	if ((xhci->xhc_state & XHCI_STATE_REMOVING) ||
			(xhci->xhc_state & XHCI_STATE_HALTED)) {
		return -ESHUTDOWN;
	}

	usb2_rhub = &xhci->usb2_rhub;
	if (usb2_rhub) {
		bus_state  = &usb2_rhub->bus_state;
		num_ports  = usb2_rhub->num_ports;
		usb2_suspended_ports  = bus_state->suspended_ports;
		usb2_suspended_ports ^= (BIT(num_ports) - 1);
		usb2_suspended_ports &= (BIT(num_ports) - 1);
		for (i = 0; i < num_ports; i++) {
			if (usb2_suspended_ports & (1UL << i)) {
				port = usb2_rhub->ports[i];
				status = readl(port->addr);

				xhci_dbg(xhci,
					  "USB20: portsc[%i]: 0x%04X\n",
					  i, status);

				if (!(status & PORT_CONNECT))
					usb2_suspended_ports &= ~(1UL << i);
			}
		}

		if (usb2_suspended_ports) {
			ret = -EBUSY;
			goto ebusy;
		}
	}

	usb3_rhub = &xhci->usb3_rhub;
	if (usb3_rhub) {
		bus_state  = &usb3_rhub->bus_state;
		num_ports  = usb3_rhub->num_ports;
		usb3_suspended_ports  = bus_state->suspended_ports;
		usb3_suspended_ports ^= (BIT(num_ports) - 1);
		usb3_suspended_ports &= (BIT(num_ports) - 1);
		for (i = 0; i < num_ports; i++) {
			if (usb3_suspended_ports & BIT(i)) {
				port = usb3_rhub->ports[i];
				status = readl(port->addr);

				xhci_dbg(xhci, "USB3: portsc[%i]: 0x%04X\n",
					  i, status);

				if (!(status & PORT_CONNECT))
					usb3_suspended_ports &= ~BIT(i);
			}
		}

		if (usb3_suspended_ports) {
			ret = -EBUSY;
			goto ebusy;
		}
	}

ebusy:
	xhci_dbg(xhci, "%s: USB2: 0x%08X, USB3: 0x%08X ret: %i\n",
		  __func__, usb2_suspended_ports,
		  usb3_suspended_ports, ret);

	return ret;
}

static int __maybe_unused xhci_mtk_runtime_suspend(struct device *dev)
{
	bool wakeup = device_may_wakeup(dev);
	struct xhci_hcd_mtk  *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd;
	struct xhci_hcd *xhci;
	int ret = 0;

	if (!mtk->hcd)
		return -ESHUTDOWN;

	hcd = mtk->hcd;
	xhci = hcd_to_xhci(hcd);
	if ((xhci->xhc_state & XHCI_STATE_REMOVING) ||
			(xhci->xhc_state & XHCI_STATE_HALTED)) {
		return -ESHUTDOWN;
	}

	mtk->seal_status = SEAL_BUSY;
	ret = xhci_mtk_bus_status(dev);
	if (wakeup && !ret) {
		mtk->seal_status = SEAL_SUSPENDING;
		xhci_mtk_suspend(dev);
		xhci_mtk_seal_wakeup_enable(mtk, true);
		mtk->seal_status = SEAL_SUSPENDED;
		xhci_dbg(xhci, "%s: seals xHCI controller\n", __func__);
	}

	xhci_dbg(xhci, "%s: seals wakeup = %i, ret = %i!\n",
		  __func__, wakeup, ret);

	return ret;
}

static int __maybe_unused xhci_mtk_runtime_resume(struct device *dev)
{
	bool wakeup = device_may_wakeup(dev);
	struct xhci_hcd_mtk  *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd;
	struct xhci_hcd *xhci;

	if (!mtk->hcd)
		return -ESHUTDOWN;

	hcd = mtk->hcd;
	xhci = hcd_to_xhci(hcd);
	if ((xhci->xhc_state & XHCI_STATE_REMOVING) ||
			(xhci->xhc_state & XHCI_STATE_HALTED)) {
		return -ESHUTDOWN;
	}

	/*
	 *  list cases by one extra interrupt named seal to process!!!
	 *  Who to process these module reinitilization after SPM wakeup
	 *  case 1: usb remote wakeup, therefore xHCI need reinitilizate also.
	 *  case 2: other-wakeup-source wakeup, therefore, xHCI need reinit
	 *  case 3: usb client driver can invoke it in runtime mechanism
	 *  case 4: user active
	 */
	if (wakeup) {
		xhci_mtk_seal_wakeup_enable(mtk, false);
		xhci_mtk_resume(dev);
		xhci_dbg(xhci, "%s: unseals xHCI controller\n", __func__);
	}
	mtk->seal_status = SEAL_RESUMED;

	xhci_dbg(xhci, "%s: unseals wakeup = %i\n", __func__, wakeup);

	return 0;
}

static int __maybe_unused xhci_mtk_runtime_idle(struct device *dev)
{
	int ret = 0;

	dev_dbg(dev, "%s: xhci_mtk_runtime_ready %i",
		 __func__, xhci_mtk_runtime_ready);

	if (!xhci_mtk_runtime_ready)
		ret = -EAGAIN;

	return ret;
}

static const struct dev_pm_ops xhci_mtk_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(xhci_mtk_suspend, xhci_mtk_resume)
	SET_RUNTIME_PM_OPS(xhci_mtk_runtime_suspend,
			   xhci_mtk_runtime_resume,
			   xhci_mtk_runtime_idle)
};

#define DEV_PM_OPS (IS_ENABLED(CONFIG_PM) ? &xhci_mtk_pm_ops : NULL)

static const struct of_device_id mtk_xhci_of_match[] = {
	{ .compatible = "mediatek,mt8173-xhci"},
	{ .compatible = "mediatek,mtk-xhci"},
	{ },
};
MODULE_DEVICE_TABLE(of, mtk_xhci_of_match);

static struct platform_driver mtk_xhci_driver = {
	.probe	= xhci_mtk_probe,
	.remove	= xhci_mtk_remove,
	.driver	= {
		.name = "xhci-mtk",
		.pm = DEV_PM_OPS,
		.of_match_table = mtk_xhci_of_match,
	},
};

static int __init xhci_mtk_init(void)
{
	xhci_mtk_runtime_ready = 0;
	xhci_init_driver(&xhci_mtk_hc_driver, &xhci_mtk_overrides);
	return platform_driver_register(&mtk_xhci_driver);
}
module_init(xhci_mtk_init);

static void __exit xhci_mtk_exit(void)
{
	platform_driver_unregister(&mtk_xhci_driver);
}
module_exit(xhci_mtk_exit);

MODULE_AUTHOR("Chunfeng Yun <chunfeng.yun@mediatek.com>");
MODULE_DESCRIPTION("MediaTek xHCI Host Controller Driver");
MODULE_LICENSE("GPL v2");
