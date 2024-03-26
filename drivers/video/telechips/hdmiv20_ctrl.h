// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 - present Telechips.co and/or its affiliates.
 */
#ifndef TCC_HDMI_V2_0_CTRL_H
#define TCC_HDMI_V2_0_CTRL_H

void hdmi_update_panel_info(
	int vic, int depth, int encoding, int framepacking);
int hdmi_get_framepacking(void);
// int hdmi_get_width_from_hdmi_info(void);
int hdmiv20_panel_init(unsigned int lcd_id, unsigned int vic);
int hdmiv20_set_power(int on, int lcd_id, int img_width, int img_height);
int hdmiv20_set_panel_ctrl(int level);

#endif
