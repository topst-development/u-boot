// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/
#include <linux/delay.h>

#include "dptx_v14.h"
#include "dptx_reg.h"
#include "dptx_dbg.h"

#define MAX_TRY_PHY_PLL_LOCK 100

#define CHECK_REG_OFFSET(x) (((x) < (uint32_t)DP_MAX_OFFSET) ? (bool)true : (bool)false)

struct SNPY_CFG_Reg_Data {
	uint8_t ucLink_Rate;
	uint32_t uiReg_Add;
	uint32_t uiReg_Val;
};

static struct SNPY_CFG_Reg_Data stSNPY_CFG_Reg_Data[] = {
	{(uint8_t)LINK_RATE_RBR,  DP_REGISTER_BANK_REG_0, REG_BANK_REG_0_RBR_INIT},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_0, REG_BANK_REG_0_HBR_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_0, REG_BANK_REG_0_HBR2_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_0, REG_BANK_REG_0_HBR3_INIT},

	{(uint8_t)LINK_RATE_RBR,  DP_REGISTER_BANK_REG_20, REG_BANK_REG_20_INIT},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_21, REG_BANK_REG_21_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_21, REG_BANK_REG_21_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_21, REG_BANK_REG_21_INIT},

	{(uint8_t)LINK_RATE_RBR,  DP_REGISTER_BANK_REG_11, REG_BANK_REG_11_INIT},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_20, REG_BANK_REG_20_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_20, REG_BANK_REG_20_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_1, REG_BANK_REG_1_INIT},

	{(uint8_t)LINK_RATE_RBR,  DP_REGISTER_BANK_REG_12, REG_BANK_REG_12_RBR_INIT},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_1, REG_BANK_REG_1_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_1, REG_BANK_REG_1_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_2, REG_BANK_REG_2_INIT},

	{(uint8_t)LINK_RATE_RBR,  DP_REGISTER_BANK_REG_13, REG_BANK_REG_13_RBR_INIT},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_2, REG_BANK_REG_2_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_2, REG_BANK_REG_2_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_3, REG_BANK_REG_3_INIT},

	{(uint8_t)LINK_RATE_RBR,  DP_REGISTER_BANK_REG_14, REG_BANK_REG_14_RBR_INIT},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_3, REG_BANK_REG_3_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_3, REG_BANK_REG_3_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_4, REG_BANK_REG_4_INIT},

	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_4, REG_BANK_REG_4_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_4, REG_BANK_REG_4_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_7, REG_BANK_REG_7_INIT},

	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_7, REG_BANK_REG_7_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_7, REG_BANK_REG_7_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_8, REG_BANK_REG_8_INIT},

	{(uint8_t)LINK_RATE_RBR,  DP_REGISTER_BANK_REG_17, REG_BANK_REG_17_INIT},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_8, REG_BANK_REG_8_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_8, REG_BANK_REG_8_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_9, REG_BANK_REG_9_INIT},

	{(uint8_t)LINK_RATE_RBR,  DP_REGISTER_BANK_REG_20, REG_BANK_REG_20_INIT},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_9, REG_BANK_REG_9_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_9, REG_BANK_REG_9_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_10, REG_BANK_REG_10_HBR3_1_INIT},

	{(uint8_t)LINK_RATE_RBR,  DP_REGISTER_BANK_REG_21, REG_BANK_REG_21_INIT},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_10, REG_BANK_REG_10_HBR_1_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_10, REG_BANK_REG_10_HBR_1_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_11, REG_BANK_REG_11_INIT},

	{(uint8_t)LINK_RATE_RBR,  DP_REGISTER_BANK_REG_23, REG_BANK_REG_23_INIT},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_11, REG_BANK_REG_11_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_11, REG_BANK_REG_11_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_13, DP_REGISTER_BANK_REG_RESET},

	{(uint8_t)LINK_RATE_RBR,  DP_REGISTER_BANK_REG_22, REG_BANK_REG_22_INIT},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_13, REG_BANK_REG_13_HBR_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_13, REG_BANK_REG_13_HBR2_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_14, DP_REGISTER_BANK_REG_RESET},
	{(uint8_t)LINK_RATE_RBR, DP_REGISTER_BANK_REG_17, 0},
	{(uint8_t)LINK_RATE_RBR,  DP_REGISTER_BANK_REG_22, DP_REGISTER_BANK_REG_RESET},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_14, REG_BANK_REG_14_HBR_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_14, REG_BANK_REG_14_HBR2_INIT},

	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_17, REG_BANK_REG_17_INIT},

	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_17, REG_BANK_REG_17_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_17, REG_BANK_REG_17_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_20, REG_BANK_REG_20_INIT},


	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_20, REG_BANK_REG_20_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_20, REG_BANK_REG_20_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_10, REG_BANK_REG_10_HBR3_INIT},


	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_10, REG_BANK_REG_10_HBR_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_10, REG_BANK_REG_10_HBR2_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_12, REG_BANK_REG_12_HBR3_INIT},

	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_12, REG_BANK_REG_12_HBR_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_12, REG_BANK_REG_10_HBR2_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_23, REG_BANK_REG_23_INIT},

	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_23, REG_BANK_REG_23_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_23, REG_BANK_REG_23_INIT},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_22, REG_BANK_REG_22_INIT},
	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_22, REG_BANK_REG_22_INIT},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_22, REG_BANK_REG_22_INIT},

	{(uint8_t)LINK_RATE_HBR, DP_REGISTER_BANK_REG_17, 0},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_17, 0},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_17, 0},

	{(uint8_t)LINK_RATE_HBR,  DP_REGISTER_BANK_REG_22, DP_REGISTER_BANK_REG_RESET},
	{(uint8_t)LINK_RATE_HBR2, DP_REGISTER_BANK_REG_22, DP_REGISTER_BANK_REG_RESET},
	{(uint8_t)LINK_RATE_HBR3, DP_REGISTER_BANK_REG_22, DP_REGISTER_BANK_REG_RESET},

	{(uint8_t)LINK_RATE_MAX, DP_REGISTER_BANK_REG_MAX, DP_REGISTER_BANK_REG_RESET}
};

