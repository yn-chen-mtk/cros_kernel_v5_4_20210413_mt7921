/* SPDX-License-Identifier: GPL-2.0 */

/* Copyright (c) 2012-2018, The Linux Foundation. All rights reserved.
 * Copyright (C) 2018-2020 Linaro Ltd.
 */
#ifndef _IPA_REG_H_
#define _IPA_REG_H_

#include <linux/bitfield.h>

#include "ipa_version.h"

struct ipa;

/**
 * DOC: IPA Registers
 *
 * IPA registers are located within the "ipa-reg" address space defined by
 * Device Tree.  The offset of each register within that space is specified
 * by symbols defined below.  The address space is mapped to virtual memory
 * space in ipa_mem_init().  All IPA registers are 32 bits wide.
 *
 * Certain register types are duplicated for a number of instances of
 * something.  For example, each IPA endpoint has an set of registers
 * defining its configuration.  The offset to an endpoint's set of registers
 * is computed based on an "base" offset, plus an endpoint's ID multiplied
 * and a "stride" value for the register.  For such registers, the offset is
 * computed by a function-like macro that takes a parameter used in the
 * computation.
 *
 * Some register offsets depend on execution environment.  For these an "ee"
 * parameter is supplied to the offset macro.  The "ee" value is a member of
 * the gsi_ee enumerated type.
 *
 * The offset of a register dependent on endpoint ID is computed by a macro
 * that is supplied a parameter "ep", "txep", or "rxep".  A register with an
 * "ep" parameter is valid for any endpoint; a register with a "txep" or
 * "rxep" parameter is valid only for TX or RX endpoints, respectively.  The
 * "*ep" value is assumed to be less than the maximum valid endpoint ID
 * for the current hardware, and that will not exceed IPA_ENDPOINT_MAX.
 *
 * The offset of registers related to filter and route tables is computed
 * by a macro that is supplied a parameter "er".  The "er" represents an
 * endpoint ID for filters, or a route ID for routes.  For filters, the
 * endpoint ID must be less than IPA_ENDPOINT_MAX, but is further restricted
 * because not all endpoints support filtering.  For routes, the route ID
 * must be less than IPA_ROUTE_MAX.
 *
 * The offset of registers related to resource types is computed by a macro
 * that is supplied a parameter "rt".  The "rt" represents a resource type,
 * which is is a member of the ipa_resource_type_src enumerated type for
 * source endpoint resources or the ipa_resource_type_dst enumerated type
 * for destination endpoint resources.
 *
 * Some registers encode multiple fields within them.  For these, each field
 * has a symbol below defining a field mask that encodes both the position
 * and width of the field within its register.
 *
 * In some cases, different versions of IPA hardware use different offset or
 * field mask values.  In such cases an inline_function(ipa) is used rather
 * than a MACRO to define the offset or field mask to use.
 *
 * Finally, some registers hold bitmasks representing endpoints.  In such
 * cases the @available field in the @ipa structure defines the "full" set
 * of valid bits for the register.
 */

/* The next field is not supported for IPA v4.1 */
#define IPA_REG_COMP_CFG_OFFSET				0x0000003c
#define ENABLE_FMASK				GENMASK(0, 0)
#define GSI_SNOC_BYPASS_DIS_FMASK		GENMASK(1, 1)
#define GEN_QMB_0_SNOC_BYPASS_DIS_FMASK		GENMASK(2, 2)
#define GEN_QMB_1_SNOC_BYPASS_DIS_FMASK		GENMASK(3, 3)
#define IPA_DCMP_FAST_CLK_EN_FMASK		GENMASK(4, 4)
/* The remaining fields are not present for IPA v3.5.1 */
#define IPA_QMB_SELECT_CONS_EN_FMASK		GENMASK(5, 5)
#define IPA_QMB_SELECT_PROD_EN_FMASK		GENMASK(6, 6)
#define GSI_MULTI_INORDER_RD_DIS_FMASK		GENMASK(7, 7)
#define GSI_MULTI_INORDER_WR_DIS_FMASK		GENMASK(8, 8)
#define GEN_QMB_0_MULTI_INORDER_RD_DIS_FMASK	GENMASK(9, 9)
#define GEN_QMB_1_MULTI_INORDER_RD_DIS_FMASK	GENMASK(10, 10)
#define GEN_QMB_0_MULTI_INORDER_WR_DIS_FMASK	GENMASK(11, 11)
#define GEN_QMB_1_MULTI_INORDER_WR_DIS_FMASK	GENMASK(12, 12)
#define GEN_QMB_0_SNOC_CNOC_LOOP_PROT_DIS_FMASK	GENMASK(13, 13)
#define GSI_SNOC_CNOC_LOOP_PROT_DISABLE_FMASK	GENMASK(14, 14)
#define GSI_MULTI_AXI_MASTERS_DIS_FMASK		GENMASK(15, 15)
#define IPA_QMB_SELECT_GLOBAL_EN_FMASK		GENMASK(16, 16)
#define IPA_ATOMIC_FETCHER_ARB_LOCK_DIS_FMASK	GENMASK(20, 17)

