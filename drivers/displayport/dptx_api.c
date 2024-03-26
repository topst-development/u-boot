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

#define MAX_CHECK_HPD_NUM		200

static struct Dptx_Params *pstDpDrv_Handle = NULL;


int32_t Dpv14_Tx_API_Init(struct dptx_api_init_params *pstdptx_api_init_params)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle = NULL;
	struct Dptx_Init_Params stDptx_Init_Params;

	if (pstdptx_api_init_params->ucnum_of_dps > (uint8_t)DPTX_INPUT_STREAM_MAX) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid Num Of Streams as %u", pstdptx_api_init_params->ucnum_of_dps);

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (pstdptx_api_init_params->eMaxRate >= DPTX_LINK_RATE_MAX) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid link rate as %u", (u32)pstdptx_api_init_params->eMaxRate);

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if ((pstdptx_api_init_params->eMaxLanes != DPTX_PHY_LANE_1) &&
		(pstdptx_api_init_params->eMaxLanes != DPTX_PHY_LANE_2) &&
		(pstdptx_api_init_params->eMaxLanes != DPTX_PHY_LANE_4)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid link lane as %u", (u32)pstdptx_api_init_params->eMaxLanes);

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	stDptx_Init_Params.bsideband_msg = pstdptx_api_init_params->bsideband_msg;
	stDptx_Init_Params.bsdm_bypass = pstdptx_api_init_params->bsdm_bypass;
	stDptx_Init_Params.btrvc_bypass = pstdptx_api_init_params->btrvc_bypass;
	stDptx_Init_Params.bphy_lane_swap = pstdptx_api_init_params->bphy_lane_swap;
	stDptx_Init_Params.ucnum_of_dps = pstdptx_api_init_params->ucnum_of_dps;
	stDptx_Init_Params.ucMaxRate = pstdptx_api_init_params->eMaxRate;
	stDptx_Init_Params.ucMaxLanes = pstdptx_api_init_params->eMaxLanes;
	stDptx_Init_Params.ucPixelEn_Type = (uint8_t)pstdptx_api_init_params->eVidEncodingType;
	stDptx_Init_Params.ePhy_Dev_model = (uint8_t)pstdptx_api_init_params->ucphy_model;

	(void)memcpy((void *)stDptx_Init_Params.aucMux_Id, (void *)pstdptx_api_init_params->ucmux_id, sizeof(uint8_t) * DPTX_INPUT_STREAM_MAX);
	(void)memcpy((void *)stDptx_Init_Params.aucVcp_Id, (void *)pstdptx_api_init_params->ucvcp_id, sizeof(uint8_t) * DPTX_INPUT_STREAM_MAX);
	(void)memcpy((void *)stDptx_Init_Params.auiVIC, (void *)pstdptx_api_init_params->uiVIC, sizeof(uint32_t) * DPTX_INPUT_STREAM_MAX);

	iRetVal = Dptx_V14_Init(&stDptx_Init_Params);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	pstHandle = Dptx_V14_Get_Device_Handle();
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	pstDpDrv_Handle = pstHandle;

return_funcs:
	return iRetVal;
}

int32_t Dpv14_Tx_API_Deinit(void)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle = NULL;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Core_Deinit(pstHandle);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_V14_Deinit(pstHandle);

return_funcs:
	return iRetVal;
}

int32_t Dpv14_Tx_API_Start(uint8_t ucNum_Of_Dps, uint32_t auiPixel_Clk[DPTX_INPUT_STREAM_MAX])
{
	int8_t ucHPD_Status;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiHDP_CheckTry;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Vidin_Set_Video_PPClk(pstHandle, ucNum_Of_Dps, auiPixel_Clk);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	for (uiHDP_CheckTry = 0; uiHDP_CheckTry < (uint32_t)MAX_CHECK_HPD_NUM; uiHDP_CheckTry++) {
		iRetVal = Dptx_Intr_Get_HotPlug_Status(pstHandle, &ucHPD_Status);
		if ((iRetVal == DPTX_RETURN_NO_ERROR) && (ucHPD_Status == (int8_t)HPD_STATUS_PLUGGED)) {
			dptx_info("Hot plugged after %ums\n", (uiHDP_CheckTry * 10U));
			break;
		}

		mdelay(10);
	}

	if (uiHDP_CheckTry == (uint32_t)MAX_CHECK_HPD_NUM) {
		dptx_err("Hot unplugged after %ums\n", (uiHDP_CheckTry * 10U));

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Intr_Handle_Hotplug(pstHandle);

return_funcs:
	return iRetVal;
}

int32_t Dpv14_Tx_API_Stop(void)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle = NULL;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	} else  {
		/* For coverity */
		iRetVal = Dptx_Intr_Handle_HotUnplug(pstHandle);
	}

	return iRetVal;
}

