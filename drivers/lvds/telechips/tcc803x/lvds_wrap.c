// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/reg_physical.h>
#include "lvds_wrap.h"

#define debug_pr(level, fmt...) (CONFIG_LOGLEVEL > (signed)level) ? (void)printf(fmt) : 0;

#ifndef REG_LVDS_WRAP
#define REG_LVDS_WRAP (0x1B940000u) // 16 word
#endif

static void LVDS_wrap_writel(
	uint32_t lvds_reg_value, uint32_t lvds_reg_address)
{
	writel(lvds_reg_value, (uintptr_t)lvds_reg_address);
}

static uint32_t LVDS_wrap_readl(uint32_t lvds_reg_address)
{
	return readl((uintptr_t)lvds_reg_address);
}

/* LVDS_WRAP_SetConfigure
 * Set Tx splitter configuration
 * lr : tx splitter output mode - 0: even/odd, 1: left/right
 * bypass : tx splitter bypass mode
 * width : tx splitter width - single port: real width, dual port: half width
 */
void LVDS_WRAP_SetConfigure(
	uint32_t lr, uint32_t bypass, uint32_t width)
{
	uint32_t val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_CFG) &
		~(TS_CFG_WIDTH_MASK | TS_CFG_MODE_MASK |
		TS_CFG_LR_MASK | TS_CFG_BP_MASK));

	val |= (((width & 0xFFFu) << TS_CFG_WIDTH_SHIFT) |
		((bypass & 0x1u) << TS_CFG_BP_SHIFT) |
		((lr & 0x1u) << TS_CFG_LR_SHIFT));
	LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_CFG);

}

void LVDS_WRAP_SetSyncPolarity(uint32_t sync_)
{
	uint32_t val = (LVDS_wrap_readl(REG_LVDS_WRAP + SAL_1) &
		~(SAL_HS_MASK | SAL_VS_MASK));

	if (!(bool)(sync_ & 0x2u)) {
		val |= (0x1u << SAL_VS_SHIFT);
	}

	if (!(bool)(sync_ & 0x4u)) {
		val |= (0x1u << SAL_HS_SHIFT);
	}

	LVDS_wrap_writel(val, REG_LVDS_WRAP + SAL_1);
}

/* LVDS_WRAP_SetDataSwap
 * Set Tx splitter output data swap
 * ch : Tx splitter output channel(0, 1, 2, 3)
 * set : Tx splitter data swap mode
 */
void LVDS_WRAP_SetDataSwap(uint32_t ch, uint32_t set)
{
	uint32_t val;

	switch (ch) {
	case 0:
		val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_CFG) & ~(TS_CFG_SWAP0_MASK));
		val |= ((set & 0x3u) << TS_CFG_SWAP0_SHIFT);
		LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_CFG);
		break;
	case 1:
		val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_CFG) & ~(TS_CFG_SWAP1_MASK));
		val |= ((set & 0x3u) << TS_CFG_SWAP1_SHIFT);
		LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_CFG);
		break;
	case 2:
		val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_CFG) & ~(TS_CFG_SWAP2_MASK));
		val |= ((set & 0x3u) << TS_CFG_SWAP2_SHIFT);
		LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_CFG);
		break;
	case 3:
		val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_CFG) & ~(TS_CFG_SWAP3_MASK));
		val |= ((set & 0x3u) << TS_CFG_SWAP3_SHIFT);
		LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_CFG);
		break;
	default:
		debug_pr(LOGL_ERR, "[ERROR][%s] : invalid parameter(%u, %u)\n", __func__, ch, set);
		break;
	}
}

/* LVDS_WRAP_SetMuxOutput
 * Set Tx splitter MUX output selection
 * mux: the type of mux (DISP_MUX_TYPE, TS_MUX_TYPE)
 * select : the select
 */