#define IPA_REG_CLKON_CFG_OFFSET			0x00000044
#define RX_FMASK				GENMASK(0, 0)
#define PROC_FMASK				GENMASK(1, 1)
#define TX_WRAPPER_FMASK			GENMASK(2, 2)
#define MISC_FMASK				GENMASK(3, 3)
#define RAM_ARB_FMASK				GENMASK(4, 4)
#define FTCH_HPS_FMASK				GENMASK(5, 5)
#define FTCH_DPS_FMASK				GENMASK(6, 6)
#define HPS_FMASK				GENMASK(7, 7)
#define DPS_FMASK				GENMASK(8, 8)
#define RX_HPS_CMDQS_FMASK			GENMASK(9, 9)
#define HPS_DPS_CMDQS_FMASK			GENMASK(10, 10)
#define DPS_TX_CMDQS_FMASK			GENMASK(11, 11)
#define RSRC_MNGR_FMASK				GENMASK(12, 12)
#define CTX_HANDLER_FMASK			GENMASK(13, 13)
#define ACK_MNGR_FMASK				GENMASK(14, 14)
#define D_DCPH_FMASK				GENMASK(15, 15)
#define H_DCPH_FMASK				GENMASK(16, 16)
#define DCMP_FMASK				GENMASK(17, 17)
#define NTF_TX_CMDQS_FMASK			GENMASK(18, 18)
#define TX_0_FMASK				GENMASK(19, 19)
#define TX_1_FMASK				GENMASK(20, 20)
#define FNR_FMASK				GENMASK(21, 21)
/* The remaining fields are not present for IPA v3.5.1 */
#define QSB2AXI_CMDQ_L_FMASK			GENMASK(22, 22)
#define AGGR_WRAPPER_FMASK			GENMASK(23, 23)
#define RAM_SLAVEWAY_FMASK			GENMASK(24, 24)
#define QMB_FMASK				GENMASK(25, 25)
#define WEIGHT_ARB_FMASK			GENMASK(26, 26)
#define GSI_IF_FMASK				GENMASK(27, 27)
#define GLOBAL_FMASK				GENMASK(28, 28)
#define GLOBAL_2X_CLK_FMASK			GENMASK(29, 29)

#define IPA_REG_ROUTE_OFFSET				0x00000048
#define ROUTE_DIS_FMASK				GENMASK(0, 0)
#define ROUTE_DEF_PIPE_FMASK			GENMASK(5, 1)
#define ROUTE_DEF_HDR_TABLE_FMASK		GENMASK(6, 6)
#define ROUTE_DEF_HDR_OFST_FMASK		GENMASK(16, 7)
#define ROUTE_FRAG_DEF_PIPE_FMASK		GENMASK(21, 17)
#define ROUTE_DEF_RETAIN_HDR_FMASK		GENMASK(24, 24)

#define IPA_REG_SHARED_MEM_SIZE_OFFSET			0x00000054
#define SHARED_MEM_SIZE_FMASK			GENMASK(15, 0)
#define SHARED_MEM_BADDR_FMASK			GENMASK(31, 16)

#define IPA_REG_QSB_MAX_WRITES_OFFSET			0x00000074
#define GEN_QMB_0_MAX_WRITES_FMASK		GENMASK(3, 0)
#define GEN_QMB_1_MAX_WRITES_FMASK		GENMASK(7, 4)

