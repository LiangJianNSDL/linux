// SPDX-License-Identifier: GPL-2.0
/*
 * Loongson PCI Host Controller Driver
 *
 * Copyright (C) 2020 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <linux/of_device.h>
#include <linux/of_pci.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/pci-acpi.h>
#include <linux/pci-ecam.h>

#include "../pci.h"

/* Device IDs */
#define DEV_PCIE_PORT_0	0x7a09
#define DEV_PCIE_PORT_1	0x7a19
#define DEV_PCIE_PORT_2	0x7a29

#define DEV_LS2K_APB	0x7a02
#define DEV_LS7A_CONF	0x7a10
#define DEV_LS7A_LPC	0x7a0c
#define DEV_LS7A_GMAC	0x7a03
#define DEV_LS7A_DC1	0x7a06
#define DEV_LS7A_DC2	0x7a36
#define DEV_LS7A_GPU	0x7a15
#define DEV_LS7A_AHCI	0x7a08
#define DEV_LS7A_EHCI	0x7a14
#define DEV_LS7A_OHCI	0x7a24

#define FLAG_CFG0	BIT(0)
#define FLAG_CFG1	BIT(1)
#define FLAG_DEV_FIX	BIT(2)

struct pci_controller_data {
	u32 flags;
	struct pci_ops *ops;
};

struct loongson_pci {
	void __iomem *cfg0_base;
	void __iomem *cfg1_base;
	struct platform_device *pdev;
	struct pci_controller_data *data;
};

/* Fixup wrong class code in PCIe bridges */
static void bridge_class_quirk(struct pci_dev *dev)
{
	dev->class = PCI_CLASS_BRIDGE_PCI << 8;
}
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LOONGSON,
			DEV_PCIE_PORT_0, bridge_class_quirk);
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LOONGSON,
			DEV_PCIE_PORT_1, bridge_class_quirk);
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LOONGSON,
			DEV_PCIE_PORT_2, bridge_class_quirk);

static void system_bus_quirk(struct pci_dev *pdev)
{
	/*
	 * The address space consumed by these devices is outside the
	 * resources of the host bridge.
	 */
	pdev->mmio_always_on = 1;
	pdev->non_compliant_bars = 1;
}
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LOONGSON,
			DEV_LS2K_APB, system_bus_quirk);
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LOONGSON,
			DEV_LS7A_CONF, system_bus_quirk);
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LOONGSON,
			DEV_LS7A_LPC, system_bus_quirk);

static void loongson_mrrs_quirk(struct pci_dev *pdev)
{
	/*
	 * Some Loongson PCIe ports have h/w limitations of maximum read
	 * request size. They can't handle anything larger than this. So
	 * force this limit on any devices attached under these ports.
	 */
	struct pci_host_bridge *bridge = pci_find_host_bridge(pdev->bus);

	if (!bridge)
		return;

	bridge->no_inc_mrrs = 1;
}
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LOONGSON,
			DEV_PCIE_PORT_0, loongson_mrrs_quirk);
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LOONGSON,
			DEV_PCIE_PORT_1, loongson_mrrs_quirk);
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LOONGSON,
			DEV_PCIE_PORT_2, loongson_mrrs_quirk);

static void loongson_bmaster_quirk(struct pci_dev *pdev)
{
	/*
	 * Some Loongson PCIe ports will cause CPU deadlock if disable
	 * the Bus Master bit during poweroff/reboot.
	 */
	struct pci_host_bridge *bridge = pci_find_host_bridge(pdev->bus);

	if (!bridge)
		return;

	bridge->no_dis_bmaster = 1;
}
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LOONGSON,
			DEV_PCIE_PORT_0, loongson_bmaster_quirk);
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LOONGSON,
			DEV_PCIE_PORT_1, loongson_bmaster_quirk);
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_LOONGSON,
			DEV_PCIE_PORT_2, loongson_bmaster_quirk);

