// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <linux/err.h>
#include <asm/io.h>
#include <asm/arch/vioc/vioc_global.h>
#include <asm/arch/vioc/reg_physical.h>
#include <asm/arch/vioc/vioc_lut.h>
#include <telechips/fb_dm.h>

#define REG_VIOC_LUT(offset) (phys_to_virt(HwVIOCLUT_BASE + (offset)))
#define LUT_CTRL_R REG_VIOC_LUT(0UL)
#define LUT_CONFIG_R(x) REG_VIOC_LUT(0x04UL + (4UL * x))
#define LUT_TABLE_R REG_VIOC_LUT(0x400U)

// LUT Control
#define L_TABLE_SEL_SHIFT 0U
#define L_TABLE_SEL_MASK (0xFU << L_TABLE_SEL_SHIFT)

// LUT VIOCk Config
#define L_CONFIG_SEL_SHIFT 0U
#define L_CONFIG_SEL_MASK (0xFFU << L_CONFIG_SEL_SHIFT)

#define L_CFG_EN_SHIFT 31U
#define L_CFG_EN_MASK ((u32)0x1U << L_CFG_EN_SHIFT)

// vioc lut config register write & read
#define lut_writel __raw_writel
#define lut_readl __raw_readl

#define TCC_LUT_DEBUG_TABLE 0

#define drp_table_info(msg...)                        \
	if (TCC_LUT_DEBUG_TABLE == 1) {                        \
		pr_info(msg);                                 \
	}

static int lut_get_pluginComponent_index(unsigned int tvc_n)
{
	int ret = -1;
	unsigned int temp = 0U; /* avoid MISRA C-2012 Rule 10.8 */

	switch (get_vioc_type(tvc_n)) {
	case get_vioc_type(VIOC_RDMA):
		switch (get_vioc_index(tvc_n)) {
#if !defined(CONFIG_TCC750X)
		case 16:
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_info(" >>plugin to rdma16\n");
			ret = 17;
			break;
		case 17:
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_info(" >>plugin to rdma17\n");
			ret = 19;
			break;
#endif
		default:
			/* avoid MISRA C-2012 Rule 10.8 */
			temp = get_vioc_index(tvc_n);
			ret = (int)temp;
			break;
		}
		break;
#if !defined(CONFIG_TCC750X)
	case get_vioc_type(VIOC_VIN):
		switch (get_vioc_index(tvc_n)) {
		case 0:
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_info(" >>plugin to vin0\n");
			ret = 16;
			break;
		case 1:
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_info(" >>plugin to vin1\n");
			ret = 18;
			break;
		default:
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("%s component is wrong. type(%u) index(%u)\n",
			__func__, get_vioc_type(tvc_n), get_vioc_index(tvc_n));
			ret = -1;
			break;
		}
		break;
#endif
	case get_vioc_type(VIOC_WDMA):
		/* avoid MISRA C-2012 Rule 10.8 */
		temp = (20U + (get_vioc_index(tvc_n)));
		ret = (int)temp;
		break;
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("%s component type(%u) is wrong. \n",
			__func__, get_vioc_type(tvc_n));
		ret = -1;
		break;
	}

	return ret;
}

static int lut_get_Component_index_to_tvc(unsigned int plugin_n)
{
	int ret = -1;

	if (plugin_n <= 0xfU) {
		/* Prevent KCS warning */
		ret = (int)VIOC_RDMA00 + (int)plugin_n;
	}
#if !defined(CONFIG_TCC750X)
	else if (plugin_n == 0x10U) {
		/* Prevent KCS warning */
		ret = (int)VIOC_VIN00;
	} else if (plugin_n == 0x11U) {
		/* Prevent KCS warning */
		ret = (int)VIOC_RDMA16;
	} else if (plugin_n == 0x12U) {
		/* Prevent KCS warning */
		ret = (int)VIOC_VIN10;
	} else if (plugin_n == 0x13U) {
		/* Prevent KCS warning */
		ret = (int)VIOC_RDMA17;
	}
#endif
	else if (plugin_n <= 0x1CU) {
		/* Prevent KCS warning */
		ret = ((int)VIOC_WDMA00 + ((int)plugin_n - 0x14));
	} else {
		/* Prevent KCS warning */
		ret = -1;
	}

	return ret;
}


