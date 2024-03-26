// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/reg_physical.h>
#include <asm/arch/vioc/lvds_wrap.h>

#ifndef REG_LVDS_WRAP
#define REG_LVDS_WRAP (0x1B940000) // 16 word
#endif

/* LVDS_WRAP_SetConfigure
 * Set Tx splitter configuration
 * lr : tx splitter output mode - 0: even/odd, 1: left/right
 * bypass : tx splitter bypass mode
 * width : tx splitter width - single port: real width, dual port: half width
 */
void LVDS_WRAP_SetConfigure(
	unsigned int lr, unsigned int bypass, unsigned int width)
{
	void __iomem *reg = (void __iomem *)REG_LVDS_WRAP;

	if (reg) {
		unsigned int val =
			(readl(reg + TS_CFG)
			 & ~(TS_CFG_WIDTH_MASK | TS_CFG_MODE_MASK
			     | TS_CFG_LR_MASK | TS_CFG_BP_MASK));
		val |= (((width & 0xFFF) << TS_CFG_WIDTH_SHIFT)
			| ((bypass & 0x1) << TS_CFG_BP_SHIFT)
			| ((lr & 0x1) << TS_CFG_LR_SHIFT));
		writel(val, reg + TS_CFG);
	}
}

void LVDS_WRAP_SetSyncPolarity(unsigned int sync)
{
	void __iomem *reg = (void __iomem *)REG_LVDS_WRAP;
	unsigned int val;

	val = (readl(reg + SAL_1) & ~(SAL_HS_MASK | SAL_VS_MASK));
	if (!(sync & 0x2))
		val |= (0x1 << SAL_VS_SHIFT);

	if (!(sync & 0x4))
		val |= (0x1 << SAL_HS_SHIFT);

	writel(val, reg + SAL_1);
}

/* LVDS_WRAP_SetDataSwap
 * Set Tx splitter output data swap
 * ch : Tx splitter output channel(0, 1, 2, 3)
 * set : Tx splitter data swap mode
 */
void LVDS_WRAP_SetDataSwap(unsigned int ch, unsigned int set)
{
	void __iomem *reg = (void __iomem *)REG_LVDS_WRAP;

	if (reg) {
		unsigned int val;

		switch (ch) {
		case 0:
			val = (readl(reg + TS_CFG) & ~(TS_CFG_SWAP0_MASK));
			val |= ((set & 0x3) << TS_CFG_SWAP0_SHIFT);
			writel(val, reg + TS_CFG);
			break;
		case 1:
			val = (readl(reg + TS_CFG) & ~(TS_CFG_SWAP1_MASK));
			val |= ((set & 0x3) << TS_CFG_SWAP1_SHIFT);
			writel(val, reg + TS_CFG);
			break;
		case 2:
			val = (readl(reg + TS_CFG) & ~(TS_CFG_SWAP2_MASK));
			val |= ((set & 0x3) << TS_CFG_SWAP2_SHIFT);
			writel(val, reg + TS_CFG);
			break;
		case 3:
			val = (readl(reg + TS_CFG) & ~(TS_CFG_SWAP3_MASK));
			val |= ((set & 0x3) << TS_CFG_SWAP3_SHIFT);
			writel(val, reg + TS_CFG);
			break;
		default:
			pr_err("%s: invalid parameter(%d, %d)\n", __func__, ch,
			       set);
			break;
		}
	}
}

/* LVDS_WRAP_SetMuxOutput
 * Set Tx splitter MUX output selection
 * mux: the type of mux (DISP_MUX_TYPE, TS_MUX_TYPE)
 * select : the select
 */
