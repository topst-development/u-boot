// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <config.h>
#include <cpu_func.h>
#include <malloc.h>
#include <lcd.h>
#include <mach/clock.h>

#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <asm/telechips/gpio.h>
#include <asm/telechips/tcc_ckc.h>
#include <asm/telechips/vioc/vioc_global.h>
#include <asm/telechips/vioc/reg_physical.h>
#include <asm/telechips/vioc/vioc_disp.h>
#include <asm/telechips/vioc/vioc_config.h>
#include <asm/telechips/vioc/vioc_outcfg.h>
#include <asm/telechips/vioc/vioc_rdma.h>
#include <asm/telechips/vioc/vioc_wmix.h>
#include <asm/telechips/vioc/vioc_dump.h>

#include <linux/io.h>
#include <asm/telechips/vioc/lvds_phy.h>

#ifdef CONFIG_TCC_HDMI_V2_0
#include <asm/telechips/hdmi_v2_0.h>
#include <lcd_HDMIV20.h>
#endif

#define SPLASH_PARTION_NAME "splash"

//DECLARE_GLOBAL_DATA_PTR;

struct TCC_FB_Params_t {
	bool bActivated;
	bool bDevice_Connected;
	bool bDP_PanelMode;
	bool bSDM_Bypass;
	bool bSRVC_Bypass;
	bool bDP_PHYLaneSwap;
	bool keepgoing;
	void *pvFB_BaseAddr;
	const char *pcDevice_Name;
	uint8_t ucDisplayInput_Path;
	uint8_t ucDisplayLCD_Mux;
	uint8_t ucDisplayLCD_Port;
	uint8_t ucDisplayLCD_Power;
	uint8_t ucDP_PanelI2CPort;
	unsigned int uiDP_VideoCode;
	struct vidinfo stPanel_Info;
};

#ifdef CONFIG_TCC803X
#ifdef CONFIG_R5_LVDS_CTRL
#include <linux/soc/telechips/tcc803x_multi_mbox.h>
#include <mailbox.h>
unsigned int lvds_cnt;
unsigned int hdmi_cnt;
#endif
#endif

struct vidinfo panel_info;
void fb_time_stamp(const char *log_stamp);
void fb_time_stamp_show(void);

static struct TCC_FB_Params_t stTCC_FB_Params_t[DISPLAY_CONTROLLER_MAX];

//#define CONFIG_FB_TIME_STAME_ON
#define FB_MAX_TIME_STAMPS 20
struct fb_time_stamp_data {
	ulong time;
	char *log;
};

#if defined(CONFIG_FB_TIME_STAME_ON)
#define fb_time_stame_log(fmt, ...) {					      \
		snprintf(fb_log_data, sizeof(fb_log_data), fmt, __VA_ARGS__); \
		fb_time_stamp(fb_log_data)				      \
	}
#else
#define fb_time_stame_log(fmt, ...)
#endif

#if defined(CONFIG_FB_TIME_STAME_ON)
static struct fb_time_stamp_data fb_time_stamp_data[FB_MAX_TIME_STAMPS];
static int fb_time_stamp_index;
static char fb_log_data[1024];
#endif

void fb_time_stamp(const char *log_stamp)
{
	(void)*log_stamp;
#if defined(CONFIG_FB_TIME_STAME_ON)
	if (fb_time_stamp_index < FB_MAX_TIME_STAMPS) {
		fb_time_stamp_data[fb_time_stamp_index].time = get_timer(0);
		if (log_stamp) {
			fb_time_stamp_data[fb_time_stamp_index].log_stamp =
				strdup(log_stamp);
		}
		fb_time_stamp_index++;
	}
#endif
}

void fb_time_stamp_show(void)
{
#if defined(CONFIG_FB_TIME_STAME_ON)
	int i;

	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	pr_err("FB TIME_STAMP\r\n");
	for (i = 0; i < FB_MAX_TIME_STAMPS; i++) {
		if (fb_time_stamp_data[i].time) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err(" [%d][%s] : %ldms <%ldms>\r\n", i,
			       fb_time_stamp_data[i].log ?
				       fb_time_stamp_data[i].log :
				       "none",
			       fb_time_stamp_data[i].time,
			       (i && fb_time_stamp_data[i].time) ?
				       fb_time_stamp_data[i].time
					       - fb_time_stamp_data[i - 1]
							 .time :
				       0);
		}
	}
#endif
}

ushort *configuration_get_cmap(void)
{
#if defined(CONFIG_LCD_LOGO)
	return bmp_logo_palette;
#else
	return NULL;
#endif
}

static void lcd_panel_get_size(
	const struct TCC_FB_Params_t *pstTCC_FB_Params_t,
	unsigned int *puiLine_Length,
	unsigned int *puiTotal_Length)
{
	/* coverity[cert_int34_c_violation : FALSE] */
	/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
	uint32_t bpix =  (uint32_t)pstTCC_FB_Params_t->stPanel_Info.vl_bpix;
	switch (bpix) {
	case 2:
		bpix = 4U;
		break;
	case 3:
		bpix = 8U;
		break;
	case 4:
		bpix = 16U;
		break;
	case 5:
		bpix = 32U;
		break;
	default:
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s][Error] bpix not valied. [%u]\n",
		       __func__, bpix);
		break;
	}

	*puiLine_Length =
	  ((unsigned int)(pstTCC_FB_Params_t->stPanel_Info.vl_col * bpix) / 8U);

	*puiTotal_Length = (*puiLine_Length & 0xFFFFU) *
		(pstTCC_FB_Params_t->stPanel_Info.vl_row & 0xFFFFU);
}

static void lcd_panel_set_panel_info(const char *pcDevice_Name,
					const struct vidinfo *pstPanel_Infor)
{
	struct vidinfo stPanel_Info_Temp;
	int done = 0;

	if (pstPanel_Infor == NULL) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s]Error: Panel infor pointer is NULL\n", __func__);
		done = 1;
	}

#ifdef CONFIG_LCD_TM123XDHP90
	if (done == 0) {
		tm123xdhp90_get_panel_info(&stPanel_Info_Temp);

		if (strcmp(stPanel_Info_Temp.name, pcDevice_Name) == 0) {
			tm123xdhp90_set_panel_info(pstPanel_Infor);
			done = 1;
		}
	}
#endif

#ifdef CONFIG_LCD_HSD123KPW2_SERDES
	if (done == 0) {
		hsd123kpw2_serdes_get_panel_info(&stPanel_Info_Temp);
		if (strcmp(stPanel_Info_Temp.name, pcDevice_Name) == 0) {
			hsd123kpw2_serdes_set_panel_info(pstPanel_Infor);
			done = 1;
		}
	}
#endif

#ifdef CONFIG_LCD_FLD0800
	if (done == 0) {
		fld0800_get_panel_info(&stPanel_Info_Temp);
		if (strcmp(stPanel_Info_Temp.name, pcDevice_Name) == 0) {
			fld0800_set_panel_info(pstPanel_Infor);
			done = 1;
		}
	}
#endif

#ifdef CONFIG_LCD_SLVDS_SAMPLE
	if (done == 0) {
		slvds_sample_get_panel_info(&stPanel_Info_Temp);
		if (strcmp(stPanel_Info_Temp.name, pcDevice_Name) == 0) {
			slvds_sample_set_panel_info(pstPanel_Infor);
			done = 1;
		}
	}
#endif

#ifdef CONFIG_LCD_SLVDS_EXT
	if (done == 0) {
		slvds_ext_get_panel_info(&stPanel_Info_Temp);
		if (strcmp(stPanel_Info_Temp.name, pcDevice_Name) == 0) {
			slvds_ext_set_panel_info(pstPanel_Infor);
			done = 1;
		}
	}
#endif

#if defined(CONFIG_TCC_DISPLAY_PORT)
	if (done == 0) {
		Dpv14_get_panel_info(
			(uint8_t)(pstPanel_Infor->dev.platform_id & 0xFFU),
			&stPanel_Info_Temp);
		if (strcmp(stPanel_Info_Temp.name, pcDevice_Name) == 0) {
			Dpv14_set_panel_info(
			 (uint8_t)(pstPanel_Infor->dev.platform_id & 0xFFU),
			 pstPanel_Infor);
			done = 1;
		}
	}
#endif

#if defined(CONFIG_LCD_HDMIV20)
	if (done == 0) {
		hdmiv20_get_panel_info(&stPanel_Info_Temp);
		if (strcmp(stPanel_Info_Temp.name, pcDevice_Name) == 0) {
			hdmiv20_set_panel_info(pstPanel_Infor);
			done = 1;
		}
	}
#endif
	if (done == 0) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s]Error: Panel not found : %s\n",
		       __func__, stPanel_Info_Temp.name);
	}
}

static int lcd_panel_get_panel_info(const char *pcDevice_Name,
					struct vidinfo *pstPanel_Infor)
{
	int ret = 0;
	int done = 0;

