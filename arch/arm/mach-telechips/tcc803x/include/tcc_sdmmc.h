// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TCC_SDMMC_H__
#define __TCC_SDMMC_H__

/* SD Host Controller Registers */
#define SDHC_BTCONT           0x070
#define SDHC_PRESET4          0x074
#define SDHC_VENDOR           0x078

/* Channel Control Registers */
/* Channel Control Offset */
#define SDHC_CHCTRL_TAPDLY              0x00
 #define SDHC_CHCTRL_OTAP_MASK			0x1F
 #define SDHC_CHCTRL_OTAP_SHIFT			8
 #define SDHC_CHCTRL_OTAP_EN_SHIFT		13
#define SDHC_CHCTRL_CAP_0               0x04
#define SDHC_CHCTRL_CAP_1               0x08
#define SDHC_CHCTRL_PRESET_0            0x0C
#define SDHC_CHCTRL_RESET_1             0x10
#define SDHC_CHCTRL_PRESET_2            0x14
#define SDHC_CHCTRL_PRESET_3            0x18
#define SDHC_CHCTRL_PRESET_4            0x1C
#define SDHC_CHCTRL_MAXCRNT             0x20
#define SDHC_CHCTRL_CQ                  0x24
#define SDHC_CHCTRL_DBG_0               0x3C
#define SDHC_CHCTRL_DBG_1               0x40
#define SDHC_CHCTRL_DBG_2               0x44
#define SDHC_CHCTRL_MON                 0x48
#define SDHC_CHCTRL_CD_WP               0x4C
/* Channel Clock Delay Offset */
#define SDHC_CHCTRL_AUTO_TUNE_RTL       0x00
#define SDHC_CHCTRL_SMPL_SFT_CLK_DIV    0x04
#define SDHC_CHCTRL_SMPL_SFT_CLK_CTL    0x08
#define SDHC_CHCTRL_SMPL_SFT_TIM_CTL    0x0C
 #define SDHC_CHCTL_SMPL_SFT_TAP_MASK		0x0F
 #define SDHC_CHCTL_SMPL_SFT_POS_TAP_SHIFT	4
 #define SDHC_CHCTL_SMPL_SFT_NEG_TAP_SHIFT	20
#define SDHC_CHCTRL_TX_CLK_DLY_0_1      0x10
#define SDHC_CHCTRL_TX_CLK_DLY_2        0x14
#define SDHC_CHCTRL_SD0_DQS_DLY         0x18
#define SDHC_CHCTRL_RX_CLK_DLY_FINE_VAL 0x2C
/* Channel CMD and Data Delay Offset */
#define SDHC_CHCTRL_SD_CMD_DLY          0x00
#define SDHC_CHCTRL_SD_DATA0_DLY        0x04
#define SDHC_CHCTRL_SD_DATA1_DLY        0x08
#define SDHC_CHCTRL_SD_DATA2_DLY        0x0C
#define SDHC_CHCTRL_SD_DATA3_DLY        0x10
#define SDHC_CHCTRL_SD_DATA4_DLY        0x14
#define SDHC_CHCTRL_SD_DATA5_DLY        0x18
#define SDHC_CHCTRL_SD_DATA6_DLY        0x1C
#define SDHC_CHCTRL_SD_DATA7_DLY        0x20

#define TCC_SDHC_MAX_CONT_NUM 2

struct tcc_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	void *base;
	void *chctrl_base;
	void *chclkdly_base;
	void *chcddly_base;
	uint32_t max_clk;
	uint32_t peri_clk;
	int32_t index;
	int32_t bus_width;

	uint32_t taps[4];
	uint32_t hs400_pos_tap;
	uint32_t hs400_neg_tap;
};

#endif /* __TCC_SDMMC_H__ */
