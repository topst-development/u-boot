// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/vioc_wdma.h>
#include <asm/arch/vioc/vioc_global.h>
#include <asm/arch/vioc/reg_physical.h>
#include <telechips/fb_dm.h>

#define NOP __asm("NOP")

void VIOC_WDMA_SetImageEnable(void __iomem *reg,
			      unsigned int nContinuous)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value =
		(__raw_readl(reg + WDMACTRL_OFFSET)
		 & ~(WDMACTRL_IEN_MASK | WDMACTRL_CONT_MASK
		     | WDMACTRL_UPD_MASK));
	/*
	 * redundant update UPD has problem
	 * So if UPD is high, do not update UPD bit.
	 */
	value |=
		(((u32)0x1U << WDMACTRL_IEN_SHIFT)
		 | (nContinuous << WDMACTRL_CONT_SHIFT)
		 | ((u32)0x1U << WDMACTRL_UPD_SHIFT));

	__raw_writel(value, reg + WDMACTRL_OFFSET);
}

void VIOC_WDMA_GetImageEnable(void __iomem *reg, unsigned int *enable)
{
	void *tmp_pWDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pWDMA = reg;
	reg = tmp_pWDMA;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*enable =
		((__raw_readl(reg + WDMACTRL_OFFSET) & WDMACTRL_IEN_MASK)
		 >> WDMACTRL_IEN_SHIFT);
}

void VIOC_WDMA_SetImageDisable(void __iomem *reg)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value =
		(__raw_readl(reg + WDMACTRL_OFFSET)
		 & ~(WDMACTRL_IEN_MASK | WDMACTRL_UPD_MASK));
	value |= (((u32)0x0U << WDMACTRL_IEN_SHIFT) |
		((u32)0x1U << WDMACTRL_UPD_SHIFT));
	__raw_writel(value, reg + WDMACTRL_OFFSET);
}

void VIOC_WDMA_SetImageUpdate(void __iomem *reg)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + WDMACTRL_OFFSET) & ~(WDMACTRL_UPD_MASK));
	value |= ((u32)0x1U << WDMACTRL_UPD_SHIFT);
	__raw_writel(value, reg + WDMACTRL_OFFSET);
}

void VIOC_WDMA_SetContinuousMode(
	void __iomem *reg, unsigned int enable)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + WDMACTRL_OFFSET) & ~(WDMACTRL_CONT_MASK));
	value |= (enable << WDMACTRL_CONT_SHIFT);

	__raw_writel(value, reg + WDMACTRL_OFFSET);
}

void VIOC_WDMA_SetImageFormat(void __iomem *reg, unsigned int nFormat)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + WDMACTRL_OFFSET) & ~(WDMACTRL_FMT_MASK));
	value |= (nFormat << WDMACTRL_FMT_SHIFT);

	__raw_writel(value, reg + WDMACTRL_OFFSET);
}

#ifdef CONFIG_VIOC_10BIT
void VIOC_WDMA_SetDataFormat(void __iomem *reg, unsigned int fmt_type,
			     unsigned int fill_mode)
{
	u32 value;

	value = (readl(reg + WDMACTRL_OFFSET) &
		 ~(WDMACTRL_FMT10FILL_MASK | WDMACTRL_FMT10_MASK));
	value |= ((fill_mode << WDMACTRL_FMT10FILL_SHIFT) |
		  (fmt_type << WDMACTRL_FMT10_SHIFT));

	writel(value, reg + WDMACTRL_OFFSET);
}
#endif //

void VIOC_WDMA_SetImageRGBSwapMode(
	void __iomem *reg, unsigned int rgb_mode)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + WDMACTRL_OFFSET) & ~(WDMACTRL_SWAP_MASK));
	value |= (rgb_mode << WDMACTRL_SWAP_SHIFT);

	__raw_writel(value, reg + WDMACTRL_OFFSET);
}

