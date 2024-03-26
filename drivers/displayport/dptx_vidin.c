/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
*/

/*
* Modified by Telechips Inc.
*/

#include <linux/drm_dp_helper.h>

#include "dptx_v14.h"
#include "dptx_drm_dp_addition.h"
#include "dptx_reg.h"
#include "dptx_dbg.h"

//#define ENABLE_AVGEN_AUDIO_DEBUG

#define	IEC60958_3_CH_STATUS_MODE0_LEFT							0x80
#define	IEC60958_3_CH_STATUS_MODE0_RIGHT						0x40

#define	IEC60958_3_CH_STATUS_MAX_WORD_LENGTH_20BITS				0x00
#define	IEC60958_3_CH_STATUS_MAX_WORD_LENGTH_24BITS				0x08

#define	IEC60958_3_CH_STATUS_SAMPLE_WORD_LENGTH_20BITS			0x04
#define	IEC60958_3_CH_STATUS_SAMPLE_WORD_LENGTH_21BITS			0x03
#define	IEC60958_3_CH_STATUS_SAMPLE_WORD_LENGTH_22BITS			0x02
#define	IEC60958_3_CH_STATUS_SAMPLE_WORD_LENGTH_23BITS			0x01
#define	IEC60958_3_CH_STATUS_SAMPLE_WORD_LENGTH_24BITS			0x05

#define AUDIO_INFOFREAME_HEADER				0x441B8400

// Rounding up to the nearest multiple of a number
#define ROUND_UP_TO_NEAREST(numToRound, mult) ((((numToRound+(mult)-1) / (mult)) * (mult)))

static uint8_t dptx_bit_field(const uint16_t data, uint8_t shift, uint8_t width)
{
	return ((data >> shift) & ((((u16)1) << width) - 1));
}

static uint16_t dptx_concat_bits(uint8_t bhi, uint8_t ohi, uint8_t nhi, uint8_t blo, uint8_t olo, uint8_t nlo)
{
	return (dptx_bit_field(bhi, ohi, nhi) << nlo) | dptx_bit_field(blo, olo, nlo);
}

