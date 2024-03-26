// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/vioc_global.h>
#include <asm/arch/vioc/vioc_outcfg.h>
#include <asm/arch/vioc/reg_physical.h>
#include <linux/bug.h>

/*
 * b00 : Display Device 0 Component
 * b01 : Display Device 1 Component
 * b10 : Display Device 2 Component
 * b11 : NOT USED
 */

void VIOC_OUTCFG_SetOutConfig(unsigned int nType, unsigned int nDisp)
{
	void __iomem *reg = VIOC_OUTCONFIG_GetAddress();
	u32 val;
	int ret = -1;

	if (reg == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][OUTCFG] %s pOUTCFG_reg is NULL\n", __func__);
	} else {
		nDisp = get_vioc_index(nDisp);
		//pr_info("[INF][OUTCFG] %s : addr:%lx nType:%d nDisp:%d\n", __func__,
			//(unsigned long)reg, nType, nDisp);
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		switch (nType) {
		case VIOC_OUTCFG_HDMI:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + MISC) & ~(MISC_HDMISEL_MASK));
			val |= ((nDisp & 0x3U) << MISC_HDMISEL_SHIFT);
			ret = 0;
			break;
		case VIOC_OUTCFG_SDVENC:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + MISC) & ~(MISC_SDVESEL_MASK));
			val |= ((nDisp & 0x3U) << MISC_SDVESEL_SHIFT);
			ret = 0;
			break;
		case VIOC_OUTCFG_HDVENC:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + MISC) & ~(MISC_HDVESEL_MASK));
			val |= ((nDisp & 0x3U) << MISC_HDVESEL_SHIFT);
			ret = 0;
			break;
		case VIOC_OUTCFG_M80:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + MISC) & ~(MISC_M80SEL_MASK));
			val |= ((nDisp & 0x3U) << MISC_M80SEL_SHIFT);
			ret = 0;
			break;
		case VIOC_OUTCFG_MRGB:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + MISC) & ~(MISC_MRGBSEL_MASK));
			val |= ((nDisp & 0x3U) << MISC_MRGBSEL_SHIFT);
			ret = 0;
			break;
		default:
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			WARN_ON(1);
			ret = -1;
			break;
		}
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("[ERR][OUTCFG] %s, wrong type(0x%08x)\n", __func__,
				nType);
		} else {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			__raw_writel(val, reg + MISC);
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_info("%s(OUTCFG.MISC=0x%08x)\n", __func__, val);
		}
	}
}

void __iomem *VIOC_OUTCONFIG_GetAddress(void)
{
	return phys_to_virt(HwVIOC_OUTCFG);
}
