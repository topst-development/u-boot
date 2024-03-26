
// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
 /*
  * Copyright (C) 2020 Telechips Inc.
   */
#ifndef DT_BINDING_TELECHIPS_FB_DM_DISPLAY_H
#define DT_BINDING_TELECHIPS_FB_DM_DISPLAY_H

/* define for LVDS -----------------------------------------------*/
#define PHY_PORT_S0 0
#define PHY_PORT_D0 2
#define PHY_PORT_D1 3

#define CLK_LANE 0
#define DATA0_LANE 1
#define DATA1_LANE 2
#define DATA2_LANE 3
#define DATA3_LANE 4

#define LVDS_DUAL 0
#define LVDS_SINGLE 1
/* define for LVDS */

/* define for DP V1.4 --------------------------------------------*/
#define CUSTOM_VIC_1027 	1027
#define CUSTOM_CEA861_720P 	4
#define CUSTOM_CEA861_1080P 	16
#define CUSTOM_CEA861_4K 	97

#define PHY_LANE_SWAP_OFF 	0
#define PHY_LANE_SWAP_ON 	1

#define MAX968XX_LANE_SWAP_OFF 	0
#define MAX968XX_LANE_SWAP_ON 	1

#define SDM_BYPASS_DISABLE 	0
#define SDM_BYPASS_ENABLE 	1

#define TRVC_BYPASS_DISABLE 	0
#define TRVC_BYPASS_ENABLE 	1

#define PANEL_MODE_OFF 		0
#define PANEL_MODE_ON 		1

#define VCP_ID_1 		1
#define VCP_ID_2 		2
#define VCP_ID_3 		3
#define VCP_ID_4 		4

#define LCD_MUX0 		0
#define LCD_MUX1 		1
#define LCD_MUX2 		2
#define LCD_MUX3 		3
#define LCD_MUX4 		4
//#define LCD_MUX_MAX LCD_MUX4

#define PHY_LINK_LANE_1 	1
#define PHY_LINK_LANE_2 	2
#define PHY_LINK_LANE_4 	4

#define PHY_LINK_RATE_RBR 	0
#define PHY_LINK_RATE_HBR 	1
#define PHY_LINK_RATE_HBR2 	2
#define PHY_LINK_RATE_HBR3 	3

#define PIXEL_ENCODING_RGB 	0
#define PIXEL_ENCODING_YCBCR422 1
#define PIXEL_ENCODING_YCBCR444 2
/* define VCP_ID_1 DP V1.4 */

/* define for DSI ------------------------------------------------*/
#define DSI_AUTO		0
#define DSI_MANUAL		1

#define DSI_PORT0		0
#define DSI_PORT1		1

#define DSI_PHY_LANE1		1
#define DSI_PHY_LANE2		2
#define DSI_PHY_LANE3		3
#define DSI_PHY_LANE4		4

#define NO_OF_LANE_ND   	1
#define NO_OF_MAX_LANE  	4
#define DSI_COLOR_CODE_24BIT	5

#define SER_DES_MODE_NORMAL			 0		// DSI - SERDES - LVDS
#define SER_DES_MODE_SLT			1		// DSI - SERDES - CSI(750X)
#define SER_DES_MODE_LOOPBACK		2		// DSI - SERDES - CSI(807X)

/* define for FB -------------------------------------------------*/
#define FB_DRAW_TYPE_BITMAP_HEADER 0
#define FB_DRAW_TYPE_SPLASH 1

#define TCC_VIDEO_BPP16 4
#define TCC_VIDEO_BPP32 5

#define LCD_MUX_BYPASS_DISABLE 0
#define LCD_MUX_BYPASS_ENABLE 1


#endif

