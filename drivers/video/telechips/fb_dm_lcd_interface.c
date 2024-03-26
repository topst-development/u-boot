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
#include <lcd.h>
#include <dm.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <asm/arch/vioc/reg_physical.h>
#include <asm/arch/vioc/vioc_disp.h>
#include <asm/arch/vioc/vioc_rdma.h>
#include <asm/arch/vioc/vioc_wmix.h>
#include <asm/arch/vioc/vioc_wdma.h>
#include <asm/arch/vioc/vioc_scaler.h>
#include <asm/arch/vioc/vioc_config.h>

#include <video.h>
#include <display.h>
#include <telechips/fb_dm.h>

#define TCC8059_EVB 0
#define TCC8050_EVB 1

#define CFG_MISC1 (HwVIOC_CONFIG + 0x84UL)

#if defined(CONFIG_TCC807X)
#define CFG_MISC2 (HwVIOC_CONFIG + 0x88UL)
#endif

#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
static void lcdc_save_overlay_priority(void __iomem *WMixerBaseAddr,
				       unsigned int *ovp)
{
	VIOC_WMIX_GetOverlayPriority(WMixerBaseAddr, ovp);

	pr_info("[%s] : previous ovp value = %u\n",__func__, *ovp);

	if (*ovp == 5U) { //if reset value
		/* For coverity */
		*ovp = 24U; //set default ovp value
	}
}

int lcdc_display_device_reset(struct udevice *fb_dev)
{
	struct tcc_fb_dm_priv *priv = dev_get_priv(fb_dev);
	void __iomem *disp_reg_base, *rdma_reg_base;
	void __iomem *wmix_reg_base, *wdma_reg_base;
	uint32_t disp_id = 0, rdma_id = 0;
	uint32_t wmix_id = 0, wdma_id = 0;
	int scaler_id = -1;
	int i, ret = 0;

	pr_info("[%s] : lcd_id = %u\n", __func__, priv->lcd_id);

	disp_id = ((unsigned int)VIOC_DISP0 + (unsigned int)(priv->lcd_id & 0xFFU));
	wdma_id = ((unsigned int)VIOC_WDMA00 + (unsigned int)(priv->lcd_id & 0xFFU));
	wmix_id = ((unsigned int)VIOC_WMIX0 + (unsigned int)(priv->lcd_id & 0xFFU));
	rdma_id = ((unsigned int)VIOC_RDMA00 + ((unsigned int)(priv->lcd_id & 0xFFU) << 2U));

	disp_reg_base = VIOC_DISP_GetAddress(disp_id);
	wmix_reg_base = VIOC_WMIX_GetAddress(wmix_id);
	wdma_reg_base = VIOC_WDMA_GetAddress(wdma_id);
	rdma_reg_base = VIOC_RDMA_GetAddress(rdma_id);

	lcdc_save_overlay_priority(wmix_reg_base, &priv->ovp);

	VIOC_WDMA_SetImageDisable(wdma_reg_base);

	scaler_id = VIOC_CONFIG_GetScaler_PluginToWDMA(wdma_id);
	if (scaler_id >= 0) {
		pr_notice("[INFO][FB_DM_INT]Scaler %d was plugged to wdma %u\n", scaler_id, priv->lcd_id);
		VIOC_CONFIG_PlugOut(scaler_id);
	}

	ret = vioc_disp_get_turn_onoff(disp_reg_base);
	if (ret != 0) {
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

		pr_notice(
			"[INFO][FB_DM_INT] It takes %dms until 'display disable done' interrupt occurs\r\n",
			i);
	}

	VIOC_CONFIG_SWReset(wdma_id, (unsigned int)VIOC_CONFIG_RESET);
	if (scaler_id >= 0) {
		/* For KCS */
		VIOC_CONFIG_SWReset(scaler_id, (unsigned int)VIOC_CONFIG_RESET);
	}
	VIOC_CONFIG_SWReset(disp_id, (unsigned int)VIOC_CONFIG_RESET);
	VIOC_CONFIG_SWReset(wmix_id, (unsigned int)VIOC_CONFIG_RESET);
	VIOC_CONFIG_SWReset(rdma_id, (unsigned int)VIOC_CONFIG_RESET);

	VIOC_CONFIG_SWReset(rdma_id, (unsigned int)VIOC_CONFIG_CLEAR);
	VIOC_CONFIG_SWReset(wmix_id, (unsigned int)VIOC_CONFIG_CLEAR);
	VIOC_CONFIG_SWReset(disp_id, (unsigned int)VIOC_CONFIG_CLEAR);
	if (scaler_id >= 0) {
		/* For KCS */
		VIOC_CONFIG_SWReset(scaler_id, (unsigned int)VIOC_CONFIG_CLEAR);
	}
	VIOC_CONFIG_SWReset(wdma_id, (unsigned int)VIOC_CONFIG_CLEAR);

	return 0;
}
#endif //#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)

