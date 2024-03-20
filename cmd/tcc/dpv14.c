// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/arch/gpio.h>
#include <asm/telechips/gpio.h>

#include "tcc_lcd_interface.h"
#include "Dptx_api.h"

#define NUM_OF_BYTES_REGBANK_TEST	2

#define CTS_RBR_HBR			0
#define CTS_HBR2_HBR3			1
#define CTS_EQ_SETTING_LIST		2
#define CTS_EQ_SETTING			3
#define CTS_VSW_SETTING			4
#define CTS_PREEMP_SETTING		5
#define CTS_CATEGORY_MAX		6

#define	SSC_ON		true
#define	SSC_OFF		false

#define	CLK_XIN		0x00
#define	CLK_PAD		0x08
#define	CLK_INTERNAL	0x0C


struct DPV14_CTS_Params_t {
	bool bSSC_Enabled;
	unsigned char ucClk_Selection;
	enum DPTX_LINK_RATE eLink_Rate;
	enum DPTX_PRE_EMPHASIS_LEVEL ePreEmp;
	enum DPTX_VOLTAGE_SWING_LEVEL eVSW;
	enum DPTX_TRAINING_PATTERN_TYPE ePattern_Type;
};

struct Dptx_EQ_Tuning_Params_t {
	uint32_t uiTx_EQ_Main;
	uint32_t uiTx_EQ_Post;
	uint32_t uiTx_EQ_Pre;
	uint32_t uiTx_EQ_VBoost;
};

static struct Dptx_EQ_Tuning_Params_t stDptx_EQ_Tuning_Params[] = {
	{9,  0, 0, 3},  {11, 8,  0,  3}, {13, 16, 0, 3}, {13, 24, 0, 7},
	{15, 0, 0, 3}, {18, 12, 0, 3}, {18, 20, 0, 3},
	{20, 0, 0, 3}, {20, 16, 0, 7},
	{24, 0, 0, 7},
};

static struct DPV14_CTS_Params_t stDPV14_CTS_RBR_HBR_Params_t[] = {
	/* Dummy */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_INVALID,
		DPTX_VOLTAGE_SWING_LEVEL_INVALID,
		DPTX_TRAINING_CTRL_TPS_INVALID },

	/* 1~ : Source Eye Diagram Test(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_OFF, CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_OFF, CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 5~ : Source Total Jitter Test(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 9~ : Source Non-ISI Jitter Test (TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 13~ : Source Non Pre-Emphasis Level Test(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 17~ : Source Pre-Emphasis Level Test(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 21~ : Source Non Transition Voltage Range Measurement Test(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 25~ : Source Peak to Peak Voltage Test(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 29~ : Source Inter-Pair Skew Test (Informative)(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 31~ : Source Main Link Frequency Compliance Test(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 33~ : Source (SSC) Modulation Frequency Test(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 35~ : Source (SSC) Modulation Deviation Test(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 37~ : Source(SSC) Deviation HF Variation Test (Informative)(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 39~ : Source Eye Diagram Test (TP3_EQ)(Informative) */
	{ SSC_ON, CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 41~ : Source Total Jitter Test (TP3_EQ)(Informative) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 45~ : Source AC Common Mode Test (Informative)(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 49~ : Source Intra-Pair Skew Test (Informative)(TP2) */
	{ SSC_ON,  CLK_XIN, LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
};


static struct DPV14_CTS_Params_t stDPV14_CTS_HBR23_Params_t[] = {
	/* Dummy */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_INVALID,
		DPTX_VOLTAGE_SWING_LEVEL_INVALID,
		DPTX_TRAINING_CTRL_TPS_INVALID },

	/* 1~ : Source Total Jitter Test (TP3_EQ) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_CP2520_1 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_CP2520_1 },

	/* 5~ : Source Total Jitter Test (TP2_CTLE and TP3_CTLE) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_4 },

	/* 7~ : Source Non-ISI Jitter Test (TP2) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },

	/* 8~ : Source Non-ISI Jitter Test (TP2_CTLE and TP3_CTLE) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_4 },

	/* 10~ : Source Deterministic Jitter Test (TP3_EQ) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_CP2520_1 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_CP2520_1 },

	/* 14~ : Source Random Jitter Test (TP3_EQ) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 16~ : Source Non-ISI Jitter Test (TP2) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },

	/* 17~ : Source Intra-Pair Skew */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 19~ : Source Inter-Pair Skew Test (TP2) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 21~ : Source Peak to Peak Voltage Test (TP2) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_CUSTOM80 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3
	, DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_CUSTOM80 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_CUSTOM80 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_CUSTOM80 },

	/* 25~ : Source Main Link Frequency Compliance Test (TP2) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 27~ : Source (SSC) Modulation Frequency Test (TP2) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 29~ : Source (SSC) Modulation Deviation Test (Tp2) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 31~ : Source (SSC) Deviation HF Variation Test (Tp2) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 33~ : Source Eye Diagram Test (TP3_EQ) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_CP2520_1 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_CP2520_1 },

	/* 35~ : Source Eye Diagram Test (TP3_CTLE) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_4 },

	/* 37~ : Source Eye Diagram Test (TP2_CTLE) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_4 },

	/* 39~ : Source Eye Diagram Test (TP_RX_DFE) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_4 },

	/* 41~ : Source AC Common Mode Test (TP2) */
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },
	{ SSC_ON, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_4 },

	/* 45~ : Source Level and Equalization Verification Test (TP2) */
	{ SSC_OFF, CLK_XIN, LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_CUSTOM80 },
	{ SSC_OFF, CLK_XIN, LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_CUSTOM80 },
};

//extern int drv_lcd_init( void );
//extern void tcclcd_gpio_set_value(unsigned int n, unsigned int on);

