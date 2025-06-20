/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
*/

/*
* Modified by Telechips Inc.
*/

#include <linux/drm_dp_helper.h>
#include <linux/delay.h>

#include "dptx_v14.h"
#include "dptx_drm_dp_addition.h"
#include "dptx_reg.h"
#include "dptx_dbg.h"

#define	MAX_NUM_OF_LOOP_PHY_STATUS	100u

#define PHY_LANE_ID_0			0
#define PHY_LANE_ID_1			1
#define PHY_LANE_ID_2			2
#define PHY_LANE_ID_3			3

#define	PHY_NUM_OF_1_LANE		1
#define	PHY_NUM_OF_2_LANE		2
#define	PHY_NUM_OF_4_LANE		4

enum dptx_reset_target {
	DPTX_RESET_COREA_ALL = 0,
	DPTX_RESET_PHY,
	DPTX_RESET_HDCP,
};

static int32_t dptx_core_check_vendor_id(struct Dptx_Params *dev_param)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	uint32_t dptx_chip_id;

	dptx_chip_id = Dptx_Reg_Readl(dev_param, DPTX_ID);
	if (dptx_chip_id != (((uint32_t)DPTX_ID_DEVICE_ID << DPTX_ID_DEVICE_ID_SHIFT) | DPTX_ID_VENDOR_ID)) {
		dptx_err("Invalid DPTX Id : 0x%x<->0x%x ",
			  dptx_chip_id,
			  (((uint32_t)DPTX_ID_DEVICE_ID << DPTX_ID_DEVICE_ID_SHIFT) | DPTX_ID_VENDOR_ID));
		ret = DPTX_RETURN_ENODEV;
	}
	return ret;
}

static void dptx_core_enable_global_intr(struct Dptx_Params *dev_param, uint32_t interrupt_enable_bits)
{
	uint32_t reg_val;

	interrupt_enable_bits &= ~(DPTX_IEN_AUX_REPLY | DPTX_IEN_AUX_CMD_INVALID |
				   DPTX_IEN_VIDEO_FIFO_OVERFLOW);

	reg_val = Dptx_Reg_Readl(dev_param, DPTX_IEN);
	reg_val |= interrupt_enable_bits ;
	Dptx_Reg_Writel(dev_param, DPTX_IEN, reg_val);
}

static void dptx_core_disable_global_intr(struct Dptx_Params *dev_param, uint32_t interrupt_disable_bits)
{
	uint32_t reg_val;

	interrupt_disable_bits |= (DPTX_IEN_AUX_REPLY | DPTX_IEN_AUX_CMD_INVALID |
				   DPTX_IEN_VIDEO_FIFO_OVERFLOW);

	reg_val = Dptx_Reg_Readl(dev_param, DPTX_IEN);
	reg_val &= ~interrupt_disable_bits;
	Dptx_Reg_Writel(dev_param, DPTX_IEN, reg_val);
}

static void dptx_core_set_phy_width(struct Dptx_Params *dev_param, uint32_t phy_data_width)
{
	uint32_t reg_val;

	reg_val = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);

	reg_val &= ~DPTX_PHYIF_CTRL_WIDTH_MASK;
	reg_val |= ((phy_data_width << DPTX_PHYIF_CTRL_WIDTH_SHIFT) & DPTX_PHYIF_CTRL_WIDTH_MASK);

	Dptx_Reg_Writel(dev_param, DPTX_PHYIF_CTRL, reg_val);
}


/**
 * @brief Initializes the DPTX core with optional reset targets.
 *
 * This function initializes the DisplayPort Transmitter (DPTX) core. The function allows
 * resetting the entire core, just the PHY depending on the `reset_target` parameter.
 *
 * @param[in] dev_param Pointer to the DPTX driver context structure.
 * @param[in] reset_target Specifies the reset target:
 *                         - 0: Resets the entire core including PHY.
 *                         - 1: Resets only the PHY.
 *
 * @return int32_t Returns 0 on success, or an error code on failure.
 */
