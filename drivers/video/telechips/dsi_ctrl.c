// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/

#include <config.h>
#include <common.h>
#include <linux/compat.h>

#include <dm/uclass.h>
#include <telechips/dsi_ctrl.h>
#include <mach/clock.h>
#include <linux/delay.h>
#include <i2c.h>
#include <dm.h>

#include "dsih_dphy.h"
#include "dsih_api.h"


#define RPI_TOUCH_LCD_ADDR     0x45 // 8A = 45 << 1
#define MAX96789_SER_ADDR	0x40 // 80 = 40 << 1
#define MAX96878_DES_ADDR	0x6A // HDMI Setting (OK)
#define MAX96878_DES2_ADDR	0x48 // 1st DP Setting (OK)
#define MAX96712_DES_ADDR   0x2A
// #define MAX96878_DES_ADDR	0x4a // 2st DP Setting

#ifdef CONFIG_TCC807X
#define SERDES_I2C_PORT 7
#endif
#ifdef CONFIG_TCC750X
#define SERDES_I2C_PORT 0
#endif

//#define SER_PATTERN 1
//#define SER_DESKEW 1
struct SER_DES_REG{
  unsigned int devaddr;
  unsigned int regaddr;
  uint8_t value;
};

struct SER_DES_REG rpi_touch_lcd[] = {
       {RPI_TOUCH_LCD_ADDR, 0x86, 0xFF},
       {0,0,0}
};

struct SER_DES_REG stserdes_porta[] = {
	/*
	 * DSI Port A - PIPE X - LINK A (GMSL2)
	 */
	// Reset All and release
	{MAX96789_SER_ADDR, 0x10, 0x80}, {MAX96789_SER_ADDR, 0x10, 0x11},
	{0xFF, 0xFFFF, 0xFF}, // delay 50ms

	{MAX96789_SER_ADDR, 0x330, 0x06}, // MIPI Rx PHY - BOTH Port A, B enabled
	{MAX96789_SER_ADDR, 0x331, 0x11}, // Number of Lanes default 2lanes

	// Enable A link -> X transmit X channal
	{MAX96789_SER_ADDR, 0x010, 0x23}, {MAX96789_SER_ADDR, 0x002, 0x53},

	// ADDED 0127 */
	#if 1
	// HSYNC_WIDTH_L / VSYNC_WIDTH_L
	{MAX96789_SER_ADDR, 0x385, 0x08}, {MAX96789_SER_ADDR, 0x386, 0x01},
	{MAX96789_SER_ADDR, 0x387, 0x00}, // HSYNC_WIDTH_H/VSYNC_WIDTH_H
	// VFP_L / VBP_H
	{MAX96789_SER_ADDR, 0x3A5, 0x0A}, {MAX96789_SER_ADDR, 0x3A7, 0x00},
	{MAX96789_SER_ADDR, 0x3A6, 0xA0}, // VFP_H/VBP_L
	// VRES_L / VRES_H
	{MAX96789_SER_ADDR, 0x3A8, 0xD0}, {MAX96789_SER_ADDR, 0x3A9, 0x02},
	// HFP_L / HBP_H
	{MAX96789_SER_ADDR, 0x3AA, 0x1C}, {MAX96789_SER_ADDR, 0x3AC, 0x01},
	{MAX96789_SER_ADDR, 0x3AB, 0xC0}, // HFP_H/HBP_L
	// HRES_L / HRES_H
	{MAX96789_SER_ADDR, 0x3AD, 0x80}, {MAX96789_SER_ADDR, 0x3AE, 0x07},
	{MAX96789_SER_ADDR, 0x3A4, 0xC1}, // FIFO/DESKEW_EN
	#endif
	{MAX96878_DES_ADDR, 0x0005, 0xB0}, // GMSL2 mode w/ sink mode
	{MAX96878_DES_ADDR, 0x01CE, 0x4E}, // DES oLDI setting		/* GPIO / I2C Setting */
	{MAX96789_SER_ADDR, 0x001, 0x8}, // I2C pass-through

#ifdef CONFIG_TCC750X
	{MAX96789_SER_ADDR, 0x380, 0x0D}, // Pol
#endif
#ifdef CONFIG_TCC807X
	{MAX96789_SER_ADDR, 0x380, 0x3D}, // Pol
	{MAX96789_SER_ADDR, 0x390, 0x3D}, // Pol
#endif

