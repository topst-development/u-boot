// SPDX-License-Identifier: (GPL-2.0-or-later OR MIT)
/*
 * Copyright (C) 2023 Telechips Inc.
 */
#ifndef DT_BIND_TCC_DISP_DEFS_H
#define DT_BIND_TCC_DISP_DEFS_H

#define DT_DISPDEV_NONE         0x00

#define DT_DISPDEV_LVDS0	0x10
#define DT_DISPDEV_LVDS1	0x11
#define DT_DISPDEV_LVDS2	0x12

#define DT_DISPDEV_HDMI		0x20

#define DT_DISPDEV_DP0		0x30
#define DT_DISPDEV_DP1		0x31
#define DT_DISPDEV_DP2		0x32
#define DT_DISPDEV_DP3		0x33

#define DT_DISPDEV_DSI0		0x40
#define DT_DISPDEV_DSI1		0x41

/* DISPLAY PORT */
#define DPTX_PHY_DEVICE_SNPS	0
#define DPTX_PHY_DEVICE_SEC	1
#define DPTX_PHY_DEVICE_UNKNOWN	2

/* EVB LCD POWER TYPE */
/* EVB: TCC8059 */
#define TCC_EVB_LCD_ONE_POW	0

/* EVB: TCC8050, TCC8053 and TCC8070 */
#define TCC_EVB_LCD_FOUR_POW	1

#define TCC_EVB_LCD_POW_MAX	2

#endif
