/*
 * ChromeOS backport definitions
 * Copyright (C) 2015-2017 Intel Deutschland GmbH
 * Copyright (C) 2018-2019 Intel Corporation
 */
#include <linux/if_ether.h>
#include <net/cfg80211.h>
#include <linux/errqueue.h>
#include <generated/utsrelease.h>
/* ipv6_addr_is_multicast moved - include old header */
#include <net/addrconf.h>
#include <net/ieee80211_radiotap.h>
#include <crypto/hash.h>

/* make sure we include iw_handler.h to get wireless_nlevent_flush() */
#include <net/iw_handler.h>

/* common backward compat code */

#define BACKPORTS_GIT_TRACKED "chromium:" UTS_RELEASE
#define BACKPORTS_BUILD_TSTAMP __DATE__ " " __TIME__

/* Dummy RHEL macros */
#define RHEL_RELEASE_CODE 0
#define RHEL_RELEASE_VERSION(a,b) 1

#ifndef fallthrough
#define fallthrough /* fall through */
#endif

#define netif_threaded_napi_add netif_napi_add

#ifndef netdev_alloc_pcpu_stats
#define netdev_alloc_pcpu_stats(type)				\
({								\
	typeof(type) __percpu *pcpu_stats = alloc_percpu(type); \
	if (pcpu_stats)	{					\
		int i;						\
		for_each_possible_cpu(i) {			\
			typeof(type) *stat;			\
			stat = per_cpu_ptr(pcpu_stats, i);	\
			u64_stats_init(&stat->syncp);		\
		}						\
	}							\
	pcpu_stats;						\
})
#endif /* netdev_alloc_pcpu_stats */

#define netdev_tstats(dev)	dev->tstats
#define netdev_assign_tstats(dev, e)	dev->tstats = (e);

static inline void netdev_attach_ops(struct net_device *dev,
				     const struct net_device_ops *ops)
{
	dev->netdev_ops = ops;
}

#define WIPHY_FLAG_HAS_CHANNEL_SWITCH 0

#define WIPHY_PARAM_DYN_ACK		(1 << 5)

#define mc_addr(ha)	(ha)->addr

#define NL80211_FEATURE_STATIC_SMPS		(1 << 24)
#define NL80211_FEATURE_DYNAMIC_SMPS		(1 << 25)
#define NL80211_FEATURE_SUPPORTS_WMM_ADMISSION	(1 << 26)
/* cannot be supported on this kernel */
#define NL80211_FEATURE_TDLS_CHANNEL_SWITCH	0

static inline bool
cfg80211_ibss_userspace_handles_dfs(struct cfg80211_ibss_params *params)
{
	return params->userspace_handles_dfs;
}

static inline bool wdev_cac_started(struct wireless_dev *wdev)
{
	return wdev->cac_started;
}

#define const_since_3_16 const

static inline unsigned int
csa_n_counter_offsets_beacon(struct cfg80211_csa_settings *s)
{
	return s->n_counter_offsets_beacon;
}

static inline unsigned int
csa_n_counter_offsets_presp(struct cfg80211_csa_settings *s)
{
	return s->n_counter_offsets_presp;
}

static inline const u16 *
csa_counter_offsets_beacon(struct cfg80211_csa_settings *s)
{
	return s->counter_offsets_beacon;
}

static inline const u16 *
csa_counter_offsets_presp(struct cfg80211_csa_settings *s)
{
	return s->counter_offsets_presp;
}

#define cfg80211_ap_settings_smps_mode(params) ((params)->smps_mode)

/* backport wiphy_ext_feature_set/_isset
 *
 * To do so, define our own versions thereof that check for a negative
 * feature index and in that case ignore it entirely. That allows us to
 * define the ones that the cfg80211 version doesn't support to -1.
 */
static inline void mt76_wiphy_ext_feature_set(struct wiphy *wiphy, int ftidx)
{
	if (ftidx < 0)
		return;
	wiphy_ext_feature_set(wiphy, ftidx);
}

static inline bool mt76_wiphy_ext_feature_isset(struct wiphy *wiphy,
						   int ftidx)
{
	if (ftidx < 0)
		return false;
	return wiphy_ext_feature_isset(wiphy, ftidx);
}
#define wiphy_ext_feature_set mt76_wiphy_ext_feature_set
#define wiphy_ext_feature_isset mt76_wiphy_ext_feature_isset

static inline
size_t mt76_get_auth_data_len(struct cfg80211_auth_request *req)
{
#if CFG80211_VERSION < KERNEL_VERSION(4,10,0)
	return req->sae_data_len;
#else
	return req->auth_data_len;
#endif
}

static inline
const u8 *mt76_get_auth_data(struct cfg80211_auth_request *req)
{
#if CFG80211_VERSION < KERNEL_VERSION(4,10,0)
	return req->sae_data;
#else
	return req->auth_data;
#endif
}

static inline
size_t mt76_get_fils_kek_len(struct cfg80211_assoc_request *req)
{
#if CFG80211_VERSION < KERNEL_VERSION(4,10,0)
	return 0;
#else
	return req->fils_kek_len;
#endif
}