static void print_audio_port_mux_help(void)
{
	pr_force("Verifying DP audio port_mux \r\n");
	pr_force("dpv14 audio_port_mux <n> - Set cases \r\n"
		    "                n - 0x0 : Audio3 \r\n"
		    "                n - 0x1 : Audio4 \r\n"
		    "                n - 0x2 : Audio5 \r\n"
		    "                n - 0x3 : Audio6 \r\n"
		    "                n - 0x4 : Audio0 \r\n"
		    "                n - 0x5 : Audio1 \r\n"
		    "                n - 0x6 : Audio2 \r\n"
		    "                n - 0x7 : Audio7 \r\n");
}

static void print_cts_help(void)
{
	pr_force("Testing signal quality with CTS \r\n");
	pr_force("dpv14 cts <n> <m> - Set cases \r\n"
				"				n - 0  : CTS Test for RBR & HBR   \r\n"
				"					m - 1~ : Test items \r\n"
				"				n - 1  : CTS Test for HBR2 & HBR3   \r\n"
				"					m - 1~ : Test items \r\n"
				"				n - 2 : CTS Test for EQ list Setting \r\n"
				"					m - 1~ : Test items \r\n"
				"				n - 3 : CTS Test for EQ Setting   \r\n"
				"					m - 1 : EQ Main \r\n"
				"					m - 2 : EQ Post \r\n"
				"					m - 3 : EQ Pre \r\n"
				"					m - 4 : Vboost \r\n"
				"				n - 4  : CTS Test for VSW Setting   \r\n"
				"				n - 5  : CTS Test for Pre-emp Setting   \r\n"
				"				n - 6  : CTS Test for PHY Pattern Ctrl   \r\n"
				"					m - 0 : No pattern \r\n"
				"					m - 1 : LFSR7 \r\n");
}

static void print_link_help(void)
{
	pr_force("Verifying DP link \r\n");
	pr_force("dpv14 link <n><m> - Set cases \r\n"
			"		n - 0 : set Link rate \r\n"
			"				m - 0 :RBR  \r\n"
			"				m - 1 :HBR  \r\n"
			"				m - 2 :HBR2  \r\n"
			"				m - 3 :HBR3  \r\n"
			"		 n - 1 : set Link lane \r\n"
			"				m - 1 :1 lane  \r\n"
			"				m - 2 :2 lanes \r\n"
			"				m - 4 :4 lanes \r\n"
			"		 n - 2 : set Link lane \r\n"
			"				m - 0 :RGB  \r\n"
			"				m - 1 :YCBCR422 \r\n"
			"				m - 2 :YCBCR444 \r\n");
}

static void print_pll_help(void)
{
	pr_force("Verifying DP Pll \r\n");
	pr_force("dpv14 pll <n> - Set cases \r\n"
			"	n - 1 : BLOCK0[0x124C001C]~ BLOCK2[0x124C0028] 0 MHz \r\n"
			"			 => Read Register PLLPMS[0x124C0010] - Lock Status[Bit 23]  \r\n"
			"	n - 2 : BLOCK0[0x124C001C] 200MHz \r\n"
			"			BLOCK1[0x124C0020] 160MHz \r\n"
			"			BLOCK1[0x124C0024] 100MHz \r\n"
			"			BLOCK2[0x124C0028] 40MHz \r\n"
			"			 => Read Register PLLPMS[0x124C0010] - Lock Status[Bit 23] \r\n");
}

static void print_regaccess_help(void)
{
	pr_force("Verifying DP Register Bank \r\n");
	pr_force("dpv14 regbank <n> - Set cases \r\n"
			"	n - 1 : register access mode testing \r\n");
}

static void print_regbank_help(void)
{
	pr_force("Verifying DP Register Bank \r\n");
	pr_force("dpv14 regbank <n> - Set cases \r\n"
			"	n - 1 : dp_cfg_pw_ok + \r\n"
			"			dp_cfg_pw_lock(unlock) + \r\n"
			"			dp_cfg_access(unlock) \r\n"
			"		 => Write Register( DSC 0x000C ~ 0x0010 ) as 0x5A5A5A5A \r\n"
			"			-> Read Register( DSC 0x000C ~ 0x3010 ) \r\n"
			"		2 : dp_cfg_pw_ok( PW writing ) + \r\n"
			"			dp_cfg_pw_lock( unlock ) + \r\n"
			"			dp_cfg_access( lock ) \r\n"
			"		=> Write Register( DSC 0x000C ~ 0x0010 ) as 0x5555AAAA \r\n"
			"			-> Read Register( DSC 0x000C ~ 0x3010 ) \r\n"
			"	3 : dp_cfg_pw_ok( PW writing ) + \r\n"
			"		dp_cfg_pw_lock( lock ) + \r\n"
			"		dp_cfg_access( unlock ) \r\n"
			"		=> Write Register( DSC 0x000C ~ 0x0010 ) as 0xAAAA5555 \r\n"
			"			-> Read Register( DSC 0x000C ~ 0x3010 ) \r\n");
}

static void print_read_edid_help(void)
{
	pr_force("Verifying DP PHY Power on / off \r\n");
	pr_force("dpv14 power <n> - Set cases \r\n"
			"	n - 0 ~ 4 : Stream index 0 ~ 4\r\n");
}

static void print_set_panel_power_help(void)
{
	pr_force("Backlight on / off \r\n");
	pr_force("dpv14 ppwr <n> - Set cases \r\n"
		"	n - 0: Off backlight \r\n"
		"	n - 1: On backlight\r\n");
}

#if 0
static int do_dpv14_lcd_init(
						cmd_tbl_t *cmdtp,
						int flag,
						int argc,
						char * const argv[])
{
//	drv_lcd_init();

//	dpv14_test_ser_des_I2C();

	return 0;
}
#endif

