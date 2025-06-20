// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef LVDS_DDI_CONFIG_H
#define LVDS_DDI_CONFIG_H

/*
 * LVDS Register Offset
 */
#define LVDS_CTRL (0x40u)
#define LVDS_TXO_SEL0 (0x44u)
#define LVDS_TXO_SEL1 (0x48u)
#define LVDS_TXO_SEL2 (0x4Cu)
#define LVDS_TXO_SEL3 (0x50u)
#define LVDS_TXO_SEL4 (0x54u)
#define LVDS_TXO_SEL5 (0x58u)
#define LVDS_TXO_SEL6 (0x5Cu)
#define LVDS_TXO_SEL7 (0x60u)
#define LVDS_TXO_SEL8 (0x64u)
#define LVDS_CFG0 (0x70u)
#define LVDS_CFG1 (0x74u)
#define LVDS_CFG2 (0x78u)
#define LVDS_CFG3 (0x7Cu)


/*
 * LVDS Control Register
 * ---------+--------+------+-------+------------------------------------
 *   Field  | Name   | Type | Reset | Description
 * ---------+--------+------+-------+------------------------------------
 *   31:30    SEL      R/W    0x0    LVDS Select
 *                                    0: Connect port from LCDC0 to LVDS
 *                                    1: Connect port from LCDC1 to LVDS
 *                                    2/3: Reserved
 * ---------+--------+------+-------+------------------------------------
 *   20:15    P        R/W    0xA    LVDS PLL P value
 *                                    0xA: Default value
 *                                    Others: Reserved
 * ---------+--------+------+-------+------------------------------------
 *   14:8     M        R/W    0xA    LVDS PLL M value
 *                                    0xA: Default value
 *                                    Others: Reserved
 * ---------+--------+------+-------+------------------------------------
 *   7:5      S        R/W    0x1    LVDS PLL S value
 *                                    0x1: 30~80 MHz
 *                                    0x2: 80~160 MHz
 *                                    Others: Reserved
 * ---------+--------+------+-------+------------------------------------
 *   4        VSEL     R/W    0x0    LVDS VCO Range Select
 *                                    0: 30~74 MHz
 *                                    1: 74~160 MHz
 * ---------+--------+------+-------+------------------------------------
 *   3        OC       R/W    0x0    LVDS Output Control
 *                                    0: Tx output state Hi-Z
 *                                       when reset is enabled
 *                                    1: Tx output state Ground
 *                                       when reset is enabled
 * ---------+--------+------+-------+------------------------------------
 *   2        EM       R/W    0x0    LVDS Enable
 *                                    0: Disable
 *                                    1: Enable
 * ---------+--------+------+-------+------------------------------------
 *   1        RST      R/W    0x0    LVDS Reset
 *                                    0: Reset
 *                                    1: Normal
 * ---------+--------+------+-------+------------------------------------
 */
#define LVDS_CTRL_SEL_SHIFT (30)
#define LVDS_CTRL_TC_SHIFT (21)
#define LVDS_CTRL_P_SHIFT (15)
#define LVDS_CTRL_M_SHIFT (8)
#define LVDS_CTRL_S_SHIFT (5)
#define LVDS_CTRL_VSEL_SHIFT (4)
#define LVDS_CTRL_OC_SHIFT (3)
#define LVDS_CTRL_EN_SHIFT (2)
#define LVDS_CTRL_RST_SHIFT (1)

#define LVDS_CTRL_SEL_MASK (0x3UL << LVDS_CTRL_SEL_SHIFT)
#define LVDS_CTRL_TC_MASK (0x7UL << LVDS_CTRL_TC_SHIFT)
#define LVDS_CTRL_P_MASK (0x3FUL << LVDS_CTRL_P_SHIFT)
#define LVDS_CTRL_M_MASK (0x7FUL << LVDS_CTRL_M_SHIFT)
#define LVDS_CTRL_S_MASK (0x7UL << LVDS_CTRL_S_SHIFT)
#define LVDS_CTRL_VSEL_MASK (0x1UL << LVDS_CTRL_VSEL_SHIFT)
#define LVDS_CTRL_OC_MASK (0x1UL << LVDS_CTRL_OC_SHIFT)
#define LVDS_CTRL_EN_MASK (0x1UL << LVDS_CTRL_EN_SHIFT)
#define LVDS_CTRL_RST_MASK (0x1UL << LVDS_CTRL_RST_SHIFT)