static inline
const u8 *mt76_get_fils_kek(struct cfg80211_assoc_request *req)
{
#if CFG80211_VERSION < KERNEL_VERSION(4,10,0)
	return NULL;
#else
	return req->fils_kek;
#endif
}

static inline
const u8 *mt76_get_fils_nonces(struct cfg80211_assoc_request *req)
{
#if CFG80211_VERSION < KERNEL_VERSION(4,10,0)
	return NULL;
#else
	return req->fils_nonces;
#endif
}


struct backport_sinfo {
	u32 filled;
	u32 connected_time;
	u32 inactive_time;
	u64 rx_bytes;
	u64 tx_bytes;
	u16 llid;
	u16 plid;
	u8 plink_state;
	s8 signal;
	s8 signal_avg;

	u8 chains;
	s8 chain_signal[IEEE80211_MAX_CHAINS];
	s8 chain_signal_avg[IEEE80211_MAX_CHAINS];

	struct rate_info txrate;
	struct rate_info rxrate;
	u32 rx_packets;
	u32 tx_packets;
	u32 tx_retries;
	u32 tx_failed;
	u32 rx_dropped_misc;
	struct sta_bss_parameters bss_param;
	struct nl80211_sta_flag_update sta_flags;

	int generation;

	const u8 *assoc_req_ies;
	size_t assoc_req_ies_len;

	u32 beacon_loss_count;
	s64 t_offset;
	enum nl80211_mesh_power_mode local_pm;
	enum nl80211_mesh_power_mode peer_pm;
	enum nl80211_mesh_power_mode nonpeer_pm;

	u32 expected_throughput;

	u64 rx_beacon;
	u8 rx_beacon_signal_avg;
#if CFG80211_VERSION < KERNEL_VERSION(4,18,0)
	/*
	 * With < 4.18 we use an array here, like before, so we don't
	 * need to alloc/free it
	 */
	struct cfg80211_tid_stats pertid[IEEE80211_NUM_TIDS + 1];
#else
	struct cfg80211_tid_stats *pertid;
#endif
	s8 ack_signal;
	s8 avg_ack_signal;
};

/* these are constants in nl80211.h, so it's
 * harmless to define them unconditionally
 */
#define NL80211_STA_INFO_RX_DROP_MISC		28
#define NL80211_STA_INFO_BEACON_RX		29
#define NL80211_STA_INFO_BEACON_SIGNAL_AVG	30
#define NL80211_STA_INFO_TID_STATS		31
#define NL80211_TID_STATS_RX_MSDU		1
#define NL80211_TID_STATS_TX_MSDU		2
#define NL80211_TID_STATS_TX_MSDU_RETRIES	3
#define NL80211_TID_STATS_TX_MSDU_FAILED	4

static inline void mt76_convert_sinfo(struct backport_sinfo *bpsinfo,
					 struct station_info *sinfo)
{
	memset(sinfo, 0, sizeof(*sinfo));
#define COPY(x)	sinfo->x = bpsinfo->x
#define MCPY(x)	memcpy(&sinfo->x, &bpsinfo->x, sizeof(sinfo->x))
	COPY(connected_time);
	COPY(inactive_time);
	COPY(rx_bytes);
	COPY(tx_bytes);
	COPY(llid);
	COPY(plid);
	COPY(plink_state);
	COPY(signal);
	COPY(signal_avg);
	COPY(chains);
	MCPY(chain_signal);
	MCPY(chain_signal_avg);
	COPY(txrate);
	COPY(rxrate);
	COPY(rx_packets);
	COPY(tx_packets);
	COPY(tx_retries);
	COPY(tx_failed);
	COPY(rx_dropped_misc);
	COPY(bss_param);
	COPY(sta_flags);
	COPY(generation);
	COPY(assoc_req_ies);
	COPY(assoc_req_ies_len);
	COPY(beacon_loss_count);
	COPY(t_offset);
	COPY(local_pm);
	COPY(peer_pm);
	COPY(nonpeer_pm);
	COPY(expected_throughput);
#if CFG80211_VERSION >= KERNEL_VERSION(4,18,0)
	COPY(ack_signal);
	COPY(avg_ack_signal);
#endif
	COPY(rx_beacon);
	COPY(rx_beacon_signal_avg);
	MCPY(pertid);
	COPY(filled);
#undef COPY
}
typedef struct station_info cfg_station_info_t;
#define station_info backport_sinfo

static inline void
backport_cfg80211_new_sta(struct net_device *dev, const u8 *mac_addr,
			  struct station_info *sinfo, gfp_t gfp)
{
	cfg_station_info_t cfg_info;

	mt76_convert_sinfo(sinfo, &cfg_info);
	cfg80211_new_sta(dev, mac_addr, &cfg_info, gfp);
}
#define cfg80211_new_sta backport_cfg80211_new_sta

