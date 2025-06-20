// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/reg_physical.h>
#include <mach/chipinfo.h>
#include <linux/delay.h>
#include "lvds_phy.h"

#ifndef HwLVDS_PHY_DUAL
#define HwLVDS_PHY_DUAL (0x1B942000u)
#endif

#ifndef HwLVDS_PHY_SINGLE
#define HwLVDS_PHY_SINGLE (0x1B941000u)
#endif

#define REG_LVDS_PORT_DUAL(port) \
	(uint32_t)(HwLVDS_PHY_DUAL + ((port) << 10))
#define REG_LVDS_PORT_SINGLE(port) \
	(uint32_t)(HwLVDS_PHY_SINGLE + ((port) << 10))

#define LVDS_PHY_VCO_RANGE_MIN (560000000u)  // 560Mhz
#define LVDS_PHY_VCO_RANGE_MAX (1120000000u) // 1120Mhz
#define LVDS_PHY_UPSAMPLE_RATIO_MAX (0x4u)   // 0~4

//#define LVDS_DEBUG
#ifdef LVDS_DEBUG
#define LVDS_DBG(fmt, args...) pr_info("[ LVDS ] " fmt, ##args)
#else
#define LVDS_DBG(fmt, args...) \
	do {                   \
	} while ((bool)0)
#endif // LVDS_DEBUG

#define ABS_DIFF(a, b) (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))

static uint32_t lvds_phy_regs[] = {
	REG_LVDS_PORT_SINGLE(0u),
	0u,
	REG_LVDS_PORT_DUAL(2u),
	REG_LVDS_PORT_DUAL(3u)
};

/* for upsample ratio calculation
 * n = upsample ratio
 * X = 2^n (X = 1, 2, 4, 8 16)
 */
static uint32_t ref_ratio_arr[5][2] = {
	{0u, 1u},
	{1u, 2u},
	{2u, 4u},
	{3u, 8u},
	{4u, 16u}
};

static uint32_t LVDS_PHY_GetAddress(uint32_t port)
{
	uint32_t phy_address = 0u;

	if (port < (u32)LVDS_PHY_PORT_MAX) {
		phy_address = lvds_phy_regs[port];
	}

	return phy_address;
}

void LVDS_PHY_LaneSwap(
	uint32_t s_port_en, uint32_t lvds_main, uint32_t lvds_sub,
	const uint32_t *lane_main, const uint32_t *lane_sub)
{
	uint32_t idx;

	for (idx = 0u; idx < (u32)LVDS_PHY_LANE_MAX; idx++) {
		LVDS_PHY_SetLaneSwap(lvds_main, idx, lane_main[idx]);
		if ((bool)s_port_en) {
			LVDS_PHY_SetLaneSwap(lvds_sub, idx, lane_sub[idx]);
		}
	}
}

/* LVDS_PHY_GetCalibrationLevel
 * Get setting value for VCM/VSW calibration
 * vcm : typical vcm level of lvds panel
 * vsw : typical vsw level of lvds panel
 */
static void LVDS_PHY_GetCalibrationLevel(
	uint32_t vcm, uint32_t vsw, uint32_t *vcmcal,
	uint32_t *swingcal)
{
	uint32_t swing_max, swing_min;
	uint32_t index, step;

	if (vcm < 770u) { /* 0 : 190~370*/
		*vcmcal = 0u;
		swing_min = 190u;
		swing_max = 370u;
	}
	else if (vcm < 870u) { /* 1 : 210 ~ 470*/
		*vcmcal = 1u;
		swing_min = 210u;
		swing_max = 470u;
	}
	else if (vcm < 960u) { /* 2 : 210 ~ 540 */
		*vcmcal = 2u;
		swing_min = 210u;
		swing_max = 540u;
	}
	else if (vcm < 1050u) { /* 3 : 210 ~ 570 */
		*vcmcal = 3u;
		swing_min = 210u;
		swing_max = 570u;
	}
	else if (vcm < 1130u) { /* 4 : 210 ~ 560 */
		*vcmcal = 4u;
		swing_min = 210u;
		swing_max = 560u;
	}
	else if (vcm < 1210u) { /* 5 : 210 ~ 530 */
		*vcmcal = 5u;
		swing_min = 210u;
		swing_max = 530u;
	}
	else if (vcm < 1290u) { /* 6 : 210 ~ 500 */
		*vcmcal = 6u;
		swing_min = 210u;
		swing_max = 500u;
	} else { /* 7 : 210 ~ 460 */
		*vcmcal = 7u;
		swing_min = 210u;
		swing_max = 460u;
	}

	step = (swing_max - swing_min) / 16u;
	for (index = 0u; index < 16u; index++) {
		*swingcal = index;
		if (vsw <= (swing_min + (step * index))) {
			break;
		}
	}
}

/* LVDS_PHY_GetUpsampleRatio
 * Get upsample ratio value for Automatic FCON
 * p_port : the primary port number of lvds phy
 * s_port : the secondary port number of lvds phy
 * freq : lvds pixel clock
 */
uint32_t LVDS_PHY_GetUpsampleRatio(
	uint32_t p_port, uint32_t s_port, uint32_t freq)
{
	const uint32_t p_reg = LVDS_PHY_GetAddress(p_port);
	const uint32_t s_reg = LVDS_PHY_GetAddress(s_port);
	uint32_t idx =0u;

	if ((bool)p_reg) {
		uint32_t found_ratio = 0u;
		uint32_t pxclk = ((bool)s_reg) ? (freq / 2u) : freq;

		for (idx = 0u; idx < ARRAY_SIZE(ref_ratio_arr); idx++) {
			if ((pxclk < UINT_MAX) && (ref_ratio_arr[idx][1u] < UINT_MAX)) {
				if (((pxclk * 7U) * ref_ratio_arr[idx][1u])    > LVDS_PHY_VCO_RANGE_MIN) {
				    found_ratio = 1u;
				    break;
				}
			}
		}

		if(found_ratio == 0u) {
			LVDS_DBG("error in %s: can not get upsample ratio (%dMhz)\n",__func__,(pxclk / 1000000u));
		}
	} else {
		LVDS_DBG("error in %s can not get hw address\n", __func__);
	}

	return idx;
}

