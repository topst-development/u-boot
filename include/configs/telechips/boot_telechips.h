/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TELECHIPS_BOOT_TELECHIPS_CONFIG_H
#define TELECHIPS_BOOT_TELECHIPS_CONFIG_H

/*
 * Command: preboot
 */

#define COMMAND_PREBOOT \
	"env set first_try true; " \
	"blkread dev ${bootdev} ${bootdevnum}; " \
	"run select_ab; " \
	"if test ${boot_reason} = fastboot; then " \
		"echo Entering fastboot mode...; " \
		"run fastbootcmd; " \
	"fi"

/*
 * Command: bootcmd
 */

#if defined(CONFIG_TELECHIPS_FB_DM)
#define COMMAND_RUN \
	"run preload loadimg fb_backlight loadfdt setupfdt postload bootkernel" "\0"
#else
#define COMMAND_RUN \
	"run preload loadimg loadfdt setupfdt postload bootkernel" "\0"
#endif

#define COMMAND_BOOT \
	COMMAND_RUN  \
	"preload=" "true" "\0" \
	"postload=" "true" "\0" \
	"fb_backlight=" "fb_dm show" "\0" \
	"loadimg=" COMMAND_LOADIMG "\0" \
	"loadfdt=" COMMAND_LOADFDT "\0" \
	"setupfdt=" COMMAND_SETUPFDT "\0" \
	"bootkernel=" COMMAND_BOOTKERNEL

#if defined(CONFIG_TCC_DMMETA_IMAGE)
#define COMMAND_LOADIMG \
	"blkread info ${rootpart}${_slot} root_devnum root_partnum; " \
	"if test $? -eq 0; then " \
		"blkread dmmeta ${rootpart}${_slot} " ROOT_DEV "; " \
		"env set bootargs " \
			"\"${bootargs} root=/dev/dm-0 rootwait ${dmsetup}\"; " \
	"fi; " \
	"blkread image ${bootpart}${_slot} ${kerneladdr}"
#else
#define COMMAND_LOADIMG \
	"blkread info ${rootpart}${_slot} root_devnum root_partnum; " \
	"if test $? -eq 0; then " \
		"env set bootargs ${bootargs} root=" ROOT_DEV " rootwait; " \
	"fi; " \
	"blkread image ${bootpart}${_slot} ${kerneladdr}"
#endif

#define COMMAND_LOADFDT \
	"blkread fdt ${fdtpart}${_slot} ${fdt_addr}"

#define COMMAND_SETUPFDT \
	"fdt addr ${fdt_addr}; fdt resize"

#define COMMAND_BOOTKERNEL \
	"if ${first_try}; then " \
		"env set bootargs \"${bootargs} bootreason=${boot_reason}\"; " \
		"env set first_try false; " \
	"fi; " \
	"bootm ${kerneladdr} - ${fdt_addr}"

/*
 * Command: fastbootcmd
 */

#define COMMAND_FASTBOOT \
	"fastboot " FASTBOOT_DEVICE

#endif /* TELECHIPS_BOOT_TELECHIPS_CONFIG_H */