static inline void
backport_cfg80211_del_sta_sinfo(struct net_device *dev, const u8 *mac_addr,
				struct station_info *sinfo, gfp_t gfp)
{
	cfg_station_info_t cfg_info;

	mt76_convert_sinfo(sinfo, &cfg_info);
	cfg80211_del_sta_sinfo(dev, mac_addr, &cfg_info, gfp);
}
#define cfg80211_del_sta_sinfo backport_cfg80211_del_sta_sinfo

typedef struct survey_info cfg_survey_info_t;
#if CFG80211_VERSION < KERNEL_VERSION(5,5,0)
#define survey_info bp_survey_info
struct survey_info {
	struct ieee80211_channel *channel;
	u64 time;
	u64 time_busy;
	u64 time_ext_busy;
	u64 time_rx;
	u64 time_tx;
	u64 time_scan;
	u64 time_bss_rx;
	u32 filled;
	s8 noise;
};
#define SURVEY_INFO_TIME_BSS_RX 0
static inline void mt76_convert_survey_info(struct survey_info *survey,
					    cfg_survey_info_t *cfg)
{
	cfg->channel = survey->channel;
	cfg->time = survey->time;
	cfg->time_busy = survey->time_busy;
	cfg->time_ext_busy = survey->time_ext_busy;
	cfg->time_rx = survey->time_rx;
	cfg->time_tx = survey->time_tx;
	cfg->time_scan = survey->time_scan;
	cfg->noise = survey->noise;
	cfg->filled = survey->filled;
}
#else
typedef struct survey_info cfg_survey_info_t;
static inline void mt76_convert_survey_info(struct survey_info *survey,
					    cfg_survey_info_t *cfg)
{
	memcpy(cfg, survey, sizeof(*cfg));
}
#endif

static inline bool ieee80211_viftype_ocb(unsigned int iftype)
{
	return iftype == NL80211_IFTYPE_OCB;
}

static inline bool ieee80211_viftype_nan(unsigned int iftype)
{
	return iftype == NL80211_IFTYPE_NAN;
}

static inline
bool ieee80211_has_nan_iftype(unsigned int iftype)
{
	return iftype & BIT(NL80211_IFTYPE_NAN);
}

#if CFG80211_VERSION < KERNEL_VERSION(99,0,0)
#define nan_conf_cdw_2g(conf) 1
#define nan_conf_cdw_5g(conf) 1
#else
#define nan_conf_cdw_2g(conf) ((conf)->cdw_2g)
#define nan_conf_cdw_5g(conf) ((conf)->cdw_5g)
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,20,0)
#define beacon_ftm_len(beacon, m) 0
#else
#define beacon_ftm_len(beacon, m) ((beacon)->m)
#endif

static inline int
cfg80211_sta_support_p2p_ps(struct station_parameters *params, bool p2p_go)
{
	return params->support_p2p_ps;
}

static inline u8*
cfg80211_scan_req_bssid(struct cfg80211_scan_request *scan_req)
{
	return scan_req->bssid;
}

static inline
bool ieee80211_nan_has_band(struct cfg80211_nan_conf *conf, u8 band)
{
	return conf->bands & BIT(band);
}

static inline
void ieee80211_nan_set_band(struct cfg80211_nan_conf *conf, u8 band)
{
	conf->bands |= BIT(band);
}

static inline u8 ieee80211_nan_bands(struct cfg80211_nan_conf *conf)
{
	return conf->bands;
}

#if CFG80211_VERSION < KERNEL_VERSION(4,19,0)
#define IEEE80211_HE_PPE_THRES_MAX_LEN		25

/**
 * enum nl80211_he_gi - HE guard interval
 * @NL80211_RATE_INFO_HE_GI_0_8: 0.8 usec
 * @NL80211_RATE_INFO_HE_GI_1_6: 1.6 usec
 * @NL80211_RATE_INFO_HE_GI_3_2: 3.2 usec
 */
enum nl80211_he_gi {
	NL80211_RATE_INFO_HE_GI_0_8,
	NL80211_RATE_INFO_HE_GI_1_6,
	NL80211_RATE_INFO_HE_GI_3_2,
};

/**
 * @enum nl80211_he_ru_alloc - HE RU allocation values
 * @NL80211_RATE_INFO_HE_RU_ALLOC_26: 26-tone RU allocation
 * @NL80211_RATE_INFO_HE_RU_ALLOC_52: 52-tone RU allocation
 * @NL80211_RATE_INFO_HE_RU_ALLOC_106: 106-tone RU allocation
 * @NL80211_RATE_INFO_HE_RU_ALLOC_242: 242-tone RU allocation
 * @NL80211_RATE_INFO_HE_RU_ALLOC_484: 484-tone RU allocation
 * @NL80211_RATE_INFO_HE_RU_ALLOC_996: 996-tone RU allocation
 * @NL80211_RATE_INFO_HE_RU_ALLOC_2x996: 2x996-tone RU allocation
 */
