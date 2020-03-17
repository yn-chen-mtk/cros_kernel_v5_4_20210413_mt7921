// SPDX-License-Identifier: GPL-2.0
//
// Copyright (c) 2020 MediaTek Inc.
// Author: Weiyi Lu <weiyi.lu@mediatek.com>

#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/seq_file.h>

#include "clkdbg.h"

#define DUMP_INIT_STATE		0

/*
 * clkdbg dump_regs
 */

enum {
	topckgen,
	infracfg,
	pericfg,
	scpsys,
	apmixedsys,
	scp_adsp,
	imp_iic_wrap_c,
	audsys,
	imp_iic_wrap_e,
	imp_iic_wrap_s,
	imp_iic_wrap_ws,
	imp_iic_wrap_w,
	imp_iic_wrap_n,
	msdc_top,
	msdc,
	mfgcfg,
	mmsys,
	imgsys,
	imgsys2,
	vdecsys_soc,
	vdecsys,
	vencsys,
	apu_conn,
	apu_vcore,
	apu0,
	apu1,
	apu_mdla0,
	camsys,
	camsys_rawa,
	camsys_rawb,
	camsys_rawc,
	ipesys,
	mdpsys,
};

#define REGBASE_V(_phys, _id_name) { .phys = _phys, .name = #_id_name }

/*
 * checkpatch.pl ERROR:COMPLEX_MACRO
 *
 * #define REGBASE(_phys, _id_name) [_id_name] = REGBASE_V(_phys, _id_name)
 */

static struct regbase rb[] = {
	[topckgen] = REGBASE_V(0x10000000, topckgen),
	[infracfg] = REGBASE_V(0x10001000, infracfg),
	[pericfg] = REGBASE_V(0x10003000, pericfg),
	[scpsys]   = REGBASE_V(0x10006000, scpsys),
	[apmixedsys] = REGBASE_V(0x1000c000, apmixedsys),
	[scp_adsp] = REGBASE_V(0x10720000, scp_adsp),
	[imp_iic_wrap_c] = REGBASE_V(0x11007000, imp_iic_wrap_c),
	[audsys] = REGBASE_V(0x11210000, audsys),
	[imp_iic_wrap_e] = REGBASE_V(0x11cb1000, imp_iic_wrap_e),
	[imp_iic_wrap_s] = REGBASE_V(0x11d03000, imp_iic_wrap_s),
	[imp_iic_wrap_ws] = REGBASE_V(0x11d23000, imp_iic_wrap_ws),
	[imp_iic_wrap_w] = REGBASE_V(0x11e01000, imp_iic_wrap_w),
	[imp_iic_wrap_n] = REGBASE_V(0x11f02000, imp_iic_wrap_n),
	[msdc_top] = REGBASE_V(0x11f10000, msdc_top),
	[msdc] = REGBASE_V(0x11f60000, msdc),
	[mfgcfg] = REGBASE_V(0x13fbf000, mfgcfg),
	[mmsys] = REGBASE_V(0x14000000, mmsys),
	[imgsys] = REGBASE_V(0x15020000, imgsys),
	[imgsys2] = REGBASE_V(0x15820000, imgsys2),
	[vdecsys_soc] = REGBASE_V(0x1600f000, vdecsys_soc),
	[vdecsys] = REGBASE_V(0x1602f000, vdecsys),
	[vencsys] = REGBASE_V(0x17000000, vencsys),
	[apu_conn] = REGBASE_V(0x19020000, apu_conn),
	[apu_vcore] = REGBASE_V(0x19029000, apu_vcore),
	[apu0] = REGBASE_V(0x19030000, apu0),
	[apu1] = REGBASE_V(0x19031000, apu1),
	[apu_mdla0] = REGBASE_V(0x19034000, apu_mdla0),
	[camsys] = REGBASE_V(0x1a000000, camsys),
	[camsys_rawa] = REGBASE_V(0x1a04f000, camsys_rawa),
	[camsys_rawb] = REGBASE_V(0x1a06f000, camsys_rawb),
	[camsys_rawc] = REGBASE_V(0x1a08f000, camsys_rawc),
	[ipesys] = REGBASE_V(0x1b000000, ipesys),
	[mdpsys] = REGBASE_V(0x1f000000, mdpsys),
};

