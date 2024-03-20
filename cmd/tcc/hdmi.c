// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 - present Telechips and/or its affiliates.
 */
#include <common.h>
#include <lcd.h>
#include <stdio_dev.h>
#include <asm/arch/gpio.h>
#include<lcd_HDMIV20.h>
#include <asm/telechips/hdmi_v2_0.h>
#include <asm/telechips/gpio.h>
/* hdmi core */
#include <include/hdmi_access.h>
#include <include/hdmi_misc.h>
#include <include/video_params.h>
/* hdmi libs */
#include <bsp/i2cm.h>
#include <phy/phy.h>
#include <api/api.h>
#include <scdc/scdc.h>
#include <scdc/scrambling.h>
#include <phy/phy_private.h>
#include <core/packets.h>
#include <hdmi_edid.h>
#include "hdmi.h"

DECLARE_GLOBAL_DATA_PTR;

/*  HDMI VIC-----------------------------------------------------------------*/
enum {
	HF_PB4 = 0,
	HF_PB5,
	HF_PB6,
	HF_PB7,
	HF_PB8,
	HF_PB9,
	HF_PB10,
	HF_PB11,

};

static int hdmi_3d_valid;

static int pb6_fstructure;      /* 3D_F_Structure */
static int pb6_add_info_present;	/* 3D_AdditionalInfo_present */
static int pb6_disp_present;    /* 3D_DisparityData_Present */
static int pb6_meta_preset;     /* 3D_Meta_present */

static int pb7_ext_data;
static int pb8_dualview;	/* 3D_DualView */
static int pb8_viewdependcy;    /* 3D_ViewDependency */
static int pb8_preferred;       /* 3D_Preferred2DView */

static int pb9_disp_version;    /* 3D_DisparityData_version */

static unsigned char disparity_data[17];
static int pb9_disp_version2_depth;

static enum em_encoding current_vic_encoding = RGB;
static int current_vic_depth = 8;
static int current_framepacking;
static int current_vic;

/*  HDMI DDC-----------------------------------------------------------------*/
enum {
	FILTER_SCL = 0,
	FILTER_SDA,
	FILTER_VAL_MAX
};

static int filter_enable[FILTER_VAL_MAX] = {0, 0};
static int filter_val[FILTER_VAL_MAX] = {0, 0};

#if defined(CONFIG_LVDS_TEST)
/*---------------------------------------------------------------------------
 * HDMI LVDS
 *---------------------------------------------------------------------------
 */
#include <i2c.h>
#include <dm.h>

struct lvds_data_reg {
	unsigned char addr;
	unsigned char reg;
	unsigned char val;
};

