// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 - present Telechips.co and/or its affiliates.
 */
#include <config.h>
#include <common.h>
#include <lcd.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <telechips/hdmi_v2_0.h>
#include <asm/io.h>
#include <include/video_params.h>
#include <include/hdmi_misc.h>
#include <hdmi_api_lib/include/api/api.h>
#include <hdmi_edid.h>
#include "hdmi_v2_0.h"
#include "hdmiv20_ctrl.h"
#if defined(CONFIG_HDMI_PANEL_MODE)
#include <i2c.h>
#include <dm.h>
#endif

struct hdmiv20_info_t {
	int hdmi_mode;
	int hdmi_vic;
	int hdmi_depth;
	int hdmi_framepacking;
	enum em_encoding hdmi_encoding;
	int cmd_mode;
};

static struct hdmiv20_info_t hdmiv20_info = {
	.hdmi_vic	       = 0,
	.hdmi_depth	     = (int)HDMI_COLOR_DEPTH,
	.hdmi_framepacking      = 0,
	.hdmi_encoding	  = HDMI_COLOR_SPACE,
	.cmd_mode	       = 0,
};

#if defined(CONFIG_HDMI_PANEL_MODE)
struct hdmi_lvds_data {
	unsigned int addr;
	/* coverity[misra_c_2012_rule_5_8_violation : FALSE] */
	unsigned int reg;
	unsigned char val;
	unsigned char mismatch;
};

static struct hdmi_lvds_data hdmi_rd_info_vic_1024[] = {
	{0x98, 0xea, 0x20, 0},
	{0x98, 0xeb, 0xc4, 0},
	{0x00, 0x00, 0x00, 0},
};