static int32_t dptx_core_init_internal(struct Dptx_Params *dev_param, enum dptx_reset_target reset_target)
{
	uint32_t phy_data_width = (uint32_t)PHY_DATA_WIDTH_20BITS;
	uint32_t soft_reset_val = DPTX_SRST_CTRL_ALL;
	uint32_t interrupt_bits = DPTX_IEN_ALL_INTR;
	int32_t ret = DPTX_RETURN_NO_ERROR;
	uint32_t reg_val;

	ret = dptx_core_check_vendor_id(dev_param);
	if (DPTX_RETURN_SUCCESS(ret)) {
		if (reset_target == DPTX_RESET_PHY) {
			/*  If the condition is to reset only the PHY */
			soft_reset_val = DPTX_SRST_CTRL_PHY;

			interrupt_bits = DPTX_IEN_HPD;
		}
		dptx_core_disable_global_intr(dev_param, interrupt_bits);

		Dptx_Core_Soft_Reset(dev_param, soft_reset_val);

		#if defined(DEBUG_DPTX_CHIP_ID)
		if (reset_target == DPTX_RESET_COREA_ALL) {
			/*  If the condition is to reset the entire core, including PHY */
			char dptx_chip_ver[15];
			uint32_t dptx_version;

			memset(dptx_chip_ver, 0, sizeof(dptx_chip_ver));

			dptx_version = Dptx_Reg_Readl(dev_param, DPTX_VER_NUMBER);
			dptx_chip_ver[0] = (dptx_version >> 24) & 0xff;
			dptx_chip_ver[1] = '.';
			dptx_chip_ver[2] = (dptx_version >> 16) & 0xff;
			dptx_chip_ver[3] = (dptx_version >> 8) & 0xff;
			dptx_chip_ver[4] = (dptx_version & 0xff);

			dptx_version = Dptx_Reg_Readl(dev_param, DPTX_VER_TYPE);
			dptx_chip_ver[5] = '-';
			dptx_chip_ver[6] = (dptx_version >> 24) & 0xff;
			dptx_chip_ver[7] = (dptx_version >> 16) & 0xff;
			dptx_chip_ver[8] = (dptx_version >> 8) & 0xff;
			dptx_chip_ver[9] = (dptx_version & 0xff);

			dptx_debug("Core version: %s ", dptx_chip_ver);
		}
		#endif

		if (dev_param->ePhy_Dev == PHY_DEVICE_SEC) {
			phy_data_width = (uint32_t)PHY_DATA_WIDTH_40BITS;
		}
		dptx_core_set_phy_width(dev_param, phy_data_width);

		if (reset_target == DPTX_RESET_COREA_ALL) {
			/*  If the condition is to reset the entire core, including PHY */
			reg_val = Dptx_Reg_Readl(dev_param, DPTX_HPD_IEN);
			reg_val |= (DPTX_HPD_IEN_IRQ_EN | DPTX_HPD_IEN_HOT_PLUG_EN |
					DPTX_HPD_IEN_HOT_UNPLUG_EN | DPTX_HPDSTS_UNPLUG_ERR_EN);
			Dptx_Reg_Writel(dev_param, DPTX_HPD_IEN, reg_val);

			reg_val = Dptx_Reg_Readl(dev_param, DPTX_HDCP_API_INT_MSK);
			reg_val |= DPTX_HDCP22_GPIOINT;
			Dptx_Reg_Writel(dev_param, DPTX_HDCP_API_INT_MSK, reg_val);
			reg_val = Dptx_Reg_Readl(dev_param, DPTX_TYPE_C_CTRL);
			reg_val &= ~(DPTX_TYPEC_DISABLE_ACK);
			reg_val &= ~(DPTX_TYPEC_DISABLE_STATUS);
			reg_val |= DPTX_TYPEC_INTRURPPT_STATUS;
			Dptx_Reg_Writel(dev_param, DPTX_TYPE_C_CTRL, reg_val);

			reg_val = Dptx_Reg_Readl(dev_param, DPTX_CCTL);
			reg_val |= DPTX_CCTL_ENH_FRAME_EN;
			reg_val &= ~DPTX_CCTL_SCALE_DOWN_MODE;
			reg_val &= ~DPTX_CCTL_FAST_LINK_TRAINED_EN;
			Dptx_Reg_Writel(dev_param, DPTX_CCTL, reg_val);
		}
		/* Enable DisplayPort HPD Interrupts */
		dptx_core_enable_global_intr(dev_param, interrupt_bits);
	}

	return ret;
}

/**
 * @brief Initializes the DisplayPort Core and other necessary components for DisplayPort usage.
 *
 * This function is responsible for initializing the DisplayPort Core along with other necessary
 * components to enable the usage of DisplayPort. It sets up the required parameters and
 * configurations to prepare the DisplayPort for operation.
 *
 * @param[in] dev_param Pointer to the DPTX driver context structure.
 *
 * @return int32_t Returns 0 on successful initialization, or an error code on failure.
 */
