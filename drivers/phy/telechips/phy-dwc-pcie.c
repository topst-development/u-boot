// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <generic-phy.h>
#include <dm.h>
#include <asm/io.h>
#include <regmap.h>
#include <mach/clock.h>
#include <asm/arch/tcc_ckc.h>
#include <linux/soc/telechips/tcc-pcie-phy.h>

/*
 * PCIe controller wrapper phy configuration registers
 */
#define PCIE_PHY_REG08		(0x020U)
#define PCIE_PHY_REG21		(0x54U)
#define PCIE_PHY_REG22		(0x58U)
#define PCIE_PHY_REG23		(0x5CU)
#define PCIE_PHY_REG24		(0x60U)
#define PCIE_PHY_REG37		(0x94U)
#define PCIE_PHY_REG38		(0x98U)
#define PCIE_PHY_REG43		(0x0ACU)

#define PCIE_GENERATION_MAX		(0x3U)

#define EXT_TX_EQ_MAIN_MASK		(0x3FU)
#define EXT_TX_EQ_MAIN_MAX		(40U)
#define EXT_TX_EQ_MAIN_SHIFT(g) \
	(6 * ((g) - 1))

#define EXT_TX_EQ_PRE_MASK		(0x3FU)
#define EXT_TX_EQ_PRE_MAX		(0x2B)
#define EXT_TX_EQ_PRE_SHIFT(g) \
	(4 + (5 * ((g) - 1)))

#define EXT_TX_EQ_POST_MASK		(0x3FU)
#define EXT_TX_EQ_POST_MAX		EXT_TX_EQ_POST_MASK
#define EXT_TX_EQ_POST_SHIFT(g) \
	(6 * ((g) - 1))

#define EXT_TX_VBOOST_LVL_MASK		(0x7U)
#define EXT_TX_VBOOST_LVL_MAX		EXT_TX_VBOOST_LVL_MASK
#define EXT_TX_VBOOST_LVL_SHIFT		(16U)

#define EXT_TX_IBOOST_LVL_MASK		(0xFU)
#define EXT_TX_IBOOST_LVL_MAX		EXT_TX_IBOOST_LVL_MASK
#define EXT_TX_IBOOST_LVL_SHIFT		(0U)

#define EXT_RX_EQ_CTLE_BOOST_MASK		(0x1FU)
#define EXT_RX_EQ_CTLE_BOOST_MAX		EXT_RX_EQ_CTLE_BOOST_MASK
#define EXT_RX_EQ_CTLE_BOOST_SHIFT(g) \
	(12 + (5 * ((g) - 1)))

#define EXT_RX_EQ_ATT_LVL_MASK		(0x7U)
#define EXT_RX_EQ_ATT_LVL_MAX		EXT_RX_EQ_ATT_LVL_MASK
#define EXT_RX_EQ_ATT_LVL_SHIFT(g) \
	(3 * ((g) - 1))

#define EXT_RX_LOS_THRESHOLD_MASK		(0x7U)
#define EXT_RX_LOS_THRESHOLD_MAX		EXT_RX_LOS_THRESHOLD_MASK
#define EXT_RX_LOS_THRESHOLD_SHIFT		(4U)

struct dwc_pcie_phy {
	struct device   *dev;
	struct regmap   *reg_map;
};

static void check_params(struct phy_configure_opts_pcie *cfg)
{
	if (cfg != NULL) {
		if (cfg->gen > PCIE_GENERATION_MAX) {
			cfg->gen = PCIE_GENERATION_MAX;
		}

		if (cfg->txX_eq_main > EXT_TX_EQ_MAIN_MAX) {
			cfg->txX_eq_main = EXT_TX_EQ_MAIN_MAX;
		}

		if (cfg->txX_eq_pre > EXT_TX_EQ_PRE_MAX) {
			cfg->txX_eq_pre = EXT_TX_EQ_PRE_MAX;
		}

		if (cfg->txX_eq_post > EXT_TX_EQ_POST_MAX) {
			cfg->txX_eq_post = EXT_TX_EQ_POST_MAX;
		}

		cfg->tx_vboost_lvl &= EXT_TX_VBOOST_LVL_MAX;
		cfg->tx_iboost_lvl &= EXT_TX_IBOOST_LVL_MAX;
		cfg->rx_eq_ctle_boost &= EXT_RX_EQ_CTLE_BOOST_MAX;
		cfg->rx_eq_att_lvl &= EXT_RX_EQ_ATT_LVL_MAX;
	}
}