static struct hdmi_lvds_data hdmi_lvds_vic_1024[] = {
	/* 1920x720p@60 HDMI Input_LVDS Output Port A and B*/
	{0x98, 0xFF, 0x80, 0}, /*  ;default 0x00, I2C reset - self clearing */
	{0xFF, 0x00, 0x05, 0}, /* wait 5ms */
	{0x98, 0xF4, 0x80, 0}, /* ;default 0x00, CEC Map Address set to 0x80 */
	/* ;default 0x00, Infoframe Map Address set to 0x7C */
	{0x98, 0xF5, 0x7C, 0},
	/* ;default 0x00, DPLL Map Address set to 0x4C */
	{0x98, 0xF8, 0x4C, 0},
	/* ;default 0x00, KSV Map Address set to 0x64 */
	{0x98, 0xF9, 0x64, 0},
	/* ;default 0x00, EDID Map Address set to 0x6C */
	{0x98, 0xFA, 0x6C, 0},
	/* ;default 0x00, HDMI Map Address set to 0x68 */
	{0x98, 0xFB, 0x68, 0},
	/* ;default 0x00, CP Map Address set to 0x44 */
	{0x98, 0xFD, 0x44, 0},
	/* ;default 0x00, LVDS Map Address set to 0xC0 */
	{0x98, 0xE9, 0xC0, 0},
	/*
	 * ;default 0x08, [5:0] - VID_STD
	 * [5:0] = 6'b010010 - WXGA 1360x768p@60Hz
	 */
	{0x98, 0x00, 0x12, 0},
	/* ;default 0x06, [3:0] - Prim_Mode[3:0] = 4'b0110 - HDMI-Gr */
	{0x98, 0x01, 0x06, 0},
	/*
	 * ;default 0xF0, [7:4] - INP_COLOR_SPACE
	 * [3:0] = 4'b1111 - color space determined by HDMI block,
	 * [1] RGB_OUT - 1'b1 - RGB color space output
	 */
	{0x98, 0x02, 0xF2, 0},
	/*
	 * ;default 0x00, [7:0] - OP_FORMAT_SEL
	 * [7:0] = 8'b01000010 - 36-bit 4:4:4 SDR Mode 0
	 */
	{0x98, 0x03, 0x42, 0},
	{0x98, 0x04, 0x63, 0}, /* ;enable CP timing adjustment */
	/*
	 * ;default 0x2C
	 * [2] - AVCODE_INSERT_EN = 1'b0 - added 0x20 disable data blanking
	 * for script reloadi
	 */
	{0x98, 0x05, 0x20, 0},
	/* ;default 0x62, [5] - POWER_DOWN = 1'b0 - Powers up part  */
	{0x98, 0x0C, 0x42, 0},
	/*
	 * ;default 0xBE, [4] - TRI_AUDIO = 1'b0 = untristate Audio ,
	 * [3] - TRI_LLC = 1'b1 = tristate LLC,
	 * Bit[1] - TRI_PIX = 1'b1 = Tristate Pixel Pads
	 */
	{0x98, 0x15, 0xAE, 0},
	{0x44, 0x6C, 0x00, 0}, /* ;default 0x10, ADI Recommended write */
	{0x44, 0x8B, 0x40, 0}, /* ;HS DE adjustment */
	{0x44, 0x8C, 0x02, 0}, /* ;HS DE adjustment */
	{0x44, 0x8D, 0x02, 0}, /* ;HS DE adjustment */
	/*
	 * ;outputs default colors as given in DEF_COL_CHA,
	 * DEF_COL_CHB, and DEF_COL_CHC
	 */
	{0x44, 0xBF, 0x16, 0},
	{0x44, 0xC0, 0x00, 0}, /* ;default color of G */
	{0x44, 0xC1, 0x00, 0}, /* ;default color of R */
	{0x44, 0xC2, 0x00, 0}, /* ;default color of B */
	/* ;default 0x83, BCAPS[7:0] - Disable HDCP 1.1 features */
	{0x64, 0x40, 0x81, 0},
	{0x68, 0x03, 0x98, 0}, /* ;default 0x18, ADI Recommended write */
	{0x68, 0x10, 0xA5, 0}, /* ;default 0x25, ADI Recommended write */
	{0x68, 0x1B, 0x08, 0}, /* ;default 0x18, ADI Recommended write */
	{0x68, 0x45, 0x04, 0}, /* ;default 0x00, ADI Recommended write */
	{0x68, 0x97, 0xC0, 0}, /* ;default 0x80, ADI Recommended write */
	{0x68, 0x3D, 0x10, 0}, /* ;default 0x00, ADI Recommended write */
	{0x68, 0x3E, 0x7B, 0}, /* ;default 0x79, ADI recommended write */
	{0x68, 0x3F, 0x5E, 0}, /* ;default 0x63, ADI Recommended Write */
	{0x68, 0x4E, 0xFE, 0}, /* ;default 0x7B, ADI recommended write */
	{0x68, 0x4F, 0x08, 0}, /* ;default 0x63, ADI recommended write */
	{0x68, 0x57, 0xA3, 0}, /* ;default 0x30, ADI recommended write */
	{0x68, 0x58, 0x07, 0}, /* ;default 0x01, ADI recommended write */
	{0x68, 0x6F, 0x08, 0}, /* ;default 0x00, ADI Recommended write */
	{0x68, 0x83, 0xFE, 0}, /* ;default 0xFF, ADI recommended write */
	{0x68, 0x86, 0x9B, 0}, /* ;default 0x00, ADI recommended write */
	{0x68, 0x85, 0x10, 0}, /* ;default 0x16, ADI recommended write */
	{0x68, 0x89, 0x01, 0}, /* ;default 0x00, ADI recommended write */
	{0x68, 0x9B, 0x03, 0}, /* ;default 0x0B, ADI Recommended write */
	{0x68, 0x9C, 0x80, 0}, /* ;default 0x08, ADI Recommended write */
	{0x68, 0x9C, 0xC0, 0}, /* ;default 0x08, ADI Recommended write */
	{0x68, 0x9C, 0x00, 0}, /* ;default 0x08, ADI Recommended write */
	/*
	 * ;default 0x02, Bit [0] tx_mode_itu656 - 1'b0 = OLDI mode,
	 * Bit [1] tx_pdn - 1'b0 = LVDS TX powered on,
	 * Bit [3] tx_pll_en - 1'b1 =  power up LVDS PLL
	 */
	{0xC0, 0x40, 0x08, 0},
	{0xC0, 0x43, 0x03, 0}, /* ;default 0x00, ADI Recommended write */
	{0xC0, 0x44, 0x00, 0}, /* ;default 0x00, PLL GEAR < 200MHz */
	{0xC0, 0x45, 0x04, 0}, /* ;default 0x1E, ADI Recommended write */
	{0xC0, 0x46, 0x53, 0}, /* ;default 0x77, ADI Recommended write */
	{0xC0, 0x47, 0x03, 0}, /* ;default 0x02, ADI Recommended write */
	/*
	 * ;default 0x71, Bit [6] tx_oldi_hs_pol - 1'b0 = HS Polarity Neg,
	 * Bit [5] tx_oldi_vs_pol - 1'b0 = VS Polarity Neg,
	 * Bit [4] tx_oldi_de_pol - 1'b1 =  DE Polarity Pos,
	 * Bit [3] tx_enable_ns_mapping - 1'b0 = normal oldi 8-bit mapping,
	 * Bit [2] tx_656_all_lanes_enable - 1'b0 = disable 656 data on lanes
	 * Bit [1] tx_oldi_balanced_mode - 1'b0 = Non DC balanced ,
	 * Bit [0] tx_color_mode - 1'b1 = 8-bit mode
	 */
	{0xC0, 0x4C, 0x19, 0},
	/*
	 * ;default 0x08, Bit [3:2] tx_color_depth[1:0] - 2'b01 = 8-bit Mode,
	 * Bit [1] tx_int_res - 1'b0 = Res bit 0,
	 * Bit[0] tx_mux_int_res - 1'b0 = disable programming of res bit
	 */
	{0xC0, 0x4E, 0x04, 0},
	{0x00, 0x00, 0x00, 0}
};