	// MFP2 (GPIO02) - MFP18 (GPIO18) // LCD_ON
	{MAX96789_SER_ADDR, 0x2C4, 0x83}, // GPIO2 Ser setting
	{MAX96789_SER_ADDR, 0x2C5, 0xAA}, // SER GPIO_TX_ID  : 0x2 (to GPIO18)
	{MAX96878_DES_ADDR, 0x236, 0x84}, // GPIO18 Des setting
	{MAX96878_DES_ADDR, 0x238, 0x6A}, // DES GPIO_RX_ID	: 0x1 (from GPIO2)

	// MFP3 (GPIO03) - MFP17 (GPIO17) // RESET
	{MAX96789_SER_ADDR, 0x2C7, 0x83}, // GPIO3 Ser setting
	{MAX96789_SER_ADDR, 0x2C8, 0xAB}, // SER GPIO_TX_ID  : 0x2 (to GPIO17)
	{MAX96878_DES_ADDR, 0x233, 0x84}, // GPIO18 Des setting
	{MAX96878_DES_ADDR, 0x235, 0x6B}, // DES GPIO_RX_ID	: 0x1 (from GPIO2)

	// MFP7 (GPIO07) - MFP2 (GPIO2) // BL_EN
	{MAX96789_SER_ADDR, 0x2D3, 0x83}, // GPIO3 Ser setting
	{MAX96789_SER_ADDR, 0x2D4, 0xAC}, // SER GPIO_TX_ID  : 0x2 (to GPIO17)
	{MAX96878_DES_ADDR, 0x206, 0x84}, // GPIO18 Des setting
	{MAX96878_DES_ADDR, 0x208, 0x6C}, // DES GPIO_RX_ID	: 0x1 (from GPIO2)

#ifdef SER_PATTERN
	{MAX96789_SER_ADDR, 0x1C8, 0x04},{MAX96789_SER_ADDR, 0x1CA, 0x0},
	{MAX96789_SER_ADDR, 0x1CB, 0x0},{MAX96789_SER_ADDR, 0x1CC, 0x0},
	{MAX96789_SER_ADDR, 0x1CD, 0x0},{MAX96789_SER_ADDR, 0x1CE, 0xF},
	{MAX96789_SER_ADDR, 0x1CF, 0x80},{MAX96789_SER_ADDR, 0x1D0, 0x16},
	{MAX96789_SER_ADDR, 0x1D1, 0x67},{MAX96789_SER_ADDR, 0x1D2, 0x00},
	{MAX96789_SER_ADDR, 0x1D3, 0x00},{MAX96789_SER_ADDR, 0x1D4, 0x00},
	{MAX96789_SER_ADDR, 0x1D5, 0x00},{MAX96789_SER_ADDR, 0x1D6, 0x00},
	{MAX96789_SER_ADDR, 0x1D7, 0x08},{MAX96789_SER_ADDR, 0x1D8, 0x07},
	{MAX96789_SER_ADDR, 0x1D9, 0xB8},{MAX96789_SER_ADDR, 0x1DA, 0x02},
	{MAX96789_SER_ADDR, 0x1DB, 0xE6},{MAX96789_SER_ADDR, 0x1DC, 0x0},
	{MAX96789_SER_ADDR, 0x1DD, 0x5D},{MAX96789_SER_ADDR, 0x1DE, 0x24},
	{MAX96789_SER_ADDR, 0x1DF, 0x07},{MAX96789_SER_ADDR, 0x1E0, 0x80},
	{MAX96789_SER_ADDR, 0x1E1, 0x0},{MAX96789_SER_ADDR, 0x1E2, 0x40},
	{MAX96789_SER_ADDR, 0x1E3, 0x02},{MAX96789_SER_ADDR, 0x1E4, 0xD0},
	{MAX96789_SER_ADDR, 0x1E5, 0x01},{MAX96789_SER_ADDR, 0x1E7, 0xFF},
	{MAX96789_SER_ADDR, 0x1E8, 0xFF},{MAX96789_SER_ADDR, 0x1E9, 0xFF},
	{MAX96789_SER_ADDR, 0x1EA, 0x0},{MAX96789_SER_ADDR, 0x1EB, 0x0},
	{MAX96789_SER_ADDR, 0x1EC, 0x0},{MAX96789_SER_ADDR, 0x1ED, 0xF0},
	{MAX96789_SER_ADDR, 0x1EE, 0xF0},{MAX96789_SER_ADDR, 0x1EF, 0x78},
	{MAX96789_SER_ADDR, 0x1C8, 0xE3}, // 0x01
#endif
	{0,0,0}
};
struct SER_DES_REG stserdes_porta_dsi_to_csi[] = {
	//D231006
	//Author : Sang Lim
	//SER:{MAX96789 Port A 2 lanes-PIPE X , DES : {MAX96712, LINK A 4 lanes
	//DT : RGB888
	//I2C Master : Connected to {MAX96789

