/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_FB_DM_LCD_INTERFACE_H
#define TCC_FB_DM_LCD_INTERFACE_H

#if defined(CONFIG_TCC807X)
int lcdc_set_svsc_clk(const struct udevice *fb_dev);
int lcdc_set_svsc_evs(const struct udevice *fb_dev);
#endif
int lcdc_turn_on_display(const struct udevice *fb_dev);
int lcdc_display_device_init(const struct udevice *fb_dev);
int lcdc_mux_select(unsigned int mux_select, unsigned int lcd_disp);
int lcdc_display_device_reset(const struct udevice *fb_dev);
void tcc_lcdc_color_setting(const struct tcc_fb_dm_priv *priv,
			    unsigned int contrast,
			    unsigned int brightness,
			    unsigned int hue);
void tcc_lcdc_dithering_setting(const struct tcc_fb_dm_priv *priv);
void tcclcd_image_ch_set(const struct tcc_fb_dm_priv *priv);
void tcclcd_gpioexp_set_value(unsigned int on);

struct udevice *tcc_get_remote_dev(const struct udevice *dev);

#endif // TCC_FB_DM_LCD_INTERFACE_H
