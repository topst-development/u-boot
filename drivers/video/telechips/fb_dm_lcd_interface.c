// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

/*****************************************************************************
 *
 * Header Files Include
 *
 ****************************************************************************/
#include <config.h>
#include <common.h>
#include <i2c.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <asm/arch/vioc/reg_physical.h>
#include <asm/arch/vioc/vioc_disp.h>
#include <asm/arch/vioc/vioc_rdma.h>
#include <asm/arch/vioc/vioc_wmix.h>
#include <asm/arch/vioc/vioc_wdma.h>
#include <asm/arch/vioc/vioc_scaler.h>
#include <asm/arch/vioc/vioc_config.h>
#if defined(CONFIG_TCC807X)
#include <asm/arch/vioc/vioc_ddicfg.h>
#endif

#include <video.h>
#include <display.h>
#include <telechips/fb_dm.h>
#include <telechips/fb_dm_lcd_interface.h>

#define TCC8059_EVB 0
#define TCC8050_EVB 1

static void lcdc_save_overlay_priority(void __iomem *wmix_reg_base,
				       unsigned int *priv_ovp)
{
	unsigned int current_ovp = 5u;

	if (wmix_reg_base != NULL) {
		VIOC_WMIX_GetOverlayPriority(wmix_reg_base, &current_ovp);
	}

	if (current_ovp != 5u) {
		/*
		 * If the ovp of wmix is not the reset value of 5, it is
		 * considered that someone else has changed the ovp of wmix.
		 */
		*priv_ovp = current_ovp;
	}
}

int lcdc_display_device_reset(const struct udevice *fb_dev)
{
	struct tcc_fb_dm_priv *priv = dev_get_priv(fb_dev);
	void __iomem *disp_reg_base = NULL;
	void __iomem *rdma_reg_base = NULL;
	void __iomem *wmix_reg_base = NULL;
	void __iomem *wdma_reg_base = NULL;
	int scaler_id = -1;
	int i, ret = 0;

	rdma_reg_base = VIOC_RDMA_GetAddress(priv->lcd_rdma);
	if (priv->lcd_wmix != 0u) {
		wmix_reg_base = VIOC_WMIX_GetAddress(priv->lcd_wmix);
	}
	disp_reg_base = VIOC_DISP_GetAddress(priv->lcd_disp);
	if (priv->lcd_wdma != 0u) {
		wdma_reg_base = VIOC_WDMA_GetAddress(priv->lcd_wdma);
	}

	if (wmix_reg_base != NULL) {
		lcdc_save_overlay_priority(wmix_reg_base, &priv->ovp);
	}

	if (wdma_reg_base != NULL) {
		VIOC_WDMA_SetImageDisable(wdma_reg_base);
	}

	scaler_id = VIOC_CONFIG_GetScaler_PluginToWDMA(priv->lcd_wdma);
	if (scaler_id >= 0) {
		debug_pr(LOGL_INFO, "[INFO][FB_DM_INT]Scaler %d was plugged to wdma %u\n",
			  scaler_id, get_vioc_index(priv->lcd_wdma));
		(void)VIOC_CONFIG_PlugOut((unsigned int)scaler_id);
	}

	if (disp_reg_base != NULL) {
		ret = vioc_disp_get_turn_onoff(disp_reg_base);
	}
	if ((ret != 0) && (rdma_reg_base != NULL)) {
		VIOC_RDMA_SetImageDisable(rdma_reg_base);
		vioc_disp_clean_dd_status(disp_reg_base);
		VIOC_DISP_TurnOff(disp_reg_base);
		for (i = 0; i < 30; i++) {
			ret = vioc_disp_get_dd_status(disp_reg_base);
			if (ret != 0) {
				/* For KCS */
				break;
			}
			mdelay(1);
		}

		debug_pr(LOGL_INFO,
			"[INFO][FB_DM_INT] It takes %dms until 'display disable done' interrupt occurs\r\n",
			i);
	}

	if (priv->lcd_wdma != 0u) {
		VIOC_CONFIG_SWReset(priv->lcd_wdma, (unsigned int)VIOC_CONFIG_RESET);
	}
	if (scaler_id >= 0) {
		/* For KCS */
		VIOC_CONFIG_SWReset((unsigned int)scaler_id, (unsigned int)VIOC_CONFIG_RESET);
	}
	VIOC_CONFIG_SWReset(priv->lcd_disp, (unsigned int)VIOC_CONFIG_RESET);
	if (priv->lcd_wmix != 0u) {
		VIOC_CONFIG_SWReset(priv->lcd_wmix, (unsigned int)VIOC_CONFIG_RESET);
	}
	VIOC_CONFIG_SWReset(priv->lcd_rdma, (unsigned int)VIOC_CONFIG_RESET);

	VIOC_CONFIG_SWReset(priv->lcd_rdma, (unsigned int)VIOC_CONFIG_CLEAR);
	if (priv->lcd_wmix != 0u) {
		VIOC_CONFIG_SWReset(priv->lcd_wmix, (unsigned int)VIOC_CONFIG_CLEAR);
	}
	VIOC_CONFIG_SWReset(priv->lcd_disp, (unsigned int)VIOC_CONFIG_CLEAR);
	if (scaler_id >= 0) {
		/* For KCS */
		VIOC_CONFIG_SWReset((unsigned)scaler_id, (unsigned int)VIOC_CONFIG_CLEAR);
	}
	if (priv->lcd_wdma != 0u) {
		VIOC_CONFIG_SWReset(priv->lcd_wdma, (unsigned int)VIOC_CONFIG_CLEAR);
	}

	return 0;
}

