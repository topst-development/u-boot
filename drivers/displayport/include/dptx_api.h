// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/

#ifndef DPTX_APPLICATION_INTERFACE_H
#define DPTX_APPLICATION_INTERFACE_H

#include <linux/compat.h>
#include <telechips/dpv14_ctrl.h>

#define DPTX_API_RETURN_MST_ACT_TIMEOUT		1001

enum DPTX_LINK_RATE {
	DPTX_LINK_RATE_RBR = 0,
	DPTX_LINK_RATE_HBR = 1,
	DPTX_LINK_RATE_HBR2 = 2,
	DPTX_LINK_RATE_HBR3 = 3,
	DPTX_LINK_RATE_MAX = 4
};

enum DPTX_LINK_LANE {
	DPTX_PHY_LANE_NUM_1 = 1,
	DPTX_PHY_LANE_NUM_2 = 2,
	DPTX_PHY_LANE_NUM_4 = 4,
};

enum DPTX_PRE_EMPHASIS_LEVEL {
	DPTX_PRE_EMPHASIS_LEVEL_0 = 0,
	DPTX_PRE_EMPHASIS_LEVEL_1 = 1,
	DPTX_PRE_EMPHASIS_LEVEL_2 = 2,
	DPTX_PRE_EMPHASIS_LEVEL_3 = 3,
	DPTX_PRE_EMPHASIS_LEVEL_INVALID = 0xFF
};

enum DPTX_VOLTAGE_SWING_LEVEL {
	DPTX_VOLTAGE_SWING_LEVEL_0 = 0,
	DPTX_VOLTAGE_SWING_LEVEL_1 = 1,
	DPTX_VOLTAGE_SWING_LEVEL_2 = 2,
	DPTX_VOLTAGE_SWING_LEVEL_3 = 3,
	DPTX_VOLTAGE_SWING_LEVEL_INVALID = 0xFF
};

enum DPTX_TRAINING_PATTERN_TYPE {
	DPTX_TRAINING_CTRL_TPS_NONE = 0x00,
	DPTX_TRAINING_CTRL_TPS_1_D102 = 0x01,
	DPTX_TRAINING_CTRL_TPS_2 = 0x02,
	DPTX_TRAINING_CTRL_TPS_3 = 0x03,
	DPTX_TRAINING_CTRL_TPS_4 = 0x04,
	DPTX_TRAINING_CTRL_TPS_SYM_ERM = 0x05,
	DPTX_TRAINING_CTRL_TPS_PRBS7 = 0x06,
	DPTX_TRAINING_CTRL_TPS_CUSTOM80 = 0x07,
	DPTX_TRAINING_CTRL_TPS_CP2520_1 = 0x08,
	DPTX_TRAINING_CTRL_TPS_CP2520_2 = 0x09,
	DPTX_TRAINING_CTRL_TPS_INVALID = 0xFF,
};

enum DPTX_INPUT_STREAM_INDEX {
	DPTX_INPUT_STREAM_0 = 0,
	DPTX_INPUT_STREAM_1 = 1,
	DPTX_INPUT_STREAM_2 = 2,
	DPTX_INPUT_STREAM_3 = 3,
	DPTX_INPUT_STREAM_MAX	= 4
};

enum DPTX_VIDEO_ENCODING_TYPE {
	VIDEO_ENCODING_RGB = 0,
	VIDEO_ENCODING_YCBCR422 = 1,
	VIDEO_ENCODING_YCBCR444 = 2,
	VIDEO_ENCODING_MAX = 3
};

enum DPTX_DTD_FORMAT_TYPE {
	DTD_FORMAT_CEA_861 = 0,/* CEA-861-F */
	DTD_FORMAT_VESA_CVT = 1,/* VESA CVT */
	DTD_FORMAT_VESA_DMT = 2 /* VESA DMT */
};

enum DPTX_DTD_REFRESH_RATE {
	DTD_REFRESH_RATE_INVALID	= 0,
	DTD_REFRESH_RATE_49920 = 49920,
	DTD_REFRESH_RATE_50080 = 50080,
	DTD_REFRESH_RATE_59940 = 59940,
	DTD_REFRESH_RATE_60000 = 60000,
	DTD_REFRESH_RATE_60054 = 60054
};