	if (pstPanel_Infor == NULL) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s]Error: Panel infor pointer is NULL\n", __func__);
		ret = -1;
	}

	if (ret == 0) {
#if defined(CONFIG_TCC_DISPLAY_PORT)
		uint8_t uiPlatform_id =
			(uint8_t)(pstPanel_Infor->dev.platform_id & 0xFFU);
#endif

#ifdef CONFIG_LCD_TM123XDHP90
			tm123xdhp90_get_panel_info(pstPanel_Infor);
			if (strcmp(pstPanel_Infor->name, pcDevice_Name) == 0) {
				done = 1;
			}
#endif
#ifdef CONFIG_LCD_HSD123KPW2_SERDES
		if (done == 0) {
			hsd123kpw2_serdes_get_panel_info(pstPanel_Infor);
			if (strcmp(pstPanel_Infor->name, pcDevice_Name) == 0) {
				done = 1;
			}
		}
#endif
#ifdef CONFIG_LCD_FLD0800
		if (done == 0) {
			fld0800_get_panel_info(pstPanel_Infor);
			if (strcmp(pstPanel_Infor->name, pcDevice_Name) == 0) {
				done = 1;
			}
		}
#endif
#ifdef CONFIG_LCD_SLVDS_SAMPLE
		if (done == 0) {
			slvds_sample_get_panel_info(pstPanel_Infor);
			if (strcmp(pstPanel_Infor->name, pcDevice_Name) == 0) {
				done = 1;
			}
		}
#endif
#ifdef CONFIG_LCD_SLVDS_EXT
		if (done == 0) {
			slvds_ext_get_panel_info(pstPanel_Infor);
			if (strcmp(pstPanel_Infor->name, pcDevice_Name) == 0) {
				done = 1;
			}
		}
#endif
#if defined(CONFIG_TCC_DISPLAY_PORT)
		if (done == 0) {
			if (uiPlatform_id < (uint8_t)PANEL_DP_MAX) {
				Dpv14_get_panel_info(uiPlatform_id, pstPanel_Infor);
				if (strcmp(pstPanel_Infor->name, pcDevice_Name) == 0) {
					done = 1;
				}
			}
		}
#endif
#if defined(CONFIG_LCD_HDMIV20)
		if (done == 0) {
			hdmiv20_get_panel_info(pstPanel_Infor);
			if (strcmp(pstPanel_Infor->name, pcDevice_Name) == 0) {
				done = 1;
			}
		}
#endif
	}

	if (done == 0) {
		ret = -1; //not found any panel
	}

	return ret;
}

static void lcd_panel_set_lvds_path(
	const char *pcDevice_Name, uint8_t ucPlatform_Id,
	uint8_t ucLvds_Port, unsigned int ucLcd_Mux,
	unsigned int ucLcd_Bypass, struct vidinfo *pstPanel_Infor)
{
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("[%s]lvds_port = %u, lcdc_mux = %u\n", __func__,
		(unsigned int)ucLvds_Port, (unsigned int)ucLcd_Mux);

	if (ucPlatform_Id == (uint8_t)PANEL_LVDS_DUAL) {
		pstPanel_Infor->dev.platform_id = PANEL_LVDS_DUAL;
		pstPanel_Infor->dev.output_port1 = LVDS_PHY_PORT_D0;
		pstPanel_Infor->dev.output_port2 = LVDS_PHY_PORT_D1;
	} else {
		pstPanel_Infor->dev.platform_id = PANEL_LVDS_SINGLE;
		pstPanel_Infor->dev.output_port1 = ucLvds_Port;
		pstPanel_Infor->dev.output_port2 = LVDS_PHY_PORT_MAX;
	}

	pstPanel_Infor->dev.lcdc_select = ucLcd_Mux;
	pstPanel_Infor->dev.lcdc_bypass = ucLcd_Bypass;
	lcd_panel_set_panel_info(pcDevice_Name, pstPanel_Infor);
}

static void lcd_panel_init_params(void *pvFB_BaseAddr)
{
#if defined(CONFIG_DISPLAY_PORT_V14)
	uint8_t ucDP_Platform_Id = PANEL_DP0;
#endif
	struct TCC_FB_Params_t *pstTCC_FB_Params_t;

	(void)memset(&stTCC_FB_Params_t[DISPLAY_CONTROLLER_0], 0,
	       (sizeof(struct TCC_FB_Params_t) * (ulong)DISPLAY_CONTROLLER_MAX));

#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC0)
	pstTCC_FB_Params_t = &stTCC_FB_Params_t[DISPLAY_CONTROLLER_0];

	pstTCC_FB_Params_t->bActivated = true;
	pstTCC_FB_Params_t->pvFB_BaseAddr = pvFB_BaseAddr;
	pstTCC_FB_Params_t->pcDevice_Name = CONFIG_DC0_LCD_DEVICE;
#if defined(CONFIG_DC0_DP_DISPLAY_DEVICE)
#if defined(CONFIG_DP_PANEL_MODE)
	pstTCC_FB_Params_t->bDP_PanelMode = true;
	pstTCC_FB_Params_t->uiDP_VideoCode = (unsigned int)CONFIG_DP_PANEL_VIC;
	pstTCC_FB_Params_t->ucDP_PanelI2CPort =
		(uint8_t)CONFIG_DP_PANEL_I2C_PORT;
#else
	pstTCC_FB_Params_t->uiDP_VideoCode =
		(unsigned int)CONFIG_DC0_DP_OUTPUT_VIC;
#endif
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)ucDP_Platform_Id;
	ucDP_Platform_Id++;
#else
#if defined(CONFIG_DC0_DUAL_PORT_LVDS)
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)PANEL_LVDS_DUAL;
#elif defined(CONFIG_DC0_SINGLE_PORT_LVDS)
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)PANEL_LVDS_SINGLE;
#else // CONFIG_DC0_LCD_HDMIV20
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)PANEL_HDMI;
#endif
#if !defined(CONFIG_DC0_LCD_HDMIV20)
	pstTCC_FB_Params_t->ucDisplayLCD_Port =
		(uint8_t)CONFIG_DC0_DISP_LVDS_LCD_PORT;
#endif
#endif /* #if defined( CONFIG_DC0_DP_DISPLAY_DEVICE ) */

	pstTCC_FB_Params_t->ucDisplayInput_Path =
		(uint8_t)PANEL_PATH_DISP0;
	#if defined(CONFIG_DC0_DISP_LCD_MUX)
	pstTCC_FB_Params_t->ucDisplayLCD_Mux =
		(uint8_t)CONFIG_DC0_DISP_LCD_MUX;
	if (pstTCC_FB_Params_t->ucDisplayLCD_Mux
	    == (uint8_t)PANEL_LCD_MUX2) {
		#if defined(CONFIG_DC0_SDM_BYPASS)
		pstTCC_FB_Params_t->bSDM_Bypass = true;
		#endif
	}
	if (pstTCC_FB_Params_t->ucDisplayLCD_Mux
	    == (uint8_t)PANEL_LCD_MUX3) {
		#if defined(CONFIG_DC0_SRVC_BYPASS)
		pstTCC_FB_Params_t->bSRVC_Bypass = true;
		#endif
	}
	#endif

	pstTCC_FB_Params_t->ucDisplayLCD_Power =
		(uint8_t)CONFIG_DC0_DISP_LCD_POWER;

#if defined(CONFIG_DP_PHY_LANE02_13_SWAP)
	pstTCC_FB_Params_t->bDP_PHYLaneSwap = true;
#endif

#endif /* #if defined( CONFIG_DISPLAY_OUTPUT_PATH_DC0 ) */

#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC1)
	pstTCC_FB_Params_t = &stTCC_FB_Params_t[DISPLAY_CONTROLLER_1];

	pstTCC_FB_Params_t->bActivated = true;
#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
	pstTCC_FB_Params_t->pvFB_BaseAddr =
		(void *)(pvFB_BaseAddr + (CONFIG_SYS_VIDEO_LOGO_MAX_SIZE *
					  DISPLAY_CONTROLLER_1));
#else
	pstTCC_FB_Params_t->pvFB_BaseAddr = (void *)(pvFB_BaseAddr);
#endif
	pstTCC_FB_Params_t->pcDevice_Name = (char *)CONFIG_DC1_LCD_DEVICE;
#if defined(CONFIG_DC1_DP_DISPLAY_DEVICE)
#if defined(CONFIG_DP_PANEL_MODE)
	pstTCC_FB_Params_t->bDP_PanelMode = true;
	pstTCC_FB_Params_t->uiDP_VideoCode = (unsigned int)CONFIG_DP_PANEL_VIC;
	pstTCC_FB_Params_t->ucDP_PanelI2CPort =
		(uint8_t)CONFIG_DP_PANEL_I2C_PORT;
#else
	pstTCC_FB_Params_t->uiDP_VideoCode =
		(unsigned int)CONFIG_DC1_DP_OUTPUT_VIC;