#if defined(CONFIG_TCC807X)
int lcdc_set_svsc_clk(const struct udevice *fb_dev)
{
	const struct tcc_fb_dm_priv *priv = dev_get_priv(fb_dev);
	if (priv->svsc_trvc_en == 1u) {
		VIOC_DDICONFIG_SetPeriClock(get_vioc_index(priv->lcd_disp), 2);
	} else if (priv->svsc_sdm_en == 1u) {
		VIOC_DDICONFIG_SetPeriClock(get_vioc_index(priv->lcd_disp), 1);
	} else {
		/* nothing */
	}

	return 0;
}

int lcdc_set_svsc_evs(const struct udevice *fb_dev)
{
	const struct tcc_fb_dm_priv *priv = dev_get_priv(fb_dev);

	if (priv->svsc_trvc_en == 1u) {
		(void)VIOC_CONFIG_SetEVS(0xB, get_vioc_index(priv->lcd_disp));
	} else if (priv->svsc_sdm_en == 1u) {
		(void)VIOC_CONFIG_SetEVS(0xA, get_vioc_index(priv->lcd_disp));
	} else {
		/* nothing*/
	}

	return 0;
}
#endif

int lcdc_turn_on_display(const struct udevice *fb_dev)
{
	const struct tcc_fb_dm_priv *priv = dev_get_priv(fb_dev);
	void __iomem *disp_reg_base;

	disp_reg_base = VIOC_DISP_GetAddress(priv->lcd_disp);

	if (disp_reg_base != NULL) {
		VIOC_DISP_TurnOn(disp_reg_base);
	}

	return 0;
}