static struct hdmi_lvds_data hdmi_lvds_vic_1025[] = {
	/* 1024x600@60 HDMI Input_LVDS Output Port A */
	{0x98, 0xFF, 0x80, 0}, /*  ;default 0x00, I2C reset - self clearing */
	{0xFF, 0x00, 0x0A, 0}, /* wait 10ms */
	{0x98, 0xF4, 0x80, 0}, /* ;default 0x00, CEC Map Address set to 0x80 */
	/* ;default 0x00, Infoframe Map Address set to 0x7C */
	{0x98, 0xF5, 0x7C, 0},
	/* ;default 0x00, DPLL Map Address set to 0x4C */
	{0x98, 0xF8, 0x4C, 0},
	{0x98, 0xF9, 0x64, 0}, /* ;default 0x00, KSV Map Address set to 0x64 */
	/* ;default 0x00, EDID Map Address set to 0x6C */
	{0x98, 0xFA, 0x6C, 0},
	/* ;default 0x00, HDMI Map Address set to 0x68 */
	{0x98, 0xFB, 0x68, 0},
	{0x98, 0xFD, 0x44, 0}, /* ;default 0x00, CP Map Address set to 0x44 */
	/* ;default 0x00, LVDS Map Address set to 0xC0 */
	{0x98, 0xE9, 0xC0, 0},
	/*
	 * ;default 0x08, [5:0] - VID_STD
	 * [5:0] = 6'b010010 - WXGA 1360x768p@60Hz
	 */
	{0x98, 0x00, 0x12, 0},
	/* ;default 0x06, [3:0] - Prim_Mode[3:0] = 4'b0110 - HDMI-Gr */
	{0x98, 0x01, 0x06, 0},
	/*
	 * ;default 0xF0, [7:4] - INP_COLOR_SPACE
	 * [3:0] = 4'b1111 - color space determined by HDMI block,
	 * [1] RGB_OUT - 1'b1 - RGB color space output
	 */
	{0x98, 0x02, 0xF2, 0},
	/*
	 * ;default 0x00, [7:0] - OP_FORMAT_SEL
	 * [7:0] = 8'b01000010 - 36-bit 4:4:4 SDR Mode 0
	 */
	{0x98, 0x03, 0x42, 0},
	/* ;default 0x2C  [2] - AVCODE_INSERT_EN = 1'b0 */
	{0x98, 0x05, 0x28, 0},
	/* ;default 0x62, [5] - POWER_DOWN = 1'b0 - Powers up part  */
	{0x98, 0x0C, 0x42, 0},
	/*
	 * ;default 0xBE, Bit[4] - TRI_AUDIO = 1'b0 = untristate Audio ,
	 * Bit[3] - TRI_LLC = 1'b1 = tristate LLC,
	 * Bit[1] - TRI_PIX = 1'b1 = Tristate Pixel Pads
	 */
	{0x98, 0x15, 0xAE, 0},
	{0x44, 0x6C, 0x00, 0}, /* ;default 0x10, ADI Recommended write */
	{0x44, 0x8B, 0x40, 0}, /* ;HS DE adjustment */
	{0x44, 0x8C, 0x02, 0}, /* ;HS DE adjustment */
	{0x44, 0x8D, 0x02, 0}, /* ;HS DE adjustment */
	/* ;default 0x83, BCAPS[7:0] - Disable HDCP 1.1 features */
	{0x64, 0x40, 0x81, 0},
	{0x68, 0x03, 0x98, 0}, /* ;default 0x18, ADI Recommended write */
	{0x68, 0x10, 0xA5, 0}, /* ;default 0x25, ADI Recommended write */
	{0x68, 0x1B, 0x08, 0}, /* ;default 0x18, ADI Recommended write */
	{0x68, 0x45, 0x04, 0}, /* ;default 0x00, ADI Recommended write */
	{0x68, 0x97, 0xC0, 0}, /* ;default 0x80, ADI Recommended write */
	{0x68, 0x3D, 0x10, 0}, /* ;default 0x00, ADI Recommended write */
	{0x68, 0x3E, 0x7B, 0}, /* ;default 0x79, ADI recommended write */
	{0x68, 0x3F, 0x5E, 0}, /* ;default 0x63, ADI Recommended Write */
	{0x68, 0x4E, 0xFE, 0}, /* ;default 0x7B, ADI recommended write */
	{0x68, 0x4F, 0x08, 0}, /* ;default 0x63, ADI recommended write */
	{0x68, 0x57, 0xA3, 0}, /* ;default 0x30, ADI recommended write */
	{0x68, 0x58, 0x07, 0}, /* ;default 0x01, ADI recommended write */
	{0x68, 0x6F, 0x08, 0}, /* ;default 0x00, ADI Recommended write */
	{0x68, 0x83, 0xFE, 0}, /* ;default 0xFF, ADI recommended write */
	{0x68, 0x86, 0x9B, 0}, /* ;default 0x00, ADI recommended write */
	{0x68, 0x85, 0x10, 0}, /* ;default 0x16, ADI recommended write */
	{0x68, 0x89, 0x01, 0}, /* ;default 0x00, ADI recommended write */
	{0x68, 0x9B, 0x03, 0}, /* ;default 0x0B, ADI Recommended write */
	{0x68, 0x9C, 0x80, 0}, /* ;default 0x08, ADI Recommended write */
	{0x68, 0x9C, 0xC0, 0}, /* ;default 0x08, ADI Recommended write */
	{0x68, 0x9C, 0x00, 0}, /* ;default 0x08, ADI Recommended write */
	/*
	 * ;default 0x02, Bit [0] tx_mode_itu656 - 1'b0 = OLDI mode,
	 * Bit [1] tx_pdn - 1'b0 = LVDS TX powered on,
	 * Bit [3] tx_pll_en - 1'b1 =  power up LVDS PLL
	 */
	{0xC0, 0x40, 0x08, 0},
	{0xC0, 0x43, 0x03, 0}, /* ;default 0x00, ADI Recommended write */
	{0xC0, 0x44, 0x00, 0}, /* ;default 0x00, PLL GEAR < 200MHz */
	{0xC0, 0x45, 0x04, 0}, /* ;default 0x1E, ADI Recommended write */
	{0xC0, 0x46, 0x53, 0}, /* ;default 0x77, ADI Recommended write */
	{0xC0, 0x47, 0x03, 0}, /* ;default 0x02, ADI Recommended write */
	/*
	 * ;default 0x71, Bit [6] tx_oldi_hs_pol - 1'b0 = HS Polarity Neg,
	 * Bit [5] tx_oldi_vs_pol - 1'b0 = VS Polarity Neg,
	 * Bit [4] tx_oldi_de_pol - 1'b1 =  DE Polarity Pos,
	 * Bit [3] tx_enable_ns_mapping - 1'b0 = normal oldi 8-bit mapping,
	 * Bit [2] tx_656_all_lanes_enable - 1'b0 = disable 656 data on lanes,
	 * Bit [1] tx_oldi_balanced_mode - 1'b0 = Non DC balanced,
	 * Bit [0] tx_color_mode - 1'b1 = 8-bit mode
	 */
	{0xC0, 0x4C, 0x19, 0},
	/*
	 * ;default 0x08, Bit [3:2] tx_color_depth[1:0] - 2'b01 = 8-bit Mode,
	 * Bit [1] tx_int_res - 1'b0 = Res bit 0,
	 * Bit [0] tx_mux_int_res - 1'b0 = disable programming of res bit
	 */
	{0xC0, 0x4E, 0x34, 0},
	{0x00, 0x00, 0x00, 0},
};

