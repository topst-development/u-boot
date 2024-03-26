// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <regmap.h>
#include <mach/clock.h>

#define U20DH_PCFG0     (0x0U)
#define U20DH_LCFG0     (0x18U)
#define U20DH_MUXSEL    (0x28U)

struct tcc_dwc2_usb_phy {
	struct device   *dev;
	struct regmap   *reg_map;
};

static int32_t tcc_dwc2_usb_phy_init(struct phy *usb_phy)
{
	const struct udevice *dev;
	const struct tcc_dwc2_usb_phy *priv;
	int32_t ret = 0;

	if (usb_phy == NULL) {
		pr_err("[%s] Failed to find USB PHY\n", __func__);
		ret = -ENODEV;
	} else {
		dev = usb_phy->dev;
		priv = dev_get_priv(dev);

		/* MUX SELECTION = OTG Controller */
		(void)regmap_write(priv->reg_map, U20DH_MUXSEL, 0x1F);
		(void)tcc_set_hsiobus_swreset((int32_t)HSIOBUS_DWC_OTG, true);

#ifdef CONFIG_TCC807X
		// Set USB 2.0 PHY Reference Clock Frequency
		(void)regmap_update_bits(priv->reg_map, U20DH_PCFG0,
				(uint)(BIT(2) | BIT(0)), 0);
		(void)regmap_update_bits(priv->reg_map, U20DH_PCFG0,
				(uint)(BIT(3) | BIT(1)),
				(uint)(BIT(3) | BIT(1)));
#endif

		/* PHY POR SET*/
		(void)regmap_write(priv->reg_map, U20DH_LCFG0, 0x0);
		(void)regmap_update_bits(priv->reg_map, U20DH_PCFG0,
				(uint)(BIT(31) | BIT(24) | BIT(20)), 0);
		mdelay(4);
		(void)regmap_update_bits(priv->reg_map, U20DH_LCFG0,
				(uint)BIT(29), (uint)BIT(29));

		(void)tcc_set_hsiobus_swreset((int32_t)HSIOBUS_DWC_OTG, false);
	}

	return ret;
}

static int32_t tcc_dwc2_usb_phy_exit(struct phy *usb_phy)
{
	(void)usb_phy;

	return 0;
}

static struct phy_ops tcc_dwc2_usb_phy_ops = {
	.init = tcc_dwc2_usb_phy_init,
	.exit = tcc_dwc2_usb_phy_exit,
};

static int32_t tcc_dwc2_usb_phy_probe(struct udevice *dev)
{
	struct tcc_dwc2_usb_phy *priv = dev_get_priv(dev);
	int32_t ret;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->reg_map);
	if (ret != 0) {
		pr_err("[%s] Failed to regmap_init_mem()\n", __func__);
	}

	return ret;
}

static const struct udevice_id tcc_dwc2_usb_phy_ids[] = {
	{ .compatible = "telechips,tcc_dwc_otg_phy" },
	{ },
};

U_BOOT_DRIVER(tcc_dwc2_usb_phy) = {
	.name      = "tcc_dwc2_phy",
	.id        = UCLASS_PHY,
	.of_match  = tcc_dwc2_usb_phy_ids,
	.ops       = &tcc_dwc2_usb_phy_ops,
	.probe     = tcc_dwc2_usb_phy_probe,
	.priv_auto = sizeof(struct tcc_dwc2_usb_phy),
};