static int do_dpv14_audio_port_mux(
						struct cmd_tbl_s *cmdtp,
						int flag,
						int argc,
						char * const argv[])
{
	uint32_t uiAudSel;
	unsigned int data0 = 0, data1 = 0, data2 = 0, data3 = 0;
	unsigned long	ulCases;

	cmdtp++;
	pr_info("\nflag: %d, argc: %d\n", flag, argc);

	if (argc != 2) {
		print_audio_port_mux_help();
		goto return_funcs;
	}

	if (strcmp(argv[1], "help") == 0) {
		/* For coverity */
		print_audio_port_mux_help();
	} else {
		ulCases = simple_strtoul(argv[1], NULL, 16);
		data0 = (unsigned int)(ulCases & 0x0FU);
		data1 = (unsigned int)((ulCases >> 8) & 0x0FU);
		data2 = (unsigned int)((ulCases >> 16) & 0x0FU);
		data3 = (unsigned int)((ulCases >> 24) & 0x0FU);

		if ((data0 < 8U) && (data1 < 8U) && (data2 < 8U) && (data3 < 8U)) {
			pr_force("\n[%s:%d]Audio portmux set %d\n",
						__func__,
						__LINE__,
						(u32)ulCases);

			uiAudSel = (uint32_t)(ulCases & 0xFFFFFFFFU);
			Dpv14_Tx_API_Set_Audio_Sel(uiAudSel);
		} else {
			pr_force("\n[%s:%d]Error! Invalid input! %d\n",
						__func__,
						__LINE__,
						(u32)ulCases);

			print_audio_port_mux_help();
		}
	}

return_funcs:
	return  0;
}

static int do_dpv14_set_DPLink_Params(
					struct cmd_tbl_s  *cmdtp,
					int flag,
					int argc,
					char *const argv[])
{
	int32_t ret = 0;
	unsigned long	ulCases, ulElement;
	enum DPTX_LINK_RATE eRate;
	enum DPTX_LINK_LANE eLane;
	enum DPTX_VIDEO_ENCODING_TYPE eEncodeType;

	cmdtp++;
	pr_info("\nflag: %d, argc: %d\n", flag, argc);

	if (strcmp(argv[1], "help") == 0) {
		print_link_help();
		ret = 0;
		goto return_funcs;
	} else {
		ulCases = simple_strtoul(argv[1], NULL, 10);
		ulElement = simple_strtoul(argv[2], NULL, 10);

		if (ulCases == 0U) {
			if (ulElement >= (unsigned long)LINK_RATE_MAX) {
				print_link_help();
				ret = 0;
				goto return_funcs;
			}

			pr_force("\n Set Link rate to %d", (uint32_t)ulElement);

			eRate = (enum DPTX_LINK_RATE)ulElement;
			ret = Dpv14_Tx_API_Set_MaxLinkRate_Supported(
						eRate);
			if (ret != 0) {
				/* For coverity */
				goto return_funcs;
			}

			ret = Dpv14_Tx_API_Perform_HPD_WorkingFlow();
			if (ret != 0) {
				/* For coverity */
				goto return_funcs;
			}
		} else if (ulCases == 1U) {
			if ((ulElement == 0U) ||
				(ulElement == 3U) ||
				(ulElement >= (unsigned long)DPTX_PHY_LANE_MAX)) {
				print_link_help();
				ret = 0;
				goto return_funcs;
			}

			pr_force("\n Set Link lane to %d", (uint32_t)ulElement);

			eLane = (enum DPTX_LINK_LANE)ulElement;
			ret = Dpv14_Tx_API_Set_MaxLinkLane_Supported(
						eLane);
			if (ret != 0) {
				/* For coverity */
				goto return_funcs;
			}

			ret = Dpv14_Tx_API_Perform_HPD_WorkingFlow();
			if (ret != 0) {
				/* For coverity */
				goto return_funcs;
			}
		} else if (ulCases == 2U) {
			if (ulElement >= (unsigned long)VIDEO_ENCODING_MAX) {
				print_link_help();
				ret = 0;
				goto return_funcs;
			}

			pr_force("\n Set Color space to %d", (uint32_t)ulElement);

			eEncodeType = (enum DPTX_VIDEO_ENCODING_TYPE)ulElement;
			ret = Dpv14_Tx_API_Set_Video_ColorSpace(
					eEncodeType);
			if (ret != 0) {
				/* For coverity */
				goto return_funcs;
			}

			ret = Dpv14_Tx_API_Perform_HPD_WorkingFlow();
			if (ret != 0) {
				/* For coverity */
				goto return_funcs;
			}
		} else {
			print_link_help();
			ret = 0;
			goto return_funcs;
		}
	}

return_funcs:
	return  ret;
}

static int do_dpv14_pll(
					struct cmd_tbl_s *cmdtp,
					int flag,
					int argc,
					char *const argv[])
{
	uint8_t ucPLL_Status;
	int32_t ret = 0;
	unsigned long ulCases;

	cmdtp++;
	pr_info("\nflag: %d, argc: %d\n", flag, argc);

	if (argc != 2) {
		print_pll_help();
		goto return_funcs;
	}
	if (strcmp(argv[1], "help") == 0) {
		print_pll_help();
		goto return_funcs;
	} else {
		ulCases = simple_strtoul(argv[1], NULL, 10);

		if (ulCases == 1U) {

			pr_force("\n **************[pll case 1************");

			ret = Dpv14_Tx_API_Get_PLL_Status(&ucPLL_Status);
			if (ret != 0) {
				/* For coverity */
				goto return_funcs;
			}

			if (ucPLL_Status != 0U) {
				/* For coverity */
				pr_force("\nCase %d: Result PASS\n",
						(u32)ulCases);
			} else {
				/* For coverity */
				pr_force("\nCase %d: Result FAIL\n",
						(u32)ulCases);
			}
		} else {
			/* For coverity */
			pr_force("\n[%s:%d]Error: invalid pll case (%d)\n",
						__func__,
						__LINE__,
						(u32)ulCases);
		}
		pr_force("\n ****************[End]**************\n");
	}

return_funcs:
	return ret;
}

