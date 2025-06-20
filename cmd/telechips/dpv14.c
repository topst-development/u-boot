// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include "dptx_api.h"
#include "dptx_v14.h"

#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
#include <linux/virtual_dbg.h>
#endif

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
	LINK_TEST_SSC = 2,
	LINK_TEST_UNKNOWN
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
	(void)pr_force("dpv14 link <category> <value>\n"
	" category:\n"
	"  - rate : set link rate\n"
	"  - lane : set link lane\n"
	"  - ssc : set spread specturm clock\n");

	(void)pr_force(" value (for 'link rate' category):\n"
	"  - 0 : Reduced Bit Rate\n"
	"  - 1 : High Bit Rate\n"
	"  - 2 : High Bit Rate 2\n"
	"  - 3 : High Bit Rate 3\n"
	"  >>> eg) dpv14 rate 3\n\n");

	(void)pr_force(" value (for 'link lane' category):\n"
	"  - 1 : 1 lane\n"
	"  - 2 : 2 lanes\n"
	"  - 4 : 4 lanes\n"
	"  >>> eg) dpv14 lane 4\n\n");

	(void)pr_force(" value (for 'ssc' category):\n"
	"  - 0 : Disable spread specturm clock\n"
	"  - 1 : Enable spread specturm clock\n"
	"  >>> eg) dpv14 ssc 1\n\n");
}

static void print_updated_link_status(void)
{
	uint32_t link_rate = dpv14_api_get_current_link_rate();
	uint32_t link_lane = dpv14_api_get_current_link_lane();
	bool ssc_en = dpv14_api_get_current_ssc_en();

	(void)pr_force("Link Status\n");
	(void)pr_force(" Link Rate = %s\n",
		(link_rate ==  (uint32_t)LINK_RATE_RBR)? "rbr " :
		(link_rate ==  (uint32_t)LINK_RATE_HBR)? "hbr " :
		(link_rate ==  (uint32_t)LINK_RATE_HBR2)? "hbr2" : "hbr3");
	(void)pr_force(" Link Lane = %u\n", link_lane);
	(void)pr_force(" Link ssc  = %s\n", ssc_en ? "enabled" : "disabled");
	(void)pr_force("\n");
}

static void print_pll_help(void)
{
	(void)pr_force("Verifying DP Pll locking\n");
	(void)pr_force("dpv14 pll <n> - Set cases \n"
			"	n : N/A \n");
}

static void print_regbank_help(void)
{
	(void)pr_force("Verifying DP Register Bank \n");
	(void)pr_force("dpv14 regbank <n> - Set cases \n"
			"	n - 0 : dp_cfg_pw_ok + \n"
			"			dp_cfg_pw_lock(unlock) + \n"
			"			dp_cfg_access(unlock) \n"
			"		1 : dp_cfg_pw_ok( PW writing ) + \n"
			"			dp_cfg_pw_lock( unlock ) + \n"
			"			dp_cfg_access( lock ) \n"
			"		2 : dp_cfg_access( unlock -> lock ) \n");
}

static void print_read_edid_help(void)
{
	(void)pr_force("Verifying DP PHY Power on / off \n");
	(void)pr_force("dpv14 power <n> - Set cases \n"
			"	n - 0 ~ 4 : Stream index 0 ~ 4\n");
}

static void print_cts_help(void)
{
	(void)pr_force("Testing signal quality with CTS \n");
	(void)pr_force("dpv14 cts list - show cts lists \n");
	(void)pr_force("dpv14 cts <n> <m> - Set cases \n"
			"				n - 0  : CTS Test for RBR & HBR   \n"
			"					m - 1~ : Test items \n"
			"				n - 1  : CTS Test for HBR2 & HBR3   \n"
			"					m - 1~ : Test items \n");
}

