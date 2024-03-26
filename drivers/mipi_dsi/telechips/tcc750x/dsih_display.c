// SPDX-License-Identifier: GPL-2.0
/**
 * Copyright (C) 2018 Synopsys, Inc.
 *
 * @file dsih_displays.c
 * @brief included as a part of MIPI DSI Host driver
 * included as a part of Synopsys MIPI DSI Host controller driver
 *
 * @author Luis Oliveira <luis.oliveira@synopsys.com>
 * Modified by Telechips
 */

#include "dsih_display.h"
#include "dsih_api.h"
#include "dsih_hal.h"
#include "dsih_dphy.h"

void copy_dpi_param_changes(dsih_dpi_video_t * from_param,
			    dsih_dpi_video_t * to_param)
{
	to_param->no_of_lanes = from_param->no_of_lanes;
	to_param->non_continuous_clock = from_param->non_continuous_clock;
	to_param->virtual_channel = from_param->virtual_channel;
	to_param->video_mode = from_param->video_mode;
	to_param->byte_clock = from_param->byte_clock;
	to_param->pixel_clock = from_param->pixel_clock;
	to_param->color_coding = from_param->color_coding;
	to_param->is_18_loosely = from_param->is_18_loosely;
	to_param->h_polarity = from_param->h_polarity;
	to_param->h_active_pixels = from_param->h_active_pixels;
	to_param->h_sync_pixels = from_param->h_sync_pixels;
	to_param->h_back_porch_pixels = from_param->h_back_porch_pixels;
	to_param->h_total_pixels = from_param->h_total_pixels;
	to_param->v_polarity = from_param->v_polarity;
	to_param->v_active_lines = from_param->v_active_lines;
	to_param->v_sync_lines = from_param->v_sync_lines;
	to_param->v_back_porch_lines = from_param->v_back_porch_lines;
	to_param->v_total_lines = from_param->v_total_lines;
	to_param->max_hs_to_lp_cycles = from_param->max_hs_to_lp_cycles;
	to_param->max_lp_to_hs_cycles = from_param->max_lp_to_hs_cycles;
	to_param->max_clk_hs_to_lp_cycles = from_param->max_clk_hs_to_lp_cycles;
	to_param->max_clk_lp_to_hs_cycles = from_param->max_clk_lp_to_hs_cycles;
	to_param->eotp_rx_en = from_param->eotp_rx_en;
	to_param->eotp_tx_en = from_param->eotp_tx_en;
	to_param->no_of_chunks = from_param->no_of_chunks;
	to_param->null_packet_size = from_param->null_packet_size;
	to_param->dpi_lp_cmd_en = from_param->dpi_lp_cmd_en;
}