//FIXME:
int lcdc_display_device_init(const struct udevice *fb_dev)
{
	struct tcc_fb_dm_priv *priv = dev_get_priv(fb_dev);
	struct stLTIMING stLcd_Timing_Params;
	struct stLCDCTR stLcd_Ctrl_Params;
	void __iomem *disp_reg_base = NULL;
	void __iomem *wmix_reg_base = NULL;

	unsigned int dalign_swappost = 0u;
	unsigned int wmix_ovp_max;
	unsigned int lcd_width;
	unsigned int lcd_height;

	disp_reg_base = VIOC_DISP_GetAddress(priv->lcd_disp);
	if (priv->lcd_wmix != 0u) {
		wmix_reg_base = VIOC_WMIX_GetAddress(priv->lcd_wmix);
	}

	wmix_ovp_max = (((unsigned int)MCTRL_OVP_MASK) >> MCTRL_OVP_SHIFT) - 1u;

	debug_pr(LOGL_DEBUG, "%s line(%d) wmix_ovp_max = %u\n", __func__, __LINE__, wmix_ovp_max);

	if (priv->ovp > wmix_ovp_max) {
		priv->ovp = 24U; //set default ovp value
	}

	VIOC_DISP_TurnOff(disp_reg_base);

	(void)memset(&stLcd_Ctrl_Params, 0, sizeof(stLcd_Ctrl_Params));
	(void)memset(&stLcd_Timing_Params, 0, sizeof(stLcd_Timing_Params));

	/* horizontal set */
	stLcd_Timing_Params.lpw = priv->pan_timing.hsync_len.typ;
	stLcd_Timing_Params.lpc = priv->pan_timing.hactive.typ;
	stLcd_Timing_Params.lswc = priv->pan_timing.hback_porch.typ;
	stLcd_Timing_Params.lewc = priv->pan_timing.hfront_porch.typ;

	/* vertical set */
	if (priv->interlaced != 0U) {
		stLcd_Timing_Params.fpw = (priv->pan_timing.vsync_len.typ - 1U);
		stLcd_Timing_Params.flc = (priv->pan_timing.vactive.typ - 1U);
		stLcd_Timing_Params.fswc = (priv->pan_timing.vback_porch.typ  - 1U);
		stLcd_Timing_Params.fewc = priv->pan_timing.vfront_porch.typ;

		stLcd_Timing_Params.fpw2 = stLcd_Timing_Params.fpw;
		stLcd_Timing_Params.flc2 = stLcd_Timing_Params.flc;
		/* CERT INT30-C */
		if (stLcd_Timing_Params.fswc < UINT_MAX) {
			stLcd_Timing_Params.fswc2 = stLcd_Timing_Params.fswc + 1U;
		} else {
			stLcd_Timing_Params.fswc2 = stLcd_Timing_Params.fswc;
		}
		stLcd_Timing_Params.fewc2 = (stLcd_Timing_Params.fewc > 0U) ?
			(stLcd_Timing_Params.fewc -1U) : 0U;
	} else {
		stLcd_Timing_Params.fpw = priv->pan_timing.vsync_len.typ - 1U;
		stLcd_Timing_Params.flc = priv->pan_timing.vactive.typ - 1U;
		stLcd_Timing_Params.fswc = priv->pan_timing.vback_porch.typ - 1U;
		stLcd_Timing_Params.fewc = priv->pan_timing.vfront_porch.typ - 1U;

		stLcd_Timing_Params.fpw2 = stLcd_Timing_Params.fpw;
		stLcd_Timing_Params.flc2 = stLcd_Timing_Params.flc;
		stLcd_Timing_Params.fswc2 = stLcd_Timing_Params.fswc;
		stLcd_Timing_Params.fewc2 = stLcd_Timing_Params.fewc;
	}

	debug_pr(LOGL_INFO, "  lpw = %u\n", (u32)stLcd_Timing_Params.lpw);
	debug_pr(LOGL_INFO, "  lpc = %u\n", (u32)stLcd_Timing_Params.lpc);
	debug_pr(LOGL_INFO, "  lswc = %u\n", (u32)stLcd_Timing_Params.lswc);
	debug_pr(LOGL_INFO, "  lewc = %u\n", (u32)stLcd_Timing_Params.lewc);
	debug_pr(LOGL_INFO, "  fpw = %u\n", (u32)stLcd_Timing_Params.fpw);
	debug_pr(LOGL_INFO, "  flc = %u\n", (u32)stLcd_Timing_Params.flc);
	debug_pr(LOGL_INFO, "  fswc = %u\n", (u32)stLcd_Timing_Params.fswc);
	debug_pr(LOGL_INFO, "  fewc = %u\n", (u32)stLcd_Timing_Params.fewc);

	VIOC_DISP_SetTimingParam(disp_reg_base, &stLcd_Timing_Params);

	stLcd_Ctrl_Params.iv = (((unsigned)priv->pan_timing.flags & (unsigned)DISPLAY_FLAGS_VSYNC_LOW) != 0U) ? 1U : 0U;
	stLcd_Ctrl_Params.ih = (((unsigned)priv->pan_timing.flags & (unsigned)DISPLAY_FLAGS_HSYNC_LOW) != 0U) ? 1U : 0U;
	stLcd_Ctrl_Params.id = (((unsigned)priv->pan_timing.flags & (unsigned)DISPLAY_FLAGS_DE_LOW) != 0U) ? 1U : 0U;
	stLcd_Ctrl_Params.dp = priv->pixel_repetition_input;

	switch (priv->lcd_pxdw) {
	case (uint8_t)PIXEL_ENCODING_RGB:
		stLcd_Ctrl_Params.pxdw = (unsigned int)VIOC_PXDW_FMT_24_RGB888;
		stLcd_Ctrl_Params.r2y = 0U;
		break;
	case (uint8_t)PIXEL_ENCODING_YCBCR422:
		#if defined(CONFIG_TCC807X)
		/*
		 * The Displayport in TCC807x_CS should be set pxdw to 9 when
		 * output as YUV422
		 */
		stLcd_Ctrl_Params.pxdw = (unsigned int)VIOC_PXDW_FMT_16_YV;
		dalign_swappost = 3u;
		#else
		stLcd_Ctrl_Params.pxdw = (unsigned int)VIOC_PXDW_FMT_16_YU;
		#endif
		stLcd_Ctrl_Params.r2y = 1U;
		break;
	case (uint8_t)PIXEL_ENCODING_YCBCR444:
		stLcd_Ctrl_Params.pxdw = (unsigned int)VIOC_PXDW_FMT_24_RGB888;
		stLcd_Ctrl_Params.r2y = 1U;
		break;
	default:
		stLcd_Ctrl_Params.pxdw = (unsigned int)VIOC_PXDW_FMT_24_RGB888;
		stLcd_Ctrl_Params.r2y = 0U;
		debug_pr(LOGL_WARNING, "\n[%s]Err: Invalid DP id as %u, set default pwdx[%d] r2y[%d]\n", __func__, __LINE__, stLcd_Ctrl_Params.pxdw, stLcd_Ctrl_Params.r2y);
		break;
	}

#if defined(CONFIG_TCC807X)
	if ((priv->svsc_trvc_en == 1u) || (priv->svsc_sdm_en == 1u)) {
		stLcd_Ctrl_Params.svscen = 1U;
	}
#endif

	if (priv->interlaced != 0U) {
		stLcd_Ctrl_Params.tv = 1U;
	} else {
		stLcd_Ctrl_Params.ni = 1U; //not interlaced
	}

	lcd_width = (priv->pixel_repetition_input != 0U) ?
		(priv->pan_timing.hactive.typ >> 1U) : priv->pan_timing.hactive.typ;

	lcd_height = priv->pan_timing.vactive.typ;

	VIOC_DISP_SetSwapaf(disp_reg_base, dalign_swappost);
	VIOC_DISP_SetControlConfigure(disp_reg_base, &stLcd_Ctrl_Params);

	/* wmixer control */
	if (wmix_reg_base != NULL) {
		VIOC_WMIX_SetOverlayPriority(wmix_reg_base, priv->ovp);
		VIOC_WMIX_SetBGColor(wmix_reg_base, 0, 0, 0, 0);
		VIOC_WMIX_SetSize(wmix_reg_base, lcd_width, lcd_height);
		VIOC_WMIX_SetUpdate(wmix_reg_base);
	}

	VIOC_DISP_SetSize(disp_reg_base, lcd_width, lcd_height);
	VIOC_DISP_SetBGColor(disp_reg_base, 0, 0, 0, 0);

	return 0;
}