	//----{MAX96789----//
	{MAX96789_SER_ADDR, 0x0330,0x06},   //MIPI 2X4 mode. Enable AnB
	{MAX96789_SER_ADDR, 0x0331,0x11},   //Port AnB 2 lanes
	{MAX96789_SER_ADDR,0x0010,0x31},   //One Shot Reset

	{0xFF, 0xFFFF, 0xFF},
	{0xFF, 0xFFFF, 0xFF},
	{0xFF, 0xFFFF, 0xFF},                //Delay    150ms delay
	//-END {MAX96789-//

	//{MAX96712_DES_ADDR,0x00,0x03,0xFF,  // DIS_REM_CC=1 for all links
	{MAX96712_DES_ADDR, 0x040B,0xC0},  // Disable MIPI BPP for PIPE0=24
	{MAX96712_DES_ADDR, 0x0006,0xF1},  // Enable LINK A only
	{MAX96712_DES_ADDR, 0x00F0,0x60},  // PIPE0 - LINK A, STR_ID=00
	{MAX96712_DES_ADDR, 0x0018,0x0F},  // One shot reset

	{0xFF, 0xFFFF, 0xFF},
	{0xFF, 0xFFFF, 0xFF},
	{0xFF, 0xFFFF, 0xFF},                  //Delay 150ms

	{MAX96712_DES_ADDR, 0x01D9,0x59},  // VS inversion

	// Initial VC
	{MAX96712_DES_ADDR,0x040C,0x00},  // VC=00 for pipe0 & pipe1
	{MAX96712_DES_ADDR,0x040D,0x00},  // VC=00 for pipe2 & pipe3

	//RGB888
	{MAX96712_DES_ADDR,0x040E,0xA4},  // RGB888 -- 0x24  10 10 0100
	{MAX96712_DES_ADDR,0x040F,0x7E},  //
	{MAX96712_DES_ADDR,0x0410,0x5A},  //
	{MAX96712_DES_ADDR,0x0411,0x90},  //
	{MAX96712_DES_ADDR,0x0412,0x40},  //

	//MIPI PHY setting
	//2x4 mode setting
	{MAX96712_DES_ADDR,0x08A0,0x04},  //MIPI 2X4 mode

	//ane ping for 4-lane port A and B
	{MAX96712_DES_ADDR,0x08A3,0xE4},  //No MIPI lanes swapping
	{MAX96712_DES_ADDR,0x08A4,0xE4},  //No MIPI lanes swapping

	//MIPI PHY enabled
	{MAX96712_DES_ADDR,0x08A2,0xF4},  //Enable All MIPI PHY

	//software override & MIPI data rate : 800Mbps
	{MAX96712_DES_ADDR,0x0415,0xEA},  //VC/DT override, 1000Mbps
	{MAX96712_DES_ADDR,0x0418,0xEA},  //VC/DT override, 1000Mbps

	//Pipo MIPI Controller Mapping
	// vi pipe 0  map FS/FE
	{MAX96712_DES_ADDR,0x090B,0x07},
	{MAX96712_DES_ADDR,0x092D,0x15},  // map to MIPI Controller 1  00 01 01 01
	{MAX96712_DES_ADDR,0x090D,0x24},  // SRC Long packet. 00 10 1100
	{MAX96712_DES_ADDR,0x090E,0x24},  // DST Long packet. Map to VC0. 00 10 1100
	{MAX96712_DES_ADDR,0x090F,0x00},  // SRC short packet.
	{MAX96712_DES_ADDR,0x0910,0x00},  // DST short packet. 00 00 0000
	{MAX96712_DES_ADDR,0x0911,0x01},  // SRC short packet.
	{MAX96712_DES_ADDR,0x0912,0x01},  // DST short packet.

	//{MAX96712_DES_ADDR,0x00,0x03,0xAA,  // DIS_REM_CC=0. Enable REM_CC for all links
	//0x00,0x0A,                 //Delay 10ms
	// {MAX96712_DES_ADDR,0x040B,0xC2}, // MIPI enable
	{0,0,0}
};

struct SER_DES_REG stserdes_port_both[] = {
	/*
	 * DSI Port A - PIPE X - LINK A (GMSL2)
	 * DSI Port B - PIPE Z - LINK B (GMSL2)
	 */
	{MAX96789_SER_ADDR, 0x10, 0x80}, // Reset All
	{MAX96789_SER_ADDR, 0x10, 0x11}, // Reset All release
	{0xFF, 0xFFFF, 0xFF}, // delay 50ms

	{MAX96789_SER_ADDR, 0x330, 0x06}, // MIPI Rx PHY - BOTH Port A, B enabled
	{MAX96789_SER_ADDR, 0x331, 0x11}, // Number of Lanes default 2lanes