struct lvds_data_reg lvds_data_reg[] = {
	/* 1920x720p@60 HDMI Input_LVDS Output Port A and B*/
	{0x98, 0xFF, 0x80}, /*  ;default 0x00, I2C reset */
	{0xFF, 0x00, 0x05}, /* wait 5ms */
	{0x98, 0xF4, 0x80}, /* ;default 0x00, CEC Map Address set to 0x80 */
	/* ;default 0x00, Infoframe Map Address set to 0x7C */
	{0x98, 0xF5, 0x7C},
	{0x98, 0xF8, 0x4C}, /* ;default 0x00, DPLL Map Address set to 0x4C */
	{0x98, 0xF9, 0x64}, /* ;default 0x00, KSV Map Address set to 0x64 */
	{0x98, 0xFA, 0x6C}, /* ;default 0x00, EDID Map Address set to 0x6C */
	{0x98, 0xFB, 0x68}, /* ;default 0x00, HDMI Map Address set to 0x68 */
	{0x98, 0xFD, 0x44}, /* ;default 0x00, CP Map Address set to 0x44 */
	{0x98, 0xE9, 0xC0}, /* ;default 0x00, LVDS Map Address set to 0xC0 */
	/*
	 * ;default 0x08, [5:0] - VID_STD[5:0] = 6'b010010 -
	 * WXGA 1360x768p@60Hz
	 */
	{0x98, 0x00, 0x12},
	/* ;default 0x06, [3:0] - Prim_Mode[3:0] = 4'b0110 - HDMI-Gr */
	{0x98, 0x01, 0x06},
	/*
	 * ;default 0xF0, [7:4] - INP_COLOR_SPACE[3:0] = 4'b1111 - color space
	 * determined by HDMI block, [1] RGB_OUT - 1'b1 - RGB color space output
	 */
	{0x98, 0x02, 0xF2},
	/*
	 * ;default 0x00, [7:0] - OP_FORMAT_SEL[7:0] = 8'b01000010 - 36-bit
	 * 4:4:4 SDR Mode 0
	 */
	{0x98, 0x03, 0x42},
	{0x98, 0x04, 0x63}, /* ;enable CP timing adjustment */
	/*
	 * ;default 0x2C  [2] - AVCODE_INSERT_EN = 1'b0 - added 0x20 disable
	 * data blanking for script reload
	 */
	{0x98, 0x05, 0x20},
	/* ;default 0x62, [5] - POWER_DOWN = 1'b0 - Powers up part  */
	{0x98, 0x0C, 0x42},
	/*
	 * ;default 0xBE, [4] - TRI_AUDIO = 1'b0 = untristate Audio ,
	 * {3] - TRI_LLC = 1'b1 = tristate LLC, Bit{1] - TRI_PIX = 1'b1 =
	 * Tristate Pixel Pads
	 */
	{0x98, 0x15, 0xAE},
	{0x44, 0x6C, 0x00}, /* ;default 0x10, ADI Recommended write */
	{0x44, 0x8B, 0x40}, /* ;HS DE adjustment */
	{0x44, 0x8C, 0x02}, /* ;HS DE adjustment */
	{0x44, 0x8D, 0x02}, /* ;HS DE adjustment */
	/* ;default 0x83, BCAPS[7:0] - Disable HDCP 1.1 features */
	{0x64, 0x40, 0x81},
	{0x68, 0x03, 0x98}, /* ;default 0x18, ADI Recommended write */
	{0x68, 0x10, 0xA5}, /* ;default 0x25, ADI Recommended write */
	{0x68, 0x1B, 0x08}, /* ;default 0x18, ADI Recommended write */
	{0x68, 0x45, 0x04}, /* ;default 0x00, ADI Recommended write */
	{0x68, 0x97, 0xC0}, /* ;default 0x80, ADI Recommended write */
	{0x68, 0x3D, 0x10}, /* ;default 0x00, ADI Recommended write */
	{0x68, 0x3E, 0x7B}, /* ;default 0x79, ADI recommended write */
	{0x68, 0x3F, 0x5E}, /* ;default 0x63, ADI Recommended Write */
	{0x68, 0x4E, 0xFE}, /* ;default 0x7B, ADI recommended write */
	{0x68, 0x4F, 0x08}, /* ;default 0x63, ADI recommended write */
	{0x68, 0x57, 0xA3}, /* ;default 0x30, ADI recommended write */
	{0x68, 0x58, 0x07}, /* ;default 0x01, ADI recommended write */
	{0x68, 0x6F, 0x08}, /* ;default 0x00, ADI Recommended write */
	{0x68, 0x83, 0xFE}, /* ;default 0xFF, ADI recommended write */
	{0x68, 0x86, 0x9B}, /* ;default 0x00, ADI recommended write */
	{0x68, 0x85, 0x10}, /* ;default 0x16, ADI recommended write */
	{0x68, 0x89, 0x01}, /* ;default 0x00, ADI recommended write */
	{0x68, 0x9B, 0x03}, /* ;default 0x0B, ADI Recommended write */
	{0x68, 0x9C, 0x80}, /* ;default 0x08, ADI Recommended write */
	{0x68, 0x9C, 0xC0}, /* ;default 0x08, ADI Recommended write */
	{0x68, 0x9C, 0x00}, /* ;default 0x08, ADI Recommended write */
	/*
	 * ;default 0x02, Bit [0] tx_mode_itu656 - 1'b0 = OLDI mode, Bit [1]
	 * tx_pdn - 1'b0 = LVDS TX powered on, Bit [3] tx_pll_en - 1'b1 =  power
	 *  up LVDS PLL
	 */
	{0xC0, 0x40, 0x08},
	{0xC0, 0x43, 0x03}, /* ;default 0x00, ADI Recommended write */
	{0xC0, 0x44, 0x00}, /* ;default 0x00, PLL GEAR < 200MHz */
	{0xC0, 0x45, 0x04}, /* ;default 0x1E, ADI Recommended write */
	{0xC0, 0x46, 0x53}, /* ;default 0x77, ADI Recommended write */
	{0xC0, 0x47, 0x03}, /* ;default 0x02, ADI Recommended write */
	/*
	 * ;default 0x71, Bit [6] tx_oldi_hs_pol - 1'b0 = HS Polarity Neg,
	 * Bit [5] tx_oldi_vs_pol - 1'b0 = VS Polarity Neg, Bit [4]
	 * tx_oldi_de_pol - 1'b1 =  DE Polarity Pos, Bit [3]
	 * tx_enable_ns_mapping - 1'b0 = normal oldi 8-bit mapping,
	 * Bit [2] tx_656_all_lanes_enable - 1'b0 = disable 656 data on lanes,
	 * Bit [1] tx_oldi_balanced_mode - 1'b0 = Non DC balanced,
	 * Bit [0] tx_color_mode - 1'b1 = 8-bit mode
	 */
	{0xC0, 0x4C, 0x19},
	/*
	 * ;default 0x08, Bit [3:2] tx_color_depth[1:0] - 2'b01 = 8-bit
	 *  Mode,Bit [1] tx_int_res - 1'b0 = Res bit 0,
	 * Bit [0] tx_mux_int_res - 1'b0 = disable programming of res bit
	 */
	{0xC0, 0x4E, 0x04},
	{0x00, 0x00, 0x00}
};

