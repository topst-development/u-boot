// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/

#include <config.h>
#include <common.h>
#include <linux/compat.h>

#include <mach/chipinfo.h>
#include <dm/uclass.h>
#include <telechips/dpv14_ctrl.h>
#include <telechips/dpv14_max968xx.h>
#include <dt-bindings/display/telechips-dispdef.h>
#include <mach/reboot.h>
#include "dptx_api.h"


#define DPV14_CTRL_DRV_MAJOR_VER		1
#define DPV14_CTRL_DRV_MINOR_VER		0
#define DPV14_CTRL_DRV_PATCH_VER		0


static struct dpv14_drv_params stdpv14_params = {0, };

static void dpv14_reinit_dp_drv(void)
{
	uint8_t ucDpIdx;
	const struct dpv14_drv_params *pstdpv14_params;
	struct dptx_api_init_params stdptx_api_init_params;

	pstdpv14_params = &stdpv14_params;

	debug_pr(LOGL_WARNING, "[%s:%d]Re-initializing DP Link..\n", __func__, __LINE__);

	(void)Dpv14_Tx_API_Stop();
	(void)Dpv14_Tx_API_Deinit();

	stdptx_api_init_params.bsideband_msg = (pstdpv14_params->bpanel_mode) ? (bool)false : (bool)true;
	stdptx_api_init_params.bsdm_bypass = pstdpv14_params->bsdm_bypass;
	stdptx_api_init_params.btrvc_bypass = pstdpv14_params->btrvc_bypass;
	stdptx_api_init_params.bphy_lane_swap = pstdpv14_params->bphy_lane_swap;
	stdptx_api_init_params.ucnum_of_dps = pstdpv14_params->ucnum_of_dps;
	stdptx_api_init_params.eMaxRate = (enum DPTX_LINK_RATE)pstdpv14_params->ucmax_rate;
	stdptx_api_init_params.eMaxLanes = (enum DPTX_LINK_LANE)pstdpv14_params->ucmax_lane;
	stdptx_api_init_params.eVidEncodingType = (enum DPTX_VIDEO_ENCODING_TYPE)pstdpv14_params->ucpixel_encoding;

	for (ucDpIdx = 0; ucDpIdx < (unsigned)DPTX_INPUT_STREAM_MAX; ucDpIdx++) {
		stdptx_api_init_params.ucmux_id[ucDpIdx] = (uint8_t)(pstdpv14_params->uidd_mux_id[ucDpIdx] & 0xFFU);
		stdptx_api_init_params.ucvcp_id[ucDpIdx] = pstdpv14_params->ucvcp_id[ucDpIdx];
		stdptx_api_init_params.uiVIC[ucDpIdx] = pstdpv14_params->uivic[ucDpIdx];
	}
	stdptx_api_init_params.ucphy_model = pstdpv14_params->ucphy_model;
	if (stdptx_api_init_params.ucphy_model == DPTX_PHY_DEVICE_SNPS) {
		if (pstdpv14_params->ucchip_rev == (uint8_t)TCC80XX_REV_ES) {
			stdptx_api_init_params.eMaxRate = (pstdpv14_params->ucmax_rate > (uint8_t)DPTX_LINK_RATE_HBR2) ? (enum DPTX_LINK_RATE)DPTX_LINK_RATE_HBR2 : (enum DPTX_LINK_RATE)pstdpv14_params->ucmax_rate;

			for (ucDpIdx = 0; ucDpIdx < (unsigned)DPTX_INPUT_STREAM_MAX; ucDpIdx++) {
				stdptx_api_init_params.ucmux_id[ucDpIdx] = ucDpIdx;
			}
		}
	}

	(void)Dpv14_Tx_API_Init(&stdptx_api_init_params);
}

static int32_t dpv14_start_dp_drv(void)
{
	int32_t ret = 0;
	struct dpv14_drv_params *pstdpv14_params;

	pstdpv14_params = &stdpv14_params;

	debug_pr(LOGL_DEBUG, "[%s:%d]P Clk : %u %u %u %u\n", __func__, __LINE__,
				pstdpv14_params->uipclk[0],
				pstdpv14_params->uipclk[1],
				pstdpv14_params->uipclk[2],
				pstdpv14_params->uipclk[3]);

	ret = Dpv14_Tx_API_Start(pstdpv14_params->ucnum_of_dps, pstdpv14_params->uipclk);

	return ret;
}

