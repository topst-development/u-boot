// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>

//#include <asm/telechips/gpio.h>

//#include "tcc_lcd_interface.h"
#include "dptx_api.h"

#define NUM_OF_TESTS	2

#define	SSC_ON		true
#define	SSC_OFF		false

#define	CLK_XIN		0x00
#define	CLK_PAD		0x08
#define	CLK_INTERNAL	0x0C


enum LINK_TEST_TYPE {
	LINK_TEST_RATE = 0,
	LINK_TEST_LANE = 1,
	LINK_TEST_VID_ENCODING = 2,
	LINK_TEST_UNKNOWN = 3
};

enum PROTECT_TEST_TYPE {
	PROTECT_TEST_UNLOCK_SUCC = 0,
	PROTECT_TEST_UNLOCK_FAIL = 1,
	PROTECT_TEST_ACCESS_MODE = 2,
	PROTECT_TEST_UNKNOWN = 3
};

enum CTS_TEST_TYPE {
	CTS_TEST_RBR_HBR = 0,
	CTS_TEST_HBR2_HBR3 = 1,
	CTS_TEST_EQ_SET_BY_LIST = 2,
	CTS_TEST_EQ_SET = 3,
	CTS_TEST_VSW_SET = 4,
	CTS_TEST_PREEMP_SET = 5,
	CTS_TEST_UNKNOWN = 6
};

enum TCC_EVB_TYPE {
	TCC_EVB_8059_SV01 = 0,
	TCC_EVB_8050_SV01 = 1,
	TCC_EVB_8050_SV10 = 2,
	TCC_EVB_TYPE_UNKNOWN = 3
};

enum PANEL_GPIO_TYPE {
	PANEL_1ST_GPIO_PWR_OFF = 0,
	PANEL_1ST_GPIO_PWR_ON = 1,
	PANEL_1ST_GPIO_BL_OFF = 2,
	PANEL_1ST_GPIO_BL_ON = 3,
	PANEL_1ST_GPIO_RST_OFF = 4,
	PANEL_1ST_GPIO_RST_ON = 5,
	PANEL_2ND_GPIO_BL_OFF = 6,
	PANEL_2ND_GPIO_BL_ON = 7,
	PANEL_2ND_GPIO_RST_OFF = 8,
	PANEL_2ND_GPIO_RST_ON = 9,
	PANEL_3RD_GPIO_BL_OFF = 10,
	PANEL_3RD_GPIO_BL_ON = 11,
	PANEL_3RD_GPIO_RST_OFF = 12,
	PANEL_3RD_GPIO_RST_ON = 13,
	PANEL_4TH_GPIO_BL_OFF = 14,
	PANEL_4TH_GPIO_BL_ON = 15,
	PANEL_4TH_GPIO_RST_OFF = 16,
	PANEL_4TH_GPIO_RST_ON = 17,
	PANEL_GPIO_UNKNOWN = 18
};

struct DPV14_CTS_Test_t {
	bool bSSC_Enabled;
	unsigned char ucClk_Selection;
	enum DPTX_LINK_RATE eLink_Rate;
	enum DPTX_PRE_EMPHASIS_LEVEL ePreEmp;
	enum DPTX_VOLTAGE_SWING_LEVEL eVSW;
	enum DPTX_TRAINING_PATTERN_TYPE ePattern_Type;
};

struct DPV14_EQ_Tuning_t {
	uint32_t uiTx_EQ_Main;
	uint32_t uiTx_EQ_Post;
	uint32_t uiTx_EQ_Pre;
	uint32_t uiTx_EQ_VBoost;
};

static struct DPV14_EQ_Tuning_t stEQ_Tuning_t[] = {
	{9,  0, 0, 3},  {11, 8,  0,  3}, {13, 16, 0, 3}, {13, 24, 0, 7},
	{15, 0, 0, 3}, {18, 12, 0, 3}, {18, 20, 0, 3},
	{20, 0, 0, 3}, {20, 16, 0, 7},
	{24, 0, 0, 7},
};

static struct DPV14_CTS_Test_t stRBR_HBR_Test_t[] = {
	/* Dummy */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_INVALID,
		DPTX_VOLTAGE_SWING_LEVEL_INVALID,
		DPTX_TRAINING_CTRL_TPS_INVALID },

	/* 1~ : Source Eye Diagram Test(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_OFF, CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_OFF, CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 5~ : Source Total Jitter Test(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 9~ : Source Non-ISI Jitter Test (TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 13~ : Source Non Pre-Emphasis Level Test(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 17~ : Source Pre-Emphasis Level Test(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 21~ : Source Non Transition Voltage Range Measurement Test(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 25~ : Source Peak to Peak Voltage Test(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 29~ : Source Inter-Pair Skew Test (Informative)(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 31~ : Source Main Link Frequency Compliance Test(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 33~ : Source (SSC) Modulation Frequency Test(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 35~ : Source (SSC) Modulation Deviation Test(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 37~ : Source(SSC) Deviation HF Variation Test (Informative)(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 39~ : Source Eye Diagram Test (TP3_EQ)(Informative) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 41~ : Source Total Jitter Test (TP3_EQ)(Informative) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 45~ : Source AC Common Mode Test (Informative)(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 49~ : Source Intra-Pair Skew Test (Informative)(TP2) */
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_RBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
};


