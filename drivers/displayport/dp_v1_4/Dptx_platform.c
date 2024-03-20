// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/

#include <linux/delay.h>

#include "Dptx_v14.h"
#include "Dptx_reg.h"
#include "Dptx_drm_dp_addition.h"
#include "Dptx_dbg.h"

#define MAX_TRY_PLL_LOCK 10

#define CHECK_REG_OFFSET(x) (((x) < (uint32_t)DP_MAX_OFFSET) ? (bool)true : (bool)false)

static struct Dptx_Params *pstDrv_Handle;

int32_t Dptx_Platform_Init(uint8_t ucNumOfStreams, uint8_t ucLink_Rate, uint8_t ucLink_Lanes)
{
	uint8_t ucPLL_LockStatus;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Params *pstDptx = NULL;

	if (ucNumOfStreams > (uint8_t)PHY_INPUT_STREAM_MAX) {
		dptx_err("Invalid parameter as ucNumOfStreams == %d",
					ucNumOfStreams);
		iRetVal = DPTX_RETURN_EINVAL;
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		pstDptx = (struct Dptx_Params *)malloc(sizeof(*pstDptx));
		if (pstDptx != NULL) {
			/* For coverity */
			(void)memset(pstDptx, 0, sizeof(*pstDptx));
		} else {
			dptx_err("failed to alloc memory");
			iRetVal = DPTX_RETURN_ENOMEM;
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		pstDptx->pucEdidBuf = (uint8_t *)malloc((size_t)DPTX_EDID_BUFLEN);
		if (pstDptx->pucEdidBuf == NULL) {
			dptx_err("failed to alloc EDID memory");
			iRetVal = DPTX_RETURN_ENOMEM;
		} else {
			pstDptx->pucSecondary_EDID = (uint8_t *)malloc((size_t)DPTX_EDID_BUFLEN);
			if (pstDptx->pucSecondary_EDID == NULL) {
				dptx_err("failed to alloc secondary EDID memory");
				iRetVal = DPTX_RETURN_ENOMEM;
			}
		}
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		pstDptx->ucNumOfStreams                 = ucNumOfStreams;
		pstDptx->bMultStreamTransport           =
				(ucNumOfStreams > 1U) ? (bool)true : (bool)false;

		pstDptx->uiDPLink_BaseAddr              = (uint32_t)DP_DDIBUS_BASE_REG_ADDRESS;
		pstDptx->uiHDCP22_RegAddr_Offset        = (uint32_t)DP_HDCP_OFFSET;
		pstDptx->uiRegBank_RegAddr_Offset       = (uint32_t)DP_REGISTER_BANK_OFFSET;
		pstDptx->uiCKC_RegAddr_Offset           = (uint32_t)DP_CKC_OFFSET;
		pstDptx->uiProtect_RegAddr_Offset       = (uint32_t)DP_PROTECT_OFFSET;

		pstDptx->bSpreadSpectrum_Clock          = (bool)true;
		pstDptx->bEstablish_Timing_Present      = (bool)false;
		pstDptx->bPanelDisplay_Mode                     = (bool)true;
		pstDptx->bSideBand_MSG_Supported        = (bool)false;
		pstDptx->ucMultiPixel                   = (uint8_t)DPTX_MP_SINGLE_PIXEL;
		pstDptx->eEstablished_Timing            = DMT_NONE;
		pstDptx->ucMax_Rate                     = ucLink_Rate;
		pstDptx->ucMax_Lanes                    = ucLink_Lanes;
		pstDptx->ucPHY_Ref_Clk                  = (uint8_t)PHY_REF_CLK_DIRECT_XIN;

		pstDptx->aucVCP_Id[PHY_INPUT_STREAM_0]  = 1U;
		pstDptx->aucVCP_Id[PHY_INPUT_STREAM_1]  = 2U;
		pstDptx->aucVCP_Id[PHY_INPUT_STREAM_2]  = 3U;
		pstDptx->aucVCP_Id[PHY_INPUT_STREAM_3]  = 4U;

		dptx_notice("DP Ver %d.%d.%d : ",
					(uint32_t)TCC_DPTX_DRV_MAJOR_VER,
					(uint32_t)TCC_DPTX_DRV_MINOR_VER,
					(uint32_t)TCC_DPTX_DRV_SUBTITLE_VER);
		dptx_notice(" -.Register access mode to AP");
		dptx_notice(" -.The number of lanes = %d ", (uint32_t)pstDptx->ucMax_Lanes);
		dptx_notice(" -.Max rate = %s ",
					(pstDptx->ucMax_Rate == (uint8_t)DPTX_PHYIF_CTRL_RATE_RBR) ? "RBR" :
					(pstDptx->ucMax_Rate == (uint8_t)DPTX_PHYIF_CTRL_RATE_HBR) ?  "HBR" :
					(pstDptx->ucMax_Rate == (uint8_t)DPTX_PHYIF_CTRL_RATE_HBR2) ? "HB2":"HBR3");
		dptx_notice(" -.MST = %s -> %d streams ",
					(pstDptx->bMultStreamTransport == (bool)true) ? "On" : "Off",
					(u32)pstDptx->ucNumOfStreams);
		dptx_notice(" -.Pixel mode = %s ",
					(pstDptx->ucMultiPixel == (uint8_t)DPTX_MP_SINGLE_PIXEL) ? "Single" :
					(pstDptx->ucMultiPixel == (uint8_t)DPTX_MP_DUAL_PIXEL) ? "Dual":"Quad");
		dptx_notice(" -.Spread Spectrum Clock = %s ",
					(pstDptx->bSpreadSpectrum_Clock == (bool)true) ? "On" : "Off");
	}


	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Platform_Set_RegisterAccess_Mode(
					pstDptx,
					(bool)DPTX_REGISTER_ACCESS_DDIBUS);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Platform_Set_PW(pstDptx);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Platform_Set_CfgLock(pstDptx, true);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Platform_Set_CfgAccess(pstDptx, true);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Platform_Set_PLL_Divisor(pstDptx);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Platform_Get_PLLLock_Status(pstDptx, &ucPLL_LockStatus);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Platform_Set_PLL_ClockSource(
					pstDptx,
					(u8)CLKCTRL_PLL_DIVIDER_OUTPUT);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		iRetVal = Dptx_Platform_Set_RegisterBank(pstDptx, (enum PHY_RATE)pstDptx->ucMax_Rate);
	}

	if (iRetVal == DPTX_RETURN_NO_ERROR) {
		/* For coverity */
		pstDrv_Handle = pstDptx;
	} else {
		/* For coverity */
		Dptx_Platform_Free_Handle(pstDptx);
	}

	return iRetVal;
}