static void loongson_pci_pin_quirk(struct pci_dev *pdev)
{
	pdev->pin = 1 + (PCI_FUNC(pdev->devfn) & 3);
}
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_LOONGSON,
			DEV_LS7A_DC1, loongson_pci_pin_quirk);
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_LOONGSON,
			DEV_LS7A_DC2, loongson_pci_pin_quirk);
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_LOONGSON,
			DEV_LS7A_GPU, loongson_pci_pin_quirk);
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_LOONGSON,
			DEV_LS7A_GMAC, loongson_pci_pin_quirk);
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_LOONGSON,
			DEV_LS7A_AHCI, loongson_pci_pin_quirk);
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_LOONGSON,
			DEV_LS7A_EHCI, loongson_pci_pin_quirk);
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_LOONGSON,
			DEV_LS7A_OHCI, loongson_pci_pin_quirk);
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_LOONGSON,
			DEV_PCIE_PORT_0, loongson_pci_pin_quirk);
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_LOONGSON,
			DEV_PCIE_PORT_1, loongson_pci_pin_quirk);
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_LOONGSON,
			DEV_PCIE_PORT_2, loongson_pci_pin_quirk);

static struct loongson_pci *pci_bus_to_loongson_pci(struct pci_bus *bus)
{
	struct pci_config_window *cfg;

	if (acpi_disabled)
		return (struct loongson_pci *)(bus->sysdata);
	else {
		cfg = bus->sysdata;
		return (struct loongson_pci *)(cfg->priv);
	}
}

static void __iomem *cfg1_map(struct loongson_pci *priv, int bus,
				unsigned int devfn, int where)
{
	unsigned long addroff = 0x0;

	if (bus != 0)
		addroff |= BIT(28); /* Type 1 Access */
	addroff |= (where & 0xff) | ((where & 0xf00) << 16);
	addroff |= (bus << 16) | (devfn << 8);
	return priv->cfg1_base + addroff;
}

static void __iomem *cfg0_map(struct loongson_pci *priv, int bus,
				unsigned int devfn, int where)
{
	unsigned long addroff = 0x0;

	if (bus != 0)
		addroff |= BIT(24); /* Type 1 Access */
	addroff |= (bus << 16) | (devfn << 8) | where;
	return priv->cfg0_base + addroff;
}

static void __iomem *pci_loongson_map_bus(struct pci_bus *bus, unsigned int devfn,
			       int where)
{
	unsigned char busnum = bus->number;
	unsigned int device = PCI_SLOT(devfn);
	unsigned int function = PCI_FUNC(devfn);
	struct loongson_pci *priv = pci_bus_to_loongson_pci(bus);

	if (pci_is_root_bus(bus))
		busnum = 0;

	/*
	 * Do not read more than one device on the bus other than
	 * the host bus. For our hardware the root bus is always bus 0.
	 */
	if ((priv->data->flags & FLAG_DEV_FIX) && bus->self) {
		if ((busnum != 0) && (device > 0))
			return NULL;
		if ((busnum == 0) && (device >= 9 && device <= 20 && function > 0))
			return NULL;
	}

	/* CFG0 can only access standard space */
	if (where < PCI_CFG_SPACE_SIZE && priv->cfg0_base)
		return cfg0_map(priv, busnum, devfn, where);

	/* CFG1 can access extended space */
	if (where < PCI_CFG_SPACE_EXP_SIZE && priv->cfg1_base)
		return cfg1_map(priv, busnum, devfn, where);

	return NULL;
}

#ifdef CONFIG_OF

static int loongson_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;
	u8 val;

	irq = of_irq_parse_and_map_pci(dev, slot, pin);
	if (irq > 0)
		return irq;

	/* Care i8259 legacy systems */
	pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &val);
	/* i8259 only have 15 IRQs */
	if (val > 15)
		return 0;

	return val;
}

