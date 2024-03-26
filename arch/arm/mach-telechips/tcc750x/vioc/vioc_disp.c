// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/vioc_disp.h>
#include <asm/arch/vioc/vioc_global.h>
#include <asm/arch/vioc/reg_physical.h>

void VIOC_DISP_SetSwapbf(void __iomem *reg, unsigned int swapbf)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = __raw_readl(reg + DALIGN) & ~(DALIGN_SWAPBF_MASK);
	value |= (swapbf << DALIGN_SWAPBF_SHIFT);
	__raw_writel(value, reg + DALIGN);
}

void VIOC_DISP_GetSwapbf(void __iomem *reg, unsigned int *swapbf)
{
	void *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = reg;
	reg = tmp_pDISP;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*swapbf = (__raw_readl(reg + DALIGN) & DALIGN_SWAPBF_MASK)
		>> DALIGN_SWAPBF_SHIFT;
}

void VIOC_DISP_SetSwapaf(void __iomem *reg, unsigned int swapaf)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = __raw_readl(reg + DALIGN) & ~(DALIGN_SWAPAF_MASK);
	value |= (swapaf << DALIGN_SWAPAF_SHIFT);
	__raw_writel(value, reg + DALIGN);
}

void VIOC_DISP_GetSwapaf(void __iomem *reg, unsigned int *swapaf)
{
	void *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = reg;
	reg = tmp_pDISP;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*swapaf = (__raw_readl(reg + DALIGN) & DALIGN_SWAPAF_MASK)
		>> DALIGN_SWAPAF_SHIFT;
}

void VIOC_DISP_GetSize(void __iomem *reg, unsigned int *nWidth,
		       unsigned int *nHeight)
{
	void *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = reg;
	reg = tmp_pDISP;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*nWidth = (__raw_readl(reg + DDS) & DDS_HSIZE_MASK) >>
			DDS_HSIZE_SHIFT;
	*nHeight = (__raw_readl(reg + DDS) & DDS_VSIZE_MASK) >>
			DDS_VSIZE_SHIFT;
}

void VIOC_DISP_SetSize(void __iomem *reg, unsigned int nWidth,
		unsigned int nHeight)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel((nHeight << DDS_VSIZE_SHIFT) | (nWidth << DDS_HSIZE_SHIFT),
			reg + DDS);
}

// BG0 : Red, BG1 : Green , BG2, Blue
void VIOC_DISP_SetBGColor(void __iomem *reg, unsigned int BG0,
		unsigned int BG1, unsigned int BG2, unsigned int BG3)
{
	u32 value;

	value =
		(((BG3 & 0xFFU) << DBC_BG3_SHIFT)
		 | ((BG2 & 0xFFU) << DBC_BG2_SHIFT)
		 | ((BG1 & 0xFFU) << DBC_BG1_SHIFT)
		 | ((BG0 & 0xFFU) << DBC_BG0_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(value, reg + DBC);
}

void VIOC_DISP_SetPosition(void __iomem *reg, unsigned int startX,
		unsigned int startY)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel((startY << DPOS_YPOS_SHIFT) | (startX << DPOS_XPOS_SHIFT),
			reg + DPOS);
}

void VIOC_DISP_GetPosition(void __iomem *reg, unsigned int *startX,
		unsigned int *startY)
{
	void *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = reg;
	reg = tmp_pDISP;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*startX = (__raw_readl(reg + DPOS) & DPOS_XPOS_MASK) >> DPOS_XPOS_SHIFT;
	*startY = (__raw_readl(reg + DPOS) & DPOS_YPOS_MASK) >> DPOS_YPOS_SHIFT;
}


void VIOC_DISP_SetColorEnhancement(void __iomem *reg, signed char contrast,
		signed char brightness, signed char hue)
{
	u32 value = 0;

	/*
	 *	pDISP->uLENH.bREG.CONRAST = contrast;
	 *	pDISP->uLENH.bREG.BRIGHTNESS = brightness;
	 *	pDISP->uLENH.bREG.HUE = hue;
	 */
	if ((contrast < 0) || (brightness < 0) || (hue < 0)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		(void)pr_err("[ERR][DISP] %s parameter is wrong contrast(%d), brightness(%d) hue(%d)\n",
				__func__, contrast, brightness, hue);
	} else {
		value = (DCENH_HUE_MASK & ((u32)hue << DCENH_HUE_SHIFT))
			|(DCENH_BRIGHT_MASK & ((u32)brightness << DCENH_BRIGHT_SHIFT))
			|(DCENH_CONTRAST_MASK & (u32)contrast);
		if (hue == 0) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			__raw_writel(value, reg + DCENH);
		} else {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			__raw_writel(value | DCENH_HEN_MASK, reg + DCENH);
		}
	}
}