/* LVDS_PHY_GetRefCnt
 * Get Reference count value for Automatic FCON
 * p_port : the primary port number of lvds phy
 * s_port : the secondary port number of lvds phy
 * freq : lvds pixel clock
 * upsample_ratio : upsample_ratio
 * Formula : REF_CNT = ((pixel clk * 7 * (2^upsample_ratio))/20)/24*16*16
 */
uint32_t LVDS_PHY_GetRefCnt(
	uint32_t p_port, uint32_t s_port, uint32_t freq,
	uint32_t upsample_ratio)
{
	const uint32_t p_reg = LVDS_PHY_GetAddress(p_port);
	const uint32_t s_reg = LVDS_PHY_GetAddress(s_port);
	uint32_t ret = 0u;

	if ((bool)p_reg) {
		uint32_t pxclk = ((bool)s_reg) ? (freq / 2u) : freq;

		if (upsample_ratio > LVDS_PHY_UPSAMPLE_RATIO_MAX) {
			LVDS_DBG("error in %s: invaild parameter (pxclk:%dMhz / upsample_ratio:%d)\n",
				__func__, (pxclk / 1000000u), upsample_ratio);
		}
		else {
			u64 clk_tmp = (((u64)pxclk * 7u) * ref_ratio_arr[upsample_ratio][1u]);
			if(clk_tmp < UINT_MAX) {
				ret = (((u32)clk_tmp / 20u)	/ 24u * 16u * 16u) / 1000000u;
			}
		}
	}
	else {
		LVDS_DBG("error in %s can not get hw address\n", __func__);
	}

	return ret;
}

static void LVDS_writel(
	uint32_t lvds_reg_value, uint32_t lvds_reg_address)
{
	writel(lvds_reg_value, (uintptr_t)lvds_reg_address);
}

static uint32_t LVDS_readl(uint32_t lvds_reg_address)
{
	return readl((uintptr_t)lvds_reg_address);
}

/* LVDS_PHY_SetFormat
 * Set LVDS phy format information
 * port : the port number of lvds phy
 * balance : balanced mode enable (0-disable, 1-enable)
 * depth : color depth(0-6bit, 1-8bit)
 * format : 0-VESA, 1-JEIDA
 * freq : 0~4 video frequency
 */
void LVDS_PHY_SetFormat(
	uint32_t port, uint32_t balance, uint32_t depth,
	uint32_t format, uint32_t freq)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t value;

	if ((bool)phy_reg) {
		value =	(LVDS_readl(phy_reg + LVDS_FORMAT) &
			~(LVDS_FORMAT_BALANCED_EN_MASK |
			  LVDS_FORMAT_COLOR_DEPTH_MASK |
			  LVDS_FORMAT_COLOR_FORMAT_MASK |
			  LVDS_FORMAT_UPSAMPLE_RATIO_MASK));
		value |= (((balance & 0x1u) << LVDS_FORMAT_BALANCED_EN_SHIFT) |
			((depth & 0x1u) << LVDS_FORMAT_COLOR_DEPTH_SHIFT) |
			((format & 0x1u) << LVDS_FORMAT_COLOR_FORMAT_SHIFT) |
			((freq & 0x7u) << LVDS_FORMAT_UPSAMPLE_RATIO_SHIFT));
		LVDS_writel(value, phy_reg + LVDS_FORMAT);
	}
}

/* LVDS_PHY_SetUserMode
 * Control lane skew and p/n swap
 * port : the port number of lvds phy
 * lane : lane type
 * skew : lane skew value
 * swap : lane p/n swap (0-Normal, 1-Swap p/n)
 */