void VIOC_WDMA_SetImageInterlaced(void __iomem *reg, unsigned int intl)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + WDMACTRL_OFFSET) & ~(WDMACTRL_INTL_MASK));
	value |= (intl << WDMACTRL_INTL_SHIFT);

	__raw_writel(value, reg + WDMACTRL_OFFSET);
}

void VIOC_WDMA_SetImageR2YMode(
	void __iomem *reg, unsigned int r2y_mode)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + WDMACTRL_OFFSET) & ~(WDMACTRL_R2YMD_MASK));
	value |= (r2y_mode << WDMACTRL_R2YMD_SHIFT);

	__raw_writel(value, reg + WDMACTRL_OFFSET);
}

void VIOC_WDMA_SetImageR2YEnable(
	void __iomem *reg, unsigned int enable)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + WDMACTRL_OFFSET) & ~(WDMACTRL_R2Y_MASK));
	value |= (enable << WDMACTRL_R2Y_SHIFT);

	__raw_writel(value, reg + WDMACTRL_OFFSET);
}

void VIOC_WDMA_SetImageY2RMode(
	void __iomem *reg, unsigned int y2r_mode)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + WDMACTRL_OFFSET) & ~(WDMACTRL_Y2RMD_MASK));
	value |= (y2r_mode << WDMACTRL_Y2RMD_SHIFT);

	__raw_writel(value, reg + WDMACTRL_OFFSET);
}

void VIOC_WDMA_SetImageY2REnable(
	void __iomem *reg, unsigned int enable)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + WDMACTRL_OFFSET) & ~(WDMACTRL_Y2R_MASK));
	value |= (enable << WDMACTRL_Y2R_SHIFT);

	__raw_writel(value, reg + WDMACTRL_OFFSET);
}

void VIOC_WDMA_SetImageSize(
	void __iomem *reg, unsigned int sw, unsigned int sh)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = ((sh << WDMASIZE_HEIGHT_SHIFT) | (sw << WDMASIZE_WIDTH_SHIFT));
	__raw_writel(value, reg + WDMASIZE_OFFSET);
}

void VIOC_WDMA_SetImageBase(
	void __iomem *reg, unsigned int nBase0, unsigned int nBase1,
	unsigned int nBase2)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(nBase0 << WDMABASE0_BASE0_SHIFT, reg + WDMABASE0_OFFSET);
	__raw_writel(nBase1 << WDMABASE1_BASE1_SHIFT, reg + WDMABASE1_OFFSET);
	__raw_writel(nBase2 << WDMABASE2_BASE2_SHIFT, reg + WDMABASE2_OFFSET);
}

