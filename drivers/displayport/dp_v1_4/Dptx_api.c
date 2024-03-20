// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/

#include <linux/drm_dp_helper.h>
#include <linux/delay.h>

#include "Dptx_api.h"
#include "Dptx_v14.h"
#include "Dptx_reg.h"
#include "Dptx_dbg.h"
#include "Dptx_drm_dp_addition.h"

#define MAX_CHECK_HPD_NUM		200

void Dpv14_Tx_API_Release_ColdResetMask(void)
{
	uint32_t uiRegAddr;
	uint32_t uiRegMap_R_HsmRstn_Msk = 0, uiRegMap_W_HsmRstn_Msk = 0;

	uiRegAddr = (uint32_t)HSM_RSTN_MSK;

	uiRegMap_R_HsmRstn_Msk = Dptx_Reg_Direct_Read(uiRegAddr);

	if ((uiRegMap_R_HsmRstn_Msk & (uint32_t)COLD_RSTN_MSK) != 0U) {
		uiRegMap_W_HsmRstn_Msk =
				(uiRegMap_R_HsmRstn_Msk & ~((uint32_t)COLD_RSTN_MSK));
		Dptx_Reg_Direct_Write(uiRegAddr,
								uiRegMap_W_HsmRstn_Msk);

		dptx_dbg("DP Cold reset mask: 0x%x -> 0x%x",
					uiRegMap_R_HsmRstn_Msk,
					uiRegMap_W_HsmRstn_Msk);
	}
}

void Dpv14_Tx_API_Config_RegisterAccess_Mode(bool bAP_Accessible)
{
	uint32_t uiRegAddr;
	uint32_t uiRegMap_R_APBSel = 0, uiRegMap_W_APBSel = 0;

	uiRegAddr = (uint32_t)DPTX_APB_SEL_REG;

	uiRegMap_R_APBSel = Dptx_Reg_Direct_Read(uiRegAddr);

	if (bAP_Accessible == (bool)true) {
		/* For coverity */
		uiRegMap_W_APBSel = (uiRegMap_R_APBSel | (uint32_t)DPTX_APB_SEL_MASK);
	} else {
		/* For coverity */
		uiRegMap_W_APBSel = (uiRegMap_R_APBSel & ~((uint32_t)DPTX_APB_SEL_MASK));
	}

	Dptx_Reg_Direct_Write(uiRegAddr, uiRegMap_W_APBSel);

	dptx_dbg("APB Sel to %s...Reg[0x%08x]: 0x%08x -> 0x%08x\n",
				bAP_Accessible ? "AP":"Micom",
				uiRegAddr,
				uiRegMap_R_APBSel,
				uiRegMap_W_APBSel);
}

void Dpv14_Tx_API_Config_PW(void)
{
	uint32_t uiRegAddr;
	uint32_t uiProtect_Cfg_PW;

	uiRegAddr = (uint32_t)(
				(uint32_t)DP_DDIBUS_BASE_REG_ADDRESS +
				(uint32_t)DP_PROTECT_OFFSET +
				(uint32_t)DP_PORTECT_CFG_PW_OK);

	uiProtect_Cfg_PW = 0x8050ACE5U;

	Dptx_Reg_Direct_Write(uiRegAddr, uiProtect_Cfg_PW);

	dptx_dbg("RegisterBank_PW to 0x%x", uiProtect_Cfg_PW);
}

void Dpv14_Tx_API_Config_CfgAccess(bool bAccessible)
{
	uint32_t uiRegAddr;

	uiRegAddr = (uint32_t)(
				(uint32_t)DP_DDIBUS_BASE_REG_ADDRESS +
				(uint32_t)DP_PROTECT_OFFSET +
				(uint32_t)DP_PORTECT_CFG_ACCESS);

	if (bAccessible == (bool)true) {
		/* For coverity */
		Dptx_Reg_Direct_Write(
				uiRegAddr,
				(uint32_t)DP_PORTECT_CFG_ACCESSABLE);
	} else {
		/* For coverity */
		Dptx_Reg_Direct_Write(
				uiRegAddr,
				(uint32_t)DP_PORTECT_CFG_NOT_ACCESSABLE);
	}

	dptx_dbg("Cfg access - bAccessable %d", (u32)bAccessible);
}

void Dpv14_Tx_API_Config_CfgLock(bool bAccessible)
{
	uint32_t uiRegAddr;

	uiRegAddr = (uint32_t)(
				(uint32_t)DP_DDIBUS_BASE_REG_ADDRESS +
				(uint32_t)DP_PROTECT_OFFSET +
				(uint32_t)DP_PORTECT_CFG_LOCK);

	if (bAccessible == (bool)true) {
		/* For coverity */
		Dptx_Reg_Direct_Write(
				uiRegAddr,
				(uint32_t)DP_PORTECT_CFG_UNLOCKED);
	} else {
		/* For coverity */
		Dptx_Reg_Direct_Write(
				uiRegAddr,
				(uint32_t)DP_PORTECT_CFG_LOCKED);
	}

	dptx_dbg("RegisterBank_CfgLock - bAccessable %d", (u32)bAccessible);
}