int32_t Dptx_Core_Init(struct Dptx_Params *dev_param)
{
	return dptx_core_init_internal(dev_param, DPTX_RESET_COREA_ALL);
}

/**
 * @brief Initializes only the DisplayPort PHY for DisplayPort usage.
 *
 * This function is responsible for initializing the PHY (Physical Layer) of the DisplayPort.
 * It configures and prepares the PHY layer for proper operation, ensuring that the physical transmission
 * of data over the DisplayPort interface is correctly set up.
 *
 * @param[in] dev_param Pointer to the DPTX driver context structure.
 *
 * @return int32_t Returns 0 on successful initialization, or an error code on failure.
 */
int32_t dptx_core_phy_init(struct Dptx_Params *dev_param)
{
	return dptx_core_init_internal(dev_param, DPTX_RESET_PHY);
}

int32_t Dptx_Core_Deinit(struct Dptx_Params *dev_param)
{
	dptx_core_disable_global_intr(dev_param, DPTX_IEN_ALL_INTR);
	Dptx_Core_Soft_Reset(dev_param, DPTX_SRST_CTRL_ALL);

	return DPTX_RETURN_NO_ERROR;
}

void Dptx_Core_Soft_Reset(struct Dptx_Params *dev_param, uint32_t uiReset_Bits)
{
	uint32_t uiRegMap_Reset, uiRegMap_BitMask;

	uiRegMap_BitMask = (uiReset_Bits & DPTX_SRST_CTRL_ALL);

	uiRegMap_Reset = Dptx_Reg_Readl(dev_param, DPTX_SRST_CTRL);
	uiRegMap_Reset |= uiRegMap_BitMask;
	Dptx_Reg_Writel(dev_param, DPTX_SRST_CTRL, uiRegMap_Reset);

	udelay(20);

	uiRegMap_Reset = Dptx_Reg_Readl(dev_param, DPTX_SRST_CTRL);
	uiRegMap_Reset &= ~uiRegMap_BitMask;
	Dptx_Reg_Writel(dev_param, DPTX_SRST_CTRL, uiRegMap_Reset);
}

int32_t Dptx_Core_Clear_General_Interrupt(struct Dptx_Params *dev_param, uint32_t uiClear_Bits)
{
	uint32_t ucRegMap_GeneralIntr;

	ucRegMap_GeneralIntr = uiClear_Bits;
	Dptx_Reg_Writel(dev_param, DPTX_ISTS, ucRegMap_GeneralIntr);

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Core_Set_PHY_Spread_Spectrum_Clock(struct Dptx_Params *dev_param, bool ssc_en)
{
	int32_t ret_val = DPTX_RETURN_NO_ERROR;
	uint32_t reg_val = 0;

	if (dev_param == NULL) {
		ret_val = DPTX_RETURN_EINVAL;
	} else {
		reg_val = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);

		if (ssc_en) {
			reg_val = (reg_val & ~((uint32_t)DPTX_PHYIF_CTRL_SSC_DIS));
		} else {
			reg_val = (reg_val | (uint32_t)DPTX_PHYIF_CTRL_SSC_DIS);
		}

		Dptx_Reg_Writel(dev_param, DPTX_PHYIF_CTRL, reg_val);
	}

	return ret_val;
}


/**
 * @brief Determines whether Spread Spectrum Clock (SSC) should be enabled.
 *
 * This function checks the DPCD capabilities of the connected sink device
 * and decides whether SSC should be enabled. The decision is based on the
 * hardware configuration and the sink device's support for downspread.
 *
 * @note Before calling this function, ensure that the DPCD (DisplayPort Configuration Data)
 *       has been read and the sink capabilities are populated in `dev_param->aucDPCD_Caps`.
 *
 * @param[in] dev_param DisplayPort driver context pointer containing general
 *                      information for DisplayPort control.
 *
 * @retval DPTX_RETURN_NO_ERROR If the function completes successfully.
 */
