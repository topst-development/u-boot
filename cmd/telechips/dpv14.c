// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include "dptx_api.h"
#include "dptx_v14.h"


/* Version Information */
#define DPV14_CMD_MAJOR_VER	"1"
#define DPV14_CMD_MINOR_VER	"0"
#define DPV14_CMD_PATCH_VER	"1"

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
	LINK_TEST_UNKNOWN = LINK_TEST_VID_ENCODING
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

static const unsigned int dpctp_max_address = 0xFFFFFU;

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

static void dp_log(const char *fmt, ...)
{
        va_list va;

        va_start(va, fmt);
        (void)vprintf(fmt, va);
        va_end(va);
}

static void print_link_help(void)
{
	dp_log("dpv14 link <n><m> - Set cases \r\n"
			"		n - 0 : set Link rate \r\n"
			"				m - 0 :RBR  \r\n"
			"				m - 1 :HBR  \r\n"
			"				m - 2 :HBR2  \r\n"
			"				m - 3 :HBR3  \r\n"
			"		 n - 1 : set Link lane \r\n"
			"				m - 1 :1 lane  \r\n"
			"				m - 2 :2 lanes \r\n"
			"				m - 4 :4 lanes \r\n");
}

static void print_pll_help(void)
{
	dp_log("Verifying DP Pll locking\r\n");
	dp_log("dpv14 pll <n> - Set cases \r\n"
			"	n : N/A \r\n");
}