#define REGNAME(_base, _ofs, _name)	\
	{ .base = &rb[_base], .ofs = _ofs, .name = #_name }

static struct regname rn[] = {
	REGNAME(apmixedsys, 0x050, PLLON_CON0),
	REGNAME(apmixedsys, 0x054, PLLON_CON1),
	REGNAME(apmixedsys, 0x058, PLLON_CON2),
	REGNAME(apmixedsys, 0x05C, PLLON_CON3),
	REGNAME(apmixedsys, 0x208, ARMPLL_LL_CON0),
	REGNAME(apmixedsys, 0x20C, ARMPLL_LL_CON1),
	REGNAME(apmixedsys, 0x210, ARMPLL_LL_CON2),
	REGNAME(apmixedsys, 0x214, ARMPLL_LL_CON3),
	REGNAME(apmixedsys, 0x218, ARMPLL_BL0_CON0),
	REGNAME(apmixedsys, 0x21C, ARMPLL_BL0_CON1),
	REGNAME(apmixedsys, 0x220, ARMPLL_BL0_CON2),
	REGNAME(apmixedsys, 0x224, ARMPLL_BL0_CON3),
	REGNAME(apmixedsys, 0x228, ARMPLL_BL1_CON0),
	REGNAME(apmixedsys, 0x22C, ARMPLL_BL1_CON1),
	REGNAME(apmixedsys, 0x230, ARMPLL_BL1_CON2),
	REGNAME(apmixedsys, 0x234, ARMPLL_BL1_CON3),
	REGNAME(apmixedsys, 0x238, ARMPLL_BL2_CON0),
	REGNAME(apmixedsys, 0x23C, ARMPLL_BL2_CON1),
	REGNAME(apmixedsys, 0x240, ARMPLL_BL2_CON2),
	REGNAME(apmixedsys, 0x244, ARMPLL_BL2_CON3),
	REGNAME(apmixedsys, 0x248, ARMPLL_BL3_CON0),
	REGNAME(apmixedsys, 0x24C, ARMPLL_BL3_CON1),
	REGNAME(apmixedsys, 0x250, ARMPLL_BL3_CON2),
	REGNAME(apmixedsys, 0x254, ARMPLL_BL3_CON3),
	REGNAME(apmixedsys, 0x258, CCIPLL_CON0),
	REGNAME(apmixedsys, 0x25C, CCIPLL_CON1),
	REGNAME(apmixedsys, 0x260, CCIPLL_CON2),
	REGNAME(apmixedsys, 0x264, CCIPLL_CON3),
	REGNAME(apmixedsys, 0x268, MFGPLL_CON0),
	REGNAME(apmixedsys, 0x26c, MFGPLL_CON1),
	REGNAME(apmixedsys, 0x270, MFGPLL_CON2),
	REGNAME(apmixedsys, 0x274, MFGPLL_PWR_CON),
	REGNAME(apmixedsys, 0x308, UNIVPLL_CON0),
	REGNAME(apmixedsys, 0x30c, UNIVPLL_CON1),
	REGNAME(apmixedsys, 0x310, UNIVPLL_CON2),
	REGNAME(apmixedsys, 0x314, UNIVPLL_PWR_CON),
	REGNAME(apmixedsys, 0x318, APLL1_CON0),
	REGNAME(apmixedsys, 0x31c, APLL1_CON1),
	REGNAME(apmixedsys, 0x320, APLL1_CON2),
	REGNAME(apmixedsys, 0x328, APLL1_PWR_CON),
	REGNAME(apmixedsys, 0x32c, APLL2_CON0),
	REGNAME(apmixedsys, 0x330, APLL2_CON1),
	REGNAME(apmixedsys, 0x334, APLL2_CON2),
	REGNAME(apmixedsys, 0x33c, APLL2_PWR_CON),
	REGNAME(apmixedsys, 0x340, MAINPLL_CON0),
	REGNAME(apmixedsys, 0x344, MAINPLL_CON1),
	REGNAME(apmixedsys, 0x348, MAINPLL_CON2),
	REGNAME(apmixedsys, 0x34c, MAINPLL_PWR_CON),
	REGNAME(apmixedsys, 0x350, MSDCPLL_CON0),
	REGNAME(apmixedsys, 0x354, MSDCPLL_CON1),
	REGNAME(apmixedsys, 0x358, MSDCPLL_CON2),
	REGNAME(apmixedsys, 0x35c, MSDCPLL_PWR_CON),
	REGNAME(apmixedsys, 0x360, MMPLL_CON0),
	REGNAME(apmixedsys, 0x364, MMPLL_CON1),
	REGNAME(apmixedsys, 0x368, MMPLL_CON2),
	REGNAME(apmixedsys, 0x36c, MMPLL_PWR_CON),
	REGNAME(apmixedsys, 0x370, ADSPPLL_CON0),
	REGNAME(apmixedsys, 0x374, ADSPPLL_CON1),
	REGNAME(apmixedsys, 0x378, ADSPPLL_CON2),
	REGNAME(apmixedsys, 0x37c, ADSPPLL_PWR_CON),
	REGNAME(apmixedsys, 0x380, TVDPLL_CON0),
	REGNAME(apmixedsys, 0x384, TVDPLL_CON1),
	REGNAME(apmixedsys, 0x388, TVDPLL_CON2),
	REGNAME(apmixedsys, 0x38c, TVDPLL_PWR_CON),
	REGNAME(apmixedsys, 0x390, MPLL_CON0),
	REGNAME(apmixedsys, 0x394, MPLL_CON1),
	REGNAME(apmixedsys, 0x39C, MPLL_CON3),
	REGNAME(apmixedsys, 0x3a0, APUPLL_CON0),
	REGNAME(apmixedsys, 0x3a4, APUPLL_CON1),
	REGNAME(apmixedsys, 0x3a8, APUPLL_CON2),
	REGNAME(apmixedsys, 0x3ac, APUPLL_PWR_CON),
	REGNAME(apmixedsys, 0x3b4, NPUPLL_CON0),
	REGNAME(apmixedsys, 0x3b8, NPUPLL_CON1),
	REGNAME(apmixedsys, 0x3bc, NPUPLL_CON2),
	REGNAME(apmixedsys, 0x3c0, NPUPLL_PWR_CON),
	REGNAME(apmixedsys, 0x3c4, USBPLL_CON0),
	REGNAME(apmixedsys, 0x3c8, USBPLL_CON1),
	REGNAME(apmixedsys, 0x3cc, USBPLL_CON2),
	REGNAME(apmixedsys, 0x3cc, USBPLL_PWR_CON),
	REGNAME(topckgen, 0x010, CLK_CFG_0),
	REGNAME(topckgen, 0x020, CLK_CFG_1),
	REGNAME(topckgen, 0x030, CLK_CFG_2),
	REGNAME(topckgen, 0x040, CLK_CFG_3),
	REGNAME(topckgen, 0x050, CLK_CFG_4),
	REGNAME(topckgen, 0x060, CLK_CFG_5),
	REGNAME(topckgen, 0x070, CLK_CFG_6),
	REGNAME(topckgen, 0x080, CLK_CFG_7),
	REGNAME(topckgen, 0x090, CLK_CFG_8),
	REGNAME(topckgen, 0x0a0, CLK_CFG_9),
	REGNAME(topckgen, 0x0b0, CLK_CFG_10),
	REGNAME(topckgen, 0x0c0, CLK_CFG_11),
	REGNAME(topckgen, 0x0d0, CLK_CFG_12),
	REGNAME(topckgen, 0x0e0, CLK_CFG_13),
	REGNAME(topckgen, 0x0f0, CLK_CFG_14),
	REGNAME(topckgen, 0x100, CLK_CFG_15),
	REGNAME(topckgen, 0x110, CLK_CFG_16),
	REGNAME(topckgen, 0x320, CLK_AUDDIV_0),
	REGNAME(scpsys, 0x0000, POWERON_CONFIG_EN),
	REGNAME(scpsys, 0x016C, PWR_STATUS),
	REGNAME(scpsys, 0x0170, PWR_STATUS_2ND),
	REGNAME(scpsys, 0x0178, OTHER_PWR_STATUS),
	REGNAME(scpsys, 0x300, MD1_PWR_CON),
	REGNAME(scpsys, 0x304, CONN_PWR_CON),
	REGNAME(scpsys, 0x308, MFG0_PWR_CON),
	REGNAME(scpsys, 0x30C, MFG1_PWR_CON),
	REGNAME(scpsys, 0x310, MFG2_PWR_CON),
	REGNAME(scpsys, 0x314, MFG3_PWR_CON),
	REGNAME(scpsys, 0x318, MFG4_PWR_CON),
	REGNAME(scpsys, 0x31C, MFG5_PWR_CON),
	REGNAME(scpsys, 0x320, MFG6_PWR_CON),
	REGNAME(scpsys, 0x324, IFR_PWR_CON),
	REGNAME(scpsys, 0x328, IFR_SUB_PWR_CON),
	REGNAME(scpsys, 0x32C, DPY_PWR_CON),
	REGNAME(scpsys, 0x330, ISP_PWR_CON),
	REGNAME(scpsys, 0x334, ISP2_PWR_CON),
	REGNAME(scpsys, 0x338, IPE_PWR_CON),
	REGNAME(scpsys, 0x33C, VDE_PWR_CON),
	REGNAME(scpsys, 0x340, VDE2_PWR_CON),
	REGNAME(scpsys, 0x344, VEN_PWR_CON),
	REGNAME(scpsys, 0x348, VEN_CORE1_PWR_CON),
	REGNAME(scpsys, 0x34C, MDP_PWR_CON),
	REGNAME(scpsys, 0x350, DIS_PWR_CON),
	REGNAME(scpsys, 0x354, AUDIO_PWR_CON),
	REGNAME(scpsys, 0x358, ADSP_PWR_CON),
	REGNAME(scpsys, 0x35C, CAM_PWR_CON),
	REGNAME(scpsys, 0x360, CAM_RAWA_PWR_CON),
	REGNAME(scpsys, 0x364, CAM_RAWB_PWR_CON),
	REGNAME(scpsys, 0x368, CAM_RAWC_PWR_CON),
	REGNAME(scpsys, 0x3AC, DP_TX_PWR_CON),
	REGNAME(scpsys, 0x3C4, DPY2_PWR_CON),
	REGNAME(scpsys, 0x398, MD_EXT_BUCK_ISO_CON),
	REGNAME(scpsys, 0x39C, EXT_BUCK_ISO),
	REGNAME(scpsys, 0x3A4, MSDC_PWR_CON),
	REGNAME(infracfg, 0x090, MODULE_SW_CG_0),
	REGNAME(infracfg, 0x094, MODULE_SW_CG_1),
	REGNAME(infracfg, 0x0ac, MODULE_SW_CG_2),
	REGNAME(infracfg, 0x0c8, MODULE_SW_CG_3),
	REGNAME(infracfg, 0x0d8, MODULE_SW_CG_5),
	REGNAME(infracfg, 0x0e8, MODULE_SW_CG_4),
	REGNAME(pericfg, 0x20c, PERIAXI_SI0_CTL),
	REGNAME(scp_adsp, 0x180, AUDIODSP_CK_CG),
	REGNAME(imp_iic_wrap_c, 0xe00, AP_CLOCK_CG_RO_CEN),
	REGNAME(audsys, 0x000, AUDIO_TOP_CON0),
	REGNAME(audsys, 0x004, AUDIO_TOP_CON1),
	REGNAME(audsys, 0x008, AUDIO_TOP_CON2),
	REGNAME(imp_iic_wrap_e, 0xe00, AP_CLOCK_CG_RO_EST),
	REGNAME(imp_iic_wrap_s, 0xe00, AP_CLOCK_CG_RO_SOU),
	REGNAME(imp_iic_wrap_ws, 0xe00, AP_CLOCK_CG_RO_WEST_SOU),
	REGNAME(imp_iic_wrap_w, 0xe00, AP_CLOCK_CG_RO_WST),
	REGNAME(imp_iic_wrap_n, 0xe00, AP_CLOCK_CG_RO_NOR),
	REGNAME(msdc_top, 0x000, MSDC_CTL_CKEN),
	REGNAME(msdc, 0x0b4, PATCH_BIT1),
	REGNAME(mfgcfg, 0x000, MFG_CG),
	REGNAME(mmsys, 0x100, MMSYS_CG_CON0),
	REGNAME(mmsys, 0x110, MMSYS_CG_CON1),
	REGNAME(mmsys, 0x1a0, MMSYS_CG_CON2),
	REGNAME(imgsys, 0x000, IMG1_CG_CON),
	REGNAME(imgsys2, 0x000, IMG2_CG_CON),
	REGNAME(vdecsys_soc, 0x000, VDEC_CKEN),
	REGNAME(vdecsys_soc, 0x008, LARB_CKEN_CON),
	REGNAME(vdecsys_soc, 0x200, LAT_CKEN),
	REGNAME(vdecsys, 0x000, VDEC_CKEN),
	REGNAME(vdecsys, 0x008, LARB_CKEN_CON),
	REGNAME(vdecsys, 0x200, LAT_CKEN),
	REGNAME(vencsys, 0x000, VENCSYS_CG),
	REGNAME(apu_conn, 0x000, APU_CONN_CG),
	REGNAME(apu_vcore, 0x000, APUSYS_VCORE_CG),
	REGNAME(apu0, 0x100, CORE0_CG),
	REGNAME(apu1, 0x100, CORE1_CG),
	REGNAME(apu_mdla0, 0x000, MDLA_CG),
	REGNAME(camsys, 0x000, CAMSYS_CG_CON),
	REGNAME(camsys_rawa, 0x000, CAMSYS_RAWA_CG_CON),
	REGNAME(camsys_rawb, 0x000, CAMSYS_RAWB_CG_CON),
	REGNAME(camsys_rawc, 0x000, CAMSYS_RAWC_CG_CON),
	REGNAME(ipesys, 0x000, IMG_CG),
	REGNAME(mdpsys, 0x100, MDPSYS_CG_CON0),
	REGNAME(mdpsys, 0x120, MDPSYS_CG_CON2),
	{}
};

static const struct regname *get_all_regnames(void)
{
	return rn;
}

static void __init init_regbase(void)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(rb); i++)
		rb[i].virt = ioremap(rb[i].phys, PAGE_SIZE);
}