static struct DPV14_CTS_Test_t stHBR23_Test_t[] = {
	/* Dummy */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_INVALID,
		DPTX_VOLTAGE_SWING_LEVEL_INVALID,
		DPTX_TRAINING_CTRL_TPS_INVALID },

	/* 1~ : Source Total Jitter Test (TP3_EQ) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_CP2520_1 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON,  CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_CP2520_1 },

	/* 5~ : Source Total Jitter Test (TP2_CTLE and TP3_CTLE) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_4 },

	/* 7~ : Source Non-ISI Jitter Test (TP2) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },

	/* 8~ : Source Non-ISI Jitter Test (TP2_CTLE and TP3_CTLE) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_4 },

	/* 10~ : Source Deterministic Jitter Test (TP3_EQ) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_CP2520_1 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_CP2520_1 },

	/* 14~ : Source Random Jitter Test (TP3_EQ) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 16~ : Source Non-ISI Jitter Test (TP2) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },

	/* 17~ : Source Intra-Pair Skew */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 19~ : Source Inter-Pair Skew Test (TP2) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_PRBS7 },

	/* 21~ : Source Peak to Peak Voltage Test (TP2) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_CUSTOM80 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3
	, DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_CUSTOM80 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_CUSTOM80 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_CUSTOM80 },

	/* 25~ : Source Main Link Frequency Compliance Test (TP2) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 27~ : Source (SSC) Modulation Frequency Test (TP2) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 29~ : Source (SSC) Modulation Deviation Test (Tp2) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 31~ : Source (SSC) Deviation HF Variation Test (Tp2) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_1_D102 },

	/* 33~ : Source Eye Diagram Test (TP3_EQ) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_CP2520_1 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_CP2520_1 },

	/* 35~ : Source Eye Diagram Test (TP3_CTLE) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_4 },

	/* 37~ : Source Eye Diagram Test (TP2_CTLE) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_4 },

	/* 39~ : Source Eye Diagram Test (TP_RX_DFE) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_4 },

	/* 41~ : Source AC Common Mode Test (TP2) */
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_PRBS7 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_3, DPTX_TRAINING_CTRL_TPS_4 },
	{ SSC_ON, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_3,
		DPTX_VOLTAGE_SWING_LEVEL_0, DPTX_TRAINING_CTRL_TPS_4 },

	/* 45~ : Source Level and Equalization Verification Test (TP2) */
	{ SSC_OFF, CLK_XIN, DPTX_LINK_RATE_HBR2, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_CUSTOM80 },
	{ SSC_OFF, CLK_XIN, DPTX_LINK_RATE_HBR3, DPTX_PRE_EMPHASIS_LEVEL_0,
		DPTX_VOLTAGE_SWING_LEVEL_2, DPTX_TRAINING_CTRL_TPS_CUSTOM80 },
};

static void print_link_help(void)
{
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("Verifying DP link \r\n");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("dpv14 link <n><m> - Set cases \r\n"
			"		n - 0 : set Link rate \r\n"
			"				m - 0 :RBR  \r\n"
			"				m - 1 :HBR  \r\n"
			"				m - 2 :HBR2  \r\n"
			"				m - 3 :HBR3  \r\n"
			"		 n - 1 : set Link lane \r\n"
			"				m - 1 :1 lane  \r\n"
			"				m - 2 :2 lanes \r\n"
			"				m - 4 :4 lanes \r\n"
			"		 n - 2 : Vid encoding type \r\n"
			"				m - 0 :RGB  \r\n"
			"				m - 1 :YCBCR422 \r\n"
			"				m - 2 :YCBCR444 \r\n");
}

static void print_pll_help(void)
{
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("Verifying DP Pll locking\r\n");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("dpv14 pll <n> - Set cases \r\n"
			"	n : N/A \r\n");
}

static void print_regbank_help(void)
{
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("Verifying DP Register Bank \r\n");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("dpv14 regbank <n> - Set cases \r\n"
			"	n - 0 : dp_cfg_pw_ok + \r\n"
			"			dp_cfg_pw_lock(unlock) + \r\n"
			"			dp_cfg_access(unlock) \r\n"
			"		1 : dp_cfg_pw_ok( PW writing ) + \r\n"
			"			dp_cfg_pw_lock( unlock ) + \r\n"
			"			dp_cfg_access( lock ) \r\n"
			"		2 : dp_cfg_access( unlock -> lock ) \r\n");
}

static void print_read_edid_help(void)
{
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("Verifying DP PHY Power on / off \r\n");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("dpv14 power <n> - Set cases \r\n"
			"	n - 0 ~ 4 : Stream index 0 ~ 4\r\n");
}

static void print_cts_help(void)
{
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("Testing signal quality with CTS \r\n");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("dpv14 cts <n> <m> - Set cases \r\n"
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
			"				n - 5  : CTS Test for Pre-emp Setting   \r\n");
}