static int do_dpv14_register_access_mode(
						struct cmd_tbl_s *cmdtp,
						int flag,
						int argc,
						char *const argv[])
{
	uint8_t ucCount;
	unsigned int auiWriteBuf[NUM_OF_BYTES_REGBANK_TEST];
	unsigned int auiReadBuf[NUM_OF_BYTES_REGBANK_TEST];
	unsigned long ulCases;

	cmdtp++;
	pr_info("\nflag: %d, argc: %d\n", flag, argc);

	if (strcmp(argv[1], "help") == 0) {
		/*For coverity*/
		print_regaccess_help();
	} else {
		ulCases = simple_strtoul(argv[1], NULL, 10);

		if (ulCases == 1U) {
			Dpv14_Tx_API_Config_RegisterAccess_Mode(true);

			for (ucCount = 0;
					ucCount < (uint8_t)NUM_OF_BYTES_REGBANK_TEST;
					ucCount++) {
				if (ucCount == 0U) {
					/*For coverity*/
					auiWriteBuf[ucCount] = 0x00005A5AU;
				} else {
					/*For coverity*/
					auiWriteBuf[ucCount] = 0x5A5A0000U;
				}

				auiReadBuf[ucCount] = 0x00U;
			}

			pr_force("\n ***********[reg access case ]***********");
			pr_force("\n 1st stage : Reg. Access mode to DDI");
			pr_force("\n Expect=> Write[0x1280000C]:0x00005A5A");
			pr_force("\n	<-> Read[0x1280000C] : 0x00005A5A");
			pr_force("\n	Write[0x12800010] : 0x5A5A0000");
			pr_force("\n	<-> Read[0x12800010] : 0x5A5A0000");

			(void)Dpv14_Tx_API_Write_RegisterBank_Value(
						auiWriteBuf,
						(uint32_t)0x000CU,
						(uint8_t)NUM_OF_BYTES_REGBANK_TEST);
			(void)Dpv14_Tx_API_Read_RegisterBank_Value(
						auiReadBuf,
						(uint32_t)0x000CU,
						(uint8_t)NUM_OF_BYTES_REGBANK_TEST);

			pr_force("\n\n Actual => Write[0x1280000C] : 0x%08X",
					auiWriteBuf[0]);
			pr_force("\n\n		<-> Read[0x1280000C] : 0x%08X",
					auiReadBuf[0]);
			pr_force("\n\n Actual => Write[0x12800010] : 0x%08X",
					auiWriteBuf[1]);
			pr_force("\n\n		<-> Read[0x12800010] : 0x%08X",
					auiReadBuf[1]);

			for (ucCount = 0;
					ucCount < (uint8_t)NUM_OF_BYTES_REGBANK_TEST;
					ucCount++) {
				if (auiWriteBuf[ucCount] !=
					auiReadBuf[ucCount]) {
					pr_force("\n\n Result FAIL\n");
					break;
				}
			}
			if (ucCount == (uint8_t)NUM_OF_BYTES_REGBANK_TEST) {
				/*For coverity*/
				pr_force("\n\n Result => PASS");
			}

			for (ucCount = 0;
					ucCount < (uint8_t)NUM_OF_BYTES_REGBANK_TEST;
					ucCount++) {
				/*For coverity*/
				auiWriteBuf[ucCount] = 0;
			}
			(void)Dpv14_Tx_API_Write_RegisterBank_Value(
						auiWriteBuf,
						(uint32_t)0x000C,
						(uint8_t)NUM_OF_BYTES_REGBANK_TEST);

			Dpv14_Tx_API_Config_RegisterAccess_Mode(false);
			for (ucCount = 0;
					ucCount < (uint8_t)NUM_OF_BYTES_REGBANK_TEST;
					ucCount++) {
				if (ucCount == 0U) {
					/*For coverity*/
					auiWriteBuf[ucCount] = 0x00005A5AU;
				} else {
					/*For coverity*/
					auiWriteBuf[ucCount] = 0x5A5A0000U;
				}
			}

			pr_force("\n\n2nd stage: Reg. Access mode to MICOM");
			pr_force("\n Expect=> Write[0x1280000C]:0x00005A5A");
			pr_force("\n	<-> Read[0x1280000C]: 0x00000000");
			pr_force("\n	Write[0x12800010]: 0x5A5A0000");
			pr_force("\n	<-> Read[0x12800010]: 0x00000000");

			(void)Dpv14_Tx_API_Write_RegisterBank_Value(
						auiWriteBuf,
						(uint32_t)0x000C,
						(uint8_t)NUM_OF_BYTES_REGBANK_TEST);
			(void)Dpv14_Tx_API_Read_RegisterBank_Value(
						auiReadBuf,
						(uint32_t)0x000C,
						(uint8_t)NUM_OF_BYTES_REGBANK_TEST);

			pr_force("\n\n Actual => Write[0x1280000C] : 0x%08X",
						auiWriteBuf[0]);
			pr_force("\n		<-> Read[0x1280000C] : 0x%08X",
						auiReadBuf[0]);
			pr_force("\n Actual => Write[0x12800010] : 0x%08X",
						auiWriteBuf[1]);
			pr_force("\n		<-> Read[0x12800010] : 0x%08X",
						auiReadBuf[1]);

			for (ucCount = 0;
					ucCount < (uint8_t)NUM_OF_BYTES_REGBANK_TEST;
					ucCount++) {
				if (auiWriteBuf[ucCount] ==
					auiReadBuf[ucCount]) {
					pr_force("\n\n Result => FAIL\n");
					break;
				}
			}
			if (ucCount == (uint8_t)NUM_OF_BYTES_REGBANK_TEST) {
				/*For coverity*/
				pr_force("\n\n Result => PASS ");
			}

			pr_force("\n **************[End]**************\n");
			Dpv14_Tx_API_Config_RegisterAccess_Mode(true);
		} else {
			/*For coverity*/
			pr_force("\n[%s:%d]Error: invalid regbank case (%d)\n",
							__func__,
							__LINE__,
							(u32)ulCases);
		}
	}

	return 0;
}

