// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/

#ifndef DPTX_API_H
#define DPTX_API_H

#include <linux/compat.h>

#define DPTX_API_RETURN_I2C_OVER_AUX_NO_ACK 1000
#define DPTX_API_RETURN_MST_ACT_TIMEOUT			 1001

enum DPTX_LINK_RATE {
	LINK_RATE_RBR = 0,
	LINK_RATE_HBR = 1,
	LINK_RATE_HBR2 = 2,
	LINK_RATE_HBR3 = 3,
	LINK_RATE_MAX = 4
};

enum DPTX_LINK_LANE {
	DPTX_PHY_LANE_1		= 1,
	DPTX_PHY_LANE_2		= 2,
	DPTX_PHY_LANE_4		= 4,
	DPTX_PHY_LANE_MAX	= 5
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
	DPTX_TRAINING_CTRL_TPS_2	 = 0x02,
	DPTX_TRAINING_CTRL_TPS_3	 = 0x03,
	DPTX_TRAINING_CTRL_TPS_4	 = 0x04,
	DPTX_TRAINING_CTRL_TPS_SYM_ERM = 0x05,
	DPTX_TRAINING_CTRL_TPS_PRBS7     = 0x06,
	DPTX_TRAINING_CTRL_TPS_CUSTOM80 = 0x07,
	DPTX_TRAINING_CTRL_TPS_CP2520_1 = 0x08,
	DPTX_TRAINING_CTRL_TPS_CP2520_2 = 0x09,
	DPTX_TRAINING_CTRL_TPS_INVALID = 0xFF,
};

enum DPTX_INPUT_STREAM_INDEX {
	DPTX_INPUT_STREAM_0		= 0,
	DPTX_INPUT_STREAM_1		= 1,
	DPTX_INPUT_STREAM_2		= 2,
	DPTX_INPUT_STREAM_3		= 3,
	DPTX_INPUT_STREAM_MAX	= 4
};

enum DPTX_VIDEO_ENCODING_TYPE {
	VIDEO_ENCODING_RGB = 0,
	VIDEO_ENCODING_YCBCR422 = 1,
	VIDEO_ENCODING_YCBCR444 = 2,
	VIDEO_ENCODING_MAX = 3
};

enum DPTX_DTD_FORMAT_TYPE {
	DTD_FORMAT_CEA_861		= 0,/* CEA-861-F */
	DTD_FORMAT_VESA_CVT		= 1,/* VESA CVT */
	DTD_FORMAT_VESA_DMT		= 2 /* VESA DMT */
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
	u8	interlaced;/* 1 : interlaced, 0 : progressive */
	u8	h_sync_polarity;
	u8	v_sync_polarity;
	u16	pixel_repetition_input;
	u16	h_active;
	u16	h_blanking;
	u16	h_image_size;
	u16	h_sync_offset;
	u16	h_sync_pulse_width;
	u16	v_active;
	u16	v_blanking;
	u16	v_image_size;
	u16	v_sync_offset;
	u16	v_sync_pulse_width;
	u32	uiPixel_Clock;
};


void Dpv14_Tx_API_Release_ColdResetMask(void);
void Dpv14_Tx_API_Config_RegisterAccess_Mode(bool bAP_Accessible);
void Dpv14_Tx_API_Config_PW(void);
void Dpv14_Tx_API_Config_CfgAccess(bool bAccessible);
void Dpv14_Tx_API_Config_CfgLock(bool bAccessible);
void Dpv14_Tx_API_Config_PHY_StandardLane_PinConfig(void);
void Dpv14_Tx_API_Config_SDM_BypassControl(bool bSDM_Bypass);
void Dpv14_Tx_API_Config_SRVC_BypassControl(bool bSRVC_Bypass);
void Dpv14_Tx_API_Config_MuxSelect(uint32_t uiMux_Index, uint8_t ucDP_PortIndex);
void Dpv14_Tx_API_Reset_PLL_Blk(void);

int32_t Dpv14_Tx_API_Init(
			uint8_t ucNumOfStreams,
					enum DPTX_LINK_RATE eLinkRate,
					enum DPTX_LINK_LANE	 eLinkLanes);
int32_t Dpv14_Tx_API_Deinit(void);
int32_t Dpv14_Tx_API_Start(void);
int32_t Dpv14_Tx_API_Stop(void);
int32_t Dpv14_Tx_API_Set_Video_Code(
			uint8_t ucNumOfStreams,
			uint32_t auiDefaultVideoCode[DPTX_INPUT_STREAM_MAX]);
int32_t Dpv14_Tx_API_Set_Video_PeriPixelClock(
			uint8_t ucNumOfStreams,
			uint32_t auiPeri_PixelClock[DPTX_INPUT_STREAM_MAX]);
int32_t Dpv14_Tx_API_Set_VCPID_MST(
			uint8_t ucNumOfStreams,
			uint8_t aucVCP_Id[DPTX_INPUT_STREAM_MAX]);
int32_t Dpv14_Tx_API_Set_Video_ColorSpace(
			enum DPTX_VIDEO_ENCODING_TYPE eVideoType);
void Dpv14_Tx_API_Set_Audio_Sel(uint32_t uiData);
void Dpv14_Tx_API_Get_Audio_Sel(uint32_t *pucData);
int32_t Dpv14_Tx_API_Set_PanelDisplay_Mode(uint8_t ucPanelDisplay_Mode);
int32_t Dpv14_Tx_API_Set_SideBand_Msg_Supported(
			uint8_t ucSideBand_MSG_Supported);
int32_t Dpv14_Tx_API_Set_MaxLinkRate_Supported(enum DPTX_LINK_RATE eLink_Rate);
int32_t Dpv14_Tx_API_Set_MaxLinkLane_Supported(enum DPTX_LINK_LANE eLink_Lane);
int32_t Dpv14_Tx_API_Perform_HPD_WorkingFlow(void);
int32_t Dpv14_Tx_API_Write_RegisterBank_Value(
			const uint32_t *puiBuffer,
			uint32_t uiOffset,
			uint8_t ucLength);
int32_t Dpv14_Tx_API_Read_RegisterBank_Value(
			uint32_t *puiBuffer,
			uint32_t uiOffset,
			uint8_t ucLength);
int32_t Dpv14_Tx_API_Get_PLL_Status(uint8_t *pucPll_Locked);
int32_t Dpv14_Tx_API_Set_CTS(
			bool bSSC_Enabled,
			uint8_t ucClk_Selection,
			uint32_t uiPattern_Type,
			enum DPTX_LINK_RATE eLink_Rate,
			enum DPTX_PRE_EMPHASIS_LEVEL ePreEmp,
			enum DPTX_VOLTAGE_SWING_LEVEL eVSW);
int32_t Dpv14_Tx_API_Set_EQ(uint32_t uiEQ_Main,
			uint32_t uiEQ_Post,
			uint32_t uiEQ_Pre,
			uint32_t uiEQ_VBoost);
int32_t Dpv14_Tx_API_Set_LinkPrams(uint8_t ucParam, uint8_t ucLevel);
int32_t Dpv14_Tx_API_Get_Dtd_From_VideoCode(
			uint32_t uiVideo_Code,
			struct DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
			uint32_t uiRefreshRate,
			uint8_t ucVideoFormat);
int32_t Dpv14_Tx_API_Get_Dtd_From_PreferredVIC(
			struct DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
			uint8_t ucStream_Index);
int32_t Dpv14_Tx_API_Get_Dtd_From_Edid(
			struct  DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
			uint8_t ucStream_Index);
#endif /* DPTX_API_H  */