int32_t dptx_core_check_ssc_enable(struct Dptx_Params *dev_param)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;

	if (dev_param == NULL) {
		dptx_err("dev_param is NULL");
		ret = -DPTX_RETURN_EINVAL;
	} else {
		if ((dev_param->aucDPCD_Caps[DP_MAX_DOWNSPREAD] & SINK_TDOWNSPREAD_MASK) == 0u) {
			dev_param->stDptxLink.ssc_en = (bool)false;
			dptx_force("The SINK does not support down spread, possibly due to DPCD version 1.1");
		} else {
			if (dev_param->hw_config.support_spread_specturm_clock == 1u) {
				dev_param->stDptxLink.ssc_en = (bool)true;
			}
		}
		//dptx_force("SSC_EN %s", dev_param->stDptxLink.ssc_en ? "enabled" : "disabled");
	}
	return ret;
}

int32_t dptx_core_set_phy_spread_specturm_clock(struct Dptx_Params *dev_param)
{
	int32_t ret_val = DPTX_RETURN_NO_ERROR;
	uint32_t reg_val = 0;

	if (dev_param == NULL) {
		ret_val = DPTX_RETURN_EINVAL;
		dptx_err("dev_param is NULL\n");
	} else {
		reg_val = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);

		if (dev_param->stDptxLink.ssc_en) {
			reg_val = (reg_val & ~((uint32_t)DPTX_PHYIF_CTRL_SSC_DIS));
		} else {
			reg_val = (reg_val | (uint32_t)DPTX_PHYIF_CTRL_SSC_DIS);
		}

		Dptx_Reg_Writel(dev_param, DPTX_PHYIF_CTRL, reg_val);
	}

	return ret_val;
}

bool dptx_core_get_phy_spread_specturm_clock(struct Dptx_Params *dev_param)
{
	bool ssc_en = (bool)false;
	uint32_t reg_val = 0;

	if (dev_param == NULL) {
		dptx_err("dev_param is NULL\n");
	} else {
		reg_val = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);

		if ((reg_val & (uint32_t)DPTX_PHYIF_CTRL_SSC_DIS) == 0u) {
			ssc_en = (bool)true;
		}
	}

	return ssc_en;
}

int32_t Dptx_Core_Set_PHY_NumOfLanes(struct Dptx_Params *dev_param, uint8_t num_of_lanes)
{
	uint32_t phy_lanes;
	uint32_t regmap_phyifctrl;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	switch (num_of_lanes) {
	case 1:
		phy_lanes = 0u;
		break;
	case 2:
		phy_lanes = 1u;
		break;
	case 4:
		phy_lanes = 2u;
		break;
	default:
		dptx_err("invalid number of lanes -> %u lanes", num_of_lanes);
		ret_val = DPTX_RETURN_EINVAL;
		break;
	}

	if (DPTX_RETURN_SUCCESS(ret_val)) {
		regmap_phyifctrl = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);
		regmap_phyifctrl &= ~DPTX_PHYIF_CTRL_LANES_MASK;
		regmap_phyifctrl |= (uint32_t)(phy_lanes << DPTX_PHYIF_CTRL_LANES_SHIFT);
		Dptx_Reg_Writel(dev_param, DPTX_PHYIF_CTRL, regmap_phyifctrl);
	}

	return ret_val;
}

int32_t Dptx_Core_Get_PHY_NumOfLanes(struct Dptx_Params *dev_param, uint8_t *p_num_of_lanes)
{
	int32_t ret_val = DPTX_RETURN_NO_ERROR;
	uint8_t num_of_lanes;
	uint32_t regmap_phy_if_ctrl;
	uint32_t tmp_lanes = 0xFFFFFFFFu;

	regmap_phy_if_ctrl = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);
	num_of_lanes = (uint8_t)((regmap_phy_if_ctrl & DPTX_PHYIF_CTRL_LANES_MASK) >> DPTX_PHYIF_CTRL_LANES_SHIFT);

	if (num_of_lanes <= 2u) {
		tmp_lanes = (uint32_t)1u << num_of_lanes;
	}

	if (tmp_lanes <= 4u) {
		*p_num_of_lanes = (uint8_t)tmp_lanes;
	} else {
		dptx_err("Invalid num_of_lanes: %u\n", tmp_lanes);
		ret_val = DPTX_RETURN_EINVAL;
	}

	return ret_val;
}