static int32_t dptx_vidin_set_video_sampler(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	uint8_t	ucVideoMapping = 0;
	uint32_t uiRegMap_VidSampleCtrl;
	struct Dptx_Video_Params *pstVideoParams = &pstDptx->stVideoParams;

	uiRegMap_VidSampleCtrl = Dptx_Reg_Readl(pstDptx, DPTX_VSAMPLE_CTRL_N(ucStream_Index));

	uiRegMap_VidSampleCtrl &= ~(DPTX_VSAMPLE_CTRL_VMAP_BPC_MASK | DPTX_VSAMPLE_CTRL_MULTI_PIXEL_MASK);

	switch ((enum PIXEL_ENCODING_TYPE)pstVideoParams->ucPixel_Encoding) {
	case PIXEL_ENCODING_TYPE_RGB:
		if (pstVideoParams->ucBitPerComponent == (uint8_t)COLOR_DEPTH_8) {
			ucVideoMapping = 1;
		} else {
			dptx_err("Invalid bpc = %d ", pstVideoParams->ucBitPerComponent);
			return DPTX_RETURN_EINVAL;
		}
		break;
	case PIXEL_ENCODING_TYPE_YCBCR422:
		if (pstVideoParams->ucBitPerComponent == (uint8_t)COLOR_DEPTH_8) {
			ucVideoMapping = 9;
		} else {
			dptx_err("Invalid bpc = %d ", pstVideoParams->ucBitPerComponent);
			return DPTX_RETURN_EINVAL;
		}
		break;
	case PIXEL_ENCODING_TYPE_YCBCR444:
		if (pstVideoParams->ucBitPerComponent == (uint8_t)COLOR_DEPTH_8) {
			ucVideoMapping = 5;
		} else {
			dptx_err("Invalid bpc = %d ", pstVideoParams->ucBitPerComponent);
			return DPTX_RETURN_EINVAL;
		}
		break;
	default:
		dptx_err("Invalid encoding type = %d", (uint32_t)pstVideoParams->ucPixel_Encoding);
		return DPTX_RETURN_EINVAL;
	}

	uiRegMap_VidSampleCtrl |= (ucVideoMapping << DPTX_VSAMPLE_CTRL_VMAP_BPC_SHIFT);
	uiRegMap_VidSampleCtrl |= (pstDptx->ucMultiPixel << DPTX_VSAMPLE_CTRL_MULTI_PIXEL_SHIFT);

	Dptx_Reg_Writel(pstDptx, DPTX_VSAMPLE_CTRL_N(ucStream_Index), uiRegMap_VidSampleCtrl);

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_vidin_set_video_config(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	uint32_t uiVideo_Code, uiRegMap_VidConfig = 0;
	struct Dptx_Video_Params *pstVideoParams = &pstDptx->stVideoParams;
	struct Dptx_Dtd_Params *pstDtd = &pstVideoParams->stDtdParams[ucStream_Index];

{
	uiVideo_Code = pstVideoParams->auiVideo_Code[ucStream_Index];

	if (pstVideoParams->ucVideo_Format == CEA_861) {
		if (uiVideo_Code == 5 || uiVideo_Code == 6 || uiVideo_Code == 7 ||
			uiVideo_Code == 10 || uiVideo_Code == 11 || uiVideo_Code == 20 ||
			uiVideo_Code == 21 || uiVideo_Code == 22 || uiVideo_Code == 39 ||
			uiVideo_Code == 25 || uiVideo_Code == 26 || uiVideo_Code == 40 ||
			uiVideo_Code == 44 || uiVideo_Code == 45 || uiVideo_Code == 46 ||
			uiVideo_Code == 50 || uiVideo_Code == 51 || uiVideo_Code == 54 ||
			uiVideo_Code == 55 || uiVideo_Code == 58 || uiVideo_Code  == 59) {
			uiRegMap_VidConfig |= DPTX_VIDEO_CONFIG1_IN_OSC_EN;
		}
	}

	if (pstDtd->interlaced == 1)
		uiRegMap_VidConfig |= DPTX_VIDEO_CONFIG1_O_IP_EN;

	uiRegMap_VidConfig |= (pstDtd->h_active << DPTX_VIDEO_H_ACTIVE_SHIFT);
	uiRegMap_VidConfig |= (pstDtd->h_blanking << DPTX_VIDEO_H_BLANK_SHIFT);

	Dptx_Reg_Writel(pstDptx, DPTX_VIDEO_CONFIG1_N(ucStream_Index), uiRegMap_VidConfig);
}

{
	uiRegMap_VidConfig = 0;

	uiRegMap_VidConfig |= (pstDtd->v_active << DPTX_VIDEO_V_ACTIVE_SHIFT);
	uiRegMap_VidConfig |= (pstDtd->v_blanking << DPTX_VIDEO_V_BLANK_SHIFT);

	Dptx_Reg_Writel(pstDptx, DPTX_VIDEO_CONFIG2_N(ucStream_Index), uiRegMap_VidConfig);
}

{
	uiRegMap_VidConfig = 0;

	uiRegMap_VidConfig |= (pstDtd->h_sync_offset << DPTX_VIDEO_H_FRONT_PORCH);
	uiRegMap_VidConfig |= (pstDtd->h_sync_pulse_width << DPTX_VIDEO_H_SYNC_WIDTH);

	Dptx_Reg_Writel(pstDptx, DPTX_VIDEO_CONFIG3_N(ucStream_Index), uiRegMap_VidConfig);
}

{
	uiRegMap_VidConfig = 0;

	uiRegMap_VidConfig |= (pstDtd->v_sync_offset << DPTX_VIDEO_V_FRONT_PORCH);
	uiRegMap_VidConfig |= (pstDtd->v_sync_pulse_width << DPTX_VIDEO_V_SYNC_WIDTH);

	Dptx_Reg_Writel(pstDptx, DPTX_VIDEO_CONFIG4_N(ucStream_Index), uiRegMap_VidConfig);
}

{
	uiRegMap_VidConfig = Dptx_Reg_Readl(pstDptx, DPTX_VIDEO_CONFIG5_N(ucStream_Index));

	uiRegMap_VidConfig = uiRegMap_VidConfig & (~DPTX_VIDEO_CONFIG5_TU_MASK);
	uiRegMap_VidConfig = (uiRegMap_VidConfig | (pstVideoParams->ucAverage_BytesPerTu << DPTX_VIDEO_CONFIG5_TU_SHIFT));

	if (pstDptx->bMultStreamTransport) {
		uiRegMap_VidConfig &= (~DPTX_VIDEO_CONFIG5_TU_FRAC_MASK_MST);
		uiRegMap_VidConfig |= (pstVideoParams->ucAver_BytesPer_Tu_Frac << DPTX_VIDEO_CONFIG5_TU_FRAC_SHIFT_MST);
	} else {
		uiRegMap_VidConfig &= (~DPTX_VIDEO_CONFIG5_TU_FRAC_MASK_SST);
		uiRegMap_VidConfig |= (pstVideoParams->ucAver_BytesPer_Tu_Frac << DPTX_VIDEO_CONFIG5_TU_FRAC_SHIFT_SST);
	}

	uiRegMap_VidConfig &= (~DPTX_VIDEO_CONFIG5_INIT_THRESHOLD_MASK);
	uiRegMap_VidConfig |= (pstVideoParams->ucInit_Threshold << DPTX_VIDEO_CONFIG5_INIT_THRESHOLD_SHIFT);

	Dptx_Reg_Writel(pstDptx, DPTX_VIDEO_CONFIG5_N(ucStream_Index), uiRegMap_VidConfig);
}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_vidin_set_video_msa(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	uint8_t	ucBpc_Mapping = 0, ucColorimetry_Mapping = 0;
	uint32_t uiRegMap_Msa1 = 0, uiRegMap_Msa2, uiRegMap_Msa3;
	struct Dptx_Video_Params *pstVideoParams =	&pstDptx->stVideoParams;
	struct Dptx_Dtd_Params	*pstDtd = &pstVideoParams->stDtdParams[ucStream_Index];

{
	uiRegMap_Msa1 |= ((pstDtd->h_blanking - pstDtd->h_sync_offset) << DPTX_VIDEO_MSA1_H_START_SHIFT);
	uiRegMap_Msa1 |= ((pstDtd->v_blanking - pstDtd->v_sync_offset) << DPTX_VIDEO_MSA1_V_START_SHIFT);

	Dptx_Reg_Writel(pstDptx, DPTX_VIDEO_MSA1_N(ucStream_Index), uiRegMap_Msa1);
}

{
	uiRegMap_Msa2 = Dptx_Reg_Readl(pstDptx, DPTX_VIDEO_MSA2_N(ucStream_Index));

	uiRegMap_Msa2 &= ~(DPTX_VIDEO_VMSA2_BPC_MASK | DPTX_VIDEO_VMSA2_COL_MASK);

	switch ((enum PIXEL_ENCODING_TYPE)pstVideoParams->ucPixel_Encoding) {
	case PIXEL_ENCODING_TYPE_RGB:
		if (pstVideoParams->ucRGB_Standard == S_RGB)
			ucColorimetry_Mapping = 4;
		else if (pstVideoParams->ucRGB_Standard == Legacy_RGB)
			ucColorimetry_Mapping = 0;
		break;
	case PIXEL_ENCODING_TYPE_YCBCR422:
		if (pstVideoParams->ucColorimetry == ITU601)
			ucColorimetry_Mapping = 5;
		else if (pstVideoParams->ucColorimetry == ITU709)
			ucColorimetry_Mapping = 13;
		break;
	case PIXEL_ENCODING_TYPE_YCBCR444:
		if (pstVideoParams->ucColorimetry == ITU601)
			ucColorimetry_Mapping = 6;
		else if (pstVideoParams->ucColorimetry == ITU709)
			ucColorimetry_Mapping = 14;
		break;
	default:
		dptx_err("Invalid encoding type = 0x%x ", pstVideoParams->ucPixel_Encoding);
		return DPTX_RETURN_EINVAL;
	}

	switch ((enum PIXEL_ENCODING_TYPE)pstVideoParams->ucPixel_Encoding) {
	case PIXEL_ENCODING_TYPE_RGB:
		if (pstVideoParams->ucBitPerComponent == COLOR_DEPTH_8) {
			ucBpc_Mapping = 1;
		} else {
			dptx_err("Invalid bpc = %d ", pstVideoParams->ucBitPerComponent);
			return DPTX_RETURN_EINVAL;
		}
		break;
	case PIXEL_ENCODING_TYPE_YCBCR422:
	case PIXEL_ENCODING_TYPE_YCBCR444:
		if (pstVideoParams->ucBitPerComponent == COLOR_DEPTH_8) {
			ucBpc_Mapping = 1;
		} else {
			dptx_err("Invalid bpc = %d ", pstVideoParams->ucBitPerComponent);
			return DPTX_RETURN_EINVAL;
		}
		break;
	default:
		dptx_err("Invalid encoding type = 0x%x ", pstVideoParams->ucPixel_Encoding);
		return DPTX_RETURN_EINVAL;
	}

	uiRegMap_Msa2 |= (ucColorimetry_Mapping << DPTX_VIDEO_VMSA2_COL_SHIFT);
	uiRegMap_Msa2 |= (ucBpc_Mapping << DPTX_VIDEO_VMSA2_BPC_SHIFT);

	Dptx_Reg_Writel(pstDptx, DPTX_VIDEO_MSA2_N(ucStream_Index), uiRegMap_Msa2);
}

{
	uiRegMap_Msa3 = Dptx_Reg_Readl(pstDptx, DPTX_VIDEO_MSA3_N(ucStream_Index));

	uiRegMap_Msa3 &= ~DPTX_VIDEO_VMSA3_PIX_ENC_MASK;

	Dptx_Reg_Writel(pstDptx, DPTX_VIDEO_MSA3_N(ucStream_Index), uiRegMap_Msa3);
}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_vidin_set_video_hblank_interval(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	uint32_t uiRegMap_VidHblankInterval;
	uint32_t uiLink_Clock, uiHBlank_Interval;
	struct Dptx_Video_Params	*pstVideoParams = &pstDptx->stVideoParams;
	struct Dptx_Dtd_Params		*pstDtd = &pstVideoParams->stDtdParams[ucStream_Index];

	switch (pstDptx->stDptxLink.ucLinkRate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		uiLink_Clock = 40500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		uiLink_Clock = 67500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		uiLink_Clock = 135000;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		uiLink_Clock = 202500;
		break;
	default:
		dptx_err("Invalid rate 0x%x ", pstDptx->stDptxLink.ucLinkRate);
		return DPTX_RETURN_EINVAL;
	}

	if (pstDptx->bMultStreamTransport)
		uiHBlank_Interval = ((pstDtd->h_blanking / 16) * pstVideoParams->ucAverage_BytesPerTu * uiLink_Clock / pstDtd->uiPixel_Clock);
	else
		uiHBlank_Interval = (pstDtd->h_blanking * uiLink_Clock / pstDtd->uiPixel_Clock);

	uiRegMap_VidHblankInterval = uiHBlank_Interval;

	uiRegMap_VidHblankInterval |= (DPTX_VIDEO_HBLANK_INTERVAL_ENABLE << DPTX_VIDEO_HBLANK_INTERVAL_SHIFT);

	Dptx_Reg_Writel(pstDptx, DPTX_VIDEO_HBLANK_INTERVAL_N(ucStream_Index), uiRegMap_VidHblankInterval);

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_vidin_config_video_input(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	int32_t iRetVal;
	struct Dptx_Video_Params *pstVideoParams = &pstDptx->stVideoParams;
	struct Dptx_Dtd_Params *pstDtd = &pstVideoParams->stDtdParams[ucStream_Index];

	iRetVal = dptx_vidin_set_video_sampler(pstDptx, ucStream_Index);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

{
	uint32_t uiRegMap_VidPolarityCtrl = 0;

	if (pstDtd->h_sync_polarity == 1)
		uiRegMap_VidPolarityCtrl |= DPTX_POL_CTRL_H_SYNC_POL_EN;

	if (pstDtd->v_sync_polarity == 1)
		uiRegMap_VidPolarityCtrl |= DPTX_POL_CTRL_V_SYNC_POL_EN;

	Dptx_Reg_Writel(pstDptx, DPTX_VSAMPLE_POLARITY_CTRL_N(ucStream_Index), uiRegMap_VidPolarityCtrl);
}

	iRetVal = dptx_vidin_set_video_config(pstDptx, ucStream_Index);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	iRetVal = dptx_vidin_set_video_msa(pstDptx, ucStream_Index);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	iRetVal = dptx_vidin_set_video_hblank_interval(pstDptx, ucStream_Index);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

static void dptx_vidin_reset_dtd(struct Dptx_Dtd_Params *pstDtd)
{
	pstDtd->pixel_repetition_input	= 0;
	pstDtd->uiPixel_Clock			= 0;
	pstDtd->h_active				= 0;
	pstDtd->h_blanking				= 0;
	pstDtd->h_sync_offset			= 0;
	pstDtd->h_sync_pulse_width		= 0;
	pstDtd->h_image_size			= 0;
	pstDtd->v_active				= 0;
	pstDtd->v_blanking				= 0;
	pstDtd->v_sync_offset			= 0;
	pstDtd->v_sync_pulse_width		= 0;
	pstDtd->v_image_size			= 0;
	pstDtd->interlaced				= 0;
	pstDtd->v_sync_polarity			= 0;
	pstDtd->h_sync_polarity			= 0;
}

int32_t Dptx_Vidin_Init(struct Dptx_Params *pstDptx, uint8_t ucPixelEncodingType)
{
	struct Dptx_Video_Params *pstVideoParams = &pstDptx->stVideoParams;

	memset(&pstVideoParams->aucInput_Mute[0], 0, (sizeof(uint8_t) * PHY_INPUT_STREAM_MAX));

	pstVideoParams->ucBitPerComponent		= COLOR_DEPTH_8;
	pstVideoParams->ucPixel_Encoding		= ucPixelEncodingType;
	pstVideoParams->ucColorimetry			= ITU601;
	pstVideoParams->ucRGB_Standard			= Legacy_RGB;
	pstVideoParams->ucAverage_BytesPerTu	= 30;
	pstVideoParams->ucAver_BytesPer_Tu_Frac	= 0;
	pstVideoParams->ucInit_Threshold		= 15;
	pstVideoParams->ucPattern_Mode			= RAMP;
	pstVideoParams->uiRefresh_Rate			= 60000;
	pstVideoParams->ucVideo_Format			= CEA_861;

	(void)memcpy((void *)pstVideoParams->auiVideo_Code, (void *)pstDptx->auiVIC, sizeof(uint32_t) * PHY_INPUT_STREAM_MAX);

//	Dptx_Vidin_Disable_Audio_Timestamp(pstDptx);
//	Dptx_Vidin_Disable_Audio_SDP(pstDptx);

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Vidin_Set_Video_PPClk(struct Dptx_Params *pstDptx, uint8_t ucNumOfStreams, uint32_t auiPeri_PixelClock[PHY_INPUT_STREAM_MAX])
{
	struct Dptx_Video_Params *pstVideoParams = &pstDptx->stVideoParams;

	if (auiPeri_PixelClock == NULL) {
		dptx_err("Invalid parameter as auiPeri_PixelClock == NULL");
		return DPTX_RETURN_EINVAL;
	}

	for (uint8_t ucElements = 0; ucElements < ucNumOfStreams; ucElements++)
		pstVideoParams->uiPeri_Pixel_Clock[ucElements] = auiPeri_PixelClock[ucElements];

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Vidin_Set_Video_TimingChange_FromVIC(struct Dptx_Params *pstDptx, uint32_t uiVideo_Code, uint8_t ucStream_Index)
{
	int32_t	iRetVal;
	struct Dptx_Dtd_Params stDtd;
	struct Dptx_Video_Params *pstVideoParams = &pstDptx->stVideoParams;

	if (ucStream_Index >= PHY_INPUT_STREAM_MAX) {
		dptx_err("Invalid stream index(%d) ", ucStream_Index);
		return DPTX_RETURN_EINVAL;
	}

	if ((!pstDptx->bMultStreamTransport) && (ucStream_Index >= PHY_INPUT_STREAM_1)) {
		dptx_err("Invalid stream index with MST(%d) ", ucStream_Index);
		return DPTX_RETURN_EINVAL;
	}

	iRetVal = Dptx_Vidin_Fill_Dtd(&stDtd, uiVideo_Code, pstVideoParams->uiRefresh_Rate, pstVideoParams->ucVideo_Format);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	dptx_dbg("Stream %d :  ", ucStream_Index);
	dptx_dbg("Video code: %d", uiVideo_Code);
	dptx_dbg("Refresh rate: %d", pstVideoParams->uiRefresh_Rate);
	dptx_dbg("DTD format: %s", pstVideoParams->ucVideo_Format == CEA_861 ? "CEA_861":"Others");
	dptx_dbg("Pixel Clk from %d -> %d", stDtd.uiPixel_Clock, pstVideoParams->uiPeri_Pixel_Clock[ucStream_Index]);

	stDtd.uiPixel_Clock = pstVideoParams->uiPeri_Pixel_Clock[ucStream_Index];

	memcpy(&pstVideoParams->stDtdParams[ucStream_Index], &stDtd, sizeof(struct Dptx_Dtd_Params));

	iRetVal = Dptx_Vidin_Calculate_Video_Average_TU_Symbols(pstDptx,
																	pstDptx->stDptxLink.ucNumOfLanes,
																	pstDptx->stDptxLink.ucLinkRate,
																	pstVideoParams->ucBitPerComponent,
																	pstVideoParams->ucPixel_Encoding,
																	stDtd.uiPixel_Clock,
																	ucStream_Index);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	pstVideoParams->auiVideo_Code[ucStream_Index] = uiVideo_Code;

	iRetVal = Dptx_Vidin_Set_Video_TimingChange(pstDptx, ucStream_Index);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Vidin_Set_Video_TimingChange(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	int32_t iRetVal;

	Dptx_Vidin_Disable_Video_Stream(pstDptx, ucStream_Index);

	iRetVal = dptx_vidin_config_video_input(pstDptx, ucStream_Index);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	Dptx_Vidin_Enable_Video_Stream(pstDptx, ucStream_Index);

	return DPTX_RETURN_NO_ERROR;
}

void Dptx_Vidin_Enable_Video_Stream(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	uint32_t uiRegMap_VSampleCtrl;

	pstDptx->stVideoParams.aucInput_Mute[ucStream_Index] = 0;

	uiRegMap_VSampleCtrl = Dptx_Reg_Readl(pstDptx, DPTX_VSAMPLE_CTRL_N(ucStream_Index));

	uiRegMap_VSampleCtrl |= DPTX_VSAMPLE_CTRL_STREAM_EN;

	Dptx_Reg_Writel(pstDptx, DPTX_VSAMPLE_CTRL_N(ucStream_Index), uiRegMap_VSampleCtrl);
}

void Dptx_Vidin_Disable_Video_Stream(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	uint32_t uiRegMap_VSampleCtrl;

	pstDptx->stVideoParams.aucInput_Mute[ucStream_Index] = 1;

	uiRegMap_VSampleCtrl = Dptx_Reg_Readl(pstDptx, DPTX_VSAMPLE_CTRL_N(ucStream_Index));

	uiRegMap_VSampleCtrl &= ~DPTX_VSAMPLE_CTRL_STREAM_EN;

	Dptx_Reg_Writel(pstDptx, DPTX_VSAMPLE_CTRL_N(ucStream_Index), uiRegMap_VSampleCtrl);
}

int32_t Dptx_Vidin_Calculate_Video_Average_TU_Symbols(struct Dptx_Params *pstDptx, int iNumOfLane, int iLinkRate, int iBpc, int iEncodingType, int iPixel_Clock, uint8_t ucStream_Index)
{
	int iLink_Rate, iLink_Clock, iColor_Depth, iT1 = 0, iT2 = 0;
	int iH_AverageSymbol_Per_TU, iAverageSymbol_Per_TU, iTransfer_Unit_Fraction;
	struct Dptx_Video_Params *pstVideoParams = &pstDptx->stVideoParams;
	struct Dptx_Dtd_Params *pstDtd = &pstVideoParams->stDtdParams[ucStream_Index];

	switch (iLinkRate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		iLink_Rate = 162;
		iLink_Clock = 40500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		iLink_Rate = 270;
		iLink_Clock = 67500;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		iLink_Rate = 540;
		iLink_Clock = 135000;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		iLink_Rate = 810;
		iLink_Clock = 202500;
		break;
	default:
		dptx_err("Invalid rate param = %d", iLinkRate);
		return DPTX_RETURN_EINVAL;
	}

	switch ((enum VIDEO_PIXEL_COLOR_DEPTH)iBpc) {
	case COLOR_DEPTH_8:
		iColor_Depth = (iEncodingType == PIXEL_ENCODING_TYPE_YCBCR422) ? VIDEO_LINK_BPP_YCbCr422 : VIDEO_LINK_BPP_RGB_YCbCr444;
		iColor_Depth *= COLOR_DEPTH_8;
		break;
	default:
		dptx_err("Invalid Bpc(%d)", iBpc);
		return DPTX_RETURN_EINVAL;
	}

	iH_AverageSymbol_Per_TU = ((8 * iColor_Depth * iPixel_Clock) / (iNumOfLane * iLink_Rate));
	iAverageSymbol_Per_TU  = (iH_AverageSymbol_Per_TU / 1000);
	if (iAverageSymbol_Per_TU > DPTX_MAX_LINK_SYMBOLS) {
		dptx_err("iTransfer_Unit(%d) > DPTX_MAX_LINK_SYMBOLS", iAverageSymbol_Per_TU);
		return DPTX_RETURN_ENODEV;
	}

	iTransfer_Unit_Fraction = ((iH_AverageSymbol_Per_TU / 100) - (iAverageSymbol_Per_TU * 10));

	pstVideoParams->ucAverage_BytesPerTu	= iAverageSymbol_Per_TU;
	pstVideoParams->ucAver_BytesPer_Tu_Frac = iTransfer_Unit_Fraction;

	if (pstDptx->ucMultiPixel == DPTX_MP_SINGLE_PIXEL) {
		if (iAverageSymbol_Per_TU < 6)
			pstVideoParams->ucInit_Threshold = 32;
		else if (pstDtd->h_blanking <= 80)
			pstVideoParams->ucInit_Threshold = 12;
		else
			pstVideoParams->ucInit_Threshold = 16;
	} else {
		switch ((enum VIDEO_PIXEL_COLOR_DEPTH)iBpc) {
		case COLOR_DEPTH_8:
			if ((iEncodingType == PIXEL_ENCODING_TYPE_RGB) || (iEncodingType == PIXEL_ENCODING_TYPE_YCBCR444)) {
				if (pstDptx->ucMultiPixel == DPTX_MP_DUAL_PIXEL)
					iT1 = ((1000 / 3) *iNumOfLane);
				else
					iT1 = ((3000 / 16) * iNumOfLane);
			} else if (iEncodingType == PIXEL_ENCODING_TYPE_YCBCR422) {
				iT1 = ((1000 / 2) *iNumOfLane);
			} else {
				dptx_err("Invalid encoding type(%d)", iEncodingType);
				return DPTX_RETURN_EINVAL;
			}
			break;
		default:
			dptx_err("Invalid param iBpc = %d", iBpc);
			return DPTX_RETURN_EINVAL;
		}

		iT2 = ((iLink_Clock * 1000) / iPixel_Clock);

		pstVideoParams->ucInit_Threshold = (iT1 * iT2 * iAverageSymbol_Per_TU / (1000 * 1000));
	}

	if (!pstDptx->bMultStreamTransport) {
		dptx_info("SST %s Pixel => ", pstDptx->ucMultiPixel == DPTX_MP_SINGLE_PIXEL ? "Single" :
											pstDptx->ucMultiPixel == DPTX_MP_DUAL_PIXEL ? "Dual" : "Quad");
		dptx_info(" -.Pixel clock: %d", iPixel_Clock);
		dptx_info(" -.Num of Lanes: %d ", iNumOfLane);
		dptx_info(" -.Link rate: %s", iLinkRate == DPTX_PHYIF_CTRL_RATE_RBR ? "RBR" :
										(iLinkRate == DPTX_PHYIF_CTRL_RATE_HBR) ? "HBR" :
										(iLinkRate == DPTX_PHYIF_CTRL_RATE_HBR2) ? "HBR2" : "HBR3");
		dptx_info(" -.VCP Id: %d", pstDptx->aucVCP_Id[ucStream_Index]);
		dptx_info(" -.Link Bpc: %d", iLink_Rate);
		dptx_info(" -.Link Clk: %d", iLink_Clock);
		dptx_info(" -.Average Symbols per TU: %d <- (( 8 * %d * %d ) / ( %d * %d ))",
										pstVideoParams->ucAverage_BytesPerTu,
										iColor_Depth,
										iPixel_Clock,
										iNumOfLane,
										iLink_Rate);
		dptx_info(" -.Fraction per TU: %d -> %d", iH_AverageSymbol_Per_TU,
													pstVideoParams->ucAver_BytesPer_Tu_Frac);
		dptx_info(" -.Init thresh: %d",	pstVideoParams->ucInit_Threshold);
	}

	if (pstDptx->bMultStreamTransport)   {
		uint32_t uiAverage_BytesPerTu, uiAver_BytesPer_Tu_Frac;
		int iNumerator, iDenominator;

		iColor_Depth = (iEncodingType == PIXEL_ENCODING_TYPE_YCBCR422) ? VIDEO_LINK_BPP_YCbCr422 : VIDEO_LINK_BPP_RGB_YCbCr444;
		iColor_Depth *= COLOR_DEPTH_8;

		iNumerator = (8 * iColor_Depth * iPixel_Clock);
		iDenominator = (iLink_Rate * iNumOfLane);

		uiAverage_BytesPerTu = ((iNumerator / iDenominator) / 1000);
		uiAver_BytesPer_Tu_Frac = ((((iNumerator / iDenominator) - (uiAverage_BytesPerTu * 1000)) * 64) / 1000);

		pstVideoParams->ucAverage_BytesPerTu	= (uint8_t)uiAverage_BytesPerTu;
		pstVideoParams->ucAver_BytesPer_Tu_Frac = (uint8_t)uiAver_BytesPer_Tu_Frac;

		dptx_info("MST %s Pixel : ", pstDptx->ucMultiPixel == DPTX_MP_SINGLE_PIXEL ? "Single" :
										pstDptx->ucMultiPixel == DPTX_MP_DUAL_PIXEL ? "Dual":"Quad");
		dptx_info(" -.Pixel clock: %d", iPixel_Clock);
		dptx_info(" -.Num of Lanes: %d", iNumOfLane);
		dptx_info(" -.Link rate: %s", iLinkRate == DPTX_PHYIF_CTRL_RATE_RBR ? "RBR" :
											(iLinkRate == DPTX_PHYIF_CTRL_RATE_HBR) ? "HBR" :
											(iLinkRate == DPTX_PHYIF_CTRL_RATE_HBR2) ? "HBR2":"HBR3");
		dptx_info(" -.VCP Id: %d", pstDptx->aucVCP_Id[ucStream_Index]);
		dptx_info(" -.Link Bpc: %d", iLink_Rate);
		dptx_info(" -.Link Clk: %d", iLink_Clock);
		dptx_info(" -.Average Symbols per TU: %d <- (( 8 * %d * %d ) / ( %d * %d ))",
										pstVideoParams->ucAverage_BytesPerTu,
										iColor_Depth,
										iPixel_Clock,
										iNumOfLane,
										iLink_Rate);
		dptx_info(" -.Fraction Per TU: %d", pstVideoParams->ucAver_BytesPer_Tu_Frac);
		dptx_info(" -.Init thresh: %d", pstVideoParams->ucInit_Threshold);
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Vidin_Parse_Dtd(struct Dptx_Dtd_Params *pstDtd, uint8_t aucData[18])
{
	pstDtd->pixel_repetition_input = 0;

	pstDtd->uiPixel_Clock = ((aucData[1] << 8) | aucData[0]);
	if (pstDtd->uiPixel_Clock < 0x01) {
		dptx_dbg("Invalid pixel clock %d ... ", pstDtd->uiPixel_Clock);
		return DPTX_RETURN_ENODEV;
	}

	dptx_dbg("Pixel clock %d ... ", pstDtd->uiPixel_Clock);

	pstDtd->h_active = dptx_concat_bits(aucData[4], 4, 4, aucData[2], 0, 8);
	pstDtd->h_blanking = dptx_concat_bits(aucData[4], 0, 4, aucData[3], 0, 8);
	pstDtd->h_sync_offset = dptx_concat_bits(aucData[11], 6, 2, aucData[8], 0, 8);
	pstDtd->h_sync_pulse_width = dptx_concat_bits(aucData[11], 4, 2, aucData[9], 0, 8);
	pstDtd->h_image_size = dptx_concat_bits(aucData[14], 4, 4, aucData[12], 0, 8);

	pstDtd->v_active = dptx_concat_bits(aucData[7], 4, 4, aucData[5], 0, 8);
	pstDtd->v_blanking = dptx_concat_bits(aucData[7], 0, 4, aucData[6], 0, 8);
	pstDtd->v_sync_offset = dptx_concat_bits(aucData[11], 2, 2, aucData[10], 4, 4);
	pstDtd->v_sync_pulse_width = dptx_concat_bits(aucData[11], 0, 2,	aucData[10], 0, 4);
	pstDtd->v_image_size = dptx_concat_bits(aucData[14], 0, 4, aucData[13], 0, 8);

	if (dptx_bit_field(aucData[17], 4, 1) != 1) {
		dptx_err("Invalid bit field ... ");
		return DPTX_RETURN_ENODEV;
	}

	if (dptx_bit_field(aucData[17], 3, 1) != 1) {
		dptx_err("Invalid bit field ... ");
		return DPTX_RETURN_ENODEV;
	}

	pstDtd->interlaced = dptx_bit_field(aucData[17], 7, 1) == 1;
	pstDtd->v_sync_polarity = dptx_bit_field(aucData[17], 2, 1) == 0;
	pstDtd->h_sync_polarity	 = dptx_bit_field(aucData[17], 1, 1) == 0;

	if (pstDtd->interlaced == 1)
		pstDtd->v_active /= 2;

	pstDtd->uiPixel_Clock *= 10;

	dptx_dbg("Getting preferred video timing from edid => ");
	dptx_dbg(" -.Pixel clock: %d", pstDtd->uiPixel_Clock);
	dptx_dbg("-.interlaced: %d",  pstDtd->interlaced);
	dptx_dbg("-. h_active: %d h_blanking: %d h_sync_offset: %d",
								pstDtd->h_active,
								pstDtd->h_blanking,
								pstDtd->h_sync_offset);
	dptx_dbg("-. h_sync_pulse_width: %d h_image_size: %d h_sync_polarity: %d",
								pstDtd->h_sync_pulse_width,
								pstDtd->h_image_size,
								pstDtd->h_sync_polarity);
	dptx_dbg("-. v_active: %d v_blanking: %d v_sync_offset: %d",
								pstDtd->v_active,
								pstDtd->v_blanking,
								pstDtd->v_sync_offset);
	dptx_dbg("-. v_sync_pulse_width: %d v_image_size: %d v_sync_polarity: %d\n",
								pstDtd->v_sync_pulse_width,
								pstDtd->v_image_size,
								pstDtd->v_sync_polarity);

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Vidin_Fill_DTD_BasedOn_EST_Timings(struct Dptx_Params *pstDptx, struct Dptx_Dtd_Params *pstDTD)
{
	int32_t	iRetVal;
	uint32_t uiVideo_Code;
	struct Dptx_Video_Params *pstVideoParams = &pstDptx->stVideoParams;

	switch (pstDptx->eEstablished_Timing) {
	case DMT_640x480_60hz:
		pstVideoParams->ucVideo_Format = VESA_DMT;
		uiVideo_Code = 4;
		dptx_dbg("Set Video mode to DMT 640x480");
		break;
	case DMT_800x600_60hz:
		pstVideoParams->ucVideo_Format = VESA_DMT;
		uiVideo_Code = 9;
		dptx_dbg("Set Video mode to DMT 800x600");
		break;
	case DMT_1024x768_60hz:
		pstVideoParams->ucVideo_Format = VESA_DMT;
		uiVideo_Code = 16;
		dptx_dbg("Set Video mode to DMT 1024x768");
		break;
	case DMT_NONE:
	default:
		dptx_err("Not Found selected timing in Established timings ");
		return DPTX_RETURN_EINVAL;
	}

	iRetVal = Dptx_Vidin_Fill_Dtd(pstDTD, uiVideo_Code, pstVideoParams->uiRefresh_Rate, pstVideoParams->ucVideo_Format);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	return DPTX_RETURN_NO_ERROR;
}


int32_t Dptx_Vidin_Fill_Dtd(struct Dptx_Dtd_Params *pstDtd, uint32_t uiVideo_Code, uint32_t uiRefreshRate, uint8_t ucVideoFormat)
{
	dptx_vidin_reset_dtd(pstDtd);

	pstDtd->h_image_size = 16;
	pstDtd->v_image_size = 9;

	if (ucVideoFormat == CEA_861) {
		switch (uiVideo_Code) {
		case DP_CUSTOM_1025_DTD_VIC:
			pstDtd->h_active = 1024;
			pstDtd->v_active = 600;
			pstDtd->h_blanking = 320;
			pstDtd->v_blanking = 35;
			pstDtd->h_sync_offset = 150;
			pstDtd->v_sync_offset = 15;
			pstDtd->h_sync_pulse_width = 20;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 51200;
			break;
		case DP_CUSTOM_1026_DTD_VIC:
			pstDtd->h_active = 5760;
			pstDtd->v_active = 900;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 26;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case DP_CUSTOM_1027_DTD_VIC:
			pstDtd->h_active = 1920;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 64;
			pstDtd->v_blanking = 21;
			pstDtd->h_sync_offset = 30;
			pstDtd->v_sync_offset = 10;
			pstDtd->h_sync_pulse_width = 4;
			pstDtd->v_sync_pulse_width = 2;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 88200;
			break;
		case 1: /* 640x480p @ 59.94/60Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 640;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 16;
			pstDtd->v_sync_offset = 10;
			pstDtd->h_sync_pulse_width = 96;
			pstDtd->v_sync_pulse_width = 2;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
		pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 25175;
			break;
		case 2: /* 720x480p @ 59.94/60Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 720;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 138;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 16;
			pstDtd->v_sync_offset = 9;
			pstDtd->h_sync_pulse_width = 62;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 27000;
			break;
		case 3: /* 720x480p @ 59.94/60Hz 16:9 */
			pstDtd->h_active = 720;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 138;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 16;
			pstDtd->v_sync_offset = 9;
			pstDtd->h_sync_pulse_width = 62;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 27000;
			break;
		case 69:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 370;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 110;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 4: /* 1280x720p @ 59.94/60Hz 16:9 */
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 370;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 110;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 5: /* 1920x1080i @ 59.94/60Hz 16:9 */
			pstDtd->h_active = 1920;
			pstDtd->v_active = 540;
			pstDtd->h_blanking = 280;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 6: /* 720(1440)x480i @ 59.94/60Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1440;
			pstDtd->v_active = 240;
			pstDtd->h_blanking = 276;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 38;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 124;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 27000;
			break;
		case 7: /* 720(1440)x480i @ 59.94/60Hz 16:9 */
			pstDtd->h_active = 1440;
			pstDtd->v_active = 240;
			pstDtd->h_blanking = 276;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 38;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 124;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 27000;
			break;
		case 8: /* 720(1440)x240p @ 59.826/60.054/59.886/60.115Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1440;
			pstDtd->v_active = 240;
			pstDtd->h_blanking = 276;
			pstDtd->v_blanking = (uiRefreshRate == 59940) ? 22 : 23;
			pstDtd->h_sync_offset = 38;
			pstDtd->v_sync_offset = (uiRefreshRate == 59940) ? 4 : 5;
			pstDtd->h_sync_pulse_width = 124;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 27000;
			break;
		case 9: /* 720(1440)x240p @59.826/60.054/59.886/60.115Hz 16:9 */
			pstDtd->h_active = 1440;
			pstDtd->v_active = 240;
			pstDtd->h_blanking = 276;
			pstDtd->v_blanking = (uiRefreshRate == 59940) ? 22 : 23;
			pstDtd->h_sync_offset = 38;
			pstDtd->v_sync_offset = (uiRefreshRate == 59940) ? 4 : 5;
			pstDtd->h_sync_pulse_width = 124;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 27000;
			break;
		case 10: /* 2880x480i @ 59.94/60Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 2880;
			pstDtd->v_active = 240;
			pstDtd->h_blanking = 552;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 76;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 248;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 11: /* 2880x480i @ 59.94/60Hz 16:9 */
			pstDtd->h_active = 2880;
			pstDtd->v_active = 240;
			pstDtd->h_blanking = 552;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 76;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 248;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 12: /* 2880x240p @ 59.826/60.054/59.886/60.115Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 2880;
			pstDtd->v_active = 240;
			pstDtd->h_blanking = 552;
			pstDtd->v_blanking = (uiRefreshRate == 60054) ? 22 : 23;
			pstDtd->h_sync_offset = 76;
			pstDtd->v_sync_offset = (uiRefreshRate == 60054) ? 4 : 5;
			pstDtd->h_sync_pulse_width = 248;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 13: /* 2880x240p @ 59.826/60.054/59.886/60.115Hz 16:9 */
			pstDtd->h_active = 2880;
			pstDtd->v_active = 240;
			pstDtd->h_blanking = 552;
			pstDtd->v_blanking = (uiRefreshRate == 60054) ? 22 : 23;
			pstDtd->h_sync_offset = 76;
			pstDtd->v_sync_offset = (uiRefreshRate == 60054) ? 4 : 5;
			pstDtd->h_sync_pulse_width = 248;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 14: /* 1440x480p @ 59.94/60Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1440;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 276;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 32;
			pstDtd->v_sync_offset = 9;
			pstDtd->h_sync_pulse_width = 124;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 15: /* 1440x480p @ 59.94/60Hz 16:9 */
			pstDtd->h_active = 1440;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 276;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 32;
			pstDtd->v_sync_offset = 9;
			pstDtd->h_sync_pulse_width = 124;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 76:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 280;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 148500;
			break;
		case 16: /* 1920x1080p @ 59.94/60Hz 16:9 */
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 280;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 148500;
			break;
		case 17: /* 720x576p @ 50Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 720;
			pstDtd->v_active = 576;
			pstDtd->h_blanking = 144;
			pstDtd->v_blanking = 49;
			pstDtd->h_sync_offset = 12;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 64;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 27000;
			break;
		case 18: /* 720x576p @ 50Hz 16:9 */
			pstDtd->h_active = 720;
			pstDtd->v_active = 576;
			pstDtd->h_blanking = 144;
			pstDtd->v_blanking = 49;
			pstDtd->h_sync_offset = 12;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 64;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 27000;
			break;
		case 68:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 700;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 440;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 19: /* 1280x720p @ 50Hz 16:9 */
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 700;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 440;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 20: /* 1920x1080i @ 50Hz 16:9 */
			pstDtd->h_active = 1920;
			pstDtd->v_active = 540;
			pstDtd->h_blanking = 720;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 528;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 21: /* 720(1440)x576i @ 50Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1440;
			pstDtd->v_active = 288;
			pstDtd->h_blanking = 288;
			pstDtd->v_blanking = 24;
			pstDtd->h_sync_offset = 24;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 126;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 27000;
			break;
		case 22: /* 720(1440)x576i @ 50Hz 16:9 */
			pstDtd->h_active = 1440;
			pstDtd->v_active = 288;
			pstDtd->h_blanking = 288;
			pstDtd->v_blanking = 24;
			pstDtd->h_sync_offset = 24;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 126;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 27000;
			break;
		case 23: /* 720(1440)x288p @ 50Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1440;
			pstDtd->v_active = 288;
			pstDtd->h_blanking = 288;
			pstDtd->v_blanking = (uiRefreshRate == 50080) ? 24 : ((uiRefreshRate == 49920) ? 25 : 26);
			pstDtd->h_sync_offset = 24;
			pstDtd->v_sync_offset = (uiRefreshRate == 50080) ? 2 : ((uiRefreshRate == 49920) ? 3 : 4);
			pstDtd->h_sync_pulse_width = 126;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 27000;
			break;
		case 24: /* 720(1440)x288p @ 50Hz 16:9 */
			pstDtd->h_active = 1440;
			pstDtd->v_active = 288;
			pstDtd->h_blanking = 288;
			pstDtd->v_blanking = (uiRefreshRate == 50080) ? 24 : ((uiRefreshRate == 49920) ? 25 : 26);
			pstDtd->h_sync_offset = 24;
			pstDtd->v_sync_offset = (uiRefreshRate == 50080) ? 2 : ((uiRefreshRate == 49920) ? 3 : 4);
			pstDtd->h_sync_pulse_width = 126;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 27000;
			break;
		case 25: /* 2880x576i @ 50Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 2880;
			pstDtd->v_active = 288;
			pstDtd->h_blanking = 576;
			pstDtd->v_blanking = 24;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 252;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 26: /* 2880x576i @ 50Hz 16:9 */
			pstDtd->h_active = 2880;
			pstDtd->v_active = 288;
			pstDtd->h_blanking = 576;
			pstDtd->v_blanking = 24;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 252;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 27: /* 2880x288p @ 50Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 2880;
			pstDtd->v_active = 288;
			pstDtd->h_blanking = 576;
			pstDtd->v_blanking = (uiRefreshRate == 50080) ? 24 : ((uiRefreshRate == 49920) ? 25 : 26);
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = (uiRefreshRate == 50080) ? 2 : ((uiRefreshRate == 49920) ? 3 : 4);
			pstDtd->h_sync_pulse_width = 252;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 28: /* 2880x288p @ 50Hz 16:9 */
			pstDtd->h_active = 2880;
			pstDtd->v_active = 288;
			pstDtd->h_blanking = 576;
			pstDtd->v_blanking = (uiRefreshRate == 50080) ? 24 : ((uiRefreshRate == 49920) ? 25 : 26);
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = (uiRefreshRate == 50080) ? 2 : ((uiRefreshRate == 49920) ? 3 : 4);
			pstDtd->h_sync_pulse_width = 252;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 29: /* 1440x576p @ 50Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1440;
			pstDtd->v_active = 576;
			pstDtd->h_blanking = 288;
			pstDtd->v_blanking = 49;
			pstDtd->h_sync_offset = 24;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 128;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 30: /* 1440x576p @ 50Hz 16:9 */
			pstDtd->h_active = 1440;
			pstDtd->v_active = 576;
			pstDtd->h_blanking = 288;
			pstDtd->v_blanking = 49;
			pstDtd->h_sync_offset = 24;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 128;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 75:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 720;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 528;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 148500;
			break;
		case 31: /* 1920x1080p @ 50Hz 16:9 */
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 720;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 528;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 148500;
			break;
		case 72:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 830;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 638;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 32: /* 1920x1080p @ 23.976/24Hz 16:9 */
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 830;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 638;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 73:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 720;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 528;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 33: /* 1920x1080p @ 25Hz 16:9 */
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 720;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 528;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 74:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 280;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 34: /* 1920x1080p @ 29.97/30Hz 16:9 */
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 280;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 35: /* 2880x480p @ 60Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 2880;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 552;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 64;
			pstDtd->v_sync_offset = 9;
			pstDtd->h_sync_pulse_width = 248;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 36: /* 2880x480p @ 60Hz 16:9 */
			pstDtd->h_active = 2880;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 552;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 64;
			pstDtd->v_sync_offset = 9;
			pstDtd->h_sync_pulse_width = 248;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 37: /* 2880x576p @ 50Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 2880;
			pstDtd->v_active = 576;
			pstDtd->h_blanking = 576;
			pstDtd->v_blanking = 49;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 256;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 38: /* 2880x576p @ 50Hz 16:9 */
			pstDtd->h_active = 2880;
			pstDtd->v_active = 576;
			pstDtd->h_blanking = 576;
			pstDtd->v_blanking = 49;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 256;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 39: /* 1920x1080i (1250 total) @ 50Hz 16:9 */
			pstDtd->h_active = 1920;
			pstDtd->v_active = 540;
			pstDtd->h_blanking = 384;
			pstDtd->v_blanking = 85;
			pstDtd->h_sync_offset = 32;
			pstDtd->v_sync_offset = 23;
			pstDtd->h_sync_pulse_width = 168;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 72000;
			break;
		case 40: /* 1920x1080i @ 100Hz 16:9 */
			pstDtd->h_active = 1920;
			pstDtd->v_active = 540;
			pstDtd->h_blanking = 720;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 528;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 148500;
			break;
		case 70:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 700;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 440;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 148500;
			break;
		case 41: /* 1280x720p @ 100Hz 16:9 */
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 700;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 440;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 148500;
			break;
		case 42: /* 720x576p @ 100Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 720;
			pstDtd->v_active = 576;
			pstDtd->h_blanking = 144;
			pstDtd->v_blanking = 49;
			pstDtd->h_sync_offset = 12;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 64;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 43: /* 720x576p @ 100Hz 16:9 */
			pstDtd->h_active = 720;
			pstDtd->v_active = 576;
			pstDtd->h_blanking = 144;
			pstDtd->v_blanking = 49;
			pstDtd->h_sync_offset = 12;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 64;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 44: /* 720(1440)x576i @ 100Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1440;
			pstDtd->v_active = 288;
			pstDtd->h_blanking = 288;
			pstDtd->v_blanking = 24;
			pstDtd->h_sync_offset = 24;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 126;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 45: /* 720(1440)x576i @ 100Hz 16:9 */
			pstDtd->h_active = 1440;
			pstDtd->v_active = 288;
			pstDtd->h_blanking = 288;
			pstDtd->v_blanking = 24;
			pstDtd->h_sync_offset = 24;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 126;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 46: /* 1920x1080i @ 119.88/120Hz 16:9 */
			pstDtd->h_active = 1920;
			pstDtd->v_active = 540;
			pstDtd->h_blanking = 288;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 148500;
			break;
		case 71:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 370;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 110;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 148500;
			break;
		case 47: /* 1280x720p @ 119.88/120Hz 16:9 */
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 370;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 110;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 148500;
			break;
		case 48: /* 720x480p @ 119.88/120Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 720;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 138;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 16;
			pstDtd->v_sync_offset = 9;
			pstDtd->h_sync_pulse_width = 62;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 49: /* 720x480p @ 119.88/120Hz 16:9 */
			pstDtd->h_active = 720;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 138;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 16;
			pstDtd->v_sync_offset = 9;
			pstDtd->h_sync_pulse_width = 62;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 50: /* 720(1440)x480i @ 119.88/120Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1440;
			pstDtd->v_active = 240;
			pstDtd->h_blanking = 276;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 38;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 124;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 51: /* 720(1440)x480i @ 119.88/120Hz 16:9 */
			pstDtd->h_active = 1440;
			pstDtd->v_active = 240;
			pstDtd->h_blanking = 276;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 38;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 124;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 54000;
			break;
		case 52: /* 720X576p @ 200Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 720;
			pstDtd->v_active = 576;
			pstDtd->h_blanking = 144;
			pstDtd->v_blanking = 49;
			pstDtd->h_sync_offset = 12;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 64;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 53: /* 720X576p @ 200Hz 16:9 */
			pstDtd->h_active = 720;
			pstDtd->v_active = 576;
			pstDtd->h_blanking = 144;
			pstDtd->v_blanking = 49;
			pstDtd->h_sync_offset = 12;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 64;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 54: /* 720(1440)x576i @ 200Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1440;
			pstDtd->v_active = 288;
			pstDtd->h_blanking = 288;
			pstDtd->v_blanking = 24;
			pstDtd->h_sync_offset = 24;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 126;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 55: /* 720(1440)x576i @ 200Hz 16:9 */
			pstDtd->h_active = 1440;
			pstDtd->v_active = 288;
			pstDtd->h_blanking = 288;
			pstDtd->v_blanking = 24;
			pstDtd->h_sync_offset = 24;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 126;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 56: /* 720x480p @ 239.76/240Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 720;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 138;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 16;
			pstDtd->v_sync_offset = 9;
			pstDtd->h_sync_pulse_width = 62;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 57: /* 720x480p @ 239.76/240Hz 16:9 */
			pstDtd->h_active = 720;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 138;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 16;
			pstDtd->v_sync_offset = 9;
			pstDtd->h_sync_pulse_width = 62;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 58: /* 720(1440)x480i @ 239.76/240Hz 4:3 */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1440;
			pstDtd->v_active = 240;
			pstDtd->h_blanking = 276;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 38;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 124;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 59: /* 720(1440)x480i @ 239.76/240Hz 16:9 */
			pstDtd->h_active = 1440;
			pstDtd->v_active = 240;
			pstDtd->h_blanking = 276;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 38;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 124;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 1;
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 65:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 2020;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 1760;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 594000;
			break;
		case 60: /* 1280x720p @ 23.97/24Hz 16:9 */
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 2020;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 1760;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 594000;
			break;
		case 66:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 2680;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 2420;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 61: /* 1280x720p @ 25Hz 16:9 */
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 2680;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 2420;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 67:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 2020;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 1760;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 62: /* 1280x720p @ 29.97/30Hz  16:9 */
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 2020;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 1760;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 74250;
			break;
		case 78:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 280;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case 63: /* 1920x1080p @ 119.88/120Hz 16:9 */
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 280;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case 77:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 720;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 528;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case 64: /* 1920x1080p @ 100Hz 16:9 */
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 720;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 528;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case 79:
			pstDtd->h_active = 1680;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 1620;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 1360;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 594000;
			break;
		case 80:
			pstDtd->h_active = 1680;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 1488;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 1228;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 594000;
			break;
		case 81:
			pstDtd->h_active = 1680;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 960;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 700;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 594000;
			break;
		case 82:
			pstDtd->h_active = 1680;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 520;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 260;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 82500;
			break;
		case 83:
			pstDtd->h_active = 1680;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 520;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 260;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 99000;
			break;
		case 84:
			pstDtd->h_active = 1680;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 320;
			pstDtd->v_blanking = 105;
			pstDtd->h_sync_offset = 60;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 165000;
			break;
		case 85:
			pstDtd->h_active = 1680;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 320;
			pstDtd->v_blanking = 105;
			pstDtd->h_sync_offset = 60;
			pstDtd->v_sync_offset = 5;
			pstDtd->h_sync_pulse_width = 40;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 198000;
			break;
		case 86:
			pstDtd->h_active = 2560;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 1190;
			pstDtd->v_blanking = 20;
			pstDtd->h_sync_offset = 998;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 99000;
			break;
		case 87:
			pstDtd->h_active = 2560;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 640;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 448;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 90000;
			break;
		case 88:
			pstDtd->h_active = 2560;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 960;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 768;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 118800;
			break;
		case 89:
			pstDtd->h_active = 2560;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 740;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 548;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 185625;
			break;
		case 90:
			pstDtd->h_active = 2560;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 440;
			pstDtd->v_blanking = 20;
			pstDtd->h_sync_offset = 248;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 198000;
			break;
		case 91:
			pstDtd->h_active = 2560;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 410;
			pstDtd->v_blanking = 170;
			pstDtd->h_sync_offset = 218;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 371250;
			break;
		case 92:
			pstDtd->h_active = 2560;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 740;
			pstDtd->v_blanking = 170;
			pstDtd->h_sync_offset = 548;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 495000;
			break;
		case 101:
			pstDtd->h_active = 4096;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 1184;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 968;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 59400;
			break;
		case 100:
			pstDtd->h_active = 4096;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 304;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case 99:
			pstDtd->h_active = 4096;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 1184;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 968;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case 102:
			pstDtd->h_active = 4096;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 304;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 594000;
			break;
		case 103:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 3840;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 1660;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 1276;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case 93:		/* 4k x 2k, 30Hz */
			pstDtd->h_active = 3840;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 1660;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 1276;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case 104:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 3840;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 1440;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 1056;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case 94:
			pstDtd->h_active = 3840;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 1440;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 1056;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case 105:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 3840;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 560;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 176;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case 95:
			pstDtd->h_active = 3840;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 560;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 176;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		case 106:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 3840;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 1440;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 1056;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 594000;
			break;
		case 96:
			pstDtd->h_active = 3840;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 1440;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 1056;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 594000;
			break;
		case 107:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 3840;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 560;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 176;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 594000;
			break;
		case 97:
			pstDtd->h_active = 3840;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 560;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 176;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 594000;
			break;
		case 98:
			pstDtd->h_active = 4096;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 1404;
			pstDtd->v_blanking = 90;
			pstDtd->h_sync_offset = 1020;
			pstDtd->v_sync_offset = 8;
			pstDtd->h_sync_pulse_width = 88;
			pstDtd->v_sync_pulse_width = 10;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;
			pstDtd->uiPixel_Clock = 297000;
			break;
		default:
			dptx_err("Invalid video code = %d ", uiVideo_Code);
			return DPTX_RETURN_EINVAL;
		}
	} else if (ucVideoFormat == VESA_CVT) {
		switch (uiVideo_Code) {
		case 1:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 640;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 20;
			pstDtd->h_sync_offset = 8;
			pstDtd->v_sync_offset = 1;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 8;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 23750;
			break;
		case 2:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 800;
			pstDtd->v_active = 600;
			pstDtd->h_blanking = 224;
			pstDtd->v_blanking = 24;
			pstDtd->h_sync_offset = 31;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 81;
			pstDtd->v_sync_pulse_width = 4;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 38250;
			break;
		case 3:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1024;
			pstDtd->v_active = 768;
			pstDtd->h_blanking = 304;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 104;
			pstDtd->v_sync_pulse_width = 4;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 63500;
			break;
		case 4:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 960;
			pstDtd->h_blanking = 416;
			pstDtd->v_blanking = 36;
			pstDtd->h_sync_offset = 80;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 128;
			pstDtd->v_sync_pulse_width = 4;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 101250;
			break;
		case 5:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1400;
			pstDtd->v_active = 1050;
			pstDtd->h_blanking = 464;
			pstDtd->v_blanking = 39;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 144;
			pstDtd->v_sync_pulse_width = 4;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 121750;
			break;
		case 6:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1600;
			pstDtd->v_active = 1200;
			pstDtd->h_blanking = 560;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 112;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 68;
			pstDtd->v_sync_pulse_width = 4;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 161000;
			break;
		case 12:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 1024;
			pstDtd->h_blanking = 432;
			pstDtd->v_blanking = 39;
			pstDtd->h_sync_offset = 80;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 136;
			pstDtd->v_sync_pulse_width = 7;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 109000;
			break;
		case 13:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 768;
			pstDtd->h_blanking = 384;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 64;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 128;
			pstDtd->v_sync_pulse_width = 7;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 79500;
			break;
		case 16:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 720;
			pstDtd->h_blanking = 384;
			pstDtd->v_blanking = 28;
			pstDtd->h_sync_offset = 64;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 128;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 74500;
			break;
		case 17:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1360;
			pstDtd->v_active = 768;
			pstDtd->h_blanking = 416;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 72;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 136;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 84750;
			break;
		case 20:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 656;
			pstDtd->v_blanking = 40;
			pstDtd->h_sync_offset = 128;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 200;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 173000;
			break;
		case 22:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 2560;
			pstDtd->v_active = 1440;
			pstDtd->h_blanking = 928;
			pstDtd->v_blanking = 53;
			pstDtd->h_sync_offset = 192;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 272;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 312250;
			break;
		case 28:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 800;
			pstDtd->h_blanking = 400;
			pstDtd->v_blanking = 31;
			pstDtd->h_sync_offset = 72;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 128;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 83500;
			break;
		case 34:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1200;
			pstDtd->h_blanking = 672;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 136;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 200;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 193250;
			break;
		case 38:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 3840;
			pstDtd->v_active = 2400;
			pstDtd->h_blanking = 80;
			pstDtd->v_blanking = 69;
			pstDtd->h_sync_offset = 320;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 424;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 580128;
			break;
		case 40:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1600;
			pstDtd->v_active = 1200;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 35;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 4;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 124076;
			break;
		case 41:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 2048;
			pstDtd->v_active = 1536;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 44;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 4;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 208000;
			break;
		default:
			dptx_err("Invalid video code = %d\n", uiVideo_Code);
			return DPTX_RETURN_EINVAL;
		}
	}  else if (ucVideoFormat == VESA_DMT) {
		switch (uiVideo_Code) {
		case 1: // HISilicon timing
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 3600;
			pstDtd->v_active = 1800;
			pstDtd->h_blanking = 120;
			pstDtd->v_blanking = 128;
			pstDtd->h_sync_offset = 20;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 20;
			pstDtd->v_sync_pulse_width = 2;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 645500;
			break;
		case 2:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 3840;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 62;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 533000;
			break;
		case 4:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 640;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 144;
			pstDtd->v_blanking = 29;
			pstDtd->h_sync_offset = 8;
			pstDtd->v_sync_offset = 2;
			pstDtd->h_sync_pulse_width = 96;
			pstDtd->v_sync_pulse_width = 2;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 25175;
			break;
		case 13:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 800;
			pstDtd->v_active = 600;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 36;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 4;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 73250;
			break;
		case 14: /* 848x480p@60Hz */
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 848;
			pstDtd->v_active = 480;
			pstDtd->h_blanking = 240;
			pstDtd->v_blanking = 37;
			pstDtd->h_sync_offset = 16;
			pstDtd->v_sync_offset = 6;
			pstDtd->h_sync_pulse_width = 112;
			pstDtd->v_sync_pulse_width = 8;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI)  */;
			pstDtd->uiPixel_Clock = 33750;
			break;
		case 22:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 768;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 22;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 7;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 68250;
			break;
		case 35:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 1024;
			pstDtd->h_blanking = 408;
			pstDtd->v_blanking = 42;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 1;
			pstDtd->h_sync_pulse_width = 112;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 39:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1360;
			pstDtd->v_active = 768;
			pstDtd->h_blanking = 432;
			pstDtd->v_blanking = 27;
			pstDtd->h_sync_offset = 64;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 112;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 85500;
			break;
		case 40:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1360;
			pstDtd->v_active = 768;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 148250;
			break;
		case 81:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1366;
			pstDtd->v_active = 768;
			pstDtd->h_blanking = 426;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 70;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 142;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 85500;
			break;
		case 86:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1366;
			pstDtd->v_active = 768;
			pstDtd->h_blanking = 134;
			pstDtd->v_blanking = 32;
			pstDtd->h_sync_offset = 14;
			pstDtd->v_sync_offset = 1;
			pstDtd->h_sync_pulse_width = 56;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 72000;
			break;
		case 87:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 4096;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 80;
			pstDtd->v_blanking = 62;
			pstDtd->h_sync_offset = 8;
			pstDtd->v_sync_offset = 48;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 8;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 556744;
			break;
		case 88:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 4096;
			pstDtd->v_active = 2160;
			pstDtd->h_blanking = 80;
			pstDtd->v_blanking = 62;
			pstDtd->h_sync_offset = 8;
			pstDtd->v_sync_offset = 48;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 8;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 556188;
			break;
		case 41:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1400;
			pstDtd->v_active = 1050;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 4;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 101000;
			break;
		case 42:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1400;
			pstDtd->v_active = 1050;
			pstDtd->h_blanking = 464;
			pstDtd->v_blanking = 39;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 144;
			pstDtd->v_sync_pulse_width = 4;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 121750;
			break;
		case 46:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1440;
			pstDtd->v_active = 900;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 26;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 88750;
			break;
		case 47:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1440;
			pstDtd->v_active = 900;
			pstDtd->h_blanking = 464;
			pstDtd->v_blanking = 34;
			pstDtd->h_sync_offset = 80;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 152;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 106500;
			break;
		case 51:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1600;
			pstDtd->v_active = 1200;
			pstDtd->h_blanking = 560;
			pstDtd->v_blanking = 50;
			pstDtd->h_sync_offset = 64;
			pstDtd->v_sync_offset = 1;
			pstDtd->h_sync_pulse_width = 192;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 162000;
			break;
		case 57:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1680;
			pstDtd->v_active = 1050;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 119000;
			break;
		case 58:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1680;
			pstDtd->v_active = 1050;
			pstDtd->h_blanking = 560;
			pstDtd->v_blanking = 39;
			pstDtd->h_sync_offset = 104;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 176;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 146250;
			break;
		case 68:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1200;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 35;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 154000;
			break;
		case 69:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1200;
			pstDtd->h_blanking = 672;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 136;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 200;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 193250;
			break;
		case 82:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1080;
			pstDtd->h_blanking = 280;
			pstDtd->v_blanking = 45;
			pstDtd->h_sync_offset = 88;
			pstDtd->v_sync_offset = 4;
			pstDtd->h_sync_pulse_width = 44;
			pstDtd->v_sync_pulse_width = 5;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 148500;
			break;
		case 83:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1600;
			pstDtd->v_active = 900;
			pstDtd->h_blanking = 200;
			pstDtd->v_blanking = 100;
			pstDtd->h_sync_offset = 24;
			pstDtd->v_sync_offset = 1;
			pstDtd->h_sync_pulse_width = 80;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 9:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 800;
			pstDtd->v_active = 600;
			pstDtd->h_blanking = 256;
			pstDtd->v_blanking = 28;
			pstDtd->h_sync_offset = 40;
			pstDtd->v_sync_offset = 1;
			pstDtd->h_sync_pulse_width = 128;
			pstDtd->v_sync_pulse_width = 4;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 40000;
			break;
		case 16:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1024;
			pstDtd->v_active = 768;
			pstDtd->h_blanking = 320;
			pstDtd->v_blanking = 38;
			pstDtd->h_sync_offset = 24;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 136;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 65000;
			break;
		case 23:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 768;
			pstDtd->h_blanking = 384;
			pstDtd->v_blanking = 30;
			pstDtd->h_sync_offset = 64;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 128;
			pstDtd->v_sync_pulse_width = 7;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 79500;
			break;
		case 62:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active =  1792;
			pstDtd->v_active = 1344;
			pstDtd->h_blanking = 656;
			pstDtd->v_blanking =  50;
			pstDtd->h_sync_offset = 128;
			pstDtd->v_sync_offset = 1;
			pstDtd->h_sync_pulse_width = 200;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0; /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 204750;
			break;
		case 32:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 960;
			pstDtd->h_blanking = 520;
			pstDtd->v_blanking = 40;
			pstDtd->h_sync_offset = 96;
			pstDtd->v_sync_offset = 1;
			pstDtd->h_sync_pulse_width = 112;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;  /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 108000;
			break;
		case 73:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1920;
			pstDtd->v_active = 1440;
			pstDtd->h_blanking = 680;
			pstDtd->v_blanking = 60;
			pstDtd->h_sync_offset = 128;
			pstDtd->v_sync_offset = 1;
			pstDtd->h_sync_pulse_width = 208;
			pstDtd->v_sync_pulse_width = 3;
			pstDtd->h_sync_polarity = 0;
			pstDtd->v_sync_polarity = 1;
			pstDtd->interlaced = 0;  /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 234000;
			break;
		case 27:
			pstDtd->h_image_size = 4;
			pstDtd->v_image_size = 3;
			pstDtd->h_active = 1280;
			pstDtd->v_active = 800;
			pstDtd->h_blanking = 160;
			pstDtd->v_blanking = 23;
			pstDtd->h_sync_offset = 48;
			pstDtd->v_sync_offset = 3;
			pstDtd->h_sync_pulse_width = 32;
			pstDtd->v_sync_pulse_width = 6;
			pstDtd->h_sync_polarity = 1;
			pstDtd->v_sync_polarity = 0;
			pstDtd->interlaced = 0;  /* (progressive_nI) */
			pstDtd->uiPixel_Clock = 71000;
			break;
		default:
			dptx_err("Invalid video code = %d", uiVideo_Code);
			return DPTX_RETURN_EINVAL;
		}
	} else {
		dptx_err("Invalid video format = %d\n", ucVideoFormat);
		return DPTX_RETURN_EINVAL;
	}

	return DPTX_RETURN_NO_ERROR;
}

#if 0
void Dptx_Vidin_Disable_Audio_SDP(struct Dptx_Params *pstDptx)
{
	uint32_t uiRegMap_AudEnableSDP;
	uint8_t	ucElements = 0;

	uiRegMap_AudEnableSDP = Dptx_Reg_Readl(pstDptx, DPTX_SDP_VERTICAL_CTRL);
	uiRegMap_AudEnableSDP &= ~DPTX_EN_AUDIO_STREAM_SDP;

	for (ucElements = 0; ucElements < pstDptx->ucNumOfStreams; ucElements++)
		Dptx_Reg_Writel(pstDptx, DPTX_SDP_VERTICAL_CTRL_N(ucElements), uiRegMap_AudEnableSDP);

	uiRegMap_AudEnableSDP = Dptx_Reg_Readl(pstDptx, DPTX_SDP_HORIZONTAL_CTRL);
	uiRegMap_AudEnableSDP &= ~DPTX_EN_AUDIO_STREAM_SDP;

	for (ucElements = 0; ucElements < pstDptx->ucNumOfStreams; ucElements++)
		Dptx_Reg_Writel(pstDptx, DPTX_SDP_HORIZONTAL_CTRL_N(ucElements), uiRegMap_AudEnableSDP);
}

void Dptx_Vidin_Disable_Audio_Timestamp(struct Dptx_Params *dptx)
{
	uint32_t uiRegMap_AudEnableTimestamp;
	uint8_t ucElements = 0;

	uiRegMap_AudEnableTimestamp = Dptx_Reg_Readl(dptx, DPTX_SDP_VERTICAL_CTRL);
	uiRegMap_AudEnableTimestamp &= ~DPTX_EN_AUDIO_TIMESTAMP_SDP;

	for (ucElements = 0; ucElements < dptx->ucNumOfStreams; ucElements++)
		Dptx_Reg_Writel(dptx, DPTX_SDP_VERTICAL_CTRL_N(ucElements), uiRegMap_AudEnableTimestamp);

	uiRegMap_AudEnableTimestamp = Dptx_Reg_Readl(dptx, DPTX_SDP_HORIZONTAL_CTRL);
	uiRegMap_AudEnableTimestamp &= ~DPTX_EN_AUDIO_TIMESTAMP_SDP;

	for (ucElements = 0; ucElements < dptx->ucNumOfStreams; ucElements++)
		Dptx_Reg_Writel(dptx, DPTX_SDP_HORIZONTAL_CTRL_N(ucElements), uiRegMap_AudEnableTimestamp);
}
#endif

