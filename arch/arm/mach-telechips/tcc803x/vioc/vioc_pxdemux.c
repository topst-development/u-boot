// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/reg_physical.h>
#include <asm/arch/vioc/vioc_pxdemux.h>

#ifndef REG_VIOC_PXDEMUX
#define REG_VIOC_PXDEMUX	(HwVIOC_BASE + 0x390000U) // 16 word
#endif

/* VIOC_PXDEMUX_SetConfigure
 * Set pixel demuxer configuration
 * idx : pixel demuxer idx
 * lr : pixel demuxer output - mode 0: even/odd, 1: left/right
 * bypass : pixel demuxer bypass mode
 * width : pixel demuxer width - single port: real width, dual port: half width
 */
void VIOC_PXDEMUX_SetConfigure(
	unsigned int idx, unsigned int lr, unsigned int bypass,
	unsigned int width)
{
	void __iomem *reg = phys_to_virt(REG_VIOC_PXDEMUX);

	if (idx >= (unsigned int)PD_IDX_MAX) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("%s in error, invalid parameter(idx: %u)\n", __func__, idx);
	} else {
		unsigned int offset = (idx != 0U) ? PD1_CFG : PD0_CFG;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		unsigned int val = (__raw_readl(reg + offset) &
				    ~(PD_CFG_WIDTH_MASK | PD_CFG_MODE_MASK |
				      PD_CFG_LR_MASK | PD_CFG_BP_MASK));
		val |= (((width & 0xFFFU) << PD_CFG_WIDTH_SHIFT) |
			((bypass & 0x1U) << PD_CFG_BP_SHIFT) |
			((lr & 0x1U) << PD_CFG_LR_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(val, reg + offset);
	}
}

/* VIOC_PXDEMUX_SetDataSwap
 * Set pixel demuxter output data swap
 * idx: pixel demuxer idx
 * ch : pixel demuxer output channel(0, 1, 2, 3)
 * set : pixel demuxer data swap mode
 */
void VIOC_PXDEMUX_SetDataSwap(
	unsigned int idx, unsigned int ch, unsigned int set)
{
	void __iomem *reg = phys_to_virt(REG_VIOC_PXDEMUX);

	if ((idx >= (unsigned int)PD_IDX_MAX)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][P_DEMUX] %s: invalid parameter(idx: %u, ch: %u, set: 0x%08x)\n",
			__func__, idx, ch, set);
	} else {
		unsigned int offset, val;

		offset = (idx != 0U) ? PD1_CFG : PD0_CFG;
		switch (ch) {
		case 0:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + offset)
				& ~(PD_CFG_SWAP0_MASK));
			val |= ((set & 0x3U) << PD_CFG_SWAP0_SHIFT);
			__raw_writel(val, reg + offset);
			break;
		case 1:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + offset)
				& ~(PD_CFG_SWAP1_MASK));
			val |= ((set & 0x3U) << PD_CFG_SWAP1_SHIFT);
			__raw_writel(val, reg + offset);
			break;
		case 2:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + offset)
				& ~(PD_CFG_SWAP2_MASK));
			val |= ((set & 0x3U) << PD_CFG_SWAP2_SHIFT);
			__raw_writel(val, reg + offset);
			break;
		case 3:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + offset)
				& ~(PD_CFG_SWAP3_MASK));
			val |= ((set & 0x3U) << PD_CFG_SWAP3_SHIFT);
			__raw_writel(val, reg + offset);
			break;
		default:
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("[ERR][P_DEMUX] %s: invalid parameter(%u, %u)\n",
				__func__, ch, set);
			break;
		}
	}
}

/* VIOC_PXDEMUX_SetMuxOutput
 * Set MUX output selection
 * mux: the type of mux (PD_MUX3TO1_TYPE, PD_MUX5TO1_TYPE)
 * select : the selecti
 */
