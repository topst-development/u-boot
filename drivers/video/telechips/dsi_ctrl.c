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

#define MAX96789_SER_ADDR	0x40 // 80 = 40 << 1
#define MAX96878_DES_ADDR	0x6A // HDMI Setting (OK)
#define MAX96878_DES2_ADDR	0x48 // 1st DP Setting (OK)
// #define MAX96878_DES_ADDR	0x4a // 2st DP Setting

#ifdef CONFIG_TCC807X
#define SERDES_I2C_PORT 7
#endif
#ifdef CONFIG_TCC750X
#define SERDES_I2C_PORT 0
#endif

//#define SER_PATTERN 1
//#define SER_DESKEW 1

struct SER_DES_BUS_DEV{
	struct udevice *i2c_ser_dev;
	struct udevice *i2c_des_dev;
	struct udevice *i2c_des2_dev;
};

struct SER_DES_REG{
  int devaddr;
  unsigned int regaddr;
  uint8_t value;
};

static struct SER_DES_REG stserdes_porta[] = {
	/*
	 * DSI Port A - PIPE X - LINK A (GMSL2)
	 */
	{0xFF, 0xFFFF, 0xFF}, // delay 50ms
	// Reset All and release
	{MAX96789_SER_ADDR, 0x10, 0x80}, {MAX96789_SER_ADDR, 0x10, 0x11},

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
	{MAX96878_DES_ADDR, 0x01CE, 0x4E}, // DES oLDI setting		// GPIO/I2C Setting
	{MAX96789_SER_ADDR, 0x001, 0x8}, // I2C pass-through

#ifdef CONFIG_TCC750X
	{MAX96789_SER_ADDR, 0x380, 0x0D}, // Pol
#endif
#ifdef CONFIG_TCC807X
	{MAX96789_SER_ADDR, 0x380, 0x3D}, // Pol
	{MAX96789_SER_ADDR, 0x390, 0x3D}, // Pol
#endif

	// MFP2 (GPIO02) - MFP18 (GPIO18) // LCD_ON
	{MAX96789_SER_ADDR, 0x2C5, 0xAA}, // SER GPIO_TX_ID  : 0xA (  to GPIO18)
	{MAX96878_DES_ADDR, 0x238, 0x6A}, // DES GPIO_RX_ID  : 0xA (from GPIO2)
	{MAX96878_DES_ADDR, 0x236, 0x85}, // GPIO18 Des setting
	{MAX96789_SER_ADDR, 0x2C4, 0x83}, // GPIO2 Ser setting
	{MAX96878_DES_ADDR, 0x236, 0x84}, // GPIO18 Des setting

	// MFP3 (GPIO03) - MFP17 (GPIO17) // RESET
	{MAX96789_SER_ADDR, 0x2C8, 0xAB}, // SER GPIO_TX_ID  : 0xB   (to GPIO17)
	{MAX96878_DES_ADDR, 0x235, 0x6B}, // DES GPIO_RX_ID  : 0xB (from GPIO3)
	{MAX96878_DES_ADDR, 0x233, 0x85}, // GPIO17 Des setting
	{MAX96789_SER_ADDR, 0x2C7, 0x83}, // GPIO3 Ser setting
	{MAX96878_DES_ADDR, 0x233, 0x84}, // GPIO17 Des setting

	// MFP7 (GPIO07) - MFP2 (GPIO2) // BL_EN
	{MAX96789_SER_ADDR, 0x2D4, 0xAC}, // SER GPIO_TX_ID  : 0xC (  to GPIO2)
	{MAX96878_DES_ADDR, 0x208, 0x6C}, // DES GPIO_RX_ID  : 0xC (from GPIO7)
	{MAX96878_DES_ADDR, 0x206, 0x85}, // GPIO2 Des setting
	{MAX96789_SER_ADDR, 0x2D3, 0x83}, // GPIO7 Ser setting
	{MAX96878_DES_ADDR, 0x206, 0x84}, // GPIO2 Des setting

