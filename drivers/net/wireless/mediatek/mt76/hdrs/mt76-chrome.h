#ifndef __MT76_CHROME
#define __MT76_CHROME
/* This file is pre-included from the Makefile (cc command line)
 *
 * ChromeOS backport definitions
 * Copyright (C) 2016-2017 Intel Deutschland GmbH
 * Copyright (C) 2018      Intel Corporation
 */

#include <hdrs/mac80211-exp.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/compiler.h>

/* include rhashtable this way to get our copy if another exists */
#include <linux/list_nulls.h>
#ifndef NULLS_MARKER
#define NULLS_MARKER(value) (1UL | (((long)value) << 1))
#endif
#include <hdrs/linux/rhashtable-types.h>
#include "linux/rhashtable.h"

#include <linux/errno.h>
#include <linux/idr.h>
#include <linux/vmalloc.h>

/* get the CPTCFG_* preprocessor symbols */
#include <hdrs/config.h>


#include <linux/kthread.h>
#include <net/genetlink.h>
#include <linux/crypto.h>
#include <linux/moduleparam.h>
#include <linux/debugfs.h>
#include <linux/hrtimer.h>
#include <crypto/algapi.h>
#include <linux/pci.h>
#include <linux/if_vlan.h>
#include <linux/overflow.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>

#define LINUX_VERSION_IS_LESS(x1,x2,x3) (LINUX_VERSION_CODE < KERNEL_VERSION(x1,x2,x3))
#define LINUX_VERSION_IS_GEQ(x1,x2,x3)  (LINUX_VERSION_CODE >= KERNEL_VERSION(x1,x2,x3))
#define LINUX_VERSION_IN_RANGE(x1,x2,x3, y1,y2,y3) \
        (LINUX_VERSION_IS_GEQ(x1,x2,x3) && LINUX_VERSION_IS_LESS(y1,y2,y3))
#define LINUX_BACKPORT(sym) backport_ ## sym

#include "net/fq.h"
/*
 * Need to include these here, otherwise we get the regular kernel ones
 * pre-including them makes it work, even though later the kernel ones
 * are included again, but they (hopefully) have the same include guard
 * ifdef/define so the second time around nothing happens
 *
 * We still keep them in the correct directory so if they don't exist in
 * the kernel (e.g. bitfield.h won't) the preprocessor can find them.
 */
#include <hdrs/linux/ieee80211.h>
#include <hdrs/linux/average.h>
#include <hdrs/linux/bitfield.h>
#include <hdrs/net/ieee80211_radiotap.h>
#define IEEE80211RADIOTAP_H 1 /* older kernels used this include protection */

/* mac80211 & backport - order matters, need this inbetween */
#include <hdrs/mac80211-bp.h>

#include <hdrs/net/codel.h>
#include <hdrs/net/mac80211.h>

#if LINUX_VERSION_IS_GEQ(5,3,0)
/*
 * In v5.3, this function was renamed, so rename it here for v5.3+.
 * When we merge v5.3 back from upstream, the opposite should be done
 * (i.e. we will have _boottime_ and need to rename to _boot_ in <
 * v5.3 instead).
*/
#define ktime_get_boot_ns ktime_get_boottime_ns
#endif /* > 5.3.0 */

#ifndef ETH_P_80221
#define ETH_P_80221	0x8917	/* IEEE 802.21 Media Independent Handover Protocol */
#endif

#if LINUX_VERSION_IS_LESS(4,15,0)
static inline
void backport_genl_dump_check_consistent(struct netlink_callback *cb,
					 void *user_hdr)
{
	struct genl_family dummy_family = {
		.hdrsize = 0,
	};

	genl_dump_check_consistent(cb, user_hdr, &dummy_family);
}
#define genl_dump_check_consistent LINUX_BACKPORT(genl_dump_check_consistent)
#endif /* LINUX_VERSION_IS_LESS(4,15,0) */

/* avoid conflicts with other headers */
#ifdef is_signed_type
#undef is_signed_type
#endif

#ifndef offsetofend
/**
 * offsetofend(TYPE, MEMBER)
 *
 * @TYPE: The type of the structure
 * @MEMBER: The member within the structure to get the end offset of
 */
#define offsetofend(TYPE, MEMBER) \
	(offsetof(TYPE, MEMBER)	+ sizeof(((TYPE *)0)->MEMBER))
#endif


int __alloc_bucket_spinlocks(spinlock_t **locks, unsigned int *lock_mask,
			     size_t max_size, unsigned int cpu_mult,
			     gfp_t gfp, const char *name,
			     struct lock_class_key *key);

#define alloc_bucket_spinlocks(locks, lock_mask, max_size, cpu_mult, gfp)    \
	({								     \
		static struct lock_class_key key;			     \
		int ret;						     \
									     \
		ret = __alloc_bucket_spinlocks(locks, lock_mask, max_size,   \
					       cpu_mult, gfp, #locks, &key); \
		ret;							\
	})
void free_bucket_spinlocks(spinlock_t *locks);

#if LINUX_VERSION_IS_LESS(4, 17, 0)

#define sg_init_marker LINUX_BACKPORT(sg_init_marker)
/**
 * sg_init_marker - Initialize markers in sg table
 * @sgl:	   The SG table
 * @nents:	   Number of entries in table
 *
 **/
static inline void sg_init_marker(struct scatterlist *sgl,
				  unsigned int nents)
{
#ifdef CONFIG_DEBUG_SG
	unsigned int i;

	for (i = 0; i < nents; i++)
		sgl[i].sg_magic = SG_MAGIC;
#endif
	sg_mark_end(&sgl[nents - 1]);
}

#endif /* LINUX_VERSION_IS_LESS(4, 17, 0) */

#if LINUX_VERSION_IS_LESS(4,18,0)
#define firmware_request_nowarn(fw, name, device) request_firmware(fw, name, device)
#endif

#if LINUX_VERSION_IS_LESS(4,19,0)
#ifndef atomic_fetch_add_unless
static inline int atomic_fetch_add_unless(atomic_t *v, int a, int u)
{
		return __atomic_add_unless(v, a, u);
}
#endif
#endif /* LINUX_VERSION_IS_LESS(4,19,0) */

#if LINUX_VERSION_IS_LESS(5,2,0)
#define nla_parse_deprecated nla_parse
#define nla_parse_nested_deprecated nla_parse_nested
#endif

#if defined(CONFIG_PCI)
#if LINUX_VERSION_IS_LESS(5,3,0)

static inline int
backport_pci_disable_link_state(struct pci_dev *pdev, int state)
{
	u16 aspmc;

	pci_disable_link_state(pdev, state);

	pcie_capability_read_word(pdev, PCI_EXP_LNKCTL, &aspmc);
	if ((state & PCIE_LINK_STATE_L0S) &&
	    (aspmc & PCI_EXP_LNKCTL_ASPM_L0S))
		return -EPERM;

	if ((state & PCIE_LINK_STATE_L1) &&
	    (aspmc & PCI_EXP_LNKCTL_ASPM_L1))
		return -EPERM;

	return 0;
}
#define pci_disable_link_state backport_pci_disable_link_state

#endif /* < 5.3 */
#endif /* defined(CONFIG_PCI) */

#ifndef SDIO_VENDOR_ID_MEDIATEK
#define SDIO_VENDOR_ID_MEDIATEK 0x037a
#endif

#ifndef module_sdio_driver
#define module_sdio_driver(__sdio_driver) \
	module_driver(__sdio_driver, sdio_register_driver, \
		      sdio_unregister_driver)
#endif

#endif /* __MT76_CHROME */
