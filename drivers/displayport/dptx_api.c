// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/

#include <linux/drm_dp_helper.h>
#include <linux/delay.h>

#include "dptx_api.h"
#include "dptx_v14.h"
#include "dptx_reg.h"
#include "dptx_dbg.h"
#include "dptx_drm_dp_addition.h"

/*
 * For some DisplayPort to HDMI active cables, it takes more than 2 seconds for
 * The link to stabilize after connection. In other words, the time required for
 * HPD (Hot Plug Detect) to be asserted can exceed 2 seconds.
 * To accommodate this behavior, the maximum wait time has been increased
 * to 3 seconds.
 * One confirmed DP-to-HDMI cable exhibiting this behavior is the Richevity
 * DP2HDMI active cable.
 */
#define MAX_CHECK_HPD_NUM 300u

static struct Dptx_Params *dptx_api_dev_param;

int32_t Dpv14_Tx_API_Init(const struct dpv14_drv_params *drv_params)
{
	struct Dptx_Params *dev_param = NULL;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	if (drv_params->num_of_dps > (uint8_t)DPTX_INPUT_STREAM_MAX) {
		dptx_err("Invalid Num Of Streams as %u", drv_params->num_of_dps);

		ret = -DPTX_RETURN_EINVAL;
	} else if (drv_params->max_rate >= (uint8_t)DPTX_LINK_RATE_MAX) {
		dptx_err("Invalid link rate as %u", drv_params->max_rate);

		ret = -DPTX_RETURN_EINVAL;
	} else if ((drv_params->max_lane != (uint8_t)DPTX_PHY_LANE_NUM_1) &&
		(drv_params->max_lane != (uint8_t)DPTX_PHY_LANE_NUM_2) &&
		(drv_params->max_lane != (uint8_t)DPTX_PHY_LANE_NUM_4)) {
		dptx_err("Invalid link lane as %u", drv_params->max_lane);

		ret = -DPTX_RETURN_EINVAL;
	} else if (Dptx_V14_Init(drv_params) != DPTX_RETURN_NO_ERROR) {
		ret = -DPTX_RETURN_EINVAL;
	} else {
		dev_param = Dptx_V14_Get_Device_Handle();
		if (dev_param == NULL) {
			dptx_err("Failed to get handle");

			ret = -DPTX_RETURN_EACCES;
		} else {
			dptx_api_dev_param = dev_param;
		}
	}

	return ret;
}

int32_t Dpv14_Tx_API_Deinit(void)
{
	struct Dptx_Params *dev_param = NULL;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	dev_param = dptx_api_dev_param;
	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	} else if (Dptx_Core_Deinit(dev_param) != DPTX_RETURN_NO_ERROR) {
		ret = -DPTX_RETURN_EINVAL;
	} else {
		ret = Dptx_V14_Deinit(dev_param);
	}

	return ret;
}

int32_t Dpv14_Tx_API_Start(uint8_t num_of_dps, const uint32_t pixel_clocks[DPTX_INPUT_STREAM_MAX])
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *dev_param;
	uint32_t hpd_check_count;

	dev_param = dptx_api_dev_param;
	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	} else if (Dptx_Vidin_Set_Video_PPClk(dev_param, num_of_dps, pixel_clocks) != DPTX_RETURN_NO_ERROR) {
		ret = -DPTX_RETURN_EINVAL;
	} else {
		for (hpd_check_count = 0u; hpd_check_count < MAX_CHECK_HPD_NUM; hpd_check_count++) {
			if (dptx_intr_get_hotplug_status(dev_param) == HPD_STATUS_PLUGGED) {
				dptx_info("Hot plugged after %ums\n", (hpd_check_count * 10U));
				break;
			}

			mdelay(10);
		}

		if (hpd_check_count == MAX_CHECK_HPD_NUM) {
			dptx_err("Hot unplugged after %ums\n", (hpd_check_count * 10U));

			ret = -DPTX_RETURN_EINVAL;
		} else {
			ret = Dptx_Intr_Handle_Hotplug(dev_param);
		}
	}

	return ret;
}

int32_t Dpv14_Tx_API_Stop(void)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *dev_param;

	dev_param = dptx_api_dev_param;
	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
		ret = -DPTX_RETURN_EACCES;
	} else {
		ret = Dptx_Intr_Handle_HotUnplug(dev_param);
	}

	return ret;
}

int32_t Dpv14_Tx_API_Set_PW(void)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *dev_param;

	dev_param = dptx_api_dev_param;
	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	} else {
		(void)Dptx_Protect_Set_PW(dev_param);
	}

	return ret;
}

int32_t Dpv14_Tx_API_Set_CfgLock(bool request_access)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	bool lock_required = (bool)true;
	struct Dptx_Params *dev_param;

	dev_param = dptx_api_dev_param;
	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	} else {
		if (request_access) {
			lock_required = (bool)false;
		}
		ret = Dptx_Protect_Set_CfgLock(dev_param, lock_required);
	}

	return ret;
}