	// MFP5 (GPIO5) // MIPI DET LED_ON
	{MAX96878_DES_ADDR, 0x20F, 0x90}, // LED ON

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
static struct SER_DES_REG stserdes_port_both[] = {
	/*
	 * DSI Port A - PIPE X - LINK A (GMSL2)
	 * DSI Port B - PIPE Z - LINK B (GMSL2)
	 */
	{0xFF, 0xFFFF, 0xFF}, // delay
	{MAX96789_SER_ADDR, 0x10, 0x80}, // Reset All
	{MAX96789_SER_ADDR, 0x10, 0x11}, // Reset All release

	{MAX96789_SER_ADDR, 0x330, 0x06}, // MIPI Rx PHY - BOTH Port A, B enabled
	{MAX96789_SER_ADDR, 0x331, 0x11}, // Number of Lanes default 2lanes

	{MAX96789_SER_ADDR, 0x005B, 0x22}, // DES video stream select = 0b10

	{MAX96789_SER_ADDR, 0x010, 0x23}, // Enable Both Link
	{MAX96789_SER_ADDR, 0x002, 0x53}, // Only Video transmit Channal X, Z

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
	{MAX96878_DES_ADDR, 0x01CE, 0x4E}, // DES oLDI setting		// GPIO/I2C Setting
	
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
	{MAX96878_DES2_ADDR, 0x01CE, 0x4E}, // DES oLDI setting		// GPIO/I2C Setting

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
	{MAX96789_SER_ADDR, 0x2C5, 0xAA}, // SER GPIO_TX_ID  : 0xA (  to GPIO18)
	{MAX96878_DES_ADDR, 0x238, 0x6A}, // DES GPIO_RX_ID  : 0xA (from GPIO2)
	{MAX96878_DES_ADDR, 0x236, 0x85}, // GPIO18 Des setting
	{MAX96789_SER_ADDR, 0x2C4, 0x83}, // GPIO2 Ser setting
	{MAX96878_DES_ADDR, 0x236, 0x84}, // GPIO18 Des setting

	// MFP3 (GPIO03) - MFP17 (GPIO17) // RESET
	{MAX96789_SER_ADDR, 0x2C8, 0xAB}, // SER GPIO_TX_ID  : 0xB   (to GPIO17)
	{MAX96878_DES_ADDR, 0x235, 0x6B}, // DES GPIO_RX_ID  : 0xB (from GPIO3)
	{MAX96878_DES_ADDR, 0x233, 0x85}, // GPIO17 Des setting
	{MAX96789_SER_ADDR, 0x2C7, 0x83}, // GPIO3 Ser setting
	{MAX96878_DES_ADDR, 0x233, 0x84}, // GPIO17 Des setting

	// MFP7 (GPIO07) - MFP2 (GPIO2) // BL_EN
	{MAX96789_SER_ADDR, 0x2D4, 0xAC}, // SER GPIO_TX_ID  : 0xC (  to GPIO2)
	{MAX96878_DES_ADDR, 0x208, 0x6C}, // DES GPIO_RX_ID  : 0xC (from GPIO7)
	{MAX96878_DES_ADDR, 0x206, 0x85}, // GPIO2 Des setting
	{MAX96789_SER_ADDR, 0x2D3, 0x83}, // GPIO7 Ser setting
	{MAX96878_DES_ADDR, 0x206, 0x84}, // GPIO2 Des setting

	// MFP5 (GPIO5) // MIPI DET LED_ON
	{MAX96878_DES_ADDR, 0x20F, 0x90}, // LED ON