static int32_t dptx_cfg_reg_init(struct Dptx_Params *pstDptx, uint8_t ucLinkRate)
{
	uint8_t ucLink_Rate;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiReg_Addr, uiReg_R_data, uiReg_22_data, uiReg_W_data, uiReg_Offset = pstDptx->uiRegBank_RegAddr_Offset;
	uint32_t uiElements;
	const struct SNPY_CFG_Reg_Data *pstSNPY_CFG_Reg_Data;

	if (pstDptx->ePhy_Dev != PHY_DEVICE_SNPS) {
		dptx_debug("Nothing to do not for Synopsys PHY");
	} else if (!CHECK_REG_OFFSET(uiReg_Offset)) {
		dptx_err("Invalid reg offset as 0x%x", uiReg_Offset);

		iRetVal = DPTX_RETURN_EINVAL;
	} else {
		uiReg_R_data = Dptx_Reg_Readl(pstDptx, (uiReg_Offset + (uint32_t)DP_REGISTER_BANK_REG_22));

		uiReg_22_data = ((uiReg_R_data & (uint32_t)AXI_SLAVE_BRIDGE_RST_MASK) != 0U) ? (uint32_t)0x00000008U : (uint32_t)0x00000000U;

		pstSNPY_CFG_Reg_Data = stSNPY_CFG_Reg_Data;

		for (uiElements = 0; (pstSNPY_CFG_Reg_Data[uiElements].uiReg_Add != (uint32_t)DP_REGISTER_BANK_REG_MAX); uiElements++) {
			ucLink_Rate = pstSNPY_CFG_Reg_Data[uiElements].ucLink_Rate;
			uiReg_W_data = pstSNPY_CFG_Reg_Data[uiElements].uiReg_Val;
			uiReg_Addr = pstSNPY_CFG_Reg_Data[uiElements].uiReg_Add;

			if (ucLinkRate != ucLink_Rate) {
				continue;
			}

			if ((uiReg_Addr == (uint32_t)DP_REGISTER_BANK_REG_22) && (uiReg_W_data == (uint32_t)DP_REGISTER_BANK_REG_RESET)) {
				uiReg_W_data = uiReg_22_data;

				/* It should be wait 10us before releasing a software reset of the DP PHY. */
				udelay(10);
			}

			if ((ucLink_Rate == (uint8_t)LINK_RATE_HBR3) && (uiReg_Addr == (uint32_t)DP_REGISTER_BANK_REG_13)) {
				uiReg_W_data = (pstDptx->stDptxLink.ssc_en) ? REG_BANK_REG_13_HBR3_SSC : REG_BANK_REG_13_HBR3_INIT;
			}

			if ((ucLink_Rate == (uint8_t)LINK_RATE_HBR3) && (uiReg_Addr == (uint32_t)DP_REGISTER_BANK_REG_14)) {
				uiReg_W_data = (pstDptx->stDptxLink.ssc_en) ? REG_BANK_REG_14_HBR3_SSC : REG_BANK_REG_14_HBR3_INIT;
			}

			if (uiReg_Addr == (uint32_t)DP_REGISTER_BANK_REG_20) {
				uiReg_W_data &= ~((uint32_t)3u << 12);
				uiReg_W_data |= ((pstDptx->hw_config.aux_hysteresis & 0x3u) << 12);
			}

			//dptx_info("Reg[0x%x] = 0x%x", pstSNPY_CFG_Reg_Data[uiElements].uireg_add, pstSNPY_CFG_Reg_Data[uiElements].uireg_val);

			Dptx_Reg_Writel(pstDptx, (uiReg_Offset + uiReg_Addr), uiReg_W_data);
		}

		(void)dptx_cfg_set_manual_phy_signal_quality(pstDptx, ucLinkRate,
					 VOLTAGE_SWING_LEVEL_0, PRE_EMPHASIS_LEVEL_0);
	}

	return iRetVal;
}

