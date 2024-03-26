// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <dm.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <generic-phy.h>
#include <linux/soc/telechips/tcc-pcie-phy.h>

struct sf_pcie_phy {
	struct device	*dev;
	void __iomem	*phy_base;
	void __iomem	*pma_base;
};

/*
 * PCIe controller wrapper phy configuration registers
 */
#define PCIE_PHY_REG00          (0x00U)
#define PCIE_PHY_REG04          (0x10U)

/*
 * PCIe controller wrapper pma configuration registers
 */
#define PCIE_PMA_CMN_REG00B		(0x002CU)
#define PCIE_PMA_CMN_REG016		(0x0058U)
#define PCIE_PMA_CMN_REG055		(0x0154U)
#define PCIE_PMA_CMN_REG130		(0x04C0U)
#define PCIE_PMA_CMN_REG13C		(0x04F0U)
#define PCIE_PMA_TRSV_REG403		(0x100CU)
#define PCIE_PMA_TRSV_REG404		(0x1010U)
#define PCIE_PMA_TRSV_REG405		(0x1014U)
#define PCIE_PMA_TRSV_REG406		(0x1018U)
#define PCIE_PMA_TRSV_REG407		(0x101CU)
#define PCIE_PMA_TRSV_REG408		(0x1020U)
#define PCIE_PMA_TRSV_REG409		(0x1024U)
#define PCIE_PMA_TRSV_REG40A		(0x1028U)
#define PCIE_PMA_TRSV_REG40B		(0x102CU)
#define PCIE_PMA_TRSV_REG47C		(0x11F0U)
#define PCIE_PMA_TRSV_REG486		(0x1218U)
#define PCIE_PMA_TRSV_REG56D		(0x15B4U)
#define PCIE_PMA_TRSV_REG486		(0x1218U)

/*
 * Mask/shift bits in PCIe related registers
 */
#define PCIE_CLK_CFG_RESERVED_CON_SHIFT		(20U)

#define PCIE_CLK_CFG_BGR_EN_SHIFT		(30U)
#define PCIE_CLK_CFG_BGR_EN_MASK		((u32)0x1U << PCIE_CLK_CFG_BGR_EN_SHIFT)

#define PCIE_CLK_CFG_PLL_LOCK_SHIFT		(7U)
#define PCIE_CLK_CFG_PLL_LOCK_MASK		((u32)0x1U << PCIE_CLK_CFG_PLL_LOCK_SHIFT)

#define PCIE_PHY_REG_PCS_RSTN_SHIFT		(3U)
#define PCIE_PHY_REG_PMA_PORT0_RSTN_SHIFT		(2U)
#define PCIE_PHY_REG_PMA_CMN_RSTN_SHIFT		(1U)
#define PCIE_PHY_REG_PMA_INIT_RSTN_SHIFT		(0U)
#define PCIE_PHY_REG_PCS_RSTN_MASK		((u32)0x1U << PCIE_PHY_REG_PCS_RSTN_SHIFT)
#define PCIE_PHY_REG_PMA_PORT0_RSTN_MASK		((u32)0x1U << PCIE_PHY_REG_PMA_PORT0_RSTN_SHIFT)
#define PCIE_PHY_REG_PMA_CMN_RSTN_MASK		((u32)0x1U << PCIE_PHY_REG_PMA_CMN_RSTN_SHIFT)
#define PCIE_PHY_REG_PMA_INIT_RSTN_MASK		((u32)0x1U << PCIE_PHY_REG_PMA_INIT_RSTN_SHIFT)

#define PCIE_PHY_REG_LPLL_REF_CLK_SEL_SHIFT		(3U)
#define PCIE_PHY_REG_PORT0_REF_CLK_EN_SHIFT		(0U)
#define PCIE_PHY_REG_LPLL_REF_CLK_SEL_MASK		((u32)0x3U << PCIE_PHY_REG_LPLL_REF_CLK_SEL_SHIFT)
#define PCIE_PHY_REG_PORT0_REF_CLK_EN_MASK		((u32)0x1U << PCIE_PHY_REG_PORT0_REF_CLK_EN_SHIFT)

#define PCIE_PHY_REG_PMA_POWER_OFF_SHIFT		(1U)
#define PCIE_PHY_REG_PMA_POWER_OFF_MASK		((u32)0x1U << PCIE_PHY_REG_PMA_POWER_OFF_SHIFT)