int32_t Dpv14_Tx_API_Set_CfgAccess(bool request_access)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *dev_param;

	dev_param = dptx_api_dev_param;
	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	} else {
		ret = Dptx_Protect_Set_CfgAccess(dev_param, request_access);
	}

	return ret;

}

int32_t Dpv14_Tx_API_Set_MaxLinkRate_Supported(enum DPTX_LINK_RATE link_rate)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *dev_param;

	dev_param = dptx_api_dev_param;
	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	} else if (link_rate >= DPTX_LINK_RATE_MAX) {
		dptx_err("Invalid link rate %u", (uint32_t)link_rate);

		ret = -DPTX_RETURN_EINVAL;
	} else {
		dev_param->ucMax_Rate = (uint8_t)link_rate;
	}

	return ret;
}

int32_t Dpv14_Tx_API_Set_MaxLinkLane_Supported(enum DPTX_LINK_LANE link_lane)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *dev_param;

	dev_param = dptx_api_dev_param;
	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	} else if (link_lane > DPTX_PHY_LANE_NUM_4) {
		dptx_err("Invalid link lane %u", (uint32_t)link_lane);

		ret = -DPTX_RETURN_EINVAL;
	} else {
		dev_param->ucMax_Lanes = (uint8_t)link_lane;
	}

	return ret;
}

int32_t dpv14_tx_api_set_spread_specturm_clock(bool ssc_en)
{
	struct Dptx_Params *dev_param = dptx_api_dev_param;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	if (dev_param == NULL) {
		dptx_err("dev_param is NULL");

		ret = -DPTX_RETURN_EACCES;
	} else {
		dev_param->stDptxLink.ssc_en = ssc_en;
	}

	return ret;
}

int32_t Dpv14_Tx_API_Perform_HPD_WorkingFlow(void)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *dev_param;

	dev_param = dptx_api_dev_param;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	} else if (Dptx_Intr_Handle_HotUnplug(dev_param) != DPTX_RETURN_NO_ERROR) {
		ret = -DPTX_RETURN_EINVAL;
	} else if (dptx_cfg_reset_phy(dev_param, dev_param->ucMax_Rate,
			       dev_param->ucMax_Lanes) != DPTX_RETURN_NO_ERROR) {
		dptx_err("Failed dptx_cfg_reset_phy");
		ret = -DPTX_RETURN_EINVAL;
	} else {
		/* Reset  hysteresi to 1 */
		dev_param->hw_config.aux_hysteresis = 1u;

		if (Dptx_Intr_Handle_Hotplug(dev_param) != DPTX_RETURN_NO_ERROR) {
			ret = -DPTX_RETURN_EINVAL;
		}
	}

	return ret;
}

int32_t Dpv14_Tx_API_Get_Dtd_From_VideoCode(
			uint32_t video_code,
			struct DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
			uint32_t video_refresh_rate,
			uint8_t video_format_standard)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	struct Dptx_Dtd_Params stDtd;

	if (pstDptx_Dtd_Params == NULL) {
		dptx_err("Ptr. of Dtd params is NULL");
		ret = -DPTX_RETURN_EINVAL;
	} else if (video_format_standard > (uint8_t)VFS_VESA_DMT) {
		dptx_err("video format standard %u is out of range", video_format_standard);
		ret = -DPTX_RETURN_EINVAL;
	} else {
		ret = Dptx_Vidin_Fill_Dtd(&stDtd, video_code,
				       video_refresh_rate,
				       (enum VIDEO_FORMAT_STANDARD_TYPE)video_format_standard);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		pstDptx_Dtd_Params->interlaced = stDtd.interlaced;
		pstDptx_Dtd_Params->h_sync_polarity = stDtd.h_sync_polarity;
		pstDptx_Dtd_Params->v_sync_polarity = stDtd.v_sync_polarity;
		pstDptx_Dtd_Params->pixel_repetition_input = stDtd.pixel_repetition_input;
		pstDptx_Dtd_Params->h_active = stDtd.h_active;
		pstDptx_Dtd_Params->h_blanking = stDtd.h_blanking;
		pstDptx_Dtd_Params->h_image_size = stDtd.h_image_size;
		pstDptx_Dtd_Params->h_sync_offset = stDtd.h_sync_offset;
		pstDptx_Dtd_Params->h_sync_pulse_width = stDtd.h_sync_pulse_width;
		pstDptx_Dtd_Params->v_active = stDtd.v_active;
		pstDptx_Dtd_Params->v_blanking = stDtd.v_blanking;
		pstDptx_Dtd_Params->v_image_size = stDtd.v_image_size;
		pstDptx_Dtd_Params->v_sync_offset = stDtd.v_sync_offset;
		pstDptx_Dtd_Params->v_sync_pulse_width = stDtd.v_sync_pulse_width;
		pstDptx_Dtd_Params->uiPixel_Clock = stDtd.uiPixel_Clock;
	}

	return ret;
}