void LVDS_PHY_SetUserMode(
	uint32_t port, uint32_t lane, uint32_t skew,
	uint32_t pn_swap)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t value;

	if ((bool)phy_reg) {
		switch (lane) {
		case LVDS_PHY_CLK_LANE:
			value = (LVDS_readl(phy_reg + LVDS_USER_MODE_PHY_IF_SET0) &
				~(LVDS_USER_MODE_PHY_IF_SET0_CLK_PN_SWAP_MASK |
				LVDS_USER_MODE_PHY_IF_SET0_CLK_LANE_SKEW_MASK));
			value |= (((skew & 0x7u) << LVDS_USER_MODE_PHY_IF_SET0_CLK_LANE_SKEW_SHIFT) |
				((pn_swap & 0x1u) << LVDS_USER_MODE_PHY_IF_SET0_CLK_PN_SWAP_SHIFT));
			LVDS_writel(value, phy_reg + LVDS_USER_MODE_PHY_IF_SET0);
			break;
		case LVDS_PHY_DATA0_LANE:
			value = (LVDS_readl(phy_reg + LVDS_USER_MODE_PHY_IF_SET0) &
				~(LVDS_USER_MODE_PHY_IF_SET0_DATA0_PN_SWAP_MASK |
				LVDS_USER_MODE_PHY_IF_SET0_DATA0_LANE_SKEW_MASK));
			value |= (((skew & 0x7u) << LVDS_USER_MODE_PHY_IF_SET0_DATA0_LANE_SKEW_SHIFT) |
				((pn_swap & 0x1u) << LVDS_USER_MODE_PHY_IF_SET0_DATA0_PN_SWAP_SHIFT));
			LVDS_writel(value, phy_reg + LVDS_USER_MODE_PHY_IF_SET0);
			break;
		case LVDS_PHY_DATA1_LANE:
			value = (LVDS_readl(phy_reg + LVDS_USER_MODE_PHY_IF_SET0) &
				~(LVDS_USER_MODE_PHY_IF_SET0_DATA1_PN_SWAP_MASK |
				LVDS_USER_MODE_PHY_IF_SET0_DATA1_LANE_SKEW_MASK));
			value |= (((skew & 0x7u) << LVDS_USER_MODE_PHY_IF_SET0_DATA1_LANE_SKEW_SHIFT) |
				((pn_swap & 0x1u) << LVDS_USER_MODE_PHY_IF_SET0_DATA1_PN_SWAP_SHIFT));
			LVDS_writel(value, phy_reg + LVDS_USER_MODE_PHY_IF_SET0);
			break;
		case LVDS_PHY_DATA2_LANE:
			value = (LVDS_readl(phy_reg + LVDS_USER_MODE_PHY_IF_SET0) &
				~(LVDS_USER_MODE_PHY_IF_SET0_DATA2_PN_SWAP_MASK |
				LVDS_USER_MODE_PHY_IF_SET0_DATA2_LANE_SKEW_MASK));
			value |= (((skew & 0x7u) << LVDS_USER_MODE_PHY_IF_SET0_DATA2_LANE_SKEW_SHIFT) |
				((pn_swap & 0x1u) << LVDS_USER_MODE_PHY_IF_SET0_DATA2_PN_SWAP_SHIFT));
			LVDS_writel(value, phy_reg + LVDS_USER_MODE_PHY_IF_SET0);
			break;
		case LVDS_PHY_DATA3_LANE:
			value = (LVDS_readl(phy_reg + LVDS_USER_MODE_PHY_IF_SET0) &
				~(LVDS_USER_MODE_PHY_IF_SET0_DATA3_PN_SWAP_MASK |
				LVDS_USER_MODE_PHY_IF_SET0_DATA3_LANE_SKEW_MASK));
			value |= (((skew & 0x7u) << LVDS_USER_MODE_PHY_IF_SET0_DATA3_LANE_SKEW_SHIFT) |
				((pn_swap & 0x1u) << LVDS_USER_MODE_PHY_IF_SET0_DATA3_PN_SWAP_SHIFT));
			LVDS_writel(value, phy_reg + LVDS_USER_MODE_PHY_IF_SET0);
			break;
		case LVDS_PHY_LANE_MAX:
		default:
			LVDS_DBG("%s in error, invaild parameter(lane:%u)\n",__func__,lane);
			break;
		}
	}
}

void LVDS_PHY_SetLaneSwap(
	uint32_t port, uint32_t lane, uint32_t select)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t lane_offset = 0u;
	uint32_t value;

	if ((bool)phy_reg) {
		if(lane < (u32)LVDS_PHY_LANE_MAX) {
			lane_offset = LVDS_USER_MODE_PHY_IF_SET1_SET_LANE0_SHIFT + (lane * 0x4u);
			value =(LVDS_readl(phy_reg + LVDS_USER_MODE_PHY_IF_SET1) & ~(0x7UL << lane_offset));
			value |= ((select & 0x7u) << lane_offset);
			LVDS_writel(value, phy_reg + LVDS_USER_MODE_PHY_IF_SET1);
		}
		else {
			LVDS_DBG("%s in error, invaild parameter(lane:%u)\n",__func__, lane);
		}
	}
}

/* LVDS_PHY_SetFifoEnableTiming
 * Select FIFO2 enable timing
 * port : the port number of lvds phy
 * cycle : FIFO enable after n clock cycle (0~3 cycle)
 */
void LVDS_PHY_SetFifoEnableTiming(uint32_t port, uint32_t cycle)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t value;

	if ((bool)phy_reg) {
		LVDS_writel(0x00000000u, phy_reg + LVDS_STARTUP_MODE);
		value =(LVDS_readl(phy_reg + LVDS_STARTUP_MODE) &
			~(LVDS_STARTUP_MODE_FIFO2_RD_EN_TIMING_MASK));
		value |= ((cycle & 0x3u) << LVDS_STARTUP_MODE_FIFO2_RD_EN_TIMING_SHIFT);
		LVDS_writel(value, phy_reg + LVDS_STARTUP_MODE);
	}
}

/* LVDS_PHY_SetPortOption
 * Selects a port option for dual pixel mode
 * port : the port number of lvds phy
 * port_mode : the mode of this port(0-main port, 1-sub port)
 * sync_swap : swap vsync/hsync position (0-do not swap, 1-swap)
 * use_other_port : 0-normal, 1-use sync from other port
 * lane_en : lane enable (CLK, DATA0~3)
 * sync_transmit_src : sync transmit source (0-normal, 1-other port)
 */
void LVDS_PHY_SetPortOption(
	uint32_t port, uint32_t port_mode, uint32_t sync_swap,
	uint32_t use_other_port, uint32_t lane_en,
	uint32_t sync_transmit_src)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t value;

	if ((bool)phy_reg) {
		value =(LVDS_readl(phy_reg + LVDS_PORT) & ~(LVDS_PORT_SET_SECONDARY_PORT_MASK |
			LVDS_PORT_VSYNC_HSYNC_SWAP_MASK | LVDS_PORT_USE_SYNC_FROM_OP_MASK |
			LVDS_PORT_LANE_EN_MASK | LVDS_PORT_SYNC_TRANSMITTED_MASK));

		value |=(((port_mode & 0x1u)  << LVDS_PORT_SET_SECONDARY_PORT_SHIFT)|
			((sync_swap & 0x1u)  << LVDS_PORT_VSYNC_HSYNC_SWAP_SHIFT)|
			((use_other_port & 0x1u)   << LVDS_PORT_USE_SYNC_FROM_OP_SHIFT)|
			((lane_en & 0x1Fu) << LVDS_PORT_LANE_EN_SHIFT)|
			((sync_transmit_src & 0x7u)   << LVDS_PORT_SYNC_TRANSMITTED_SHIFT));
		LVDS_writel(value, phy_reg + LVDS_PORT);
	}
}

