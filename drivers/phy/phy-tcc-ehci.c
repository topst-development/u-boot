// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 * Author(s): Jeong Su Kim, <deviance@telechips.com>
 */

#include <asm/io.h>
#include <dm.h>
#include <generic-phy.h>
#include <irq.h>
#include <mach/clock.h>
#include <regmap.h>

#define USB20H_PCFG0    (0x4U)
#define USB20H_PCFG1    (0x8U)
#define USB20H_PCFG2    (0xCU)
#define USB20H_PCFG3    (0x10U)
#define USB20H_PCFG4    (0x14U)
#define USB20H_LCFG0    (0x1CU)

struct tcc_ehci_usb_phy {
	struct device   *dev;
	struct regmap   *reg_map;
	bool mux_port;
	int32_t irq_bc;
	int32_t irq_ls;
};

static void tcc_ehci_usb_print_pcfg_reset_value(const struct phy *usb_phy)
{
	const struct udevice *dev = usb_phy->dev;
	const struct tcc_ehci_usb_phy *priv = dev_get_priv(dev);
	uint32_t val;

	(void)regmap_read(priv->reg_map, USB20H_PCFG1, &val);
	dev_info(dev, "[EHCI USB 2.0 PHY PCFG1]\n");
	dev_info(dev, "Reset value : 0x%08x\n", val);
}

static void tcc_ehci_usb_print_pcfg_set_value(const struct phy *usb_phy)
{
	const struct udevice *dev = usb_phy->dev;
	const struct tcc_ehci_usb_phy *priv = dev_get_priv(dev);
	uint32_t pcfg1 = 0xE31C2433U;
	uint32_t val;

	(void)regmap_read(priv->reg_map, USB20H_PCFG1, &val);
	dev_info(dev, "[EHCI USB 2.0 PHY PCFG1]\n");
	dev_info(dev, "The value we are going to set is 0x%08x\n", pcfg1);
	dev_info(dev, "Actually set value : 0x%08x\n", val);
}

static void tcc_ehci_usb_phy_irq_handler(void *arg)
{
	const struct udevice *dev = arg;
	const struct tcc_ehci_usb_phy *priv = dev_get_priv(dev);
	uint32_t val;

	(void)regmap_read(priv->reg_map, USB20H_PCFG4, &val);
	dev_info(dev, "[%s] Charger detection interrupt!\n", __func__);
	dev_info(dev, "Charger detection IRQ bit(24): %d (PCFG4: 0x%08x)\n",
			((val & BIT(24)) != 0U) ? 1 : 0, val);

	(void)regmap_update_bits(priv->reg_map, USB20H_PCFG2,
			(uint)BIT(9), (uint)BIT(9));
	mdelay(50);
	(void)regmap_update_bits(priv->reg_map, USB20H_PCFG2,
			(uint)(BIT(8) | BIT(9)), 0);

	(void)regmap_update_bits(priv->reg_map, USB20H_PCFG4,
			(uint)BIT(31), (uint)BIT(31));
	udelay(1);
	(void)regmap_update_bits(priv->reg_map, USB20H_PCFG4,
			(uint)BIT(31), 0);

	dev_info(dev, "[%s] Enable charger detection!!!\n", __func__);
}

static void tcc_ehci_usb_line_state_irq_handler(void *arg)
{
	const struct udevice *dev = arg;
	const struct tcc_ehci_usb_phy *priv = dev_get_priv(dev);
	uint32_t val;

	(void)regmap_read(priv->reg_map, USB20H_PCFG0, &val);
	dev_info(dev, "[%s] Line state interrupt!\n", __func__);
	dev_info(dev, "INTR_RSTn: %d, WKUP_RSTn: %d (PCFG0: 0x%08x)\n",
			((val & BIT(23)) != 0U) ? 1 : 0,
			((val & BIT(22)) != 0U) ? 1 : 0, val);

	if (priv->mux_port) {
		dev_info(dev, "Wakeup source: %d\n",
				(((*(volatile uint32_t *)0x14400020) &
				  BIT(6)) != 0U) ? 1 : 0);
	} else {
		dev_info(dev, "Wakeup Source: %d\n",
				(((*(volatile uint32_t *)0x14400020) &
				  BIT(5)) != 0U) ? 1 : 0);
	}

	(void)regmap_update_bits(priv->reg_map, USB20H_PCFG0,
			(uint)BIT(23), 0);
	udelay(5);
	(void)regmap_update_bits(priv->reg_map, USB20H_PCFG0,
			(uint)BIT(23), (uint)BIT(23));
}

