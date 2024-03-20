// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 - present Telechips and/or its affiliates.
 */
#ifndef TCC_LCD_INTERFACE_H
#define TCC_LCD_INTERFACE_H
void lcdc_initialize(unsigned int lcdctrl_num, const struct vidinfo *lcd_spec);
void lcdc_deinitialize(unsigned int display_device_id);
int lcdc_mux_select(unsigned int mux_id, unsigned int lcdc_id);
void tcclcd_image_ch_set(unsigned int lcdctrl_num,
                       const struct tcc_lcdc_image_update *ImageInfo);
void tcclcd_gpio_config(unsigned int n, unsigned int flags);
void tcclcd_gpio_set_value(unsigned int n, unsigned int on);
unsigned char tcclcd_get_board_type(void);
void LCDC_IO_Set(char DD_num, char DP_num, unsigned int bit_per_pixel);
void LCDC_IO_Disable(char DP_num, unsigned int bit_per_pixel);
int lcdc_get_compatible_display(int rdma_id);
int lcdc_is_display_device_ready(int rdma_id);
void tcclcd_gpioexp_set_value(unsigned int on);
void tcc_lcdc_color_setting(
		uint8_t lcdc, signed char contrast, signed char brightness,
		signed char hue);
void tcc_lcdc_dithering_setting(uint8_t lcdc);

#endif
