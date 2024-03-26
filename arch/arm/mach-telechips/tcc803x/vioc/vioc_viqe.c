// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/vioc_viqe.h>
#include <asm/arch/vioc/vioc_global.h>
#include <asm/arch/vioc/reg_physical.h>

#define NOP __asm("NOP")

/******************************* VIQE Control *******************************/
void VIOC_VIQE_SetImageSize(
	void __iomem *reg, unsigned int width, unsigned int height)
{
	u32 val;

	val = (((height & 0x7FFU) << VSIZE_HEIGHT_SHIFT)
			| ((width & 0x7FFU) << VSIZE_WIDTH_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + VSIZE);
}

void VIOC_VIQE_SetImageY2RMode(
	void __iomem *reg, unsigned int y2r_mode)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VTIMEGEN) & ~(VTIMEGEN_Y2RMD_MASK));
	val |= ((y2r_mode & 0x3U) << VTIMEGEN_Y2RMD_SHIFT);
	__raw_writel(val, reg + VTIMEGEN);
}
void VIOC_VIQE_SetImageY2REnable(
	void __iomem *reg, unsigned int enable)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VTIMEGEN) & ~(VTIMEGEN_Y2REN_MASK));
	val |= ((enable & 0x1U) << VTIMEGEN_Y2REN_SHIFT);
	__raw_writel(val, reg + VTIMEGEN);
}

void VIOC_VIQE_SetControlMisc(
			void __iomem *reg,
			unsigned int no_hor_intpl,
			unsigned int fmt_conv_disable,
			unsigned int fmt_conv_disable_using_fmt,
			unsigned int update_disable, unsigned int cfgupd,
			unsigned int h2h)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VCTRL)
	       & ~(VCTRL_CFGUPD_MASK | VCTRL_UPD_MASK | VCTRL_FCDUF_MASK
		   | VCTRL_FCD_MASK | VCTRL_NHINTPL_MASK));
	val |= (((cfgupd & 0x1U) << VCTRL_CFGUPD_SHIFT)
		| ((update_disable & 0x1U) << VCTRL_UPD_SHIFT)
		| ((fmt_conv_disable_using_fmt & 0x1U) << VCTRL_FCDUF_SHIFT)
		| ((fmt_conv_disable & 0x1U) << VCTRL_FCD_SHIFT)
		| (((~no_hor_intpl) & 0x1U) << VCTRL_NHINTPL_SHIFT));
	__raw_writel(val, reg + VCTRL);

	val = (__raw_readl(reg + VTIMEGEN) & ~(VTIMEGEN_H2H_MASK));
	val |= ((h2h & 0xFFU) << VTIMEGEN_H2H_SHIFT);
	__raw_writel(val, reg + VTIMEGEN);
}

void VIOC_VIQE_SetControlDontUse(
	void __iomem *reg, unsigned int global_en_dont_use,
	unsigned int top_size_dont_use,
	unsigned int stream_deintl_info_dont_use)
{
	u32 val;
	unsigned int *tmp_pVIQE = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pVIQE = reg;
	reg = tmp_pVIQE;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VMISC)
	       & ~(VMISC_SDDU_MASK | VMISC_TSDU_MASK | VMISC_GENDU_MASK));
	val |= (((stream_deintl_info_dont_use & 0x1U) << VMISC_SDDU_SHIFT)
		| ((top_size_dont_use & 0x1U) << VMISC_TSDU_SHIFT)
		| ((global_en_dont_use & 0x1U) << VMISC_GENDU_SHIFT));

	__raw_writel(val, reg + VMISC);
}

void VIOC_VIQE_SetControlClockGate(
	void __iomem *reg, unsigned int deintl_dis,
	unsigned int d3d_dis, unsigned int pm_dis)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VCTRL)
	       & ~(VCTRL_CGPMD_MASK | VCTRL_CGDND_MASK | VCTRL_CGDID_MASK));
	val |= (((pm_dis & 0x1U) << VCTRL_CGPMD_SHIFT)
		| ((d3d_dis & 0x1U) << VCTRL_CGDND_SHIFT)
		| ((deintl_dis & 0x1U) << VCTRL_CGDID_SHIFT));
	__raw_writel(val, reg + VCTRL);
}

