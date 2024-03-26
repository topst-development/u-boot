// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/vioc_vin.h>
#include <asm/arch/vioc/vioc_global.h>
#include <asm/arch/vioc/reg_physical.h>
#include <telechips/fb_dm.h>

#define VIOC_VIN_IREQ_UPD_MASK 0x00000001U
#define VIOC_VIN_IREQ_EOF_MASK 0x00000002U
#define VIOC_VIN_IREQ_VS_MASK 0x00000004U
#define VIOC_VIN_IREQ_INVS_MASK 0x00000008U

#define NOP __asm("NOP")

/* VIN polarity Setting */
void VIOC_VIN_SetSyncPolarity(
	void __iomem *reg,
	unsigned int hs_active_low,
	unsigned int vs_active_low,
	unsigned int field_bfield_low,
	unsigned int de_active_low,
	unsigned int gen_field_en,
	unsigned int pxclk_pol)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VIN_CTRL)
	       & ~(VIN_CTRL_GFEN_MASK | VIN_CTRL_DEAL_MASK | VIN_CTRL_FOL_MASK
		   | VIN_CTRL_VAL_MASK | VIN_CTRL_HAL_MASK
		   | VIN_CTRL_PXP_MASK));
	val |= (((gen_field_en & 0x1U) << VIN_CTRL_GFEN_SHIFT)
		| ((de_active_low & 0x1U) << VIN_CTRL_DEAL_SHIFT)
		| ((field_bfield_low & 0x1U) << VIN_CTRL_FOL_SHIFT)
		| ((vs_active_low & 0x1U) << VIN_CTRL_VAL_SHIFT)
		| ((hs_active_low & 0x1U) << VIN_CTRL_HAL_SHIFT)
		| ((pxclk_pol & 0x1U) << VIN_CTRL_PXP_SHIFT));
	__raw_writel(val, reg + VIN_CTRL);
}

/* VIN Configuration 1 */
void VIOC_VIN_SetCtrl(
	void __iomem *reg, unsigned int conv_en,
	unsigned int hsde_connect_en, unsigned int vs_mask, unsigned int fmt,
	unsigned int data_order)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VIN_CTRL)
	       & ~(VIN_CTRL_CONV_MASK | VIN_CTRL_HDCE_MASK | VIN_CTRL_VM_MASK
		   | VIN_CTRL_FMT_MASK | VIN_CTRL_DO_MASK));
	val |= (((conv_en & 0x1U) << VIN_CTRL_CONV_SHIFT)
		| ((hsde_connect_en & 0x1U) << VIN_CTRL_HDCE_SHIFT)
		| ((vs_mask & 0x1U) << VIN_CTRL_VM_SHIFT)
		| ((fmt & 0xFU) << VIN_CTRL_FMT_SHIFT)
		| ((data_order & 0x7U) << VIN_CTRL_DO_SHIFT));
	__raw_writel(val, reg + VIN_CTRL);
}

/* Interlace mode setting */
void VIOC_VIN_SetInterlaceMode(
	void __iomem *reg, unsigned int intl_en, unsigned int intpl_en)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VIN_CTRL)
	       & ~(VIN_CTRL_INTEN_MASK | VIN_CTRL_INTPLEN_MASK));
	val |= (((intl_en & 0x1U) << VIN_CTRL_INTEN_SHIFT)
		| ((intpl_en & 0x1U) << VIN_CTRL_INTPLEN_SHIFT));
	__raw_writel(val, reg);
}

/* VIN Capture mode Enable */
void VIOC_VIN_SetCaptureModeEnable(
	void __iomem *reg, unsigned int cap_en)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VIN_CTRL) & ~(VIN_CTRL_CP_MASK));
	val |= ((cap_en & 0x1U) << VIN_CTRL_CP_SHIFT);
	__raw_writel(val, reg + VIN_CTRL);
}

/* VIN Enable/Disable */
void VIOC_VIN_SetEnable(void __iomem *reg, unsigned int vin_en)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VIN_CTRL) & ~(VIN_CTRL_EN_MASK));
	val |= ((vin_en & 0x1U) << VIN_CTRL_EN_SHIFT);
	__raw_writel(val, reg + VIN_CTRL);
}