void VIOC_DISP_GetColorEnhancement(void __iomem *reg, signed char *contrast,
		signed char  *brightness, signed char *hue)
{
    /*
     *	contrast = pDISP->uLENH.bREG.CONRAST;
     *	brightness = pDISP->uLENH.bREG.BRIGHTNESS;
     *	hue = pDISP->uLENH.bREG.HUE;
     */
     unsigned int temp = 0; /* avoid CERT-C Integers Rule INT31-C */
     void *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = reg;
	reg = tmp_pDISP;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	temp = (__raw_readl(reg + DCENH) & DCENH_CONTRAST_MASK) >>
				DCENH_CONTRAST_SHIFT;
	if (temp <= 0xFFU) {
		*contrast = (signed char)temp;
	}

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	temp = (__raw_readl(reg + DCENH) & DCENH_BRIGHT_MASK) >>
				DCENH_BRIGHT_SHIFT;
	if (temp <= 0xFFU) {
		*brightness = (signed char)temp;
	}

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	temp = (__raw_readl(reg + DCENH) & DCENH_HUE_MASK) >> DCENH_HUE_SHIFT;
	if (temp <= 0xFFU) {
		*hue = (signed char)temp;
	}
}


void VIOC_DISP_SetClippingEnable(void __iomem *reg,
		unsigned int enable)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + DCTRL) & ~(DCTRL_CLEN_MASK));
	value |= (enable << DCTRL_CLEN_SHIFT);
	__raw_writel(value, reg + DCTRL);
}

void VIOC_DISP_GetClippingEnable(void __iomem *reg,
		unsigned int *enable)
{
	void *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = reg;
	reg = tmp_pDISP;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*enable = (__raw_readl(reg + DCTRL) & DCTRL_CLEN_MASK)
		>> DCTRL_CLEN_SHIFT;
}

void VIOC_DISP_SetClipping(void __iomem *reg,
		unsigned int uiUpperLimitY,
		unsigned int uiLowerLimitY,
		unsigned int uiUpperLimitUV,
		unsigned int uiLowerLimitUV)
{
	u32 value;

	value = (uiUpperLimitY << DCPY_CLPH_SHIFT)
		| (uiLowerLimitY << DCPY_CLPL_SHIFT);
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(value, reg + DCPY);

	value = (uiUpperLimitUV << DCPC_CLPH_SHIFT)
		| (uiLowerLimitUV << DCPC_CLPL_SHIFT);
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(value, reg + DCPC);
}

void VIOC_DISP_GetClipping(void __iomem *reg,
		unsigned int *uiUpperLimitY,
		unsigned int *uiLowerLimitY,
		unsigned int *uiUpperLimitUV,
		unsigned int *uiLowerLimitUV)
{
	void *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = reg;
	reg = tmp_pDISP;

	/* avoid MISRA C-2012 Rule 2.7 */
    (void)reg;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*uiUpperLimitY =
		(__raw_readl(reg + DCPY) & DCPY_CLPH_MASK) >> DCPY_CLPH_SHIFT;
	*uiLowerLimitY =
		(__raw_readl(reg + DCPY) & DCPY_CLPL_MASK) >> DCPY_CLPL_SHIFT;
	*uiUpperLimitUV =
		(__raw_readl(reg + DCPC) & DCPC_CLPH_MASK) >> DCPC_CLPH_SHIFT;
	*uiLowerLimitUV =
		(__raw_readl(reg + DCPC) & DCPC_CLPL_MASK) >> DCPC_CLPL_SHIFT;
}

