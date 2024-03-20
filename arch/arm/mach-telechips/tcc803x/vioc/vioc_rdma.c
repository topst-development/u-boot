// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <tcc_fb.h>
#include <asm/io.h>
#include <asm/telechips/vioc/vioc_rdma.h>
#include <asm/telechips/vioc/vioc_global.h>
#include <asm/telechips/vioc/reg_physical.h>

#define VIOC_RDMA_IREQ_SRC_MAX	7

#define NOP __asm("NOP")

int vioc_rdma_get_image_upd(void __iomem *reg)
{
	void *tmp_pRDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pRDMA = reg;
	reg = tmp_pRDMA;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	return ((__raw_readl(reg + RDMACTRL) & RDMACTRL_UPD_MASK) != 0U) ? 1 : 0;
}

void VIOC_RDMA_SetImageUpdate(void __iomem *reg)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_UPD_MASK));
	val |= ((u32)0x1U << RDMACTRL_UPD_SHIFT);
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetImageEnable(void __iomem *reg)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL)
	       & ~(RDMACTRL_IEN_MASK | RDMACTRL_UPD_MASK));
	val |= (((u32)0x1U << RDMACTRL_IEN_SHIFT) | ((u32)0x1U << RDMACTRL_UPD_SHIFT));
	__raw_writel(val, reg + RDMACTRL);
}

int vioc_rdma_get_image_enable(void __iomem *reg)
{
	void *tmp_pRDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pRDMA = reg;
	reg = tmp_pRDMA;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	return ((__raw_readl(reg + RDMACTRL) & RDMACTRL_IEN_MASK) != 0U) ? 1 : 0;
}

void VIOC_RDMA_SetImageDisable(void __iomem *reg)
{
	u32 val;
	int i;

	/* Check RDMA is enabled */
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	if ((__raw_readl(reg + RDMACTRL) & RDMACTRL_IEN_MASK) == 0U) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	val = (__raw_readl(reg + RDMASTAT) & ~(RDMASTAT_EOFR_MASK));
	val |= ((u32)0x1U << RDMASTAT_EOFR_SHIFT);
	__raw_writel(val, reg + RDMASTAT);
	__raw_writel(0x00000000, reg + RDMASIZE);

	val = (__raw_readl(reg + RDMACTRL)
		& ~(RDMACTRL_IEN_MASK | RDMACTRL_UPD_MASK));
	val |= ((u32)0x1U << RDMACTRL_UPD_SHIFT);
	__raw_writel(val, reg + RDMACTRL);

	/*
	 * RDMA Scale require to check STD_DEVEOF status bit, For synchronous
	 * updating with EOF Status Channel turn off when scaler changed, so now
	 * blocking.
	 */

	/* Wait for EOF */
	for (i = 0; i < 60; i++) {
		/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
		/* coverity[cert_dcl37_c_violation : FALSE] */
		mdelay(1);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg + RDMASTAT) & RDMASTAT_SDEOF_MASK) != 0U) {
			/* prevent KCS warning */
			break;
		}
	}
	if (i == 60) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("%s : [RDMA:0x%p] is not disabled\n",
		       __func__, reg);
	}
FUNC_EXIT:
	return;
}

// disable and no wait
void VIOC_RDMA_SetImageDisableNW(void __iomem *reg)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(0x00000000, reg + RDMASIZE);
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_IEN_MASK));
	if ((val & RDMACTRL_UPD_MASK) == 0U) {
		val |= ((u32)0x1U << RDMACTRL_UPD_SHIFT);
		__raw_writel(val, reg + RDMACTRL);
	}
}