/*
 * LVDS_TXO_SELn (LVDS TX_Data n)                          (n = 0,1,2~,8)
 * ---------+------------+------+-------+--------------------------------
 *   Field  | Name       | Type | Reset | Description
 * ---------+------------+------+-------+--------------------------------
 *   31:24    SEL_3+(4*n)  R/W    N/A
 *   23:16    SEL_2+(4*n)  R/W    N/A
 *   15:8     SEL_1+(4*n)  R/W    N/A
 *   7:0      SEL_0+(4*n)  R/W    N/A
 * ---------+------------+------+-------+--------------------------------
 *  Description
 * --------------+-------------------------------------------------------
 *  SEL0 ~ SEL34   Connection bit of LPXDATA and TXIN of LVDS.
 *  ~              The mean of 0~23 is the bit of TXIN.
 *                 For example, the SEL_0 is 0,
 *                 connection path is LPXDATA[0] and TXIN0.
 *                 The other example, the SEL_0 is 1,
 *                 connection path is LPXDATA[1] and TXIN0.
 *                 ----------------+---------------------
 *                 SEL_x           | Bit of LPXDATA
 *                 ----------------+---------------------
 *                  0              | LPXDATA[0]
 *                  1              | LPXDATA[1]
 *                  ~              | ~
 *                  23             | LPXDATA[23]
 *                  24             | LDE
 *                  25             | LHS
 *                  26             | LVS
 *                  27:29, 32:34   | 0
 *                  30:31          | 1
 *                  TXCLK          | LPXCLK
 * --------------+-------------------------------------------------------
 */
#define LVDS_TXO_SEL(x) (LVDS_TXO_SEL0 + (4UL * (x)))
#define LVDS_TXO_SEL_N_MAX 9

/*
 * LVDS Configuration Register 0
 * --------+------------+------+-------+---------------------------------
 *   Field | Name       | Type | Reset | Description
 * --------+------------+------+-------+---------------------------------
 *   31      CK_POL_SEL   R/W    0x0    Input clock polarity select
 *                                       0: Bypass
 *                                       1: Inversion
 * --------+------------+------+-------+---------------------------------
 *   30     AUTO_DSK_SEL  R/W    0x0    Auto deskew select for analog
 *                                       0: Normal
 *                                       1: Auto-deskew
 * --------+------------+------+-------+---------------------------------
 *   29:26   SK_BIAS      R/W    0xC    Bias current control pin for Skew
 * --------+------------+------+-------+---------------------------------
 *   25      SKEWINI      R/W    0x1    Skew selection pin
 *                                       0: Bypass
 *                                       1: Skew enable
 * --------+------------+------+-------+---------------------------------
 *   24      SKEW_EN_H    R/W    0x1    Skew block power down
 *                                       0: Power Down
 *                                       1: Operating
 * --------+------------+------+-------+---------------------------------
 *   23      CNTB_TDLY    R/W    0x1    Delay control pin for each channel
 *                                       0: 25 ps
 *                                       1: 50 ps
 * --------+------------+------+-------+---------------------------------
 *   22      SEL_DATABF   R/W    0x0    Input clock 1/2 division
 *                                      control pin
 *                                       0: Bypass
 *                                       1: Divide 2
 * --------+------------+------+-------+---------------------------------
 *   21:20  SKEW_REG_CUR  R/W    0x3    Regulator bias current selection
 *                                      pin in SKEW block
 * --------+------------+------+-------+---------------------------------
 *   19:14  LOCK_PPM_SET  R/W    0x1B   PPM setting for PLL lock
 * --------+------------+------+-------+---------------------------------
 *   13:2    DESKEW_CNT   R/W    0x138  Adjust the period of
 *             _SET                     de-skew region
 * --------+------------+------+-------+---------------------------------
 *   1       AUTO_SEL     R/W    0x0    Auto de-skew selection pin
 *                                       0: Auto-deskew
 *                                       1: Not auto-deskew
 * --------+------------+------+-------+---------------------------------
 *   0       VBLK_FLAG    R/W    0x0    Flag signal for V-blank
 * --------+------------+------+-------+---------------------------------
 */
