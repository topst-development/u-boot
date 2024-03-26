// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/reg_physical.h>
#include <asm/arch/vioc/vioc_blk.h>
#include <asm/arch/vioc/vioc_global.h>
#include <asm/arch/vioc/vioc_config.h>
#include <asm/arch/vioc/vioc_scaler.h>
#include <asm/arch/vioc/vioc_ddicfg.h>
#include <asm/arch/vioc/vioc_viqe.h>
#include <mach/chipinfo.h>
#include <linux/delay.h>

static void __iomem *pIREQ_reg = (void __iomem *)(HwVIOC_BASE + 0xF0000);

static int vioc_config_sc_rdma_sel[] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,  0x8,
	0x9, 0xA, 0xB, 0xC, 0xD
};

static int vioc_config_sc_wdma_sel[] = {
	0xE, 0xF, 0x10, 0x11
};

static int vioc_config_viqe_rdma_sel[] = {
	-1, -1, -1, 0x0, -1, -1, -1, 0x1,
	-1, 0x02, 0x03, 0x04, 0x05, 0x06,
};

//static int vioc_config_lut_rdma_sel[] = {
//	0,    0x1,  0x2,  0x03, 0x04, 0x05, -1, 0x07, 0x08, // rdma 0 ~ 8
//	0x09, 0x0A, 0x0B, 0x0C, 0x0D, -1,   -1, 0x11, 0x13,
//};

//static int vioc_config_lut_vin_sel[] = {
//	0x10,
//	-1,
//};
//static int vioc_config_lut_wdma_sel[] = {
//	0x14, 0x15, 0x16, 0x17, -1, 0x19, 0x1A, 0x1B, 0x1C,
//};

#if defined(CONFIG_VIOC_AFBCDEC)
static int vioc_config_AFBCDec_rdma_sel[] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,  0x8,
	0x9, 0xA, 0xB, 0xC, 0xD,
};
#endif

static struct VIOC_PATH_INFO_T vioc_path_info[] = {
	{
		.mix = 4,
		{
			VIOC_RDMA00,
			VIOC_RDMA01,
			VIOC_RDMA02,
			VIOC_RDMA03
		},
		VIOC_WMIX0,
		VIOC_DISP0,
		{
			VIOC_WDMA00,
			0,
		},
	},
	{
		.mix = 4,
		{
			VIOC_RDMA04,
			VIOC_RDMA05,
			VIOC_RDMA06,
			VIOC_RDMA07
		},
		VIOC_WMIX1,
		VIOC_DISP1,
		{
			VIOC_WDMA01,
			0,
		},
	},
	{
		.mix = 2,
		{
			VIOC_RDMA08,
			VIOC_RDMA09,
		},
		VIOC_WMIX2,
		VIOC_DISP2,
		{
			0,
			0,
		},
	},
	{
		.mix = 2,
		{
			VIOC_RDMA10,
			VIOC_RDMA11,
			0,
			0,
		},
		VIOC_WMIX3,
		0,
		{
			VIOC_WDMA02,
			VIOC_WDMA03,
		},
	},
	{
		.mix = 1,
		{
			VIOC_RDMA12,
			0,
			0,
			0,
		},
		0,
		0,
		{
			VIOC_WDMA04,
			0,
		},
	},
	{
		.mix = 1,
		{
			VIOC_RDMA13,
			0,
			0,
			0,
		},
		0,
		0,
		{
			VIOC_WDMA05,
			0,
		},
	},
	{
		.mix = 1,
		{
			VIOC_RDMA14,
			0,
			0,
			0,
		},
		0,
		VIOC_DISP3,
		{
			0,
			0,
		},
	},
	{
		.mix = 1,
		{
			VIOC_RDMA15,
			0,
			0,
			0,
		},
		0,
		VIOC_DISP4,
		{
			0,
			0,
		},
	},

};

static void __iomem *CalcAddressViocComponent(unsigned int component)
{
	void __iomem *reg = NULL;

	switch (get_vioc_type(component)) {
	case get_vioc_type(VIOC_RDMA):
		break;
	case get_vioc_type(VIOC_SCALER):
		switch (get_vioc_index(component)) {
		case 0:
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg = (pIREQ_reg + CFG_PATH_SC0_OFFSET);
			break;
		case 1:
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg = (pIREQ_reg + CFG_PATH_SC1_OFFSET);
			break;
		case 2:
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg = (pIREQ_reg + CFG_PATH_SC2_OFFSET);
			break;
		case 3:
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg = (pIREQ_reg + CFG_PATH_SC3_OFFSET);
			break;
		default:
			reg = NULL;
			break;
		}
		break;
	case get_vioc_type(VIOC_VIQE):
		switch (get_vioc_index(component)) {
		case 0:
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			reg = (pIREQ_reg + CFG_PATH_VIQE0_OFFSET);
			break;
		default:
			reg = NULL;
			break;
		}
		break;
#ifdef CONFIG_VIOC_MAP_DECOMP
	case get_vioc_type(VIOC_MC):
		switch (get_vioc_index(component)) {
		case 0:
			reg = (pIREQ_reg + CFG_PATH_MC_OFFSET);
			break;
		default:
			reg = NULL;
			break;
		}
		break;
#endif
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s-%d: wierd component(0x%x) type(0x%x) index(%u)\n",
		       __func__, __LINE__, component, get_vioc_type(component),
		       get_vioc_index(component));
		reg = NULL;
		break;
	}

	return reg;
}

static int CheckScalerPathSelection(unsigned int component)
{
	int ret = 0;

	switch (get_vioc_type(component)) {
	case get_vioc_type(VIOC_RDMA):
		ret = vioc_config_sc_rdma_sel[get_vioc_index(component)];
		break;
	case get_vioc_type(VIOC_WDMA):
		ret = vioc_config_sc_wdma_sel[get_vioc_index(component)];
		break;
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s, wrong path parameter(0x%08x)\n",
		       __func__, component);
		ret = -1;
		break;
	}
	return ret;
}

static int CheckViqePathSelection(unsigned int component)
{
	int ret = 0;

	switch (get_vioc_type(component)) {
	case get_vioc_type(VIOC_RDMA):
		ret = vioc_config_viqe_rdma_sel[get_vioc_index(component)];
		break;
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s, wrong path parameter(0x%08x)\n",
		       __func__, component);
		ret = -1;
		break;
	}
	return ret;
}