enum nl80211_he_ru_alloc {
	NL80211_RATE_INFO_HE_RU_ALLOC_26,
	NL80211_RATE_INFO_HE_RU_ALLOC_52,
	NL80211_RATE_INFO_HE_RU_ALLOC_106,
	NL80211_RATE_INFO_HE_RU_ALLOC_242,
	NL80211_RATE_INFO_HE_RU_ALLOC_484,
	NL80211_RATE_INFO_HE_RU_ALLOC_996,
	NL80211_RATE_INFO_HE_RU_ALLOC_2x996,
};

#define RATE_INFO_BW_HE_RU	(RATE_INFO_BW_160 + 1)

/**
 * struct ieee80211_sta_he_cap - STA's HE capabilities
 *
 * This structure describes most essential parameters needed
 * to describe 802.11ax HE capabilities for a STA.
 *
 * @has_he: true iff HE data is valid.
 * @he_cap_elem: Fixed portion of the HE capabilities element.
 * @he_mcs_nss_supp: The supported NSS/MCS combinations.
 * @ppe_thres: Holds the PPE Thresholds data.
 */
struct ieee80211_sta_he_cap {
	bool has_he;
	struct ieee80211_he_cap_elem he_cap_elem;
	struct ieee80211_he_mcs_nss_supp he_mcs_nss_supp;
	u8 ppe_thres[IEEE80211_HE_PPE_THRES_MAX_LEN];
};

/**
 * struct ieee80211_sband_iftype_data
 *
 * This structure encapsulates sband data that is relevant for the interface
 * types defined in %types
 *
 * @types_mask: interface types (bits)
 * @he_cap: holds the HE capabilities
 */
struct ieee80211_sband_iftype_data {
	u16 types_mask;
	struct ieee80211_sta_he_cap he_cap;
};
#endif

#if CFG80211_VERSION < KERNEL_VERSION(5,3,0)
/**
 * ieee80211_get_he_sta_cap - return HE capabilities for an sband's STA
 * @sband: the sband to search for the STA on
 *
 * Return: pointer to the struct ieee80211_sta_he_cap, or NULL is none found
 *	Currently, not supported
 */
#define ieee80211_get_he_sta_cap backport_ieee80211_get_he_sta_cap
static inline const struct ieee80211_sta_he_cap *
ieee80211_get_he_sta_cap(const struct ieee80211_supported_band *sband)
{
	return NULL;
}

static inline const struct ieee80211_sta_he_cap *
ieee80211_get_he_iftype_cap(const struct ieee80211_supported_band *sband,
			    u8 iftype)
{
	return NULL;
}

/**
 * struct ieee80211_he_obss_pd - AP settings for spatial reuse
 *
 * @enable: is the feature enabled.
 * @min_offset: minimal tx power offset an associated station shall use
 * @max_offset: maximum tx power offset an associated station shall use
 */
struct ieee80211_he_obss_pd {
	bool enable;
	u8 min_offset;
	u8 max_offset;
};
#endif

#ifndef SHASH_DESC_ON_STACK
#define SHASH_DESC_ON_STACK(shash, ctx)				 \
	char __##shash##_desc[sizeof(struct shash_desc) +	 \
	       crypto_shash_descsize(ctx)] CRYPTO_MINALIGN_ATTR; \
	struct shash_desc *shash = (struct shash_desc *)__##shash##_desc

#endif

#define possible_write_pnet(pnet, net) write_pnet(pnet, net)
#define possible_read_pnet(pnet) read_pnet(pnet)

#define netdev_set_priv_destructor(_dev, _destructor) \
	(_dev)->needs_free_netdev = true; \
	(_dev)->priv_destructor = (_destructor);
#define netdev_set_def_destructor(_dev) \
	(_dev)->needs_free_netdev = true;

static inline void set_rate_info_bw(struct rate_info *ri, int bw)
{
	ri->bw = bw;
}

#if LINUX_VERSION_IS_LESS(4,20,0)
static inline struct sk_buff *__skb_peek(const struct sk_buff_head *list_)
{
	return list_->next;
}
#endif

#if LINUX_VERSION_IS_LESS(4,15,0)
#define NL80211_EXT_FEATURE_FILS_MAX_CHANNEL_TIME -1
#define NL80211_EXT_FEATURE_ACCEPT_BCAST_PROBE_RESP -1
#define NL80211_EXT_FEATURE_OCE_PROBE_REQ_HIGH_TX_RATE -1
#define NL80211_EXT_FEATURE_OCE_PROBE_REQ_DEFERRAL_SUPPRESSION -1
#define NL80211_SCAN_FLAG_FILS_MAX_CHANNEL_TIME BIT(4)
#define NL80211_SCAN_FLAG_ACCEPT_BCAST_PROBE_RESP BIT(5)
#define NL80211_SCAN_FLAG_OCE_PROBE_REQ_HIGH_TX_RATE BIT(6)
#define NL80211_SCAN_FLAG_OCE_PROBE_REQ_DEFERRAL_SUPPRESSION BIT(7)
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,4,0) ||		\
	(CFG80211_VERSION >= KERNEL_VERSION(4,5,0) &&	\
	 CFG80211_VERSION < KERNEL_VERSION(4,17,0))