static void print_cts_list(void)
{
	size_t num_of_rbr_tests, num_of_hbr3_tests;
	size_t loop;

	num_of_rbr_tests = (uint64_t)(sizeof(stRBR_HBR_Test_t) / sizeof(struct DPV14_CTS_Test_t));
	num_of_hbr3_tests = (uint64_t)(sizeof(stHBR23_Test_t) / sizeof(struct DPV14_CTS_Test_t));

	(void)pr_force("CTS List\n\n");
	(void)pr_force("RBR & HBR TEST\n");
	(void)pr_force("INDEX SSC  LINK-RATE VOLTAGE PREEMP PATTERN\n");
	(void)pr_force("-------------------------------------------\n");

	for (loop = 0u; loop < num_of_rbr_tests; loop++) {
		(void)pr_force("[%03lu]  %s     %s     %03u   %03u   %03u\n",
			loop, stRBR_HBR_Test_t[loop].bSSC_Enabled ? "EN " : "DIS",
			(stRBR_HBR_Test_t[loop].eLink_Rate == DPTX_LINK_RATE_RBR) ? "RBR " :
			(stRBR_HBR_Test_t[loop].eLink_Rate == DPTX_LINK_RATE_HBR) ? "HBR " :
			(stRBR_HBR_Test_t[loop].eLink_Rate == DPTX_LINK_RATE_HBR2) ? "HBR2" : "HBR3",
			stRBR_HBR_Test_t[loop].eVSW,
			stRBR_HBR_Test_t[loop].ePreEmp,
			(uint32_t)stRBR_HBR_Test_t[loop].ePattern_Type);
	}
	(void)pr_force("-------------------------------------------\n\n");

	(void)pr_force("HBR2 & HBR3 TEST\n");
	(void)pr_force("INDEX SSC  LINK-RATE VOLTAGE PREEMP PATTERN\n");
	(void)pr_force("-------------------------------------------\n");

	for (loop = 0u; loop < num_of_hbr3_tests; loop++) {
		(void)pr_force("[%03lu]  %s     %s     %03u   %03u   %03u\n",
		loop, stHBR23_Test_t[loop].bSSC_Enabled ? "EN " : "DIS",
		(stHBR23_Test_t[loop].eLink_Rate == DPTX_LINK_RATE_RBR) ? "RBR " :
		(stHBR23_Test_t[loop].eLink_Rate == DPTX_LINK_RATE_HBR) ? "HBR " :
		(stHBR23_Test_t[loop].eLink_Rate == DPTX_LINK_RATE_HBR2) ? "HBR2" : "HBR3",
		stHBR23_Test_t[loop].eVSW,
		stHBR23_Test_t[loop].ePreEmp,
		(uint32_t)stHBR23_Test_t[loop].ePattern_Type);
	}
	(void)pr_force("-------------------------------------------\n");
}