void LVDS_WRAP_SetMuxOutput(
	unsigned int mux, unsigned int ch,
	unsigned int select, unsigned int enable)
{
	void __iomem *reg = (void __iomem *)REG_LVDS_WRAP;
	unsigned int val;

	if (reg) {
		switch (mux) {
		case DISP_MUX_TYPE:
			val = (readl(reg + DISP_MUX_SEL)
			       & ~(DISP_MUX_SEL_SEL_MASK));
			val |= ((select & 0x3) << DISP_MUX_EN_EN_SHIFT);
			writel(val, reg + DISP_MUX_SEL);
			val = (readl(reg + DISP_MUX_EN)
			       & ~(DISP_MUX_EN_EN_MASK));
			val |= ((enable & 0x1) << DISP_MUX_EN_EN_SHIFT);
			writel(val, reg + DISP_MUX_EN);
			break;
		case TS_MUX_TYPE:
			switch (ch) {
			case 0:
				val = (readl(reg + TS_MUX_SEL0)
				       & ~(TS_MUX_SEL_SEL_MASK));
				val |= ((select & 0x7) << TS_MUX_SEL_SEL_SHIFT);
				writel(val, reg + TS_MUX_SEL0);
				val = (readl(reg + TS_MUX_EN0)
				       & ~(TS_MUX_EN_EN_MASK));
				val |= ((enable & 0x1) << TS_MUX_EN_EN_SHIFT);
				writel(val, reg + TS_MUX_EN0);
				break;
			case 1:
				val = (readl(reg + TS_MUX_SEL1)
				       & ~(TS_MUX_SEL_SEL_MASK));
				val |= ((select & 0x7) << TS_MUX_SEL_SEL_SHIFT);
				writel(val, reg + TS_MUX_SEL1);
				val = (readl(reg + TS_MUX_EN1)
				       & ~(TS_MUX_EN_EN_MASK));
				val |= ((enable & 0x1) << TS_MUX_EN_EN_SHIFT);
				writel(val, reg + TS_MUX_EN1);
				break;
			case 2:
				val = (readl(reg + TS_MUX_SEL2)
				       & ~(TS_MUX_SEL_SEL_MASK));
				val |= ((select & 0x7) << TS_MUX_SEL_SEL_SHIFT);
				writel(val, reg + TS_MUX_SEL2);
				val = (readl(reg + TS_MUX_EN2)
				       & ~(TS_MUX_EN_EN_MASK));
				val |= ((enable & 0x1) << TS_MUX_EN_EN_SHIFT);
				writel(val, reg + TS_MUX_EN2);
				break;
			case 3:
				val = (readl(reg + TS_MUX_SEL3)
				       & ~(TS_MUX_SEL_SEL_MASK));
				val |= ((select & 0x7) << TS_MUX_SEL_SEL_SHIFT);
				writel(val, reg + TS_MUX_SEL3);
				val = (readl(reg + TS_MUX_EN3)
				       & ~(TS_MUX_EN_EN_MASK));
				val |= ((enable & 0x1) << TS_MUX_EN_EN_SHIFT);
				writel(val, reg + TS_MUX_EN3);
				break;
			default:
				goto error_mux_output;
			}
			break;
		case MUX_TYPE_MAX:
		default:
			goto error_mux_output;
		}
	}
	return;
error_mux_output:
	pr_err("%s in error, invalid parameter(mux: %d, ch: %d)\n", __func__,
	       mux, ch);
}

/* LVDS_WRAP_SetDataPath
 * Set Data output format of tx splitter
 * ch : channel number of tx splitter mux
 * path : path number of tx splitter mux
 * set : data output format of tx splitter mux
 */
void LVDS_WRAP_SetDataPath(unsigned int ch, unsigned int path, unsigned int set)
{
	void __iomem *reg = (void __iomem *)REG_LVDS_WRAP;
	unsigned int offset;

	if ((path < 0) || (path >= TS_TXOUT_SEL_MAX))
		goto error_data_path;

	if (reg) {
		switch (ch) {
		case 0:
			offset = TXOUT_SEL0_0;
			break;
		case 1:
			offset = TXOUT_SEL0_1;
			break;
		case 2:
			offset = TXOUT_SEL0_2;
			break;
		case 3:
			offset = TXOUT_SEL0_3;
			break;
		default:
			goto error_data_path;
		}

		writel((set & 0xFFFFFFFF), reg + (offset + (0x4 * path)));
	}
	return;
error_data_path:
	pr_err("%s in error, invalid parameter(ch: %d, path: %d)\n", __func__,
	       ch, path);
}