#if defined(CONFIG_VIOC_MAP_DECOMP)
static int CheckMCPathSelection(unsigned int component, unsigned int mc)
{
	int ret = 0;

	if (get_vioc_type(component) != get_vioc_type(VIOC_RDMA)) {
		/* Prevent KCS warning */
		ret = -1;
	}
#if 0
	if (get_vioc_type(mc) == VIOC_MC) {
		if (get_vioc_index(mc) != 0U) {
			// VIOC_MC1 for tcc805x
			if (component == VIOC_WMIX0) {
				/* Prevent KCS warning */
				ret = -2;
			}
		} else {
			if (component > VIOC_WMIX0) {
				// VIOC_MC0 for tcc805x
				/* Prevent KCS warning */
				ret = -2;
			}
		}
	}
#endif
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s, ret:%d wrong path parameter(Path: 0x%08x MC: 0x%08x)\n",
		       __func__, ret, component, mc);
	}

	return ret;
}
#endif

int VIOC_AUTOPWR_Enalbe(unsigned int component, unsigned int onoff)
{
	int shift_bit = -1;
	u32 value = 0;
	int ret = -1;

	switch (get_vioc_type(component)) {
	case get_vioc_type(VIOC_RDMA):
		shift_bit = (int)PWR_AUTOPD_RDMA_SHIFT;
		ret = 0;
		break;

#if defined(CONFIG_VIOC_MAP_DECOMP)
	case get_vioc_type(VIOC_MC):
		shift_bit = (int)PWR_AUTOPD_MC_SHIFT;
		ret = 0;
		break;
#endif
	case get_vioc_type(VIOC_WMIX):
		shift_bit = (int)PWR_AUTOPD_MIX_SHIFT;
		ret = 0;
		break;
	case get_vioc_type(VIOC_WDMA):
		shift_bit = (int)PWR_AUTOPD_WDMA_SHIFT;
		ret = 0;
		break;

	case get_vioc_type(VIOC_SCALER):
		shift_bit = (int)PWR_AUTOPD_SC_SHIFT;
		break;
	case get_vioc_type(VIOC_VIQE):
		shift_bit = (int)PWR_AUTOPD_VIQE_SHIFT;
		ret = 0;
		break;
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][CONFIG] %s component type(%u) is wrong\n",
			__func__, get_vioc_type(component));
		ret = -1;
		break;
	}
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	/* shift_bit >= 0, Always */
	if (onoff != 0U) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(pIREQ_reg + PWR_AUTOPD_OFFSET)
			 | (((u32)1U) << (unsigned int)shift_bit));
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(pIREQ_reg + PWR_AUTOPD_OFFSET)
			 & ~(((u32)1U) << (unsigned int)shift_bit));

		__raw_writel(value, pIREQ_reg + PWR_AUTOPD_OFFSET);
	}
	ret = 0;

FUNC_EXIT:
	return ret;
}

int VIOC_CONFIG_Get_Path_Info(unsigned int wmix_id, struct VIOC_PATH_INFO_T *info)
{
	int ret = -1;

	if (info != NULL) {
		if (get_vioc_index(wmix_id) < VIOC_WMIX_MAX) {
			(void)memcpy(info, &vioc_path_info[get_vioc_index(wmix_id)],
			       sizeof(struct VIOC_PATH_INFO_T));
			ret = 0;
		}
	}
	return ret;
}

int VIOC_CONFIG_ResetAll(void)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
#if defined(PWR_BLK_SWR0_OFFSET)
	__raw_writel(0xFFFFFFFF, pIREQ_reg + PWR_BLK_SWR0_OFFSET);
#endif
#if defined(PWR_BLK_SWR1_OFFSET)
	__raw_writel(0xFFFFFFFF, pIREQ_reg + PWR_BLK_SWR1_OFFSET);
#endif
#if defined(PWR_BLK_SWR2_OFFSET)
	__raw_writel(0xFFFFFFFF, pIREQ_reg + PWR_BLK_SWR2_OFFSET);
#endif
#if defined(PWR_BLK_SWR3_OFFSET)
	__raw_writel(0xFFFFFFFF, pIREQ_reg + PWR_BLK_SWR3_OFFSET);
#endif
#if defined(PWR_BLK_SWR4_OFFSET)
	__raw_writel(0xFFFFFFFF, pIREQ_reg + PWR_BLK_SWR4_OFFSET);
#endif

/* resume */
#if defined(PWR_BLK_SWR4_OFFSET)
	__raw_writel(0x00000000, pIREQ_reg + PWR_BLK_SWR4_OFFSET);
#endif
#if defined(PWR_BLK_SWR3_OFFSET)
	__raw_writel(0x00000000, pIREQ_reg + PWR_BLK_SWR3_OFFSET);
#endif
#if defined(PWR_BLK_SWR2_OFFSET)
	__raw_writel(0x00000000, pIREQ_reg + PWR_BLK_SWR2_OFFSET);
#endif
#if defined(PWR_BLK_SWR1_OFFSET)
	__raw_writel(0x00000000, pIREQ_reg + PWR_BLK_SWR1_OFFSET);
#endif
#if defined(PWR_BLK_SWR0_OFFSET)
	__raw_writel(0x00000000, pIREQ_reg + PWR_BLK_SWR0_OFFSET);
#endif
	return 0;
}