/*
 * clkdbg fmeter
 */

#include <linux/delay.h>

#define clk_readl(addr)		readl(addr)
#define clk_writel(addr, val)	\
	do { writel(val, addr); wmb(); } while (0) /* sync write */

#define FMCLK(_t, _i, _n) { .type = _t, .id = _i, .name = _n }

static const struct fmeter_clk fclks[] = {
	FMCLK(CKGEN,  1, "hd_faxi_ck"),
	FMCLK(CKGEN,  2, "hg_fspm_ck"),
	FMCLK(CKGEN,  3, "hf_fscp_ck"),
	FMCLK(CKGEN,  4, "hd_fbus_aximem_ck"),
	FMCLK(CKGEN,  5, "hf_fdisp_ck"),
	FMCLK(CKGEN,  6, "hf_fmdp_ck"),
	FMCLK(CKGEN,  7, "hf_fimg1_ck"),
	FMCLK(CKGEN,  8, "hf_fimg2_ck"),
	FMCLK(CKGEN,  9, "hf_fipe_ck"),
	FMCLK(CKGEN, 10, "hf_fdpe_ck"),
	FMCLK(CKGEN, 11, "hf_fcam_ck"),
	FMCLK(CKGEN, 12, "hf_fccu_ck"),
	FMCLK(CKGEN, 13, "hf_fdsp_ck"),
	FMCLK(CKGEN, 14, "hf_fdsp1_ck"),
	FMCLK(CKGEN, 15, "hf_fdsp2_ck"),
	FMCLK(CKGEN, 16, "hf_fdsp5_ck"),
	FMCLK(CKGEN, 17, "hf_fdsp7_ck"),
	FMCLK(CKGEN, 18, "hf_fipu_if_ck"),
	FMCLK(CKGEN, 19, "hf_fmfg_ck"),
	FMCLK(CKGEN, 20, "f_fcamtg_ck"),
	FMCLK(CKGEN, 21, "f_fcamtg2_ck"),
	FMCLK(CKGEN, 22, "f_fcamtg3_ck"),
	FMCLK(CKGEN, 23, "f_fcamtg4_ck"),
	FMCLK(CKGEN, 24, "f_fcamtg5_ck"),
	FMCLK(CKGEN, 25, "f_fcamtg6_ck"),
	FMCLK(CKGEN, 26, "f_fuart_ck"),
	FMCLK(CKGEN, 27, "hf_fspi_ck"),
	FMCLK(CKGEN, 28, "hf_fmsdc50_0_hclk_ck"),
	FMCLK(CKGEN, 29, "hf_fmsdc50_0_ck"),
	FMCLK(CKGEN, 30, "hf_fmsdc30_1_ck"),
	FMCLK(CKGEN, 31, "hf_fmsdc30_2_ck"),
	FMCLK(CKGEN, 32, "hf_faudio_ck"),
	FMCLK(CKGEN, 33, "hf_faud_intbus_ck"),
	FMCLK(CKGEN, 34, "f_fpwrap_ulposc_ck"),
	FMCLK(CKGEN, 35, "hf_fatb_ck"),
	FMCLK(CKGEN, 36, "hf_fpwrmcu_ck"),
	FMCLK(CKGEN, 37, "hf_fdpi_ck"),
	FMCLK(CKGEN, 38, "hf_fscam_ck"),
	FMCLK(CKGEN, 39, "f_fdisp_pwm_ck"),
	FMCLK(CKGEN, 40, "f_fusb_top_ck"),
	FMCLK(CKGEN, 41, "f_fssusb_xhci_ck"),
	FMCLK(CKGEN, 42, "f_fi2c_ck"),
	FMCLK(CKGEN, 43, "f_fseninf_ck"),
	FMCLK(CKGEN, 44, "f_fseninf1_ck"),
	FMCLK(CKGEN, 45, "f_fseninf2_ck"),
	FMCLK(CKGEN, 46, "f_fseninf3_ck"),
	FMCLK(CKGEN, 47, "hf_ftl_ck"),
	FMCLK(CKGEN, 48, "hf_fdxcc_ck"),
	FMCLK(CKGEN, 49, "hf_faud_engen1_ck"),
	FMCLK(CKGEN, 50, "hf_faud_engen2_ck"),
	FMCLK(CKGEN, 51, "hf_faes_ufsfde_ck"),
	FMCLK(CKGEN, 52, "hf_fufs_ck"),
	FMCLK(CKGEN, 53, "hf_faud_1_ck"),
	FMCLK(CKGEN, 54, "hf_faud_2_ck"),
	FMCLK(CKGEN, 55, "hf_fadsp_ck"),
	FMCLK(CKGEN, 56, "hf_fdpmaif_main_ck"),
	FMCLK(CKGEN, 57, "hf_fvenc_ck"),
	FMCLK(CKGEN, 58, "hf_fvdec_ck"),
	FMCLK(CKGEN, 59, "hf_fcamtm_ck"),
	FMCLK(CKGEN, 60, "hf_fpwm_ck"),
	FMCLK(CKGEN, 61, "hf_faudio_h_ck"),
	FMCLK(CKGEN, 62, "hf_fspmi_mst_ck"),
	FMCLK(CKGEN, 63, "hg_fdvfsrc_ck"),
	FMCLK(ABIST,  1, "AD_ADSPPLL_CK"),
	FMCLK(ABIST,  2, "AD_APLL1_CK"),
	FMCLK(ABIST,  3, "AD_APLL2_CK"),
	FMCLK(ABIST,  4, "AD_APPLLGP_MON_FM_CK"),
	FMCLK(ABIST,  5, "AD_APUPLL_CK"),
	FMCLK(ABIST,  6, "AD_ARMPLL_BL_CK"),
	FMCLK(ABIST,  7, "AD_NPUPLL_CK"),
	FMCLK(ABIST, 10, "AD_ARMPLL_LL_CK"),
	FMCLK(ABIST, 11, "AD_CCIPLL_CK"),
	FMCLK(ABIST, 12, "AD_CSI0A_CDPHY_DELAYCAL_CK"),
	FMCLK(ABIST, 13, "AD_CSI0B_CDPHY_DELAYCAL_CK"),
	FMCLK(ABIST, 14, "AD_CSI1A_DPHY_DELAYCAL_CK"),
	FMCLK(ABIST, 15, "AD_CSI1B_DPHY_DELAYCAL_CK"),
	FMCLK(ABIST, 16, "AD_CSI2A_DPHY_DELAYCAL_CK"),
	FMCLK(ABIST, 17, "AD_CSI2B_DPHY_DELAYCAL_CK"),
	FMCLK(ABIST, 18, "AD_CSI3A_DPHY_DELAYCAL_CK"),
	FMCLK(ABIST, 19, "AD_CSI3B_DPHY_DELAYCAL_CK"),
	FMCLK(ABIST, 20, "AD_DSI0_LNTC_DSICLK"),
	FMCLK(ABIST, 21, "AD_DSI0_MPPLL_TST_CK"),
	FMCLK(ABIST, 23, "mfgpll_ck"),
	FMCLK(ABIST, 24, "AD_MAINPLL_CK"),
	FMCLK(ABIST, 25, "AD_MDPLL_FS26M_CK"),
	FMCLK(ABIST, 26, "AD_MGPLL_CK"),
	FMCLK(ABIST, 27, "AD_MPLL_CK"),
	FMCLK(ABIST, 28, "AD_MMPLL_D3_CK"),
	FMCLK(ABIST, 29, "AD_MPLL_CK"),
	FMCLK(ABIST, 30, "AD_MSDCPLL_CK"),
	FMCLK(ABIST, 31, "AD_RCLRPLL_DIV4_CK_ch2"),
	FMCLK(ABIST, 32, "AD_RCLRPLL_DIV4_CK_ch13"),
	FMCLK(ABIST, 33, "AD_RPHYPLL_DIV4_CK_ch2"),
	FMCLK(ABIST, 34, "AD_RPHYPLL_DIV4_CK_ch13"),
	FMCLK(ABIST, 35, "AD_TVDPLL_CK"),
	FMCLK(ABIST, 36, "AD_ULPOSC2_CK"),
	FMCLK(ABIST, 37, "AD_ULPOSC_CK"),
	FMCLK(ABIST, 38, "AD_UNIVPLL_CK"),
	FMCLK(ABIST, 39, "AD_USB20_192M_CK"),
	FMCLK(ABIST, 40, "AD_USBPLL_192M_CK"),
	FMCLK(ABIST, 41, "UFS_MP_CLK2FREQ"),
	FMCLK(ABIST, 42, "ad_wbg_dig_bpll_ck"),
	FMCLK(ABIST, 43, "ad_wbg_dig_wpll_ck960"),
	FMCLK(ABIST, 44, "fmem_ck_aft_dcm_ch0"),
	FMCLK(ABIST, 45, "fmem_ck_aft_dcm_ch1"),
	FMCLK(ABIST, 46, "fmem_ck_aft_dcm_ch2"),
	FMCLK(ABIST, 47, "fmem_ck_aft_dcm_ch3"),
	FMCLK(ABIST, 48, "fmem_ck_bfe_dcm_ch0"),
	FMCLK(ABIST, 49, "fmem_ck_bfe_dcm_ch1"),
	FMCLK(ABIST, 50, "hd_466m_fmem_ck_infrasys"),
	FMCLK(ABIST, 51, "mcusys_arm_clk_out_all"),
	FMCLK(ABIST, 52, "msdc01_in_ck"),
	FMCLK(ABIST, 53, "msdc02_in_ck"),
	FMCLK(ABIST, 54, "msdc11_in_ck"),
	FMCLK(ABIST, 55, "msdc12_in_ck"),
	FMCLK(ABIST, 56, "msdc21_in_ck"),
	FMCLK(ABIST, 57, "msdc22_in_ck"),
	FMCLK(ABIST, 58, "rtc32k_ck_i"),
	FMCLK(ABIST, 60, "ckmon1_ck"),
	FMCLK(ABIST, 61, "ckmon2_ck"),
	FMCLK(ABIST, 62, "ckmon3_ck"),
	FMCLK(ABIST, 63, "ckmon4_ck"),
	{}
};

