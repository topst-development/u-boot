// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/
#include "dptx_v14.h"
#include "dptx_reg.h"
#include "dptx_drm_dp_addition.h"
#include "dptx_dbg.h"
#include <linux/types.h>
#include <linux/compat.h>


#define CHECK_REG_OFFSET(x) (((x) < (uint32_t)DP_MAX_OFFSET) ? (bool)true : (bool)false)

static struct Dptx_Params *pstDrv_Handle;

static void dptx_v14_set_reg_ap_access(const struct Dptx_Params *pstDptx)
{
	uint32_t uiRegAddr;
	uint32_t uiRegMap_R_APBSel = 0, uiRegMap_W_APBSel = 0;

	uiRegAddr = (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) ? (uint32_t)DPTX_APB_SEL_SEC_PHY : (uint32_t)DPTX_APB_SEL_SNOP_PHY;

	uiRegMap_R_APBSel = Dptx_Reg_Direct_Read(uiRegAddr);
	uiRegMap_W_APBSel = (uiRegMap_R_APBSel | (uint32_t)DPTX_APB_SEL_MASK);
	Dptx_Reg_Direct_Write(uiRegAddr, uiRegMap_W_APBSel);

	dptx_dbg("APB Sel to AP :Reg[0x%08x]: 0x%08x->0x%08x\n", uiRegAddr, uiRegMap_R_APBSel, uiRegMap_W_APBSel);
}

static void dptx_v14_release_coldrst_mask(void)
{
	uint32_t uiRegAddr;
	uint32_t uiR_HsmRstn_Msk = 0, uiW_HsmRstn_Msk = 0;

	uiRegAddr = (uint32_t)PMU_HSM_RSTN_MSK;

	uiR_HsmRstn_Msk = Dptx_Reg_Direct_Read(uiRegAddr);

	if ((uiR_HsmRstn_Msk & (uint32_t)PMU_COLD_RSTN_MSK) != 0U) {
		uiW_HsmRstn_Msk = (uiR_HsmRstn_Msk & ~((uint32_t)PMU_COLD_RSTN_MSK));

		Dptx_Reg_Direct_Write(uiRegAddr, uiW_HsmRstn_Msk);

		dptx_dbg("Release Dp cold reset mask: 0x%x->0x%x", uiR_HsmRstn_Msk, uiW_HsmRstn_Msk);
	}
}

static void dptx_v14_init_pre_condition(const struct Dptx_Params *pstDptx)
{
	dptx_v14_release_coldrst_mask();
	dptx_v14_set_reg_ap_access(pstDptx);
}

static void dptx_v14_init_protect(struct Dptx_Params *pstDptx)
{
	(void)Dptx_Protect_Set_PW(pstDptx);
	(void)Dptx_Protect_Set_CfgLock(pstDptx, (bool)DP_PORTECT_CFG_UNLOCKED);
	(void)Dptx_Protect_Set_CfgAccess(pstDptx, (bool)DP_PORTECT_CFG_ACCESSABLE);
}

static void dptx_v14_init_clk(struct Dptx_Params *pstDptx)
{
	uint8_t ucPLL_LockStatus;

	Dptx_Clk_Reset_PLL(pstDptx);
	Dptx_Clk_Set_PLL_Divisor(pstDptx);
	Dptx_Clk_Get_PLLLock_Status(pstDptx, &ucPLL_LockStatus);
	Dptx_Clk_Set_PLL_ClkSrc(pstDptx, (u8)CLKCTRL_PLL_DIVIDER_OUTPUT);
}

static int32_t dptx_v14_init_wrap(struct Dptx_Params *pstDptx)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;

	dptx_v14_init_protect(pstDptx);
	dptx_v14_init_clk(pstDptx);

	iRetVal = Dptx_Cfg_Init(pstDptx, pstDptx->ucMax_Rate);
	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		iRetVal = (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) ?
			Dptx_Sec_PHY_Init(pstDptx, pstDptx->ucMax_Rate, pstDptx->ucMax_Lanes) :
			DPTX_RETURN_NO_ERROR;
	}

	return iRetVal;
}