#define IPA_REG_QSB_MAX_READS_OFFSET			0x00000078
#define GEN_QMB_0_MAX_READS_FMASK		GENMASK(3, 0)
#define GEN_QMB_1_MAX_READS_FMASK		GENMASK(7, 4)
/* The next two fields are not present for IPA v3.5.1 */
#define GEN_QMB_0_MAX_READS_BEATS_FMASK		GENMASK(23, 16)
#define GEN_QMB_1_MAX_READS_BEATS_FMASK		GENMASK(31, 24)

static inline u32 ipa_reg_filt_rout_hash_en_offset(enum ipa_version version)
{
	if (version == IPA_VERSION_3_5_1)
		return 0x000008c;

	return 0x0000148;
}

static inline u32 ipa_reg_filt_rout_hash_flush_offset(enum ipa_version version)
{
	if (version == IPA_VERSION_3_5_1)
		return 0x0000090;

	return 0x000014c;
}

/* The next four fields are used for the hash enable and flush registers */
#define IPV6_ROUTER_HASH_FMASK			GENMASK(0, 0)
#define IPV6_FILTER_HASH_FMASK			GENMASK(4, 4)
#define IPV4_ROUTER_HASH_FMASK			GENMASK(8, 8)
#define IPV4_FILTER_HASH_FMASK			GENMASK(12, 12)

/* ipa->available defines the valid bits in the STATE_AGGR_ACTIVE register */
static inline u32 ipa_reg_state_aggr_active_offset(enum ipa_version version)
{
	if (version == IPA_VERSION_3_5_1)
		return 0x0000010c;

	return 0x000000b4;
}

#define IPA_REG_BCR_OFFSET				0x000001d0
/* The next two fields are not present for IPA v4.2 */
#define BCR_CMDQ_L_LACK_ONE_ENTRY_FMASK		GENMASK(0, 0)
#define BCR_TX_NOT_USING_BRESP_FMASK		GENMASK(1, 1)
/* The next field is invalid for IPA v4.1 */
#define BCR_TX_SUSPEND_IRQ_ASSERT_ONCE_FMASK	GENMASK(2, 2)
/* The next two fields are not present for IPA v4.2 */
#define BCR_SUSPEND_L2_IRQ_FMASK		GENMASK(3, 3)
#define BCR_HOLB_DROP_L2_IRQ_FMASK		GENMASK(4, 4)
#define BCR_DUAL_TX_FMASK			GENMASK(5, 5)
#define BCR_ENABLE_FILTER_DATA_CACHE_FMASK	GENMASK(6, 6)
#define BCR_NOTIF_PRIORITY_OVER_ZLT_FMASK	GENMASK(7, 7)
#define BCR_FILTER_PREFETCH_EN_FMASK		GENMASK(8, 8)
#define BCR_ROUTER_PREFETCH_EN_FMASK		GENMASK(9, 9)

/* Backward compatibility register value to use for each version */
static inline u32 ipa_reg_bcr_val(enum ipa_version version)
{
	if (version == IPA_VERSION_3_5_1)
		return BCR_CMDQ_L_LACK_ONE_ENTRY_FMASK |
			BCR_TX_NOT_USING_BRESP_FMASK |
			BCR_SUSPEND_L2_IRQ_FMASK |
			BCR_HOLB_DROP_L2_IRQ_FMASK |
			BCR_DUAL_TX_FMASK;

	if (version == IPA_VERSION_4_0 || version == IPA_VERSION_4_1)
		return BCR_CMDQ_L_LACK_ONE_ENTRY_FMASK |
			BCR_SUSPEND_L2_IRQ_FMASK |
			BCR_HOLB_DROP_L2_IRQ_FMASK |
			BCR_DUAL_TX_FMASK;

	return 0x00000000;
}

/* The value of the next register must be a multiple of 8 */
#define IPA_REG_LOCAL_PKT_PROC_CNTXT_BASE_OFFSET	0x000001e8

/* ipa->available defines the valid bits in the AGGR_FORCE_CLOSE register */
#define IPA_REG_AGGR_FORCE_CLOSE_OFFSET			0x000001ec

#define IPA_REG_COUNTER_CFG_OFFSET			0x000001f0
#define AGGR_GRANULARITY_FMASK			GENMASK(8, 4)

/* The internal inactivity timer clock is used for the aggregation timer */
#define TIMER_FREQUENCY	32000		/* 32 KHz inactivity timer clock */