#define CLK_MISC_CFG_0	(rb[topckgen].virt + 0x140)
#define CLK_DBG_CFG		(rb[topckgen].virt + 0x17C)
#define CLK26CALI_0		(rb[topckgen].virt + 0x220)
#define CLK26CALI_1		(rb[topckgen].virt + 0x224)

static unsigned int mt_get_ckgen_freq(unsigned int ID)
{
	int output = 0, i = 0;
	unsigned int temp, clk_dbg_cfg, clk_misc_cfg_0, clk26cali_1 = 0;

	clk_dbg_cfg = clk_readl(CLK_DBG_CFG);
	clk_writel(CLK_DBG_CFG, (clk_dbg_cfg & 0xFFFFC0FC)|(ID << 8)|(0x1));

	clk_misc_cfg_0 = clk_readl(CLK_MISC_CFG_0);
	clk_writel(CLK_MISC_CFG_0, (clk_misc_cfg_0 & 0x00FFFFFF));

	clk26cali_1 = clk_readl(CLK26CALI_1);
	clk_writel(CLK26CALI_0, 0x1000);
	clk_writel(CLK26CALI_0, 0x1010);

	/* wait frequency meter finish */
	while (clk_readl(CLK26CALI_0) & 0x10) {
		udelay(10);
		i++;
		if (i > 20)
			break;
	}
	/* illegal pass */
	if (i == 0) {
		clk_writel(CLK26CALI_0, 0x0000);
		//re-trigger
		clk_writel(CLK26CALI_0, 0x1000);
		clk_writel(CLK26CALI_0, 0x1010);
		while (clk_readl(CLK26CALI_0) & 0x10) {
			udelay(10);
			i++;
			if (i > 20)
				break;
		}
	}

	temp = clk_readl(CLK26CALI_1) & 0xFFFF;

	output = (temp * 26000) / 1024;

	clk_writel(CLK_DBG_CFG, clk_dbg_cfg);
	clk_writel(CLK_MISC_CFG_0, clk_misc_cfg_0);
	/*clk_writel(CLK26CALI_0, clk26cali_0);*/
	/*clk_writel(CLK26CALI_1, clk26cali_1);*/

	clk_writel(CLK26CALI_0, 0x0000);
	/*print("ckgen meter[%d] = %d Khz\n", ID, output);*/
	if (i > 20)
		return 0;
	else
		return output;
}