static int do_dpv14_register_bank(
					struct cmd_tbl_s *cmdtp,
					int flag,
					int argc,
					char *const argv[])
{
	unsigned char	ucCount;
	unsigned long	ulCases;
	unsigned int	auiWriteBuf[NUM_OF_BYTES_REGBANK_TEST];
	unsigned int	auiReadBuf[NUM_OF_BYTES_REGBANK_TEST];

	cmdtp++;
	pr_info("\nflag: %d, argc: %d\n", flag, argc);

	if (strcmp(argv[1], "help") == 0) {
		print_regbank_help();
	} else {
		ulCases = simple_strtoul(argv[1], NULL, 10);

		if (ulCases == 1U) {
			Dpv14_Tx_API_Config_RegisterAccess_Mode(true);

			Dpv14_Tx_API_Config_PW();
			Dpv14_Tx_API_Config_CfgLock(true);
			Dpv14_Tx_API_Config_CfgAccess(true);

			for (ucCount = 0;
					ucCount < (uint8_t)NUM_OF_BYTES_REGBANK_TEST;
					ucCount++) {
				if (ucCount == 0U) {
					/*For coverity*/
					auiWriteBuf[ucCount] = 0x00005A5AU;
				} else {
					/*For coverity*/
					auiWriteBuf[ucCount] = 0x5A5A0000U;
				}

				auiReadBuf[ucCount] = 0x00U;
			}

			pr_force("\n *********[regbank case 1]***********");
			pr_force("\n Expect=> Write[0x1280000C]:0x00005A5A");
			pr_force("\n	<-> Read[0x1280000C]:0x00005A5A");
			pr_force("\n	Write[0x12800010]:0x5A5A0000");
			pr_force("\n	<-> Read[0x12800010]:0x5A5A0000");

			(void)Dpv14_Tx_API_Write_RegisterBank_Value(
					auiWriteBuf,
					(uint32_t)0x000C,
					(uint8_t)NUM_OF_BYTES_REGBANK_TEST);
			(void)Dpv14_Tx_API_Read_RegisterBank_Value(
					auiReadBuf,
					(uint32_t)0x000C,
					(uint8_t)NUM_OF_BYTES_REGBANK_TEST);

			pr_force("\n\n Actual => Write[0x1280000C] : 0x%08X",
						auiWriteBuf[0]);
			pr_force("\n		<-> Read[0x1280000C] : 0x%08X",
						auiReadBuf[0]);
			pr_force("\n Actual => Write[0x12800010] : 0x%08X",
						auiWriteBuf[1]);
			pr_force("\n		<-> Read[0x12800010] : 0x%08X",
						auiReadBuf[1]);

			for (ucCount = 0;
					ucCount < (uint8_t)NUM_OF_BYTES_REGBANK_TEST;
					ucCount++) {
				if (auiWriteBuf[ucCount] !=
					auiReadBuf[ucCount]) {
					pr_force("\n\n Result FAIL\n");
					break;
				}
			}
			if (ucCount == (uint8_t)NUM_OF_BYTES_REGBANK_TEST) {
				/*For coverity*/
				pr_force("\n\n Result => PASS ");
			}

			pr_force("\n **************[End]************\n");

			for (ucCount = 0;
					ucCount < (uint8_t)NUM_OF_BYTES_REGBANK_TEST;
					ucCount++) {
				auiWriteBuf[ucCount] = 0U;
			}
			(void)Dpv14_Tx_API_Write_RegisterBank_Value(
					auiWriteBuf,
					0x000C,
					(uint8_t)NUM_OF_BYTES_REGBANK_TEST);
		} else if (ulCases == 2U) {
			Dpv14_Tx_API_Config_RegisterAccess_Mode(true);

			Dpv14_Tx_API_Config_PW();
			Dpv14_Tx_API_Config_CfgLock(true);
			Dpv14_Tx_API_Config_CfgAccess(false);

			for (ucCount = 0;
					ucCount < (uint8_t)NUM_OF_BYTES_REGBANK_TEST;
					ucCount++) {
				if (ucCount == 0U) {
					/*For coverity*/
					auiWriteBuf[ucCount] = 0x00005A5AU;
				} else {
					/*For coverity*/
					auiWriteBuf[ucCount] = 0x5A5A0000U;
				}

				auiReadBuf[ucCount] = 0x00U;
			}

			pr_force("\n ***********[regbank case 2]************");
			pr_force("\n Expect=> Write[0x1280000C]:0x00005A5A");
			pr_force("\n	<-> Read[0x1280000C]:0x00005A5A");
			pr_force("\n	Write[0x12800010]:0x5A5A0000");
			pr_force("\n	<-> Read[0x12800010]:0x5A5A0000");

			(void)Dpv14_Tx_API_Write_RegisterBank_Value(
					auiWriteBuf,
					(uint32_t)0x000C,
					(uint8_t)NUM_OF_BYTES_REGBANK_TEST);
			(void)Dpv14_Tx_API_Read_RegisterBank_Value(
					auiReadBuf,
					(uint32_t)0x000C,
					(uint8_t)NUM_OF_BYTES_REGBANK_TEST);

			pr_force("\n\n Actual => Write[0x1280000C] : 0x%08X",
						auiWriteBuf[0]);
			pr_force("\n	<-> Read[0x1280000C] : 0x%08X",
						auiReadBuf[0]);
			pr_force("\n Actual => Write[0x12800010] : 0x%08X",
						auiWriteBuf[1]);
			pr_force("\n	<-> Read[0x12800010] : 0x%08X",
						auiReadBuf[1]);

			for (ucCount = 0;
					ucCount < (uint8_t)NUM_OF_BYTES_REGBANK_TEST;
					ucCount++) {
				if (auiWriteBuf[ucCount] ==
					auiReadBuf[ucCount]) {
					pr_force("\n\n Result => FAIL");
					break;
				}
			}
			if (ucCount == (uint8_t)NUM_OF_BYTES_REGBANK_TEST) {
				/*For coverity*/
				pr_force("\n\n Result => PASS ");
			}
			pr_force("\n *************[End]***********\n");
		} else {
			/*For coverity*/
			pr_force("\n[%s:%d]Error: invalid regbank case (%d)",
							__func__,
							__LINE__,
							(u32)ulCases);
		}
	}

	return 0;
}