void Dpv14_Tx_API_Config_PHY_StandardLane_PinConfig(void)
{
	uint32_t uiRegAddr;
	uint32_t uiRegMap_R_StdEn, uiRegMap_W_StdEn;

	uiRegAddr = (uint32_t)(
				(uint32_t)DP_DDIBUS_BASE_REG_ADDRESS +
				(uint32_t)DP_REGISTER_BANK_OFFSET +
				(uint32_t)DP_REGISTER_BANK_REG_24);

	uiRegMap_R_StdEn = Dptx_Reg_Direct_Read(uiRegAddr);
	uiRegMap_W_StdEn = (uiRegMap_R_StdEn | (uint32_t)STD_EN_MASK);
	Dptx_Reg_Direct_Write(uiRegAddr, uiRegMap_W_StdEn);

	dptx_notice("PHY Lanes are reswapped: 0x%08x -> 0x%08x",
				uiRegMap_R_StdEn,
				uiRegMap_W_StdEn);
}

void Dpv14_Tx_API_Config_SDM_BypassControl(bool bSDM_Bypass)
{
	uint32_t uiRegAddr;
	uint32_t uiRegMap_R_SDMBypass, uiRegMap_W_SDMBypass;

	uiRegAddr = (uint32_t)(
				(uint32_t)DP_DDIBUS_BASE_REG_ADDRESS +
				(uint32_t)DP_REGISTER_BANK_OFFSET +
				(uint32_t)DP_REGISTER_BANK_REG_24);

	uiRegMap_R_SDMBypass = Dptx_Reg_Direct_Read(uiRegAddr);

	if (bSDM_Bypass == (bool)true) {
		/* For coverity */
		uiRegMap_W_SDMBypass = (uiRegMap_R_SDMBypass | (uint32_t)SDM_DIS_MASK);
	} else {
		/* For coverity */
		uiRegMap_W_SDMBypass = (uiRegMap_R_SDMBypass & ~((uint32_t)SDM_DIS_MASK));
	}

	Dptx_Reg_Direct_Write(uiRegAddr, uiRegMap_W_SDMBypass);

	dptx_notice("SDM Bypass %s...Reg[0x12480060]: 0x%08x -> 0x%08x ",
				(bSDM_Bypass == (bool)true) ? "On":"Off",
				uiRegMap_R_SDMBypass,
				uiRegMap_W_SDMBypass);
}

void Dpv14_Tx_API_Config_SRVC_BypassControl(bool bSRVC_Bypass)
{
	uint32_t uiRegAddr;
	u32 uiRegMap_R_SRVCBypass, uiRegMap_W_SRVCBypass;

	uiRegAddr = (uint32_t)(
				(uint32_t)DP_DDIBUS_BASE_REG_ADDRESS +
				(uint32_t)DP_REGISTER_BANK_OFFSET +
				(uint32_t)DP_REGISTER_BANK_REG_24);

	uiRegMap_R_SRVCBypass = Dptx_Reg_Direct_Read(uiRegAddr);

	if (bSRVC_Bypass == (bool)true) {
		/* For coverity */
		uiRegMap_W_SRVCBypass =
		(uiRegMap_R_SRVCBypass | (uint32_t)SRVC_DIS_MASK);
	} else {
		/* For coverity */
		uiRegMap_W_SRVCBypass =
		(uiRegMap_R_SRVCBypass & ~((uint32_t)SRVC_DIS_MASK));
	}

	Dptx_Reg_Direct_Write(uiRegAddr, uiRegMap_W_SRVCBypass);

	dptx_notice("SRVC Bypass %s...Reg[0x12480060]: 0x%08x -> 0x%08x ",
				(bSRVC_Bypass ==(bool)true) ? "On":"Off",
				uiRegMap_R_SRVCBypass,
				uiRegMap_W_SRVCBypass);
}