#define LVDS_CFG0_CK_POL_SEL_SHIFT (31)
#define LVDS_CFG0_AUTO_DSK_SEL_SHIFT (30)
#define LVDS_CFG0_SK_BIAS_SHIFT (26)
#define LVDS_CFG0_SKEWINI_SHIFT (25)
#define LVDS_CFG0_SKEW_EN_H_SHIFT (24)
#define LVDS_CFG0_CNTB_TDLY_SHIFT (23)
#define LVDS_CFG0_SEL_DATABF_SHIFT (22)
#define LVDS_CFG0_SKEW_REG_CUR_SHIFT (20)
#define LVDS_CFG0_LOCK_PPM_SET_SHIFT (14)
#define LVDS_CFG0_DESKEW_CNT_SET_SHIFT (2)
#define LVDS_CFG0_AUTO_SEL_SHIFT (1)
#define LVDS_CFG0_VBLK_FLAG_SHIFT (0)


#define LVDS_CFG0_CK_POL_SEL_MASK (0x1UL << LVDS_CFG0_CK_POL_SEL_SHIFT)
#define LVDS_CFG0_AUTO_DSK_SEL_MASK (0x1UL << LVDS_CFG0_AUTO_DSK_SEL_SHIFT)
#define LVDS_CFG0_SK_BIAS_MASK (0xFUL << LVDS_CFG0_SK_BIAS_SHIFT)
#define LVDS_CFG0_SKEWINI_MASK (0x1UL << LVDS_CFG0_SKEWINI_SHIFT)
#define LVDS_CFG0_SKEW_EN_H_MASK (0x1UL << LVDS_CFG0_SKEW_EN_H_SHIFT)
#define LVDS_CFG0_CNTB_TDLY_MASK (0x1UL << LVDS_CFG0_CNTB_TDLY_SHIFT)
#define LVDS_CFG0_SEL_DATABF_MASK (0x1UL << LVDS_CFG0_SEL_DATABF_SHIFT)
#define LVDS_CFG0_SKEW_REG_CUR_MASK (0x3UL << LVDS_CFG0_SKEW_REG_CUR_SHIFT)
#define LVDS_CFG0_LOCK_PPM_SET_MASK (0x3FUL << LVDS_CFG0_LOCK_PPM_SET_SHIFT)
#define LVDS_CFG0_DESKEW_CNT_SET_MASK (0xFFFUL << LVDS_CFG0_DESKEW_CNT_SET_SHIFT)
#define LVDS_CFG0_AUTO_SEL_MASK (0x1UL << LVDS_CFG0_AUTO_SEL_SHIFT)
#define LVDS_CFG0_VBLK_FLAG_MASK (0x1UL << LVDS_CFG0_VBLK_FLAG_SHIFT)

/*
 * LVDS Configuration Register 1
 * --------+------------+------+-------+---------------------------------
 *   Field | Name       | Type | Reset | Description
 * --------+------------+------+-------+---------------------------------
 *   31      ATE_MODE     R/W    0x0    Function or ATE
 *                                       0: Function
 *                                       1: ATE
 * --------+------------+------+-------+---------------------------------
 *   30    TEST_CON_MODE  R/W    0x0    DA or I2C
 *                                       0: DA
 *                                       1: I2C
 * --------+------------+------+-------+---------------------------------
 *   26      FLT_CNT      R/W    0x0    Filter control pin for PLL
 * --------+------------+------+-------+---------------------------------
 *   25     VOD_ONLY_CNT  R/W    0x0    Pre-emphasis’s pre-driver
 *                                      control pin
 *                                       0: Disable
 *                                       1: Enable
 * --------+------------+------+-------+---------------------------------
 *   24     CNNCT_MODE    R/W    0x0    Connectivity mode selection pin
 *             _SEL                      0: TX operating
 *                                       1: Connectivity check
 * --------+------------+------+-------+---------------------------------
 *   23:22   CNNCT_CNT    R/W    0x0    Connectivity control pin
 *                                       0: TX operating
 *                                       1: Connectivity check
 *                                       Others: Reserved
 * --------+------------+------+-------+---------------------------------
 *   21      LOCK_CNT     R/W    0x0    Lock signal selection pin
 *                                       0: Lock enable
 *                                       1: Lock disable
 * --------+------------+------+-------+---------------------------------
 *   20      VOD_HIGH_S   R/W    0x0    VOD control pin
 *                                       0: Normal with pre-emphasis
 *                                       1: Vod only
 * --------+------------+------+-------+---------------------------------
 *   19      SRC_TRH      R/W    0x0    Source termination resistor
 *                                      selection pin
 *                                       0: Off
 *                                       1: Termination
 * --------+------------+------+-------+---------------------------------
 *   18:11   CNT_VOD_H    R/W    0x97   TX driver output differential
 *                                      voltage level control pin
 * --------+------------+------+-------+---------------------------------
 *   10:3    CNT_PEN_H    R/W    0x1    TX driver pre-emphasis
 *                                      level control
 * --------+------------+------+-------+---------------------------------
 *   2:0     FC_CODE      R/W    0x4    Vos control pin
 * --------+------------+------+-------+---------------------------------
 */