static void print_regbank_help(void)
{
	dp_log("Verifying DP Register Bank \r\n");
	dp_log("dpv14 regbank <n> - Set cases \r\n"
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
	dp_log("Verifying DP PHY Power on / off \r\n");
	dp_log("dpv14 power <n> - Set cases \r\n"
			"	n - 0 ~ 4 : Stream index 0 ~ 4\r\n");
}

static void print_cts_help(void)
{
	dp_log("Testing signal quality with CTS \r\n");
	dp_log("dpv14 cts <n> <m> - Set cases \r\n"
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
	dp_log("Panel power control\r\n");
	dp_log("dpv14 ppwr <n> - Set cases\r\n"
		"		 n - 0 : TCC8059 EVB SV01\r\n"
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


static int do_dpv14_set_link(struct cmd_tbl  *cmdtp, int flag,
			     int argc, char *const argv[])
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	unsigned long test_case, test_param;

	(void)cmdtp;
	(void)flag;

	if ((argc != 3) || (strcmp(argv[1], "help") == 0)) {
		rc = CMD_RET_USAGE;
	} else{
		test_case = simple_strtoul(argv[1], NULL, 10u);
		if (test_case >= (unsigned long)LINK_TEST_UNKNOWN) {
			rc = CMD_RET_USAGE;
		}
	}
	if (rc == CMD_RET_SUCCESS) {
		test_param = simple_strtoul(argv[2], NULL, 10u);

		switch (test_case) {
		case (unsigned long)LINK_TEST_RATE:
			if (test_param >= (unsigned long)DPTX_LINK_RATE_MAX) {
				rc = CMD_RET_USAGE;
			}
			if (rc == CMD_RET_SUCCESS) {
				(void)Dpv14_Tx_API_Set_MaxLinkRate_Supported((enum DPTX_LINK_RATE)test_param);
				(void)Dpv14_Tx_API_Perform_HPD_WorkingFlow();
			}
			break;
		case (unsigned long)LINK_TEST_LANE:
			if ((test_param != 1U) && (test_param != 2U) && (test_param != 4U)) {
				rc = CMD_RET_USAGE;
			}
			if (rc == CMD_RET_SUCCESS) {
				(void)Dpv14_Tx_API_Set_MaxLinkLane_Supported((enum DPTX_LINK_LANE)test_param);
				(void)Dpv14_Tx_API_Perform_HPD_WorkingFlow();
			}
			break;
		default:
			rc = CMD_RET_USAGE;
			break;
		}
	}
	if (rc != CMD_RET_SUCCESS) {
		print_link_help();
	}

	return (int)rc;
}

static int do_dpv14_pll(struct cmd_tbl *cmdtp, int flag,
			int argc, char *const argv[])
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	uint8_t ucPLL_Status;
	int ret;

	(void)cmdtp;
	(void)flag;
	(void)argc;

	if (strcmp(argv[1], "help") == 0) {
		rc = CMD_RET_USAGE;
	} else {
		dp_log("\n **************[pll case]************");
		ret = Dpv14_Tx_API_Get_PLL_Status(&ucPLL_Status);
		if (ret != 0) {
			rc = CMD_RET_USAGE;
		}
		dp_log("\n ****************[End]**************\n");
	}
	if (rc == CMD_RET_SUCCESS) {
		if (ucPLL_Status != 0U) {
			dp_log("\nResult PASS\n");
		} else {
			dp_log("\nResult FAIL\n");
		}
	}

	if (rc != CMD_RET_SUCCESS) {
		print_pll_help();
	}
	return (int)rc;
}

static int32_t dpv14_set_unlock_protect_fail(bool bAccess, bool bCfgAccess)
{
	uint32_t auiWriteBuf[NUM_OF_TESTS];
	uint32_t auiReadBuf[NUM_OF_TESTS];
	int32_t ret = 0;
	uint8_t ucIdx;

	(void)bAccess;

	(void)Dpv14_Tx_API_Set_CfgLock(true);
	(void)Dpv14_Tx_API_Set_CfgAccess(bCfgAccess);

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

	dp_log("\n *********[regbank case 2]***********");
	dp_log("\n Expect=> Write[0x1280000C]:0x00005A5A");
	dp_log("\n	<-> Read[0x1280000C]:0xXXXXXXXX");
	dp_log("\n	Write[0x12800010]:0x5A5A0000");
	dp_log("\n	<-> Read[0x12800010]:0xXXXXXXXX");

	(void)Dpv14_Tx_API_Write_RegisterBank_Value(auiWriteBuf,
						    (uint32_t)0x000C,
						    (uint8_t)NUM_OF_TESTS);

	(void)Dpv14_Tx_API_Read_RegisterBank_Value(auiReadBuf,
						   (uint32_t)0x000C,
						   (uint8_t)NUM_OF_TESTS);

	dp_log("\n\nActual=> W[0x1280000C]: 0x%08X", auiWriteBuf[0]);
	dp_log("\n		 <-> R[0x1280000C]: 0x%08X", auiReadBuf[0]);
	dp_log("\nActual=> Write0x12800010]: 0x%08X", auiWriteBuf[1]);
	dp_log("\n		  <-> R[0x12800010]: 0x%08X", auiReadBuf[1]);

	for (ucIdx = 0; ucIdx < (uint8_t)NUM_OF_TESTS; ucIdx++) {
		if (auiWriteBuf[ucIdx] == auiReadBuf[ucIdx]) {
			dp_log("\n\n Result FAIL\n");
			break;
		}
	}
	if (ucIdx == (uint8_t)NUM_OF_TESTS) {
		dp_log("\n\n Result => PASS ");
	}

	dp_log("\n **************[End]************\n");

	for (ucIdx = 0; ucIdx < (uint8_t)NUM_OF_TESTS; ucIdx++) {
		/* For KCS */
		auiWriteBuf[ucIdx] = 0U;
	}

	(void)Dpv14_Tx_API_Write_RegisterBank_Value(auiWriteBuf,
						    0x000C, (uint8_t)NUM_OF_TESTS);

	return ret;
}

static int32_t dpv14_set_unlock_protect_succ(void)
{
	uint8_t ucIdx;
	int32_t ret = 0;
	uint32_t auiWriteBuf[NUM_OF_TESTS];
	uint32_t auiReadBuf[NUM_OF_TESTS];

	(void)Dpv14_Tx_API_Set_CfgLock(true);
	(void)Dpv14_Tx_API_Set_CfgAccess(true);

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

	dp_log("\n *********[regbank case 1]***********");
	dp_log("\n Expect=> Write[0x1280000C]:0x00005A5A");
	dp_log("\n	<-> Read[0x1280000C]:0x00005A5A");
	dp_log("\n	Write[0x12800010]:0x5A5A0000");
	dp_log("\n	<-> Read[0x12800010]:0x5A5A0000");

	(void)Dpv14_Tx_API_Write_RegisterBank_Value(auiWriteBuf,
						    (uint32_t)0x000C,
						    (uint8_t)NUM_OF_TESTS);

	(void)Dpv14_Tx_API_Read_RegisterBank_Value(auiReadBuf,
						   (uint32_t)0x000C,
						   (uint8_t)NUM_OF_TESTS);

	dp_log("\n\nActual=> W[0x1280000C]: 0x%08X", auiWriteBuf[0]);
	dp_log("\n       <-> R[0x1280000C]: 0x%08X", auiReadBuf[0]);
	dp_log("\nActual=> Write0x12800010]: 0x%08X", auiWriteBuf[1]);
	dp_log("\n        <-> R[0x12800010]: 0x%08X", auiReadBuf[1]);

	for (ucIdx = 0; ucIdx < (uint8_t)NUM_OF_TESTS; ucIdx++) {
		if (auiWriteBuf[ucIdx] != auiReadBuf[ucIdx]) {
			dp_log("\n\n Result FAIL\n");
			break;
		}
	}
	if (ucIdx == (uint8_t)NUM_OF_TESTS) {
		dp_log("\n\n Result => PASS ");
	}

	dp_log("\n **************[End]************\n");

	for (ucIdx = 0; ucIdx < (uint8_t)NUM_OF_TESTS; ucIdx++) {
		/* For KCS */
		auiWriteBuf[ucIdx] = 0U;
	}

	(void)Dpv14_Tx_API_Write_RegisterBank_Value(auiWriteBuf,
						    0x000C, (uint8_t)NUM_OF_TESTS);

	return ret;
}

static int do_dpv14_register_bank(struct cmd_tbl *cmdtp,
				  int flag, int argc, char *const argv[])
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	uint64_t ulTestCase;
	enum PROTECT_TEST_TYPE eProtect_Test_Type;

	(void)cmdtp;
	(void)flag;
	(void)argc;

	if (strcmp(argv[1], "help") == 0) {
		rc = CMD_RET_USAGE;
	} else {
		ulTestCase = simple_strtoul(argv[1], NULL, 10);
		if (ulTestCase >= (uint64_t)PROTECT_TEST_UNKNOWN) {
			rc = CMD_RET_USAGE;
		}
	}
	if (rc == CMD_RET_SUCCESS) {
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
			rc = CMD_RET_USAGE;
			break;
		}
	}
	if (rc != CMD_RET_SUCCESS) {
		print_regbank_help();
	}
	return (int)rc;
}