static void print_set_panel_power_help(void)
{
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("Panel power control\r\n");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("dpv14 ppwr <n> - Set cases\r\n"
		"		n - 0 : TCC8059 EVB SV01\r\n"
		"				m - 0 :1st & 2nd Power off\r\n"
		"				m - 1 :1st & 2nd Power on\r\n"
		"				m - 2 :1st & 2nd bl off\r\n"
		"				m - 3 :1st & 2nd bl on\r\n"
		"				m - 4 :1st & 2nd reset off\r\n"
		"				m - 5 :1st & 2nd reset on\r\n"
		"		 n - 1 : TCC8050 EVB SV01 \r\n"
		"				m - 0 :Power off\r\n"
		"				m - 1 :Power on\r\n"
		"				m - 2 :1st bl off\r\n"
		"				m - 3 :1st bl on\r\n"
		"				m - 4 :1st reset off\r\n"
		"				m - 5 :1st reset on\r\n"
		"				m - 6 :2nd bl off\r\n"
		"				m - 7 :2nd bl on\r\n"
		"				m - 8 :2nd reset off\r\n"
		"				m - 9 :2nd reset on\r\n"
		"				m - 10 :3rd bl off\r\n"
		"				m - 11 :3rd bl on\r\n"
		"				m - 12 :3rd reset off\r\n"
		"				m - 13 :3rd reset on\r\n"
		"				m - 14 :4th bl off\r\n"
		"				m - 15 :4th bl on\r\n"
		"				m - 16 :4th reset off\r\n"
		"				m - 17 :4th reset on\r\n"
		"		 n - 2 : TCC8050 EVB SV10 \r\n"
		"				m - 0 :Power off\r\n"
		"				m - 1 :Power on\r\n"
		"				m - 2 :1st bl off\r\n"
		"				m - 3 :1st bl on\r\n"
		"				m - 4 :1st reset off\r\n"
		"				m - 5 :1st reset on\r\n"
		"				m - 6 :2nd bl off\r\n"
		"				m - 7 :2nd bl on\r\n"
		"				m - 8 :2nd reset off\r\n"
		"				m - 9 :2nd reset on\r\n"
		"				m - 10 :3rd bl off\r\n"
		"				m - 11 :3rd bl on\r\n"
		"				m - 12 :3rd reset off\r\n"
		"				m - 13 :3rd reset on\r\n"
		"				m - 14 :4th bl off\r\n"
		"				m - 15 :4th bl on\r\n"
		"				m - 16 :4th reset off\r\n"
		"				m - 17 :4th reset on\r\n");
}


/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int do_dpv14_set_link(struct cmd_tbl  *cmdtp,
										int flag,
										int argc,
										char *const argv[])
{
	int32_t ret = 0;
	uint64_t ulTestCase, ulElement;
	enum DPTX_LINK_RATE eRate;
	enum DPTX_LINK_LANE eLane;
	enum DPTX_VIDEO_ENCODING_TYPE eEncodeType;
	enum LINK_TEST_TYPE eLink_Test;

	(void)cmdtp;
	(void)flag;
	(void)argc;

	if (strcmp(argv[1], "help") == 0) {
		print_link_help();

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	ulTestCase = simple_strtoul(argv[1], NULL, 10);
	if (ulTestCase >= (uint64_t)LINK_TEST_UNKNOWN) {
		print_link_help();

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	eLink_Test = (enum LINK_TEST_TYPE)ulTestCase;

	ulElement = simple_strtoul(argv[2], NULL, 10);

	switch (eLink_Test) {
	case LINK_TEST_RATE:
		if (ulElement >= (uint64_t)DPTX_LINK_RATE_MAX) {
			print_link_help();

			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto return_funcs;
		}

		eRate = (enum DPTX_LINK_RATE)ulElement;

		(void)Dpv14_Tx_API_Set_MaxLinkRate_Supported(eRate);
		(void)Dpv14_Tx_API_Perform_HPD_WorkingFlow();
		break;
	case LINK_TEST_LANE:
		if ((ulElement != 1U) && (ulElement != 2U) && (ulElement != 4U)) {
			print_link_help();

			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto return_funcs;
		}

		eLane = (enum DPTX_LINK_LANE)ulElement;

		(void)Dpv14_Tx_API_Set_MaxLinkLane_Supported(eLane);
		(void)Dpv14_Tx_API_Perform_HPD_WorkingFlow();
		break;
	case LINK_TEST_VID_ENCODING:
		if (ulElement >= (uint64_t)VIDEO_ENCODING_MAX) {
			print_link_help();

			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto return_funcs;
		}

		eEncodeType = (enum DPTX_VIDEO_ENCODING_TYPE)ulElement;

		(void)Dpv14_Tx_API_Perform_HPD_WorkingFlow();
		break;
	default:
		print_link_help();
		break;
	}

return_funcs:
	return  ret;
}

/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int do_dpv14_pll(struct cmd_tbl *cmdtp,
								int flag,
								int argc,
								char *const argv[])
{
	uint8_t ucPLL_Status;
	int32_t ret = 0;

	(void)cmdtp;
	(void)flag;
	(void)argc;

	if (strcmp(argv[1], "help") == 0) {
		print_pll_help();

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n **************[pll case]************");

	ret = Dpv14_Tx_API_Get_PLL_Status(&ucPLL_Status);
	if (ret != 0) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (ucPLL_Status != 0U) {
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		(void)pr_force("\nResult PASS\n");
	} else {
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		(void)pr_force("\nResult FAIL\n");
	}

	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n ****************[End]**************\n");

return_funcs:
	return ret;
}

static int32_t dpv14_set_unlock_protect_fail(bool bAccess,
															bool bCfgAccess)
{
	uint8_t ucIdx;
	int32_t ret = 0;
	uint32_t auiWriteBuf[NUM_OF_TESTS];
	uint32_t auiReadBuf[NUM_OF_TESTS];

	Dpv14_Tx_API_Set_CfgLock(true);
	Dpv14_Tx_API_Set_CfgAccess(bCfgAccess);

	for (ucIdx = 0; ucIdx < (uint8_t)NUM_OF_TESTS; ucIdx++) {
		if ((ucIdx % 2U ) == 0U) {
			/*For KCS*/
			auiWriteBuf[ucIdx] = 0x00005A5AU;
		} else {
			/*For KCS*/
			auiWriteBuf[ucIdx] = 0x5A5A0000U;
		}

		auiReadBuf[ucIdx] = 0x00U;
	}

	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n *********[regbank case 2]***********");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n Expect=> Write[0x1280000C]:0x00005A5A");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n	<-> Read[0x1280000C]:0xXXXXXXXX");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n	Write[0x12800010]:0x5A5A0000");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n	<-> Read[0x12800010]:0xXXXXXXXX");

	(void)Dpv14_Tx_API_Write_RegisterBank_Value(
								auiWriteBuf,
								(uint32_t)0x000C,
								(uint8_t)NUM_OF_TESTS);

	(void)Dpv14_Tx_API_Read_RegisterBank_Value(
								auiReadBuf,
								(uint32_t)0x000C,
								(uint8_t)NUM_OF_TESTS);

	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n\nActual=> W[0x1280000C]: 0x%08X", auiWriteBuf[0]);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n		 <-> R[0x1280000C]: 0x%08X", auiReadBuf[0]);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\nActual=> Write0x12800010]: 0x%08X", auiWriteBuf[1]);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n		  <-> R[0x12800010]: 0x%08X", auiReadBuf[1]);

	for (ucIdx = 0; ucIdx < (uint8_t)NUM_OF_TESTS; ucIdx++) {
		if (auiWriteBuf[ucIdx] == auiReadBuf[ucIdx]) {
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			(void)pr_force("\n\n Result FAIL\n");
			break;
		}
	}
	if (ucIdx == (uint8_t)NUM_OF_TESTS) {
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		(void)pr_force("\n\n Result => PASS ");
	}

	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n **************[End]************\n");

	for (ucIdx = 0; ucIdx < (uint8_t)NUM_OF_TESTS; ucIdx++) {
		/* For KCS */
		auiWriteBuf[ucIdx] = 0U;
	}

	(void)Dpv14_Tx_API_Write_RegisterBank_Value(
								auiWriteBuf,
								0x000C,
								(uint8_t)NUM_OF_TESTS);

	return ret;
}

