// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
 /*
  * Copyright (C) 2020 Telechips Inc.
   */
#ifndef DT_BINDINGS_TCC750X_FB_DM_DISPLAY_H
#define DT_BINDINGS_TCC750X_FB_DM_DISPLAY_H

#define LCD_MUX_MAX LCD_MUX2

/* PERI_ name from tcc_ckc.h */
#define tcc_fb_dm_get_peri_id(lcd_id, peri_id) {                                \
	switch(lcd_id) {                                                        \
		case 0: peri_id = PERI_DISP; break;                            \
		default: peri_id = 0xFFFFU; break; /* error */                  \
	}                                                                       \
}
#endif