struct ieee80211_wmm_ac {
	u16 cw_min;
	u16 cw_max;
	u16 cot;
	u8 aifsn;
};

struct ieee80211_wmm_rule {
	struct ieee80211_wmm_ac client[IEEE80211_NUM_ACS];
	struct ieee80211_wmm_ac ap[IEEE80211_NUM_ACS];
};

static inline int
reg_query_regdb_wmm(char *alpha2, int freq, u32 *ptr,
		    struct ieee80211_wmm_rule *rule)
{
	pr_debug_once(KERN_DEBUG
		      "mt76: ETSI WMM data not implemented yet!\n");
	return -ENODATA;
}
#endif /* < 4.4.0 || (>= 4.5.0 && < 4.17.0) */

#if CFG80211_VERSION < KERNEL_VERSION(99,0,0)
/* not yet upstream */
static inline bool ieee80211_viftype_nan_data(unsigned int iftype)
{
	return false;
}

static inline bool ieee80211_has_nan_data_iftype(unsigned int iftype)
{
	return false;
}
#endif

#if CFG80211_VERSION < KERNEL_VERSION(99,0,0)
/* not yet upstream */
static inline int
cfg80211_crypto_n_ciphers_group(struct cfg80211_crypto_settings *crypto)
{
	return 1;
}

static inline u32
cfg80211_crypto_ciphers_group(struct cfg80211_crypto_settings *crypto,
			      int idx)
{
	WARN_ON(idx != 0);
	return crypto->cipher_group;
}
#else
static inline int
cfg80211_crypto_n_ciphers_group(struct cfg80211_crypto_settings *crypto)
{
	return crypto->n_ciphers_group;
}

static inline u32
cfg80211_crypto_ciphers_group(struct cfg80211_crypto_settings *crypto,
			      int idx)
{
	return crypto->cipher_groups[idx];
}
#endif

#ifndef VHT_MUMIMO_GROUPS_DATA_LEN
#define VHT_MUMIMO_GROUPS_DATA_LEN (WLAN_MEMBERSHIP_LEN +\
				    WLAN_USER_POSITION_LEN)
#endif

#if CFG80211_VERSION >= KERNEL_VERSION(4,20,0)
#define cfg_he_cap(params) params->he_cap
#else
#define cfg_he_cap(params) NULL

/* Layer 2 Update frame (802.2 Type 1 LLC XID Update response) */
struct iapp_layer2_update {
	u8 da[ETH_ALEN];	/* broadcast */
	u8 sa[ETH_ALEN];	/* STA addr */
	__be16 len;		/* 6 */
	u8 dsap;		/* 0 */
	u8 ssap;		/* 0 */
	u8 control;
	u8 xid_info[3];
} __packed;

static inline
void cfg80211_send_layer2_update(struct net_device *dev, const u8 *addr)
{
	struct iapp_layer2_update *msg;
	struct sk_buff *skb;

	/* Send Level 2 Update Frame to update forwarding tables in layer 2
	 * bridge devices */

	skb = dev_alloc_skb(sizeof(*msg));
	if (!skb)
		return;
	msg = skb_put(skb, sizeof(*msg));

	/* 802.2 Type 1 Logical Link Control (LLC) Exchange Identifier (XID)
	 * Update response frame; IEEE Std 802.2-1998, 5.4.1.2.1 */

	eth_broadcast_addr(msg->da);
	ether_addr_copy(msg->sa, addr);
	msg->len = htons(6);
	msg->dsap = 0;
	msg->ssap = 0x01;	/* NULL LSAP, CR Bit: Response */
	msg->control = 0xaf;	/* XID response lsb.1111F101.
				 * F=0 (no poll command; unsolicited frame) */
	msg->xid_info[0] = 0x81;	/* XID format identifier */
	msg->xid_info[1] = 1;	/* LLC types/classes: Type 1 LLC */
	msg->xid_info[2] = 0;	/* XID sender's receive window size (RW) */

	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	memset(skb->cb, 0, sizeof(skb->cb));
	netif_rx_ni(skb);
}

#define NL80211_EXT_FEATURE_CAN_REPLACE_PTK0 -1

int ieee80211_get_vht_max_nss(struct ieee80211_vht_cap *cap,
			      enum ieee80211_vht_chanwidth bw,
			      int mcs, bool ext_nss_bw_capable);
#endif /* >= 4.20 */

#if CFG80211_VERSION < KERNEL_VERSION(4,19,0)
#define NL80211_EXT_FEATURE_SCAN_RANDOM_SN		-1
#define NL80211_EXT_FEATURE_SCAN_MIN_PREQ_CONTENT	-1
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,20,0)
#define NL80211_EXT_FEATURE_ENABLE_FTM_RESPONDER	-1
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,17,0)
#define NL80211_EXT_FEATURE_CONTROL_PORT_OVER_NL80211	-1

/* define it here so we can set the values in mac80211... */
struct sta_opmode_info {
	u32 changed;
	enum nl80211_smps_mode smps_mode;
	enum nl80211_chan_width bw;
	u8 rx_nss;
};