static int do_dpv14_read_edid(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	uint8_t ucStreamIdx;
	uint64_t ulTestCase;
	struct DPTX_Dtd_Params_t stDptx_Dtd_Params;

	(void)cmdtp;
	(void)flag;
	(void)argc;

	if (strcmp(argv[1], "help") == 0) {
		rc = CMD_RET_USAGE;
	} else {
		ulTestCase = simple_strtoul(argv[1], NULL, 10);

		ucStreamIdx = (uint8_t)(ulTestCase & 0xFFU);
		if (ucStreamIdx >= (uint8_t)DPTX_INPUT_STREAM_MAX) {
			rc = CMD_RET_USAGE;
		}
	}
	if (rc == CMD_RET_SUCCESS) {
		(void)memset(&stDptx_Dtd_Params, 0, sizeof(stDptx_Dtd_Params));
		(void)Dpv14_Tx_API_Get_Dtd_From_Edid(&stDptx_Dtd_Params, ucStreamIdx);

		dp_log("\n[Display timing set from EDID\n");
		dp_log("	Pixel clk = %u\n",
				(u32)stDptx_Dtd_Params.uiPixel_Clock);
		dp_log("	Repetition = %u\n",
				(u32)stDptx_Dtd_Params.pixel_repetition_input);
		dp_log("	%s\n",
				((bool)stDptx_Dtd_Params.interlaced) ?
				"Interlace":"Progressive");
		dp_log("	H Sync Polarity(%u), V Sync Polarity(%u)\n",
				(u32)stDptx_Dtd_Params.h_sync_polarity,
				(u32)stDptx_Dtd_Params.h_sync_polarity);
		dp_log("	H Active(%u), V Active(%u)\n",
				(u32)stDptx_Dtd_Params.h_active,
				(u32)stDptx_Dtd_Params.v_active);
		dp_log("	H Image size(%u), V Image size(%u)\n",
				(u32)stDptx_Dtd_Params.h_image_size,
				(u32)stDptx_Dtd_Params.v_image_size);
		dp_log("	H Blanking(%u), V Blanking(%u)\n",
				(u32)stDptx_Dtd_Params.h_blanking,
				(u32)stDptx_Dtd_Params.v_blanking);
		dp_log("	H Sync offset(%u), V Sync offset(%u)\n",
				(u32)stDptx_Dtd_Params.h_sync_offset,
				(u32)stDptx_Dtd_Params.v_sync_offset);
		dp_log("	H Sync plus W(%u), V Sync plus W(%u)\n",
				(u32)stDptx_Dtd_Params.h_sync_pulse_width,
				(u32)stDptx_Dtd_Params.v_sync_pulse_width);
	}

	if (rc != CMD_RET_SUCCESS) {
		print_read_edid_help();
	}
	return (int)rc;
}