	{MAX96789_SER_ADDR, 0x005B, 0x22}, // DES video stream select = 0b10

	{MAX96789_SER_ADDR, 0x010, 0x23}, // Enable Both Link
	{MAX96789_SER_ADDR, 0x002, 0x53}, // Only Video transmit Channal X, Z
	{0xFF, 0xFFFF, 0xFF}, // delay 50ms
	{0xFF, 0xFFFF, 0xFF}, // delay 50ms

	// DSI0 */
	{MAX96789_SER_ADDR, 0x385, 0x08}, // HSYNC_WIDTH_L
	{MAX96789_SER_ADDR, 0x386, 0x01}, // VSYNC_WIDTH_L
	{MAX96789_SER_ADDR, 0x387, 0x00}, // HSYNC_WIDTH_H/VSYNC_WIDTH_H
	{MAX96789_SER_ADDR, 0x3A5, 0x0A}, // VFP_L
	{MAX96789_SER_ADDR, 0x3A7, 0x00}, // VBP_H
	{MAX96789_SER_ADDR, 0x3A6, 0xA0}, // VFP_H/VBP_L
	{MAX96789_SER_ADDR, 0x3A8, 0xD0}, // VRES_L
	{MAX96789_SER_ADDR, 0x3A9, 0x02}, // VRES_H
	{MAX96789_SER_ADDR, 0x3AA, 0x1C}, // HFP_L
	{MAX96789_SER_ADDR, 0x3AC, 0x01}, // HBP_H
	{MAX96789_SER_ADDR, 0x3AB, 0xC0}, // HFP_H/HBP_L
	{MAX96789_SER_ADDR, 0x3AD, 0x80}, // HRES_L
	{MAX96789_SER_ADDR, 0x3AE, 0x07}, // HRES_H
	{MAX96789_SER_ADDR, 0x3A4, 0xC1}, // FIFO/DESKEW_EN

	{MAX96878_DES_ADDR, 0x0005, 0xB0}, // GMSL2 mode w/ sink mode
	{MAX96878_DES_ADDR, 0x01CE, 0x4E}, // DES oLDI setting		/* GPIO / I2C Setting */
	
	/* DSI1 */
	{MAX96789_SER_ADDR, 0x395, 0x08}, // HSYNC_WIDTH_L
	{MAX96789_SER_ADDR, 0x396, 0x01}, // VSYNC_WIDTH_L
	{MAX96789_SER_ADDR, 0x397, 0x00}, // HSYNC_WIDTH_H/VSYNC_WIDTH_H
	{MAX96789_SER_ADDR, 0x3B1, 0x0A}, // VFP_L
	{MAX96789_SER_ADDR, 0x3B3, 0x00}, // VBP_H
	{MAX96789_SER_ADDR, 0x3B2, 0xA0}, // VFP_H/VBP_L
	{MAX96789_SER_ADDR, 0x3B4, 0xD0}, // VRES_L
	{MAX96789_SER_ADDR, 0x3B5, 0x02}, // VRES_H
	{MAX96789_SER_ADDR, 0x3B6, 0x1C}, // HFP_L
	{MAX96789_SER_ADDR, 0x3B8, 0x01}, // HBP_H
	{MAX96789_SER_ADDR, 0x3B7, 0xC0}, // HFP_H/HBP_L
	{MAX96789_SER_ADDR, 0x3B9, 0x80}, // HRES_L
	{MAX96789_SER_ADDR, 0x3BA, 0x07}, // HRES_H
	{MAX96789_SER_ADDR, 0x3B0, 0xC1}, // FIFO/DESKEW_EN

	{MAX96878_DES2_ADDR, 0x0005, 0xB0}, // GMSL2 mode w/ sink mode

	{MAX96878_DES2_ADDR, 0x00A0, 0x02}, // DES video stream select = 0b10
	{MAX96878_DES2_ADDR, 0x01CE, 0x4E}, // DES oLDI setting		/* GPIO / I2C Setting */

	{MAX96789_SER_ADDR, 0x001, 0x8}, // I2C pass-through
	{MAX96878_DES2_ADDR, 0x0010, 0x31}, // One shot reset

#ifdef CONFIG_TCC705X
	{MAX96789_SER_ADDR, 0x380, 0x0D}, // Pol
#endif
#ifdef CONFIG_TCC807X
	{MAX96789_SER_ADDR, 0x380, 0x3D}, // Pol
	{MAX96789_SER_ADDR, 0x390, 0x3D}, // Pol
#endif
	// MFP2 (GPIO02) - MFP18 (GPIO18) // LCD_ON
	{MAX96789_SER_ADDR, 0x2C4, 0x83}, // GPIO2 Ser setting
	{MAX96789_SER_ADDR, 0x2C5, 0xAA}, // SER GPIO_TX_ID  : 0x2 (to GPIO18)
	{MAX96878_DES_ADDR, 0x236, 0x84}, // GPIO18 Des setting
	{MAX96878_DES_ADDR, 0x238, 0x6A}, // DES GPIO_RX_ID	: 0x1 (from GPIO2)

