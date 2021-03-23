# SPDX-License-Identifier: GPL-2.0-only
#
export CPTCFG_MAC80211=m
export CPTCFG_MAC80211_LEDS=y
export CPTCFG_MAC80211_DEBUGFS=y
export CPTCFG_MAC80211_RC_MINSTREL=y

subdir-ccflags-y := -I$(srctree)/$(src) -include "hdrs/mt76-chrome.h"
subdir-ccflags-y += -I$(srctree)/$(src)/hdrs/

subdir-ccflags-y += $(call cc-option,-Wno-error=date-time)
subdir-ccflags-y += $(call cc-option,-Wno-date-time)
# otherwise ieee80211_band vs. nl80211_band warnings come up
subdir-ccflags-y += $(call cc-option,-Wno-enum-compare)
subdir-ccflags-y += $(call cc-option,-Wno-enum-conversion)
subdir-ccflags-y += $(call cc-option,-Wno-int-in-bool-context)
# cfg80211 compatibility porting mechanism sometimes causes these
subdir-ccflags-y += $(call cc-option,-Wno-unused-const-variable)
subdir-ccflags-y += $(call cc-option,-Wno-unused-variable)

obj-$(CONFIG_MT76_CORE) += mac80211/
obj-$(CONFIG_MT76_CORE) += mt76.o
obj-$(CONFIG_MT76_USB) += mt76-usb.o
obj-$(CONFIG_MT76_SDIO) += mt76-sdio.o
obj-$(CONFIG_MT76x02_LIB) += mt76x02-lib.o
obj-$(CONFIG_MT76x02_USB) += mt76x02-usb.o
obj-$(CONFIG_MT76_CONNAC_LIB) += mt76-connac-lib.o

mt76-y := \
	mmio.o util.o trace.o dma.o mac80211.o debugfs.o eeprom.o \
	tx.o agg-rx.o mcu.o

mt76-$(CONFIG_PCI) += pci.o
mt76-$(CONFIG_NL80211_TESTMODE) += testmode.o

mt76-usb-y := usb.o usb_trace.o
mt76-sdio-y := sdio.o

CFLAGS_trace.o := -I$(src)
CFLAGS_usb_trace.o := -I$(src)
CFLAGS_mt76x02_trace.o := -I$(src)

mt76x02-lib-y := mt76x02_util.o mt76x02_mac.o mt76x02_mcu.o \
		 mt76x02_eeprom.o mt76x02_phy.o mt76x02_mmio.o \
		 mt76x02_txrx.o mt76x02_trace.o mt76x02_debugfs.o \
		 mt76x02_dfs.o mt76x02_beacon.o

mt76x02-usb-y := mt76x02_usb_mcu.o mt76x02_usb_core.o

mt76-connac-lib-y := mt76_connac_mcu.o mt76_connac_mac.o

obj-$(CONFIG_MT76x0_COMMON) += mt76x0/
obj-$(CONFIG_MT76x2_COMMON) += mt76x2/
obj-$(CONFIG_MT7603E) += mt7603/
obj-$(CONFIG_MT7615_COMMON) += mt7615/
obj-$(CONFIG_MT7915E) += mt7915/
obj-$(CONFIG_MT7921E) += mt7921/