/* Compute the value to use in the AGGR_GRANULARITY field representing the
 * given number of microseconds.  The value is one less than the number of
 * timer ticks in the requested period.  0 not a valid granularity value.
 */
static inline u32 ipa_aggr_granularity_val(u32 usec)
{
	return DIV_ROUND_CLOSEST(usec * TIMER_FREQUENCY, USEC_PER_SEC) - 1;
}

#define IPA_REG_TX_CFG_OFFSET				0x000001fc
/* The first three fields are present for IPA v3.5.1 only */
#define TX0_PREFETCH_DISABLE_FMASK		GENMASK(0, 0)
#define TX1_PREFETCH_DISABLE_FMASK		GENMASK(1, 1)
#define PREFETCH_ALMOST_EMPTY_SIZE_FMASK	GENMASK(4, 2)
/* The next six fields are present for IPA v4.0 and above */
#define PREFETCH_ALMOST_EMPTY_SIZE_TX0_FMASK	GENMASK(5, 2)
#define DMAW_SCND_OUTSD_PRED_THRESHOLD_FMASK	GENMASK(9, 6)
#define DMAW_SCND_OUTSD_PRED_EN_FMASK		GENMASK(10, 10)
#define DMAW_MAX_BEATS_256_DIS_FMASK		GENMASK(11, 11)
#define PA_MASK_EN_FMASK			GENMASK(12, 12)
#define PREFETCH_ALMOST_EMPTY_SIZE_TX1_FMASK	GENMASK(16, 13)
/* The next two fields are present for IPA v4.2 only */
#define SSPND_PA_NO_START_STATE_FMASK		GENMASK(18, 18)
#define SSPND_PA_NO_BQ_STATE_FMASK		GENMASK(19, 19)

#define IPA_REG_FLAVOR_0_OFFSET				0x00000210
#define IPA_MAX_PIPES_FMASK			GENMASK(3, 0)
#define IPA_MAX_CONS_PIPES_FMASK		GENMASK(12, 8)
#define IPA_MAX_PROD_PIPES_FMASK		GENMASK(20, 16)
#define IPA_PROD_LOWEST_FMASK			GENMASK(27, 24)

static inline u32 ipa_reg_idle_indication_cfg_offset(enum ipa_version version)
{
	if (version == IPA_VERSION_4_2)
		return 0x00000240;

	return 0x00000220;
}

#define ENTER_IDLE_DEBOUNCE_THRESH_FMASK	GENMASK(15, 0)
#define CONST_NON_IDLE_ENABLE_FMASK		GENMASK(16, 16)

/* # IPA source resource groups available based on version */
static inline u32 ipa_resource_group_src_count(enum ipa_version version)
{
	switch (version) {
	case IPA_VERSION_3_5_1:
	case IPA_VERSION_4_0:
	case IPA_VERSION_4_1:
		return 4;

	case IPA_VERSION_4_2:
		return 1;

	default:
		return 0;
	}
}

/* # IPA destination resource groups available based on version */
static inline u32 ipa_resource_group_dst_count(enum ipa_version version)
{
	switch (version) {
	case IPA_VERSION_3_5_1:
		return 3;

	case IPA_VERSION_4_0:
	case IPA_VERSION_4_1:
		return 4;

	case IPA_VERSION_4_2:
		return 1;

	default:
		return 0;
	}
}

/* Not all of the following are valid (depends on the count, above) */
#define IPA_REG_SRC_RSRC_GRP_01_RSRC_TYPE_N_OFFSET(rt) \
					(0x00000400 + 0x0020 * (rt))
#define IPA_REG_SRC_RSRC_GRP_23_RSRC_TYPE_N_OFFSET(rt) \
					(0x00000404 + 0x0020 * (rt))
#define IPA_REG_SRC_RSRC_GRP_45_RSRC_TYPE_N_OFFSET(rt) \
					(0x00000408 + 0x0020 * (rt))
#define IPA_REG_DST_RSRC_GRP_01_RSRC_TYPE_N_OFFSET(rt) \
					(0x00000500 + 0x0020 * (rt))
#define IPA_REG_DST_RSRC_GRP_23_RSRC_TYPE_N_OFFSET(rt) \
					(0x00000504 + 0x0020 * (rt))
#define IPA_REG_DST_RSRC_GRP_45_RSRC_TYPE_N_OFFSET(rt) \
					(0x00000508 + 0x0020 * (rt))
