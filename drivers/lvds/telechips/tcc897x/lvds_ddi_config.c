// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/reg_physical.h>
#include "lvds_ddi_config.h"


#ifndef debug_pr
#define debug_pr(level, fmt...) (CONFIG_LOGLEVEL > (signed)level) ? (void)printf(fmt) : 0;
#endif

#ifndef REG_LVDS_DDI_CONFIG
#define REG_LVDS_DDI_CONFIG (0x72380000u)
#endif

#define SUM_TO(n) (((1u + (n)) * (n)) / 2u)
#define ADD_B_TO_A(a, b, max_a) do { if (((max_a) - (a)) > (b)) { (a) = (a) + (b); } } while ((bool)0)

/**
 * The maximum number of data lanes physically supported by this PHY is 5,
 * but the number of data lanes required by OpenLDI is 4.
 * This internal value is to support data lane swapping when designing the board.
 */
#define LVDS_PHY_DATA_LANE_MAX (5u)

/**
 * LVDS_writel() - Write data to DDI_CONFIG register
 *
 * @reg_value: Data to be written
 * @reg_addr:  Register address
 */
static void LVDS_writel(uint32_t reg_value, uint32_t reg_addr)
{
	writel(reg_value, (uintptr_t)reg_addr);
}

/**
 * LVDS_readl() - Read data from DDI_CONFIG register
 *
 * @reg_addr: Register address
 */
static uint32_t LVDS_readl(uint32_t reg_addr)
{
	return readl((uintptr_t)reg_addr);
}

/**
 * LVDS_DDI_CONF_SetDispController() - LVDS selection
 *
 * @select: 0 - Connect port from LCDC0 to LVDS
 *          1 - Connect port from LCDC1 to LVDS
 */
void LVDS_DDI_CONF_SetDispController(uint32_t select)
{
	uint32_t val;

	if (select < (u32)LVDS_LCD_PORT_MAX) {
		val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CTRL) & ~(LVDS_CTRL_SEL_MASK));
		val |= ((select & 0x3u) << LVDS_CTRL_SEL_SHIFT);
		LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CTRL);
	} else {
		debug_pr(LOGL_ERR, "%s in error, invalid parameter(select: %u)\n", __func__, select);
	}
}

/**
 * LVDS_DDI_CONF_ConfigPLL() - LVDS PLL configuration
 *
 * @main_divider: Main-divider (1 <= m <= 127, default : 10)
 * @pre_divider: Pre-divider (1 <= p <= 63, default : 10)
 * @post_scaler: Post-scaler (0 <= s <= 7, default : 0)
 */
void LVDS_DDI_CONF_ConfigPLL(
	uint32_t main_divider, uint32_t pre_divider, uint32_t post_scaler)
{
	uint32_t val;

	if ((main_divider != 0u) && (pre_divider != 0u)) {
		val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CTRL) & \
				~(LVDS_CTRL_M_MASK|LVDS_CTRL_P_MASK|LVDS_CTRL_S_MASK));
		val |= ((main_divider & 0x7Fu) << LVDS_CTRL_M_SHIFT);
		val |= ((pre_divider & 0x3Fu) << LVDS_CTRL_P_SHIFT);
		val |= ((post_scaler & 0x7u) << LVDS_CTRL_S_SHIFT);
		LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CTRL);
	} else {
		debug_pr(LOGL_ERR, "%s in error, invalid parameter(m: %u, p:%u)\n", \
			__func__, main_divider, pre_divider);
	}
}

/**
 * LVDS_DDI_CONF_SetVCORange() - LVDS VCO range selection
 *
 * @select: 0 - 30~74 MHz
 *          1 - 74~160 MHz
 */
void LVDS_DDI_CONF_SetVCORange(uint32_t select)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CTRL) & ~(LVDS_CTRL_VSEL_MASK));
	val |= ((select & 0x1u) << LVDS_CTRL_VSEL_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CTRL);
}

/**
 * LVDS_DDI_CONF_SetTxOutControl() - LVDS Tx output state control
 *
 * @select: 0 - Tx output state Hi-Z when reset is enabled
 *          1 - Tx output state Ground when reset is enabled
 */
void LVDS_DDI_CONF_SetTxOutControl(uint32_t select)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CTRL) & ~(LVDS_CTRL_OC_MASK));
	val |= ((select & 0x1u) << LVDS_CTRL_OC_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CTRL);
}