int VIOC_CONFIG_PlugIn(unsigned int component, unsigned int select)
{
	u32 value = 0U;
	unsigned int loop = 0U;
	void __iomem *reg;
	int plugin_path;
	int ret = (int)VIOC_PATH_DISCONNECTED;

	reg = CalcAddressViocComponent(component);
	if (reg == NULL) {
		/* Prevent KCS warning */
		ret = VIOC_DEVICE_INVALID;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	/* Check selection has type value. If has, select value is invalid */
	switch (get_vioc_type(component)) {
	case get_vioc_type(VIOC_SCALER):
		plugin_path = CheckScalerPathSelection(select);
		if (plugin_path < 0) {
			/* Prevent KCS warning */
			ret = VIOC_DEVICE_INVALID;
		} else {
			ret = (int)VIOC_PATH_DISCONNECTED;
		}
		break;
	case get_vioc_type(VIOC_VIQE):
		plugin_path = CheckViqePathSelection(select);
		if (plugin_path < 0) {
			/* Prevent KCS warning */
			ret = VIOC_DEVICE_INVALID;
		} else {
			ret = (int)VIOC_PATH_DISCONNECTED;
		}
		break;
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s, wrong component type:(0x%08x)\n",
		       __func__, component);
		ret = VIOC_DEVICE_INVALID;
		break;
	}
	if (ret == VIOC_DEVICE_INVALID) {
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
	}

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = ((__raw_readl(reg) & CFG_PATH_STS_MASK) >> CFG_PATH_STS_SHIFT);
	if (value == VIOC_PATH_CONNECTED) {
		value =
			((__raw_readl(reg) & CFG_PATH_SEL_MASK)
			 >> CFG_PATH_SEL_SHIFT);
		if (value != (unsigned int)plugin_path) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("[WAN][VIOC_CONFIG] %s, VIOC(T:%u I:%u) is plugged-out by force (from 0x%08x to %d)!!\n",
			       __func__, get_vioc_type(component),
			       get_vioc_index(component), value, plugin_path);
			(void)VIOC_CONFIG_PlugOut(component);
		}
	}

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg) & ~(CFG_PATH_SEL_MASK | CFG_PATH_EN_MASK));
	value |= (((unsigned int)plugin_path << CFG_PATH_SEL_SHIFT) | ((u32)0x1U << CFG_PATH_EN_SHIFT));
	__raw_writel(value, reg);

	if ((__raw_readl(reg) & CFG_PATH_ERR_MASK) != 0U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s, path configuration error(ERR_MASK). device is busy. VIOC(T:%u I:%u) Path:%d\n",
		       __func__, get_vioc_type(component),
		       get_vioc_index(component), plugin_path);
		value = (__raw_readl(reg) & ~(CFG_PATH_EN_MASK));
		__raw_writel(value, reg);
	}

	loop = 50U;
	while ((bool)1) {
		unsigned int ext_flag = 0;

		/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
		/* coverity[cert_dcl37_c_violation : FALSE] */
		mdelay(1);
		loop--;
		value =
			((__raw_readl(reg) & CFG_PATH_STS_MASK)
			 >> CFG_PATH_STS_SHIFT);
		if (value == VIOC_PATH_CONNECTED) {
			ret = (int)VIOC_PATH_CONNECTED;
			ext_flag = 1U;
		} else {
			if (loop < 1U) {
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				pr_err("[ERR][VIOC_CONFIG] %s, path configuration error(TIMEOUT). device is busy. VIOC(T:%u I:%u) Path:%d\n",
				       __func__, get_vioc_type(component),
				       get_vioc_index(component), plugin_path);
				ret = (int)VIOC_DEVICE_BUSY;
				ext_flag = 1U;
			}
		}
		if (ext_flag == 1U) {
			break;
		}
	}

FUNC_EXIT:
	if ((ret != (int)VIOC_PATH_CONNECTED) && (reg != NULL)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s, in error, Type :0x%x Select:0x%x cfg_reg:0x%x\n",
	       __func__, component, select, __raw_readl(reg));
	}
	return ret;
}

int VIOC_CONFIG_PlugOut(unsigned int component)
{
	u32 value = 0U;
	unsigned int loop = 0U;
	void __iomem *reg;
	int ret = (int)VIOC_PATH_CONNECTED;

	reg = CalcAddressViocComponent(component);
	if (reg == NULL) {
		ret = VIOC_DEVICE_INVALID;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = ((__raw_readl(reg) & CFG_PATH_STS_MASK) >> CFG_PATH_STS_SHIFT);
	if (value == VIOC_PATH_DISCONNECTED) {
		__raw_writel(0x00000000, reg);
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_warn("[WAN][VIOC_CONFIG] %s, VIOC(T:%u I:%u) was already plugged-out!!\n",
		       __func__, get_vioc_type(component),
		       get_vioc_index(component));
		ret = (int)VIOC_PATH_DISCONNECTED;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	value = (__raw_readl(reg) & ~(CFG_PATH_EN_MASK));
	__raw_writel(value, reg);

	if ((__raw_readl(reg) & CFG_PATH_ERR_MASK) != 0U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s, path configuration error(ERR_MASK). device is busy. VIOC(T:%u I:%u)\n",
		       __func__, get_vioc_type(component),
		       get_vioc_index(component));
		value = (__raw_readl(reg) & ~(CFG_PATH_EN_MASK));
		__raw_writel(value, reg);
		ret = (int)VIOC_DEVICE_BUSY;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	loop = 100U;
	while ((bool)1) {
		unsigned int ext_flag = 0;

		/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
		/* coverity[cert_dcl37_c_violation : FALSE] */
		mdelay(1);
		loop--;
		value =
			((__raw_readl(reg) & CFG_PATH_STS_MASK)
			 >> CFG_PATH_STS_SHIFT);
		if (value == VIOC_PATH_DISCONNECTED) {
			__raw_writel(0x00000000, reg);
			ret = (int)VIOC_PATH_DISCONNECTED;
			ext_flag = 1U;
		}
		if (loop < 1U) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("[ERR][VIOC_CONFIG] %s, path configuration error(TIMEOUT). device is busy. VIOC(T:%u I:%u)\n",
			       __func__, get_vioc_type(component),
			       get_vioc_index(component));
			ret = VIOC_DEVICE_BUSY;
			ext_flag = 1U;
		}
		if (ext_flag == 1U) {
			break;
		}
	}

FUNC_EXIT:
	if ((ret != (int)VIOC_PATH_DISCONNECTED) && (reg != NULL)) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s, in error, Type :0x%x cfg_reg:0x%x\n",
		       __func__, component, __raw_readl(reg));
	}
	return (int)VIOC_DEVICE_INVALID;
}

/* support bypass mode DMA */
static const unsigned int bypassDMA[] = {
	/* RDMA */
	VIOC_RDMA00,
	VIOC_RDMA03,
	VIOC_RDMA04,
	VIOC_RDMA07,
	VIOC_RDMA08,
	VIOC_RDMA10,

	0x00 // just for final recognition
};