/* The next four fields are used for all resource group registers */
#define X_MIN_LIM_FMASK				GENMASK(5, 0)
#define X_MAX_LIM_FMASK				GENMASK(13, 8)
/* The next two fields are not always present (if resource count is odd) */
#define Y_MIN_LIM_FMASK				GENMASK(21, 16)
#define Y_MAX_LIM_FMASK				GENMASK(29, 24)

#define IPA_REG_ENDP_INIT_CTRL_N_OFFSET(ep) \
					(0x00000800 + 0x0070 * (ep))
/* The next field should only used for IPA v3.5.1 */
#define ENDP_SUSPEND_FMASK			GENMASK(0, 0)
#define ENDP_DELAY_FMASK			GENMASK(1, 1)

#define IPA_REG_ENDP_INIT_CFG_N_OFFSET(ep) \
					(0x00000808 + 0x0070 * (ep))
#define FRAG_OFFLOAD_EN_FMASK			GENMASK(0, 0)
#define CS_OFFLOAD_EN_FMASK			GENMASK(2, 1)
#define CS_METADATA_HDR_OFFSET_FMASK		GENMASK(6, 3)
#define CS_GEN_QMB_MASTER_SEL_FMASK		GENMASK(8, 8)

/** enum ipa_cs_offload_en - checksum offload field in ENDP_INIT_CFG_N */
enum ipa_cs_offload_en {
	IPA_CS_OFFLOAD_NONE		= 0x0,
	IPA_CS_OFFLOAD_UL		= 0x1,
	IPA_CS_OFFLOAD_DL		= 0x2,
};

#define IPA_REG_ENDP_INIT_HDR_N_OFFSET(ep) \
					(0x00000810 + 0x0070 * (ep))
#define HDR_LEN_FMASK				GENMASK(5, 0)
#define HDR_OFST_METADATA_VALID_FMASK		GENMASK(6, 6)
#define HDR_OFST_METADATA_FMASK			GENMASK(12, 7)
#define HDR_ADDITIONAL_CONST_LEN_FMASK		GENMASK(18, 13)
#define HDR_OFST_PKT_SIZE_VALID_FMASK		GENMASK(19, 19)
#define HDR_OFST_PKT_SIZE_FMASK			GENMASK(25, 20)
#define HDR_A5_MUX_FMASK			GENMASK(26, 26)
#define HDR_LEN_INC_DEAGG_HDR_FMASK		GENMASK(27, 27)
#define HDR_METADATA_REG_VALID_FMASK		GENMASK(28, 28)

#define IPA_REG_ENDP_INIT_HDR_EXT_N_OFFSET(ep) \
					(0x00000814 + 0x0070 * (ep))
#define HDR_ENDIANNESS_FMASK			GENMASK(0, 0)
#define HDR_TOTAL_LEN_OR_PAD_VALID_FMASK	GENMASK(1, 1)
#define HDR_TOTAL_LEN_OR_PAD_FMASK		GENMASK(2, 2)
#define HDR_PAYLOAD_LEN_INC_PADDING_FMASK	GENMASK(3, 3)
#define HDR_TOTAL_LEN_OR_PAD_OFFSET_FMASK	GENMASK(9, 4)
#define HDR_PAD_TO_ALIGNMENT_FMASK		GENMASK(13, 10)

/* Valid only for RX (IPA producer) endpoints */
#define IPA_REG_ENDP_INIT_HDR_METADATA_MASK_N_OFFSET(rxep) \
					(0x00000818 + 0x0070 * (rxep))

/* Valid only for TX (IPA consumer) endpoints */
#define IPA_REG_ENDP_INIT_MODE_N_OFFSET(txep) \
					(0x00000820 + 0x0070 * (txep))
#define MODE_FMASK				GENMASK(2, 0)
#define DEST_PIPE_INDEX_FMASK			GENMASK(8, 4)
#define BYTE_THRESHOLD_FMASK			GENMASK(27, 12)
#define PIPE_REPLICATION_EN_FMASK		GENMASK(28, 28)
#define PAD_EN_FMASK				GENMASK(29, 29)
#define HDR_FTCH_DISABLE_FMASK			GENMASK(30, 30)