static void print_lvds_help(void)
{
	pr_force(
		"hdmi lvds [enable/disable]\r\n"
		"	 enable : enable hdmi to lvds mode\r\n"
		"	 disable : disable hdmi to lvds mode\r\n");
}

static void dump_lvds_reg(void)
{
	int loop;
	unsigned char data;

	i2c_set_bus_num(0); // SERDES_I2C_SCL is connected I2C_CH0_PORT

	pr_force("dump\r\n");
	for (loop = 0; !(
		lvds_data_reg[loop].addr == 0 &&
		lvds_data_reg[loop].reg == 0 &&
		lvds_data_reg[loop].val == 0); loop++) {
		if (lvds_data_reg[loop].addr == 0xFF) {
			continue;
		} else {
			i2c_read(
				lvds_data_reg[loop].addr >> 1,
				lvds_data_reg[loop].reg, 1, &data, 1);
			pr_force(
				"[%02d] A_0x%02x R_0x%02x 0x%02x :0x%02x\r\n",
				loop, lvds_data_reg[loop].addr,
				lvds_data_reg[loop].reg,
				lvds_data_reg[loop].val, data);
		}
	}
}

static void set_lvds_module(void)
{
	int loop;
	unsigned char data;

	pr_force("enable\r\n");
	i2c_set_bus_num(0); // SERDES_I2C_SCL is connected I2C_CH0_PORT

	for (
		loop = 0; !(lvds_data_reg[loop].addr == 0 &&
		lvds_data_reg[loop].reg == 0 &&
		lvds_data_reg[loop].val == 0) ; loop++) {
		if (lvds_data_reg[loop].addr == 0xFF) {
			mdelay(lvds_data_reg[loop].val);
		} else {
			if (i2c_write(
				lvds_data_reg[loop].addr >> 1,
				lvds_data_reg[loop].reg, 1,
				&lvds_data_reg[loop].val, 1))
				pr_err("%d error\r\n", loop);
			mdelay(1);
			i2c_read(
				lvds_data_reg[loop].addr >> 1,
				lvds_data_reg[loop].reg, 1, &data, 1);
			if (lvds_data_reg[loop].val != data)
				pr_force(
				"[%02d] A_0x%02x R_0x%02x 0x%02x : 0x%02x\r\n",
				loop, lvds_data_reg[loop].addr,
				lvds_data_reg[loop].reg,
				lvds_data_reg[loop].val, data);
		}
	}
}

static void set_lvds_gpio(void)
{
	unsigned int bit = 0x04000000;

	// LVDS Test
	writel(readl(0x1B9356C4) | bit, 0x1B9356C4);
	gpio_config(GPIO_LCD_RESET, GPIO_OUTPUT);
	gpio_config(GPIO_LCD_ON, GPIO_OUTPUT);

	// Disable Backlight.
	writel(readl(0x1B9356C0) & (~bit), 0x1B9356C0);
	gpio_set(GPIO_LCD_RESET, 0);
	gpio_set(GPIO_LCD_ON, 0);

	gpio_set(GPIO_LCD_ON, 1);
	udelay(20);

	gpio_set(GPIO_LCD_RESET, 1);

	// Enable Backlight
	writel(readl(0x1B9356C0) | bit, 0x1B9356C0);
}

static int
do_hdmi_lvds(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc < 2 || argc > 3) {
		print_lvds_help();
		return 0;
	}

	if (/* help */
		!strcmp(argv[1], "help") && argc == 2) {
		print_lvds_help();
	} else if (
		!strcmp(argv[1], "enable") && argc == 2) {
		set_lvds_gpio();
		set_lvds_module();
	} else if (
		!strcmp(argv[1], "dump") && argc == 2) {
		dump_lvds_reg();
	}

	return 0;
}
#endif

/*---------------------------------------------------------------------------
 * HDMI VIC
 *---------------------------------------------------------------------------
 */