static int do_dpv14_read_edid(
						struct cmd_tbl_s *cmdtp,
						int flag,
						int argc,
						char *const argv[])
{
	uint8_t ucStreamIdx;
	unsigned long	ulCases;
	struct DPTX_Dtd_Params_t stDptx_Dtd_Params;

	cmdtp++;
	pr_info("\nflag: %d, argc: %d\n", flag, argc);

	if (strcmp(argv[1], "help") == 0) {
		print_read_edid_help();
	} else {
		ulCases = simple_strtoul(argv[1], NULL, 10);

	ucStreamIdx = (uint8_t)(ulCases & 0xFFU);
	if (ucStreamIdx < 4U) {
		/*For coverity*/
		(void)Dpv14_Tx_API_Get_Dtd_From_Edid(
			&stDptx_Dtd_Params,
			ucStreamIdx);
	} else {
		/*For coverity*/
		pr_force("\nErr : invalid stream index as %d\n", ucStreamIdx);
	}

		pr_force("\n[Display timing set from EDID\n");
		pr_force("	Pixel clk = %d\n",
				(u32)stDptx_Dtd_Params.uiPixel_Clock);
		pr_force("	Repetition = %d\n",
				(u32)stDptx_Dtd_Params.pixel_repetition_input);
		pr_force("	%s\n",
				(stDptx_Dtd_Params.interlaced) ?
				"Interlace":"Progressive");
		pr_force("	H Sync Polarity(%d), V Sync Polarity(%d)\n",
				(u32)stDptx_Dtd_Params.h_sync_polarity,
				(u32)stDptx_Dtd_Params.h_sync_polarity);
		pr_force("	H Active(%d), V Active(%d)\n",
				(u32)stDptx_Dtd_Params.h_active,
				(u32)stDptx_Dtd_Params.v_active);
		pr_force("	H Image size(%d), V Image size(%d)\n",
				(u32)stDptx_Dtd_Params.h_image_size,
				(u32)stDptx_Dtd_Params.v_image_size);
		pr_force("	H Blanking(%d), V Blanking(%d)\n",
				(u32)stDptx_Dtd_Params.h_blanking,
				(u32)stDptx_Dtd_Params.v_blanking);
		pr_force("	H Sync offset(%d), V Sync offset(%d)\n",
				(u32)stDptx_Dtd_Params.h_sync_offset,
				(u32)stDptx_Dtd_Params.v_sync_offset);
		pr_force("	H Sync plus W(%d), V Sync plus W(%d)\n",
				(u32)stDptx_Dtd_Params.h_sync_pulse_width,
				(u32)stDptx_Dtd_Params.v_sync_pulse_width);
	}

	return  0;
}