int lcdc_turn_on_display(struct udevice *fb_dev)
{
	struct tcc_fb_dm_priv *priv = dev_get_priv(fb_dev);
	void __iomem *pioDisplayBaseAddr;

	/* coverity[cert_int36_c_violation : FALSE] */
	pioDisplayBaseAddr = (void __iomem *)TCC_VIOC_DISP_BASE(priv->lcd_id);

	VIOC_DISP_TurnOn(pioDisplayBaseAddr);

	return 0;
}

//FIXME:
int lcdc_display_device_init(struct udevice *fb_dev)
{
	struct stLTIMING stLcd_Timing_Params;
	struct stLCDCTR stLcd_Ctrl_Params;
	void __iomem *pioDisplayBaseAddr, *pioWMixerBaseAddr;
	struct tcc_fb_dm_priv *priv = dev_get_priv(fb_dev);
	unsigned int lcd_width;
	unsigned int lcd_height;
	unsigned int addr_int_display;
	unsigned int addr_int_wmix;

	/* coverity[cert_int36_c_violation : FALSE] */
	pioDisplayBaseAddr = (void __iomem *)TCC_VIOC_DISP_BASE(priv->lcd_id);
	if (VIOC_RDMA_HAS_WMIX(priv->lcd_id)) {
		/* coverity[cert_int36_c_violation : FALSE] */
		pioWMixerBaseAddr = (void __iomem *)TCC_VIOC_WMIX_BASE(priv->lcd_id);
	} else {
		pioWMixerBaseAddr = NULL; //no wmix path
	}

	addr_int_display = (unsigned int)((unsigned long)pioDisplayBaseAddr & UINT_MAX);
	addr_int_wmix = (unsigned int)((unsigned long)pioWMixerBaseAddr & UINT_MAX);

	if (priv->ovp == 0U) {
		/* For coverity */
		priv->ovp = 24U; //set default ovp value
	}

	pr_info("[%s] reg DISP : 0x%x, WMIX : 0x%x ovp : %d\n",
		__func__, addr_int_display, addr_int_wmix,
		priv->ovp);

	VIOC_DISP_TurnOff(pioDisplayBaseAddr);

	(void)memset(&stLcd_Ctrl_Params, 0, sizeof(stLcd_Ctrl_Params));
	(void)memset(&stLcd_Timing_Params, 0, sizeof(stLcd_Timing_Params));

	/* horizontal set */
	stLcd_Timing_Params.lpw = priv->pan_timing.hsync_len.typ;
	stLcd_Timing_Params.lpc = priv->pan_timing.hactive.typ;
	stLcd_Timing_Params.lswc = priv->pan_timing.hback_porch.typ;
	stLcd_Timing_Params.lewc = priv->pan_timing.hfront_porch.typ;

	/* vertical set */
	if (priv->interlaced != 0U) {
		stLcd_Timing_Params.fpw = (priv->pan_timing.vsync_len.typ - 1);
		stLcd_Timing_Params.flc = (priv->pan_timing.vactive.typ - 1);
		stLcd_Timing_Params.fswc = (priv->pan_timing.vback_porch.typ  - 1);
		stLcd_Timing_Params.fewc = priv->pan_timing.vfront_porch.typ;

		stLcd_Timing_Params.fpw2 = stLcd_Timing_Params.fpw;
		stLcd_Timing_Params.flc2 = stLcd_Timing_Params.flc;
		/* CERT INT30-C */
		if(stLcd_Timing_Params.fswc < UINT_MAX) {
			stLcd_Timing_Params.fswc2 = stLcd_Timing_Params.fswc + 1U;
		} else {
			stLcd_Timing_Params.fswc2 = stLcd_Timing_Params.fswc;
		}
		stLcd_Timing_Params.fewc2 = (stLcd_Timing_Params.fewc > 0U) ?
			(stLcd_Timing_Params.fewc -1U) : 0U;
	} else {
		stLcd_Timing_Params.fpw = priv->pan_timing.vsync_len.typ - 1;
		stLcd_Timing_Params.flc = priv->pan_timing.vactive.typ - 1;
		stLcd_Timing_Params.fswc = priv->pan_timing.vback_porch.typ - 1;
		stLcd_Timing_Params.fewc = priv->pan_timing.vfront_porch.typ - 1;

		stLcd_Timing_Params.fpw2 = stLcd_Timing_Params.fpw;
		stLcd_Timing_Params.flc2 = stLcd_Timing_Params.flc;
		stLcd_Timing_Params.fswc2 = stLcd_Timing_Params.fswc;
		stLcd_Timing_Params.fewc2 = stLcd_Timing_Params.fewc;
	}

	pr_info("  lpw = %u\n", (u32)stLcd_Timing_Params.lpw);
	pr_info("  lpc = %u\n", (u32)stLcd_Timing_Params.lpc);
	pr_info("  lswc = %u\n", (u32)stLcd_Timing_Params.lswc);
	pr_info("  lewc = %u\n", (u32)stLcd_Timing_Params.lewc);
	pr_info("  fpw = %u\n", (u32)stLcd_Timing_Params.fpw);
	pr_info("  flc = %u\n", (u32)stLcd_Timing_Params.flc);
	pr_info("  fswc = %u\n", (u32)stLcd_Timing_Params.fswc);
	pr_info("  fewc = %u\n", (u32)stLcd_Timing_Params.fewc);

	VIOC_DISP_SetTimingParam(pioDisplayBaseAddr, &stLcd_Timing_Params);

	stLcd_Ctrl_Params.iv = ((priv->pan_timing.flags & DISPLAY_FLAGS_VSYNC_LOW) != 0) ? 1 : 0;
	stLcd_Ctrl_Params.ih = ((priv->pan_timing.flags & DISPLAY_FLAGS_HSYNC_LOW) != 0) ? 1 : 0;
	stLcd_Ctrl_Params.id = ((priv->pan_timing.flags & DISPLAY_FLAGS_DE_LOW) != 0) ? 1 : 0;
	stLcd_Ctrl_Params.dp = priv->pixel_repetition_input;

	switch(priv->lcd_pxdw){
	case (uint8_t)PIXEL_ENCODING_RGB:
		stLcd_Ctrl_Params.pxdw = (unsigned int)DCTRL_PXDW_RGB;
		stLcd_Ctrl_Params.r2y = 0U;
		break;
	case (uint8_t)PIXEL_ENCODING_YCBCR422:
		stLcd_Ctrl_Params.pxdw = (unsigned int)DCTRL_PXDW_YCBCR422;
		stLcd_Ctrl_Params.r2y = 1U;
		break;
	case (uint8_t)PIXEL_ENCODING_YCBCR444:
		stLcd_Ctrl_Params.pxdw = (unsigned int)DCTRL_PXDW_YCBCR444;
		stLcd_Ctrl_Params.r2y = 1U;
		break;
	default:
		pr_err("\n[%s]Err: Invalid DP id as %u\n", __func__, __LINE__);
		stLcd_Ctrl_Params.pxdw = (unsigned int)DCTRL_PXDW_RGB;
		stLcd_Ctrl_Params.r2y = 0U;
		break;
	}


	if(priv->interlaced != 0U) {
		stLcd_Ctrl_Params.tv = 1U;
	} else {
		stLcd_Ctrl_Params.ni = 1U; //not interlaced
	}

	lcd_width = (priv->pixel_repetition_input != 0U) ?
		(priv->pan_timing.hactive.typ >> 1U) : priv->pan_timing.hactive.typ;

	lcd_height = priv->pan_timing.vactive.typ;

	VIOC_DISP_SetControlConfigure(pioDisplayBaseAddr, &stLcd_Ctrl_Params);

	/* wmixer control */
	VIOC_WMIX_SetOverlayPriority(pioWMixerBaseAddr, priv->ovp);
	VIOC_WMIX_SetBGColor(pioWMixerBaseAddr, 0, 0, 0, 0);
	VIOC_WMIX_SetSize(pioWMixerBaseAddr, lcd_width, lcd_height);
	VIOC_WMIX_SetUpdate(pioWMixerBaseAddr);

	VIOC_DISP_SetSize(pioDisplayBaseAddr, lcd_width, lcd_height);
	VIOC_DISP_SetBGColor(pioDisplayBaseAddr, 0, 0, 0, 0);

	return 0;
}

