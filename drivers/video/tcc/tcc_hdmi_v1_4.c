/*
 * Copyright (c) 2010 Telechips, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <config.h>
#include <common.h>
#include <lcd.h>

#include <asm/arch/globals.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>

#include <asm/telechips/vioc/structures_display.h>
#include <asm/telechips/gpio.h>
#include <asm/telechips/tcc_ckc.h>
#include <asm/telechips/vioc/vioc_config.h>
#include <asm/telechips/vioc/vioc_disp.h>
#include <asm/telechips/vioc/vioc_outcfg.h>
#include <asm/telechips/vioc/vioc_rdma.h>
#include <asm/telechips/vioc/vioc_wmix.h>

#include <asm/telechips/hdmi_v1_4_hdmi.h>
#include <asm/telechips/hdmi_v1_4_audio.h>

#include <hdmi_v1_4.h>

#define HDMI_1280X720_60P

#if defined(HDMI_640X480_60P)
#define HDMI_VIDEO_MODE_VIC     1
#define HDMI_VIDEO_MODE_HZ      60000
#define HDMI_COLOR_DEPTH        COLOR_DEPTH_8
#endif

#if defined(HDMI_720X480_60P)
#define HDMI_VIDEO_MODE_VIC     2
#define HDMI_VIDEO_MODE_HZ      60000
#define HDMI_COLOR_DEPTH        COLOR_DEPTH_8
#endif

#if defined(HDMI_1280X720_60P)
#define HDMI_VIDEO_MODE_VIC     4
#define HDMI_VIDEO_MODE_HZ      60000
#define HDMI_COLOR_DEPTH        COLOR_DEPTH_8
#endif

#if defined(HDMI_1920X1080_60P)
#define HDMI_VIDEO_MODE_VIC     16
#define HDMI_VIDEO_MODE_HZ      60000
#define HDMI_COLOR_DEPTH        COLOR_DEPTH_8
#endif



ushort *configuration_get_cmap(void)
{
#if defined(CONFIG_LCD_LOGO)
	return bmp_logo_palette;
#else
	return NULL;
#endif
}

static void
tcc_image_ch_set(
	char display_device_id, struct tcc_lcdc_image_update *ImageInfo)
{
	VIOC_WMIX *pWIXBase;
	VIOC_RDMA *pRDMA;

	if (display_device_id) {
		pRDMA =
			(VIOC_RDMA *)(
				(unsigned int)HwVIOC_RDMA04 +
				ImageInfo->Lcdc_layer * RDMA_OFFSET);
		pWIXBase = (VIOC_WMIX *)HwVIOC_WMIX1;

	} else {
		pRDMA = (VIOC_RDMA *)(
			(unsigned int)HwVIOC_RDMA00 + ImageInfo->Lcdc_layer *
			RDMA_OFFSET);
		pWIXBase = (VIOC_WMIX *)HwVIOC_WMIX0;
	}

	pr_info(
		"%s lcdc:%d lcdc_ch:%d pRDMA:0x%08x\n",
		   __func__, display_device_id, ImageInfo->Lcdc_layer,
		   (unsigned int)pRDMA);
	pr_info(
		"%s pos_x:%d pos_y:%d img_wd:%d img_ht:%d lcd_wd:%d lcd_ht:%d\n",
		   __func__, ImageInfo->offset_x, ImageInfo->offset_y,
		   ImageInfo->Image_width, ImageInfo->Image_height,
		   ImageInfo->Frame_width, ImageInfo->Frame_height);

	VIOC_RDMA_SetImageY2RMode(pRDMA, 0);
	if (
		ImageInfo->fmt >= TCC_LCDC_IMG_FMT_YUV420SP &&
		ImageInfo->fmt <= TCC_LCDC_IMG_FMT_YUV422ITL1) {
		VIOC_RDMA_SetImageR2YEnable(pRDMA, FALSE);
		VIOC_RDMA_SetImageY2REnable(pRDMA, TRUE);
	} else {
		VIOC_RDMA_SetImageR2YEnable(pRDMA, FALSE);
		VIOC_RDMA_SetImageY2REnable(pRDMA, FALSE);
	}

	VIOC_RDMA_SetImageOffset(
		pRDMA, ImageInfo->fmt, ImageInfo->Image_width);
	VIOC_RDMA_SetImageFormat(pRDMA, ImageInfo->fmt);
	VIOC_RDMA_SetImageSize(
		pRDMA, ImageInfo->Image_width, ImageInfo->Image_height);

	// image address
	VIOC_RDMA_SetImageBase(
		pRDMA, ImageInfo->addr0, ImageInfo->addr1, ImageInfo->addr2);

	// image position
	VIOC_WMIX_SetPosition(
		pWIXBase, ImageInfo->Lcdc_layer, ImageInfo->offset_x,
		ImageInfo->offset_y);

	// image enable
	if (ImageInfo->enable)
		VIOC_RDMA_SetImageEnable(pRDMA);
	else
		VIOC_RDMA_SetImageDisable(pRDMA);

	VIOC_WMIX_SetUpdate(pWIXBase);
}


static void
lcdc_set_logo(
	unsigned char display_device_id, unsigned int display_width,
	unsigned int display_height, unsigned int logo_base)
{
	struct tcc_lcdc_image_update Image_info = {0};

	Image_info.addr0 = logo_base;
	Image_info.Lcdc_layer = 0;
	Image_info.enable = 1;
	Image_info.offset_x =  0;
	Image_info.offset_y =  0;
	Image_info.Frame_width = Image_info.Image_width = display_width;
	Image_info.Frame_height = Image_info.Image_height = display_height;
	Image_info.fmt = TCC_LCDC_IMG_FMT_RGB565;

	tcc_image_ch_set(display_device_id, &Image_info);

	mdelay(1);
}

static void
tcc_hdmi_init(unsigned char display_device_id, unsigned int logo_base)
{
	uint width, height;
	VIOC_DISP *pDISP;
	VIOC_WMIX *pWIMX;
	VIOC_RDMA *pRDMA;

	stLTIMING HDMI_TIMEp;
	stLCDCTR pCtrlParam;

	volatile PDDICONFIG pDDI_Config = (PDDICONFIG)HwDDI_CONFIG_BASE;

	const struct HDMIVideoParameter video = {
	#if (HDMI_MODE_TYPE == 1)
		HDMI,
	#else
		DVI,
	#endif
		gRefHdmiVideoModeList[HDMI_VIDEO_MODE_TYPE].vfmt_val,
		HDMI_CS_RGB,
		HDMI_CD_24,
		HDMI_COLORIMETRY_NO_DATA,
		gRefHdmiVideoModeList[HDMI_VIDEO_MODE_TYPE].ratio,
		HDMI_SOURCE_EXTERNAL,
		HDMI_2D_VIDEO_FORMAT
	};

	const struct HDMIAudioParameter audio = {
		I2S_PORT,
		HDMI_ASP,
		LPCM_FORMAT,
		CH_2,
		SF_44KHZ,
		WORD_16,
		{
			I2S_BPC_16,
			I2S_BASIC,
			I2S_64FS
		},
	};

	pr_info("%s LCDC NUM:%d\n", __func__, display_device_id);

	if (display_device_id) {
		pDISP = (VIOC_DISP *)HwVIOC_DISP1;
		pWIMX = (VIOC_WMIX *)HwVIOC_WMIX1;
		pRDMA = (VIOC_RDMA *)HwVIOC_RDMA04;
		tcc_set_peri(PERI_LCD1, ENABLE, HDMI_PCLK_RATE, 0);
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_HDMI, VIOC_OUTCFG_DISP1);
	} else {
		pDISP = (VIOC_DISP *)HwVIOC_DISP0;
		pWIMX = (VIOC_WMIX *)HwVIOC_WMIX0;
		pRDMA = (VIOC_RDMA *)HwVIOC_RDMA00;
		tcc_set_peri(PERI_LCD0, ENABLE, HDMI_PCLK_RATE, 0);
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_HDMI, VIOC_OUTCFG_DISP0);
	}

	tcc_set_peri(PERI_HDMI_PCLK, ENABLE, (50*1000*1000), 0);
	tcc_set_peri(PERI_HDMI, ENABLE, XIN_CLK_RATE, 0);

	width = gRefHdmiVideoModeList[HDMI_VIDEO_MODE_TYPE].width;
	height = gRefHdmiVideoModeList[HDMI_VIDEO_MODE_TYPE].height;

	HDMI_TIMEp.lpw = LCDCTimimgParams[video.resolution].lpw;
	HDMI_TIMEp.lpc = LCDCTimimgParams[video.resolution].lpc + 1;
	HDMI_TIMEp.lswc = LCDCTimimgParams[video.resolution].lswc + 1;
	HDMI_TIMEp.lewc = LCDCTimimgParams[video.resolution].lewc + 1;

	HDMI_TIMEp.vdb = LCDCTimimgParams[video.resolution].vdb;
	HDMI_TIMEp.vdf = LCDCTimimgParams[video.resolution].vdf;
	HDMI_TIMEp.fpw = LCDCTimimgParams[video.resolution].fpw;
	HDMI_TIMEp.flc = LCDCTimimgParams[video.resolution].flc;
	HDMI_TIMEp.fswc = LCDCTimimgParams[video.resolution].fswc;
	HDMI_TIMEp.fewc = LCDCTimimgParams[video.resolution].fewc;
	HDMI_TIMEp.fpw2 = LCDCTimimgParams[video.resolution].fpw2;
	HDMI_TIMEp.flc2 = LCDCTimimgParams[video.resolution].flc2;
	HDMI_TIMEp.fswc2 = LCDCTimimgParams[video.resolution].fswc2;
	HDMI_TIMEp.fewc2 = LCDCTimimgParams[video.resolution].fewc2;

	VIOC_DISP_SetTimingParam(pDISP, &HDMI_TIMEp);

	memset(&pCtrlParam, 0x00, sizeof(pCtrlParam));
	pCtrlParam.id = LCDCTimimgParams[video.resolution].id;
	pCtrlParam.iv = LCDCTimimgParams[video.resolution].iv;
	pCtrlParam.ih = LCDCTimimgParams[video.resolution].ih;
	pCtrlParam.ip = LCDCTimimgParams[video.resolution].ip;
	pCtrlParam.clen = 0;
	if (video.colorSpace == HDMI_CS_RGB) {
		pCtrlParam.r2y = 0;
		pCtrlParam.pxdw = 12; //RGB888
	} else {
		pCtrlParam.r2y = 1;
		pCtrlParam.pxdw = 8; //RGB888
	}

	pCtrlParam.dp = LCDCTimimgParams[video.resolution].dp;
	pCtrlParam.ni = LCDCTimimgParams[video.resolution].ni;
	pCtrlParam.tv = LCDCTimimgParams[video.resolution].tv;
	pCtrlParam.opt = 0;
	pCtrlParam.stn = 0;
	pCtrlParam.evsel = 0;
	pCtrlParam.ovp = 0;

	VIOC_DISP_TurnOff(pDISP);
	VIOC_RDMA_SetImageDisable(pRDMA);

	VIOC_DISP_SetControlConfigure(pDISP, &pCtrlParam);

	VIOC_WMIX_SetOverlayPriority(pWIMX, 24);
	VIOC_WMIX_SetBGColor(pWIMX, 0x00, 0x00, 0x00, 0xff);
	VIOC_WMIX_SetSize(pWIMX, width, height);
	VIOC_WMIX_SetChromaKey(pWIMX, 0, 0, 0, 0, 0, 0xF8, 0xFC, 0xF8);
	VIOC_WMIX_SetUpdate(pWIMX);

	BITSET(pDDI_Config->PWDN.nREG, Hw2);
	BITCLR(pDDI_Config->SWRESET.nREG, Hw2);
	BITSET(pDDI_Config->SWRESET.nREG, Hw2);

	hdmi_ddi_config_init();

	hdmi_set_video_mode(&video);
	hdmi_set_hdmi_mode(video.mode);

	hdmi_check_phy_ready();

	VIOC_DISP_SetSize(pDISP, width, height);
	VIOC_DISP_SetBGColor(pDISP, 0, 0, 0);
	VIOC_DISP_TurnOn(pDISP);

	if (video.mode == HDMI)
		hdmi_set_audio_mode(&audio);

	lcdc_set_logo(display_device_id, width, height, logo_base);

	hdmi_start();
}

struct vidinfo panel_info = {
#if defined(HDMI_640X480_60P)
	.vl_col         = 640,
	.vl_row         = 480,
#endif
#if defined(HDMI_720X480_60P)
	.vl_col         = 720,
	.vl_row         = 480,
#endif
#if defined(HDMI_1280X720_60P)
	.vl_col         = 1280,
	.vl_row         = 720,
#endif
#if defined(HDMI_1920X1080_60P)
	.vl_col         = 1920,
	.vl_row         = 1080,
#endif
	.vl_bpix        = 5, // 16BPP
	.vl_rot         = 0, // 0"
};

void lcd_ctrl_init(void *lcdbase)
{
	pr_info("%s 0x%x\r\n", __func__, (unsigned int)lcdbase);
	init_panel_info(&panel_info);
	panel_info.lcdbase = (unsigned int)lcdbase;
}

void lcd_enable(void)
{
#ifdef CONFIG_CMD_BMP
	lcd_set_flush_dcache(1);
#ifdef CONFIG_SPLASH_SCREEN_ALIGN
#define BMP_ALIGN_CENTER	0x7FFF
	bmp_display(
		panel_info.dev.logo_data, BMP_ALIGN_CENTER, BMP_ALIGN_CENTER);
#else
	bmp_display(panel_info.dev.logo_data, 0, 0);
#endif
#endif

	tcc_hdmi_init(0, (unsigned int)panel_info.lcdbase);
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue) { }

int get_panel_info(const char *name, struct vidinfo *vid)
{
	if (vid)
		memcpy(vid, &panel_info, sizeof(panel_info));
}

int tcc_get_panel_id(void)
{
	return panel_info.id;
}
