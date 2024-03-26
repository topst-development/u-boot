// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/

#include <config.h>
#include <common.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <i2c.h>
#include <dm/uclass.h>

#include <telechips/dpv14_max968xx.h>
#include <dt-bindings/display/telechips-dispdef.h>


/******* Ser Register *********/
#define SER_DEV_REV 0x000E
#define SER_REV_ES2 0x01
#define SER_REV_ES4 0x03
#define SER_REV_ALL 0x0F

#define LINK_ENABLE          0x7000
#define SER_MISC_CONFIG_B1   0x7019
#define MST_FUNCTION_DISABLE 0x00
#define MST_FUNCTION_ENABLE  0x01
#define SER_LANE_SWAP_B0    0x7030
#define SER_LANE_SWAP_B1 0x7031
/******* Ser Register *********/

/******* Des Register ********/
#define DES_DEV_REV          0x000E
#define DES_REV_ES2          0x01
#define DES_REV_ES3          0x02

#define DES_STREAM_SELECT    0x00A0
#define DES_DROP_VIDEO       0x0307
/******* Ser Register *********/

#define MAX_RETRY_I2C_RW             3
#define MAX_I2C_CHECK_RETRY          3

#define SER_DES_I2C_SPEED            400000
#define SER_DES_I2C_REG_ADD_LEN      2
#define INVALID_I2C_DEV_ADD          0xFF

#define DP0_PANEL_SER_I2C_DEV_ADD 0xC0
#define DP0_PANEL_DES_I2C_DEV_ADD 0x90
#define DP1_PANEL_SER_I2C_DEV_ADD INVALID_I2C_DEV_ADD
#define DP1_PANEL_DES_I2C_DEV_ADD 0x94
#define DP2_PANEL_SER_I2C_DEV_ADD INVALID_I2C_DEV_ADD
#define DP2_PANEL_DES_I2C_DEV_ADD 0x98
#define DP3_PANEL_SER_I2C_DEV_ADD INVALID_I2C_DEV_ADD
#define DP3_PANEL_DES_I2C_DEV_ADD 0xD0

#define SERDES_DELAY_ADDR               0xEFFF
#define DP_SER_DES_INVALID_REG_ADDR     0xFFFF


struct Max968xx_Reg_Data {
	int32_t ii2c_dev_add;
	uint32_t uireg_add;
	uint32_t uireg_val;
	uint8_t ucevb_power_type;
	uint8_t ucser_rev;
};

struct Max968xx_Reg_Params {
	uint32_t uicustom_vic;
	struct Max968xx_Reg_Data *pstDES_ES3_Reg;
	struct Max968xx_Reg_Data *pstDES_ES2_Reg;
};

struct MAX968XX_Drv_Params {
	uint8_t ucevb_power_type;
	uint8_t ucser_rev;
	uint8_t ucdes_rev;
	uint8_t aucvcp_id[MAX968XX_INPUT_STREAM_MAX];
	uint8_t auclane_order[DP_LANE_4];
	int32_t iser_i2c_devadd;
	int32_t ides_i2c_devadd;
	struct udevice  *pstser_i2c_udevice;
	struct udevice  *pstdes_i2c_udevice;
};