int32_t Dpv14_Tx_API_Get_Dtd_From_Edid(struct DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
				       uint8_t dp_stream_id)
{
	struct Dptx_Params *dev_param = dptx_api_dev_param;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	} else if (dp_stream_id >= (uint8_t)DPTX_INPUT_STREAM_MAX) {
		dptx_err("Invalid DP index %u", dp_stream_id);

		ret = -DPTX_RETURN_EINVAL;
	} else if (Dptx_Intr_Handle_Edid(dev_param, dp_stream_id) != DPTX_RETURN_NO_ERROR) {
		ret = -DPTX_RETURN_EINVAL;
	} else {
		(void)memcpy(pstDptx_Dtd_Params,
			     &dev_param->video_params[dp_stream_id].dtd_param,
			     sizeof(struct  DPTX_Dtd_Params_t));
	}

	return ret;
}

int32_t Dpv14_Tx_API_Write_RegisterBank_Value(const uint32_t *reg_data,
					      uint32_t reg_offset,
					      uint8_t data_cnt)
{
	uint8_t ucCount;
	int32_t ret = DPTX_RETURN_NO_ERROR;
	uint32_t uiRegOffset, uiOneRegOffset;
	struct Dptx_Params *dev_param = dptx_api_dev_param;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	} else if (reg_data == NULL) {
		dptx_err("Invalid parameter as reg_data is NULL");

		ret = -DPTX_RETURN_EINVAL;
	} else if (data_cnt == 0U) {
		dptx_err("Invalid parameter as Length is 0");

		ret = -DPTX_RETURN_EINVAL;
	} else if (reg_offset > (uint32_t)DP_MAX_OFFSET) {
		dptx_err("Invalid offset as 0x%x", reg_offset);

		ret = -DPTX_RETURN_EINVAL;
	} else if (dev_param->uiRegBank_RegAddr_Offset > (uint32_t)DP_MAX_OFFSET) {
		dptx_err("Invalid RegBank offset as 0x%x", dev_param->uiRegBank_RegAddr_Offset);

		ret = -DPTX_RETURN_EINVAL;
	} else {
		for (ucCount = 0; ucCount < data_cnt; ucCount++) {
			uiRegOffset = (dev_param->uiRegBank_RegAddr_Offset + reg_offset);
			uiOneRegOffset = (uint32_t)(sizeof(uint32_t) * ucCount);

			uiRegOffset += uiOneRegOffset;

			Dptx_Reg_Writel(dev_param, uiRegOffset, reg_data[ucCount]);
		}
	}

	return ret;
}

int32_t Dpv14_Tx_API_Read_RegisterBank_Value(uint32_t *reg_data,
					     uint32_t reg_offset,
					     uint8_t data_cnt)
{
	uint8_t ucCount;
	int32_t ret = DPTX_RETURN_NO_ERROR;
	uint32_t uiRegOffset, uiOneRegOffset;
	struct Dptx_Params *dev_param = dptx_api_dev_param;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	} else if (reg_data == NULL) {
		dptx_err("Invalid parameter as reg_data is NULL");

		ret = -DPTX_RETURN_EINVAL;
	} else if (data_cnt == 0U) {
		dptx_err("Invalid parameter as Length is 0");

		ret = -DPTX_RETURN_EINVAL;
	} else if (reg_offset > (uint32_t)DP_MAX_OFFSET) {
		dptx_err("Invalid offset as 0x%x", reg_offset);

		ret = -DPTX_RETURN_EINVAL;
	} else if (dev_param->uiRegBank_RegAddr_Offset > (uint32_t)DP_MAX_OFFSET) {
		dptx_err("Invalid RegBank offset as 0x%x",
					dev_param->uiRegBank_RegAddr_Offset);

		ret = -DPTX_RETURN_EINVAL;
	} else {
		for (ucCount = 0; ucCount < data_cnt; ucCount++) {
			uiRegOffset = (dev_param->uiRegBank_RegAddr_Offset + reg_offset);
			uiOneRegOffset = (uint32_t)(sizeof(uint32_t) * ucCount);

			uiRegOffset = (uiRegOffset + uiOneRegOffset);

			reg_data[ucCount] = Dptx_Reg_Readl(dev_param, uiRegOffset);
		}
	}

	return ret;
}

int32_t Dpv14_Tx_API_Get_PLL_Status(uint8_t *pll_locked)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *dev_param;

	dev_param = dptx_api_dev_param;
	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	} else if (pll_locked == NULL) {
		dptx_err("pll_locked is NULL");

		ret = -DPTX_RETURN_EINVAL;
	} else {
		Dptx_Clk_Get_PLLLock_Status(dev_param, pll_locked);
	}

	return ret;
}

