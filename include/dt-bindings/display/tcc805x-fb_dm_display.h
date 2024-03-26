// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
 /*
  * Copyright (C) 2020 Telechips Inc.
   */
#ifndef DT_BINDINGS_TCC805X_FB_DM_DISPLAY_H
#define DT_BINDINGS_TCC805X_FB_DM_DISPLAY_H


#define LCD_MUX_MAX LCD_MUX3

/* define for DP V1.4 */
#define I2C_PORT_TO_SERDES 0

/* PERI_ name from tcc_ckc.h */
#define tcc_fb_dm_get_peri_id(lcd_id, peri_id) {                                \
	switch(lcd_id) {                                                        \
		case 0: peri_id = PERI_LCD0; break;                            \
		case 1: peri_id = PERI_LCD1; break;                            \
		case 2: peri_id = PERI_LCD2; break;                            \
		case 3: peri_id = PERI_LCD3; break;                            \
		default: peri_id = 0xFFFFU; break; /* error */                  \
	}                                                                       \
}
#endif