#define STA_OPMODE_MAX_BW_CHANGED	0
#define STA_OPMODE_SMPS_MODE_CHANGED	0
#define STA_OPMODE_N_SS_CHANGED		0

/* ...but make the user an empty function, since we don't have it in cfg80211 */
#define cfg80211_sta_opmode_change_notify(...)  do { } while (0)

/*
 * we should never call this function since we force
 * cfg_control_port_over_nl80211 to be 0.
 */
#define cfg80211_rx_control_port(...) do { } while (0)

#define cfg_control_port_over_nl80211(params) 0
#else
#if CFG80211_VERSION >= KERNEL_VERSION(4,17,0) && \
	CFG80211_VERSION < KERNEL_VERSION(4,18,0)
static inline bool mt76_cfg80211_rx_control_port(struct net_device *dev,
				    struct sk_buff *skb, bool unencrypted)
{
	struct ethhdr *ehdr;

	/*
	 * Try to linearize the skb, because in 4.17
	 * cfg80211_rx_control_port() is broken and needs it to be
	 * linear.  If it fails, too bad, we fail too.
	 */
	if (skb_linearize(skb))
		return false;

	ehdr = eth_hdr(skb);

	return cfg80211_rx_control_port(dev, skb->data, skb->len,
				ehdr->h_source,
				be16_to_cpu(skb->protocol), unencrypted);
}
#define cfg80211_rx_control_port mt76_cfg80211_rx_control_port
#endif
#define cfg_control_port_over_nl80211(params) (params)->control_port_over_nl80211
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,18,0)
#define RATE_INFO_FLAGS_HE_MCS 0
#define NL80211_EXT_FEATURE_TXQS -1

/*
 * This function just allocates tidstats and returns 0 if it
 * succeeded.  Since pre-4.18 tidstats is pre-allocated as part of
 * sinfo, we can simply return 0 because it's already allocated.
 */
#define cfg80211_sinfo_alloc_tid_stats(...) 0

#define WIPHY_PARAM_TXQ_LIMIT		0
#define WIPHY_PARAM_TXQ_MEMORY_LIMIT	0
#define WIPHY_PARAM_TXQ_QUANTUM		0

#define ieee80211_data_to_8023_exthdr mt76_ieee80211_data_to_8023_exthdr
int ieee80211_data_to_8023_exthdr(struct sk_buff *skb, struct ethhdr *ehdr,
				  const u8 *addr, enum nl80211_iftype iftype,
				  u8 data_offset);
#else
static inline int
backport_cfg80211_sinfo_alloc_tid_stats(struct station_info *sinfo, gfp_t gfp)
{
	int ret;
	cfg_station_info_t cfg_info = {};

	ret = cfg80211_sinfo_alloc_tid_stats(&cfg_info, gfp);
	if (ret)
		return ret;

	sinfo->pertid = cfg_info.pertid;

	return 0;
}
#define cfg80211_sinfo_alloc_tid_stats backport_cfg80211_sinfo_alloc_tid_stats
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,19,0)
#define NL80211_SCAN_FLAG_RANDOM_SN		0
#define NL80211_SCAN_FLAG_MIN_PREQ_CONTENT	0
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,19,0) */

#if CFG80211_VERSION < KERNEL_VERSION(4,20,0)
enum nl80211_ftm_responder_stats {
	__NL80211_FTM_STATS_INVALID,
	NL80211_FTM_STATS_SUCCESS_NUM,
	NL80211_FTM_STATS_PARTIAL_NUM,
	NL80211_FTM_STATS_FAILED_NUM,
	NL80211_FTM_STATS_ASAP_NUM,
	NL80211_FTM_STATS_NON_ASAP_NUM,
	NL80211_FTM_STATS_TOTAL_DURATION_MSEC,
	NL80211_FTM_STATS_UNKNOWN_TRIGGERS_NUM,
	NL80211_FTM_STATS_RESCHEDULE_REQUESTS_NUM,
	NL80211_FTM_STATS_OUT_OF_WINDOW_TRIGGERS_NUM,
	NL80211_FTM_STATS_PAD,

	/* keep last */
	__NL80211_FTM_STATS_AFTER_LAST,
	NL80211_FTM_STATS_MAX = __NL80211_FTM_STATS_AFTER_LAST - 1
};

struct cfg80211_ftm_responder_stats {
	u32 filled;
	u32 success_num;
	u32 partial_num;
	u32 failed_num;
	u32 asap_num;
	u32 non_asap_num;
	u64 total_duration_ms;
	u32 unknown_triggers_num;
	u32 reschedule_requests_num;
	u32 out_of_window_triggers_num;
};
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,20,0) */

#ifndef ETH_P_PREAUTH
#define ETH_P_PREAUTH  0x88C7	/* 802.11 Preauthentication */
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,21,0)
enum nl80211_preamble {
	NL80211_PREAMBLE_LEGACY,
	NL80211_PREAMBLE_HT,
	NL80211_PREAMBLE_VHT,
	NL80211_PREAMBLE_DMG,
};