int lcdc_mux_select(unsigned int mux_id, unsigned int lcdc_id)
{
#if defined(CONFIG_TCC807X)
	unsigned int misc1_val, misc2_val;
#else
	unsigned int val;
#endif
	int ret = 0;
	unsigned int shift_lcd_sel = 0;
	unsigned int mask_lcd_sel = 0x3u;
	unsigned int set_lcd_sel = 0;
	/* coverity[cert_int36_c_violation : FALSE] */
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	void __iomem* pcfg_misc1 = (void __iomem*)CFG_MISC1;
#if defined(CONFIG_TCC807X)
	void __iomem* pcfg_misc2 = (void __iomem*)CFG_MISC2;
#endif

	if ((mux_id >= (unsigned int)PANEL_LCD_MUX_MAX) ||
				(lcdc_id >= (unsigned int)PANEL_PATH_MAX)) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Invalid params - lcd mux id(%u), display input(%u)\n",
		       __func__, __LINE__, mux_id, lcdc_id);
		ret = -1;
	}

#if defined(CONFIG_TCC807X)
	if (ret == 0) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		misc1_val = __raw_readl(pcfg_misc1);
		misc2_val = __raw_readl(pcfg_misc2);

		switch (mux_id) {
		case 0:
			shift_lcd_sel = 24U;
			mask_lcd_sel = 0x07000000U;
			set_lcd_sel = (lcdc_id << shift_lcd_sel);
			misc1_val &= ~mask_lcd_sel;
			misc1_val |= set_lcd_sel;
			break;
		case 1:
			shift_lcd_sel = 28U;
			mask_lcd_sel = 0x70000000U;
			set_lcd_sel = (lcdc_id << shift_lcd_sel);
			misc1_val &= ~mask_lcd_sel;
			misc1_val |= set_lcd_sel;
			break;
		case 2:
			shift_lcd_sel = 0U;
			mask_lcd_sel = 0x00000007U;
			set_lcd_sel = (lcdc_id << shift_lcd_sel);
			misc2_val &= ~mask_lcd_sel;
			misc2_val |= set_lcd_sel;
			break;
		case 3:
			shift_lcd_sel = 4U;
			mask_lcd_sel = 0x00000070U;
			set_lcd_sel = (lcdc_id << shift_lcd_sel);
			misc2_val &= ~mask_lcd_sel;
			misc2_val |= set_lcd_sel;
			break;
		case 4:
			shift_lcd_sel = 8U;
			mask_lcd_sel = 0x00000700U;
			set_lcd_sel = (lcdc_id << shift_lcd_sel);
			misc2_val &= ~mask_lcd_sel;
			misc2_val |= set_lcd_sel;
			break;
		}

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(misc1_val, pcfg_misc1);
		__raw_writel(misc2_val, pcfg_misc2);
	}