int VIOC_CONFIG_WMIXPath(unsigned int component_num, unsigned int mode)
{
	/* mode - 0: BY-PSSS PATH, 1: WMIX PATH */
	u32 value;
	unsigned int i, shift_mix_path, support_bypass = 0;
	void __iomem *config_reg = pIREQ_reg;
	int ret = -1;

	for (i = 0; i < (sizeof(bypassDMA) / sizeof(unsigned int)); i++) {
		if (component_num == bypassDMA[i]) {
			support_bypass = 1;
			break;
		}
	}

	/* Set variable to ignore value */
	shift_mix_path = 0xFFU;

	if (support_bypass == 1U) {
		switch (get_vioc_type(component_num)) {
		case get_vioc_type(VIOC_RDMA):
			switch (get_vioc_index(component_num)) {
			case get_vioc_index(VIOC_RDMA00):
				shift_mix_path = CFG_MISC0_MIX00_SHIFT;
				break;
			case get_vioc_index(VIOC_RDMA03):
				shift_mix_path = CFG_MISC0_MIX03_SHIFT;
				break;
			case get_vioc_index(VIOC_RDMA04):
				shift_mix_path = CFG_MISC0_MIX10_SHIFT;
				break;
			case get_vioc_index(VIOC_RDMA07):
				shift_mix_path = CFG_MISC0_MIX13_SHIFT;
				break;
			case get_vioc_index(VIOC_RDMA08):
				shift_mix_path = CFG_MISC0_MIX20_SHIFT;
				break;
			case get_vioc_index(VIOC_RDMA10):
				shift_mix_path = CFG_MISC0_MIX30_SHIFT;
				break;
			default:
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				pr_err("[ERR][CONFIG] %s component index(%u) is wrong\n",
					__func__, get_vioc_index(component_num));
				break;
			}
			break;
		default:
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("[ERR][CONFIG] %s component type(%u) is wrong\n",
					__func__, get_vioc_type(component_num));
			break;
		}

		if (shift_mix_path != 0xFFU) {
			VIOC_CONFIG_WMIXPathReset(component_num, 1);
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			value = __raw_readl(config_reg + CFG_MISC0_OFFSET)
				& ~((u32)1U << shift_mix_path);
			if (mode != 0U) {
				/* Prevent KCS warning */
				value |= (u32)1U << shift_mix_path;
			}
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			__raw_writel(value, config_reg + CFG_MISC0_OFFSET);
			VIOC_CONFIG_WMIXPathReset(component_num, 0);
		}

		ret = 0;
	} else {

		// pr_err("%s-%d :: ERROR This component(0x%x)
		// doesn't support mixer bypass(%d) mode, %d/%d!!\n",
		//		__func__, __LINE__, component_num, support_bypass,
		//		shift_mix_path, shift_vin_rdma_path);
		ret = -1;
	}

	return ret;
}

void VIOC_CONFIG_WMIXPathReset(unsigned int component_num, unsigned int resetmode)
{
	/* reset - 0: Normal, 1: Mixing PATH reset */
	u32 value;
	unsigned int i, shift_mix_path, support_bypass = 0U;
	//unsigned int shift_vin_rdma_path;
	void __iomem *config_reg = pIREQ_reg;

	for (i = 0; i < (sizeof(bypassDMA) / sizeof(unsigned int)); i++) {
		if (component_num == bypassDMA[i]) {
			support_bypass = 1U;
			break;
		}
	}

	if (support_bypass == 1U) {
		shift_mix_path = 0xFFU; // ignore value

		switch (get_vioc_type(component_num)) {
		case get_vioc_type(VIOC_RDMA):
			switch (get_vioc_index(component_num)) {
			case get_vioc_index(VIOC_RDMA00):
				shift_mix_path = WMIX_PATH_SWR_MIX00_SHIFT;
				break;
			case get_vioc_index(VIOC_RDMA03):
				shift_mix_path = WMIX_PATH_SWR_MIX03_SHIFT;
				break;
			case get_vioc_index(VIOC_RDMA04):
				shift_mix_path = WMIX_PATH_SWR_MIX10_SHIFT;
				break;
			case get_vioc_index(VIOC_RDMA07):
				shift_mix_path = WMIX_PATH_SWR_MIX13_SHIFT;
				break;
			case get_vioc_index(VIOC_RDMA08):
				shift_mix_path = WMIX_PATH_SWR_MIX20_SHIFT;
				break;
			case get_vioc_index(VIOC_RDMA10):
				shift_mix_path = WMIX_PATH_SWR_MIX30_SHIFT;
				break;
			default:
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				pr_err("[ERR][CONFIG] %s component index(%u) is wrong\n",
					__func__, get_vioc_index(component_num));
				break;
			}
			break;
		default:
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("[ERR][CONFIG] %s component type(%u) is wrong\n",
					__func__, get_vioc_type(component_num));
			break;
		}

		if (shift_mix_path != 0xFFU) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			value = __raw_readl(
					config_reg + CFG_WMIX_PATH_SWR_OFFSET)
					& ~((u32)1U << shift_mix_path);
			if (resetmode != 0U) {
				/* Prevent KCS warning */
				value |= (u32)1U << shift_mix_path;
			}
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			__raw_writel(
				value, config_reg + CFG_WMIX_PATH_SWR_OFFSET);
		}
	}
}

#if defined(CONFIG_VIOC_MAP_DECOMP)
/*
 *	VIOC_CONFIG_MCPath
 *	Connect Mapconverter or RDMA block on component path
 *	component : VIOC_RDMA03, VIOC_RDMA07, VIOC_RDMA11,
 *	VIOC_RDMA13, VIOC_RDMA15,
 *	VIOC_RDMA16, VIOC_RDMA17
 *	mc : VIOC_MC0, VIOC_MC1
 */