	// MFP3 (GPIO03) - MFP17 (GPIO17) // RESET
	{MAX96789_SER_ADDR, 0x2C7, 0x83}, // GPIO3 Ser setting
	{MAX96789_SER_ADDR, 0x2C8, 0xAB}, // SER GPIO_TX_ID  : 0x2 (to GPIO17)
	{MAX96878_DES_ADDR, 0x233, 0x84}, // GPIO18 Des setting
	{MAX96878_DES_ADDR, 0x235, 0x6B}, // DES GPIO_RX_ID	: 0x1 (from GPIO2)

	// MFP7 (GPIO07) - MFP2 (GPIO2) // BL_EN
	{MAX96789_SER_ADDR, 0x2D3, 0x83}, // GPIO3 Ser setting
	{MAX96789_SER_ADDR, 0x2D4, 0xAC}, // SER GPIO_TX_ID  : 0x2 (to GPIO17)
	{MAX96878_DES_ADDR, 0x206, 0x84}, // GPIO18 Des setting
	{MAX96878_DES_ADDR, 0x208, 0x6C}, // DES GPIO_RX_ID	: 0x1 (from GPIO2)

	/* DSI1 */
	// MFP2 (GPIO02) - MFP18 (GPIO18) // LCD_ON
	{MAX96789_SER_ADDR, 0x2E2, 0x83}, // GPIO2 Ser setting
	{MAX96789_SER_ADDR, 0x2E3, 0xAD}, // SER GPIO_TX_ID  : 0x2 (to GPIO18)
	{MAX96878_DES2_ADDR, 0x236, 0x84}, // GPIO18 Des setting
	{MAX96878_DES2_ADDR, 0x238, 0x6D}, // DES GPIO_RX_ID	: 0x1 (from GPIO2)

	// MFP3 (GPIO03) - MFP17 (GPIO17) // RESET
	{MAX96789_SER_ADDR, 0x2E5, 0x83}, // GPIO3 Ser setting
	{MAX96789_SER_ADDR, 0x2E6, 0xAE}, // SER GPIO_TX_ID  : 0x2 (to GPIO17)
	{MAX96878_DES2_ADDR, 0x233, 0x84}, // GPIO18 Des setting
	{MAX96878_DES2_ADDR, 0x235, 0x6E}, // DES GPIO_RX_ID	: 0x1 (from GPIO2)

	// MFP7 (GPIO07) - MFP2 (GPIO2) // BL_EN
	{MAX96789_SER_ADDR, 0x2E8, 0x83}, // GPIO3 Ser setting
	{MAX96789_SER_ADDR, 0x2E9, 0xAF}, // SER GPIO_TX_ID  : 0x2 (to GPIO17)
	{MAX96878_DES2_ADDR, 0x206, 0x84}, // GPIO18 Des setting
	{MAX96878_DES2_ADDR, 0x208, 0x6F}, // DES GPIO_RX_ID	: 0x1 (from GPIO2)

	{0,0,0}
};

struct SER_DES_REG stserdes_portb[] = {
	/*
	 * DSI Port B - PIPE Z - LINK B (GMSL2)
	 */
	{MAX96789_SER_ADDR, 0x330, 0x06}, // MIPI Rx PHY - BOTH Port A, B enabled
	{MAX96789_SER_ADDR, 0x331, 0x11}, // Number of Lanes default 2lanes

	{MAX96789_SER_ADDR, 0x010, 0x22}, // reset oneshot / LINK_CFG  Link B enable
	{MAX96789_SER_ADDR, 0x002, 0x43},

