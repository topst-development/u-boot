// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <config.h>
#include <common.h>
#include <lcd.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <telechips/fb_dm.h>
#include <telechips/lvds_ctrl.h>

#include <asm/io.h>
#include <mach/chipinfo.h>

#include <dm.h>
#include <display.h>

#ifdef CONFIG_R5_LVDS_CTRL
struct mbox_chan mbox_ch;

static int lvds_wait_ack(struct tcc_mbox_data *msg, unsigned int *cnt);
static void lvds_wrap_r5_ctrl(
	unsigned int lvds_type, unsigned int val, unsigned int select,
	unsigned int wrap_cmd);
static void lvds_phy_r5_ctrl(
	unsigned int lvds_type, unsigned int port_main, unsigned int clk_freq,
	unsigned int vcm, unsigned int vsw, unsigned int phy_cmd);
#endif
#define ABS_DIFF(a, b) (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))

/* coverity[HIS_metric_violation : FALSE] */
struct lvds_hw_info_t *lvds_register_hw_info(
	struct lvds_hw_info_t *l_hw, unsigned int l_type, unsigned int port1,
	unsigned int port2, unsigned long p_clk, unsigned int lcdc_select,
	unsigned int lcdc_bypass, unsigned int xres)
{
	struct lvds_hw_info_t *lvds_ptr;

	if (((l_hw != NULL) &&
	    ((l_type == (u32)PANEL_LVDS_DUAL) ||
	     (l_type == (u32)PANEL_LVDS_SINGLE)))) {
		l_hw->lvds_type = l_type;
		l_hw->port_main = port1;
		l_hw->port_sub = port2;
		l_hw->p_clk = p_clk;
		l_hw->lcdc_mux_id = lcdc_select;
		l_hw->lcdc_mux_bypass = lcdc_bypass;
		l_hw->xres = xres;
		if (l_type == (u32)PANEL_LVDS_SINGLE) {
			l_hw->ts_mux_id = (l_hw->port_main + 2U)
				% 4U;	  // lvds 2,3,0 map to ts_mux 0,1,2
		} else {		      // dual
			l_hw->ts_mux_id = 0x3U; // not used
		}
		lvds_ptr = l_hw;
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("%s :\n lvds_type = %u\n port_main = %u\n port_sub = %u\n p_clk = %lu\n lcdc_mux_id = %u\n lcdc_mux_bypass = %u\n xres = %u\n ts_mux_id = %u\n",
			__func__, l_hw->lvds_type, l_hw->port_main, l_hw->port_sub,
			l_hw->p_clk, l_hw->lcdc_mux_id, l_hw->lcdc_mux_bypass,
			l_hw->xres, l_hw->ts_mux_id);
	} else {
		lvds_ptr = NULL;
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("%s : invalid lvds_ptr.\n", __func__);
	}

	return lvds_ptr;
}

void lvds_splitter_init(struct lvds_hw_info_t *lvds_hw)
{
#ifdef CONFIG_R5_LVDS_CTRL
	unsigned int val = (lvds_hw->lvds_type == PANEL_LVDS_DUAL) ?
		lvds_hw->xres :
		lvds_hw->ts_mux_id;
	unsigned int boot_mode = __raw_readl(0x14400014) & 0xf;

	if (boot_mode == SNOR_3B || boot_mode == SNOR_4B) {
		lvds_wrap_r5_ctrl(
			lvds_hw->lvds_type, val, lvds_hw->lcdc_mux_id,
			LVDS_WRAP_CMD_INIT);
	} else {
		lvds_wrap_core_init(
			lvds_hw->lvds_type, lvds_hw->xres, lvds_hw->ts_mux_id,
			lvds_hw->lcdc_mux_id, lvds_hw->lcdc_mux_bypass,
			lvds_hw->txout_main, lvds_hw->txout_sub);
	}
#else
	lvds_wrap_core_init(
		lvds_hw->lvds_type, lvds_hw->xres, lvds_hw->ts_mux_id,
		lvds_hw->lcdc_mux_id, lvds_hw->lcdc_mux_bypass,
		lvds_hw->txout_main, lvds_hw->txout_sub);
#endif
}