void VIOC_WDMA_SetImageOffset(
	void __iomem *reg, unsigned int imgFmt, unsigned int imgWidth)
{
	unsigned int offset0 = 0;
	unsigned int offset1 = 0;
	u32 value = 0;

	if (imgWidth > 0x1FFFU) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][WMIX] %s imgWidth(%u) is wrong\n", __func__, imgWidth);
	} else {
		switch (imgFmt) {
		case (unsigned int)TCC_LCDC_IMG_FMT_1BPP: // 1bpp indexed color
			offset0 = (1U * imgWidth) / 8U;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_2BPP: // 2bpp indexed color
			offset0 = (1U * imgWidth) / 4U;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_4BPP: // 4bpp indexed color
			offset0 = (1U * imgWidth) / 2U;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_8BPP: // 8bpp indexed color
			offset0 = (1U * imgWidth);
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB332: // RGB332 - 1bytes aligned -
						// R[7:5],G[4:2],B[1:0]
			offset0 = 1U * imgWidth;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB444: // RGB444 - 2bytes aligned -
						// A[15:12],R[11:8],G[7:3],B[3:0]
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB565: // RGB565 - 2bytes aligned -
						// R[15:11],G[10:5],B[4:0]
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB555: // RGB555 - 2bytes aligned -
						// A[15],R[14:10],G[9:5],B[4:0]
			offset0 = 2U * imgWidth;
			break;
		// case (unsigned int)TCC_LCDC_IMG_FMT_RGB888:
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB888: // RGB888 - 4bytes aligned -
						// A[31:24],R[23:16],G[15:8],B[7:0]
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB666: // RGB666 - 4bytes aligned -
						// A[23:18],R[17:12],G[11:6],B[5:0]
			offset0 = 4U * imgWidth;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB888_3: // RGB888 - 3 bytes aligned :
						// B1[31:24],R0[23:16],G0[15:8],B0[7:0]
		case (unsigned int)TCC_LCDC_IMG_FMT_ARGB6666_3: // ARGB6666 - 3 bytes aligned :
						// A[23:18],R[17:12],G[11:6],B[5:0]
			offset0 = 3U * imgWidth;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_444SEP: /* YUV444 or RGB444 Format */
			offset0 = imgWidth;
			offset1 = imgWidth;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_YUV420SP: // YCbCr 4:2:0 Separated format - Not
						// Supported for Image 1 and 2
			offset0 = imgWidth;
			offset1 = imgWidth / 2U;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_YUV422SP: // YCbCr 4:2:2 Separated format - Not
						// Supported for Image 1 and 2
			offset0 = imgWidth;
			offset1 = imgWidth / 2U;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_UYVY: // YCbCr 4:2:2 Sequential format
		case (unsigned int)TCC_LCDC_IMG_FMT_VYUY: // YCbCr 4:2:2 Sequential format
		case (unsigned int)TCC_LCDC_IMG_FMT_YUYV: // YCbCr 4:2:2 Sequential format
		case (unsigned int)TCC_LCDC_IMG_FMT_YVYU: // YCbCr 4:2:2 Sequential format
			offset0 = 2U * imgWidth;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_YUV420ITL0: // YCbCr 4:2:0 interleved type 0
						// format - Not Supported for Image 1
						// and 2
		case (unsigned int)TCC_LCDC_IMG_FMT_YUV420ITL1: // YCbCr 4:2:0 interleved type 1
						// format - Not Supported for Image 1
						// and 2
			offset0 = imgWidth;
			offset1 = imgWidth;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_YUV422ITL0: // YCbCr 4:2:2 interleved type 0
						// format - Not Supported for Image 1
						// and 2
		case (unsigned int)TCC_LCDC_IMG_FMT_YUV422ITL1: // YCbCr 4:2:2 interleved type 1
						// format - Not Supported for Image 1
						// and 2
			offset0 = imgWidth;
			offset1 = imgWidth;
			break;
		default:
			offset0 = imgWidth;
			offset1 = imgWidth;
			break;
		}

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + WDMAOFFS_OFFSET)
			& ~(WDMAOFFS_OFFSET1_MASK | WDMAOFFS_OFFSET0_MASK));
		value |=
			((offset1 << WDMAOFFS_OFFSET1_SHIFT)
			| (offset0 << WDMAOFFS_OFFSET0_SHIFT));
		__raw_writel(value, reg + WDMAOFFS_OFFSET);
	}
}

void VIOC_WDMA_SetIreqMask(
	void __iomem *reg, unsigned int mask, unsigned int set)
{
	/*
	 * set 1 : IREQ Masked(interrupt disable),
	 * set 0 : IREQ UnMasked(interrput enable)
	 */
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + WDMAIRQMSK_OFFSET) & ~(mask));

	if (set == 1U) { /* Interrupt Disable*/
		/* Prevent KCS warning */
		value |= mask;
	}

	__raw_writel(value, reg + WDMAIRQMSK_OFFSET);
}

void VIOC_WDMA_SetIreqStatus(void __iomem *reg, unsigned int mask)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + WDMAIRQSTS_OFFSET) & ~(mask));
	value |= mask;
	__raw_writel(value, reg + WDMAIRQSTS_OFFSET);
}