/** enum ipa_mode - mode field in ENDP_INIT_MODE_N */
enum ipa_mode {
	IPA_BASIC			= 0x0,
	IPA_ENABLE_FRAMING_HDLC		= 0x1,
	IPA_ENABLE_DEFRAMING_HDLC	= 0x2,
	IPA_DMA				= 0x3,
};

#define IPA_REG_ENDP_INIT_AGGR_N_OFFSET(ep) \
					(0x00000824 +  0x0070 * (ep))
#define AGGR_EN_FMASK				GENMASK(1, 0)
#define AGGR_TYPE_FMASK				GENMASK(4, 2)
#define AGGR_BYTE_LIMIT_FMASK			GENMASK(9, 5)
#define AGGR_TIME_LIMIT_FMASK			GENMASK(14, 10)
#define AGGR_PKT_LIMIT_FMASK			GENMASK(20, 15)
#define AGGR_SW_EOF_ACTIVE_FMASK		GENMASK(21, 21)
#define AGGR_FORCE_CLOSE_FMASK			GENMASK(22, 22)
#define AGGR_HARD_BYTE_LIMIT_ENABLE_FMASK	GENMASK(24, 24)

/** enum ipa_aggr_en - aggregation enable field in ENDP_INIT_AGGR_N */
enum ipa_aggr_en {
	IPA_BYPASS_AGGR			= 0x0,
	IPA_ENABLE_AGGR			= 0x1,
	IPA_ENABLE_DEAGGR		= 0x2,
};

/** enum ipa_aggr_type - aggregation type field in ENDP_INIT_AGGR_N */
enum ipa_aggr_type {
	IPA_MBIM_16			= 0x0,
	IPA_HDLC			= 0x1,
	IPA_TLP				= 0x2,
	IPA_RNDIS			= 0x3,
	IPA_GENERIC			= 0x4,
	IPA_COALESCE			= 0x5,
	IPA_QCMAP			= 0x6,
};

/* Valid only for RX (IPA producer) endpoints */
#define IPA_REG_ENDP_INIT_HOL_BLOCK_EN_N_OFFSET(rxep) \
					(0x0000082c +  0x0070 * (rxep))
#define HOL_BLOCK_EN_FMASK			GENMASK(0, 0)

/* Valid only for RX (IPA producer) endpoints */
#define IPA_REG_ENDP_INIT_HOL_BLOCK_TIMER_N_OFFSET(rxep) \
					(0x00000830 +  0x0070 * (rxep))
/* The next two fields are present for IPA v4.2 only */
#define BASE_VALUE_FMASK			GENMASK(4, 0)
#define SCALE_FMASK				GENMASK(12, 8)

/* Valid only for TX (IPA consumer) endpoints */
#define IPA_REG_ENDP_INIT_DEAGGR_N_OFFSET(txep) \
					(0x00000834 + 0x0070 * (txep))
#define DEAGGR_HDR_LEN_FMASK			GENMASK(5, 0)
#define SYSPIPE_ERR_DETECTION_FMASK		GENMASK(6, 6)
#define PACKET_OFFSET_VALID_FMASK		GENMASK(7, 7)
#define PACKET_OFFSET_LOCATION_FMASK		GENMASK(13, 8)
#define IGNORE_MIN_PKT_ERR_FMASK		GENMASK(14, 14)
#define MAX_PACKET_LEN_FMASK			GENMASK(31, 16)

#define IPA_REG_ENDP_INIT_RSRC_GRP_N_OFFSET(ep) \
					(0x00000838 + 0x0070 * (ep))
/* Encoded value for RSRC_GRP endpoint register RSRC_GRP field */
static inline u32 rsrc_grp_encoded(enum ipa_version version, u32 rsrc_grp)
{
	switch (version) {
	case IPA_VERSION_4_2:
		return u32_encode_bits(rsrc_grp, GENMASK(0, 0));
	default:
		return u32_encode_bits(rsrc_grp, GENMASK(1, 0));
	}
}

/* Valid only for TX (IPA consumer) endpoints */
#define IPA_REG_ENDP_INIT_SEQ_N_OFFSET(txep) \
					(0x0000083c + 0x0070 * (txep))
#define HPS_SEQ_TYPE_FMASK			GENMASK(3, 0)
#define DPS_SEQ_TYPE_FMASK			GENMASK(7, 4)
#define HPS_REP_SEQ_TYPE_FMASK			GENMASK(11, 8)
#define DPS_REP_SEQ_TYPE_FMASK			GENMASK(15, 12)