static int dptx_cfg_set_phy_sram_ext_ld_done(struct Dptx_Params *pstDptx)
{
	uint32_t regdata, regoffset;
	int ret = DPTX_RETURN_NO_ERROR;

	regoffset = pstDptx->uiRegBank_RegAddr_Offset + (uint32_t)DP_REGISTER_BANK_REG_17;

	if (!CHECK_REG_OFFSET(regoffset)) {
		dptx_err("Invalid reg offset as 0x%x", regoffset);
		ret = -DPTX_RETURN_EINVAL;
	} else {
		regdata = Dptx_Reg_Readl(pstDptx, regoffset);
		regdata |= ((uint32_t)1u << 29);
		Dptx_Reg_Writel(pstDptx, regoffset, regdata);
	}

	return ret;
}

static bool dptx_cfg_get_phy_sram_init_done(struct Dptx_Params *pstDptx)
{
	uint32_t regdata, regoffset;
	bool ret = (bool)true;

	regoffset = pstDptx->uiRegBank_RegAddr_Offset + (uint32_t)DP_REGISTER_BANK_REG_17;

	if (!CHECK_REG_OFFSET(regoffset)) {
		dptx_err("Invalid reg offset as 0x%x", regoffset);
		ret = (bool)false;
	} else {
		regdata = Dptx_Reg_Readl(pstDptx, regoffset);
		ret = ((regdata & ((uint32_t)1u << 24)) != 0u) ? (bool)true : (bool)false;
	}

	return ret;
}

/*
 * This workaround solves the problem of lane 1 of PHY getting stuck in Rx VCO
 * calibration even though the D3 PHY is DPYX only.
 * Note: This workaround will be only applied to D3 (PHY_DEVICE_SNPS)
 */
static int32_t dptx_cfg_bypass_rx_vco_calibration(struct Dptx_Params *pstDptx)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	unsigned int loop;

	if (pstDptx->ePhy_Dev == PHY_DEVICE_SNPS) {
		/*
		 * 1. DP Workaround
		 * Wait up to 400us for SRAM initialization to complete.
		 * Typically it will be completed within 100us.
		 */
		for (loop = 0u; loop < 400u ; loop++) {
			if (dptx_cfg_get_phy_sram_init_done(pstDptx)) {
				break;
			}
			udelay(1);
		}

		/*
		 * 2. DP Workaround
		 * RAWLANEN_DIG_AON_FAST_FLAGS. FAST_RX_VCO_CAL
		 */
		(void)dptx_phy_write(pstDptx, 0x315c, 0x4000);
		(void)dptx_phy_write(pstDptx, 0x325c, 0x4000);

		/* 3. DP Workaround - Set sram ext ld done */
		ret = dptx_cfg_set_phy_sram_ext_ld_done(pstDptx);
	}
	return ret;
}

static int32_t dptx_cfg_set_sdm_bypass(struct Dptx_Params *pstDptx, bool bypass)
{
	uint32_t reg_offset, reg_mask, reg_val, reg_base;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	reg_base = pstDptx->uiRegBank_RegAddr_Offset;

	if (!CHECK_REG_OFFSET(reg_base)) {
		dptx_err("Invalid reg offset as 0x%x", reg_base);

		ret = DPTX_RETURN_EINVAL;
	} else {
		/* SDM */
		if (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) {
			reg_offset = (uint32_t)DP_CFG_VIDEO_MUX;
			reg_mask = (uint32_t)CFG_SDM_DIS_MASK;
		} else {
			reg_offset = (uint32_t)DP_REGISTER_BANK_REG_24;
			reg_mask = (uint32_t)SDM_DIS_MASK;
		}
		reg_val = Dptx_Reg_Readl(pstDptx, (reg_base + reg_offset));
		if (bypass) {
			reg_val |= reg_mask;
		} else {
			reg_val &= ~reg_mask;
		}
		Dptx_Reg_Writel(pstDptx, (reg_base + reg_offset), reg_val);
	}
	return ret;
}