void VIOC_DISP_SetDither(void __iomem *reg, unsigned int ditherEn,
		unsigned int ditherSel, unsigned char mat[4][4])
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value =
		(__raw_readl(reg + DDMAT0)
		 & ~(DDMAT0_DITH03_MASK | DDMAT0_DITH02_MASK
		     | DDMAT0_DITH01_MASK | DDMAT0_DITH00_MASK));
	value |=
		(((u32)mat[0][3] << DDMAT0_DITH03_SHIFT)
		 | ((u32)mat[0][2] << DDMAT0_DITH02_SHIFT)
		 | ((u32)mat[0][1] << DDMAT0_DITH01_SHIFT)
		 | ((u32)mat[0][0] << DDMAT0_DITH00_SHIFT));
	__raw_writel(value, reg + DDMAT0);

	value =
		(__raw_readl(reg + DDMAT0)
		 & ~(DDMAT0_DITH13_MASK | DDMAT0_DITH12_MASK
		     | DDMAT0_DITH11_MASK | DDMAT0_DITH10_MASK));
	value |=
		(((u32)mat[1][3] << DDMAT0_DITH13_SHIFT)
		 | ((u32)mat[1][2] << DDMAT0_DITH12_SHIFT)
		 | ((u32)mat[1][1] << DDMAT0_DITH11_SHIFT)
		 | ((u32)mat[1][0] << DDMAT0_DITH10_SHIFT));
	__raw_writel(value, reg + DDMAT0);

	value =
		(__raw_readl(reg + DDMAT1)
		 & ~(DDMAT1_DITH23_MASK | DDMAT1_DITH22_MASK
		     | DDMAT1_DITH21_MASK | DDMAT1_DITH20_MASK));
	value |=
		(((u32)mat[2][3] << DDMAT1_DITH23_SHIFT)
		 | ((u32)mat[2][2] << DDMAT1_DITH22_SHIFT)
		 | ((u32)mat[2][1] << DDMAT1_DITH21_SHIFT)
		 | ((u32)mat[2][0] << DDMAT1_DITH20_SHIFT));
	__raw_writel(value, reg + DDMAT1);

	value =
		(__raw_readl(reg + DDMAT1)
		 & ~(DDMAT1_DITH33_MASK | DDMAT1_DITH32_MASK
		     | DDMAT1_DITH31_MASK | DDMAT1_DITH30_MASK));
	value |=
		(((u32)mat[3][3] << DDMAT1_DITH33_SHIFT)
		 | ((u32)mat[3][2] << DDMAT1_DITH32_SHIFT)
		 | ((u32)mat[3][1] << DDMAT1_DITH31_SHIFT)
		 | ((u32)mat[3][0] << DDMAT1_DITH30_SHIFT));
	__raw_writel(value, reg + DDMAT1);

	value =
		(__raw_readl(reg + DDITH)
		 & ~(DDITH_DEN_MASK | DDITH_DSEL_MASK));
	value |=
		((ditherEn << DDITH_DEN_SHIFT)
		 | (ditherSel << DDITH_DSEL_SHIFT));
	__raw_writel(value, reg + DDITH);
}

