// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 - present Telechips.co and/or its affiliates.
 */
#include <config.h>
#include <common.h>
#include <lcd.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/vioc/reg_physical.h>
#include <asm/arch/vioc/vioc_global.h>
#include <asm/arch/vioc/vioc_disp.h>
#include <asm/arch/tcc_ckc.h>
#include <asm/arch/vioc/vioc_blk.h>
#include <asm/arch/vioc/vioc_config.h>
#include <asm/arch/vioc/vioc_disp.h>
#include <asm/arch/vioc/vioc_outcfg.h>
#include <asm/arch/vioc/vioc_rdma.h>
#include <asm/arch/vioc/vioc_wmix.h>
#include <asm/arch/vioc/vioc_disp.h>
#include <telechips/hdmi_v2_0.h>
#include <linux/delay.h>
#include <include/hdmi_access.h>
#include <include/video_params.h>
#include <api/api.h>
#include <hdmiv20_ctrl.h>
#include <hdmi_v2_0.h>

#if defined(CONFIG_TCC_FB_USES_SCALER)
#include <asm/arch/vioc/vioc_scaler.h>
#endif // CONFIG_TCC_FB_USES_SCALER

//DECLARE_GLOBAL_DATA_PTR;

/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static unsigned int tcc_hdmi_get_vactive(struct video_params *video)
{
	unsigned int vactive = 0;

	if (video != NULL) {
		vactive = video->mDtd.mVActive;
		if ((video->mHdmiVideoFormat == 2U) && (video->m3dStructure == 0U)) {
			/* Check HDMI 3D-Frame Packing */
			if ((bool)video->mDtd.mInterlaced) {
				/* KCS */
				vactive =
					(vactive << 2U) +
					((3U * video->mDtd.mVBlanking) + 2U);
			}
			else {
				/* KCS */
				vactive =
				(vactive << 1U) + video->mDtd.mVBlanking;
			}
		} else if ((bool)video->mDtd.mInterlaced) {
			/* Check HDMI Interlaced Mode */
			vactive <<= 1U;
		} else {
			/* KCS */
			/* .. */
		}
	}

	//pr_info("%s active=%d\r\n", __func__, vactive);
	return vactive;
}

void tcc_hdmi_display_device_reset(unsigned char display_device_id)
{
	void __iomem *rdma;
	void __iomem *disp;
	unsigned long base_addr;
	unsigned int rdma_id, wmix_id, disp_id;
	unsigned int uitemp;
	unsigned long ultemp;
	base_addr = TCC_VIOC_DISP_BASE(display_device_id);
	/* coverity[cert_int36_c_violation : FALSE] */
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	disp = (void __iomem *)base_addr;

	/* Disable RDMA */
	base_addr = TCC_VIOC_RDMA_BASE(0U);
	uitemp = (unsigned int)display_device_id;
	uitemp = (uitemp << 10U);
	ultemp = (unsigned long)(uitemp & 0xFFFFFFFFFFFFFFFFU);
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	/* coverity[cert_int36_c_violation : FALSE] */
	rdma = (void __iomem *)(
		base_addr + ultemp);

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
		rdma_id = 0;
		wmix_id = 0;
		disp_id = 0;
		break;
	}
	if(rdma_id == 0U) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto reset_exit;
	}
	VIOC_RDMA_SetImageDisable(rdma);
	/* Wait Disable */

	/* Disable Display Device */
	VIOC_DISP_TurnOff(disp);

	/* Wait Disable */
	mdelay(200);

	VIOC_CONFIG_SWReset(disp_id, (unsigned int)VIOC_CONFIG_RESET);
	VIOC_CONFIG_SWReset(wmix_id, (unsigned int)VIOC_CONFIG_RESET);
	VIOC_CONFIG_SWReset(rdma_id, (unsigned int)VIOC_CONFIG_RESET);

	VIOC_CONFIG_SWReset(rdma_id, (unsigned int)VIOC_CONFIG_CLEAR);
	VIOC_CONFIG_SWReset(wmix_id, (unsigned int)VIOC_CONFIG_CLEAR);
	VIOC_CONFIG_SWReset(disp_id, (unsigned int)VIOC_CONFIG_CLEAR);
reset_exit:
	return;
}