static int32_t dptx_cfg_set_trvc_bypass(struct Dptx_Params *pstDptx, bool bypass)
{
	uint32_t reg_offset, reg_mask, reg_val, reg_base;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	reg_base = pstDptx->uiRegBank_RegAddr_Offset;

	if (!CHECK_REG_OFFSET(reg_base)) {
		dptx_err("Invalid reg offset as 0x%x", reg_base);

		ret = DPTX_RETURN_EINVAL;
	} else {
		/* TRVC */
		if (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) {
			reg_offset = (uint32_t)DP_CFG_VIDEO_MUX;
			reg_mask = (uint32_t)CFG_SRVC_DIS_MASK;
		} else {
			reg_offset = (uint32_t)DP_REGISTER_BANK_REG_24;
			reg_mask = (uint32_t)SRVC_DIS_MASK;
		}
		reg_val = Dptx_Reg_Readl(pstDptx, (reg_base + reg_offset));
		if (bypass) {
			reg_val |= reg_mask;
		} else {
			reg_val &= ~reg_mask;
		}
		Dptx_Reg_Writel(pstDptx, (reg_base +reg_offset), reg_val);
	}
	return ret;
}

static int32_t dptx_cfg_set_video_mux_select(struct Dptx_Params *pstDptx, uint8_t ucMux_Index, uint8_t ucDP_Idx)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint8_t ucRegMap_MuxSel_Shift = 0;
	uint32_t uiRegAddr, uiReg_Offset = pstDptx->uiRegBank_RegAddr_Offset;
	uint32_t uiRegMap_MuxSel_Mask = 0;
	uint32_t uiRegMap_R_MuxSel = 0;
	uint32_t uiRegMap_W_MuxSel = 0;

	if (!CHECK_REG_OFFSET(uiReg_Offset)) {
		dptx_err("Invalid reg offset as 0x%x", uiReg_Offset);

		iRetVal = DPTX_RETURN_EINVAL;
	} else {
		uiRegAddr = (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) ? (uint32_t)DP_CFG_VIDEO_MUX : (uint32_t)DP_REGISTER_BANK_REG_24;

		switch (ucDP_Idx) {
		case (uint8_t)PHY_INPUT_STREAM_0:
			uiRegMap_MuxSel_Mask = (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) ? (uint32_t)CFG_S0_MUX_SEL_MASK : (uint32_t)SOURCE0_MUX_SEL_MASK;
			ucRegMap_MuxSel_Shift = (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) ? (uint8_t)CFG_S0_MUX_SEL_SHIFT : (uint8_t)SOURCE0_MUX_SEL_SHIFT;
			break;
		case (uint8_t)PHY_INPUT_STREAM_1:
			uiRegMap_MuxSel_Mask = (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) ? (uint32_t)CFG_S1_MUX_SEL_MASK : (uint32_t)SOURCE1_MUX_SEL_MASK;
			ucRegMap_MuxSel_Shift = (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) ? (uint8_t)CFG_S1_MUX_SEL_SHIFT : (uint8_t)SOURCE1_MUX_SEL_SHIFT;
			break;
		case (uint8_t)PHY_INPUT_STREAM_2:
			uiRegMap_MuxSel_Mask = (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) ? (uint32_t)CFG_S2_MUX_SEL_MASK : (uint32_t)SOURCE2_MUX_SEL_MASK;
			ucRegMap_MuxSel_Shift = (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) ? (uint8_t)CFG_S2_MUX_SEL_SHIFT : (uint8_t)SOURCE2_MUX_SEL_SHIFT;
			break;
		case (uint8_t)PHY_INPUT_STREAM_3:
		default:
			uiRegMap_MuxSel_Mask = (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) ? (uint32_t)CFG_S3_MUX_SEL_MASK : (uint32_t)SOURCE3_MUX_SEL_MASK;
			ucRegMap_MuxSel_Shift = (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) ? (uint8_t)CFG_S3_MUX_SEL_SHIFT : (uint8_t)SOURCE3_MUX_SEL_SHIFT;
			break;
		}

		uiRegMap_R_MuxSel = Dptx_Reg_Readl(pstDptx, (uint32_t)(uiReg_Offset + uiRegAddr));
		uiRegMap_W_MuxSel = (uiRegMap_R_MuxSel & ~uiRegMap_MuxSel_Mask);
		uiRegMap_W_MuxSel = (uiRegMap_W_MuxSel | (uint32_t)((uint32_t)ucMux_Index << ucRegMap_MuxSel_Shift));
		Dptx_Reg_Writel(pstDptx, (uiReg_Offset + uiRegAddr), uiRegMap_W_MuxSel);

		dptx_debug("Mux select[0x%x](0x%x -> 0x%x): Mux %u -> DP %u",
						(uiReg_Offset + uiRegAddr),
						uiRegMap_R_MuxSel,
						uiRegMap_W_MuxSel,
						ucMux_Index,
						ucDP_Idx);
	}

	return iRetVal;
}

