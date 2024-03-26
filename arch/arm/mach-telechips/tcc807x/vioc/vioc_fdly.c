// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/vioc_fdly.h>
#include <asm/arch/vioc/vioc_global.h>
#include <asm/arch/vioc/vioc_scaler.h>
#include <asm/arch/vioc/vioc_config.h>
#include <asm/arch/vioc/reg_physical.h>

void VIOC_FDLY_SetFormat(void __iomem *reg, unsigned int fmt)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = __raw_readl(reg + FDLYCTRL) & ~(FDLYCTRL_FMT_MASK);
	value |= (fmt << FDLYCTRL_FMT_SHIFT);
	__raw_writel(value, reg + FDLYCTRL);
}

void VIOC_FDLY_SetRate(void __iomem *reg, unsigned int rate)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = __raw_readl(reg + FDLYRATE) & ~(FDLYRATE_MAXRATE_MASK);
	value |= (rate << FDLYRATE_MAXRATE_SHIFT);
	__raw_writel(value, reg + FDLYRATE);
}

void VIOC_FDLY_SetRateEnable(void __iomem *reg, unsigned int en)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = __raw_readl(reg + FDLYRATE) & ~(FDLYRATE_REN_MASK);
	value |= (en << FDLYRATE_REN_SHIFT);
	__raw_writel(value, reg + FDLYRATE);
}

void VIOC_FDLY_SetBase0(void __iomem *reg, unsigned int baseAddress)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = __raw_readl(reg + FDLYRBASE0) & ~(FDLYBASA_BASE0_MASK);
	value |= (baseAddress << FDLYBASE_BASE0_SHIFT);
	__raw_writel(value, reg + FDLYRBASE0);
}

void VIOC_FDLY_SetBase1(void __iomem *reg, unsigned int baseAddress)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = __raw_readl(reg + FDLYRBASE1) & ~(FDLYBASA_BASE1_MASK);
	value |= (baseAddress << FDLYBASE_BASE1_SHIFT);
	__raw_writel(value, reg + FDLYRBASE1);
}

void VIOC_FDLY_SetDefaultColor(void __iomem *reg, unsigned int bg3,
		unsigned int bg2, unsigned int bg1, unsigned int bg0)
{
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = __raw_readl(reg + FDLYRFLT) & ~(FDLYDFLT_BG3_MASK |
			FDLYDFLT_BG2_MASK | FDLYDFLT_BG1_MASK |
			FDLYDFLT_BG0_MASK);
	value |= ((bg3 << FDLYDFLT_BG3_SHIFT) | (bg2 << FDLYDFLT_BG2_SHIFT) |
			(bg1 << FDLYDFLT_BG1_SHIFT) | (bg0 << FDLYDFLT_BG0_SHIFT));
	__raw_writel(value, reg + FDLYRFLT);
}

void VIOC_FDLY_Dump(void __iomem *reg)
{
	unsigned int cnt = 0;
	void *tmp_pFDLY = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pFDLY = reg;
	reg = tmp_pFDLY;

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_err("VIOC_FDLY Dump\n");

	while (cnt < 0x1CU) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("0x%x:\n", __raw_readl(reg+cnt));
		cnt += 0x04U;
	}
}

void VIOC_FDLY_PlugIn(unsigned int sc, unsigned int en)
{
	void __iomem *config_reg;
	u32 value;
	unsigned int sc_n;

	config_reg = VIOC_IREQConfig_GetAddress();

	sc_n = get_vioc_index(sc);

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_info("Scaler num by frame delay %u\n", sc_n);
	value = ((en << CFG_PATH_FDLY_EN_SHIFT) |
			(sc_n << CFG_PATH_FDLY_SEL_SHIFT));

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_info("VIOC_FDLY_Plugin Value [0x%x] to [0x%p]\n",
		value, config_reg + CFG_PATH_FDLY_OFFSET);

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(value, config_reg + CFG_PATH_FDLY_OFFSET);
}

void __iomem *VIOC_FDLY_GetAddress(void)
{
	/*
	 *	int Num = get_vioc_index(vioc_id);
	 */
	return phys_to_virt(HwVIOC_FDLY);
}