#define PCIE_PMA_ANA_LCPLL_SDM_DENOMINATOR_IC1234_G4_SHIFT		(0U)
#define PCIE_PMA_ANA_LCPLL_SDM_DENOMINATOR_IC1234_G4_MASK		((u32)0xFFU << PCIE_PMA_ANA_LCPLL_SDM_DENOMINATOR_IC1234_G4_SHIFT)

#define PCIE_PMA_IGNORE_ADAP_DONE_SHIFT		(0U)
#define PCIE_PMA_IGNORE_ADAP_DONE_MASK		((u32)0x1U << PCIE_PMA_IGNORE_ADAP_DONE_SHIFT)

#define PCIE_PMA_ANA_LCPLL_AFC_VCO_CNT_RUN_NUM_SHIFT		(3U)
#define PCIE_PMA_ANA_LCPLL_AFC_VCO_CNT_RUN_NUM_MASK		((u32)0x1FU << PCIE_PMA_ANA_LCPLL_AFC_VCO_CNT_RUN_NUM_SHIFT)

#define PCIE_PMA_ANA_LCPLL_AVC_MAN_CAP_BIAS_CODE_SHIFT		(3U)
#define PCIE_PMA_ANA_LCPLL_AVC_MAN_CAP_BIAS_CODE_MASK		((u32)0x7U << PCIE_PMA_ANA_LCPLL_AVC_MAN_CAP_BIAS_CODE_SHIFT)

#define PCIE_PMA_ANA_TX_DRV_LVL_CTRL_SHIFT		(5U)
#define PCIE_PMA_ANA_TX_DRV_LVL_CTRL_MASK		((u32)0x1U << PCIE_PMA_ANA_TX_DRV_LVL_CTRL_SHIFT)

#define PCIE_PMA_ANA_TX_DRV_LVL_CTRL_G_SHIFT		(0U)
#define PCIE_PMA_ANA_TX_DRV_LVL_CTRL_G_MASK		((0x1FU << PCIE_PMA_ANA_TX_DRV_LVL_CTRL_G_SHIFT))

#define PCIE_PMA_ANA_TX_DRV_LVL_CTRL_G4_SHIFT		(1U)
#define PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_SHIFT		(0U)
#define PCIE_PMA_ANA_TX_DRV_LVL_CTRL_G4_MASK		((u32)0x1FU << PCIE_PMA_ANA_TX_DRV_LVL_CTRL_G_SHIFT)
#define PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_MASK		((u32)0x1U << PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_SHIFT)

#define PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G1_SHIFT		(4U)
#define PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G2_SHIFT		(0U)
#define PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G1_MASK		((u32)0xFU << PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G1_SHIFT)
#define PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G2_MASK		((u32)0xFU << PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G2_SHIFT)

#define PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G3_SHIFT		(4U)
#define PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G4_SHIFT		(0U)
#define PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G3_MASK		((u32)0xFU << PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G3_SHIFT)
#define PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G4_MASK		((u32)0xFU << PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G4_SHIFT)

#define PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_SHIFT		(4U)
#define PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_MASK		((u32)0x1U << PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_SHIFT)

#define PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G1_SHIFT		(0U)
#define PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G1_MASK		((u32)0xFU << PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G1_SHIFT)

#define PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G2_SHIFT		(4U)
#define PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G3_SHIFT		(0U)
#define PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G2_MASK		((u32)0xFU << PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G2_SHIFT)
#define PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G3_MASK		((u32)0xFU << PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G3_SHIFT)

#define PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G4_SHIFT		(2U)
#define PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G4_MASK		((u32)0xFU << PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G4_SHIFT)

#define PCIE_PMA_LN0_ANA_RX_CDR_CCO_VCI_AMP_I_CTRL_SHIFT		(2U)
#define PCIE_PMA_LN0_ANA_RX_CDR_CCO_VCI_AMP_I_CTRL_MASK		((u32)0x3U << PCIE_PMA_LN0_ANA_RX_CDR_CCO_VCI_AMP_I_CTRL_SHIFT)

#define PCIE_PMA_RX_OC_NUM_OF_SAMPLE_SHIFT		(2U)
#define PCIE_PMA_RX_OC_EN_SHIFT		(0U)
#define PCIE_PMA_RX_OC_NUM_OF_SAMPLE_MASK		((u32)0x7U << PCIE_PMA_RX_OC_NUM_OF_SAMPLE_SHIFT)
#define PCIE_PMA_RX_OC_EN_MASK		((u32)0x1U << PCIE_PMA_RX_OC_EN_SHIFT)