int32_t Dptx_Platform_Deinit(struct Dptx_Params *pstDptx)
{
	Dptx_Platform_Free_Handle(pstDptx);

	pstDrv_Handle = NULL;

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Platform_Set_RegisterAccess_Mode(
			struct Dptx_Params *pstDptx,
			bool bAP_Accessable)
{
	uint32_t uiRegAddr;
	unsigned int uiRegMap_R_APBSel = 0, uiRegMap_W_APBSel = 0;

	uiRegAddr = (uint32_t)DPTX_APB_SEL_REG;

	uiRegMap_R_APBSel = Dptx_Reg_Direct_Read(uiRegAddr);

	if (bAP_Accessable) {
		/* For coverity */
		pstDptx->uiDPLink_BaseAddr = (uint32_t)DP_DDIBUS_BASE_REG_ADDRESS;
		uiRegMap_W_APBSel = (uiRegMap_R_APBSel | (uint32_t)DPTX_APB_SEL_MASK);
	} else {
		/* For coverity */
		pstDptx->uiDPLink_BaseAddr = (uint32_t)DP_MICOM_BASE_REG_ADDRESS;
		uiRegMap_W_APBSel = (uiRegMap_R_APBSel & ~((uint32_t)DPTX_APB_SEL_MASK));
	}

	Dptx_Reg_Direct_Write(uiRegAddr, uiRegMap_W_APBSel);

	dptx_dbg("APB Sel-> %s, Reg[0x%p]: 0x%08x->0x%08x\n",
				bAP_Accessable ? "AP":"Micom",
				uiRegAddr,
				uiRegMap_R_APBSel,
				uiRegMap_W_APBSel);

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Platform_Set_PW(struct Dptx_Params *pstDptx)
{
	bool bOffsetInRange;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;

	bOffsetInRange = CHECK_REG_OFFSET(pstDptx->uiProtect_RegAddr_Offset);

	if (bOffsetInRange == (bool)true) {
		Dptx_Reg_Writel(pstDptx,
						(uint32_t)(pstDptx->uiProtect_RegAddr_Offset +
						(uint32_t)DP_PORTECT_CFG_PW_OK),
						(uint32_t)DP_PORTECT_CFG_PW_VAL);
	} else {
		/* For coverity */
		iRetVal = DPTX_RETURN_EINVAL;
	}

	return iRetVal;
}

int32_t Dptx_Platform_Set_CfgAccess(
			struct Dptx_Params *pstDptx,
			bool bAccessable)
{
	bool bOffsetInRange;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;

	bOffsetInRange = CHECK_REG_OFFSET(pstDptx->uiProtect_RegAddr_Offset);

	if (bOffsetInRange == (bool)true) {
		if (bAccessable == (bool)true) {
			/* For coverity */
			Dptx_Reg_Writel(pstDptx,
						(uint32_t)(pstDptx->uiProtect_RegAddr_Offset +
						(uint32_t)DP_PORTECT_CFG_ACCESS),
						(uint32_t)DP_PORTECT_CFG_ACCESSABLE);
		} else {
			/* For coverity */
			Dptx_Reg_Writel(pstDptx,
						(uint32_t)(pstDptx->uiProtect_RegAddr_Offset +
						(uint32_t)DP_PORTECT_CFG_ACCESS),
						(uint32_t)DP_PORTECT_CFG_NOT_ACCESSABLE);
		}
	} else {
		/* For coverity */
		iRetVal = DPTX_RETURN_EINVAL;
	}

	return iRetVal;
}

int32_t Dptx_Platform_Set_CfgLock(
			struct Dptx_Params *pstDptx,
			bool bAccessable)
{
	bool bOffsetInRange;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;

	bOffsetInRange = CHECK_REG_OFFSET(pstDptx->uiProtect_RegAddr_Offset);

	if (bOffsetInRange == (bool)true) {
		if (bAccessable == (bool)true) {
			/* For coverity */
			Dptx_Reg_Writel(pstDptx,
						(uint32_t)(pstDptx->uiProtect_RegAddr_Offset +
						(uint32_t)DP_PORTECT_CFG_LOCK),
						(uint32_t)DP_PORTECT_CFG_UNLOCKED);
		} else {
			/* For coverity */
			Dptx_Reg_Writel(pstDptx,
						(uint32_t)(pstDptx->uiProtect_RegAddr_Offset +
						(uint32_t)DP_PORTECT_CFG_LOCK),
						(uint32_t)DP_PORTECT_CFG_LOCKED);
		}
	} else {
		/* For coverity */
		iRetVal = DPTX_RETURN_EINVAL;
	}

	return iRetVal;
}

int32_t Dptx_Platform_Set_PLL_Divisor(struct Dptx_Params *pstDptx)
{
	bool bOffsetInRange;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;

	bOffsetInRange = CHECK_REG_OFFSET(pstDptx->uiCKC_RegAddr_Offset);

	if (bOffsetInRange == (bool)true) {
		Dptx_Reg_Writel(pstDptx,
				(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_PLLCON),
				0x00000FC0U);
		Dptx_Reg_Writel(pstDptx,
				(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_PLLMON),
				0x00008800U);
		Dptx_Reg_Writel(pstDptx,
			(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKDIVC0),
			(uint32_t)DIV_CFG_CLK_200HMZ);
		Dptx_Reg_Writel(pstDptx,
			(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKDIVC1),
			(uint32_t)DIV_CFG_CLK_160HMZ);
		Dptx_Reg_Writel(pstDptx,
			(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKDIVC2),
			(uint32_t)DIV_CFG_CLK_100HMZ);
		Dptx_Reg_Writel(pstDptx,
			(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKDIVC3),
			(uint32_t)DIV_CFG_CLK_40HMZ);
		Dptx_Reg_Writel(pstDptx,
			(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_PLLPMS),
			0x05026403U);
		Dptx_Reg_Writel(pstDptx,
			(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_PLLPMS),
			0x85026403U);
	} else {
		/* For coverity */
		iRetVal = DPTX_RETURN_EINVAL;
	}

	return iRetVal;
}

int32_t Dptx_Platform_Set_PLL_ClockSource(
			struct Dptx_Params *pstDptx,
			uint8_t ucClockSource)
{
	bool bOffsetInRange;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiRegMap_Val = 0;

	bOffsetInRange = CHECK_REG_OFFSET(pstDptx->uiCKC_RegAddr_Offset);

	uiRegMap_Val |= ucClockSource;

	if (bOffsetInRange == (bool)true) {
		Dptx_Reg_Writel(pstDptx,
			(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKCTRL0),
			uiRegMap_Val);
		Dptx_Reg_Writel(pstDptx,
			(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKCTRL1),
			uiRegMap_Val);
		Dptx_Reg_Writel(pstDptx,
			(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKCTRL2),
			uiRegMap_Val);
		Dptx_Reg_Writel(pstDptx,
			(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKCTRL3),
			uiRegMap_Val);
	} else {
		/* For coverity */
		iRetVal = DPTX_RETURN_EINVAL;
	}

	return iRetVal;
}

int32_t Dptx_Platform_Get_PLLLock_Status(
			struct Dptx_Params *pstDptx,
			uint8_t *pucPll_Locked)
{
	bool bPllLock, bOffsetInRange;
	uint8_t ucCount = 0;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiRegMap_PllPMS;

	bOffsetInRange = CHECK_REG_OFFSET(pstDptx->uiCKC_RegAddr_Offset);

	if (bOffsetInRange == (bool)true) {
		do {
			uiRegMap_PllPMS = Dptx_Reg_Readl(pstDptx,
			(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_PLLPMS));

			bPllLock =
			((uiRegMap_PllPMS & (uint32_t)DPTX_PLLPMS_LOCK_MASK) != 0U) ? (bool)true : (bool)false;

			if (bPllLock == (bool)false) {
				/* For coverity */
				mdelay(1);
			}

			ucCount++;
		} while ((bPllLock == (bool)false) && (ucCount < (uint8_t)MAX_TRY_PLL_LOCK));

		if (bPllLock == (bool)true) {
			*pucPll_Locked = 1U;
			dptx_dbg("Success to get PLL Locking after %dms",
					(uint32_t)(ucCount + 1U));
		} else {
			*pucPll_Locked = 0U;
			dptx_err("Fail to get PLL Locking");
		}
	}else {
		/* For coverity */
		iRetVal = DPTX_RETURN_EINVAL;
	}

	return iRetVal;
}

int32_t Dptx_Platform_Set_RegisterBank(
			struct Dptx_Params *pstDptx,
			enum PHY_RATE eLinkRate)
{
	bool bOffsetInRange;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiReg_R_data, uiReg_W_data;

	bOffsetInRange = CHECK_REG_OFFSET(pstDptx->uiRegBank_RegAddr_Offset);

	if (bOffsetInRange == (bool)true) {
		uiReg_R_data = Dptx_Reg_Readl(pstDptx,
					(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
					(uint32_t)DP_REGISTER_BANK_REG_22));

		if ((uiReg_R_data & (uint32_t)AXI_SLAVE_BRIDGE_RST_MASK) != 0U) {
			/* For coverity */
			uiReg_W_data = (uint32_t)0x00000008U;
		} else {
			/* For coverity */
			uiReg_W_data = (uint32_t)0x00000000U;
		}

		dptx_notice("Writing REG_22[0x12480058]:0x%x -> 0x%x",
					uiReg_R_data,
					uiReg_W_data);

		switch (eLinkRate) {
		case RATE_RBR:
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_0),
				(uint32_t)0x004D0000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_20),
				(uint32_t)0x00002501U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_11),
				(uint32_t)0x00000401U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_12),
				(uint32_t)0x00000000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_13),
				(uint32_t)0x00000000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_14),
				(uint32_t)0x00000000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_16),
				(uint32_t)0x00000000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_15),
				(uint32_t)0x08080808U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_17),
				(uint32_t)0x10000000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_20),
				(uint32_t)0x00002501U);

			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_21),
				(uint32_t)pstDptx->ucPHY_Ref_Clk);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_23),
				(uint32_t)0x00000008U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_22),
				(uint32_t)0x00000801U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_22),
				(uint32_t)uiReg_W_data);
			break;
		case RATE_HBR:
		case RATE_HBR2:
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_0),
				(uint32_t)0x002D0000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_21),
				(uint32_t)pstDptx->ucPHY_Ref_Clk);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_20),
				(uint32_t)0x00002501U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_1),
				(uint32_t)0xA00F0001U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_2),
				(uint32_t)0x00A80122U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_3),
				(uint32_t)0x001E8A00U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_4),
				(uint32_t)0x00000004U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_7),
				(uint32_t)0x00000C0CU);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_8),
				(uint32_t)0x05460546);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_9),
				(uint32_t)0x00000011U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_10),
				(uint32_t)0x00700000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_11),
				(uint32_t)0x00000401U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_13),
				(uint32_t)0xA8000122U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_14),
				(uint32_t)0x418A001EU);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_16),
				(uint32_t)0x00004000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_15),
				(uint32_t)0x0B000000);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_17),
				(uint32_t)0x10000000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_20),
				(uint32_t)0x00002501U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_10),
				(uint32_t)0x08700000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_12),
				(uint32_t)0xA00F0003U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_23),
				(uint32_t)0x00000008U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_22),
				(uint32_t)0x00000801U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_22),
				(uint32_t)uiReg_W_data);
			break;
		case RATE_HBR3:
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_0),
				(uint32_t)0x002D0000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_21),
				(uint32_t)pstDptx->ucPHY_Ref_Clk);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_1),
				(uint32_t)0xA00F0001U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_2),
				(uint32_t)0x00A80122U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_3),
				(uint32_t)0x001E8A00U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_4),
				(uint32_t)0x00000004U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_7),
				(uint32_t)0x00000C0CU);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_8),
				(uint32_t)0x05460546U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_9),
				(uint32_t)0x00000011U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_10),
				(uint32_t)0x00700000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_11),
				(uint32_t)0x00000401U);

			if (pstDptx->bSpreadSpectrum_Clock) {
				Dptx_Reg_Writel(pstDptx,
					(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
					(uint32_t)DP_REGISTER_BANK_REG_13),
					(uint32_t)0xA8000616U);
				Dptx_Reg_Writel(pstDptx,
					(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
					(uint32_t)DP_REGISTER_BANK_REG_14),
					(uint32_t)0x115C0045U);
			} else {
				Dptx_Reg_Writel(pstDptx,
					(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
					(uint32_t)DP_REGISTER_BANK_REG_13),
					(uint32_t)0xA90003F8U);
				Dptx_Reg_Writel(pstDptx,
					(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
					(uint32_t)DP_REGISTER_BANK_REG_14),
					(uint32_t)0x11000000U);
			}

			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_16),
				(uint32_t)0x00004000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_15),
				(uint32_t)0x8B000000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_17),
				(uint32_t)0x10000000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_20),
				(uint32_t)0x00002501U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_10),
				(uint32_t)0x08700000U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_12),
				(uint32_t)0xA00F0003U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_23),
				(uint32_t)0x00000008U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_22),
				(uint32_t)0x00000801U);
			Dptx_Reg_Writel(pstDptx,
				(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
				(uint32_t)DP_REGISTER_BANK_REG_22),
				(uint32_t)uiReg_W_data);
			break;
		default:
			dptx_err("Invalid PHY rate %d\n", eLinkRate);
			break;
		} 
	} else {
	/* For coverity */
	iRetVal = DPTX_RETURN_EINVAL;
	}

	return iRetVal;
}