/* LS2K/LS7A accept 8/16/32-bit PCI config operations */
static struct pci_ops loongson_pci_ops = {
	.map_bus = pci_loongson_map_bus,
	.read	= pci_generic_config_read,
	.write	= pci_generic_config_write,
};

/* RS780/SR5690 only accept 32-bit PCI config operations */
static struct pci_ops loongson_pci_ops32 = {
	.map_bus = pci_loongson_map_bus,
	.read	= pci_generic_config_read32,
	.write	= pci_generic_config_write32,
};

static const struct pci_controller_data ls2k_pci_data = {
	.flags = FLAG_CFG1 | FLAG_DEV_FIX,
	.ops = &loongson_pci_ops,
};

static const struct pci_controller_data ls7a_pci_data = {
	.flags = FLAG_CFG1 | FLAG_DEV_FIX,
	.ops = &loongson_pci_ops,
};

static const struct pci_controller_data rs780e_pci_data = {
	.flags = FLAG_CFG0,
	.ops = &loongson_pci_ops32,
};

static const struct of_device_id loongson_pci_of_match[] = {
	{ .compatible = "loongson,ls2k-pci",
		.data = (void *)&ls2k_pci_data, },
	{ .compatible = "loongson,ls7a-pci",
		.data = (void *)&ls7a_pci_data, },
	{ .compatible = "loongson,rs780e-pci",
		.data = (void *)&rs780e_pci_data, },
	{}
};

static int loongson_pci_probe(struct platform_device *pdev)
{
	struct loongson_pci *priv;
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct pci_host_bridge *bridge;
	struct resource *regs;

	if (!node)
		return -ENODEV;

	bridge = devm_pci_alloc_host_bridge(dev, sizeof(*priv));
	if (!bridge)
		return -ENODEV;

	priv = pci_host_bridge_priv(bridge);
	priv->pdev = pdev;
	priv->data = (struct pci_controller_data *)of_device_get_match_data(dev);

	if (priv->data->flags & FLAG_CFG0) {
		regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (!regs)
			dev_err(dev, "missing mem resources for cfg0\n");
		else {
			priv->cfg0_base = devm_pci_remap_cfg_resource(dev, regs);
			if (IS_ERR(priv->cfg0_base))
				return PTR_ERR(priv->cfg0_base);
		}
	}

	if (priv->data->flags & FLAG_CFG1) {
		regs = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		if (!regs)
			dev_info(dev, "missing mem resource for cfg1\n");
		else {
			priv->cfg1_base = devm_pci_remap_cfg_resource(dev, regs);
			if (IS_ERR(priv->cfg1_base))
				priv->cfg1_base = NULL;
		}
	}

	bridge->sysdata = priv;
	bridge->ops = priv->data->ops;
	bridge->map_irq = loongson_map_irq;

	return pci_host_probe(bridge);
}

static struct platform_driver loongson_pci_driver = {
	.driver = {
		.name = "loongson-pci",
		.of_match_table = loongson_pci_of_match,
	},
	.probe = loongson_pci_probe,
};
builtin_platform_driver(loongson_pci_driver);

#endif

#ifdef CONFIG_ACPI

static int loongson_pci_ecam_init(struct pci_config_window *cfg)
{
	struct device *dev = cfg->parent;
	struct loongson_pci *priv;
	struct pci_controller_data *data;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	cfg->priv = priv;
	data->flags = FLAG_CFG1 | FLAG_DEV_FIX,
	priv->data = data;
	priv->cfg1_base = cfg->win - (cfg->busr.start << 16);

	return 0;
}

const struct pci_ecam_ops loongson_pci_ecam_ops = {
	.bus_shift = 16,
	.init	   = loongson_pci_ecam_init,
	.pci_ops   = {
		.map_bus = pci_loongson_map_bus,
		.read	 = pci_generic_config_read,
		.write	 = pci_generic_config_write,
	}
};

#endif
