// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TCC_DPV14_H__
#define __TCC_DPV14_H__

#include <telechips/fb_dm.h>


#define VIC_TO_READ_EDID     0

enum DPV14_DTD_TYPE {
	DTD_TYPE_READ_EDID = 0,
	DTD_TYPE_CEA861_VIC = 1,
	DTD_TYPE_MAX = 2
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
	enum display_flags flags;
};

struct dpv14_drv_params {
	bool bpanel_mode;
	bool bskip_dp_init;
	bool bsdm_bypass;
	bool btrvc_bypass;
	bool bphy_lane_swap;
	uint8_t ucchip_rev;
	uint8_t ucdp_id;
	uint8_t ucnum_of_dps;
	uint8_t uci2c_port;
	uint8_t ucmax_rate;
	uint8_t ucmax_lane;
	uint8_t ucpixel_encoding;
	uint8_t ucvcp_id[PANEL_DP_MAX];
	uint32_t uidd_mux_id[PANEL_DP_MAX];
	uint32_t uivic[PANEL_DP_MAX];
	uint32_t uipclk[PANEL_DP_MAX];


	uint8_t ucphy_model;
	uint8_t ucevb_power_type;
};


int32_t dpv14_Init(struct dpv14_drv_params *pstdpv14_drv_params);
int32_t dpv14_enable(uint32_t uiperi_pclk[PANEL_DP_MAX]);
int32_t dpv14_get_dtd(uint8_t ucdp_index,
							uint32_t uivic,
							enum DPV14_DTD_TYPE edtd_type,
							struct dpv14_display_timing *pstdisplay_timing);
int32_t dpv14_get_reset_type(enum DPV14_RESET_TYPE *pereset_type);

#endif