void VIOC_DISP_SetTimingParam(void __iomem *reg, struct
		stLTIMING *pTimeParam)
{
	u32 value;
	struct stLTIMING *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = pTimeParam;
	pTimeParam = tmp_pDISP;

	if ((pTimeParam->lpc < 1U) || (pTimeParam->lswc < 1U) || (pTimeParam->lewc < 1U)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][DISP] %s parameter is wrong lpc(%u), lswc(%u) lewc(%u)\n",
				__func__, pTimeParam->lpc, pTimeParam->lswc, pTimeParam->lewc);
	} else {
		//	Horizon
		value = (((pTimeParam->lpc - 1U) << DHTIME1_LPC_SHIFT) |
				((pTimeParam->lpw - 1U) << DHTIME1_LPW_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, reg + DHTIME1);

		value = (((pTimeParam->lswc - 1U) << DHTIME2_LSWC_SHIFT) |
				((pTimeParam->lewc - 1U) << DHTIME2_LEWC_SHIFT));
		__raw_writel(value, reg + DHTIME2);

		//	Vertical timing
		value = (__raw_readl(reg + DVTIME1) &
				~(DVTIME1_FLC_MASK | DVTIME1_FPW_MASK));
		value |= ((pTimeParam->flc << DVTIME1_FLC_SHIFT) |
				(pTimeParam->fpw << DVTIME1_FPW_SHIFT));
		__raw_writel(value, reg + DVTIME1);

		value = ((pTimeParam->fswc << DVTIME2_FSWC_SHIFT) |
				(pTimeParam->fewc << DVTIME2_FEWC_SHIFT));
		__raw_writel(value, reg + DVTIME2);

		value = ((pTimeParam->flc2 << DVTIME3_FLC_SHIFT) |
				(pTimeParam->fpw2 << DVTIME3_FPW_SHIFT));
		__raw_writel(value, reg + DVTIME3);

		value = ((pTimeParam->fswc2 << DVTIME4_FSWC_SHIFT) |
				(pTimeParam->fewc2 << DVTIME4_FEWC_SHIFT));
		__raw_writel(value, reg + DVTIME4);
	}
}

void VIOC_DISP_SetControlConfigure(void __iomem *reg,
		struct stLCDCTR *pCtrlParam)
{
	u32 value;
	struct stLCDCTR *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = pCtrlParam;
	pCtrlParam = tmp_pDISP;

	value =
		((pCtrlParam->evp << DCTRL_EVP_SHIFT)
		 | (pCtrlParam->evs << DCTRL_EVS_SHIFT)
		 | (pCtrlParam->r2ymd << DCTRL_R2YMD_SHIFT)
		 | (pCtrlParam->advi << DCTRL_ADVI_SHIFT)
		 | (pCtrlParam->ccir656 << DCTRL_656_SHIFT)
		 | (pCtrlParam->ckg << DCTRL_CKG_SHIFT)
		 | ((u32)0x1U << DCTRL_SREQ_SHIFT /* Reset default */)
		 | (pCtrlParam->pxdw << DCTRL_PXDW_SHIFT)
		 | (pCtrlParam->id << DCTRL_ID_SHIFT)
		 | (pCtrlParam->iv << DCTRL_IV_SHIFT)
		 | (pCtrlParam->ih << DCTRL_IH_SHIFT)
		 | (pCtrlParam->ip << DCTRL_IP_SHIFT)
		 | (pCtrlParam->clen << DCTRL_CLEN_SHIFT)
		 | (pCtrlParam->r2y << DCTRL_R2Y_SHIFT)
		 | (pCtrlParam->dp << DCTRL_DP_SHIFT)
		 | (pCtrlParam->ni << DCTRL_NI_SHIFT)
		 | (pCtrlParam->tv << DCTRL_TV_SHIFT)
		 | ((u32)0x1U << DCTRL_SRST_SHIFT /* Auto recovery */)
		 | (pCtrlParam->y2r << DCTRL_Y2R_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(value, reg + DCTRL);
}

void VIOC_DISP_SetR2YMD(void __iomem *reg, unsigned char R2YMD)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + DCTRL) & ~(DCTRL_R2YMD_MASK));
	value |= ((u32)R2YMD << DCTRL_R2YMD_SHIFT);
	__raw_writel(value, reg + DCTRL);
}