void VIOC_RDMA_SetImageFormat(void __iomem *reg, unsigned int nFormat)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_FMT_MASK));
	val |= (nFormat << RDMACTRL_FMT_SHIFT);
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetImageRGBSwapMode(
	void __iomem *reg, unsigned int rgb_mode)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_SWAP_MASK));
	val |= (rgb_mode << RDMACTRL_SWAP_SHIFT);
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetImageAlphaEnable(
	void __iomem *reg, unsigned int enable)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_AEN_MASK));
	val |= (enable << RDMACTRL_AEN_SHIFT);
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_GetImageAlphaEnable(void __iomem *reg,
				   unsigned int *enable)
{
   void *tmp_pRDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

   /* avoid MISRA C-2012 Rule 8.13 */
   tmp_pRDMA = reg;
   reg = tmp_pRDMA;

   /* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
   /* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*enable =
		((__raw_readl(reg + RDMACTRL) & RDMACTRL_AEN_MASK)
		 >> RDMACTRL_AEN_SHIFT);
}

void VIOC_RDMA_SetImageAlphaSelect(
	void __iomem *reg, unsigned int select)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_ASEL_MASK));
	val |= (select << RDMACTRL_ASEL_SHIFT);
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetImageY2RMode(
	void __iomem *reg, unsigned int y2r_mode)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) &
	       ~(RDMACTRL_Y2RMD_MASK | RDMACTRL_Y2RMD2_MASK));
	val |= (((y2r_mode & 0x3U) << RDMACTRL_Y2RMD_SHIFT) |
		(((y2r_mode & 0x4U) >> 2U) << RDMACTRL_Y2RMD2_SHIFT));
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetImageY2REnable(
	void __iomem *reg, unsigned int enable)
{
	u32 val;
	void *tmp_pRDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pRDMA = reg;
	reg = tmp_pRDMA;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_Y2R_MASK));
	val |= (enable << RDMACTRL_Y2R_SHIFT);
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetImageR2YMode(
	void __iomem *reg, unsigned int r2y_mode)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) &
	       ~(RDMACTRL_R2YMD_MASK | RDMACTRL_R2YMD2_MASK));
	val |= (((r2y_mode & 0x3U) << RDMACTRL_R2YMD_SHIFT) |
		(((r2y_mode & 0x4U) >> 2U) << RDMACTRL_R2YMD2_SHIFT));
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetImageR2YEnable(
	void __iomem *reg, unsigned int enable)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_R2Y_MASK));
	val |= (enable << RDMACTRL_R2Y_SHIFT);
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetImageAlpha(
	void __iomem *reg, unsigned int nAlpha0, unsigned int nAlpha1)
{
	u32 val;

	val = ((nAlpha1 << RDMAALPHA_A13_SHIFT) |
	       (nAlpha0 << RDMAALPHA_A02_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + RDMAALPHA);
}

void VIOC_RDMA_GetImageAlpha(void __iomem *reg, unsigned int *nAlpha0,
			     unsigned int *nAlpha1)
{
	void *tmp_pRDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pRDMA = reg;
	reg = tmp_pRDMA;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*nAlpha1 = ((__raw_readl(reg + RDMAALPHA) & RDMAALPHA_A13_MASK) >>
		    RDMAALPHA_A13_SHIFT);
	*nAlpha0 = ((__raw_readl(reg + RDMAALPHA) & RDMAALPHA_A02_MASK) >>
		    RDMAALPHA_A02_SHIFT);
}

void VIOC_RDMA_SetImageUVIEnable(
	void __iomem *reg, unsigned int enable)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_UVI_MASK));
	val |= (enable << RDMACTRL_UVI_SHIFT);
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetImage3DMode(void __iomem *reg, unsigned int mode)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_3DM_MASK));
	val |= (mode << RDMACTRL_3DM_SHIFT);
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetImageSize(
	void __iomem *reg, unsigned int sw, unsigned int sh)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = ((sh << RDMASIZE_HEIGHT_SHIFT) | ((sw << RDMASIZE_WIDTH_SHIFT)));
	__raw_writel(val, reg + RDMASIZE);
}

void VIOC_RDMA_GetImageSize(void __iomem *reg, unsigned int *sw,
			    unsigned int *sh)
{
	void *tmp_pRDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pRDMA = reg;
	reg = tmp_pRDMA;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	*sw = ((__raw_readl(reg + RDMASIZE) & RDMASIZE_WIDTH_MASK)
	       >> RDMASIZE_WIDTH_SHIFT);
	*sh = ((__raw_readl(reg + RDMASIZE) & RDMASIZE_HEIGHT_MASK)
	       >> RDMASIZE_HEIGHT_SHIFT);
}

void VIOC_RDMA_SetImageBase(
	void __iomem *reg, unsigned int nBase0, unsigned int nBase1,
	unsigned int nBase2)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(nBase0, reg + RDMABASE0);
	__raw_writel(nBase1, reg + RDMABASE1);
	__raw_writel(nBase2, reg + RDMABASE2);
}

void VIOC_RDMA_SetImageRBase(
	void __iomem *reg, unsigned int nBase0, unsigned int nBase1,
	unsigned int nBase2)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(nBase0, reg + RDMA_RBASE0);
	__raw_writel(nBase1, reg + RDMA_RBASE1);
	__raw_writel(nBase2, reg + RDMA_RBASE2);
}

void VIOC_RDMA_SetImageOffset(
	void __iomem *reg, unsigned int imgFmt, unsigned int imgWidth)
{
	u32 val;
	unsigned int offset0 = 0, offset1 = 0;

	if (imgWidth > 0x1FFFU) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][RDMA] %s imgWidth(%u) is wrong\n", __func__, imgWidth);
		} else {
		switch (imgFmt) {
		case (unsigned int)TCC_LCDC_IMG_FMT_1BPP:
			// 1bpp indexed color
			offset0 = (1U * imgWidth) / 8U;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_2BPP:
			// 2bpp indexed color
			offset0 = (1U * imgWidth) / 4U;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_4BPP:
			// 4bpp indexed color
			offset0 = (1U * imgWidth) / 2U;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_8BPP:
			// 8bpp indexed color
			offset0 = (1U * imgWidth);
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB332:
			// RGB332 - 1bytes aligned -
						// R[7:5],G[4:2],B[1:0]
			offset0 = 1U * imgWidth;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB444:
			// RGB444 - 2bytes aligned -
						// A[15:12],R[11:8],G[7:3],B[3:0]
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB565:
			// RGB565 - 2bytes aligned -
						// R[15:11],G[10:5],B[4:0]
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB555:
			// RGB555 - 2bytes aligned -
						// A[15],R[14:10],G[9:5],B[4:0]
			offset0 = 2U * imgWidth;
			break;
		// case TCC_LCDC_IMG_FMT_RGB888:
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB888:
			// RGB888 - 4bytes aligned -
						// A[31:24],R[23:16],G[15:8],B[7:0]
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB666:
			// RGB666 - 4bytes aligned -
						// A[23:18],R[17:12],G[11:6],B[5:0]
			offset0 = 4U * imgWidth;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_RGB888_3:
			// RGB888 - 3 bytes aligned :
						// B1[31:24],R0[23:16],G0[15:8],B0[7:0]
		case (unsigned int)TCC_LCDC_IMG_FMT_ARGB6666_3:
			// ARGB6666 - 3 bytes aligned :
						// A[23:18],R[17:12],G[11:6],B[5:0]
			offset0 = 3U * imgWidth;
			break;

		case (unsigned int)TCC_LCDC_IMG_FMT_444SEP:
			/* YUV444 or RGB444 Format */
			offset0 = imgWidth;
			offset1 = imgWidth;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_YUV420SP:
			// YCbCr 4:2:0 Separated format - Not
						// Supported for Image 1 and 2
			offset0 = imgWidth;
			offset1 = imgWidth / 2U;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_YUV422SP:
			// YCbCr 4:2:2 Separated format - Not
						// Supported for Image 1 and 2
			offset0 = imgWidth;
			offset1 = imgWidth / 2U;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_UYVY:
			// YCbCr 4:2:2 Sequential format
		case (unsigned int)TCC_LCDC_IMG_FMT_VYUY:
			// YCbCr 4:2:2 Sequential format
		case (unsigned int)TCC_LCDC_IMG_FMT_YUYV:
			// YCbCr 4:2:2 Sequential format
		case (unsigned int)TCC_LCDC_IMG_FMT_YVYU:
			// YCbCr 4:2:2 Sequential format
			offset0 = 2U * imgWidth;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_YUV420ITL0:
			// YCbCr 4:2:0 interleved type 0
			// format - Not Supported for Image 1
			// and 2
		case (unsigned int)TCC_LCDC_IMG_FMT_YUV420ITL1:
			// YCbCr 4:2:0 interleved type 1
			// format - Not Supported for Image 1
			// and 2
			offset0 = imgWidth;
			offset1 = imgWidth;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_YUV422ITL0:
			// YCbCr 4:2:2 interleved type 0
			// format - Not Supported for Image 1
			// and 2
		case (unsigned int)TCC_LCDC_IMG_FMT_YUV422ITL1:
			// YCbCr 4:2:2 interleved type 1
			// format - Not Supported for Image 1
			// and 2
			offset0 = imgWidth;
			offset1 = imgWidth;
			break;
		case (unsigned int)TCC_LCDC_IMG_FMT_MAX:
		default:
			offset0 = imgWidth;
			offset1 = imgWidth;
			break;
		}

		val = ((offset1 << RDMAOFFS_OFFSET1_SHIFT)
			| (offset0 << RDMAOFFS_OFFSET0_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(val, reg + RDMAOFFS);
	}
}