static int32_t dpv14_init_dp_drv(const struct dpv14_drv_params *pstdpv14_params)
{
	struct dptx_api_init_params stdptx_api_init_params;
	uint8_t ucDpIdx;
	int32_t ret = 0;

	stdptx_api_init_params.bsideband_msg = (pstdpv14_params->bpanel_mode) ? (bool)false : (bool)true;
	stdptx_api_init_params.bsdm_bypass = pstdpv14_params->bsdm_bypass;
	stdptx_api_init_params.btrvc_bypass = pstdpv14_params->btrvc_bypass;
	stdptx_api_init_params.bphy_lane_swap = pstdpv14_params->bphy_lane_swap;
	stdptx_api_init_params.ucnum_of_dps = pstdpv14_params->ucnum_of_dps;
	stdptx_api_init_params.eMaxRate = (enum DPTX_LINK_RATE)pstdpv14_params->ucmax_rate;
	stdptx_api_init_params.eMaxLanes = (enum DPTX_LINK_LANE)pstdpv14_params->ucmax_lane;
	stdptx_api_init_params.eVidEncodingType = (enum DPTX_VIDEO_ENCODING_TYPE)pstdpv14_params->ucpixel_encoding;

	for (ucDpIdx = 0; ucDpIdx < (unsigned)DPTX_INPUT_STREAM_MAX; ucDpIdx++) {
		stdptx_api_init_params.ucmux_id[ucDpIdx] = (uint8_t)(pstdpv14_params->uidd_mux_id[ucDpIdx] & 0xFFU);
		stdptx_api_init_params.ucvcp_id[ucDpIdx] = pstdpv14_params->ucvcp_id[ucDpIdx];
		stdptx_api_init_params.uiVIC[ucDpIdx] = pstdpv14_params->uivic[ucDpIdx];
	}

	stdptx_api_init_params.ucphy_model = pstdpv14_params->ucphy_model;
	if (stdptx_api_init_params.ucphy_model == DPTX_PHY_DEVICE_SNPS) {
		if (pstdpv14_params->ucchip_rev == (uint8_t)TCC80XX_REV_ES) {
			stdptx_api_init_params.eMaxRate =
				(pstdpv14_params->ucmax_rate > (uint8_t)DPTX_LINK_RATE_HBR2) ?
				(enum DPTX_LINK_RATE)DPTX_LINK_RATE_HBR2 :
				(enum DPTX_LINK_RATE)pstdpv14_params->ucmax_rate;

			for (ucDpIdx = 0; ucDpIdx < (unsigned)DPTX_INPUT_STREAM_MAX; ucDpIdx++) {
				stdptx_api_init_params.ucmux_id[ucDpIdx] = ucDpIdx;
			}
		}
	}
	ret = Dpv14_Tx_API_Init(&stdptx_api_init_params);

	return ret;
}

static int32_t dpv14_get_disp_timing_from_dtd(uint32_t uivic,
					      const struct DPTX_Dtd_Params_t *pstdtd_param,
					      struct dpv14_display_timing *pstdisp_timing)
{
	uint32_t uih_blanking, uiv_blanking;
	uint32_t uih_sync_offset, uiv_sync_offset;
	uint32_t uih_sync_width, uiv_sync_width;
	int32_t ret = 0;

	uih_blanking = (uint32_t)pstdtd_param->h_blanking;
	uiv_blanking = (uint32_t)pstdtd_param->v_blanking;

	uih_sync_offset = (uint32_t)pstdtd_param->h_sync_offset;
	uiv_sync_offset = (uint32_t)pstdtd_param->v_sync_offset;

	uih_sync_width = (uint32_t)pstdtd_param->h_sync_pulse_width;
	uiv_sync_width = (uint32_t)pstdtd_param->v_sync_pulse_width;

	if (pstdtd_param->uiPixel_Clock <= (UINT_MAX / 1000U)) {
		pstdisp_timing->uipixelclock = (pstdtd_param->uiPixel_Clock * 1000U);
	}

	pstdisp_timing->uihactive = (uint32_t)pstdtd_param->h_active;
	pstdisp_timing->uihfront_porch = uih_sync_offset;
	pstdisp_timing->uihsync_len = uih_sync_width;
	if (uih_blanking >= (uih_sync_offset + uih_sync_width)) {
		pstdisp_timing->uihback_porch = (uih_blanking - (uih_sync_offset + uih_sync_width));
	}

	pstdisp_timing->uivactive = (uint32_t)pstdtd_param->v_active;
	if (pstdtd_param->interlaced != 0U) {
		/*For KCS*/
		pstdisp_timing->uivactive <<= 1U;
	}

