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
#include <telechips/vioc.h>

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
		pr_err("[ERR][OUTCFG] %s pOUTCFG_reg is NULL\n", __func__);
	} else {
		nDisp = get_vioc_index(nDisp);
		//pr_info("[INF][OUTCFG] %s : addr:%lx nType:%d nDisp:%d\n", __func__,
			//(unsigned long)reg, nType, nDisp);

		switch (nType) {
		case VIOC_OUTCFG_HDMI:
			val = (__raw_readl(reg + MISC) & ~(MISC_HDMISEL_MASK));
			val |= ((nDisp & 0x3U) << MISC_HDMISEL_SHIFT);
			ret = 0;
			break;
		case VIOC_OUTCFG_SDVENC:
			val = (__raw_readl(reg + MISC) & ~(MISC_SDVESEL_MASK));
			val |= ((nDisp & 0x3U) << MISC_SDVESEL_SHIFT);
			ret = 0;
			break;
		case VIOC_OUTCFG_HDVENC:
			val = (__raw_readl(reg + MISC) & ~(MISC_HDVESEL_MASK));
			val |= ((nDisp & 0x3U) << MISC_HDVESEL_SHIFT);
			ret = 0;
			break;
		case VIOC_OUTCFG_M80:
			val = (__raw_readl(reg + MISC) & ~(MISC_M80SEL_MASK));
			val |= ((nDisp & 0x3U) << MISC_M80SEL_SHIFT);
			ret = 0;
			break;
		case VIOC_OUTCFG_MRGB:
			val = (__raw_readl(reg + MISC) & ~(MISC_MRGBSEL_MASK));
			val |= ((nDisp & 0x3U) << MISC_MRGBSEL_SHIFT);
			ret = 0;
			break;
		default:
			WARN_ON(1);
			ret = -1;
			break;
		}
		if (ret < 0) {
			pr_err("[ERR][OUTCFG] %s, wrong type(0x%08x)\n", __func__,
				nType);
		} else {
			__raw_writel(val, reg + MISC);
			pr_info("%s(OUTCFG.MISC=0x%08x)\n", __func__, val);
		}
	}
}

void __iomem *VIOC_OUTCONFIG_GetAddress(void)
{
	return phys_to_virt(HwVIOC_OUTCFG);
}
