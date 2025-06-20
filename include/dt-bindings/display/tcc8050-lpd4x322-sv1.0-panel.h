// SPDX-License-Identifier: (GPL-2.0-or-later OR MIT)
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef DT_BIND_TCC8050_LPD4X322_SV1_0_DISP_H
#define DT_BIND_TCC8050_LPD4X322_SV1_0_DISP_H

/* Definition for H/W LCD ports */
#define LCD_PORT1_PWR "gpa-21"
#define LCD_PORT1_RST "gpg-6"
#define LCD_PORT1_BLK "gph-6"

#define LCD_PORT2_PWR "gpmb-28"
#define LCD_PORT2_RST "gpc-12"
#define LCD_PORT2_BLK "gph-7"

#define LCD_PORT3_PWR "gpc-9"
#define LCD_PORT3_RST "gpc-8"
#define LCD_PORT3_BLK "gpmb-29"

#define LCD_PORT4_PWR "gpmc-23"
#define LCD_PORT4_RST "gpmc-25"
#define LCD_PORT4_BLK "gpmc-21"

#define DP_HPD_GPIO "gpc-14"
#define DP_SERDES_INTB "gpsd0-11"
#define DP_SERDES_LOCK "gpmc-0"

#endif
