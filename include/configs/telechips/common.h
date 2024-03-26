/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TELECHIPS_COMMON_CONFIG_H
#define TELECHIPS_COMMON_CONFIG_H

#include <linux/sizes.h>

/*
 * Boot Devices
 */

#if defined(CONFIG_SD_BOOT)
#define BOOT_DEVICE "mmc"
#define BOOT_DEVICE_NUM "0"
#define ROOT_DEV "/dev/mmcblk${root_devnum}p${root_partnum}"
#elif defined(CONFIG_UFS_BOOT)
#define BOOT_DEVICE "scsi"
#define BOOT_DEVICE_NUM "2"
#define ROOT_DEV "/dev/sdc${root_partnum}"
#else
#error "Please select boot media"
#endif

#if defined(CONFIG_USB_FUNCTION_FASTBOOT)
#define FASTBOOT_DEVICE "usb " __stringify(CONFIG_FASTBOOT_USB_DEV)
#elif defined(CONFIG_UDP_FUNCTION_FASTBOOT)
#define FASTBOOT_DEVICE "udp"
#else
#define FASTBOOT_DEVICE
#endif

/*
 * Common Command Set
 */

#define COMMAND_PREBOOT \
	"blkread dev ${bootdev} ${bootdevnum}; " \
	"run select_ab; " \
	"if test ${boot_reason} = fastboot; then " \
		"echo Entering fastboot mode...; " \
		"run fastbootcmd; " \
	"fi"

#if CONFIG_IS_ENABLED(CMD_AB_SELECT)
#define COMMAND_AB \
	"ab_select slot ${bootdev} ${bootdevnum}#${miscpart}; " \
	"if test $? -ne 0; then " \
		"echo Falling into fastboot mode...; " \
		"run fastbootcmd; " \
		"while true; do done;" \
	"fi; "
#endif

#define COMMAND_FASTBOOT \
	"fastboot " FASTBOOT_DEVICE

/*
 * Non-Kconfig Configs
 */

#define CONFIG_SYS_BOOTM_LEN (SZ_64M)
#define CONFIG_SYS_CBSIZE (SZ_2K)
#define CONFIG_SYS_MMC_MAX_BLK_COUNT (1024U)

/*
 * GMAC non cacheable DMA descriptor region.
 */

#define CONFIG_SYS_NONCACHED_MEMORY (SZ_1M)

/*
 * Network Configuration
 */

#define IPADDR_DEFAULT "192.168.0.99"
#define GATEWAYIP_DEFAULT "192.168.0.1"
#define NETMASK_DEFAULT "255.255.255.0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"preboot="	COMMAND_PREBOOT "\0" \
	"bootcmd="	COMMAND_BOOT "\0" \
	"fastbootcmd="	COMMAND_FASTBOOT "\0" \
	"select_ab="	COMMAND_AB "\0" \
	"bootdev="	BOOT_DEVICE "\0" \
	"bootdevnum="	BOOT_DEVICE_NUM "\0" \
	"kerneladdr="	__stringify(ADDR_KERNEL) "\0" \
	"fdt_addr="	__stringify(ADDR_FDT) "\0" \
	"bootpart="	PART_KERNEL "\0" \
	"fdtpart="	PART_FDT "\0" \
	"rootpart="	PART_SYSTEM "\0" \
	"miscpart="	PART_MISC "\0" \
	"slot="		"-\0" \
	"fdt_high="	"0xffffffffffffffff\0" \
	"ipaddr="	IPADDR_DEFAULT "\0" \
	"gatewayip="	GATEWAYIP_DEFAULT "\0" \
	"netmask="	NETMASK_DEFAULT "\0" \
	ARCH_EXTRA_ENV_SETTINGS \

/*
 * MMC Tuning Configurations
 */
#if defined(CONFIG_CMD_TCC_MMC_TUNING)
#define CONFIG_FIXED_SDHCI_ALIGNED_BUFFER 0x20000000
#endif

#endif /* TELECHIPS_COMMON_CONFIG_H */