void LVDS_WRAP_SetMuxOutput(
	uint32_t mux, uint32_t ch,
	uint32_t select, uint32_t enable)
{
	uint32_t val;
	uint32_t err_mux_output = 0u;

	switch (mux) {
	case DISP_MUX_TYPE:
		val = (LVDS_wrap_readl(REG_LVDS_WRAP + DISP_MUX_SEL) & ~(DISP_MUX_SEL_SEL_MASK));
		val |= ((select & 0x3u) << DISP_MUX_SEL_SEL_SHIFT);
		LVDS_wrap_writel(val, REG_LVDS_WRAP + DISP_MUX_SEL);
		val = (LVDS_wrap_readl(REG_LVDS_WRAP + DISP_MUX_EN) & ~(DISP_MUX_EN_EN_MASK));
		val |= ((enable & 0x1u) << DISP_MUX_EN_EN_SHIFT);
		LVDS_wrap_writel(val, REG_LVDS_WRAP + DISP_MUX_EN);
		break;
	case TS_MUX_TYPE:
		switch (ch) {
		case 0u:
			val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_SEL0) & ~(TS_MUX_SEL_SEL_MASK));
			val |= ((select & 0x7u) << TS_MUX_SEL_SEL_SHIFT);
			LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_MUX_SEL0);
			val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_EN0) & ~(TS_MUX_EN_EN_MASK));
			val |= ((enable & 0x1u) << TS_MUX_EN_EN_SHIFT);
			LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_MUX_EN0);
			break;
		case 1u:
			val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_SEL1) & ~(TS_MUX_SEL_SEL_MASK));
			val |= ((select & 0x7u) << TS_MUX_SEL_SEL_SHIFT);
			LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_MUX_SEL1);
			val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_EN1) & ~(TS_MUX_EN_EN_MASK));
			val |= ((enable & 0x1u) << TS_MUX_EN_EN_SHIFT);
			LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_MUX_EN1);
			break;
		case 2u:
			val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_SEL2) & ~(TS_MUX_SEL_SEL_MASK));
			val |= ((select & 0x7u) << TS_MUX_SEL_SEL_SHIFT);
			LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_MUX_SEL2);
			val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_EN2) & ~(TS_MUX_EN_EN_MASK));
			val |= ((enable & 0x1u) << TS_MUX_EN_EN_SHIFT);
			LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_MUX_EN2);
			break;
		case 3u:
			val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_SEL3) & ~(TS_MUX_SEL_SEL_MASK));
			val |= ((select & 0x7u) << TS_MUX_SEL_SEL_SHIFT);
			LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_MUX_SEL3);
			val = (LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_EN3) & ~(TS_MUX_EN_EN_MASK));
			val |= ((enable & 0x1u) << TS_MUX_EN_EN_SHIFT);
			LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_MUX_EN3);
			break;
		default:
			err_mux_output = 1u;
			break;
		}
		break;
	default:
		err_mux_output = 1u;
		break;
	}

	if ((bool)err_mux_output) {
		debug_pr(LOGL_ERR, "[ERROR][%s] : invalid parameter(mux: %u, ch: %u)\n", __func__,
			mux, ch);

	}
}

/* LVDS_WRAP_GetMuxOutput
 * Get Tx splitter MUX output selection
 * mux: the type of mux (DISP_MUX_TYPE, TS_MUX_TYPE)
 */
void LVDS_WRAP_GetMuxOutput(
	uint32_t mux, uint32_t ch,
	uint32_t *select, uint32_t *enable)
{
	uint32_t val;
	uint32_t err_mux_output = 0u;

	switch (mux) {
	case DISP_MUX_TYPE:
		val = LVDS_wrap_readl(REG_LVDS_WRAP + DISP_MUX_SEL);
		*select = ((u32)(val & DISP_MUX_SEL_SEL_MASK) >> DISP_MUX_SEL_SEL_SHIFT);
		val = LVDS_wrap_readl(REG_LVDS_WRAP + DISP_MUX_EN);
		*enable = ((u32)(val & DISP_MUX_EN_EN_MASK) >> DISP_MUX_EN_EN_SHIFT);
		break;
	case TS_MUX_TYPE:
		switch (ch) {
		case 0u:
			val = LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_SEL0);
			*select = ((u32)(val & TS_MUX_SEL_SEL_MASK) >> TS_MUX_SEL_SEL_SHIFT);
			val = LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_EN0);
			*enable = ((u32)(val & TS_MUX_EN_EN_MASK) >> TS_MUX_EN_EN_SHIFT);
			break;
		case 1u:
			val = LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_SEL1);
			*select = ((u32)(val & TS_MUX_SEL_SEL_MASK) >> TS_MUX_SEL_SEL_SHIFT);
			val = LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_EN1);
			*enable = ((u32)(val & TS_MUX_EN_EN_MASK) >> TS_MUX_EN_EN_SHIFT);
			break;
		case 2u:
			val = LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_SEL2);
			*select = ((u32)(val & TS_MUX_SEL_SEL_MASK) >> TS_MUX_SEL_SEL_SHIFT);
			val = LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_EN2);
			*enable = ((u32)(val & TS_MUX_EN_EN_MASK) >> TS_MUX_EN_EN_SHIFT);
			break;
		case 3u:
			val = LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_SEL3);
			*select = ((u32)(val & TS_MUX_SEL_SEL_MASK) >> TS_MUX_SEL_SEL_SHIFT);
			val = LVDS_wrap_readl(REG_LVDS_WRAP + TS_MUX_EN3);
			*enable = ((u32)(val & TS_MUX_EN_EN_MASK) >> TS_MUX_EN_EN_SHIFT);
			break;
		default:
			err_mux_output = 1u;
			break;
		}
		break;
	default:
		err_mux_output = 1u;
		break;
	}

	if ((bool)err_mux_output) {
		debug_pr(LOGL_ERR, "[ERROR][%s] : invalid parameter(mux: %u, ch: %u)\n", __func__,
			mux, ch);

	}
}