void tcc_set_lut_table(unsigned int lut_n, const unsigned int *table)
{
	unsigned int i, reg_off, lut_index;
	void __iomem *table_reg = LUT_TABLE_R;
	void __iomem *ctrl_reg = LUT_CTRL_R;

	// lut table select
	lut_index = get_vioc_index(lut_n);
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	lut_writel(lut_index, ctrl_reg);

	// lut table setting
	for (i = 0U; i < LUT_TABLE_SIZE; i++) {
		reg_off = (0xFFU & i);
		/* avoid CERT-C Integers Rule INT30-C */
		if (reg_off < (UINT_MAX / 4U)) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			lut_writel(table[i], table_reg + (reg_off * 0x4U));
		}
	}
}

int tcc_set_lut_plugin(unsigned int lut_n, unsigned int plugComp)
{
	void __iomem *reg;
	int plugin, ret = -1;
	unsigned int value, lut_index;

	lut_index = get_vioc_index(lut_n);
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_info("%s lut_index_%u\r\n", __func__, lut_index);
	// select lut config register

	reg = LUT_CONFIG_R(lut_index);
	value = lut_readl(reg);
	plugin = lut_get_pluginComponent_index(plugComp);
	if (plugin < 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("%s plugcomp(0x%x) is out of range \r\n", __func__,
		       plugComp);
		ret = -1;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}
	value = (value & ~(L_TABLE_SEL_MASK))
		| ((unsigned int)plugin << L_TABLE_SEL_SHIFT);
	lut_writel(value, reg);
	ret = 0;

FUNC_EXIT:
	return ret;
}

int tcc_get_lut_plugin(unsigned int lut_n)
{
	void __iomem *reg = NULL;
	unsigned int value, lut_index;
	int ret;
	void *tmp_pLUT = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pLUT = reg;
	reg = tmp_pLUT;

	lut_index = get_vioc_index(lut_n);
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_info("%s lut_index = %u\r\n", __func__, lut_index);

	reg = LUT_CONFIG_R(lut_index);
	value = lut_readl(reg);

	ret = lut_get_Component_index_to_tvc(value & (L_CONFIG_SEL_MASK));

	return ret;
}

void tcc_set_lut_enable(unsigned int lut_n, unsigned int enable)
{
	void __iomem *reg = NULL;
	unsigned int lut_index;
	void *tmp_pLUT = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pLUT = reg;
	reg = tmp_pLUT;

	lut_index = get_vioc_index(lut_n);

	reg = LUT_CONFIG_R(lut_index);

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_info("%s lut_index_%u %s\r\n", __func__, lut_index,
		 (enable == 1U) ? "enable" : "disable");
	// enable , disable
	if (enable == 1U) {
		u32 val = 0U;

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		lut_writel(val | (L_CFG_EN_MASK), reg);
	} else {
		u32 val = 0U;

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		lut_writel(val & (~(L_CFG_EN_MASK)), reg);
	}
}

int tcc_get_lut_enable(unsigned int lut_n)
{
	void __iomem *reg = NULL;
	unsigned int lut_index = get_vioc_index(lut_n);
	int ret = -1;
	void *tmp_pLUT = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pLUT = reg;
	reg = tmp_pLUT;

	reg = LUT_CONFIG_R(lut_index);

	// enable , disable
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	if ((lut_readl(reg) & (L_CFG_EN_MASK)) != 0U) {
		/* Prevent KCS warning */
		ret = 1;
	} else {
		/* Prevent KCS warning */
		ret = 0;
	}

	return ret;
}