static int32_t dpv14_set_eq(enum CTS_TEST_TYPE Test_Type,
									uint32_t uiMain,
									uint32_t uiPost,
									uint32_t uiPre,
									uint32_t uiVboost,
									uint64_t ulTest_Idx)
{
	uint64_t ulNumOfEQTests;
	int32_t ret = 0;

	switch (Test_Type) {
	case CTS_TEST_EQ_SET_BY_LIST:
		ulNumOfEQTests = (uint64_t)(sizeof(stEQ_Tuning_t) / sizeof(struct DPV14_EQ_Tuning_t));

		if (ulTest_Idx >= ulNumOfEQTests) {
			ret = -EINVAL;
		} else {
			ret = Dpv14_Tx_API_Set_EQ(stEQ_Tuning_t[ulTest_Idx].uiTx_EQ_Main,
						  stEQ_Tuning_t[ulTest_Idx].uiTx_EQ_Post,
						  stEQ_Tuning_t[ulTest_Idx].uiTx_EQ_Pre,
						  stEQ_Tuning_t[ulTest_Idx].uiTx_EQ_VBoost);
		}
		break;
	case CTS_TEST_EQ_SET:
		ret = Dpv14_Tx_API_Set_EQ(uiMain, uiPost, uiPre, uiVboost);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (ret != 0) {
		print_cts_help();
	}
	return ret;
}

static int32_t dpv14_set_preemp_vsw(enum CTS_TEST_TYPE Test_Type, uint8_t uclevel)
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
	uint64_t ulNumOfHBRTests, ulNumOfHBR3Tests;
	int32_t ret = 0;

	ulNumOfHBRTests = (uint64_t)(sizeof(stRBR_HBR_Test_t) / sizeof(struct DPV14_CTS_Test_t));
	ulNumOfHBR3Tests = (uint64_t)(sizeof(stHBR23_Test_t) / sizeof(struct DPV14_CTS_Test_t));

	switch (Test_Type) {
	case CTS_TEST_RBR_HBR:
		if (ulTest_Idx >= ulNumOfHBRTests) {
			ret = -EINVAL;
		} else {
			ret = Dpv14_Tx_API_Set_CTS(stRBR_HBR_Test_t[ulTest_Idx].bSSC_Enabled,
						   stRBR_HBR_Test_t[ulTest_Idx].ucClk_Selection,
						   (uint32_t)stRBR_HBR_Test_t[ulTest_Idx].ePattern_Type,
						   stRBR_HBR_Test_t[ulTest_Idx].eLink_Rate,
						   stRBR_HBR_Test_t[ulTest_Idx].ePreEmp,
						   stRBR_HBR_Test_t[ulTest_Idx].eVSW);
		}
		break;
	case CTS_TEST_HBR2_HBR3:
		if (ulTest_Idx >= ulNumOfHBR3Tests) {
			ret = -EINVAL;
		} else {
			ret = Dpv14_Tx_API_Set_CTS(stHBR23_Test_t[ulTest_Idx].bSSC_Enabled,
						   stHBR23_Test_t[ulTest_Idx].ucClk_Selection,
						   (uint32_t)stHBR23_Test_t[ulTest_Idx].ePattern_Type,
						   stHBR23_Test_t[ulTest_Idx].eLink_Rate,
						   stHBR23_Test_t[ulTest_Idx].ePreEmp,
						   stHBR23_Test_t[ulTest_Idx].eVSW);
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (ret != 0) {
		print_cts_help();
	}
	return ret;
}

static int do_dpv14_cts(struct cmd_tbl *cmdtp, int flag,
			int argc, char *const argv[])
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	uint32_t uiEQ_Main = 0, uiEQ_Post = 0, uiEQ_Pre = 0, uiEQ_Vboost = 0;
	uint64_t ulTestCase, ulTestIdx = 0;
	uint64_t ulEQ_Main, ulEQ_Post, ulEQ_Pre, ulEQ_Vboost;
	uint64_t ulLevel;
	uint8_t ucLevel;
	enum CTS_TEST_TYPE Cts_Test_Type;

	(void)cmdtp;
	(void)flag;
	(void)argc;

	if (strcmp(argv[1], "help") == 0) {
		rc = CMD_RET_USAGE;
	} else {
		ulTestCase = simple_strtoul(argv[1], NULL, 10);

		if (ulTestCase >= (uint64_t)CTS_TEST_UNKNOWN) {
			rc = CMD_RET_USAGE;
		}
	}
	if (rc == CMD_RET_SUCCESS) {
		Cts_Test_Type = (enum CTS_TEST_TYPE)ulTestCase;

		switch (Cts_Test_Type) {
		case CTS_TEST_RBR_HBR:
		case CTS_TEST_HBR2_HBR3:
			ulTestIdx = simple_strtoul(argv[2], NULL, 10);

			(void)dpv14_set_eye_rate(Cts_Test_Type, ulTestIdx);
			break;
		case CTS_TEST_EQ_SET_BY_LIST:
			ulTestIdx = simple_strtoul(argv[2], NULL, 10);

			(void)dpv14_set_eq(Cts_Test_Type, uiEQ_Main,
					   uiEQ_Post, uiEQ_Pre,
					   uiEQ_Vboost, ulTestIdx);
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

			(void)dpv14_set_eq(Cts_Test_Type, uiEQ_Main,
					   uiEQ_Post, uiEQ_Pre,
					   uiEQ_Vboost, ulTestIdx);
			break;
		case CTS_TEST_VSW_SET:
		case CTS_TEST_PREEMP_SET:
			ulLevel = simple_strtoul(argv[2], NULL, 10);
			ucLevel = (uint8_t)(ulLevel & 0xFFU);

			(void)dpv14_set_preemp_vsw(Cts_Test_Type, ucLevel);
			break;
		default:
			rc = CMD_RET_USAGE;
			break;
		}
	}
	if (rc != CMD_RET_SUCCESS) {
		print_cts_help();
	}
	return (int)rc;
}

static int32_t dpv14_set_power_ctrl_8050sv10(enum PANEL_GPIO_TYPE eGPIO_Type)
{
	int32_t ret = 0;

	(void)eGPIO_Type;

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

	(void)eGPIO_Type;
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

	(void)eGPIO_Type;

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

static int do_dpv14_set_panel_power(struct cmd_tbl *cmdtp,
				    int flag, int argc,
				    char *const argv[])
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	unsigned long ulTestCase, ulPanelPower;
	enum PANEL_GPIO_TYPE ePanel_Gpio_Type;
	enum TCC_EVB_TYPE eEvb_Type;

	(void)cmdtp;
	(void)flag;
	(void)argc;

	if (strcmp(argv[1], "help") == 0) {
		rc = CMD_RET_FAILURE;
	} else {
		ulTestCase = simple_strtoul(argv[1], NULL, 10);
		ulPanelPower = simple_strtoul(argv[2], NULL, 10);

		if (ulTestCase >= (unsigned long)TCC_EVB_TYPE_UNKNOWN) {
			rc = CMD_RET_FAILURE;
		}
	}
	if (rc == CMD_RET_SUCCESS) {
		eEvb_Type = (enum TCC_EVB_TYPE)(ulTestCase);

		if (ulPanelPower >= (unsigned long)PANEL_GPIO_UNKNOWN) {
			rc = CMD_RET_FAILURE;
		}
	}
	if (rc == CMD_RET_SUCCESS) {
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
			rc = CMD_RET_FAILURE;
			break;
		}
	}
	if (rc == CMD_RET_SUCCESS) {
		print_set_panel_power_help();
	}
	return (int)rc;
}


