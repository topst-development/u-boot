// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 - present Telechips.co and/or its affiliates.
 */
#include <config.h>
#include <common.h>
#include <lcd.h>
#include <i2c.h>

#include <asm/arch/gpio.h>
#include <asm/io.h>
#include <asm/telechips/vioc/reg_physical.h>
#include <asm/telechips/vioc/vioc_global.h>
#include <asm/telechips/vioc/vioc_disp.h>
#include <asm/telechips/gpio.h>
#include <asm/telechips/tcc_ckc.h>
#include <asm/telechips/vioc/vioc_blk.h>
#include <asm/telechips/vioc/vioc_config.h>
#include <asm/telechips/vioc/vioc_disp.h>
#include <asm/telechips/vioc/vioc_outcfg.h>
#include <asm/telechips/vioc/vioc_rdma.h>
#include <asm/telechips/vioc/vioc_wmix.h>
#include <asm/telechips/vioc/vioc_disp.h>
#include <asm/telechips/hdmi_v2_0.h>
#include <include/hdmi_access.h>
#include <include/video_params.h>
#include <api/api.h>
#include <lcd_HDMIV20.h>
#include <tcc_hdmi_v2_0.h>

#if defined(CONFIG_TCC_FB_USES_SCALER)
#include <asm/telechips/vioc/vioc_scaler.h>
#endif // CONFIG_TCC_FB_USES_SCALER

DECLARE_GLOBAL_DATA_PTR;

static unsigned int tcc_hdmi_get_vactive(struct video_params *video)
{
	unsigned int vactive = 0;

	if (video != NULL) {
		vactive = video->mDtd.mVActive;
		if (video->mHdmiVideoFormat == 2 && video->m3dStructure == 0) {
			/* Check HDMI 3D-Frame Packing */
			if (video->mDtd.mInterlaced)
				vactive =
					(vactive << 2) +
					(3 * video->mDtd.mVBlanking + 2);
			else
				vactive =
				(vactive << 1) + video->mDtd.mVBlanking;
		} else if (video->mDtd.mInterlaced) {
			/* Check HDMI Interlaced Mode */
			vactive <<= 1;
		}
	}

	//pr_info("%s active=%d\r\n", __func__, vactive);
	return vactive;
}

void tcc_hdmi_display_device_reset(unsigned char display_device_id)
{
	void __iomem *rdma;
	void __iomem *disp;
	unsigned long base_address;
	unsigned int rdma_id, wmix_id, disp_id;

	base_address = TCC_VIOC_DISP_BASE(display_device_id);
	disp = (void __iomem *)base_address;

	/* Disable RDMA */
	base_address = TCC_VIOC_RDMA_BASE(0);
	rdma = (void __iomem *)(
		base_address + (display_device_id << 10));

	switch (display_device_id) {
	case 0:
		rdma_id = VIOC_RDMA00;
		wmix_id = VIOC_WMIX0;
		disp_id = VIOC_DISP0;
		break;
	case 1:
		rdma_id = VIOC_RDMA04;
		wmix_id = VIOC_WMIX1;
		disp_id = VIOC_DISP1;
		break;
	case 2:
		rdma_id = VIOC_RDMA08;
		wmix_id = VIOC_WMIX2;
		disp_id = VIOC_DISP2;
		break;
	default:
		return;
	}
	VIOC_RDMA_SetImageDisable(rdma);
	/* Wait Disable */

	/* Disable Display Device */
	VIOC_DISP_TurnOff(disp);

	/* Wait Disable */
	mdelay(200);

	VIOC_CONFIG_SWReset(disp_id, VIOC_CONFIG_RESET);
	VIOC_CONFIG_SWReset(wmix_id, VIOC_CONFIG_RESET);
	VIOC_CONFIG_SWReset(rdma_id, VIOC_CONFIG_RESET);

	VIOC_CONFIG_SWReset(rdma_id, VIOC_CONFIG_CLEAR);
	VIOC_CONFIG_SWReset(wmix_id, VIOC_CONFIG_CLEAR);
	VIOC_CONFIG_SWReset(disp_id, VIOC_CONFIG_CLEAR);
}

unsigned int tcc_hdmi_get_width_from_vic(unsigned int vic)
{
	struct video_params video;

	if (hdmi_dtd_fill(&video.mDtd, vic, 0) < 0) {
		pr_info("%s Force 1280x720@60p\r\n", __func__);
		hdmi_dtd_fill(&video.mDtd, 4, 60000); // FORCE 1280x720@60p
	}
	return video.mDtd.mHActive;
}