#define LVDS_CFG1_ATE_MODE_SHIFT (31)
#define LVDS_CFG1_TEST_CON_MODE_SHIFT (30)
#define LVDS_CFG1_FLT_CNT_SHIFT (26)
#define LVDS_CFG1_VOD_ONLY_CNT_SHIFT (25)
#define LVDS_CFG1_CNNCT_MODE_SEL_SHIFT (24)
#define LVDS_CFG1_CNNCT_CNT_SHIFT (22)
#define LVDS_CFG1_LOCK_CNT_SHIFT (21)
#define LVDS_CFG1_VOD_HIGH_S_SHIFT (20)
#define LVDS_CFG1_SRC_TRH_SHIFT (19)
#define LVDS_CFG1_CNT_VOD_H_SHIFT (11)
#define LVDS_CFG1_CNT_PEN_H_SHIFT (3)
#define LVDS_CFG1_FC_CODE_SHIFT (0)


#define LVDS_CFG1_ATE_MODE_MASK (0x1UL << LVDS_CFG1_ATE_MODE_SHIFT)
#define LVDS_CFG1_TEST_CON_MODE_MASK (0x1UL << LVDS_CFG1_TEST_CON_MODE_SHIFT)
#define LVDS_CFG1_FLT_CNT_MASK (0x1UL << LVDS_CFG1_FLT_CNT_SHIFT)
#define LVDS_CFG1_VOD_ONLY_CNT_MASK (0x1UL << LVDS_CFG1_VOD_ONLY_CNT_SHIFT)
#define LVDS_CFG1_CNNCT_MODE_SEL_MASK (0x1UL << LVDS_CFG1_CNNCT_MODE_SEL_SHIFT)
#define LVDS_CFG1_CNNCT_CNT_MASK (0x3UL << LVDS_CFG1_CNNCT_CNT_SHIFT)
#define LVDS_CFG1_LOCK_CNT_MASK (0x1UL << LVDS_CFG1_LOCK_CNT_SHIFT)
#define LVDS_CFG1_VOD_HIGH_S_MASK (0x1UL << LVDS_CFG1_VOD_HIGH_S_SHIFT)
#define LVDS_CFG1_SRC_TRH_MASK (0x1UL << LVDS_CFG1_SRC_TRH_SHIFT)
#define LVDS_CFG1_CNT_VOD_H_MASK (0xFFUL << LVDS_CFG1_CNT_VOD_H_SHIFT)
#define LVDS_CFG1_CNT_PEN_H_MASK (0xFFUL << LVDS_CFG1_CNT_PEN_H_SHIFT)
#define LVDS_CFG1_FC_CODE_MASK (0x7UL << LVDS_CFG1_FC_CODE_SHIFT)

/*
 * LVDS Configuration Register 2
 */

/*
 * LVDS Configuration Register 3
 */

#define LVDS_CNNCT_MODE_MAX LVDS_CNNCT_MODE_RESERVED_1
enum {
	LVDS_CNNCT_MODE_TX_OP = 0,
	LVDS_CNNCT_MODE_CHECK,
	LVDS_CNNCT_MODE_RESERVED_1,
	LVDS_CNNCT_MODE_RESERVED_2
};