static struct hdmi_lvds_data hdmi_rd_info_vic_1027[] = {
	{0xFFFF, 0x0000, 0x32, 0}, // Delay
	{0xD4, 0x000D, 0xC0, 0},
	{0x44, 0x000D, 0x83, 0},
	{0x00, 0x0000, 0x00, 0},
};

static struct hdmi_lvds_data hdmi_lvds_vic_1027[] = {
	/* 1920x720@60 PVLBJT_020_01 */
	{0xD4, 0x0005, 0x70, 0},
	{0xD4, 0x01CE, 0x4E, 0},
	{0x44, 0x0001, 0xC8, 0},
	/* ##Audio */
	{0x44, 0x0120, 0x80, 0},
	{0x44, 0x0130, 0x80, 0},
	{0x44, 0x2129, 0x1C, 0},
	{0xD4, 0x0140, 0x01, 0},

	/* ##GPIO Setting */
	/* IO RX_EN should be '1' */
	{0xD4, 0x022f, 0x2f, 0},
	{0xD4, 0x022d, 0x63, 1},
	/* OLDI DES PT1 enable */
	{0xD4, 0x0006, 0x01, 0},
	/* OLDI DES ROUTE PT1 pins to GMSLPT1 */
	{0xD4, 0x0071, 0x02, 0},
	/* HDMI Ser PT1 Enable */
	{0x44, 0x0001, 0xD8, 0},
	/*-PALEN GPIO----------------------------------------------*/
	/* GPIO# 6 RX/TX TX ID 6 */
	{0x44, 0x0212, 0x41, 1},
	{0x44, 0x0213, 0x46, 0},
	{0x44, 0x0214, 0x40, 0},
	/* GPIO# 7 RX/TX TX ID 7 */
	{0x44, 0x0215, 0x41, 1},
	{0x44, 0x0216, 0x47, 0},
	{0x44, 0x0217, 0x40, 0},
	/* GPIO# 8 RX/TX TX ID 8 */
	{0x44, 0x0218, 0x41, 1},
	{0x44, 0x0219, 0x48, 0},
	{0x44, 0x021a, 0x40, 0},
	{0x44, 0x0030, 0x48, 0},
	{0x44, 0x0031, 0x88, 0},

