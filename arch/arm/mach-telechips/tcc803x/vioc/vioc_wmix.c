// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/telechips/vioc/vioc_wmix.h>
#include <asm/telechips/vioc/vioc_global.h>
#include <asm/telechips/vioc/reg_physical.h>

#define VIOC_WMIX_IREQ_MUPD_MASK	0x00000001U
#define VIOC_WMIX_IREQ_MEOFWF_MASK	0x00000002U
#define VIOC_WMIX_IREQ_MEOFWR_MASK	0x00000004U
#define VIOC_WMIX_IREQ_MEOFR_MASK	0x00000008U
#define VIOC_WMIX_IREQ_MEOFF_MASK	0x00000010U

void VIOC_WMIX_SetOverlayPriority(
	void __iomem *reg, unsigned int nOverlayPriority)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + MCTRL) & ~(MCTRL_OVP_MASK));
	val |= (nOverlayPriority << MCTRL_OVP_SHIFT);
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + MCTRL);
}

void VIOC_WMIX_GetOverlayPriority(
	void __iomem *reg, unsigned int *nOverlayPriority)
{
	void *tmp_pWMIX = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pWMIX = reg;
	reg = tmp_pWMIX;

	//	*nOverlayPriority = pWMIX->uCTRL.nREG & 0x1F;
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*nOverlayPriority =
		(((__raw_readl(reg + MCTRL)) & MCTRL_OVP_MASK)
		 >> MCTRL_OVP_SHIFT);
}

void VIOC_WMIX_SetUpdate(void __iomem *reg)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + MCTRL) & ~(MCTRL_UPD_MASK));
	val |= ((u32)0x1U << MCTRL_UPD_SHIFT);
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + MCTRL);
}