void VIOC_VIQE_SetControlEnable(
		void __iomem *reg,
		unsigned int his_cdf_or_lut_en,
		unsigned int his_en, unsigned int gamut_en,
		unsigned int denoise3d_en,
		unsigned int deintl_en)
{
	u32 val;

	/* avoid MISRA C-2012 Rule 2.7 */
    (void)his_cdf_or_lut_en;
	(void)his_en;
	(void)gamut_en;
	(void)denoise3d_en;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + DI_DEC0_CTRL) & ~(DI_DEC_CTRL_EN_MASK));
	val |= ((deintl_en & 0x1U) << DI_DEC_CTRL_EN_SHIFT);
	__raw_writel(val, reg + DI_DEC0_CTRL);

	val = (__raw_readl(reg + DI_DEC1_CTRL) & ~(DI_DEC_CTRL_EN_MASK));
	val |= ((deintl_en & 0x1U) << DI_DEC_CTRL_EN_SHIFT);
	__raw_writel(val, reg + DI_DEC1_CTRL);

	val = (__raw_readl(reg + DI_DEC2_CTRL) & ~(DI_DEC_CTRL_EN_MASK));
	val |= ((deintl_en & 0x1U) << DI_DEC_CTRL_EN_SHIFT);
	__raw_writel(val, reg + DI_DEC2_CTRL);

	val = (__raw_readl(reg + DI_COM0_CTRL) & ~(DI_COM0_CTRL_EN_MASK));
	val |= ((deintl_en & 0x1U) << DI_COM0_CTRL_EN_SHIFT);
	__raw_writel(val, reg + DI_COM0_CTRL);

	val = (__raw_readl(reg + VCTRL)
	       & ~(VCTRL_DIEN_MASK | VCTRL_DNEN_MASK | VCTRL_GMEN_MASK
		   | VCTRL_HIEN_MASK | VCTRL_HILUT_MASK));
	val |= ((deintl_en & 0x1U) << VCTRL_DIEN_SHIFT);
	__raw_writel(val, reg + VCTRL);
}

void VIOC_VIQE_SetControlMode(
	void __iomem *reg, unsigned int his_cdf_or_lut_en,
	unsigned int his_en, unsigned int gamut_en, unsigned int denoise3d_en,
	unsigned int deintl_en)
{
	u32 val;

	/* avoid MISRA C-2012 Rule 2.7 */
    (void)his_cdf_or_lut_en;
	(void)his_en;
	(void)gamut_en;
	(void)denoise3d_en;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + VCTRL)
	       & ~(VCTRL_DIEN_MASK | VCTRL_DNEN_MASK | VCTRL_GMEN_MASK
		   | VCTRL_HIEN_MASK | VCTRL_HILUT_MASK));
	val |= ((deintl_en & 0x1U) << VCTRL_DIEN_SHIFT);
	__raw_writel(val, reg + VCTRL);
}

void VIOC_VIQE_SetControlRegister(
	void __iomem *reg, unsigned int width, unsigned int height,
	unsigned int fmt)
{
	/* avoid MISRA C-2012 Rule 2.7 */
	(void)fmt;

	VIOC_VIQE_SetImageSize(reg, width, height);
	VIOC_VIQE_SetControlMisc(
		reg, OFF, OFF, ON, OFF, ON,
		0x16); /* All of variables are the recommended value */
	VIOC_VIQE_SetControlDontUse(
		reg, OFF, OFF,
		OFF); /* All of variables are the recommended value */
	VIOC_VIQE_SetControlClockGate(
		reg, OFF, OFF,
		OFF); /* All of variables are the recommended value */
}