/* LVDS_PHY_LaneEnable
 * Set lvds lane enable/disable
 * port : the port number of lvds phy
 */
void LVDS_PHY_LaneEnable(uint32_t port, uint32_t enable)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t value;

	if ((bool)phy_reg) {
		value = (LVDS_readl(phy_reg + LVDS_PORT) & ~(LVDS_PORT_LANE_EN_MASK));

		if ((bool)enable) {
			value |= (0x1Fu << LVDS_PORT_LANE_EN_SHIFT);
		}
		LVDS_writel(value, phy_reg + LVDS_PORT);
	}
}

/* LVDS_PHY_FifoEnable
 * Set lvds phy fifo enable
 * port : the port number of lvds phy
 */
void LVDS_PHY_FifoEnable(uint32_t port, uint32_t enable)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t value;

	if ((bool)phy_reg) {
		value = (LVDS_readl(phy_reg + LVDS_EN) &
			~(LVDS_EN_FIFO2_EN_MASK | LVDS_EN_FIFO1_EN_MASK|
			  LVDS_EN_FIFO0_EN_MASK | LVDS_EN_DATA_EN_MASK));

		if ((bool)enable) {
			value |= ((0x1u << LVDS_EN_FIFO2_EN_SHIFT) |
				(0x1u << LVDS_EN_FIFO1_EN_SHIFT) |
				(0x1u << LVDS_EN_FIFO0_EN_SHIFT) |
				(0x1u << LVDS_EN_DATA_EN_SHIFT));
		}
		LVDS_writel(value, phy_reg + LVDS_EN);
	}
}

/* LVDS_PHY_FifoReset
 * Reset LVDS PHY Fifo
 * port : the port number of lvds phy
 * reset : 0-release, 1-reset
 */
void LVDS_PHY_FifoReset(uint32_t port, uint32_t reset)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);

	if ((bool)phy_reg) {
		if ((bool)reset) {
			LVDS_writel(0x0000118Fu, phy_reg + LVDS_RESETB);
		}
		else {
			LVDS_writel(0x00001FFFu, phy_reg + LVDS_RESETB);
		}
	}
}

/* LVDS_PHY_SWReset
 * Control lvds phy swreset
 * port : the port number of lvds phy
 * reset : 0-release, 1-reset
 */
void LVDS_PHY_SWReset(uint32_t port, uint32_t reset)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);

	if ((bool)phy_reg) {
		if ((bool)reset) {
			LVDS_writel(0x00000000u, phy_reg + LVDS_RESETB);
		}
		else {
			LVDS_writel(0x00001FFFu, phy_reg + LVDS_RESETB);
		}
	}
}

/* LVDS_PHY_ClockEnable
 * Control lvds phy clock enable
 * port : the port number of lvds phy
 * enable : 0-disable, 1-enable
 */
void LVDS_PHY_ClockEnable(uint32_t port, uint32_t enable)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);

	if ((bool)phy_reg) {
		if ((bool)enable) {
			LVDS_writel(0x000100FF, phy_reg + LVDS_CLK_SET);
		}
		else {
			LVDS_writel(0x00000000, phy_reg + LVDS_CLK_SET);
		}
	}
}

/* LVDS_PHY_SetStrobe
 * Sets LVDS strobe registers
 * port : the port number of lvds phy
 * mode : 0-Manual, 1- Auto
 * enable : 0-disable, 1-enable
 */
void LVDS_PHY_SetStrobe(
	uint32_t port, uint32_t mode, uint32_t enable)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t value;

	if ((bool)phy_reg) {
		value = (LVDS_readl(phy_reg + LVDS_STB_EN) & ~(LVDS_STB_EN_STB_EN_MASK));
		value |= ((enable & 0x1u) << LVDS_STB_EN_STB_EN_SHIFT);
		LVDS_writel(value, phy_reg + LVDS_STB_EN);

		value =	(LVDS_readl(phy_reg + LVDS_AUTO_STB_SET) & ~(LVDS_AUTO_STB_SET_STB_AUTO_EN_MASK));
		value |= ((mode & 0x1u) << LVDS_AUTO_STB_SET_STB_AUTO_EN_SHIFT);
		LVDS_writel(value, phy_reg + LVDS_AUTO_STB_SET);
	}
}

/* LVDS_PHY_SetFcon
 * Setup LVDS PHY Manual/Automatic Coarse tunning
 * port : the port number of lvds phy
 * mode : coarse tunning method 0-manual, 1-automatic
 * loop : feedback loop mode 0-closed loop, 1-open loop
 * fcon : frequency control value
 */
