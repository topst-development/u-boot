/* SPDX-License-Identifier: GPL-2.0 */
/**
 * Copyright (C) 2018 Synopsys, Inc.
 *
 * @file dsih_displays.h
 * @brief Synopsys MIPI DSI Host display support
 * included as a part of Synopsys MIPI DSI Host controller driver
 *
 * @author Luis Oliveira <luis.oliveira@synopsys.com>
 * Modified by Telechips
 */

#ifndef MIPI_DISPLAYS_H_
#define MIPI_DISPLAYS_H_

#include "dsih_includes.h"

enum display_types{
	TREMOLO_S = 1,
	TREMOLO_M = 2,
	SHARP_LS045K3SX02 = 3,
	RPI_DISPLAY = 4,
	SNPS_DSI_DEVICE = 5,
	TESTING
};

void copy_dpi_param_changes(dsih_dpi_video_t * from_param,
			    dsih_dpi_video_t * to_param);
#endif