enum nl80211_peer_measurement_status {
	NL80211_PMSR_STATUS_SUCCESS,
	NL80211_PMSR_STATUS_REFUSED,
	NL80211_PMSR_STATUS_TIMEOUT,
	NL80211_PMSR_STATUS_FAILURE,
};

enum nl80211_peer_measurement_type {
	NL80211_PMSR_TYPE_INVALID,

	NL80211_PMSR_TYPE_FTM,

	NUM_NL80211_PMSR_TYPES,
	NL80211_PMSR_TYPE_MAX = NUM_NL80211_PMSR_TYPES - 1
};

enum nl80211_peer_measurement_ftm_failure_reasons {
	NL80211_PMSR_FTM_FAILURE_UNSPECIFIED,
	NL80211_PMSR_FTM_FAILURE_NO_RESPONSE,
	NL80211_PMSR_FTM_FAILURE_REJECTED,
	NL80211_PMSR_FTM_FAILURE_WRONG_CHANNEL,
	NL80211_PMSR_FTM_FAILURE_PEER_NOT_CAPABLE,
	NL80211_PMSR_FTM_FAILURE_INVALID_TIMESTAMP,
	NL80211_PMSR_FTM_FAILURE_PEER_BUSY,
	NL80211_PMSR_FTM_FAILURE_BAD_CHANGED_PARAMS,
};

struct cfg80211_pmsr_ftm_result {
	const u8 *lci;
	const u8 *civicloc;
	unsigned int lci_len;
	unsigned int civicloc_len;
	enum nl80211_peer_measurement_ftm_failure_reasons failure_reason;
	u32 num_ftmr_attempts, num_ftmr_successes;
	s16 burst_index;
	u8 busy_retry_time;
	u8 num_bursts_exp;
	u8 burst_duration;
	u8 ftms_per_burst;
	s32 rssi_avg;
	s32 rssi_spread;
	struct rate_info tx_rate, rx_rate;
	s64 rtt_avg;
	s64 rtt_variance;
	s64 rtt_spread;
	s64 dist_avg;
	s64 dist_variance;
	s64 dist_spread;

	u16 num_ftmr_attempts_valid:1,
	    num_ftmr_successes_valid:1,
	    rssi_avg_valid:1,
	    rssi_spread_valid:1,
	    tx_rate_valid:1,
	    rx_rate_valid:1,
	    rtt_avg_valid:1,
	    rtt_variance_valid:1,
	    rtt_spread_valid:1,
	    dist_avg_valid:1,
	    dist_variance_valid:1,
	    dist_spread_valid:1;
};

struct cfg80211_pmsr_result {
	u64 host_time, ap_tsf;
	enum nl80211_peer_measurement_status status;

	u8 addr[ETH_ALEN];

	u8 final:1,
	   ap_tsf_valid:1;

	enum nl80211_peer_measurement_type type;

	union {
		struct cfg80211_pmsr_ftm_result ftm;
	};
};

struct cfg80211_pmsr_ftm_request_peer {
	enum nl80211_preamble preamble;
	u16 burst_period;
	u8 requested:1,
	   asap:1,
	   request_lci:1,
	   request_civicloc:1;
	u8 num_bursts_exp;
	u8 burst_duration;
	u8 ftms_per_burst;
	u8 ftmr_retries;
};

struct cfg80211_pmsr_request_peer {
	u8 addr[ETH_ALEN];
	struct cfg80211_chan_def chandef;
	u8 report_ap_tsf:1;
	struct cfg80211_pmsr_ftm_request_peer ftm;
};

struct cfg80211_pmsr_request {
	u64 cookie;
	void *drv_data;
	u32 n_peers;
	u32 nl_portid;

	u32 timeout;

	u8 mac_addr[ETH_ALEN] __aligned(2);
	u8 mac_addr_mask[ETH_ALEN] __aligned(2);

	struct list_head list;

	struct cfg80211_pmsr_request_peer peers[];
};

static inline void cfg80211_pmsr_report(struct wireless_dev *wdev,
					struct cfg80211_pmsr_request *req,
					struct cfg80211_pmsr_result *result,
					gfp_t gfp)
{
	/* nothing */
}

static inline void cfg80211_pmsr_complete(struct wireless_dev *wdev,
					  struct cfg80211_pmsr_request *req,
					  gfp_t gfp)
{
	kfree(req);
}

#endif /* CFG80211_VERSION < KERNEL_VERSION(4,21,0) */

#if CFG80211_VERSION < KERNEL_VERSION(4, 19, 0)
static inline void
ieee80211_sband_set_num_iftypes_data(struct ieee80211_supported_band *sband,
				     u16 n)
{
}

static inline u16
ieee80211_sband_get_num_iftypes_data(struct ieee80211_supported_band *sband)
{
	return 0;
}

static inline void
ieee80211_sband_set_iftypes_data(struct ieee80211_supported_band *sband,
				 const struct ieee80211_sband_iftype_data *data)
{
}