void lvds_phy_init(struct lvds_hw_info_t *lvds_hw)
{
	unsigned int ref_clk;

	if (lvds_hw->ts_mux_id == 0x3U) { // lvds dual mode
		ref_clk = lvds_hw->p_clk * 2U;
	} else { // lvds single mode
		ref_clk = lvds_hw->p_clk;
	}
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("[%s] ref_clk for LVDS PHY = %u\n", __func__, ref_clk);

	unsigned int upsample_ratio = LVDS_PHY_GetUpsampleRatio(
		lvds_hw->port_main, lvds_hw->port_sub, ref_clk);
	unsigned int ref_cnt = LVDS_PHY_GetRefCnt(
		lvds_hw->port_main, lvds_hw->port_sub, ref_clk, upsample_ratio);

#ifdef CONFIG_R5_LVDS_CTRL
	unsigned int boot_mode = __raw_readl(0x14400014) & 0xf;

	if (boot_mode == SNOR_3B || boot_mode == SNOR_4B) {
		lvds_phy_r5_ctrl(
			lvds_hw->lvds_type, lvds_hw->port_main, ref_clk,
			lvds_hw->vcm, lvds_hw->vsw, LVDS_PHY_CMD_INIT);
	} else {
		lvds_phy_core_init(
			lvds_hw->lvds_type, lvds_hw->port_main,
			lvds_hw->port_sub, upsample_ratio, ref_cnt,
			lvds_hw->vcm, lvds_hw->vsw, lvds_hw->lane_main,
			lvds_hw->lane_sub);
	}
#else
	lvds_phy_core_init(
		lvds_hw->lvds_type, lvds_hw->port_main, lvds_hw->port_sub,
		upsample_ratio, ref_cnt, lvds_hw->vcm, lvds_hw->vsw,
		lvds_hw->lane_main, lvds_hw->lane_sub);
	return;
#endif
}

void lvds_wrap_core_init(
	unsigned int lvds_type, unsigned int width, unsigned int tx_mux_id,
	unsigned int lcdc_mux_id, unsigned int lcdc_bypass,
	unsigned int (*sel0)[TXOUT_DATA_PER_LINE],
	unsigned int (*sel1)[TXOUT_DATA_PER_LINE])
{
	LVDS_WRAP_SetAccessCode();
	if (lvds_type == (u32)PANEL_LVDS_DUAL) {
		unsigned int idx;

		LVDS_WRAP_SetConfigure(0, 0, width);
		for (idx = 0U; idx < (u32)TS_SWAP_CH_MAX; idx++) {
			LVDS_WRAP_SetDataSwap(idx, idx);
		}

		LVDS_WRAP_SetMuxOutput(DISP_MUX_TYPE, 0, lcdc_mux_id, 1);
#if defined(CONFIG_TCC805X)
		if (lcdc_bypass != 0U) {
			LVDS_WRAP_SM_Bypass(lcdc_mux_id, lcdc_bypass);
		}
#endif
		LVDS_WRAP_SetMuxOutput(
			TS_MUX_TYPE, TS_MUX_IDX0, TS_MUX_PATH_CORE, 1);
		LVDS_WRAP_SetDataArray(TS_MUX_IDX0, sel0);
		LVDS_WRAP_SetMuxOutput(
			TS_MUX_TYPE, TS_MUX_IDX1, TS_MUX_PATH_CORE, 1);
		LVDS_WRAP_SetDataArray(TS_MUX_IDX1, sel1);
	} else if (lvds_type == (u32)PANEL_LVDS_SINGLE) {
#if defined(CONFIG_DC_DISP_LVDS_LCD_PORT1) \
	&& !defined(CONFIG_DC_DISP_LVDS_LCD_PORT0)
		LVDS_WRAP_SetMuxOutput(TS_MUX_TYPE, 0, lcdc_mux_id, 1);
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("%s : Single LVDS use LVDS port1 only\n", __func__);
#endif
		LVDS_WRAP_SetMuxOutput(TS_MUX_TYPE, tx_mux_id, lcdc_mux_id, 1);
#if defined(CONFIG_TCC805X)
		if (lcdc_bypass != 0U) {
			LVDS_WRAP_SM_Bypass(lcdc_mux_id, lcdc_bypass);
		}
#endif
		LVDS_WRAP_SetDataArray(tx_mux_id, sel0);
	} else {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("%s : unknown lvds type. lvds wrap not initialized.\n",
		       __func__);
	}
}