static int do_dpv14_cts(
				struct cmd_tbl_s *cmdtp,
				int flag,
				int argc,
				char *const argv[])
{
	int32_t ret = 0;
	uint8_t ucPreEmp_Level, ucVSW_Level;
	uint32_t uiEQ_Main, uiEQ_Post, uiEQ_Pre, uiEQ_Vboost;
	unsigned long ulElements;
	unsigned long ulCases, ulTetstItem;
	unsigned long ulEQ_Main, ulEQ_Post, ulEQ_Pre, ulEQ_Vboost;
	unsigned long ulPreEmp_Level, ulVSW_Level;

	cmdtp++;
	pr_info("\nflag: %d, argc: %d\n", flag, argc);

	if (strcmp(argv[1], "help") == 0) {
		/*For coverity*/
		print_cts_help();
	} else {
		ulCases = simple_strtoul(argv[1], NULL, 10);

		if (ulCases >= (unsigned long)CTS_CATEGORY_MAX) {
			print_cts_help();
			goto return_funcs;
		}

		if (ulCases == (unsigned long)CTS_RBR_HBR) {
			ulTetstItem = simple_strtoul(argv[2], NULL, 10);

			ulElements =
					(unsigned long)(sizeof(stDPV14_CTS_RBR_HBR_Params_t) /
					sizeof(struct  DPV14_CTS_Params_t));
			if (ulTetstItem >=  ulElements) {
				print_cts_help();
				goto return_funcs;
			}

			(void)Dpv14_Tx_API_Set_CTS(
		stDPV14_CTS_RBR_HBR_Params_t[ulTetstItem].bSSC_Enabled,
		stDPV14_CTS_RBR_HBR_Params_t[ulTetstItem].ucClk_Selection,
		(uint32_t)stDPV14_CTS_RBR_HBR_Params_t[ulTetstItem].ePattern_Type,
		stDPV14_CTS_RBR_HBR_Params_t[ulTetstItem].eLink_Rate,
		stDPV14_CTS_RBR_HBR_Params_t[ulTetstItem].ePreEmp,
		stDPV14_CTS_RBR_HBR_Params_t[ulTetstItem].eVSW);
		} else if (ulCases == (unsigned long)CTS_HBR2_HBR3) {
			ulTetstItem = simple_strtoul(argv[2], NULL, 10);

			ulElements =
					(unsigned long)(sizeof(stDPV14_CTS_HBR23_Params_t) /
					sizeof(struct DPV14_CTS_Params_t));
			if (ulTetstItem >=  ulElements) {
				print_cts_help();
				goto return_funcs;
			}

			(void)Dpv14_Tx_API_Set_CTS(
		stDPV14_CTS_HBR23_Params_t[ulTetstItem].bSSC_Enabled,
		stDPV14_CTS_HBR23_Params_t[ulTetstItem].ucClk_Selection,
		(uint32_t)stDPV14_CTS_HBR23_Params_t[ulTetstItem].ePattern_Type,
		stDPV14_CTS_HBR23_Params_t[ulTetstItem].eLink_Rate,
		stDPV14_CTS_HBR23_Params_t[ulTetstItem].ePreEmp,
		stDPV14_CTS_HBR23_Params_t[ulTetstItem].eVSW);
		} else if (ulCases == (unsigned long)CTS_EQ_SETTING_LIST) {
			ulTetstItem = simple_strtoul(argv[2], NULL, 10);

			ulElements =
					(unsigned long)(sizeof(stDptx_EQ_Tuning_Params) /
					sizeof(struct Dptx_EQ_Tuning_Params_t));
			if (ulTetstItem >=  ulElements) {
				print_cts_help();
				goto return_funcs;
			}

			(void)Dpv14_Tx_API_Set_EQ(
		stDptx_EQ_Tuning_Params[ulTetstItem].uiTx_EQ_Main,
		stDptx_EQ_Tuning_Params[ulTetstItem].uiTx_EQ_Post,
		stDptx_EQ_Tuning_Params[ulTetstItem].uiTx_EQ_Pre,
		stDptx_EQ_Tuning_Params[ulTetstItem].uiTx_EQ_VBoost);
		} else if (ulCases == (unsigned long)CTS_EQ_SETTING) {
			ulEQ_Main = simple_strtoul(argv[2], NULL, 10);
			ulEQ_Post = simple_strtoul(argv[3], NULL, 10);
			ulEQ_Pre = simple_strtoul(argv[4], NULL, 10);
			ulEQ_Vboost = simple_strtoul(argv[5], NULL, 10);

			uiEQ_Main = (uint32_t)(ulEQ_Main & 0xFFFFFFFFU);
			uiEQ_Post = (uint32_t)(ulEQ_Post & 0xFFFFFFFFU);
			uiEQ_Pre = (uint32_t)(ulEQ_Pre & 0xFFFFFFFFU);
			uiEQ_Vboost = (uint32_t)(ulEQ_Vboost & 0xFFFFFFFFU);

			(void)Dpv14_Tx_API_Set_EQ(
					uiEQ_Main,
					uiEQ_Post,
					uiEQ_Pre,
					uiEQ_Vboost);
		} else if (ulCases == (unsigned long)CTS_VSW_SETTING) {
			ulPreEmp_Level = simple_strtoul(argv[2], NULL, 10);

			ucPreEmp_Level = (uint8_t)(ulPreEmp_Level & 0xFFU);

			(void)Dpv14_Tx_API_Set_LinkPrams(1, ucPreEmp_Level);
		} else {
			ulVSW_Level = simple_strtoul(argv[2], NULL, 10);

			ucVSW_Level = (uint8_t)(ulVSW_Level & 0xFFU);

			(void)Dpv14_Tx_API_Set_LinkPrams(0, ucVSW_Level);
		}
	}

return_funcs:
	return ret;
}

