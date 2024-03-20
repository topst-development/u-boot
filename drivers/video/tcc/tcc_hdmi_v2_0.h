// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 - present Telechips.co and/or its affiliates.
 */
#ifndef __TCC_FB_HDMI_V2_0_H__
#define __TCC_FB_HDMI_V2_0_H__

void tcc_hdmi_display_device_reset(unsigned char display_device_id);
unsigned int tcc_hdmi_get_width_from_vic(unsigned int vic);
void tcc_hdmi_init(
	unsigned char display_device_id, int hdmi_mode,
	int vic, int hz, int depth, enum em_encoding encoding,
	int img_width, int img_height);
#endif //__TCC_FB_HDMI_V2_0_H__
