/******************************************************************************
 *
 * Copyright(c) 2018-2019 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 *  Intel Linux Wireless <linuxwifi@intel.com>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 *****************************************************************************/
#ifndef __MT76_CHROME_CONFIG
#define __MT76_CHROME_CONFIG
/* This must match the CPTCFG_* symbols defined in the Makefile */
#define CPTCFG_MAC80211_MODULE 1
#define CPTCFG_MAC80211_LEDS 1
#define CPTCFG_MAC80211_HAS_RC 1
#define CPTCFG_MAC80211_RC_MINSTREL 1
#define CPTCFG_MAC80211_RC_DEFAULT_MINSTREL 1
#define CPTCFG_MAC80211_RC_DEFAULT "minstrel_ht"
#define CPTCFG_MAC80211_DEBUGFS 1
#define CPTCFG_MAC80211_STA_HASH_MAX_SIZE 0
#define CPTCFG_REJECT_NONUPSTREAM_NL80211 1

#define CPTCFG_IWL_TIMEOUT_FACTOR 1

#ifdef CONFIG_NL80211_TESTMODE
#define CPTCFG_NL80211_TESTMODE 1
#endif

#define CFG80211_VERSION LINUX_VERSION_CODE

#endif
