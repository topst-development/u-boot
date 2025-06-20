// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <regmap.h>
#include <mach/clock.h>

#ifndef HSIOBUS_DWC_OTG
#define HSIOBUS_DWC_OTG 0x0
#endif

#define PCFG0		(0x0U)
#define PCFG1		(0x4U)
#define PCFG2		(0x8U)
#define PCFG3		(0xCU)
#define PCFG4		(0x10U)
#define LCFG0		(0x18U)
#define CLK_RST_MUXSEL	(0x28U)
#define CFG_MST_ADDR	(0x68U)

#define FSEL_12MHz_24MHz 0x2
#define FSEL_24MHz 0x5
#define DOUBLE_BANDWIDTH 0x8

struct telechips_usb_data {
	bool hsio_reset;
	bool mux_sel;
	unsigned int addr_padding;
	unsigned int ref_clk;
};

struct tcc_dwc2_usb_phy {
	struct device   *dev;
	struct regmap   *reg_map;
};

static int32_t tcc_dwc2_usb_phy_init(struct phy *usb_phy)
{
	const struct udevice *dev;
	const struct tcc_dwc2_usb_phy *priv;
	const struct telechips_usb_data *data =
		(struct telechips_usb_data *)dev_get_driver_data(usb_phy->dev);
	int32_t ret = 0;

	if (usb_phy == NULL) {
		pr_err("[%s] Failed to find USB PHY\n", __func__);
		ret = -ENODEV;
	} else {
		dev = usb_phy->dev;
		priv = dev_get_priv(dev);

		if (data->mux_sel) {
			ret = regmap_write(priv->reg_map, CLK_RST_MUXSEL, 0x1F);
		}

		if (data->hsio_reset) {
			(void)tcc_set_hsiobus_swreset((int32_t)HSIOBUS_DWC_OTG, true);
		} else {
			ret = regmap_write(priv->reg_map, CLK_RST_MUXSEL, 0x3);
		}

		// Set USB 2.0 PHY Reference Clock
		(void)regmap_update_bits(priv->reg_map, PCFG0, 0xF, 0);
		(void)regmap_update_bits(priv->reg_map, PCFG0, 0xF, data->ref_clk);

		// Reset clock domain
		(void)regmap_update_bits(priv->reg_map, LCFG0, BIT(29), 0);

		(void)regmap_update_bits(priv->reg_map, PCFG0,
				(uint)(BIT(31) | BIT(24) | BIT(20)), 0);
		mdelay(4);
		(void)regmap_update_bits(priv->reg_map, LCFG0, BIT(29), BIT(29));

		if (data->addr_padding) {
			(void)regmap_update_bits(priv->reg_map, CFG_MST_ADDR,
				       BIT(0), BIT(0));
		}

		if (data->hsio_reset) {
			(void)tcc_set_hsiobus_swreset((int32_t)HSIOBUS_DWC_OTG, false);
		} else {
			(void)regmap_write(priv->reg_map, CLK_RST_MUXSEL, 0x7);
		}
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

static const struct telechips_usb_data dwc2_phy_data = {
	.hsio_reset = true,
	.mux_sel = true,
	.ref_clk = FSEL_12MHz_24MHz | DOUBLE_BANDWIDTH,
};

static const struct telechips_usb_data tcn100x_dwc2_phy_data = {
	.hsio_reset = false,
	.mux_sel = false,
	.addr_padding = 0x1,
	.ref_clk = FSEL_12MHz_24MHz | DOUBLE_BANDWIDTH,
};

struct telechip_usb_data {
	bool hsio_reset;
};

static const struct udevice_id tcc_dwc2_usb_phy_ids[] = {
	{ .compatible = "telechips,tcc_dwc_otg_phy",
	  .data = (ulong)&dwc2_phy_data },
	{ .compatible = "telechips,tcn100x-dwc2-phy",
	  .data = (ulong)&tcn100x_dwc2_phy_data },
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