enum {
	TS_TXOUT_SEL0 = 0,
	TS_TXOUT_SEL1,
	TS_TXOUT_SEL2,
	TS_TXOUT_SEL3,
	TS_TXOUT_SEL4,
	TS_TXOUT_SEL5,
	TS_TXOUT_SEL6,
	TS_TXOUT_SEL7,
	TS_TXOUT_SEL8,
	TS_TXOUT_SEL_MAX
};

/* NOTE:
 * Since OpenLDI has required 4 data lanes for 24-bit single pixel transmission,
 * the driver is designed to use 4 data lines even if 5 lines are physically supported.
 */
enum {
	LVDS_PHY_CLK_LANE = 0,
	LVDS_PHY_DATA0_LANE,
	LVDS_PHY_DATA1_LANE,
	LVDS_PHY_DATA2_LANE,
	LVDS_PHY_DATA3_LANE,
	LVDS_PHY_LANE_MAX
};

enum {
	LVDS_PHY_PORT_S0 = 0,
	LVDS_PHY_PORT_MAX
};


enum {
	LVDS_LCD_PORT_0 = 0,
	LVDS_LCD_PORT_1,
	LVDS_LCD_PORT_MAX
};

/* NOTE:
 * Since OpenLDI has required 4 data lanes for 24-bit single pixel transmission,
 * the driver is designed to use 4 data lines even if 5 lines are physically supported.
 */
#define TXOUT_MAX_LINE (4u)
#define TXOUT_DATA_PER_LINE (7u)
#define TXOUT_GET_DATA(i)                                     \
	((TXOUT_DATA_PER_LINE - 1u) - ((i) % TXOUT_DATA_PER_LINE) \
	 + (TXOUT_DATA_PER_LINE * ((i) / TXOUT_DATA_PER_LINE)))

#define TXOUT_DUMMY (31)
#define TXOUT_VS (26)
#define TXOUT_HS (25)
#define TXOUT_DE (24)
#define TXOUT_R_D(x) ((x) + 0x10)
#define TXOUT_G_D(x) ((x) + 0x8)
#define TXOUT_B_D(x) (x)

/* LVDS_CTRL */
extern void LVDS_DDI_CONF_SetDispController(uint32_t select);
extern void LVDS_DDI_CONF_ConfigPLL(uint32_t main_divider, uint32_t pre_divider, uint32_t post_scaler);
extern void LVDS_DDI_CONF_SetVCORange(uint32_t select);
extern void LVDS_DDI_CONF_SetTxOutControl(uint32_t select);
extern void LVDS_DDI_CONF_Enable(uint32_t enable);
extern void LVDS_DDI_CONF_Reset(uint32_t reset);
/* LVDS_TXO_SELn */
extern void LVDS_DDI_CONF_SetDataArray(uint32_t data[TXOUT_MAX_LINE][TXOUT_DATA_PER_LINE]);
extern void LVDS_DDI_CONF_SetDataArrayWithLineOrder(uint32_t data[TXOUT_MAX_LINE][TXOUT_DATA_PER_LINE], const uint32_t *lane_main);
/* LVDS_CFG0 */
extern void LVDS_DDI_CONF_SetClockPolarity(uint32_t inversion);
extern void LVDS_DDI_CONF_EnableAutoDeskew(uint32_t enable);
extern void LVDS_DDI_CONF_BiasControl(uint32_t level);
/* LVDS_CFG1 */
extern void LVDS_DDI_CONF_LockControl(uint32_t enable);
extern void LVDS_DDI_CONF_ConnectivityControl(uint32_t select);
extern void LVDS_DDI_CONF_VODControl(uint32_t enable, uint32_t pre_driver_enable);
extern void LVDS_DDI_CONF_SetSourceTermination(uint32_t enable);
extern void LVDS_DDI_CONF_SetVODLevel(uint32_t level);
extern void LVDS_DDI_CONF_SetPreEmphasisLevel(uint32_t level);
extern void LVDS_DDI_CONF_SetVosLevel(uint32_t level);
/* LVDS_CFG2 */
/* LVDS_CFG3 */
#endif