struct   DPTX_Dtd_Params_t {
	u8 interlaced;/* 1 : interlaced, 0 : progressive */
	u8 h_sync_polarity;
	u8 v_sync_polarity;
	u16 pixel_repetition_input;
	u16 h_active;
	u16 h_blanking;
	u16 h_image_size;
	u16 h_sync_offset;
	u16 h_sync_pulse_width;
	u16 v_active;
	u16 v_blanking;
	u16 v_image_size;
	u16 v_sync_offset;
	u16 v_sync_pulse_width;
	u32 uiPixel_Clock;
};

int32_t Dpv14_Tx_API_Init(const struct dpv14_drv_params *drv_params);
int32_t Dpv14_Tx_API_Deinit(void);
int32_t Dpv14_Tx_API_Start(uint8_t num_of_dps, const uint32_t pixel_clocks[DPTX_INPUT_STREAM_MAX]);
int32_t Dpv14_Tx_API_Stop(void);
int32_t Dpv14_Tx_API_Set_PW(void);
int32_t Dpv14_Tx_API_Set_CfgLock(bool request_access);
int32_t Dpv14_Tx_API_Set_CfgAccess(bool request_access);
int32_t Dpv14_Tx_API_Set_MaxLinkRate_Supported(enum DPTX_LINK_RATE link_rate);
int32_t Dpv14_Tx_API_Set_MaxLinkLane_Supported(enum DPTX_LINK_LANE link_lane);
int32_t dpv14_tx_api_set_spread_specturm_clock(bool ssc_en);
int32_t Dpv14_Tx_API_Perform_HPD_WorkingFlow(void);
int32_t Dpv14_Tx_API_Write_RegisterBank_Value(const uint32_t *reg_data,
					      uint32_t reg_offset,
					      uint8_t data_cnt);
int32_t Dpv14_Tx_API_Read_RegisterBank_Value(uint32_t *reg_data,
					     uint32_t reg_offset,
					     uint8_t data_cnt);
int32_t Dpv14_Tx_API_Get_PLL_Status(uint8_t *pll_locked);
int32_t Dpv14_Tx_API_Set_CTS(bool ssc_enable,
			     uint32_t pattern_type,
			     enum DPTX_LINK_RATE link_rate,
			     enum DPTX_PRE_EMPHASIS_LEVEL pre_emphasis,
			     enum DPTX_VOLTAGE_SWING_LEVEL voltage_swing);

int32_t Dpv14_Tx_API_Get_Dtd_From_VideoCode(
			uint32_t video_code,
			struct DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
			uint32_t video_refresh_rate,
			uint8_t video_format_standard);
int32_t Dpv14_Tx_API_Get_Dtd_From_Edid(struct DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
				       uint8_t dp_stream_id);

uint32_t dpv14_api_get_current_link_rate(void);
uint32_t dpv14_api_get_current_link_lane(void);
bool dpv14_api_get_current_ssc_en(void);
uint8_t dpv14_api_get_pre_emphsis_value(void);
int32_t dpv14_api_set_pre_emphsis_value(uint8_t pre_emphsis);
uint8_t dpv14_api_get_voltage_swing_value(void);
int32_t dpv14_api_set_voltage_swing_value(uint8_t vsl);
uint32_t dpv14_api_get_main_eq_value(void);
int32_t dpv14_api_set_main_eq_value(uint8_t main_eq);
uint32_t dpv14_api_get_post_eq_value(void);
int32_t dpv14_api_set_post_eq_value(uint8_t post_eq);
int32_t dpv14_api_get_main_eq_table(uint32_t main_eq[16], uint32_t link_rate);
int32_t dpv14_api_set_main_eq_table(uint32_t main_eq[16], uint32_t link_rate);
int32_t dpv14_api_get_post_eq_table(uint32_t post_eq[16], uint32_t link_rate);
int32_t dpv14_api_set_post_eq_table(uint32_t post_eq[16], uint32_t link_rate);
#endif /* DPTX_APPLICATION_INTERFACE_H  */