	pstdisp_timing->uivfront_porch = uiv_sync_offset;
	if (pstdtd_param->interlaced != 0U) {
		pstdisp_timing->uivfront_porch = (pstdisp_timing->uivfront_porch << 1U);

		if ((uivic == 39U) && (pstdisp_timing->uivfront_porch > 2U)) {
			/*For KCS*/
			pstdisp_timing->uivfront_porch -= 2U;
		}
	}

	pstdisp_timing->uivsync_len = uiv_sync_width;
	if (pstdtd_param->interlaced != 0U) {
		/*For KCS*/
		pstdisp_timing->uivsync_len = (pstdisp_timing->uivsync_len << 1U);
	}

	if (uiv_blanking >= (uiv_sync_offset + uiv_sync_width)) {
		pstdisp_timing->uivback_porch = (uiv_blanking - (uiv_sync_offset + uiv_sync_width));
	}

	if (pstdtd_param->interlaced != 0U) {
		/*For KCS*/
		pstdisp_timing->uivback_porch = (pstdisp_timing->uivback_porch << 1U);
	}

	pstdisp_timing->uiinterlaced = (uint32_t)pstdtd_param->interlaced;
	pstdisp_timing->uipixel_repetition = (uint32_t)pstdtd_param->pixel_repetition_input;
	pstdisp_timing->flags = (pstdtd_param->h_sync_polarity == 0U) ? (unsigned)DISPLAY_FLAGS_HSYNC_LOW : (unsigned)DISPLAY_FLAGS_HSYNC_HIGH;
	pstdisp_timing->flags |= (pstdtd_param->v_sync_polarity == 0U) ? (unsigned)DISPLAY_FLAGS_VSYNC_LOW : (unsigned)DISPLAY_FLAGS_VSYNC_HIGH;

	debug_pr(LOGL_DEBUG, "\n[%s:%d]VIC(%d) : \n", __func__, __LINE__, uivic);
	debug_pr(LOGL_DEBUG, " Pixel clk = %u\n", pstdisp_timing->uipixelclock);
	debug_pr(LOGL_DEBUG, " %s\n", (pstdisp_timing->uiinterlaced != 0U) ? "Interlace" : "Progressive" );
	debug_pr(LOGL_DEBUG, " Repetition = %u\n", pstdisp_timing->uipixel_repetition);
	debug_pr(LOGL_DEBUG, " flags = 0x%x\n", pstdisp_timing->flags);
	debug_pr(LOGL_DEBUG, " H active(%u) x V active(%u)\n", pstdisp_timing->uihactive, pstdisp_timing->uivactive);
	debug_pr(LOGL_DEBUG, " H front porch(%u), V front porch(%u)\n", pstdisp_timing->uihfront_porch, pstdisp_timing->uivfront_porch);
	debug_pr(LOGL_DEBUG, " H back porch(%u), V back porch(%u)\n", pstdisp_timing->uihback_porch, pstdisp_timing->uivback_porch);
	debug_pr(LOGL_DEBUG, " H sync len(%u), V sync len(%u)\n", pstdisp_timing->uihsync_len, pstdisp_timing->uivsync_len);

	return ret;
}

int32_t dpv14_get_reset_type(enum DPV14_RESET_TYPE *pereset_type)
{
	bool bcore_reset;
	int32_t ret = 0;

	*pereset_type = RESET_TYPE_COLD;

	bcore_reset = core_reset_occurred();
	if (bcore_reset) {
		/*For KCS*/
		*pereset_type = RESET_TYPE_CORE;
	}

	return ret;
}

int32_t dpv14_get_dtd(uint8_t ucdp_index, uint32_t uivic, enum DPV14_DTD_TYPE edtd_type,
		      struct dpv14_display_timing *pstdisplay_timing)
{
	int32_t ret = 0;
	struct DPTX_Dtd_Params_t stdtd_param;

	(void)memset(&stdtd_param, 0, sizeof(struct DPTX_Dtd_Params_t));

