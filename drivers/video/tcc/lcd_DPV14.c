// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/

#include <config.h>
#include <common.h>
#include <linux/compat.h>

#include <asm/arch/gpio.h>
#include <asm/io.h>
#include <asm/telechips/gpio.h>
#include <asm/telechips/vioc/vioc_global.h>
#include <asm/telechips/vioc/reg_physical.h>
#include <asm/telechips/vioc/vioc_disp.h>
#include <asm/telechips/vioc/vioc_blk.h>
#include <asm/telechips/vioc/vioc_config.h>
#include <asm/telechips/vioc/vioc_disp.h>
#include <asm/telechips/vioc/vioc_outcfg.h>
#include <asm/telechips/vioc/vioc_rdma.h>
#include <asm/telechips/vioc/vioc_wmix.h>
#include <mach/chipinfo.h>
#include <i2c.h>
#include <dm/uclass.h>

#include <lcd.h>
#include "Dptx_api.h"

#define TCC805X_ES_REVISION 0x00
#define TCC805X_CS_REVISION 0x01
#define TCC805X_BX_REVISION 0x02


/******* Ser Register *********/
#define SER_DEV_REV 0x000E
#define SER_REV_ES2 0x01
#define SER_REV_ES4 0x03
#define SER_REV_ALL 0x0F

#define LINK_ENABLE          0x7000
#define SER_MISC_CONFIG_B1   0x7019
#define MST_FUNCTION_DISABLE 0x00
#define MST_FUNCTION_ENABLE  0x01

#define LANE_REMAP_B0             0x7030
#define LANE_REMAP_B1             0x7031
#define MAX_LANE_COUNT            0x7070
#define MAX_LINK_RATE             0x7074
#define AUX_RD_INTERVAL           0x70A0
#define MST_VS0_ENABLE            0x7A00
#define MST_VS0_VID_FIFO_OVERFLOW 0x7A10
#define MST_VS0_DTG_ENABLE        0x7A14
#define MST_VS0_DTG_MODE          0x7A18
#define MST_VS0_DMA_VSYNC_B0      0x7A24
#define MST_VS0_DMA_VSYNC_B2      0x7A26
#define MST_VS1_ENABLE            0x7B00
#define MST_VS1_VID_FIFO_OVERFLOW 0x7B10
#define MST_VS1_DTG_ENABLE        0x7B14
#define MST_VS1_DTG_MODE          0x7B18
#define MST_VS1_DMA_VSYNC_B0      0x7B24
#define MST_VS1_DMA_VSYNC_B2      0x7B26
#define MST_VS2_ENABLE            0x7C00
#define MST_VS2_VID_FIFO_OVERFLOW 0x7C10
#define MST_VS2_DTG_ENABLE        0x7C14
#define MST_VS2_DTG_MODE          0x7C18
#define MST_VS2_DMA_VSYNC_B0      0x7C24
#define MST_VS2_DMA_VSYNC_B2      0x7C26
#define MST_VS3_ENABLE            0x7D00
#define MST_VS3_VID_FIFO_OVERFLOW 0x7D10
#define MST_VS3_DTG_ENABLE        0x7D14
#define MST_VS3_DTG_MODE          0x7D18
#define MST_VS3_DMA_VSYNC_B0      0x7D24
#define MST_VS3_DMA_VSYNC_B2      0x7D26
#define GPIOC_0_GPIO_A            0x0200
#define GPIOC_0_GPIO_B            0x0201
#define GPIOC_0_GPIO_C            0x0202
#define GPIOC_0_GPIO_D            0x0203
#define GPIOC_0_GPIO_E            0x0204
#define GPIOC_0_GPIO_F            0x0205
#define GPIOC_0_GPIO_G            0x0206

#define GPIOC_1_GPIO_A            0x0208
#define GPIOC_1_GPIO_B            0x0209
#define GPIOC_1_GPIO_C            0x020A
#define GPIOC_1_GPIO_D            0x020B
#define GPIOC_1_GPIO_E            0x020C
#define GPIOC_1_GPIO_F            0x020D
#define GPIOC_1_GPIO_G            0x020E

#define GPIO_2_GPIO_A             0x0210
#define GPIO_2_GPIO_B             0x0211
#define GPIO_2_GPIO_C             0x0212
#define GPIO_2_GPIO_D             0x0213

#define GPIO_A_STRENGTH_1MOHM              0x80 /* 0 : 40KOhm */
#define GPIO_A_HIGH_PRIORITY               0x40 /* 0 : Low priority */
#define GPIO_A_EN_JITTER_COMPENSATION      0x20 /* 0 : Disable */
#define GPIO_A_OUTPUT_TO_1                 0x10 /* 0 : To 0 */
#define GPIO_A_DISABEL_OUTPUT              0x01 /* 0 : Enable */

#define GPIO_B_BUFFER_NONE              0x00
#define GPIO_B_BUFFER_PULLDOWN          0x80
#define GPIO_B_BUFFER_PULLUP            0x40
#define GPIO_B_OUTPUT_TPYE_PUSH_PULL    0x20 /* 0 : Open drain */

/******* Des Register ********/
#define DES_DEV_REV          0x000E


#define DES_REV_ES2          0x01
#define DES_REV_ES3          0x02

#define DES_STREAM_SELECT    0x00A0
#define DES_DROP_VIDEO       0x0307


#define TCC8059_EVB_TYPE     0
#define TCC8050_EVB_TYPE     1
#define TCC8050_EMS_TYPE     2
#define TCC_ALL_EVB_TYPE     0x0F

#define VIC_TO_READ_EDID     0

#define DES_REG_VIDEO_RX8            0x0108
#define VIDEO_RX8_VID_LOCK_MASK      0x40
#define VIDEO_RX8_VID_PKT_DET_MASK   0x20

#define SER_REG_VIDEO_S0_TX2         0x0102
#define SER_REG_VIDEO_S1_TX2         0x0112
#define VIDEO_TX2_PCLKDET_MASK       0x80

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

#define DP_DEFAULT_HORIZONTAL_ACTIVE 1920
#define DP_DEFAULT_VERTICAL_ACTIVE   720
#define DP_DEFAULT_PIXEL_CLOCK       74250

#define SERDES_DELAY_ADDR               0xEFFF
#define DP_SER_DES_INVALID_REG_ADDR     0xFFFF


//Temp for GPIO handling of Coverity
#undef GPIO_FN0
#define GPIO_FN0     ((unsigned int)1U << GPIO_FN_SHIFT)

#undef GPIO_FN1
#define GPIO_FN1     ((unsigned int)2U << GPIO_FN_SHIFT)

#undef GPIO_INPUT
#define GPIO_INPUT      ((unsigned int)0x0002U)

#undef GPIO_OUTPUT
#define GPIO_OUTPUT     ((unsigned int)0x0001U)
//Temp for GPIO handling of Coverity

struct DP_V14_SER_DES_Reg_Data {
	int32_t         iDev_Addr;
	uint32_t        uiReg_Addr;
	uint32_t        uiReg_Val;
	uint8_t ucDeviceType;
	uint8_t ucSER_Revision;
};

struct DP_V14_Panel_Reg_Data_Params {
	uint32_t uiLVDS_VideoCode;
	struct DP_V14_SER_DES_Reg_Data *pstDES_ES3_Reg_t;
	struct DP_V14_SER_DES_Reg_Data *pstDES_ES2_Reg_t;
};

struct DP_V14_Params {
	bool bActivated;
	bool bPower_On;
	bool bDP_PanelMode;
	uint8_t ucSerDes_I2CPort;
	uint8_t ucNumOfDPs;
	uint8_t ucVCP_Id;
	uint32_t uiVideo_Code;
	uint8_t ucSER_Rev;
	uint8_t ucDES_Rev;
	int32_t iSER_I2C_DevAddr;
	int32_t iDES_I2C_DevAddr;
	struct udevice  *pstSER_I2C_Dev;
	struct udevice  *pstDES_I2C_Dev;
	struct vidinfo stDpv14_panel_info;
};