static int32_t dpv14_tx_api_cts_reset_pattern(struct Dptx_Params *dev_param, uint32_t pattern_type)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;

	if (pattern_type == (uint32_t)DPTX_TRAINING_CTRL_TPS_CUSTOM80) {
		Dptx_Reg_Writel(dev_param, DPTX_CUSTOMPAT0, 0x3E0F83E0);
		Dptx_Reg_Writel(dev_param, DPTX_CUSTOMPAT1, 0x3E0F83E0);
		Dptx_Reg_Writel(dev_param, DPTX_CUSTOMPAT2, 0x3E0F83E0);
	}

	if (Dptx_Core_Set_PHY_Pattern(dev_param, (uint32_t)pattern_type) != DPTX_RETURN_NO_ERROR) {
		ret = -DPTX_RETURN_EINVAL;
	} else if (Dptx_Core_Enable_PHY_XMIT(dev_param, (uint32_t)DPTX_PHY_LANE_NUM_4)
		!= DPTX_RETURN_NO_ERROR) {
		ret = -DPTX_RETURN_EINVAL;
	} else {
		ret = -DPTX_RETURN_NO_ERROR;
	}

	return ret;
}

static int32_t dpv14_tx_api_set_ssc(struct Dptx_Params *dev_param, bool ssc_enable)
{
	uint32_t reg_val = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);

	reg_val &= ~DPTX_PHYIF_CTRL_SSC_DIS;

	if (!ssc_enable) {
		reg_val |= DPTX_PHYIF_CTRL_SSC_DIS;
	}
	Dptx_Reg_Writel(dev_param, DPTX_PHYIF_CTRL, reg_val);

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dpv14_tx_api_prepare_cts(struct Dptx_Params *dev_param, bool ssc_enable)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;

	dptx_info("Starting... Num of lanes(%u), Link rate(%u)",
		  dev_param->stDptxLink.ucNumOfLanes, dev_param->stDptxLink.ucLinkRate);

	ret = Dptx_Core_Set_PHY_PowerState(dev_param, PHY_POWER_DOWN_PHY_CLOCK);
	if (DPTX_RETURN_ERROR(ret)) {
		dptx_err("Failed PHY_POWER_DOWN_PHY_CLOCK()");
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Core_Get_PHY_BUSY_Status(dev_param, dev_param->stDptxLink.ucNumOfLanes);
		if (DPTX_RETURN_ERROR(ret)) {
			dptx_err("Failed Dptx_Core_Get_PHY_BUSY_Status()");
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Core_Set_PHY_NumOfLanes(dev_param, (uint8_t)dev_param->stDptxLink.ucNumOfLanes);
		if (DPTX_RETURN_ERROR(ret)) {
			dptx_err("Failed Dptx_Core_Set_PHY_NumOfLanes()");
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Core_Get_PHY_BUSY_Status(dev_param, dev_param->stDptxLink.ucNumOfLanes);
		if (DPTX_RETURN_ERROR(ret)) {
			dptx_err("Failed Dptx_Core_Get_PHY_BUSY_Status()");
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Core_Set_PHY_Rate(dev_param, (enum PHY_LINK_RATE)dev_param->stDptxLink.ucLinkRate);
		if (DPTX_RETURN_ERROR(ret)) {
			dptx_err("Failed Dptx_Core_Set_PHY_Rate()");
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Core_Get_PHY_BUSY_Status(dev_param, dev_param->stDptxLink.ucNumOfLanes);
		if (DPTX_RETURN_ERROR(ret)) {
			dptx_err("Failed Dptx_Core_Get_PHY_BUSY_Status()");
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = dpv14_tx_api_set_ssc(dev_param, ssc_enable);
		if (DPTX_RETURN_ERROR(ret)) {
			dptx_err("Failed dpv14_tx_api_set_ssc()");
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Core_Get_PHY_BUSY_Status(dev_param, dev_param->stDptxLink.ucNumOfLanes);
		if (DPTX_RETURN_ERROR(ret)) {
			dptx_err("Failed Dptx_Core_Get_PHY_BUSY_Status()");
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Core_Set_PHY_PowerState(dev_param, PHY_POWER_ON);
		if (DPTX_RETURN_ERROR(ret)) {
			dptx_err("Failed PHY_POWER_ON()");
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Core_Get_PHY_BUSY_Status(dev_param, dev_param->stDptxLink.ucNumOfLanes);
		if (DPTX_RETURN_ERROR(ret)) {
			dptx_err("Failed Dptx_Core_Get_PHY_BUSY_Status()");
		}
	}
	return ret;
}

int32_t Dpv14_Tx_API_Set_CTS(bool ssc_enable,
			     uint32_t pattern_type,
			     enum DPTX_LINK_RATE link_rate,
			     enum DPTX_PRE_EMPHASIS_LEVEL pre_emphasis,
			     enum DPTX_VOLTAGE_SWING_LEVEL voltage_swing)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	uint32_t reg_val;
	struct Dptx_Params *dev_param;

	dev_param = dptx_api_dev_param;
	if (dev_param == NULL) {
		dptx_err("Failed to get handle");

		ret = -DPTX_RETURN_EACCES;
	}

	if (DPTX_RETURN_SUCCESS(ret)) {
		dptx_cts("CTS Test => ");
		dptx_cts(" -.Link rate: %s",
					(link_rate == DPTX_LINK_RATE_RBR) ? "RBR" :
					(link_rate == DPTX_LINK_RATE_HBR) ? "HBR" :
					(link_rate == DPTX_LINK_RATE_HBR2) ? "HBR2" : "HBR3");
		dptx_cts(" -.SSC: %s", ssc_enable ? "Enable" : "Disable");
		dptx_cts(" -.Pre Emp: level %u", (uint32_t)pre_emphasis);
		dptx_cts(" -.VSW: level %u", (uint32_t)voltage_swing);
		dptx_cts(" -.Pattern: %s",
			(pattern_type == (uint32_t)DPTX_TRAINING_CTRL_TPS_1_D102) ?
			"D10.2" :
			(pattern_type == (uint32_t)DPTX_TRAINING_CTRL_TPS_PRBS7) ?
			"PRBS7" :
			(pattern_type == (uint32_t)DPTX_TRAINING_CTRL_TPS_CUSTOM80) ?
			"CUSTOM80" :
			(pattern_type == (uint32_t)DPTX_TRAINING_CTRL_TPS_CP2520_1) ?
			"CP2520_P1" :
			(pattern_type == (uint32_t)DPTX_TRAINING_CTRL_TPS_CP2520_2) ?
			"CP2520_P2" : "TPS 4");

		dev_param->stDptxLink.ucLinkRate = (uint8_t)link_rate;
		dev_param->stDptxLink.ucNumOfLanes = (uint8_t)DPTX_PHY_LANE_NUM_4;
		dev_param->stDptxLink.ssc_en = ssc_enable;
		ret = dptx_cfg_reset_phy(dev_param,
					 dev_param->stDptxLink.ucLinkRate,
					 dev_param->stDptxLink.ucNumOfLanes);
	}

	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = dptx_power_up_sink_device_and_check_aux_hysteresis(dev_param);
		if (DPTX_RETURN_ERROR(ret)) {
			dptx_err("DPCDW - DP_SET_POWER_D0");
		} else {
			(void)memset(dev_param->aucDPCD_Caps, 0, DPTX_SINK_CAP_SIZE);
			ret = drm_addition_read_dpcd_caps(dev_param);
			if (DPTX_RETURN_ERROR(ret)) {
				dptx_err("DPCDR - DP_DPCD_REV");
			}
		}
		if ((dev_param->aucDPCD_Caps[DP_MAX_DOWNSPREAD] & SINK_TDOWNSPREAD_MASK) == 0u) {
			dptx_force("SSC = %s, DUT is not support SSC", dev_param->stDptxLink.ssc_en ? "enable" : "disable");
			//dev_param->stDptxLink.ssc_en = (bool)true;
		} else {
			dptx_force("SSC = %s, DUT is support SSC", dev_param->stDptxLink.ssc_en ? "enable" : "disable");
			//dev_param->stDptxLink.ssc_en = (bool)false;
		}

		ret = dpv14_tx_api_prepare_cts(dev_param, ssc_enable);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Core_Set_PHY_Pattern(dev_param, DPTX_PHYIF_CTRL_TPS_NONE);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		dev_param->stDptxLink.aucPreEmphasis_level[0] = (uint8_t)pre_emphasis;
		dev_param->stDptxLink.aucPreEmphasis_level[1] = (uint8_t)pre_emphasis;
		dev_param->stDptxLink.aucPreEmphasis_level[2] = (uint8_t)pre_emphasis;
		dev_param->stDptxLink.aucPreEmphasis_level[3] = (uint8_t)pre_emphasis;
		dev_param->stDptxLink.aucVoltageSwing_level[0] = (uint8_t)voltage_swing;
		dev_param->stDptxLink.aucVoltageSwing_level[1] = (uint8_t)voltage_swing;
		dev_param->stDptxLink.aucVoltageSwing_level[2] = (uint8_t)voltage_swing;
		dev_param->stDptxLink.aucVoltageSwing_level[3] = (uint8_t)voltage_swing;

		ret = dptx_core_set_phy_sigan_quality(dev_param);
	}

	if (DPTX_RETURN_SUCCESS(ret)) {
			ret = dpv14_tx_api_cts_reset_pattern(dev_param, pattern_type);
	}

	if (DPTX_RETURN_SUCCESS(ret)) {
		reg_val = Dptx_Reg_Readl(dev_param, (uint32_t)DPTX_CCTL);
		reg_val = (reg_val | (uint32_t)DPTX_CCTL_ENH_FRAME_EN);
		Dptx_Reg_Writel(dev_param, DPTX_CCTL, reg_val);
	}

	return  ret;
}

uint32_t dpv14_api_get_current_link_rate(void)
{
	const struct Dptx_Params *dev_param = (const struct Dptx_Params *)dptx_api_dev_param;
	uint32_t link_rate = (uint32_t)LINK_RATE_RBR;


	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
	} else {
		link_rate = dev_param->stDptxLink.ucLinkRate;
	}

	return link_rate;
}

uint32_t dpv14_api_get_current_link_lane(void)
{
	const struct Dptx_Params *dev_param = (const struct Dptx_Params *)dptx_api_dev_param;
	uint32_t link_lane = (uint32_t)PHY_LANE_0;


	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
	} else {
		link_lane = dev_param->stDptxLink.ucNumOfLanes;
	}

	return link_lane;
}

bool dpv14_api_get_current_ssc_en(void)
{
	const struct Dptx_Params *dev_param = (const struct Dptx_Params *)dptx_api_dev_param;
	bool ssc_en = (bool)false;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
	} else {
		ssc_en = dev_param->stDptxLink.ssc_en;
	}

	return ssc_en;
}

uint8_t dpv14_api_get_pre_emphsis_value(void)
{
	const struct Dptx_Params *dev_param = (const struct Dptx_Params *)dptx_api_dev_param;
	uint8_t pre_emphsis = 0u;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
	} else {
		pre_emphsis = dev_param->stDptxLink.aucPreEmphasis_level[0];
	}
	return pre_emphsis;
}

int32_t dpv14_api_set_pre_emphsis_value(uint8_t pre_emphsis)
{
	struct Dptx_Params *dev_param = dptx_api_dev_param;
	int32_t ret = DPTX_RETURN_NO_ERROR;
	uint8_t prev_pre_emphsis;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
		ret = -DPTX_RETURN_ENODEV;
	} else {
		if (pre_emphsis >= 4u) {
			dptx_err("Out of range - pre-emphasis level = %u", pre_emphsis);
			ret = -DPTX_RETURN_EINVAL;
		} else {
			prev_pre_emphsis = dev_param->stDptxLink.aucPreEmphasis_level[0];
			dev_param->stDptxLink.aucPreEmphasis_level[0] = pre_emphsis;
			dev_param->stDptxLink.aucPreEmphasis_level[1] = pre_emphsis;
			dev_param->stDptxLink.aucPreEmphasis_level[2] = pre_emphsis;
			dev_param->stDptxLink.aucPreEmphasis_level[3] = pre_emphsis;

			ret = dptx_core_set_phy_sigan_quality(dev_param);
			if (DPTX_RETURN_ERROR(ret)) {
				dev_param->stDptxLink.aucPreEmphasis_level[0] = prev_pre_emphsis;
				dev_param->stDptxLink.aucPreEmphasis_level[1] = prev_pre_emphsis;
				dev_param->stDptxLink.aucPreEmphasis_level[2] = prev_pre_emphsis;
				dev_param->stDptxLink.aucPreEmphasis_level[3] = prev_pre_emphsis;
			}
		}
	}
	return ret;
}