int VIOC_CONFIG_MCPath(unsigned int component, unsigned int mc)
{
	int ret = 0;
	u32 value;
	void __iomem *reg = (pIREQ_reg + CFG_PATH_MC_OFFSET);

	if (CheckMCPathSelection(component, mc) < 0) {
		ret = -1;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	switch (component) {
	case VIOC_RDMA03:
		if (get_vioc_type(mc) == get_vioc_type(VIOC_MC)) {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD03_MASK) != 0U) {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD03_MASK));
				value |= (((u32)0x1U << CFG_PATH_MC_RD03_SHIFT)
						| ((u32)0x0U << CFG_PATH_MC_MC0_SEL_SHIFT));
				__raw_writel(value, reg);
			}
		} else {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD03_MASK) == 0U) {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD03_MASK));
				__raw_writel(value, reg);
			}
		}
		break;
	case VIOC_RDMA07:
		if (get_vioc_type(mc) == get_vioc_type(VIOC_MC)) {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD07_MASK) != 0U) {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD07_MASK));
				value |= (((u32)0x1U << CFG_PATH_MC_RD07_SHIFT)
						| ((u32)0x1U << CFG_PATH_MC_MC0_SEL_SHIFT));
				__raw_writel(value, reg);
			}
		} else {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD07_MASK) == 0U)  {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD07_MASK));
				__raw_writel(value, reg);
			}
		}
		break;
	case VIOC_RDMA09:
		if (get_vioc_type(mc) == get_vioc_type(VIOC_MC)) {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD09_MASK) != 0U) {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD09_MASK));
				value |=
					(((u32)0x1U << CFG_PATH_MC_RD09_SHIFT)
					 | ((u32)0x2U << CFG_PATH_MC_MC0_SEL_SHIFT));
				__raw_writel(value, reg);
			}
		} else {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD09_MASK) == 0U) {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD09_MASK));
				__raw_writel(value, reg);
			}
		}
		break;
	case VIOC_RDMA10:
		if (get_vioc_type(mc) == get_vioc_type(VIOC_MC)) {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD10_MASK) != 0U)  {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD10_MASK));
				value |=
					(((u32)0x1U << CFG_PATH_MC_RD10_SHIFT)
					 | ((u32)0x3U << CFG_PATH_MC_MC0_SEL_SHIFT));
				__raw_writel(value, reg);
			}
		} else {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD10_MASK) == 0U) {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD10_MASK));
				__raw_writel(value, reg);
			}
		}
		break;
	case VIOC_RDMA11:
		if (get_vioc_type(mc) == get_vioc_type(VIOC_MC)) {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD11_MASK) != 0U)  {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD11_MASK));
				value |=
					(((u32)0x1U << CFG_PATH_MC_RD11_SHIFT)
					 | ((u32)0x4U << CFG_PATH_MC_MC0_SEL_SHIFT));
				__raw_writel(value, reg);
			}
		} else {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD11_MASK) == 0U) {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD11_MASK));
				__raw_writel(value, reg);
			}
		}
		break;
	case VIOC_RDMA12:
		if (get_vioc_type(mc) == get_vioc_type(VIOC_MC)) {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD12_MASK) != 0U)  {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD12_MASK));
				value |=
					(((u32)0x1U << CFG_PATH_MC_RD12_SHIFT)
					 | ((u32)0x5U << CFG_PATH_MC_MC0_SEL_SHIFT));
				__raw_writel(value, reg);
			}
		} else {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD12_MASK) == 0U) {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD12_MASK));
				__raw_writel(value, reg);
			}
		}
		break;
	case VIOC_RDMA13:
		if (get_vioc_type(mc) == get_vioc_type(VIOC_MC)) {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD13_MASK) != 0U)  {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD13_MASK));
				value |=
					(((u32)0x1U << CFG_PATH_MC_RD13_SHIFT)
					 | ((u32)0x6U << CFG_PATH_MC_MC0_SEL_SHIFT));
				__raw_writel(value, reg);
			}
		} else {
			if ((__raw_readl(reg) & CFG_PATH_MC_RD13_MASK) == 0U) {
				/* Prevent KCS warning */
			} else {
				value = (__raw_readl(reg) & ~(CFG_PATH_MC_RD13_MASK));
				__raw_writel(value, reg);
			}

		}
		break;
	default:
		ret = -1;
		break;
	}

FUNC_EXIT:
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s, in error, Path: 0x%x MC: %x cfg_reg:0x%x\n",
		       __func__, component, mc, __raw_readl(reg));
	}
	return ret;
}
#endif

void VIOC_CONFIG_SWReset(unsigned int component, unsigned int mode)
{
	u32 value;
	void __iomem *reg = pIREQ_reg;

	if ((mode != (unsigned int)VIOC_CONFIG_RESET) && (mode != (unsigned int)VIOC_CONFIG_CLEAR)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s, in error, invalid mode:%u\n",
		       __func__, mode);
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	switch (get_vioc_type(component)) {
	case get_vioc_type(VIOC_DISP):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + PWR_BLK_SWR1_OFFSET)
			 & ~(PWR_BLK_SWR1_DEV_MASK));
		value |=
			(mode
			 << (PWR_BLK_SWR1_DEV_SHIFT
			     + get_vioc_index(component)));
		__raw_writel(value, (reg + PWR_BLK_SWR1_OFFSET));
		break;

	case get_vioc_type(VIOC_WMIX):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + PWR_BLK_SWR1_OFFSET)
			 & ~(PWR_BLK_SWR1_WMIX_MASK));
		value |=
			(mode
			 << (PWR_BLK_SWR1_WMIX_SHIFT
			     + get_vioc_index(component)));
		__raw_writel(value, (reg + PWR_BLK_SWR1_OFFSET));
		break;

	case get_vioc_type(VIOC_WDMA):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + PWR_BLK_SWR1_OFFSET)
			 & ~(PWR_BLK_SWR1_WDMA_MASK));
		value |=
			(mode
			 << (PWR_BLK_SWR1_WDMA_SHIFT
			     + get_vioc_index(component)));
		__raw_writel(value, (reg + PWR_BLK_SWR1_OFFSET));
		break;

	case get_vioc_type(VIOC_FIFO):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + PWR_BLK_SWR1_OFFSET)
			 & ~(PWR_BLK_SWR1_FIFO_MASK));
		value |=
			(mode
			 << (PWR_BLK_SWR1_FIFO_SHIFT
			     + get_vioc_index(component)));
		__raw_writel(value, (reg + PWR_BLK_SWR1_OFFSET));
		break;

	case get_vioc_type(VIOC_RDMA):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + PWR_BLK_SWR0_OFFSET)
			 & ~(PWR_BLK_SWR0_RDMA_MASK));
		value |=
			(mode
			 << (PWR_BLK_SWR0_RDMA_SHIFT
			     + get_vioc_index(component)));
		__raw_writel(value, (reg + PWR_BLK_SWR0_OFFSET));
		break;

	case get_vioc_type(VIOC_SCALER):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + PWR_BLK_SWR0_OFFSET)
			 & ~(PWR_BLK_SWR0_SC_MASK));
		value |=
			(mode
			 << (PWR_BLK_SWR0_SC_SHIFT
			     + get_vioc_index(component)));
		__raw_writel(value, (reg + PWR_BLK_SWR0_OFFSET));
		break;

	case get_vioc_type(VIOC_VIQE):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + PWR_BLK_SWR1_OFFSET)
			 & ~(PWR_BLK_SWR1_VIQE0_MASK));
		value |= (mode << PWR_BLK_SWR1_VIQE0_SHIFT);
		__raw_writel(value, (reg + PWR_BLK_SWR1_OFFSET));
		break;
#if defined(CONFIG_VIOC_MAP_DECOMP)
	case get_vioc_type(VIOC_MC):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + PWR_BLK_SWR1_OFFSET)
			 & ~(PWR_BLK_SWR1_MC_MASK));
		value |=
			(mode
			 << (PWR_BLK_SWR1_MC_SHIFT
			     + get_vioc_index(component)));
		__raw_writel(value, (reg + PWR_BLK_SWR1_OFFSET));
		break;
#endif
#if defined(CONFIG_VIOC_AFBCDEC)
	case get_vioc_type(VIOC_AFBCDEC):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + CFG_WMIX_PATH_SWR_OFFSET)
			 & ~(WMIX_PATH_SWR_AD_MASK));
		value |=
			(resetmode
			 << (WMIX_PATH_SWR_AD_SHITF
			     + get_vioc_index(component)));
		__raw_writel(value, (reg + CFG_WMIX_PATH_SWR_OFFSET));
		break;
