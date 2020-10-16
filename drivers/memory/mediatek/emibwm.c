// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 * Author: Sagy Shih <sagy.shih@mediatek.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/arm-smccc.h>
#include<linux/ktime.h>
#include <linux/delay.h>
#ifdef MTK_SIP_READY
#include <linux/soc/mediatek/mtk_sip_svc.h>
#endif
#include <dramc.h>
#include <emi.h>

#define WACT		0x420

static void *__iomem emibwm_get_cen_base(void);
static int emibwm_probe(struct platform_device *pdev);
static int emibwm_remove(struct platform_device *dev);

static struct platform_device *emibwm_pdev;
static int dis_sta;
ktime_t tm_begin, tm_end, tm_diff;
unsigned long long tm_us;

static char *	int_to_str(unsigned int n, char *str)
{
	int i = 0, j = 0;
	char temp[16] = {0};
	char *ret_str = str;

	do {
		temp[i++] = n % 10 + '0';
		n /= 10;
	} while(n);

	while(i--)
		str[j++] = temp[i];

	return ret_str + j;
}

// return result string of a/b: eg:  a:1  b:3  result: 0.3333333...
// 	max_digits: means the digits after dot,  if 6, result: 0.333333
// thread-unsafe: use internal static result buffer which stores result
const char *int_div_to_double(unsigned long long a, unsigned long long b, unsigned int max_digits)
{
	static char result[128];
	unsigned int i = 0;
	unsigned int max_len = sizeof(result) - 1;
	char *p;
	unsigned long long temp, extra;
	int first = 1;

	memset(result, '0', max_len);	// clear to '0'
	if(max_digits < max_len)
		max_len = max_digits;

	if(b == 0)
		return result;

	p = result;

	temp = a / b;
	extra = a % b;
	p = int_to_str(temp, p);

	while(i < max_len && extra) {
		if(first) {
			p[0] = '.';

			first = 0;
			++p;
			++i;
		}

		a = extra * 10;
		temp = a / b;
		extra = a % b;

		p = int_to_str(temp, p);

		++i;
	}

	*p = '\0';
	return result;
}


static unsigned long emibwm_readl(unsigned long offset)
{
	unsigned long v;
	void *__iomem emicen;

	emicen = emibwm_get_cen_base();

	v = readl(emicen + offset);

	pr_info("[EMIBWM] read 0x%lx --> %lx\n", offset, v);
	return v;
}

static void emibwm_writel(unsigned long v, unsigned long offset)
{
	unsigned long ret;
	void *__iomem emicen;

	emicen = emibwm_get_cen_base();

	writel(v, emicen + offset);

	ret = readl(emicen + offset);
	pr_info("[EMIBWM] write 0x%lx --> %lx\n", offset, ret);
}

static void emibwm_tick_begin(void)
{
	tm_begin = ktime_get();
}

static void emibwm_tick_end(void)
{
	tm_end = ktime_get();
	tm_diff = ktime_sub(tm_end, tm_begin);
	tm_us = ktime_to_us(tm_diff);
}

unsigned long long emibwm_get_total_bw(void)
{
	unsigned long long val;

	val = emibwm_readl(WACT) * 8;	// 8: monitor unit bytes

	return val;
}

static void emibwm_show_bw(unsigned long long bw, unsigned int mon_interval_ms)
{
	unsigned long long ms;
	const char *res;

	ms = tm_us / 1000;
	if (mon_interval_ms == 0 || ms == 0)
		pr_info("invalid interval 0 ms...Are you joking?");
	else {
		pr_info("[BWINFO]real time:%llu ms, mon time:%u ms, %llx --> %llu MB/s\n", 
			tm_us / 1000, mon_interval_ms,
			bw, bw / 1024 / 1024 * 1000 / ms);

		res = int_div_to_double(bw * 1000, ms * 1024 * 1024, 2);
		pr_info("\t Accurate bw: %s MB/s", res);
		res = int_div_to_double(bw * 1000000, tm_us * 1024 * 1024, 2);
		pr_info("\t  %s MB/s\n", res);
	}
}

static void emibwm_process_curr_bw(unsigned int mon_interval_ms)
{
	unsigned long long bw;

	bw = emibwm_get_total_bw();
	emibwm_show_bw(bw, mon_interval_ms);
}