	/* DSI1 */
	// MFP12 (GPIO12) - MFP18 (GPIO18) // LCD_ON
	{MAX96789_SER_ADDR, 0x2E3, 0xAD}, // SER GPIO_TX_ID  : 0xD (  to GPIO18)
	{MAX96878_DES2_ADDR,0x238, 0x6D}, // DES GPIO_RX_ID  : 0xD (from GPIO12)
	{MAX96878_DES2_ADDR,0x236, 0x85}, // GPIO18 Des setting
	{MAX96789_SER_ADDR, 0x2E2, 0x83}, // GPIO12 Ser setting
	{MAX96878_DES2_ADDR,0x236, 0x84}, // GPIO18 Des setting

	// MFP13 (GPIO13) - MFP17 (GPIO17) // RESET
	{MAX96789_SER_ADDR, 0x2E6, 0xAE}, // SER GPIO_TX_ID  : 0xE (  to GPIO17)
	{MAX96878_DES2_ADDR,0x235, 0x6E}, // DES GPIO_RX_ID  : 0xE (from GPIO13)
	{MAX96878_DES2_ADDR,0x233, 0x85}, // GPIO17 Des setting
	{MAX96789_SER_ADDR, 0x2E5, 0x83}, // GPIO13 Ser setting
	{MAX96878_DES2_ADDR,0x233, 0x84}, // GPIO17 Des setting

	// MFP14 (GPIO14) - MFP2 (GPIO2) // BL_EN
	{MAX96789_SER_ADDR, 0x2E9, 0xAF}, // SER GPIO_TX_ID  : 0xF (  to GPIO2)
	{MAX96878_DES2_ADDR,0x208, 0x6F}, // DES GPIO_RX_ID  : 0xF (from GPIO14)
	{MAX96878_DES2_ADDR,0x206, 0x85}, // GPIO14 Des setting
	{MAX96789_SER_ADDR, 0x2E8, 0x83}, // GPIO2 Ser setting
	{MAX96878_DES2_ADDR,0x206, 0x84}, // GPIO14 Des setting

	// MFP5 (GPIO5) // MIPI DET LED_ON
	{MAX96878_DES2_ADDR, 0x20F, 0x90}, // LED ON
	{0,0,0}
};

#if 0 // Use this table when using portb only
static struct SER_DES_REG stserdes_portb[] = {
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