#endif
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)ucDP_Platform_Id;
	ucDP_Platform_Id++;
#else
#if defined(CONFIG_DC1_DUAL_PORT_LVDS)
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)PANEL_LVDS_DUAL;
#elif defined(CONFIG_DC1_SINGLE_PORT_LVDS)
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)PANEL_LVDS_SINGLE;
#else // CONFIG_DC1_LCD_HDMIV20
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)PANEL_HDMI;
#endif
#if !defined(CONFIG_DC1_LCD_HDMIV20)
	pstTCC_FB_Params_t->ucDisplayLCD_Port =
		(uint8_t)CONFIG_DC1_DISP_LVDS_LCD_PORT;
#endif
#endif /* #if defined( CONFIG_DC1_DP_DISPLAY_DEVICE ) */

	pstTCC_FB_Params_t->ucDisplayInput_Path =
		(uint8_t)PANEL_PATH_DISP1;
	#if defined(CONFIG_DC1_DISP_LCD_MUX)
	pstTCC_FB_Params_t->ucDisplayLCD_Mux =
		(uint8_t)CONFIG_DC1_DISP_LCD_MUX;
	if (pstTCC_FB_Params_t->ucDisplayLCD_Mux
	    == (uint8_t)PANEL_LCD_MUX2) {
		#if defined(CONFIG_DC1_SDM_BYPASS)
		pstTCC_FB_Params_t->bSDM_Bypass = true;
		#endif
	}
	if (pstTCC_FB_Params_t->ucDisplayLCD_Mux
	    == (uint8_t)PANEL_LCD_MUX3) {
		#if defined(CONFIG_DC1_SRVC_BYPASS)
		pstTCC_FB_Params_t->bSRVC_Bypass = true;
		#endif
	}
	#endif
	pstTCC_FB_Params_t->ucDisplayLCD_Power =
		(uint8_t)CONFIG_DC1_DISP_LCD_POWER;

#if defined(CONFIG_DP_PHY_LANE02_13_SWAP)
	pstTCC_FB_Params_t->bDP_PHYLaneSwap = true;
#endif

#endif /* #if defined( CONFIG_DISPLAY_OUTPUT_PATH_DC1 ) */

#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC2)
	pstTCC_FB_Params_t = &stTCC_FB_Params_t[DISPLAY_CONTROLLER_2];

	pstTCC_FB_Params_t->bActivated = true;
#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
	pstTCC_FB_Params_t->pvFB_BaseAddr =
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		(void *)(pvFB_BaseAddr + (CONFIG_SYS_VIDEO_LOGO_MAX_SIZE *
					  DISPLAY_CONTROLLER_2));
#else
	pstTCC_FB_Params_t->pvFB_BaseAddr = (void *)(pvFB_BaseAddr);
#endif
	pstTCC_FB_Params_t->pcDevice_Name = CONFIG_DC2_LCD_DEVICE;
#if defined(CONFIG_DC2_DP_DISPLAY_DEVICE)
#if defined(CONFIG_DP_PANEL_MODE)
	pstTCC_FB_Params_t->bDP_PanelMode = true;
	pstTCC_FB_Params_t->uiDP_VideoCode = (unsigned int)CONFIG_DP_PANEL_VIC;
	pstTCC_FB_Params_t->ucDP_PanelI2CPort =
		(uint8_t)CONFIG_DP_PANEL_I2C_PORT;
#else
	pstTCC_FB_Params_t->uiDP_VideoCode =
		(unsigned int)CONFIG_DC2_DP_OUTPUT_VIC;
#endif
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)ucDP_Platform_Id;
	ucDP_Platform_Id++;
#else
#if defined(CONFIG_DC2_DUAL_PORT_LVDS)
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)PANEL_LVDS_DUAL;
#elif defined(CONFIG_DC2_SINGLE_PORT_LVDS)
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)PANEL_LVDS_SINGLE;
#else // CONFIG_DC2_LCD_HDMIV20
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)PANEL_HDMI;
#endif
#if !defined(CONFIG_DC2_LCD_HDMIV20)
	pstTCC_FB_Params_t->ucDisplayLCD_Port =
		(uint8_t)CONFIG_DC2_DISP_LVDS_LCD_PORT;
#endif
#endif /* #if defined( CONFIG_DC2_DP_DISPLAY_DEVICE ) */

	pstTCC_FB_Params_t->ucDisplayInput_Path =
		(uint8_t)PANEL_PATH_DISP2;
	#if defined(CONFIG_DC2_DISP_LCD_MUX)
	pstTCC_FB_Params_t->ucDisplayLCD_Mux =
		(uint8_t)CONFIG_DC2_DISP_LCD_MUX;
	if (pstTCC_FB_Params_t->ucDisplayLCD_Mux
	    == (uint8_t)PANEL_LCD_MUX2) {
		#if defined(CONFIG_DC2_SDM_BYPASS)
		pstTCC_FB_Params_t->bSDM_Bypass = true;
		#endif
	}
	if (pstTCC_FB_Params_t->ucDisplayLCD_Mux
	    == (uint8_t)PANEL_LCD_MUX3) {
		#if defined(CONFIG_DC2_SRVC_BYPASS)
		pstTCC_FB_Params_t->bSRVC_Bypass = true;
		#endif
	}
	#endif
	pstTCC_FB_Params_t->ucDisplayLCD_Power =
		(uint8_t)CONFIG_DC2_DISP_LCD_POWER;
#if defined(CONFIG_DP_PHY_LANE02_13_SWAP)
	pstTCC_FB_Params_t->bDP_PHYLaneSwap = true;
#endif

#endif /* #if defined( CONFIG_DISPLAY_OUTPUT_PATH_DC2 ) */

#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC3)
	pstTCC_FB_Params_t = &stTCC_FB_Params_t[DISPLAY_CONTROLLER_3];

	pstTCC_FB_Params_t->bActivated = true;

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
	pstTCC_FB_Params_t->pvFB_BaseAddr =
		(void *)(pvFB_BaseAddr +
			 (CONFIG_SYS_VIDEO_LOGO_MAX_SIZE *
			  DISPLAY_CONTROLLER_3));
#else
	pstTCC_FB_Params_t->pvFB_BaseAddr = (void *)(pvFB_BaseAddr);
#endif

	pstTCC_FB_Params_t->pcDevice_Name = (char *)CONFIG_DC3_LCD_DEVICE;

#if defined(CONFIG_DC3_DP_DISPLAY_DEVICE)
#if defined(CONFIG_DP_PANEL_MODE)
	pstTCC_FB_Params_t->bDP_PanelMode = true;
	pstTCC_FB_Params_t->uiDP_VideoCode = (unsigned int)CONFIG_DP_PANEL_VIC;
	pstTCC_FB_Params_t->ucDP_PanelI2CPort =
		(uint8_t)CONFIG_DP_PANEL_I2C_PORT;
#else
	pstTCC_FB_Params_t->uiDP_VideoCode =
		(unsigned int)CONFIG_DC3_DP_OUTPUT_VIC;
#endif
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)ucDP_Platform_Id;
#else

#if defined(CONFIG_DC3_DUAL_PORT_LVDS)
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)PANEL_LVDS_DUAL;
#else
	pstTCC_FB_Params_t->stPanel_Info.dev.platform_id =
		(unsigned int)PANEL_LVDS_SINGLE;
#endif

	pstTCC_FB_Params_t->ucDisplayLCD_Port =
		(uint8_t)CONFIG_DC3_DISP_LVDS_LCD_PORT;
#endif /* #if defined( CONFIG_DC3_DP_DISPLAY_DEVICE ) */

	pstTCC_FB_Params_t->ucDisplayInput_Path =
		(uint8_t)PANEL_PATH_DISP3;
	pstTCC_FB_Params_t->ucDisplayLCD_Mux =
		(uint8_t)CONFIG_DC3_DISP_LCD_MUX;
	pstTCC_FB_Params_t->ucDisplayLCD_Power =
		(uint8_t)CONFIG_DC3_DISP_LCD_POWER;

	if (pstTCC_FB_Params_t->ucDisplayLCD_Mux
	    == (uint8_t)PANEL_LCD_MUX2) {
#if defined(CONFIG_DC3_SDM_BYPASS)
		pstTCC_FB_Params_t->bSDM_Bypass = true;
#endif
	}
	if (pstTCC_FB_Params_t->ucDisplayLCD_Mux
	    == (uint8_t)PANEL_LCD_MUX3) {
#if defined(CONFIG_DC3_SRVC_BYPASS)
		pstTCC_FB_Params_t->bSRVC_Bypass = true;
#endif
	}

#if defined(CONFIG_DP_PHY_LANE02_13_SWAP)
	pstTCC_FB_Params_t->bDP_PHYLaneSwap = true;
#endif

#endif /* #if defined( CONFIG_DISPLAY_OUTPUT_PATH_DC3 ) */