	/* GPIO# 2 RX/TX RX ID 6 - BLEN SER_GPIO6->DES_GPIO2 */
	{0xD4, 0x0206, 0x84, 1},
	{0xD4, 0x0207, 0xb0, 0},
	{0xD4, 0x0208, 0x26, 0},
	{0x44, 0x0212, 0x43, 1},

	/* GPIO# 17 RX/TX RX ID 8 LCDRST SER_GPIO8->DES_GPIO17 */
	{0xD4, 0x0233, 0x84, 1},
	{0xD4, 0x0234, 0xb0, 0},
	{0xD4, 0x0235, 0x28, 0},
	{0x44, 0x0218, 0x43, 1},

	/* GPIO# 18 RX/TX RX ID 7 LCDON SER_GPIO7->DES_GPIO18 */
	{0xD4, 0x0236, 0x84, 1},
	{0xD4, 0x0237, 0xb0, 0},
	{0xD4, 0x0238, 0x27, 0},
	{0x44, 0x0215, 0x43, 1},
	/*---------------------------------------------------------*/

	{0x00, 0x0000, 0x00, 0},
};

static struct hdmi_lvds_data_info_table_t {
	unsigned int vic;
	unsigned int reglen; /* 1-8bit, 2-16bit */
	unsigned int devcnt;
	struct hdmi_lvds_data *hdmi_rd_info;
	struct hdmi_lvds_data *hdmi_lvds_regs;
} hdmi_lvds_data_info_table[] = {
	{
		.vic = 1024,
		.reglen = 1,
		.devcnt = 4,
		.hdmi_rd_info = hdmi_rd_info_vic_1024,
		.hdmi_lvds_regs = hdmi_lvds_vic_1024,
	},
	{
		.vic = 1025,
		.reglen = 1,
		.devcnt = 4,
		.hdmi_rd_info = NULL,
		.hdmi_lvds_regs = hdmi_lvds_vic_1025,
	},
	{
		.vic = 1027,
		.reglen = 2,
		.devcnt = 3,
		.hdmi_rd_info = hdmi_rd_info_vic_1027,
		.hdmi_lvds_regs = hdmi_lvds_vic_1027,
	},
};

static int hdmiv20_get_lvds_data_index(int vic)
{
	int index = -1;
	unsigned int i;
	unsigned long elements = sizeof(hdmi_lvds_data_info_table) /
				sizeof(struct hdmi_lvds_data_info_table_t);

	for (i = 0; i < elements; i++) {
		if(vic >= 0) {
			if ((unsigned int)vic == hdmi_lvds_data_info_table[i].vic) {
				if(i <= (UINT_MAX/2U)) {
					/* KCS */
					index = (int)i;
				}
				break;
			}
		}
	}

	return index;
}
#endif

// static int hdmiv20_panel_init(void);
// static int hdmiv20_set_power(int on);
// static int hdmiv20_set_backlight_level(int level);

// static struct vidinfo hdmiv20_panel_info = {
// 	.name = "HDMIV20",
// 	.manufacturer = "hdmiv20",
// 	.id = PANEL_ID_HDMI,
// 	#if defined(CONFIG_TCC_FB_USES_SCALER)
// 	.vl_col = 1920,
// 	.vl_row = 1080,
// 	#else
// 	.vl_col = 640,
// 	.vl_row = 480,
// 	#endif // CONFIG_TCC_FB_USES_SCALER
// 	.vl_bpix = 5, // 32BPP
// 	.vl_rot = 0, // 0"
// 	.init = hdmiv20_panel_init,
// 	.set_power = hdmiv20_set_power,
// 	.set_backlight_level = hdmiv20_set_backlight_level,
// };