void Dpv14_Tx_API_Config_MuxSelect(uint32_t uiMux_Index, uint8_t ucDP_PortIndex)
{
	uint8_t ucRegMap_MuxSel_Shift = 0;
	uint32_t uiRegAddr;
	uint32_t uiRegMap_MuxSel_Mask = 0;
	uint32_t uiRegMap_R_MuxSel = 0;
	uint32_t uiRegMap_W_MuxSel = 0;

	uiRegAddr = (uint32_t)(
				(uint32_t)DP_DDIBUS_BASE_REG_ADDRESS +
				(uint32_t)DP_REGISTER_BANK_OFFSET +
				(uint32_t)DP_REGISTER_BANK_REG_24);

	switch (ucDP_PortIndex) {
	case (uint8_t)DPTX_INPUT_STREAM_0:
		uiRegMap_MuxSel_Mask = (uint32_t)SOURCE0_MUX_SEL_MASK;
		ucRegMap_MuxSel_Shift = (uint8_t)SOURCE0_MUX_SEL_SHIFT;
		break;
	case (uint8_t)DPTX_INPUT_STREAM_1:
		uiRegMap_MuxSel_Mask = (uint32_t)SOURCE1_MUX_SEL_MASK;
		ucRegMap_MuxSel_Shift = (uint8_t)SOURCE1_MUX_SEL_SHIFT;
		break;
	case (uint8_t)DPTX_INPUT_STREAM_2:
		uiRegMap_MuxSel_Mask = (uint32_t)SOURCE2_MUX_SEL_MASK;
		ucRegMap_MuxSel_Shift = (uint8_t)SOURCE2_MUX_SEL_SHIFT;
		break;
	case (uint8_t)DPTX_INPUT_STREAM_3:
		uiRegMap_MuxSel_Mask = (uint32_t)SOURCE3_MUX_SEL_MASK;
		ucRegMap_MuxSel_Shift = (uint8_t)SOURCE3_MUX_SEL_SHIFT;
		break;
	default:
		dptx_dbg("Invalid DP index: %d\n",  ucDP_PortIndex);
		break;
	}

	uiRegMap_R_MuxSel = Dptx_Reg_Direct_Read(uiRegAddr);

	uiRegMap_W_MuxSel = (uiRegMap_R_MuxSel & ~uiRegMap_MuxSel_Mask);

	uiRegMap_W_MuxSel = ((uiRegMap_W_MuxSel) | (uint32_t)(uiMux_Index << ucRegMap_MuxSel_Shift));

	Dptx_Reg_Direct_Write(uiRegAddr, uiRegMap_W_MuxSel);

	dptx_notice("Mux select[0x12480060](0x%x -> 0x%x): Mux %d -> DP %d ",
					uiRegMap_R_MuxSel,
					uiRegMap_W_MuxSel,
					uiMux_Index,
					ucDP_PortIndex);
}

void Dpv14_Tx_API_Reset_PLL_Blk(void)
{
	uint32_t uiRegAddr;
	uint32_t uiRegMap_PLLPMS;

	uiRegAddr = (uint32_t)(
				(uint32_t)DP_DDIBUS_BASE_REG_ADDRESS +
				(uint32_t)DP_CKC_OFFSET +
				(uint32_t)DPTX_CKC_CFG_CLKCTRL0);
	Dptx_Reg_Direct_Write(uiRegAddr, (u32)0x00);

	uiRegAddr = (uint32_t)(
				(uint32_t)DP_DDIBUS_BASE_REG_ADDRESS +
				(uint32_t)DP_CKC_OFFSET +
				(uint32_t)DPTX_CKC_CFG_CLKCTRL1);
	Dptx_Reg_Direct_Write(uiRegAddr, (u32)0x00);

	uiRegAddr = (uint32_t)(
				(uint32_t)DP_DDIBUS_BASE_REG_ADDRESS +
				(uint32_t)DP_CKC_OFFSET +
				(uint32_t)DPTX_CKC_CFG_CLKCTRL2);
	Dptx_Reg_Direct_Write(uiRegAddr, (u32)0x00);

	uiRegAddr = (uint32_t)(
				(uint32_t)DP_DDIBUS_BASE_REG_ADDRESS +
				(uint32_t)DP_CKC_OFFSET +
				(uint32_t)DPTX_CKC_CFG_CLKCTRL3);
	Dptx_Reg_Direct_Write(uiRegAddr, (u32)0x00);

	uiRegAddr = (uint32_t)(
				(uint32_t)DP_DDIBUS_BASE_REG_ADDRESS +
				(uint32_t)DP_CKC_OFFSET +
				(uint32_t)DPTX_CKC_CFG_PLLPMS);
	uiRegMap_PLLPMS = Dptx_Reg_Direct_Read(uiRegAddr);
	uiRegMap_PLLPMS = (uiRegMap_PLLPMS | (uint32_t)PLLPMS_BYPASS_MASK);
	Dptx_Reg_Direct_Write(uiRegAddr, uiRegMap_PLLPMS);

	uiRegMap_PLLPMS = Dptx_Reg_Direct_Read(uiRegAddr);
	uiRegMap_PLLPMS = (uiRegMap_PLLPMS & ~((uint32_t)PLLPMS_RESETB_MASK));
	Dptx_Reg_Direct_Write(uiRegAddr, uiRegMap_PLLPMS);

	dptx_notice("Clk path to XIN ...\n");
}