#if defined(CONFIG_DISPLAY_PORT_V14)
	(void)ucDP_Platform_Id; //To avoid unused value warning
#endif
}

static void
/* coverity[HIS_metric_violation : FALSE] */
lcd_panel_on(struct vidinfo *pstPanel_Infor, uint8_t ucDisplay_Path)
{
	uint8_t ucPlatform_Id;
	uint8_t ucDP_Index;
	int iRetVal;
	ulong flag_clk_en = CKC_ENABLE;
	ulong lclk = 0;
	struct tcc_lcdc_image_update ImageInfo;
	struct TCC_FB_Params_t *pstTCC_FB_Params_t =
		&stTCC_FB_Params_t[ucDisplay_Path];
	int done = 0;

	if (pstPanel_Infor == NULL) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Panel infor pointer is NULL\n", __func__,
		       __LINE__);
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto out_func;
	}

	ucPlatform_Id = (uint8_t)(pstPanel_Infor->dev.platform_id & 0xFFU);

	switch (pstPanel_Infor->dev.lcdc_id) {
	case PANEL_PATH_DISP0:
		(void)tcc_set_peri(
			PERI_LCD0, flag_clk_en,
			(pstPanel_Infor->clk_div != 0UL) ?
				(pstPanel_Infor->clk_freq
				 * (pstPanel_Infor->clk_div * 2UL)) :
				pstPanel_Infor->clk_freq,
			0);
		lclk = tcc_get_peri(PERI_LCD0);
		break;
	case PANEL_PATH_DISP1:
		(void)tcc_set_peri(
			PERI_LCD1, flag_clk_en,
			(pstPanel_Infor->clk_div != 0UL) ?
				(pstPanel_Infor->clk_freq
				 * (pstPanel_Infor->clk_div * 2UL)) :
				pstPanel_Infor->clk_freq,
			0);
		lclk = tcc_get_peri(PERI_LCD1);
		break;
	case PANEL_PATH_DISP2:
		(void)tcc_set_peri(
			PERI_LCD2, flag_clk_en,
			(pstPanel_Infor->clk_div != 0UL) ?
				(pstPanel_Infor->clk_freq
				 * (pstPanel_Infor->clk_div * 2UL)) :
				pstPanel_Infor->clk_freq,
			0);
		lclk = tcc_get_peri(PERI_LCD2);
		break;
	case PANEL_PATH_DISP3:
#if !defined(CONFIG_TCC803X)
		(void)tcc_set_peri(
			PERI_LCD3, flag_clk_en,
			(pstPanel_Infor->clk_div != 0UL) ?
				(pstPanel_Infor->clk_freq
				 * (pstPanel_Infor->clk_div * 2UL)) :
				pstPanel_Infor->clk_freq,
			0);
		lclk = tcc_get_peri(PERI_LCD3);
#endif
		break;
	default:
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Invalid lcdc number(%d)\n", __func__,
		       __LINE__, pstPanel_Infor->dev.lcdc_id);
		done = 1;
		break;
	}
	if (done == 1) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto out_func;
	}

	if ((ucPlatform_Id == (uint8_t)PANEL_DP0)
	    || (ucPlatform_Id == (uint8_t)PANEL_DP1)
	    || (ucPlatform_Id == (uint8_t)PANEL_DP2)
	    || (ucPlatform_Id == (uint8_t)PANEL_DP3)) {
		iRetVal = lcd_panel_get_panel_info(
			pstTCC_FB_Params_t->pcDevice_Name, pstPanel_Infor);
		if (iRetVal < 0) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err("***[%s:%d]Error: Can not find panel information(%s)\n",
			       __func__, __LINE__,
			       pstTCC_FB_Params_t->pcDevice_Name);
		}

		pstPanel_Infor->lcdbase = pstTCC_FB_Params_t->pvFB_BaseAddr;
		pstPanel_Infor->dev.lcdc_id =
			pstTCC_FB_Params_t->ucDisplayInput_Path;
		pstPanel_Infor->dev.platform_id = ucPlatform_Id;
		pstPanel_Infor->dev.lcdc_select =
			pstTCC_FB_Params_t->ucDisplayLCD_Mux;

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
		pstTCC_FB_Params_t->stPanel_Info.dev.uiDisplayPathNum =
			(unsigned int)ucDisplay_Path;
#endif

		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		/* coverity[cert_fio47_c_violation : FALSE] */
		pr_info("[%s:%d]DP %d => Display controller %d, Dtd clk %ld -> tcc clk %ld\n",
			__func__, __LINE__, ucPlatform_Id,
			pstPanel_Infor->dev.lcdc_id,
			(pstPanel_Infor->clk_freq / 1000UL), (lclk / 1000UL));

		pstPanel_Infor->clk_freq = (lclk / 1000UL);

		lcd_panel_set_panel_info(
			pstTCC_FB_Params_t->pcDevice_Name, pstPanel_Infor);

	} else {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("[%s:%d]%s lcdc:%d clk:%ld set clk:%ld\n", __func__,
			__LINE__, pstPanel_Infor->name,
			pstPanel_Infor->dev.lcdc_id,
			pstPanel_Infor->clk_freq, lclk);
	}

	(void)memset(&ImageInfo, 0, sizeof(ImageInfo));

	ImageInfo.addr0 = pstPanel_Infor->lcdbase;
	ImageInfo.Lcdc_layer = 0;
	ImageInfo.enable = 1;

#if defined(CONFIG_BOOT_LOGO_SPLASH)
#if defined(CONFIG_FB_SIZE_TO_PANEL_SIZE)
	uint32_t uiTotal_LineLen, uiTotal_Pixels;
	uint32_t uiLogo_X = 0, uiLogo_Y = 0, uiH = 0;
	uint8_t ucBpix_Byte;
	uint8_t *pucTemp_FB, *pucFB, *pucFB_Flush;
	uint8_t *pucSplash_FB;
#endif

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
	unsigned int uiLogo_Width = 0, uiLogo_Height = 0;
	uint8_t *pucImageName;
	int is_image_load_failed = 0;

	switch (ucDisplay_Path) {
	case DISPLAY_CONTROLLER_0:
		pucImageName = (uint8_t *)DC0_24BIT_SPLASH_BMP_NAME;
		break;
	case DISPLAY_CONTROLLER_1:
		pucImageName = (uint8_t *)DC1_24BIT_SPLASH_BMP_NAME;
		break;
	case DISPLAY_CONTROLLER_2:
		pucImageName = (uint8_t *)DC2_24BIT_SPLASH_BMP_NAME;
		break;
	case DISPLAY_CONTROLLER_3:
		pucImageName = (uint8_t *)DC3_24BIT_SPLASH_BMP_NAME;
		break;
	default:
		pucImageName = NULL;
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("\n[%s:%d][Individual]Error: Invalid path(%d)\n",
		       __func__, __LINE__, ucDisplay_Path);
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto out_func;
	}

#if defined(CONFIG_FB_SIZE_TO_PANEL_SIZE)
	pucTemp_FB = (uint8_t *)malloc(CONFIG_SYS_VIDEO_LOGO_MAX_SIZE);
	if (pucTemp_FB == NULL) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("\n[%s:%d]Error: malloc fails\n", __func__, __LINE__);
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto out_func;
	}
	pucSplash_FB = pucTemp_FB;

	if (splash_image_load_v2(
		    (uint8_t *)SPLASH_PARTION_NAME, pucImageName,
		    (void *)pucTemp_FB, &uiLogo_Width, &uiLogo_Height)
	    < 0)
#else
	if (splash_image_load_v2(
		    (uint8_t *)SPLASH_PARTION_NAME, pucImageName,
		    ImageInfo.addr0, &uiLogo_Width, &uiLogo_Height)
	    < 0)
#endif
	{
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("\n[%s:%d][Individual]Error: Failed to load a image from the splash partition named '%s'\n",
		       __func__, __LINE__, pucImageName);
		is_image_load_failed = 1;
	} else {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("\n[%s:%d][Individual](0x%p)Succeed to load a image from the splash partition named '%s'\n",
			__func__, __LINE__, ImageInfo.addr0, pucImageName);
	}

	if (is_image_load_failed == 1) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto out_func;
	}

	if ((uiLogo_Width > MAX_INDIVIDUAL_IMAGE_WIDTH)
	    || (uiLogo_Height > MAX_INDIVIDUAL_IMAGE_HEIGHT)) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("\n[%s:%d][Individual]Error: Not supported resollution is larger than 1920 x 1080(%d x %d)\n",
		       __func__, __LINE__, uiLogo_Width, uiLogo_Height);
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto out_func;
	}

	if ((uiLogo_Width == 0) || (uiLogo_Height == 0)) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("\n[%s:%d][Individual]Error: Invalid logo width(%d) or height(%d)\n",
		       __func__, __LINE__, uiLogo_Width, uiLogo_Height);
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto out_func;
	}