void LVDS_PHY_SetFcon(
	uint32_t port, uint32_t mode, uint32_t loop,
	uint32_t division, uint32_t fcon)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t value;
	uint32_t target_th = 0x2020UL; /* Alphachips Guide for FCON */

	if ((bool)phy_reg) {
		switch (mode) {
		case LVDS_PHY_FCON_MANUAL:
			value = (LVDS_readl(phy_reg + LVDS_CTSET1) & ~(LVDS_CTSET1_MPLL_CTLCK_MASK |
				LVDS_CTSET1_MPLL_DIVN_MASK | LVDS_CTSET1_MPLL_FCON_MASK));
			value |= (((loop & 0x1u) << LVDS_CTSET1_MPLL_CTLCK_SHIFT) |
				((division & 0x3u)  << LVDS_CTSET1_MPLL_DIVN_SHIFT) |
				((fcon & 0x3FFu) << LVDS_CTSET1_MPLL_FCON_SHIFT));
			LVDS_writel(value, phy_reg + LVDS_CTSET1); // offset: 0x094

			value = (LVDS_readl(phy_reg + LVDS_FCOPT) &
				~(LVDS_FCOPT_CLK_DET_SEL_MASK  | LVDS_FCOPT_CT_SEL_MASK));
			value |= ((mode & 0x1u) << LVDS_FCOPT_CT_SEL_SHIFT);
			LVDS_writel(value, phy_reg + LVDS_FCOPT); // offset: 0x09C
			break;
		case LVDS_PHY_FCON_AUTOMATIC:
			value = (LVDS_readl(phy_reg + LVDS_FCCNTR1) &
				~(LVDS_FCCNTR1_CONTIN_TARGET_TH_MASK | LVDS_FCCNTR1_REF_CNT_MASK));
			value |= (((target_th & 0xFFFFu) << LVDS_FCCNTR1_CONTIN_TARGET_TH_SHIFT) |
				((fcon & 0xFFFFu) << LVDS_FCCNTR1_REF_CNT_SHIFT));
			LVDS_writel(value, phy_reg + LVDS_FCCNTR1); // offset: 0x0B0

			value = (LVDS_readl(phy_reg + LVDS_CTSET1) & ~(LVDS_CTSET1_MPLL_CTLCK_MASK |
				LVDS_CTSET1_MPLL_DIVN_MASK | LVDS_CTSET1_MPLL_FCON_MASK));
			value |= (((loop & 0x1u) << LVDS_CTSET1_MPLL_CTLCK_SHIFT) | /* Shoulb be set to 'Open' loop */
				((division & 0x3u)  << LVDS_CTSET1_MPLL_DIVN_SHIFT));
			LVDS_writel(value, phy_reg + LVDS_CTSET1); // offset: 0x094

			value =	(LVDS_readl(phy_reg + LVDS_CTSET0) &
				~(LVDS_CTSET0_ENABLE_MASK | LVDS_CTSET0_RUN_MASK));
			value |= (0x1u << LVDS_CTSET0_ENABLE_SHIFT);
			LVDS_writel(value, phy_reg + LVDS_CTSET0); // offset: 0x090

			value = (LVDS_readl(phy_reg + LVDS_FCOPT) &
				~(LVDS_FCOPT_CLK_DET_SEL_MASK | LVDS_FCOPT_CT_SEL_MASK));
			value |= (((mode & 0x1u) << LVDS_FCOPT_CT_SEL_SHIFT)|
				(0x1u << LVDS_FCOPT_CLK_DET_SEL_SHIFT));
			LVDS_writel(value, phy_reg + LVDS_FCOPT); // offset: 0x09C

			LVDS_writel(0x10101010u, phy_reg + LVDS_FCCNTR0);
			/* Alphachips Guide for FCON */
			break;
		case LVDS_PHY_FCON_MAX:
		default:
			LVDS_DBG("%s in error, invaild parameter(mode: %u)\n",__func__, mode);
			break;
		}
	}
}

/* LVDS_PHY_SetCFcon
 * Check fcon status and setup cfcon value
 * port : the port number of lvds phy
 * mode : fcon control mode 0-Manual, 1-Automatic
 * enable : 0-cfcon disable 1-cfcon enable
 */
void LVDS_PHY_SetCFcon(uint32_t port, uint32_t mode, uint32_t enable)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t value;
	uint32_t time_out = 0u;
	uint32_t update_step = 0x1u;
	uint32_t pd_fcstat;

	if ((bool)phy_reg) {
		if (mode == (u32)LVDS_PHY_FCON_AUTOMATIC) {
			while (time_out < 100u) {
				time_out++;
				pd_fcstat = (LVDS_readl(phy_reg + LVDS_FCSTAT) &
					(LVDS_FCST_CLK_OK_MASK | LVDS_FCST_DONE_MASK | LVDS_FCST_ERROR_MASK));
				if (pd_fcstat == (LVDS_FCST_CLK_OK_MASK | LVDS_FCST_DONE_MASK)) {
					if ((bool)enable) {
						/* Alphachips Guide For CFCON */
						LVDS_writel(0x0000000Cu | (update_step << 4), phy_reg + LVDS_FCCONTINSET0);
						LVDS_writel(0x00000960u, phy_reg + LVDS_FCCONTINSET1);
						LVDS_writel(0x003FF005u, phy_reg + LVDS_FCCONTINSET2);
						LVDS_writel(0x0000000Du | (update_step << 4), phy_reg + LVDS_FCCONTINSET0);
					}
					break;
				} else {
					mdelay(1);
				}
			}
		}
		if(time_out >= 100u) {
			LVDS_DBG("%s time out\n", __func__);
		}

		/* Change loop mode to 'Closed' loop */
		value = (LVDS_readl(phy_reg + LVDS_CTSET1) & ~(LVDS_CTSET1_MPLL_CTLCK_MASK));
		value |= (0x1UL << LVDS_CTSET1_MPLL_CTLCK_SHIFT);
		LVDS_writel(value, phy_reg + LVDS_CTSET1); // offset: 0x094
	}
}

void LVDS_PHY_CheckPLLStatus(uint32_t p_port, uint32_t s_port)
{
	uint32_t p_reg = LVDS_PHY_GetAddress(p_port);
	uint32_t s_reg = LVDS_PHY_GetAddress(s_port);
	uint32_t time_out = 0u;
	uint32_t p_pllstatus;
	uint32_t s_pllstatus;
	uint32_t exit_loop = 0u;

	if ((bool)p_reg) {
		while (time_out < 100u) {
			time_out++;
			p_pllstatus = (LVDS_readl(p_reg + LVDS_MONITOR_DEBUG1) & LVDS_MONITOR_DEBUG1_PLL_STATUS_MASK);
			if ((bool)s_reg) {
				s_pllstatus = (LVDS_readl(s_reg + LVDS_MONITOR_DEBUG1) & LVDS_MONITOR_DEBUG1_PLL_STATUS_MASK);
				if ((p_pllstatus == LVDS_MONITOR_DEBUG1_PLL_STATUS_MASK) &&
					(s_pllstatus == LVDS_MONITOR_DEBUG1_PLL_STATUS_MASK)) {
					exit_loop = 1u;
				}
			}
			else {
				if (p_pllstatus == LVDS_MONITOR_DEBUG1_PLL_STATUS_MASK) {
					exit_loop = 1u;
				}
			}
			if((bool)exit_loop) {
				break;
			}
			mdelay(1);
		}
		if(time_out >= 100u) {
			LVDS_DBG("%s time out\n", __func__);
		}
	}
}

