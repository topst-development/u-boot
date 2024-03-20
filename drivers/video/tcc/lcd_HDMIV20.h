// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 - present Telechips.co and/or its affiliates.
 */
#ifndef __LCD_HDMI_V2_0_H__
#define __LCD_HDMI_V2_0_H__

void hdmiv20_get_panel_info(struct vidinfo *vid);
void hdmiv20_set_panel_info(struct vidinfo *vid);
void hdmi_update_panel_info(
	int vic, int depth, int encoding, int framepacking);
int hdmi_get_framepacking(void);
#endif