/**
 * enum ipa_seq_type - HPS and DPS sequencer type fields in ENDP_INIT_SEQ_N
 * @IPA_SEQ_DMA_ONLY:		only DMA is performed
 * @IPA_SEQ_2ND_PKT_PROCESS_PASS_NO_DEC_UCP:
 *	second packet processing pass + no decipher + microcontroller
 * @IPA_SEQ_PKT_PROCESS_NO_DEC_NO_UCP_DMAP:
 *	packet processing + no decipher + no uCP + HPS REP DMA parser
 * @IPA_SEQ_INVALID:		invalid sequencer type
 *
 * The values defined here are broken into 4-bit nibbles that are written
 * into fields of the INIT_SEQ_N endpoint registers.
 */
enum ipa_seq_type {
	IPA_SEQ_DMA_ONLY			= 0x0000,
	IPA_SEQ_2ND_PKT_PROCESS_PASS_NO_DEC_UCP	= 0x0004,
	IPA_SEQ_PKT_PROCESS_NO_DEC_NO_UCP_DMAP	= 0x0806,
	IPA_SEQ_INVALID				= 0xffff,
};

#define IPA_REG_ENDP_STATUS_N_OFFSET(ep) \
					(0x00000840 + 0x0070 * (ep))
#define STATUS_EN_FMASK				GENMASK(0, 0)
#define STATUS_ENDP_FMASK			GENMASK(5, 1)
#define STATUS_LOCATION_FMASK			GENMASK(8, 8)
/* The next field is not present for IPA v3.5.1 */
#define STATUS_PKT_SUPPRESS_FMASK		GENMASK(9, 9)

/* The next register is only present for IPA versions that support hashing */
#define IPA_REG_ENDP_FILTER_ROUTER_HSH_CFG_N_OFFSET(er) \
					(0x0000085c + 0x0070 * (er))
#define FILTER_HASH_MSK_SRC_ID_FMASK		GENMASK(0, 0)
#define FILTER_HASH_MSK_SRC_IP_FMASK		GENMASK(1, 1)
#define FILTER_HASH_MSK_DST_IP_FMASK		GENMASK(2, 2)
#define FILTER_HASH_MSK_SRC_PORT_FMASK		GENMASK(3, 3)
#define FILTER_HASH_MSK_DST_PORT_FMASK		GENMASK(4, 4)
#define FILTER_HASH_MSK_PROTOCOL_FMASK		GENMASK(5, 5)
#define FILTER_HASH_MSK_METADATA_FMASK		GENMASK(6, 6)
#define IPA_REG_ENDP_FILTER_HASH_MSK_ALL	GENMASK(6, 0)

#define ROUTER_HASH_MSK_SRC_ID_FMASK		GENMASK(16, 16)
#define ROUTER_HASH_MSK_SRC_IP_FMASK		GENMASK(17, 17)
#define ROUTER_HASH_MSK_DST_IP_FMASK		GENMASK(18, 18)
#define ROUTER_HASH_MSK_SRC_PORT_FMASK		GENMASK(19, 19)
#define ROUTER_HASH_MSK_DST_PORT_FMASK		GENMASK(20, 20)
#define ROUTER_HASH_MSK_PROTOCOL_FMASK		GENMASK(21, 21)
#define ROUTER_HASH_MSK_METADATA_FMASK		GENMASK(22, 22)
#define IPA_REG_ENDP_ROUTER_HASH_MSK_ALL	GENMASK(22, 16)

#define IPA_REG_IRQ_STTS_OFFSET	\
				IPA_REG_IRQ_STTS_EE_N_OFFSET(GSI_EE_AP)
#define IPA_REG_IRQ_STTS_EE_N_OFFSET(ee) \
					(0x00003008 + 0x1000 * (ee))

#define IPA_REG_IRQ_EN_OFFSET \
				IPA_REG_IRQ_EN_EE_N_OFFSET(GSI_EE_AP)
#define IPA_REG_IRQ_EN_EE_N_OFFSET(ee) \
					(0x0000300c + 0x1000 * (ee))

#define IPA_REG_IRQ_CLR_OFFSET \
				IPA_REG_IRQ_CLR_EE_N_OFFSET(GSI_EE_AP)