static void *__iomem emibwm_get_cen_base(void)
{
	struct emibwm_dev_t *emibwm_dev_ptr;
	
	emibwm_dev_ptr =
			(struct emibwm_dev_t *)platform_get_drvdata(emibwm_pdev);

	return emibwm_dev_ptr->emi_cen_base[0];
}

static void emibwm_counter_pause(void)
{
    unsigned int value;
    unsigned long bmen;

	bmen = BMEN;

    value = emibwm_readl(bmen);
    emibwm_writel(value | BUS_MON_PAUSE, bmen);

	emibwm_tick_end();
}

static void emibwm_counter_continue(void)
{
    unsigned int value;
    unsigned long bmen;

	bmen = BMEN;

    value = emibwm_readl(bmen);
    emibwm_writel(value & ~BUS_MON_PAUSE, bmen);
}

static void emibwm_counter_enable(const unsigned int enable)
{
    unsigned int value, value_set;
    unsigned long bmen;

	bmen = BMEN;

    value = emibwm_readl(bmen);
    if (!enable) {  /* disable monitor circuit */
        //  bit3 =1     bit0 = 0-> clear
        value_set = (value) | (BUS_MON_IDLE);
        emibwm_writel(value_set, bmen);

        value_set = ((value) | (BUS_MON_IDLE)) & ~(BUS_MON_EN);
        emibwm_writel(value_set, bmen);

        value_set = ((value) & ~(BUS_MON_IDLE)) & ~(BUS_MON_EN);
        emibwm_writel(value_set, bmen);
    } else {            /* enable monitor circuit */
        //  bit3 =0     &   bit0=1
        value_set = (value & ~(BUS_MON_IDLE));
        emibwm_writel(value_set, bmen);

        value_set = (value & ~(BUS_MON_IDLE)) | (BUS_MON_EN);
        emibwm_writel(value_set, bmen);

		emibwm_tick_begin();
    }
}

void emibwm_mon_start(void)
{
    mtk_dramc_dcm_disable();
    emibwm_counter_enable(0);

    emibwm_counter_enable(1);
    mtk_dramc_dcm_restore();
}

void emibwm_mon_restart(void)
{
    emibwm_counter_continue();
    mtk_dramc_dcm_disable();
    emibwm_counter_enable(0);

    emibwm_counter_enable(1);
    mtk_dramc_dcm_restore();
}

void emibwm_mon_stop(void)
{
    emibwm_counter_pause();
}

static void emibwm_sleep(unsigned int mon_interval_ms)
{
	if (mon_interval_ms < 20)
		usleep_range(mon_interval_ms * 1000, mon_interval_ms * 1000 + 100);
	else
		msleep(mon_interval_ms);
}

static void emibwm_mon_auto(unsigned int mon_interval_ms)
{
	while (!dis_sta) {
		emibwm_mon_restart();
		emibwm_sleep(mon_interval_ms);
		emibwm_mon_stop();

		emibwm_process_curr_bw(mon_interval_ms);
	}
}

static ssize_t emibwm_ctrl_show(struct device_driver *driver, char *buf)
{
	
	return strlen(buf);
}

static ssize_t emibwm_ctrl_store
	(struct device_driver *driver, const char *buf, size_t count)
{
	struct emibwm_dev_t *emibwm_dev_ptr;
	char *command;
	char *backup_command;
	char *ptr;
	char *token[MTK_EMI_MAX_TOKEN];
	int i, ret;

	if (!emibwm_pdev)
		return count;

	emibwm_dev_ptr =
		(struct emibwm_dev_t *)platform_get_drvdata(emibwm_pdev);

	if ((strlen(buf) + 1) > MTK_EMI_MAX_CMD_LEN) {
		pr_info("%s: store command overflow\n", __func__);
		return count;
	}

	pr_info("%s: store: %s\n", __func__, buf);

	command = kmalloc((size_t) MTK_EMI_MAX_CMD_LEN, GFP_KERNEL);
	if (!command)
		return count;
	backup_command = command;
	if (!command)
		return count;
	strncpy(command, buf, (size_t) MTK_EMI_MAX_CMD_LEN);

	for (i = 0; i < MTK_EMI_MAX_TOKEN; i++) {
		ptr = strsep(&command, " ");
		if (!ptr)
			break;
		token[i] = ptr;
	}

	if (!strncmp(buf, "ENABLE", strlen("ENABLE"))) {
		if (i < 2)
			goto emibwm_ctrl_store_end;

		pr_info("%s: %s\n", __func__, token[0]);
		emibwm_mon_restart();

	} else if (!strncmp(buf, "DISABLE", strlen("DISABLE"))) {
		// mark disable status
		dis_sta = 1;
		pr_info("%s: %s\n",
			__func__, token[0]);

		emibwm_mon_stop();
		emibwm_process_curr_bw(tm_us / 1000);
	} else if (!strncmp(buf, "AUTO", strlen("AUTO"))) {
		unsigned long mon_interval_ms = 0;

		dis_sta = 0;
		// first stop the monitor
		emibwm_mon_stop();

		if(i > 1) {
			ret = kstrtoul(token[1], 10, &mon_interval_ms);
			if (ret != 0)
				pr_info("%s: fail to parse mon interval ms\n", __func__);	
		}
		pr_info("%s: %s mon interval: %d ms\n",
			__func__, token[0], mon_interval_ms ? mon_interval_ms : 1000);

		emibwm_mon_auto(mon_interval_ms);
	} else
		pr_info("%s: unknown command\n", __func__);

emibwm_ctrl_store_end:
	kfree(backup_command);

	return count;
}