static int
do_hdmi_hfvsif(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int val;
	int pb_index;
	int loop;
	struct product_params productParams;

	memset(&productParams, 0, sizeof(productParams));

	if (argc < 2 || argc > 3) {
		pr_err("%s argc=%d\r\n", __func__, argc);
		return CMD_RET_USAGE;
	}

	/* default setting */
	productParams.mVendorPayload[HF_PB4] = 0x1; // version 1
	productParams.mOUI = 0x01D85DC4;

	if (/* 3D Valid */
		!strcmp(argv[1], "3d_valid") && argc == 3) {
		val = simple_strtoul(argv[2], NULL, 10);
		hdmi_3d_valid = val ? 1 : 0;
	} else if (/* 3D F_Structure */
		!strcmp(argv[1], "f_structure") && argc == 3) {
		val = simple_strtoul(argv[2], NULL, 10);
		pb6_fstructure = val;
	} else if (!strcmp(argv[1], "f_extdata") && argc == 3) {
		val = simple_strtoul(argv[2], NULL, 10);
		pb7_ext_data = val;
	} else if (!strcmp(argv[1], "disparity_present") && argc == 3) {
		val = simple_strtoul(argv[2], NULL, 10);
		pb6_disp_present = val ? 1 : 0;
		if (pb8_dualview && pb6_disp_present) {
			pb6_disp_present = 0;
			pr_force("Dualview - disparity_presentis disabled\r\n");
		}
	} else if (!strcmp(argv[1], "disparity_version") && argc == 3) {
		val = simple_strtoul(argv[2], NULL, 10);
		pb9_disp_version = val;
	} else if (!strcmp(argv[1], "dualview") && argc == 3) {
		val = simple_strtoul(argv[2], NULL, 10);
		pb8_dualview = val ? 1 : 0;
		if (pb8_dualview && pb6_disp_present) {
			pb8_dualview = 0;
			pr_force(
				"Disparity_present - Dualview is disabled\r\n");
		}
	} else if (!strcmp(argv[1], "viewdependcy") && argc == 3) {
		val = simple_strtoul(argv[2], NULL, 10);
		pb8_viewdependcy = val;
		if (pb8_dualview) {
			pr_force("DualView-viewdependcy is disabled\r\n");
			pb8_viewdependcy = 0;
		}
	} else if (!strcmp(argv[1], "preferredview") && argc == 3) {
		val = simple_strtoul(argv[2], NULL, 10);
		pb8_preferred = val;
		if (
			pb8_dualview && !(
				pb8_viewdependcy == 0 &&
				pb8_viewdependcy == 3)) {
			pr_force(
				"dualview is enabled preferred is set to 0\r\n");
			pb8_preferred = 0;
		}

	} else if (!strcmp(argv[1], "meta_present") && argc == 3) {
		val = simple_strtoul(argv[2], NULL, 10);
		pb6_meta_preset = val;
	}

	/* PB6 */
	pb_index = HF_PB5;
	productParams.mVendorPayload[pb_index++] = hdmi_3d_valid;
	productParams.mVendorPayloadLength = 5;
	if (hdmi_3d_valid) {
		if (pb8_dualview || pb8_viewdependcy || pb8_preferred)
			pb6_add_info_present = 1;
		else
			pb6_add_info_present = 0;
		productParams.mVendorPayload[pb_index++] =
			((pb6_fstructure & 0xF) << 4) |
			(pb6_add_info_present << 3) | (pb6_disp_present << 2) |
			(pb6_meta_preset << 1);
		productParams.mVendorPayloadLength++;

		/* PB7 3D_Valid and 3D_FStructure == 0b1000 ... 0b1111 */
		if (pb6_fstructure > 7) {
			productParams.mVendorPayload[pb_index++] =
						(pb7_ext_data & 0xF) << 4;
			productParams.mVendorPayloadLength++;
		}

		/* PB8 3D_Additionalinfo_present is set (1) */
		if (pb6_add_info_present) {
			productParams.mVendorPayload[pb_index++] =
			(pb8_dualview << 4) | ((pb8_viewdependcy & 0x3) << 2) |
			((pb8_preferred & 0x3) << 0);
			productParams.mVendorPayloadLength++;
		}
		/* PB9 ~*/
		if (pb6_disp_present) {
			int disparity_length;

			switch (pb9_disp_version) {
			case 0:
			default:
				disparity_length = 0;
				break;
			case 1:
				disparity_length = 3;
				break;
			case 2:
				switch (++pb9_disp_version2_depth) {
				case 1:
					disparity_data[0] = 0;
					break;
				case 2:
					disparity_data[0] = 2;
					break;
				case 3:
					disparity_data[0] = 3;
					break;
				case 4:
					disparity_data[0] = 4;
					break;
				case 5:
					disparity_data[0] = 5;
					break;
				case 6:
					disparity_data[0] = 10;
					break;
				case 7:
				default:
					disparity_data[0] = 17;
				pb9_disp_version2_depth = 0;
					break;
				}
				disparity_length = disparity_data[0];
				if (disparity_length == 0)
					disparity_length = 1;
				break;
			case 3:
				switch (++pb9_disp_version2_depth) {
				case 1:
					disparity_data[0] = 0;
					break;
				case 2:
					disparity_data[0] = 2;
					break;
				case 3:
					disparity_data[0] = 3;
					break;
				case 4:
					disparity_data[0] = 4;
					break;
				case 5:
					disparity_data[0] = 5;
					break;
				case 6:
					disparity_data[0] = 10;
					break;
				case 7:
				default:
					disparity_data[0] = 17;
				pb9_disp_version2_depth = 0;
					break;
				}
				disparity_length = disparity_data[0];
				if (disparity_length == 0)
					disparity_length = 1;
				disparity_length += 4;
				break;
		}

			productParams.mVendorPayload[pb_index++]
				= (pb9_disp_version << 5) | disparity_length;
			productParams.mVendorPayloadLength++;
			productParams.mVendorPayload[pb_index++] =
							disparity_data[0];
			productParams.mVendorPayloadLength++;

			for (loop = 1; loop < disparity_length; loop++) {
				productParams.mVendorPayload[pb_index++] = loop;
				productParams.mVendorPayloadLength++;
			}
		}

		if (pb6_meta_preset) {
			/* meta type is 0, length is 8 */
			productParams.mVendorPayload[pb_index++] =
							(0 << 5) | (8 << 0);
			productParams.mVendorPayloadLength++;
			for (loop = 0; loop < 8; loop++) {
				productParams.mVendorPayload[pb_index++] = loop;
				productParams.mVendorPayloadLength++;
			}
		}
	}
	packets_VendorSpecificInfoFrame(
		productParams.mOUI,
		productParams.mVendorPayload,
		productParams.mVendorPayloadLength, 1);
	return 0;
}