int32_t Dpv14_Tx_API_Init(
			uint8_t ucNumOfStreams,
			enum DPTX_LINK_RATE eLinkRate,
			enum DPTX_LINK_LANE eLinkLanes)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle = NULL;

	if (eLinkRate >= LINK_RATE_MAX) {
		dptx_err("Invalid link rate as %d", (u32)eLinkRate);
		iRetVal = DPTX_RETURN_EINVAL;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if ((eLinkLanes != DPTX_PHY_LANE_1) &&
			(eLinkLanes != DPTX_PHY_LANE_2) &&
			(eLinkLanes != DPTX_PHY_LANE_4)) {
			dptx_err("Invalid link lane as %d", (u32)eLinkLanes);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		iRetVal = Dptx_Platform_Init(
				ucNumOfStreams,
				(u8)eLinkRate,
				(u8)eLinkLanes);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		pstHandle = Dptx_Platform_Get_Device_Handle();
		if (pstHandle == NULL) {
			dptx_err("Failed to get handle");
			iRetVal = DPTX_RETURN_EACCES;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Avgen_Init(pstHandle);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Core_Link_Power_On(pstHandle);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Core_Init(pstHandle);
	}

	return iRetVal;
}

int32_t Dpv14_Tx_API_Deinit(void)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle = NULL;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Core_Deinit(pstHandle);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Platform_Deinit(pstHandle);
	}

	return iRetVal;
}

int32_t Dpv14_Tx_API_Start(void)
{
	bool bHPD_Status;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiHDP_CheckTry;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (pstHandle->bPanelDisplay_Mode == (bool)true) {
			/* For coverity */
			bHPD_Status = (bool)HPD_STATUS_PLUGGED;
		} else {
			for (uiHDP_CheckTry = 0;
				uiHDP_CheckTry < (uint32_t)MAX_CHECK_HPD_NUM;
				uiHDP_CheckTry++) {
				iRetVal = Dptx_Intr_Get_HotPlug_Status(
							pstHandle,
							&bHPD_Status);
				if ((iRetVal == DPTX_RETURN_NO_ERROR) &&
					(bHPD_Status == (bool)HPD_STATUS_PLUGGED)) {
					/* For coverity */
					dptx_notice("Hot plugged after %dms",
					(uiHDP_CheckTry * 10));
					break;
				}

				mdelay(10);
			}

			if (uiHDP_CheckTry == (uint32_t)MAX_CHECK_HPD_NUM) {
				dptx_warn("Hot unplugged after %dms",
				(uiHDP_CheckTry * 100));
			}
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (bHPD_Status == (bool)HPD_STATUS_PLUGGED) {
			/* For coverity */
			iRetVal = Dptx_Intr_Handle_Hotplug(pstHandle);
		}
	} 

	return iRetVal;
}

int32_t Dpv14_Tx_API_Stop(void)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle = NULL;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Intr_Handle_HotUnplug(pstHandle);
	}

	return iRetVal;
}

int32_t Dpv14_Tx_API_Set_Video_Code(
			uint8_t ucNumOfStreams,
			uint32_t auiDefaultVideoCode[DPTX_INPUT_STREAM_MAX])
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (ucNumOfStreams > (uint8_t)DPTX_INPUT_STREAM_MAX) {
			dptx_err("Invalid DP index %d", ucNumOfStreams);
			iRetVal=  DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (auiDefaultVideoCode == NULL) {
			dptx_err("VIC array is NULL");
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		iRetVal = Dptx_Avgen_Set_Video_Code(
					pstHandle,
					ucNumOfStreams,
					auiDefaultVideoCode);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		dptx_dbg("ucNumOfStreams(%d), VIC(%d, %d, %d, %d)",
					ucNumOfStreams,
					auiDefaultVideoCode[0],
					auiDefaultVideoCode[1],
					auiDefaultVideoCode[2],
					auiDefaultVideoCode[3]);
	}

	return iRetVal;
}

int32_t Dpv14_Tx_API_Set_Video_PeriPixelClock(
			uint8_t ucNumOfStreams,
			uint32_t auiPeri_PixelClock[DPTX_INPUT_STREAM_MAX])
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (ucNumOfStreams > (uint8_t)DPTX_INPUT_STREAM_MAX) {
			dptx_err("Invalid DP index %d", ucNumOfStreams);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (auiPeri_PixelClock == NULL) {
			dptx_err("PClk array is NULL");
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		iRetVal = Dptx_Avgen_Set_Video_PeriPixelClock(
					pstHandle,
					ucNumOfStreams,
					auiPeri_PixelClock);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		dptx_dbg("ucNumOfStreams(%d), P Clk(%d, %d, %d, %d)",
					ucNumOfStreams,
					auiPeri_PixelClock[0],
					auiPeri_PixelClock[1],
					auiPeri_PixelClock[2],
					auiPeri_PixelClock[3]);
	}

	return iRetVal;
}