uint8_t dpv14_api_get_voltage_swing_value(void)
{
	const struct Dptx_Params *dev_param = (const struct Dptx_Params *)dptx_api_dev_param;
	uint8_t vsl = 0u;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
	} else {
		vsl = dev_param->stDptxLink.aucVoltageSwing_level[0];
	}
	return vsl;
}

int32_t dpv14_api_set_voltage_swing_value(uint8_t vsl)
{
	struct Dptx_Params *dev_param = dptx_api_dev_param;
	int32_t ret = DPTX_RETURN_NO_ERROR;
	uint8_t prev_vsl;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
		ret = -DPTX_RETURN_ENODEV;
	} else {
		if (vsl >= 4u) {
			dptx_err("Out of range - voltage swing level = %u", vsl);
			ret = -DPTX_RETURN_EINVAL;
		} else {
			prev_vsl = dev_param->stDptxLink.aucVoltageSwing_level[0];
			dev_param->stDptxLink.aucVoltageSwing_level[0] = vsl;
			dev_param->stDptxLink.aucVoltageSwing_level[1] = vsl;
			dev_param->stDptxLink.aucVoltageSwing_level[2] = vsl;
			dev_param->stDptxLink.aucVoltageSwing_level[3] = vsl;

			ret = dptx_core_set_phy_sigan_quality(dev_param);
			if (DPTX_RETURN_ERROR(ret)) {
				dev_param->stDptxLink.aucVoltageSwing_level[0] = prev_vsl;
				dev_param->stDptxLink.aucVoltageSwing_level[1] = prev_vsl;
				dev_param->stDptxLink.aucVoltageSwing_level[2] = prev_vsl;
				dev_param->stDptxLink.aucVoltageSwing_level[3] = prev_vsl;
			}
		}
	}
	return ret;
}