unsigned int VIOC_VIN_IsEnable(void __iomem *reg)
{
	void *tmp_pVIN = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pVIN = reg;
	reg = tmp_pVIN;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	return ((__raw_readl(reg + VIN_CTRL) & (VIN_CTRL_EN_MASK))
		>> VIN_CTRL_EN_SHIFT);
}

/* Image size setting */
void VIOC_VIN_SetImageSize(
	void __iomem *reg, unsigned int width, unsigned int height)
{
	u32 val;

	val = (((height & 0xFFFFU) << VIN_SIZE_HEIGHT_SHIFT)
	       | ((width & 0xFFFFU) << VIN_SIZE_WIDTH_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + VIN_SIZE);
}

/* Image offset setting */
void VIOC_VIN_SetImageOffset(
	void __iomem *reg, unsigned int offs_width,
	unsigned int offs_height, unsigned int offs_height_intl)
{
	u32 val;

	val = (((offs_height & 0xFFFFU) << VIN_OFFS_OFS_HEIGHT_SHIFT)
	       | ((offs_width & 0xFFFFU) << VIN_OFFS_OFS_WIDTH_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + VIN_OFFS);

	val = ((offs_height_intl & 0xFFFFU) << VIN_OFFS_INTL_OFS_HEIGHT_SHIFT);
	__raw_writel(val, reg + VIN_OFFS_INTL);
}

void VIOC_VIN_SetImageCropSize(
	void __iomem *reg, unsigned int width, unsigned int height)
{
	u32 val;

	val = (((height & 0xFFFFU) << VIN_CROP_SIZE_HEIGHT_SHIFT)
	       | ((width & 0xFFFFU) << VIN_CROP_SIZE_WIDTH_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + VIN_CROP_SIZE);
}

void VIOC_VIN_SetImageCropOffset(
	void __iomem *reg, unsigned int offs_width,
	unsigned int offs_height)
{
	u32 val;

	val = (((offs_height & 0xFFFFU) << VIN_CROP_OFFS_OFS_HEIGHT_SHIFT)
	       | ((offs_width & 0xFFFFU) << VIN_CROP_OFFS_OFS_WIDTH_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + VIN_CROP_OFFS);
}

/* Y2R conversion mode setting */
void VIOC_VIN_SetY2RMode(void __iomem *reg, unsigned int y2r_mode)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VIN_MISC) & ~(VIN_MISC_Y2RM_MASK));
	val |= ((y2r_mode & 0x7U) << VIN_MISC_Y2RM_SHIFT);
	__raw_writel(val, reg + VIN_MISC);
}

/* Y2R conversion Enable/Disable */
void VIOC_VIN_SetY2REnable(void __iomem *reg, unsigned int y2r_en)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VIN_MISC) & ~(VIN_MISC_Y2REN_MASK));
	val |= ((y2r_en & 0x1U) << VIN_MISC_Y2REN_SHIFT);
	__raw_writel(val, reg + VIN_MISC);
}

/* initialize LUT, for example, LUT values are set to inverse function. */
void VIOC_VIN_SetLUT(void __iomem *reg, unsigned int *pLUT)
{
	unsigned int *pLUT0, *pLUT1, *pLUT2, uiCount;
	u32 val = 0U;

	pLUT0 = (unsigned int *)(pLUT + 0U);
	pLUT1 = (unsigned int *)(pLUT0 + (256U / 4U));
	pLUT2 = (unsigned int *)(pLUT1 + (256U / 4U));

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VIN_MISC));
	__raw_writel(
		((val & ~(VIN_MISC_LUTIF_MASK)) |
		 (0x1U << VIN_MISC_LUTIF_SHIFT)),
		reg + VIN_MISC); /* Access Look-Up Table Using Slave Port */

	for (uiCount = 0U; uiCount < 256U; uiCount = uiCount + 4U) {
		/* Initialize Look-up Table */
		*pLUT0 = ((uiCount + 3U) << 24U) | ((uiCount + 2U) << 16U) |
			   ((uiCount + 1U) << 8U) | ((uiCount + 0U) << 0U);
		*pLUT1 = ((uiCount + 3U) << 24U) | ((uiCount + 2U) << 16U) |
			   ((uiCount + 1U) << 8U) | ((uiCount + 0U) << 0U);
		*pLUT2 = ((uiCount + 3U) << 24U) | ((uiCount + 2U) << 16U) |
			   ((uiCount + 1U) << 8U) | ((uiCount + 0U) << 0U);
		pLUT0++;
		pLUT1++;
		pLUT2++;
	}

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = __raw_readl(reg + VIN_MISC);
	__raw_writel(
		(val & ~(VIN_MISC_LUTIF_MASK)),
		reg + VIN_MISC); /* Access Look-Up Table Using Vin Module */
}