int32_t Dpv14_Tx_API_Set_CfgAccess(bool bAccessible)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Protect_Set_CfgAccess(pstHandle, bAccessible);

return_funcs:
	return iRetVal;

}

int32_t Dpv14_Tx_API_Set_CfgLock(bool bAccessible)
{
	bool bLocked;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	bLocked = bAccessible ? (bool)false : (bool)true;

	iRetVal = Dptx_Protect_Set_CfgLock(pstHandle, bLocked);

return_funcs:
	return iRetVal;
}

int32_t Dpv14_Tx_API_Set_MaxLinkRate_Supported(enum DPTX_LINK_RATE eLink_Rate)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (eLink_Rate >= DPTX_LINK_RATE_MAX) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid link rate %u", (uint32_t)eLink_Rate);

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	pstHandle->ucMax_Rate = (uint8_t)eLink_Rate;

return_funcs:
	return iRetVal;
}

int32_t Dpv14_Tx_API_Set_MaxLinkLane_Supported(enum DPTX_LINK_LANE eLink_Lane)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (eLink_Lane >= DPTX_PHY_LANE_MAX) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid link lane %u", (uint32_t)eLink_Lane);

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	pstHandle->ucMax_Lanes = (uint8_t)eLink_Lane;

return_funcs:
	return iRetVal;
}

int32_t Dpv14_Tx_API_Perform_HPD_WorkingFlow(void)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Intr_Handle_HotUnplug(pstHandle);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Intr_Handle_Hotplug(pstHandle);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

return_funcs:
	return iRetVal;
}

int32_t Dpv14_Tx_API_Get_Dtd_From_VideoCode(
			uint32_t uiVideo_Code,
			struct DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
			uint32_t uiRefreshRate,
			uint8_t ucVideoFormat)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Dtd_Params stDtd;

	if (pstDptx_Dtd_Params == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Ptr. of Dtd params is NULL");

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Vidin_Fill_Dtd(&stDtd,
									uiVideo_Code,
									uiRefreshRate,
									ucVideoFormat);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

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

return_funcs:
	return iRetVal;
}

int32_t Dpv14_Tx_API_Get_Dtd_From_Edid(
			struct DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
			uint8_t ucStream_Index)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (ucStream_Index >= (uint8_t)DPTX_INPUT_STREAM_MAX) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid DP index %u", ucStream_Index);

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Intr_Handle_Edid(pstHandle, ucStream_Index);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	(void)memcpy(pstDptx_Dtd_Params,
					&pstHandle->stVideoParams.stDtdParams[ucStream_Index],
					sizeof(struct  DPTX_Dtd_Params_t));

return_funcs:
	return iRetVal;
}

