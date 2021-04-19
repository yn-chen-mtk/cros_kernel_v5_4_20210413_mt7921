// SPDX-License-Identifier: ISC
/*
 * Copyright (C) 2019 Lorenzo Bianconi <lorenzo@kernel.org>
 */

#include "mt76.h"
#include <linux/pci.h>

#define  PCIE_L1SS_CAP_CHK \
	(PCI_L1SS_CAP_PCIPM_L1_1 | PCI_L1SS_CAP_PCIPM_L1_2 | \
	PCI_L1SS_CAP_ASPM_L1_1 |PCI_L1SS_CAP_ASPM_L1_2)
#define  PCIE_L1SS_CTL_CHK \
	(PCI_L1SS_CTL1_PCIPM_L1_1 | PCI_L1SS_CTL1_PCIPM_L1_2 | \
	PCI_L1SS_CTL1_ASPM_L1_1 |PCI_L1SS_CTL1_ASPM_L1_2)

static void _mt76_pci_config_L1(struct pci_dev *pdev, u8 enable)
{
	u32 reg32;
	int pos;

	if (!pdev)
		return;

	/* capability check */
	pos = pdev->pcie_cap;
	pci_read_config_dword(pdev, pos + PCI_EXP_LNKCAP, &reg32);
	if (!(reg32 & PCI_EXP_LNKCAP_ASPMS)) {
		dev_dbg(&pdev->dev, "ASPM L1: Invalid cap 0x%X\n", reg32);
		return;
	}

	/* set config */
	pci_read_config_dword(pdev, pos + PCI_EXP_LNKCTL, &reg32);
	if (enable) {
		if (reg32 & PCI_EXP_LNKCTL_ASPMC)
			return;
		reg32 |= (PCI_EXP_LNKCTL_ASPMC);
	} else {
		if (!(reg32 & PCI_EXP_LNKCTL_ASPMC))
			return;
		reg32 &= ~(PCI_EXP_LNKCTL_ASPMC);
	}
	dev_dbg(&pdev->dev, "%s ASPM L1\n", (enable)?"enabling":"disabling");

	pci_write_config_dword(pdev, pos + PCI_EXP_LNKCTL, reg32);
}

static void _mt76_pci_config_L1ss(struct pci_dev *pdev, u8 enable)
{
	int pos;
	u32 reg32;

	if (!pdev)
		return;

	/* capability check */
	pos = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_L1SS);
	pci_read_config_dword(pdev, pos + PCI_L1SS_CAP, &reg32);
	if (!(reg32 & (PCIE_L1SS_CAP_CHK))) {
		dev_dbg(&pdev->dev, "ASPM L1SS: Invalid cap 0x%X\n", reg32);
		return;
	}

	/* set config */
	pci_read_config_dword(pdev, pos + PCI_L1SS_CTL1, &reg32);
	if (enable) {
		if (reg32 & PCIE_L1SS_CTL_CHK)
			return;
		reg32 |= (PCIE_L1SS_CTL_CHK);
	} else {
		if (!(reg32 & PCIE_L1SS_CTL_CHK))
			return;
		reg32 &= ~(PCIE_L1SS_CTL_CHK);
	}

	dev_dbg(&pdev->dev, "%s ASPM L1SS\n", (enable)?"enabling":"disabling");

	pci_write_config_dword(pdev, pos + PCI_L1SS_CTL1, reg32);
}

void mt76_pci_disable_aspm(struct pci_dev *pdev)
{
	_mt76_pci_config_L1(pdev, false);
	_mt76_pci_config_L1ss(pdev, false);
}
EXPORT_SYMBOL_GPL(mt76_pci_disable_aspm);

void mt76_pci_enable_aspm(struct pci_dev *pdev)
{
	_mt76_pci_config_L1ss(pdev, true);
	_mt76_pci_config_L1(pdev, true);
}
EXPORT_SYMBOL_GPL(mt76_pci_enable_aspm);