#else
	if (ret == 0) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(pcfg_misc1);
		shift_lcd_sel = (mux_id * 2U) + 24U;
		mask_lcd_sel = mask_lcd_sel << shift_lcd_sel; //0b11

		//clear lcd_sel
		val &= ~mask_lcd_sel;

		set_lcd_sel = lcdc_id << shift_lcd_sel; //0b11
		val |= set_lcd_sel;

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(val, pcfg_misc1);
	}
#endif

	return ret;
}

int lcdc_get_compatible_display(int rdma_id)
{
	int ret = 0;

	if ((rdma_id < 0) || (rdma_id > 13)) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("%s rdma id is invalid [%d]\n", __func__, rdma_id);
		ret = -EINVAL; //EINVAL
	}

	if (ret != -EINVAL) {
		if (rdma_id <= 3) {
			ret = 0;
		} else if (rdma_id <= 7) {
			ret = 1;
		} else if (rdma_id <= 11) {
			ret = 2;
		} else {//if ((rdma_id >= 12) && (rdma_id <= 13)) {
			ret = 3;
		}
	}

	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("%s : rdma_id = %d, compatible disp_id = %d\n",
		__func__, rdma_id, ret);


	return ret;
}

int lcdc_is_display_device_ready(int rdma_id)
{
	int disp_id = 0;
	int ret = 0;
	void __iomem *pDISPBase = NULL;

	disp_id = lcdc_get_compatible_display(rdma_id);

	if (disp_id == -EINVAL) {
		ret = -EINVAL;
	}

	if (ret == 0) {
		switch (disp_id) {
		case PANEL_PATH_DISP0:
			/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
			pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP0;
			break;
#if defined(VIOC_DISP1)
		case PANEL_PATH_DISP1:
			/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
			pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP1;
			break;
#endif
#if defined(VIOC_DISP2)
		case PANEL_PATH_DISP2:
			/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
			pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP2;
			break;
#endif
#if defined(VIOC_DISP3)
		case PANEL_PATH_DISP3:
			/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
			pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP3;
			break;
#endif
#if defined(VIOC_DISP4)
		case PANEL_PATH_DISP4:
			/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
			pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP4;
			break;
#endif
		default:
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err("[%s:%d]Error: Invalid params - display input(%d)\n",
			       __func__, __LINE__, disp_id);
			ret = -EINVAL;
			break;
		}
	}
	if (ret == 0) {
		ret = vioc_disp_get_turn_onoff(pDISPBase);
	}
	if (ret == 1) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("[INFO] %s : display device is running\n",
			 __func__);
	} else {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("[INFO] %s : display device is NOT running\n",
		       __func__);
	}

	return ret;
}