int32_t Dpv14_Tx_API_Write_RegisterBank_Value(
			const uint32_t *puiBuffer,
			uint32_t uiOffset,
			uint8_t ucLength)
{
	uint8_t ucCount;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiRegOffset, uiOneRegOffset;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (puiBuffer == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid parameter as puiBuffer is NULL");

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (ucLength == 0U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid parameter as Length is 0");

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (uiOffset > (uint32_t)DP_MAX_OFFSET) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid offset as 0x%x", uiOffset);

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (pstHandle->uiRegBank_RegAddr_Offset > (uint32_t)DP_MAX_OFFSET) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid RegBank offset as 0x%x",
					pstHandle->uiRegBank_RegAddr_Offset);

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	for (ucCount = 0; ucCount < ucLength; ucCount++) {
		uiRegOffset = (pstHandle->uiRegBank_RegAddr_Offset + uiOffset);
		uiOneRegOffset = (uint32_t)(sizeof(uint32_t) * ucCount);

		uiRegOffset += uiOneRegOffset;

		Dptx_Reg_Writel(pstHandle, uiRegOffset, puiBuffer[ucCount]);
	}

return_funcs:
	return iRetVal;
}

int32_t Dpv14_Tx_API_Read_RegisterBank_Value(
			uint32_t *puiBuffer,
			uint32_t uiOffset,
			uint8_t ucLength)
{
	uint8_t ucCount;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiRegOffset, uiOneRegOffset;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (puiBuffer == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid parameter as puiBuffer is NULL");

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (ucLength == 0U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid parameter as Length is 0");

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (uiOffset > (uint32_t)DP_MAX_OFFSET) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid offset as 0x%x", uiOffset);

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (pstHandle->uiRegBank_RegAddr_Offset > (uint32_t)DP_MAX_OFFSET) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid RegBank offset as 0x%x",
					pstHandle->uiRegBank_RegAddr_Offset);

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	for (ucCount = 0; ucCount < ucLength; ucCount++) {
		uiRegOffset = (pstHandle->uiRegBank_RegAddr_Offset + uiOffset);
		uiOneRegOffset = (uint32_t)(sizeof(uint32_t) * ucCount);

		uiRegOffset = (uiRegOffset + uiOneRegOffset);

		puiBuffer[ucCount] = Dptx_Reg_Readl(pstHandle, uiRegOffset);
	}

return_funcs:
	return iRetVal;
}

int32_t Dpv14_Tx_API_Get_PLL_Status(uint8_t *pucPll_Locked)
{
	uint8_t ucPllLock = 0;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;
	} else  {
		Dptx_Clk_Get_PLLLock_Status(pstHandle, &ucPllLock);

		*pucPll_Locked = ucPllLock;
	}

	return iRetVal;
}

static int32_t dpv14_tx_api_cts_reset_pattern(struct Dptx_Params *pHandle,
																uint32_t uiPattern)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;

	if (uiPattern == (uint32_t)DPTX_TRAINING_CTRL_TPS_CUSTOM80) {
		Dptx_Reg_Writel(pHandle, DPTX_CUSTOMPAT0, 0x3E0F83E0);
		Dptx_Reg_Writel(pHandle, DPTX_CUSTOMPAT1, 0x3E0F83E0);
		Dptx_Reg_Writel(pHandle, DPTX_CUSTOMPAT2, 0x3E0F83E0);
	}

	iRetVal = Dptx_Core_Set_PHY_Pattern(pHandle, (uint32_t)uiPattern);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Core_Enable_PHY_XMIT(pHandle, (uint32_t)DPTX_PHY_LANE_4);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

return_funcs:
	return iRetVal;
}

static int32_t dpv14_tx_api_cts_set__preemp_vsw(
								struct Dptx_Params *pHandle,
								enum DPTX_PRE_EMPHASIS_LEVEL ePreEmp,
								enum DPTX_VOLTAGE_SWING_LEVEL eVSW)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiLane_Index;

	for (uiLane_Index = 0; uiLane_Index < (uint32_t)DPTX_PHY_LANE_4; uiLane_Index++) {
		(void)Dptx_Core_Set_PHY_PreEmphasis(
							pHandle,
							uiLane_Index,
							(enum PHY_PRE_EMPHASIS_LEVEL)ePreEmp);

		(void)Dptx_Core_Set_PHY_VSW(
							pHandle,
							uiLane_Index,
							(enum PHY_VOLTAGE_SWING_LEVEL)eVSW);
	}

	return iRetVal;
}