static int do_dpv14_set_link(struct cmd_tbl  *cmdtp, int flag,
			     int argc, char *const argv[])
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	unsigned long test_case, test_param;

	(void)cmdtp;
	(void)flag;

	if ((argc == 2) && (strcmp(argv[1], "help") == 0)) {
		rc = CMD_RET_FAILURE;
	} else if (argc != 3) {
		rc = CMD_RET_FAILURE;
	} else{
		if (strcmp(argv[1], "rate") == 0) {
			test_case = (unsigned long)LINK_TEST_RATE;
		} else if (strcmp(argv[1], "lane") == 0) {
			test_case = (unsigned long)LINK_TEST_LANE;
		} else if (strcmp(argv[1], "ssc") == 0) {
			test_case = (unsigned long)LINK_TEST_SSC;
		} else {
			test_case = (unsigned long)LINK_TEST_UNKNOWN;
		}
	}
	if (rc == CMD_RET_SUCCESS) {
		test_param = simple_strtoul(argv[2], NULL, 10u);

		switch (test_case) {
		case (unsigned long)LINK_TEST_RATE:
			if (test_param >= (unsigned long)DPTX_LINK_RATE_MAX) {
				rc = CMD_RET_FAILURE;
			}
			if (rc == CMD_RET_SUCCESS) {
				#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
				trace_set_name("dp_link_rate");
				#endif
				(void)Dpv14_Tx_API_Set_MaxLinkRate_Supported((enum DPTX_LINK_RATE)test_param);
				(void)Dpv14_Tx_API_Perform_HPD_WorkingFlow();
				#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
				trace_set_name("unknown");
				#endif
			}
			break;
		case (unsigned long)LINK_TEST_LANE:
			if ((test_param != 1U) && (test_param != 2U) && (test_param != 4U)) {
				rc = CMD_RET_FAILURE;
			}
			if (rc == CMD_RET_SUCCESS) {
				#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
				trace_set_name("dp_link_lane");
				#endif
				(void)Dpv14_Tx_API_Set_MaxLinkLane_Supported((enum DPTX_LINK_LANE)test_param);
				(void)Dpv14_Tx_API_Perform_HPD_WorkingFlow();
				#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
				trace_set_name("unknown");
				#endif
			}
			break;
		case (unsigned long)LINK_TEST_SSC:
			if ((test_param != 0U) && (test_param != 1U)) {
				rc = CMD_RET_FAILURE;
			}
			if (rc == CMD_RET_SUCCESS) {
				#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
				trace_set_name("dp_link_ssc");
				#endif
				if (test_param == 1u) {
					(void)dpv14_tx_api_set_spread_specturm_clock((bool)true);
				} else {
					(void)dpv14_tx_api_set_spread_specturm_clock((bool)false);
				}
				(void)Dpv14_Tx_API_Perform_HPD_WorkingFlow();
				#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
				trace_set_name("unknown");
				#endif
			}
			break;
		default:
			rc = CMD_RET_FAILURE;
			break;
		}
	}
	if (rc != CMD_RET_SUCCESS) {
		print_link_help();
	}

	print_updated_link_status();

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
		(void)pr_force("\n **************[pll case]************");
		#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
		trace_set_name("dp_pll");
		#endif
		ret = Dpv14_Tx_API_Get_PLL_Status(&ucPLL_Status);
		if (ret != 0) {
			rc = CMD_RET_USAGE;
		}
		#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
		trace_set_name("unknown");
		#endif
		(void)pr_force("\n ****************[End]**************\n");
	}
	if (rc == CMD_RET_SUCCESS) {
		if (ucPLL_Status != 0U) {
			(void)pr_force("\nResult PASS\n");
		} else {
			(void)pr_force("\nResult FAIL\n");
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

	(void)Dpv14_Tx_API_Set_PW();
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

	(void)pr_force("\n *********[regbank case 2]***********");
	(void)pr_force("\n Expect=> Write[0x1280000C]:0x00005A5A");
	(void)pr_force("\n	<-> Read[0x1280000C]:0xXXXXXXXX");
	(void)pr_force("\n	Write[0x12800010]:0x5A5A0000");
	(void)pr_force("\n	<-> Read[0x12800010]:0xXXXXXXXX");

	(void)Dpv14_Tx_API_Write_RegisterBank_Value(auiWriteBuf,
						    (uint32_t)0x000C,
						    (uint8_t)NUM_OF_TESTS);

	(void)Dpv14_Tx_API_Read_RegisterBank_Value(auiReadBuf,
						   (uint32_t)0x000C,
						   (uint8_t)NUM_OF_TESTS);

	(void)pr_force("\n\nActual=> W[0x1280000C]: 0x%08X", auiWriteBuf[0]);
	(void)pr_force("\n		 <-> R[0x1280000C]: 0x%08X", auiReadBuf[0]);
	(void)pr_force("\nActual=> Write0x12800010]: 0x%08X", auiWriteBuf[1]);
	(void)pr_force("\n		  <-> R[0x12800010]: 0x%08X", auiReadBuf[1]);

	for (ucIdx = 0; ucIdx < (uint8_t)NUM_OF_TESTS; ucIdx++) {
		if (auiWriteBuf[ucIdx] == auiReadBuf[ucIdx]) {
			(void)pr_force("\n\n Result FAIL\n");
			break;
		}
	}
	if (ucIdx == (uint8_t)NUM_OF_TESTS) {
		(void)pr_force("\n\n Result => PASS ");
	}

	(void)pr_force("\n **************[End]************\n");

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

	(void)Dpv14_Tx_API_Set_PW();
	(void)Dpv14_Tx_API_Set_CfgLock(false);
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

	(void)pr_force("\n *********[regbank case 1]***********");
	(void)pr_force("\n Expect=> Write[0x1280000C]:0x00005A5A");
	(void)pr_force("\n	<-> Read[0x1280000C]:0x00005A5A");
	(void)pr_force("\n	Write[0x12800010]:0x5A5A0000");
	(void)pr_force("\n	<-> Read[0x12800010]:0x5A5A0000");

	(void)Dpv14_Tx_API_Write_RegisterBank_Value(auiWriteBuf,
						    (uint32_t)0x000C,
						    (uint8_t)NUM_OF_TESTS);

	(void)Dpv14_Tx_API_Read_RegisterBank_Value(auiReadBuf,
						   (uint32_t)0x000C,
						   (uint8_t)NUM_OF_TESTS);

	(void)pr_force("\n\nActual=> W[0x1280000C]: 0x%08X", auiWriteBuf[0]);
	(void)pr_force("\n       <-> R[0x1280000C]: 0x%08X", auiReadBuf[0]);
	(void)pr_force("\nActual=> Write0x12800010]: 0x%08X", auiWriteBuf[1]);
	(void)pr_force("\n        <-> R[0x12800010]: 0x%08X", auiReadBuf[1]);

	for (ucIdx = 0; ucIdx < (uint8_t)NUM_OF_TESTS; ucIdx++) {
		if (auiWriteBuf[ucIdx] != auiReadBuf[ucIdx]) {
			(void)pr_force("\n\n Result FAIL\n");
			break;
		}
	}
	if (ucIdx == (uint8_t)NUM_OF_TESTS) {
		(void)pr_force("\n\n Result => PASS ");
	}

	(void)pr_force("\n **************[End]************\n");

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
			#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
			trace_set_name("dp_regbank_0");
			#endif
			(void)dpv14_set_unlock_protect_succ();
			#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
			trace_set_name("unknown");
			#endif
			break;
		case PROTECT_TEST_UNLOCK_FAIL:
			#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
			trace_set_name("dp_regbank_1");
			#endif
			(void)dpv14_set_unlock_protect_fail((bool)true, (bool)false);
			#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
			trace_set_name("unknown");
			#endif
			break;
		case PROTECT_TEST_ACCESS_MODE:
			#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
			trace_set_name("dp_regbank_2");
			#endif
			(void)dpv14_set_unlock_protect_succ();
			(void)dpv14_set_unlock_protect_fail((bool)false, (bool)true);
			#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
			trace_set_name("unknown");
			#endif
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
		#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
		trace_set_name("dp_edid");
		#endif
		(void)Dpv14_Tx_API_Get_Dtd_From_Edid(&stDptx_Dtd_Params, ucStreamIdx);
		#if defined(CONFIG_DISPLAY_PORT_V14_RBTC)
		trace_set_name("unknown");
		#endif
		(void)pr_force("\n[Display timing set from EDID\n");
		(void)pr_force("	Pixel clk = %u\n",
				(u32)stDptx_Dtd_Params.uiPixel_Clock);
		(void)pr_force("	Repetition = %u\n",
				(u32)stDptx_Dtd_Params.pixel_repetition_input);
		(void)pr_force("	%s\n",
				((bool)stDptx_Dtd_Params.interlaced) ?
				"Interlace":"Progressive");
		(void)pr_force("	H Sync Polarity(%u), V Sync Polarity(%u)\n",
				(u32)stDptx_Dtd_Params.h_sync_polarity,
				(u32)stDptx_Dtd_Params.h_sync_polarity);
		(void)pr_force("	H Active(%u), V Active(%u)\n",
				(u32)stDptx_Dtd_Params.h_active,
				(u32)stDptx_Dtd_Params.v_active);
		(void)pr_force("	H Image size(%u), V Image size(%u)\n",
				(u32)stDptx_Dtd_Params.h_image_size,
				(u32)stDptx_Dtd_Params.v_image_size);
		(void)pr_force("	H Blanking(%u), V Blanking(%u)\n",
				(u32)stDptx_Dtd_Params.h_blanking,
				(u32)stDptx_Dtd_Params.v_blanking);
		(void)pr_force("	H Sync offset(%u), V Sync offset(%u)\n",
				(u32)stDptx_Dtd_Params.h_sync_offset,
				(u32)stDptx_Dtd_Params.v_sync_offset);
		(void)pr_force("	H Sync plus W(%u), V Sync plus W(%u)\n",
				(u32)stDptx_Dtd_Params.h_sync_pulse_width,
				(u32)stDptx_Dtd_Params.v_sync_pulse_width);
	}

	if (rc != CMD_RET_SUCCESS) {
		print_read_edid_help();
	}
	return (int)rc;
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
	uint64_t ulTestCase, ulTestIdx = 0;
	enum CTS_TEST_TYPE Cts_Test_Type;
	char *const *sub_argv = &argv[1];
	int sub_argc = argc -1;

	(void)cmdtp;
	(void)flag;

	if (sub_argc == 1) {
		if (strcmp(sub_argv[0], "list") == 0) {
			print_cts_list();
		}
	} else if (sub_argc != 2) {
		rc = CMD_RET_FAILURE;
	} else {
		ulTestCase = simple_strtoul(sub_argv[0], NULL, 10);

		if (ulTestCase >= (uint64_t)CTS_TEST_UNKNOWN) {
			rc = CMD_RET_USAGE;
		}
		if (rc == CMD_RET_SUCCESS) {
			Cts_Test_Type = (enum CTS_TEST_TYPE)ulTestCase;

			switch (Cts_Test_Type) {
			case CTS_TEST_RBR_HBR:
			case CTS_TEST_HBR2_HBR3:
				ulTestIdx = simple_strtoul(sub_argv[1], NULL, 10);

				(void)dpv14_set_eye_rate(Cts_Test_Type, ulTestIdx);
				break;
			default:
				rc = CMD_RET_USAGE;
				break;
			}
		}
	}
	if (rc != CMD_RET_SUCCESS) {
		print_cts_help();
	}
	return (int)rc;
}