/**
 * LVDS_DDI_CONF_Enable() - Enable LVDS PHY
 *
 * @enable: 0 - Disable
 *          1 - Enable
 */
void LVDS_DDI_CONF_Enable(uint32_t enable)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CTRL) & ~(LVDS_CTRL_EN_MASK));
	val |= ((enable & 0x1u) << LVDS_CTRL_EN_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CTRL);
}

/**
 * LVDS_DDI_CONF_Reset() - Hardware reset for LVDS PHY
 *
 * @reset: 0 - Reset
 *         1 - Normal
 */
void LVDS_DDI_CONF_Reset(uint32_t reset)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CTRL) & ~(LVDS_CTRL_RST_MASK));
	val |= ((reset & 0x1u) << LVDS_CTRL_RST_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CTRL);
}

/**
 * LVDS_GetTxDataOrder() - get the order of the data lanes and the unused index
 *
 * @data: The array included the data output format
 * @lane_main: The array included the output order of data lanes
 * @return: Returns the index of an unspecified data lane.
 *
 * Since the CLK lane is physically fixed,
 * only the order of the data lanes is considered.
 * Also, although 5 data lanes are physically supported,
 * the OpenLDI interface actually requires 4 data lanes.
 * Therefore, lane_main must contain 4 data lanes and 1 clock lane.
 */
static uint32_t LVDS_GetTxDataOrder(
	const uint32_t *lane_main, uint32_t *txdata_order)
{
	uint32_t idx, tx_idx = 0u;
	uint32_t unused_idx, sum_of_idx = 0u;

	(void)memset(txdata_order, 0, LVDS_PHY_DATA_LANE_MAX * sizeof(uint32_t));

	for (idx = 0u; idx < (u32)LVDS_PHY_LANE_MAX; idx++) {
		if (lane_main[idx] != (u32)LVDS_PHY_CLK_LANE) {
			ADD_B_TO_A(sum_of_idx, lane_main[idx], UINT32_MAX);
			txdata_order[tx_idx] = lane_main[idx];
			ADD_B_TO_A(tx_idx, 1u, UINT32_MAX);
		}
	}

	unused_idx = SUM_TO(LVDS_PHY_DATA_LANE_MAX) - sum_of_idx;
	if ((unused_idx == 0u) || (unused_idx > LVDS_PHY_DATA_LANE_MAX)) {
		debug_pr(LOGL_ERR, "%s in error, invalid data lane configuration.\n", __func__);
		unused_idx = LVDS_PHY_DATA_LANE_MAX;
	}
	txdata_order[tx_idx] = unused_idx;

	return unused_idx;
}

/**
 * LVDS_DDI_CONF_SetDataArrayWithLineOrder() - Set the data output format by lane order
 *
 * @data: The array included the data output format
 * @lane_main: The array included the order of the data lanes
 *
 * This function swaps the data lane order by changing the connection bit of LPXDATA.
 * Please be noted that the clock lane (CLK_LANE) is not applicable and is ignored.
 * dm_display node example:
 *     mode = <LVDS_SINGLE>;
 *     phy-ports = <PHY_PORT_S0>;
 *     lane-main = <DATA0_LANE DATA1_LANE DATA2_LANE DATA3_LANE CLK_LANE>;
 */
