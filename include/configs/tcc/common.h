// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_COMMON_CONFIG_H
#define TCC_COMMON_CONFIG_H

#include <linux/sizes.h>

#define CONFIG_SYS_BOOTM_LEN	(SZ_64M)
#define CONFIG_SYS_CBSIZE	(SZ_2K)

/*
 * Default Setup
 */

#if defined(CONFIG_ARM64)
#  define KERNEL_ARCH		64
#  define FDT_HIGH		FFFFFFFFFFFFFFFF
#else
#  define KERNEL_ARCH		32
#  define FDT_HIGH		FFFFFFFF
#endif

/*
 * Kernel Bootargs Snippets
 */

#if defined(CONFIG_TCC_MAINCORE)
#  define CONSOLE_BOOTARGS	"console=ttyAMA0,115200n8"
#else
#  define CONSOLE_BOOTARGS	"console=ttyAMA4,115200n8"
#endif

#define ANDROID_BOOT_NORMAL(x)	"androidboot.force_normal_boot=" __stringify(x)
#define INITRD_BOOTARGS(x, y)	"initrd=" __stringify(x) "," __stringify(y)

/*
 * Default Kernel Bootargs
 */

#if defined(CONFIG_CMD_BOOT_ANDROID)
#  define KERNEL_BOOTARGS	CONSOLE_BOOTARGS " ro rootwait init=/init"
#elif defined(CONFIG_ARM64)
#  define KERNEL_BOOTARGS	CONSOLE_BOOTARGS " cgroup_disable=memory"
#else
#  define KERNEL_BOOTARGS	CONSOLE_BOOTARGS
#endif

#if defined(CONFIG_CMD_BOOT_ANDROID) && defined(CONFIG_ANDROID_RECOVERY_AS_BOOT)
#  define SYSTEM_BOOTARGS	ANDROID_BOOT_NORMAL(1)
#elif defined(CONFIG_TCC_SUBCORE) && defined(CONFIG_BOOT_SUBCORE_BY_MAINCORE)
#  define SYSTEM_BOOTARGS	INITRD_BOOTARGS(SUBCORE_INITRD_ADDR, 256M)
#endif

#define NORMAL_BOOTARGS		KERNEL_BOOTARGS " " SYSTEM_BOOTARGS

/*
 * Default Recovery Bootargs
 */

#if defined(CONFIG_CMD_BOOT_ANDROID)
#  define RECOVERY_BOOTARGS	CONSOLE_BOOTARGS " " ANDROID_BOOT_NORMAL(0)
#else
#  define RECOVERY_BOOTARGS	NORMAL_BOOTARGS " update_mode"
#endif

/*
 * Boot Environment Configuration
 */

#if defined(CONFIG_ENV_IS_IN_MMC) && defined(CONFIG_TCC_BOOT_SDHC)
#  define CONFIG_SYS_MMC_ENV_DEV CONFIG_TCC_BOOT_SDHC
#endif

#if defined(CONFIG_ENV_IS_IN_SCSI) && defined(CONFIG_UFS_BOOT)
#  define CONFIG_SYS_SCSI_ENV_DEV 2
#endif

#if defined(CONFIG_NAND_BOOT)
#  define BOOT_DEVICE	"nand"
#elif defined(CONFIG_SD_BOOT)
#  define BOOT_DEVICE	"mmc"
#elif defined(CONFIG_UFS_BOOT)
#  define BOOT_DEVICE	"scsi"
#  define UFS_BOOT
#else
#  error "Please select boot media"
#endif

#endif /* TCC_COMMON_CONFIG_H */
