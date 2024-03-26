// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/

#include "dptx_v14.h"
#include "dptx_reg.h"
#include "dptx_drm_dp_addition.h"
#include "dptx_dbg.h"


#define CHECK_REG_OFFSET(x) (((x) < (uint32_t)DP_MAX_OFFSET) ? (bool)true : (bool)false)

static struct Dptx_Params *pstDrv_Handle;

static void dptx_v14_set_reg_ap_access(struct Dptx_Params *pstDptx)
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

static void dptx_v14_init_pre_condition(struct Dptx_Params *pstDptx)
{
	dptx_v14_release_coldrst_mask();
	dptx_v14_set_reg_ap_access(pstDptx);
}

static void dptx_v14_init_protect(struct Dptx_Params *pstDptx)
{
	Dptx_Protect_Set_PW(pstDptx);
	Dptx_Protect_Set_CfgLock(pstDptx, (bool)DP_PORTECT_CFG_UNLOCKED);
	Dptx_Protect_Set_CfgAccess(pstDptx, (bool)DP_PORTECT_CFG_ACCESSABLE);
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
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iRetVal = (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) ? Dptx_Sec_PHY_Init(pstDptx, pstDptx->ucMax_Rate, pstDptx->ucMax_Lanes) : DPTX_RETURN_NO_ERROR;

return_funcs:
	return iRetVal;
}


static int32_t dptx_v14_alloc_mem(struct Dptx_Params **ppstDptx)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstDptx = NULL;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_3_violation : FALSE] */
	pstDptx = (struct Dptx_Params *)malloc(sizeof(*pstDptx));
	if (pstDptx == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("failed to alloc Drv memory");

		iRetVal = DPTX_RETURN_ENOMEM;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	(void)memset(pstDptx, 0, sizeof(*pstDptx));

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_3_violation : FALSE] */
	pstDptx->pucEdidBuf = (uint8_t *)malloc((size_t)DPTX_EDID_BUFLEN);
	if (pstDptx->pucEdidBuf == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_warn("failed to alloc EDID memory");
	}

	pstDrv_Handle = pstDptx;
	*ppstDptx = pstDptx;

return_funcs:
	return iRetVal;
}

static int32_t dptx_v14_init_params(struct Dptx_Init_Params *pstInit_Params, struct Dptx_Params **ppstDptx)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstDptx = NULL;

	iRetVal = dptx_v14_alloc_mem(&pstDptx);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	pstDptx->ucNumOfStreams                 = pstInit_Params->ucnum_of_dps;
	pstDptx->bMultStreamTransport           = (pstInit_Params->ucnum_of_dps > 1U) ? (bool)true : (bool)false;

	pstDptx->pvDPLink_BaseAddr              = (void *)DP_DDIBUS_BASE_REG_ADDRESS;
	pstDptx->uiHDCP22_RegAddr_Offset        = (uint32_t)DP_HDCP_OFFSET;
	pstDptx->uiRegBank_RegAddr_Offset       = (uint32_t)DP_REGISTER_BANK_OFFSET;
	pstDptx->uiCKC_RegAddr_Offset           = (uint32_t)DP_CKC_OFFSET;
	pstDptx->uiProtect_RegAddr_Offset       = (uint32_t)DP_PROTECT_OFFSET;
	pstDptx->uiSEC_PHY_Reg_Offset			= (uint32_t)DP_SEC_PHY_OFFSET;

	pstDptx->bSpreadSpectrum_Clock          = (bool)true;
	pstDptx->bEstablish_Timing_Present      = (bool)false;
	pstDptx->bSideBand_MSG_Supported        = pstInit_Params->bsideband_msg;
	pstDptx->bSdm_Bypass					= pstInit_Params->bsdm_bypass;
	pstDptx->bTrvc_Bypass					= pstInit_Params->btrvc_bypass;
	pstDptx->bPhy_Lane_Std					= pstInit_Params->bphy_lane_swap;

	pstDptx->ucMultiPixel                   = (uint8_t)DPTX_MP_SINGLE_PIXEL;
	pstDptx->ucMax_Rate                     = pstInit_Params->ucMaxRate;
	pstDptx->ucMax_Lanes                    = pstInit_Params->ucMaxLanes;
	pstDptx->ucPHY_Ref_Clk                  = (uint8_t)PHY_REF_CLK_DIRECT_XIN;
	pstDptx->eEstablished_Timing            = DMT_NONE;
	pstDptx->ePhy_Dev						= pstInit_Params->ePhy_Dev_model;

	(void)memcpy((void *)pstDptx->aucMuxId, (void *)pstInit_Params->aucMux_Id, sizeof(uint8_t) * PHY_INPUT_STREAM_MAX);
	(void)memcpy((void *)pstDptx->aucVCP_Id, (void *)pstInit_Params->aucVcp_Id, sizeof(uint8_t) * PHY_INPUT_STREAM_MAX);
	(void)memcpy((void *)pstDptx->auiVIC, (void *)pstInit_Params->auiVIC, sizeof(uint32_t) * PHY_INPUT_STREAM_MAX);

	*ppstDptx = pstDptx;