/******************************* DI Control *******************************/
void VIOC_VIQE_SetDeintlBase(
	void __iomem *reg, unsigned int frmnum, unsigned int base0,
	unsigned int base1, unsigned int base2, unsigned int base3)
{
	if (frmnum == 0U) {
		__raw_writel(
			(base0 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE0);
		__raw_writel(
			(base1 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE1);
		__raw_writel(
			(base2 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE2);
		__raw_writel(
			(base3 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE3);
	} else if (frmnum == 1U) {
		__raw_writel(
			(base0 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE0A);
		__raw_writel(
			(base1 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE1A);
		__raw_writel(
			(base2 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE2A);
		__raw_writel(
			(base3 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE3A);
	} else if (frmnum == 2U) {
		__raw_writel(
			(base0 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE0B);
		__raw_writel(
			(base1 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE1B);
		__raw_writel(
			(base2 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE2B);
		__raw_writel(
			(base3 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE3B);
	} else {
		__raw_writel(
			(base0 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE0C);
		__raw_writel(
			(base1 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE1C);
		__raw_writel(
			(base2 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE2C);
		__raw_writel(
			(base3 & 0xFFFFFFFFU) << DI_BASE_BASE_SHIFT,
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg + DI_BASE3C);
	}
}

void VIOC_VIQE_SwapDeintlBase(void __iomem *reg, int mode)
{
	unsigned int curr_viqe_base[4];
	unsigned int next_viqe_base[4];

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	curr_viqe_base[3] = __raw_readl(reg + DI_BASE3);
	curr_viqe_base[2] = __raw_readl(reg + DI_BASE2);
	curr_viqe_base[1] = __raw_readl(reg + DI_BASE1);
	curr_viqe_base[0] = __raw_readl(reg + DI_BASE0);

	switch (mode) {
	case (int)DUPLI_MODE:
		next_viqe_base[3] = curr_viqe_base[2];
		next_viqe_base[2] = curr_viqe_base[1];
		next_viqe_base[1] = curr_viqe_base[0];
		next_viqe_base[0] = curr_viqe_base[3];
		break;
	case (int)SKIP_MODE:
		next_viqe_base[3] = curr_viqe_base[2];
		next_viqe_base[2] = curr_viqe_base[1];
		next_viqe_base[1] = curr_viqe_base[0];
		next_viqe_base[0] = curr_viqe_base[3];
		break;
	case (int)NORMAL_MODE:
	default:
		next_viqe_base[3] = curr_viqe_base[3];
		next_viqe_base[2] = curr_viqe_base[2];
		next_viqe_base[1] = curr_viqe_base[1];
		next_viqe_base[0] = curr_viqe_base[0];
		break;
	}

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(next_viqe_base[3], reg + DI_BASE3);
	__raw_writel(next_viqe_base[2], reg + DI_BASE2);
	__raw_writel(next_viqe_base[1], reg + DI_BASE1);
	__raw_writel(next_viqe_base[0], reg + DI_BASE0);
}

void VIOC_VIQE_SetDeintlSize(
	void __iomem *reg, unsigned int width, unsigned int height)
{
	u32 val;

	val = ((((height >> 1U) & 0x7FFU) << DI_SIZE_HEIGHT_SHIFT)
	       | ((width & 0x7FFU) << DI_SIZE_WIDTH_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + DI_SIZE);
}

void VIOC_VIQE_SetDeintlMisc(
	void __iomem *reg, unsigned int uvintpl, unsigned int cfgupd,
	unsigned int dma_enable, unsigned int h2h,
	unsigned int top_size_dont_use)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + DI_CTRL)
	       & ~(DI_CTRL_H2H_MASK | DI_CTRL_CFGUPD_MASK | DI_CTRL_EN_MASK
		   | DI_CTRL_UVINTPL_MASK | DI_CTRL_TSDU_MASK));
	val |= (((h2h & 0xFFU) << DI_CTRL_H2H_SHIFT)
		| ((cfgupd & 0x1U) << DI_CTRL_CFGUPD_SHIFT)
		| ((dma_enable & 0x1U) << DI_CTRL_EN_SHIFT)
		| ((uvintpl & 0x1U) << DI_CTRL_UVINTPL_SHIFT)
		| ((top_size_dont_use & 0x1U) << DI_CTRL_TSDU_SHIFT));
	__raw_writel(val, reg + DI_CTRL);
}

void VIOC_VIQE_SetDeintlControl(
	void __iomem *reg, unsigned int fmt,
	unsigned int eof_control_ready, unsigned int dec_divisor,
	unsigned int ac_k0_limit, unsigned int ac_k1_limit,
	unsigned int ac_k2_limit)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + DI_DEC0_MISC)
	       & ~(DI_DEC_MISC_DEC_DIV_MASK | DI_DEC_MISC_ECR_MASK));
	val |= (((dec_divisor & 0x3U) << DI_DEC_MISC_DEC_DIV_SHIFT)
		| ((eof_control_ready & 0x1U) << DI_DEC_MISC_ECR_SHIFT));
	__raw_writel(val, reg + DI_DEC0_MISC);

	val = (__raw_readl(reg + DI_DEC1_MISC)
	       & ~(DI_DEC_MISC_DEC_DIV_MASK | DI_DEC_MISC_ECR_MASK));
	val |= (((dec_divisor & 0x3U) << DI_DEC_MISC_DEC_DIV_SHIFT)
		| ((eof_control_ready & 0x1U) << DI_DEC_MISC_ECR_SHIFT));
	__raw_writel(val, reg + DI_DEC1_MISC);

	val = (__raw_readl(reg + DI_DEC2_MISC)
	       & ~(DI_DEC_MISC_DEC_DIV_MASK | DI_DEC_MISC_ECR_MASK));
	val |= (((dec_divisor & 0x3U) << DI_DEC_MISC_DEC_DIV_SHIFT)
		| ((eof_control_ready & 0x1U) << DI_DEC_MISC_ECR_SHIFT));
	__raw_writel(val, reg + DI_DEC2_MISC);

	val = (__raw_readl(reg + DI_COM0_MISC) & ~(DI_COM0_MISC_FMT_MASK));
	val |= (((fmt & 0xFU) << DI_COM0_MISC_FMT_SHIFT));
	__raw_writel(val, reg + DI_COM0_MISC);

	val = (__raw_readl(reg + DI_COM0_AC)
	       & ~(DI_COM0_AC_K2_AC_MASK | DI_COM0_AC_K1_AC_MASK
		   | DI_COM0_AC_K0_AC_MASK));
	val |= (((ac_k2_limit & 0x3FU) << DI_COM0_AC_K2_AC_SHIFT)
		| ((ac_k1_limit & 0x3FU) << DI_COM0_AC_K1_AC_SHIFT)
		| ((ac_k0_limit & 0x3FU) << DI_COM0_AC_K0_AC_SHIFT));
	__raw_writel(val, reg + DI_COM0_AC);
}

/******************************* DI Core Control
 * *******************************/
static void VIOC_VIQE_InitDeintlCoreBypass(void __iomem *reg)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(0x00010b31, reg + DI_CTRL2);
	__raw_writel(0x02040408, reg + DI_ENGINE0);
	__raw_writel(0x7f32040f, reg + DI_ENGINE1);
	__raw_writel(0x00800410, reg + DI_ENGINE2);
	__raw_writel(0x01002000, reg + DI_ENGINE3);
	__raw_writel(0x12462582, reg + DI_ENGINE4);
	__raw_writel(0x010085f4, reg + PD_THRES0);
	__raw_writel(0x001e140f, reg + PD_THRES1);
	__raw_writel(0x6f40881e, reg + PD_JUDDER);
	__raw_writel(0x00095800, reg + PD_JUDDER_M);
	__raw_writel(0x00000000, reg + DI_MISCC);

	__raw_writel(0x00000000, reg + DI_REGION0);
	__raw_writel(0x00000000, reg + DI_REGION1);
	__raw_writel(0x00000000, reg + DI_INT);
	__raw_writel(0x0008050a, reg + DI_PD_SAW);
}

static void VIOC_VIQE_InitDeintlCoreSpatial(void __iomem *reg)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(0x00030a31, reg + DI_CTRL2);
	__raw_writel(0x02040408, reg + DI_ENGINE0);
	__raw_writel(0x2812050f, reg + DI_ENGINE1);
	__raw_writel(0x00800410, reg + DI_ENGINE2);
	__raw_writel(0x01002000, reg + DI_ENGINE3);
	__raw_writel(0x12462582, reg + DI_ENGINE4);
	__raw_writel(0x010085f4, reg + PD_THRES0);
	__raw_writel(0x001e140f, reg + PD_THRES1);
	__raw_writel(0x6f408805, reg + PD_JUDDER);
	__raw_writel(0x00095800, reg + PD_JUDDER_M);
	__raw_writel(0x00000000, reg + DI_MISCC);

	__raw_writel(0x00000000, reg + DI_REGION0);
	__raw_writel(0x00000000, reg + DI_REGION1);
	__raw_writel(0x00000000, reg + DI_INT);
	__raw_writel(0x0008050a, reg + DI_PD_SAW);
}

void VIOC_VIQE_InitDeintlCoreTemporal(void __iomem *reg)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(0x00010b31, reg + DI_CTRL2);
	__raw_writel(0x02040408, reg + DI_ENGINE0);
	__raw_writel(0x7f32050f, reg + DI_ENGINE1);
	__raw_writel(0x00800410, reg + DI_ENGINE2);
	__raw_writel(0x01002000, reg + DI_ENGINE3);
	__raw_writel(0x12462582, reg + DI_ENGINE4);
	__raw_writel(0x01f085f4, reg + PD_THRES0);
	__raw_writel(0x001e140f, reg + PD_THRES1);
	__raw_writel(0x6f40880B, reg + PD_JUDDER);
	__raw_writel(0x00095801, reg + PD_JUDDER_M);
	__raw_writel(0x06120401, reg + DI_MISCC);

	__raw_writel(0x00000000, reg + DI_REGION0);
	__raw_writel(0x00000000, reg + DI_REGION1);
	__raw_writel(0x00000000, reg + DI_INT);
	__raw_writel(0x0008050a, reg + DI_PD_SAW);
}

void VIOC_VIQE_SetDeintlFMT(void __iomem *reg, unsigned int enable)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + DI_FMT) & ~(DI_FMT_TFCD_MASK));
	val |= ((enable & 0x1U) << DI_FMT_TFCD_SHIFT);
	__raw_writel(val, reg + DI_FMT);
}

void VIOC_VIQE_SetDeintlMode(void __iomem *reg,
			     enum VIOC_VIQE_DEINTL_MODE mode)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	if (mode == VIOC_VIQE_DEINTL_MODE_BYPASS) {
		__raw_writel(0x20010b31, reg + DI_CTRL2);
		__raw_writel(0x7f32040f, reg + DI_ENGINE1);
		__raw_writel(0x6f40881e, reg + PD_JUDDER);

		val = (__raw_readl(reg + DI_CTRL2) & ~(DI_CTRL2_BYPASS_MASK));
		val |= ((u32)0x1U << DI_CTRL2_BYPASS_SHIFT);
		__raw_writel(val, reg + DI_CTRL2); // bypass

		val = (__raw_readl(reg + DI_CTRL) & ~(DI_CTRL_EN_MASK));
		val |= ((u32)0x1U << DI_CTRL_EN_SHIFT);
		__raw_writel(val, reg + DI_CTRL); // DI DMA enable
	} else if (mode == VIOC_VIQE_DEINTL_MODE_2D) {
		__raw_writel(0x00020a31, reg + DI_CTRL2);
		__raw_writel(0x2812050f, reg + DI_ENGINE1);
		__raw_writel(0x6f408805, reg + PD_JUDDER);

		val = (__raw_readl(reg + DI_CTRL2) & ~(DI_CTRL2_BYPASS_MASK));
		val |= ((u32)0x0U << DI_CTRL2_BYPASS_SHIFT);
		__raw_writel(val, reg + DI_CTRL2); // bypass

		val = (__raw_readl(reg + DI_CTRL) & ~(DI_CTRL_EN_MASK));
		val |= ((u32)0x1U << DI_CTRL_EN_SHIFT);
		__raw_writel(val, reg + DI_CTRL);      // DI DMA enable
	} else if (mode == VIOC_VIQE_DEINTL_MODE_3D) { // Temporal Mode - using
						       // 4-field frames.
		__raw_writel(0x00010b31, reg + DI_CTRL2);
		__raw_writel(0x7f32050f, reg + DI_ENGINE1);
		__raw_writel(0x6f4088FF, reg + PD_JUDDER);

		val = (__raw_readl(reg + DI_CTRL2) & ~(DI_CTRL2_BYPASS_MASK));
		val |= ((u32)0x0U << DI_CTRL2_BYPASS_SHIFT);
		__raw_writel(val, reg + DI_CTRL2); // bypass

		val = (__raw_readl(reg + DI_CTRL) & ~(DI_CTRL_EN_MASK));
		val |= ((u32)0x1U << DI_CTRL_EN_SHIFT);
		__raw_writel(val, reg + DI_CTRL); // DI DMA enable
	} else {
		/* avoid MISRA C-2012 Rule 15.7 */
	}
}

void VIOC_VIQE_SetDeintlModeWeave(void __iomem *reg)
{
	u32 val;

	// BITCLR(pVIQE->cDEINTL.nDI_CTRL, ((0<<5)|(0<<4)|(0<<0)));
	// 0x280

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + DI_CTRL2)
	       & ~(DI_CTRL2_JS_MASK | DI_CTRL2_MRTM_MASK | DI_CTRL2_MRSP_MASK));
	__raw_writel(val, reg + DI_CTRL2);

    //BITCSET(pVIQE->cDEINTL.nDI_ENGINE0, 0xffffffff, 0x0204ff08);	// 0x284
	__raw_writel(0x0204ff08, reg + DI_ENGINE0);

    //BITCLR(pVIQE->cDEINTL.nDI_ENGINE3, (0xfff<<20));                // 0x290
	val = (__raw_readl(reg + DI_ENGINE3) & ~(DI_ENGINE3_STTHW_MASK));
	__raw_writel(val, reg + DI_ENGINE3);

    //BITCSET(pVIQE->cDEINTL.nDI_ENGINE4, 0xffffffff, 0x124f2582);	// 0x294
	__raw_writel(0x124f2582, reg + DI_ENGINE4);
}

void VIOC_VIQE_SetDeintlRegion(
	void __iomem *reg, unsigned int region_enable, unsigned int region_idx_x_start,
	unsigned int region_idx_x_end, unsigned int region_idx_y_start, unsigned int region_idx_y_end)
{
	u32 val;

	val = (((region_enable & 0x1U) << DI_REGION0_EN_SHIFT)
	       | ((region_idx_x_end & 0x3FFU) << DI_REGION0_XEND_SHIFT)
	       | ((region_idx_x_start & 0x3FFU) << DI_REGION0_XSTART_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + DI_REGION0);

	val = (((region_idx_y_end & 0x3FFU) << DI_REGION1_YEND_SHIFT)
	       | ((region_idx_y_start & 0x3FFU) << DI_REGION1_YSTART_SHIFT));
	__raw_writel(val, reg + DI_REGION1);
}

void VIOC_VIQE_SetDeintlCore(
	void __iomem *reg, unsigned int width,
	unsigned int height, unsigned int fmt,
	unsigned int bypass,
	unsigned int top_size_dont_use)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	if (bypass == 1U) {
		val = (__raw_readl(reg + DI_CTRL2) & ~(DI_CTRL2_PDEN_MASK));
		__raw_writel(val, reg + DI_CTRL2);
	}

	val = (__raw_readl(reg + DI_CTRL2) & ~(DI_CTRL2_BYPASS_MASK));
	val |= ((bypass & 0x1U) << DI_CTRL2_BYPASS_SHIFT);
	__raw_writel(val, reg + DI_CTRL2);

	val = (((height & 0x7FFU) << DI_CSIZE_HEIGHT_SHIFT)
	       | ((width & 0x7FFU) << DI_CSIZE_WIDTH_SHIFT));
	__raw_writel(val, reg + DI_CSIZE);

	val = (__raw_readl(reg + DI_FMT)
	       & ~(DI_FMT_TSDU_MASK | DI_FMT_F422_MASK));
	val |= (((top_size_dont_use & 0x1U) << DI_FMT_TSDU_SHIFT)
		| ((fmt & 0x1U) << DI_FMT_F422_SHIFT));
	__raw_writel(val, reg + DI_FMT);
}

void VIOC_VIQE_SetDeintlRegister(
	void __iomem *reg, unsigned int fmt,
	unsigned int top_size_dont_use,
	unsigned int width, unsigned int height,
	enum VIOC_VIQE_DEINTL_MODE mode,
	unsigned int base0, unsigned int base1,
	unsigned int base2, unsigned int base3)
{
	unsigned int bypass = 0U;
	unsigned int dma_enable = 0U;

	/* avoid MISRA C-2012 Rule 2.7 */
    (void)top_size_dont_use;

	if (mode == VIOC_VIQE_DEINTL_MODE_BYPASS) {
		bypass = 1U;
		VIOC_VIQE_InitDeintlCoreBypass(reg);
	} else if (mode == VIOC_VIQE_DEINTL_MODE_2D) {
		VIOC_VIQE_InitDeintlCoreSpatial(reg);
	} else { // VIOC_VIQE_DEINTL_MODE_3D
		dma_enable = 1U;
		VIOC_VIQE_InitDeintlCoreTemporal(reg);
	}

	VIOC_VIQE_SetDeintlBase(reg, 0U, base0, base1, base2, base3);
	VIOC_VIQE_SetDeintlSize(reg, width, height);
	VIOC_VIQE_SetDeintlMisc(
		reg, OFF, ON, dma_enable, 0x16U,
		OFF); /* All of variables are the recommended value */
	VIOC_VIQE_SetDeintlControl(
		reg, fmt, ON, 0x3, 0x31U, 0x2AU,
		0x23); /* All of variables are the recommended value */
	VIOC_VIQE_SetDeintlCore(reg, width, height, fmt, bypass, OFF);
}

void VIOC_VIQE_SetDeintlJudderCnt(void __iomem *reg, unsigned int cnt)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + PD_JUDDER) & ~(PD_JUDDER_CNTS_MASK));
	val |= ((cnt & 0xFFU) << PD_JUDDER_CNTS_SHIFT);
	__raw_writel(val, reg + PD_JUDDER);

	val = (__raw_readl(reg + PD_JUDDER_M) & ~(PD_JUDDER_M_JDH_MASK));
	val |= ((u32)0x1U << PD_JUDDER_M_JDH_SHIFT);
	__raw_writel(val, reg + PD_JUDDER_M);

	val = (__raw_readl(reg + PD_THRES0)
	       & ~(PD_THRES0_CNTSCO_MASK | PD_THRES0_CNTS_MASK));
	val |= ((u32)0xFU << PD_THRES0_CNTSCO_SHIFT);
	__raw_writel(val, reg + PD_THRES0);
}

void VIOC_VIQE_InitDeintlCoreVinMode(void __iomem *reg)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(0x00202000, reg + DI_ENGINE3);
}

void VIOC_VIQE_DUMP(void __iomem *reg, unsigned int vioc_id)
{
	unsigned int cnt = 0;

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_err("VIQE-%u :: 0x%p\n", vioc_id, reg);
	while (cnt < 0x300U) {
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

void __iomem *VIOC_VIQE_GetAddress(unsigned int vioc_id)
{
	unsigned int Num = get_vioc_index(vioc_id);
	void __iomem *ret = NULL;

	if (Num >= VIOC_VIQE_MAX) {
		/* Prevent KCS warning */
		ret = NULL;
	} else {
		ret = phys_to_virt(TCC_VIOC_VIQE_BASE((unsigned long)Num));
	}

	return ret;
}