void VIOC_DISP_SetR2Y(void __iomem *reg, unsigned char R2Y)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + DCTRL) & ~(DCTRL_R2Y_MASK));
	value |= ((u32)R2Y << DCTRL_R2Y_SHIFT);
	__raw_writel(value, reg + DCTRL);
}

void VIOC_DISP_SetY2RMD(void __iomem *reg, unsigned char Y2RMD)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + DCTRL) & ~(DCTRL_Y2RMD_MASK));
	value |= ((u32)Y2RMD << DCTRL_Y2RMD_SHIFT);
	__raw_writel(value, reg + DCTRL);
}

void VIOC_DISP_SetY2R(void __iomem *reg, unsigned char Y2R)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + DCTRL) & ~(DCTRL_Y2R_MASK));
	value |= ((u32)Y2R << DCTRL_Y2R_SHIFT);
	__raw_writel(value, reg + DCTRL);
}

void VIOC_DISP_SetSWAP(void __iomem *reg, unsigned char SWAP)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + DCTRL) & ~(DCTRL_SWAPBF_MASK));
	value |= ((u32)SWAP << DCTRL_SWAPBF_SHIFT);
	__raw_writel(value, reg + DCTRL);
}

void VIOC_DISP_SetCKG(void __iomem *reg, unsigned char CKG)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + DCTRL) & ~(DCTRL_CKG_MASK));
	value |= ((u32)CKG << DCTRL_CKG_SHIFT);
	__raw_writel(value, reg + DCTRL);
}

int vioc_disp_get_turn_onoff(void __iomem *reg)
{
	void *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = reg;
	reg = tmp_pDISP;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	return ((__raw_readl(reg + DCTRL) & DCTRL_LEN_MASK) != 0U) ? 1 : 0;
}

void VIOC_DISP_TurnOn(void __iomem *reg)
{
	u32 value;

#if CONFIG_IS_ENABLED(EARLYCAMERA_SOLUTION_BOOTSTAGE)
	bootstage_mark_name(BOOTSTAGE_ID_LCDC_ON, "[earlycam]display_lcdc_on");
#endif

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	if ((__raw_readl(reg + DCTRL) & DCTRL_LEN_MASK) != 0U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		//pr_info("[INFO][DISP] %s Display is already enabled\n", __func__);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = (__raw_readl(reg + DCTRL) & ~(DCTRL_LEN_MASK));
		value |= ((u32)0x1U << DCTRL_LEN_SHIFT);
		__raw_writel(value, reg + DCTRL);
	}
}

void VIOC_DISP_TurnOff(void __iomem *reg)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	if ((__raw_readl(reg + DCTRL) & DCTRL_LEN_MASK) == 0U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		//pr_info("[INFO][DISP] %s Display is already disabled\n", __func__);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = (__raw_readl(reg + DCTRL) & ~(DCTRL_LEN_MASK));
		value |= ((u32)0x0U << DCTRL_LEN_SHIFT);
		__raw_writel(value, reg + DCTRL);
	}
}

void VIOC_DISP_SetPXDW(void __iomem *reg, unsigned char PXDW)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + DCTRL) & ~(DCTRL_PXDW_MASK));
	value |= ((u32)PXDW << DCTRL_PXDW_SHIFT);
	__raw_writel(value, reg + DCTRL);
}

void VIOC_DISP_SetControl(void __iomem *reg,
		struct stLCDCPARAM *pLcdParam)
{
	struct stLCDCPARAM *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = pLcdParam;
	pLcdParam = tmp_pDISP;

	/* LCD Controller Stop */
	VIOC_DISP_TurnOff(reg);
	/* LCD Controller CTRL Parameter Set */
	VIOC_DISP_SetControlConfigure(reg, &pLcdParam->LCDCTRL);
	/* LCD Timing Se */
	VIOC_DISP_SetTimingParam(reg, &pLcdParam->LCDCTIMING);
	/* LCD Display Size Set */
	VIOC_DISP_SetSize(reg, pLcdParam->LCDCTIMING.lpc,
			pLcdParam->LCDCTIMING.flc);
	/* LCD Controller Enable */
	VIOC_DISP_TurnOn(reg);
}