#else
#if defined(CONFIG_FB_SIZE_TO_PANEL_SIZE)
	pucTemp_FB = (uint8_t *)malloc(CONFIG_SYS_VIDEO_LOGO_MAX_SIZE);
	if (pucTemp_FB == NULL) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("\n[%s:%d]Error: malloc fails\n", __func__, __LINE__);
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto out_func;
	}
	pucSplash_FB = pucTemp_FB;

	if (splash_image_load_v2(
		    (uint8_t *)SPLASH_PARTION_NAME, "bootlogo",
		    (void *)pucTemp_FB, &uiLogo_Width, &uiLogo_Height)
	    < 0)
#else
	if (splash_image_load_v2(
		    (uint8_t *)SPLASH_PARTION_NAME, "bootlogo",
		    ImageInfo.addr0, &uiLogo_Width, &uiLogo_Height)
	    < 0)
#endif
	{
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("*** Failed to load a image from the splash partition\n");
	} else {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("*** Succeed to load splash image from the splash partition.\n");
	}
#endif

#if defined(CONFIG_FB_SIZE_TO_PANEL_SIZE)
	/* coverity[cert_int34_c_violation : FALSE] */
	ucBpix_Byte = NBITS(pstPanel_Infor->vl_bpix) / 8;

	uiTotal_LineLen = (pstPanel_Infor->vl_col * ucBpix_Byte);
	uiTotal_Pixels = (uiTotal_LineLen * pstPanel_Infor->vl_row);

	pucFB = (uint8_t *)ImageInfo.addr0;

	memset((void *)pucFB, 0, uiTotal_Pixels);

	flush_dcache_range((ulong)pucFB, (ulong)(pucFB + uiTotal_Pixels));

	ImageInfo.Frame_width = ImageInfo.Image_width = pstPanel_Infor->vl_col;
	ImageInfo.Frame_height = ImageInfo.Image_height =
		pstPanel_Infor->vl_row;

	if (pstPanel_Infor->vl_col > MAX_INDIVIDUAL_IMAGE_WIDTH) {
		ImageInfo.Frame_width = ImageInfo.Image_width =
			MAX_INDIVIDUAL_IMAGE_WIDTH;
		ImageInfo.offset_x =
			((pstPanel_Infor->vl_col - MAX_INDIVIDUAL_IMAGE_WIDTH)
			 / 2);
	}
	if (pstPanel_Infor->vl_row > MAX_INDIVIDUAL_IMAGE_HEIGHT) {
		ImageInfo.Frame_height = ImageInfo.Image_height =
			MAX_INDIVIDUAL_IMAGE_HEIGHT;
		ImageInfo.offset_y =
			((pstPanel_Infor->vl_row - MAX_INDIVIDUAL_IMAGE_HEIGHT)
			 / 2);
	}

	if (ImageInfo.Frame_width > uiLogo_Width) {
		uiLogo_X = ((ImageInfo.Frame_width - uiLogo_Width) / 2);
	} else if (ImageInfo.Frame_width < uiLogo_Width) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Log width(%d) is larger than pannel width(%d)\n",
		       __func__, __LINE__, (u32)uiLogo_Width,
		       (unsigned int)ImageInfo.Frame_width);
		free(pucSplash_FB);
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto out_func;
	}

	if (ImageInfo.Frame_height > uiLogo_Height) {
		uiLogo_Y = ((ImageInfo.Frame_height - uiLogo_Height) / 2);
	} else if (ImageInfo.Frame_height < uiLogo_Height) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Log width(%d) is larger than pannel width(%d)\n",
		       __func__, __LINE__, (u32)uiLogo_Width,
		       (unsigned int)ImageInfo.Frame_height);
		free(pucSplash_FB);
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto out_func;
	}

	pucFB = (uint8_t *)(ImageInfo.addr0 +
			    (uiTotal_LineLen * uiLogo_Y) +
			    (uiLogo_X * ucBpix_Byte));
	pucFB_Flush = pucFB;

	for (uiH = 0; uiH < uiLogo_Height; uiH++) {
		memcpy((void *)pucFB, (void *)pucTemp_FB,
		       (uiLogo_Width * ucBpix_Byte));
		pucFB += uiTotal_LineLen;
		pucTemp_FB += (uiLogo_Width * ucBpix_Byte);
	}

	flush_dcache_range(
		(ulong)pucFB_Flush,
		(ulong)(pucFB_Flush + (uiTotal_LineLen * uiLogo_Height)));
	free(pucSplash_FB);

	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("[%s:%d]Bpix(%d), Log_X(%d), Log_Y(%d)\n", __func__, __LINE__,
		ucBpix_Byte, uiLogo_X, uiLogo_Y);

#else
	ImageInfo.Frame_width = ImageInfo.Image_width = uiLogo_Width;
	ImageInfo.Frame_height = ImageInfo.Image_height = uiLogo_Height;

	if (pstPanel_Infor->vl_col > uiLogo_Width) {
		ImageInfo.offset_x =
			((pstPanel_Infor->vl_col - uiLogo_Width) / 2);
	} else if (pstPanel_Infor->vl_col < uiLogo_Width) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Log width(%d) is larger than pannel width(%d) -> it makes logo cut right side\n",
		       __func__, __LINE__, (u32)uiLogo_Width,
		       (unsigned int)pstPanel_Infor->vl_col);
		ImageInfo.Image_width = pstPanel_Infor->vl_col;
	}

	if (pstPanel_Infor->vl_row > uiLogo_Height) {
		ImageInfo.offset_y =
			((pstPanel_Infor->vl_row - uiLogo_Height) / 2);
	} else if (pstPanel_Infor->vl_row < uiLogo_Height) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Log width(%d) is larger than pannel width(%d) -> it makes logo cut bottom side\n",
		       __func__, __LINE__, (u32)uiLogo_Width,
		       (unsigned int)pstPanel_Infor->vl_col);
		ImageInfo.Image_height = pstPanel_Infor->vl_row;
	}
#endif

	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("[%s:%d]BMP -> Col(%d), Row(%d)\n", __func__, __LINE__,
		(u32)uiLogo_Width, (u32)uiLogo_Height);
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("[%s:%d]FB  -> W(%d), H(%d), Frame W(%d), Frame H(%d), x offset(%d), y offset(%d)\n",
		__func__, __LINE__, (u32)ImageInfo.Frame_width,
		(u32)ImageInfo.Frame_height, (u32)ImageInfo.Image_width,
		(u32)ImageInfo.Image_height, (u32)ImageInfo.offset_x,
		(u32)ImageInfo.offset_y);
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("[%s:%d]Pannel -> Col(%d), Row(%d)\n", __func__, __LINE__,
		(u32)pstPanel_Infor->vl_col, (u32)pstPanel_Infor->vl_row);
#else

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
	ImageInfo.Frame_width = pstPanel_Infor->vl_col;
	ImageInfo.Image_width = pstPanel_Infor->vl_col;
	ImageInfo.Frame_height = pstPanel_Infor->vl_row;
	ImageInfo.Image_height = pstPanel_Infor->vl_row;

	if (pstPanel_Infor->vl_col > (ushort)MAX_INDIVIDUAL_IMAGE_WIDTH) {
		ImageInfo.Frame_width = MAX_INDIVIDUAL_IMAGE_WIDTH;
		ImageInfo.Image_width = MAX_INDIVIDUAL_IMAGE_WIDTH;
	}
	if (pstPanel_Infor->vl_row > (ushort)MAX_INDIVIDUAL_IMAGE_HEIGHT) {
		ImageInfo.Frame_height = MAX_INDIVIDUAL_IMAGE_HEIGHT;
		ImageInfo.Image_height = MAX_INDIVIDUAL_IMAGE_HEIGHT;
	}

	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("[%s:%d]FB  -> W(%d), H(%d), Frame W(%d), Frame H(%d), x offset(%d), y offset(%d)\n",
		__func__, __LINE__, (u32)ImageInfo.Frame_width,
		(u32)ImageInfo.Frame_height, (u32)ImageInfo.Image_width,
		(u32)ImageInfo.Image_height, (u32)ImageInfo.offset_x,
		(u32)ImageInfo.offset_y);
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("[%s:%d]Pannel -> Col(%d), Row(%d)\n", __func__, __LINE__,
		(u32)pstPanel_Infor->vl_col, (u32)pstPanel_Infor->vl_row);
#else
	ImageInfo.Frame_width = pstPanel_Infor->vl_col;
	ImageInfo.Image_width = pstPanel_Infor->vl_col;
	ImageInfo.Frame_height = pstPanel_Infor->vl_row;
	ImageInfo.Image_height = pstPanel_Infor->vl_row;
#endif