static unsigned int mt_get_abist_freq(unsigned int ID)
{
	int output = 0, i = 0;
	unsigned int temp, clk_dbg_cfg, clk_misc_cfg_0, clk26cali_1 = 0;

	clk_dbg_cfg = clk_readl(CLK_DBG_CFG);
	clk_writel(CLK_DBG_CFG, (clk_dbg_cfg & 0xFFC0FFFC)|(ID << 16));

	clk_misc_cfg_0 = clk_readl(CLK_MISC_CFG_0);
	clk_writel(CLK_MISC_CFG_0, (clk_misc_cfg_0 & 0x00FFFFFF) | (1 << 24));

	clk26cali_1 = clk_readl(CLK26CALI_1);

	clk_writel(CLK26CALI_0, 0x1000);
	clk_writel(CLK26CALI_0, 0x1010);

	/* wait frequency meter finish */
	while (clk_readl(CLK26CALI_0) & 0x10) {
		udelay(10);
		i++;
		if (i > 20)
			break;
	}
	/* illegal pass */
	if (i == 0) {
		clk_writel(CLK26CALI_0, 0x0000);
		//re-trigger
		clk_writel(CLK26CALI_0, 0x1000);
		clk_writel(CLK26CALI_0, 0x1010);
		while (clk_readl(CLK26CALI_0) & 0x10) {
			udelay(10);
			i++;
			if (i > 20)
				break;
		}
	}

	temp = clk_readl(CLK26CALI_1) & 0xFFFF;

	output = (temp * 26000) / 1024;

	clk_writel(CLK_DBG_CFG, clk_dbg_cfg);
	clk_writel(CLK_MISC_CFG_0, clk_misc_cfg_0);
	/*clk_writel(CLK26CALI_0, clk26cali_0);*/
	/*clk_writel(CLK26CALI_1, clk26cali_1);*/
	clk_writel(CLK26CALI_0, 0x0000);
	/*pr_debug("%s = %d Khz\n", abist_array[ID-1], output);*/
	if (i > 20)
		return 0;
	else
		return (output * 2);
}

static u32 fmeter_freq_op(const struct fmeter_clk *fclk)
{
	if (fclk->type == ABIST)
		return mt_get_abist_freq(fclk->id);
	else if (fclk->type == CKGEN)
		return mt_get_ckgen_freq(fclk->id);
	return 0;
}

static const struct fmeter_clk *get_all_fmeter_clks(void)
{
	return fclks;
}

/*
 * clkdbg dump_state
 */