/* coverity[HIS_metric_violation : FALSE] */
void lvds_phy_core_init(
	unsigned int lvds_type, unsigned int lvds_main, unsigned int lvds_sub,
	unsigned int upsample_ratio, unsigned int ref_cnt, unsigned int vcm,
	unsigned int vsw, unsigned int *LVDS_LANE_MAIN,
	unsigned int *LVDS_LANE_SUB)
{
	unsigned int status;
	unsigned int mfcon = 0; // main fcon
	unsigned int sfcon = 0; // sub fcon
	unsigned int pre_mfcon = 0;
	unsigned int pre_sfcon = 0;
	unsigned int fcon_threshold = 2;
	int init_flag = 1;

	unsigned int s_port_en = 0U;

	if ((lvds_type != (u32)PANEL_LVDS_DUAL) &&
			 (lvds_type != (u32)PANEL_LVDS_SINGLE)) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("%s : unknown lvds type. lvds phy not initialized.\n",
		       __func__);
		init_flag = 0;
	} else if (lvds_type == (u32)PANEL_LVDS_DUAL) {
		s_port_en = 1U;
	} else { //Single LVDS
		s_port_en = 0U;
	}

	if (init_flag != 0) {
		LVDS_PHY_ClockEnable(lvds_main, 1);
		if (s_port_en != 0U) {
			LVDS_PHY_ClockEnable(lvds_sub, 1);
		}

		LVDS_PHY_SWReset(lvds_main, 1);
		if (s_port_en != 0U) {
			LVDS_PHY_SWReset(lvds_sub, 1);
		}

		udelay(1000); // Alphachips Guide

		LVDS_PHY_SWReset(lvds_main, 0);
		if (s_port_en != 0U) {
			LVDS_PHY_SWReset(lvds_sub, 0);
		}

		/* LVDS PHY Strobe setup */
		LVDS_PHY_SetStrobe(lvds_main, 1, 1);
		if (s_port_en != 0U) {
			LVDS_PHY_SetStrobe(lvds_sub, 1, 1);
		}

		LVDS_PHY_StrobeConfig(
				      lvds_main, lvds_sub, upsample_ratio, LVDS_PHY_INIT, vcm, vsw);

		LVDS_PHY_LaneEnable(lvds_main, 0);
		if (s_port_en != 0U) {
			LVDS_PHY_LaneEnable(lvds_sub, 0);
		}

		LVDS_PHY_SetPortOption(lvds_main, 0, 0, 0, 0x0, 0x0);
		if (s_port_en != 0U) {
			LVDS_PHY_SetPortOption(lvds_sub, 1, 0, 1, 0x0, 0x7);
		}

		LVDS_PHY_LaneSwap(
				  s_port_en, lvds_main, lvds_sub, LVDS_LANE_MAIN, LVDS_LANE_SUB);

		LVDS_PHY_StrobeConfig(
				      lvds_main, lvds_sub, upsample_ratio, LVDS_PHY_READY, vcm, vsw);

		LVDS_PHY_SetFcon(
				 lvds_main, LVDS_PHY_FCON_AUTOMATIC, 0, 0,
				 ref_cnt); // fcon value, for 44.1Mhz
		if (s_port_en != 0U) {
			LVDS_PHY_SetFcon(
					 lvds_sub, LVDS_PHY_FCON_AUTOMATIC, 0, 0,
					 ref_cnt); // fcon value, for 44.1Mhz
		}

		LVDS_PHY_FConEnable(lvds_main, 1);
		if (s_port_en != 0U) {
			LVDS_PHY_FConEnable(lvds_sub, 1);
		}

		// dummy startup clk2a enable, Is it needed for single LVDS?
		if (s_port_en != 0U) {
			LVDS_PHY_StrobeConfig(
					      lvds_main, lvds_sub, upsample_ratio, LVDS_PHY_START,
					      vcm, vsw);
		}

		LVDS_PHY_SetCFcon(lvds_main, LVDS_PHY_FCON_AUTOMATIC, 1);
		if (s_port_en != 0U) {
			LVDS_PHY_SetCFcon(lvds_sub, LVDS_PHY_FCON_AUTOMATIC, 1);
		}

		mfcon = LVDS_PHY_Fcon_Value(lvds_main);
		if (s_port_en != 0U) {
			sfcon = LVDS_PHY_Fcon_Value(lvds_sub);
		}

		(void)LVDS_PHY_CheckFcon(lvds_main, lvds_sub, mfcon, sfcon);

		LVDS_PHY_StrobeConfig(
				      lvds_main, lvds_sub, upsample_ratio, LVDS_PHY_START, vcm, vsw);

		if (s_port_en != 0U) {
			mfcon = LVDS_PHY_Fcon_Value(lvds_main);
			sfcon = LVDS_PHY_Fcon_Value(lvds_sub);
			(void)LVDS_PHY_CheckFcon(lvds_main, lvds_sub, mfcon, sfcon);
			// save mfcon & sfcon after PLL is considered as locked.
			pre_mfcon = LVDS_PHY_Fcon_Value(lvds_main);
			pre_sfcon = LVDS_PHY_Fcon_Value(lvds_sub);
		}

		/* LVDS PHY digital setup */
		LVDS_PHY_SetFormat(lvds_main, 0, 1, 0, upsample_ratio);
		if (s_port_en != 0U) {
			LVDS_PHY_SetFormat(lvds_sub, 0, 1, 0, upsample_ratio);
		}

		LVDS_PHY_SetFifoEnableTiming(lvds_main, 0x3);
		if (s_port_en != 0U) {
			LVDS_PHY_SetFifoEnableTiming(lvds_sub, 0x3);
		}

		/* LVDS PHY Main/Sub Lane Disable */
		LVDS_PHY_LaneEnable(lvds_main, 0);
		if (s_port_en != 0U) {
			LVDS_PHY_LaneEnable(lvds_sub, 0);
		}

		/* LVDS PHY Main port FIFO Disable */
		LVDS_PHY_FifoEnable(lvds_main, 0);
		if (s_port_en != 0U) {
			LVDS_PHY_FifoEnable(lvds_sub, 0);
		}

		LVDS_PHY_FifoReset(lvds_main, 1);
		if (s_port_en != 0U) {
			LVDS_PHY_FifoReset(lvds_sub, 1);
		}

		udelay(1000); // Alphachips Guide

		LVDS_PHY_FifoReset(lvds_main, 0);
		if (s_port_en != 0U) {
			LVDS_PHY_FifoReset(lvds_sub, 0);
		}

		/* LVDS PHY Main/Sub port FIFO Enable */
		LVDS_PHY_FifoEnable(lvds_main, 1);
		if (s_port_en != 0U) {
			LVDS_PHY_FifoEnable(lvds_sub, 1);
		}

		/* LVDS PHY Main/Sub port Lane Enable */
		LVDS_PHY_LaneEnable(lvds_main, 1);
		if (s_port_en != 0U) {
			LVDS_PHY_LaneEnable(lvds_sub, 1);
		}

		if (s_port_en != 0U) {
			/* LVDS PHY Main/Sub port Lane Enable(to apply new power on
			 * sequence)
			 */
			mfcon = LVDS_PHY_Fcon_Value(lvds_main);
			sfcon = LVDS_PHY_Fcon_Value(lvds_sub);
			if ((ABS_DIFF(pre_mfcon, mfcon) > fcon_threshold)
			    || (ABS_DIFF(pre_sfcon, sfcon) > fcon_threshold)) {
				/* LVDS PHY Main/Sub port FIFO Disable & Reset*/
				LVDS_PHY_FifoEnable(lvds_main, 0);
				LVDS_PHY_FifoEnable(lvds_sub, 0);

				LVDS_PHY_FifoReset(lvds_main, 1);
				LVDS_PHY_FifoReset(lvds_sub, 1);

				LVDS_PHY_StrobeConfig(
						      lvds_main, lvds_sub, upsample_ratio,
						      LVDS_PHY_START, vcm, vsw);

				udelay(1000); // Alphachips Guide
				LVDS_PHY_FifoReset(lvds_main, 0);
				LVDS_PHY_FifoReset(lvds_sub, 0);

				/* LVDS PHY Main/Sub port FIFO Enable */
				LVDS_PHY_FifoEnable(lvds_main, 1);
				LVDS_PHY_FifoEnable(lvds_sub, 1);
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
				pr_info("%s : [LVDS RESET] LVDS PHY mfcon : %u, pre_mfcon = %u, sfcon = %u, pre_sfcon = %u, fcon_threshold = %u\n"
					, __func__, mfcon, pre_mfcon, sfcon, pre_sfcon,
					fcon_threshold);
			} else {
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
				pr_info("%s : [LVDS OK] LVDS PHY mfcon : %u, pre_mfcon = %u, sfcon = %u, pre_sfcon = %u, fcon_threshold = %u\n"
					, __func__, mfcon, pre_mfcon, sfcon, pre_sfcon,
					fcon_threshold);
			}
		}
		// Restore VS to 2
		LVDS_PHY_VsSet(lvds_main, lvds_sub, 2);

		status = LVDS_PHY_CheckStatus(lvds_main, lvds_sub);
		if ((status & 0x1U) == 0U) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err("%s: LVDS_PHY Primary port(%u) is in death [error]\n",
			       __func__, lvds_main);
		} else {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
			pr_info("%s: LVDS_PHY Primary port(%u) is alive\n", __func__,
				lvds_main);
		}
		if (s_port_en != 0U) {
			if ((status & 0x2U) == 0U) {
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				pr_err("%s: LVDS_PHY Secondary port(%u) is in death [error]\n",
				       __func__, lvds_sub);
			} else {
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
				pr_info("%s: LVDS_PHY Secondary port(%u) is alive\n",
					__func__, lvds_sub);
			}
		}
	}
}