int32_t Dptx_Platform_Set_Tx_EQ(
		struct Dptx_Params *pstDptx,
		uint32_t uiEQ_Main,
		uint32_t uiEQ_Post,
		uint32_t uiEQ_Pre,
		uint8_t ucEQ_VBoost_7)
{
	bool bOffsetInRange;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiTx_EQ_Main = 0, uiTx_EQ_Post = 0;
	uint32_t uiTx_EQ_Pre = 0, uiTx_EQ_PrePost = 0;

	bOffsetInRange = CHECK_REG_OFFSET(pstDptx->uiRegBank_RegAddr_Offset);

	if (bOffsetInRange == (bool)true) {

		uiTx_EQ_Main = ((uint32_t)OVERRIDE_SIGNAL_Q4_MASK |
						(uint32_t)OVERRIDE_SIGNAL_Q3_MASK |
						(uint32_t)OVERRIDE_SIGNAL_Q2_MASK |
						(uint32_t)OVERRIDE_SIGNAL_Q1_MASK);

		uiTx_EQ_Main |= uiEQ_Main;
		uiTx_EQ_Post = uiEQ_Post;
		uiTx_EQ_Pre = uiEQ_Pre;
		uiTx_EQ_PrePost = (uiTx_EQ_Post | uiTx_EQ_Pre);

		dptx_notice("Set EQ as EQ Main 0x%x, EQ PrePost 0x%x, Vboost %s",
					uiTx_EQ_Main,
					uiTx_EQ_PrePost,
					(ucEQ_VBoost_7 != 0U) ? "Vboost 7" : "Vboost 3");

		Dptx_Reg_Writel(pstDptx,
					(pstDptx->uiRegBank_RegAddr_Offset +
					(uint32_t)DP_REGISTER_BANK_REG_16),
					uiTx_EQ_PrePost);
		Dptx_Reg_Writel(pstDptx,
					(pstDptx->uiRegBank_RegAddr_Offset +
					(uint32_t)DP_REGISTER_BANK_REG_15),
					uiTx_EQ_Main);

		if (ucEQ_VBoost_7 != 0U) {
			Dptx_Reg_Writel(pstDptx,
						(uint32_t)(pstDptx->uiRegBank_RegAddr_Offset +
						(uint32_t)DP_REGISTER_BANK_REG_10),
						(uint32_t)0x08700000U);
		} else {
			Dptx_Reg_Writel(pstDptx,
						(u32)(pstDptx->uiRegBank_RegAddr_Offset +
						(uint32_t)DP_REGISTER_BANK_REG_10),
						(uint32_t)0x08300000U);
		}
	} else {
		/* For coverity */
		iRetVal = DPTX_RETURN_EINVAL;
	}

	return iRetVal;
}

void Dptx_Platform_Free_Handle(struct Dptx_Params       *pstDptx_Handle)
{
	if (pstDptx_Handle != NULL) {
		if (pstDptx_Handle->pucSecondary_EDID != NULL) {
			free(pstDptx_Handle->pucSecondary_EDID);
			pstDptx_Handle->pucSecondary_EDID = NULL;
		}

		if (pstDptx_Handle->pucEdidBuf != NULL) {
			free(pstDptx_Handle->pucEdidBuf);
			pstDptx_Handle->pucEdidBuf = NULL;
		}

		free(pstDptx_Handle);
	}
}

struct Dptx_Params *Dptx_Platform_Get_Device_Handle(void)
{
	return pstDrv_Handle;
}