// static inline unsigned int
//  (unsigned int reg, int reglen)
// {
// 	unsigned int reg_cnv_address = 0;
// 	unsigned char reg_buf[4] = {0, 0, 0, 0};

// 	switch (reglen) {
// 	case 1:
// 		reg_cnv_address = reg;
// 		break;
// 	case 2:
// 		/*
// 		 * support 16bit address
// 		 * CHIP ADDR | REG ADDR[MSB] | REG ADDR[LSB]| DATA...
// 		 */
// 		reg &= 0xFFFFU;
// 		reg_buf[0] = (unsigned char)((reg >> 8) & 0xFFU);
// 		reg_buf[1] = (unsigned char)(reg & 0xFFU);
// 		(void)memcpy(&reg_cnv_address, reg_buf, 4);
// 		break;
// 	default:
// 		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
// 		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
// 		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
// 		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
// 		pr_err("[ERR][PANEL] invalied reg len\r\n");
// 		break;
// 	}
// 	return reg_cnv_address;
// }

void tcc_hdmi_set_i2c(void)
{
	#if defined(CONFIG_HDMI_PANEL_MODE)
	int loop, retry, i2c_port;
	int index;
	unsigned int reglen;
	int ret;
	int ret_process = 0;
	unsigned char data = 0;
	unsigned int reg_cnv_address;
	unsigned int addr_prv = 0xFFFFFFFU;
	/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
	struct hdmi_lvds_data *hdmi_rd_info, *hdmi_lvds_regs;

	struct udevice  *i2c_dev = NULL;
	struct udevice  *bus_dev = NULL;

	index = hdmiv20_get_lvds_data_index(hdmiv20_info.hdmi_vic);

	if (index < 0) {
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_err("[ERR][PANEL] panel is no valid\r\n");
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto set_i2c_exit;
	}

	hdmi_rd_info = hdmi_lvds_data_info_table[index].hdmi_rd_info;
	hdmi_lvds_regs =
		hdmi_lvds_data_info_table[index].hdmi_lvds_regs;
	if (hdmi_lvds_regs == NULL) {
		/* KCS */
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto set_i2c_exit; 
	}
	reglen = hdmi_lvds_data_info_table[index].reglen;
	if ((reglen != 1U) && (reglen != 2U)) {
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_err("[ERR][PANEL] I2C reg width is not valid\r\n");
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto set_i2c_exit;
	}

	i2c_port = CONFIG_HDMI_PANEL_I2C_PORT;
	if (i2c_port < 0) {
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_err("[ERR][PANEL] I2C port is not valid\r\n");
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto set_i2c_exit;
	}
	/*
	 * read chip revision code to check
	 * whether the chip is conneted or not
	 */
	ret = uclass_get_device_by_seq(UCLASS_I2C, i2c_port, &bus_dev);
	if (!(bool)ret) {
		ret = dm_i2c_set_bus_speed(bus_dev, 400000);
		if ((bool)ret) {
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
			pr_err(
				"\n[%s:%d]Error: from dm_i2c_set_bus_speed().. Ret(0x%d)\n",
				__func__, __LINE__, ret);
		}
	} else {
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_err(
			"\n[%s:%d]Error: can't get i2c bus %d\n",
			__func__, __LINE__, i2c_port);
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto set_i2c_exit;
	}

	for (loop = 0; !((hdmi_rd_info[loop].addr == 0U) &&
			(hdmi_rd_info[loop].reg == 0U) &&
			(hdmi_rd_info[loop].val == 0U)) ; loop++) {
		if (((reglen == 1U) && (hdmi_rd_info[loop].addr == 0xFFU)) ||
			((reglen == 2U) && (hdmi_rd_info[loop].addr == 0xFFFFU))) {
			mdelay(hdmi_rd_info[loop].val);
			continue;
		}
		if (hdmi_rd_info[loop].addr == addr_prv) {
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto read_process;
		}
		for (retry = 80; retry >= 0; retry--) {
			unsigned int uitemp_aadr = hdmi_lvds_regs[loop].addr >> 1U;
			int itemp_addr = (int)uitemp_aadr;
			ret = i2c_get_chip_for_busnum(i2c_port, itemp_addr, reglen, &i2c_dev);
			if (!(bool)ret) {
				addr_prv = hdmi_rd_info[loop].addr;
				break;
			}
			if (!(bool)retry) {
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
				pr_err("HDMI: VIC[%d] I2C[0x%x] - Failed to get i2c handle.\r\n\n",
					hdmiv20_info.hdmi_vic,
					hdmi_rd_info[loop].addr >> 1);
				ret_process = -1;
				//return;
			} else {
				pr_info(
					"[%d] Getting i2c handle retry left %d\n",
					__LINE__, retry);
				mdelay(2);
			}
		}

read_process:
		if(ret_process != -1) {
			reg_cnv_address = hdmi_rd_info[loop].reg;
			for (retry = 3; retry > 0; retry--) {
				(void)dm_i2c_read(i2c_dev, reg_cnv_address, &data, 1);
				if (hdmi_rd_info[loop].val == data) {
					break;
				}
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
				pr_info("WARNING [%d][%02d] %d A_0x%02x R_0x%02x 0x%02x : 0x%02x\r\n",
					__LINE__, loop, retry,
					hdmi_rd_info[loop].addr,
					hdmi_rd_info[loop].reg,
					hdmi_rd_info[loop].val, data);
				if (!(bool)retry) {
					/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
					/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
					/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
					/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
					pr_info("[ERR][PANEL] Pannel is not attached\r\n");
					ret_process = -1;
					//return;
				}
			}
		}

	}
	if(ret_process == -1) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto set_i2c_exit;
	}
	ret_process = 0;
	addr_prv = 0xFFFFFFFU;

	for (
		loop = 0; !(
			(hdmi_lvds_regs[loop].addr == 0U) &&
			(hdmi_lvds_regs[loop].reg == 0U) &&
			(hdmi_lvds_regs[loop].val == 0U)) ; loop++) {
		if (
			((reglen == 1U) && (hdmi_lvds_regs[loop].addr == 0xFFU)) ||
			((reglen == 2U) && (hdmi_lvds_regs[loop].addr == 0xFFFFU))) {
			mdelay(hdmi_lvds_regs[loop].val);
			continue;
		}
		if (hdmi_lvds_regs[loop].addr == addr_prv) {
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto write_process;		
		}


		i2c_dev = NULL;
		for (retry = 3; retry >= 0; retry--) {
			unsigned int uitemp_aadr = hdmi_lvds_regs[loop].addr >> 1U;
			int itemp_addr = (int)uitemp_aadr;
			ret = i2c_get_chip_for_busnum(
					i2c_port, itemp_addr,
					reglen, &i2c_dev);
			if (!(bool)ret) {
				/* KCS */
				break;
			}
			if (!(bool)retry) {
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
				pr_err(
					"[%s][%d][HDMI I2C write]failed to get i2c handle for 0x%u\n",
					__func__, __LINE__,
					hdmi_lvds_regs[loop].addr >> 1);
				ret_process = -1;
			}
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
			pr_info("[%d] Getting i2c handle retry left %d\n",
				__LINE__, retry);
		}
		if(ret_process == -1) {
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto set_i2c_exit;
		}
		addr_prv = hdmi_lvds_regs[loop].addr;

write_process:
		reg_cnv_address = hdmi_lvds_regs[loop].reg;
		for (retry = 3; retry > 0; retry--) {
			/* coverity[cert_exp34_c_violation : FALSE] */
			/* coverity[var_deref_model : FALSE] */
			(void)dm_i2c_write(
				i2c_dev, reg_cnv_address,
				&hdmi_lvds_regs[loop].val, 1);
			if (
				(hdmi_lvds_regs[loop].mismatch == 0U) &&
				(hdmi_lvds_regs[loop].reg != 0xFFU)) {
				udelay(1);
				(void)dm_i2c_read(
					i2c_dev, reg_cnv_address, &data, 1);
				if (hdmi_lvds_regs[loop].val == data) {
					/* KCS */
					break;
				}
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
				pr_info(
					"  WARNING [%d][%02d] A_0x%02x R_0x%02x 0x%02x : 0x%02x\r\n",
					__LINE__, loop,
					hdmi_lvds_regs[loop].addr,
					hdmi_lvds_regs[loop].reg,
					hdmi_lvds_regs[loop].val, data);
			}
		}
	}
	#endif