/* read the fcon value of the port */
uint32_t LVDS_PHY_Fcon_Value(uint32_t port)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t fcon = 0u;

	if ((bool)phy_reg) {
		fcon = ((LVDS_readl(phy_reg + LVDS_FCRESEVAL)) >> 3) & 0x3ffu;
	}
	return fcon;
}

void LVDS_PHY_CheckFcon(
	uint32_t p_port, uint32_t s_port, uint32_t mfcon,
	uint32_t sfcon)
{
	uint32_t p_reg = LVDS_PHY_GetAddress(p_port);
	uint32_t s_reg = LVDS_PHY_GetAddress(s_port);
	uint32_t fcon_check_maxcnt = 200000u; 	/* the max count to check fcon and status */
	uint32_t p_pllstatus = 0u;
	uint32_t s_pllstatus = 0u;
	uint32_t lock_max = 1000u; 	/* the count : sequential phydet value : 0xf */
	uint32_t lock_cnt = 0u;
	uint32_t loop_cnt = 0u;

	(void)mfcon;
	(void)sfcon;

	if ((bool)p_reg) {
		while ((loop_cnt < fcon_check_maxcnt) && (lock_cnt < lock_max)) {
			p_pllstatus = (LVDS_readl(p_reg + LVDS_MONITOR_DEBUG1) & LVDS_MONITOR_DEBUG1_PLL_STATUS_MASK);
			if ((bool)s_reg) {
				s_pllstatus =(LVDS_readl(s_reg + LVDS_MONITOR_DEBUG1) & LVDS_MONITOR_DEBUG1_PLL_STATUS_MASK);

				if ((p_pllstatus == LVDS_MONITOR_DEBUG1_PLL_STATUS_MASK) &&
					(s_pllstatus == LVDS_MONITOR_DEBUG1_PLL_STATUS_MASK)) {
					lock_cnt++;
				}
				else {
					lock_cnt = 0u;
				}
			}
			else { // single LVDS
				if (p_pllstatus == LVDS_MONITOR_DEBUG1_PLL_STATUS_MASK) {
					lock_cnt++;
				}
				else {
					lock_cnt = 0u;
				}
			}
			loop_cnt++;
		}
	}

	pr_info("%s :[%s] PLL locking loop_cnt = %u, lock_cnt = %u\n",__func__,
		(lock_cnt < lock_max) ? "FAILED":"OK", loop_cnt, lock_cnt);
}

/* LVDS_PHY_FConEnable
 * Controls FCON running enable
 * port : the port number of lvds phy
 * enable : 0-disable, 1-enable
 */
void LVDS_PHY_FConEnable(uint32_t port, uint32_t enable)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t value;

	if ((bool)phy_reg) {
		value = (LVDS_readl(phy_reg + LVDS_CTSET0) & ~(LVDS_CTSET0_ENABLE_MASK | LVDS_CTSET0_RUN_MASK));
		value |=(((enable & 0x1u) << LVDS_CTSET0_ENABLE_SHIFT) | ((enable & 0x1u) << LVDS_CTSET0_RUN_SHIFT));
		LVDS_writel(value, phy_reg + LVDS_CTSET0);
	}
}

/* LVDS_PHY_StrobeWrite
 * Write LVDS PHY Strobe register
 * reg : LVDS PHY Strobe register address
 * offset : LVDS PHY register offset
 * value : the value you want
 */
static void LVDS_PHY_StrobeWrite(
	uint32_t phy_reg, uint32_t offset, uint32_t value)
{
	uint32_t time_out = 0u;
	uint64_t phy_offset = (u64)phy_reg + offset;
	uint64_t strobe_done_offset = (u64)phy_reg + LVDS_AUTO_STB_DONE;

	if((phy_offset < UINT_MAX) && (strobe_done_offset < UINT_MAX)) {
		LVDS_writel(value, (u32)phy_offset);
		while (time_out < 10u) {
			time_out++;
			if (LVDS_readl((u32)strobe_done_offset) == 0x1u) {
				break;
			}
			mdelay(1);
		}

		if(time_out >= 10u) {
			LVDS_DBG("%s time out\n", __func__);
		}
	}
}

void LVDS_PHY_VsSet(uint32_t p_port, uint32_t s_port, uint32_t vs)
{
	uint32_t p_reg = LVDS_PHY_GetAddress(p_port);
	uint32_t s_reg = LVDS_PHY_GetAddress(s_port);

	LVDS_PHY_StrobeWrite(p_reg, 0x3A0u,	0x00000094u | ((vs & 0x3u) << 5)); // STB_PLL ADDR 0x8
	if ((bool)s_reg) {
		LVDS_PHY_StrobeWrite(s_reg, 0x3A0u, 0x00000094u | ((vs & 0x3u) << 5)); // STB_PLL ADDR 0x8
	}
}

/* LVDS_PHY_Config
 * Setup LVDS PHY Strobe. (Alphachips Guide Value Only)
 * p_port : the primary port number
 * s_port : the secondary port number
 * upsample_ratio : division ratio (0: fcon automatic, 0~4: fcon manual)
 * step : the step of lvds phy configure
 * vcm : typical vcm level in mV
 * vsw : typical vsw level in mV
 */