#ifdef CONFIG_R5_LVDS_CTRL

static void lvds_wrap_r5_ctrl(
	unsigned int lvds_type, unsigned int val, unsigned int select,
	unsigned int wrap_cmd)
{
	struct tcc_mbox_data mbox_msg;
	unsigned int delay_cnt = 0;
	unsigned int subChannel;
	int ret;

	/*MBOX_SYSTEM_LEVEL << 16 | MBOX_SYSTEM_ID_LVDS */
	subChannel = (1 << 16) | 1;
	mbox_msg.cmd[0] = subChannel;
	mbox_msg.cmd[1] = LVDS_TYPE_WRAP; // 0: lvds_phy, 1: lvds_wrap
	if (lvds_type == (u32)PANEL_LVDS_SINGLE)
		mbox_msg.cmd[2] = 0; // single
	else if (lvds_type == (u32)PANEL_LVDS_DUAL)
		mbox_msg.cmd[2] = 1; // dual
	else {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("%s : unknown lvds type. lvds r5 wrap not initialized.\n",
		       __func__);
		return;

	}
	mbox_msg.cmd[3] = val;    // dual : width , single : tx_mux_id
	mbox_msg.cmd[4] = select; // lcd mux id:
	mbox_msg.cmd[5] = 0;
	mbox_msg.cmd[6] = wrap_cmd;
	mbox_msg.data_len = 0;

	ret = mbox_send(&mbox_ch, &mbox_msg);

	if (ret != 0) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("%s : mbox send error : %d\n", __func__, ret);
	} else {
		ret = lvds_wait_ack(&mbox_msg, &delay_cnt);
	}

	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("%s : mbox send message [%s], delay cnt = %d\n", __func__,
		ret ? "error" : "ok", delay_cnt);
}