int lcdc_mux_select(unsigned int mux_select, unsigned int lcd_disp)
{
	unsigned int misc_val;
	#if defined(CONFIG_TCC807X)
	unsigned int misc2_val;
	#endif

	unsigned int shift_lcd_sel = 0u;
	unsigned int mask_lcd_sel = 0x3u;
	unsigned int set_lcd_sel = 0u;
	int ret = 0;

	void __iomem *vioc_config_base = VIOC_IREQConfig_GetAddress();

	if ((mux_select >= (unsigned int)PANEL_LCD_MUX_MAX) ||
				(lcd_disp >= (unsigned int)PANEL_PATH_MAX)) {
		debug_pr(LOGL_ERR, "[%s:%d]Error: Invalid params - lcd mux id(%u), display input(%u)\n",
		       __func__, __LINE__, mux_select, lcd_disp);
		ret = -1;
	}

	#if defined(CONFIG_TCC807X)
	if (ret == 0) {
		misc_val = __raw_readl(vioc_config_base + CFG_MISC1_OFFSET);
		misc2_val = __raw_readl(vioc_config_base + CFG_MISC2_OFFSET);

		switch (mux_select) {
		case 0:
			shift_lcd_sel = 24U;
			mask_lcd_sel = 0x07000000U;
			set_lcd_sel = (lcd_disp << shift_lcd_sel);
			misc_val &= ~mask_lcd_sel;
			misc_val |= set_lcd_sel;
			break;
		case 1:
			shift_lcd_sel = 28U;
			mask_lcd_sel = 0x70000000U;
			set_lcd_sel = (lcd_disp << shift_lcd_sel);
			misc_val &= ~mask_lcd_sel;
			misc_val |= set_lcd_sel;
			break;
		case 2:
			shift_lcd_sel = 0U;
			mask_lcd_sel = 0x00000007U;
			set_lcd_sel = (lcd_disp << shift_lcd_sel);
			misc2_val &= ~mask_lcd_sel;
			misc2_val |= set_lcd_sel;
			break;
		case 3:
			shift_lcd_sel = 4U;
			mask_lcd_sel = 0x00000070U;
			set_lcd_sel = (lcd_disp << shift_lcd_sel);
			misc2_val &= ~mask_lcd_sel;
			misc2_val |= set_lcd_sel;
			break;
		case 4:
			shift_lcd_sel = 8U;
			mask_lcd_sel = 0x00000700U;
			set_lcd_sel = (lcd_disp << shift_lcd_sel);
			misc2_val &= ~mask_lcd_sel;
			misc2_val |= set_lcd_sel;
			break;
		default:
			debug_pr(LOGL_ERR, "[ERR][%s:%d] Can't find mux_select %d !\n", __func__, __LINE__, mux_select);
			ret = -1;
			break;
		}

		if (ret == 0) {
			__raw_writel(misc_val, vioc_config_base + CFG_MISC1_OFFSET);
			__raw_writel(misc2_val, vioc_config_base + CFG_MISC2_OFFSET);
		}
	}
	#else
	if (ret == 0) {
		misc_val = __raw_readl(vioc_config_base + CFG_MISC1_OFFSET);
		shift_lcd_sel = (mux_select * 2U) + 24U;
		mask_lcd_sel = mask_lcd_sel << shift_lcd_sel; //0b11

		//clear lcd_sel
		misc_val &= ~mask_lcd_sel;

		set_lcd_sel = lcd_disp << shift_lcd_sel; //0b11
		misc_val |= set_lcd_sel;

		__raw_writel(misc_val, vioc_config_base + CFG_MISC1_OFFSET);
	}
	#endif

	return ret;
}