static struct DP_V14_SER_DES_Reg_Data pstDP_Panel_VIC_1027_DesES3_RegVal[] = {
	{0xD0, 0x0010, 0x80, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0010, 0x80, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0010, 0x80, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0010, 0x80, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0010, 0x80, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	10, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* AGC CR Init 8G1 */
	{0xC0, 0x60AA, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x61AA, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x62AA, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x63AA, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	/* BST CR Init 8G1 */
	{0xC0, 0x60B6, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x61B6, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x62B6, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x63B6, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	/* AGC CR Init 5G4 */
	{0xC0, 0x60A9, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x61A9, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x62A9, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x63A9, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	/* BST CR Init 5G4 */
	{0xC0, 0x60B5, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x61B5, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x62B5, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x63B5, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	/* AGC CR Init 2G7 */
	{0xC0, 0x60A8, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x61A8, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x62A8, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x63A8, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	/* BST CR Init 2G7 */
	{0xC0, 0x60B4, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x61B4, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x62B4, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x63B4, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	/* Set 8G1 Error Channel Phase */
	{0xC0, 0x6070, 0xA5, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6071, 0x65, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6170, 0xA5, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6171, 0x65, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6270, 0xA5, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6271, 0x65, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6370, 0xA5, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6371, 0x65, TCC_ALL_EVB_TYPE, SER_REV_ES2},

	/**********************
	* MST Setting        *
	**********************
	*/
	/* Turn off video-GM03 */
	{0xC0, 0x6420, 0x10, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Disable MST_VS0_DTG_ENABLE */
	{0xC0, 0x7A14, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Disable LINK_ENABLE */
	{0xC0, 0x7000, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Enable MST */
	{0xC0, SER_MISC_CONFIG_B1, MST_FUNCTION_ENABLE,
	TCC_ALL_EVB_TYPE, SER_REV_ALL}, /* 100ms delay */
	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/* Set AUX_RD_INTERVAL to 16ms */
	{0xC0, 0x70A0, 0x04, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Max rate : 1E -> 8.1Gbps, 14 -> 5.4Gbps, 0A -> 2.7Gbps */
	{0xC0, 0x7074, 0x1E, TCC_ALL_EVB_TYPE, SER_REV_ES4},
	{0xC0, 0x7074, 0x0A, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	/* Max lane count to 4 */
	{0xC0, 0x7070, 0x04, TCC_ALL_EVB_TYPE, SER_REV_ALL},

#if defined(CONFIG_SERDES_LANE02_13_SWAP)
	{0xC0, 0x7030, 0x4E, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Swap = 0 <-> 2, 1 <-> 3 */
	{0xC0, 0x7031, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/*1ms delay */
	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	1, TCC_ALL_EVB_TYPE, SER_REV_ALL},
#endif

	/* Enable LINK_ENABLE */
	{0xC0, 0x7000, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	50, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/* Enable MST_VS0_DTG_ENABLE */
	{0xC0, 0x7A14, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Enable MST_VS1_DTG_ENABLE */
	{0xC0, 0x7B14, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Enable MST_VS2_DTG_ENABLE */
	{0xC0, 0x7C14, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Enable MST_VS3_DTG_ENABLE */
	{0xC0, 0x7D14, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/* Disable MST_VS0_DTG_ENABLE */
	{0xC0, 0x7A14, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Enable MST_VS0_DTG_ENABLE */
	{0xC0, 0x7A14, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Turn on video */
	{0xC0, 0x6420, 0x11, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Turn off video */
	{0xC0, 0x6420, 0x10, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Turn on video */
	{0xC0, 0x6420, 0x11, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/* VID_LINK_SEL_X, Y, Z, U of SER will be written 01 */
	{0xC0, 0x0100, 0x61, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0110, 0x61, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0120, 0x61, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0130, 0x61, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x04CF, 0xBF, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x05CF, 0xBF, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x06CF, 0xBF, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x07CF, 0xBF, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/*****************************************
	*      Configure GM03 DP_RX Payload IDs *
	*****************************************
	*/
	/*
	* Sets the MST payload ID of the video stream for video output
	* port 0, 1, 2, 3
	*/
	{0xC0, 0x7904, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7908, 0x02, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x790C, 0x03, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7910, 0x04, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Video FIFO Overflow Clear */
	{0xC0, 0x7A10, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* MST virtual sink device 0 enable */
	{0xC0, 0x7A00, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* DMA mode enable */
	{0xC0, 0x7A18, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7A24, 0x1F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7A26, 0x1F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7A14, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7B10, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7B00, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7B18, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7B24, 0x1F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7B26, 0x1F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7B14, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7C10, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7C00, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7C18, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7C24, 0x1F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7C26, 0x1F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7C14, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7D10, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7D00, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7D18, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7D24, 0x1F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7D26, 0x1F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7D14, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/*
	* MAIN_STREAM_ENABLE_MAIN_STREAM_ENABLE will be written 0001
	*/
	{0x90, 0x6184, 0x0F, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0x90, DES_DROP_VIDEO, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x94, DES_DROP_VIDEO, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x98, DES_DROP_VIDEO, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xD0, DES_DROP_VIDEO, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},

#if defined(CONFIG_LCD0_DP_MST_VCP_ID)
	{0x90, DES_STREAM_SELECT, (CONFIG_LCD0_DP_MST_VCP_ID - 1),
	TCC_ALL_EVB_TYPE, SER_REV_ALL},
#else
	{0x90, DES_STREAM_SELECT, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
#endif

#if defined(CONFIG_LCD1_DP_MST_VCP_ID)
	{0x94, DES_STREAM_SELECT, (CONFIG_LCD1_DP_MST_VCP_ID - 1),
	TCC_ALL_EVB_TYPE, SER_REV_ALL},
#else
	{0x94, DES_STREAM_SELECT, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
#endif

#if defined(CONFIG_LCD2_DP_MST_VCP_ID)
	{0x98, DES_STREAM_SELECT, (CONFIG_LCD2_DP_MST_VCP_ID - 1),
	TCC_ALL_EVB_TYPE, SER_REV_ALL},
#else
	{0x98, DES_STREAM_SELECT, 0x02, TCC_ALL_EVB_TYPE, SER_REV_ALL},
#endif

#if defined(CONFIG_LCD3_DP_MST_VCP_ID)
	{0xD0, DES_STREAM_SELECT, (CONFIG_LCD3_DP_MST_VCP_ID - 1),
	TCC_ALL_EVB_TYPE, SER_REV_ALL},
#else
	{0xD0, DES_STREAM_SELECT, 0x03, TCC_ALL_EVB_TYPE, SER_REV_ALL},
#endif

	/* EDP_VIDEO_CTRL0_VIDEO_OUT_EN of SER will be written 0000  */
	{0xC0, 0x6420, 0x10, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* EDP_VIDEO_CTRL0_VIDEO_OUT_EN of SER will be written 1111  */
	{0xC0, 0x6420, 0x1F, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/* EDP_VIDEO_CTRL0_VIDEO_OUT_EN of SER will be written 0000  */
	{0xC0, 0x6420, 0x10, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* EDP_VIDEO_CTRL0_VIDEO_OUT_EN of SER will be written 1111  */
	{0xC0, 0x6420, 0x1F, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/*****************************************
	*      Des & GPIO & I2C Setting         *
	*****************************************
	*/
	/* Enable Displays on each of the GMSL3 OLDIdes */
	/* 1st LCD */
	{0x90, 0x0005, 0x70, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x01CE, 0x4E, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* 2nd LCD */
	{0x94, 0x0005, 0x70, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x01CE, 0x4E, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* 3rd LCD */
	{0x98, 0x0005, 0x70, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x01CE, 0x4E, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* 4th LCD */
	{0xD0, 0x0005, 0x70, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xD0, 0x01CE, 0x4E, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/* LCD Reset 1 : Ser GPIO #1 RX/TX RX ID 1  --> LCD Reset #1 */
	{0xC0, 0x0208, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0209, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0233, 0x84, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0234, 0xB1, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Des1 GPIO #17 <-- RX/TX RX ID 1 */
	{0x90, 0x0235, 0x61, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/* LCD Reset 2(TCC8059) : Ser GPIO #1 RX/TX RX ID 1  --> LCD Reset 1 */
	{0x94, 0x0233, 0x84, TCC8059_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0234, 0xB1, TCC8059_EVB_TYPE, SER_REV_ALL},
	/* Des2 GPIO #17 <-- RX/TX RX ID 1 */
	{0x94, 0x0235, 0x61, TCC8059_EVB_TYPE, SER_REV_ALL},

	/* LCD Reset 3(TCC8059) : Ser GPIO #1 RX/TX RX ID 1  --> LCD Reset 3 */
	{0x98, 0x0233, 0x84, TCC8059_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0234, 0xB1, TCC8059_EVB_TYPE, SER_REV_ALL},
	/* Des3 GPIO #17 <-- RX/TX RX ID 1 */
	{0x98, 0x0235, 0x61, TCC8059_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x020B, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x020B, 0x21, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/*
	* LCD Reset 2(TCC8050) : Ser GPIO #11 RX/TX RX ID 11   -->
	* LCD Reset #2
	*/
	{0xC0, 0x0258, 0x01, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0259, 0x0B, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0233, 0x84, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0234, 0xB1, TCC8050_EVB_TYPE, SER_REV_ALL},
	/* Des2 GPIO #17 <-- RX/TX RX ID 11 */
	{0x94, 0x0235, 0x6B, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x025B, 0x20, TCC8050_EVB_TYPE, SER_REV_ALL},
	/* Toggle */
	{0xC0, 0x025B, 0x21, TCC8050_EVB_TYPE, SER_REV_ALL},

	/* LCD Reset 3 : Ser GPIO #15 RX/TX RX ID 15    --> LCD Reset 3 */
	{0xC0, 0x0278, 0x01, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0279, 0x0F, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0233, 0x84, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0234, 0xB1, TCC8050_EVB_TYPE, SER_REV_ALL},
	/* Des3 GPIO #17 <-- RX/TX RX ID 15 */
	{0x98, 0x0235, 0x6F, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x027B, 0x20, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x027B, 0x21, TCC8050_EVB_TYPE, SER_REV_ALL},

	/* LCD Reset 4 : Ser GPIO #22 RX/TX RX ID 22    --> LCD Reset #4 */
	{0xC0, 0x02B0, 0x01, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x02B1, 0x16, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xD0, 0x0233, 0x84, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xD0, 0x0234, 0xB1, TCC8050_EVB_TYPE, SER_REV_ALL},
	/* Des1 GPIO #17 <-- RX/TX RX ID 15 */
	{0xD0, 0x0235, 0x6F, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x02B3, 0x20, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x02B3, 0x21, TCC8050_EVB_TYPE, SER_REV_ALL},

	/* LCD on : Ser GPIO #24 RX/TX RX ID 24 --> LCD On #1, 2, 3, 4 */
	{0xC0, 0x02C0, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x02C1, 0x18, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0236, 0x84, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0237, 0xB2, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Des1 GPIO #18 <-- RX/TX RX ID 24 */
	{0x90, 0x0238, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0236, 0x84, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0237, 0xB2, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Des2 GPIO #18 <-- RX/TX RX ID 24 */
	{0x94, 0x0238, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0236, 0x84, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0237, 0xB2, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Des3 GPIO #18 <-- RX/TX RX ID 24 */
	{0x98, 0x0238, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xD0, 0x0236, 0x84, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xD0, 0x0237, 0xB2, TCC8050_EVB_TYPE, SER_REV_ALL},
	/* Des4 GPIO #18 <-- RX/TX RX ID 24 */
	{0xD0, 0x0238, 0x78, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x02C3, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Toggle */
	{0xC0, 0x02C3, 0x21, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/* Backlight on 1 : Ser GPIO #0 RX/TX RX ID 0 --> Backlight On 1 */
	{0xC0, 0x0200, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0201, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0068, 0x48, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0069, 0x48, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0206, 0x84, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0207, 0xA2, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Des1 GPIO #2 <-- RX/TX RX ID 0 */
	{0x90, 0x0208, 0x60, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0048, 0x08, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0049, 0x08, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/*
	* Backlight on 2(TCC8059) : Ser GPIO #0 RX/TX RX ID 0 -->
	* Backlight On 2
	*/
	{0x94, 0x0206, 0x84, TCC8059_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0207, 0xA2, TCC8059_EVB_TYPE, SER_REV_ALL},
	/* Des1 GPIO #2 <-- RX/TX RX ID 0 */
	{0x94, 0x0208, 0x60, TCC8059_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0048, 0x08, TCC8059_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0049, 0x08, TCC8059_EVB_TYPE, SER_REV_ALL},

	/*
	* Backlight on 3(TCC8059) : Ser GPIO #0 RX/TX RX ID 0 -->
	* Backlight On 2
	*/
	{0x98, 0x0206, 0x84, TCC8059_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0207, 0xA2, TCC8059_EVB_TYPE, SER_REV_ALL},
	/* Des1 GPIO #2 <-- RX/TX RX ID 0 */
	{0x98, 0x0208, 0x60, TCC8059_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0048, 0x08, TCC8059_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0049, 0x08, TCC8059_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0203, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0203, 0x21, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/* Backlight on 2 : Ser GPIO #0 RX/TX RX ID 5 --> Backlight On 2 */
	{0xC0, 0x0228, 0x01, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0229, 0x05, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0206, 0x84, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0207, 0xA2, TCC8050_EVB_TYPE, SER_REV_ALL},
	/* Des2 GPIO #2 <-- RX/TX RX ID 5 */
	{0x94, 0x0208, 0x65, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0048, 0x08, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x0049, 0x08, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x022B, 0x20, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x022B, 0x21, TCC8050_EVB_TYPE, SER_REV_ALL},

	/* Backlight on 3 : Ser GPIO #0 RX/TX RX ID 14 --> Backlight On 3*/
	{0xC0, 0x0270, 0x01, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0271, 0x0E, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0206, 0x84, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0207, 0xA2, TCC8050_EVB_TYPE, SER_REV_ALL},
	/* Des1 GPIO #2 <-- RX/TX RX ID 14 */
	{0x98, 0x0208, 0x6E, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0048, 0x08, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0049, 0x08, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0273, 0x20, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0273, 0x21, TCC8050_EVB_TYPE, SER_REV_ALL},

	/* Backlight on 4 : Ser GPIO #0 RX/TX RX ID 21 --> Backlight On 4*/
	{0xC0, 0x02A8, 0x01, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x02A9, 0x15, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xD0, 0x0206, 0x84, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xD0, 0x0207, 0xA2, TCC8050_EVB_TYPE, SER_REV_ALL},
	/* Des1 GPIO #2 <-- RX/TX RX ID 21 */
	{0xD0, 0x0208, 0x75, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xD0, 0x0048, 0x08, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x0049, 0x08, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0273, 0x20, TCC8050_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0273, 0x21, TCC8050_EVB_TYPE, SER_REV_ALL},
	/*****************************************
	*      I2C Setting                      *
	*****************************************
	*/
	/* Des1, 2, 3 GPIO #14 I2C Driving */
	{0x90, 0x020C, 0x90, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x94, 0x020C, 0x90, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x98, 0x020C, 0x90, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xD0, 0x020C, 0x90, TCC8050_EVB_TYPE, SER_REV_ALL},

	{0x0, 0x0, 0x0, 0, SER_REV_ALL}
};

static struct DP_V14_SER_DES_Reg_Data pstDP_Panel_VIC_1027_DesES2_RegVal[] = {
	{0x90, 0x0010, 0x80, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0010, 0x80, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0x90, 0x0308, 0x03, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x03E0, 0x07, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x14A6, 0x0F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x1460, 0x87, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x141F, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x1431, 0x08, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x141D, 0x02, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x14E1, 0x22, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0x90, 0x04D4, 0x43, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0423, 0x47, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x04E1, 0x22, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x03E0, 0x07, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0xC0, 0x0050, 0x66, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0x90, 0x001A, 0x10, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0022, 0x10, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0xC0, 0x0029, 0x02, TCC_ALL_EVB_TYPE},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	300, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0xC0, 0x6421, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x6420, 0x10, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7019, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7A14, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0xC0, 0x60AA, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x61AA, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x62AA, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x63AA, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x60B6, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x61B6, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x62B6, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x63B6, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x60A9, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x61A9, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x62A9, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x63A9, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x60B5, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x61B5, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x62B5, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x63B5, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x60A8, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x61A8, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x62A8, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x63A8, 0x78, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x60B4, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x61B4, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x62B4, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x63B4, 0x20, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6070, 0xA5, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6071, 0x65, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6170, 0xA5, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6171, 0x65, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6270, 0xA5, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6271, 0x65, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6370, 0xA5, TCC_ALL_EVB_TYPE, SER_REV_ES2},
	{0xC0, 0x6371, 0x65, TCC_ALL_EVB_TYPE, SER_REV_ES2},

	{0xC0, 0x70A0, 0x04, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x6064, 0x06, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x6065, 0x06, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x6164, 0x06, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x6165, 0x06, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x6264, 0x06, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x6265, 0x06, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x6364, 0x06, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x6365, 0x06, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0xC0, 0x7000, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7054, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	1, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	/* Max rate : 1E -> 8.1Gbps, 14 -> 5.4Gbps, 0A -> 2.7Gbps */
	{0xC0, 0x7074, 0x0A, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Max lane count to 4*/
	{0xC0, 0x7070, 0x04, TCC_ALL_EVB_TYPE, SER_REV_ALL},

#if defined(CONFIG_SERDES_LANE02_13_SWAP)
	{0xC0, 0x7030, 0x4E, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	/* Swap = 0 <-> 2, 1 <-> 3 */
	{0xC0, 0x7031, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	1, TCC_ALL_EVB_TYPE, SER_REV_ALL},
#endif

	{0xC0, 0x7000, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	1, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0xC0, 0x7A18, 0x05, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7A28, 0xFF, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7A2A, 0xFF, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7A24, 0xFF, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7A27, 0x0F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x7A14, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x6420, 0x11, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x6420, 0x10, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x6420, 0x11, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0x90, 0x0005, 0x70, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x01CE, 0x4E, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0xC0, 0x0210, 0x40, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0211, 0x40, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0212, 0x0F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0213, 0x02, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0220, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0221, 0x04, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0223, 0x21, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0208, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0209, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x020B, 0x21, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x02C0, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x02C1, 0x18, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x02C3, 0x21, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0200, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0201, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0203, 0x21, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0068, 0x48, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0069, 0x48, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0x90, 0x022D, 0x43, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x022E, 0x6f, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x022F, 0x6f, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0230, 0x84, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0231, 0xb0, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0232, 0x44, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0233, 0x8c, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0234, 0xb1, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0235, 0x41, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0236, 0x84, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0237, 0xb2, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0238, 0x58, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0206, 0x84, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0207, 0xA2, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0208, 0x40, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0048, 0x08, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0049, 0x08, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0xC0, 0x009E, 0x00, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0079, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0006, 0x01, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x0071, 0x02, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0210, 0x60, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0212, 0x4F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0xC0, 0x0213, 0x02, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x022D, 0x63, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x022E, 0x6F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x022F, 0x2F, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x022A, 0x18, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x90, 0x020C, 0x80, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0x90, 0x020C, 0x90, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0x90, 0x020C, 0x80, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{SERDES_DELAY_ADDR, DP_SER_DES_INVALID_REG_ADDR,
	100, TCC_ALL_EVB_TYPE, SER_REV_ALL},

	{0x90, 0x020C, 0x90, TCC_ALL_EVB_TYPE, SER_REV_ALL},
	{0x0, 0x0, 0x0, 0, SER_REV_ALL}
};


static struct DP_V14_Panel_Reg_Data_Params stSerDes_RegData_t[] = {
	{
	.uiLVDS_VideoCode = 1027,
	.pstDES_ES3_Reg_t = pstDP_Panel_VIC_1027_DesES3_RegVal,
	.pstDES_ES2_Reg_t = pstDP_Panel_VIC_1027_DesES2_RegVal
	}
};


static int Dpv14_Panel_Init(const struct DP_Init_Params *pvParams);
static int Dpv14_set_power(unsigned char Params, int iPowerOn);
static int Dpv14_set_backlight_level(unsigned char Params, int iLevel);
static void dpv14_save_overlay_priority(
				void __iomem *WMixerBaseAddr,
				unsigned int *ovp);

static struct DP_V14_Params stDP_V14_Params_t[PANEL_DP_MAX] = {
{
	.bActivated             = false,
	.bPower_On              = false,
	.bDP_PanelMode          = true,
	.ucSerDes_I2CPort       = 0,
	.ucNumOfDPs             = 0,
	.uiVideo_Code           = 1027,  //0
	.ucSER_Rev              = 0,
	.ucDES_Rev              = 0,
	.iSER_I2C_DevAddr       = DP0_PANEL_SER_I2C_DEV_ADD,
	.iDES_I2C_DevAddr       = DP0_PANEL_DES_I2C_DEV_ADD,
	.pstSER_I2C_Dev         = NULL,
	.pstDES_I2C_Dev         = NULL,
		{
		.name           = "DPV14",
		.manufacturer   = "DispalyPortv14",
		.id             = PANEL_ID_DP0,
		.vl_bpix        = 5, // 32BPP
		.vl_rot         = 0, // 0
		.init           = NULL,
		.set_power      = NULL,
		.set_backlight_level = NULL,
		.init_ex        = Dpv14_Panel_Init,
		.set_power_ex   = Dpv14_set_power,
		.set_backlight_level_ex = Dpv14_set_backlight_level
		},
},
{
	.bActivated             = false,
	.bPower_On              = false,
	.bDP_PanelMode          = true,
	.ucSerDes_I2CPort       = 0,
	.ucNumOfDPs                     = 0,
	.uiVideo_Code           = 1027,
	.iSER_I2C_DevAddr       = DP1_PANEL_SER_I2C_DEV_ADD,
	.ucSER_Rev              = 0,
	.ucDES_Rev              = 0,
	.iDES_I2C_DevAddr       = DP1_PANEL_DES_I2C_DEV_ADD,
	.pstSER_I2C_Dev         = NULL,
	.pstDES_I2C_Dev         = NULL,
		{
		.name           = "DPV14",
		.manufacturer   = "DispalyPortv14",
		.id             = PANEL_ID_DP1,
		.vl_bpix        = 5, // 32BPP
		.vl_rot         = 0, // 0
		.init           = NULL,
		.set_power      = NULL,
		.set_backlight_level = NULL,
		.init_ex        = Dpv14_Panel_Init,
		.set_power_ex   = Dpv14_set_power,
		.set_backlight_level_ex = Dpv14_set_backlight_level
		},
},
{
	.bActivated             = false,
	.bPower_On              = false,
	.bDP_PanelMode          = true,
	.ucSerDes_I2CPort       = 0,
	.ucNumOfDPs             = 0,
	.uiVideo_Code           = 1027,
	.iSER_I2C_DevAddr       = DP2_PANEL_SER_I2C_DEV_ADD,
	.ucSER_Rev              = 0,
	.ucDES_Rev              = 0,
	.iDES_I2C_DevAddr       = DP2_PANEL_DES_I2C_DEV_ADD,
	.pstSER_I2C_Dev         = NULL,
	.pstDES_I2C_Dev         = NULL,
		{
		.name           = "DPV14",
		.manufacturer   = "DispalyPortv14",
		.id             = PANEL_ID_DP2,
		.vl_bpix        = 5, // 32BPP
		.vl_rot         = 0, // 0
		.init           = NULL,
		.set_power      = NULL,
		.set_backlight_level = NULL,
		.init_ex        = Dpv14_Panel_Init,
		.set_power_ex   = Dpv14_set_power,
		.set_backlight_level_ex = Dpv14_set_backlight_level
		},
},
{
	.bActivated             = false,
	.bPower_On              = false,
	.bDP_PanelMode          = true,
	.ucSerDes_I2CPort       = 0,
	.ucNumOfDPs             = 0,
	.uiVideo_Code           = 1027,
	.iSER_I2C_DevAddr       = DP3_PANEL_SER_I2C_DEV_ADD,
	.ucSER_Rev              = 0,
	.ucDES_Rev              = 0,
	.iDES_I2C_DevAddr       = DP3_PANEL_DES_I2C_DEV_ADD,
	.pstSER_I2C_Dev         = NULL,
	.pstDES_I2C_Dev         = NULL,
		{
		.name           = "DPV14",
		.manufacturer   = "DispalyPortv14",
		.id             = PANEL_ID_DP3,
		.vl_bpix        = 5, // 32BPP
		.vl_rot         = 0, // 0
		.init           = NULL,
		.set_power      = NULL,
		.set_backlight_level = NULL,
		.init_ex        = Dpv14_Panel_Init,
		.set_power_ex   = Dpv14_set_power,
		.set_backlight_level_ex = Dpv14_set_backlight_level
		},
	}
};

static void Reg32_Bitcset(void *reg,
								uint32_t uiMask,
								uint32_t uiData)
{
	uint32_t reg_val;

	reg_val = readl(reg);

	reg_val &= ~uiMask;
	reg_val |= uiData;

	writel(reg_val, reg);
}

static int dpv14_update_serdes_reg(uint8_t ucNumOfDPs,
												uint8_t ucTableIdx)
{
	uint8_t ucDPIdx, ucRW_Data, ucRetry = 0;
	uint8_t ucSER_Rev = 0;
	uint8_t ucDES_Rev = 0, ucBoardType;
	int32_t iRetVal = 0;
	uint32_t uiIdx;
	const struct DP_V14_Params *pstDP_V14_Params_t;
	const struct DP_V14_SER_DES_Reg_Data *pstSerDes_Reg_t;
	struct udevice *pstDev = NULL;

	for (ucDPIdx = 0; ucDPIdx < ucNumOfDPs; ucDPIdx++) {
		pstDP_V14_Params_t = &stDP_V14_Params_t[ucDPIdx];

		if (ucDPIdx == (u8)PANEL_DP0) {
			if ((pstDP_V14_Params_t->pstSER_I2C_Dev == NULL) ||
				(pstDP_V14_Params_t->pstDES_I2C_Dev == NULL)) {
				pr_err("\n[%s:%d]Error: DP 0->SerDes handle isn't valid\n",
							__func__,
							__LINE__);

				iRetVal = -ENXIO;
				goto return_funcs;
			}

			ucDES_Rev = pstDP_V14_Params_t->ucDES_Rev;
			ucSER_Rev = pstDP_V14_Params_t->ucSER_Rev;
		} else {
			if (pstDP_V14_Params_t->pstDES_I2C_Dev == NULL) {
				pr_notice("\n[%s:%d]DP %u Des handle is not valid\n",
							__func__,
							__LINE__,
							ucDPIdx);
			}
		}
	}

	ucBoardType = tcclcd_get_board_type();

	pr_info("\n              ucNumOfDPs : %u ",  ucNumOfDPs);
	pr_info("\n              Table index : %u ", ucTableIdx);
	pr_info("\n              Board type : %s\n",
	(ucBoardType == (u8)TCC8050_EVB_TYPE) ? "TCC8050 EVB" :
	(ucBoardType == (u8)TCC8050_EMS_TYPE) ? "TCC8050 EMS" :
	(ucBoardType == (u8)TCC8059_EVB_TYPE) ? "TCC8059 EVB":"Unknown");

	if (ucBoardType >  (uint8_t)TCC8050_EMS_TYPE) {
		pr_warn("\n[%s:%d]Unknown board type(%d)->set to TCC8050 EVB\n",
					__func__,
					__LINE__,
					ucBoardType);

		ucBoardType = (u8)TCC8050_EVB_TYPE;
	}

	if (ucBoardType != (u8)TCC8059_EVB_TYPE) {
		/*For coverity*/
		ucBoardType = (u8)TCC8050_EVB_TYPE;
	}

	if ((ucDES_Rev == (u8)DES_REV_ES2) && (ucNumOfDPs > 1U)) {
		pr_warn("\n[%s:%d]Warn: MST isn't supported by Des ES2\n",
				__func__,
				__LINE__);

		iRetVal = -ENXIO;
		goto return_funcs;
	}

	if (ucDES_Rev == (u8)DES_REV_ES2) {
		pstSerDes_Reg_t = stSerDes_RegData_t[ucTableIdx].pstDES_ES2_Reg_t;

		pr_notice("\n[%s:%d]Updating DES ES2 Tables... Revision(%u)",
					__func__,
					__LINE__,
					ucDES_Rev);
	} else {
		pstSerDes_Reg_t = stSerDes_RegData_t[ucTableIdx].pstDES_ES3_Reg_t;

		pr_notice("\n[%s:%d]Updating DES ES3 Tables... Revision(%u)",
					__func__,
					__LINE__,
					ucDES_Rev);
	}

	for (uiIdx = 0;
		!((pstSerDes_Reg_t[uiIdx].iDev_Addr == 0) &&
		(pstSerDes_Reg_t[uiIdx].uiReg_Addr == 0U) &&
		(pstSerDes_Reg_t[uiIdx].uiReg_Val == 0U));
		uiIdx++) {
		if (pstSerDes_Reg_t[uiIdx].iDev_Addr == (int32_t)SERDES_DELAY_ADDR) {
			mdelay(pstSerDes_Reg_t[uiIdx].uiReg_Val);
			continue;
		}

		pstDev = NULL;
		pstDP_V14_Params_t = &stDP_V14_Params_t[PANEL_DP0];

		if (pstDP_V14_Params_t->iSER_I2C_DevAddr ==
			pstSerDes_Reg_t[uiIdx].iDev_Addr) {
			/* For coverity */
			pstDev = pstDP_V14_Params_t->pstSER_I2C_Dev;
		} else {
			for (ucDPIdx = 0; ucDPIdx < ucNumOfDPs; ucDPIdx++) {
				pstDP_V14_Params_t = &stDP_V14_Params_t[ucDPIdx];

				if (pstDP_V14_Params_t->iDES_I2C_DevAddr ==
					pstSerDes_Reg_t[uiIdx].iDev_Addr) {
					pstDev = pstDP_V14_Params_t->pstDES_I2C_Dev;
					break;
				}
			}
		}

		if (pstDev == NULL) {
			/* For coverity */
			continue;
		}

		if ((pstSerDes_Reg_t[uiIdx].ucDeviceType != (u8)TCC_ALL_EVB_TYPE) &&
			(ucBoardType != pstSerDes_Reg_t[uiIdx].ucDeviceType)) {
			/* For coverity */
			continue;
		}

		if ((ucDES_Rev != (u8)DES_REV_ES2) &&
			(ucNumOfDPs == 1U) &&
			((pstSerDes_Reg_t[uiIdx].uiReg_Addr == (u32)DES_DROP_VIDEO) ||
			(pstSerDes_Reg_t[uiIdx].uiReg_Addr == (u32)DES_STREAM_SELECT))){
			/* For coverity */
			continue;
		}

		if ((pstSerDes_Reg_t[uiIdx].ucSER_Revision != (u8)SER_REV_ALL) &&
			(ucSER_Rev != pstSerDes_Reg_t[uiIdx].ucSER_Revision)) {
			/* For coverity */
			continue;
		}

		ucRW_Data = (u8)pstSerDes_Reg_t[uiIdx].uiReg_Val;

		if ((ucDES_Rev != (u8)DES_REV_ES2) &&
			(ucNumOfDPs == 1U) &&
			(pstSerDes_Reg_t[uiIdx].uiReg_Addr == (u32)SER_MISC_CONFIG_B1)) {

			pr_info("\n[%s:%d]Set to SST...", __func__, __LINE__);
			ucRW_Data = MST_FUNCTION_DISABLE;
		}
		if ((pstSerDes_Reg_t[uiIdx].ucDeviceType == (u8)TCC8059_EVB_TYPE) &&
			(ucNumOfDPs == 1U) &&
			(pstSerDes_Reg_t[uiIdx].uiReg_Addr == 0x0079U)) {

			pr_info("\n[%s:%d] Serializer Enable only I2C PT 1(1Des)\n",
					__func__,
					__LINE__);

			/* TCC8059 SST - Enable PT1(1Des) */
			ucRW_Data = 0x01;
		}
		if ((pstSerDes_Reg_t[uiIdx].ucDeviceType == (u8)TCC8050_EVB_TYPE) &&
			(ucNumOfDPs < 3U) &&
			(pstSerDes_Reg_t[uiIdx].uiReg_Addr == 0x0079U)) {

			pr_info("\n[%s:%d] Serializer Enable I2C PT1(1 Des), PT2(1Des)\n",
					__func__,
					__LINE__);

			/* TCC8050 SST/2MST - Enable PT1(1Des), PT2(1Des) */
			ucRW_Data = 0x03;
		}

		for (ucRetry = 0; ucRetry < (u8)MAX_RETRY_I2C_RW; ucRetry++) {
			iRetVal = dm_i2c_write(pstDev,
									pstSerDes_Reg_t[uiIdx].uiReg_Addr,
									&ucRW_Data,
									1);
			if (iRetVal == 0) {
				/* For coverity */
				break;
			}
		}

		if (iRetVal != 0) {
			pr_err("\n[%s:%d]Error: writting fail on Dev(%d), Reg(%u), Idx(%u)\n",
					__func__,
					__LINE__,
					pstSerDes_Reg_t[uiIdx].iDev_Addr,
					pstSerDes_Reg_t[uiIdx].uiReg_Addr,
					uiIdx);

			goto return_funcs;
		}
	}

	pr_notice("\n[%s:%d]I2C Resister update is successfully done !!!\n",
				__func__,
				__LINE__);
	pr_notice("written %u registers\n", uiIdx);

return_funcs:
	return iRetVal;
}

static int dpv14_get_panel_data_table_index(
					uint32_t uiVideoCode,
					uint8_t *pucTableIdx)
{
	uint8_t ucIdx;
	int32_t iRetVal = 0;
	uint32_t uiVIC;
	uint64_t ulNumOfTables;

	ulNumOfTables =
			(sizeof(stSerDes_RegData_t) /
			sizeof(struct DP_V14_Panel_Reg_Data_Params));

	if (ulNumOfTables == 0U) {
		pr_err("\n[%s:%d]Error: there is no panel data\n",
				__func__,
				__LINE__);

		iRetVal = -ENXIO;
	}

	if (iRetVal == 0) {
		for (ucIdx = 0; ucIdx < ulNumOfTables; ucIdx++) {
			uiVIC = stSerDes_RegData_t[ucIdx].uiLVDS_VideoCode;
			if (uiVIC == uiVideoCode) {
				/* For coverity */
				break;
			}
		}

		if (ucIdx == ulNumOfTables) {
			pr_err("\n[%s:%d]Error:Can't find VIC %u\n",
					__func__,
					__LINE__,
					uiVideoCode);

			iRetVal = -ENXIO;
		}

		*pucTableIdx = ucIdx;
	}

	return iRetVal;
}

static int32_t dpv14_init_ser_des(u8 ucDPIndex)
{
	uint8_t ucRetry ;
	uint8_t ucSer_Revision, ucDes_Revision;
	int32_t iRetVal = 0;
	int32_t iSerDevAdd = 0, iDesDevAdd = 0;
	struct udevice *pstBus = NULL;
	struct DP_V14_Params *pstDP_V14_Params_t;

	pstDP_V14_Params_t = &stDP_V14_Params_t[ucDPIndex];

	if (ucDPIndex == (u8)PANEL_DP0) {
		pr_notice("\n[%s:%d]SerDes I2C speed -> %d\n\n",
				__func__,
				__LINE__,
				SER_DES_I2C_SPEED);

		iRetVal = uclass_get_device_by_seq(
							UCLASS_I2C,
							(int)pstDP_V14_Params_t->ucSerDes_I2CPort,
							&pstBus);

		if (iRetVal == 0) {
			iRetVal = dm_i2c_set_bus_speed(
							pstBus,
							(unsigned int)SER_DES_I2C_SPEED);
			if (iRetVal != 0) {
				/* For coverity */
				pr_err("\n[%s:%d]Error: from dm_i2c_set_bus_speed() \n",
						__func__,
						__LINE__);
			}
		} else {
			pr_err("\n[%s:%d]Error: can't get i2c bus %u\n",
					__func__,
					__LINE__,
					(u32)pstDP_V14_Params_t->ucSerDes_I2CPort);
		}
	}

	if (pstDP_V14_Params_t->iSER_I2C_DevAddr != (int32_t)INVALID_I2C_DEV_ADD) {
		for (ucRetry = 0; ucRetry < (u8)MAX_I2C_CHECK_RETRY; ucRetry++) {

			iSerDevAdd = pstDP_V14_Params_t->iSER_I2C_DevAddr;

			if (iSerDevAdd >= 0) {
				uint32_t TempU32;

				TempU32 = ((uint32_t)iSerDevAdd >> 1U);
				iSerDevAdd = (int32_t)TempU32;
			} else {
				/* For coverity */
				iSerDevAdd = 0;
			}

			iRetVal = i2c_get_chip_for_busnum(
							(int)pstDP_V14_Params_t->ucSerDes_I2CPort,
							iSerDevAdd,
							(uint)SER_DES_I2C_REG_ADD_LEN,
							&pstDP_V14_Params_t->pstSER_I2C_Dev);
			if (iRetVal == 0) {
				pr_notice("[%s:%d]Get Ser I2C handle from add(%d) after %u trials\n",
							__func__,
							__LINE__,
							pstDP_V14_Params_t->iSER_I2C_DevAddr,
							(u32)ucRetry);
				break;
			}
		}

		if (iRetVal == 0) {
			for (ucRetry = 0; ucRetry < (u8)MAX_RETRY_I2C_RW; ucRetry++) {
				iRetVal = dm_i2c_read(  pstDP_V14_Params_t->pstSER_I2C_Dev,
										(uint)SER_DEV_REV,
										&ucSer_Revision,
										1);
				if (iRetVal == 0) {
					pstDP_V14_Params_t->ucSER_Rev = ucSer_Revision;

					pr_notice("[%s:%d]Get SER Revision Num as %u\n",
								__func__,
								__LINE__,
								(u32)(pstDP_V14_Params_t->ucSER_Rev));
					break;
				}
			}

			if (iRetVal != 0) {
				pr_err("\n[%s:%d]Error: fails to get Rev from add(%d)\n",
						__func__,
						__LINE__,
						pstDP_V14_Params_t->iSER_I2C_DevAddr);
			}
		} else {
			pr_err("\n[%s:%d]Error: fail to get Ser I2C handle from add(%d)\n",
					__func__,
					__LINE__,
					pstDP_V14_Params_t->iSER_I2C_DevAddr);

			pstDP_V14_Params_t->iSER_I2C_DevAddr = (int32_t)INVALID_I2C_DEV_ADD;
		}
	} 

	if (pstDP_V14_Params_t->iDES_I2C_DevAddr != (int32_t)INVALID_I2C_DEV_ADD) {
		for (ucRetry = 0; ucRetry < (u8)MAX_I2C_CHECK_RETRY; ucRetry++) {

			iDesDevAdd = pstDP_V14_Params_t->iDES_I2C_DevAddr;
			if (iDesDevAdd >= 0) {
				uint32_t TempU32;

				TempU32 = ((uint32_t)iDesDevAdd >> 1U);
				iDesDevAdd = (int32_t)TempU32;
			} else {
				/* For coverity */
				iDesDevAdd = 0;
			}

			iRetVal = i2c_get_chip_for_busnum(
						(int)pstDP_V14_Params_t->ucSerDes_I2CPort,
						iDesDevAdd,
						(uint)SER_DES_I2C_REG_ADD_LEN,
						&pstDP_V14_Params_t->pstDES_I2C_Dev);
			if (iRetVal == 0) {
				pr_notice("[%s:%d]Got DES I2C handle from add(%d) after %u times\n",
							__func__,
							__LINE__,
							pstDP_V14_Params_t->iDES_I2C_DevAddr,
							(u32)ucRetry);

				break;
			}
		}

		if (iRetVal == 0) {
			if (pstDP_V14_Params_t->iDES_I2C_DevAddr  == (int32_t)DP0_PANEL_DES_I2C_DEV_ADD) {
				for (ucRetry = 0; ucRetry < (u8)MAX_RETRY_I2C_RW; ucRetry++) {
					iRetVal = dm_i2c_read(  pstDP_V14_Params_t->pstDES_I2C_Dev,
											(uint)DES_DEV_REV,
											&ucDes_Revision,
											1);
					if (iRetVal == 0) {
						pstDP_V14_Params_t->ucDES_Rev = ucDes_Revision;

						pr_notice("[%s:%d]Get DES Rev as %u\n",
						__func__,
						__LINE__,
						(u32)(pstDP_V14_Params_t->ucDES_Rev));

						break;
					}
				}

			if (ucRetry == (uint8_t)MAX_RETRY_I2C_RW) {
				pr_err("\n[%s:%d]Error: fails to get Rev from Dev add(%d)\n",
						__func__,
						__LINE__,
						pstDP_V14_Params_t->iDES_I2C_DevAddr);

				iRetVal = -ENXIO;
			}
		}
	} else {
		pr_err("\n[%s:%d]Error: fail to get Des I2C handle from add(%d)\n",
					__func__,
					__LINE__,
					pstDP_V14_Params_t->iDES_I2C_DevAddr);

			pstDP_V14_Params_t->iDES_I2C_DevAddr = (int32_t)INVALID_I2C_DEV_ADD;
		}
	}

	return iRetVal;
}

static void dpv14_turn_on_display(uint8_t ucNumOfDPs)
{
	uint8_t ucIdx = 0;
	uint32_t TempU32;
	uint64_t lcdc_id;
	void __iomem *pioDispBaseAdd;
	const struct DP_V14_Params *pstParams_t;

	for (ucIdx = 0; ucIdx < ucNumOfDPs; ucIdx++) {
		pstParams_t = &stDP_V14_Params_t[ucIdx];

		TempU32 = pstParams_t->stDpv14_panel_info.dev.lcdc_id;
		lcdc_id = (uint64_t)TempU32;

		pioDispBaseAdd = (void __iomem *)TCC_VIOC_DISP_BASE(lcdc_id);

		VIOC_DISP_TurnOn(pioDispBaseAdd);
	}
}

static int dpv14_display_device_init(
							uint8_t ucDPIdx,
							uint8_t ucDC_Index,
							uint16_t usPanel_Width,
							uint16_t usPanel_Height,
							uint32_t uiVideoCode,
							uint32_t ovp)
{
	//uint8_t ucPixelEncoding;
	uint16_t TempU16 = 0;
	uint32_t uiSwapbf, uiWidth = 0, uiHeight = 0;
	uint32_t uiDispOffset_X = 0, uiDispOffset_Y = 0;
	uint32_t TempU32 = 0;
	int32_t iRetVal = 0;
	uint64_t ulDContIdx;
	void __iomem *pioDisplayBaseAddr, *pioWMixerBaseAddr;
	struct stLTIMING stLcd_Timing_Params;
	struct stLCDCTR stLcd_Ctrl_Params;
	struct DPTX_Dtd_Params_t stDptx_Dtd_Params;

	ulDContIdx = (uint64_t)ucDC_Index;

	pioDisplayBaseAddr = (void __iomem *)TCC_VIOC_DISP_BASE(ulDContIdx);
	pioWMixerBaseAddr = (void __iomem *)TCC_VIOC_WMIX_BASE(ulDContIdx);

	if (uiVideoCode == (uint32_t)VIC_TO_READ_EDID) {
		iRetVal = Dpv14_Tx_API_Get_Dtd_From_PreferredVIC(
							&stDptx_Dtd_Params,
							ucDPIdx);
		if (iRetVal != 0) {
			pr_err("\n[%s:%d]Error: fail to get timing from EDID\n",
					__func__,
					__LINE__);

			iRetVal = -ENXIO;
			goto return_funcs;
		}
	} else {
		iRetVal = Dpv14_Tx_API_Get_Dtd_From_VideoCode(
								uiVideoCode,
								&stDptx_Dtd_Params,
								(uint32_t)DTD_REFRESH_RATE_60000,
								(uint8_t)DTD_FORMAT_CEA_861);
		if (iRetVal != 0) {
			pr_err("\n[%s:%d]Error: fail to get timing from VIC(%u)\n",
					__func__,
					__LINE__,
					uiVideoCode);

			iRetVal = -ENXIO;
			goto return_funcs;
		}
	}

	pr_notice("\n[%s:%d]Display timing set from VIC(%u) :\n",
				__func__,
				__LINE__,
				uiVideoCode);
	pr_notice("             Pixel clk = %u\n",
				(u32)stDptx_Dtd_Params.uiPixel_Clock);
	pr_notice("             Repetition = %u\n",
				(u32)stDptx_Dtd_Params.pixel_repetition_input);
	pr_notice("             %s\n", (stDptx_Dtd_Params.interlaced) ?
				"Interlace" : "Progressive");
	pr_notice("             H Sync Polarity(%u), V Sync Polarity(%u)\n",
				(u32)stDptx_Dtd_Params.h_sync_polarity,
				(u32)stDptx_Dtd_Params.v_sync_polarity);
	pr_notice("             H Active(%u), V Active(%u)\n",
				(u32)stDptx_Dtd_Params.h_active,
				(u32)stDptx_Dtd_Params.v_active);
	pr_notice("             H Image size(%u), V Image size(%u)\n",
				(u32)stDptx_Dtd_Params.h_image_size,
				(u32)stDptx_Dtd_Params.v_image_size);
	pr_notice("             H Blanking(%u), V Blanking(%u)\n",
				(u32)stDptx_Dtd_Params.h_blanking,
				(u32)stDptx_Dtd_Params.v_blanking);
	pr_notice("             H Sync offset(%u), V Sync offset(%u)\n",
				(u32)stDptx_Dtd_Params.h_sync_offset,
				(u32)stDptx_Dtd_Params.v_sync_offset);
	pr_notice("             H Sync plus W(%u), V Sync plus W(%u)\n",
				(u32)stDptx_Dtd_Params.h_sync_pulse_width,
				(u32)stDptx_Dtd_Params.v_sync_pulse_width);

	//ucPixelEncoding = (uint8_t)LCD_PIXEL_ENCODING_TYPE;

	VIOC_DISP_TurnOff(pioDisplayBaseAddr);

	(void)memset(&stLcd_Ctrl_Params, 0, sizeof(stLcd_Ctrl_Params));
	(void)memset(&stLcd_Timing_Params, 0, sizeof(stLcd_Timing_Params));

	stLcd_Timing_Params.lpw = stDptx_Dtd_Params.h_sync_pulse_width;
	stLcd_Timing_Params.lpc = stDptx_Dtd_Params.h_active;

{
	if (stDptx_Dtd_Params.h_blanking >
		(stDptx_Dtd_Params.h_sync_offset + stDptx_Dtd_Params.h_sync_pulse_width)) {
		TempU16 = (stDptx_Dtd_Params.h_blanking -
					(stDptx_Dtd_Params.h_sync_offset +
					stDptx_Dtd_Params.h_sync_pulse_width));

		TempU32 = (uint32_t)TempU16;
		stLcd_Timing_Params.lswc = TempU32;
	}

	stLcd_Timing_Params.lewc = stDptx_Dtd_Params.h_sync_offset;
}

	if (stDptx_Dtd_Params.interlaced != 0U) {
		TempU16 = (((stDptx_Dtd_Params.v_sync_pulse_width << 1U) - 1U) & 0xFFFFU);
		TempU32 = (uint32_t)TempU16;
		stLcd_Timing_Params.fpw = TempU32;

		if (stDptx_Dtd_Params.v_blanking >
			(stDptx_Dtd_Params.v_sync_offset + stDptx_Dtd_Params.v_sync_pulse_width)) {
			TempU16 = (stDptx_Dtd_Params.v_blanking -
						(stDptx_Dtd_Params.v_sync_offset +
						stDptx_Dtd_Params.v_sync_pulse_width));
			TempU16 = (TempU16 << 1U);

			if (TempU16 > 0U) {
				/* For coverity */
				TempU16 = (TempU16 - 1U);
			}

			TempU32 = (uint32_t)TempU16;
			stLcd_Timing_Params.fswc = TempU32;
		}

		TempU16 = ((stDptx_Dtd_Params.v_sync_offset << 1) & 0xFFFFU);
		TempU32 = (uint32_t)TempU16;
		stLcd_Timing_Params.fewc = TempU32;

		stLcd_Timing_Params.fswc2 = (stLcd_Timing_Params.fswc + 1U);
		stLcd_Timing_Params.fewc2 = (stLcd_Timing_Params.fewc > 0U) ?
		(stLcd_Timing_Params.fewc - 1U) : 0U;

		stDptx_Dtd_Params.v_active <<= 1U;

		if (uiVideoCode == 39U) {
			/* For coverity */
			stLcd_Timing_Params.fewc -= 2U;
		}
	} else {
		TempU16 = ((stDptx_Dtd_Params.v_sync_pulse_width - 1U) & 0xFFFFU);
		TempU32 = (uint32_t)TempU16;
		stLcd_Timing_Params.fpw = TempU32;

		if (stDptx_Dtd_Params.v_blanking > 
			(stDptx_Dtd_Params.v_sync_offset +
			stDptx_Dtd_Params.v_sync_pulse_width)) {
			TempU16 = (((stDptx_Dtd_Params.v_blanking -
			(stDptx_Dtd_Params.v_sync_offset +
			stDptx_Dtd_Params.v_sync_pulse_width)) - 1U) & 0xFFFFU);

			TempU32 = (uint32_t)TempU16;
			stLcd_Timing_Params.fswc = TempU32;
		}

		TempU16 = ((stDptx_Dtd_Params.v_sync_offset - 1U) & 0xFFFFU);
		TempU32 = (uint32_t)TempU16;
		stLcd_Timing_Params.fewc = TempU32;

		stLcd_Timing_Params.fswc2 = stLcd_Timing_Params.fswc;
		stLcd_Timing_Params.fewc2 = stLcd_Timing_Params.fewc;
	}

	pr_info("               lpw = %u\n", (u32)stLcd_Timing_Params.lpw);
	pr_info("               lpc = %u\n", (u32)stLcd_Timing_Params.lpc);
	pr_info("               lswc = %u\n", (u32)stLcd_Timing_Params.lswc);
	pr_info("               fpw = %u\n", (u32)stLcd_Timing_Params.fpw);
	pr_info("               fswc = %u\n", (u32)stLcd_Timing_Params.fswc);
	pr_info("               fewc = %u\n", (u32)stLcd_Timing_Params.fewc);
	pr_info("               fswc2 = %u\n", (u32)stLcd_Timing_Params.fswc2);
	pr_info("               fewc2 = %u\n", (u32)stLcd_Timing_Params.fewc2);

{
	TempU16 = ((stDptx_Dtd_Params.v_active - 1U) & 0xFFFFU);
	TempU32 = (uint32_t)TempU16;
	stLcd_Timing_Params.flc = TempU32;

	stLcd_Timing_Params.fpw2 = stLcd_Timing_Params.fpw;
	stLcd_Timing_Params.flc2 = stLcd_Timing_Params.flc;

	TempU16 = ((stDptx_Dtd_Params.h_active >> 1U) & 0xFFFFU);
	uiWidth = (stDptx_Dtd_Params.pixel_repetition_input != 0U) ?
						TempU16 : stDptx_Dtd_Params.h_active;

	uiHeight = stDptx_Dtd_Params.v_active;
}

	VIOC_DISP_SetTimingParam(pioDisplayBaseAddr, &stLcd_Timing_Params);

	if (stDptx_Dtd_Params.interlaced != 0U) {
		/* For coverity */
		stLcd_Ctrl_Params.tv = (unsigned int)1U;
	} else {
		/* For coverity */
		stLcd_Ctrl_Params.ni = (unsigned int)1U;
	}

	stLcd_Ctrl_Params.iv = (stDptx_Dtd_Params.v_sync_polarity != 0U) ? 0U : 1U;
	stLcd_Ctrl_Params.ih = (stDptx_Dtd_Params.h_sync_polarity != 0U) ? 0U : 1U;
	stLcd_Ctrl_Params.dp = stDptx_Dtd_Params.pixel_repetition_input;

#if 1
	stLcd_Ctrl_Params.pxdw = (unsigned int)DCTRL_PXDW_RGB;
	stLcd_Ctrl_Params.r2y = 0U;
	uiSwapbf = 0;
#else
	switch (ucPixelEncoding) {
	case (uint8_t)VIDEO_ENCODING_RGB:
		stLcd_Ctrl_Params.pxdw = (unsigned int)DCTRL_PXDW_RGB;
		stLcd_Ctrl_Params.r2y = 0U;
		uiSwapbf = 0;
		break;
	case (uint8_t)VIDEO_ENCODING_YCBCR422:
		stLcd_Ctrl_Params.pxdw = (unsigned int)DCTRL_PXDW_YCBCR422;
		stLcd_Ctrl_Params.r2y = 1U;
		break;
	case (uint8_t)VIDEO_ENCODING_YCBCR444:
		stLcd_Ctrl_Params.pxdw = (unsigned int)DCTRL_PXDW_YCBCR444;
		stLcd_Ctrl_Params.r2y = 1U;
		break;
	case (uint8_t)VIDEO_ENCODING_MAX:
	default:
		pr_err("\n[%s:%d]Error: Invalid encoding type(%d)\n",
				__func__,
				__LINE__,
				ucPixelEncoding);
		stLcd_Ctrl_Params.pxdw = (unsigned int)DCTRL_PXDW_RGB;
		stLcd_Ctrl_Params.r2y = 0U;
		uiSwapbf = 0U;
		break;
	}
#endif
{
	VIOC_DISP_SetControlConfigure(pioDisplayBaseAddr, &stLcd_Ctrl_Params);

	/* VIOC_DISP api is not ready for SWAF */
	Reg32_Bitcset((pioDisplayBaseAddr + DALIGN),
					(uint32_t)DALIGN_SWAPAF_MASK,
					(uiSwapbf << (uint32_t)DALIGN_SWAPAF_SHIFT));

	/* wmixer control */
	VIOC_WMIX_SetOverlayPriority(pioWMixerBaseAddr, ovp);
	VIOC_WMIX_SetBGColor(pioWMixerBaseAddr, 0, 0, 0, 0);
	VIOC_WMIX_SetSize(pioWMixerBaseAddr, uiWidth, uiHeight);
}

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
	if ((usPanel_Width > (uint16_t)MAX_INDIVIDUAL_IMAGE_WIDTH) ||
		(usPanel_Height > (uint16_t)MAX_INDIVIDUAL_IMAGE_HEIGHT)) {
		if (usPanel_Width > (uint16_t)MAX_INDIVIDUAL_IMAGE_WIDTH) {
			TempU16 = ((usPanel_Width -
						(uint16_t)MAX_INDIVIDUAL_IMAGE_WIDTH) / 2U);

			uiDispOffset_X = (uint32_t)TempU16;
		}

		if (usPanel_Height > (uint16_t)MAX_INDIVIDUAL_IMAGE_HEIGHT) {
			TempU16 = ((usPanel_Height -
						(uint16_t)MAX_INDIVIDUAL_IMAGE_HEIGHT) / 2U);

			uiDispOffset_Y = (uint32_t)TempU16;
		}

		VIOC_WMIX_SetPosition(pioWMixerBaseAddr,
								0,
								uiDispOffset_X,
								uiDispOffset_Y);

		pr_info("[%s:%d]Panel size is larger than 1080p ->\n",
				__func__,
				__LINE__);
		pr_info("W(%u), H(%u), x offset(%u), y offset(%u)\n",
				(u32)usPanel_Width,
				(u32)usPanel_Height,
				(u32)uiDispOffset_X,
				(u32)uiDispOffset_Y);
		}       else {
		VIOC_WMIX_SetPosition(pioWMixerBaseAddr,
								0,
								(uiWidth > usPanel_Width) ?
								((uiWidth - usPanel_Width) >> 1U) : 0U,
								(uiHeight > usPanel_Height) ?
								((uiHeight - usPanel_Height) >> 1U) : 0U);
	}
#else
	VIOC_WMIX_SetPosition(pioWMixerBaseAddr,
							0,
							(uiWidth > usPanel_Width) ?
							((uiWidth - usPanel_Width) >> 1U) : 0U,
							(uiHeight > usPanel_Height) ?
							((uiHeight - usPanel_Height) >> 1U) : 0U);
#endif
	VIOC_WMIX_SetUpdate(pioWMixerBaseAddr);

	VIOC_DISP_SetSize(pioDisplayBaseAddr, uiWidth, uiHeight);
	VIOC_DISP_SetBGColor(pioDisplayBaseAddr, 0, 0, 0, 0);

return_funcs:
	return iRetVal;
}

static int dpv14_display_device_reset(
						unsigned int display_device_id,
						unsigned int *ovp)
{
	void __iomem *disp_reg_base, *rdma_reg_base;
	void __iomem *wmix_reg_base;
	unsigned int disp_id, wmix_id, rdma_id;
	int i, ret = 0;

	disp_id = ((unsigned int)VIOC_DISP0 + (unsigned int)(display_device_id & 0xFFU));
	wmix_id = ((unsigned int)VIOC_WMIX0 + (unsigned int)(display_device_id & 0xFFU));
	rdma_id = ((unsigned int)VIOC_RDMA00 + ((unsigned int)(display_device_id & 0xFFU) << 2U));

	disp_reg_base = VIOC_DISP_GetAddress(disp_id);
	wmix_reg_base = VIOC_WMIX_GetAddress(wmix_id);
	rdma_reg_base = VIOC_RDMA_GetAddress(rdma_id);

	if ((disp_reg_base == NULL) ||
		(wmix_reg_base == NULL) ||
		(rdma_reg_base == NULL)) {
		/* For coverity */
		goto return_funcs;
	}

	dpv14_save_overlay_priority(wmix_reg_base, ovp);

	ret = vioc_disp_get_turn_onoff(disp_reg_base);
	if (ret != 0) {
		VIOC_RDMA_SetImageDisable(rdma_reg_base);
		vioc_disp_clean_dd_status(disp_reg_base);
		VIOC_DISP_TurnOff(disp_reg_base);
		for (i = 0; i < 30; i++) {
			ret = vioc_disp_get_dd_status(disp_reg_base);
			if (ret != 0) {
				/* For coverity */
				break;
			}
			mdelay(1);
		}

		pr_info(
			"[INFO][DPV14] It takes %dms until 'display disable done' interrupt occurs\r\n",
			i);
	}

	VIOC_CONFIG_SWReset(disp_id, (unsigned int)VIOC_CONFIG_RESET);
	VIOC_CONFIG_SWReset(wmix_id, (unsigned int)VIOC_CONFIG_RESET);
	VIOC_CONFIG_SWReset(rdma_id, (unsigned int)VIOC_CONFIG_RESET);

	VIOC_CONFIG_SWReset(rdma_id, (unsigned int)VIOC_CONFIG_CLEAR);
	VIOC_CONFIG_SWReset(wmix_id, (unsigned int)VIOC_CONFIG_CLEAR);
	VIOC_CONFIG_SWReset(disp_id, (unsigned int)VIOC_CONFIG_CLEAR);

return_funcs:
	return 0;
}

static void dpv14_save_overlay_priority(
					void __iomem *WMixerBaseAddr,
					unsigned int *ovp)
{
	VIOC_WMIX_GetOverlayPriority(WMixerBaseAddr, ovp);

	pr_info("[INFO][DPV14] previous ovp value = %u\n", *ovp);

	if (*ovp == 5U) { //if reset value
		/* For coverity */
		*ovp = 24U; //set default ovp value
	}
}

static int Dpv14_Panel_Init(const struct DP_Init_Params *pvParams)
{
	uint8_t ucTableIdx = 0, ucDPIdx, ucBoardType;
	int32_t iRetVal = 0;
	uint32_t uiTCC805x_Rev;
	uint32_t TempU32;
	enum DPTX_LINK_RATE eLinkRate = LINK_RATE_HBR3;
	enum DPTX_LINK_LANE eLinkLanes = DPTX_PHY_LANE_4;
	struct DP_V14_Params *pstDP_V14_Params_t;
	struct vidinfo *pstDpPanel_Infor_t;
	struct DPTX_Dtd_Params_t stDptx_Dtd_Params;
	const struct DP_Init_Params *pstDP_Init_Params_t;

	pstDP_Init_Params_t = pvParams;

	if (pstDP_Init_Params_t->ucDP_Index >= (u_char)PANEL_DP_MAX) {

		pr_err("\n[%s:%d]Error: Invalid DP index %u\n",
				__func__,
				__LINE__,
				pstDP_Init_Params_t->ucDP_Index);

		iRetVal = -ENXIO;
		goto return_funcs;
	}

	pstDP_V14_Params_t =
	&stDP_V14_Params_t[pstDP_Init_Params_t->ucDP_Index];
	pstDpPanel_Infor_t =
	&stDP_V14_Params_t[pstDP_Init_Params_t->ucDP_Index].stDpv14_panel_info;

	if (pstDP_V14_Params_t->bActivated) {

		pr_err("\n[%s:%d]Error: alredy initialized.. index(%u)\n",
					__func__,
					__LINE__,
					pstDP_Init_Params_t->ucDP_Index);

		iRetVal = -ENXIO;
		goto return_funcs;
	}

	iRetVal = dpv14_display_device_reset(
				pstDpPanel_Infor_t->dev.lcdc_id,
				&pstDpPanel_Infor_t->ovp);
	if (iRetVal != 0) {
		/* For coverity */
		goto return_funcs;
	}

	if (pstDP_Init_Params_t->ucDP_Index == (u8)PANEL_DP0) {
		Dpv14_Tx_API_Release_ColdResetMask();
		Dpv14_Tx_API_Config_RegisterAccess_Mode(true);
		Dpv14_Tx_API_Config_PW();
		Dpv14_Tx_API_Config_CfgLock(true);
		Dpv14_Tx_API_Config_CfgAccess(true);
		Dpv14_Tx_API_Reset_PLL_Blk();

		tcclcd_gpio_config(TCC_GPC(14U),
			(unsigned int)(GPIO_INPUT | GPIO_FN1 | GPIO_PULLDISABLE));
	}

	pstDP_V14_Params_t->bActivated = true;
	pstDP_V14_Params_t->ucNumOfDPs = pstDP_Init_Params_t->ucNumOfDPs;
	pstDP_V14_Params_t->bDP_PanelMode =
			pstDP_Init_Params_t->bDP_PanelMode;
	pstDP_V14_Params_t->ucSerDes_I2CPort =
			pstDP_Init_Params_t->ucDP_I2CPort;
	pstDP_V14_Params_t->uiVideo_Code =
			pstDP_Init_Params_t->uiVideoCode;

	uiTCC805x_Rev = get_chip_rev();
	ucBoardType = tcclcd_get_board_type();

	pr_notice("\n[%s:%d]DP %u initializing on %s(%u) of %s(%u)\n",
				__func__,
				__LINE__,
				pstDP_Init_Params_t->ucDP_Index,
				(uiTCC805x_Rev == (uint32_t)TCC805X_ES_REVISION) ? "ES" :
				(uiTCC805x_Rev == (uint32_t)TCC805X_CS_REVISION) ? "CS" : "BX",
				uiTCC805x_Rev,
				(ucBoardType == (uint8_t)TCC8050_EVB_TYPE) ? "TCC8050 EVB" :
				(ucBoardType == (uint8_t)TCC8059_EVB_TYPE) ? "TCC8059 EVB" :
				(ucBoardType == (uint8_t)TCC8050_EMS_TYPE) ? "TCC8050 EMS" : "Unknown",
				ucBoardType);
	pr_notice("[%s:%d] -.DP %s\n",
				__func__,
				__LINE__,
				pstDP_Init_Params_t->bDP_PanelMode ?
				"Panel mode" : "Monitor mode");
	pr_notice("[%s:%d] -.The number of DPs is %u\n",
				__func__,
				__LINE__,
				pstDP_Init_Params_t->ucNumOfDPs);
	pr_notice("[%s:%d] -.DP VIC is %u\n",
				__func__,
				__LINE__,
				pstDP_Init_Params_t->uiVideoCode);
	pr_notice("[%s:%d] -.PHY Lane Swap : %s\n",
				__func__,
				__LINE__,
				pstDP_Init_Params_t->bDP_PHY_LaneSwap ? "On":"Off");
	pr_notice("[%s:%d] -.SDM Bypass %s, SRVC Bypass %s\n",
				__func__,
				__LINE__,
				pstDP_Init_Params_t->bDP_SDMBypass ? "On" : "Off",
				pstDP_Init_Params_t->bDP_SRVCBypass ? "On" : "Off");
	pr_notice("[%s:%d] -.Mux select: Mux %u -> DP %u\n",
				__func__,
				__LINE__,
				pstDpPanel_Infor_t->dev.lcdc_select,
				pstDP_Init_Params_t->ucDP_Index);

	if ((ucBoardType != (uint8_t)TCC8050_EVB_TYPE) &&
		(ucBoardType != (uint8_t)TCC8050_EMS_TYPE) &&
		(ucBoardType != (uint8_t)TCC8059_EVB_TYPE)) {
		pr_warn("\n[%s:%d]Warning: unknown board type(%u) ->\n",
				__func__,
				__LINE__,
				ucBoardType);
		pr_warn("set to TCC8050 EMS and CS revision by default\n");

		ucBoardType = (uint8_t)TCC8050_EMS_TYPE;
	}

	if (pstDP_Init_Params_t->bDP_PHY_LaneSwap) {
		if (uiTCC805x_Rev != (uint32_t)TCC805X_ES_REVISION) {
			/* For coverity */
			Dpv14_Tx_API_Config_PHY_StandardLane_PinConfig();
		} else {
			if (pstDP_Init_Params_t->ucDP_Index == (u8)PANEL_DP0) {
			/* For coverity */
			pr_warn("\n[%s:%d]Warning: Lane swap isn't supported in TCC805x ES\n",
						__func__,
						__LINE__);
			}
		}
	}

	if (pstDP_Init_Params_t->bDP_SDMBypass) {
		if (uiTCC805x_Rev != (uint32_t)TCC805X_ES_REVISION) {
			/* For coverity */
			Dpv14_Tx_API_Config_SDM_BypassControl(
					pstDP_Init_Params_t->bDP_SDMBypass);
		} else {
			/* For coverity */
			pr_warn("\n[%s:%d]Warning: SDM Bypass isn't supported in TCC805x ES\n",
						__func__,
						__LINE__);
		}
	}

	if (pstDP_Init_Params_t->bDP_SRVCBypass) {
		if (uiTCC805x_Rev != (uint32_t)TCC805X_ES_REVISION) {
			/* For coverity */
			Dpv14_Tx_API_Config_SRVC_BypassControl(
			pstDP_Init_Params_t->bDP_SRVCBypass);
		} else {
			/* For coverity */
			pr_warn("\n[%s:%d]Warning: SRVC Bypass isn't supported in TCC805x ES\n",
						__func__,
						__LINE__);
		}
	}

	if (pstDP_Init_Params_t->ucDP_Index !=
		pstDpPanel_Infor_t->dev.lcdc_select) {
		if (uiTCC805x_Rev != (uint32_t)TCC805X_ES_REVISION) {
			/* For coverity */
			Dpv14_Tx_API_Config_MuxSelect(
						pstDpPanel_Infor_t->dev.lcdc_select,
						pstDP_Init_Params_t->ucDP_Index);
		} else {
			/* For coverity */
			pr_warn("\n[%s:%d]Warning: mux %u is tring to connect DP %u in TCC805x ES\n",
						__func__,
						__LINE__,
						pstDpPanel_Infor_t->dev.lcdc_select,
						pstDP_Init_Params_t->ucDP_Index);
		}
	}

	if ((pstDP_Init_Params_t->bDP_PanelMode) &&
		(pstDP_Init_Params_t->ucDP_Index == (u8)PANEL_DP0)) {
		for (ucDPIdx = 0;
				ucDPIdx < pstDP_Init_Params_t->ucNumOfDPs;
				ucDPIdx++) {
			iRetVal = dpv14_init_ser_des(ucDPIdx);
			if (iRetVal != 0) {
				/* For coverity */
				pr_err("\n[%s:%d]Err from dpv14_init_ser_des()\n",
						__func__,
						__LINE__);
			}
		}

		iRetVal = dpv14_get_panel_data_table_index(
						pstDP_Init_Params_t->uiVideoCode,
						&ucTableIdx);

		if (iRetVal == 0) {
			/* For coverity */
			iRetVal = dpv14_update_serdes_reg(
						pstDP_Init_Params_t->ucNumOfDPs,
						ucTableIdx);
			if (iRetVal != 0) {
				/* For coverity */
				pr_err("\n[%s:%d]Err from dpv14_update_serdes_reg()\n",
				__func__,
				__LINE__);
			}
		}

		if (ucBoardType == (uint8_t)TCC8050_EVB_TYPE) {
			tcclcd_gpio_config(TCC_GPE(19U),
				(unsigned int)(GPIO_INPUT | GPIO_FN0 | GPIO_PULLDISABLE));
			tcclcd_gpio_config(TCC_GPMC(0U),
				(unsigned int)(GPIO_INPUT | GPIO_FN0 | GPIO_PULLDISABLE));
		} else if (ucBoardType == (uint8_t)TCC8050_EMS_TYPE) {
			tcclcd_gpio_config(TCC_GPSD0(11U),
				(unsigned int)(GPIO_INPUT | GPIO_FN0 | GPIO_PULLDISABLE));
			tcclcd_gpio_config(TCC_GPMB(5U),
				(unsigned int)(GPIO_INPUT | GPIO_FN0 | GPIO_PULLDISABLE));
		} else if (ucBoardType == (uint8_t)TCC8059_EVB_TYPE) {
			tcclcd_gpio_config(TCC_GPE(19U),
				(unsigned int)(GPIO_INPUT | GPIO_FN0 | GPIO_PULLDISABLE));
		} else {
			/* For coverity */
			pr_notice("\n[%s:%d]Unknown board type(%u)\n\n",
						__func__,
						__LINE__,
						ucBoardType);
		}
	}

	if (pstDP_Init_Params_t->bDP_PanelMode) {
		tcclcd_gpio_config(pstDpPanel_Infor_t->dev.display_on,
			(unsigned int)(GPIO_OUTPUT | GPIO_FN0));
		tcclcd_gpio_config(pstDpPanel_Infor_t->dev.bl_on,
			(unsigned int)(GPIO_OUTPUT | GPIO_FN0));
		tcclcd_gpio_config(pstDpPanel_Infor_t->dev.reset,
			(unsigned int)(GPIO_OUTPUT | GPIO_FN0));
		tcclcd_gpio_config(pstDpPanel_Infor_t->dev.power_on,
			(unsigned int)(GPIO_OUTPUT | GPIO_FN0));

		tcclcd_gpio_set_value(pstDpPanel_Infor_t->dev.display_on, 0);
		tcclcd_gpio_set_value(pstDpPanel_Infor_t->dev.bl_on, 0);
		tcclcd_gpio_set_value(pstDpPanel_Infor_t->dev.reset, 0);
		tcclcd_gpio_set_value(pstDpPanel_Infor_t->dev.power_on, 0);
	}

	if (pstDP_Init_Params_t->ucDP_Index == (u8)PANEL_DP0) {
		if (uiTCC805x_Rev == (uint32_t)TCC805X_ES_REVISION) {
			/* For coverity */
			eLinkRate = LINK_RATE_HBR2;
		}

		iRetVal = Dpv14_Tx_API_Init(
							pstDP_Init_Params_t->ucNumOfDPs,
							eLinkRate,
							eLinkLanes);
		if (iRetVal != 0) {
			iRetVal = -ENXIO;
			goto return_funcs;
		}
	}

	if (pstDP_V14_Params_t->uiVideo_Code == (uint32_t)VIC_TO_READ_EDID) {
		if (pstDP_Init_Params_t->bDP_PanelMode) {
			pr_err("\n[%s:%d]Error: Reading EDID isn't supported in Panel\n",
						__func__,
						__LINE__);

			iRetVal = -ENXIO;
			goto return_funcs;
		}

		iRetVal = Dpv14_Tx_API_Get_Dtd_From_Edid(
							&stDptx_Dtd_Params,
							pstDP_Init_Params_t->ucDP_Index);
		if (iRetVal != 0) {
			pr_err("\n[%s:%d]Error: fail to get dtd from EDID\n",
						__func__,
						__LINE__);

			goto return_funcs;

		}
	} else {
		iRetVal = Dpv14_Tx_API_Get_Dtd_From_VideoCode(
						pstDP_V14_Params_t->uiVideo_Code,
						&stDptx_Dtd_Params,
						(u32)DTD_REFRESH_RATE_60000,
						(u8)DTD_FORMAT_CEA_861);
		if (iRetVal != 0) {
			pr_err("\n[%s:%d]Error: Can't find dtd from DTD\n",
						__func__,
						__LINE__);

			goto return_funcs;
		}
	}

	pstDpPanel_Infor_t->vl_col = stDptx_Dtd_Params.h_active;
	pstDpPanel_Infor_t->vl_row = stDptx_Dtd_Params.v_active;

	TempU32 = stDptx_Dtd_Params.uiPixel_Clock;
	pstDpPanel_Infor_t->clk_freq = (u_long)TempU32;
	pstDpPanel_Infor_t->clk_freq *= 1000U;

	pstDpPanel_Infor_t->clk_div = 0;

	pr_info("[%s:%d]Found dtd from VIC %u -> Resolution = %u x %u\n",
				__func__,
				__LINE__,
				pstDP_Init_Params_t->uiVideoCode,
				stDptx_Dtd_Params.h_active,
				stDptx_Dtd_Params.v_active);

return_funcs:
	return iRetVal;
}

static int Dpv14_set_power(unsigned char Params, int iPowerOn)
{
	uint8_t ucSink_Supports_SideBand_MSG, ucDPIdx;
	uint8_t aucVCP_id[DPTX_INPUT_STREAM_MAX] = { 0, };
	int32_t iRetVal = 0;
	uint32_t uiTCC805x_Rev;
	uint32_t auiDefaultVideoCode[DPTX_INPUT_STREAM_MAX] = { 0, };
	uint32_t auiPeri_PixelClk[DPTX_INPUT_STREAM_MAX] = { 0, };
	enum DPTX_LINK_RATE eLinkRate = LINK_RATE_HBR3;
	enum DPTX_LINK_LANE  eLinkLanes = DPTX_PHY_LANE_4;
	struct DP_V14_Params *pstDP_V14_Params_t;
	const struct DP_V14_Params *pstParams_t;
	struct vidinfo *pstDpPanel_Infor_t;

	ucDPIdx = Params;

	if (ucDPIdx >= (uint8_t)PANEL_DP_MAX) {

	pr_err("\n[%s:%d]Error: Invalid DP index %u\n",
		__func__,
		__LINE__,
		ucDPIdx);

		iRetVal = -ENXIO;
		goto return_funcs;
	}

	pstDP_V14_Params_t = &stDP_V14_Params_t[ucDPIdx];
	pstDpPanel_Infor_t = &stDP_V14_Params_t[ucDPIdx].stDpv14_panel_info;

	if (!pstDP_V14_Params_t->bActivated) {

		pr_err("\n[%s:%d]Error: Index %u wasn't initialized\n",
					__func__,
					__LINE__,
					ucDPIdx);

		iRetVal = -ENXIO;
		goto return_funcs;
	}

	pr_notice("\n[%s:%d]DP %u Power %s =>",
				__func__,
				__LINE__,
				ucDPIdx,
				iPowerOn ? "On" : "Off");
	pr_notice("\n                 Display Controller %u",
				pstDpPanel_Infor_t->dev.lcdc_id);
	pr_notice("\n                 DEV%d_PATH",
				pstDpPanel_Infor_t->dev.lcdc_select);
	pr_notice("\n                 Display as %u x %u",
				pstDpPanel_Infor_t->vl_col,
				pstDpPanel_Infor_t->vl_row);
	pr_notice("\n                 Wmixer ovp = %u\n",
				pstDpPanel_Infor_t->ovp);

	if (iPowerOn != 0) {
		pstDP_V14_Params_t->bPower_On = (bool)true;

		if (pstDP_V14_Params_t->bDP_PanelMode == (bool)true) {
			tcclcd_gpio_set_value(
						pstDpPanel_Infor_t->dev.power_on,
						1);

			udelay(20);

			tcclcd_gpio_set_value(
						pstDpPanel_Infor_t->dev.reset,
						1);
			tcclcd_gpio_set_value(
						pstDpPanel_Infor_t->dev.display_on,
						1);
		}

		iRetVal = lcdc_mux_select(
					pstDpPanel_Infor_t->dev.lcdc_select,
					pstDpPanel_Infor_t->dev.lcdc_id);
		if (iRetVal != 0) {
			/* For coverity */
			goto return_funcs;
		}

		iRetVal = dpv14_display_device_init(
						ucDPIdx,
						(uint8_t)pstDpPanel_Infor_t->dev.lcdc_id,
						pstDpPanel_Infor_t->vl_col,
						pstDpPanel_Infor_t->vl_row,
						pstDP_V14_Params_t->uiVideo_Code,
						pstDpPanel_Infor_t->ovp);
		if (iRetVal != 0) {
			/* For coverity */
			goto return_funcs;
		}

		if ((ucDPIdx + 1U) == pstDP_V14_Params_t->ucNumOfDPs) {
			(void)memset(auiDefaultVideoCode,
						0,
						(sizeof(uint32_t) * (uint32_t)DPTX_INPUT_STREAM_MAX));
			(void)memset(auiPeri_PixelClk,
						0,
						(sizeof(uint32_t) * (uint32_t)DPTX_INPUT_STREAM_MAX));

			for (uint8_t ucElements = 0;
					ucElements < pstDP_V14_Params_t->ucNumOfDPs;
					ucElements++) {

				pstParams_t = &stDP_V14_Params_t[ucElements];

				auiDefaultVideoCode[ucElements] = pstParams_t->uiVideo_Code;
				auiPeri_PixelClk[ucElements] = 
				(uint32_t)(pstParams_t->stDpv14_panel_info.clk_freq & 0xFFFFFFFFU);

				aucVCP_id[ucElements] = (ucElements + 1U);
			}

#if defined(CONFIG_LCD0_DP_MST_VCP_ID)
			aucVCP_id[0] = (uint8_t)CONFIG_LCD0_DP_MST_VCP_ID;
#endif

#if defined(CONFIG_LCD1_DP_MST_VCP_ID)
			aucVCP_id[1] = (uint8_t)CONFIG_LCD1_DP_MST_VCP_ID;
#endif

#if defined(CONFIG_LCD2_DP_MST_VCP_ID)
			aucVCP_id[2] = (uint8_t)CONFIG_LCD2_DP_MST_VCP_ID;
#endif

#if defined(CONFIG_LCD3_DP_MST_VCP_ID)
			aucVCP_id[3] = (uint8_t)CONFIG_LCD3_DP_MST_VCP_ID;
#endif

			iRetVal = Dpv14_Tx_API_Set_Video_Code(
								pstDP_V14_Params_t->ucNumOfDPs,
								auiDefaultVideoCode);
			if (iRetVal != 0) {
				/* For coverity */
				goto return_funcs;
			}


			iRetVal = Dpv14_Tx_API_Set_Video_PeriPixelClock(
								pstDP_V14_Params_t->ucNumOfDPs,
								auiPeri_PixelClk);
			if (iRetVal != 0) {
				/* For coverity */
				goto return_funcs;
			}

			iRetVal = Dpv14_Tx_API_Set_Video_ColorSpace(
					(enum DPTX_VIDEO_ENCODING_TYPE)LCD_PIXEL_ENCODING_TYPE);
			if (iRetVal != 0) {
				/* For coverity */
				goto return_funcs;
			}


			iRetVal = Dpv14_Tx_API_Set_VCPID_MST(
						pstDP_V14_Params_t->ucNumOfDPs, aucVCP_id);
			if (iRetVal != 0) {
				/* For coverity */
				goto return_funcs;
			}

			iRetVal = Dpv14_Tx_API_Set_PanelDisplay_Mode(
						(uint8_t)pstDP_V14_Params_t->bDP_PanelMode);
			if (iRetVal != 0) {
				/* For coverity */
				goto return_funcs;
			}

			if (pstDP_V14_Params_t->bDP_PanelMode == (bool)true) {
				/* For coverity */
				ucSink_Supports_SideBand_MSG = 0U;
			} else {
				/* For coverity */
				ucSink_Supports_SideBand_MSG = 1U;
			}

			iRetVal = Dpv14_Tx_API_Set_SideBand_Msg_Supported(
			ucSink_Supports_SideBand_MSG);
			if (iRetVal != 0) {
				/* For coverity */
				goto return_funcs;
			}

			iRetVal = Dpv14_Tx_API_Start();
			if (iRetVal == 0) {
				/* For coverity */
				dpv14_turn_on_display(pstDP_V14_Params_t->ucNumOfDPs);
			} else if (iRetVal == DPTX_API_RETURN_MST_ACT_TIMEOUT) {
				pr_err("[%s:%d]Re-initializing DP Link..\n",
						__func__,
						__LINE__);

				uiTCC805x_Rev = get_chip_rev();
				if (uiTCC805x_Rev == (uint32_t)TCC805X_ES_REVISION) {
					/* For coverity */
					eLinkRate = LINK_RATE_HBR2;
				}

				iRetVal = Dpv14_Tx_API_Stop();
				if (iRetVal != 0) {
					/* For coverity */
					goto return_funcs;
				}

				iRetVal = Dpv14_Tx_API_Deinit();
				if (iRetVal != 0) {
					/* For coverity */
					goto return_funcs;
				}

				Dpv14_Tx_API_Reset_PLL_Blk();

				iRetVal = Dpv14_Tx_API_Init(
							pstDP_V14_Params_t->ucNumOfDPs,
							eLinkRate,
							eLinkLanes);
				if (iRetVal != 0) {
					/* For coverity */
					goto return_funcs;
				}

				iRetVal = Dpv14_Tx_API_Set_Video_Code(
							pstDP_V14_Params_t->ucNumOfDPs,
							auiDefaultVideoCode);
				if (iRetVal != 0) {
					/* For coverity */
					goto return_funcs;
				}

				iRetVal = Dpv14_Tx_API_Set_Video_PeriPixelClock(
							pstDP_V14_Params_t->ucNumOfDPs, auiPeri_PixelClk);
				if (iRetVal != 0) {
				/* For coverity */
				goto return_funcs;
				}

				iRetVal = Dpv14_Tx_API_Set_Video_ColorSpace(
							(enum DPTX_VIDEO_ENCODING_TYPE)LCD_PIXEL_ENCODING_TYPE);
				if (iRetVal != 0) {
					/* For coverity */
					goto return_funcs;
				}

				iRetVal = Dpv14_Tx_API_Set_VCPID_MST(
							pstDP_V14_Params_t->ucNumOfDPs, aucVCP_id);
				if (iRetVal != 0) {
					/* For coverity */
					goto return_funcs;
				}

				iRetVal = Dpv14_Tx_API_Set_PanelDisplay_Mode(
							(uint8_t)pstDP_V14_Params_t->bDP_PanelMode);
				if (iRetVal != 0) {
					/* For coverity */
					goto return_funcs;
				}

				iRetVal = Dpv14_Tx_API_Set_SideBand_Msg_Supported(ucSink_Supports_SideBand_MSG);
				if (iRetVal != 0) {
					/* For coverity */
					goto return_funcs;
				}

				iRetVal = Dpv14_Tx_API_Start();
				if (iRetVal == DPTX_API_RETURN_MST_ACT_TIMEOUT) {
					pr_err("[%s:%d]Err : CCTL.ACT timeout...",
							__func__,
							__LINE__);
					goto return_funcs;
				}

				dpv14_turn_on_display(pstDP_V14_Params_t->ucNumOfDPs);
			} else {
				/* For coverity */
				goto return_funcs;
			}
		}
	} else {
		pstDP_V14_Params_t->bPower_On = (bool)false;

		if (pstDP_V14_Params_t->bDP_PanelMode == (bool)true) {
			/* For coverity */
			tcclcd_gpio_set_value(
					pstDpPanel_Infor_t->dev.power_on,
					0);
		}

		(void)dpv14_display_device_reset(
					pstDpPanel_Infor_t->dev.lcdc_id,
					&pstDpPanel_Infor_t->ovp);

		if ((ucDPIdx + 1U) == pstDP_V14_Params_t->ucNumOfDPs) {
			iRetVal = Dpv14_Tx_API_Deinit();
			if (iRetVal != 0) {
				/* For coverity */
				pr_err("\n[%s:%d]Error from Dpv14_Tx_API_Deinit()\n",
							__func__,
							__LINE__);
			}
		}
	}

return_funcs:
	return iRetVal;
}

static int Dpv14_set_backlight_level(unsigned char Params, int iLevel)
{
	uint8_t ucDPIdx;
	int32_t iRetVal = 0;
	const struct DP_V14_Params *pstDP_V14_Params_t;
	const struct vidinfo *pstDpPanel_Infor_t;

	ucDPIdx = Params;

	if (ucDPIdx >= (uint8_t)PANEL_DP_MAX) {
		pr_err("\n[%s:%d]Error: Invalid DP index %u\n",
					__func__,
					__LINE__,
					ucDPIdx);

		iRetVal = -ENXIO;
		goto return_funcs;
	}

	pstDP_V14_Params_t = &stDP_V14_Params_t[ucDPIdx];
	pstDpPanel_Infor_t = &stDP_V14_Params_t[ucDPIdx].stDpv14_panel_info;

	if (!pstDP_V14_Params_t->bActivated) {
		pr_err("\n[%s:%d]Error: Index %u wasn't initialized\n",
					__func__,
					__LINE__,
					ucDPIdx);

		iRetVal = -ENXIO;
		goto return_funcs;
	}

	pr_notice("\n[%s:%d]DP %u backlight level to %d",
				__func__,
				__LINE__,
				ucDPIdx,
				iLevel);
	pr_notice("\n                            Display Controller %u",
				pstDpPanel_Infor_t->dev.lcdc_id);
	pr_notice("\n                            DEV%u_PATH\n",
				pstDpPanel_Infor_t->dev.lcdc_select);

	if (pstDP_V14_Params_t->bDP_PanelMode) {
		if (iLevel == 0) {
			/* For coverity */
			tcclcd_gpio_set_value(pstDpPanel_Infor_t->dev.bl_on, 0);
		} else {
			/* For coverity */
			tcclcd_gpio_set_value(pstDpPanel_Infor_t->dev.bl_on, 1);
		}
	}

return_funcs:
	return iRetVal;
 }

void Dpv14_get_panel_info(
				unsigned char ucPlatformId,
				struct vidinfo *pstPanel_Info)
{
	const struct DP_V14_Params *pstDP_V14_Params_t;

	if (ucPlatformId >= (u_char)PANEL_DP_MAX) {
		/* For coverity */
		pr_err("\n[%s:%d]Error: Invalid DP index %u\n",
					__func__,
					__LINE__,
					ucPlatformId);
	} else {
		pstDP_V14_Params_t = &stDP_V14_Params_t[ucPlatformId];

		(void)memcpy(pstPanel_Info,
					&pstDP_V14_Params_t->stDpv14_panel_info,
					sizeof(struct vidinfo));
	}
}

void Dpv14_set_panel_info(
				unsigned char ucPlatformId,
				const struct vidinfo *pstPanel_Info)
{
	struct DP_V14_Params *pstDP_V14_Params_t;

	if (ucPlatformId >= (u_char)PANEL_DP_MAX) {
		pr_err("\n[%s:%d]Error: Invalid DP index %u\n",
					__func__,
					__LINE__,
					ucPlatformId);
	} else {
		pstDP_V14_Params_t = &stDP_V14_Params_t[ucPlatformId];

		(void)memcpy(&pstDP_V14_Params_t->stDpv14_panel_info,
					pstPanel_Info,
					sizeof(struct vidinfo));
	}
}