set_i2c_exit:
	return;
}

int hdmiv20_panel_init(unsigned int lcd_id, unsigned int vic)
{
	#if !defined(CONFIG_HDMI_PANEL_MODE)
	struct hdmiv20_info_t previous_hdmiv20_info;
	#endif
	/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
	// struct lcd_platform_data *pdata = &hdmiv20_panel_info.dev;

	hdmi_ddi_config_init(lcd_id);

	#if defined(CONFIG_HDMI_PANEL_MODE)

	hdmiv20_info.hdmi_vic = vic;
	hdmiv20_info.hdmi_mode = (int)DVI;
	#else
	/* Backup hdmi info */
	previous_hdmiv20_info.hdmi_vic = hdmiv20_info.hdmi_vic;
	previous_hdmiv20_info.hdmi_mode = hdmiv20_info.hdmi_mode;
	previous_hdmiv20_info.hdmi_encoding = hdmiv20_info.hdmi_encoding;
	edid_get_optimal_settings(
		&hdmiv20_info.hdmi_mode, &hdmiv20_info.hdmi_vic,
		&hdmiv20_info.hdmi_encoding);
	if (hdmiv20_info.cmd_mode) {
		/* Restore hdmi info, because it it cmd mode */
		hdmiv20_info.hdmi_vic = previous_hdmiv20_info.hdmi_vic;
		hdmiv20_info.hdmi_mode = previous_hdmiv20_info.hdmi_mode;
		hdmiv20_info.hdmi_encoding =
			previous_hdmiv20_info.hdmi_encoding;
	}
	#endif /* CONFIG_HDMI_PANEL_MODE */

	return 0;
}

