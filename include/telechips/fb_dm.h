/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_FB_DM_H
#define TCC_FB_DM_H

#include <fdtdec.h>

#include <asm/arch/vioc/vioc_global.h>
#include <dt-bindings/display/telechips-common-fb_dm_display.h>
#ifdef CONFIG_TCC897X
#include <dt-bindings/display/tcc897x-fb_dm_display.h>
#define FB_BUF_CNT_MAX 2U
#elif CONFIG_TCC803X
#include <dt-bindings/display/tcc803x-fb_dm_display.h>
#define FB_BUF_CNT_MAX 3U
#elif CONFIG_TCC805X
#include <dt-bindings/display/tcc805x-fb_dm_display.h>
#define FB_BUF_CNT_MAX 4U
#elif CONFIG_TCC750X
#include <dt-bindings/display/tcc750x-fb_dm_display.h>
#define FB_BUF_CNT_MAX 1U
#elif CONFIG_TCC807X
#include <dt-bindings/display/tcc807x-fb_dm_display.h>
#define FB_BUF_CNT_MAX 5U
#endif

#define INVALID_DP_VIC         0xFFFFFFFFU
#define GPIO_EXT1                      0xFFFF0001U
#define MAX_RES_WIDTH 10000 //for coverity
#define MAX_RES_HEIGHT 10000 //for coverity
#define MAX_BYTES_LEN 4
#define MAX_LINE_LENGTH ((MAX_RES_WIDTH) * (MAX_BYTES_LEN))
#define FB_DM_DEFAULT_BOOTLOGO_NAME "bootlogo"


enum {
	DISPLAY_CONTROLLER_0	= 0,
	DISPLAY_CONTROLLER_1	= 1,
	DISPLAY_CONTROLLER_2	= 2,
	DISPLAY_CONTROLLER_3	= 3,
	DISPLAY_CONTROLLER_MAX		= 4
};

enum {
	PANEL_DP0 = 0,
	PANEL_DP1,
	PANEL_DP2,
	PANEL_DP3,
	PANEL_DP_MAX,
	PANEL_LVDS_DUAL,
	PANEL_LVDS_SINGLE,
	PANEL_HDMI,
	PANEL_DSI,
	PANEL_DEVICE_MAX
};

enum {
#if defined (VIOC_DISP0)
	PANEL_PATH_DISP0 = 0,
#endif
#if defined (VIOC_DISP1)
	PANEL_PATH_DISP1,
#endif
#if defined (VIOC_DISP2)
	PANEL_PATH_DISP2,
#endif
#if defined (VIOC_DISP3)
	PANEL_PATH_DISP3,
#endif
#if defined (VIOC_DISP4)
	PANEL_PATH_DISP4,
#endif
#if defined (VIOC_DISP5)
	PANEL_PATH_DISP5,
#endif
	PANEL_PATH_MAX
};

enum {
	PANEL_LCD_MUX0 = 0,
#if defined (VIOC_DISP1)
	PANEL_LCD_MUX1,
#endif
#if defined (VIOC_DISP2)
	PANEL_LCD_MUX2,
#endif
#if defined (VIOC_DISP3)
	PANEL_LCD_MUX3,
#endif
#if defined (VIOC_DISP4)
	PANEL_LCD_MUX4,
#endif
	PANEL_LCD_MUX_MAX
};

enum {
	PANEL_LCD_POWER0 = 0,
	PANEL_LCD_POWER1,
	PANEL_LCD_POWER2,
	PANEL_LCD_POWER3,
	PANEL_LCD_NC,
	PANEL_LCD_POWER_MAX
};

enum {
	PANEL_LCD_PATH0 = 0,
	PANEL_LCD_PATH1,
	PANEL_LCD_PATH2,
	PANEL_LCD_PATH3,
	PANEL_LCD_PATH_MAX
};