static int32_t Dptx_Cfg_Set_PHY_Standard_LaneCfg(struct Dptx_Params *pstDptx)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiRegAddr, uiReg_Offset = pstDptx->uiRegBank_RegAddr_Offset;
	uint32_t uiRegMap_R_StdEn, uiRegMap_W_StdEn;

	if (pstDptx->ePhy_Dev != PHY_DEVICE_SNPS) {
		dptx_debug("Nothing to do not for Synopsys PHY");
	} else if (!CHECK_REG_OFFSET(uiReg_Offset)) {
		dptx_err("Invalid reg offset as 0x%x", uiReg_Offset);

		iRetVal = DPTX_RETURN_EINVAL;
	} else {
		uiRegAddr = (uint32_t)DP_REGISTER_BANK_REG_24;
		uiRegMap_R_StdEn = Dptx_Reg_Readl(pstDptx, (uint32_t)(uiReg_Offset + uiRegAddr));

		uiRegMap_W_StdEn = (uiRegMap_R_StdEn | (uint32_t)STD_EN_MASK);

		Dptx_Reg_Writel(pstDptx, (uiReg_Offset + uiRegAddr), uiRegMap_W_StdEn);

		dptx_info("PHY Lanes sets to standard: 0x%08x -> 0x%08x", uiRegMap_R_StdEn, uiRegMap_W_StdEn);
	}

	return iRetVal;
}

int32_t dptx_cfg_set_mux_bypass(struct Dptx_Params *pstDptx)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;

	ret = dptx_cfg_set_sdm_bypass(pstDptx, pstDptx->bSdm_Bypass);
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = dptx_cfg_set_trvc_bypass(pstDptx, pstDptx->bTrvc_Bypass);
	}
	return ret;
}

int32_t Dptx_Cfg_Init(struct Dptx_Params *pstDptx, uint8_t ucLinkRate)
{
	uint8_t ucDpIdx;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	/*
	 * During the reset process of the DisplayPort PHY for initialization,
	 * DPCD communication is not possible. Therefore, the SSC status is set
	 * based on the hw_config.
	 */
	if (pstDptx->hw_config.support_spread_specturm_clock == 1u) {
		pstDptx->stDptxLink.ssc_en = (bool)true;
	} else {
		pstDptx->stDptxLink.ssc_en = (bool)false;
	}

	ret = dptx_cfg_reg_init(pstDptx, ucLinkRate);
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = dptx_cfg_bypass_rx_vco_calibration(pstDptx);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = dptx_cfg_set_sdm_bypass(pstDptx, true);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = dptx_cfg_set_trvc_bypass(pstDptx, true);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = (pstDptx->bPhy_Lane_Std) ? Dptx_Cfg_Set_PHY_Standard_LaneCfg(pstDptx) : DPTX_RETURN_NO_ERROR;
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		for (ucDpIdx = 0; ucDpIdx < pstDptx->ucNumOfStreams; ucDpIdx++) {
			ret = (pstDptx->aucMuxId[ucDpIdx] != ucDpIdx) ? dptx_cfg_set_video_mux_select(pstDptx, pstDptx->aucMuxId[ucDpIdx], ucDpIdx) : DPTX_RETURN_NO_ERROR;
			if (DPTX_RETURN_ERROR(ret)) {
				break;
			}
		}
	}

	return ret;
}

/**
 * @brief Resets the DisplayPort PHY.
 *
 * This function provides the capability to reset the DisplayPort PHY.
 *
 * @param[in] pstDptx Pointer to the DPTX driver context structure.
 * @param link_rate The link rate used as a reference when resetting the
 *                  DisplayPort PHY.
 * @param link_lanes The number of link lanes used as a reference when
 *                   resetting the DisplayPort PHY.
 *
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
int32_t dptx_cfg_reset_phy(struct Dptx_Params *pstDptx, uint8_t link_rate, uint8_t link_lanes)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;

	/* Resets DisplayPort PHY for D3 */
	ret = dptx_cfg_reg_init(pstDptx, link_rate);

	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = dptx_cfg_bypass_rx_vco_calibration(pstDptx);
	}
	/* Resets DisplayPort PHY for D5 */
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Sec_PHY_Init(pstDptx, link_rate, link_lanes);
	}
	/* Resets DisplayPort PHY for D3/D5 */
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = dptx_core_phy_init(pstDptx);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		/* Mandatory: Wait for 20 milliseconds to stabilize the Display PHY. */
		mdelay(20);
	}
	return ret;
}