static int32_t dpv14_tx_api_cts_set_ssc_rate(struct Dptx_Params *pHandle,
											bool bSSC_En,
											enum DPTX_LINK_RATE eLink_Rate)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiRegMap_PhyIfCtrl;

	iRetVal = Dptx_Core_Set_PHY_PowerState(pHandle,
										PHY_POWER_DOWN_PHY_CLOCK);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Core_Get_PHY_BUSY_Status(pHandle, (u8)DPTX_PHY_LANE_4);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("From Dptx_Core_Get_PHY_BUSY_Status()");
	}

	uiRegMap_PhyIfCtrl = Dptx_Reg_Readl(pHandle, DPTX_PHYIF_CTRL);
	if (bSSC_En) {
		/* For coverity */
		uiRegMap_PhyIfCtrl = (uiRegMap_PhyIfCtrl & ~((uint32_t)DPTX_PHYIF_CTRL_SSC_DIS));
	} else {
		/* For coverity */
		uiRegMap_PhyIfCtrl = (uiRegMap_PhyIfCtrl | (uint32_t)DPTX_PHYIF_CTRL_SSC_DIS);
	}

	Dptx_Reg_Writel(pHandle, DPTX_PHYIF_CTRL, uiRegMap_PhyIfCtrl);

	iRetVal = Dptx_Core_Set_PHY_Rate(pHandle, (enum PHY_LINK_RATE)eLink_Rate);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Core_Set_PHY_PowerState(pHandle, PHY_POWER_ON);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Core_Get_PHY_BUSY_Status(pHandle, (u8)DPTX_PHY_LANE_4);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("From Dptx_Core_Get_PHY_BUSY_Status()");
	}

return_funcs:
	return iRetVal;
}

static int32_t dpv14_tx_api_cts_reset_core(struct Dptx_Params *pHandle,
								bool bSscEn,
								enum DPTX_LINK_RATE eLinkRate,
								enum DPTX_PRE_EMPHASIS_LEVEL ePre_Emp,
								enum DPTX_VOLTAGE_SWING_LEVEL eVsw)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;

	iRetVal = Dptx_Core_Init(pHandle);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Core_Set_PHY_NumOfLanes(pHandle,
										(u8)DPTX_PHY_LANE_4);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = dpv14_tx_api_cts_set_ssc_rate(pHandle,
												bSscEn,
												eLinkRate);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = Dptx_Core_Set_PHY_Pattern(pHandle,
										DPTX_PHYIF_CTRL_TPS_NONE);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = dpv14_tx_api_cts_set__preemp_vsw(pHandle,
													ePre_Emp,
													eVsw);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

return_funcs:
	return iRetVal;
}

static int32_t dpv14_tx_api_cts_reset_regbank(struct Dptx_Params *pHandle,
													uint8_t ucClk_Sel,
													enum DPTX_LINK_RATE eLinkRate)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiRegMap_RegBank_21;

	uiRegMap_RegBank_21 = Dptx_Reg_Readl(pHandle,
				(0x80000 + DP_REGISTER_BANK_REG_21));
				uiRegMap_RegBank_21 &= (0xFFFFFFF3U);
				uiRegMap_RegBank_21 |= (u32)ucClk_Sel;
	Dptx_Reg_Writel(pHandle,
						(0x80000 + DP_REGISTER_BANK_REG_21),
						(u32)uiRegMap_RegBank_21);

	iRetVal = Dptx_Cfg_Init(pHandle, (uint8_t)eLinkRate);

	return iRetVal;
}

int32_t Dpv14_Tx_API_Set_CTS(
			bool bSSC_Enabled,
			uint8_t ucClk_Selection,
			uint32_t uiPattern_Type,
			enum DPTX_LINK_RATE eLink_Rate,
			enum DPTX_PRE_EMPHASIS_LEVEL ePreEmp,
			enum DPTX_VOLTAGE_SWING_LEVEL eVSW)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiRegMap_Cctl;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts("CTS Test => ");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts(" -.Link rate: %s",
				(eLink_Rate == DPTX_LINK_RATE_RBR) ? "RBR" :
				(eLink_Rate == DPTX_LINK_RATE_HBR) ? "HBR" :
				(eLink_Rate == DPTX_LINK_RATE_HBR2) ? "HBR2" : "HBR3");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts(" -.SSC: %s", bSSC_Enabled ? "Enable" : "Disable");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts(" -.Pre Emp: level %u", (uint32_t)ePreEmp);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts(" -.VSW: level %u", (uint32_t)eVSW);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts(" -.Pattern: %s",
		(uiPattern_Type == (uint32_t)DPTX_TRAINING_CTRL_TPS_1_D102) ?
		"D10.2" :
		(uiPattern_Type == (uint32_t)DPTX_TRAINING_CTRL_TPS_PRBS7) ?
		"PRBS7" :
		(uiPattern_Type == (uint32_t)DPTX_TRAINING_CTRL_TPS_CUSTOM80) ?
		"CUSTOM80" :
		(uiPattern_Type == (uint32_t)DPTX_TRAINING_CTRL_TPS_CP2520_1) ?
		"CP2520_P1" :
		(uiPattern_Type == (uint32_t)DPTX_TRAINING_CTRL_TPS_CP2520_2) ?
		"CP2520_P2" : "TPS 4");

	iRetVal = dpv14_tx_api_cts_reset_regbank(pstHandle,
												ucClk_Selection,
												eLink_Rate);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = dpv14_tx_api_cts_reset_core(pstHandle,
											bSSC_Enabled,
											eLink_Rate,
											ePreEmp,
											eVSW);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = dpv14_tx_api_cts_reset_pattern(pstHandle,
											uiPattern_Type);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	uiRegMap_Cctl = Dptx_Reg_Readl(pstHandle, (uint32_t)DPTX_CCTL);
	uiRegMap_Cctl = (uiRegMap_Cctl | (uint32_t)DPTX_CCTL_ENH_FRAME_EN);
	Dptx_Reg_Writel(pstHandle, DPTX_CCTL, uiRegMap_Cctl);