static void dpv14_dpcd_usage(void)
{
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	dp_log("dpv14 dpcd [r|w] <address> <value> \r\n"
			" r       : read\r\n"
			" w       : write\r\n"
			" address : dpcd address (00000F ~ 0xFFFFF) \r\n"
			" value   : \r\n"
			"           Incase of read mode value means the number of registers to read.\r\n"
			"           Otherwise, the value means the value written to the register.\r\n");
}

static int dpv14_dpcd_dump(struct Dptx_Params *dptx_params,
				unsigned int address, unsigned int count)
{
	unsigned char data;
	unsigned int loop;
	int ret = 0;

	for(loop = 0u; loop < count; loop++, address++) {
		if (Dptx_Aux_Read_DPCD(dptx_params, address, &data) != DPTX_RETURN_NO_ERROR) {
			ret = -ENODEV;
		}
		if (address > dpctp_max_address) {
			ret = -EINVAL;
		}
		if (ret < 0) {
			break;
		}
		if((loop % 16u) == 0u) {
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			dp_log("\r\n0x%05x# ", address);
		}
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		dp_log("0x%02x ", data);
	}
	dp_log("\r\n");
	return ret;
}

/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int do_dpv14_dpcd(struct cmd_tbl *cmdtp, int flag, int argc,
			 		char *const argv[])
{
	bool read_mode;

	enum command_ret_t rc = CMD_RET_SUCCESS;
	struct Dptx_Params *dptx_params;
	unsigned int address, count;
	unsigned long options[2];
	unsigned char data;

	char *const *sub_argv = &argv[0];
	int sub_argc = argc -1;

	(void)cmdtp;
	(void)flag;

	if (sub_argc != 3) {
		rc = CMD_RET_FAILURE;
	} else {
		if ((strcmp(sub_argv[1], "r") == 0) || (strcmp(sub_argv[1], "R") == 0)) {
			read_mode = (bool)true;
		} else if ((strcmp(sub_argv[1], "w") == 0) || (strcmp(sub_argv[1], "W") == 0)) {
			read_mode = (bool)false;
		} else {
			rc = CMD_RET_USAGE;
		}
	}
	if (rc == CMD_RET_SUCCESS) {
		options[0] = simple_strtoul(sub_argv[2], NULL, 16);
		options[1] = simple_strtoul(sub_argv[3], NULL, 16);

		if (options[0] > (unsigned long)dpctp_max_address) {
			rc = CMD_RET_USAGE;
		} else {
			address = (unsigned int)options[0];
		}
	}
	if (rc == CMD_RET_SUCCESS) {
		if (read_mode) {
			if (options[1] > (unsigned long)(dpctp_max_address - 1u)) {
				rc = CMD_RET_USAGE;
			} else {
				count = (unsigned int)options[1];
			}
			if (rc == CMD_RET_SUCCESS) {
				if ((address + count) > dpctp_max_address) {
					rc = CMD_RET_USAGE;
				}
			}
		} else {
			if (options[1] >= (unsigned long)0x100U) {
				rc = CMD_RET_USAGE;
			} else {
				data = (unsigned char)options[1];
			}
		}
	}
	if (rc == CMD_RET_SUCCESS) {
		dptx_params = Dptx_V14_Get_Device_Handle();
		if (dptx_params == NULL) {
			rc = CMD_RET_FAILURE;
		}
	}
	if (rc == CMD_RET_SUCCESS) {
		if (read_mode) {
			if (dpv14_dpcd_dump(dptx_params, address, count) < 0) {
				rc = CMD_RET_FAILURE;
			}
		} else {
			if (Dptx_Aux_Write_DPCD(dptx_params, address, data) < 0) {
				rc = CMD_RET_FAILURE;
			}
		}
	}
	if (rc != CMD_RET_SUCCESS) {
		dpv14_dpcd_usage();
	}
	return (int)rc;
}