/*---------------------------------------------------------------------------
 * HDMI VIC
 *---------------------------------------------------------------------------
 */
static void print_vic_help(void)
{
	pr_force(
	"hdmi vic <n> - HDMI outputs in resolution corresponding to vic\r\n"
	"hdmi vic depth <n> Set the depth of hdmi output.\r\n"
	"	 n -  8: 24-bit\r\n"
	"	     10: 32-bit\r\n"
	"	     12: 36-bit\r\n");
	pr_force(
	"hdmi vic encoding <value> Set the encoding of hdmi output.\r\n"
	"	 value - rgb : rgb encoding\r\n"
	"		 444 : ycbcr444 encoding\r\n"
	"		 422 : ycbcr422 encoding\r\n"
	"		 420 : ycbcr420 encoding, \r\n"
	"		       If it is not 4k resolution, it will be \r\n"
	"		       set to ycbcr444.\r\n");
	pr_force(
	"hdmi vic framepacking <n> Set HDMI 3D FramePacking.\r\n"
	"	 n - 1: Enable Frame Packing\r\n"
	"	     0: Disable Frame Packing\r\n");
}

static int
do_hdmi_vic(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int vic;
	int vic_tmp;
	enum em_encoding encoding_tmp = ENC_UNDEFINED;

	if (argc < 2 || argc > 3) {
		print_vic_help();
		return 0;
	}
	if (!strcmp(argv[1], "help") && argc == 2) {
		print_vic_help();
	} else if (!strcmp(argv[1], "depth") && argc == 3) {
		vic_tmp = simple_strtoul(argv[2], NULL, 10);
		switch (vic_tmp) {
		case 8:
		case 10:
		case 12:
			current_vic_depth = vic_tmp;
			break;
		default:
			return CMD_RET_USAGE;
		}
	} else if (!strcmp(argv[1], "encoding") && argc == 3) {
		if (!strcmp(argv[2], "rgb")) {
			encoding_tmp = RGB;
		} else if (!strcmp(argv[2], "444")) {
			encoding_tmp = YCC444;
		} else if (!strcmp(argv[2], "422")) {
			encoding_tmp = YCC422;
		} else if (!strcmp(argv[2], "420")) {
			if (
				current_vic == 96 || current_vic == 97 ||
				current_vic == 101 || current_vic == 102)
				encoding_tmp = YCC420;
			else
				encoding_tmp = YCC444;
		}
		if (encoding_tmp != ENC_UNDEFINED)
			current_vic_encoding = encoding_tmp;
		else
			return CMD_RET_USAGE;
	} else if (!strcmp(argv[1], "framepacking") && argc == 3) {
		vic_tmp = simple_strtoul(argv[2], NULL, 10);
		current_framepacking = (vic_tmp > 0) ? 1 : 0;
	} else {
		vic = simple_strtoul(argv[1], NULL, 10);
		pr_force("vic = %d\r\n", vic);
		if ((vic > 0 && vic < 105) || vic >= 1024)
			current_vic = vic;
		else
			return CMD_RET_USAGE;
	}
	if (current_vic > 0) {
		hdmi_update_panel_info(
			current_vic, current_vic_depth,
			current_vic_encoding, current_framepacking);
		drv_lcd_init();
	}
	return 0;
}

/*---------------------------------------------------------------------------
 * HDMI DDC
 *---------------------------------------------------------------------------
 */

static void print_ddc_help(void)
{
	pr_force(
	"hdmi ddc status - Show ddc filter status.\r\n"
	"hdmi ddc operation <n> - Test ddc operation.\r\n"
	"	 n -1: Read edid from sink device. \r\n");
	pr_force(
		"	    2: Read sink version from sink device using scdc. \r\n"
		"	    3: Read source version from sink device using scdc. \r\n"
		"	    4: Write source version to sink device using scdc. \r\n"
		"	    5: Read Character Error Detction from sink device using scdc. \r\n");
	pr_force(

		"hdmi ddc filter <n> [e|d] - Enable/Disable I2C Filter.\r\n"
		"	 n -0: SCL input filter.\r\n"
		"	    1: SDA input filter.\r\n"
		"	 e|d -e: Enable I2C Filter.\r\n"
		"	      d: Disable I2C Filter.\r\n");
	pr_force(
		"hdmi ddc filter <n> <value> - Set I2C Filter Value.\r\n"
		"	 n -0: SCL input filter.\r\n"
		"	    1: SDA input filter.\r\n");
	pr_force(
		"	 values -0 to 15.\r\n"
		"		 0:  Skip 4 times\r\n"
		"		 1:  Skip 8 times\r\n"
		"		 .\r\n"
		"		 15: Skip 64 times\r\n");
}

