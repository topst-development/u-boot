// SPDX-License-Identifier: (GPL-2.0-or-later OR MIT)
/*
 * Copyright (C) 2023 Telechips Inc.
 */
#ifndef DT_BIND_TCC8050_LPD4X322_SV0_1_DISP_H
#define DT_BIND_TCC8050_LPD4X322_SV0_1_DISP_H

/* Definition for H/W LCD ports */
#define LCD_PORT1_PWR "gpmc-19"
#define LCD_PORT1_RST "gpmc-20"
#define LCD_PORT1_BLK "gph-6"

#define LCD_PORT2_PWR "gpc-8"
#define LCD_PORT2_RST "gpc-9"
#define LCD_PORT2_BLK "gph-7"

#define LCD_PORT3_PWR "gpmc-2"
#define LCD_PORT3_RST "gpmc-18"
#define LCD_PORT3_BLK "gpe-15"

#define LCD_PORT4_PWR "gpmb-25"
#define LCD_PORT4_RST "gpmb-19"
#define LCD_PORT4_BLK "gpmc-21"

#define DP_HPD_GPIO "gpc-14"
#define DP_SERDES_INTB "gpe-19"
#define DP_SERDES_LOCK "gpmc-0"

#endif