void tcc_lcdc_color_setting(const struct tcc_fb_dm_priv *priv,
			    unsigned int contrast,
			    unsigned int brightness,
			    unsigned int hue)
{
	void __iomem *disp_reg_base = VIOC_DISP_GetAddress(priv->lcd_disp);

	if (disp_reg_base != NULL) {
		VIOC_DISP_SetColorEnhancement(disp_reg_base, contrast, brightness, hue);

		debug_pr(LOGL_INFO, "[%s:%d]lcdc:%u c :%d , b:%d h:%d\n", __func__,
			__LINE__, (unsigned int)get_vioc_index(priv->lcd_disp),
			contrast, brightness, hue);
	}
}

void tcc_lcdc_dithering_setting(const struct tcc_fb_dm_priv *priv)
{
	void __iomem *disp_reg_base;
	unsigned int value;

	disp_reg_base = VIOC_DISP_GetAddress(priv->lcd_disp);

	if (disp_reg_base != NULL) {
		/* dithering option */
		value = (__raw_readl(disp_reg_base + DCTRL) & ~(DCTRL_PXDW_MASK));
		value |= (0x5U << DCTRL_PXDW_SHIFT);
		__raw_writel(value, disp_reg_base + DCTRL);

		value = ((0x1U << DDITH_DEN_SHIFT) | (0x1U << DDITH_DSEL_SHIFT));
		__raw_writel(value, disp_reg_base + DDITH);
		__raw_writel(0x6e4ca280, disp_reg_base + DDMAT0);
		__raw_writel(0x5d7f91b3, disp_reg_base + DDMAT1);
	}
}