#endif
	default:
		if (component == VIOC_NO_COMPONENT) {
			pr_warn("%s: vioc_id(0x%x)\n", __func__, component);
		} else {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("[ERR][VIOC_CONFIG] %s, wrong component(0x%08x)\n",
					__func__, component);
		}
		break;
	}
FUNC_EXIT:
	return;
}

/*
 *	VIOC_CONFIG_Device_PlugState
 *	Check PlugInOut status of VIOC SCALER, VIQE, DEINTLS.
 *	component : VIOC_SC0, VIOC_SC1, VIOC_SC2, VIOC_VIQE, VIOC_DEINTLS
 *	pDstatus : Pointer of status value.
 *	return value : Device name of Plug in.
 */
int VIOC_CONFIG_Device_PlugState(
	unsigned int component, struct VIOC_PlugInOutCheck *VIOC_PlugIn)
{
	//	u32 value;
	void __iomem *reg = CalcAddressViocComponent(component);
	int ret = -1;
	void *tmp_pCONFIG = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pCONFIG = reg;
	reg = tmp_pCONFIG;

	if (reg == NULL) {
		/* prevent KCS warning */
		ret = VIOC_DEVICE_INVALID;
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		VIOC_PlugIn->enable =
			(__raw_readl(reg) & CFG_PATH_EN_MASK) >> CFG_PATH_EN_SHIFT;
		VIOC_PlugIn->connect_device =
			(__raw_readl(reg) & CFG_PATH_SEL_MASK) >> CFG_PATH_SEL_SHIFT;
		VIOC_PlugIn->connect_statue =
			(__raw_readl(reg) & CFG_PATH_STS_MASK) >> CFG_PATH_STS_SHIFT;
		ret = VIOC_DEVICE_CONNECTED;
	}
	return ret;
}

static unsigned int CalcPathSelectionInScaler(unsigned int RdmaNum)
{
	unsigned int ret;

	ret = get_vioc_index(RdmaNum);

	return ret;
}

static unsigned int CalcPathSelectionInViqeDeinter(unsigned int RdmaNum)
{
	unsigned int ret;

	ret = get_vioc_index(RdmaNum);

	return ret;
}

int VIOC_CONFIG_GetScaler_PluginToRDMA(unsigned int RdmaNum)
{
	unsigned int i;
	unsigned int rdma_idx;
	struct VIOC_PlugInOutCheck VIOC_PlugIn;
	int ret = -1;

	rdma_idx = CalcPathSelectionInScaler(RdmaNum);

	for (i = get_vioc_index(VIOC_SCALER0); i <= VIOC_SCALER_MAX; i++) {
		if (VIOC_CONFIG_Device_PlugState(
			    (VIOC_SCALER0 + i), &VIOC_PlugIn)
		    == VIOC_DEVICE_INVALID) {
			/* prevent KCS warning */
			continue;
		}

		if ((VIOC_PlugIn.enable == 1U)
		    && (VIOC_PlugIn.connect_device == rdma_idx))  {
			/* prevent KCS warning */
			ret = ((int)VIOC_SCALER0 + (int)i);
			break;
		}
	}
	if (i > VIOC_SCALER_MAX) {
		ret = -1;
	}

	return ret;
}

int VIOC_CONFIG_GetScaler_PluginToWDMA(unsigned int WdmaNum)
{
	unsigned int i;
	struct VIOC_PlugInOutCheck VIOC_PlugIn;
	int ret = -1;

	for (i = get_vioc_index(VIOC_SCALER0); i < VIOC_SCALER_MAX; i++) {
		if (VIOC_CONFIG_Device_PlugState(
				(VIOC_SCALER0 + i), &VIOC_PlugIn)
				== VIOC_DEVICE_INVALID) {
				/* prevent KCS warning */
			continue;
		}

		if ((VIOC_PlugIn.enable == 0U) || (VIOC_PlugIn.connect_device < 0xEU)) {
			//disabled or connected device is not WDMA
			continue;
		}

		if ((VIOC_PlugIn.connect_device - 0xEU) == get_vioc_index(WdmaNum)) {
			ret = ((int)VIOC_SCALER0 + (int)i);
		}

		if (ret != -1) {
			break;
		}
	}
	return ret;
}