#define IPA_REG_IRQ_CLR_EE_N_OFFSET(ee) \
					(0x00003010 + 0x1000 * (ee))
/**
 * enum ipa_irq_id - Bit positions representing type of IPA IRQ
 * @IPA_IRQ_UC_0:	Microcontroller event interrupt
 * @IPA_IRQ_UC_1:	Microcontroller response interrupt
 * @IPA_IRQ_TX_SUSPEND:	Data ready interrupt
 *
 * IRQ types not described above are not currently used.
 */
enum ipa_irq_id {
	IPA_IRQ_BAD_SNOC_ACCESS			= 0x0,
	/* Type (bit) 0x1 is not defined */
	IPA_IRQ_UC_0				= 0x2,
	IPA_IRQ_UC_1				= 0x3,
	IPA_IRQ_UC_2				= 0x4,
	IPA_IRQ_UC_3				= 0x5,
	IPA_IRQ_UC_IN_Q_NOT_EMPTY		= 0x6,
	IPA_IRQ_UC_RX_CMD_Q_NOT_FULL		= 0x7,
	IPA_IRQ_PROC_UC_ACK_Q_NOT_EMPTY		= 0x8,
	IPA_IRQ_RX_ERR				= 0x9,
	IPA_IRQ_DEAGGR_ERR			= 0xa,
	IPA_IRQ_TX_ERR				= 0xb,
	IPA_IRQ_STEP_MODE			= 0xc,
	IPA_IRQ_PROC_ERR			= 0xd,
	IPA_IRQ_TX_SUSPEND			= 0xe,
	IPA_IRQ_TX_HOLB_DROP			= 0xf,
	IPA_IRQ_BAM_GSI_IDLE			= 0x10,
	IPA_IRQ_PIPE_YELLOW_BELOW		= 0x11,
	IPA_IRQ_PIPE_RED_BELOW			= 0x12,
	IPA_IRQ_PIPE_YELLOW_ABOVE		= 0x13,
	IPA_IRQ_PIPE_RED_ABOVE			= 0x14,
	IPA_IRQ_UCP				= 0x15,
	IPA_IRQ_DCMP				= 0x16,
	IPA_IRQ_GSI_EE				= 0x17,
	IPA_IRQ_GSI_IPA_IF_TLV_RCVD		= 0x18,
	IPA_IRQ_GSI_UC				= 0x19,
	IPA_IRQ_COUNT,				/* Last; not an id */
};

#define IPA_REG_IRQ_UC_OFFSET \
				IPA_REG_IRQ_UC_EE_N_OFFSET(GSI_EE_AP)
#define IPA_REG_IRQ_UC_EE_N_OFFSET(ee) \
					(0x0000301c + 0x1000 * (ee))
#define UC_INTR_FMASK				GENMASK(0, 0)

/* ipa->available defines the valid bits in the SUSPEND_INFO register */
#define IPA_REG_IRQ_SUSPEND_INFO_OFFSET \
				IPA_REG_IRQ_SUSPEND_INFO_EE_N_OFFSET(GSI_EE_AP)
#define IPA_REG_IRQ_SUSPEND_INFO_EE_N_OFFSET(ee) \
					(0x00003030 + 0x1000 * (ee))

/* ipa->available defines the valid bits in the IRQ_SUSPEND_EN register */
#define IPA_REG_IRQ_SUSPEND_EN_OFFSET \
				IPA_REG_IRQ_SUSPEND_EN_EE_N_OFFSET(GSI_EE_AP)
#define IPA_REG_IRQ_SUSPEND_EN_EE_N_OFFSET(ee) \
					(0x00003034 + 0x1000 * (ee))

/* ipa->available defines the valid bits in the IRQ_SUSPEND_CLR register */
#define IPA_REG_IRQ_SUSPEND_CLR_OFFSET \
				IPA_REG_IRQ_SUSPEND_CLR_EE_N_OFFSET(GSI_EE_AP)
#define IPA_REG_IRQ_SUSPEND_CLR_EE_N_OFFSET(ee) \
					(0x00003038 + 0x1000 * (ee))

int ipa_reg_init(struct ipa *ipa);
void ipa_reg_exit(struct ipa *ipa);

#endif /* _IPA_REG_H_ */