unsigned int tcc_hdmi_get_width_from_vic(unsigned int vic)
{
	struct video_params video;

	if (hdmi_dtd_fill(&video.mDtd, vic, 0) < 0) {
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("%s Force 1280x720@60p\r\n", __func__);
		(void)hdmi_dtd_fill(&video.mDtd, 4, 60000); // FORCE 1280x720@60p
	}
	/* coverity[misra_c_2012_rule_9_1_violation : FALSE] */
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
	unsigned long base_addr;
	unsigned int vactive, width, height, swapbf;
	u16 u16temp = 0;
	void __iomem *disp, *wmix, *outconfig;
	struct stLTIMING lcdc_timing_params = {0};
	struct stLCDCTR disp_ctr_params;

	struct video_params video = {0};

	base_addr = TCC_VIOC_DISP_BASE(display_device_id);
	/* coverity[cert_int36_c_violation : FALSE] */
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	disp = (void __iomem *)base_addr;

	base_addr = TCC_VIOC_WMIX_BASE(display_device_id);
	/* coverity[cert_int36_c_violation : FALSE] */
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	wmix = (void __iomem *)base_addr;

	base_addr = TCC_VIOC_OUTCONFIG_BASE;
	/* coverity[cert_int36_c_violation : FALSE] */
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	outconfig = (void __iomem *)base_addr;
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	(void)reg32_bitcset(outconfig + MISC, MISC_HDMISEL_MASK, display_device_id);
	VIOC_DISP_TurnOff(disp);

	// Initialize Video Param
	video_params_reset(&video);
	//if((depth <= S8_MAX) && (encoding <= (enum em_encoding)U8_MAX) && (hdmi_mode <= S8_MAX)) {
		if((depth >= 0) && (encoding >= (enum em_encoding)0) && (hdmi_mode >= 0)) {
			video.mColorResolution = (u8)(((u32)depth & 0xFFU));
			video.mEncodingOut = (enum em_encoding)encoding;
			video.mHdmi = (enum em_video_mode)hdmi_mode;
		}
	//}

	if ((bool)hdmi_get_framepacking()) {
		video.mHdmiVideoFormat = 2;
		video.m3dStructure = 0;
	}
	if((vic >= 0) && (hz >= 0)) {
		/* KCS */
		if (hdmi_dtd_fill(&video.mDtd, (unsigned int)vic, (unsigned int)hz) < 0) {
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
			pr_info("Force 1280x720@60p\r\n");
			(void)hdmi_dtd_fill(&video.mDtd, 4, 60000); // FORCE 1280x720@60p
		}
	}
	video.mEncodingIn = video.mEncodingOut;

	(void)memset(&disp_ctr_params, 0x0, sizeof(disp_ctr_params));
	(void)memset(&lcdc_timing_params, 0, sizeof(lcdc_timing_params));

	/** 0 for Active low, 1 active high */
	disp_ctr_params.iv = (bool)video.mDtd.mVSyncPolarity ? (u32)0 : (u32)1;
	/** 0 for Active low, 1 active high */
	disp_ctr_params.ih = (bool)video.mDtd.mHSyncPolarity ? (u32)0 : (u32)1;
	disp_ctr_params.dp = video.mDtd.mPixelRepetitionInput;

	vactive = tcc_hdmi_get_vactive(&video);

	/* 3d data frame packing is transmitted as a progressive format */
	hdmi_interlaced_mode = (bool)(
		(video.mHdmiVideoFormat == 2U) && (video.m3dStructure == 0U)) ?
						0 : (int)video.mDtd.mInterlaced;

	if ((bool)hdmi_interlaced_mode) {
		/* KCS */
		disp_ctr_params.tv = 1;
	}
	else {
		/* KCS */
		disp_ctr_params.ni = 1;
	}

	lcdc_timing_params.lpw = video.mDtd.mHSyncPulseWidth;
	lcdc_timing_params.lpc = video.mDtd.mHActive;

	/* coverity[misra_c_2012_rule_10_8_violation : FALSE] */
		/* coverity[cert_int31_c_violation : FALSE] */
		/* coverity[cert_int02_c_violation : FALSE] */
	if(video.mDtd.mHSyncOffset <= (U16_MAX - video.mDtd.mHSyncPulseWidth)) {
		u16temp = video.mDtd.mHSyncOffset +	video.mDtd.mHSyncPulseWidth;
	}
	if((u16)(video.mDtd.mHBlanking) >= u16temp) {
		u16temp = (video.mDtd.mHBlanking -
				(video.mDtd.mHSyncOffset +
				video.mDtd.mHSyncPulseWidth));
	}
	/*u16temp = (video.mDtd.mHBlanking -
				(video.mDtd.mHSyncOffset +
				video.mDtd.mHSyncPulseWidth));
	*/
	lcdc_timing_params.lswc = (unsigned int)u16temp;
	lcdc_timing_params.lewc = video.mDtd.mHSyncOffset;

	if ((bool)hdmi_interlaced_mode) {
		u16temp = (video.mDtd.mVSyncPulseWidth << 1U);
		if(u16temp >= 1U) {
			u16temp -= 1U;
		}
		lcdc_timing_params.fpw =
			(unsigned int)u16temp;

		/* coverity[misra_c_2012_rule_10_8_violation : FALSE] */
		/* coverity[cert_int31_c_violation : FALSE] */
		/* coverity[cert_int02_c_violation : FALSE] */
		if(video.mDtd.mVSyncOffset <= (U16_MAX - video.mDtd.mVSyncPulseWidth)) {
			u16temp = video.mDtd.mVSyncOffset +	video.mDtd.mVSyncPulseWidth;
		}
		/* vsync1 = vsync */
		if((u16)(video.mDtd.mVBlanking) >= u16temp) {
			u16temp = ((video.mDtd.mVBlanking -
			(video.mDtd.mVSyncOffset +
			video.mDtd.mVSyncPulseWidth)));
			u16temp = (u16temp << 1U);
			if(u16temp >= (1U)) {
				u16temp -= 1U;
			}
		}
		/*u16temp = ((
			((video.mDtd.mVBlanking -
			(video.mDtd.mVSyncOffset +
			video.mDtd.mVSyncPulseWidth))) << 1U) - 1U);*/

		lcdc_timing_params.fswc = (unsigned int)u16temp;
		/* back porch1 = back porch */

		u16temp = (video.mDtd.mVSyncOffset << 1U);
		lcdc_timing_params.fewc = (unsigned int)u16temp;
		/* front porch1 = front porch +1 */

		lcdc_timing_params.fswc2 = (lcdc_timing_params.fswc + 1U);
		/* back porch2 = back porch1 + 1 */

		lcdc_timing_params.fewc2 = (lcdc_timing_params.fewc > 0U) ?
			(lcdc_timing_params.fewc - 1U) : (unsigned int)0;;
		/* front porch2 = front porch1 -1 */
		if (video.mDtd.mCode == 39U) {
			/* 1920x1080@50i 1250 vtotal */
			lcdc_timing_params.fewc -= 2U;
		}
	} else {

		u16temp = video.mDtd.mVSyncPulseWidth;// - (u16)1U;
		if(u16temp >= 1U) {
			/* KCS */
			u16temp -= 1U;
		}
		lcdc_timing_params.fpw = (unsigned int)u16temp;

		/* coverity[misra_c_2012_rule_10_8_violation : FALSE] */
		/* coverity[cert_int31_c_violation : FALSE] */
		/* coverity[cert_int02_c_violation : FALSE] */
		if(video.mDtd.mVSyncOffset <= (U16_MAX - video.mDtd.mVSyncPulseWidth)) {
			u16temp = video.mDtd.mVSyncOffset +	video.mDtd.mVSyncPulseWidth;
		}
		/* vsync1 = vsync */
		if((u16)(video.mDtd.mVBlanking) >= u16temp) {
			u16temp = ((video.mDtd.mVBlanking -
			(video.mDtd.mVSyncOffset +
			video.mDtd.mVSyncPulseWidth)));
			if(u16temp >= (1U)) {
				u16temp -= 1U;
			}
		}
		lcdc_timing_params.fswc = (unsigned int)u16temp;

		u16temp = video.mDtd.mVSyncOffset ;
		if(u16temp >= 1U) {
			/* KCS */
			u16temp -= 1U;
		}
		lcdc_timing_params.fewc = (unsigned int)u16temp;
		lcdc_timing_params.fswc2 = lcdc_timing_params.fswc;
		lcdc_timing_params.fewc2 = lcdc_timing_params.fewc;
	}

	/* Common Timing Parameters */
	if(vactive >= 1U) {
		/* KCS */
		lcdc_timing_params.flc = (vactive - 1U);
	}
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
	u16temp = (bool)video.mDtd.mPixelRepetitionInput ?
			(video.mDtd.mHActive >> 1U) : video.mDtd.mHActive;
	width = (unsigned int)u16temp;
	height = vactive;

	VIOC_DISP_SetTimingParam(disp, &lcdc_timing_params);

	switch (video.mEncodingOut) {
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
	case RGB:
	default:
		disp_ctr_params.pxdw = DCTRL_PXDW_RGB;
		disp_ctr_params.r2y = 0;
		swapbf = 0;
		break;
	}

	VIOC_DISP_SetControlConfigure(disp, &disp_ctr_params);

	/* VIOC_DISP api is not ready for SWAF */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	(void)reg32_bitcset(
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
	if((img_width >= 0) && (img_height >= 0)) {
		/* KCS */
		VIOC_WMIX_SetPosition(
			wmix, 0, (width > (unsigned int)img_width) ? ((width - (unsigned int)img_width) >> 1) : 0U,
			(height > (unsigned int)img_height) ? ((height - (unsigned int)img_height) >> 1) : 0U);
	}
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