static struct cmd_tbl cmd_dpv14_sub[] = {
	U_BOOT_CMD_MKENT(link, 3, 0, do_dpv14_set_link, "", ""),
	U_BOOT_CMD_MKENT(regbank, 3, 0, do_dpv14_register_bank, "", ""),
	U_BOOT_CMD_MKENT(pll, 3, 0, do_dpv14_pll, "", ""),
	U_BOOT_CMD_MKENT(ppwr, 3, 0, do_dpv14_set_panel_power, "", ""),
	U_BOOT_CMD_MKENT(redid, 3, 0, do_dpv14_read_edid, "", ""),
	U_BOOT_CMD_MKENT(cts, 3, 0, do_dpv14_cts, "", ""),
	U_BOOT_CMD_MKENT(dpcd, 3, 0, do_dpv14_dpcd, "", ""),
};

static int do_dpv14(struct cmd_tbl *cmdtp,
				   int flag, int argc, char *const argv[])
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	int table_len;
	const struct cmd_tbl *c;

	(void)argv;

	if (argc < 2) {
		rc = CMD_RET_USAGE;
	} else {

		/* Strip off leading argument */
		argc--;
		argv++;

		table_len = (int)(ARRAY_SIZE(cmd_dpv14_sub) & 0x7FFFFFFFu);
		c = find_cmd_tbl(argv[0], &cmd_dpv14_sub[0], table_len);

		if (c != NULL) {
			rc = (enum command_ret_t)c->cmd(cmdtp, flag, argc, argv);
		} else {
			rc = CMD_RET_USAGE;
		}
	}
	return (int)rc;

}

/***************************************************/
static char dpv14_help_text[] =
	"---------------------------------------------\r\n"
	"dpv14 lcd driver init \r\n"
	"dpv14 power \r\n"
	"dpv14 regbank \r\n"
	"dpv14 pll \r\n"
	"dpv14 link \r\n"
	"dpv14 colormetry mapping        \r\n"
	"dpv14 video code \r\n"
	"dpv14 video mute \r\n"
	"dpv14 colorspace \r\n"
	"dpv14 register dump \r\n"
	"dpv14 eq \r\n"
	"dpv14 cts \r\n"
	"dpv14 dpcd \r\n"
	"dpv14 redid( read edid ) \r\n"
	"----------------------------------------------\r\n";


static char usage[] = "DP cmd v"DPV14_CMD_MAJOR_VER "." DPV14_CMD_MINOR_VER "." DPV14_CMD_PATCH_VER " supports to verify dp v1.4 driver functionalities"  ;


U_BOOT_CMD(
	dpv14, 7, 1, do_dpv14,
	usage,
	dpv14_help_text
);
