// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_DPV14_MAX968XX_H
#define TCC_DPV14_MAX968XX_H
#include <telechips/dpv14_ctrl.h>

enum DP_LINK_LANE_IDX {
	LINK_LANE_0 = 0,
	LINK_LANE_1 = 1,
	LINK_LANE_2 =  2,
	LINK_LANE_3 =  3,
	LINK_LANE_4 = 4
};

enum MAX968XX_INPUT_STREAM_INDEX {
	MAX968XX_INPUT_STREAM_0 = 0,
	MAX968XX_INPUT_STREAM_1 = 1,
	MAX968XX_INPUT_STREAM_2 = 2,
	MAX968XX_INPUT_STREAM_3 = 3,
	MAX968XX_INPUT_STREAM_MAX = 4
};

#if  0
enum TCC_EVB_LCD_POW_TYPE {
	TCC_EVB_LCD_ONE_POW = 0,
	TCC_EVB_LCD_FOUR_POW = 1,
	TCC_EVB_LCD_POW_MAX = 2
};
#endif
enum TCC_REVISION_TYPE {
	TCC80XX_REV_ES = 0,
	TCC80XX_REV_CS = 1,
	TCC80XX_REV_MAX = 2
};

int32_t max968xx_init(const struct dpv14_drv_params *drv_params);

#endif
