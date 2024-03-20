// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TCC_LVDS_INIT_H__
#define __TCC_LVDS_INIT_H__

#include <lcd.h>
#include <asm/telechips/vioc/lvds_wrap.h>
#include <asm/telechips/vioc/lvds_phy.h>

#define NUM_LANE 5

#define ACK_RET_FIN 0
#define ACK_RET_SKIP 1
#define ACK_RET_WAIT 2
#define ACK_RET_CMD_INVAL -1
#define ACK_RET_TIME_OUT -2

enum {
	LVDS_WRAP_CMD_INIT = 0,
	LVDS_WRAP_CMD_RESET_PHY,
	LVDS_WRAP_CDM_MAX,
};

enum { LVDS_TYPE_WRAP, LVDS_TYPE_PHY, LVDS_TYPE_MAX };

enum {
	LVDS_PHY_CMD_INIT = 0,
	LVDS_PHY_CDM_MAX,
};

struct lvds_hw_info_t {
	unsigned int lvds_type; // dual or single port lvds
	unsigned int port_main;
	unsigned int port_sub;
	unsigned int ts_mux_id;       // ts_mux_id for single lvds
	unsigned int lcdc_mux_id;     // lcdc mux id
	unsigned int lcdc_mux_bypass; // lcdc mux bypass
	unsigned long p_clk;	   // pixel clock of LVDS
	unsigned int xres;
	unsigned int vcm;
	unsigned int vsw;
	unsigned int lane_main[LVDS_PHY_LANE_MAX];
	unsigned int lane_sub[LVDS_PHY_LANE_MAX];
	unsigned int txout_main[TXOUT_MAX_LINE][TXOUT_DATA_PER_LINE];
	unsigned int txout_sub[TXOUT_MAX_LINE][TXOUT_DATA_PER_LINE];
};

struct lvds_hw_info_t *lvds_register_hw_info(
	struct lvds_hw_info_t *l_hw, unsigned int l_type, unsigned int port1,
	unsigned int port2, unsigned long p_clk, unsigned int lcdc_select,
	unsigned int lcdc_bypass, unsigned int xres);
void lvds_splitter_init(struct lvds_hw_info_t *lvds_hw);
void lvds_phy_init(struct lvds_hw_info_t *lvds_hw);
void lvds_wrap_core_init(
	unsigned int lvds_type, unsigned int width, unsigned int tx_mux_id,
	unsigned int lcdc_mux_id, unsigned int lcdc_bypass,
	unsigned int (*sel0)[TXOUT_DATA_PER_LINE],
	unsigned int (*sel1)[TXOUT_DATA_PER_LINE]);
void lvds_phy_core_init(
	unsigned int lvds_type, unsigned int lvds_main, unsigned int lvds_sub,
	unsigned int upsample_ratio, unsigned int ref_cnt, unsigned int vcm,
	unsigned int vsw, unsigned int *LVDS_LANE_MAIN,
	unsigned int *LVDS_LANE_SUB);
#endif