static const char * const *get_all_clk_names(void)
{
	static const char * const clks[] = {
		"clk26m",
		"clk32k",
		"mainpll",
		"univpll",
		"usbpll",
		"msdcpll",
		"mmpll",
		"adsppll",
		"mfgpll",
		"tvdpll",
		"apll1",
		"apll2",
		"apupll",
		"npupll",
		"mipid26m",
		"axi_sel",
		"spm_sel",
		"scp_sel",
		"bus_aximem_sel",
		"disp_sel",
		"mdp_sel",
		"img1_sel",
		"img2_sel",
		"ipe_sel",
		"dpe_sel",
		"cam_sel",
		"ccu_sel",
		"dsp_sel",
		"dsp1_sel",
		"dsp1_npupll_sel",
		"dsp2_sel",
		"dsp2_npupll_sel",
		"dsp5_sel",
		"dsp5_apupll_sel",
		"dsp7_sel",
		"ipu_if_sel",
		"mfg_ref_sel",
		"mfg_pll_sel",
		"camtg_sel",
		"camtg2_sel",
		"camtg3_sel",
		"camtg4_sel",
		"camtg5_sel",
		"camtg6_sel",
		"uart_sel",
		"spi_sel",
		"msdc50_0_h_sel",
		"msdc50_0_sel",
		"msdc30_1_sel",
		"msdc30_2_sel",
		"audio_sel",
		"aud_intbus_sel",
		"pwrap_ulposc_sel",
		"atb_sel",
		"sspm_sel",
		"dpi_sel",
		"scam_sel",
		"disp_pwm_sel",
		"usb_top_sel",
		"ssusb_xhci_sel",
		"i2c_sel",
		"seninf_sel",
		"seninf1_sel",
		"seninf2_sel",
		"seninf3_sel",
		"tl_sel",
		"dxcc_sel",
		"aud_engen1_sel",
		"aud_engen2_sel",
		"aes_ufsfde_sel",
		"ufs_sel",
		"aud_1_sel",
		"aud_2_sel",
		"adsp_sel",
		"dpmaif_main_sel",
		"venc_sel",
		"vdec_sel",
		"camtm_sel",
		"pwm_sel",
		"audio_h_sel",
		"spmi_mst_sel",
		"dvfsrc_sel",
		"aes_msdcfde_sel",
		"mcupm_sel",
		"sflash_sel",
		"apll_i2s0_m_sel",
		"apll_i2s1_m_sel",
		"apll_i2s2_m_sel",
		"apll_i2s3_m_sel",
		"apll_i2s4_m_sel",
		"apll_i2s5_m_sel",
		"apll_i2s6_m_sel",
		"apll_i2s7_m_sel",
		"apll_i2s8_m_sel",
		"apll_i2s9_m_sel",
		"mainpll_d3",
		"mainpll_d4",
		"mainpll_d4_d2",
		"mainpll_d4_d4",
		"mainpll_d4_d8",
		"mainpll_d4_d16",
		"mainpll_d5",
		"mainpll_d5_d2",
		"mainpll_d5_d4",
		"mainpll_d5_d8",
		"mainpll_d6",
		"mainpll_d6_d2",
		"mainpll_d6_d4",
		"mainpll_d7",
		"mainpll_d7_d2",
		"mainpll_d7_d4",
		"mainpll_d7_d8",
		"univpll_d3",
		"univpll_d4",
		"univpll_d4_d2",
		"univpll_d4_d4",
		"univpll_d4_d8",
		"univpll_d5",
		"univpll_d5_d2",
		"univpll_d5_d4",
		"univpll_d5_d8",
		"univpll_d6",
		"univpll_d6_d2",
		"univpll_d6_d4",
		"univpll_d6_d8",
		"univpll_d6_d16",
		"univpll_d7",
		"apll1_ck",
		"apll1_d2",
		"apll1_d4",
		"apll1_d8",
		"apll2_ck",
		"apll2_d2",
		"apll2_d4",
		"apll2_d8",
		"mmpll_d4",
		"mmpll_d4_d2",
		"mmpll_d5",
		"mmpll_d5_d2",
		"mmpll_d6",
		"mmpll_d6_d2",
		"mmpll_d7",
		"mmpll_d9",
		"apupll_ck",
		"npupll_ck",
		"tvdpll_ck",
		"tvdpll_d2",
		"tvdpll_d4",
		"tvdpll_d8",
		"tvdpll_d16",
		"msdcpll_ck",
		"msdcpll_d2",
		"msdcpll_d4",
		"ulposc",
		"osc_d2",
		"osc_d4",
		"osc_d8",
		"osc_d10",
		"osc_d16",
		"osc_d20",
		"csw_f26m_d2",
		"adsppll_ck",
		"univpll_192m",
		"univpll_192m_d2",
		"univpll_192m_d4",
		"univpll_192m_d8",
		"univpll_192m_d16",
		"univpll_192m_d32",
		"apll12_div0",
		"apll12_div1",
		"apll12_div2",
		"apll12_div3",
		"apll12_div4",
		"apll12_divb",
		"apll12_div5",
		"apll12_div6",
		"apll12_div7",
		"apll12_div8",
		"apll12_div9",
		"infra_pmic_tmr",
		"infra_pmic_ap",
		"infra_pmic_md",
		"infra_pmic_conn",
		"infra_scpsys",
		"infra_sej",
		"infra_apxgpt",
		"infra_mcupm",
		"infra_gce",
		"infra_gce2",
		"infra_therm",
		"infra_i2c0",
		"infra_ap_dma_pseudo",
		"infra_i2c2",
		"infra_i2c3",
		"infra_pwm_h",
		"infra_pwm1",
		"infra_pwm2",
		"infra_pwm3",
		"infra_pwm4",
		"infra_pwm",
		"infra_uart0",
		"infra_uart1",
		"infra_uart2",
		"infra_uart3",
		"infra_gce_26m",
		"infra_cq_dma_fpc",
		"infra_btif",
		"infra_spi0",
		"infra_msdc0",
		"infra_msdc1",
		"infra_msdc2",
		"infra_msdc0_src",
		"infra_dvfsrc",
		"infra_gcpu",
		"infra_trng",
		"infra_auxadc",
		"infra_cpum",
		"infra_ccif1_ap",
		"infra_ccif1_md",
		"infra_auxadc_md",
		"infra_pcie_tl_26m",
		"infra_msdc1_src",
		"infra_msdc2_src",
		"infra_pcie_tl_96m",
		"infra_pcie_pl_p_250m",
		"infra_device_apc",
		"infra_ccif_ap",
		"infra_debugsys",
		"infra_audio",
		"infra_ccif_md",
		"infra_dxcc_sec_core",
		"infra_dxcc_ao",
		"infra_dbg_trace",
		"infra_devmpu_b",
		"infra_dramc_f26m",
		"infra_irtx",
		"infra_ssusb",
		"infra_disp_pwm",
		"infra_cldma_b",
		"infra_audio_26m_b",
		"infra_modem_temp_share",
		"infra_spi1",
		"infra_i2c4",
		"infra_spi2",
		"infra_spi3",
		"infra_unipro_sys",
		"infra_unipro_tick",
		"infra_ufs_mp_sap_b",
		"infra_md32_b",
		"infra_sspm",
		"infra_unipro_mbist",
		"infra_sspm_bus_h",
		"infra_i2c5",
		"infra_i2c5_arbiter",
		"infra_i2c5_imm",
		"infra_i2c1_arbiter",
		"infra_i2c1_imm",
		"infra_i2c2_arbiter",
		"infra_i2c2_imm",
		"infra_spi4",
		"infra_spi5",
		"infra_cq_dma",
		"infra_ufs",
		"infra_aes_ufsfde",
		"infra_ufs_tick",
		"infra_ssusb_xhci",
		"infra_msdc0_self",
		"infra_msdc1_self",
		"infra_msdc2_self",
		"infra_sspm_26m_self",
		"infra_sspm_32k_self",
		"infra_ufs_axi",
		"infra_i2c6",
		"infra_ap_msdc0",
		"infra_md_msdc0",
		"infra_ccif5_ap",
		"infra_ccif5_md",
		"infra_pcie_top_h_133m",
		"infra_flashif_top_h_133m",
		"infra_pcie_peri_26m",
		"infra_ccif2_ap",
		"infra_ccif2_md",
		"infra_ccif3_ap",
		"infra_ccif3_md",
		"infra_sej_f13m",
		"infra_aes",
		"infra_i2c7",
		"infra_i2c8",
		"infra_fbist2fpc",
		"infra_device_apc_sync",
		"infra_dpmaif_main",
		"infra_pcie_tl_32k",
		"infra_ccif4_ap",
		"infra_ccif4_md",
		"infra_spi6",
		"infra_spi7",
		"infra_133m",
		"infra_66m",
		"infra_66m_peri_bus",
		"infra_free_dcm_133m",
		"infra_free_dcm_66m",
		"infra_peri_bus_dcm_133m",
		"infra_peri_bus_dcm_66m",
		"infra_flashif_peri_26m",
		"infra_flashif_fsflash",
		"infra_ap_dma",
		"peri_periaxi",
		"scp_adsp_audiodsp",
		"imp_iic_wrap_c_i2c10",
		"imp_iic_wrap_c_i2c11",
		"imp_iic_wrap_c_i2c12",
		"imp_iic_wrap_c_i2c13",
		"aud_afe",
		"aud_22m",
		"aud_24m",
		"aud_apll2_tuner",
		"aud_apll_tuner",
		"aud_tdm",
		"aud_adc",
		"aud_dac",
		"aud_dac_predis",
		"aud_tml",
		"aud_nle",
		"aud_i2s1_b",
		"aud_i2s2_b",
		"aud_i2s3_b",
		"aud_i2s4_b",
		"aud_connsys_i2s_asrc",
		"aud_general1_asrc",
		"aud_general2_asrc",
		"aud_dac_hires",
		"aud_adc_hires",
		"aud_adc_hires_tml",
		"aud_adda6_adc",
		"aud_adda6_adc_hires",
		"aud_3rd_dac",
		"aud_3rd_dac_predis",
		"aud_3rd_dac_tml",
		"aud_3rd_dac_hires",
		"aud_i2s5_b",
		"aud_i2s6_b",
		"aud_i2s7_b",
		"aud_i2s8_b",
		"aud_i2s9_b",
		"imp_iic_wrap_e_i2c3",
		"imp_iic_wrap_s_i2c7",
		"imp_iic_wrap_s_i2c8",
		"imp_iic_wrap_s_i2c9",
		"imp_iic_wrap_ws_i2c1",
		"imp_iic_wrap_ws_i2c2",
		"imp_iic_wrap_ws_i2c4",
		"imp_iic_wrap_w_i2c5",
		"imp_iic_wrap_n_i2c0",
		"imp_iic_wrap_n_i2c6",
		"msdc_top_aes_0p",
		"msdc_top_src_0p",
		"msdc_top_src_1p",
		"msdc_top_src_2p",
		"msdc_top_p_msdc0",
		"msdc_top_p_msdc1",
		"msdc_top_p_msdc2",
		"msdc_top_p_cfg",
		"msdc_top_axi",
		"msdc_top_h_mst_0p",
		"msdc_top_h_mst_1p",
		"msdc_top_h_mst_2p",
		"msdc_top_mem_off_dly_26m",
		"msdc_top_32k",
		"msdc_top_ahb2axi_brg_axi",
		"msdc_axi_wrap",
		"mfg_bg3d",
		"mm_disp_mutex0",
		"mm_disp_config",
		"mm_disp_ovl0",
		"mm_disp_rdma0",
		"mm_disp_ovl0_2l",
		"mm_disp_wdma0",
		"mm_disp_ufbc_wdma0",
		"mm_disp_rsz0",
		"mm_disp_aal0",
		"mm_disp_ccorr0",
		"mm_disp_dither0",
		"mm_smi_infra",
		"mm_disp_gamma0",
		"mm_disp_postmask0",
		"mm_disp_dsc_wrap0",
		"mm_dsi0",
		"mm_disp_color0",
		"mm_smi_common",
		"mm_disp_fake_eng0",
		"mm_disp_fake_eng1",
		"mm_mdp_tdshp4",
		"mm_mdp_rsz4",
		"mm_mdp_aal4",
		"mm_mdp_hdr4",
		"mm_mdp_rdma4",
		"mm_mdp_color4",
		"mm_disp_y2r0",
		"mm_smi_gals",
		"mm_disp_ovl2_2l",
		"mm_disp_rdma4",
		"mm_disp_dpi0",
		"mm_smi_iommu",
		"mm_dsi_dsi0",
		"mm_dpi_dpi0",
		"mm_26mhz",
		"mm_32khz",
		"img_larb9",
		"img_larb10",
		"img_dip",
		"img_gals",
		"img2_larb11",
		"img2_larb12",
		"img2_mfb",
		"img2_wpe",
		"img2_mss",
		"img2_gals",
		"vdec_soc_larb1",
		"vdec_soc_lat",
		"vdec_soc_lat_active",
		"vdec_soc_vdec",
		"vdec_soc_vdec_active",
		"vdec_larb1",
		"vdec_lat",
		"vdec_lat_active",
		"vdec_vdec",
		"vdec_active",
		"venc_set0_larb",
		"venc_set1_venc",
		"venc_set2_jpgenc",
		"venc_set5_gals",
		"apu_conn_apu",
		"apu_conn_ahb",
		"apu_conn_axi",
		"apu_conn_isp",
		"apu_conn_cam_adl",
		"apu_conn_img_adl",
		"apu_conn_emi_26m",
		"apu_conn_vpu_udi",
		"apu_conn_edma_0",
		"apu_conn_edma_1",
		"apu_conn_edmal_0",
		"apu_conn_edmal_1",
		"apu_conn_mnoc",
		"apu_conn_tcm",
		"apu_conn_md32",
		"apu_conn_iommu_0",
		"apu_conn_iommu_1",
		"apu_conn_md32_32k",
		"apu_vcore_ahb",
		"apu_vcore_axi",
		"apu_vcore_adl",
		"apu_vcore_qos",
		"apu0_apu",
		"apu0_axi_m",
		"apu0_jtag",
		"apu1_apu",
		"apu1_axi_m",
		"apu1_jtag",
		"apu_mdla0_cg0",
		"apu_mdla0_cg1",
		"apu_mdla0_cg2",
		"apu_mdla0_cg3",
		"apu_mdla0_cg4",
		"apu_mdla0_cg5",
		"apu_mdla0_cg6",
		"apu_mdla0_cg7",
		"apu_mdla0_cg8",
		"apu_mdla0_cg9",
		"apu_mdla0_cg10",
		"apu_mdla0_cg11",
		"apu_mdla0_cg12",
		"apu_mdla0_apb",
		"apu_mdla0_axi_m",
		"cam_larb13",
		"cam_dfp_vad",
		"cam_larb14",
		"cam_cam",
		"cam_camtg",
		"cam_seninf",
		"cam_camsv0",
		"cam_camsv1",
		"cam_camsv2",
		"cam_camsv3",
		"cam_ccu0",
		"cam_ccu1",
		"cam_mraw0",
		"cam_fake_eng",
		"cam_ccu_gals",
		"cam2mm_gals",
		"cam_rawa_larbx",
		"cam_rawa_cam",
		"cam_rawa_camtg",
		"cam_rawb_larbx",
		"cam_rawb_cam",
		"cam_rawb_camtg",
		"cam_rawc_larbx",
		"cam_rawc_cam",
		"cam_rawc_camtg",
		"ipe_larb19",
		"ipe_larb20",
		"ipe_smi_subcom",
		"ipe_fd",
		"ipe_fe",
		"ipe_rsc",
		"ipe_dpe",
		"ipe_gals",
		"mdp_mdp_rdma0",
		"mdp_mdp_tdshp0",
		"mdp_img_dl_async0",
		"mdp_img_dl_async1",
		"mdp_mdp_rdma1",
		"mdp_mdp_tdshp1",
		"mdp_smi0",
		"mdp_apb_bus",
		"mdp_mdp_wrot0",
		"mdp_mdp_rsz0",
		"mdp_mdp_hdr0",
		"mdp_mdp_mutex0",
		"mdp_mdp_wrot1",
		"mdp_mdp_rsz1",
		"mdp_mdp_hdr1",
		"mdp_mdp_fake_eng0",
		"mdp_mdp_aal0",
		"mdp_mdp_aal1",
		"mdp_mdp_color0",
		"mdp_mdp_color1",
		"mdp_img_dl_relay0_async0",
		"mdp_img_dl_relay1_async1",
		/* end */
		NULL
	};

	return clks;
}