int32_t Dptx_Cfg_SoftReset(struct Dptx_Params *pstDptx, uint32_t uiVal)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiReg_W_data, uiReg_Offset = pstDptx->uiRegBank_RegAddr_Offset;

	if (pstDptx->ePhy_Dev != PHY_DEVICE_SEC) {
		dptx_debug("Nothing to do not for Samsung Phy");
	} else if (!CHECK_REG_OFFSET(uiReg_Offset)) {
		dptx_err("Invalid reg offset as 0x%x", uiReg_Offset);

		iRetVal = DPTX_RETURN_EINVAL;
	} else {
		uiReg_W_data = uiVal;

		dptx_debug("Set Reset : 0x%x", uiReg_W_data);

		Dptx_Reg_Writel(pstDptx, (uint32_t)(uiReg_Offset + (uint32_t)DP_CFG_SOFT_RESET), uiReg_W_data);
	}

	return iRetVal;
}

int32_t Dptx_Cfg_Set_PHY_Cfg(struct Dptx_Params *pstDptx, uint32_t uiCfg_Val)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiReg_W_data = 0, uiReg_R_data = 0, uiReg_Offset = pstDptx->uiRegBank_RegAddr_Offset;

	if (pstDptx->ePhy_Dev != PHY_DEVICE_SEC) {
		dptx_debug("Nothing to do not for Samsung Phy");

	} else if (!CHECK_REG_OFFSET(uiReg_Offset)) {
		dptx_err("Invalid reg offset as 0x%x", uiReg_Offset);

		iRetVal = DPTX_RETURN_EINVAL;
	} else {
		uiReg_R_data = Dptx_Reg_Readl(pstDptx, (uiReg_Offset + (uint32_t)DP_CFG_PHY_CFG));

		//uiReg_W_data = (uiReg_R_data | uiCfg_Val);
		uiReg_W_data = uiCfg_Val;

		dptx_debug("Set PHY CFG : 0x%x -> 0x%x", uiReg_R_data, uiReg_W_data);

		Dptx_Reg_Writel(pstDptx, (uiReg_Offset + (uint32_t)DP_CFG_PHY_CFG), uiReg_W_data);
	}

	return iRetVal;
}

static int32_t dptx_cfg_check_phy_pll_status(struct Dptx_Params *pstDptx,
					     uint32_t reg_offset, uint32_t reg_mask)
{
	int32_t ret_val = DPTX_RETURN_NO_ERROR;
	uint32_t reg_read_data;
	uint8_t try_count = 0;
	bool pll_status;

	if (!CHECK_REG_OFFSET(reg_offset)) {
		dptx_err("Invalid reg offset as 0x%x", reg_offset);

		ret_val = DPTX_RETURN_EINVAL;
	} else {
		do {
			reg_read_data = Dptx_Reg_Readl(pstDptx, (reg_offset + (uint32_t)DP_CFG_PHY_DEBUG));

			pll_status = ((reg_read_data & reg_mask) == reg_mask) ? (bool)true : (bool)false;

			if (pll_status) {
				dptx_debug("Sec Phy Pll status [0x%x] after %u us",
				       reg_read_data, (try_count * 100));
				break;
			}

			udelay(100);
			try_count++;
		} while (try_count < (uint8_t)MAX_TRY_PHY_PLL_LOCK);

		if (!pll_status) {
			dptx_err("Sec Phy Pll status [0x%x] after 10ms", reg_read_data);

			ret_val = DPTX_RETURN_EBUSY;
		}
	}

	return ret_val;
}

int32_t Dptx_Cfg_Check_PHY_Pll(struct Dptx_Params *pstDptx, bool *pbPll_Ready)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiReg_Offset = pstDptx->uiRegBank_RegAddr_Offset;

	*pbPll_Ready = (bool)false;

	if (pstDptx->ePhy_Dev != PHY_DEVICE_SEC) {
		dptx_debug("Nothing to do not for Samsung Phy");
	} else if (!CHECK_REG_OFFSET(uiReg_Offset)) {
		dptx_err("Invalid reg offset as 0x%x", uiReg_Offset);

		iRetVal = DPTX_RETURN_EINVAL;
	} else if (DPTX_RETURN_ERROR(dptx_cfg_check_phy_pll_status(pstDptx, uiReg_Offset,
					(uint32_t)PLL_LOCK_DONE))) {
		iRetVal = DPTX_RETURN_EBUSY;
	} else if (DPTX_RETURN_ERROR(dptx_cfg_check_phy_pll_status(pstDptx, uiReg_Offset,
					(uint32_t)(PLL_LOCK_RDY | PLL_LOCK_DONE)))) {
		iRetVal = DPTX_RETURN_EBUSY;
	} else {
		*pbPll_Ready = (bool)true;
	}

	return iRetVal;
}