static int32_t tcc_ehci_usb_phy_init(struct phy *usb_phy)
{
	const struct udevice *dev;
	const struct tcc_ehci_usb_phy *priv;
	int32_t ret = 0;
	int32_t i = 0;
	uint32_t *otg_phy = phys_to_virt(USB20OTG_MUX_SEL);
	uint32_t pcfg0 = 0x83000025U;
	uint32_t pcfg1 = 0xE31C2433U;
	uint32_t pcfg2 = 0x0;
	uint32_t pcfg3 = 0x0;
	uint32_t pcfg4 = 0x0;
	uint32_t lcfg0 = 0x30048020;
	uint32_t val;

	if (usb_phy == NULL) {
		dev_err(dev, "[%s] Failed to find USB PHY\n", __func__);
		ret = -ENODEV;
	} else {
		dev = usb_phy->dev;
		priv = dev_get_priv(dev);

		/* MUX SELECTION = OTG Controller */
		if (priv->mux_port) {
			*otg_phy = 0x18;

			(void)tcc_set_hsiobus_swreset((int32_t)HSIOBUS_DWC_OTG,
					true);
		} else {
			(void)tcc_set_hsiobus_swreset((int32_t)HSIOBUS_USB20H,
					true);
		}

		tcc_ehci_usb_print_pcfg_reset_value(usb_phy);

		/* Reset PHY */
		(void)regmap_write(priv->reg_map, USB20H_PCFG0, pcfg0);
		(void)regmap_write(priv->reg_map, USB20H_PCFG1, pcfg1);
		(void)regmap_write(priv->reg_map, USB20H_PCFG2, pcfg2);
		(void)regmap_write(priv->reg_map, USB20H_PCFG3, pcfg3);
		(void)regmap_write(priv->reg_map, USB20H_PCFG4, pcfg4);
		(void)regmap_write(priv->reg_map, USB20H_LCFG0, lcfg0);

		tcc_ehci_usb_print_pcfg_set_value(usb_phy);

		/* Set the POR */
		(void)regmap_update_bits(priv->reg_map, USB20H_PCFG0,
				(uint)BIT(31), (uint)BIT(31));

		/* Set the Core Reset */
		(void)regmap_update_bits(priv->reg_map, USB20H_LCFG0,
				(uint)(BIT(29) | BIT(28)), 0);
		udelay(10);

		/* Release POR & Clear SIDDQ */
		(void)regmap_update_bits(priv->reg_map, USB20H_PCFG0,
				(uint)BIT(31), 0);
		(void)regmap_update_bits(priv->reg_map, USB20H_PCFG0,
				(uint)BIT(24), 0);

		/* Set PHYVALID Enable */
		(void)regmap_update_bits(priv->reg_map, USB20H_PCFG4,
				(uint)BIT(30), (uint)BIT(30));

		/* Set DP and DM to pull down */
		(void)regmap_update_bits(priv->reg_map, USB20H_PCFG4,
				(uint)(BIT(12) | BIT(10)),
				(uint)(BIT(12) | BIT(10)));

		while (i < 10000) {
			(void)regmap_read(priv->reg_map, USB20H_PCFG4, &val);
			if ((val & BIT(27)) != 0U) {
				dev_info(dev, "USB 2.0 PHY Check Valid!\n");
				dev_info(dev, "PHY Valid Bit(27): %d (PCFG1: 0x%08x)\n",
						((val & BIT(27)) != 0U) ? 1 : 0,
						val);
				break;
			}

			i++;
			udelay(5);
		}

		(void)regmap_update_bits(priv->reg_map, USB20H_LCFG0,
				(uint)(BIT(29) | BIT(28)),
				(uint)(BIT(29) | BIT(28)));

		if (priv->mux_port) {
			(void)tcc_set_hsiobus_swreset((int32_t)HSIOBUS_DWC_OTG,
					false);
		} else {
			(void)tcc_set_hsiobus_swreset((int32_t)HSIOBUS_USB20H,
					false);
		}

		if (priv->irq_bc != 0) {
			/* Clear IRQ */
			(void)regmap_update_bits(priv->reg_map, USB20H_PCFG4,
					(uint)BIT(31), (uint)BIT(31));

			/* Disable PHY-Valid Detect */
			(void)regmap_update_bits(priv->reg_map, USB20H_PCFG4,
					(uint)BIT(30), 0);

			/* Clear IRQ */
			(void)regmap_update_bits(priv->reg_map, USB20H_PCFG4,
					(uint)BIT(31), 0);
			udelay(1);
			(void)regmap_update_bits(priv->reg_map, USB20H_PCFG2,
					(uint)(BIT(8) | BIT(10)),
					(uint)(BIT(8) | BIT(10)));
			udelay(1);

			/* Enable CHG_DET interrupt */
			(void)regmap_update_bits(priv->reg_map, USB20H_PCFG4,
					(uint)BIT(28), (uint)BIT(28));

			irq_unmask(priv->irq_bc);
		}

		if (priv->irq_ls != 0) {
			(void)regmap_update_bits(priv->reg_map, USB20H_PCFG0,
					(uint)(BIT(22) | BIT(23)),
					(uint)(BIT(22) | BIT(23)));

			irq_unmask(priv->irq_ls);
		}
	}

	return ret;
}