void tcc_lcdc_color_setting(
	uint8_t lcdc, signed char contrast, signed char brightness,
	signed char hue)
{
	void __iomem *pDISPBase;
	int ret = 0;

	switch (lcdc) {
	case (uint8_t)PANEL_PATH_DISP0:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP0;
		break;
#if defined(VIOC_DISP1)
	case (uint8_t)PANEL_PATH_DISP1:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP1;
		break;
#endif
#if defined(VIOC_DISP2)
	case (uint8_t)PANEL_PATH_DISP2:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP2;
		break;
#endif
#if defined(VIOC_DISP3)
	case (uint8_t)PANEL_PATH_DISP3:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP3;
		break;
#endif
#if defined(VIOC_DISP4)
	case PANEL_PATH_DISP4:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP4;
		break;
#endif
	default:
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Invalid params - display input(%u)\n",
		       __func__, __LINE__, (unsigned int)lcdc);
		ret = -1;
		break;
	}

	if (ret == 0) {
		VIOC_DISP_SetColorEnhancement(pDISPBase, contrast, brightness, hue);

		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("[%s:%d]lcdc:%u c :%d , b:%d h:%d\n", __func__,
			__LINE__, (unsigned int)lcdc,
			contrast, brightness, hue);
	}
}

void tcc_lcdc_dithering_setting(uint8_t lcdc)
{
	void __iomem *pDISPBase;
	unsigned int value;
	int ret = 0;

	switch (lcdc) {
	case (uint8_t)PANEL_PATH_DISP0:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP0;
		break;
#if defined(VIOC_DISP1)
	case (uint8_t)PANEL_PATH_DISP1:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP1;
		break;
#endif
#if defined(VIOC_DISP2)
	case (uint8_t)PANEL_PATH_DISP2:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP2;
		break;
#endif
#if defined(VIOC_DISP3)
	case (uint8_t)PANEL_PATH_DISP3:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP3;
		break;
#endif
#if defined(VIOC_DISP4)
	case PANEL_PATH_DISP4:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP4;
		break;
#endif
	default:
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Invalid params - display input(%u)\n",
		       __func__, __LINE__, (unsigned int)lcdc);
		ret = -1;
		break;
	}

	if (ret == 0) {
		/* dithering option */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = (__raw_readl(pDISPBase + DCTRL) & ~(DCTRL_PXDW_MASK));
		/* coverity[misra_c_2012_rule_12_2_violation : FALSE] */
		value |= (0x5U << DCTRL_PXDW_SHIFT);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DCTRL);

		/* coverity[misra_c_2012_rule_12_2_violation : FALSE] */
		value = ((0x1U << DDITH_DEN_SHIFT) | (0x1U << DDITH_DSEL_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DDITH);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(0x6e4ca280, pDISPBase + DDMAT0);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(0x5d7f91b3, pDISPBase + DDMAT1);
	}
}