static inline struct ieee80211_sband_iftype_data *
ieee80211_sband_get_iftypes_data(struct ieee80211_supported_band *sband)
{
	return NULL;
}

static inline struct ieee80211_sband_iftype_data *
ieee80211_sband_get_iftypes_data_entry(struct ieee80211_supported_band *sband,
				       u16 i)
{
	WARN_ONCE(1,
		  "Tried to use unsupported sband iftype data\n");
	return NULL;
}
#else  /* CFG80211_VERSION < KERNEL_VERSION(4,19,0) */
static inline void
ieee80211_sband_set_num_iftypes_data(struct ieee80211_supported_band *sband,
				     u16 n)
{
	sband->n_iftype_data = n;
}

static inline u16
ieee80211_sband_get_num_iftypes_data(struct ieee80211_supported_band *sband)
{
	return sband->n_iftype_data;
}

static inline void
ieee80211_sband_set_iftypes_data(struct ieee80211_supported_band *sband,
				 const struct ieee80211_sband_iftype_data *data)
{
	sband->iftype_data = data;
}

static inline const struct ieee80211_sband_iftype_data *
ieee80211_sband_get_iftypes_data(struct ieee80211_supported_band *sband)
{
	return sband->iftype_data;
}

static inline const struct ieee80211_sband_iftype_data *
ieee80211_sband_get_iftypes_data_entry(struct ieee80211_supported_band *sband,
				       u16 i)
{
	return &sband->iftype_data[i];
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,19,0) */

#if CFG80211_VERSION < KERNEL_VERSION(5,0,0)
#define NL80211_EXT_FEATURE_AIRTIME_FAIRNESS -1
#endif

#if CFG80211_VERSION < KERNEL_VERSION(5,1,0)
#define NL80211_EXT_FEATURE_EXT_KEY_ID -1
static inline int cfg80211_vendor_cmd_get_sender(struct wiphy *wiphy)
{
	/* cfg80211 doesn't really let us backport this */
	return 0;
}

static inline struct sk_buff *
cfg80211_vendor_event_alloc_ucast(struct wiphy *wiphy,
				  struct wireless_dev *wdev,
				  unsigned int portid, int approxlen,
				  int event_idx, gfp_t gfp)
{
	/*
	 * We might be able to fake backporting this, but not the
	 * associated changes to __cfg80211_send_event_skb(), at
	 * least not without duplicating all that code.
	 * And in any case, we cannot backport the get_sender()
	 * function above properly, so we might as well ignore
	 * this all.
	 */
	return NULL;
}

static inline const struct element *
cfg80211_find_elem(u8 eid, const u8 *ies, int len)
{
	return (void *)cfg80211_find_ie(eid, ies, len);
}

static inline const struct element *
cfg80211_find_ext_elem(u8 ext_eid, const u8 *ies, int len)
{
	return (void *)cfg80211_find_ext_ie(ext_eid, ies, len);
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(5,1,0) */

#if CFG80211_VERSION < KERNEL_VERSION(5,3,0)
static inline void cfg80211_bss_iter(struct wiphy *wiphy,
				     struct cfg80211_chan_def *chandef,
				     void (*iter)(struct wiphy *wiphy,
						  struct cfg80211_bss *bss,
						  void *data),
				     void *iter_data)
{
	/*
	 * It might be possible to backport this function, but that would
	 * require duplicating large portions of data structure/code, so
	 * leave it empty for now.
	 */
}

#define cfg80211_iftype_allowed backport_cfg80211_iftype_allowed
static inline
bool cfg80211_iftype_allowed(struct wiphy *wiphy, enum nl80211_iftype iftype,
			     bool is_4addr, u8 check_swif)
{
	return wiphy->software_iftypes & BIT(iftype);
}

#define RATE_INFO_FLAGS_DMG RATE_INFO_FLAGS_60G
#define RATE_INFO_FLAGS_HE 0
#define RATE_INFO_FLAGS_EDMG 0

#endif /* CFG80211_VERSION < KERNEL_VERSION(5,3,0) */

#if CFG80211_VERSION < KERNEL_VERSION(5,4,0)
#define NL80211_EXT_FEATURE_AQL -1
#endif

#ifndef IEEE80211_DEFAULT_AIRTIME_WEIGHT
#define IEEE80211_DEFAULT_AIRTIME_WEIGHT    256
#endif

/* The per TXQ device queue limit in airtime */
#ifndef IEEE80211_DEFAULT_AQL_TXQ_LIMIT_L
#define IEEE80211_DEFAULT_AQL_TXQ_LIMIT_L   5000
#endif
#ifndef IEEE80211_DEFAULT_AQL_TXQ_LIMIT_H
#define IEEE80211_DEFAULT_AQL_TXQ_LIMIT_H   12000
#endif

/* The per interface airtime threshold to switch to lower queue limit */
#ifndef IEEE80211_AQL_THRESHOLD
#define IEEE80211_AQL_THRESHOLD         24000
#endif