static struct Max968xx_Reg_Data pstVIC_1027_Max968xx_ES3_Reg[] = {
	{0xD0, 0x0010, 0x80, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x98, 0x0010, 0x80, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x94, 0x0010, 0x80, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0010, 0x80, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0010, 0x80, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	10, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* AGC CR Init 8G1 */
	{0xC0, 0x60AA, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x61AA, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x62AA, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x63AA, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	/* BST CR Init 8G1 */
	{0xC0, 0x60B6, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x61B6, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x62B6, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x63B6, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	/* AGC CR Init 5G4 */
	{0xC0, 0x60A9, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x61A9, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x62A9, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x63A9, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	/* BST CR Init 5G4 */
	{0xC0, 0x60B5, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x61B5, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x62B5, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x63B5, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	/* AGC CR Init 2G7 */
	{0xC0, 0x60A8, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x61A8, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x62A8, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x63A8, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	/* BST CR Init 2G7 */
	{0xC0, 0x60B4, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x61B4, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x62B4, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x63B4, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	/* Set 8G1 Error Channel Phase */
	{0xC0, 0x6070, 0xA5, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6071, 0x65, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6170, 0xA5, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6171, 0x65, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6270, 0xA5, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6271, 0x65, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6370, 0xA5, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6371, 0x65, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},

	/**********************
	* MST Setting        *
	**********************
	*/
	/* Turn off video-GM03 */
	{0xC0, 0x6420, 0x10, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Disable MST_VS0_DTG_ENABLE */
	{0xC0, 0x7A14, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Disable LINK_ENABLE */
	{0xC0, 0x7000, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Enable MST */
	{0xC0, SER_MISC_CONFIG_B1, MST_FUNCTION_ENABLE,
	TCC_EVB_LCD_POW_MAX, SER_REV_ALL}, /* 100ms delay */
	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/* Set AUX_RD_INTERVAL to 16ms */
	{0xC0, 0x70A0, 0x04, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Max rate : 1E -> 8.1Gbps, 14 -> 5.4Gbps, 0A -> 2.7Gbps */
	{0xC0, 0x7074, 0x1E, TCC_EVB_LCD_POW_MAX, SER_REV_ES4},
	{0xC0, 0x7074, 0x0A, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	/* Max lane count to 4 */
	{0xC0, 0x7070, 0x04, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0xC0, SER_LANE_SWAP_B0, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
		/* Swap = 0 <-> 2, 1 <-> 3 */
	{0xC0, SER_LANE_SWAP_B1, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
		/*1ms delay */
	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
		1, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/* Enable LINK_ENABLE */
	{0xC0, 0x7000, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	50, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/* Enable MST_VS0_DTG_ENABLE */
	{0xC0, 0x7A14, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Enable MST_VS1_DTG_ENABLE */
	{0xC0, 0x7B14, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Enable MST_VS2_DTG_ENABLE */
	{0xC0, 0x7C14, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Enable MST_VS3_DTG_ENABLE */
	{0xC0, 0x7D14, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/* Disable MST_VS0_DTG_ENABLE */
	{0xC0, 0x7A14, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Enable MST_VS0_DTG_ENABLE */
	{0xC0, 0x7A14, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Turn on video */
	{0xC0, 0x6420, 0x11, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Turn off video */
	{0xC0, 0x6420, 0x10, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Turn on video */
	{0xC0, 0x6420, 0x11, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/* VID_LINK_SEL_X, Y, Z, U of SER will be written 01 */
	{0xC0, 0x0100, 0x61, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0110, 0x61, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0120, 0x61, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0130, 0x61, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x04CF, 0xBF, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x05CF, 0xBF, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x06CF, 0xBF, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x07CF, 0xBF, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/*****************************************
	*      Configure GM03 DP_RX Payload IDs *
	*****************************************
	*/
	/*
	* Sets the MST payload ID of the video stream for video output
	* port 0, 1, 2, 3
	*/
	{0xC0, 0x7904, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7908, 0x02, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x790C, 0x03, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7910, 0x04, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Video FIFO Overflow Clear */
	{0xC0, 0x7A10, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* MST virtual sink device 0 enable */
	{0xC0, 0x7A00, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* DMA mode enable */
	{0xC0, 0x7A18, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7A24, 0x1F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7A26, 0x1F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7A14, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7B10, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7B00, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7B18, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7B24, 0x1F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7B26, 0x1F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7B14, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7C10, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7C00, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7C18, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7C24, 0x1F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7C26, 0x1F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7C14, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7D10, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7D00, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7D18, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7D24, 0x1F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7D26, 0x1F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7D14, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/*
	* MAIN_STREAM_ENABLE_MAIN_STREAM_ENABLE will be written 0001
	*/
	{0x90, 0x6184, 0x0F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0x90, DES_DROP_VIDEO, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x94, DES_DROP_VIDEO, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x98, DES_DROP_VIDEO, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xD0, DES_DROP_VIDEO, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0x90, DES_STREAM_SELECT, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x94, DES_STREAM_SELECT, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x98, DES_STREAM_SELECT, 0x02, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xD0, DES_STREAM_SELECT, 0x03, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/* EDP_VIDEO_CTRL0_VIDEO_OUT_EN of SER will be written 0000  */
	{0xC0, 0x6420, 0x10, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* EDP_VIDEO_CTRL0_VIDEO_OUT_EN of SER will be written 1111  */
	{0xC0, 0x6420, 0x1F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/* EDP_VIDEO_CTRL0_VIDEO_OUT_EN of SER will be written 0000  */
	{0xC0, 0x6420, 0x10, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* EDP_VIDEO_CTRL0_VIDEO_OUT_EN of SER will be written 1111  */
	{0xC0, 0x6420, 0x1F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/*****************************************
	*      Des & GPIO & I2C Setting         *
	*****************************************
	*/
	/* Enable Displays on each of the GMSL3 OLDIdes */
	/* 1st LCD */
	{0x90, 0x0005, 0x70, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x01CE, 0x4E, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* 2nd LCD */
	{0x94, 0x0005, 0x70, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x94, 0x01CE, 0x4E, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* 3rd LCD */
	{0x98, 0x0005, 0x70, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x98, 0x01CE, 0x4E, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* 4th LCD */
	{0xD0, 0x0005, 0x70, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xD0, 0x01CE, 0x4E, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/* LCD Reset 1 : Ser GPIO #1 RX/TX RX ID 1  --> LCD Reset #1 */
	{0xC0, 0x0208, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0209, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0233, 0x84, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0234, 0xB1, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Des1 GPIO #17 <-- RX/TX RX ID 1 */
	{0x90, 0x0235, 0x61, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/* LCD Reset 2(TCC8059) : Ser GPIO #1 RX/TX RX ID 1  --> LCD Reset 1 */
	{0x94, 0x0233, 0x84, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	{0x94, 0x0234, 0xB1, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	/* Des2 GPIO #17 <-- RX/TX RX ID 1 */
	{0x94, 0x0235, 0x61, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},

	/* LCD Reset 3(TCC8059) : Ser GPIO #1 RX/TX RX ID 1  --> LCD Reset 3 */
	{0x98, 0x0233, 0x84, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	{0x98, 0x0234, 0xB1, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	/* Des3 GPIO #17 <-- RX/TX RX ID 1 */
	{0x98, 0x0235, 0x61, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	{0xC0, 0x020B, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x020B, 0x21, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/*
	* LCD Reset 2(TCC8050) : Ser GPIO #11 RX/TX RX ID 11   -->
	* LCD Reset #2
	*/
	{0xC0, 0x0258, 0x01, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x0259, 0x0B, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x94, 0x0233, 0x84, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x94, 0x0234, 0xB1, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	/* Des2 GPIO #17 <-- RX/TX RX ID 11 */
	{0x94, 0x0235, 0x6B, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x025B, 0x20, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	/* Toggle */
	{0xC0, 0x025B, 0x21, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},

	/* LCD Reset 3 : Ser GPIO #15 RX/TX RX ID 15    --> LCD Reset 3 */
	{0xC0, 0x0278, 0x01, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x0279, 0x0F, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x98, 0x0233, 0x84, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x98, 0x0234, 0xB1, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	/* Des3 GPIO #17 <-- RX/TX RX ID 15 */
	{0x98, 0x0235, 0x6F, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x027B, 0x20, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x027B, 0x21, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},

	/* LCD Reset 4 : Ser GPIO #22 RX/TX RX ID 22    --> LCD Reset #4 */
	{0xC0, 0x02B0, 0x01, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x02B1, 0x16, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xD0, 0x0233, 0x84, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xD0, 0x0234, 0xB1, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	/* Des1 GPIO #17 <-- RX/TX RX ID 15 */
	{0xD0, 0x0235, 0x6F, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x02B3, 0x20, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x02B3, 0x21, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},

	/* LCD on : Ser GPIO #24 RX/TX RX ID 24 --> LCD On #1, 2, 3, 4 */
	{0xC0, 0x02C0, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x02C1, 0x18, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0236, 0x84, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0237, 0xB2, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Des1 GPIO #18 <-- RX/TX RX ID 24 */
	{0x90, 0x0238, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x94, 0x0236, 0x84, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x94, 0x0237, 0xB2, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Des2 GPIO #18 <-- RX/TX RX ID 24 */
	{0x94, 0x0238, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x98, 0x0236, 0x84, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x98, 0x0237, 0xB2, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Des3 GPIO #18 <-- RX/TX RX ID 24 */
	{0x98, 0x0238, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xD0, 0x0236, 0x84, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xD0, 0x0237, 0xB2, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	/* Des4 GPIO #18 <-- RX/TX RX ID 24 */
	{0xD0, 0x0238, 0x78, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x02C3, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Toggle */
	{0xC0, 0x02C3, 0x21, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/* Backlight on 1 : Ser GPIO #0 RX/TX RX ID 0 --> Backlight On 1 */
	{0xC0, 0x0200, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0201, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0068, 0x48, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0069, 0x48, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0206, 0x84, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0207, 0xA2, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Des1 GPIO #2 <-- RX/TX RX ID 0 */
	{0x90, 0x0208, 0x60, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0048, 0x08, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0049, 0x08, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/*
	* Backlight on 2(TCC8059) : Ser GPIO #0 RX/TX RX ID 0 -->
	* Backlight On 2
	*/
	{0x94, 0x0206, 0x84, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	{0x94, 0x0207, 0xA2, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	/* Des1 GPIO #2 <-- RX/TX RX ID 0 */
	{0x94, 0x0208, 0x60, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	{0x94, 0x0048, 0x08, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	{0x94, 0x0049, 0x08, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},

	/*
	* Backlight on 3(TCC8059) : Ser GPIO #0 RX/TX RX ID 0 -->
	* Backlight On 2
	*/
	{0x98, 0x0206, 0x84, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	{0x98, 0x0207, 0xA2, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	/* Des1 GPIO #2 <-- RX/TX RX ID 0 */
	{0x98, 0x0208, 0x60, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	{0x98, 0x0048, 0x08, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	{0x98, 0x0049, 0x08, TCC_EVB_LCD_ONE_POW, SER_REV_ALL},
	{0xC0, 0x0203, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0203, 0x21, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/* Backlight on 2 : Ser GPIO #0 RX/TX RX ID 5 --> Backlight On 2 */
	{0xC0, 0x0228, 0x01, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x0229, 0x05, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x94, 0x0206, 0x84, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x94, 0x0207, 0xA2, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	/* Des2 GPIO #2 <-- RX/TX RX ID 5 */
	{0x94, 0x0208, 0x65, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x94, 0x0048, 0x08, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x94, 0x0049, 0x08, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x022B, 0x20, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x022B, 0x21, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},

	/* Backlight on 3 : Ser GPIO #0 RX/TX RX ID 14 --> Backlight On 3*/
	{0xC0, 0x0270, 0x01, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x0271, 0x0E, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x98, 0x0206, 0x84, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x98, 0x0207, 0xA2, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	/* Des1 GPIO #2 <-- RX/TX RX ID 14 */
	{0x98, 0x0208, 0x6E, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x98, 0x0048, 0x08, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x98, 0x0049, 0x08, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x0273, 0x20, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x0273, 0x21, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},

	/* Backlight on 4 : Ser GPIO #0 RX/TX RX ID 21 --> Backlight On 4*/
	{0xC0, 0x02A8, 0x01, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x02A9, 0x15, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xD0, 0x0206, 0x84, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xD0, 0x0207, 0xA2, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	/* Des1 GPIO #2 <-- RX/TX RX ID 21 */
	{0xD0, 0x0208, 0x75, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xD0, 0x0048, 0x08, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0x98, 0x0049, 0x08, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x0273, 0x20, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	{0xC0, 0x0273, 0x21, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},
	/*****************************************
	*      I2C Setting                      *
	*****************************************
	*/
	/* Des1, 2, 3 GPIO #14 I2C Driving */
	{0x90, 0x020C, 0x90, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x94, 0x020C, 0x90, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x98, 0x020C, 0x90, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xD0, 0x020C, 0x90, TCC_EVB_LCD_FOUR_POW, SER_REV_ALL},

	{0x0, 0x0, 0x0, 0, SER_REV_ALL}
};

static struct Max968xx_Reg_Data pstVIC_1027_Max968xx_ES2_Reg[] = {
	{0x90, 0x0010, 0x80, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0010, 0x80, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0x90, 0x0308, 0x03, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x03E0, 0x07, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x14A6, 0x0F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x1460, 0x87, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x141F, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x1431, 0x08, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x141D, 0x02, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x14E1, 0x22, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0x90, 0x04D4, 0x43, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0423, 0x47, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x04E1, 0x22, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x03E0, 0x07, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0xC0, 0x0050, 0x66, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0x90, 0x001A, 0x10, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0022, 0x10, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0xC0, 0x0029, 0x02, TCC_EVB_LCD_POW_MAX},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	300, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0xC0, 0x6421, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x6420, 0x10, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7019, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7A14, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0xC0, 0x60AA, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x61AA, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x62AA, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x63AA, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x60B6, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x61B6, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x62B6, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x63B6, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x60A9, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x61A9, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x62A9, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x63A9, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x60B5, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x61B5, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x62B5, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x63B5, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x60A8, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x61A8, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x62A8, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x63A8, 0x78, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x60B4, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x61B4, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x62B4, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x63B4, 0x20, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6070, 0xA5, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6071, 0x65, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6170, 0xA5, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6171, 0x65, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6270, 0xA5, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6271, 0x65, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6370, 0xA5, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},
	{0xC0, 0x6371, 0x65, TCC_EVB_LCD_POW_MAX, SER_REV_ES2},

	{0xC0, 0x70A0, 0x04, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x6064, 0x06, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x6065, 0x06, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x6164, 0x06, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x6165, 0x06, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x6264, 0x06, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x6265, 0x06, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x6364, 0x06, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x6365, 0x06, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0xC0, 0x7000, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7054, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	1, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	/* Max rate : 1E -> 8.1Gbps, 14 -> 5.4Gbps, 0A -> 2.7Gbps */
	{0xC0, 0x7074, 0x0A, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Max lane count to 4*/
	{0xC0, 0x7070, 0x04, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0xC0, SER_LANE_SWAP_B0, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	/* Swap = 0 <-> 2, 1 <-> 3 */
	{0xC0, SER_LANE_SWAP_B1, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
		1, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0xC0, 0x7000, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	1, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0xC0, 0x7A18, 0x05, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7A28, 0xFF, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7A2A, 0xFF, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7A24, 0xFF, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7A27, 0x0F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x7A14, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x6420, 0x11, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x6420, 0x10, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x6420, 0x11, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0x90, 0x0005, 0x70, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x01CE, 0x4E, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0xC0, 0x0210, 0x40, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0211, 0x40, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0212, 0x0F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0213, 0x02, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0220, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0221, 0x04, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0223, 0x21, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0208, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0209, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x020B, 0x21, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x02C0, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x02C1, 0x18, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x02C3, 0x21, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0200, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0201, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0203, 0x21, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0068, 0x48, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0069, 0x48, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0x90, 0x022D, 0x43, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x022E, 0x6f, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x022F, 0x6f, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0230, 0x84, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0231, 0xb0, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0232, 0x44, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0233, 0x8c, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0234, 0xb1, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0235, 0x41, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0236, 0x84, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0237, 0xb2, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0238, 0x58, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0206, 0x84, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0207, 0xA2, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0208, 0x40, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0048, 0x08, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0049, 0x08, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0xC0, 0x009E, 0x00, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0079, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0006, 0x01, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x0071, 0x02, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0210, 0x60, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0212, 0x4F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0xC0, 0x0213, 0x02, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x022D, 0x63, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x022E, 0x6F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x022F, 0x2F, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x022A, 0x18, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x90, 0x020C, 0x80, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0x90, 0x020C, 0x90, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0x90, 0x020C, 0x80, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},

	{0x90, 0x020C, 0x90, TCC_EVB_LCD_POW_MAX, SER_REV_ALL},
	{0x0, 0x0, 0x0, 0, SER_REV_ALL}
};




static struct Max968xx_Reg_Params stmax968xx_reg_params[] = {
	{
	.uicustom_vic = 1027,
	.pstDES_ES3_Reg = pstVIC_1027_Max968xx_ES3_Reg,
	.pstDES_ES2_Reg = pstVIC_1027_Max968xx_ES2_Reg
	}
};


static struct MAX968XX_Drv_Params stmax968xx_params[MAX968XX_INPUT_STREAM_MAX] = {
{
	.ucevb_power_type = 0,
	.ucser_rev = 0,
	.ucdes_rev = 0,
	.iser_i2c_devadd = DP0_PANEL_SER_I2C_DEV_ADD,
	.ides_i2c_devadd = DP0_PANEL_DES_I2C_DEV_ADD,
	.pstser_i2c_udevice = NULL,
	.pstdes_i2c_udevice = NULL,
},
{
	.ucevb_power_type = 0,
	.ucser_rev = 0,
	.ucdes_rev = 0,
	.iser_i2c_devadd = DP1_PANEL_SER_I2C_DEV_ADD,
	.ides_i2c_devadd = DP1_PANEL_DES_I2C_DEV_ADD,
	.pstser_i2c_udevice = NULL,
	.pstdes_i2c_udevice = NULL,
},
{
	.ucevb_power_type = 0,
	.ucser_rev = 0,
	.ucdes_rev = 0,
	.iser_i2c_devadd = DP2_PANEL_SER_I2C_DEV_ADD,
	.ides_i2c_devadd = DP2_PANEL_DES_I2C_DEV_ADD,
	.pstser_i2c_udevice = NULL,
	.pstdes_i2c_udevice = NULL,
},
{
	.ucevb_power_type = 0,
	.ucser_rev = 0,
	.ucdes_rev = 0,
	.iser_i2c_devadd = DP3_PANEL_SER_I2C_DEV_ADD,
	.ides_i2c_devadd = DP3_PANEL_DES_I2C_DEV_ADD,
	.pstser_i2c_udevice = NULL,
	.pstdes_i2c_udevice = NULL,
},
};


static void max968xx_set_write_data(bool blane_reswap,
											uint8_t ucdp_id,
											uint8_t *pucw_buf,
											uint8_t ucnum_of_dps,
					struct Max968xx_Reg_Data *pstmax968xx_reg)
{
	uint8_t uclo_lane0, uclo_lane1, uclo_lane2, uclo_lane3;
	uint8_t ucreg_evb_t;
	uint32_t uireg_add;
	struct MAX968XX_Drv_Params *pstmax968xx_params;

	pstmax968xx_params = &stmax968xx_params[MAX968XX_INPUT_STREAM_0];

	ucreg_evb_t = pstmax968xx_reg->ucevb_power_type;
	uireg_add = pstmax968xx_reg->uireg_add;

	if (blane_reswap) {
		if (uireg_add == (uint32_t)SER_LANE_SWAP_B0) {
			uclo_lane0 = pstmax968xx_params->auclane_order[0];
			uclo_lane1 = pstmax968xx_params->auclane_order[1];
			uclo_lane1 = (uclo_lane1 << 2U);
			uclo_lane2 = pstmax968xx_params->auclane_order[2];
			uclo_lane2 = (uclo_lane1 << 4U);
			uclo_lane3 = pstmax968xx_params->auclane_order[3];
			uclo_lane3 = (uclo_lane1 << 6U);

			*pucw_buf = (uclo_lane0 | uclo_lane1 | uclo_lane2 | uclo_lane3);

			pr_info("[%s:%d]DP%d -> lane re-swap as 0x%x\n", __func__, __LINE__, ucdp_id, *pucw_buf);
		}

		if (uireg_add == (uint32_t)SER_LANE_SWAP_B1) {
			/* For KCS */
			*pucw_buf = 0x01U;
		}
	}

	if ((ucnum_of_dps == 1U) && (uireg_add == (uint32_t)SER_MISC_CONFIG_B1)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_debug("[%s:%d]Set to SST...", __func__, __LINE__);

		*pucw_buf = (uint8_t)MST_FUNCTION_DISABLE;
	}

	if ((ucnum_of_dps    > 1U) && (uireg_add == (uint8_t)DES_STREAM_SELECT)) {
		*pucw_buf = (pstmax968xx_params->aucvcp_id[ucdp_id] - 1U);

		pr_info("[%s:%d]DP%d -> Set VCP id to %u\n", __func__, __LINE__, ucdp_id, *pucw_buf);
	}

	if ((ucnum_of_dps == 1U) && (ucreg_evb_t == (uint8_t)TCC_EVB_LCD_ONE_POW) &&  (uireg_add == 0x0079U)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_info("[%s:%d] Serializer Enable only I2C PT 1(1Des)\n", __func__, __LINE__);

		/* TCC8059 SST - Enable PT1(1Des) */
		*pucw_buf = 0x01;
	}

	if ((ucnum_of_dps < 3U) && (ucreg_evb_t == (uint8_t)TCC_EVB_LCD_FOUR_POW) && (uireg_add == 0x0079U)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_info("[%s:%d]Serializer enables I2C PT1(1 Des), PT2(1Des)\n", __func__, __LINE__);

		/* TCC8050 SST/2MST - Enable PT1(1Des), PT2(1Des) */
		*pucw_buf = 0x03;
	}
}

static bool max968xx_check_continue(bool blane_reswap,
											uint8_t ucnum_of_dps,
											uint8_t ucser_rev,
											uint8_t ucevb_pow_type,
											struct Max968xx_Reg_Data *pstmax968xx_reg)
{
	bool bcontinue = (bool)false;
	uint8_t ucreg_evb_pow_t, ucreg_ser_rev;
	uint32_t uireg_add;

	ucreg_evb_pow_t = pstmax968xx_reg->ucevb_power_type;
	ucreg_ser_rev = pstmax968xx_reg->ucser_rev;
	uireg_add = pstmax968xx_reg->uireg_add;

	if ((ucreg_evb_pow_t != (uint8_t)TCC_EVB_LCD_POW_MAX) && (ucevb_pow_type != ucreg_evb_pow_t)) {
		bcontinue = (bool)true;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if ((ucreg_ser_rev != (uint8_t)SER_REV_ALL) && (ucser_rev != ucreg_ser_rev)) {
		bcontinue = (bool)true;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if ((ucnum_of_dps == 1U) && ((uireg_add == (uint32_t)DES_DROP_VIDEO) || (uireg_add == (uint32_t)DES_STREAM_SELECT))){
		bcontinue = (bool)true;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if ((!blane_reswap) && ((uireg_add == (uint32_t)SER_LANE_SWAP_B0) || (uireg_add == (uint32_t)SER_LANE_SWAP_B1))) {
		/* For KCS */
		bcontinue = (bool)true;
	}

return_funcs:
	return bcontinue;
}

static struct udevice *max968xx_get_i2c_udevice(uint8_t ucnum_of_dps, int32_t ii2c_add, uint8_t *pucdp_id)
{
	uint8_t ucdp_idx = 0;
	struct udevice *pstudevice = NULL;
	struct MAX968XX_Drv_Params *pstmax968xx_params;

	pstmax968xx_params = &stmax968xx_params[MAX968XX_INPUT_STREAM_0];
	
	if (pstmax968xx_params->iser_i2c_devadd == ii2c_add) {
		/* For KCS */
		pstudevice = pstmax968xx_params->pstser_i2c_udevice;
	} else {
		for (ucdp_idx = 0; ucdp_idx < ucnum_of_dps; ucdp_idx++) {
			pstmax968xx_params = &stmax968xx_params[ucdp_idx];

			if (pstmax968xx_params->ides_i2c_devadd == ii2c_add) {
				pstudevice = pstmax968xx_params->pstdes_i2c_udevice;
				break;
			}
		}
	}

	*pucdp_id = ucdp_idx;

	return pstudevice;
}

static int32_t max968xx_get_panel_board_info(uint8_t *pucser_rev,
															uint8_t *pucdes_rev,
															uint8_t *pucevb_pow_type)
{
	int32_t ret = 0;
	struct MAX968XX_Drv_Params *pstmax968xx_params;

	pstmax968xx_params = &stmax968xx_params[MAX968XX_INPUT_STREAM_0];

	if (pstmax968xx_params->pstser_i2c_udevice == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\n[%s:%d]Err: DP0 -> Ser I2C uclass isn't valid\n", __func__, __LINE__);

		ret = -ENXIO;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (pstmax968xx_params->pstdes_i2c_udevice == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\n[%s:%d]Err: DP0 -> Des I2C uclass isn't valid\n", __func__, __LINE__);

		ret = -ENXIO;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	*pucevb_pow_type = pstmax968xx_params->ucevb_power_type;
	*pucser_rev = pstmax968xx_params->ucser_rev;
	*pucdes_rev = pstmax968xx_params->ucdes_rev;

return_funcs:
	return ret;
}

static int32_t max968xx_set_reg_tables(uint8_t ucNumOfDPs, uint8_t ucTableIdx)
{
	bool bcontinue, blane_reswap = (bool)false;
	uint8_t ucdp_id;
	uint8_t ucw_data, uctry = 0, uclane_idx;
	uint8_t ucser_rev, ucdes_rev, ucevb_pow_type;
	int32_t ret = 0;
	uint32_t uielements, uiwritted_cnt = 0;
	struct Max968xx_Reg_Data *pstmax968xx_reg;
	struct udevice *pstudevice = NULL;
	struct MAX968XX_Drv_Params *pstmax968xx_params;

	pstmax968xx_params = &stmax968xx_params[MAX968XX_INPUT_STREAM_0];

	for (uclane_idx = 0; uclane_idx < DP_LANE_4; uclane_idx++) {
		if (pstmax968xx_params->auclane_order[uclane_idx] != uclane_idx) {
			blane_reswap = (bool)true;
			break;
		}
	}

	ret = max968xx_get_panel_board_info(&ucser_rev, &ucdes_rev, &ucevb_pow_type);
	if (ret != 0) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}


	pstmax968xx_reg = (ucdes_rev == (uint8_t)DES_REV_ES2) ?
						stmax968xx_reg_params[ucTableIdx].pstDES_ES2_Reg :
						stmax968xx_reg_params[ucTableIdx].pstDES_ES3_Reg;

	for (uielements = 0;
		((pstmax968xx_reg[uielements].ii2c_dev_add != 0) && (pstmax968xx_reg[uielements].uireg_add != 0U));
		uielements++) {

		if (pstmax968xx_reg[uielements].ii2c_dev_add == (int32_t)SERDES_DELAY_ADDR) {
			mdelay(pstmax968xx_reg[uielements].uireg_val);
			continue;
		}

		bcontinue = max968xx_check_continue(blane_reswap, ucNumOfDPs, ucser_rev, ucevb_pow_type, &pstmax968xx_reg[uielements]);

		if (bcontinue) {
			/* For KCS */
			continue;
		}

		pstudevice = max968xx_get_i2c_udevice(ucNumOfDPs, pstmax968xx_reg[uielements].ii2c_dev_add, &ucdp_id);
		if (pstudevice == NULL) {
			/* For KCS */
			continue;
		}

		ucw_data = (uint8_t)pstmax968xx_reg[uielements].uireg_val;

		max968xx_set_write_data(blane_reswap, ucdp_id, &ucw_data, ucNumOfDPs, &pstmax968xx_reg[uielements]);

		for (uctry = 0; uctry < (uint8_t)MAX_RETRY_I2C_RW; uctry++) {
			ret = dm_i2c_write(pstudevice, pstmax968xx_reg[uielements].uireg_add, &ucw_data, 1);
			if (ret == 0) {
				/* For coverity */
				break;
			}
		}

		if (ret != 0) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("\n[%s:%d]Error: writting fail on Dev(%d), Reg(%u), Idx(%u)\n",
					__func__,
					__LINE__,
					pstmax968xx_reg[uielements].ii2c_dev_add,
					pstmax968xx_reg[uielements].uireg_add,
					uielements);

			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto return_funcs;
		}

/*
		pr_err("\n[%u]Dev(0x%x) - Reg(0x%x) - Val(0x%x)", uiwritted_cnt,
														pstmax968xx_reg[uielements].ii2c_dev_add,
														pstmax968xx_reg[uielements].uireg_add,
														ucw_data);
*/
		uiwritted_cnt++;
	}

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_info("\n[%s:%d]%u SerDes register updating is successfully done!!!\n",
				__func__,
				__LINE__,
				uiwritted_cnt);

return_funcs:
	return ret;
}

static int max968xx_get_reg_table_idx(uint32_t uivic, uint8_t *puctable_idx)
{
	bool bfound_vic = (bool)false;
	uint8_t ucelements, ucnum_of_t;
	int32_t ret = 0;
	uint32_t uicus_vic;

	ucnum_of_t = (sizeof(stmax968xx_reg_params) / sizeof(struct Max968xx_Reg_Params));

	if (ucnum_of_t == 0U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\n[%s:%d]Err: there is no available reg table\n", __func__, __LINE__);

		ret = -ENXIO;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	for (ucelements = 0; ucelements < ucnum_of_t; ucelements++) {
		uicus_vic = stmax968xx_reg_params[ucelements].uicustom_vic;
		if (uicus_vic == uivic) {
			bfound_vic = (bool)true;
			break;
		}
	}

	
	if (bfound_vic) {
		pr_debug("[%s:%d]found vic from table %u\n", __func__, __LINE__, ucelements);
		*puctable_idx = ucelements;
	} else {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\n[%s:%d]Err: can't find VIC %u\n", __func__, __LINE__, uivic);

		ret = -ENXIO;

		*puctable_idx = 0;
	}

return_funcs:
	return ret;
}

static int32_t max968xx_get_des_rev(uint8_t ucdp_index, uint8_t uci2c_port)
{
	uint8_t uctry, ucdes_rev;
	int32_t ret = 0;
	int32_t ides_devadd = 0;
	uint32_t TempU32;
	struct MAX968XX_Drv_Params *pstmax968xx_params;

	pstmax968xx_params = &stmax968xx_params[ucdp_index];

	ides_devadd = pstmax968xx_params->ides_i2c_devadd;

	if (ides_devadd == (int32_t)INVALID_I2C_DEV_ADD) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (ides_devadd >= 0) {
		TempU32 = ((uint32_t)ides_devadd >> 1U);
		ides_devadd = (int32_t)TempU32;
	} else {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\n[%s:%d]Err: invalid Des dev add as 0x%x\n", __func__, __LINE__, (uint32_t)pstmax968xx_params->ides_i2c_devadd);

		ret = -EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	for (uctry = 0; uctry < (uint8_t)MAX_I2C_CHECK_RETRY; uctry++) {
		ret = i2c_get_chip_for_busnum((int)uci2c_port,
									ides_devadd,
									(uint)SER_DES_I2C_REG_ADD_LEN,
									&pstmax968xx_params->pstdes_i2c_udevice);
		if (ret == 0) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_debug("[%s:%d]Get Des uclass handle from add 0x%x\n", __func__, __LINE__, (uint32_t)pstmax968xx_params->ides_i2c_devadd);
			break;
		}
	}

	if (ret != 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\n[%s:%d]Err: fail to get uclass handle from add 0x%x\n", __func__, __LINE__, (uint32_t)pstmax968xx_params->ides_i2c_devadd);

		pstmax968xx_params->pstdes_i2c_udevice = NULL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	for (uctry = 0; uctry < (uint8_t)MAX_I2C_CHECK_RETRY; uctry++) {
		ret = dm_i2c_read(	pstmax968xx_params->pstdes_i2c_udevice,
							(uint)DES_DEV_REV,
							&ucdes_rev,
							1);
		if (ret == 0) {
			pstmax968xx_params->ucdes_rev = ucdes_rev;

			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_info("[%s:%d]Get Des revision as %u\n", __func__, __LINE__, ucdes_rev);
			break;
		}
	}

	if (ret != 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\n[%s:%d]Err: fails to get Des rev from add 0x%x\n", __func__, __LINE__, (uint32_t)pstmax968xx_params->ides_i2c_devadd);
	}

return_funcs:
	return ret;
}

static int32_t max968xx_get_ser_rev(uint8_t ucdp_index, uint8_t uci2c_port)
{
	uint8_t uctry, ucser_rev;
	int32_t ret = 0;
	int32_t iser_devadd = 0;
	uint32_t TempU32;
	struct MAX968XX_Drv_Params *pstmax968xx_params;

	pstmax968xx_params = &stmax968xx_params[ucdp_index];

	iser_devadd = pstmax968xx_params->iser_i2c_devadd;

	if (iser_devadd == (int32_t)INVALID_I2C_DEV_ADD) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	if (iser_devadd > 0) {
		TempU32 = ((uint32_t)iser_devadd >> 1U);
		iser_devadd = (int32_t)TempU32;
	} else {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\n[%s:%d]Err: invalid Ser dev add as 0x%x\n", __func__, __LINE__, (uint32_t)pstmax968xx_params->iser_i2c_devadd);

		ret = -EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	for (uctry = 0; uctry < (uint8_t)MAX_I2C_CHECK_RETRY; uctry++) {
		ret = i2c_get_chip_for_busnum((int)uci2c_port,
									iser_devadd,
									(uint)SER_DES_I2C_REG_ADD_LEN,
									&pstmax968xx_params->pstser_i2c_udevice);
		if (ret == 0) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_debug("[%s:%d]Get Ser uclass handle from add 0x%x\n", __func__, __LINE__, (uint32_t)pstmax968xx_params->iser_i2c_devadd);
			break;
		}
	}

	if (ret != 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\n[%s:%d]Err: fail to get uclass handle from add 0x%x\n", __func__, __LINE__, (uint32_t)pstmax968xx_params->iser_i2c_devadd);

		pstmax968xx_params->pstser_i2c_udevice = NULL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	for (uctry = 0; uctry < (uint8_t)MAX_I2C_CHECK_RETRY; uctry++) {
		ret = dm_i2c_read(	pstmax968xx_params->pstser_i2c_udevice,
							(uint)SER_DEV_REV,
							&ucser_rev,
							1);
		if (ret == 0) {
			pstmax968xx_params->ucser_rev = ucser_rev;

			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_info("[%s:%d]Get Ser rev as %u\n", __func__, __LINE__, ucser_rev);
			break;
		}
	}

	if (ret != 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\n[%s:%d]Err: fails to get Ser rev from add 0x%x\n", __func__, __LINE__, (uint32_t)pstmax968xx_params->iser_i2c_devadd);
	}

return_funcs:
	return ret;
}


static int32_t max968xx_set_i2c_speed(uint8_t uci2c_port)
{
	int32_t ret = 0;
	struct udevice *pstudevice = NULL;

	ret = uclass_get_device_by_seq(UCLASS_I2C, (int)uci2c_port, &pstudevice);
	if (ret != 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\n[%s:%d]Err: can't get i2C handle from uclass\n", __func__, __LINE__);

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	ret = dm_i2c_set_bus_speed(pstudevice, (unsigned int)SER_DES_I2C_SPEED);
	if (ret != 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("\n[%s:%d]Err: dm_i2c_set_bus_speed()\n", __func__, __LINE__);

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	pr_debug("\n[%s:%d]SerDes I2C speed to %d\n", __func__, __LINE__, SER_DES_I2C_SPEED);

return_funcs:
	return ret;
}

int32_t max968xx_init(struct MAX968XX_Init_Params *pstinit_params)
{
	uint8_t ucdp_idx, uctable_idx = 0;
	int32_t ret = 0;
	struct MAX968XX_Drv_Params *pstmax968xx_params;
	struct MAX968XX_Drv_Params *pstmax968xx_list;

	pstmax968xx_params = &stmax968xx_params[MAX968XX_INPUT_STREAM_0];

	pstmax968xx_params->ucevb_power_type = pstinit_params->ucevb_pow_type;

	(void)memcpy((void *)pstmax968xx_params->aucvcp_id, (void *)pstinit_params->aucvcp_id, sizeof(uint8_t) * MAX968XX_INPUT_STREAM_MAX);
	(void)memcpy((void *)pstmax968xx_params->auclane_order, (void *)pstinit_params->auclane_order, sizeof(uint8_t) * DP_LANE_4);

	(void)max968xx_set_i2c_speed(pstinit_params->uci2c_port);

	(void)max968xx_get_ser_rev((uint8_t)MAX968XX_INPUT_STREAM_0, pstinit_params->uci2c_port);

	for (ucdp_idx = 0; ucdp_idx < pstinit_params->ucnum_of_dps; ucdp_idx++) {
		(void)max968xx_get_des_rev(ucdp_idx, pstinit_params->uci2c_port);

		if (ucdp_idx != (uint8_t)MAX968XX_INPUT_STREAM_0) {
			pstmax968xx_list = &stmax968xx_params[ucdp_idx];

			if (pstmax968xx_params->ucdes_rev != pstmax968xx_list->ucdes_rev) {
				pr_notice("\n[%s:%d]DP0 Des rev(%u) is different from DP%u rev(%u)\n",
						__func__,
						__LINE__,
						pstmax968xx_params->ucdes_rev,
						ucdp_idx,
						pstmax968xx_list->ucdes_rev);
			}
		}
	}

	ret = max968xx_get_reg_table_idx(pstinit_params->uivic, &uctable_idx);
	if (ret == 0) {
		/* For KCS */
		(void)max968xx_set_reg_tables(pstinit_params->ucnum_of_dps, uctable_idx);
	}

	pr_info("\n[%s:%d]Init -> Max968xx\n", __func__, __LINE__);
	pr_info(" I2C Port : %u\n", pstinit_params->uci2c_port);
	pr_info(" Ser I2C add: 0x%x -> Des I2C add: 0x%x\n",
				pstmax968xx_params->iser_i2c_devadd,
				pstmax968xx_params->ides_i2c_devadd);
	pr_info(" Ser Rev. %u, Des Rev. %u\n",
				pstmax968xx_params->ucser_rev,
				pstmax968xx_params->ucdes_rev);
	pr_info(" Evb : %s\n",
				(pstmax968xx_params->ucevb_power_type == (uint8_t)TCC_EVB_LCD_ONE_POW) ? "One power port" :
				(pstmax968xx_params->ucevb_power_type == (uint8_t)TCC_EVB_LCD_FOUR_POW) ? "Four power port" :"Unknown");
	pr_info(" VIC : %u\n", pstinit_params->uivic);
	pr_info(" VCP id : %u %u %u %u\n\n",
				pstmax968xx_params->aucvcp_id[MAX968XX_INPUT_STREAM_0],
				pstmax968xx_params->aucvcp_id[MAX968XX_INPUT_STREAM_1],
				pstmax968xx_params->aucvcp_id[MAX968XX_INPUT_STREAM_2],
				pstmax968xx_params->aucvcp_id[MAX968XX_INPUT_STREAM_3]);

	return ret;
}

