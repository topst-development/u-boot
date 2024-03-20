// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_CONFIG_FALLBACKS_H
#define TCC_CONFIG_FALLBACKS_H

/*
 * include/configs/tcc/common.h
 */

#ifndef SYSTEM_BOOTARGS
#  define SYSTEM_BOOTARGS ""
#endif

/*
 * include/configs/tcc/boot.h
 */

#ifndef AVB_VERIFY_CHECK
#  define AVB_VERIFY_CHECK ""
#endif

#ifndef AVB_VERIFY_CMD_SETTINGS
#  define AVB_VERIFY_CMD_SETTINGS ""
#endif

#ifndef APPLY_DTO_COMMAND
#  define APPLY_DTO_COMMAND "true"
#endif

/*
 * include/configs/tcc/partition.h
 */

#ifndef VBMETA_PART
#  define VBMETA_PART ""
#endif

#ifndef TAMPER_EVIDENCE_PART
#  define TAMPER_EVIDENCE_PART ""
#endif

/*
 * include/configs/tccxxxx.h
 */

#ifndef START_EARLYCAMERA_SOLUTION
#  define START_EARLYCAMERA_SOLUTION ""
#endif

#ifndef INIT_EARLYCAMERA_SOLUTION
#  define INIT_EARLYCAMERA_SOLUTION(fdt_addr) ""
#endif

#ifndef STORE_SUBCORE_IMAGE_SIZE_INFO
#  define STORE_SUBCORE_IMAGE_SIZE_INFO ""
#endif

#ifndef LOAD_SUBCORE_IMAGE_FOR_BACKUP
#  define LOAD_SUBCORE_IMAGE_FOR_BACKUP ""
#endif

#ifndef BOOT_SUBCORE_COMMAND
#  define BOOT_SUBCORE_COMMAND ""
#endif

#ifndef LOAD_IMAGE_COMMAND
#  define LOAD_IMAGE_COMMAND ""
#endif

#ifndef AB_COMMAND
#  define AB_COMMAND ""
#endif

#ifndef SUBCORE_ENV_SETTINGS
#  define SUBCORE_ENV_SETTINGS ""
#endif

#ifndef MISC_ENV_SETTINGS
#  define MISC_ENV_SETTINGS ""
#endif

#ifndef BOOTMENU_ENV_SETTINGS
#  define BOOTMENU_ENV_SETTINGS ""
#endif

#ifndef BL3_PART
#  define BL3_PART ""
#endif

#ifndef PREBOOT_COMMAND
#  define PREBOOT_COMMAND ""
#endif

#ifndef BCB_COMMAND
#  define BCB_COMMAND ""
#endif

#endif /* TCC_CONFIG_FALLBACKS_H */