int32_t Dptx_Cfg_Check_PHY_Ready(struct Dptx_Params *pstDptx, bool *pbPhy_Ready)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiReg_Offset = pstDptx->uiRegBank_RegAddr_Offset;

	*pbPhy_Ready = (bool)false;

	if (pstDptx->ePhy_Dev != PHY_DEVICE_SEC) {
		dptx_debug("Nothing to do not for Samsung Phy");
	} else if (!CHECK_REG_OFFSET(uiReg_Offset)) {
		dptx_err("Invalid reg offset as 0x%x", uiReg_Offset);

		iRetVal = DPTX_RETURN_EINVAL;
	} else if (DPTX_RETURN_ERROR(dptx_cfg_check_phy_pll_status(pstDptx, uiReg_Offset,
					(uint32_t)(PHY_RDY | PLL_LOCK_RDY | PLL_LOCK_DONE)))) {
		iRetVal = DPTX_RETURN_EBUSY;
	} else {
		*pbPhy_Ready = (bool)true;
	}

	return iRetVal;
}

int32_t Dptx_Cfg_Check_Sec_PHY_SB_Ready(struct Dptx_Params *pstDptx, bool *pbSb_Ready)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiReg_Offset = pstDptx->uiRegBank_RegAddr_Offset;

	*pbSb_Ready = (bool)false;

	if (pstDptx->ePhy_Dev != PHY_DEVICE_SEC) {
		dptx_debug("Nothing to do not for Samsung Phy");
	} else if (!CHECK_REG_OFFSET(uiReg_Offset)) {
		dptx_err("Invalid reg offset as 0x%x", uiReg_Offset);

		iRetVal = DPTX_RETURN_EINVAL;
	} else if (DPTX_RETURN_ERROR(dptx_cfg_check_phy_pll_status(pstDptx, uiReg_Offset,
					(uint32_t)SB_RDY))) {
		iRetVal = DPTX_RETURN_EBUSY;
	} else {
		*pbSb_Ready  = (bool)true;
	}

	return iRetVal;
}

static uint32_t dptx_cfg_get_equalization_internal(struct Dptx_Params *dev_param,
						  bool main_equalization,
						  uint8_t link_rate)
{
	uint32_t reg_val, reg_mask, reg_shift, reg_offset;
	uint32_t eq_val = 0u; /* 0 to 31 */

	int32_t ret = DPTX_RETURN_NO_ERROR;

	if (dev_param->ePhy_Dev != PHY_DEVICE_SNPS) {
		dptx_debug("Nothing to do not for Samsung PHY");
	} else {
		if (link_rate >= (uint8_t)LINK_RATE_MAX) {
			ret = -DPTX_RETURN_EINVAL;
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			if (!CHECK_REG_OFFSET(dev_param->uiRegBank_RegAddr_Offset)) {
				dptx_err("Invalid reg offset as 0x%x", dev_param->uiRegBank_RegAddr_Offset);
				ret = -DPTX_RETURN_EINVAL;
			}
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			if (main_equalization) {
				reg_offset = DP_REGISTER_BANK_REG_15;
				reg_shift = 8u * (uint32_t)link_rate;
				reg_mask = 0x1Fu;
			} else {
				reg_offset = DP_REGISTER_BANK_REG_16;
				reg_shift = 4u * (uint32_t)link_rate;
				reg_mask = 0xFu;
			}
			reg_val = Dptx_Reg_Readl(dev_param, (dev_param->uiRegBank_RegAddr_Offset +  reg_offset));
			eq_val = (reg_val >> reg_shift) & reg_mask;
		}
	}
	return eq_val;
}

uint32_t dptx_cfg_get_main_equalization(struct Dptx_Params *dev_param, uint8_t link_rate)
{
	return dptx_cfg_get_equalization_internal(dev_param, true, link_rate);
}

uint32_t dptx_cfg_get_post_equalization(struct Dptx_Params *dev_param, uint8_t link_rate)
{
	return dptx_cfg_get_equalization_internal(dev_param, false, link_rate);
}