return_funcs:
	return iRetVal;

}

int32_t Dptx_V14_Init(struct Dptx_Init_Params *pstDptx_Init_Params)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstDptx = NULL;

	(void)dptx_v14_init_params(pstDptx_Init_Params, &pstDptx);

	dptx_v14_init_pre_condition(pstDptx);

	iRetVal = dptx_v14_init_wrap(pstDptx);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	(void)Dptx_Core_Init(pstDptx);

	Dptx_Vidin_Init(pstDptx, pstDptx_Init_Params->ucPixelEn_Type);

	dptx_info("\n");
	dptx_info("DP Ver %u.%u.%u : %u DPs enabled",
				(uint32_t)TCC_DPTX_DRV_MAJOR_VER,
				(uint32_t)TCC_DPTX_DRV_MINOR_VER,
				(uint32_t)TCC_DPTX_DRV_SUBTITLE_VER,
				pstDptx->ucNumOfStreams);
	dptx_info(" PHY model = %s ",
				(pstDptx->ePhy_Dev == PHY_DEVICE_SNPS) ? "Synopsys" : "Samsung");
	dptx_info(" Side band message = %s ",
				(pstDptx->bSideBand_MSG_Supported == (bool)true) ? "supported" : "not supported");
	dptx_info(" SDM Bypass = %s, TRVC Bypass = %s ",
				(pstDptx->bSdm_Bypass == (bool)true) ? "on" : "off",
				(pstDptx->bTrvc_Bypass == (bool)true) ? "on" : "off");
	dptx_info(" PHY Lane cfg = %s", (pstDptx->bPhy_Lane_Std == (bool)true) ? "Standard" : "Swap");
	dptx_info(" Max rate = %s, Max lane = %s",
				(pstDptx->ucMax_Rate == (uint8_t)DPTX_PHYIF_CTRL_RATE_RBR) ? "RBR" :
				(pstDptx->ucMax_Rate == (uint8_t)DPTX_PHYIF_CTRL_RATE_HBR) ? "HBR" :
				(pstDptx->ucMax_Rate == (uint8_t)DPTX_PHYIF_CTRL_RATE_HBR2) ? "HBR2" :"HBR3",
				(pstDptx->ucMax_Lanes == (uint8_t)PHY_LANE_1) ? "1 lane" :
				(pstDptx->ucMax_Lanes == (uint8_t)PHY_LANE_2) ? "2 lanes" :"4 lanes");
	dptx_info(" Encoding type = %s",
				(pstDptx_Init_Params->ucPixelEn_Type == (uint8_t)0U) ? "RGB" :
				(pstDptx_Init_Params->ucPixelEn_Type == (uint8_t)1U) ? "YCbCr222" :"YCbCr444");
	dptx_info(" Pixel mode = %s",
				(pstDptx->ucMultiPixel == (uint8_t)DPTX_MP_SINGLE_PIXEL) ? "Single" :
				(pstDptx->ucMultiPixel == (uint8_t)DPTX_MP_DUAL_PIXEL) ? "Dual":"Quad");
	dptx_info(" Spread Spectrum Clock = %s", (pstDptx->bSpreadSpectrum_Clock == (bool)true) ? "On" : "Off");
	dptx_info(" Vcp id : %u %u %u %u", pstDptx->aucVCP_Id[0], pstDptx->aucVCP_Id[1], pstDptx->aucVCP_Id[2], pstDptx->aucVCP_Id[3]);
	dptx_info(" VIC : %u %u %u %u", pstDptx->auiVIC[0], pstDptx->auiVIC[1], pstDptx->auiVIC[2], pstDptx->auiVIC[3]);
	dptx_info(" Mux id : %u %u %u %u", pstDptx->aucMuxId[0], 	pstDptx->aucMuxId[1], pstDptx->aucMuxId[2], pstDptx->aucMuxId[3]);

return_funcs:
	return iRetVal;
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
			/* coverity[misra_c_2012_rule_12_3_violation : FALSE] */
			free(pstDptx_Handle->pucEdidBuf);

			pstDptx_Handle->pucEdidBuf = NULL;
		}

		/* coverity[misra_c_2012_rule_12_3_violation : FALSE] */
		free(pstDptx_Handle);
	}
}

struct Dptx_Params *Dptx_V14_Get_Device_Handle(void)
{
	return pstDrv_Handle;
}