int VIOC_CONFIG_GetViqeDeintls_PluginToRDMA(unsigned int RdmaNum)
{
	unsigned int i;
	unsigned int rdma_idx;
	struct VIOC_PlugInOutCheck VIOC_PlugIn;
	int ret = -1;

	rdma_idx = CalcPathSelectionInViqeDeinter(RdmaNum);

	for (i = get_vioc_index(VIOC_VIQE0); i < VIOC_VIQE_MAX; i++) {
		if (VIOC_CONFIG_Device_PlugState((VIOC_VIQE0 + i), &VIOC_PlugIn)
		    == VIOC_DEVICE_INVALID) {
			/* prevent KCS warning */
			continue;
		}

		if ((VIOC_PlugIn.enable == 1U)
		    && (VIOC_PlugIn.connect_device == rdma_idx)) {
			ret = ((int)VIOC_VIQE0 + (int)i);
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
	}

	ret = -1;

FUNC_EXIT:
	return ret;
}

int VIOC_CONFIG_GetRdma_PluginToComponent(
	unsigned int ComponentNum /*Viqe, Mc, Dtrc*/)
{
	struct VIOC_PlugInOutCheck VIOC_PlugIn;
	int ret = -1;

	if (VIOC_CONFIG_Device_PlugState(ComponentNum, &VIOC_PlugIn)
	    == VIOC_DEVICE_INVALID) {
	    /* Prevent KCS warning */
		ret = -1;
	} else {
		if (VIOC_PlugIn.enable == 1U) {
			if (VIOC_PlugIn.connect_device < (UINT_MAX / 2U)) {
				/* avoid CERT-C Integers Rule INT31-C */
				ret = ((int)VIOC_RDMA + (int)VIOC_PlugIn.connect_device);
			}
		}
	}

	return ret;
}

void VIOC_CONFIG_StopRequest(unsigned int en)
{
	u32 value;
	void __iomem *reg = pIREQ_reg;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = (__raw_readl(reg + CFG_MISC1_OFFSET) & ~(CFG_MISC1_S_REQ_MASK));

	if (en != 0U) {
		/* prevent KCS warning */
		value |= ((u32)0x0U << CFG_MISC1_S_REQ_SHIFT);
	} else {
		/* prevent KCS warning */
		value |= ((u32)0x1U << CFG_MISC1_S_REQ_SHIFT);
	}

	__raw_writel(value, reg + CFG_MISC1_OFFSET);
}

int VIOC_CONFIG_DMAPath_Select(unsigned int path)
{
	unsigned int i;
	unsigned long value;
	void __iomem *reg = CalcAddressViocComponent(path);
	int ret = -1;
	void *tmp_pCONFIG = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pCONFIG = reg;
	reg = tmp_pCONFIG;

	if (reg == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_info("%s-%d :: INFO :: path(0x%x) is not configurable\n",
			__func__, __LINE__, path);
		if (path < (UINT_MAX / 2U)) { /* avoid CERT-C Integers Rule INT31-C */
			ret = (int)path;
		} else {
			ret = -1;
		}
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	// check vrdma path
	for (i = 0U; i < VIOC_RDMA_MAX; i++) {
		reg = (void __iomem *)CalcAddressViocComponent(VIOC_RDMA00 + i);

		if (reg == NULL) {
			/* Prevent KCS warning */
			continue;
		}

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = __raw_readl(reg);
		if (((value & CFG_PATH_EN_MASK) != 0U)
		    && ((value & CFG_PATH_SEL_MASK) == (get_vioc_index(path)))) {
			/* Prevent KCS warning */
			ret = ((int)VIOC_RDMA00 + (int)i);
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
	}

#if defined(CONFIG_VIOC_MAP_DECOMP)
	// check map converter
	for (i = 0U; i < VIOC_MC_MAX; i++) {
		reg = (void __iomem *)CalcAddressViocComponent(VIOC_MC0 + i);
		if (reg == NULL) {
			/* Prevent KCS warning */
			continue;
		}

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = __raw_readl(reg);

		if (((value & CFG_PATH_EN_MASK) != 0U)
		    && ((value & CFG_PATH_SEL_MASK) == (get_vioc_index(path)))) {
			ret = ((int)VIOC_MC0 + (int)i);
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
	}
#endif
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_err("%s-%d :: Info path(0x%x) doesn't have plugged-in component!!\n",
	       __func__, __LINE__, path);
	ret = -1;

FUNC_EXIT:
	return ret;
}

int VIOC_CONFIG_DMAPath_Set(unsigned int path, unsigned int dma)
{
	int loop = 0;
	u32 value = 0x0U;
	unsigned int path_sel = 0;
	unsigned int en, sel, status;
	void __iomem *cfg_path_reg;
	int ret = -1;

	cfg_path_reg = CalcAddressViocComponent(dma);
	if (cfg_path_reg == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s-%d cfg_path_reg for dma(0x%x) is NULL\n",
		       __func__, __LINE__, dma);
		ret = -1;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	switch (get_vioc_type(dma)) {
	case get_vioc_type(VIOC_RDMA):
		ret = 0;
		break;
#if defined(CONFIG_VIOC_MAP_DECOMP)
	case get_vioc_type(VIOC_MC): {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(pIREQ_reg + PWR_AUTOPD_OFFSET)
			 & ~(PWR_AUTOPD_MC_MASK));
		value |= ((u32)0x0U << PWR_AUTOPD_MC_SHIFT);
		__raw_writel(value, pIREQ_reg + PWR_AUTOPD_OFFSET);
		ret = 0;
	} break;
#endif
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s-%d cfg_path_reg for dma(0x%x) doesn't support\n",
		       __func__, __LINE__, dma);
		ret = -1;
		break;
	}
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	// check path status.
	value = __raw_readl(cfg_path_reg);

	en = ((value & CFG_PATH_EN_MASK) != 0U) ? 1U : 0U;
	status = (value & CFG_PATH_STS_MASK) >> CFG_PATH_STS_SHIFT;
	sel = (value & CFG_PATH_SEL_MASK) >> CFG_PATH_SEL_SHIFT;

	// path selection.
	if (get_vioc_type(path) == get_vioc_type(VIOC_RDMA)) {
		path_sel = get_vioc_index(path);
		ret = 0;
	} else {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s-%d path(0x%x) is very wierd.\n",
		       __func__, __LINE__, path);
		ret = -1;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	if ((en == 1U) && (sel != path_sel)) {
		loop = 50;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value & (~CFG_PATH_EN_MASK), cfg_path_reg);
		while (loop-- != 0) {
			value = __raw_readl(cfg_path_reg);
			status = (value & CFG_PATH_STS_MASK)
				>> CFG_PATH_STS_SHIFT;
			if (status == VIOC_PATH_DISCONNECTED) {
				ret = 0;
				break;
			}
			mdelay(1);
		}

		if (loop <= 0) {
			ret = -1;
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
	}

	loop = 50;
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_info("[INF][VIOC_CONFIG] W:: %s CFG_RDMA:0x%p = 0x%x\n", __func__,
		cfg_path_reg,
		(CFG_PATH_EN_MASK
		 | ((path_sel << CFG_PATH_SEL_SHIFT) & CFG_PATH_SEL_MASK)));

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(CFG_PATH_EN_MASK
		       | ((path_sel << CFG_PATH_SEL_SHIFT) & CFG_PATH_SEL_MASK),
	       cfg_path_reg);
	while (loop-- != 0) {
		value = __raw_readl(cfg_path_reg);
		status = (value & CFG_PATH_STS_MASK) >> CFG_PATH_STS_SHIFT;
		if (status != VIOC_PATH_DISCONNECTED) {
			ret = 0;
			break;
		}
		/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
		/* coverity[cert_dcl37_c_violation : FALSE] */
		mdelay(1);
	}

	if (loop <= 0) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		ret = -1;
		goto FUNC_EXIT;
	}


	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_info("[INF][VIOC_CONFIG] R:: %s CFG_RDMA:0x%p = 0x%x\n", __func__,
		cfg_path_reg, __raw_readl(cfg_path_reg));

	ret = 0;

FUNC_EXIT:
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[cert_dcl37_c_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] vioc config plug in error : setting path : 0x%x dma:%x before registe : 0x%08x\n",
			   path, dma, value);
	}
	return ret;
}