int32_t Dpv14_Tx_API_Set_VCPID_MST(
			uint8_t ucNumOfStreams,
			uint8_t aucVCP_Id[DPTX_INPUT_STREAM_MAX])
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
		}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (ucNumOfStreams > (uint8_t)DPTX_INPUT_STREAM_MAX) {
			dptx_err("Invalid DP index %d", ucNumOfStreams);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (aucVCP_Id == NULL) {
			dptx_err("VCP Id array is NULL");
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Ext_Set_VCPID_MST(pstHandle, ucNumOfStreams, aucVCP_Id);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		dptx_dbg("ucNumOfStreams(%d), VCP Id(%d, %d, %d, %d)",
					ucNumOfStreams,
					aucVCP_Id[0],
					aucVCP_Id[1],
					aucVCP_Id[2],
					aucVCP_Id[3]);
	}

	return iRetVal;
}

int32_t Dpv14_Tx_API_Set_Video_ColorSpace(
			enum DPTX_VIDEO_ENCODING_TYPE eVideoType)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (eVideoType >= VIDEO_ENCODING_MAX) {
			dptx_err("Invalid video encoding type : %d",
						(uint32_t)eVideoType);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		pstHandle->stVideoParams.ucPixel_Encoding = (uint8_t)eVideoType;

		dptx_dbg("Vidoe encoding type : %s",
					(eVideoType == VIDEO_ENCODING_RGB) ? "RGB" :
					(eVideoType == VIDEO_ENCODING_YCBCR422) ?
					"YCbCr 4:2:2" : "YCbCr 4:4:4");
	}

	return iRetVal;
}

void Dpv14_Tx_API_Set_Audio_Sel(uint32_t uiData)
{
	uint32_t        uiWriteData;
	uint32_t uiRegAddr;

	dptx_dbg("Set Audio sel 0x%08x", (u32)uiData);

	uiRegAddr = (uint32_t)DPTX_AUDIO_SEL_REG;

	uiWriteData = (uiData & (uint32_t)DPTX_AUDIO_SEL_MASK);

	Dptx_Reg_Direct_Write(uiRegAddr, uiWriteData);
}

void Dpv14_Tx_API_Get_Audio_Sel(uint32_t *pucData)
{
	uint32_t uiRegAddr;
	uint32_t uiReadData;

	uiRegAddr = (uint32_t)DPTX_AUDIO_SEL_REG;

	uiReadData = Dptx_Reg_Direct_Read(uiRegAddr);
	uiReadData = (uiReadData & (uint32_t)DPTX_AUDIO_SEL_MASK);

	dptx_dbg("Get Audio sel 0x%08x", uiReadData);

	*pucData = uiReadData;
}

int32_t Dpv14_Tx_API_Set_PanelDisplay_Mode(uint8_t ucPanelDisplay_Mode)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (ucPanelDisplay_Mode == 0U) {
			/* For coverity */
			pstHandle->bPanelDisplay_Mode = (bool)false;
		} else {
			/* For coverity */
			pstHandle->bPanelDisplay_Mode = (bool)true;
		}
	}

	dptx_dbg("Panel display mode : %s", (ucPanelDisplay_Mode == 0U) ? "Off":"On");

	return iRetVal;
}

int32_t Dpv14_Tx_API_Set_SideBand_Msg_Supported(
			uint8_t ucSideBand_MSG_Supported)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Ext_Set_SideBand_Msg_Supported(
					pstHandle,
					(bool)ucSideBand_MSG_Supported);
	}

	dptx_dbg("Sink device %s Sideband MSG protocol",
				(ucSideBand_MSG_Supported == 0U) ?
				"doesn't supports":"supports");

	return iRetVal;
}

int32_t Dpv14_Tx_API_Set_MaxLinkRate_Supported(enum DPTX_LINK_RATE eLink_Rate)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (eLink_Rate >= LINK_RATE_MAX) {
		dptx_err("Invalid link rate %d", (uint32_t)eLink_Rate);
		iRetVal = DPTX_RETURN_EINVAL;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		iRetVal = Dptx_Link_Set_MaxLinkRate_Supported(
					pstHandle,
					(enum PHY_RATE)eLink_Rate);
	}

	dptx_dbg("Set link rate to %d", (uint32_t)eLink_Rate);

	return iRetVal;
}

