// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef CAM_LCD_H
#define CAM_LCD_H

#include "tcc_lcd.h"

extern int tcc_lcd_is_disp_busy(void);
extern int tcc_lcd_set_rdma(unsigned int addr_y,
	unsigned int width, unsigned int height, unsigned int fmt);
extern void tcc_lcd_rear_camera_display(unsigned char on_off);
extern void tcc_lcd_rear_camera_wmix_channel_prioty(void);

#endif//EARLYCAM_LCD_H_