int VIOC_CONFIG_DMAPath_UnSet(int dma)
{
	unsigned int en = 0;
	int loop = 0;
	u32 value = 0;
	void __iomem *cfg_path_reg = NULL;
	int ret = -1;

	if (dma < 0) {
		ret = -1;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	// select config register.
	cfg_path_reg = CalcAddressViocComponent((unsigned int)dma);
	if (cfg_path_reg == NULL) {
		// pr_err("[ERR][VIOC_CONFIG] %s-%d cfg_path_reg for
		// dma(0x%x) is NULL\n",
		//__func__, __LINE__, dma);
		ret = -1;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	switch (get_vioc_type((unsigned int)dma)) {
	case get_vioc_type(VIOC_RDMA):
		ret = 0;
		break;
#if defined(CONFIG_VIOC_MAP_DECOMP)
	case get_vioc_type(VIOC_MC):
		ret = 0;
		break;
#endif
	default:
		// pr_err("[ERR][VIOC_CONFIG] %s-%d cfg_path_reg for
		// dma(0x%x) doesn't support\n",
		//__func__, __LINE__, dma);
		ret = -1;
		break;
	}
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	// check path status.
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = __raw_readl(cfg_path_reg);
	en = (((value & CFG_PATH_EN_MASK) != 0U) ? 1U : 0U);

	if (en == 1U) {
		// disable dma
		loop = 50;
		__raw_writel(value & (~CFG_PATH_EN_MASK), cfg_path_reg);

		// wait dma disconnected status.
		while (loop-- != 0)  {
			if (((__raw_readl(cfg_path_reg) & CFG_PATH_STS_MASK)
			     >> CFG_PATH_STS_SHIFT)
			    == VIOC_PATH_DISCONNECTED) {
			    /* Prevent KCS warning */
				ret = 0;
				break;
			}
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[cert_dcl37_c_violation : FALSE] */
			mdelay(1);
		}

		if (loop <= 0) {
			ret = -1;
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
	}

	ret = 0;

FUNC_EXIT:
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[cert_dcl37_c_violation : FALSE] */
		pr_err("[ERR][VIOC_CONFIG] %s  in error : setting  dma:%d before cfg_path_registe : 0x%08x\n",
			   __func__, dma, value);
	}
	return ret;
}

int VIOC_CONFIG_DMAPath_Support(void)
{
	int ret = -1;

	ret = 0;

	return ret;
}

void VIOC_CONFIG_DMAPath_Iint(void)
{
	unsigned int i;
	const void __iomem *cfg_path_reg;

	if (VIOC_CONFIG_DMAPath_Support() != 0) {
		for (i = 0; i < VIOC_RDMA_MAX; i++) {
			cfg_path_reg = (void __iomem *)CalcAddressViocComponent(
				VIOC_RDMA00 + i);

			if (cfg_path_reg == NULL) {
				/* Prevent KCS warning */
				continue;
			}
			(void)VIOC_CONFIG_DMAPath_Set(
				VIOC_RDMA00 + i, VIOC_RDMA00 + i);
		}
	}
}

void __iomem *VIOC_IREQConfig_GetAddress(void)
{
	return (void __iomem *)pIREQ_reg;
}

int VIOC_CONFIG_MC_PlugIn(unsigned int nValue)
{
	void __iomem *reg = pIREQ_reg;
	u32 value;

	switch (nValue) {
	case ((unsigned int)MC_RDMA03):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + CFG_PATH_MC_OFFSET)
			 & ~(CFG_PATH_MC_RD03_MASK));
		value |= ((u32)0x1U << CFG_PATH_MC_RD03_SHIFT);
		__raw_writel(value, reg + CFG_PATH_MC_OFFSET);
		break;
	case ((unsigned int)MC_RDMA07):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + CFG_PATH_MC_OFFSET)
			 & ~(CFG_PATH_MC_RD07_MASK | CFG_PATH_MC_MC0_SEL_MASK));
		value |=
			(((u32)0x0U << CFG_PATH_MC_MC0_SEL_SHIFT)
			 | (((u32)0x1U) << CFG_PATH_MC_RD07_SHIFT));
		__raw_writel(value, reg + CFG_PATH_MC_OFFSET);
		break;
	case ((unsigned int)MC_RDMA11):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + CFG_PATH_MC_OFFSET)
			 & ~(CFG_PATH_MC_RD11_MASK | CFG_PATH_MC_MC0_SEL_MASK));
		value |=
			(((u32)0x1U << CFG_PATH_MC_MC0_SEL_SHIFT)
			 | ((u32)0x1U << CFG_PATH_MC_RD11_SHIFT));
		__raw_writel(value, reg + CFG_PATH_MC_OFFSET);
		break;
	case ((unsigned int)MC_RDMA13):
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value =
			(__raw_readl(reg + CFG_PATH_MC_OFFSET)
			 & ~(CFG_PATH_MC_RD13_MASK | CFG_PATH_MC_MC0_SEL_MASK));
		value |=
			(((u32)0x2U << CFG_PATH_MC_MC0_SEL_SHIFT)
			 | ((u32)0x1U << CFG_PATH_MC_RD13_SHIFT));
		__raw_writel(value, reg + CFG_PATH_MC_OFFSET);
		break;
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[INF][VIOC_CONFIG] %s MC path(%u) is wrong\n",
			__func__, nValue);
		break;
	}
	return 0;
}

#if 0
void VIOC_CONFIG_SetLoopBack(unsigned int vin, unsigned int path)
{
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	void __iomem *reg = (void __iomem *)pIREQ_reg + TEST_LOOP_OFFSET;
	u32 value;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value =
		(__raw_readl(reg)
		 & ~(TEST_LOOP_LVIN7_MASK | TEST_LOOP_LVIN6_MASK
		     | TEST_LOOP_LVIN5_MASK | TEST_LOOP_LVIN4_MASK
		     | TEST_LOOP_LVIN3_MASK | TEST_LOOP_LVIN2_MASK
		     | TEST_LOOP_LVIN1_MASK | TEST_LOOP_LVIN0_MASK));

	switch (vin) {
	case 7:
		value |= ((path & 0x7U) << TEST_LOOP_LVIN7_SHIFT);
		break;
	case 6:
		value |= ((path & 0x7U) << TEST_LOOP_LVIN6_SHIFT);
		break;
	case 5:
		value |= ((path & 0x7U) << TEST_LOOP_LVIN5_SHIFT);
		break;
	case 4:
		value |= ((path & 0x7U) << TEST_LOOP_LVIN4_SHIFT);
		break;
	case 3:
		value |= ((path & 0x7U) << TEST_LOOP_LVIN3_SHIFT);
		break;
	case 2:
		value |= ((path & 0x7U) << TEST_LOOP_LVIN2_SHIFT);
		break;
	case 1:
		value |= ((path & 0x7U) << TEST_LOOP_LVIN1_SHIFT);
		break;
	case 0:
		value |= ((path & 0x7U) << TEST_LOOP_LVIN0_SHIFT);
		break;
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[INF][VIOC_CONFIG] %s vin(%u) is wrong\n",
			__func__, vin);
		break;
	}
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(value, reg + TEST_LOOP_OFFSET);
}
#endif
