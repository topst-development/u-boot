// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef _TCC_LCD_H_
#define _TCC_LCD_H_

#define GPIO_EXT1               0xFFFF0001

enum {
	LCDC_COMPONENT_480I_NTSC,	/* N/A */
	LCDC_COMPONENT_576I_PAL,	/* N/A */
	LCDC_COMPONENT_720P,
	LCDC_COMPONENT_1080I,
	LCDC_COMPONENT_MAX
};

#endif