	{MAX96789_SER_ADDR, 0x395, 0x08}, // HSYNC_WIDTH_L
	{MAX96789_SER_ADDR, 0x396, 0x01}, // VSYNC_WIDTH_L
	{MAX96789_SER_ADDR, 0x397, 0x00}, // HSYNC_WIDTH_H/VSYNC_WIDTH_H
	{MAX96789_SER_ADDR, 0x3B1, 0x0A}, // VFP_L
	{MAX96789_SER_ADDR, 0x3B3, 0x00}, // VBP_H
	{MAX96789_SER_ADDR, 0x3B2, 0xA0}, // VFP_H/VBP_L
	{MAX96789_SER_ADDR, 0x3B4, 0xD0}, // VRES_L
	{MAX96789_SER_ADDR, 0x3B5, 0x02}, // VRES_H
	{MAX96789_SER_ADDR, 0x3B6, 0x1C}, // HFP_L
	{MAX96789_SER_ADDR, 0x3B8, 0x01}, // HBP_H
	{MAX96789_SER_ADDR, 0x3B7, 0xC0}, // HFP_H/HBP_L
	{MAX96789_SER_ADDR, 0x3B9, 0x80}, // HRES_L
	{MAX96789_SER_ADDR, 0x3BA, 0x07}, // HRES_H
	{MAX96789_SER_ADDR, 0x3B0, 0xC1}, // FIFO/DESKEW_EN

	{MAX96878_DES2_ADDR, 0x0005, 0xB0}, // GMSL2 mode w/ sink mode

	{MAX96789_SER_ADDR, 0x005B, 0x22}, // DES video stream select = 0b10
	{MAX96878_DES2_ADDR, 0x00A0, 0x02}, // DES video stream select = 0b10

	{MAX96878_DES2_ADDR, 0x01CE, 0x4E}, // DES oLDI setting		/* GPIO / I2C Setting */
	{MAX96789_SER_ADDR, 0x001, 0x8}, // I2C pass-through
	{MAX96878_DES2_ADDR, 0x0010, 0x31}, // One shot reset

#ifdef CONFIG_TCC705X
	{MAX96789_SER_ADDR, 0x380, 0x0D}, // Pol
#endif
#ifdef CONFIG_TCC807X
	{MAX96789_SER_ADDR, 0x380, 0x3D}, // Pol
	{MAX96789_SER_ADDR, 0x390, 0x3D}, // Pol
#endif

	/* DSI1 */
	// MFP2 (GPIO02) - MFP18 (GPIO18) // LCD_ON
	{MAX96789_SER_ADDR, 0x2E2, 0x83}, // GPIO2 Ser setting
	{MAX96789_SER_ADDR, 0x2E3, 0xAD}, // SER GPIO_TX_ID  : 0x2 (to GPIO18)
	{MAX96878_DES2_ADDR, 0x236, 0x84}, // GPIO18 Des setting
	{MAX96878_DES2_ADDR, 0x238, 0x6D}, // DES GPIO_RX_ID	: 0x1 (from GPIO2)

	// MFP3 (GPIO03) - MFP17 (GPIO17) // RESET
	{MAX96789_SER_ADDR, 0x2E5, 0x83}, // GPIO3 Ser setting
	{MAX96789_SER_ADDR, 0x2E6, 0xAE}, // SER GPIO_TX_ID  : 0x2 (to GPIO17)
	{MAX96878_DES2_ADDR, 0x233, 0x84}, // GPIO18 Des setting
	{MAX96878_DES2_ADDR, 0x235, 0x6E}, // DES GPIO_RX_ID	: 0x1 (from GPIO2)

	// MFP7 (GPIO07) - MFP2 (GPIO2) // BL_EN
	{MAX96789_SER_ADDR, 0x2E8, 0x83}, // GPIO3 Ser setting
	{MAX96789_SER_ADDR, 0x2E9, 0xAF}, // SER GPIO_TX_ID  : 0x2 (to GPIO17)
	{MAX96878_DES2_ADDR, 0x206, 0x84}, // GPIO18 Des setting
	{MAX96878_DES2_ADDR, 0x208, 0x6F}, // DES GPIO_RX_ID	: 0x1 (from GPIO2)
	{0,0,0}
};