/*
 * clkdbg pwr_status
 */

static const char * const *get_pwr_names(void)
{
	static const char * const pwr_names[] = {
		[0] = "MD",
		[1] = "CONN",
		[2] = "MFG0",
		[3] = "MFG1",
		[4] = "MFG2",
		[5] = "MFG3",
		[6] = "MFG4",
		[7] = "MFG5",
		[8] = "MFG6",
		[9] = "INFRA",
		[10] = "SUB_INFRA",
		[11] = "DDRPHY",
		[12] = "ISP",
		[13] = "ISP2",
		[14] = "IPE",
		[15] = "VDEC",
		[16] = "VDEC2",
		[17] = "VEN",
		[18] = "VEN_CORE1",
		[19] = "MDP",
		[20] = "DISP",
		[21] = "AUDIO",
		[22] = "ADSP",
		[23] = "CAM",
		[24] = "CAM_RAWA",
		[25] = "CAM_RAWB",
		[26] = "CAM_RAWC",
		[27] = "DP_TX",
		[28] = "DDRPHY2",
		[29] = "MCUPM",
		[30] = "MSDC",
		[31] = "PERI",
	};

	return pwr_names;
}

static u32 get_spm_pwr_status(void)
{
	static void __iomem *scpsys_base, *pwr_sta, *pwr_sta_2nd;

	if (scpsys_base == NULL || pwr_sta == NULL || pwr_sta_2nd == NULL) {
		scpsys_base = ioremap(0x10006000, PAGE_SIZE);
		pwr_sta = scpsys_base + 0x16c;
		pwr_sta_2nd = scpsys_base + 0x170;
	}

	return clk_readl(pwr_sta) & clk_readl(pwr_sta_2nd);
}