void VIOC_WMIX_SetSize(
	void __iomem *reg, unsigned int nWidth, unsigned int nHeight)
{
	u32 val;

	val = (((nHeight & 0x1FFFU) << MSIZE_HEIGHT_SHIFT)
	       | ((nWidth & 0x1FFFU) << MSIZE_WIDTH_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + MSIZE);
}

void VIOC_WMIX_GetSize(
	void __iomem *reg, unsigned int *nWidth, unsigned int *nHeight)
{
	void *tmp_pWMIX = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pWMIX = reg;
	reg = tmp_pWMIX;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*nWidth =
		((__raw_readl(reg + MSIZE) & MSIZE_WIDTH_MASK)
		 >> MSIZE_WIDTH_SHIFT);
	*nHeight =
		((__raw_readl(reg + MSIZE) & MSIZE_HEIGHT_MASK)
		 >> MSIZE_HEIGHT_SHIFT);
}

void VIOC_WMIX_SetBGColor(
	void __iomem *reg, unsigned int nBG0, unsigned int nBG1,
	unsigned int nBG2, unsigned int nBG3)
{
	u32 val;

	val = (((nBG1 & 0xFFU) << MBG_BG1_SHIFT)
	       | ((nBG0 & 0xFFU) << MBG_BG0_SHIFT)
	       | ((nBG3 & 0xFFU) << MBG_BG3_SHIFT)
	       | ((nBG2 & 0xFFU) << MBG_BG2_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + MBG);
}

void VIOC_WMIX_SetPosition(
	void __iomem *reg, unsigned int nChannel, unsigned int nX,
	unsigned int nY)
{
	u32 val;

	if (nChannel > 3U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s nChannel(%u) is wrong\n", __func__, nChannel);
	} else {
		val = (((nY & 0x1FFFU) << MPOS_YPOS_SHIFT)
			| ((nX & 0x1FFFU) << MPOS_XPOS_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(val, reg + (MPOS0 + (0x4U * nChannel)));
	}
}

void VIOC_WMIX_GetPosition(
	void __iomem *reg, unsigned int nChannel, unsigned int *nX,
	unsigned int *nY)
{
	void *tmp_pWMIX = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pWMIX = reg;
	reg = tmp_pWMIX;

	if (nChannel > 3U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s nChannel(%u) is wrong\n", __func__, nChannel);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		*nX = ((__raw_readl(reg + (MPOS0 + (0x4U * nChannel))) & MPOS_XPOS_MASK)
			>> MPOS_XPOS_SHIFT);
		*nY = ((__raw_readl(reg + (MPOS0 + (0x4U * nChannel))) & MPOS_YPOS_MASK)
			>> MPOS_YPOS_SHIFT);
	}
}

void VIOC_WMIX_SetChromaKey(
	void __iomem *reg, unsigned int nLayer, unsigned int nKeyEn,
	unsigned int nKeyR, unsigned int nKeyG, unsigned int nKeyB,
	unsigned int nKeyMaskR, unsigned int nKeyMaskG, unsigned int nKeyMaskB)
{
	u32 val;

	if (nLayer > 2U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s nLayer(%u) is wrong\n", __func__, nLayer);
	} else {
		val = (((nKeyEn & 0x1U) << MKEY0_KEN_SHIFT)
			| ((nKeyR & 0xFFU) << MKEY0_KRYR_SHIFT)
			| ((nKeyG & 0xFFU) << MKEY0_KEYG_SHIFT)
			| ((nKeyB & 0xFFU) << MKEY0_KEYB_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(val, reg + (MKEY00 + (0x08U * nLayer)));

		val = (((nKeyMaskR & 0xFFU) << MKEY1_MKEYR_SHIFT)
			| ((nKeyMaskG & 0xFFU) << MKEY1_MKEYG_SHIFT)
			| ((nKeyMaskB & 0xFFU) << MKEY1_MKEYB_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(val, reg + (MKEY01 + (0x08U * nLayer)));
	}
}

void VIOC_WMIX_GetChromaKey(
	void __iomem *reg, unsigned int nLayer, unsigned int *nKeyEn,
	unsigned int *nKeyR, unsigned int *nKeyG, unsigned int *nKeyB,
	unsigned int *nKeyMaskR, unsigned int *nKeyMaskG,
	unsigned int *nKeyMaskB)
{
	void *tmp_pWMIX = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pWMIX = reg;
	reg = tmp_pWMIX;

	if (nLayer > 2U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s nLayer(%u) is wrong\n", __func__, nLayer);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		*nKeyEn =
			((__raw_readl(reg + (MKEY00 + (0x08U * nLayer)))
			& MKEY0_KEN_MASK)
			>> MKEY0_KEN_SHIFT);
		*nKeyR =
			((__raw_readl(reg + (MKEY00 + (0x08U * nLayer)))
			& MKEY0_KRYR_MASK)
			>> MKEY0_KRYR_SHIFT);
		*nKeyG =
			((__raw_readl(reg + (MKEY00 + (0x08U * nLayer)))
			& MKEY0_KEYG_MASK)
			>> MKEY0_KEYG_SHIFT);
		*nKeyB =
			((__raw_readl(reg + (MKEY00 + (0x08U * nLayer)))
			& MKEY0_KEYB_MASK)
			>> MKEY0_KEYB_SHIFT);

		*nKeyMaskR =
			((__raw_readl(reg + (MKEY01 + (0x08U * nLayer)))
			& MKEY1_MKEYR_MASK)
			>> MKEY1_MKEYR_SHIFT);
		*nKeyMaskG =
			((__raw_readl(reg + (MKEY01 + (0x08U * nLayer)))
			& MKEY1_MKEYG_MASK)
			>> MKEY1_MKEYG_SHIFT);
		*nKeyMaskB =
			((__raw_readl(reg + (MKEY01 + (0x08U * nLayer)))
			& MKEY1_MKEYB_MASK)
			>> MKEY1_MKEYB_SHIFT);
	}
}

void VIOC_WMIX_ALPHA_SetAlphaValueControl(
	void __iomem *reg, unsigned int layer, unsigned int region,
	unsigned int acon0, unsigned int acon1)
{
	u32 val;

	if (layer > 2U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s layer(%u) is wrong\n", __func__, layer);
	} else {
		switch (region) {
		case 0: /*Region A*/
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + (MACON0 + (0x20U * layer)))
				& ~(MACON_ACON1_00_MASK | MACON_ACON0_00_MASK));
			val |= (((acon1 & 0x7U) << MACON_ACON1_00_SHIFT)
				| ((acon0 & 0x7U) << MACON_ACON0_00_SHIFT));
			__raw_writel(val, reg + (MACON0 + (0x20U * layer)));
			break;
		case 1: /*Region B*/
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + (MACON0 + (0x20U * layer)))
				& ~(MACON_ACON1_10_MASK | MACON_ACON0_10_MASK));
			val |= (((acon1 & 0x7U) << MACON_ACON1_10_SHIFT)
				| ((acon0 & 0x7U) << MACON_ACON0_10_SHIFT));
			__raw_writel(val, reg + (MACON0 + (0x20U * layer)));
			break;
		case 2: /*Region C*/
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + (MACON0 + (0x20U * layer)))
				& ~(MACON_ACON1_11_MASK | MACON_ACON0_11_MASK));
			val |= (((acon1 & 0x7U) << MACON_ACON1_11_SHIFT)
				| ((acon0 & 0x7U) << MACON_ACON0_11_SHIFT));
			__raw_writel(val, reg + (MACON0 + (0x20U * layer)));
			break;
		case 3: /*Region D*/
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + (MACON0 + (0x20U * layer)))
				& ~(MACON_ACON1_01_MASK | MACON_ACON0_01_MASK));
			val |= (((acon1 & 0x7U) << MACON_ACON1_01_SHIFT)
				| ((acon0 & 0x7U) << MACON_ACON0_01_SHIFT));
			__raw_writel(val, reg + (MACON0 + (0x20U * layer)));
			break;
		default:
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("[ERR][WMIX] %s region(%u) is wrong\n", __func__, region);
			break;
		}
	}
}