static void print_ddc_status(void)
{
	pr_force("----------------------\r\n");
	pr_force("Filter Status \r\n");
	pr_force("----------------------\r\n");
	pr_force("       Enable Value\r\n");
	pr_force("----------------------\r\n");
	pr_force(
		" SCL : %d     %d step\r\n",
		filter_enable[FILTER_SCL], filter_val[FILTER_SCL] * 4 + 4);
	pr_force(
		" SDA : %d     %d step\r\n",
		filter_enable[FILTER_SDA], filter_val[FILTER_SDA] * 4 + 4);
	pr_force("----------------------\r\n");
}

static void read_edid_verification(void)
{
	int hdmi_mode, vic;
	enum em_encoding encoding;

	edid_set_print_enable(1);
	edid_get_optimal_settings(&hdmi_mode, &vic, &encoding);
	edid_set_print_enable(0);
}

static int run_hdmi_ddc_operation(int operation_case)
{
	int ret = 0;
	unsigned int version;

	#if defined(HDMI_I2C_FILTER)
	dwc_hdmi_set_i2c_filter(
		FILTER_SCL,
		filter_enable[FILTER_SCL], filter_val[FILTER_SCL]);
	dwc_hdmi_set_i2c_filter(
		FILTER_SDA,
		filter_enable[FILTER_SDA], filter_val[FILTER_SDA]);
	#endif

	switch (operation_case) {
	case 1:
		// Read sink version from sink device using scdc.
		read_edid_verification();
		break;
	case 2:
		// Read sink version from sink device using scdc.
		scdc_read_sink_version(&version);
		pr_force("\r\n");
		pr_force("SINK SCDC Version = %d\r\n", version);
		pr_force("\r\n");
		break;
	case 3:
		// Read source version from sink device using scdc
		scdc_read_source_version(&version);
		pr_force("\r\n");
		pr_force("SOURCE SCDC Version = %d\r\n", version);
		pr_force("\r\n");
		break;
	case 4:
		// Write source version to sink device using scdc.
		scdc_read_source_version(&version);
		version = (version < 15) ? (version + 1) : 0;
		scdc_write_source_version(version);
		version = 0;
		scdc_read_source_version(&version);
		pr_force("\r\n");
		pr_force("SOURCE SCDC Version = %d\r\n", version);
		pr_force("\r\n");
		break;
	case 5:
		// Write source version to sink device using scdc.
		scdc_error_detection();
		break;

	default:
		ret = -1;
		break;
	}
	return ret;
}

static int
do_hdmi_ddc(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ddc_tmp;

	if (argc < 2 || argc > 4) {
		print_ddc_help();
		return 0;
	}

	if (/* help */
		!strcmp(argv[1], "help") && argc == 2) {
		print_ddc_help();
	} else if (/* status */
		!strcmp(argv[1], "status") && argc == 2) {
		print_ddc_status();
	} else if (!strcmp(argv[1], "operation") && argc == 3) {
		ddc_tmp = simple_strtoul(argv[2], NULL, 10);
		if (run_hdmi_ddc_operation(ddc_tmp) < 0)
			return CMD_RET_USAGE;
	} else if (!strcmp(argv[1], "filter") && argc == 4) {
		ddc_tmp = simple_strtoul(argv[2], NULL, 10);
		if (!strcmp(argv[3], "e")) {
			pr_force(
				"%s enable i2c filter\r\n",
				(ddc_tmp == 0) ? "SCL" : "SDA");
			filter_enable[(ddc_tmp == 0) ? FILTER_SCL :
						FILTER_SDA] = 1;
		} else if (!strcmp(argv[3], "d")) {
			pr_force(
				"%s disable i2c filter\r\n",
				(ddc_tmp == 0) ? "SCL" : "SDA");
			filter_val[(ddc_tmp == 0) ? FILTER_SCL :
						FILTER_SDA] = 0;
		} else {
			int val = simple_strtoul(argv[3], NULL, 10);

			pr_force(
				"%s i2c filter value is %d\r\n",
				(ddc_tmp == 0) ? "SCL" : "SDA", val);
			filter_val[(ddc_tmp == 0) ? FILTER_SCL :
						FILTER_SDA] = val;
		}
	}
	return 0;
}

/*---------------------------------------------------------------------------
 * HDMI PHY
 *---------------------------------------------------------------------------
 */