/* LUT Enable/Disable */
void VIOC_VIN_SetLUTEnable(
	void __iomem *reg, unsigned int lut0_en, unsigned int lut1_en,
	unsigned int lut2_en)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VIN_MISC) & ~(VIN_MISC_LUTEN_MASK));
	val |= (((lut0_en & 0x1U) << VIN_MISC_LUTEN_SHIFT)
		| ((lut1_en & 0x1U) << (VIN_MISC_LUTEN_SHIFT + 1U))
		| ((lut2_en & 0x1U) << (VIN_MISC_LUTEN_SHIFT + 2U)));
	__raw_writel(val, reg + VIN_MISC);
}

void VIOC_VIN_SetDemuxPort(
	void __iomem *reg, unsigned int p0, unsigned int p1,
	unsigned int p2, unsigned int p3)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VD_CTRL)
	       & ~(VD_CTRL_SEL3_MASK | VD_CTRL_SEL2_MASK | VD_CTRL_SEL1_MASK
		   | VD_CTRL_SEL0_MASK));
	val |= (((p0 & 0x7U) << VD_CTRL_SEL0_SHIFT)
		| ((p1 & 0x7U) << VD_CTRL_SEL1_SHIFT)
		| ((p2 & 0x7U) << VD_CTRL_SEL2_SHIFT)
		| ((p3 & 0x7U) << VD_CTRL_SEL3_SHIFT));
	__raw_writel(val, reg + VD_CTRL);
}

void VIOC_VIN_SetDemuxClock(void __iomem *reg, unsigned int mode)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VD_CTRL) & ~(VD_CTRL_CM_MASK));
	val |= ((mode & 0x7U) << VD_CTRL_CM_SHIFT);
	__raw_writel(val, reg + VD_CTRL);
}

void VIOC_VIN_SetDemuxEnable(void __iomem *reg, unsigned int enable)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VD_CTRL) & ~(VD_CTRL_EN_MASK));
	val |= ((enable & 0x1U) << VD_CTRL_EN_SHIFT);
	__raw_writel(val, reg + VD_CTRL);
}

void VIOC_VIN_SetSEEnable(void __iomem *reg, unsigned int se)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VIN_CTRL) & ~(VIN_CTRL_SE_MASK));
	val |= (((se & 0x1U) << VIN_CTRL_SE_SHIFT));
	__raw_writel(val, reg + VIN_CTRL);
}

void VIOC_VIN_SetFlushBufferEnable(void __iomem *reg, unsigned int fvs)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VIN_MISC) & ~(VIN_MISC_FVS_MASK));
	val |= ((fvs & 0x1U) << VIN_MISC_FVS_SHIFT);
	__raw_writel(val, reg + VIN_MISC);
}

void __iomem *VIOC_VIN_GetAddress(unsigned int vioc_id)
{
	unsigned int Num = get_vioc_index(vioc_id);
	void __iomem *ret = NULL;

	if (Num >= VIOC_VIN_MAX) {
		/* Prevent KCS warning */
		ret = NULL;
	} else {
		ret = phys_to_virt(TCC_VIOC_VIN_BASE((unsigned long)Num));
	}

	return ret;
}