static void lvds_phy_r5_ctrl(
	unsigned int lvds_type, unsigned int port_main, unsigned int clk_freq,
	unsigned int vcm, unsigned int vsw, unsigned int phy_cmd)
{
	struct tcc_mbox_data mbox_msg;
	unsigned int delay_cnt = 0;
	unsigned int subChannel;
	int ret;

	/*MBOX_SYSTEM_LEVEL << 16 | MBOX_SYSTEM_ID_LVDS */
	subChannel = (1 << 16) | 1;
	mbox_msg.cmd[0] = subChannel;    // 0: lvds_phy, 1: lvds_wrap
	mbox_msg.cmd[1] = LVDS_TYPE_PHY; // 0: lvds_phy, 1: lvds_wrap
	if (lvds_type == (u32)PANEL_LVDS_SINGLE) {
		mbox_msg.cmd[2] = 0; // single
	} else if (lvds_type == PANEL_LVDS_DUAL) {
		mbox_msg.cmd[2] = 1; // dual
	} else {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("%s : unknown lvds type. lvds r5 phy not initialized.\n",
		       __func__);
		return;
	}
	mbox_msg.cmd[3] = port_main; // used only for single LVDS
	mbox_msg.cmd[4] = clk_freq;
	mbox_msg.cmd[5] = ((vcm & 0xfff) << 16) | (vsw & 0xfff);
	mbox_msg.cmd[6] =
		phy_cmd; // cmd 0 : off , 1 : on , 2 : reset , 3 check status
	mbox_msg.data_len = 0;

	// ret = tcc_mbox_send_message(MBOX_SYSTEM_LEVEL, MBOX_SYSTEM_ID_LVDS,
	// &mbox_msg);
	ret = mbox_send(&mbox_ch, &mbox_msg);
	if (ret != 0) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s] : mbox send error : %d\n", __func__, ret);
	} else {
		ret = lvds_wait_ack(&mbox_msg, &delay_cnt);
	}

	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("%s : mbox send message [%s], delay cnt = %d\n", __func__,
		ret ? "error" : "ok", delay_cnt);
}

