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
#include <telechips/vioc.h>

#define TCC_LUT_DEBUG_TABLE 0

#define drp_table_info(msg...)                        \
	if (TCC_LUT_DEBUG_TABLE == 1) {                        \
		pr_info(msg);                                 \
	}

void __iomem *lut_get_address(unsigned int lut_n, int *is_dev)
{
	void __iomem *reg = phys_to_virt(HwVIOCLUT_BASE);

	if (reg != NULL) {
		switch (lut_n) {
		case VIOC_LUT_DEV0:
			reg = (void __iomem *)(reg + (LUT_DEV0_CFG_OFFSET));
			*is_dev = LUT_TYPE_DEV;
			break;
	#if !defined(CONFIG_TCC750X)
		case VIOC_LUT_DEV1:
			reg = (void __iomem *)(reg + (LUT_DEV1_CFG_OFFSET));
			*is_dev = LUT_TYPE_DEV;
			break;
		case VIOC_LUT_DEV2:
			reg = (void __iomem *)(reg + (LUT_DEV2_CFG_OFFSET));
			*is_dev = LUT_TYPE_DEV;
			break;
		#if defined(CONFIG_TCC805X)
		case VIOC_LUT_DEV3:
			reg = (void __iomem *)(reg + (LUT_DEV3_CFG_OFFSET));
			*is_dev = LUT_TYPE_DEV;
			break;
		#endif
	#endif
		case VIOC_LUT_COMP0:
			reg = (void __iomem *)(reg + (LUT_VIOC0_CFG_OFFSET));
			*is_dev = LUT_TYPE_COMP;
			break;
	#if !defined(CONFIG_TCC750X)
		case VIOC_LUT_COMP1:
			reg = (void __iomem *)(reg + (LUT_VIOC1_CFG_OFFSET));
			*is_dev = LUT_TYPE_COMP;
			break;
	#endif
		default:
			(void)pr_err("[ERR][LUT] %s lut number 0x%x is out of range\n", __func__, lut_n);
			reg = NULL;
			*is_dev = LUT_TYPE_NONE;
			break;
		}
	}

	return reg;
}

int lut_get_pluginComponent_index(unsigned int tvc_n)
{
	int ret = -1;
	unsigned int temp = 0U; /* avoid MISRA C-2012 Rule 10.8 */

	switch (get_vioc_type(tvc_n)) {
	case get_vioc_type(VIOC_RDMA):
#if defined(CONFIG_TCC750X)
		{
			if ((get_vioc_index(tvc_n)) <= 6U) {
				temp = get_vioc_index(tvc_n);
				ret = (int)temp;
			} else {
				(void)pr_err("%s component is wrong. type(%d) index(%d)\n", __func__, get_vioc_type(tvc_n), get_vioc_index(tvc_n));
				ret = -1;
			}
		}
		break;
#else
		switch (get_vioc_index(tvc_n)) {
		case 16:
			ret = 17;
			break;
		case 17:
			ret = 19;
			break;
		default:
			temp = get_vioc_index(tvc_n);
			ret = (int)temp;
			break;
		}
		break;
	case get_vioc_type(VIOC_VIN):
		switch (get_vioc_index(tvc_n)) {
		case 0:
			ret = 16;
			break;
		case 1:
			ret = 18;
			break;
		default:
			(void)pr_err("%s component is wrong. type(%d) index(%d)\n", __func__, get_vioc_type(tvc_n), get_vioc_index(tvc_n));
			ret = -1;
			break;
		}
		break;
#endif
	case get_vioc_type(VIOC_WDMA):
#if defined(CONFIG_TCC750X)
		{
			if ((get_vioc_index(tvc_n)) <= 2U) {
				temp = (7U + (get_vioc_index(tvc_n)));
				ret = (int)temp;
			} else {
				(void)pr_err("%s component is wrong. type(%d) index(%d)\n", __func__, get_vioc_type(tvc_n), get_vioc_index(tvc_n));
				ret = -1;
			}
		}
		break;
#elif defined(CONFIG_TCC805X)
		switch (get_vioc_index(tvc_n)) {
		case (get_vioc_index(VIOC_WDMA13)):
			temp = 0x1D;
			ret = (int)temp;
			break;

		default:
			temp = (20U + (get_vioc_index(tvc_n)));
			ret = (int)temp;
			break;
		}
		break;
#else
		/* avoid MISRA C-2012 Rule 10.8 */
		temp = (20U + (get_vioc_index(tvc_n)));
		ret = (int)temp;
		break;
#endif
	default:
		(void)pr_err("%s component type(%d) is wrong. \n", __func__, get_vioc_type(tvc_n));
		break;
	}

	return ret;
}

int lut_get_Component_index_to_tvc(unsigned int plugin_n)
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
	}
#if defined(CONFIG_TCC805X)
	else if (plugin_n == 0x1DU) {
		ret = (int)VIOC_WDMA13;
	}
#endif
	else {
		/* Prevent KCS warning */
		ret = -EINVAL;
	}

	return ret;
}