	{MAX96878_DES2_ADDR, 0x01CE, 0x4E}, // DES oLDI setting		// GPIO/I2C Setting
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
#endif

int tcc_dsi_set_serdes_busnum(struct udevice **i2c_dev, int MAX9XXXX_ADDR)
{
	int i2c_port = SERDES_I2C_PORT;
	int retry = 0;
	int ret = 0;

	if (i2c_port < 0) {
		debug_pr(LOGL_ERR, "[ERR][I2C] not valid Port\n");
		return -EINVAL;
	}

	ret = uclass_get_device_by_seq(UCLASS_I2C, i2c_port, i2c_dev);
	if (!ret) {
		ret = dm_i2c_set_bus_speed(*i2c_dev, 100000);
		if (ret) {
			debug_pr(LOGL_ERR,
				"\n[%s:%d]Error: from dm_i2c_set_bus_speed().. Ret(0x%x)\n",
				__func__, __LINE__, ret);
		}
	} else {
		debug_pr(LOGL_ERR,
			"\n[%s:%d]Error: can't get i2c bus %d\n",
			__func__, __LINE__, i2c_port);
		return -EINVAL;
	}
	for (retry = 100; retry >= 0 ; retry--) {
		ret = i2c_get_chip_for_busnum(i2c_port,
					      MAX9XXXX_ADDR,
					      2,
					      i2c_dev);
		if (ret) {
			if (retry == 0) {
				debug_pr(LOGL_ERR, "[%d] Failed to get i2c handle for device addr 0x%x\n",
					__LINE__,
					MAX9XXXX_ADDR);
				return -EINVAL;
			}
			mdelay(1);
		} else {
			if(retry != 100)
				debug_pr(LOGL_INFO, "retry %d - for addr 0x%x\n", retry,
					MAX9XXXX_ADDR);
			break;
		}
	}
	return 0;
}

void tcc_dsi_set_serdes_overwrite(struct SER_DES_REG *pstserdes, const struct display_timing *timings, int loop)
{
	if( (pstserdes[loop].regaddr == 0x0385U) || (pstserdes[loop].regaddr == 0x0395U) ) {
		pstserdes[loop].value = (uint8_t)(timings->hsync_len.typ & 0xffu);
	}
	if( (pstserdes[loop].regaddr == 0x0386U) || (pstserdes[loop].regaddr == 0x0396U) ) {
		pstserdes[loop].value = (uint8_t)(timings->vsync_len.typ & 0xffu);
	}
	if( (pstserdes[loop].regaddr == 0x0387U) || (pstserdes[loop].regaddr == 0x0397U) ) {
		pstserdes[loop].value = (uint8_t)((((timings->hsync_len.typ >> 8) & 0xfu) << 4) | ((timings->hsync_len.typ >> 8) & 0xfu));
	}
	if( (pstserdes[loop].regaddr == 0x03A5U) || (pstserdes[loop].regaddr == 0x03B1U) ) {
		pstserdes[loop].value = (uint8_t)(timings->vfront_porch.typ & 0xffu);
	}
	if( (pstserdes[loop].regaddr == 0x03A7U) || (pstserdes[loop].regaddr == 0x03B3U) ) {
		pstserdes[loop].value = (uint8_t)((timings->vback_porch.typ >> 4) & 0xffu);
	}
	if( (pstserdes[loop].regaddr == 0x03A6U) || (pstserdes[loop].regaddr == 0x03B2U) ) {
		pstserdes[loop].value = (uint8_t)(((timings->vback_porch.typ & 0xfu) << 4) | ((timings->vfront_porch.typ >> 8) & 0xfu));
	}
	if( (pstserdes[loop].regaddr == 0x03A8U) || (pstserdes[loop].regaddr == 0x03B4U) ) {
		pstserdes[loop].value = (uint8_t)(timings->vactive.typ & 0xffu);
	}
	if ((pstserdes[loop].regaddr == 0x03A9U) || (pstserdes[loop].regaddr == 0x03B5U) ) {
		pstserdes[loop].value = (uint8_t)((timings->vactive.typ >> 8) & 0xfu);
	}
	if( (pstserdes[loop].regaddr == 0x03AAU) || (pstserdes[loop].regaddr == 0x03B6U) ) {
		pstserdes[loop].value = (uint8_t)(timings->hfront_porch.typ & 0xffu);
	}
	if( (pstserdes[loop].regaddr == 0x03ACU) || (pstserdes[loop].regaddr == 0x03B8U) ) {
		pstserdes[loop].value = (uint8_t)((timings->hback_porch.typ >> 4) & 0xffu);
	}
	if( (pstserdes[loop].regaddr == 0x03ABU) || (pstserdes[loop].regaddr == 0x03B7U) ) {
		pstserdes[loop].value = (uint8_t)(((timings->hback_porch.typ & 0xfu) << 4) | ((timings->hfront_porch.typ >> 8) & 0xfu));
	}
	if( (pstserdes[loop].regaddr == 0x03ADU) || (pstserdes[loop].regaddr == 0x03B9U) ) {
		pstserdes[loop].value = (uint8_t)(timings->hactive.typ & 0xffu);
	}
	if( (pstserdes[loop].regaddr == 0x03AEU) || (pstserdes[loop].regaddr == 0x03BAU) ) {
		pstserdes[loop].value = (uint8_t)((timings->hactive.typ >> 8) & 0xffu);
	}
}

static void tcc_dsi_set_serdes(const struct display_timing *timings, unsigned int lane0, unsigned int lane1)
{
	int loop, retry, ret;

	struct SER_DES_BUS_DEV i2c_ser_des_dev;
	struct udevice *i2c_write_dev;
	struct SER_DES_REG *pstserdes;

	if(lane1 == 0U) { // Only A port used
		pstserdes = stserdes_porta;
	} else { // Both A,B used
		pstserdes = stserdes_port_both;
	}
	ret =  tcc_dsi_set_serdes_busnum(&i2c_ser_des_dev.i2c_ser_dev, MAX96789_SER_ADDR);
	ret |= tcc_dsi_set_serdes_busnum(&i2c_ser_des_dev.i2c_des_dev, MAX96878_DES_ADDR);

	if(ret == 0) {
		for (loop = 0; !((pstserdes[loop].devaddr == 0) &&
				(pstserdes[loop].regaddr == 0U) &&
				(pstserdes[loop].value == 0U)); loop++) {
			if ((pstserdes[loop].devaddr == 0xFF) &&
				(pstserdes[loop].regaddr == 0xFFFFU) &&
				(pstserdes[loop].value == 0xFFU)){
				mdelay(100);
				continue;
			}

			// Overwrite
			if(pstserdes[loop].regaddr == 0x0331U) {
				if(lane1 >= 1U) {
					pstserdes[loop].value = (uint8_t)(((lane1 -1U) << 4)&0xFFU);
				}
				if(lane0 >= 1U) {
					pstserdes[loop].value |= (uint8_t)((lane0 -1U)&0xFFU);
				}
			}
			tcc_dsi_set_serdes_overwrite(pstserdes, timings, loop);

			// I2C Write
			if(pstserdes[loop].devaddr == MAX96789_SER_ADDR) {
				i2c_write_dev = i2c_ser_des_dev.i2c_ser_dev;
			} else if(pstserdes[loop].devaddr == MAX96878_DES_ADDR) {
				i2c_write_dev = i2c_ser_des_dev.i2c_des_dev;
			} else if(pstserdes[loop].devaddr == MAX96878_DES2_ADDR) {
				static int init_busnum = 1;
				if(init_busnum == 1) {
					ret = tcc_dsi_set_serdes_busnum(&i2c_ser_des_dev.i2c_des2_dev, MAX96878_DES2_ADDR);
					if(ret != 0) {
						debug_pr(LOGL_ERR, "[%s:%d] ser/des i2c dev set fail\n", __func__, __LINE__);
						break;
					}
					init_busnum = 0;
				}
				i2c_write_dev = i2c_ser_des_dev.i2c_des2_dev;
			}
			for(retry = 0 ; retry <= 300 ; retry++) {
				ret = dm_i2c_write(i2c_write_dev, pstserdes[loop].regaddr,
					&pstserdes[loop].value, 1);
				if(ret == 0) {
					//debug_pr(LOGL_ERR, "OK%02d  -  [Dev 0x%02x regaddr 0x%04x value0x%02x ]\n", retry, pstserdes[loop].devaddr, pstserdes[loop].regaddr, pstserdes[loop].value);
					break;
				} else {
					if(retry == 300)
						debug_pr(LOGL_ERR, "X%02d - [Dev 0x%02x regaddr 0x%04x value0x%02x ]\n", retry, pstserdes[loop].devaddr, pstserdes[loop].regaddr, pstserdes[loop].value);
				}
				mdelay(1);
			}
		}
	} else {
		debug_pr(LOGL_ERR, "[%s:%d] ser/des i2c dev set fail\n", __func__, __LINE__);
	}

	debug_pr(LOGL_INFO, "[%d][%s] Done\n", __LINE__, __func__);
}


#ifdef CONFIG_TCC807X
int32_t dsi_enable_v2(struct mipi_dsi_dev *dev, const struct display_timing *timings)
{
	int32_t ret = 0;
	struct dsih_core_ipi_t *ipi_video = &dev->ipi_cfg;

	//dev->pclk =  (tcc_get_peri(PERI_LCD2) / 1000);
	if(dev->pclk > (ULONG_MAX / 24UL)) {
		debug_pr(LOGL_ERR, "pclk * 24 is over. Please check the pclk value.\n");
		ret = -EINVAL;
	} else {
		dev->data_rate = (dev->pclk) * 24U / dev->phy_cfg.phy_lanes;
	}

	if(ret ==0) {
		dev->data_rate /= 1000U;
		debug_pr(LOGL_ERR, "PHY data_rate : %ldmbps\n", dev->data_rate);
		if((dev->data_rate % 10U) != 0U) {
			debug_pr(LOGL_ERR, "[DSI]Request D-PHY Data rate not supported!\n Please set as multiple of 10mbps\n");
			ret = -EINVAL;
			//dev->data_rate /= 10U;
			//dev->data_rate += 1U;
			//dev->data_rate *= 10U;
		}
		if((dev->data_rate > 2500U) || (dev->data_rate < 100U)) {
			debug_pr(LOGL_ERR, "D-PHY Bandwidth out of range\n");
			ret = -EINVAL;
		}
	}

	if(ret == 0) {
		ipi_video->vid_hsa_time = timings->hsync_len.typ;
		ipi_video->vid_hbp_time = timings->hback_porch.typ;
		ipi_video->vid_hfp_time = timings->hfront_porch.typ;
		ipi_video->vid_hact_time = timings->hactive.typ;
		if ( (ipi_video->vid_hsa_time > (UINT_MAX - ipi_video->vid_hbp_time))
			|| ((ipi_video->vid_hsa_time  + ipi_video->vid_hbp_time) > (UINT_MAX - ipi_video->vid_hact_time))
			|| ((ipi_video->vid_hsa_time  + ipi_video->vid_hbp_time + ipi_video->vid_hact_time) > (UINT_MAX - timings->hback_porch.typ)) ) {
			debug_pr(LOGL_ERR, "vid_hline_time is over. Please check the hline_time.\n");
			ipi_video->vid_hline_time = 0U;
		} else {
			ipi_video->vid_hline_time = ipi_video->vid_hsa_time + ipi_video->vid_hbp_time + ipi_video->vid_hact_time + timings->hback_porch.typ;
		}
		ipi_video->vid_vsa_lines = timings->vsync_len.typ;
		ipi_video->vid_vbp_lines = timings->vback_porch.typ;
		ipi_video->vid_vact_lines = timings->vactive.typ;
		ipi_video->vid_vfp_lines = timings->vfront_porch.typ;

		debug_pr(LOGL_INFO, "ipi_video->vid_hsa_time  :  %d\n", ipi_video->vid_hsa_time );
		debug_pr(LOGL_INFO, "ipi_video->vid_hbp_time  :  %d\n", ipi_video->vid_hbp_time );
		debug_pr(LOGL_INFO, "ipi_video->vid_hact_time  : %d\n", ipi_video->vid_hact_time );
		debug_pr(LOGL_INFO, "ipi_video->vid_hline_time : %d\n", ipi_video->vid_hline_time);
		debug_pr(LOGL_INFO, "ipi_video->vid_vsa_lines  : %d\n", ipi_video->vid_vsa_lines );
		debug_pr(LOGL_INFO, "ipi_video->vid_vbp_lines  : %d\n", ipi_video->vid_vbp_lines );
		debug_pr(LOGL_INFO, "ipi_video->vid_vact_lines : %d\n", ipi_video->vid_vact_lines);
		debug_pr(LOGL_INFO, "ipi_video->vid_vfp_lines  : %d\n", ipi_video->vid_vfp_lines );

		if(dev->port == 0U) {
			tcc_dsi_set_serdes(timings, dev->lane0, dev->lane1);
		}
		tcc_dsi_phy_init(dev);
		mdelay(200);
		(void)mipi_dsih_ipi_video(dev);
	}

	return ret;
}
#endif

#ifdef CONFIG_TCC750X
int32_t dsi_enable_v1(struct mipi_dsi_dev *dev, const struct display_timing *timings)
{

	int32_t ret = 0;

	dsih_dpi_video_t *video = &dev->dpi_video;
	dev->core_addr = (void __iomem *)(uintptr_t)DSI_CORE_ADDR;
	dev->cfg_addr = (void __iomem *)(uintptr_t)CAM_CFG_ADDR;
	volatile unsigned int* reg = (volatile unsigned int*)(uintptr_t)MIPI_DSI_CTRL;

	video->h_active_pixels = timings->hactive.typ;
	video->h_sync_pixels = timings->hsync_len.typ;
	video->h_back_porch_pixels = timings->hback_porch.typ;
	if ( (timings->hactive.typ > (UINT_MAX - timings->hback_porch.typ))
		|| ((timings->hactive.typ + timings->hback_porch.typ) > (UINT_MAX - timings->hfront_porch.typ))
		|| ((timings->hactive.typ + timings->hback_porch.typ + timings->hfront_porch.typ) > (UINT_MAX - timings->hsync_len.typ)) ) {
		debug_pr(LOGL_ERR, "h_total_pixels is overflow, Please check the h_total_pixels value.\n");
		video->h_total_pixels = 0U;
	} else {
		video->h_total_pixels = timings->hactive.typ + timings->hback_porch.typ + timings->hfront_porch.typ + timings->hsync_len.typ;
	}

	video->v_active_lines = timings->vactive.typ; /* vadr */
	video->v_sync_lines = timings->vsync_len.typ;
	video->v_back_porch_lines = timings->vback_porch.typ; /* vbp */
	video->v_total_lines = timings->vactive.typ + timings->vsync_len.typ
						+ timings->vback_porch.typ + timings->vfront_porch.typ;	  /* vfp */
	video->h_polarity = (((unsigned)timings->flags & (unsigned)DISPLAY_FLAGS_HSYNC_LOW) != 0U) ? 1 : 0;
	video->v_polarity = (((unsigned)timings->flags & (unsigned)DISPLAY_FLAGS_VSYNC_LOW) != 0U) ? 1 : 0;
	video->data_en_polarity = (((unsigned)timings->flags & (unsigned)DISPLAY_FLAGS_DE_LOW) != 0U) ? 1 : 0;

	video->pixel_clock = (tcc_get_peri(PERI_DISP) / 1000U);
	debug_pr(LOGL_ERR, "pixel_clock : %lu khs\n", video->pixel_clock);
		#if 1
	// TODO : Data rate cal
	dev->data_rate = video->pixel_clock * 24UL / video->no_of_lanes;
	dev->data_rate /= 1000U;
	debug_pr(LOGL_ERR, "pixel_clock : %lu khs\n", video->pixel_clock);
	debug_pr(LOGL_ERR, "PHY data_rate : %ldmbps\n", dev->data_rate);
	if((dev->data_rate % 10U) != 0U) {
		// need round up
		debug_pr(LOGL_ERR, "[DSI]Request D-PHY Data rate not supported!\n Please set as multiple of 10mbps");
		ret = -EINVAL;
	}
	if( (dev->data_rate > 2500U) || (dev->data_rate < 100U) ) {
		debug_pr(LOGL_ERR, "D-PHY Bandwidth out of range\n");
		ret = -EINVAL;
	}
		#else
			dev->data_rate = 1050;
		#endif

	if(ret == 0) {
		video->byte_clock = (dev->data_rate * 1000U) / 8U;
		debug_pr(LOGL_INFO, "PHY byte clock : %ld kbps\n", video->byte_clock);

		video->video_mode = VIDEO_NON_BURST_WITH_SYNC_PULSES;  /* Only Video value */
		if( (video->color_coding == COLOR_CODE_18BIT_CONFIG1)
				|| (video->color_coding == COLOR_CODE_18BIT_CONFIG2) ) { // 18BPP
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

		tcc_dsi_set_serdes(timings, video->no_of_lanes, 0);
		(void)mipi_dsih_dpi_video(dev);
		mdelay(100);
		tcc_dsi_phy_init(dev);
		__raw_writel(0x10002, reg);
	}

	return ret;
}
#endif