#define PCIE_PMA_LN0_OV_I_PMAD_RX_CTLE_RS_MF_CTRL_G4_SHIFT		(0U)
#define PCIE_PMA_LN0_OV_I_PMAD_RX_CTLE_RS_MF_CTRL_G4_MASK		((u32)0x1FU << PCIE_PMA_LN0_OV_I_PMAD_RX_CTLE_RS_MF_CTRL_G4_SHIFT)

#define PCIE_PMA_PLL_LOCK_SHIFT		(2U)
#define PCIE_PMA_PLL_LOCK_MASK		((u32)0x1U << PCIE_PMA_PLL_LOCK_SHIFT)

#define PCIE_EQ_TX_DRV_MAIN_MASK		(0x1FU)
#define PCIE_EQ_TX_DRV_MAIN_MAX		(0x0BU)

#define PCIE_EQ_TX_DRV_PRE_MASK		(0xFU)
#define PCIE_EQ_TX_DRV_PRE_MAX		(0xAU)

#define PCIE_EQ_TX_DRV_POST_MASK		(0xFU)
#define PCIE_EQ_TX_DRV_POST_MAX		(0xEU)

enum pcie_link_speed {
	PCIE_GEN1 = 1,
	PCIE_GEN2,
	PCIE_GEN3,
	PCIE_GEN4
};

struct ber_preset {
	int32_t phase;
	uint32_t offset;
	uint32_t value;
};