void tcc_dsi_set_serdes(unsigned int lane0, unsigned int lane1, unsigned int ser_des_mode)
{
	int i2c_port, loop, retry, ret;

	struct udevice *i2c_dev;
	struct udevice *bus_dev = NULL;
	struct SER_DES_REG *pstserdes;

	i2c_port = SERDES_I2C_PORT;

	if (i2c_port < 0) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[ERR][I2C] not valid Port\n");
		return;
	}
	ret = uclass_get_device_by_seq(UCLASS_I2C, i2c_port, &bus_dev);
	if (!ret) {
		ret = dm_i2c_set_bus_speed(bus_dev, 100000);
		if (ret) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err(
				"\n[%s:%d]Error: from dm_i2c_set_bus_speed().. Ret(0x%x)\n",
				__func__, __LINE__, ret);
		}
	} else {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err(
			"\n[%s:%d]Error: can't get i2c bus %d\n",
			__func__, __LINE__, i2c_port);
		return;
	}

	if( ser_des_mode != 0U ) {
		pstserdes = stserdes_porta_dsi_to_csi;
	}
	else {
		if(lane1 == 5) {  // zminc : Temp code for RPi LCD
			pstserdes = rpi_touch_lcd;
		}
		else {
			if(lane1 == 0) { // Only A port used
				pstserdes = stserdes_porta;
			} else { // Both A,B used
				pstserdes = stserdes_port_both;
			}
		}
	}

	for (loop = 0; !(pstserdes[loop].devaddr == 0 &&
			pstserdes[loop].regaddr == 0 &&
			pstserdes[loop].value == 0); loop++) {
		if (pstserdes[loop].devaddr == 0xFF &&
			pstserdes[loop].regaddr == 0xFFFF &&
			pstserdes[loop].value == 0xFF){
			mdelay(100);
			continue;
		}
		// Trying get I2C dev
		for (retry = 100; retry >= 0 ; retry--) {
			ret = i2c_get_chip_for_busnum(i2c_port,
						      pstserdes[loop].devaddr,
						      2,
						      &i2c_dev);
			if (ret) {
				if (retry == 0) {
					/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
					/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
					/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
					pr_err("[%d] Failed to get i2c handle for addr 0x%x - 0x%x - 0x%x\n",
						__LINE__,
						pstserdes[loop].devaddr, pstserdes[loop].regaddr, pstserdes[loop].value);
					return;
				}
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				//pr_err("[%d] Getting i2c handle retry left %d\n",
				//       __LINE__, retry);
				mdelay(10);
			} else {
				if(retry != 100)
					pr_info("retry %d - for addr 0x%x - 0x%x - 0x%x\n", retry,
						pstserdes[loop].devaddr, pstserdes[loop].regaddr, pstserdes[loop].value);
				break;
			}
		}
		// Getting I2C dev Done
		i2c_set_chip_offset_len(i2c_dev, 2);

		if(pstserdes[loop].regaddr == 0x0331) {
			pstserdes[loop].value = ((lane1 -1) << 4);
			pstserdes[loop].value |= (lane0 -1);
		}
		dm_i2c_write(i2c_dev, pstserdes[loop].regaddr,
			&pstserdes[loop].value, 1);
		mdelay(10);
	}

	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("[%d][%s] Done\n", __LINE__, __func__);
}


#ifdef CONFIG_TCC807X
int32_t dsi_enable_v2(struct mipi_dsi_dev *dev, const struct display_timing *timings)
{

	int32_t ret = 0;
	struct dsih_core_ipi_t *ipi_video = &dev->ipi_cfg;

	//dev->pclk =  (tcc_get_peri(PERI_LCD2) / 1000);
	dev->data_rate = (dev->pclk) * 24 / dev->phy_cfg.phy_lanes;
	dev->data_rate /= 1000;
	pr_err("PHY data_rate : %dmbps\n", dev->data_rate);
	if(dev->data_rate % 10) {
		pr_err("[DSI]Request D-PHY Data rate not supported!\n Please set as multiple of 10mbps\n");
		dev->data_rate /= 10;
		dev->data_rate += 1;
		dev->data_rate *= 10;
	}
	if(dev->data_rate > 2500 || dev->data_rate < 100) {
		pr_err("D-PHY Bandwidth out of range\n");
		return 0;
	}

	ipi_video->vid_hsa_time = timings->hsync_len.typ;
	ipi_video->vid_hbp_time = timings->hback_porch.typ;
	ipi_video->vid_hfp_time = timings->hfront_porch.typ;
	ipi_video->vid_hact_time = timings->hactive.typ;
	ipi_video->vid_hline_time = ipi_video->vid_hsa_time + ipi_video->vid_hbp_time + ipi_video->vid_hact_time + timings->hback_porch.typ;
	ipi_video->vid_vsa_lines = timings->vsync_len.typ;
	ipi_video->vid_vbp_lines = timings->vback_porch.typ;
	ipi_video->vid_vact_lines = timings->vactive.typ;
	ipi_video->vid_vfp_lines = timings->vfront_porch.typ;

	pr_info("ipi_video->vid_hsa_time  :  %d\n", ipi_video->vid_hsa_time );
	pr_info("ipi_video->vid_hbp_time  :  %d\n", ipi_video->vid_hbp_time );
	pr_info("ipi_video->vid_hact_time  : %d\n", ipi_video->vid_hact_time );
	pr_info("ipi_video->vid_hline_time : %d\n", ipi_video->vid_hline_time);
	pr_info("ipi_video->vid_vsa_lines  : %d\n", ipi_video->vid_vsa_lines );
	pr_info("ipi_video->vid_vbp_lines  : %d\n", ipi_video->vid_vbp_lines );
	pr_info("ipi_video->vid_vact_lines : %d\n", ipi_video->vid_vact_lines);
	pr_info("ipi_video->vid_vfp_lines  : %d\n", ipi_video->vid_vfp_lines );

	if(dev->port == 0) {
		tcc_dsi_set_serdes(dev->lane0, dev->lane1, 0);
	}
	tcc_dsi_phy_init(dev);
	mdelay(200);
	mipi_dsih_ipi_video(dev);
	return ret;
}
#endif