void tcclcd_image_ch_set(const struct tcc_fb_dm_priv *priv)
{
	uint32_t base_addr0, base_addr1, base_addr2;
	void __iomem *rdma_reg_base = NULL;
	void __iomem *wmix_reg_base = NULL;

	int win_channel, ret = 0;

	rdma_reg_base = VIOC_RDMA_GetAddress(priv->lcd_rdma);
	if (priv->lcd_wmix != 0u) {
		wmix_reg_base = VIOC_WMIX_GetAddress(priv->lcd_wmix);
	}

	if (rdma_reg_base == NULL) {
		debug_pr(LOGL_ERR, "[%s:%d]Error: Invaild lcdc number(%u)\n", __func__,
		       __LINE__, get_vioc_index(priv->lcd_rdma));
		ret = -EINVAL;
	}

	if (ret == 0) {
		VIOC_RDMA_SetImageY2RMode(rdma_reg_base, 0); /* Y2RMode Default 0 (Studio Color) */

		if ((priv->image_info.fmt >= TCC_LCDC_IMG_FMT_444SEP)
		    && (priv->image_info.fmt <= TCC_LCDC_IMG_FMT_YUV422ITL1)) {
#if defined(LCD_DISPLAY_Y2R_ENABLE)
			VIOC_RDMA_SetImageR2YEnable(rdma_reg_base, 0U);
			VIOC_RDMA_SetImageY2REnable(rdma_reg_base, 0U);
#else
			VIOC_RDMA_SetImageR2YEnable(rdma_reg_base, 0U);
			VIOC_RDMA_SetImageY2REnable(rdma_reg_base, 1U);
#endif
		} else {
#if defined(LCD_DISPLAY_Y2R_ENABLE)
			VIOC_RDMA_SetImageR2YEnable(rdma_reg_base, 1U);
			VIOC_RDMA_SetImageY2REnable(rdma_reg_base, 0U);
#else
			VIOC_RDMA_SetImageR2YEnable(rdma_reg_base, 0U);
			VIOC_RDMA_SetImageY2REnable(rdma_reg_base, 0U);
#endif
		}

		VIOC_RDMA_SetImageOffset(rdma_reg_base, (unsigned int)priv->image_info.fmt,
					 priv->image_info.image_width);
		VIOC_RDMA_SetImageFormat(rdma_reg_base, (unsigned int)priv->image_info.fmt);
		VIOC_RDMA_SetImageRGBSwapMode(rdma_reg_base, 0); /* R-G-B */
		VIOC_RDMA_SetImageSize(
				       rdma_reg_base, priv->image_info.image_width, priv->image_info.image_height);

		/* image address */
		base_addr0 = (uint32_t)((unsigned long)priv->image_info.buf_addr0 & UINT_MAX);
		base_addr1 = (uint32_t)((unsigned long)priv->image_info.buf_addr1 & UINT_MAX);
		base_addr2 = (uint32_t)((unsigned long)priv->image_info.buf_addr2 & UINT_MAX);
		VIOC_RDMA_SetImageBase(
				       rdma_reg_base, base_addr0, base_addr1, base_addr2);

		/* Disable Alpha blend */
		VIOC_RDMA_SetImageAlphaEnable(rdma_reg_base, 0);

		win_channel = VIOC_RDMA_GetImageNum(priv->lcd_rdma);

		if (wmix_reg_base != NULL) {
			if (win_channel >= 0) {
				VIOC_WMIX_SetPosition(wmix_reg_base, (unsigned int)win_channel, priv->image_info.offset_x,
						      priv->image_info.offset_y);
			} else {
				debug_pr(LOGL_ERR, "[ERR] win_channel value is %d\n", win_channel);
			}
		}

		/* image enable */
		if (priv->image_info.enable != 0U) {
			VIOC_RDMA_SetImageEnable(rdma_reg_base);
		} else {
			VIOC_RDMA_SetImageDisable(rdma_reg_base);
		}
		if (wmix_reg_base != NULL) {
			VIOC_WMIX_SetUpdate(wmix_reg_base);
		}
	}
}