	if (edtd_type >= DTD_TYPE_MAX) {
		debug_pr(LOGL_ERR, "\n[%s:%d]Err: invalid dtd type as %d\n",
			 __func__, __LINE__, edtd_type);

		ret = -EINVAL;
	}
	if(ret == 0) {
		switch (edtd_type) {
		case DTD_TYPE_READ_EDID:
			ret = Dpv14_Tx_API_Get_Dtd_From_Edid(&stdtd_param, ucdp_index);
			if (ret != 0) {
				/* For KCS */
				debug_pr(LOGL_ERR, "\n[%s:%d]Err: can't find dtd from EDID\n", __func__, __LINE__);
			}
			break;
		case DTD_TYPE_CEA861_VIC:
		default:
			ret = Dpv14_Tx_API_Get_Dtd_From_VideoCode(uivic, &stdtd_param,
								  (uint32_t)DTD_REFRESH_RATE_60000,
								  (uint8_t)DTD_FORMAT_CEA_861);
			if (ret != 0) {
				/* For KCS */
				debug_pr(LOGL_ERR, "\n[%s:%d]Err: fail to get dtd from vic(%u)\n",
					 __func__, __LINE__, uivic);
			}
			break;
		}

		debug_pr(LOGL_INFO, "\n[%s:%d]VIC(%u) :\n", __func__, __LINE__, uivic);
		debug_pr(LOGL_INFO, "Pixel clk = %u\n", (u32)stdtd_param.uiPixel_Clock);
		debug_pr(LOGL_INFO, "Repetition = %u\n", (u32)stdtd_param.pixel_repetition_input);
		debug_pr(LOGL_INFO, "%s\n", (stdtd_param.interlaced != 0U) ? "Interlace" : "Progressive");
		debug_pr(LOGL_INFO, "H Sync Polarity(%u), V Sync Polarity(%u)\n", (u32)stdtd_param.h_sync_polarity, (u32)stdtd_param.v_sync_polarity);
		debug_pr(LOGL_INFO, "H Active(%u), V Active(%u)\n", (u32)stdtd_param.h_active, (u32)stdtd_param.v_active);
		debug_pr(LOGL_INFO, "H Image size(%u), V Image size(%u)\n", (u32)stdtd_param.h_image_size, (u32)stdtd_param.v_image_size);
		debug_pr(LOGL_INFO, "H Blanking(%u), V Blanking(%u)\n", (u32)stdtd_param.h_blanking, (u32)stdtd_param.v_blanking);
		debug_pr(LOGL_INFO, "H Sync offset(%u), V Sync offset(%u)\n", (u32)stdtd_param.h_sync_offset, (u32)stdtd_param.v_sync_offset);
		debug_pr(LOGL_INFO, "H Sync width W(%u), V Sync width W(%u)\n", (u32)stdtd_param.h_sync_pulse_width, (u32)stdtd_param.v_sync_pulse_width);

		ret = dpv14_get_disp_timing_from_dtd(uivic, &stdtd_param, pstdisplay_timing);
	}

	return ret;
}

int32_t dpv14_enable(const uint32_t uiperi_pclk[DPTX_INPUT_STREAM_MAX])
{
	uint8_t ucdp_idx;
	int32_t ret = 0;
	struct dpv14_drv_params *pstdpv14_params;

	pstdpv14_params = &stdpv14_params;

	if (!pstdpv14_params->bskip_dp_init) {
		for (ucdp_idx = 0; ucdp_idx < pstdpv14_params->ucnum_of_dps; ucdp_idx++) {
			pstdpv14_params->uipclk[ucdp_idx] = uiperi_pclk[ucdp_idx];
		}

		ret = dpv14_start_dp_drv();
		if (ret == DPTX_API_RETURN_MST_ACT_TIMEOUT) {
			dpv14_reinit_dp_drv();

			ret = dpv14_start_dp_drv();
			if (ret == DPTX_API_RETURN_MST_ACT_TIMEOUT) {
				debug_pr(LOGL_ERR, "[%s:%d]Err: MST ACT Sequence timeout...", __func__, __LINE__);
			}
		}
	}
	return ret;
}