void LVDS_PHY_StrobeConfig(
	uint32_t p_port, uint32_t s_port, uint32_t upsample_ratio,
	uint32_t step, uint32_t vcm, uint32_t vsw)
{
	uint32_t p_reg = LVDS_PHY_GetAddress(p_port);
	uint32_t s_reg = LVDS_PHY_GetAddress(s_port);
	uint32_t vcmcal, swingcal;
	uint32_t value;
	uint32_t cpzs_main = 0x0000001Fu;
	uint32_t cpzs_sub = 0x0000001Fu;
	uint32_t strobe_LFR1S = 0x08u;
	uint32_t strobe_LFC1S = 0x18u;
	uint32_t strobe_LFC2S = 0x25u;
	uint32_t vs = 0u;

	if ((bool)p_reg) {
		switch (step) {
		case LVDS_PHY_INIT:
			LVDS_DBG("S1_LPF2\n");
			LVDS_PHY_GetCalibrationLevel(vcm, vsw, &vcmcal, &swingcal);
			value = ((swingcal & 0xFu) | ((vcmcal & 0x7u) << 4));

			LVDS_PHY_StrobeWrite(p_reg, 0x380u, 0x000000FFu);
			LVDS_PHY_StrobeWrite(p_reg, 0x384u, 0x0000001Fu);
			LVDS_PHY_StrobeWrite(p_reg, 0x388u, 0x00000032u);
			LVDS_PHY_StrobeWrite(p_reg, 0x38Cu, cpzs_main);
			LVDS_PHY_StrobeWrite(p_reg, 0x390u, 0x00000000u);
			LVDS_PHY_StrobeWrite(p_reg, 0x394u, strobe_LFR1S);
			LVDS_PHY_StrobeWrite(p_reg, 0x398u, strobe_LFC1S);
			LVDS_PHY_StrobeWrite(p_reg, 0x39Cu, strobe_LFC2S);
			LVDS_PHY_StrobeWrite(p_reg, 0x3A0u, 0x00000094u | ((vs & 0x3u) << 5));
			LVDS_PHY_StrobeWrite(p_reg, 0x3A4u, 0x00000000u);
			LVDS_PHY_StrobeWrite(p_reg, 0x3A8u, 0x0000000Cu);
			LVDS_PHY_StrobeWrite(p_reg, 0x3ACu, 0x00000000u);
			LVDS_PHY_StrobeWrite(p_reg, 0x3B0u, 0x00000000u);
			if ((bool)s_reg) {
				LVDS_PHY_StrobeWrite(p_reg, 0x3B4u, 0x00000007u);
			}
			else {
				LVDS_PHY_StrobeWrite(p_reg, 0x3B4u, 0x00000001u);
			}
			LVDS_PHY_StrobeWrite(p_reg, 0x3B8u, 0x00000001u);

			if ((bool)s_reg) {
				LVDS_PHY_StrobeWrite(s_reg, 0x380u, 0x000000FFu);
				LVDS_PHY_StrobeWrite(s_reg, 0x384u, 0x0000001Fu);
				LVDS_PHY_StrobeWrite(s_reg, 0x388u, 0x00000032u);
				LVDS_PHY_StrobeWrite(s_reg, 0x38Cu, cpzs_sub);
				LVDS_PHY_StrobeWrite(s_reg, 0x390u, 0x00000000u);
				LVDS_PHY_StrobeWrite(s_reg, 0x394u, strobe_LFR1S);
				LVDS_PHY_StrobeWrite(s_reg, 0x398u, strobe_LFC1S);
				LVDS_PHY_StrobeWrite(s_reg, 0x39Cu, strobe_LFC2S);
				LVDS_PHY_StrobeWrite(s_reg, 0x3A0u, 0x00000094u | ((vs & 0x3u) << 5));
				LVDS_PHY_StrobeWrite(s_reg, 0x3A4u, 0x00000000u);
				LVDS_PHY_StrobeWrite(s_reg, 0x3A8u, 0x0000000Cu);
				LVDS_PHY_StrobeWrite(s_reg, 0x3ACu, 0x00000000u);
				LVDS_PHY_StrobeWrite(s_reg, 0x3B0u, 0x00000000u);
				LVDS_PHY_StrobeWrite(s_reg, 0x3B4u, 0x0000000Cu);
				LVDS_PHY_StrobeWrite(s_reg, 0x3B8u, 0x00000001u);
			}

			LVDS_PHY_StrobeWrite(p_reg, 0x204u, value);
			LVDS_PHY_StrobeWrite(p_reg, 0x244u, value);
			LVDS_PHY_StrobeWrite(p_reg, 0x284u, value);
			LVDS_PHY_StrobeWrite(p_reg, 0x2C4u, value);
			LVDS_PHY_StrobeWrite(p_reg, 0x304u, value);
			if ((bool)s_reg) {
				LVDS_PHY_StrobeWrite(s_reg, 0x204u, value);
				LVDS_PHY_StrobeWrite(s_reg, 0x244u, value);
				LVDS_PHY_StrobeWrite(s_reg, 0x284u, value);
				LVDS_PHY_StrobeWrite(s_reg, 0x2C4u, value);
				LVDS_PHY_StrobeWrite(s_reg, 0x304u, value);
			}
			break;
		case LVDS_PHY_READY:
			value = (upsample_ratio & 0x7u);
			LVDS_PHY_StrobeWrite(p_reg, 0x380u, 0x00000000u);
			LVDS_PHY_StrobeWrite(p_reg, 0x380u, 0x000000FFu);
			LVDS_PHY_StrobeWrite(p_reg, 0x384u, 0x0000001Fu);
			LVDS_PHY_StrobeWrite(p_reg, 0x3A4u, value);
			LVDS_PHY_StrobeWrite(p_reg, 0x3ACu, value);
			if ((bool)s_reg) {
				LVDS_PHY_StrobeWrite(s_reg, 0x380u, 0x00000000u);
				LVDS_PHY_StrobeWrite(s_reg, 0x380u, 0x000000FFu);
				LVDS_PHY_StrobeWrite(s_reg, 0x384u, 0x0000001Fu);
				LVDS_PHY_StrobeWrite(s_reg, 0x3A4u, value);
				LVDS_PHY_StrobeWrite(s_reg, 0x3ACu, value);
			}
			break;
		case LVDS_PHY_START:
			if ((bool)s_reg) {
				LVDS_PHY_StrobeWrite(s_reg, 0x3B8u, 0x00000001u);
				LVDS_PHY_StrobeWrite(s_reg, 0x3B4u, 0x0000003Cu);

				LVDS_PHY_StrobeWrite(p_reg, 0x3B8u, 0x00000001u);
				// PHY Start up  OFF
				LVDS_PHY_StrobeWrite(p_reg, 0x3B4u, 0x000000C7u);
				// PHY Start up ON
				LVDS_PHY_StrobeWrite(p_reg, 0x3B4u, 0x000000D7u);
			}
			else {
				value = (LVDS_PHY_GetRegValue(p_port, 0x3B4u) & ~(0x000000F0u));
				LVDS_PHY_StrobeWrite(p_reg, 0x3B4u, value);
				LVDS_PHY_StrobeWrite(p_reg, 0x3B8u, 0x00000000u);
				LVDS_PHY_StrobeWrite(p_reg, 0x3B8u, 0x00000001u);
				value =	(LVDS_PHY_GetRegValue(p_port, 0x3B4u) & ~(0x000000F0u));
				value |= (0x1u << 4);
				LVDS_PHY_StrobeWrite(p_reg, 0x3B4u, value);
			}

			LVDS_PHY_StrobeWrite(p_reg, 0x200u, 0x00000079u);
			LVDS_PHY_StrobeWrite(p_reg, 0x240u, 0x00000079u);
			LVDS_PHY_StrobeWrite(p_reg, 0x280u, 0x00000079u);
			LVDS_PHY_StrobeWrite(p_reg, 0x2C0u, 0x00000079u);
			LVDS_PHY_StrobeWrite(p_reg, 0x300u, 0x00000079u);
			if ((bool)s_reg) {
				LVDS_PHY_StrobeWrite(s_reg, 0x200u, 0x00000079u);
				LVDS_PHY_StrobeWrite(s_reg, 0x240u, 0x00000079u);
				LVDS_PHY_StrobeWrite(s_reg, 0x280u, 0x00000079u);
				LVDS_PHY_StrobeWrite(s_reg, 0x2C0u, 0x00000079u);
				LVDS_PHY_StrobeWrite(s_reg, 0x300u, 0x00000079u);
			}
			break;
//		case LVDS_PHY_CONFIG_MAX:
		default:
			LVDS_DBG("%s in error, invaild parameter(step: %u)\n",__func__, step);
			break;
		}
	}
}