static struct ber_preset ber_preset_table[] = {
	/* Preset 0 */
	{ 0, 0x5F8U, 0x00U, },
	{ 0, 0x600U, 0x23U, },
	{ 0, 0x600U, 0x44U, },
	{ 0, 0x608U, 0x61U, },
	{ 0, 0x60CU, 0x55U, },
	{ 0, 0x610U, 0x14U, },
	{ 0, 0x614U, 0x23U, },
	{ 0, 0x618U, 0x1AU, },
	{ 0, 0x61CU, 0x04U, },
	{ 0, 0x5F8U, 0x04U, },
	{ 0, 0x5F8U, 0x00U, },
	/* Preset 1 */
	{ 1, 0x5F8U, 0x08U, },
	{ 1, 0x604U, 0x42U, },
	{ 1, 0x5F8U, 0x0CU, },
	{ 1, 0x5F8U, 0x08U, },
	/* Preset 2 */
	{ 2, 0x5F8U, 0x10U, },
	{ 2, 0x604U, 0x40U, },
	{ 2, 0x5F8U, 0x14U, },
	{ 2, 0x5F8U, 0x10U, },
	/* Preset 3 */
	{ 3, 0x5F8U, 0x18U, },
	{ 3, 0x604U, 0x45U, },
	{ 3, 0x5F8U, 0x1CU, },
	{ 3, 0x5F8U, 0x18U, },
	/* Preset 4 */
	{ 4, 0x5F8U, 0x20U, },
	{ 4, 0x604U, 0x46U, },
	{ 4, 0x5F8U, 0x24U, },
	{ 4, 0x5F8U, 0x20U, },
	/* Preset 5 */
	{ 5, 0x5F8U, 0x28U, },
	{ 5, 0x604U, 0x48U, },
	{ 5, 0x5F8U, 0x2CU, },
	{ 5, 0x5F8U, 0x28U, },
	/* Preset 6 */
	{ 6, 0x5F8U, 0x30U, },
	{ 6, 0x604U, 0x4AU, },
	{ 6, 0x5F8U, 0x34U, },
	{ 6, 0x5F8U, 0x30U, },
	/* Preset 7 */
	{ 7, 0x5F8U, 0x38U, },
	{ 7, 0x604U, 0x4CU, },
	{ 7, 0x5F8U, 0x3CU, },
	{ 7, 0x5F8U, 0x38U, },
	/* Preset 8 */
	{ 8, 0x5F8U, 0x40U, },
	{ 8, 0x600U, 0x20U, },
	{ 8, 0x604U, 0x20U, },
	{ 8, 0x608U, 0x01U, },
	{ 8, 0x5F8U, 0x44U, },
	{ 8, 0x5F8U, 0x40U, },
	/* Preset 9 */
	{ 9, 0x5F8U, 0x48U, },
	{ 9, 0x600U, 0x20U, },
	{ 9, 0x604U, 0x21U, },
	{ 9, 0x608U, 0x01U, },
	{ 9, 0x5F8U, 0x4CU, },
	{ 9, 0x5F8U, 0x48U, },
	/* Preset A */
	{ 10, 0x5F8U, 0x50U, },
	{ 10, 0x600U, 0x26U, },
	{ 10, 0x604U, 0x80U, },
	{ 10, 0x608U, 0x41U, },
	{ 10, 0x60CU, 0xAFU, },
	{ 10, 0x610U, 0x26U, },
	{ 10, 0x614U, 0x34U, },
	{ 10, 0x618U, 0x24U, },
	{ 10, 0x61CU, 0x06U, },
	{ 10, 0x5F8U, 0x54U, },
	{ 10, 0x5F8U, 0x50U, },
	/* Preset B */
	{ 11, 0x5F8U, 0x58U, },
	{ 11, 0x604U, 0x81U, },
	{ 11, 0x5F8U, 0x5CU, },
	{ 11, 0x5F8U, 0x58U, },
	/* Preset C */
	{ 12, 0x5F8U, 0x60U, },
	{ 12, 0x604U, 0x82U, },
	{ 12, 0x5F8U, 0x64U, },
	{ 12, 0x5F8U, 0x60U, },
	/* Preset D */
	{ 13, 0x5F8U, 0x68U, },
	{ 13, 0x604U, 0x83U, },
	{ 13, 0x5F8U, 0x6CU, },
	{ 13, 0x5F8U, 0x68U, },
	/* Preset E */
	{ 14, 0x5F8U, 0x70U, },
	{ 14, 0x604U, 0x84U, },
	{ 14, 0x5F8U, 0x74U, },
	{ 14, 0x5F8U, 0x70U, },
	/* Preset F */
	{ 15, 0x5F8U, 0x78U, },
	{ 15, 0x600U, 0x26U, },
	{ 15, 0x604U, 0x85U, },
	{ 15, 0x608U, 0x80U, },
	{ 15, 0x60CU, 0x7FU, },
	{ 15, 0x610U, 0x2DU, },
	{ 15, 0x614U, 0x34U, },
	{ 15, 0x618U, 0x24U, },
	{ 15, 0x61CU, 0x05U, },
	{ 15, 0x5F8U, 0x7CU, },
	{ 15, 0x5F8U, 0x78U, },
	/* Preset 10 */
	{ 16, 0x5F8U, 0x80U, },
	{ 16, 0x604U, 0x86U, },
	{ 16, 0x5F8U, 0x84U, },
	{ 16, 0x5F8U, 0x80U, },
	/* Preset 11 */
	{ 17, 0x5F8U, 0x88U, },
	{ 17, 0x604U, 0x87U, },
	{ 17, 0x5F8U, 0x8CU, },
	{ 17, 0x5F8U, 0x88U, },
	/* Preset 12 */
	{ 18, 0x5F8U, 0x90U, },
	{ 18, 0x604U, 0x88U, },
	{ 18, 0x5F8U, 0x94U, },
	{ 18, 0x5F8U, 0x90U, },
	/* Preset 13 */
	{ 19, 0x5F8U, 0x98U, },
	{ 19, 0x604U, 0x89U, },
	{ 19, 0x5F8U, 0x9CU, },
	{ 19, 0x5F8U, 0x98U, },
};

static inline uint32_t sf_pcie_phy_readl(void __iomem *addr, uint32_t offset)
{
	return readl(addr + offset);
}

static inline void sf_pcie_phy_writel(void __iomem *addr, uint32_t offset,
		uint32_t data, uint32_t mask)
{
	writel(((readl(addr + offset) & ~mask)|data), addr + offset);
}

static void check_params(struct phy_configure_opts_pcie *config)
{
	if (config != NULL) {
		if (config->gen > (u8)PCIE_GEN4) {
			config->gen = (u8)PCIE_GEN4;
		}

		if (config->txX_eq_main > PCIE_EQ_TX_DRV_MAIN_MAX) {
			config->txX_eq_main = PCIE_EQ_TX_DRV_MAIN_MAX;
		}

		if (config->txX_eq_pre > PCIE_EQ_TX_DRV_PRE_MAX) {
			config->txX_eq_pre = PCIE_EQ_TX_DRV_PRE_MAX;
		}

		if (config->txX_eq_post > PCIE_EQ_TX_DRV_POST_MAX) {
			config->txX_eq_post = PCIE_EQ_TX_DRV_POST_MAX;
		}
	}
}