/* set 1 : IREQ Masked( interrupt disable), set 0 : IREQ UnMasked( interrput
 * enable)
 */
void VIOC_DISP_SetIreqMask(void __iomem *reg, unsigned int mask,
		unsigned int set)
{
	u32 val = 0U;
	void *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = reg;
	reg = tmp_pDISP;

	if (set == 0U) { /* Interrupt Enable*/
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg + DIM);
		__raw_writel((val & ~(mask)), reg + DIM);
	} else { /* Interrupt Diable*/
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg + DIM);
		__raw_writel(
			((val & ~(mask)) | mask), reg + DIM);
	}
}

/* set 1 : IREQ Masked( interrupt disable), set 0 : IREQ UnMasked( interrput
 * enable)
 */
void VIOC_DISP_SetStatus(void __iomem *reg, unsigned int set)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(set, reg + DSTATUS);
}

void VIOC_DISP_GetStatus(void __iomem *reg, unsigned int *status)
{
	void *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = reg;
	reg = tmp_pDISP;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*status = __raw_readl(reg + DSTATUS);
}

void vioc_disp_clean_fu_status(void __iomem *reg)
{
	VIOC_DISP_SetStatus(reg, DSTATUS_FU_MASK);
}

void vioc_disp_clean_dd_status(void __iomem *reg)
{
	VIOC_DISP_SetStatus(reg, DSTATUS_DD_MASK);
}

int vioc_disp_get_fu_status(void __iomem *reg)
{
	unsigned int status;
	void *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = reg;
	reg = tmp_pDISP;

	VIOC_DISP_GetStatus(reg, &status);
	return ((status & DSTATUS_FU_MASK) != 0U) ? 1 : 0;
}

int vioc_disp_get_dd_status(void __iomem *reg)
{
	unsigned int status;
	void *tmp_pDISP = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDISP = reg;
	reg = tmp_pDISP;

	VIOC_DISP_GetStatus(reg, &status);
	return ((status & DSTATUS_DD_MASK) != 0U) ? 1 : 0;
}

void VIOC_DISP_EmergencyFlagDisable(void __iomem *reg)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + DEFR) & ~(DEFR_MEN_MASK));
	value |= ((u32)0x3U << DEFR_MEN_SHIFT);
	__raw_writel(value, reg + DEFR);
}

void VIOC_DISP_EmergencyFlag_SetEofm(void __iomem *reg,
				     unsigned int eofm)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + DEFR) & ~(DEFR_EOFM_MASK));
	value |= ((eofm & 0x3U) << DEFR_EOFM_SHIFT);
	__raw_writel(value, reg + DEFR);
}

void VIOC_DISP_EmergencyFlag_SetHdmiVs(void __iomem *reg,
				       unsigned int hdmivs)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + DEFR) & ~(DEFR_HDMIVS_MASK));
	value |= ((hdmivs & 0x3U) << DEFR_HDMIVS_SHIFT);
	__raw_writel(value, reg + DEFR);
}

void vioc_disp_set_clkdiv(void __iomem *reg, unsigned int divide)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + DCLKDIV) & ~(DCLKDIV_PXCLKDIV_MASK));
	value |= ((divide << DCLKDIV_PXCLKDIV_SHIFT) & DCLKDIV_PXCLKDIV_MASK);
	__raw_writel(value, reg + DCLKDIV);
}