static void dpv14_dpcd_usage(void)
{
	(void)pr_force("dpv14 dpcd [r|w] <address> <value> \n"
			" r       : read\n"
			" w       : write\n"
			" address : dpcd address (00000F ~ 0xFFFFF) \n"
			" value   : \n"
			"           Incase of read mode value means the number of registers to read.\n"
			"           Otherwise, the value means the value written to the register.\n");
}

static int dpv14_dpcd_dump(struct Dptx_Params *dptx_params,
				unsigned int address, unsigned int count)
{
	unsigned char data;
	unsigned int loop;
	int ret = 0;

	for(loop = 0u; loop < count; loop++) {
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
			(void)pr_force("\n0x%05x# ", address);
		}
		(void)pr_force("0x%02x ", data);
		address++;
	}
	(void)pr_force("\n");
	return ret;
}

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
			if (options[1] > 0x7FFFFFFFUL) {
				rc = CMD_RET_USAGE;
			} else if ((unsigned int)(options[1] & 0x7FFFFFFFUL) > (dpctp_max_address - 1u)) {
				rc = CMD_RET_USAGE;
			} else {
				count = (unsigned int)(options[1] & 0xFFFFFFFFUL);
			}
			if (rc == CMD_RET_SUCCESS) {
				if (count > (0xFFFFFFFFU - address)) {
					rc = CMD_RET_USAGE;
				} else if (address > (0xFFFFFFFFU - count)) {
					rc = CMD_RET_USAGE;
				} else if ((address + count) > dpctp_max_address) {
					rc = CMD_RET_USAGE;
				} else {
					rc = CMD_RET_SUCCESS;
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

static void print_phy_help(void)
{
	(void)pr_force("\nUSAGE: dpv14 phy <category> <option1> <option2> <value>\n"
		" category:\n"
		"  - oc : signal output control - set voltage swing and pre-emphasis\n"
		"  - eq : signal quality equalization - set main and post equalization\n"
		"  - table : set table for signal quality equalization\n");
	(void)pr_force(	" option1 (for 'oc' category):\n"
		"  - vsw : set voltage swing level <n>\n"
		"  - pre : set pre-emphasis level <n>\n"
		" value (for 'oc' category):\n"
		"    - n : signal output control level <0 to 3>\n"
		"  >>> eg) dpv14 phy oc vsw 1 \n\n"
		" option1 (for 'eq' category):\n"
		"  - main : set main equalization level <n>\n"
		"  - post : set main equalization level <n>\n"
		" value (for 'eq' category):\n"
		"    - n : signal equalization level <0 to %u>\n"
		"  >>> eg) dpv14 phy eq main 13 \n\n", DPTX_EQ_MAIN_MAX);
	(void)pr_force(	" option1 (for 'table' category - link rate):\n"
		"  - main : set table for main equalization \n"
		"  - post : set table for main equalization \n"
		" option2 (for 'table' category):\n"
		"  - rbr  : Reduced Bit Rate\n"
		"  - hbr  : High Bit Rate\n"
		"  - hbr2 : High Bit Rate 2\n"
		"  - hbr3 : High Bit Rate 3\n"
		" value (for 'table' category):\n"
		"  - n1 n2 ... n16 : signal equalization level <0 to %u>\n"
		"         + --------------------+\n"
		"         | pre   0   1   2   3 |\n"
		"         + --------------------+\n"
		"     vsw | 0 |  n1  n2  n3  n4 |\n"
		"         | 1 |  n5  n6  n6  *8 |\n"
		"         | 2 |  n9 n10 *11 *12 |\n"
		"         | 3 | n13 *14 *15 *16 |\n"
		"         + --------------------+\n"
		"     Note:'*' indicates a value that falls outside the range\n"
		"          allowed by the DisplayPort specification. The input\n"
		"          value is internally ignored. \n"
		"  >>> eg) dpv14 phy table main rbr 1 6 13 13 3 6 11 0 5 10 0 0 9 0 0 0 \n\n",
		DPTX_EQ_POST_MAX);
}

static void print_phy_output_control(void)
{
	uint32_t link_rate = dpv14_api_get_current_link_rate();
	uint8_t vsl = dpv14_api_get_voltage_swing_value();
	uint8_t pre_emp = dpv14_api_get_pre_emphsis_value();

	(void)pr_force("PHY Signal output control - %s\n",
		(link_rate ==  (uint32_t)LINK_RATE_RBR)? "rbr " :
		(link_rate ==  (uint32_t)LINK_RATE_HBR)? "hbr " :
		(link_rate ==  (uint32_t)LINK_RATE_HBR2)? "hbr2" : "hbr3");
	(void)pr_force(" Voltage swing level <%u> \n"
	       " Pre-emphsis <%u> \n",
		vsl,
		pre_emp);
	(void)pr_force("\n");
}
static void print_updated_phy_output_control(bool vsw, uint32_t prev_value)
{
	uint32_t link_rate = dpv14_api_get_current_link_rate();
	uint8_t vsl = dpv14_api_get_voltage_swing_value();
	uint8_t pre_emp = dpv14_api_get_pre_emphsis_value();

	(void)pr_force("PHY Signal output control - %s\n",
		(link_rate ==  (uint32_t)LINK_RATE_RBR)? "rbr " :
		(link_rate ==  (uint32_t)LINK_RATE_HBR)? "hbr " :
		(link_rate ==  (uint32_t)LINK_RATE_HBR2)? "hbr2" : "hbr3");
	if (vsw) {
		(void)pr_force(" Voltage swing level <%u> -> <%u>\n"
		       " Pre-emphsis <%u> \n",
		       prev_value,
		       vsl,
		       pre_emp);
	} else {
		(void)pr_force(" Voltage swing level <%u>\n"
		       " Pre-emphsis <%u> -> <%u>\n",
		       vsl,
		       prev_value,
		       pre_emp);
	}
	(void)pr_force("\n");
}

static void print_phy_quality_equalization(void)
{
	uint32_t link_rate = dpv14_api_get_current_link_rate();
	uint32_t main_eq = dpv14_api_get_main_eq_value();
	uint32_t post_eq = dpv14_api_get_post_eq_value();

	(void)pr_force("PHY Signal quality equalization - %s\n",
		(link_rate ==  (uint32_t)LINK_RATE_RBR)? "rbr " :
		(link_rate ==  (uint32_t)LINK_RATE_HBR)? "hbr " :
		(link_rate ==  (uint32_t)LINK_RATE_HBR2)? "hbr2" : "hbr3");
	(void)pr_force(" Main Equalization <%u> \n"
	       " Post Equalizations <%u> \n",
		main_eq,
		post_eq);
	(void)pr_force("\n");
}

static void print_updated_phy_quality_equalization(bool main_eq, uint32_t prev_value)
{
	uint32_t link_rate = dpv14_api_get_current_link_rate();
	uint32_t main_eq_val = dpv14_api_get_main_eq_value();
	uint32_t post_eq_val = dpv14_api_get_post_eq_value();

	(void)pr_force("PHY Signal quality equalization - %s\n",
		(link_rate ==  (uint32_t)LINK_RATE_RBR)? "rbr " :
		(link_rate ==  (uint32_t)LINK_RATE_HBR)? "hbr " :
		(link_rate ==  (uint32_t)LINK_RATE_HBR2)? "hbr2" : "hbr3");
	if (main_eq) {
		(void)pr_force(" Main Equalization <%u> -> <%u>\n"
		       " Post Equalizations <%u> \n",
		       prev_value,
		       main_eq_val,
		       post_eq_val);

	} else {
		(void)pr_force(" Main Equalization <%u>\n"
		       " Post Equalizations <%u> -> <%u>\n",
		       main_eq_val,
		       prev_value,
		       post_eq_val);
	}
	(void)pr_force("\n");
}

static void print_eq_table(bool main_eq, const uint32_t eq_table[16], uint32_t link_rate) {
	(void)pr_force("%s %s: %03u %03u %03u %03u %03u %03u %03u %03u %03u %03u %03u %03u %03u %03u %03u %03u\n",
           	main_eq ? "main" : "post",
		(link_rate ==  (uint32_t)LINK_RATE_RBR)? "rbr " :
		(link_rate ==  (uint32_t)LINK_RATE_HBR)? "hbr " :
		(link_rate ==  (uint32_t)LINK_RATE_HBR2)? "hbr2" : "hbr3",
		eq_table[0], eq_table[1], eq_table[2], eq_table[3],
		eq_table[4], eq_table[5], eq_table[6], eq_table[7],
		eq_table[8], eq_table[9], eq_table[10], eq_table[11],
		eq_table[12], eq_table[13], eq_table[14], eq_table[15]);
}

static void print_phy_table_for_quality_equalization(uint32_t link_rate)
{
	uint32_t eq_table[16] = {0,};

	(void)pr_force("PHY table for signal quality equalization\n");
	(void)dpv14_api_get_main_eq_table(eq_table, link_rate);
	print_eq_table(true, eq_table, link_rate);
	(void)dpv14_api_get_post_eq_table(eq_table, link_rate);
	print_eq_table(false, eq_table, link_rate);
	(void)pr_force("\n");
}

static void print_updated_table_phy_quality_equalization(bool main_eq, const uint32_t prev_eq_table[16], uint32_t link_rate)
{
	uint32_t eq_table[16] = {0,};

	if (main_eq) {
		(void)dpv14_api_get_main_eq_table(eq_table, link_rate);
		print_eq_table(true, prev_eq_table, link_rate);
		(void)pr_force(" to\n");
		print_eq_table(true, eq_table, link_rate);
	} else {
		(void)dpv14_api_get_post_eq_table(eq_table, link_rate);
		print_eq_table(false, prev_eq_table, link_rate);
		(void)pr_force(" to\n");
		print_eq_table(false, eq_table, link_rate);
	}
	(void)pr_force("\n");
}

static enum command_ret_t do_dpv14_phy_output_control(char *const *sub_argv, int sub_argc)
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	unsigned long cmd_value;
	uint32_t prev_value;
	uint8_t update_value;

	/* output control */
	if (sub_argc > 1) {
		/* option - "vsw", "prw" */
		if (strcmp(sub_argv[1], "vsw") == 0) {
			if (sub_argc == 3) {
				cmd_value = simple_strtoul(sub_argv[2], NULL, 10);
				if (cmd_value > 3u) {
					(void)pr_force("Not allowed voltage swing %lu level\n\n", cmd_value);
					rc = CMD_RET_FAILURE;
				}
				if (rc == CMD_RET_SUCCESS) {
					update_value = (uint8_t)cmd_value;
					prev_value = dpv14_api_get_voltage_swing_value();
					(void)dpv14_api_set_voltage_swing_value(update_value);
					print_updated_phy_output_control(true, prev_value);
				}
			} else {
				rc = CMD_RET_FAILURE;
			}
		} else if (strcmp(sub_argv[1], "pre") == 0) {
			if (sub_argc == 3) {
				cmd_value = simple_strtoul(sub_argv[2], NULL, 10);
				if (cmd_value > 3u) {
					(void)pr_force("Not allowed pre-emphasis %lu level\n\n", cmd_value);
					rc = CMD_RET_FAILURE;
				}
				if (rc == CMD_RET_SUCCESS) {
					update_value = (uint8_t)cmd_value;
					prev_value = dpv14_api_get_pre_emphsis_value();
					(void)dpv14_api_set_pre_emphsis_value(update_value);
					print_updated_phy_output_control(false, prev_value);
				}
			} else {
				rc = CMD_RET_FAILURE;
			}
		} else {
			rc = CMD_RET_FAILURE;
		}
	} else {
		print_phy_output_control();
	}
	return rc;
}

static enum command_ret_t do_dpv14_phy_quality_equalization(char *const *sub_argv, int sub_argc)
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	unsigned long cmd_value;
	uint32_t prev_value;
	uint8_t update_value;

	/* signal quality equalization */
	if (sub_argc > 1) {
		/* option - "main", "post" */
		if (strcmp(sub_argv[1], "main") == 0) {
			if (sub_argc == 3) {
				cmd_value = simple_strtoul(sub_argv[2], NULL, 10);
				if (cmd_value > 0xFFu) {
					rc = CMD_RET_FAILURE;
				}
				if (rc == CMD_RET_SUCCESS) {
					update_value = (uint8_t)cmd_value;
					prev_value = dpv14_api_get_main_eq_value();
					(void)dpv14_api_set_main_eq_value(update_value);
					print_updated_phy_quality_equalization(true, prev_value);
				}
			} else {
				rc = CMD_RET_FAILURE;
			}
		} else if (strcmp(sub_argv[1], "post") == 0) {
			if (sub_argc == 3) {
				cmd_value = simple_strtoul(sub_argv[2], NULL, 10);
				if (cmd_value > 0xFFu) {
					rc = CMD_RET_FAILURE;
				}
				if (rc == CMD_RET_SUCCESS) {
					update_value = (uint8_t)cmd_value;
					prev_value = dpv14_api_get_post_eq_value();
					(void)dpv14_api_set_post_eq_value(update_value);
					print_updated_phy_quality_equalization(false, prev_value);
				}
			} else {
				rc = CMD_RET_FAILURE;
			}
		} else {
			rc = CMD_RET_FAILURE;
		}
	} else {
		print_phy_quality_equalization();
	}
	return rc;
}