int32_t Dptx_Core_Set_PHY_PowerState(struct Dptx_Params *dev_param, enum PHY_POWER_STATE power_state)
{
	uint32_t regmap_phy_if_ctrl;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	regmap_phy_if_ctrl = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);
	regmap_phy_if_ctrl &= ~DPTX_PHYIF_CTRL_LANE_PWRDOWN_MASK;

	switch (power_state) {
	case PHY_POWER_ON:
	case PHY_POWER_DOWN_SWITCHING_RATE:
	case PHY_POWER_DOWN_PHY_CLOCK:
	case PHY_POWER_DOWN_REF_CLOCK:
		regmap_phy_if_ctrl |= ((uint32_t)power_state << DPTX_PHYIF_CTRL_LANE_PWRDOWN_SHIFT);
		Dptx_Reg_Writel(dev_param, DPTX_PHYIF_CTRL, regmap_phy_if_ctrl);
		break;
	default:
		dptx_err("Invalid power state: %d\n", (uint32_t)power_state);
		ret_val = DPTX_RETURN_EINVAL;
		break;
	}


	return ret_val;
}

int32_t Dptx_Core_Get_PHY_BUSY_Status(struct Dptx_Params *dev_param, uint8_t num_of_lanes)
{
	uint32_t regmap_phy_if_ctrl, bit_mask = 0, count = 0;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	if (dev_param->ePhy_Dev != PHY_DEVICE_SNPS) {
		dptx_debug("Nothing to do not for Synopsys PHY");
	} else {
		switch (num_of_lanes) {
		case PHY_NUM_OF_4_LANE:
			bit_mask |= DPTX_PHYIF_CTRL_BUSY(3);
			bit_mask |= DPTX_PHYIF_CTRL_BUSY(2);
			bit_mask |= DPTX_PHYIF_CTRL_BUSY(1);
			bit_mask |= DPTX_PHYIF_CTRL_BUSY(0);
			break;
		case PHY_NUM_OF_2_LANE:
			bit_mask |= DPTX_PHYIF_CTRL_BUSY(1);
			bit_mask |= DPTX_PHYIF_CTRL_BUSY(0);
			break;
		case PHY_NUM_OF_1_LANE:
			bit_mask |= DPTX_PHYIF_CTRL_BUSY(0);
			break;
		default:
			dptx_err("Invalid number of lanes %d", (uint32_t)num_of_lanes);
			ret_val = DPTX_RETURN_EINVAL;
			break;
		}

		if (DPTX_RETURN_SUCCESS(ret_val)) {
			do {
				regmap_phy_if_ctrl = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);

				if ((regmap_phy_if_ctrl & bit_mask) == 0u) {
					break;
				}

				if (count == MAX_NUM_OF_LOOP_PHY_STATUS) {
					dptx_err("PHY BUSY timed out");
					ret_val = DPTX_RETURN_ENODEV;
				}

				mdelay(1);
			} while (count++ < MAX_NUM_OF_LOOP_PHY_STATUS);
		}
	}

	return ret_val;
}

int32_t Dptx_Core_Set_PHY_Rate(struct Dptx_Params *dev_param, enum PHY_LINK_RATE eRate)
{
	uint32_t uiPhyIfCtrl;

	uiPhyIfCtrl = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);
	uiPhyIfCtrl &= ~DPTX_PHYIF_CTRL_RATE_MASK;
	uiPhyIfCtrl |= (uint32_t)eRate << DPTX_PHYIF_CTRL_RATE_SHIFT;

	Dptx_Reg_Writel(dev_param, DPTX_PHYIF_CTRL, uiPhyIfCtrl);

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Core_Get_PHY_Rate(struct Dptx_Params *dev_param, uint8_t *pucPHY_Rate)
{
	uint32_t UiRegMap_PHY_IF_Ctrl, uiRate;

	UiRegMap_PHY_IF_Ctrl = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);
	uiRate = (UiRegMap_PHY_IF_Ctrl & DPTX_PHYIF_CTRL_RATE_MASK) >> DPTX_PHYIF_CTRL_RATE_SHIFT;

	*pucPHY_Rate = (uint8_t)uiRate;

	return DPTX_RETURN_NO_ERROR;
}

