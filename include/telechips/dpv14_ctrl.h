/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TCC_DPV14_H__
#define __TCC_DPV14_H__

#include <telechips/fb_dm.h>

#define VIC_TO_READ_EDID     0U

enum DP_LINK_LANE {
	DP_LANE_1 = 1,
	DP_LANE_2 =  2,
	DP_LANE_4 = 4
};

enum DPV14_DTD_TYPE {
	DTD_TYPE_READ_EDID = 0,
	DTD_TYPE_CEA861_VIC = 1,
	DTD_TYPE_VESA_CVT_VIC = 2,
	DTD_TYPE_VESA_DMT_VIC = 3,
	DTD_TYPE_MAX = 4
};

enum DPV14_RESET_TYPE {
	RESET_TYPE_COLD = 0,
	RESET_TYPE_CORE = 1,
	RESET_TYPE_INVALID = 2
};

struct dpv14_display_timing {
	uint32_t uipixelclock;
	uint32_t uiinterlaced;
	uint32_t uipixel_repetition;
	uint32_t uihactive;		/* hor. active video */
	uint32_t uihfront_porch;	/* hor. front porch */
	uint32_t uihback_porch;	/* hor. back porch */
	uint32_t uihsync_len;		/* hor. sync len */
	uint32_t uivactive;		/* ver. active video */
	uint32_t uivfront_porch;	/* ver. front porch */
	uint32_t uivback_porch;	/* ver. back porch */
	uint32_t uivsync_len;		/* ver. sync len */
	uint32_t flags;
};

enum {
	VIC_CFG_VIDEO_FORMAT = 0,
	VIC_CFG_VIDEO_CODE,
	VIC_CFG_NUM
};

enum {
	DP_BANDWIDTH_CFG_LANE = 0,
	DP_BANDWIDTH_CFG_RATE,
	DP_BANDWIDTH_CFG_NUM
};

struct dptx_phy_eq {
	uint32_t main_eq[16];
	uint32_t post_eq[16];
};

/**
 * @struct dptx_hw_config
 * @brief Structure representing the DisplayPort hardware configuration.
 *
 * This structure contains the configuration for DisplayPort transmitter.
 */
struct dptx_hw_config {
	/**
	 * @brief AUX hysteresis voltage levels.
	 *
	 * - [D3] :
	 *   - 0: 0mV
	 *   - 1: 10mV
	 *   - 2: 30mV
	 *   - 3: 40mV
	 * - [D5] Latch based:
	 *   - 0: +-3.81mV
	 *   - 1: +-38.8mV
	 *   - 2: +-51m
	 *   - 3: +-64.5mV
	 */
	uint32_t aux_hysteresis;

	/**
	 * @brief Indicates if hardware configuration supports sideband messaging.
	 *
	 * - 0: Not supported
	 * - 1: Supported
	 */
	uint32_t support_sideband_msg;

	/**
	 * @brief Indicates if hardware configuration supports spread specturm clock.
	 *
	 * - 0: Not supported
	 * - 1: Supported
	 */
	uint32_t support_spread_specturm_clock;

 	bool phy_eq_manual_mode;
	struct dptx_phy_eq phy_eq[4];
};

struct dpv14_drv_params {
	bool panel_mode;
	bool sdm_bypass;
	bool trvc_bypass;
	bool phy_lane_swap;
	uint8_t chip_rev;
	uint8_t dp_id;
	uint8_t num_of_dps;
	uint8_t i2c_port;
	uint8_t max_rate;
	uint8_t max_lane;
	uint8_t pixel_encoding;
	uint8_t vcp_id[PANEL_DP_MAX];
	uint32_t dd_mux_select[PANEL_DP_MAX];
	uint32_t vic[PANEL_DP_MAX][VIC_CFG_NUM];
	uint32_t pclk[PANEL_DP_MAX];

	uint8_t phy_model;
	uint8_t evb_power_type;

	uint8_t lane_order[DP_LANE_4];
	struct dptx_hw_config hw_config;
};


void dpv14_debug_pr_drv_params(enum log_level_t log_level, const struct dpv14_drv_params *dpv14_params);
int32_t dpv14_Init(const struct dpv14_drv_params *drv_params);
int32_t dpv14_enable(const uint32_t uiperi_pclk[PANEL_DP_MAX]);
int32_t dpv14_get_dtd(uint8_t ucdp_index,
							uint32_t uivic,
							enum DPV14_DTD_TYPE edtd_type,
							struct dpv14_display_timing *pstdisplay_timing);
int32_t dpv14_get_reset_type(enum DPV14_RESET_TYPE *pereset_type);

#endif