static int32_t dptx_v14_alloc_mem(struct Dptx_Params **ppstDptx)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstDptx = NULL;

	pstDptx = (struct Dptx_Params *)malloc(sizeof(*pstDptx));
	if (pstDptx == NULL) {
		dptx_err("failed to alloc Drv memory");

		iRetVal = DPTX_RETURN_ENOMEM;
	} else {
		(void)memset(pstDptx, 0, sizeof(*pstDptx));

		pstDptx->pucEdidBuf = (uint8_t *)malloc((size_t)DPTX_EDID_BUFLEN);
		if (pstDptx->pucEdidBuf == NULL) {
			dptx_warn("failed to alloc EDID memory");
		}

		pstDrv_Handle = pstDptx;
		*ppstDptx = pstDptx;
	}

	return iRetVal;
}

static int32_t dptx_v14_init_params(const struct dpv14_drv_params *drv_params, struct Dptx_Params **ppstDptx)
{
	uintptr_t link_base_address = (uintptr_t)DP_DDIBUS_BASE_REG_ADDRESS;
	uint32_t dp_stream_id;

	struct dptx_video_params *video_params = NULL;
	struct Dptx_Params *pstDptx = NULL;

	int32_t ret = DPTX_RETURN_NO_ERROR;

	ret = dptx_v14_alloc_mem(&pstDptx);
	if (DPTX_RETURN_SUCCESS(ret)) {
		pstDptx->ucNumOfStreams = drv_params->num_of_dps;
		pstDptx->bMultStreamTransport = (drv_params->num_of_dps > 1U) ? (bool)true : (bool)false;

		pstDptx->pvDPLink_BaseAddr = (void *)link_base_address;
		pstDptx->uiHDCP22_RegAddr_Offset = (uint32_t)DP_HDCP_OFFSET;
		pstDptx->uiRegBank_RegAddr_Offset = (uint32_t)DP_REGISTER_BANK_OFFSET;
		pstDptx->uiCKC_RegAddr_Offset = (uint32_t)DP_CKC_OFFSET;
		pstDptx->uiProtect_RegAddr_Offset = (uint32_t)DP_PROTECT_OFFSET;
		pstDptx->uiSEC_PHY_Reg_Offset = (uint32_t)DP_SEC_PHY_OFFSET;

		pstDptx->bEstablish_Timing_Present = (bool)false;
		pstDptx->bSdm_Bypass = drv_params->sdm_bypass;
		pstDptx->bTrvc_Bypass = drv_params->trvc_bypass;
		pstDptx->bPhy_Lane_Std = drv_params->phy_lane_swap;

		pstDptx->ucMax_Rate = drv_params->max_rate;
		pstDptx->ucMax_Lanes = drv_params->max_lane;
		pstDptx->ucPHY_Ref_Clk = (uint8_t)PHY_REF_CLK_DIRECT_XIN;
		pstDptx->eEstablished_Timing = DMT_NONE;
		pstDptx->ePhy_Dev = (enum PHY_DEVICE_MODEL)drv_params->phy_model;

		(void)memcpy(pstDptx->aucVCP_Id, drv_params->vcp_id, sizeof(uint8_t) * (size_t)PHY_INPUT_STREAM_MAX);
		for (dp_stream_id = 0u; dp_stream_id < (uint32_t)PHY_INPUT_STREAM_MAX; dp_stream_id++) {
			video_params = &pstDptx->video_params[dp_stream_id];
			video_params->video_code = drv_params->vic[dp_stream_id][VIC_CFG_VIDEO_CODE];
			if (drv_params->vic[dp_stream_id][VIC_CFG_VIDEO_FORMAT] > (uint32_t)VFS_VESA_DMT) {
				dptx_err("video format standard %u is out of range", drv_params->vic[dp_stream_id][VIC_CFG_VIDEO_FORMAT]);
				ret = -DPTX_RETURN_EINVAL;
			} else {
				video_params->video_format_standard =
					(enum VIDEO_FORMAT_STANDARD_TYPE)drv_params->vic[dp_stream_id][VIC_CFG_VIDEO_FORMAT];
				if (drv_params->pixel_encoding > (uint32_t)PIXEL_ENCODING_TYPE_YCBCR444) {
					dptx_err("pixel encoding %u is out of range", drv_params->pixel_encoding);
					ret = -DPTX_RETURN_EINVAL;
				} else {
					video_params->pixel_encoding = (enum PIXEL_ENCODING_TYPE)drv_params->pixel_encoding;
				}
			}
			if (DPTX_RETURN_SUCCESS(ret)) {
				if (drv_params->dd_mux_select[dp_stream_id] <= 4u) {
					pstDptx->aucMuxId[dp_stream_id] =
						(uint8_t)drv_params->dd_mux_select[dp_stream_id];
				}
			}
			if (DPTX_RETURN_ERROR(ret)) {
				break;
			}
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		(void)memcpy(&pstDptx->hw_config, &drv_params->hw_config, sizeof(drv_params->hw_config));

		*ppstDptx = pstDptx;
	}

	return ret;

}

int32_t Dptx_V14_Init(const struct dpv14_drv_params *drv_params)
{
	int32_t ret = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstDptx = NULL;

	ret = dptx_v14_init_params(drv_params, &pstDptx);
	if (DPTX_RETURN_SUCCESS(ret)) {
		dptx_v14_init_pre_condition(pstDptx);

		ret = dptx_v14_init_wrap(pstDptx);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		(void)Dptx_Core_Init(pstDptx);

		(void)Dptx_Vidin_Init(pstDptx);

		dptx_info("\n");
		dptx_info("DP Ver %u.%u.%u : %u DPs enabled",
					(uint32_t)TCC_DPTX_DRV_MAJOR_VER,
					(uint32_t)TCC_DPTX_DRV_MINOR_VER,
					(uint32_t)TCC_DPTX_DRV_SUBTITLE_VER,
					pstDptx->ucNumOfStreams);
		dptx_info(" PHY model = %s ",
					(pstDptx->ePhy_Dev == PHY_DEVICE_SNPS) ? "Synopsys" : "Samsung");
		dptx_info(" SDM Bypass = %s, TRVC Bypass = %s ",
					(pstDptx->bSdm_Bypass == (bool)true) ? "on" : "off",
					(pstDptx->bTrvc_Bypass == (bool)true) ? "on" : "off");
		dptx_info(" PHY Lane cfg = %s", (pstDptx->bPhy_Lane_Std == (bool)true) ? "Standard" : "Swap");
		dptx_info(" Max rate = %s, Max lane = %s",
					(pstDptx->ucMax_Rate == (uint8_t)DPTX_PHYIF_CTRL_RATE_RBR) ? "RBR" :
					(pstDptx->ucMax_Rate == (uint8_t)DPTX_PHYIF_CTRL_RATE_HBR) ? "HBR" :
					(pstDptx->ucMax_Rate == (uint8_t)DPTX_PHYIF_CTRL_RATE_HBR2) ? "HBR2" : "HBR3",
					(pstDptx->ucMax_Lanes == (uint8_t)PHY_LANE_1) ? "1 lane" :
					(pstDptx->ucMax_Lanes == (uint8_t)PHY_LANE_2) ? "2 lanes" : "4 lanes");
		dptx_info(" Encoding type = %s",
					(drv_params->pixel_encoding == (uint8_t)0U) ? "RGB" :
					(drv_params->pixel_encoding == (uint8_t)1U) ? "YCbCr222" : "YCbCr444");
		dptx_info(" Vcp id : %u %u %u %u", pstDptx->aucVCP_Id[0], pstDptx->aucVCP_Id[1], pstDptx->aucVCP_Id[2], pstDptx->aucVCP_Id[3]);
		dptx_info(" VIC : %u %u %u %u", pstDptx->video_params[0].video_code,
						pstDptx->video_params[1].video_code,
						pstDptx->video_params[2].video_code,
						pstDptx->video_params[3].video_code);
		dptx_info(" Mux id : %u %u %u %u", pstDptx->aucMuxId[0], pstDptx->aucMuxId[1], pstDptx->aucMuxId[2], pstDptx->aucMuxId[3]);
	}

	return ret;
}

int32_t Dptx_V14_Deinit(struct Dptx_Params *pstDptx)
{
	Dptx_V14_Free_Handle(pstDptx);

	pstDrv_Handle = NULL;

	return DPTX_RETURN_NO_ERROR;
}

void Dptx_V14_Free_Handle(struct Dptx_Params       *pstDptx_Handle)
{
	if (pstDptx_Handle != NULL) {
		if (pstDptx_Handle->pucEdidBuf != NULL) {
			free(pstDptx_Handle->pucEdidBuf);

			pstDptx_Handle->pucEdidBuf = NULL;
		}

		free(pstDptx_Handle);
	}
}

struct Dptx_Params *Dptx_V14_Get_Device_Handle(void)
{
	return pstDrv_Handle;
}