static int32_t Dptx_Core_Set_PHY_PreEmphasis(struct Dptx_Params *dev_param,
					     uint32_t line_idx,
					     enum PHY_PRE_EMPHASIS_LEVEL pre_emphasis_level)
{
	uint32_t regmap_phy_tx_eq;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	if (dev_param->ePhy_Dev != PHY_DEVICE_SNPS) {
		dptx_debug("Nothing to do not for Synopsys PHY");

		ret_val = DPTX_RETURN_NO_ERROR;
	} else if (line_idx > (uint32_t)PHY_LANE_ID_3) {
		dptx_err("Invalid lane %d ", line_idx);

		ret_val = DPTX_RETURN_EINVAL;
	} else {
		if (pre_emphasis_level > PRE_EMPHASIS_LEVEL_3) {
			dptx_err("Invalid pre-emphasis level %d, using 3 ", pre_emphasis_level);
			pre_emphasis_level = PRE_EMPHASIS_LEVEL_3;
		}

		regmap_phy_tx_eq = Dptx_Reg_Readl(dev_param, DPTX_PHY_TX_EQ);
		regmap_phy_tx_eq &= ~(DPTX_PHY_TX_EQ_PREEMP_MASK(line_idx));
		regmap_phy_tx_eq |= ((uint32_t)pre_emphasis_level << DPTX_PHY_TX_EQ_PREEMP_SHIFT(line_idx))
			& DPTX_PHY_TX_EQ_PREEMP_MASK(line_idx);

		Dptx_Reg_Writel(dev_param, DPTX_PHY_TX_EQ, regmap_phy_tx_eq);
	}

	return ret_val;
}

static int32_t Dptx_Core_Set_PHY_VSW(struct Dptx_Params *dev_param,
				     uint32_t line_idx,
				     enum PHY_VOLTAGE_SWING_LEVEL voltage_swing_level)
{
	uint32_t regmap_phy_tx_eq;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	if (dev_param->ePhy_Dev != PHY_DEVICE_SNPS) {
		dptx_debug("Nothing to do not for Synopsys PHY");

		ret_val = DPTX_RETURN_NO_ERROR;
	} else if (line_idx > (uint32_t)PHY_LANE_ID_3) {
		dptx_err("Invalid lane %d ", line_idx);

		ret_val = DPTX_RETURN_EINVAL;
	} else {
		if (voltage_swing_level > VOLTAGE_SWING_LEVEL_3) {
			dptx_err("Invalid vswing level %d, using 3 ", voltage_swing_level);
			voltage_swing_level = VOLTAGE_SWING_LEVEL_3;
		}

		regmap_phy_tx_eq = Dptx_Reg_Readl(dev_param, DPTX_PHY_TX_EQ);
		regmap_phy_tx_eq &= ~(DPTX_PHY_TX_EQ_VSWING_MASK(line_idx));
		regmap_phy_tx_eq |= ((uint32_t)voltage_swing_level << DPTX_PHY_TX_EQ_VSWING_SHIFT(line_idx))
			& DPTX_PHY_TX_EQ_VSWING_MASK(line_idx);

		Dptx_Reg_Writel(dev_param, DPTX_PHY_TX_EQ, regmap_phy_tx_eq);
	}

	return ret_val;
}


int32_t dptx_core_set_phy_lane_sigan_quality(struct Dptx_Params *dev_param,
					     uint32_t lane_idx, uint8_t link_rate,
					     enum PHY_VOLTAGE_SWING_LEVEL voltage_swing,
					     enum PHY_PRE_EMPHASIS_LEVEL pre_emphasis)
{
	int32_t ret =  DPTX_RETURN_NO_ERROR;

	if (dev_param->hw_config.phy_eq_manual_mode) {
		ret = dptx_cfg_set_manual_phy_signal_quality(dev_param, link_rate,
					       voltage_swing, pre_emphasis);
		if (DPTX_RETURN_SUCCESS(ret)) {
			ret = dptx_sec_set_phy_sigan_quality(dev_param, lane_idx,
					link_rate, voltage_swing, pre_emphasis);
		}
	} else {
		ret = Dptx_Core_Set_PHY_PreEmphasis(dev_param, lane_idx, pre_emphasis);
		if (DPTX_RETURN_SUCCESS(ret)) {
			ret = Dptx_Core_Set_PHY_VSW(dev_param, lane_idx, voltage_swing);
		}
	}
	return ret;
}

int32_t dptx_core_set_phy_sigan_quality(struct Dptx_Params *dptx_param)
{
	uint32_t lane_idx;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	for (lane_idx = 0u; lane_idx < dptx_param->stDptxLink.ucNumOfLanes; lane_idx++) {
		ret = dptx_core_set_phy_lane_sigan_quality(dptx_param, lane_idx,
					     dptx_param->stDptxLink.ucLinkRate,
					     (enum PHY_VOLTAGE_SWING_LEVEL)dptx_param->stDptxLink.aucVoltageSwing_level[lane_idx],
					     (enum PHY_PRE_EMPHASIS_LEVEL)dptx_param->stDptxLink.aucPreEmphasis_level[lane_idx]);
		if (DPTX_RETURN_ERROR(ret)) {
			break;
		}
	}
	return ret;
}