//enum {
//	PANEL_ID_AT070TN93,
//	PANEL_ID_ED090NA,
//	PANEL_ID_FLD0800,
//	PANEL_ID_SLVDS_SAMPLE,
//	PANEL_ID_SLVDS_EXT,
//	PANEL_ID_HSD123JPW1,
//	PANEL_ID_HDMI,
//	PANEL_ID_LA070WV6,
//	PANEL_ID_TM123XDHP90,
//	PANEL_ID_HSD123KPW2_SERDES,
//	PANEL_ID_COMPOSITE,
//	PANEL_ID_COMPONENT,
//	PANEL_ID_DP0,
//	PANEL_ID_DP1,
//	PANEL_ID_DP2,
//	PANEL_ID_DP3,
//	PANEL_ID_DP_MAX
//};
//
enum TCC_LCDC_IMG_FMT_TYPE {
	TCC_LCDC_IMG_FMT_1BPP,
	TCC_LCDC_IMG_FMT_2BPP,
	TCC_LCDC_IMG_FMT_4BPP,
	TCC_LCDC_IMG_FMT_8BPP,
	TCC_LCDC_IMG_FMT_RGB332 = 8,
	TCC_LCDC_IMG_FMT_RGB444 = 9,
	TCC_LCDC_IMG_FMT_RGB565 = 10,
	TCC_LCDC_IMG_FMT_RGB555 = 11,
	TCC_LCDC_IMG_FMT_RGB888 = 12,
	TCC_LCDC_IMG_FMT_RGB666 = 13,
	TCC_LCDC_IMG_FMT_RGB888_3       = 14,
	TCC_LCDC_IMG_FMT_ARGB6666_3 = 15,
	TCC_LCDC_IMG_FMT_COMP = 16,
	TCC_LCDC_IMG_FMT_DECOMP = (TCC_LCDC_IMG_FMT_COMP),
	TCC_LCDC_IMG_FMT_444SEP = 21,
	TCC_LCDC_IMG_FMT_UYVY = 22,
	TCC_LCDC_IMG_FMT_VYUY = 23,
	TCC_LCDC_IMG_FMT_YUV420SP = 24,
	TCC_LCDC_IMG_FMT_YUV422SP = 25,
	TCC_LCDC_IMG_FMT_YUYV = 26,
	TCC_LCDC_IMG_FMT_YVYU = 27,
	TCC_LCDC_IMG_FMT_YUV420ITL0 = 28,
	TCC_LCDC_IMG_FMT_YUV420ITL1 = 29,
	TCC_LCDC_IMG_FMT_YUV422ITL0 = 30,
	TCC_LCDC_IMG_FMT_YUV422ITL1 = 31,
	TCC_LCDC_IMG_FMT_MAX
};

enum tcc_fb_dm_status {
	TCC_FBDM_STAT_DISABLED = 0,
	TCC_FBDM_STAT_ENABLED,
	TCC_FBDM_STAT_FB_PARSE_ERR,
	TCC_FBDM_STAT_REMOTE_ERR,
	TCC_FBDM_STAT_FB_BUF_ERR,
	TCC_FBDM_STAT_FB_PERI_ERR,
	TCC_FBDM_STAT_MAX
};

struct DP_Init_Params {
	bool		bDP_PanelMode;
	bool		bDP_SDMBypass;
	bool		bDP_SRVCBypass;
	bool		bDP_PHY_LaneSwap;
	unsigned char	ucDP_Index;
	unsigned char	ucNumOfDPs;
	unsigned char	ucDP_I2CPort;
	unsigned int	uiVideoCode;
};

struct tcc_fb_dm_image_info {
	unsigned int lcd_layer;
	unsigned int enable;
	unsigned int frame_width;
	unsigned int frame_height;

	unsigned int image_width;
	unsigned int image_height;
	unsigned int offset_x;
	unsigned int offset_y;
	unsigned int buf_addr0;
	unsigned int buf_addr1;
	unsigned int buf_addr2;
	enum TCC_LCDC_IMG_FMT_TYPE fmt;
	unsigned int logo_base;
};

struct tcc_fb_dm_priv {
	unsigned int id;						/* fb id */
	struct display_timing pan_timing;		/* timing values */
	unsigned int interlaced;				/* progressive or interlaced */
	unsigned int pixel_repetition_input;
	unsigned int lcd_id;					/* DISP number */
	unsigned int clk_div;					/* clock division */
	unsigned int ovp;						/* overlay priority */
	struct tcc_fb_dm_image_info image_info;	/* image information for RDMA */
	unsigned int fb_draw_type;				/* bmp display or splash image */
	unsigned int fb_buf_id;					/* fb buffer id */
	unsigned long fb_buf_addr;
	unsigned int target_bpix;				/* bpix */
	struct udevice *output_media;			/* output media udevice */
	unsigned int media_id;					/* panel id(PANEL_XXX) */
	enum tcc_fb_dm_status fb_stat;			/* status of fb */
	uint32_t lcd_pxdw;                      /* immage format */
	const unsigned char *splash_img_name;            /* bmp image name in splash */
	unsigned int lcd_mux_id;                /* lcd mux id */
};

struct tcc_fb_dm_versions {
        /** @major: driver major number */
        int major;
        /** @minor: driver minor number */
        int minor;
        /** @patchlevel: driver patch level */
        int patchlevel;
        /** @name: driver name */
        const char *name;
        /** @date: driver date */
        const char *date;
};

struct tcc_fb_dm_global_info {
	unsigned int logo_flag[FB_BUF_CNT_MAX];
	struct tcc_fb_dm_versions tcc_fb_dm_version;
};

#endif // TCC_FB_DM_H