void LVDS_DDI_CONF_SetDataArrayWithLineOrder(
	uint32_t data[TXOUT_MAX_LINE][TXOUT_DATA_PER_LINE], const uint32_t *lane_main)
{
	uint32_t idx, value, path;
	uint32_t data0, data1, data2, data3;

	uint32_t unused_idx, txout_idx;
	uint32_t txdata_order[LVDS_PHY_DATA_LANE_MAX];
	uint32_t swapdata[LVDS_PHY_DATA_LANE_MAX][TXOUT_DATA_PER_LINE];
	const uint32_t *lvdsdata = (const uint32_t *)swapdata;

	unused_idx = LVDS_GetTxDataOrder(lane_main, txdata_order);

	// Set 4 data lanes with the data output format
	for (idx = 0u; idx < TXOUT_MAX_LINE; idx++) {
		if (txdata_order[idx] > 0u) {
			txout_idx = txdata_order[idx] - 1u;
			(void)memcpy(&swapdata[txout_idx][0], &data[idx][0],
				TXOUT_DATA_PER_LINE * sizeof(uint32_t));
		} else {
			debug_pr(LOGL_ERR, "%s in error, invalid data lane configuration.\n", __func__);
		}
	}
	// Set to High(b'1) for an unspecified data lane.
	if (unused_idx > 0u) {
		unused_idx--;
		for (idx = 0u; idx < TXOUT_DATA_PER_LINE; idx++) {
			swapdata[unused_idx][idx] = 30u + (idx % 2u);
		}
	}

	for (idx = 0u; idx < (LVDS_PHY_DATA_LANE_MAX * TXOUT_DATA_PER_LINE); idx += 4u) {
		path = idx / 4u;

		data0 = TXOUT_GET_DATA(idx);
		data1 = TXOUT_GET_DATA(idx + 1u);
		data2 = TXOUT_GET_DATA(idx + 2u);

		if (path < (u32)TS_TXOUT_SEL8) {
			data3 = TXOUT_GET_DATA(idx + 3u);
			value =	((lvdsdata[data3] << 24) | (lvdsdata[data2] << 16) |
				(lvdsdata[data1] << 8) | (lvdsdata[data0]));
		} else {
			value =	((lvdsdata[data2] << 16) |
				(lvdsdata[data1] << 8) | (lvdsdata[data0]));
		}

		LVDS_writel(value, REG_LVDS_DDI_CONFIG + LVDS_TXO_SEL(path));
		debug_pr(LOGL_DEBUG, "[%s] TXO_SEL[%u] = 0x%08X.\n", __func__, path, value);
	}
}

/**
 * LVDS_DDI_CONF_SetDataArray() - Set the data output format
 *
 * @data: The array included the data output format
 *
 * Also, if you need to change the order of the data lanes,
 * see LVDS_DDI_CONF_SetDataArrayWithLineOrder().
 */
void LVDS_DDI_CONF_SetDataArray(uint32_t data[TXOUT_MAX_LINE][TXOUT_DATA_PER_LINE])
{
	const uint32_t *lvdsdata = (const uint32_t *)data;
	uint32_t idx, value, path;
	uint32_t data0, data1, data2, data3;

	for (idx = 0u; idx < (TXOUT_MAX_LINE * TXOUT_DATA_PER_LINE); idx += 4u) {
		data0 = TXOUT_GET_DATA(idx);
		data1 = TXOUT_GET_DATA(idx + 1u);
		data2 = TXOUT_GET_DATA(idx + 2u);
		data3 = TXOUT_GET_DATA(idx + 3u);

		path = idx / 4u;
		value =	((lvdsdata[data3] << 24) |(lvdsdata[data2] << 16) |
				(lvdsdata[data1] << 8) | (lvdsdata[data0]));

		LVDS_writel(value, REG_LVDS_DDI_CONFIG + LVDS_TXO_SEL(path));
		debug_pr(LOGL_DEBUG, "[%s] TXO_SEL[%u] = 0x%08X.\n", __func__, path, value);
	}
}

/**
 * LVDS_DDI_CONF_SetClockPolarity() - Input clock polarity selection
 *
 * @inversion: 0 - Bypass (default)
 *             1 - Inversion
 */
void LVDS_DDI_CONF_SetClockPolarity(uint32_t inversion)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CFG0) & ~(LVDS_CFG0_CK_POL_SEL_MASK));
	val |= ((inversion & 0x1u) << LVDS_CFG0_CK_POL_SEL_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CFG0);
}

/**
 * LVDS_DDI_CONF_EnableAutoDeskew() - Auto deskew selection
 *
 * @enable: 0 - Bypass (default)
 *          1 - Auto deskew
 */
void LVDS_DDI_CONF_EnableAutoDeskew(uint32_t enable)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CFG0) & ~(LVDS_CFG0_AUTO_DSK_SEL_MASK));
	val |= ((enable & 0x1u) << LVDS_CFG0_AUTO_DSK_SEL_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CFG0);
}

/**
 * LVDS_DDI_CONF_BiasControl() - Bias current control pin for Skew
 *
 * @level: 12(0xC) - Default current value
 *         Others  - Reserved
 *
 * If you want to know about Skew Bias in detail, feel free to contact to our FAE.
 */
void LVDS_DDI_CONF_BiasControl(uint32_t level)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CFG0) & ~(LVDS_CFG0_SK_BIAS_MASK));
	val |= ((level & 0xFu) << LVDS_CFG0_SK_BIAS_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CFG0);
}