#endif /* #if defined(CONFIG_BOOT_LOGO_SPLASH) */

	/* coverity[cert_int34_c_violation : FALSE] */
	/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
	if (NBITS(pstPanel_Infor->vl_bpix) == 16) {
		ImageInfo.fmt = TCC_LCDC_IMG_FMT_RGB565;
	} else {
		ImageInfo.fmt = TCC_LCDC_IMG_FMT_RGB888;
	}

	fb_time_stame_log(
		"display device[%d] - tcclcd_image_ch_set",
		pstPanel_Infor->dev.lcdc_id);
	tcclcd_image_ch_set(pstPanel_Infor->dev.lcdc_id, &ImageInfo);

	if ((ucPlatform_Id == (uint8_t)PANEL_DP0)
	    || (ucPlatform_Id == (uint8_t)PANEL_DP1)
	    || (ucPlatform_Id == (uint8_t)PANEL_DP2)
	    || (ucPlatform_Id == (uint8_t)PANEL_DP3)) {
		ucDP_Index = ucPlatform_Id;
		(void)pstPanel_Infor->set_power_ex(ucDP_Index, 1);
	} else {
		(void)pstPanel_Infor->set_power(1);
	}

	if ((ucPlatform_Id == (uint8_t)PANEL_DP0)
	    || (ucPlatform_Id == (uint8_t)PANEL_DP1)
	    || (ucPlatform_Id == (uint8_t)PANEL_DP2)
	    || (ucPlatform_Id == (uint8_t)PANEL_DP3)) {
		ucDP_Index = ucPlatform_Id;
		(void)pstPanel_Infor->set_backlight_level_ex(ucDP_Index, 1);
	} else {
		(void)pstPanel_Infor->set_backlight_level(1);
	}
#if defined(CONFIG_FB_TIME_STAME_ON)
	fb_time_stamp_show();
#endif
out_func:
	return;
}

//static void lcd_panel_off(vidinfo_t *pstPanel_Infor)
//{
//	uint8_t ucPlatform_Id;
//
//	if (pstPanel_Infor == NULL) {
//		pr_err("[%s:%d]Error: Panel infor pointer is NULL\n", __func__,
//		       __LINE__);
//		return;
//	}
//
//	ucPlatform_Id = pstPanel_Infor->dev.platform_id;
//
//	if ((ucPlatform_Id == (uint8_t)PANEL_DP0)
//	    || (ucPlatform_Id == (uint8_t)PANEL_DP1)
//	    || (ucPlatform_Id == (uint8_t)PANEL_DP2)
//	    || (ucPlatform_Id == (uint8_t)PANEL_DP3)) {
//		pstPanel_Infor->set_power_ex(ucPlatform_Id, 0);
//	} else {
//		pstPanel_Infor->set_power(0);
//	}
//
//	switch (pstPanel_Infor->dev.lcdc_id) {
//	case PANEL_PATH_DISP0:
//		tcc_set_peri(
//			PERI_LCD0, CKC_DISABLE,
//			pstPanel_Infor->clk_div ?
//				(pstPanel_Infor->clk_freq
//				 * (pstPanel_Infor->clk_div * 2)) :
//				pstPanel_Infor->clk_freq,
//			0);
//		break;
//	case PANEL_PATH_DISP1:
//		tcc_set_peri(
//			PERI_LCD1, CKC_DISABLE,
//			pstPanel_Infor->clk_div ?
//				(pstPanel_Infor->clk_freq
//				 * (pstPanel_Infor->clk_div * 2)) :
//				pstPanel_Infor->clk_freq,
//			0);
//		break;
//	case PANEL_PATH_DISP2:
//		tcc_set_peri(
//			PERI_LCD2, CKC_DISABLE,
//			pstPanel_Infor->clk_div ?
//				(pstPanel_Infor->clk_freq
//				 * (pstPanel_Infor->clk_div * 2)) :
//				pstPanel_Infor->clk_freq,
//			0);
//		break;
//	case PANEL_PATH_DISP3:
//#if !defined(CONFIG_TCC803X)
//		tcc_set_peri(
//			PERI_LCD3, CKC_DISABLE,
//			pstPanel_Infor->clk_div ?
//				(pstPanel_Infor->clk_freq
//				 * (pstPanel_Infor->clk_div * 2)) :
//				pstPanel_Infor->clk_freq,
//			0);
//#endif
//		break;
//	default:
//		pr_err("[%s:%d]Error: Invalid lcdc number(%d)\n", __func__,
//		       __LINE__, pstPanel_Infor->dev.lcdc_id);
//		break;
//	}
//}


static void
lcd_panel_set_backlight(const char *name, int level,
			struct vidinfo *pstPanel_Infor)
{
	uint8_t ucPlatform_Id;
	int done = 0;

	if (lcd_panel_get_panel_info(name, pstPanel_Infor) < 0) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: can not find panel information(%s)\n",
		       __func__, __LINE__, name);
		done = 1;
	}

	if (done == 0) {
		ucPlatform_Id = (uint8_t)(pstPanel_Infor->dev.platform_id & 0xFFU);
		if ((ucPlatform_Id == (uint8_t)PANEL_DP0)
		    || (ucPlatform_Id == (uint8_t)PANEL_DP1)
		    || (ucPlatform_Id == (uint8_t)PANEL_DP2)
		    || (ucPlatform_Id == (uint8_t)PANEL_DP3)) {
			(void)pstPanel_Infor->set_backlight_level_ex(ucPlatform_Id, 1);
		} else {
			(void)pstPanel_Infor->set_backlight_level(level);
		}
	}
}

#if defined(CONFIG_FB_DUMP_DISPLAY_PATH)
static void fb_dump_display_path(int display_device_id)
{
	void __iomem *reg_base;
	unsigned int *data;
	unsigned long i;
	int done = 0;

	data = malloc(2048);
	if (!data) {
		pr_err("[%s:%d]Error: Can not alloc\n", __func__, __LINE__);
		done = 1;
	}

	if (done == 0) {
		reg_base = VIOC_DISP_GetAddress(VIOC_DISP0 + display_device_id);
		vioc_read_and_dump_regs((void __iomem *)reg_base, data, 0x80, 1);
		mdelay(10);
		vioc_read_and_dump_regs((void __iomem *)reg_base, data, 0x80, 1);

		reg_base = VIOC_WMIX_GetAddress(VIOC_WMIX0 + display_device_id);
		vioc_read_and_dump_regs((void __iomem *)reg_base, data, 0x20, 1);
		mdelay(10);
		vioc_read_and_dump_regs((void __iomem *)reg_base, data, 0x20, 1);

		for (i = 0; i < 1; i++) {
			reg_base = VIOC_RDMA_GetAddress(
				VIOC_RDMA00 + i + (display_device_id << 2));
			vioc_read_and_dump_regs((void __iomem *)reg_base,
						data, 0x40, 1);
			mdelay(10);
			vioc_read_and_dump_regs((void __iomem *)reg_base,
						data, 0x40, 1);
		}

		reg_base = VIOC_IREQConfig_GetAddress();
		vioc_read_and_dump_regs((void __iomem *)reg_base, data, 0xb0, 1);

		pr_force(
			 "pclk = %15luHz\r\n",
			 tcc_get_peri(PERI_LCD0 + display_device_id));
		pr_force("ddi = %15luHz\r\n", tcc_get_clkctrl(FBUS_DDI));
		free(data);
	}
}
#endif