int32_t Dptx_Core_Set_PHY_Pattern(struct Dptx_Params *dev_param, uint32_t uiPattern)
{
	uint32_t uiPhyTPSSelection = 0;

	uiPhyTPSSelection = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);
	uiPhyTPSSelection &= ~DPTX_PHYIF_CTRL_TPS_SEL_MASK;
	uiPhyTPSSelection |= ((uiPattern << DPTX_PHYIF_CTRL_TPS_SEL_SHIFT) & DPTX_PHYIF_CTRL_TPS_SEL_MASK);

	Dptx_Reg_Writel(dev_param, DPTX_PHYIF_CTRL, uiPhyTPSSelection);

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Core_Enable_PHY_XMIT(struct Dptx_Params *dev_param, uint32_t num_of_lanes)
{
	uint32_t regmap_phy_if_ctrl, bit_mask = 0;
	int ret_val = DPTX_RETURN_NO_ERROR;

	regmap_phy_if_ctrl = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);

	switch (num_of_lanes) {
	case PHY_NUM_OF_4_LANE:
		bit_mask |= (uint32_t)DPTX_PHYIF_CTRL_XMIT_EN(3);
		bit_mask |= (uint32_t)DPTX_PHYIF_CTRL_XMIT_EN(2);
		bit_mask |= (uint32_t)DPTX_PHYIF_CTRL_XMIT_EN(1);
		bit_mask |= (uint32_t)DPTX_PHYIF_CTRL_XMIT_EN(0);
		break;
	case PHY_NUM_OF_2_LANE:
		bit_mask |= (uint32_t)DPTX_PHYIF_CTRL_XMIT_EN(1);
		bit_mask |= (uint32_t)DPTX_PHYIF_CTRL_XMIT_EN(0);
		break;
	case PHY_NUM_OF_1_LANE:
		bit_mask |= (uint32_t)DPTX_PHYIF_CTRL_XMIT_EN(0);
		break;
	default:
		dptx_err("Invalid number of lanes %d", (uint32_t)num_of_lanes);

		ret_val = DPTX_RETURN_EINVAL;
		break;
	}

	if (DPTX_RETURN_SUCCESS(ret_val)) {
		regmap_phy_if_ctrl |= bit_mask;

		Dptx_Reg_Writel(dev_param, DPTX_PHYIF_CTRL, regmap_phy_if_ctrl);
	}

	return ret_val;
}

int32_t Dptx_Core_Disable_PHY_XMIT(struct Dptx_Params *dev_param, uint32_t num_of_lanes)
{
	uint32_t regmap_phy_if_ctrl, bit_mask = 0;
	int ret_val = DPTX_RETURN_NO_ERROR;

	regmap_phy_if_ctrl = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);

	switch (num_of_lanes) {
	case PHY_NUM_OF_4_LANE:
		bit_mask |= DPTX_PHYIF_CTRL_XMIT_EN(3);
		bit_mask |= DPTX_PHYIF_CTRL_XMIT_EN(2);
		bit_mask |= DPTX_PHYIF_CTRL_XMIT_EN(1);
		bit_mask |= DPTX_PHYIF_CTRL_XMIT_EN(0);
		break;
	case PHY_NUM_OF_2_LANE:
		bit_mask |= DPTX_PHYIF_CTRL_XMIT_EN(1);
		bit_mask |= DPTX_PHYIF_CTRL_XMIT_EN(0);
		break;
	case PHY_NUM_OF_1_LANE:
		bit_mask |= DPTX_PHYIF_CTRL_XMIT_EN(0);
		break;
	default:
		dptx_err("Invalid number of lanes %d", (uint32_t)num_of_lanes);

		ret_val = DPTX_RETURN_EINVAL;
		break;
	}

	if (DPTX_RETURN_SUCCESS(ret_val)) {
		regmap_phy_if_ctrl &= ~bit_mask;

		Dptx_Reg_Writel(dev_param, DPTX_PHYIF_CTRL, regmap_phy_if_ctrl);
	}

	return ret_val;
}