void VIOC_WDMA_ClearEOFR(void __iomem *reg)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value =
		(__raw_readl(reg + WDMAIRQSTS_OFFSET)
		 & ~(WDMAIRQSTS_EOFR_MASK));
	value |= ((u32)0x1U << WDMAIRQSTS_EOFR_SHIFT);
	__raw_writel(value, reg + WDMAIRQSTS_OFFSET);
}

void VIOC_WDMA_ClearEOFF(void __iomem *reg)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value =
		(__raw_readl(reg + WDMAIRQSTS_OFFSET)
		 & ~(WDMAIRQSTS_EOFF_MASK));
	value |= ((u32)0x1U << WDMAIRQSTS_EOFF_SHIFT);
	__raw_writel(value, reg + WDMAIRQSTS_OFFSET);
}

void VIOC_WDMA_GetStatus(void __iomem *reg, unsigned int *status)
{
	void *tmp_pWDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pWDMA = reg;
	reg = tmp_pWDMA;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*status = __raw_readl(reg + WDMAIRQSTS_OFFSET);
}

unsigned int VIOC_WDMA_IsImageEnable(void __iomem *reg)
{
	void *tmp_pWDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pWDMA = reg;
	reg = tmp_pWDMA;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	return ((__raw_readl(reg + WDMACTRL_OFFSET) & WDMACTRL_IEN_MASK) >>
		WDMACTRL_IEN_SHIFT);
}

unsigned int VIOC_WDMA_IsContinuousMode(void __iomem *reg)
{
	void *tmp_pWDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pWDMA = reg;
	reg = tmp_pWDMA;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	return ((__raw_readl(reg + WDMACTRL_OFFSET) & WDMACTRL_CONT_MASK) >>
		WDMACTRL_CONT_SHIFT);
}

unsigned int VIOC_WDMA_Get_CAddress(void __iomem *reg)
{
	u32 value = 0U;
	void *tmp_pWDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pWDMA = reg;
	reg = tmp_pWDMA;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = __raw_readl(reg + WDMACADDR_OFFSET);

	return value;
}

void VIOC_WDMA_SetRate(void __iomem *reg, unsigned int ren,
						unsigned int maxrate,
						unsigned int syncmd,
						unsigned int sen,
						unsigned int syncsel)
{
	u32 value = 0U;

	/* avoid MISRA C-2012 Rule 2.7 */
	(void)ren;
	(void)maxrate;
	(void)syncmd;
	(void)sen;
	(void)syncsel;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + WDMARATE_OFFSET) &
		~(WDMARATE_REN_MASK | WDMARATE_MAXRATE_MASK |
		WDMARATE_SYNCMD_MASK |
		WDMARATE_SEN_MASK | WDMARATE_SYNCSEL_MASK));

	value |= (((u32)0x1U << WDMARATE_REN_SHIFT) |
		((u32)0xFFU << WDMARATE_MAXRATE_SHIFT) |
		((u32)0x7U << WDMARATE_SYNCMD_SHIFT) |
		((u32)0x1U << WDMARATE_SEN_SHIFT) |
		((u32)0xFFU << WDMARATE_SYNCSEL_SHIFT));
	__raw_writel(value, reg + WDMARATE_OFFSET);
}

void VIOC_WDMA_DUMP(void __iomem *reg, unsigned int vioc_id)
{
	unsigned int cnt = 0;

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_err("WDMA-%u :: 0x%p\n", vioc_id, reg);
	while (cnt < 0x70U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		pr_err("0x%p: 0x%08x\n", reg + cnt,
			       __raw_readl(reg + cnt));
		cnt += 0x4U;
	}
}

void __iomem *VIOC_WDMA_GetAddress(unsigned int vioc_id)
{
	unsigned int Num = get_vioc_index(vioc_id);
	void __iomem *ret = NULL;

	if (Num >= VIOC_WDMA_MAX) {
		/* Prevent KCS warning */
		ret = NULL;
	} else {
		ret = phys_to_virt(TCC_VIOC_WDMA_BASE((unsigned long)Num));
	}
	
	return ret;
}

/* EOF */