static int32_t tcc_ehci_usb_phy_exit(struct phy *usb_phy)
{
	const struct udevice *dev;
	const struct tcc_ehci_usb_phy *priv;
	int32_t ret = 0;

	if (usb_phy == NULL) {
		dev_err(dev, "[%s] Failed to find USB PHY\n", __func__);
		ret = -ENODEV;
	} else {
		dev = usb_phy->dev;
		priv = dev_get_priv(dev);

		(void)regmap_update_bits(priv->reg_map, USB20H_PCFG4,
				(uint)BIT(31), (uint)BIT(31));
		udelay(1);
		(void)regmap_update_bits(priv->reg_map, USB20H_PCFG4,
				(uint)BIT(31), 0);
		(void)regmap_update_bits(priv->reg_map, USB20H_PCFG2,
				(uint)BIT(8), (uint)BIT(8));

		irq_mask(priv->irq_bc);
		irq_mask(priv->irq_ls);
	}

	return ret;
}

static struct phy_ops tcc_ehci_usb_phy_ops = {
	.init = tcc_ehci_usb_phy_init,
	.exit = tcc_ehci_usb_phy_exit,
};

static int32_t tcc_ehci_usb_phy_probe(struct udevice *dev)
{
	struct tcc_ehci_usb_phy *priv = dev_get_priv(dev);
	int32_t ret;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->reg_map);
	if (ret != 0) {
		dev_err(dev, "[%s] Failed to regmap_init_mem()\n", __func__);
	} else {
		if (dev_read_bool(dev, "mux-port")) {
			priv->mux_port = true;
		} else {
			priv->mux_port = false;
		}

		if (dev_read_bool(dev, "support-bc12")) {
			if (priv->mux_port) {
				priv->irq_bc = 148 + (int32_t)INT_GIC_OFFSET;
			} else {
				priv->irq_bc = 143 + (int32_t)INT_GIC_OFFSET;
			}

			irq_install_handler(priv->irq_bc,
					(interrupt_handler_t *)
					tcc_ehci_usb_phy_irq_handler, dev);
			irq_mask(priv->irq_bc);
		}

		if (dev_read_bool(dev, "support-line-state")) {
			if (priv->mux_port) {
				priv->irq_ls = 149 + (int32_t)INT_GIC_OFFSET;
			} else {
				priv->irq_ls = 144 + (int32_t)INT_GIC_OFFSET;
			}

			irq_install_handler(priv->irq_ls,
					(interrupt_handler_t *)
					tcc_ehci_usb_line_state_irq_handler,
					dev);
			irq_mask(priv->irq_ls);
		}
	}

	return ret;
}

static const struct udevice_id tcc_ehci_usb_phy_ids[] = {
	{ .compatible = "telechips,tcc_ehci_phy" },
	{ },
};

U_BOOT_DRIVER(tcc_ehci_usb_phy) = {
	.name           = "tcc_ehci_phy",
	.id             = UCLASS_PHY,
	.of_match       = tcc_ehci_usb_phy_ids,
	.ops            = &tcc_ehci_usb_phy_ops,
	.probe          = tcc_ehci_usb_phy_probe,
	.priv_auto_alloc_size = sizeof(struct tcc_ehci_usb_phy),
};
