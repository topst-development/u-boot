// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <telechips/fb_dm.h>
#include <asm/io.h>
#include <asm/arch/vioc/vioc_scaler.h>
#include <asm/arch/vioc/reg_physical.h>
#include <asm/arch/vioc/vioc_global.h>

#define NOP __asm("NOP")

void VIOC_SC_SetBypass(void __iomem *reg, unsigned int nOnOff)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + SCCTRL) & ~(SCCTRL_BP_MASK));
	val |= (nOnOff << SCCTRL_BP_SHIFT);
	__raw_writel(val, reg + SCCTRL);
}

void VIOC_SC_SetUpdate(void __iomem *reg)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + SCCTRL)
	       & ~(SCCTRL_UPD_MASK /*| SCCTRL_FFC_MASK*/));
	val |= ((u32)0x1U << SCCTRL_UPD_SHIFT);
	__raw_writel(val, reg + SCCTRL);
}

void VIOC_SC_SetSrcSize(
	void __iomem *reg, unsigned int nWidth, unsigned int nHeight)
{
	u32 val;

	val = ((nHeight << SCSSIZE_HEIGHT_SHIFT)
	       | (nWidth << SCSSIZE_WIDTH_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + SCSSIZE);
}

#if defined(CONFIG_MC_WORKAROUND)
unsigned int VIOC_SC_GetPlusSize(
	unsigned int src_height,
	unsigned int dst_height)
{
	unsigned int plus_temp, plus_height;

	if (src_height <= dst_height) {
		// up-scaling
		plus_temp = (0x4U) * ((dst_height * 10U)/src_height);
		plus_height = (plus_temp / 10U);
		if ((plus_temp % 10U) > 0U) {
			/* Prevent KCS warning */
			plus_height += 1U;
		}
	} else {
		//down-scaling
		plus_temp = ((src_height + 0x4U) * 100U) /
			((src_height * 10U) / dst_height);
		plus_height = (plus_temp / 10U) - src_height;
		if ((plus_temp % 10U) > 0U) {
			plus_height += 1U;
			/* prevent KCS warning */
		}
	}

	//pr_info("%s-%d :: %d -> %d scale, %d => %d\n", __func__, __LINE__,
	//	src_height, dst_height, plus_temp, plus_height);

	return 0x4U;
	//return plus_height;
}
#endif

void VIOC_SC_SetDstSize(
	void __iomem *reg, unsigned int nWidth, unsigned int nHeight)
{
	u32 val;

	val = ((nHeight << SCDSIZE_HEIGHT_SHIFT)
	       | (nWidth << SCDSIZE_WIDTH_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + SCDSIZE);
}

void VIOC_SC_SetOutSize(
	void __iomem *reg, unsigned int nWidth, unsigned int nHeight)
{
	u32 val;

	val = ((nHeight << SCOSIZE_HEIGHT_SHIFT)
	       | (nWidth << SCOSIZE_WIDTH_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + SCOSIZE);
}

void VIOC_SC_SetOutPosition(
	void __iomem *reg, unsigned int nXpos, unsigned int nYpos)
{
	u32 val;

	val = ((nYpos << SCOPOS_YPOS_SHIFT) | (nXpos << SCOPOS_XPOS_SHIFT));
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + SCOPOS);
}

void VIOC_SCALER_DUMP(void __iomem *reg, unsigned int vioc_id)
{
	unsigned int cnt = 0;

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_err("SCALER-%u :: 0x%p\n", vioc_id, reg);
	while (cnt < 0x20U) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("0x%p: 0x%08x\n", reg + cnt,
			       __raw_readl(reg + cnt));
		cnt += 0x4U;

	}
}

void __iomem *VIOC_SC_GetAddress(unsigned int vioc_id)
{
	unsigned int Num = get_vioc_index(vioc_id);
	void __iomem *ret = NULL;

	if (Num >= VIOC_SCALER_MAX) {
		/* Prevent KCS warning */
		ret = NULL;
	} else {
		ret = phys_to_virt((HwVIOC_SC0 + (SC_OFFSET * (unsigned long)Num)));
	}

	return ret;
}