static int32_t dptx_cfg_set_equalization_internal(struct Dptx_Params *dev_param,
						  bool main_equalization,
						  uint8_t link_rate, uint32_t eq_val)
{
	uint32_t reg_val, reg_shift, reg_offset;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	if (dev_param->ePhy_Dev != PHY_DEVICE_SNPS) {
		dptx_debug("Nothing to do not for Samsung PHY");
	} else {
		if (eq_val > 31u) {  /* 0 to 31 */
			dptx_err("Invalid post equalization as %u", eq_val);
			ret = -DPTX_RETURN_EINVAL;
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			if (link_rate >= (uint8_t)LINK_RATE_MAX) {
				ret = -DPTX_RETURN_EINVAL;
			}
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			if (!CHECK_REG_OFFSET(dev_param->uiRegBank_RegAddr_Offset)) {
				dptx_err("Invalid reg offset as 0x%x", dev_param->uiRegBank_RegAddr_Offset);
				ret = -DPTX_RETURN_EINVAL;
			}
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			if (main_equalization) {
				reg_offset = DP_REGISTER_BANK_REG_15;
				reg_shift = 8u * (uint32_t)link_rate;
				reg_val = ((uint32_t)1u << (reg_shift + 7u)) | ((eq_val & 0x1Fu) << reg_shift);
			} else {
				reg_offset = DP_REGISTER_BANK_REG_16;
				reg_shift = 4u * (uint32_t)link_rate;
				reg_val = (eq_val & 0xFu) << reg_shift;
			}

			Dptx_Reg_Writel(dev_param,
					(dev_param->uiRegBank_RegAddr_Offset + reg_offset),
					reg_val);
		}
	}
	return ret;
}

int32_t dptx_cfg_set_main_equalization(struct Dptx_Params *dev_param, uint8_t link_rate, uint32_t eq_val)
{
	return dptx_cfg_set_equalization_internal(dev_param, true, link_rate, eq_val);
}

int32_t dptx_cfg_set_post_equalization(struct Dptx_Params *dev_param, uint8_t link_rate, uint32_t eq_val)
{
	return dptx_cfg_set_equalization_internal(dev_param, false, link_rate, eq_val);
}

int32_t dptx_cfg_set_manual_phy_signal_quality(struct Dptx_Params *dev_param,
					      uint8_t link_rate,
					      enum PHY_VOLTAGE_SWING_LEVEL voltage_swing,
					      enum PHY_PRE_EMPHASIS_LEVEL pre_emphasis)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	uint32_t eq_val, linear_idx;

	if (dev_param->ePhy_Dev != PHY_DEVICE_SNPS) {
		dptx_info("Nothing to do not for Synopsys PHY");
	} else {
		linear_idx = ((uint32_t)voltage_swing * 4u) + (uint32_t)pre_emphasis;

		if (!dev_param->hw_config.phy_eq_manual_mode) {
			ret = -DPTX_RETURN_EINVAL;
		}
		if ((linear_idx == 7u) || (linear_idx == 10u) ||
			(linear_idx == 11u) || (linear_idx == 13u) ||
			(linear_idx == 14u) || (linear_idx == 15u) ||
			(linear_idx >= 16u)) {
			/**
			 * These values represent combinations of Voltage Swing (VSW)
			 * and Pre-Emphasis (PRE) that are invalid according to
			 * DisplayPor specifications.
			 *
			 * The array layout represents the following combinations:
			 *
			 * - VSW 0 PRE 0, VSW 0 PRE 1, ..., VSW 0 PRE 3
			 * - VSW 1 PRE 0, VSW 1 PRE 1, ..., VSW 1 PRE 3
			 * - ...
			 * - VSW 3 PRE 0, VSW 3 PRE 1, ..., VSW 3 PRE 3
			 *
			 * The loop index sequentially maps to these combinations.
			 * Invalid EQ configurations are as follows:
			 * - Index 7  (VSW 1 PRE 3)
			 * - Index 10 (VSW 2 PRE 2)
			 * - Index 11 (VSW 2 PRE 3)
			 * - Index 13 (VSW 3 PRE 1)
			 * - Index 14 (VSW 3 PRE 2)
			 * - Index 15 (VSW 3 PRE 3)
			 */
			dptx_err("Not allowed voltage swing %u level and pre-emphsis %u level", voltage_swing, pre_emphasis);
			ret = -DPTX_RETURN_EINVAL;
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			if (link_rate >= (uint8_t)LINK_RATE_MAX) {
				ret = -DPTX_RETURN_EINVAL;
			}
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			eq_val = dev_param->hw_config.phy_eq[link_rate].main_eq[linear_idx];
			dptx_info("main %u %u = %u", voltage_swing, pre_emphasis, eq_val);
			ret = dptx_cfg_set_main_equalization(dev_param, link_rate, eq_val);
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			eq_val = dev_param->hw_config.phy_eq[link_rate].post_eq[linear_idx];
			dptx_info("post %u %u = %u", voltage_swing, pre_emphasis, eq_val);
			ret = dptx_cfg_set_post_equalization(dev_param, link_rate, eq_val);
		}
	}
	return ret;
}
