// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/


#include <asm/io.h>
#include <linux/delay.h>

#include "dptx_v14.h"
#include "dptx_reg.h"
#include "dptx_dbg.h"

#define MAX_TRY_PLL_LOCK 10

#define CHECK_REG_OFFSET(x) (((x) < (uint32_t)DP_MAX_OFFSET) ? (bool)true : (bool)false)

void Dptx_Clk_Reset_PLL(struct Dptx_Params *pstDptx)
{
	bool bOffsetInRange;
	uint32_t uiRegMap_PLLPMS, uiReg_Offset;

	bOffsetInRange = CHECK_REG_OFFSET(pstDptx->uiCKC_RegAddr_Offset);
	if (!bOffsetInRange) {
		dptx_err("Invalid reg offset as 0x%x", pstDptx->uiCKC_RegAddr_Offset);

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	uiReg_Offset = (uint32_t)(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKCTRL0);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, (u32)0x00);

	uiReg_Offset = (uint32_t)(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKCTRL1);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, (u32)0x00);

	uiReg_Offset = (uint32_t)(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKCTRL2);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, (u32)0x00);

	uiReg_Offset = (uint32_t)(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKCTRL3);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, (u32)0x00);

	uiReg_Offset = (uint32_t)(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_PLLPMS);
	uiRegMap_PLLPMS = Dptx_Reg_Readl(pstDptx, uiReg_Offset);
	uiRegMap_PLLPMS = (uiRegMap_PLLPMS | (uint32_t)PLLPMS_BYPASS_MASK);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, uiRegMap_PLLPMS);

	uiReg_Offset = (uint32_t)(pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_PLLPMS);
	uiRegMap_PLLPMS = Dptx_Reg_Readl(pstDptx, uiReg_Offset);
	uiRegMap_PLLPMS = (uiRegMap_PLLPMS & ~((uint32_t)PLLPMS_RESETB_MASK));
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, uiRegMap_PLLPMS);

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dptx_debug("Restting Clk...");

return_funcs:
	return;
}

void Dptx_Clk_Set_PLL_Divisor(struct Dptx_Params *pstDptx)
{
	bool bOffsetInRange;
	uint32_t uiReg_Offset;

	bOffsetInRange = CHECK_REG_OFFSET(pstDptx->uiCKC_RegAddr_Offset);
	if (!bOffsetInRange) {
		dptx_err("Invalid reg offset as 0x%x", pstDptx->uiCKC_RegAddr_Offset);

		goto return_funcs;
	}

	uiReg_Offset = (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_PLLCON);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, 0x00000FC0U);

	uiReg_Offset = (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_PLLMON);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, 0x00008800U);

	uiReg_Offset = (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKDIVC0);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, (uint32_t)DIV_CFG_CLK_200HMZ);
//	Dptx_Reg_Writel(pstDptx, uiReg_Offset, (uint32_t)0x00000082U);

	uiReg_Offset = (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKDIVC1);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, (uint32_t)DIV_CFG_CLK_160HMZ);
	//Dptx_Reg_Writel(pstDptx, uiReg_Offset, (uint32_t)0x000000B1U);

	uiReg_Offset = (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKDIVC2);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, (uint32_t)DIV_CFG_CLK_100HMZ);
	//Dptx_Reg_Writel(pstDptx, uiReg_Offset, (uint32_t)0x00000085U);
	
	uiReg_Offset = (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKDIVC3);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, (uint32_t)DIV_CFG_CLK_40HMZ);
	//Dptx_Reg_Writel(pstDptx, uiReg_Offset, (uint32_t)0x00000098U);

	uiReg_Offset = (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_PLLPMS);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, 0x05026403U);

	uiReg_Offset = (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_PLLPMS);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, 0x85026403U);

	dptx_debug("Set Clk divisor...");

return_funcs:
	return;
}

void Dptx_Clk_Set_PLL_ClkSrc(struct Dptx_Params *pstDptx, uint8_t ucClockSource)
{
	bool bOffsetInRange;
	uint32_t uiRegMap_Val = 0, uiReg_Offset;

	bOffsetInRange = CHECK_REG_OFFSET(pstDptx->uiCKC_RegAddr_Offset);
	if (!bOffsetInRange) {
		dptx_err("Invalid reg offset as 0x%x", pstDptx->uiCKC_RegAddr_Offset);

		goto return_funcs;
	}

	uiRegMap_Val |= ucClockSource;

	uiReg_Offset = (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKCTRL0);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, uiRegMap_Val);

	uiReg_Offset = (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKCTRL1);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, uiRegMap_Val);

	uiReg_Offset = (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKCTRL2);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, uiRegMap_Val);

	uiReg_Offset = (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_CLKCTRL3);
	Dptx_Reg_Writel(pstDptx, uiReg_Offset, uiRegMap_Val);

	dptx_debug("Set PLL Clk source...");

return_funcs:
	return;
}

void Dptx_Clk_Get_PLLLock_Status(struct Dptx_Params *pstDptx, uint8_t *pucPll_Locked)
{
	bool bPllLock, bOffsetInRange;
	uint8_t ucCount = 0;
	uint32_t uiRegMap_PllPMS;

	bOffsetInRange = CHECK_REG_OFFSET(pstDptx->uiCKC_RegAddr_Offset);
	if (!bOffsetInRange) {
		dptx_err("Invalid reg offset as 0x%x", pstDptx->uiCKC_RegAddr_Offset);

		goto return_funcs;
	}

	do {
		uiRegMap_PllPMS = Dptx_Reg_Readl(pstDptx, (pstDptx->uiCKC_RegAddr_Offset + (uint32_t)DPTX_CKC_CFG_PLLPMS));

		bPllLock = ((uiRegMap_PllPMS & (uint32_t)DPTX_PLLPMS_LOCK_MASK) == 0U) ? (bool)false : (bool)true;

		if (bPllLock == (bool)false) {
			/* For coverity */
			mdelay(1);
		}

		ucCount++;
	} while ((bPllLock == (bool)false) && (ucCount < (uint8_t)MAX_TRY_PLL_LOCK));

	if (bPllLock == (bool)true) {
		*pucPll_Locked = 1U;

		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_debug("Success to get PLL Locking after %u ms", ucCount);
	} else {
		*pucPll_Locked = 0U;

		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dptx_err("Fail to get PLL Locking");
	}

return_funcs:
	return;
}