static int phy_depth = 8;

struct v_phy_closk {
	unsigned int real_clock;
	unsigned int dtd_clock;
};

static void print_phy_help(void)
{
	pr_force(
	"phy list - Show list of pixel clocks supported by the hdmi phy.\r\n"
	"hdmi phy enable <n> - Enable the hdmi phy with nth pixel clock.\r\n"
	"hdmi phy depth <n> - Set the depth of hdmi phy. \r\n"
	"	 n -  8: 24-bit\r\n"
	"	     10: 32-bit\r\n"
	"	     12: 36-bit\r\n");
}

static struct v_phy_closk v_phy_closk[] = {
	{ 25175000,  25175000},
	{ 25200000,  25200000},
	{ 27000000,  27000000},
	{ 27027000,  27027000},
	{ 44100000,  44100000},
	{ 50350000,  50350000},
	{ 54000000,  54000000},
	{ 54054000,  54054000},
	{ 59340659,  59340000},
	{ 59400000,  59400000},
	{ 72000000,  72000000},
	{ 74175824,  74175000},
	{ 74250000,  74250000},
	{ 82500000,  82500000},
	{ 88200000,  88200000},
	{ 90000000,  90000000},
	{ 98901098,  98901000},
	{ 99000000,  99000000},
	{108000000, 108000000},
	{108108000, 108108000},
	{118681318, 118681000},
	{118800000, 118800000},
	{144000000, 144000000},
	{148351648, 148350000},
	{148500000, 148500000},
	{165000000, 165000000},
	{180000000, 180000000},
	{185625000, 185625000},
	{197802197, 197802000},
	{198000000, 198000000},
	{216000000, 216000000},
	{216216000, 216216000},
	{237362637, 237362000},
	{237600000, 237600000},
	{296703296, 296700000},
	{297000000, 297000000},
	{330000000, 330000000},
	{395604395, 395604000},
	{396000000, 396000000},
	{371250000, 371250000},
	{494505494, 494505000},
	{495000000, 495000000},
	{593406593, 593400000},
	{594000000, 594000000},
	{0, 0}
};

static int print_phy_list(int phy_dept)
{
	int index;
	int items = 0;

	pr_force("\r\n");
	pr_force("======================\r\n");
	pr_force("PHY List \r\n");
	pr_force("----------------------\r\n");
	for (index = 0; ; index++) {
		if (v_phy_closk[index].real_clock > 0) {
			if (
				phy_dept > 8 &&
				v_phy_closk[index].real_clock >= 494505494) {
				break;
			}
			items++;
		} else {
			break;
		}
		pr_force(
			"[%d]: %dHz\r\n", index + 1,
			v_phy_closk[index].real_clock);
	}
	pr_force("----------------------\r\n");

	return items;
}

static int run_hdmi_phy_config(int phy_depth, unsigned int select_index)
{
	int success = -1;
	unsigned int pixel_clock;
	unsigned int tmds_clock;
	unsigned int real_tmds_clock;

	volatile void __iomem *ddibus_io =
			(volatile void __iomem *)TCC_DBUSCFG_BASE;

	pr_force("%s index =%d\r\n", __func__, select_index + 1);
	pixel_clock = v_phy_closk[select_index].dtd_clock;
	tmds_clock = hdmi_phy_get_actual_tmds_bit_ratio(pixel_clock, phy_depth);
	real_tmds_clock = hdmi_phy_get_actual_tmds_bit_ratio(
			v_phy_closk[select_index].real_clock, phy_depth);
	success = tcc_hdmi_phy_config(
		ddibus_io, pixel_clock, tmds_clock, phy_depth, 1, 0);
	if (success < 0)
		pr_force(
			"Failed.. Real TMDS CLock is %dHz\r\n",
			real_tmds_clock);
	else
		pr_force("Real TMDS CLock is %dHz\r\n", real_tmds_clock);
	hdmi_api_wait_phylock();

	return success;
}

static int do_hdmi_phy(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int phy_tmp;

	if (argc < 2 || argc > 3) {
		print_phy_help();
		return 0;
	}

	if (!strcmp(argv[1], "help") && argc == 2) {
		print_phy_help();
	} else if (!strcmp(argv[1], "list") && argc == 2) {
		print_phy_list(8);
	} else if (!strcmp(argv[1], "enable") && argc == 3) {
		phy_tmp = simple_strtoul(argv[2], NULL, 10);
		if (phy_tmp > 0)
			run_hdmi_phy_config(phy_depth, phy_tmp - 1);
	} else if (!strcmp(argv[1], "depth") && argc == 3) {
		phy_tmp = simple_strtoul(argv[2], NULL, 10);
		if (phy_tmp > 0) {
			phy_depth = phy_tmp;
			pr_force("Set phy depth to %d\r\n\r\n", phy_depth);
		}
	}
	return 0;
}

