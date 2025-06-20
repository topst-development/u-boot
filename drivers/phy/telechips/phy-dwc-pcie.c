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

#define PCIE_CLK_CFG00		(0x00U)
#define PCIE_CLK_CFG04		(0x10U)

#define PCIE_GENERATION_MAX		(0x3U)

#define EXT_TX_EQ_MAIN_MASK		(0x3FU)
#define EXT_TX_EQ_MAIN_MAX		(40U)
#define EXT_TX_EQ_MAIN_SHIFT(g) \
	(6 * ((g) - 1))

#define EXT_TX_EQ_PRE_MASK		(0x3FU)
#define EXT_TX_EQ_PRE_MAX		(10U)
#define EXT_TX_EQ_PRE_SHIFT(g) \
	(4 + (5 * ((g) - 1)))

#define EXT_TX_EQ_POST_MASK		(0x3FU)
#define EXT_TX_EQ_POST_MAX		(15U)
#define EXT_TX_EQ_POST_SHIFT(g) \
	(6 * ((g) - 1))

#define EXT_TX_EQ_OVRD_MASK		(0x1U)
#define EXT_TX_EQ_OVRD_SHIFT(g) \
	((g) - 1)

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

#define ERIO_TX_LVL_CTRL_SHIFT		(11U)
#define ERIO_TX_LVL_CTRL_MASK		((u32)0xFU << ERIO_TX_LVL_CTRL_SHIFT)
#define ERIO_TX_LVL_CTRL_MAX		(0xFU)

struct dwc_pcie_phy {
	struct device   *dev;
	void __iomem *phy_base;
	void __iomem *clk_base;
};

static inline uint32_t dwc_pcie_phy_readl(void __iomem *addr, uint32_t offset)
{
	return readl(addr + offset);
}

static inline void dwc_pcie_phy_writel(void __iomem *addr, uint32_t offset,
		uint32_t data, uint32_t mask)
{
	writel(((readl(addr + offset) & ~mask)|data), addr + offset);
}

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

		if (cfg->tx_clk_swing_lvl > ERIO_TX_LVL_CTRL_MAX) {
			cfg->tx_clk_swing_lvl = ERIO_TX_LVL_CTRL_MAX;
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
			dwc_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG22, val, mask);

			shift = EXT_TX_EQ_PRE_SHIFT(cfg->gen);
			mask = EXT_TX_EQ_PRE_MASK << shift;
			val = cfg->txX_eq_pre << shift;
			dwc_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG21, val, mask);

			shift = EXT_TX_EQ_POST_SHIFT(cfg->gen);
			mask = EXT_TX_EQ_POST_MASK << shift;
			val = cfg->txX_eq_post << shift;
			dwc_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG23, val, mask);

			shift = EXT_TX_VBOOST_LVL_SHIFT;
			mask = EXT_TX_VBOOST_LVL_MASK << shift;
			val = cfg->tx_vboost_lvl << shift;
			dwc_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG37, val, mask);

			shift = EXT_TX_IBOOST_LVL_SHIFT;
			mask = EXT_TX_IBOOST_LVL_MASK << shift;
			val = cfg->tx_iboost_lvl << shift;
			dwc_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG38, val, mask);

			shift = EXT_TX_EQ_OVRD_SHIFT(cfg->gen);
			mask = EXT_TX_EQ_OVRD_MASK << shift;
			dwc_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG21, val, mask);

			shift = EXT_RX_LOS_THRESHOLD_SHIFT;
			mask = EXT_RX_LOS_THRESHOLD_MASK << shift;
			val = cfg->rx_los_threshold << shift;
			dwc_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG38, val, mask);

			shift = EXT_RX_EQ_CTLE_BOOST_SHIFT(cfg->gen);
			mask = EXT_RX_EQ_CTLE_BOOST_MASK << shift;
			val = cfg->rx_eq_ctle_boost << shift;
			dwc_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG24, val, mask);

			shift = EXT_RX_EQ_ATT_LVL_SHIFT(cfg->gen);
			mask = EXT_RX_EQ_ATT_LVL_MASK << shift;
			val = cfg->rx_eq_att_lvl << shift;
			dwc_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG24, val, mask);

			mask = ERIO_TX_LVL_CTRL_MASK;
			val = ((u32)cfg->tx_clk_swing_lvl << ERIO_TX_LVL_CTRL_SHIFT);
			dwc_pcie_phy_writel(priv->clk_base, PCIE_CLK_CFG04, val, mask);
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

		mask = 0x2U;
		val = 0x0U;
		dwc_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG43, val, mask);
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

		mask = 0x2U;
		val = mask;
		dwc_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG43, val, mask);
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

		mask = (uint32_t)BIT(3);
		val = 0x0U;
		dwc_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG08, val, mask);

		mask = (uint32_t)BIT(31);
		val = 0x0U;
		dwc_pcie_phy_writel(priv->clk_base, PCIE_CLK_CFG00, val, mask);

		mask = (uint32_t)BIT(31);
		val = 0x0U;
		dwc_pcie_phy_writel(priv->clk_base, PCIE_CLK_CFG00, val, mask);

		mask = (uint32_t)BIT(31);
		val = 0x0U;
		dwc_pcie_phy_writel(priv->clk_base, PCIE_CLK_CFG00, val, mask);

		mask = (uint32_t)BIT(31);
		val = 0x0U;
		dwc_pcie_phy_writel(priv->clk_base, PCIE_CLK_CFG00, val, mask);

		mask = (uint32_t)BIT(31);
		val = 0x0U;
		dwc_pcie_phy_writel(priv->clk_base, PCIE_CLK_CFG00, val, mask);

		val = 0x0504C80C | (uint32_t)BIT(31);
		mask = 0xFFFFFFFFU;
		dwc_pcie_phy_writel(priv->clk_base, PCIE_CLK_CFG00, val, mask);

		mask = (uint32_t)BIT(23);
		do {
			val = dwc_pcie_phy_readl(priv->clk_base, PCIE_CLK_CFG00) & mask;
		} while(val != mask);

		mask = (uint32_t)(BIT(18) | BIT(5) | BIT(0));
		val = mask;
		dwc_pcie_phy_writel(priv->clk_base, PCIE_CLK_CFG04, val, mask);
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

		priv->phy_base = dev_remap_addr_index(dev, 0);
		if (priv->phy_base == NULL) {
			err = -EINVAL;
		}

		if (err == 0) {
			priv->clk_base = dev_remap_addr_index(dev, 1);
			if (priv->clk_base == NULL) {
				err = -EINVAL;
			}
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