void tcc_hdmi_init(
	unsigned char display_device_id, int hdmi_mode,
	int vic, int hz, int depth, enum em_encoding encoding,
	int img_width, int img_height)
{
	#if defined(CONFIG_TCC_FB_USES_SCALER)
	void __iomem *sc_reg = NULL;
	unsigned int sc_num;
	#endif // CONFIG_TCC_FB_USES_SCALER
	int hdmi_interlaced_mode;
	unsigned long base_address;
	unsigned int vactive, width, height, swapbf;
	void __iomem *disp, *wmix, *outconfig;
	struct stLTIMING lcdc_timing_params = {0};
	struct stLCDCTR disp_ctr_params;

	struct video_params video = {0};

	base_address = TCC_VIOC_DISP_BASE(display_device_id);
	disp = (void __iomem *)base_address;

	base_address = TCC_VIOC_WMIX_BASE(display_device_id);
	wmix = (void __iomem *)base_address;

	base_address = TCC_VIOC_OUTCONFIG_BASE;
	outconfig = (void __iomem *)base_address;
	reg32_bitcset(outconfig + MISC, MISC_HDMISEL_MASK, display_device_id);
	VIOC_DISP_TurnOff(disp);

	// Initialize Video Param
	video_params_reset(&video);
	video.mColorResolution = depth;
	video.mEncodingOut = encoding;
	video.mHdmi = hdmi_mode;

	if (hdmi_get_framepacking()) {
		video.mHdmiVideoFormat = 2;
		video.m3dStructure = 0;
	}
	if (hdmi_dtd_fill(&video.mDtd, vic, hz) < 0) {
		pr_info("Force 1280x720@60p\r\n");
		hdmi_dtd_fill(&video.mDtd, 4, 60000); // FORCE 1280x720@60p
	}
	video.mEncodingIn = video.mEncodingOut;

	memset(&disp_ctr_params, 0x0, sizeof(disp_ctr_params));
	memset(&lcdc_timing_params, 0, sizeof(lcdc_timing_params));

	/** 0 for Active low, 1 active high */
	disp_ctr_params.iv = video.mDtd.mVSyncPolarity ? 0 : 1;
	/** 0 for Active low, 1 active high */
	disp_ctr_params.ih = video.mDtd.mHSyncPolarity ? 0 : 1;
	disp_ctr_params.dp = video.mDtd.mPixelRepetitionInput;

	vactive = tcc_hdmi_get_vactive(&video);

	/* 3d data frame packing is transmitted as a progressive format */
	hdmi_interlaced_mode = (
		video.mHdmiVideoFormat == 2 && video.m3dStructure == 0) ?
						0 : video.mDtd.mInterlaced;

	if (hdmi_interlaced_mode)
		disp_ctr_params.tv = 1;
	else
		disp_ctr_params.ni = 1;

	lcdc_timing_params.lpw = video.mDtd.mHSyncPulseWidth;
	lcdc_timing_params.lpc = video.mDtd.mHActive;
	lcdc_timing_params.lswc = (
			video.mDtd.mHBlanking -
			(
				video.mDtd.mHSyncOffset +
				video.mDtd.mHSyncPulseWidth));
	lcdc_timing_params.lewc = video.mDtd.mHSyncOffset;

	if (hdmi_interlaced_mode) {
		lcdc_timing_params.fpw =
			(video.mDtd.mVSyncPulseWidth << 1) - 1;
		/* vsync1 = vsync */

		lcdc_timing_params.fswc = (
			((video.mDtd.mVBlanking -
			(video.mDtd.mVSyncOffset +
			video.mDtd.mVSyncPulseWidth))) << 1) - 1;
		/* back porch1 = back porch */

		lcdc_timing_params.fewc = (video.mDtd.mVSyncOffset << 1);
		/* front porch1 = front porch +1 */

		lcdc_timing_params.fswc2 = lcdc_timing_params.fswc + 1;
		/* back porch2 = back porch1 + 1 */

		lcdc_timing_params.fewc2 =
			(lcdc_timing_params.fewc > 0) ?
			(lcdc_timing_params.fewc - 1) : 0;
		/* front porch2 = front porch1 -1 */
		if (video.mDtd.mCode == 39) {
			/* 1920x1080@50i 1250 vtotal */
			lcdc_timing_params.fewc -= 2;
		}
	} else {
		lcdc_timing_params.fpw = video.mDtd.mVSyncPulseWidth - 1;
		lcdc_timing_params.fswc = (
			video.mDtd.mVBlanking - (
				video.mDtd.mVSyncOffset +
				video.mDtd.mVSyncPulseWidth)) - 1;
		lcdc_timing_params.fewc = video.mDtd.mVSyncOffset - 1;
		lcdc_timing_params.fswc2 = lcdc_timing_params.fswc;
		lcdc_timing_params.fewc2 = lcdc_timing_params.fewc;
	}

	/* Common Timing Parameters */
	lcdc_timing_params.flc = vactive - 1;
	/* vactive 1 = vactive */

	lcdc_timing_params.fpw2 = lcdc_timing_params.fpw;
	/* vsync2 = vsync1 */

	lcdc_timing_params.flc2 = lcdc_timing_params.flc;
	 /* vactive 2 = vactive */

	#if defined(CONFIG_DUMP_HDMI_DISPLAY_TIMING)
	pr_info(
		"timing dump\r\n"
		"back porch1  %d \r\n"
		"vsync1	     %d \r\n"
		"front porch1 %d \r\n"
		"vblank       %d \r\n"
		"back porch2  %d \r\n"
		"vsync2       %d \r\n"
		"front porch2 %d \r\n"
		"vblank 2     %d \r\n",
		lcdc_timing_params.fswc,
		lcdc_timing_params.fpw,
		lcdc_timing_params.fewc,
		lcdc_timing_params.fswc +
		lcdc_timing_params.fpw + lcdc_timing_params.fewc,
		lcdc_timing_params.fswc2,
		lcdc_timing_params.fpw2,
		lcdc_timing_params.fewc2,
		lcdc_timing_params.fswc2 +
		lcdc_timing_params.fpw2 + lcdc_timing_params.fewc2);
	#endif
	width = video.mDtd.mPixelRepetitionInput ?
			(video.mDtd.mHActive >> 1) : video.mDtd.mHActive;
	height = vactive;

	VIOC_DISP_SetTimingParam(disp, &lcdc_timing_params);

	switch (video.mEncodingOut) {
	case RGB:
	default:
		disp_ctr_params.pxdw = DCTRL_PXDW_RGB;
		disp_ctr_params.r2y = 0;
		swapbf = 0;
		break;
	case YCC444:
		disp_ctr_params.pxdw = DCTRL_PXDW_YCBCR444;
		disp_ctr_params.r2y = 1;
		swapbf = 2;
		break;
	case YCC422:
		disp_ctr_params.pxdw = DCTRL_PXDW_YCBCR422;
		disp_ctr_params.r2y = 1;
		swapbf = 5;
		break;
		#if defined(DCTRL_PXDW_YCBCR420)
	case YCC420:
		disp_ctr_params.pxdw = DCTRL_PXDW_YCBCR420;
		disp_ctr_params.r2y = 1;
		swapbf = 0;
		break;
		#endif
	}

	VIOC_DISP_SetControlConfigure(disp, &disp_ctr_params);

	/* VIOC_DISP api is not ready for SWAF */
	reg32_bitcset(
		disp + DALIGN, DALIGN_SWAPAF_MASK,
				swapbf << DALIGN_SWAPAF_SHIFT);

	#if defined(CONFIG_TCC898X) || \
		defined(CONFIG_TCC899X) || defined(CONFIG_TCC901X)
	VIOC_DISP_SetAlign(disp, 0);
	#endif

	#if defined(CONFIG_TCC_FB_USES_SCALER)
	/* scaler control */
	sc_num = tcc_get_scaler_number(CONFIG_HDMI_DISP_ORDER);
	if (get_vioc_index(sc_num) < get_vioc_index(VIOC_SCALER_MAX)) {
		sc_reg = (void __iomem *)VIOC_SC_GetAddress(sc_num);
		if (sc_reg != NULL) {
			VIOC_CONFIG_PlugIn(
				sc_num, VIOC_RDMA00 + (display_device_id << 2));
			VIOC_SC_SetDstSize(sc_reg, width, height);
			VIOC_SC_SetOutSize(sc_reg, width, height);
			VIOC_SC_SetOutPosition(sc_reg, 0, 0);
			VIOC_SC_SetBypass(sc_reg, 0);
			VIOC_SC_SetUpdate(sc_reg);
		}
	}
	#endif // CONFIG_TCC_FB_USES_SCALER

	/* wmixer control */
	VIOC_WMIX_SetOverlayPriority(wmix, 24);
	VIOC_WMIX_SetBGColor(wmix, 0, 0, 0, 0);
	VIOC_WMIX_SetSize(wmix, width, height);
	#if defined(CONFIG_TCC_FB_USES_SCALER)
	if (sc_reg == NULL)
		VIOC_WMIX_SetPosition(
			wmix, 0, (width > img_width) ?
			(width - img_width) >> 1 : 0,
			(height > img_height) ? (height - img_height) >> 1 : 0);
	else
		VIOC_WMIX_SetPosition(wmix, 0, 0, 0);
	#else
	VIOC_WMIX_SetPosition(
		wmix, 0, (width > img_width) ? (width - img_width) >> 1 : 0,
		(height > img_height) ? (height - img_height) >> 1 : 0);
	#endif // CONFIG_TCC_FB_USES_SCALER
	VIOC_WMIX_SetUpdate(wmix);
	VIOC_DISP_SetSize(disp, width, height);
	VIOC_DISP_SetBGColor(disp, 0, 0, 0, 0);
	VIOC_DISP_TurnOn(disp);
	tcc_hdmi_set_enable(&video);

	#if !defined(CONFIG_HDMI_PANEL_MODE)
	mdelay(85);

	/* Disable AV Mute */
	hdmi_api_avmute(0);
	#endif
}