int32_t dpv14_Init(const struct dpv14_drv_params *pstdpv14_drv_params)
{
	int32_t ret = 0;
	uint32_t uitcc80xx_rev;
	struct dpv14_drv_params *pstdpv14_params;
	struct MAX968XX_Init_Params stMAX968XX_init_params;

	pstdpv14_params = &stdpv14_params;

	(void)memcpy(pstdpv14_params, pstdpv14_drv_params,
		     sizeof(struct dpv14_drv_params));

	if (!pstdpv14_params->bskip_dp_init) {
		uitcc80xx_rev = get_chip_rev();
		pstdpv14_params->ucchip_rev = (uint8_t)(uitcc80xx_rev & 0xFFU);

		debug_pr(LOGL_INFO, "\n[%s:%d]tcc_dpv14_ctrl Ver : %d.%d.%d -> DP %u Initializing\n",
						__func__,
						__LINE__,
						DPV14_CTRL_DRV_MAJOR_VER,
						DPV14_CTRL_DRV_MINOR_VER,
						DPV14_CTRL_DRV_MINOR_VER,
						pstdpv14_params->ucdp_id);
		debug_pr(LOGL_INFO, " Num of DPs : %u\n", pstdpv14_params->ucnum_of_dps);
		debug_pr(LOGL_INFO, " %s chips revision %s(%u)\n",
					__func__,
					(uitcc80xx_rev == (uint32_t)TCC80XX_REV_ES) ? "ES" :
					(uitcc80xx_rev == (uint32_t)TCC80XX_REV_CS) ? "CS" :"Unknown -> set to CS by default",
					uitcc80xx_rev);
		debug_pr(LOGL_INFO, " DP %s\n", (pstdpv14_params->bpanel_mode) ? "Panel mode" : "Monitor mode");
		if (pstdpv14_params->bpanel_mode) {
			/* For KCS */
			debug_pr(LOGL_INFO, " 2C Port : %u\n", pstdpv14_params->uci2c_port);
		}
		debug_pr(LOGL_INFO, " SDM Bypass %s, SRVC Bypass %s\n",
					(pstdpv14_params->bsdm_bypass) ? "On" : "Off",
					(pstdpv14_params->btrvc_bypass) ? "On" : "Off");
		debug_pr(LOGL_INFO, " Phy lane swap %s\n", (pstdpv14_params->bphy_lane_swap) ? "On" : "Off");
		debug_pr(LOGL_INFO, " Encoding type: %s\n",
					(pstdpv14_params->ucpixel_encoding == (uint8_t)0U) ? "RGB" :
					(pstdpv14_params->ucpixel_encoding == (uint8_t)1U) ? "YCbCr222" :"YCbCr444");
		debug_pr(LOGL_INFO, " Max rate: %s, Max lane: %s\n",
					(pstdpv14_params->ucmax_rate == (uint8_t)0U) ? "RBR" :
					(pstdpv14_params->ucmax_rate == (uint8_t)1U) ? "HBR" :
					(pstdpv14_params->ucmax_rate == (uint8_t)2U) ? "HBR2" :"HBR3",
					(pstdpv14_params->ucmax_lane == (uint8_t)1U) ? "1 lane" :
					(pstdpv14_params->ucmax_lane == (uint8_t)2U) ? "2 lanes" :"4 lanes");
		debug_pr(LOGL_INFO, " Vcp id : %u %u %u %u\n", pstdpv14_params->ucvcp_id[0], pstdpv14_params->ucvcp_id[1], pstdpv14_params->ucvcp_id[2], pstdpv14_params->ucvcp_id[3]);
		debug_pr(LOGL_INFO, " VIC : %u %u %u %u\n", pstdpv14_params->uivic[0], pstdpv14_params->uivic[1], pstdpv14_params->uivic[2], pstdpv14_params->uivic[3]);
		debug_pr(LOGL_INFO, " Mux id : %u %u %u %u\n\n", pstdpv14_params->uidd_mux_id[0], pstdpv14_params->uidd_mux_id[1], pstdpv14_params->uidd_mux_id[2], 	pstdpv14_params->uidd_mux_id[3]);

		if (pstdpv14_params->bpanel_mode) {
			stMAX968XX_init_params.uci2c_port = pstdpv14_params->uci2c_port;
			stMAX968XX_init_params.ucnum_of_dps = pstdpv14_params->ucnum_of_dps;
			stMAX968XX_init_params.ucevb_pow_type = pstdpv14_params->ucevb_power_type;
			stMAX968XX_init_params.uivic = pstdpv14_params->uivic[MAX968XX_INPUT_STREAM_0];

			(void)memcpy((void *)stMAX968XX_init_params.aucvcp_id, (void *)pstdpv14_params->ucvcp_id, sizeof(uint8_t) * (unsigned)MAX968XX_INPUT_STREAM_MAX);

			if (!pstdpv14_params->bphy_lane_swap) {
				stMAX968XX_init_params.auclane_order[0] = (uint8_t)LINK_LANE_2;
				stMAX968XX_init_params.auclane_order[1] = (uint8_t)LINK_LANE_3;
				stMAX968XX_init_params.auclane_order[2] = (uint8_t)LINK_LANE_0;
				stMAX968XX_init_params.auclane_order[3] = (uint8_t)LINK_LANE_1;
			} else {
				stMAX968XX_init_params.auclane_order[0] = (uint8_t)LINK_LANE_0;
				stMAX968XX_init_params.auclane_order[1] = (uint8_t)LINK_LANE_1;
				stMAX968XX_init_params.auclane_order[2] = (uint8_t)LINK_LANE_2;
				stMAX968XX_init_params.auclane_order[3] = (uint8_t)LINK_LANE_3;
			}

			(void)max968xx_init(&stMAX968XX_init_params);
		}

		(void)dpv14_init_dp_drv(pstdpv14_params);
	}

	return ret;
}