/* LVDS_WRAP_SetDataPath
 * Set Data output format of tx splitter
 * ch : channel number of tx splitter mux
 * path : path number of tx splitter mux
 * set : data output format of tx splitter mux
 */
static void LVDS_WRAP_SetDataPath(
	uint32_t ch, uint32_t path, uint32_t set)
{
	uint32_t offset = 0u;

	if (path < (u32)TS_TXOUT_SEL_MAX) {
		switch (ch) {
		case 0u:
			offset = TXOUT_SEL0_0;
			break;
		case 1u:
			offset = TXOUT_SEL0_1;
			break;
		case 2u:
			offset = TXOUT_SEL0_2;
			break;
		case 3u:
			offset = TXOUT_SEL0_3;
			break;
		default:
			/*
				MISRA C-2012  Switch Statements(Rule 16.4)
				: The switch statement does not have a non-empty default clause
			*/
			break;
		}

		if ((bool)offset) {
			LVDS_wrap_writel((set & 0xFFFFFFFFu),
				REG_LVDS_WRAP + (offset + (0x4u * path)));
		}
	}

	if (!(bool)offset) {
		debug_pr(LOGL_ERR, "[ERROR][%s] : invalid parameter(ch: %u, path: %u)\n", __func__, ch, path);
	}
}

/* LVDS_WRAP_SetDataArray
 * Set the data output format of tx splitter mux
 * ch : channel number of tx splitter mux
 * data : the array included the data output format
 */
void LVDS_WRAP_SetDataArray(
	uint32_t ch, uint32_t data[TXOUT_MAX_LINE][TXOUT_DATA_PER_LINE])
{
	const uint32_t *lvdsdata = (const uint32_t *)data;
	uint32_t idx, value, path;
	uint32_t data0, data1, data2, data3;

	if (ch >= (u32)TS_MUX_IDX_MAX) {
		debug_pr(LOGL_ERR, "[ERROR][%s] : invalid parameter(ch: %d)\n", __func__, ch);
	} else {
		for (idx = 0u; idx < (TXOUT_MAX_LINE * TXOUT_DATA_PER_LINE); idx += 4u) {
			data0 = TXOUT_GET_DATA(idx);
			data1 = TXOUT_GET_DATA(idx + 1u);
			data2 = TXOUT_GET_DATA(idx + 2u);
			data3 = TXOUT_GET_DATA(idx + 3u);

			path = idx / 4u;
			value =	((lvdsdata[data3] << 24) |(lvdsdata[data2] << 16) |
				(lvdsdata[data1] << 8) | (lvdsdata[data0]));
			LVDS_WRAP_SetDataPath(ch, path, value);
		}
	}
}

/* LVDS_WRAP_SetAccessCode
 * Set the access code of LVDS Wrapper
 */
void LVDS_WRAP_SetAccessCode(void)
{
	if (LVDS_wrap_readl(0x1B9401ECu) != 0x1ACCE551u) {
		/* Please delete this code,
		 * after making a decision about safety mechanism
		 */
		LVDS_wrap_writel(0x1ACCE551u, 0x1B9401ECu);
	}
}

/* LVDS_WRAP_REsetPHY
 * software reset for PHY port
 */
void LVDS_WRAP_ResetPHY(uint32_t port, uint32_t reset)
{
	uint32_t val;

	if ((bool)reset) {
		switch (port) {
		case TS_MUX_IDX0:
		case TS_MUX_IDX1:
			// LVDS_PHY_2PORT_SYS
			val = LVDS_wrap_readl(REG_LVDS_WRAP + TS_SWRESET);
			val |= (0x1UL << 4);
			LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_SWRESET);
			break;
		case TS_MUX_IDX2:
			// LVDS_PHY_1PORT_SYS
			val = LVDS_wrap_readl(REG_LVDS_WRAP + TS_SWRESET);
			val |= (0x1UL << 3);
			LVDS_wrap_writel(val, REG_LVDS_WRAP + TS_SWRESET);
			break;
		default:
			/*
				MISRA C-2012  Switch Statements(Rule 16.4)
				: The switch statement does not have a non-empty default clause
			*/
			break;
		}
	}
	LVDS_wrap_writel(0x0u, REG_LVDS_WRAP + TS_SWRESET);
}