/* LVDS_PHY_CheckStatus
 * Check the status of lvds phy
 * p_port : the primary port number of lvds phy
 * s_port : the secondary port number of lvds phy
 */
uint32_t LVDS_PHY_CheckStatus(uint32_t p_port, uint32_t s_port)
{
	uint32_t p_reg = LVDS_PHY_GetAddress(p_port);
	uint32_t s_reg = LVDS_PHY_GetAddress(s_port);
	uint32_t ret = 0u, value;

	if ((bool)p_reg) {
		value = (LVDS_readl(p_reg + LVDS_MONITOR_DEBUG1) & (LVDS_MONITOR_DEBUG1_PLL_STATUS_MASK));
		if ((bool)(value & (LVDS_MONITOR_DEBUG1_LKVDETLOW_MASK | LVDS_MONITOR_DEBUG1_LKVDETHIGH_MASK))) {
			ret |= 0x1u;  /* primary port status b[0]*/
		}
	}

	if ((bool)s_reg) {
		value = (LVDS_readl(s_reg + LVDS_MONITOR_DEBUG1) & (LVDS_MONITOR_DEBUG1_PLL_STATUS_MASK));
		if ((bool)(value & (LVDS_MONITOR_DEBUG1_LKVDETLOW_MASK | LVDS_MONITOR_DEBUG1_LKVDETHIGH_MASK))) {
			ret |= 0x2u;  /* secondary port status b[1] */
		}
	}
	return ret;
}

/* LVDS_PHY_GetRegValue
 * Read the register corresponding to 'offset'
 * port : the port number of lvds phy
 * offset : the register offset
 */
uint32_t LVDS_PHY_GetRegValue(uint32_t port, uint32_t offset)
{
	uint32_t phy_reg = LVDS_PHY_GetAddress(port);
	uint32_t ret = 0u;
	uint32_t time_out = 0u;
	uint64_t phy_offset = 0u;

	if ((bool)phy_reg) {
		phy_offset = (u64)phy_reg + offset;
		if(phy_offset < UINT_MAX) {
			ret = LVDS_readl((u32)phy_offset);
			if (offset > 0x1FFu) { /* Write Only Registers */
				while (time_out < 10u) {
					time_out++;
					if ((bool)(LVDS_readl(phy_reg + LVDS_AUTO_STB_DONE) & 0x1u)) {
						break;
					}
					mdelay(1);
				}
			}
			if(time_out >= 10u) {
				LVDS_DBG("%s time out\n", __func__);
				ret = 0u;
			}
			else {
				ret = LVDS_readl(phy_reg + LVDS_AUTO_STB_RDATA);
			}
		}
	}

	return ret;
}
/* end of file */
