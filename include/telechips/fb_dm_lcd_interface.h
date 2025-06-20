/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_FB_DM_LCD_INTERFACE_H
#define TCC_FB_DM_LCD_INTERFACE_H

int lcdc_turn_on_display(struct udevice *fb_dev);
int lcdc_display_device_init(struct udevice *fb_dev);
int lcdc_mux_select(unsigned int mux_id, unsigned int lcdc_id);
int lcdc_get_compatible_display(int rdma_id);
int lcdc_display_device_reset(struct udevice *fb_dev);
int lcdc_is_display_device_ready(int rdma_id);
void tcc_lcdc_color_setting(
	uint8_t lcdc, signed char contrast, signed char brightness,
	signed char hue);
void tcc_lcdc_dithering_setting(uint8_t lcdc);
void tcclcd_image_ch_set(
	unsigned int lcdctrl_num, const struct tcc_fb_dm_image_info *ImageInfo);
void tcclcd_gpioexp_set_value(unsigned int on);

#endif // TCC_FB_DM_LCD_INTERFACE_H