void VIOC_WMIX_ALPHA_SetColorControl(
	void __iomem *reg, unsigned int layer, unsigned int region,
	unsigned int ccon0, unsigned int ccon1)
{
	u32 val;

	if (layer > 2U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s layer(%u) is wrong\n", __func__, layer);
	} else {
		switch (region) {
		case 0: /*Region A*/
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + (MCCON0 + (0x20U * layer)))
				& ~(MCCON_CCON1_00_MASK | MCCON_CCON0_00_MASK));
			val |= (((ccon1 & 0xFU) << MCCON_CCON1_00_SHIFT)
				| ((ccon0 & 0xFU) << MCCON_CCON0_00_SHIFT));
			__raw_writel(val, reg + (MCCON0 + (0x20U * layer)));
			break;
		case 1: /*Region B*/
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + (MCCON0 + (0x20U * layer)))
				& ~(MCCON_CCON1_10_MASK | MCCON_CCON0_10_MASK));
			val |= (((ccon1 & 0xFU) << MCCON_CCON1_10_SHIFT)
				| ((ccon0 & 0xFU) << MCCON_CCON0_10_SHIFT));
			__raw_writel(val, reg + (MCCON0 + (0x20U * layer)));
			break;
		case 2: /*Region C*/
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + (MCCON0 + (0x20U * layer)))
				& ~(MCCON_CCON1_11_MASK | MCCON_CCON0_11_MASK));
			val |= (((ccon1 & 0xFU) << MCCON_CCON1_11_SHIFT)
				| ((ccon0 & 0xFU) << MCCON_CCON0_11_SHIFT));
			__raw_writel(val, reg + (MCCON0 + (0x20U * layer)));
			break;
		case 3: /*Region D*/
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + (MCCON0 + (0x20U * layer)))
				& ~(MCCON_CCON1_01_MASK | MCCON_CCON0_01_MASK));
			val |= (((ccon1 & 0xFU) << MCCON_CCON1_01_SHIFT)
				| ((ccon0 & 0xFU) << MCCON_CCON0_01_SHIFT));
			__raw_writel(val, reg + (MCCON0 + (0x20U * layer)));
			break;
		default:
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("[ERR][WMIX] %s region(%u) is wrong\n", __func__, region);
			break;
		}
	}
}