int32_t Dpv14_Tx_API_Set_MaxLinkLane_Supported(enum DPTX_LINK_LANE eLink_Lane)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	enum PHY_LINK_LANE ePHY_LinkLane;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (eLink_Lane >= DPTX_PHY_LANE_MAX) {
			dptx_err("Invalid link lane %d", (uint32_t)eLink_Lane);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		ePHY_LinkLane = (enum PHY_LINK_LANE)eLink_Lane;

		iRetVal = Dptx_Link_Set_MaxLinkLane_Supported(
					pstHandle,
					ePHY_LinkLane);
	}

	dptx_dbg("Set link lane to %d", (uint32_t)eLink_Lane);

	return iRetVal;
}

int32_t Dpv14_Tx_API_Perform_HPD_WorkingFlow(void)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Intr_Handle_HotUnplug(pstHandle);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Intr_Handle_Hotplug(pstHandle);
	}

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
		dptx_err("Ptr. of Dtd params is NULL");
		iRetVal = DPTX_RETURN_EINVAL;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		iRetVal = Dptx_Avgen_Fill_Dtd(
					&stDtd,
					uiVideo_Code,
					uiRefreshRate,
					ucVideoFormat);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
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

	return iRetVal;
}

int32_t Dpv14_Tx_API_Get_Dtd_From_Edid(
			struct DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
			uint8_t ucStream_Index)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	const void *pvMem;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (ucStream_Index >= (uint8_t)DPTX_INPUT_STREAM_MAX) {
			dptx_err("Invalid DP index %d", ucStream_Index);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Intr_Handle_Edid(pstHandle, ucStream_Index);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		pvMem = memcpy(pstDptx_Dtd_Params,
						&pstHandle->stVideoParams.stDtdParams[ucStream_Index],
						sizeof(struct  DPTX_Dtd_Params_t));
		if (pvMem == NULL) {
			dptx_err("Failed to copy mememory");
			iRetVal = DPTX_RETURN_EACCES;
		}
	}

	return iRetVal;
}