return_funcs:
	return  iRetVal;
}

int32_t Dpv14_Tx_API_Set_EQ(uint32_t uiEQ_Main,
										uint32_t uiEQ_Post,
										uint32_t uiEQ_Pre,
										uint32_t uiEQ_VBoost)
{
	uint8_t ucEQ_VBoost_7 = 0U;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts("PHY EQ Setting => ");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts(" -.Main : %u", uiEQ_Main);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts(" -.Post : %u", uiEQ_Post);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts(" -.Pre : %u", uiEQ_Pre);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts(" -.VBoost : %u", uiEQ_VBoost);

	if (uiEQ_VBoost == 7U) {
		/* For coverity */
		ucEQ_VBoost_7 = 1U;
	}

	iRetVal = Dptx_Cfg_Set_Tx_EQ(pstHandle,
									uiEQ_Main,
									uiEQ_Post,
									uiEQ_Pre,
									ucEQ_VBoost_7);

return_funcs:
	return  iRetVal;
}

int32_t Dpv14_Tx_API_Set_LinkPrams(uint8_t ucParam, uint8_t ucLevel)
{
	uint32_t uiLane_Idx;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = pstDpDrv_Handle;
	if (pstHandle == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Failed to get handle");

		iRetVal = DPTX_RETURN_EACCES;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if ((ucParam == 0U) && (ucLevel > (uint8_t)PRE_EMPHASIS_LEVEL_3)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid Pre-Emp level as %u", ucLevel);

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if ((ucParam == 1U) && (ucLevel > (uint8_t)VOLTAGE_SWING_LEVEL_3)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Invalid VSW level as %u", ucLevel);

		iRetVal = DPTX_RETURN_EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (ucParam == 0U) {
		for (uiLane_Idx = 0; uiLane_Idx < (uint32_t)DPTX_PHY_LANE_4; uiLane_Idx++) {
			iRetVal = Dptx_Core_Set_PHY_PreEmphasis(pstHandle,
							uiLane_Idx,
							(enum PHY_PRE_EMPHASIS_LEVEL)ucLevel);
			if (iRetVal != DPTX_RETURN_NO_ERROR) {
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				dptx_err("from Dptx_Core_Set_PHY_PreEmphasis(): %u", uiLane_Idx);

				/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
				goto return_funcs;
			}
		}
	} else {
		for (uiLane_Idx = 0; uiLane_Idx < (uint32_t)DPTX_PHY_LANE_4; uiLane_Idx++) {
			iRetVal = Dptx_Core_Set_PHY_VSW(pstHandle,
				uiLane_Idx,
				(enum PHY_VOLTAGE_SWING_LEVEL)ucLevel);
			if (iRetVal != DPTX_RETURN_NO_ERROR) {
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				dptx_err("from Dptx_Core_Set_PHY_VSW(): %u", uiLane_Idx);

				/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
				goto return_funcs;
			}
		}
	}

	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts("PHY %s Setting => ", (ucParam == 0U) ? "Pre-Emp":"VSW");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_cts(" -.Level : %u", ucLevel);

return_funcs:
	return  iRetVal;
}