static void print_hpd_help(void)
{
	pr_force(
	"hdmi hpd link - Get HPD status using HDMI Link\r\n"
	"hdmi hpd force <n> - Force Sets HPD of HDMI Link \r\n"
	"		n - 0 : plug-out\r\n"
	"		    1 : plug-in\r\n"
	"hdmi hpd gpio - Get HPD status using GPIO \r\n");
}

static int do_hdmi_hpd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int hpd_status = 0;

	if (argc < 2 && argc > 3) {
		print_hpd_help();
		return 0;
	}

	if (!strcmp(argv[1], "help") && argc == 2) {
		print_hpd_help();
	} else if (!strcmp(argv[1], "link") && argc == 2) {
		hdmi_phy_enable_hpd_sense();
		hpd_status = hdmi_phy_hot_plug_detected();
		hdmi_phy_disable_hpd_sense();
	} else if (!strcmp(argv[1], "force") && argc == 3) {
		hpd_status = simple_strtoul(argv[2], NULL, 10);
		dwc_hdmi_force_set_hpd(hpd_status);
	} else if (!strcmp(argv[1], "gpio") && argc == 2) {
		hpd_status = dwc_hdmi_get_hpd();
	}

	pr_force("HPD = %d\r\n", hpd_status);
	return 0;
}

static void print_ref_help(void)
{
	enum em_hdmi_ref hdmi_ref = dwc_hdmi_get_ref_clock();

	pr_force(
	"HDMI refeprece clock source\r\n");
	pr_force(
	"hdmi ref <n> - Set HDMI reference clock source\r\n"
	"		n - 0 : pad xin\r\n"
	"		    1 : hdmi x-tal\r\n"
	"		    2 : peri clock (24MHz)\r\n"
	"		    3 : pmu xin\r\n");
	pr_force(
	"Current reference clock is %s\r\n",
		(hdmi_ref == EM_HDMI_REF_PAD_XIN) ? "pad xin" :
		(hdmi_ref == EM_HDMI_REF_PAD_HDMI_XTAL) ? "hdmi x-tal" :
		(hdmi_ref == EM_HDMI_REF_PAD_PERI_CLK) ? "peri clock" :
		"pmu xin");
}

static int do_hdmi_ref(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	enum em_hdmi_ref hdmi_ref;

	if (argc != 2) {
		print_ref_help();
		return 0;
	}

	if (!strcmp(argv[1], "help") && argc == 2) {
		print_ref_help();
		while (1) {
			int val = 0;

			val = readl(0x12380010);
			if (!(val & 1 << 24))
				pr_force("PHY_MON L\r\n");
		}
	} else {
		hdmi_ref = simple_strtoul(argv[1], NULL, 10);
		if (hdmi_ref < EM_HDMI_REF_PAD_XIN)
			hdmi_ref = EM_HDMI_REF_PAD_XIN;
		if (hdmi_ref > EM_HDMI_REF_PAD_PERI_CLK)
			hdmi_ref = EM_HDMI_REF_PAD_PERI_CLK;
		dwc_hdmi_set_ref_clock(hdmi_ref);

		pr_force(
		"Set reference clock to %s\r\n",
			(hdmi_ref == EM_HDMI_REF_PAD_XIN) ? "pad xin" :
			(hdmi_ref == EM_HDMI_REF_PAD_HDMI_XTAL) ? "hdmi x-tal" :
			(hdmi_ref == EM_HDMI_REF_PAD_PERI_CLK) ? "peri clock" :
			"pmu xin");
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

static cmd_tbl_t cmd_hdmi_sub[] = {
	U_BOOT_CMD_MKENT(ref, 3, 0, do_hdmi_ref, "", ""),
	U_BOOT_CMD_MKENT(hpd, 3, 0, do_hdmi_hpd, "", ""),
	U_BOOT_CMD_MKENT(phy, 3, 0, do_hdmi_phy, "", ""),
	U_BOOT_CMD_MKENT(ddc, 4, 0, do_hdmi_ddc, "", ""),
	U_BOOT_CMD_MKENT(vic, 3, 0, do_hdmi_vic, "", ""),
	U_BOOT_CMD_MKENT(hfvsif, 3, 0, do_hdmi_hfvsif, "", ""),
	#if defined(CONFIG_LVDS_TEST)
	U_BOOT_CMD_MKENT(lvds, 3, 0, do_hdmi_lvds, "", ""),
	#endif
};

static int do_hdmi(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_hdmi_sub[0], ARRAY_SIZE(cmd_hdmi_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

static char hdmi_help_text[] =
	"---------------------------------------------------------------\r\n"
	"hdmi ddc\r\n"
	"hdmi hpd\r\n"
	"hdmi lvds\r\n"
	"hdmi phy\r\n"
	"hdmi ref\r\n"
	"hdmi vic\r\n"
	"---------------------------------------------------------------\r\n";

U_BOOT_CMD(
	hdmi, 7, 1, do_hdmi,
	"Verify hdmi link and phy.",
	hdmi_help_text
);