/* coverity[HIS_metric_violation : FALSE] */
void lcd_ctrl_init(void *lcdbase)
{
	uint8_t numofdp = 0;
	uint8_t panel_id;
	unsigned int count;
	struct TCC_FB_Params_t *fb_params;
	struct DP_Init_Params dp_params;

	lcd_panel_init_params(lcdbase);

	// Count the number of Displayport devices
	for (count = 0; count < (unsigned int)DISPLAY_CONTROLLER_MAX; count++) {
		fb_params = &stTCC_FB_Params_t[count];
		if (fb_params->bActivated) {
			panel_id = (uint8_t)(fb_params->stPanel_Info.dev
						.platform_id & 0xFFU);
			if ((panel_id == (uint8_t)PANEL_DP0) ||
			    (panel_id == (uint8_t)PANEL_DP1) ||
			    (panel_id == (uint8_t)PANEL_DP2) ||
			    (panel_id == (uint8_t)PANEL_DP3)) {
				if (numofdp <= (uint8_t)0xFFU) {
					numofdp++;
				}
			}
			#if defined(CONFIG_FB_DUMP_DISPLAY_PATH)
			fb_dump_display_path(fb_params->ucDisplayInput_Path);
			#endif
		}
	}

	for (count = 0; count < (unsigned int)DISPLAY_CONTROLLER_MAX; count++) {
		fb_params = &stTCC_FB_Params_t[count];

		fb_params->stPanel_Info.lcdbase = fb_params->pvFB_BaseAddr;

		if (!fb_params->bActivated) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
			pr_info("[%s:%d]Info: Display controller %u is not activated\n",
				__func__, __LINE__, count);
			continue;
		}

		panel_id = (uint8_t)(fb_params->stPanel_Info.dev.platform_id & 0xFFU);

		if (lcd_panel_get_panel_info(fb_params->pcDevice_Name,
					     &fb_params->stPanel_Info) < 0) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err("[%s:%d]Error: Can not find panel information(%s)\n",
			       __func__, __LINE__,
			       fb_params->pcDevice_Name);
			break;
		}

		fb_params->bDevice_Connected = true;
		fb_params->stPanel_Info.lcdbase = fb_params->pvFB_BaseAddr;
		fb_params->stPanel_Info.dev.lcdc_id =
			fb_params->ucDisplayInput_Path;
		fb_params->stPanel_Info.dev.platform_id = panel_id;
		if (panel_id != (uint8_t)PANEL_HDMI) {
			fb_params->stPanel_Info.dev.lcdc_select =
				fb_params->ucDisplayLCD_Mux;
		}

		if (fb_params->bSDM_Bypass || fb_params->bSRVC_Bypass) {
			fb_params->stPanel_Info.dev.lcdc_bypass = 1;
		} else {
			fb_params->stPanel_Info.dev.lcdc_bypass = 0;
		}
		fb_params->stPanel_Info.dev.lcd_power =
			fb_params->ucDisplayLCD_Power;

		fb_params->keepgoing = (bool)0;

		#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
		fb_params->stPanel_Info.dev.uiDisplayPathNum = count;
		#endif

		switch (panel_id) {
		case PANEL_LVDS_DUAL:
		case PANEL_LVDS_SINGLE:
			lcd_panel_set_lvds_path(fb_params->pcDevice_Name,
					panel_id,
					fb_params->ucDisplayLCD_Port,
					fb_params->ucDisplayLCD_Mux,
					fb_params->stPanel_Info.dev.lcdc_bypass,
					&fb_params->stPanel_Info);
			break;
		default:
			/* Nothing to do */
			break;
		}

		init_panel_info(&fb_params->stPanel_Info);
		lcd_panel_set_panel_info(fb_params->pcDevice_Name,
					 &fb_params->stPanel_Info);

		if (panel_id == (uint8_t)PANEL_HDMI) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
			pr_info("\n[%s] %s is activated on HDMI\n",
				__func__,
				(count == (uint32_t)DISPLAY_CONTROLLER_0) ?
				"Display Controller0" :
				(count == (uint32_t)DISPLAY_CONTROLLER_1) ?
				"Display Controller1" :
				(count == (uint32_t)DISPLAY_CONTROLLER_2) ?
				"Display Controller2" :
				"Display Controller3");
		} else {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
			pr_info("\n[%s] %s is activated on DEV%d_PATH..\n",
				__func__,
				(count == (uint32_t)DISPLAY_CONTROLLER_0) ?
				"Display Controller0" :
				(count == (uint32_t)DISPLAY_CONTROLLER_1) ?
				"Display Controller1" :
				(count == (uint32_t)DISPLAY_CONTROLLER_2) ?
				"Display Controller2" :
				"Display Controller3",
				(int32_t)fb_params->ucDisplayLCD_Mux);
		}
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("                 Device name: %s\n",
			fb_params->pcDevice_Name);
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("                 FB Base: 0x%p on platform %u\n",
			fb_params->pvFB_BaseAddr,
			fb_params->stPanel_Info.dev
			.platform_id);
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("                 BMP Logo Base: 0x%08x\n",
			fb_params->stPanel_Info.dev.logo_data);
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("                 LCD Port: %d\n",
			(int)fb_params->ucDisplayLCD_Port);
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("                 LCD Power connector: %u\n",
			fb_params->ucDisplayLCD_Power + 1U);
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("                 Video code: %u\n",
			fb_params->uiDP_VideoCode);
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("                 Num Of DPs: %d\n\n",
			(int32_t)numofdp);

		switch (panel_id) {
		case PANEL_DP0:
		case PANEL_DP1:
		case PANEL_DP2:
		case PANEL_DP3:
			dp_params.bDP_PanelMode = fb_params->bDP_PanelMode;
			dp_params.bDP_SDMBypass = fb_params->bSDM_Bypass;
			dp_params.bDP_SRVCBypass = fb_params->bSRVC_Bypass;
			dp_params.bDP_PHY_LaneSwap = fb_params->bDP_PHYLaneSwap;
			dp_params.ucDP_Index = panel_id;
			dp_params.ucNumOfDPs = numofdp;
			dp_params.ucDP_I2CPort = fb_params->ucDP_PanelI2CPort;
			dp_params.uiVideoCode = fb_params->uiDP_VideoCode;
			(void)fb_params->stPanel_Info.init_ex(&dp_params);

			if (lcd_panel_get_panel_info(fb_params->pcDevice_Name,
						     &fb_params->stPanel_Info)
				< 0) {
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				pr_err("[%s:%d]Error: Can not find panel information(%s)\n",
				       __func__, __LINE__,
				       fb_params->pcDevice_Name);
			}
			fb_params->stPanel_Info.lcdbase =
				fb_params->pvFB_BaseAddr;
			fb_params->stPanel_Info.dev.lcdc_id =
				fb_params->ucDisplayInput_Path;
			fb_params->stPanel_Info.dev.platform_id = panel_id;
			fb_params->stPanel_Info.dev.lcdc_select =
				fb_params->ucDisplayLCD_Mux;

			#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
			fb_params->stPanel_Info.dev.uiDisplayPathNum = count;
			#endif
			break;
		default:
			if (!fb_params->keepgoing) {
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
				pr_info("[%s:%d]Panel clk_freq = %lu\r\n",
					__func__, __LINE__,
					fb_params->stPanel_Info.clk_freq);
				(void)fb_params->stPanel_Info.init();
			}
			break;
		}
		(void)memcpy(&panel_info, &fb_params->stPanel_Info,
		       sizeof(struct vidinfo));
	}
	fb_time_stame_log("%s line(%d)", __func__, __LINE__);
}

void lcd_enable(void)
{
	int iX = 0, iY = 0;
	struct TCC_FB_Params_t *pstTCC_FB_Params_t;

#if defined(CONFIG_SPLASH_SCREEN_ALIGN)
	iX = BMP_ALIGN_CENTER;
	iY = BMP_ALIGN_CENTER;
#endif

	fb_time_stame_log("%s line(%d)", __func__, __LINE__);
#if defined(CONFIG_BOOT_LOGO_LEGACY) && defined(CONFIG_CMD_BMP)
	/* Setting for flush lcd cache */
	lcd_set_flush_dcache(1);

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
	for (uint8_t ucCount = 0; ucCount < (uint8_t)DISPLAY_CONTROLLER_MAX;
	     ucCount++) {
		pstTCC_FB_Params_t = &stTCC_FB_Params_t[ucCount];
		if (pstTCC_FB_Params_t->bDevice_Connected) {
			(void)memcpy(&panel_info, &pstTCC_FB_Params_t->stPanel_Info,
			       sizeof(struct vidinfo));

			(void)bmp_display(
				pstTCC_FB_Params_t->stPanel_Info.dev.logo_data,
				iX, iY);
			flush_dcache_all(); // temporarily for cache logo
		}
	}
#else
	for (int iCount = (DISPLAY_CONTROLLER_MAX - 1);
	     iCount >= DISPLAY_CONTROLLER_0; iCount--) {
		pstTCC_FB_Params_t = &stTCC_FB_Params_t[iCount];
		if (pstTCC_FB_Params_t->bDevice_Connected) {
			(void)memcpy(&panel_info, &pstTCC_FB_Params_t->stPanel_Info,
			       sizeof(struct vidinfo));

			(void)bmp_display(
				pstTCC_FB_Params_t->stPanel_Info.dev.logo_data,
				iX, iY);
			break;
		}
	}

#endif /* #if defined( CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH ) */
#endif /* #if defined( CONFIG_BOOT_LOGO_LEGACY ) && defined( CONFIG_CMD_BMP ) \
	*/

	fb_time_stame_log("%s line(%d)", __func__, __LINE__);
#if defined(CONFIG_TCC803X)
#if defined(CONFIG_DC0_DUAL_PORT_LVDS) || defined(CONFIG_DC1_DUAL_PORT_LVDS) \
	|| defined(CONFIG_DC2_DUAL_PORT_LVDS)
	uint8_t ucPanelDL = DISPLAY_CONTROLLER_MAX;

#ifdef CONFIG_R5_LVDS_CTRL
	lvds_cnt++;
#endif
#if defined(CONFIG_DC0_DUAL_PORT_LVDS)
	ucPanelDL = (uint8_t)0;
#elif defined(CONFIG_DC1_DUAL_PORT_LVDS)
	ucPanelDL = (uint8_t)1;
#else
	ucPanelDL = (uint8_t)2;
#endif

	pstTCC_FB_Params_t = &stTCC_FB_Params_t[ucPanelDL];
	if (pstTCC_FB_Params_t->bDevice_Connected) {
		lcd_panel_on(&pstTCC_FB_Params_t->stPanel_Info, ucPanelDL);
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("Panel Dual LVDS init DC[%d]\n", ucPanelDL);
	}
#endif

#if defined(CONFIG_DC0_LCD_HDMIV20) || defined(CONFIG_DC1_LCD_HDMIV20) \
	|| defined(CONFIG_DC2_LCD_HDMIV20)
	uint8_t ucPanelHDMI = DISPLAY_CONTROLLER_MAX;

#ifdef CONFIG_R5_LVDS_CTRL
	hdmi_cnt++;
#endif
#if defined(CONFIG_DC0_LCD_HDMIV20)
	ucPanelHDMI = (uint8_t)0;
#elif defined(CONFIG_DC1_LCD_HDMIV20)
	ucPanelHDMI = (uint8_t)1;
#else
	ucPanelHDMI = (uint8_t)2;
#endif

	pstTCC_FB_Params_t = &stTCC_FB_Params_t[ucPanelHDMI];
	if (pstTCC_FB_Params_t->bDevice_Connected) {
		lcd_panel_on(&pstTCC_FB_Params_t->stPanel_Info, ucPanelHDMI);
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("\r\nPanel HDMI init DC[%d]\n", ucPanelHDMI);
	}
#endif
#ifdef CONFIG_R5_LVDS_CTRL
#if defined(CONFIG_DC0_SINGLE_PORT_LVDS)        \
	|| defined(CONFIG_DC1_SINGLE_PORT_LVDS) \
	|| defined(CONFIG_DC2_SINGLE_PORT_LVDS)
	lvds_cnt++;
#endif
#endif
	for (uint8_t ucCount = 0; ucCount < (uint8_t)DISPLAY_CONTROLLER_MAX;
	     ucCount++) {
#if defined(CONFIG_DC0_DUAL_PORT_LVDS) || defined(CONFIG_DC1_DUAL_PORT_LVDS) \
	|| defined(CONFIG_DC2_DUAL_PORT_LVDS)
		if (ucCount == ucPanelDL) {
			continue;
		}
#endif

#if defined(CONFIG_DC0_LCD_HDMIV20) || defined(CONFIG_DC1_LCD_HDMIV20) \
	|| defined(CONFIG_DC2_LCD_HDMIV20)
		if (ucCount == ucPanelHDMI) {
			continue;
		}
#endif
		pstTCC_FB_Params_t = &stTCC_FB_Params_t[ucCount];
		if (pstTCC_FB_Params_t->bDevice_Connected) {
			lcd_panel_on(
				&pstTCC_FB_Params_t->stPanel_Info, ucCount);
		}
	}
#else  // TCC805x
	for (uint8_t ucCount = 0; ucCount < (uint8_t)DISPLAY_CONTROLLER_MAX;
	     ucCount++) {
		pstTCC_FB_Params_t = &stTCC_FB_Params_t[ucCount];
		if (pstTCC_FB_Params_t->bDevice_Connected) {
			lcd_panel_on(
				&pstTCC_FB_Params_t->stPanel_Info, ucCount);
		}
	}
#endif // endif for defined(CONFIG_TCC803X)
	fb_time_stame_log("%s line(%d)", __func__, __LINE__);
}