uint32_t dpv14_api_get_main_eq_value(void)
{
	struct Dptx_Params *dev_param = dptx_api_dev_param;
	uint32_t main_eq = 0u;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
	} else {
		if (dev_param->ePhy_Dev == PHY_DEVICE_SEC) {
			main_eq = dptx_sec_get_main_equalization(dev_param, 0);
		} else {
			main_eq = dptx_cfg_get_main_equalization(dev_param,
								 dev_param->stDptxLink.ucLinkRate);
		}
	}
	return main_eq;
}

int32_t dpv14_api_set_main_eq_value(uint8_t main_eq)
{
	struct Dptx_Params *dev_param = dptx_api_dev_param;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
		ret = -DPTX_RETURN_ENODEV;
	} else {
		if (main_eq >  DPTX_EQ_MAIN_MAX) {
			dptx_err("Out of range - Main Equalization = %u", main_eq);
			ret = -DPTX_RETURN_EINVAL;
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			if (dev_param->ePhy_Dev == PHY_DEVICE_SEC) {
				ret = dptx_sec_set_main_equalization(dev_param, 0u, main_eq);
				if (DPTX_RETURN_SUCCESS(ret)) {
					ret = dptx_sec_set_main_equalization(dev_param, 1u, main_eq);
				}
				if (DPTX_RETURN_SUCCESS(ret)) {
					ret = dptx_sec_set_main_equalization(dev_param, 2u, main_eq);
				}
				if (DPTX_RETURN_SUCCESS(ret)) {
					ret = dptx_sec_set_main_equalization(dev_param, 3u, main_eq);
				}
			} else {
				ret = dptx_cfg_set_main_equalization(dev_param,
								     dev_param->stDptxLink.ucLinkRate, main_eq);
			}
		}
	}
	return ret;
}