void tcc_set_lut_table(unsigned int lut_n, const unsigned int *table)
{
	void __iomem *reg = phys_to_virt(HwVIOCLUT_BASE);
	unsigned int i = 0U, reg_off = 0U;
	unsigned int lut_index = 0U;
	unsigned int val = 0U;

	// lut table select
	lut_index = get_vioc_index(lut_n);
	val = (lut_index << LUT_CTRL_SEL_SHIFT);
	__raw_writel(val, reg + LUT_CTRL_OFFSET);

	// lut table setting
	for (i = 0U; i < LUT_TABLE_SIZE; i++) {
		reg_off = (0xFFU & i);
		/* avoid CERT-C Integers Rule INT30-C */
		if (reg_off <= 0xFFU) {
			__raw_writel(table[i], (reg + LUT_TABLE_OFFSET) + (reg_off * 0x4U));
		}
	}
}

int tcc_set_lut_plugin(unsigned int lut_n, unsigned int plugComp)
{
	void __iomem *reg = NULL;

	int plugin, ret = 0;
	int is_dev = LUT_TYPE_NONE;
	unsigned int lut_cfg_val = 0U;

	reg = lut_get_address(lut_n, &is_dev);
	if (reg == NULL) {
		(void)pr_err("[ERR][LUT] %s lut number %d is out of range\n", __func__, lut_n);
		ret = -EINVAL;
	} else {
		if (is_dev == LUT_TYPE_DEV) {
			(void)pr_err("[ERR][LUT] %s lut number %d is out of range\n", __func__, lut_n);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		plugin = lut_get_pluginComponent_index(plugComp);
		if (plugin < 0) {
			(void)pr_err("%s plugcomp(0x%x) is out of range\n", __func__, plugComp);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		lut_cfg_val |= ((unsigned int)plugin << LUT_VIOC_CFG_SEL_SHFTT);

		__raw_writel(lut_cfg_val, reg);
	}

	return ret;
}

int tcc_get_lut_plugin(unsigned int lut_n)
{
	const void __iomem *reg = NULL;
	unsigned int value = 0U;
	int ret = 0;
	int is_dev = -1;

	reg = lut_get_address(lut_n, &is_dev);
	if (reg == NULL) {
		(void)pr_err("[ERR][LUT] %s lut number %d is out of range\n", __func__, lut_n);
		ret = -EINVAL;
	}

	if (ret == 0) {
		if (is_dev == LUT_TYPE_DEV) {
			(void)pr_err("[ERR][LUT] %s lut of display controller (%d)\n", __func__, lut_n);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		value = __raw_readl(reg);
		ret = lut_get_Component_index_to_tvc((value & (LUT_VIOC_CFG_SEL_MASK)));
	}

	return ret;
}

void tcc_set_lut_enable(unsigned int lut_n, unsigned int enable)
{
	void __iomem *reg = NULL;
	int is_dev = -1;
	unsigned int val = 0;
	unsigned int i = 0U, reg_off = 0U;
	unsigned int lut_index = 0U;

	reg = lut_get_address(lut_n, &is_dev);
	if (reg == NULL) {
		(void)pr_err("[ERR][LUT] %s lut number %d is out of range\n", __func__, lut_n);
	} else {
		switch (is_dev) {
		case LUT_TYPE_COMP:
			{
				if (enable > 0U) {
					val = __raw_readl(reg) | (LUT_VIOC_CFG_EN_MASK);
					__raw_writel(val, reg);
				} else {
					val = __raw_readl(reg) & (~(LUT_VIOC_CFG_EN_MASK));
					__raw_writel(val, reg);
				}
			}
			break;
		case LUT_TYPE_DEV:
			{
				if (enable > 0U) {
					val = __raw_readl(reg) | (LUT_DEV_CFG_EN_MASK);
					__raw_writel(val, reg);
				} else {
					val = __raw_readl(reg) & (~(LUT_DEV_CFG_EN_MASK));
					__raw_writel(val, reg);

					//Chip Bug
					reg = phys_to_virt(HwVIOCLUT_BASE);
					lut_index = get_vioc_index(lut_n);
					val = (lut_index << LUT_CTRL_SEL_SHIFT);
					__raw_writel(val, reg + LUT_CTRL_OFFSET);

					// lut table setting
					for (i = 0U; i < LUT_TABLE_SIZE; i++) {
						reg_off = (0xFFU & i);
						val = (i << 16) | (i << 8) | i;
						__raw_writel(val, (reg + LUT_TABLE_OFFSET) + (reg_off * 0x4U));
					}
				}
			}
			break;
		default:
			(void)pr_err("[ERR][LUT] %s lut number %d is out of range\n", __func__, lut_n);
			break;
		}
	}
}

int tcc_get_lut_enable(unsigned int lut_n)
{
	const void __iomem *reg;
	int is_dev = -1;
	int ret = -EINVAL;

	reg = lut_get_address(lut_n, &is_dev);
	if (reg == NULL) {
		(void)pr_err("[ERR][LUT] %s lut number %d is out of range\n", __func__, lut_n);
	} else {
		switch (is_dev) {
		case LUT_TYPE_COMP:
			{
				if ((__raw_readl(reg) & ((u32)LUT_VIOC_CFG_EN_MASK)) != 0U) {
					ret = 1;
				} else {
					ret = 0;
				}
			}
			break;
		case LUT_TYPE_DEV:
			{
				if ((__raw_readl(reg) & ((u32)LUT_DEV_CFG_EN_MASK)) != 0U) {
					ret = 1;
				} else {
					ret = 0;
				}
			}
			break;
		default:
			(void)pr_err("[ERR][LUT] %s lut number %d is out of range\n", __func__, lut_n);
			break;
		}
	}

	return ret;
}