void VIOC_PXDEMUX_SetMuxOutput(enum PD_MUX_TYPE mux, unsigned int ch,
			       unsigned int select, unsigned int enable)
{
	void __iomem *reg = phys_to_virt(REG_VIOC_PXDEMUX);
	unsigned int val;
	int ret = -1;

	switch (mux) {
	case PD_MUX3TO1_TYPE:
		switch (ch) {
		case 0:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + MUX3_1_SEL0)
			       & ~(MUX3_1_SEL_SEL_MASK));
			val |= ((select & 0x3U) << MUX3_1_SEL_SEL_SHIFT);
			__raw_writel(val, reg + MUX3_1_SEL0);
			val = (__raw_readl(reg + MUX3_1_EN0)
			       & ~(MUX3_1_EN_EN_MASK));
			val |= ((enable & 0x1U) << MUX3_1_EN_EN_SHIFT);
			__raw_writel(val, reg + MUX3_1_EN0);
			ret = 0;
			break;
		case 1:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + MUX3_1_SEL1)
			       & ~(MUX3_1_SEL_SEL_MASK));
			val |= ((select & 0x3U) << MUX3_1_SEL_SEL_SHIFT);
			__raw_writel(val, reg + MUX3_1_SEL1);
			val = (__raw_readl(reg + MUX3_1_EN1)
			       & ~(MUX3_1_EN_EN_MASK));
			val |= ((enable & 0x1U) << MUX3_1_EN_EN_SHIFT);
			__raw_writel(val, reg + MUX3_1_EN1);
			ret = 0;
			break;
		default:
			ret = -1;
			break;
		}
		break;
	case PD_MUX5TO1_TYPE:
		switch (ch) {
		case 0:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + MUX5_1_SEL0)
			       & ~(MUX5_1_SEL_SEL_MASK));
			val |= ((select & 0x7U) << MUX5_1_SEL_SEL_SHIFT);
			__raw_writel(val, reg + MUX5_1_SEL0);
			val = (__raw_readl(reg + MUX5_1_EN0)
			       & ~(MUX5_1_EN_EN_MASK));
			val |= ((enable & 0x1U) << MUX5_1_EN_EN_SHIFT);
			__raw_writel(val, reg + MUX5_1_EN0);
			ret = 0;
			break;
		case 1:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + MUX5_1_SEL1)
			       & ~(MUX5_1_SEL_SEL_MASK));
			val |= ((select & 0x7U) << MUX5_1_SEL_SEL_SHIFT);
			__raw_writel(val, reg + MUX5_1_SEL1);
			val = (__raw_readl(reg + MUX5_1_EN1)
			       & ~(MUX5_1_EN_EN_MASK));
			val |= ((enable & 0x1U) << MUX5_1_EN_EN_SHIFT);
			__raw_writel(val, reg + MUX5_1_EN1);
			ret = 0;
			break;
		case 2:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + MUX5_1_SEL2)
			       & ~(MUX5_1_SEL_SEL_MASK));
			val |= ((select & 0x7U) << MUX5_1_SEL_SEL_SHIFT);
			__raw_writel(val, reg + MUX5_1_SEL2);
			val = (__raw_readl(reg + MUX5_1_EN2)
			       & ~(MUX5_1_EN_EN_MASK));
			val |= ((enable & 0x1U) << MUX5_1_EN_EN_SHIFT);
			__raw_writel(val, reg + MUX5_1_EN2);
			ret =0;
			break;
		case 3:
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			val = (__raw_readl(reg + MUX5_1_SEL3)
			       & ~(MUX5_1_SEL_SEL_MASK));
			val |= ((select & 0x7U) << MUX5_1_SEL_SEL_SHIFT);
			__raw_writel(val, reg + MUX5_1_SEL3);
			val = (__raw_readl(reg + MUX5_1_EN3)
			       & ~(MUX5_1_EN_EN_MASK));
			val |= ((enable & 0x1U) << MUX5_1_EN_EN_SHIFT);
			__raw_writel(val, reg + MUX5_1_EN3);
			ret = 0;
			break;
		default:
			ret = -1;
			break;
		}
		break;
	case PD_MUX_TYPE_MAX:
	default:
		ret = -1;
		break;
	}
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][P_DEMUX] %s: invalid parameter(mux: %u, ch: %u)\n",
	       __func__, (unsigned int)mux, ch);
	}
}

/* VIOC_PXDEMUX_SetDataPath
 * Set Data output format of pixel demuxer
 * ch : channel number of pixel demuxer 5x1
 * path : path number of pixel demuxer 5x1
 * set : data output format of pixel demuxer 5x1
 */
void VIOC_PXDEMUX_SetDataPath(
	unsigned int ch, unsigned int path, unsigned int set)
{
	void __iomem *reg = phys_to_virt(REG_VIOC_PXDEMUX);
	unsigned int offset;
	int ret = -1;

	if (path >= (unsigned int)PD_TXOUT_SEL_MAX) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][P_DEMUX] %s: invalid parameter(ch: %u, path: %u)\n",
			__func__, ch, path);
	} else {
		switch (ch) {
		case 0:
			offset = TXOUT_SEL0_0;
			 ret = 0;
			break;
		case 1:
			offset = TXOUT_SEL0_1;
			ret = 0;
			break;
		case 2:
			offset = TXOUT_SEL0_2;
			ret = 0;
			break;
		case 3:
			offset = TXOUT_SEL0_3;
			ret = 0;
			break;
		default:
			ret = -1;
			break;
		}
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("[ERR][P_DEMUX] %s: invalid parameter(ch: %u, path: %u)\n",
				__func__, ch, path);
		} else {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			__raw_writel((set & 0xFFFFFFFFU),
				reg + (offset + (0x4U * path)));
		}
	}
}

/* VIOC_PXDEMUX_SetDataArray
 * Set the data output format of pixel demuxer 5x1
 * ch : channel number of pixel demuxer 5x1
 * data : the array included the data output format
 */
void VIOC_PXDEMUX_SetDataArray(
	unsigned int ch, unsigned int data[TXOUT_MAX_LINE][TXOUT_DATA_PER_LINE])
{
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	/* coverity[cert_int36_c_violation : FALSE] */
	//const void __iomem *reg = (void __iomem *)REG_VIOC_PXDEMUX;
	const unsigned int *lvdsdata = (unsigned int *)data;
	unsigned int idx, value, path;
	unsigned int data_0, data_1, data_2, data_3;

	if (ch >= (unsigned int)PD_MUX5TO1_IDX_MAX) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("%s in error, invalid parameter(ch: %u)\n", __func__, ch);
	} else {
		for (idx = 0; idx < (TXOUT_MAX_LINE * TXOUT_DATA_PER_LINE);
			idx += 4U){
			data_0 = TXOUT_GET_DATA(idx);
			data_1 = TXOUT_GET_DATA(idx + 1U);
			data_2 = TXOUT_GET_DATA(idx + 2U);
			data_3 = TXOUT_GET_DATA(idx + 3U);

			path = idx / 4U;
			value =
				((lvdsdata[data_3] << 24U)
				| (lvdsdata[data_2] << 16U)
				| (lvdsdata[data_1] << 8U) | (lvdsdata[data_0]));
			VIOC_PXDEMUX_SetDataPath(ch, path, value);
		}
	}
}