/* LVDS_WRAP_SetDataArray
 * Set the data output format of tx splitter mux
 * ch : channel number of tx splitter mux
 * data : the array included the data output format
 */
void LVDS_WRAP_SetDataArray(
	unsigned int ch, unsigned int data[TXOUT_MAX_LINE][TXOUT_DATA_PER_LINE])
{
	void __iomem *reg = (void __iomem *)REG_LVDS_WRAP;
	unsigned int *lvdsdata = (unsigned int *)data;
	unsigned int idx, value, path;
	unsigned int data0, data1, data2, data3;

	if ((ch < 0) || (ch >= TS_MUX_IDX_MAX))
		goto error_data_array;

	if (reg) {
		for (idx = 0; idx < (TXOUT_MAX_LINE * TXOUT_DATA_PER_LINE);
		     idx += 4) {
			data0 = TXOUT_GET_DATA(idx);
			data1 = TXOUT_GET_DATA(idx + 1);
			data2 = TXOUT_GET_DATA(idx + 2);
			data3 = TXOUT_GET_DATA(idx + 3);

			path = idx / 4;
			value =
				((lvdsdata[data3] << 24)
				 | (lvdsdata[data2] << 16)
				 | (lvdsdata[data1] << 8) | (lvdsdata[data0]));
			LVDS_WRAP_SetDataPath(ch, path, value);
		}
	}
	return;
error_data_array:
	pr_err("%s in error, invalid parameter(ch: %d)\n", __func__, ch);
}

/* LVDS_WRAP_SetAccessCode
 * Set the access code of LVDS Wrapper
 */
void LVDS_WRAP_SetAccessCode(void)
{
	if (readl(0x1B9401EC) == 0x1ACCE551)
		return;

	/* Please delete this code,
	 * after making a decision about safety mechanism
	 */
	writel(0x1ACCE551, 0x1B9401EC);
}

/* LVDS_WRAP_REsetPHY
 * software reset for PHY port
 */
void LVDS_WRAP_ResetPHY(unsigned int port, unsigned int reset)
{
	void __iomem *reg =
		(void __iomem *)REG_LVDS_WRAP + TS_SWRESET;

	if (reset) {
		switch (port) {
		case TS_MUX_IDX0:
		case TS_MUX_IDX1:
			// LVDS_PHY_2PORT_SYS
			writel(readl(reg) | (0x1 << 4), reg);
			break;
		case TS_MUX_IDX2:
			// LVDS_PHY_1PORT_SYS
			writel(readl(reg) | (0x1 << 3), reg);
			break;
		}
	}
	writel(0x0, reg);
}

void LVDS_WRAP_SM_Bypass(unsigned int lcdc_mux_id, unsigned int en)
{
	void __iomem *reg =
		(void __iomem *)REG_LVDS_WRAP + SM_BYPASS;
	unsigned int val;

	if (en > 1) {
		pr_err("%s : invalid en value. en should be 0 or 1.\n",
		       __func__);
	}
	switch (lcdc_mux_id) {
	case 2:
		val = (readl(reg) & ~(0x1 << SHIFT_SDC)) | (en << SHIFT_SDC);
		writel(val, reg);
		break;
	case 3:
		val = (readl(reg) & ~(0x1 << SHIFT_SRC)) | (en << SHIFT_SRC);
		writel(val, reg);
		break;
	default:
		pr_err("%s : invalid lcdc mux id\n", __func__);
		return;
	}
	pr_info("%s : lcdc mux id = %d, SM_BYPASS = 0x%08x\n", __func__,
		lcdc_mux_id, readl(reg));
}