void lcd_backlight(void)
{
	struct TCC_FB_Params_t *pstTCC_FB_Params_t;

	for (uint8_t ucCount = 0; ucCount < (uint8_t)DISPLAY_CONTROLLER_MAX;
	     ucCount++) {
		pstTCC_FB_Params_t = &stTCC_FB_Params_t[ucCount];
		if (pstTCC_FB_Params_t->bDevice_Connected) {
			lcd_panel_set_backlight(
				pstTCC_FB_Params_t->pcDevice_Name,
				DEFAULT_BACKLIGHT,
				&pstTCC_FB_Params_t->stPanel_Info);
		}
	}
}

void lcd_update_disable(void)
{
	struct TCC_FB_Params_t *pstTCC_FB_Params_t;

	for (uint8_t ucCount = 0; ucCount < (uint8_t)DISPLAY_CONTROLLER_MAX;
	     ucCount++) {
		pstTCC_FB_Params_t = &stTCC_FB_Params_t[ucCount];
		if (pstTCC_FB_Params_t->bDevice_Connected) {
			lcd_panel_set_backlight(
				pstTCC_FB_Params_t->pcDevice_Name, 0,
				&pstTCC_FB_Params_t->stPanel_Info);
		}
	}
}

/* dummy function */
void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
	(void)regno;
	(void)red;
	(void)green;
	(void)blue;
}

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)

//#define ENABLE_INDIVIDUAL_DEBUG
void lcd_panel_sync(uint8_t ucDisplay_Path)
{
	unsigned int uiTotalNumOfPanelLineBytes = 0, uiTotalNumOfPanelBytes = 0;
	const struct TCC_FB_Params_t *pstTCC_FB_Params_t =
		&stTCC_FB_Params_t[ucDisplay_Path];
	int done = 0;

	if (ucDisplay_Path >= (uint8_t)DISPLAY_CONTROLLER_MAX) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Invalid display path '%u'\n", __func__,
		       __LINE__, (uint32_t)ucDisplay_Path);
		done = 1;
	}

	if (done == 0) {
		if (pstTCC_FB_Params_t->bDevice_Connected) {
			lcd_panel_get_size(
				pstTCC_FB_Params_t, &uiTotalNumOfPanelLineBytes,
				&uiTotalNumOfPanelBytes);
			flush_dcache_range(
				/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
				(ulong)pstTCC_FB_Params_t->stPanel_Info.lcdbase,
				/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
				((ulong)pstTCC_FB_Params_t->stPanel_Info.lcdbase
				+ uiTotalNumOfPanelBytes));

#if defined(ENABLE_INDIVIDUAL_DEBUG)
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
			pr_info("[%s:%d]Flush dcache for FB - Display Controller %d\n",
				__func__, __LINE__, ucDisplay_Path);
#endif
		}
	}
}

void lcd_panel_set_background_color(int iBG_Color)
{
	unsigned int uiTotalNumOfPanelLineBytes, uiTotalNumOfPanelBytes;
	const struct TCC_FB_Params_t *pstTCC_FB_Params_t;

	for (uint8_t ucCount = 0; ucCount < (uint8_t)DISPLAY_CONTROLLER_MAX;
	     ucCount++) {
		pstTCC_FB_Params_t = &stTCC_FB_Params_t[ucCount];

		if (pstTCC_FB_Params_t->bDevice_Connected) {
			lcd_panel_get_size(
				pstTCC_FB_Params_t, &uiTotalNumOfPanelLineBytes,
				&uiTotalNumOfPanelBytes);

			if (uiTotalNumOfPanelBytes
			    > (unsigned int)CONFIG_SYS_VIDEO_LOGO_MAX_SIZE) {
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
				pr_info("[%s:%d]%s: the total number of panel pixel size(0x%x) is larger than Max. FB size( 8MB ) supported\n",
					__func__, __LINE__,
					(ucCount == (uint8_t)DISPLAY_CONTROLLER_0) ?
						"DC0" :
						(ucCount
						 == (uint8_t)DISPLAY_CONTROLLER_1) ?
						"DC1" :
						(ucCount
						 == (uint8_t)DISPLAY_CONTROLLER_2) ?
						"DC2" :
						"DC3",
					uiTotalNumOfPanelBytes);

				uiTotalNumOfPanelBytes =
					CONFIG_SYS_VIDEO_LOGO_MAX_SIZE;
			}

			/* coverity[cert_int31_c_violation : FALSE] */
			/* coverity[cert_int02_c_violation : FALSE] */
			(void)memset(pstTCC_FB_Params_t->stPanel_Info.lcdbase,
			       iBG_Color, uiTotalNumOfPanelBytes);

#if defined(ENABLE_INDIVIDUAL_DEBUG)
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
			pr_info("[%s:%d]Display Controller %d -> clear FB(0x%x) to 32Bits BG color(0x%x)\n",
				__func__, __LINE__, ucCount,
				(unsigned int)pstTCC_FB_Params_t->pvFB_BaseAddr,
				(unsigned int)iBG_Color);
#endif
		}
	}
}

struct vidinfo *lcd_get_panel_infor_path(uint8_t ucDisplay_Path)
{
	struct vidinfo *ret_val;
	int done = 0;

	if (ucDisplay_Path >= (uint8_t)DISPLAY_CONTROLLER_MAX) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Invalid display path '%u'\n", __func__,
		       __LINE__, (uint32_t)ucDisplay_Path);
		ret_val = NULL;
		done = 1;
	}

	if (done == 0) {
		if (!stTCC_FB_Params_t[ucDisplay_Path].bDevice_Connected) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err("[%s:%d]Device isn't connected '%u'\n", __func__,
			       __LINE__, (uint32_t)ucDisplay_Path);
			ret_val = NULL;
			done = 1;
		}
	}

	if (done == 0) {
		ret_val = &stTCC_FB_Params_t[ucDisplay_Path].stPanel_Info;
	}

	return ret_val;
}
#endif