static int do_dpv14_set_panel_power(
					struct cmd_tbl_s *cmdtp,
					int flag,
					int argc,
					char *const argv[])
{
	unsigned long ulCases, ulPanelPower;

	cmdtp++;

	pr_info("\nflag: %d, argc: %d\n", flag, argc);

	if (strcmp(argv[1], "help") == 0) {
		/*For coverity*/
		print_set_panel_power_help();
	} else {
		ulCases = simple_strtoul(argv[1], NULL, 10);
		ulPanelPower = simple_strtoul(argv[2], NULL, 10);

		if (ulCases == 0U)/* TCC8059 */ {
			if (ulPanelPower == 0U) {
				/* 1st & 2nd bl off */
				tcclcd_gpio_set_value(TCC_GPMA(26U), 0);
			} else if (ulPanelPower == 1U) {
				/* 1st & 2nd bl on */
				tcclcd_gpio_set_value(TCC_GPMA(26U), 1);
			} else if (ulPanelPower == 2U) {
				/* 1st & 2nd off */ 
				tcclcd_gpio_set_value(TCC_GPB(17U), 0);
			} else if (ulPanelPower == 3U) {
				/* 1st & 2nd on */
				tcclcd_gpio_set_value(TCC_GPB(17U), 1);
			} else if (ulPanelPower == 4U) {
				/* 1st & 2nd reset off */
				tcclcd_gpio_set_value(TCC_GPB(18U), 0);
			} else if (ulPanelPower == 5U) {
				/* 1st & 2nd reset on */
				tcclcd_gpio_set_value(TCC_GPB(18U), 1);
			} else {
				/*For coverity*/
				pr_force("\nErr : invalid test case as %d\n", (uint32_t)ulPanelPower);
			}
		} else if (ulCases == 1U)/* TCC8050 */ {
			if (ulPanelPower == 0U) {
				/* Panel off */
				tcclcd_gpio_set_value(TCC_GPMC(19U), 0);
			} else if (ulPanelPower == 1U) {
				/* Panel on */
				tcclcd_gpio_set_value(TCC_GPMC(19U), 1);
			} else if (ulPanelPower == 2U) {
				/* 1st bl off */
				tcclcd_gpio_set_value(TCC_GPH(6U), 0);
			} else if (ulPanelPower == 3U) {
				/* 1st bl on */
				tcclcd_gpio_set_value(TCC_GPH(6U), 1);
			} else if (ulPanelPower == 4U) {
				/* 2nd bl off */
				tcclcd_gpio_set_value(TCC_GPH(7U), 0);
			} else if (ulPanelPower == 5U) {
				/* 2nd bl on */
				tcclcd_gpio_set_value(TCC_GPH(7U), 1);
			} else if (ulPanelPower == 6U) {
				/* 3rd bl off */
				tcclcd_gpio_set_value(TCC_GPE(15U), 0);
			} else if (ulPanelPower == 7U) {
				/* 3rd bl on */
				tcclcd_gpio_set_value(TCC_GPE(15U), 1);
			} else if (ulPanelPower == 8U) {
				/* 4th bl off */
				tcclcd_gpio_set_value(TCC_GPMC(21U), 0);
			} else if (ulPanelPower == 9U) {
				/* 4th bl on */
				tcclcd_gpio_set_value(TCC_GPMC(21U), 1);
			} else if (ulPanelPower == 10U) {
				/* 1st reset off */
				tcclcd_gpio_set_value(TCC_GPMC(20U), 0);
			} else if (ulPanelPower == 11U) {
				/* 1st reset on */
				tcclcd_gpio_set_value(TCC_GPMC(20U), 1);
			} else if (ulPanelPower == 12U) {
				/* 2nd reset off */
				tcclcd_gpio_set_value(TCC_GPC(9U), 0);
			} else if (ulPanelPower == 13U) {
				/* 2nd reset on */
				tcclcd_gpio_set_value(TCC_GPC(9U), 1);
			} else if (ulPanelPower == 14U) {
				/* 3rd reset off */
				tcclcd_gpio_set_value(TCC_GPMC(18U), 0);
			} else if (ulPanelPower == 15U) {
				/* 3rd reset on */
				tcclcd_gpio_set_value(TCC_GPMC(18U), 1);
			} else if (ulPanelPower == 16U) {
				/* 4th reset off */
				tcclcd_gpio_set_value(TCC_GPMB(19U), 0);
			} else if (ulPanelPower == 17U) {
				/* 4th reset on */
				tcclcd_gpio_set_value(TCC_GPMB(19U), 1);
			} else {
				/*For coverity*/
				pr_force("\nErr : invalid test case as %d\n", (uint32_t)ulPanelPower);
			}
		} else {
			/*For coverity*/
			pr_force("\nErr : invalid test case as %d\n", (uint32_t)ulCases);
		}
	}

	return  0;
}

static cmd_tbl_t cmd_dpv14_sub[] = {
//	U_BOOT_CMD_MKENT(lcdinit, 3, 0, do_dpv14_lcd_init, "", ""),
	U_BOOT_CMD_MKENT(audio_port_mux, 3, 0, do_dpv14_audio_port_mux, "", ""),
	U_BOOT_CMD_MKENT(link, 3, 0, do_dpv14_set_DPLink_Params, "", ""),
	U_BOOT_CMD_MKENT(regbank, 3, 0, do_dpv14_register_bank, "", ""),
	U_BOOT_CMD_MKENT(regacc, 3, 0, do_dpv14_register_access_mode, "", ""),
	U_BOOT_CMD_MKENT(pll, 3, 0, do_dpv14_pll, "", ""),
	U_BOOT_CMD_MKENT(ppwr, 3, 0, do_dpv14_set_panel_power, "", ""),
	U_BOOT_CMD_MKENT(redid, 3, 0, do_dpv14_read_edid, "", ""),
	U_BOOT_CMD_MKENT(cts, 3, 0, do_dpv14_cts, "", ""),
};

static int do_dpv14(
				struct cmd_tbl_s *cmdtp,
				int flag,
				int argc,
				char *const argv[])
{
	int ret = 0;
	int iTableLen;
	const cmd_tbl_t *c;

	if (argc < 2) {
		ret = (int)CMD_RET_USAGE;
		goto return_funcs;
	}
	argv++;

	iTableLen = (int)(ARRAY_SIZE(cmd_dpv14_sub) & 0xFFFFFFFFU);
	c = find_cmd_tbl(argv[0], &cmd_dpv14_sub[0], iTableLen);

	if (c != NULL) {
		ret = c->cmd(cmdtp, flag, argc, argv);
		goto return_funcs;
	} else {
		ret = (int)CMD_RET_USAGE;
		goto return_funcs;
	}

return_funcs:
	return ret;

}

/***************************************************/
static char dpv14_help_text[] =
	"---------------------------------------------\r\n"
	"dpv14 lcd driver init \r\n"
	"dpv14 power \r\n"
	"dpv14 regbank \r\n"
	"dpv14 pll \r\n"
	"dpv14 linkrate \r\n"
	"dpv14 colormetry mapping        \r\n"
	"dpv14 video code \r\n"
	"dpv14 video mute \r\n"
	"dpv14 colorspace \r\n"
	"dpv14 register dump \r\n"
	"dpv14 eq \r\n"
	"dpv14 cts \r\n"
	"dpv14 redid( read edid ) \r\n"
	"dpv14 audio mute (amute)\r\n"
	"dpv14 audio channel (audio_ch)\r\n"
	"dpv14 audio hbr enable (audio_hbr)\r\n"
	"dpv14 audio port mux (audio_port_mux)\r\n"
	"dpv14 audio sampling freq (audio_freq)\r\n"
	"dpv14 audio SDP infoframe (audio_sdp_info)\r\n"
	"----------------------------------------------\r\n";


static char usage[] = "Verify dp v1.4 driver functionalities";

U_BOOT_CMD(
	dpv14, 7, 1, do_dpv14,
	usage,
	dpv14_help_text
);