static int32_t dpv14_set_unlock_protect_succ(void)
{
	uint8_t ucIdx;
	int32_t ret = 0;
	uint32_t auiWriteBuf[NUM_OF_TESTS];
	uint32_t auiReadBuf[NUM_OF_TESTS];

	Dpv14_Tx_API_Set_CfgLock(true);
	Dpv14_Tx_API_Set_CfgAccess(true);

	for (ucIdx = 0; ucIdx < (uint8_t)NUM_OF_TESTS; ucIdx++) {
		if ((ucIdx % 2U ) == 0U) {
			/*For KCS*/
			auiWriteBuf[ucIdx] = 0x00005A5AU;
		} else {
			/*For KCS*/
			auiWriteBuf[ucIdx] = 0x5A5A0000U;
		}

		auiReadBuf[ucIdx] = 0x00U;
	}

	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n *********[regbank case 1]***********");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n Expect=> Write[0x1280000C]:0x00005A5A");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n	<-> Read[0x1280000C]:0x00005A5A");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n	Write[0x12800010]:0x5A5A0000");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n	<-> Read[0x12800010]:0x5A5A0000");

	(void)Dpv14_Tx_API_Write_RegisterBank_Value(
								auiWriteBuf,
								(uint32_t)0x000C,
								(uint8_t)NUM_OF_TESTS);

	(void)Dpv14_Tx_API_Read_RegisterBank_Value(
								auiReadBuf,
								(uint32_t)0x000C,
								(uint8_t)NUM_OF_TESTS);

	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n\nActual=> W[0x1280000C]: 0x%08X", auiWriteBuf[0]);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n       <-> R[0x1280000C]: 0x%08X", auiReadBuf[0]);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\nActual=> Write0x12800010]: 0x%08X", auiWriteBuf[1]);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n        <-> R[0x12800010]: 0x%08X", auiReadBuf[1]);

	for (ucIdx = 0; ucIdx < (uint8_t)NUM_OF_TESTS; ucIdx++) {
		if (auiWriteBuf[ucIdx] != auiReadBuf[ucIdx]) {
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			(void)pr_force("\n\n Result FAIL\n");
			break;
		}
	}
	if (ucIdx == (uint8_t)NUM_OF_TESTS) {
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		(void)pr_force("\n\n Result => PASS ");
	}

	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n **************[End]************\n");

	for (ucIdx = 0; ucIdx < (uint8_t)NUM_OF_TESTS; ucIdx++) {
		/* For KCS */
		auiWriteBuf[ucIdx] = 0U;
	}

	(void)Dpv14_Tx_API_Write_RegisterBank_Value(
								auiWriteBuf,
								0x000C,
								(uint8_t)NUM_OF_TESTS);

	return ret;
}

