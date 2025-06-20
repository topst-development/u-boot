// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_LVDS_INIT_H
#define TCC_LVDS_INIT_H

#ifdef CONFIG_TCC897X
#include <lvds_ddi_config.h>
#else
#include <lvds_wrap.h>
#include <lvds_phy.h>
#endif
#ifdef CONFIG_TELECHIPS_LVDS_SERDES
#include <lvds_serdes.h>
#endif

#define NUM_LANE 5

#ifdef CONFIG_R5_LVDS_CTRL
#include <mailbox.h>
#include <linux/soc/telechips/tcc_mbox.h>

#define SNOR_3B 0x2
#define SNOR_4B 0x3
#define MBOX_POLL_WAIT_TIMEOUT 300 /* 300ms */
#define MBOX_SYSTEM_LEVEL 1
#define MBOX_SYSTEM_ID_LVDS 1

#define ACK_RET_FIN 0
#define ACK_RET_SKIP 1
#define ACK_RET_WAIT 2
#define ACK_RET_CMD_INVAL -1
#define ACK_RET_TIME_OUT -2
#endif

enum {
	LVDS_WRAP_CMD_INIT = 0,
	LVDS_WRAP_CMD_RESET_PHY,
	LVDS_WRAP_CMD_MAX,
};

enum { LVDS_TYPE_WRAP, LVDS_TYPE_PHY, LVDS_TYPE_MAX };

enum {
	LVDS_PHY_CMD_INIT = 0,
	LVDS_PHY_CMD_MAX,
};

enum {
	LVDS_WRAP_CORE_SPLIT_O_E = 0,
	LVDS_WRAP_CORE_SPLIT_L_R,
	LVDS_WRAP_CORE_SPLIT_MAX,
};

struct lvds_hw_info_t {
	unsigned int lvds_type; // dual or single port lvds
	unsigned int split_mode; // split mode for dual lvds
	unsigned int port_main;
	unsigned int port_sub;
	unsigned int ts_mux_select;       // ts_mux_select for single lvds
	unsigned int lcdc_mux_select;     // lcdc mux id
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

#ifdef CONFIG_TCC897X
void lvds_ddi_config_init(struct lvds_hw_info_t *lvds_hw);
#else
void lvds_splitter_init(struct lvds_hw_info_t *lvds_hw);
void lvds_phy_init(struct lvds_hw_info_t *lvds_hw);
void lvds_wrap_core_init(
	unsigned int lvds_type, unsigned int split_mode, unsigned int width,
	unsigned int tx_mux_sel, unsigned int lcdc_mux_sel, unsigned int lcdc_bypass,
	unsigned int (*sel0)[TXOUT_DATA_PER_LINE],
	unsigned int (*sel1)[TXOUT_DATA_PER_LINE]);
void lvds_phy_core_init(
	unsigned int lvds_type, unsigned int lvds_main, unsigned int lvds_sub,
	unsigned int upsample_ratio, unsigned int ref_cnt, unsigned int vcm,
	unsigned int vsw, const unsigned int *LVDS_LANE_MAIN,
	const unsigned int *LVDS_LANE_SUB);

#ifdef CONFIG_R5_LVDS_CTRL
void tcc_lvds_ctrl_register_mbox(const struct mbox_chan *ch);
int tcc_lvds_ctrl_is_mbox_registered(void);
#endif
#endif
#endif
