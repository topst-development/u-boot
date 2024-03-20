// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC8030_EVB_CONFIG_H
#define TCC8030_EVB_CONFIG_H

#include "tcc803x.h"

/* LCD Configurations */
#define CONFIG_BMP_16BPP
#define CONFIG_BMP_24BPP
#define CONFIG_BMP_32BPP
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SPLASH_SCREEN_ALIGN

#define CONFIG_FB_ADDR                  0x2A000000
#define LCD_BPP                         LCD_COLOR16

/* 8MB -> it's up to 1920 x 1080 ARBG 32Bits. other than, it's limited. */
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE  (1024 * 1024 * 8)

/* LCD GPIO pins Configurations */
#define LCDC_NUM                        0
#define GPIO_LCD_ON                     TCC_GPB(17)
#define GPIO_LCD_BL                     TCC_GPMA(26)
#define GPIO_LCD_DISPLAY                GPIO_NC
#define GPIO_LCD_RESET                  TCC_GPB(18)

/* LVDS Specific GPIO pins */
#define GPIO_LVDS_STBYB                 GPIO_NC
#define GPIO_LVDS_EN                    GPIO_NC
#define GPIO_V_5P0_EN                   GPIO_NC

/* RGB I/F specific GPIO pins */
#define GPIO_L0_LPXCLK                  TCC_GPB(0)
#define GPIO_L0_LHSYNC                  TCC_GPB(1)
#define GPIO_L0_LVSYNC                  TCC_GPB(2)
#define GPIO_L0_LACBIAS                 TCC_GPB(19)
#define GPIO_L0_LPD(X)                  TCC_GPB(X)
#define GPIO_L0_LPD_ADJUST_VALUE1       3
#define GPIO_L0_LPD_ADJUST_VALUE2       4

#define GPIO_L1_LPXCLK                  GPIO_NC
#define GPIO_L1_LHSYNC                  GPIO_NC
#define GPIO_L1_LVSYNC                  GPIO_NC
#define GPIO_L1_LACBIAS                 GPIO_NC
#define GPIO_L1_LPD(X)                  GPIO_NC
#define GPIO_L1_LPD_ADJUST_VALUE1       2
#define GPIO_L1_LPD_ADJUST_VALUE2       2

#if defined(CONFIG_SPLASH_SCREEN_ALIGN)
#define BMP_ALIGN_CENTER       0x7FFF
#endif

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)
#define DC0_24BIT_SPLASH_BMP_NAME       "DC0_24Bit"
#define DC1_24BIT_SPLASH_BMP_NAME       "DC1_24Bit"
#define DC2_24BIT_SPLASH_BMP_NAME       "DC2_24Bit"
#define DC3_24BIT_SPLASH_BMP_NAME       "DC3_24Bit"

#define MAX_INDIVIDUAL_IMAGE_WIDTH      1920
#define MAX_INDIVIDUAL_IMAGE_HEIGHT     1080

#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC0)
#define DC0_TCCLOGO_HEADER	   "mach/tcc_logo_cid_24bit.h"
#endif

#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC1)
#define DC1_TCCLOGO_HEADER	   "mach/tcc_logo_hvac_24bit.h"
#endif

#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC2)
#define DC2_TCCLOGO_HEADER	   "mach/tcc_logo_cluster_24bit.h"
#endif

#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC3)
#define DC3_TCCLOGO_HEADER	   "mach/tcc_logo_hud_24bit.h"
#endif

#else
#define TCCLOGO_HEADER	  "mach/tcc_logo_24bit.h"
#endif /* #if defined( CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH ) */

/* PIXEL ENCODING TYPE
 * RGB      - 0
 * YCbCr422 - 1
 * YCbCr444 - 2
 */
#define LCD_PIXEL_ENCODING_TYPE         0

/* Telechips USB Configurations */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS     2

/***********************************************************
 ** Network
 ************************************************************/
#define CONFIG_DW_ALTDESCRIPTOR
#define CONFIG_PHY_RESET_DELAY      10000

#endif	/* TCC8030_EVB_CONFIG_H */