void VIOC_RDMA_SetImageBfield(void __iomem *reg, unsigned int bfield)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_BF_MASK));
	val |= (bfield << RDMACTRL_BF_SHIFT);
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetImageBFMD(void __iomem *reg, unsigned int bfmd)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_BFMD_MASK));
	val |= (bfmd << RDMACTRL_BFMD_SHIFT);
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetImageIntl(void __iomem *reg, unsigned int intl_en)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMACTRL) & ~(RDMACTRL_INTL_MASK));
	val |= (intl_en << RDMACTRL_INTL_SHIFT);
	__raw_writel(val, reg + RDMACTRL);
}

void VIOC_RDMA_SetIreqMask(
	void __iomem *reg, unsigned int mask, unsigned int set)
{
	if (set == 0U) { /* Interrupt Enable*/
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(~mask, reg + RDMAIRQMSK);
	} else { /* Interrupt Diable*/
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(mask, reg + RDMAIRQMSK);
	}
}

void VIOC_RDMA_SetStatus(void __iomem *reg, unsigned int mask)
{
   void *tmp_pRDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

   /* avoid MISRA C-2012 Rule 8.13 */
   tmp_pRDMA = reg;
   reg = tmp_pRDMA;

   /* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
   /* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(mask, reg + RDMASTAT);
}

unsigned int VIOC_RDMA_GetStatus(void __iomem *reg)
{
	void *tmp_pRDMA = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pRDMA = reg;
	reg = tmp_pRDMA;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	return __raw_readl(reg + RDMASTAT);
}

void VIOC_RDMA_SetIssue(
	void __iomem *reg, unsigned int burst_length,
	unsigned int issue_cnt)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMASCALE) & ~(RDMASCALE_ISSUE_MASK));
	val |= ((burst_length << 8U) | issue_cnt) << RDMASCALE_ISSUE_SHIFT;
	__raw_writel(val, reg + RDMASCALE);
}

void VIOC_RDMA_SetImageScale(
	void __iomem *reg, unsigned int scaleX, unsigned int scaleY)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + RDMASCALE))
		& ~(RDMASCALE_XSCALE_MASK | RDMASCALE_YSCALE_MASK);
	val |= ((scaleX << RDMASCALE_XSCALE_SHIFT)
		| (scaleY << RDMASCALE_YSCALE_SHIFT));
	__raw_writel(val, reg + RDMASCALE);
}

void VIOC_RDMA_DUMP(void __iomem *reg, unsigned int vioc_id)
{
	unsigned int cnt = 0U;
	void __iomem *pReg = reg;

	if (pReg == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][RDMA] %s reg is NULL\n", __func__);
	} else {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("RDMA-%u :: 0x%p\n", vioc_id, pReg);
		while (cnt < 0x50U) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("0x%p: 0x%08x\n", pReg + cnt,
			       __raw_readl(pReg + cnt));
			cnt += 0x4U;
		}
	}
}

void __iomem *VIOC_RDMA_GetAddress(unsigned int vioc_id)
{
	unsigned int Num = get_vioc_index(vioc_id);
	void __iomem *ret = NULL;

	if (Num >= VIOC_RDMA_MAX) {
		/* Prevnet KCS warning */
		ret = NULL;
	} else {
		ret = phys_to_virt(TCC_VIOC_RDMA_BASE((unsigned long)Num));
	}

	return ret;
}
/* EOF */
