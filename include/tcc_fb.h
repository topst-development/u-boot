/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_FB_H
#define TCC_FB_H

#ifdef MULTIPLE_BOOTLOGO
#define BOOTLOGO_WIDTH		640
#define BOOTLOGO_HEIGHT		480
#endif

#define MAX_BACKLIGHT       255
#define DEFAULT_BACKLIGHT   150

#define ID_INVERT 0x01U /* Invered Data Enable(ACBIS pin)  anctive Low */
#define IV_INVERT 0x02U /* Invered Vertical sync  anctive Low */
#define IH_INVERT 0x04U /* Invered Horizontal sync       anctive Low */
#define IP_INVERT 0x08U /* Invered Pixel Clock : anctive Low */

#define INVALID_DP_VIC		0xFFFFFFFFU
#define GPIO_EXT1			0xFFFF0001U

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
	PANEL_DEVICE_MAX
};

enum {
	PANEL_PATH_DISP0 = 0,
	PANEL_PATH_DISP1,
	PANEL_PATH_DISP2,
	PANEL_PATH_DISP3,
	PANEL_PATH_MAX
};

enum {
	PANEL_LCD_MUX0 = 0,
	PANEL_LCD_MUX1,
	PANEL_LCD_MUX2,
	PANEL_LCD_MUX3,
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

enum {
	PANEL_ID_AT070TN93,
	PANEL_ID_ED090NA,
	PANEL_ID_FLD0800,
	PANEL_ID_SLVDS_SAMPLE,
	PANEL_ID_SLVDS_EXT,
	PANEL_ID_HSD123JPW1,
	PANEL_ID_HDMI,
	PANEL_ID_LA070WV6,
	PANEL_ID_TM123XDHP90,
	PANEL_ID_HSD123KPW2_SERDES,
	PANEL_ID_COMPOSITE,
	PANEL_ID_COMPONENT,
	PANEL_ID_DP0,
	PANEL_ID_DP1,
	PANEL_ID_DP2,
	PANEL_ID_DP3,
	PANEL_ID_DP_MAX
};

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

struct tcc_lcdc_image_update {
	unsigned int Lcdc_layer;
	unsigned int enable;
	unsigned int Frame_width;
	unsigned int Frame_height;

	unsigned int Image_width;
	unsigned int Image_height;
	unsigned int offset_x;
	unsigned int offset_y;
	void *addr0;
	void *addr1;
	void *addr2;
	enum TCC_LCDC_IMG_FMT_TYPE fmt;
};

struct lcd_platform_data {
	unsigned int power_on;
	unsigned int display_on;
	unsigned int bl_on;
	unsigned int reset;

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
	unsigned int uiDisplayPathNum;
#endif

	unsigned int lcdc_id;
	unsigned int logo_data;
	unsigned int platform_id; /* 1 or 2 port LVDS and DP */
	unsigned int output_port1; /* platform output port number1 */
	unsigned int output_port2; /* platform output port number2 */
	unsigned int lcdc_select; /* lcdc mux number in ddi bus */
	unsigned int lcdc_bypass; /* lcdc mux bypass setting(SDM & SRVC) */
	unsigned int lcd_power; /*lcd panel power connector number*/
	void *lcdc_hw; /* platform H/W specific */
};

struct vidinfo {
	const char *name;
	const char *manufacturer;

	struct lcd_platform_data dev;

	ushort id;		/* panel ID */
	ushort vl_col;		/* Number of columns (i.e. 800) */
	ushort vl_row;		/* Number of rows (i.e. 480) */
	ushort vl_rot;		/* Rotation of Display (0, 1, 2, 3) */
	ushort vl_width;	/* Width of display area in millimeters */
	ushort vl_height;	/* Height of display area in millimeters */
	u_char vl_bpix;		/* bits per pixels */

	u_long clk_freq;	/* LCD clock frequency */
	unsigned int clk_div;		/* clock divider */
	unsigned int bus_width;	/* LCD bus width */
	unsigned int lpw;		/* line pulse width */
	unsigned int lpc;		/* line pulse count */
	unsigned int lswc;		/* line start wait clock */
	unsigned int lewc;		/* line end wait clock */
	unsigned int vdb;		/* back porch vsync delay */
	unsigned int vdf;		/* front porch vsync delay */
	unsigned int fpw1;		/* frame pulse width 1 */
	unsigned int flc1;		/* frame line count 1 */
	unsigned int fswc1;		/* frame start wait cycle 1 */
	unsigned int fewc1;		/* frame end wait cycle 1 */
	unsigned int fpw2;		/* frame pulse width 2 */
	unsigned int flc2;		/* frame line count 2 */
	unsigned int fswc2;		/* frame start wait cycle 2 */
	unsigned int fewc2;		/* frame end wait cycle 2 */
	unsigned int sync_invert;

	unsigned int ovp;		/* overlay priority */

	unsigned int vcm;		/* common voltage level in mV */
	unsigned int vsw;		/* swing voltage level in mV */

	ushort  *cmap;          /* Pointer to the colormap */
	void    *priv;          /* Pointer to driver-specific data */

	int (*init)(void);
	int (*set_power)(int on);
	int (*set_backlight_level)(int level);
	int (*init_ex)(const struct DP_Init_Params *pvParams);
	int (*set_power_ex)(unsigned char Params, int iPowerOn);
	int (*set_backlight_level_ex)(unsigned char Params, int iLevel);
	void *lcdbase;
};

void lcd_ctrl_init(void *lcdbase);
void lcd_enable(void);
void lcd_backlight(void);
void lcd_update_disable(void);
void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue);


#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
void lcd_panel_sync(uint8_t ucDisplay_Path);
void lcd_panel_set_background_color(int iBG_Color);
struct vidinfo *lcd_get_panel_infor_path(uint8_t ucDisplay_Path);
#endif

/* coverity[misra_c_2012_rule_8_6_violation : FALSE] */
extern void init_panel_info(struct vidinfo *vid);

#if defined(CONFIG_TCC_DISPLAY_PORT)
/* coverity[misra_c_2012_rule_8_6_violation : FALSE] */
void Dpv14_get_panel_info(unsigned char ucPlatformId,
						struct vidinfo *pstPanel_Info);
/* coverity[misra_c_2012_rule_8_6_violation : FALSE] */
void Dpv14_set_panel_info(unsigned char ucPlatformId,
						const struct vidinfo *pstPanel_Info);
#endif

#ifdef CONFIG_LCD_TM123XDHP90
/* coverity[misra_c_2012_rule_8_6_violation : FALSE] */
void tm123xdhp90_get_panel_info(struct vidinfo *vid);
/* coverity[misra_c_2012_rule_8_6_violation : FALSE] */
void tm123xdhp90_set_panel_info(const struct vidinfo *vid);
//struct lvds_hw_info_t tm123xdhp90_lvds_info;
//struct vidinfo tm123xdhp90_panel_info;
#endif

#ifdef CONFIG_LCD_FLD0800
/* coverity[misra_c_2012_rule_8_6_violation : FALSE] */
void fld0800_get_panel_info(struct vidinfo *vid);
/* coverity[misra_c_2012_rule_8_6_violation : FALSE] */
void fld0800_set_panel_info(const struct vidinfo *vid);
#endif

#endif // TCC_FB_H