/**
 * LVDS_DDI_CONF_LockControl() - Lock signal selection
 *
 * @enable: 0 - Lock enable (default)
 *          1 - Lock disable
 */
void LVDS_DDI_CONF_LockControl(uint32_t enable)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CFG1) & ~(LVDS_CFG1_LOCK_CNT_MASK));
	val |= (((enable & 0x1u) ^ 0x1u) << LVDS_CFG1_LOCK_CNT_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CFG1);
}

/**
 * LVDS_DDI_CONF_ConnectivityControl() - Connectivity control
 *
 * @select: 0 - Tx operating (default)
 *          1 - Connectivity check
 *          2 - Reserved
 *          3 - Reserved
 */
void LVDS_DDI_CONF_ConnectivityControl(uint32_t select)
{
	uint32_t val;

	if (select >= (u32)LVDS_CNNCT_MODE_MAX) {
		debug_pr(LOGL_ERR, "%s in error, invalid parameter(select: %d)\n", __func__, select);
	} else {
		val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CFG1) & \
				~(LVDS_CFG1_CNNCT_CNT_MASK | LVDS_CFG1_CNNCT_MODE_SEL_MASK));
		val |= ((select & 0x3u) << LVDS_CFG1_CNNCT_CNT_SHIFT);
		val |= ((select & 0x1u) << LVDS_CFG1_CNNCT_MODE_SEL_SHIFT);
		LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CFG1);
	}
}

/**
 * LVDS_DDI_CONF_VODControl() - VOD control
 *
 * @enable: 0 - Normal mode with pre-emphasis (default)
 *          1 - VOD only
 * @pre_driver_enable: 0 - Disable
 *                     1 - Enable
 *
 * Pre-emphasis's pre-driver control pin is only for normal mode.
 */

void LVDS_DDI_CONF_VODControl(uint32_t enable, uint32_t pre_driver_enable)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CFG1) & \
			~(LVDS_CFG1_VOD_HIGH_S_MASK | LVDS_CFG1_VOD_ONLY_CNT_MASK));
	val |= ((enable & 0x1u) << LVDS_CFG1_VOD_HIGH_S_SHIFT);
	val |= ((pre_driver_enable & 0x1u) << LVDS_CFG1_VOD_ONLY_CNT_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CFG1);
}

/**
 * LVDS_DDI_CONF_SetSourceTermination() - Source termination resistor selection
 *
 * @enable: 0 - Off
 *          1 - Termination
 */
void LVDS_DDI_CONF_SetSourceTermination(uint32_t enable)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CFG1) & ~(LVDS_CFG1_SRC_TRH_MASK));
	val |= ((enable & 0x1u) << LVDS_CFG1_SRC_TRH_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CFG1);
}

/**
 * LVDS_DDI_CONF_SetVODLevel() - Tx output differential voltage level control
 *
 * @level: 151(0x97) - default value
 */
void LVDS_DDI_CONF_SetVODLevel(uint32_t level)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CFG1) & ~(LVDS_CFG1_CNT_VOD_H_MASK));
	val |= ((level & 0xFFu) << LVDS_CFG1_CNT_VOD_H_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CFG1);
}

/**
 * LVDS_DDI_CONF_SetPreEmphasisLevel() - Tx output pre-emphasis level control
 *
 * @level: 1(0x01) - default value
 *
 * Test only. This function is for debugging
 */
void LVDS_DDI_CONF_SetPreEmphasisLevel(uint32_t level)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CFG1) & ~(LVDS_CFG1_CNT_PEN_H_MASK));
	val |= ((level & 0xFFu) << LVDS_CFG1_CNT_PEN_H_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CFG1);
}

/**
 * LVDS_DDI_CONF_SetVosLevel() - Vos control
 *
 * @level: 4(0x04) - default value
 */
void LVDS_DDI_CONF_SetVosLevel(uint32_t level)
{
	uint32_t val;

	val = (LVDS_readl(REG_LVDS_DDI_CONFIG + LVDS_CFG1) & ~(LVDS_CFG1_FC_CODE_MASK));
	val |= ((level & 0x7u) << LVDS_CFG1_FC_CODE_SHIFT);
	LVDS_writel(val, REG_LVDS_DDI_CONFIG + LVDS_CFG1);
}