static int32_t sf_pcie_phy_set_tx_main(const struct sf_pcie_phy *priv,
		const struct phy_configure_opts_pcie *config)
{
	int32_t err = 0;

	if ((priv != NULL) && (config != NULL)) {
		if (config->txX_eq_main != 0U) {
			uint32_t val, mask, shift, offset;

			offset = PCIE_PMA_TRSV_REG403 + ((u32)config->gen * 0x4U);
			mask = (config->gen == (u8)PCIE_GEN4) ?
				PCIE_PMA_ANA_TX_DRV_LVL_CTRL_G4_MASK :
				PCIE_PMA_ANA_TX_DRV_LVL_CTRL_G_MASK;
			shift = (config->gen == (u8)PCIE_GEN4) ?
				PCIE_PMA_ANA_TX_DRV_LVL_CTRL_G4_SHIFT :
				PCIE_PMA_ANA_TX_DRV_LVL_CTRL_G_SHIFT;
			val = (sf_pcie_phy_readl(priv->pma_base, offset) & mask) >> shift;
			if (config->txX_eq_main != val) {
				val = ((u32)config->txX_eq_main << shift);
				sf_pcie_phy_writel(priv->pma_base, offset, val, mask);
			}

			offset = PCIE_PMA_TRSV_REG403;
			mask = PCIE_PMA_ANA_TX_DRV_LVL_CTRL_SHIFT;
			val = mask;
			sf_pcie_phy_writel(priv->pma_base, offset, val, mask);
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t sf_pcie_phy_set_tx_pre(const struct sf_pcie_phy *priv,
		const struct phy_configure_opts_pcie *config)
{
	int32_t err = 0;

	if ((priv != NULL) && (config != NULL)) {
		if (config->txX_eq_pre != 0x0U) {
			uint32_t val, mask, shift, offset;

			switch(config->gen) {
				case PCIE_GEN1:
					offset = PCIE_PMA_TRSV_REG409;
					mask = PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G1_MASK;
					shift = PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G1_SHIFT;
					break;
				case PCIE_GEN2:
					offset = PCIE_PMA_TRSV_REG40A;
					mask = PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G2_MASK;
					shift = PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G2_SHIFT;
					break;
				case PCIE_GEN3:
					offset = PCIE_PMA_TRSV_REG40A;
					mask = PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G3_MASK;
					shift = PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G3_SHIFT;
					break;
				case PCIE_GEN4:
					offset = PCIE_PMA_TRSV_REG40B;
					mask = PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G4_MASK;
					shift = PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_G4_SHIFT;
					break;
				default:
					err = -EINVAL;
					break;
			}

			if (err == 0) {
				val = (sf_pcie_phy_readl(priv->pma_base, offset) & mask) >> shift;
				if (config->txX_eq_post != val) {
					val = ((u32)config->txX_eq_post << shift);
					sf_pcie_phy_writel(priv->pma_base, offset, val, mask);
				}

				offset = PCIE_PMA_TRSV_REG409;
				mask = PCIE_PMA_ANA_TX_DRV_PRE_LVL_CTRL_MASK;
				val = mask;
				sf_pcie_phy_writel(priv->pma_base, offset, val, mask);
			}
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t sf_pcie_phy_set_tx_post(const struct sf_pcie_phy *priv,
		const struct phy_configure_opts_pcie *config)
{
	int32_t err = 0;

	if ((priv != NULL) && (config != NULL)) {
		if (config->txX_eq_post != 0x0U) {
			uint32_t val, mask, shift, offset;
			switch(config->gen) {
			case PCIE_GEN1:
				offset = PCIE_PMA_TRSV_REG407;
				mask = PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G1_MASK;
				shift = PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G1_SHIFT;
				break;
			case PCIE_GEN2:
				offset = PCIE_PMA_TRSV_REG407;
				mask = PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G2_MASK;
				shift = PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G2_SHIFT;
				break;
			case PCIE_GEN3:
				offset = PCIE_PMA_TRSV_REG408;
				mask = PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G3_MASK;
				shift = PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G3_SHIFT;
				break;
			case PCIE_GEN4:
				offset = PCIE_PMA_TRSV_REG408;
				mask = PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G4_MASK;
				shift = PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_G4_SHIFT;
				break;
			default:
				err = -EINVAL;
				break;
			}

			if (err == 0) {
				val = (sf_pcie_phy_readl(priv->pma_base, offset) & mask) >> shift;
				if (config->txX_eq_post != val) {
					val = ((u32)config->txX_eq_post << shift);
					sf_pcie_phy_writel(priv->pma_base, offset, val, mask);
				}

				offset = PCIE_PMA_TRSV_REG406;
				mask = PCIE_PMA_ANA_TX_DRV_POST_LVL_CTRL_MASK;
				val = mask;
				sf_pcie_phy_writel(priv->pma_base, offset, val, mask);
			}
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t sf_pcie_phy_configure(struct phy *phy, void *params)
{
	int32_t err = 0;

	if ((phy != NULL) && (params != NULL)) {
		const struct udevice *dev = phy->dev;
		struct sf_pcie_phy *priv = dev_get_priv(dev);
		struct phy_configure_opts_pcie *config =
			(struct phy_configure_opts_pcie *)params;

		check_params(config);
		if (config->gen >= (u8)PCIE_GEN1) {
			err = sf_pcie_phy_set_tx_main((const struct sf_pcie_phy *)priv,
					(const struct phy_configure_opts_pcie *)config);
			if (err == 0) {
				err = sf_pcie_phy_set_tx_pre((const struct sf_pcie_phy *)priv,
						(const struct phy_configure_opts_pcie *)config);
			}

			if (err == 0) {
				err = sf_pcie_phy_set_tx_post((const struct sf_pcie_phy *)priv,
						(const struct phy_configure_opts_pcie *)config);
			}
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t sf_pcie_phy_power_on(struct phy *phy)
{
	int32_t err = 0;

	if (phy != NULL) {
		const struct udevice *dev = phy->dev;
		struct sf_pcie_phy *priv = dev_get_priv(dev);
		uint32_t val, mask;

		mask = PCIE_PHY_REG_PMA_POWER_OFF_MASK;
		val = 0x0U;
		sf_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG04, val, mask);
	} else {
		err = -ENODEV;
	}

	return err;
}

static int32_t sf_pcie_phy_power_off(struct phy *phy)
{
	int32_t err = 0;

	if (phy != NULL) {
		const struct udevice *dev = phy->dev;
		struct sf_pcie_phy *priv = dev_get_priv(dev);
		uint32_t val, mask;

		mask = PCIE_PHY_REG_PMA_POWER_OFF_MASK;
		val = mask;
		sf_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG04, val, mask);
	} else {
		err = -ENODEV;
	}

	return err;
}

static int32_t sf_pcie_phy_reset(struct phy *phy)
{
	int32_t err = 0;

	if (phy != NULL) {
		const struct udevice *dev = phy->dev;
		struct sf_pcie_phy *priv = dev_get_priv(dev);
		uint32_t val, mask;

		/* assert tx and rx resets */
		val = 0x0U;
		mask = PCIE_PHY_REG_PCS_RSTN_MASK;
		sf_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG00, val, mask);

		udelay(1);

		/* deassert tx, rx and global reset */
		mask = PCIE_PHY_REG_PCS_RSTN_MASK | PCIE_PHY_REG_PMA_INIT_RSTN_MASK;
		val = mask;
		sf_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG00, val, mask);

		udelay(1);

		/* deassert port and common blk resets */
		mask = PCIE_PHY_REG_PMA_CMN_RSTN_MASK | PCIE_PHY_REG_PMA_PORT0_RSTN_MASK;
		val = mask;
		sf_pcie_phy_writel(priv->phy_base, PCIE_PHY_REG00, val, mask);

		udelay(500); //need to find proper delay
	} else {
		err = -ENODEV;
	}

	return err;
}

static int32_t sf_pcie_phy_init(struct phy *phy)
{
	int32_t err = 0;

	if (phy != NULL) {
		const struct udevice *dev = phy->dev;
		struct sf_pcie_phy *priv = dev_get_priv(dev);
		uint32_t val, mask, idx, presets;

		val = (0x3U << PCIE_PMA_RX_OC_NUM_OF_SAMPLE_SHIFT);
		mask = PCIE_PMA_RX_OC_NUM_OF_SAMPLE_MASK | PCIE_PMA_RX_OC_EN_MASK;
		sf_pcie_phy_writel(priv->pma_base, PCIE_PMA_TRSV_REG486, val, mask);

		val = (0x11U << PCIE_PMA_ANA_LCPLL_AFC_VCO_CNT_RUN_NUM_SHIFT);
		mask = PCIE_PMA_ANA_LCPLL_AFC_VCO_CNT_RUN_NUM_MASK;
		sf_pcie_phy_writel(priv->pma_base, PCIE_PMA_CMN_REG00B, val, mask);

		val = (0x4U << PCIE_PMA_ANA_LCPLL_AVC_MAN_CAP_BIAS_CODE_SHIFT);
		mask = PCIE_PMA_ANA_LCPLL_AVC_MAN_CAP_BIAS_CODE_MASK;
		sf_pcie_phy_writel(priv->pma_base, PCIE_PMA_CMN_REG016, val, mask);

		val = 0x0U;
		mask = PCIE_PMA_LN0_ANA_RX_CDR_CCO_VCI_AMP_I_CTRL_MASK;
		for (idx = 0U; idx < 4U; idx++) {
			sf_pcie_phy_writel(priv->pma_base,
					(PCIE_PMA_TRSV_REG47C + (idx * 0x1000U)),
					val, mask);
		}

		val = (0x6U << PCIE_PMA_LN0_OV_I_PMAD_RX_CTLE_RS_MF_CTRL_G4_SHIFT);
		mask = PCIE_PMA_LN0_OV_I_PMAD_RX_CTLE_RS_MF_CTRL_G4_MASK;
		for (idx = 0U; idx < 4U; idx++) {
			sf_pcie_phy_writel(priv->pma_base,
					(PCIE_PMA_TRSV_REG56D + (idx * 0x1000U)),
					val, mask);
		}

		mask = 0xFFFFFFFFU;
		presets = (sizeof(ber_preset_table)/sizeof(struct ber_preset));
		for (idx = 0U; idx < presets; idx++) {
			if (ber_preset_table[idx].offset != mask) {
				sf_pcie_phy_writel(priv->pma_base,
						ber_preset_table[idx].offset,
						ber_preset_table[idx].value,
						mask);
			}
		}

		val = (0x1U << PCIE_PMA_IGNORE_ADAP_DONE_SHIFT);
		mask = PCIE_PMA_IGNORE_ADAP_DONE_MASK;
		sf_pcie_phy_writel(priv->pma_base, PCIE_PMA_CMN_REG130, val, mask);
	} else {
		err = -ENODEV;
	}

	return err;
}

static int32_t sf_pcie_phy_exit(struct phy *phy)
{
	int32_t err = 0;

	if (phy != NULL) {
		const struct udevice *dev = phy->dev;
		struct sf_pcie_phy *priv = dev_get_priv(dev);

		/* TODO */
		(void)priv;
	} else {
		err = -ENODEV;
	}

	return err;
}

static struct phy_ops sf_pcie_phy_ops = {
	.init = sf_pcie_phy_init,
	.exit = sf_pcie_phy_exit,
	.reset = sf_pcie_phy_reset,
	.power_on = sf_pcie_phy_power_on,
	.power_off = sf_pcie_phy_power_off,
	.configure = sf_pcie_phy_configure,
};

static int32_t sf_pcie_phy_probe(struct udevice *dev)
{
	int32_t err = 0;

	if (dev != NULL) {
		struct sf_pcie_phy *priv = dev_get_priv(dev);

		priv->phy_base = dev_remap_addr_index(dev, 0);
		if (priv->phy_base == NULL) {
			err = -EINVAL;
		}

		if (err == 0) {
			priv->pma_base = dev_remap_addr_index(dev, 1);
			if (priv->pma_base == NULL) {
				err = -EINVAL;
			}
		}
	} else {
		err = -ENODEV;
	}

	return err;
}

static const struct udevice_id sf_pcie_phy_ids[] = {
	{ .compatible = "telechips,sf_pcie_phy" },
	{ },
};

U_BOOT_DRIVER(sf_pcie_phy) = {
	.name      = "sf_pcie_phy",
	.id        = UCLASS_PHY,
	.of_match  = sf_pcie_phy_ids,
	.ops       = &sf_pcie_phy_ops,
	.probe     = sf_pcie_phy_probe,
	.priv_auto = sizeof(struct sf_pcie_phy),
};