uint32_t dpv14_api_get_post_eq_value(void)
{
	struct Dptx_Params *dev_param = dptx_api_dev_param;
	uint32_t post_eq = 0u;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
	} else {
		if (dev_param->ePhy_Dev == PHY_DEVICE_SEC) {
			post_eq = dptx_sec_get_post_equalization(dev_param, 0);
		} else {
			post_eq = dptx_cfg_get_post_equalization(dev_param, dev_param->stDptxLink.ucLinkRate);
		}
	}
	return post_eq;
}

int32_t dpv14_api_set_post_eq_value(uint8_t post_eq)
{
	struct Dptx_Params *dev_param = dptx_api_dev_param;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
		ret = -DPTX_RETURN_ENODEV;
	} else {
		if (post_eq >  DPTX_EQ_POST_MAX) {
			dptx_err("Out of range - Main Equalization = %u", post_eq);
			ret = -DPTX_RETURN_EINVAL;
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			if (dev_param->ePhy_Dev == PHY_DEVICE_SEC) {
				ret = dptx_sec_set_post_equalization(dev_param, 0u, post_eq);
				if (DPTX_RETURN_SUCCESS(ret)) {
					ret = dptx_sec_set_post_equalization(dev_param, 1u, post_eq);
				}
				if (DPTX_RETURN_SUCCESS(ret)) {
					ret = dptx_sec_set_post_equalization(dev_param, 2u, post_eq);
				}
				if (DPTX_RETURN_SUCCESS(ret)) {
					ret = dptx_sec_set_post_equalization(dev_param, 3u, post_eq);
				}
			} else {
				ret = dptx_cfg_set_post_equalization(dev_param, dev_param->stDptxLink.ucLinkRate, post_eq);
			}
		}
	}
	return ret;
}

int32_t dpv14_api_get_main_eq_table(uint32_t main_eq[16], uint32_t link_rate)
{
	const struct Dptx_Params *dev_param = (const struct Dptx_Params *)dptx_api_dev_param;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
	} else {
		if (link_rate > (uint32_t)LINK_RATE_HBR3) {
			dptx_err("link rate <%u> is out of range", link_rate);
			ret = -DPTX_RETURN_EINVAL;
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			(void)memcpy(main_eq,
				     dev_param->hw_config.phy_eq[link_rate].main_eq,
				     16u * sizeof(uint32_t));
		}
	}
	return ret;
}