static int clkdbg_pwr_status_vpu(struct seq_file *s, void *v)
{
	static void __iomem *scpsys_base;
	u32 other_pwr_sta;
	const char *st;

	scpsys_base = ioremap(0x10006000, PAGE_SIZE);
	other_pwr_sta = clk_readl(scpsys_base + 0x178);

	seq_printf(s, "OTHER_PWR_STATUS: 0x%08x\n", other_pwr_sta);

	st = (other_pwr_sta & BIT(5)) != 0U ? "ON" : "off";

	seq_printf(s, "[%2d]: %3s: %s\n", 5, st, "VPU");

	return 0;
}

/*
 * clkdbg dump_clks
 */

static void setup_provider_clk(struct provider_clk *pvdck)
{
	static const struct {
		const char *pvdname;
		u32 pwr_mask;
	} pvd_pwr_mask[] = {
		{"scp_adsp", BIT(22)},
		{"audsys", BIT(21)},
		{"msdc_top", BIT(30)},
		{"msdc", BIT(30)},
		{"mfgcfg", BIT(2)},
		{"mmsys", BIT(20)},
		{"imgsys", BIT(12)},
		{"imgsys2", BIT(13)},
		{"vdecsys_soc", BIT(15)},
		{"vdecsys", BIT(16)},
		{"vencsys", BIT(17)},
#if 0 // other_pwr_status
		{"apu_conn", BIT(5)},
		{"apu_vcore", BIT(5)},
		{"apu0", BIT(5)},
		{"apu1", BIT(5)},
		{"apu_mdla0", BIT(5)},
#endif
		{"camsys", BIT(23)},
		{"camsys_rawa", BIT(24)},
		{"camsys_rawb", BIT(25)},
		{"camsys_rawc", BIT(26)},
		{"ipesys", BIT(14)},
		{"mdpsys", BIT(19)},
	};

	size_t i;
	const char *pvdname = pvdck->provider_name;

	if (pvdname == NULL)
		return;

	for (i = 0; i < ARRAY_SIZE(pvd_pwr_mask); i++) {
		if (strcmp(pvdname, pvd_pwr_mask[i].pvdname) == 0) {
			pvdck->pwr_mask = pvd_pwr_mask[i].pwr_mask;
			return;
		}
	}
}

/*
 * init functions
 */

static struct clkdbg_ops clkdbg_mt8192_ops = {
	.get_all_fmeter_clks = get_all_fmeter_clks,
	.fmeter_freq = fmeter_freq_op,
	.get_all_regnames = get_all_regnames,
	.get_all_clk_names = get_all_clk_names,
	.get_pwr_names = get_pwr_names,
	.setup_provider_clk = setup_provider_clk,
	.get_spm_pwr_status = get_spm_pwr_status,
};

static void __init init_custom_cmds(void)
{
	static const struct cmd_fn cmds[] = {
		CMDFN("pwr_status_vpu", clkdbg_pwr_status_vpu),
		{}
	};

	set_custom_cmds(cmds);
}

static int __init clkdbg_mt8192_init(void)
{
	init_regbase();

	init_custom_cmds();
	set_clkdbg_ops(&clkdbg_mt8192_ops);

#if DUMP_INIT_STATE
	print_regs();
	print_fmeter_all();
#endif /* DUMP_INIT_STATE */

	return 0;
}
device_initcall(clkdbg_mt8192_init);