/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int do_dpv14_register_bank(
					struct cmd_tbl *cmdtp,
					int flag,
					int argc,
					char *const argv[])
{
	int32_t ret = 0;
	uint64_t ulTestCase;
	enum PROTECT_TEST_TYPE eProtect_Test_Type;

	(void)cmdtp;
	(void)flag;
	(void)argc;

	if (strcmp(argv[1], "help") == 0) {
		print_regbank_help();

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	ulTestCase = simple_strtoul(argv[1], NULL, 10);
	if (ulTestCase >= (uint64_t)PROTECT_TEST_UNKNOWN) {
		print_regbank_help();

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	eProtect_Test_Type = (enum PROTECT_TEST_TYPE)ulTestCase;

	switch (eProtect_Test_Type) {
	case PROTECT_TEST_UNLOCK_SUCC:
		(void)dpv14_set_unlock_protect_succ();
		break;
	case PROTECT_TEST_UNLOCK_FAIL:
		(void)dpv14_set_unlock_protect_fail((bool)true, (bool)false);
		break;
	case PROTECT_TEST_ACCESS_MODE:
		(void)dpv14_set_unlock_protect_succ();
		(void)dpv14_set_unlock_protect_fail((bool)false, (bool)true);
		break;
	default:
		print_regbank_help();
		break;
	}

return_funcs:
	return ret;
}

/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int do_dpv14_read_edid(
						struct cmd_tbl *cmdtp,
						int flag,
						int argc,
						char *const argv[])
{
	uint8_t ucStreamIdx;
	int32_t ret = 0;
	uint64_t ulTestCase;
	struct DPTX_Dtd_Params_t stDptx_Dtd_Params;

	(void)cmdtp;
	(void)flag;
	(void)argc;

	if (strcmp(argv[1], "help") == 0) {
		print_read_edid_help();

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	ulTestCase = simple_strtoul(argv[1], NULL, 10);

	ucStreamIdx = (uint8_t)(ulTestCase & 0xFFU);
	if (ucStreamIdx >= (uint8_t)DPTX_INPUT_STREAM_MAX) {
		print_read_edid_help();

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	ret = Dpv14_Tx_API_Get_Dtd_From_Edid(&stDptx_Dtd_Params,
											ucStreamIdx);

	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("\n[Display timing set from EDID\n");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("	Pixel clk = %u\n",
			(u32)stDptx_Dtd_Params.uiPixel_Clock);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("	Repetition = %u\n",
			(u32)stDptx_Dtd_Params.pixel_repetition_input);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("	%s\n",
			((bool)stDptx_Dtd_Params.interlaced) ?
			"Interlace":"Progressive");
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("	H Sync Polarity(%u), V Sync Polarity(%u)\n",
			(u32)stDptx_Dtd_Params.h_sync_polarity,
			(u32)stDptx_Dtd_Params.h_sync_polarity);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("	H Active(%u), V Active(%u)\n",
			(u32)stDptx_Dtd_Params.h_active,
			(u32)stDptx_Dtd_Params.v_active);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("	H Image size(%u), V Image size(%u)\n",
			(u32)stDptx_Dtd_Params.h_image_size,
			(u32)stDptx_Dtd_Params.v_image_size);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("	H Blanking(%u), V Blanking(%u)\n",
			(u32)stDptx_Dtd_Params.h_blanking,
			(u32)stDptx_Dtd_Params.v_blanking);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("	H Sync offset(%u), V Sync offset(%u)\n",
			(u32)stDptx_Dtd_Params.h_sync_offset,
			(u32)stDptx_Dtd_Params.v_sync_offset);
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	(void)pr_force("	H Sync plus W(%u), V Sync plus W(%u)\n",
			(u32)stDptx_Dtd_Params.h_sync_pulse_width,
			(u32)stDptx_Dtd_Params.v_sync_pulse_width);

return_funcs:
	return  ret;
}

static int32_t dpv14_set_eq(enum CTS_TEST_TYPE Test_Type,
									uint32_t uiMain,
									uint32_t uiPost,
									uint32_t uiPre,
									uint32_t uiVboost,
									uint64_t ulTest_Idx)
{
	int32_t ret = 0;
	uint64_t ulNumOfEQTests;

	switch (Test_Type) {
	case CTS_TEST_EQ_SET_BY_LIST:
		ulNumOfEQTests = (uint64_t)(sizeof(stEQ_Tuning_t) / sizeof(struct DPV14_EQ_Tuning_t));

		if (ulTest_Idx >= ulNumOfEQTests) {
			print_cts_help();

			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto return_funcs;
		}

		ret = Dpv14_Tx_API_Set_EQ(stEQ_Tuning_t[ulTest_Idx].uiTx_EQ_Main,
									stEQ_Tuning_t[ulTest_Idx].uiTx_EQ_Post,
									stEQ_Tuning_t[ulTest_Idx].uiTx_EQ_Pre,
									stEQ_Tuning_t[ulTest_Idx].uiTx_EQ_VBoost);
		break;
	case CTS_TEST_EQ_SET:
		ret = Dpv14_Tx_API_Set_EQ(uiMain,
									uiPost,
									uiPre,
									uiVboost);
		break;
	default:
		print_cts_help();
		break;
	}

return_funcs:
	return ret;
}

static int32_t dpv14_set_preemp_vsw(enum CTS_TEST_TYPE Test_Type,
												uint8_t uclevel)
{
	int32_t ret = 0;

	switch (Test_Type) {
	case CTS_TEST_VSW_SET:
		ret = Dpv14_Tx_API_Set_LinkPrams(1U, uclevel);
		break;
	case CTS_TEST_PREEMP_SET:
		ret = Dpv14_Tx_API_Set_LinkPrams(0U, uclevel);
		break;
	default:
		print_cts_help();
		break;
	}

	return ret;
}

static int32_t dpv14_set_eye_rate(enum CTS_TEST_TYPE Test_Type, uint64_t ulTest_Idx)
{
	int32_t ret = 0;
	uint64_t ulNumOfHBRTests, ulNumOfHBR3Tests;

	ulNumOfHBRTests = (uint64_t)(sizeof(stRBR_HBR_Test_t) / sizeof(struct DPV14_CTS_Test_t));
	ulNumOfHBR3Tests = (uint64_t)(sizeof(stHBR23_Test_t) / sizeof(struct DPV14_CTS_Test_t));

	switch (Test_Type) {
	case CTS_TEST_RBR_HBR:
		if (ulTest_Idx >= ulNumOfHBRTests) {
			print_cts_help();

			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto return_funcs;
		}

		ret = Dpv14_Tx_API_Set_CTS(stRBR_HBR_Test_t[ulTest_Idx].bSSC_Enabled,
								stRBR_HBR_Test_t[ulTest_Idx].ucClk_Selection,
						(uint32_t)stRBR_HBR_Test_t[ulTest_Idx].ePattern_Type,
								stRBR_HBR_Test_t[ulTest_Idx].eLink_Rate,
								stRBR_HBR_Test_t[ulTest_Idx].ePreEmp,
								stRBR_HBR_Test_t[ulTest_Idx].eVSW);
		break;
	case CTS_TEST_HBR2_HBR3:
		if (ulTest_Idx >= ulNumOfHBR3Tests) {
			print_cts_help();

			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto return_funcs;
		}

		ret = Dpv14_Tx_API_Set_CTS(stHBR23_Test_t[ulTest_Idx].bSSC_Enabled,
								stHBR23_Test_t[ulTest_Idx].ucClk_Selection,
						(uint32_t)stHBR23_Test_t[ulTest_Idx].ePattern_Type,
								stHBR23_Test_t[ulTest_Idx].eLink_Rate,
								stHBR23_Test_t[ulTest_Idx].ePreEmp,
								stHBR23_Test_t[ulTest_Idx].eVSW);
		break;
	default:
		print_cts_help();
		break;
	}

return_funcs:
	return ret;
}

/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int do_dpv14_cts(struct cmd_tbl *cmdtp,
								int flag,
								int argc,
								char *const argv[])
{
	int32_t ret = 0;
	uint8_t ucLevel;
	uint32_t uiEQ_Main = 0, uiEQ_Post = 0, uiEQ_Pre = 0, uiEQ_Vboost = 0;
	uint64_t ulTestCase, ulTestIdx = 0;
	uint64_t ulEQ_Main, ulEQ_Post, ulEQ_Pre, ulEQ_Vboost;
	uint64_t ulLevel;
	enum CTS_TEST_TYPE Cts_Test_Type;

	(void)cmdtp;
	(void)flag;
	(void)argc;

	if (strcmp(argv[1], "help") == 0) {
		print_cts_help();

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	ulTestCase = simple_strtoul(argv[1], NULL, 10);

	if (ulTestCase >= (uint64_t)CTS_TEST_UNKNOWN) {
		print_cts_help();

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	Cts_Test_Type = (enum CTS_TEST_TYPE)ulTestCase;

	switch (Cts_Test_Type) {
	case CTS_TEST_RBR_HBR:
	case CTS_TEST_HBR2_HBR3:
		ulTestIdx = simple_strtoul(argv[2], NULL, 10);

		ret = dpv14_set_eye_rate(Cts_Test_Type, ulTestIdx);
		break;
	case CTS_TEST_EQ_SET_BY_LIST:
		ulTestIdx = simple_strtoul(argv[2], NULL, 10);

		ret = dpv14_set_eq(Cts_Test_Type,
						uiEQ_Main,
						uiEQ_Post,
						uiEQ_Pre,
						uiEQ_Vboost,
						ulTestIdx);
		break;
	case CTS_TEST_EQ_SET:
		ulEQ_Main = simple_strtoul(argv[2], NULL, 10);
		ulEQ_Post = simple_strtoul(argv[3], NULL, 10);
		ulEQ_Pre = simple_strtoul(argv[4], NULL, 10);
		ulEQ_Vboost = simple_strtoul(argv[5], NULL, 10);

		uiEQ_Main = (uint32_t)(ulEQ_Main & 0xFFFFFFFFU);
		uiEQ_Post = (uint32_t)(ulEQ_Post & 0xFFFFFFFFU);
		uiEQ_Pre = (uint32_t)(ulEQ_Pre & 0xFFFFFFFFU);
		uiEQ_Vboost = (uint32_t)(ulEQ_Vboost & 0xFFFFFFFFU);

		ret = dpv14_set_eq(Cts_Test_Type,
						uiEQ_Main,
						uiEQ_Post,
						uiEQ_Pre,
						uiEQ_Vboost,
						ulTestIdx);
		break;
	case CTS_TEST_VSW_SET:
	case CTS_TEST_PREEMP_SET:
		ulLevel = simple_strtoul(argv[2], NULL, 10);
		ucLevel = (uint8_t)(ulLevel & 0xFFU);

		ret = dpv14_set_preemp_vsw(Cts_Test_Type, ucLevel);
		break;
	default:
		print_cts_help();
		break;
	}

return_funcs:
	return ret;
}

static int32_t dpv14_set_power_ctrl_8050sv10(enum PANEL_GPIO_TYPE eGPIO_Type)
{
	int32_t ret = 0;

#if 0
	switch (eGPIO_Type) {
	case PANEL_1ST_GPIO_PWR_OFF:
		tcclcd_gpio_set_value(TCC_GPB(21U), 0);
		break;
	case PANEL_1ST_GPIO_PWR_ON:
		tcclcd_gpio_set_value(TCC_GPB(21U), 1);
		break;
	case PANEL_1ST_GPIO_BL_OFF:
		tcclcd_gpio_set_value(TCC_GPB(6U), 0);
		break;
	case PANEL_1ST_GPIO_BL_ON:
		tcclcd_gpio_set_value(TCC_GPB(6U), 1);
		break;
	case PANEL_1ST_GPIO_RST_OFF:
		tcclcd_gpio_set_value(TCC_GPB(6U), 0);
		break;
	case PANEL_1ST_GPIO_RST_ON:
		tcclcd_gpio_set_value(TCC_GPB(6U), 1);
		break;
	case PANEL_2ND_GPIO_BL_OFF:
		tcclcd_gpio_set_value(TCC_GPB(7U), 0);
		break;
	case PANEL_2ND_GPIO_BL_ON:
		tcclcd_gpio_set_value(TCC_GPB(7U), 1);
		break;
	case PANEL_2ND_GPIO_RST_OFF:
		tcclcd_gpio_set_value(TCC_GPB(12U), 0);
		break;
	case PANEL_2ND_GPIO_RST_ON:
		tcclcd_gpio_set_value(TCC_GPB(12U), 1);
		break;
	case PANEL_3RD_GPIO_BL_OFF:
		tcclcd_gpio_set_value(TCC_GPB(29U), 0);
		break;
	case PANEL_3RD_GPIO_BL_ON:
		tcclcd_gpio_set_value(TCC_GPB(28U), 1);
		break;
	case PANEL_3RD_GPIO_RST_OFF:
		tcclcd_gpio_set_value(TCC_GPB(8U), 0);
		break;
	case PANEL_3RD_GPIO_RST_ON:
		tcclcd_gpio_set_value(TCC_GPB(8U), 1);
		break;
	case PANEL_4TH_GPIO_BL_OFF:
		tcclcd_gpio_set_value(TCC_GPB(21U), 0);
		break;
	case PANEL_4TH_GPIO_BL_ON:
		tcclcd_gpio_set_value(TCC_GPB(21U), 1);
		break;
	case PANEL_4TH_GPIO_RST_OFF:
		tcclcd_gpio_set_value(TCC_GPB(25U), 0);
		break;
	case PANEL_4TH_GPIO_RST_ON:
		tcclcd_gpio_set_value(TCC_GPB(25U), 1);
		break;
	default:
		print_set_panel_power_help();
		break;
	}
#endif

	return ret;
}

static int32_t dpv14_set_power_ctrl_8050sv01(enum PANEL_GPIO_TYPE eGPIO_Type)
{
	int32_t ret = 0;

#if 0
	switch (eGPIO_Type) {
	case PANEL_1ST_GPIO_PWR_OFF:
		tcclcd_gpio_set_value(TCC_GPB(19U), 0);
		break;
	case PANEL_1ST_GPIO_PWR_ON:
		tcclcd_gpio_set_value(TCC_GPB(19U), 1);
		break;
	case PANEL_1ST_GPIO_BL_OFF:
		tcclcd_gpio_set_value(TCC_GPB(6U), 0);
		break;
	case PANEL_1ST_GPIO_BL_ON:
		tcclcd_gpio_set_value(TCC_GPB(6U), 1);
		break;
	case PANEL_1ST_GPIO_RST_OFF:
		tcclcd_gpio_set_value(TCC_GPB(20U), 0);
		break;
	case PANEL_1ST_GPIO_RST_ON:
		tcclcd_gpio_set_value(TCC_GPB(20U), 1);
		break;
	case PANEL_2ND_GPIO_BL_OFF:
		tcclcd_gpio_set_value(TCC_GPB(7U), 0);
		break;
	case PANEL_2ND_GPIO_BL_ON:
		tcclcd_gpio_set_value(TCC_GPB(7U), 1);
		break;
	case PANEL_2ND_GPIO_RST_OFF:
		tcclcd_gpio_set_value(TCC_GPB(9U), 0);
		break;
	case PANEL_2ND_GPIO_RST_ON:
		tcclcd_gpio_set_value(TCC_GPB(9U), 1);
		break;
	case PANEL_3RD_GPIO_BL_OFF:
		tcclcd_gpio_set_value(TCC_GPB(15U), 0);
		break;
	case PANEL_3RD_GPIO_BL_ON:
		tcclcd_gpio_set_value(TCC_GPB(15U), 1);
		break;
	case PANEL_3RD_GPIO_RST_OFF:
		tcclcd_gpio_set_value(TCC_GPB(18U), 0);
		break;
	case PANEL_3RD_GPIO_RST_ON:
		tcclcd_gpio_set_value(TCC_GPB(18U), 1);
		break;
	case PANEL_4TH_GPIO_BL_OFF:
		tcclcd_gpio_set_value(TCC_GPB(21U), 0);
		break;
	case PANEL_4TH_GPIO_BL_ON:
		tcclcd_gpio_set_value(TCC_GPB(21U), 1);
		break;
	case PANEL_4TH_GPIO_RST_OFF:
		tcclcd_gpio_set_value(TCC_GPB(19U), 0);
		break;
	case PANEL_4TH_GPIO_RST_ON:
		tcclcd_gpio_set_value(TCC_GPB(19U), 1);
		break;
	default:
		print_set_panel_power_help();
		break;
	}
#endif

	return ret;
}

static int32_t dpv14_set_power_ctrl_8059sv01(enum PANEL_GPIO_TYPE eGPIO_Type)
{
	int32_t ret = 0;

#if 0
	switch (eGPIO_Type) {
	case PANEL_1ST_GPIO_PWR_OFF:
		tcclcd_gpio_set_value(TCC_GPB(17U), 0);
		break;
	case PANEL_1ST_GPIO_PWR_ON:
		tcclcd_gpio_set_value(TCC_GPB(17U), 1);
		break;
	case PANEL_1ST_GPIO_BL_OFF:
		tcclcd_gpio_set_value(TCC_GPB(26U), 0);
		break;
	case PANEL_1ST_GPIO_BL_ON:
		tcclcd_gpio_set_value(TCC_GPB(26U), 1);
		break;
	case PANEL_1ST_GPIO_RST_OFF:
		tcclcd_gpio_set_value(TCC_GPB(18U), 0);
		break;
	case PANEL_1ST_GPIO_RST_ON:
		tcclcd_gpio_set_value(TCC_GPB(18U), 1);
		break;
	default:
		print_set_panel_power_help();
		break;
	}
#endif

	return ret;
}

/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int do_dpv14_set_panel_power(
					struct cmd_tbl *cmdtp,
					int flag,
					int argc,
					char *const argv[])
{
	int32_t ret = 0;
	unsigned long ulTestCase, ulPanelPower;
	enum TCC_EVB_TYPE eEvb_Type;
	enum PANEL_GPIO_TYPE ePanel_Gpio_Type;

	(void)cmdtp;
	(void)flag;
	(void)argc;

	if (strcmp(argv[1], "help") == 0) {
		print_set_panel_power_help();

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	ulTestCase = simple_strtoul(argv[1], NULL, 10);
	ulPanelPower = simple_strtoul(argv[2], NULL, 10);

	if (ulTestCase >= (unsigned long)TCC_EVB_TYPE_UNKNOWN) {
		print_set_panel_power_help();

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	eEvb_Type = (enum TCC_EVB_TYPE)(ulTestCase);

	if (ulPanelPower >= (unsigned long)PANEL_GPIO_UNKNOWN) {
		print_set_panel_power_help();

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	ePanel_Gpio_Type = (enum PANEL_GPIO_TYPE)(ulPanelPower);

	switch (eEvb_Type) {
	case TCC_EVB_8059_SV01:
		(void)dpv14_set_power_ctrl_8059sv01(ePanel_Gpio_Type);
		break;
	case TCC_EVB_8050_SV01:
		(void)dpv14_set_power_ctrl_8050sv01(ePanel_Gpio_Type);
		break;
	case TCC_EVB_8050_SV10:
		(void)dpv14_set_power_ctrl_8050sv10(ePanel_Gpio_Type);
		break;
	default:
		print_set_panel_power_help();
		break;
	}

return_funcs:
	return  ret;
}

static struct cmd_tbl cmd_dpv14_sub[] = {
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	U_BOOT_CMD_MKENT(link, 3, 0, do_dpv14_set_link, "", ""),
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	U_BOOT_CMD_MKENT(regbank, 3, 0, do_dpv14_register_bank, "", ""),
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	U_BOOT_CMD_MKENT(pll, 3, 0, do_dpv14_pll, "", ""),
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	U_BOOT_CMD_MKENT(ppwr, 3, 0, do_dpv14_set_panel_power, "", ""),
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	U_BOOT_CMD_MKENT(redid, 3, 0, do_dpv14_read_edid, "", ""),
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	U_BOOT_CMD_MKENT(cts, 3, 0, do_dpv14_cts, "", ""),
};

static int do_dpv14(
				struct cmd_tbl *cmdtp,
				int flag,
				int argc,
				char *const argv[])
{
	int ret = 0;
	int iTableLen;
	const struct cmd_tbl *c;

	(void)argv;

	if (argc < 2) {
		ret = (int)CMD_RET_USAGE;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	iTableLen = (int)(ARRAY_SIZE(cmd_dpv14_sub) & 0xFFFFFFFFU);
	c = find_cmd_tbl(argv[1], &cmd_dpv14_sub[0], iTableLen);

	if (c != NULL) {
		ret = c->cmd(cmdtp, flag, argc, argv);

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	} else {
		ret = (int)CMD_RET_USAGE;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
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
	"----------------------------------------------\r\n";


static char usage[] = "Verify dp v1.4 driver functionalities";


/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
/* coverity[misra_c_2012_rule_21_2_violation : FALSE] */
/* coverity[cert_dcl37_c_violation : FALSE] */
/* coverity[cert_str30_c_violation : FALSE] */
U_BOOT_CMD(
	dpv14, 7, 1, do_dpv14,
	usage,
	dpv14_help_text
);