static enum command_ret_t do_dpv14_phy_table_for_quality_equalization(char *const *sub_argv, int sub_argc)
{
	uint32_t link_rate, update_value[16] = {0,}, prev_value[16] = {0,}, loop;
	enum command_ret_t rc = CMD_RET_SUCCESS;
	const char *link_args = NULL;
	unsigned long cmd_value;

	/* Table for signal quality equalization */
	if (sub_argc > 1) {
		if (sub_argc == 2) {
			link_args = sub_argv[1];
		} else {
			link_args = sub_argv[2];
		}
		if (strcmp(link_args, "rbr") == 0) {
			link_rate = (uint32_t)LINK_RATE_RBR;
		} else if (strcmp(link_args, "hbr") == 0) {
			link_rate = (uint32_t)LINK_RATE_HBR;
		} else if (strcmp(link_args, "hbr2") == 0) {
			link_rate = (uint32_t)LINK_RATE_HBR2;
		} else if (strcmp(link_args, "hbr3") == 0) {
			link_rate = (uint32_t)LINK_RATE_HBR3;
		} else {
			rc = CMD_RET_FAILURE;
		}
		if (sub_argc == 2) {
			if (rc == CMD_RET_SUCCESS) {
				print_phy_table_for_quality_equalization(link_rate);
			}
		} else {
			if (rc == CMD_RET_SUCCESS) {
				/* option - "main", "post" */
				if (strcmp(sub_argv[1], "main") == 0) {
					if (sub_argc == 19) {
						for (loop = 0u; loop < 16u; loop++) {
							cmd_value = simple_strtoul(sub_argv[3u + loop], NULL, 10);
							if (cmd_value > 0xFFu) {
								rc = CMD_RET_FAILURE;
								break;
							}
							update_value[loop] = (uint32_t)cmd_value;
						}
						if (rc == CMD_RET_SUCCESS) {
							(void)dpv14_api_get_main_eq_table(prev_value, link_rate);
							(void)dpv14_api_set_main_eq_table(update_value, link_rate);
							print_updated_table_phy_quality_equalization(true, prev_value, link_rate);
						}
					} else {
						rc = CMD_RET_FAILURE;
					}
				} else if (strcmp(sub_argv[1], "post") == 0) {
					if (sub_argc == 19) {
						for (loop = 0u; loop < 16u; loop++) {
							cmd_value = simple_strtoul(sub_argv[3u + loop], NULL, 10);
							if (cmd_value > 0xFFu) {
								rc = CMD_RET_FAILURE;
								break;
							}
							update_value[loop] = (uint32_t)cmd_value;
						}
						if (rc == CMD_RET_SUCCESS) {
							(void)dpv14_api_get_post_eq_table(prev_value, link_rate);
							(void)dpv14_api_set_post_eq_table(update_value, link_rate);
							print_updated_table_phy_quality_equalization(false, prev_value, link_rate);
						}
					} else {
						rc = CMD_RET_FAILURE;
					}
				} else {
					rc = CMD_RET_FAILURE;
				}
			} else {
				rc = CMD_RET_FAILURE;
			}
		}
	} else {
		rc = CMD_RET_FAILURE;
	}
	return rc;
}