void tcclcd_image_ch_set(
	unsigned int lcdctrl_num, const struct tcc_fb_dm_image_info *ImageInfo)
{
	void __iomem *pRDMA;
	void __iomem *pWIXBase;
	uint32_t base_addr0, base_addr1, base_addr2;
	int ret = 0;

	switch (lcdctrl_num) {
	case PANEL_PATH_DISP0:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pRDMA = (void __iomem *)(uintptr_t)((unsigned long)HwVIOC_RDMA00 +
					 ((unsigned long)ImageInfo->lcd_layer * RDMA_OFFSET));
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pWIXBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_WMIX0;
		break;
#if defined(VIOC_DISP1)
	case PANEL_PATH_DISP1:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pRDMA = (void __iomem *)(uintptr_t)((unsigned long)HwVIOC_RDMA04 +
					 ((unsigned long)ImageInfo->lcd_layer * RDMA_OFFSET));
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pWIXBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_WMIX1;
		break;
#endif
#if defined(VIOC_DISP2)
	case PANEL_PATH_DISP2:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pRDMA = (void __iomem *)(uintptr_t)(unsigned long)(HwVIOC_RDMA08 +
					 ((unsigned long)ImageInfo->lcd_layer * RDMA_OFFSET));
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pWIXBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_WMIX2;
		break;
#endif
#if defined(VIOC_DISP3)
	case PANEL_PATH_DISP3:
		#if defined(CONFIG_TCC805X)
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pRDMA = (void __iomem *)(uintptr_t)(unsigned long)(HwVIOC_RDMA12 +
					 ((unsigned long)ImageInfo->lcd_layer * RDMA_OFFSET));
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pWIXBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_WMIX3;
		#elif defined(CONFIG_TCC807X)
		pRDMA = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_RDMA14;
		pWIXBase = NULL;
		#endif
		break;
#endif
#if defined(VIOC_DISP4) /* tcc807x */
	case PANEL_PATH_DISP4:
		#if defined(CONFIG_TCC807X)
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pRDMA = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_RDMA15;
		pWIXBase = NULL;
		#endif
		break;
#endif
	default:
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Invaild lcdc number(%u)\n", __func__,
		       __LINE__, lcdctrl_num);
		ret = -1;
		break;
	}

	if (ret == 0) {

		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pr_info("[%s:%d] lcdc:%u ch:%u RDMA:0x%08lx\n", __func__, __LINE__,
			lcdctrl_num, ImageInfo->lcd_layer, (unsigned long)pRDMA);

		VIOC_RDMA_SetImageY2RMode(
					  pRDMA, 0); /* Y2RMode Default 0 (Studio Color) */

		if ((ImageInfo->fmt >= TCC_LCDC_IMG_FMT_YUV420SP)
		    && (ImageInfo->fmt <= TCC_LCDC_IMG_FMT_YUV422ITL1)) {
#if defined(LCD_DISPLAY_Y2R_ENABLE)
			VIOC_RDMA_SetImageR2YEnable(pRDMA, 0U);
			VIOC_RDMA_SetImageY2REnable(pRDMA, 0U);
#else
			VIOC_RDMA_SetImageR2YEnable(pRDMA, 0U);
			VIOC_RDMA_SetImageY2REnable(pRDMA, 1U);
#endif
		} else {
#if defined(LCD_DISPLAY_Y2R_ENABLE)
			VIOC_RDMA_SetImageR2YEnable(pRDMA, 1U);
			VIOC_RDMA_SetImageY2REnable(pRDMA, 0U);
#else
			VIOC_RDMA_SetImageR2YEnable(pRDMA, 0U);
			VIOC_RDMA_SetImageY2REnable(pRDMA, 0U);
#endif
		}

		VIOC_RDMA_SetImageOffset(pRDMA, (unsigned int)ImageInfo->fmt,
					 ImageInfo->frame_width);
		VIOC_RDMA_SetImageFormat(pRDMA, (unsigned int)ImageInfo->fmt);
		VIOC_RDMA_SetImageRGBSwapMode(pRDMA, 0); /* R-G-B */
		VIOC_RDMA_SetImageSize(
				       pRDMA, ImageInfo->image_width, ImageInfo->image_height);

		/* image address */
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		base_addr0 = (uint32_t)((unsigned long)ImageInfo->buf_addr0 & UINT_MAX);
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		base_addr1 = (uint32_t)((unsigned long)ImageInfo->buf_addr1 & UINT_MAX);
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		base_addr2 = (uint32_t)((unsigned long)ImageInfo->buf_addr2 & UINT_MAX);
		VIOC_RDMA_SetImageBase(
				       pRDMA, base_addr0, base_addr1, base_addr2);

		/* Disable Alpha blend */
		VIOC_RDMA_SetImageAlphaEnable(pRDMA, 0);

		/* image position */
		VIOC_WMIX_SetPosition(
				      pWIXBase, ImageInfo->lcd_layer, ImageInfo->offset_x,
				      ImageInfo->offset_y);

		/* image enable */
		if (ImageInfo->enable != 0U) {
			VIOC_RDMA_SetImageEnable(pRDMA);
		} else {
			VIOC_RDMA_SetImageDisable(pRDMA);
		}

		VIOC_WMIX_SetUpdate(pWIXBase);
	}
}

void tcclcd_gpioexp_set_value(unsigned int on)
{
	struct udevice *pDEV_I2C_Dev;
	struct udevice *pstBus = NULL;
	unsigned char addr, data, val;
	int ret;
	int done = 0;

	addr = 0x5a;

	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &pstBus);
	if (ret != 0) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s][%d]failed to get i2c device class\n", __func__,
		       __LINE__);
		done = 1;
	}
	if (done == 0) {
		ret = dm_i2c_set_bus_speed(pstBus, 400000);
		if (ret != 0) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err("\n[%s:%d]Error: from dm_i2c_set_bus_speed().. Ret(0x%x)\n",
			       __func__, __LINE__, (unsigned int)ret);
			done = 1;
		}
	}
	if (done == 0) {
		ret = i2c_get_chip_for_busnum(0, (int)addr, 1, &pDEV_I2C_Dev);
		if (ret != 0) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err("[%s][%d]failed to get i2c handle\n", __func__,
			       __LINE__);
			done = 1;
		}
	}

	if (done == 0) {
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