static DRIVER_ATTR_RW(emibwm_ctrl);


static void read_emi_info(void)
{
	unsigned long v;

	v = emibwm_readl(0x4);
	pr_info("[EMI]0x4: 0x%lx\n", v);
}

static const struct of_device_id emibwm_of_ids[] = {
	{.compatible = "mediatek,common-emibwm",},
	{}
};

static struct platform_driver emibwm_drv = {
	.probe = emibwm_probe,
	.remove = emibwm_remove,
	.driver = {
		.name = "emibwm_drv",
		.owner = THIS_MODULE,
		.of_match_table = emibwm_of_ids,
	},
};

static int emibwm_probe(struct platform_device *pdev)
{
	struct device_node *emibwm_node = pdev->dev.of_node;
	struct device_node *emicen_node =
		of_parse_phandle(emibwm_node, "mediatek,emi-reg", 0);
	struct emibwm_dev_t *emibwm_dev_ptr;

	unsigned int i;
	int ret;

	pr_info("%s: module probe.\n", __func__);
	emibwm_pdev = pdev;
	emibwm_dev_ptr = devm_kmalloc(&pdev->dev,
		sizeof(struct emibwm_dev_t), GFP_KERNEL);
	if (!emibwm_dev_ptr)
		return -ENOMEM;

	/* get EMI base addr */
	emibwm_dev_ptr->emi_cen_cnt = of_property_count_elems_of_size(
			emicen_node, "reg", sizeof(unsigned int) * 4);
	if (emibwm_dev_ptr->emi_cen_cnt <= 0) {
		pr_info("%s: get emi_cen_cnt fail\n", __func__);
		return -EINVAL;
	}
	emibwm_dev_ptr->emi_cen_base = devm_kmalloc_array(&pdev->dev,
		emibwm_dev_ptr->emi_cen_cnt, sizeof(phys_addr_t), GFP_KERNEL);
	if (!(emibwm_dev_ptr->emi_cen_base))
		return -ENOMEM;
	for (i = 0; i < emibwm_dev_ptr->emi_cen_cnt; i++) {
		emibwm_dev_ptr->emi_cen_base[i] = of_iomap(emicen_node, i);
		if (IS_ERR(emibwm_dev_ptr->emi_cen_base[i])) {
			pr_info("%s: unable to map EMI%d CEN base\n",
				__func__, i);
			return -EINVAL;
		}
	}

	platform_set_drvdata(pdev, emibwm_dev_ptr);

	ret = driver_create_file(&emibwm_drv.driver,
		&driver_attr_emibwm_ctrl);
	if (ret)
		pr_info("%s: fail to create emibwm_ctl\n", __func__);

	read_emi_info();

	return ret;
}

static int emibwm_remove(struct platform_device *dev)
{
	return 0;
}

static int __init emibwm_drv_init(void)
{
	int ret;

	ret = platform_driver_register(&emibwm_drv);
	if (ret) {
		pr_info("%s: init fail, ret 0x%x\n", __func__, ret);
		return ret;
	}

	return ret;
}

static void __exit emibwm_drv_exit(void)
{
	platform_driver_unregister(&emibwm_drv);
}

module_init(emibwm_drv_init);
module_exit(emibwm_drv_exit);


MODULE_DESCRIPTION("MediaTek EMIBWM Driver v0.1");