int32_t dpv14_api_set_main_eq_table(uint32_t main_eq[16], uint32_t link_rate)
{
	struct Dptx_Params *dev_param = dptx_api_dev_param;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	uint32_t loop;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
		ret = -DPTX_RETURN_ENODEV;
	} else {
		if (link_rate > (uint32_t)LINK_RATE_HBR3) {
			dptx_err("link rate <%u> is out of range", link_rate);
			ret = -DPTX_RETURN_EINVAL;
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			for (loop = 0u; loop < 16u; loop++) {
				switch (loop) {
				case 7u:
				case 10u:
				case 11u:
				case 13u:
				case 14u:
				case 15u:
				/**
				 * These values represent combinations of
				 * Voltage Swing (VSW) and Pre-Emphasis (PRE)
				 * that are invalid according to DisplayPort
				 * specifications. Invalid settings are
				 * marked as `0xFF` in the Post-EQ table.
				 *
				 * The array layout represents the following
				 * combinations:
				 *
				 * - VSW 0 PRE 0, VSW 0 PRE 1, ..., VSW 0 PRE 3
				 * - VSW 1 PRE 0, VSW 1 PRE 1, ..., VSW 1 PRE 3
				 * - ...
				 * - VSW 3 PRE 0, VSW 3 PRE 1, ..., VSW 3 PRE 3
				 *
				 * The loop index sequentially maps to these
				 * combinations.
				 * Invalid EQ configurations are as follows:
				 * - Index 7  (VSW 1 PRE 3)
				 * - Index 10 (VSW 2 PRE 2)
				 * - Index 11 (VSW 2 PRE 3)
				 * - Index 13 (VSW 3 PRE 1)
				 * - Index 14 (VSW 3 PRE 2)
				 * - Index 15 (VSW 3 PRE 3)
				 */
					main_eq[loop] = 0xFFu;
					break;
				default:
					if (main_eq[loop] > DPTX_EQ_MAIN_MAX) {
						dptx_err("main eq[%u] <%u> is out of range", loop, main_eq[loop]);
						ret = -DPTX_RETURN_EINVAL;
					}
					break;
				}
				if (DPTX_RETURN_ERROR(ret)) {
					break;
				}
			}
			if (DPTX_RETURN_SUCCESS(ret)) {
				(void)memcpy(dev_param->hw_config.phy_eq[link_rate].main_eq,
					main_eq, 16u * sizeof(uint32_t));
			}
		}
	}
	return ret;
}

int32_t dpv14_api_get_post_eq_table(uint32_t post_eq[16], uint32_t link_rate)
{
	const struct Dptx_Params *dev_param = (const struct Dptx_Params *)dptx_api_dev_param;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
	} else {
		if (link_rate > (uint32_t)LINK_RATE_HBR3) {
			dptx_err("link rate <%u> is out of range", link_rate);
			ret = -DPTX_RETURN_EINVAL;
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			(void)memcpy(post_eq,
				     dev_param->hw_config.phy_eq[link_rate].post_eq,
				     16u * sizeof(uint32_t));
		}
	}
	return ret;
}

int32_t dpv14_api_set_post_eq_table(uint32_t post_eq[16], uint32_t link_rate)
{
	struct Dptx_Params *dev_param = dptx_api_dev_param;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	uint32_t loop;

	if (dev_param == NULL) {
		dptx_err("Failed to get handle");
		ret = -DPTX_RETURN_ENODEV;
	} else {
		if (link_rate > (uint32_t)LINK_RATE_HBR3) {
			dptx_err("link rate <%u> is out of range", link_rate);
			ret = -DPTX_RETURN_EINVAL;
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			for (loop = 0u; loop < 16u; loop++) {
				switch (loop) {
				case 7u:
				case 10u:
				case 11u:
				case 13u:
				case 14u:
				case 15u:
				/**
				 * These values represent combinations of
				 * Voltage Swing (VSW) and Pre-Emphasis (PRE)
				 * that are invalid according to DisplayPort
				 * specifications. Invalid settings are
				 * marked as `0xFF` in the Post-EQ table.
				 *
				 * The array layout represents the following
				 * combinations:
				 *
				 * - VSW 0 PRE 0, VSW 0 PRE 1, ..., VSW 0 PRE 3
				 * - VSW 1 PRE 0, VSW 1 PRE 1, ..., VSW 1 PRE 3
				 * - ...
				 * - VSW 3 PRE 0, VSW 3 PRE 1, ..., VSW 3 PRE 3
				 *
				 * The loop index sequentially maps to these
				 * combinations.
				 * Invalid EQ configurations are as follows:
				 * - Index 7  (VSW 1 PRE 3)
				 * - Index 10 (VSW 2 PRE 2)
				 * - Index 11 (VSW 2 PRE 3)
				 * - Index 13 (VSW 3 PRE 1)
				 * - Index 14 (VSW 3 PRE 2)
				 * - Index 15 (VSW 3 PRE 3)
				 */
					post_eq[loop] = 0xFFu;
					break;
				default:
					if (post_eq[loop] > DPTX_EQ_MAIN_MAX) {
						dptx_err("post eq[%u] <%u> is out of range", loop, post_eq[loop]);
						ret = -DPTX_RETURN_EINVAL;
					}
					break;
				}
				if (DPTX_RETURN_ERROR(ret)) {
					break;
				}
			}
			if (DPTX_RETURN_SUCCESS(ret)) {
				(void)memcpy(dev_param->hw_config.phy_eq[link_rate].post_eq,
					post_eq, 16u * sizeof(uint32_t));
			}
		}
	}
	return ret;
}