static int lvds_wait_ack(struct tcc_mbox_data *msg, unsigned int *cnt)
{
	struct tcc_mbox_data rx_mbox_msg;
	int cmd_idx, delay_cnt = 0;
	int ret;
	int mbox_status = 0;

	if ((msg->cmd[1] == LVDS_TYPE_WRAP) || (msg->cmd[1] == LVDS_TYPE_PHY)) {
		ret = ACK_RET_WAIT;
	} else {
		pr_err("%s : unvalid command\n", __func__);
		ret = ACK_RET_CMD_INVAL;
	}

	if (ret == ACK_RET_WAIT) {
		for (delay_cnt = 0; delay_cnt < MBOX_POLL_WAIT_TIMEOUT;
		     delay_cnt++) {
			unsigned int rx_mbox_level;
			unsigned int rx_app_id;

			mbox_status = mbox_recv(&mbox_ch, &rx_mbox_msg, 50000);
			rx_app_id = rx_mbox_msg.cmd[0] & 0x1;
			rx_mbox_level = rx_mbox_msg.cmd[0] >> 16;
			if (mbox_status == TCC_MBOX_SUCCESS) {
				if ((rx_mbox_level == 1) && (rx_app_id == 1)) {
					for (cmd_idx = 1; cmd_idx <= 6;
					     cmd_idx++) {
						if (msg->cmd[cmd_idx]
						    != rx_mbox_msg.cmd[cmd_idx])
							break;
						else if (cmd_idx == 6)
							ret = ACK_RET_FIN;
					}
					if (ret == ACK_RET_FIN) {
						break;
					}
				}
			}
			mdelay(1);
		}
	}

	*cnt = delay_cnt;
	if (ret == ACK_RET_WAIT) { /* if still waiting ack after time out */
		ret = ACK_RET_TIME_OUT;
	} else if (ret == ACK_RET_SKIP) {
		ret = ACK_RET_FIN;
	}
	return ret;
}

#endif

#ifdef CONFIG_R5_LVDS_CTRL
void tcc_lvds_ctrl_register_mbox(struct mbox_chan* ch)
{
	pr_info("%s : mbox = %p\n", __func__, ch);
	mbox_ch = *ch;
}

int tcc_lvds_ctrl_is_mbox_registered(void)
{
	int ret;

	ret = (mbox_ch.dev != NULL) ? 1 : 0;
	return ret;
}
#endif