void tcclcd_gpioexp_set_value(unsigned int on)
{
	struct udevice *pDEV_I2C_Dev;
	struct udevice *pstBus = NULL;
	unsigned char addr, data, val;
	int ret;
	int done_flag = 0;

	addr = 0x5a;

	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &pstBus);
	if (ret != 0) {
		debug_pr(LOGL_ERR, "[%s][%d]failed to get i2c device class\n", __func__,
		       __LINE__);
		done_flag = 1;
	}
	if (done_flag == 0) {
		ret = dm_i2c_set_bus_speed(pstBus, 400000);
		if (ret != 0) {
			debug_pr(LOGL_ERR, "\n[%s:%d]Error: from dm_i2c_set_bus_speed().. Ret(0x%x)\n",
			       __func__, __LINE__, (unsigned int)ret);
			done_flag = 1;
		}
	}
	if (done_flag == 0) {
		ret = i2c_get_chip_for_busnum(0, (int)addr, 1, &pDEV_I2C_Dev);
		if (ret != 0) {
			debug_pr(LOGL_ERR, "[%s][%d]failed to get i2c handle\n", __func__,
			       __LINE__);
			done_flag = 1;
		}
	}

	if (done_flag == 0) {
		(void)dm_i2c_read(pDEV_I2C_Dev, 0x1A, &val, 1);
		data = 0x6; // GPIO10_TYPE[2] GPIO10_PIN[1:0]
		val &= ~data;

		if (on != 0U) {
			val |= data;
		} else {
			val |= (unsigned char)0x1U;
		}
		(void)dm_i2c_write(pDEV_I2C_Dev, 0x1A, &val, 1);
		mdelay(1);

		(void)dm_i2c_read(pDEV_I2C_Dev, 0x1E, &val, 1);
		data = 0x04; // GPIO10_MODE[2]
		val &= ~data;

		if (on != 0U) {
			val |= data;
		}
		(void)dm_i2c_write(pDEV_I2C_Dev, 0x1E, &val, 1);
	}
}

struct udevice *tcc_get_remote_dev(const struct udevice *dev)
{
	int ret = 0;
	struct udevice *remote_dev = NULL;
	u32 remote_phandle;
	ofnode remote;
	ofnode ports, port, node;

	ports = dev_read_subnode(dev, "ports");
	if (!ofnode_valid(ports)) {
		ports = dev_ofnode(dev);
	}

	port = ofnode_find_subnode(ports, "port");
	if (!ofnode_valid(port)) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : 'port' subnode not found\n",
		      __func__, dev_read_name(dev));
		ret = -EINVAL;
	} else {
		node = ofnode_first_subnode(port);
		ret = ofnode_read_u32(node, "remote-endpoint", &remote_phandle);
		if ((bool)ret) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : ret = %d\n",
				__func__, dev_read_name(dev), ret);
		}
	}

	if (ret == 0) {
		remote = ofnode_get_by_phandle(remote_phandle);
		if (!ofnode_valid(remote)) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : node not valid\n",
				__func__, dev_read_name(dev));
			ret = -EINVAL;
		}

		while (ret == 0) {
			remote = ofnode_get_parent(remote);
			if (!ofnode_valid(remote)) {
				debug_pr(LOGL_ERR, "[ERROR][%s](%s) : no UCLASS_VIDEO for remote-endpoint\n",
					__func__, dev_read_name(dev));
				ret = -EINVAL;
			} else {
				(void)uclass_find_device_by_ofnode(UCLASS_VIDEO, remote, &remote_dev);
				if (remote_dev != NULL) {
					break;
				}
			}
		}
	}
	return remote_dev;
}