int hdmiv20_set_power(int on, int lcd_id, int img_width, int img_height)
{
	/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
	//struct lcd_platform_data *pdata = &hdmiv20_panel_info.dev;

	if ((bool)on) {
		tcc_hdmi_init(
			lcd_id, hdmiv20_info.hdmi_mode,
			hdmiv20_info.hdmi_vic, 0, hdmiv20_info.hdmi_depth,
			hdmiv20_info.hdmi_encoding, img_width, img_height);
	} else {
		tcc_hdmi_set_disable();
		tcc_hdmi_display_device_reset(lcd_id);
		tcc_hdmi_set_power_off();
	}
	return 0;
}

int hdmiv20_set_panel_ctrl(int level)
{
	#if defined(CONFIG_HDMI_PANEL_MODE)
	/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
	//struct lcd_platform_data *pdata = &hdmiv20_panel_info.dev;
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("%s : %d\n", __func__, level);
	if (level == 1) {
		/* Set Panel Chip by I2C */
		tcc_hdmi_set_i2c();

		/* Disable AV Mute */
		hdmi_api_avmute(0);
	}
	#endif /* CONFIG_HDMI_PANEL_MODE */
	return 0;
}

// void hdmiv20_get_panel_info(struct vidinfo *vid)
// {
// 	//pr_info("%s\n", __func__);
// 	(void)memcpy(vid, &hdmiv20_panel_info, sizeof(struct vidinfo));
// }
// EXPORT_SYMBOL(hdmiv20_get_panel_info);

// /* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
// void hdmiv20_set_panel_info(struct vidinfo *vid)
// {
// 	//pr_info("%s\n", __func__);
// 	(void)memcpy(&hdmiv20_panel_info, vid, sizeof(struct vidinfo));
// }
// EXPORT_SYMBOL(hdmiv20_set_panel_info);

void hdmi_update_panel_info(int vic, int depth, int encoding, int framepacking)
{
	hdmiv20_info.hdmi_vic = vic;
	if (
		(depth == (int)COLOR_DEPTH_8) || (depth == (int)COLOR_DEPTH_10) ||
						(depth == (int)COLOR_DEPTH_12)) {
					/* KCS */
			hdmiv20_info.hdmi_depth = depth;
		}

	if (
		(encoding == (int)RGB) || (encoding == (int)YCC444) ||
				(encoding == (int)YCC422) || (encoding == (int)YCC420)) {
					/* KCS */
			hdmiv20_info.hdmi_encoding = (enum em_encoding)encoding;
		}

	hdmiv20_info.hdmi_framepacking = framepacking;

	hdmiv20_info.cmd_mode = 1;

	/* clear frame buffer */
	//lcd_clear(); Todo

	/* Reset Vioc Blocks */
	//Todo : connect udev parameter of tcc_hdmi_dm
	(void)hdmiv20_set_power(0,0,0,0);
}
//EXPORT_SYMBOL(hdmi_update_panel_info)

int hdmi_get_framepacking(void)
{
	return hdmiv20_info.hdmi_framepacking;
}
EXPORT_SYMBOL(hdmi_update_panel_info)

// int hdmi_get_width_from_hdmi_info(void)
// {
// 	int itemp;
// 	unsigned int uitemp;
// 	if(hdmiv20_info.hdmi_vic >= 0) {
// 		/* KCS */
// 		uitemp = tcc_hdmi_get_width_from_vic((unsigned int)hdmiv20_info.hdmi_vic);
// 	} else {
// 		/* KCS */
// 		uitemp = 0;
// 	}

// 	if(uitemp <= (UINT_MAX/2U)) {
// 		/* KCS */
// 		itemp = (int)uitemp;
// 	} else {
// 		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
// 		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
// 		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
// 		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
// 		pr_err("[%s][%d] invalid width from vic\n", __func__, __LINE__);
// 		itemp = 0;
// 	}

// 	return itemp;
// }
// EXPORT_SYMBOL(hdmi_get_width_from_hdmi_info)
