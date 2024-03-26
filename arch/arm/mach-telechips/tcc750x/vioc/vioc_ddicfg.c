// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/vioc_ddicfg.h>
#include <asm/arch/vioc/reg_physical.h>
#include <mach/chipinfo.h>

void VIOC_DDICONFIG_SetPWDN(unsigned int type, unsigned int set)
{
	void __iomem *reg = phys_to_virt(HwDDI_CONFIG_BASE);
	u32 val;
	void *tmp_pDDICONFIG = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDDICONFIG = reg;
	reg = tmp_pDDICONFIG;

	switch (type) {
	case DDICFG_TYPE_VIOC:
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + DDI_PWDN) & ~(PWDN_VIOC_MASK));
		val |= ((set & 0x1U) << PWDN_VIOC_SHIFT);
		__raw_writel(val, reg + DDI_PWDN);
		break;
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("%s: Wrong type:%u\n", __func__, type);
		break;
	}
}

void VIOC_DDICONFIG_SetSWRESET(unsigned int type, unsigned int set)
{
	void __iomem *reg = phys_to_virt(HwDDI_CONFIG_BASE);
	u32 val;

	switch (type) {
	case DDICFG_TYPE_VIOC:
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + SWRESET) & ~(SWRESET_VIOC_MASK));
		val |= ((set & 0x1U) << SWRESET_VIOC_SHIFT);
		__raw_writel(val, reg + SWRESET);
		break;
	case DDICFG_TYPE_DP_AXI:
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + SWRESET) & ~(SWRESET_DP_AXI_MASK));
		val |= ((set & 0x1U) << SWRESET_DP_AXI_SHIFT);
		__raw_writel(val, reg + SWRESET);
		break;
	case DDICFG_TYPE_ISP_AXI:
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + SWRESET) & ~(SWRESET_ISP_AXI_MASK));
		val |= ((set & 0x1U) << SWRESET_ISP_AXI_SHIFT);
		__raw_writel(val, reg + SWRESET);
		break;
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("%s: Wrong type:%u\n", __func__, type);
		break;
	}
}

void VIOC_DDICONFIG_SetPeriClock(unsigned int num, unsigned int set)
{
	void __iomem *reg = phys_to_virt(HwDDI_CONFIG_BASE);
	u32 val;
	void *tmp_pDDICONFIG = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDDICONFIG = reg;
	reg = tmp_pDDICONFIG;

	if (num > 2U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][DDICONFIG] %s num(%u) is wrong\n", __func__, num);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + DDI_PWDN) & ~((u32)0x1U << (PWDN_L0S_SHIFT + num)));
		val |= ((set & 0x1U) << (PWDN_L0S_SHIFT + num));
		__raw_writel(val, reg + DDI_PWDN);
	}
}

#if !defined(CONFIG_TCC750X)
unsigned int VIOC_DDICONFIG_GetViocRemap(void)
{
	void __iomem *reg = phys_to_virt(HwDDI_CONFIG_BASE);
	u32 val = 0U;
	void *tmp_pDDICONFIG = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pDDICONFIG = reg;
	reg = tmp_pDDICONFIG;

	if (get_chip_rev() != 0U) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg + VIOC_REMAP);
		//val = 1U;
	} else {
		/* Prevent KCS warning */
		val = 0U;
	}

#if 0
	pr_err("%s: chip(%s) remap(%s)\n", __func__,
		get_chip_rev() ? "CS" : "ES",
		val ? "on" : "off");
#endif

	return val;
}

unsigned int VIOC_DDICONFIG_SetViocRemap(unsigned int enable)
{
	u32 val = 0;

	if (get_chip_rev() != 0U) {
		void __iomem *reg = phys_to_virt(HwDDI_CONFIG_BASE);

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((enable & VIOC_REMAP_MASK), reg + VIOC_REMAP);

		val = __raw_readl(reg + VIOC_REMAP);
	}

	return val;
}
#endif

void VIOC_DDICONFIG_DUMP(void)
{
	unsigned int cnt = 0;
	void __iomem *reg = phys_to_virt(HwDDI_CONFIG_BASE);

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_err("[DBG][DDICFG] DDICONFIG :: 0x%p\n", reg);
	while (cnt < 0x50U) {
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

void __iomem *VIOC_DDICONFIG_GetAddress(void)
{
	return phys_to_virt(HwDDI_CONFIG_BASE);
}