static int32_t dwc_pcie_phy_configure(struct phy *phy, void *params)
{
	int32_t err = 0;

	if ((phy != NULL) && (params != NULL)) {
		const struct udevice *dev = phy->dev;
		const struct dwc_pcie_phy *priv = dev_get_priv(dev);
		struct phy_configure_opts_pcie *cfg =
			(struct phy_configure_opts_pcie *)params;
		uint32_t val, mask, shift;

		check_params(cfg);
		if (cfg->gen != 0) {
			shift = EXT_TX_EQ_MAIN_SHIFT(cfg->gen);
			mask = EXT_TX_EQ_MAIN_MASK << shift;
			val = cfg->txX_eq_main << shift;
			(void)regmap_update_bits(priv->reg_map, PCIE_PHY_REG22,
					mask, val);

			shift = EXT_TX_EQ_PRE_SHIFT(cfg->gen);
			mask = EXT_TX_EQ_PRE_MASK << shift;
			val = cfg->txX_eq_pre << shift;
			(void)regmap_update_bits(priv->reg_map, PCIE_PHY_REG21,
					mask, val);

			shift = EXT_TX_EQ_POST_SHIFT(cfg->gen);
			mask = EXT_TX_EQ_POST_MASK << shift;
			val = cfg->txX_eq_post << shift;
			(void)regmap_update_bits(priv->reg_map, PCIE_PHY_REG23,
					mask, val);

			shift = EXT_TX_VBOOST_LVL_SHIFT;
			mask = EXT_TX_VBOOST_LVL_MASK << shift;
			val = cfg->tx_vboost_lvl << shift;
			(void)regmap_update_bits(priv->reg_map, PCIE_PHY_REG37,
					mask, val);

			shift = EXT_TX_IBOOST_LVL_SHIFT;
			mask = EXT_TX_IBOOST_LVL_MASK << shift;
			val = cfg->tx_iboost_lvl << shift;
			(void)regmap_update_bits(priv->reg_map, PCIE_PHY_REG38,
					mask, val);

			shift = EXT_RX_LOS_THRESHOLD_SHIFT;
			mask = EXT_RX_LOS_THRESHOLD_MASK << shift;
			val = cfg->rx_los_threshold << shift;
			(void)regmap_update_bits(priv->reg_map, PCIE_PHY_REG38,
					mask, val);

			shift = EXT_RX_EQ_CTLE_BOOST_SHIFT(cfg->gen);
			mask = EXT_RX_EQ_CTLE_BOOST_MASK << shift;
			val = cfg->rx_eq_ctle_boost << shift;
			(void)regmap_update_bits(priv->reg_map, PCIE_PHY_REG24,
					mask, val);

			shift = EXT_RX_EQ_ATT_LVL_SHIFT(cfg->gen);
			mask = EXT_RX_EQ_ATT_LVL_MASK << shift;
			val = cfg->rx_eq_att_lvl << shift;
			(void)regmap_update_bits(priv->reg_map, PCIE_PHY_REG24,
					mask, val);
		} else {
			err = -EINVAL;
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t dwc_pcie_phy_power_on(struct phy *phy)
{
	int32_t err = 0;

	if (phy != NULL) {
		const struct udevice *dev = phy->dev;
		const struct dwc_pcie_phy *priv = dev_get_priv(dev);
		uint32_t val, mask;

		val = 0x0U;
		mask = 0x3U;
		(void)regmap_update_bits(priv->reg_map, PCIE_PHY_REG43,
				mask, val);
	} else {
		err = -ENODEV;
	}

	return err;
}

static int32_t dwc_pcie_phy_power_off(struct phy *phy)
{
	int32_t err = 0;

	if (phy != NULL) {
		const struct udevice *dev = phy->dev;
		const struct dwc_pcie_phy *priv = dev_get_priv(dev);
		uint32_t val, mask;

		val = 0x2U;
		mask = 0x3U;
		(void)regmap_update_bits(priv->reg_map, PCIE_PHY_REG43,
				mask, val);
	} else {
		err = -ENODEV;
	}

	return err;
}

static int32_t dwc_pcie_phy_init(struct phy *phy)
{
	int32_t err = 0;

	if (phy != NULL) {
		const struct udevice *dev = phy->dev;
		const struct dwc_pcie_phy *priv = dev_get_priv(dev);
		uint32_t val, mask;

		val = 0x0U;
		mask = (uint32_t)BIT(3);
		(void)regmap_update_bits(priv->reg_map, PCIE_PHY_REG08,
				mask, val);
	} else {
		err = -ENODEV;
	}

	return err;
}

static int32_t dwc_pcie_phy_exit(struct phy *phy)
{
	int32_t err = 0;

	if (phy != NULL) {
		const struct udevice *dev = phy->dev;
		const struct dwc_pcie_phy *priv = dev_get_priv(dev);

		/* TODO */
		(void)priv;
	} else {
		err = -ENODEV;
	}

	return err;
}

static struct phy_ops dwc_pcie_phy_ops = {
	.init = dwc_pcie_phy_init,
	.exit = dwc_pcie_phy_exit,
	.power_on = dwc_pcie_phy_power_on,
	.power_off = dwc_pcie_phy_power_off,
	.configure = dwc_pcie_phy_configure,
};

static int32_t dwc_pcie_phy_probe(struct udevice *dev)
{
	int32_t err = 0;

	if (dev != NULL) {
		struct dwc_pcie_phy *priv = dev_get_priv(dev);

		err = regmap_init_mem(dev_ofnode(dev), &priv->reg_map);
		if (err != 0) {
			pr_err("[%s] Failed to regmap_init_mem()\n", __func__);
		}
	} else {
		err = -ENODEV;
	}

	return err;
}

static const struct udevice_id dwc_pcie_phy_ids[] = {
	{ .compatible = "telechips,dwc_pcie_phy" },
	{ },
};

U_BOOT_DRIVER(dwc_pcie_phy) = {
	.name      = "dwc_pcie_phy",
	.id        = UCLASS_PHY,
	.of_match  = dwc_pcie_phy_ids,
	.ops       = &dwc_pcie_phy_ops,
	.probe     = dwc_pcie_phy_probe,
	.priv_auto = sizeof(struct dwc_pcie_phy),
};
