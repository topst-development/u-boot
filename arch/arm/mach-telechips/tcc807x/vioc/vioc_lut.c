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
		default:
			/* avoid MISRA C-2012 Rule 10.8 */
			temp = get_vioc_index(tvc_n);
			ret = (int)temp;
			break;
		}
		break;
#if 0
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

	if (plugin_n <= 0xDU) {
		/* Prevent KCS warning */
		ret = (int)VIOC_RDMA00 + (int)plugin_n;
	}
#if 0
	else if (plugin_n == 0x10U) {
		/* Prevent KCS warning */
		ret = (int)VIOC_VIN00;
	} else if (plugin_n == 0x12U) {
		/* Prevent KCS warning */
		ret = (int)VIOC_VIN10;
	}
#endif
	else if (plugin_n <= 0x11U) {
		/* Prevent KCS warning */
		ret = ((int)VIOC_WDMA00 + ((int)plugin_n - 0xE));
	} else {
		/* Prevent KCS warning */
		ret = -1;
	}

	return ret;
}


void tcc_set_lut_table(unsigned int lut_n, const unsigned int *table)
{
	unsigned int i, reg_off, lut_index;
	void __iomem *reg = phys_to_virt(HwVIOCLUT_BASE);
	u32 val;

	// lut table select
	lut_index = get_vioc_index(lut_n);
	val = (lut_index << LUT_CTRL_SEL_SHIFT);
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(val, reg + LUT_CTRL_OFFSET);

	// lut table setting
	for (i = 0U; i < LUT_TABLE_SIZE; i++) {
		reg_off = (0xFFU & i);
		/* avoid CERT-C Integers Rule INT30-C */
		if (reg_off < (UINT_MAX / 4U)) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			__raw_writel(table[i], (reg + LUT_TABLE_OFFSET) + (reg_off * 0x4U));
		}
	}
}

int tcc_set_lut_plugin(unsigned int lut_n, unsigned int plugComp)
{
	void __iomem *reg = phys_to_virt(HwVIOCLUT_BASE);
	int plugin, ret = -1;
	unsigned int value, lut_index;

	lut_index = get_vioc_index(lut_n);
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_info("%s lut_index_%u\r\n", __func__, lut_index);
	// select lut config register

	value = __raw_readl(reg + LUT_DEV0_CFG_OFFSET + (0x4U * lut_index));
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
	value = (value & ~(LUT_CTRL_SEL_MASK))
		| ((unsigned int)plugin << LUT_CTRL_SEL_SHIFT);
	__raw_writel(value, reg + LUT_DEV0_CFG_OFFSET + (0x4U * lut_index));
	ret = 0;

FUNC_EXIT:
	return ret;
}

int tcc_get_lut_plugin(unsigned int lut_n)
{
	void __iomem *reg = phys_to_virt(HwVIOCLUT_BASE);
	unsigned int value, lut_index;
	int ret;

	lut_index = get_vioc_index(lut_n);
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_info("%s lut_index = %u\r\n", __func__, lut_index);

	value = __raw_readl(reg + LUT_DEV0_CFG_OFFSET + (0x4U * lut_index));

	ret = lut_get_Component_index_to_tvc(value & (LUT_VIOC_CFG_SEL_MASK));

	return ret;
}

void tcc_set_lut_enable(unsigned int lut_n, unsigned int enable)
{
	void __iomem *reg = phys_to_virt(HwVIOCLUT_BASE);
	unsigned int lut_index;

	lut_index = get_vioc_index(lut_n);

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
		val = __raw_readl(reg + LUT_DEV0_CFG_OFFSET + (0x4U * lut_index));
		__raw_writel(val | (LUT_VIOC_CFG_EN_MASK), reg + LUT_DEV0_CFG_OFFSET + (0x4U * lut_index));
	} else {
		u32 val = 0U;

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg + LUT_DEV0_CFG_OFFSET + (0x4U * lut_index));
		__raw_writel(val & (~(LUT_VIOC_CFG_EN_MASK)), reg + LUT_DEV0_CFG_OFFSET + (0x4U * lut_index));
	}
}

int tcc_get_lut_enable(unsigned int lut_n)
{
	void __iomem *reg = phys_to_virt(HwVIOCLUT_BASE);
	unsigned int lut_index = get_vioc_index(lut_n);
	int ret = -1;

	// enable , disable
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	if ((__raw_readl(reg + LUT_DEV0_CFG_OFFSET + (0x4U * lut_index)) & (LUT_VIOC_CFG_EN_MASK)) != 0U) {
		/* Prevent KCS warning */
		ret = 1;
	} else {
		/* Prevent KCS warning */
		ret = 0;
	}

	return ret;
}
