// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/

#include <config.h>
#include <common.h>
#include <linux/compat.h>

#include <dm/uclass.h>
#include <telechips/fb_bootstage.h>
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
	const struct dpv14_drv_params *dpv14_params;

	dpv14_params = &stdpv14_params;

	debug_pr(LOGL_WARNING, "[%s:%d]Re-initializing DP Link..\n", __func__, __LINE__);

	(void)Dpv14_Tx_API_Stop();
	(void)Dpv14_Tx_API_Deinit();

	(void)Dpv14_Tx_API_Init(dpv14_params);
}

static int32_t dpv14_start_dp_drv(void)
{
	int32_t ret = 0;
	struct dpv14_drv_params *dpv14_params;

	dpv14_params = &stdpv14_params;

	debug_pr(LOGL_DEBUG, "[%s:%d]P Clk : %u %u %u %u\n", __func__, __LINE__,
				dpv14_params->pclk[0],
				dpv14_params->pclk[1],
				dpv14_params->pclk[2],
				dpv14_params->pclk[3]);

	ret = Dpv14_Tx_API_Start(dpv14_params->num_of_dps, dpv14_params->pclk);

	return ret;
}

static int32_t dpv14_init_dp_drv(const struct dpv14_drv_params *dpv14_params)
{
	return Dpv14_Tx_API_Init(dpv14_params);
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
	debug_pr(LOGL_DEBUG, " %s\n", (pstdisp_timing->uiinterlaced != 0U) ? "Interlace" : "Progressive");
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
	enum DPTX_DTD_FORMAT_TYPE dtd_format_type = DTD_FORMAT_CEA_861;

	(void)memset(&stdtd_param, 0, sizeof(struct DPTX_Dtd_Params_t));

	switch (edtd_type) {
	case DTD_TYPE_READ_EDID:
		ret = Dpv14_Tx_API_Get_Dtd_From_Edid(&stdtd_param, ucdp_index);
		if (ret != 0) {
			/* For KCS */
			debug_pr(LOGL_ERR, "\n[%s:%d]Err: can't find dtd from EDID\n", __func__, __LINE__);
		}
		break;
	case DTD_TYPE_CEA861_VIC:
		dtd_format_type = DTD_FORMAT_CEA_861;
		ret = Dpv14_Tx_API_Get_Dtd_From_VideoCode(uivic, &stdtd_param,
							  (uint32_t)DTD_REFRESH_RATE_60000,
							  (uint8_t)dtd_format_type);

		if (ret != 0) {
			/* For KCS */
			debug_pr(LOGL_ERR, "\n[%s:%d]Err: fail to get dtd from vic(%u)\n",
				 __func__, __LINE__, uivic);
		}
		break;
	case DTD_TYPE_VESA_CVT_VIC:
		dtd_format_type = DTD_FORMAT_VESA_CVT;
		ret = Dpv14_Tx_API_Get_Dtd_From_VideoCode(uivic, &stdtd_param,
							  (uint32_t)DTD_REFRESH_RATE_60000,
							  (uint8_t)dtd_format_type);

		if (ret != 0) {
			/* For KCS */
			debug_pr(LOGL_ERR, "\n[%s:%d]Err: fail to get dtd from vic(%u)\n",
				 __func__, __LINE__, uivic);
		}
		break;
	case DTD_TYPE_VESA_DMT_VIC:
		dtd_format_type = DTD_FORMAT_VESA_DMT;
		ret = Dpv14_Tx_API_Get_Dtd_From_VideoCode(uivic, &stdtd_param,
							  (uint32_t)DTD_REFRESH_RATE_60000,
							  (uint8_t)dtd_format_type);

		if (ret != 0) {
			/* For KCS */
			debug_pr(LOGL_ERR, "\n[%s:%d]Err: fail to get dtd from vic(%u)\n",
				 __func__, __LINE__, uivic);
		}
		break;
	default:
		/* For KCS */
		debug_pr(LOGL_ERR, "\n[%s:%d]Err: invalid dtd type as %d\n",
			 __func__, __LINE__, edtd_type);

		ret = -EINVAL;
		break;
	}

	if (ret == 0) {
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
	struct dpv14_drv_params *dpv14_params;

	dpv14_params = &stdpv14_params;

	for (ucdp_idx = 0; ucdp_idx < dpv14_params->num_of_dps; ucdp_idx++) {
		dpv14_params->pclk[ucdp_idx] = uiperi_pclk[ucdp_idx];
	}

	ret = dpv14_start_dp_drv();
	if (ret == -DPTX_API_RETURN_MST_ACT_TIMEOUT) {
		dpv14_reinit_dp_drv();

		ret = dpv14_start_dp_drv();
		if (ret == -DPTX_API_RETURN_MST_ACT_TIMEOUT) {
			debug_pr(LOGL_ERR, "[%s:%d]Err: MST ACT Sequence timeout...", __func__, __LINE__);
		}
	}
	return ret;
}

void dpv14_debug_pr_drv_params(enum log_level_t log_level, const struct dpv14_drv_params *dpv14_params)
{

		debug_pr(log_level, " Num of DPs : %u\n", dpv14_params->num_of_dps);
		debug_pr(log_level, " %s chips revision %s(%u)\n",
					__func__,
					(dpv14_params->chip_rev == (uint32_t)TCC80XX_REV_ES) ? "ES" :
					(dpv14_params->chip_rev == (uint32_t)TCC80XX_REV_CS) ? "CS" : "Unknown -> set to CS by default",
					dpv14_params->chip_rev);
		debug_pr(log_level, " DP %s\n", (dpv14_params->panel_mode) ? "Panel mode" : "Monitor mode");
		if (dpv14_params->panel_mode) {
			/* For KCS */
			debug_pr(log_level, " I2C Port : %u\n", dpv14_params->i2c_port);
		}
		debug_pr(log_level, " SDM Bypass %s, SRVC Bypass %s\n",
					(dpv14_params->sdm_bypass) ? "On" : "Off",
					(dpv14_params->trvc_bypass) ? "On" : "Off");
		debug_pr(log_level, " Phy lane swap %s\n", (dpv14_params->phy_lane_swap) ? "On" : "Off");
		debug_pr(log_level, " Encoding type: %s\n",
					(dpv14_params->pixel_encoding == (uint8_t)0U) ? "RGB" :
					(dpv14_params->pixel_encoding == (uint8_t)1U) ? "YCbCr222" : "YCbCr444");
		debug_pr(log_level, " Max rate: %s, Max lane: %s\n",
					(dpv14_params->max_rate == (uint8_t)0U) ? "RBR" :
					(dpv14_params->max_rate == (uint8_t)1U) ? "HBR" :
					(dpv14_params->max_rate == (uint8_t)2U) ? "HBR2" : "HBR3",
					(dpv14_params->max_lane == (uint8_t)1U) ? "1 lane" :
					(dpv14_params->max_lane == (uint8_t)2U) ? "2 lanes" : "4 lanes");
		debug_pr(log_level, " Vcp id : %u %u %u %u\n",
					dpv14_params->vcp_id[0],
					dpv14_params->vcp_id[1],
					dpv14_params->vcp_id[2],
					dpv14_params->vcp_id[3]);
		debug_pr(log_level, " VIC : %u %u %u %u\n",
					dpv14_params->vic[0][VIC_CFG_VIDEO_CODE],
					dpv14_params->vic[1][VIC_CFG_VIDEO_CODE],
					dpv14_params->vic[2][VIC_CFG_VIDEO_CODE],
					dpv14_params->vic[3][VIC_CFG_VIDEO_CODE]);
		debug_pr(log_level, " Mux id : %u %u %u %u\n\n",
					dpv14_params->dd_mux_select[0],
					dpv14_params->dd_mux_select[1],
					dpv14_params->dd_mux_select[2],
					dpv14_params->dd_mux_select[3]);
}

int32_t dpv14_Init(const struct dpv14_drv_params *drv_params)
{
	int32_t ret = 0;
	struct dpv14_drv_params *dpv14_params;

	dpv14_params = &stdpv14_params;

	(void)memcpy(dpv14_params, drv_params,
		     sizeof(struct dpv14_drv_params));

	debug_pr(LOGL_INFO, "\n[%s:%d]tcc_dpv14_ctrl Ver : %d.%d.%d -> DP %u Initializing\n",
					__func__,
					__LINE__,
					DPV14_CTRL_DRV_MAJOR_VER,
					DPV14_CTRL_DRV_MINOR_VER,
					DPV14_CTRL_DRV_MINOR_VER,
					dpv14_params->dp_id);
	dpv14_debug_pr_drv_params(LOGL_INFO, dpv14_params);
	if (dpv14_params->panel_mode) {
		BOOTSTASGE_NAME("dp: serdes init");
		(void)max968xx_init(dpv14_params);
		BOOTSTASGE_NAME("dp: serdes done");
	}

	(void)dpv14_init_dp_drv(dpv14_params);

	return ret;
}