void VIOC_WMIX_ALPHA_SetROPMode(
	void __iomem *reg, unsigned int layer, unsigned int mode)
{
	u32 val;

	if (layer > 2U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s layer(%u) is wrong\n", __func__, layer);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + (MROPC0 + (0x20U * layer)))
			& ~(MROPC_ROPMODE_MASK));
		val |= ((mode & 0x1FU) << MROPC_ROPMODE_SHIFT);
		__raw_writel(val, reg + (MROPC0 + (0x20U * layer)));
	}
}

void VIOC_WMIX_ALPHA_SetAlphaSelection(
	void __iomem *reg, unsigned int layer, unsigned int asel)
{
	u32 val;

	if (layer > 2U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s layer(%u) is wrong\n", __func__, layer);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + (MROPC0 + (0x20U * layer)))
			& ~(MROPC_ASEL_MASK));
		val |= ((asel & 0x3U) << MROPC_ASEL_SHIFT);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(val, reg + (MROPC0 + (0x20U * layer)));
	}
}

void VIOC_WMIX_ALPHA_SetAlphaValue(
	void __iomem *reg, unsigned int layer, unsigned int alpha0,
	unsigned int alpha1)
{
	u32 val;

	if (layer > 2U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s layer(%u) is wrong\n", __func__, layer);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + (MROPC0 + (0x20U * layer)))
			& ~(MROPC_ALPHA1_MASK | MROPC_ALPHA0_MASK));
		val |= (((alpha1 & 0xFFU) << MROPC_ALPHA1_SHIFT)
			| ((alpha0 & 0xFFU) << MROPC_ALPHA0_SHIFT));

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(val, reg + (MROPC0 + (0x20U * layer)));
	}
}

/* Not Used (will be deleted) */
void VIOC_WMIX_ALPHA_SetROPPattern(
	void __iomem *reg, unsigned int layer, unsigned int patR,
	unsigned int patG, unsigned int patB)
{
	u32 val;

	if (layer > 2U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s layer(%u) is wrong\n", __func__, layer);
	} else {
		val = (((patB & 0xFFU) << MPAT_BLUE_SHIFT)
			| ((patG & 0xFFU) << MPAT_GREEN_SHIFT)
			| ((patR & 0xFFU) << MPAT_RED_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(val, reg + (MPAT0 + (0x20U * layer)));
	}
}

void VIOC_WMIX_SetInterruptMask(void __iomem *reg, unsigned int nMask)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(nMask, reg + MIRQMSK);
}

unsigned int VIOC_WMIX_GetStatus(void __iomem *reg)
{
	void *tmp_pWMIX = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pWMIX = reg;
	reg = tmp_pWMIX;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	return __raw_readl(reg + MSTS);
}

void VIOC_WMIX_DUMP(void __iomem *reg, unsigned int vioc_id)
{
	unsigned int cnt = 0U;
	void __iomem *pReg = reg;

	if (pReg == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s Num:%u max:%u\n",
			__func__, vioc_id, VIOC_WMIX_MAX);
	} else {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("WMIX-%u :: 0x%p\n", vioc_id, pReg);
		while (cnt < 0x70U) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			/* coverity[cert_PRE16_c_violation : FALSE] */
			pr_err("0x%p: 0x%08x\n", pReg + cnt,
			       __raw_readl(pReg + cnt));
			cnt += 0x4U;
		}
	}
}

void __iomem *VIOC_WMIX_GetAddress(unsigned int vioc_id)
{
	unsigned int Num = get_vioc_index(vioc_id);
	void __iomem *ret = NULL;

	if (Num >= VIOC_WMIX_MAX) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s Num:%u max:%u\n", __func__, Num, VIOC_WMIX_MAX);
		ret = NULL;
	} else {
		ret = phys_to_virt(TCC_VIOC_WMIX_BASE((unsigned long)Num));
	}

	return ret;
}