#ifdef CONFIG_TCC750X
int32_t dsi_enable_v1(struct mipi_dsi_dev *dev, const struct display_timing *timings)
{

	int32_t ret = 0;

	dsih_dpi_video_t *video = &dev->dpi_video;
	dev->core_addr = (void __iomem *)DSI_CORE_ADDR;
	dev->cfg_addr = (void __iomem *)CAM_CFG_ADDR;

	video->h_active_pixels = timings->hactive.typ;
	video->h_sync_pixels = timings->hsync_len.typ;
	video->h_back_porch_pixels = timings->hback_porch.typ;
	video->h_total_pixels = timings->hactive.typ + 
				timings->hback_porch.typ + timings->hfront_porch.typ + timings->hsync_len.typ;

	video->v_active_lines = timings->vactive.typ; /* vadr */
	video->v_sync_lines = timings->vsync_len.typ;
	video->v_back_porch_lines = timings->vback_porch.typ; /* vbp */
	video->v_total_lines = timings->vactive.typ + timings->vsync_len.typ
						+ timings->vback_porch.typ + timings->vfront_porch.typ;	  /* vfp */
	video->h_polarity = ((timings->flags & DISPLAY_FLAGS_HSYNC_LOW) != 0) ? 1 : 0;
	video->v_polarity = ((timings->flags & DISPLAY_FLAGS_VSYNC_LOW) != 0) ? 1 : 0;
	video->data_en_polarity = ((timings->flags & DISPLAY_FLAGS_DE_LOW) != 0) ? 1 : 0;

	video->pixel_clock = (tcc_get_peri(PERI_DISP) / 1000);
	pr_err("pixel_clock : %lu khs\n", video->pixel_clock);
		#if 1
	// TODO : Data rate cal
	dev->data_rate = video->pixel_clock * 24 / video->no_of_lanes;
	dev->data_rate /= 1000;
	pr_err("pixel_clock : %lu khs\n", video->pixel_clock);
	pr_err("PHY data_rate : %dmbps\n", dev->data_rate);
	if(dev->data_rate % 10) {
		// need round up
		pr_err("[DSI]Request D-PHY Data rate not supported!\n Please set as multiple of 10mbps");
		return 0;
	}
	if(dev->data_rate > 2500 || dev->data_rate < 100) {
		pr_err("D-PHY Bandwidth out of range\n");
		return 0;
	}
		#else
			dev->data_rate = 1050;
		#endif
	video->byte_clock = (dev->data_rate * 1000) / 8;
	pr_info("PHY byte clock : %ld kbps\n", video->byte_clock);

	video->video_mode = VIDEO_NON_BURST_WITH_SYNC_PULSES;  /* Only Video value */
	if(video->color_coding == COLOR_CODE_18BIT_CONFIG1 
			|| video->color_coding == COLOR_CODE_18BIT_CONFIG2) { // 18BPP
		video->is_18_loosely = 1;
	} else {
		video->is_18_loosely = 0;
	}
	//BTA disable
	video->receive_ack_packets = 0;
	video->max_hs_to_lp_cycles = 0x40;	  /* value for max freq */
	video->max_lp_to_hs_cycles = 0x40;	 /* value for max freq */
	video->max_clk_hs_to_lp_cycles = 0x40;  /* value for max freq */
	video->max_clk_lp_to_hs_cycles = 0x40; /* value for max freq */
	video->dpi_lp_cmd_en =0;

	if( video->ser_des_mode != 0U ) {
		if( video->ser_des_mode == 1U ) {		// SLT
			__raw_writel(0x4a, 0x1e0201e0);
			__raw_writel(0x4a, 0x1e0201e4);
		}
		else {									// LoopBack(ND-D5)
			__raw_writel(0x42, 0x1e0201e0);
			__raw_writel(0x42, 0x1e0201e4);
		}
	}

	// zminc : 5 means enabling RPi lcd backlight, default: 0
	tcc_dsi_set_serdes(video->no_of_lanes, 5, video->ser_des_mode);
	mipi_dsih_dpi_video(dev);
	mdelay(100);
	tcc_dsi_phy_init(dev);
	__raw_writel(0x10002, 0x16004050);

	return ret;
}
#endif