int32_t Dpv14_Tx_API_Get_Dtd_From_PreferredVIC(
			struct DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
			uint8_t ucStream_Index)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	const void *pvMem;
	const struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (ucStream_Index >= (uint8_t)DPTX_INPUT_STREAM_MAX) {
			dptx_err("Invalid DP index %d", ucStream_Index);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		pvMem = memcpy(pstDptx_Dtd_Params,
					&pstHandle->stVideoParams.stDtdParams[ucStream_Index],
					sizeof(struct DPTX_Dtd_Params_t));
		if (pvMem == NULL) {
			dptx_err("Failed to copy mememory");
			iRetVal = DPTX_RETURN_EACCES;
		}
	}

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

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (puiBuffer == NULL) {
			dptx_err("Invalid parameter as puiBuffer is NULL");
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (ucLength == 0U) {
			dptx_err("Invalid parameter as Length is 0");
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (uiOffset > (uint32_t)DP_MAX_OFFSET) {
			dptx_err("Invalid offset as 0x%x", uiOffset);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (pstHandle->uiRegBank_RegAddr_Offset > (uint32_t)DP_MAX_OFFSET) {
			dptx_err("Invalid RegBank offset as 0x%x",
						pstHandle->uiRegBank_RegAddr_Offset);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		dptx_dbg("Write Register - Offset %d, Length %d",
					uiOffset,
					(uint32_t)ucLength);

		for (ucCount = 0; ucCount < ucLength; ucCount++) {
			uiRegOffset = (pstHandle->uiRegBank_RegAddr_Offset + uiOffset);
			uiOneRegOffset = (uint32_t)(sizeof(uint32_t) * ucCount);

			uiRegOffset += uiOneRegOffset;

			Dptx_Reg_Writel(pstHandle,
							uiRegOffset,
							puiBuffer[ucCount]);
		}
	}

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

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (puiBuffer == NULL) {
			dptx_err("Invalid parameter as puiBuffer is NULL");
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (ucLength == 0U) {
			dptx_err("Invalid parameter as Length is 0");
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (uiOffset > (uint32_t)DP_MAX_OFFSET) {
			dptx_err("Invalid offset as 0x%x", uiOffset);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (pstHandle->uiRegBank_RegAddr_Offset > (uint32_t)DP_MAX_OFFSET) {
			dptx_err("Invalid RegBank offset as 0x%x", pstHandle->uiRegBank_RegAddr_Offset);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		dptx_dbg("Read Register - Offset %d, Length %d",
					uiOffset,
					(uint32_t)ucLength);

		for (ucCount = 0; ucCount < ucLength; ucCount++) {
			uiRegOffset = (pstHandle->uiRegBank_RegAddr_Offset + uiOffset);
			uiOneRegOffset = (uint32_t)(sizeof(uint32_t) * ucCount);

			uiRegOffset = (uiRegOffset + uiOneRegOffset);

			puiBuffer[ucCount] = Dptx_Reg_Readl(pstHandle, uiRegOffset);
		}
	}

	return iRetVal;
}

int32_t Dpv14_Tx_API_Get_PLL_Status(uint8_t *pucPll_Locked)
{
	uint8_t ucPllLock = 0;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		iRetVal = Dptx_Platform_Get_PLLLock_Status(pstHandle, &ucPllLock);

		*pucPll_Locked = ucPllLock;
	}

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
	uint32_t uiLane_Index;
	uint32_t uiRegMap_Cctl, uiRegMap_PhyIfCtrl, uiRegMap_RegBank_21;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		dptx_cts("CTS Test => ");
		dptx_cts(" -.Link rate: %s",
					(eLink_Rate == LINK_RATE_RBR) ? "RBR" :
					(eLink_Rate == LINK_RATE_HBR) ? "HBR" :
					(eLink_Rate == LINK_RATE_HBR2) ? "HBR2" : "HBR3");
		dptx_cts(" -.SSC: %s", bSSC_Enabled ? "Enable" : "Disable");
		dptx_cts(" -.Pre Emp: %s",
					(ePreEmp == DPTX_PRE_EMPHASIS_LEVEL_0) ? "Level 0" :
					(ePreEmp == DPTX_PRE_EMPHASIS_LEVEL_1) ? "Level 1" :
					(ePreEmp == DPTX_PRE_EMPHASIS_LEVEL_2) ?
					"Levle 2" : "Level 3");
		dptx_cts(" -.VSW: %s",
					(eVSW == DPTX_VOLTAGE_SWING_LEVEL_0) ? "Level 0" :
					(eVSW == DPTX_VOLTAGE_SWING_LEVEL_1) ? "Level 1" :
					(eVSW == DPTX_VOLTAGE_SWING_LEVEL_2) ?
					"Levle 2" : "Level 3");
		dptx_cts(" -.Pattern: %s",
					(uiPattern_Type == DPTX_TRAINING_CTRL_TPS_1_D102) ?
					"D10.2" :
					(uiPattern_Type == DPTX_TRAINING_CTRL_TPS_PRBS7) ?
					"PRBS7" :
					(uiPattern_Type == DPTX_TRAINING_CTRL_TPS_CUSTOM80) ?
					"CUSTOM80" :
					(uiPattern_Type == DPTX_TRAINING_CTRL_TPS_CP2520_1) ?
					"CP2520_P1" :
					(uiPattern_Type == DPTX_TRAINING_CTRL_TPS_CP2520_2) ?
					"CP2520_P2" : "TPS 4");

		uiRegMap_RegBank_21 = Dptx_Reg_Readl(
							pstHandle,
							(0x80000 + DP_REGISTER_BANK_REG_21));
							uiRegMap_RegBank_21 &= (0xFFFFFFF3U);
							uiRegMap_RegBank_21 |= (u32)ucClk_Selection;
		Dptx_Reg_Writel(
							pstHandle,
							(0x80000 + DP_REGISTER_BANK_REG_21),
							(u32)uiRegMap_RegBank_21);

		iRetVal = Dptx_Platform_Set_RegisterBank(
							pstHandle,
							(enum PHY_RATE)eLink_Rate);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Core_Init(pstHandle);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Core_Set_PHY_NumOfLanes(
							pstHandle,
							(u8)DPTX_PHY_LANE_4);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		iRetVal = Dptx_Core_Set_PHY_PowerState(
							pstHandle,
							PHY_POWER_DOWN_PHY_CLOCK);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Core_Get_PHY_BUSY_Status(
							pstHandle,
							(u8)DPTX_PHY_LANE_4);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		uiRegMap_PhyIfCtrl = Dptx_Reg_Readl(pstHandle, DPTX_PHYIF_CTRL);
		if (bSSC_Enabled) {
			/* For coverity */
			uiRegMap_PhyIfCtrl = (uiRegMap_PhyIfCtrl & ~((uint32_t)DPTX_PHYIF_CTRL_SSC_DIS));
		} else {
			/* For coverity */
			uiRegMap_PhyIfCtrl = (uiRegMap_PhyIfCtrl | (uint32_t)DPTX_PHYIF_CTRL_SSC_DIS);
		}

		Dptx_Reg_Writel(pstHandle, DPTX_PHYIF_CTRL, uiRegMap_PhyIfCtrl);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Core_Set_PHY_Rate(pstHandle, (enum PHY_RATE)eLink_Rate);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Core_Set_PHY_PowerState(pstHandle, PHY_POWER_ON);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Core_Get_PHY_BUSY_Status(pstHandle, (u8)DPTX_PHY_LANE_4);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		iRetVal = Dptx_Core_Set_PHY_Pattern(
							pstHandle,
							DPTX_PHYIF_CTRL_TPS_NONE);
	}

	for (uiLane_Index = 0; uiLane_Index < (uint32_t)DPTX_PHY_LANE_4; uiLane_Index++) {
		if (iRetVal == DPTX_RETURN_NO_ERROR) {
			iRetVal = Dptx_Core_Set_PHY_PreEmphasis(
							pstHandle,
							uiLane_Index,
							(enum PHY_PRE_EMPHASIS_LEVEL)ePreEmp);
		}

		if (iRetVal == DPTX_RETURN_NO_ERROR) {
			iRetVal = Dptx_Core_Set_PHY_VSW(
							pstHandle,
							uiLane_Index,
							(enum PHY_VOLTAGE_SWING_LEVEL)eVSW);
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (uiPattern_Type == (uint32_t)DPTX_TRAINING_CTRL_TPS_CUSTOM80) {
			Dptx_Reg_Writel(pstHandle, DPTX_CUSTOMPAT0, 0x3E0F83E0);
			Dptx_Reg_Writel(pstHandle, DPTX_CUSTOMPAT1, 0x3E0F83E0);
			Dptx_Reg_Writel(pstHandle, DPTX_CUSTOMPAT2, 0x3E0F83E0);
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Core_Set_PHY_Pattern(pstHandle, (uint32_t)uiPattern_Type);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Core_Enable_PHY_XMIT(pstHandle, (uint32_t)DPTX_PHY_LANE_4);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		uiRegMap_Cctl = Dptx_Reg_Readl(pstHandle, (uint32_t)DPTX_CCTL);
		uiRegMap_Cctl = (uiRegMap_Cctl | (uint32_t)DPTX_CCTL_ENH_FRAME_EN);
		Dptx_Reg_Writel(pstHandle, DPTX_CCTL, uiRegMap_Cctl);
	}

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

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	dptx_cts("PHY EQ Setting => ");
	dptx_cts(" -.Main : %d", uiEQ_Main);
	dptx_cts(" -.Post : %d", uiEQ_Post);
	dptx_cts(" -.Pre : %d", uiEQ_Pre);
	dptx_cts(" -.VBoost : %d", uiEQ_VBoost);

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if (uiEQ_VBoost == 7U) {
			/* For coverity */
			ucEQ_VBoost_7 = 1U;
		}

		iRetVal = Dptx_Platform_Set_Tx_EQ(
		pstHandle,
		uiEQ_Main,
		uiEQ_Post,
		uiEQ_Pre,
		ucEQ_VBoost_7);
	}

	return  iRetVal;
}

int32_t Dpv14_Tx_API_Set_LinkPrams(uint8_t ucParam, uint8_t ucLevel)
{
	uint32_t uiLane_Index;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstHandle;

	pstHandle = Dptx_Platform_Get_Device_Handle();
	if (pstHandle == NULL) {
		dptx_err("Failed to get handle");
		iRetVal = DPTX_RETURN_EACCES;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if ((ucParam == 0U) && (ucLevel > (uint8_t)PRE_EMPHASIS_LEVEL_3)) {
			dptx_err("Invalid Pre-Emp level as %d", ucLevel);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		if ((ucParam == 1U) && (ucLevel > (uint8_t)VOLTAGE_SWING_LEVEL_3)) {
			dptx_err("Invalid VSW level as %d", ucLevel);
			iRetVal = DPTX_RETURN_EINVAL;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		for (uiLane_Index = 0; uiLane_Index < (uint32_t)DPTX_PHY_LANE_4; uiLane_Index++) {
			if (ucParam == 0U) {
				iRetVal = Dptx_Core_Set_PHY_PreEmphasis(
							pstHandle,
							uiLane_Index,
							(enum PHY_PRE_EMPHASIS_LEVEL)ucLevel);
				if (iRetVal != DPTX_RETURN_NO_ERROR) {
					dptx_err("from Dptx_Core_Set_PHY_PreEmphasis(): %d",
					uiLane_Index);
				}
			}
			if (ucParam == 1U) {
				iRetVal = Dptx_Core_Set_PHY_VSW(
							pstHandle,
							uiLane_Index,
				(enum PHY_VOLTAGE_SWING_LEVEL)ucLevel);
				if (iRetVal != DPTX_RETURN_NO_ERROR) {
					dptx_err("from Dptx_Core_Set_PHY_VSW(): %d",
					uiLane_Index);
				}
			}
		}

		dptx_cts("PHY %s Setting => ", (ucParam == 0U) ? "Pre-Emp":"VSW");
		dptx_cts(" -.Level : %d", ucLevel);
	}

	return  iRetVal;
}