static int do_dpv14_phy(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	char *const *sub_argv = &argv[1];
	int sub_argc = argc -1;

	(void)cmdtp;
	(void)flag;

	if (sub_argc < 1) {
		rc = CMD_RET_FAILURE;
	} else {
		/* category - "oc", "eq", "table" */
		if (strcmp(sub_argv[0], "oc") == 0) {
			rc = do_dpv14_phy_output_control(sub_argv, sub_argc);
		} else if (strcmp(sub_argv[0], "eq") == 0) {
			rc = do_dpv14_phy_quality_equalization(sub_argv, sub_argc);
		} else if (strcmp(sub_argv[0], "table") == 0) {
			rc = do_dpv14_phy_table_for_quality_equalization(sub_argv, sub_argc);
		} else {
			rc = CMD_RET_FAILURE;
		}
	}
	if (rc != CMD_RET_SUCCESS) {
		print_phy_help();
	}
	return (int)rc;
}

static struct cmd_tbl cmd_dpv14_sub[] = {
	U_BOOT_CMD_MKENT(link, 3, 0, do_dpv14_set_link, "", ""),
	U_BOOT_CMD_MKENT(regbank, 3, 0, do_dpv14_register_bank, "", ""),
	U_BOOT_CMD_MKENT(pll, 3, 0, do_dpv14_pll, "", ""),
	U_BOOT_CMD_MKENT(redid, 3, 0, do_dpv14_read_edid, "", ""),
	U_BOOT_CMD_MKENT(cts, 3, 0, do_dpv14_cts, "", ""),
	U_BOOT_CMD_MKENT(dpcd, 3, 0, do_dpv14_dpcd, "", ""),
	U_BOOT_CMD_MKENT(phy, 20, 0, do_dpv14_phy, "", ""),
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
	"---------------------------------------------\n"
	"dpv14 cts \n"
	"dpv14 dpcd \n"
	"dpv14 regbank \n"
	"dpv14 pll \n"
	"dpv14 link \n"
	"dpv14 redid( read edid ) \n"
	"dpv14 phy - PHY Signal Quality Tuning\n"
	"----------------------------------------------\n";


static char usage[] = "DP cmd v"DPV14_CMD_MAJOR_VER "." DPV14_CMD_MINOR_VER "." DPV14_CMD_PATCH_VER " supports to verify dp v1.4 driver functionalities"  ;


U_BOOT_CMD(
	dpv14, 21, 1, do_dpv14,
	usage,
	dpv14_help_text
);