void VIOC_DISP_DUMP(void __iomem *reg, unsigned int vioc_id)
{
	unsigned int cnt = 0;
	unsigned int dispctrl_val[2];
	void __iomem *pReg = reg;

	if (reg != NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("RAW DUMP - DISP-%u :: 0x%p\n", vioc_id, pReg);
		while (cnt < 0x60U) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("0x%p: 0x%08x\n", pReg + cnt,
					   __raw_readl(pReg + cnt));
			cnt += 0x4U;
		}

		/* DCTRL */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\r\nREADABLE DUMP DISP-%u :: 0x%p\n", vioc_id, pReg);

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		dispctrl_val[0] = __raw_readl(pReg);
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err(" PXDW %u, IH(%s) IV(%s)\r\n",
			(dispctrl_val[0] & DCTRL_PXDW_MASK) >> DCTRL_PXDW_SHIFT,
			((dispctrl_val[0] & DCTRL_IH_MASK) != 0U) ? "LOW":"HIGH",
			((dispctrl_val[0] & DCTRL_IV_MASK) != 0U) ? "LOW":"HIGH");

		/* TIMING */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		dispctrl_val[0] = __raw_readl(pReg + DHTIME1);
		dispctrl_val[1] = __raw_readl(pReg + DHTIME2);
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err(" HFP: %u, HSYNC: %u, HBP: %u, HACTIVE: %u\r\n",
			(((dispctrl_val[1] & DHTIME2_LEWC_MASK)
				>> DHTIME2_LEWC_SHIFT) + 1U),
			(((dispctrl_val[0] & DHTIME1_LPW_MASK)
				>> DHTIME1_LPW_SHIFT) + 1U),
			(((dispctrl_val[1] & DHTIME2_LSWC_MASK)
				>> DHTIME2_LSWC_SHIFT) + 1U),
			(((dispctrl_val[0] & DHTIME1_LPC_MASK)
				>> DHTIME1_LPC_SHIFT) + 1U));

		dispctrl_val[0] = __raw_readl(pReg + DVTIME1);
		dispctrl_val[1] = __raw_readl(pReg + DVTIME2);
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err(" VFP: %u, VSYNC: %u, VBP: %u, VACTIVE: %u\r\n",
			(((dispctrl_val[1] & DVTIME2_FEWC_MASK) >>
				DVTIME2_FEWC_SHIFT) + 1U),
			(((dispctrl_val[0] & DVTIME1_FPW_MASK) >>
				DVTIME1_FPW_SHIFT) + 1U),
			(((dispctrl_val[1] & DVTIME2_FSWC_MASK) >>
				DVTIME2_FSWC_SHIFT) + 1U),
			(((dispctrl_val[0] & DVTIME1_FLC_MASK) >>
				DVTIME1_FLC_SHIFT) + 1U));

		dispctrl_val[0] = __raw_readl(pReg + DVTIME1);
		dispctrl_val[1] = __raw_readl(pReg + DVTIME2);
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err(" VFP2: %u, VSYNC2: %u, VBP2: %u, VACTIVE2: %u\r\n",
			(((dispctrl_val[1] & DVTIME4_FEWC_MASK) >>
				DVTIME4_FEWC_SHIFT) + 1U),
			(((dispctrl_val[0] & DVTIME3_FPW_MASK) >>
				DVTIME3_FPW_SHIFT) + 1U),
			(((dispctrl_val[1] & DVTIME4_FSWC_MASK) >>
				DVTIME4_FSWC_SHIFT) + 1U),
			(((dispctrl_val[0] & DVTIME3_FLC_MASK) >>
				DVTIME3_FLC_SHIFT)+ 1U));
	}
}

void __iomem *VIOC_DISP_GetAddress(unsigned int vioc_id)
{
	unsigned int Num = get_vioc_index(vioc_id);
	void __iomem *ret = NULL;

	if (Num >= VIOC_DISP_MAX) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][DISP] %s Num:%u , max :%u\n",
		__func__, Num, VIOC_DISP_MAX);
		ret = NULL;
	} else {
		ret = phys_to_virt(TCC_VIOC_DISP_BASE((unsigned long)Num));
	}
	return ret;
}
