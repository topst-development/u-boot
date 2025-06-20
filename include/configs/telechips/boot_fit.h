/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TELECHIPS_BOOT_FIT_CONFIG_H
#define TELECHIPS_BOOT_FIT_CONFIG_H

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

#define COMMAND_BOOT \
	"run preload loadimg postload bootkernel" "\0" \
	"preload=" "true" "\0" \
	"postload=" "true" "\0" \
	"loadimg=" COMMAND_LOADIMG "\0" \
	"bootkernel=" COMMAND_BOOTKERNEL

#define COMMAND_LOADIMG \
	"blkread info ${rootpart}${_slot} root_devnum root_partnum; " \
	"if test $? -eq 0; then " \
		"setenv bootargs ${bootargs} root=" ROOT_DEV "; " \
	"fi; " \
	"blkread fdt ${bootpart}${_slot} ${kerneladdr}"

#define COMMAND_BOOTKERNEL \
	"if ${first_try}; then " \
		"env set bootargs ${bootargs} bootreason=${boot_reason}; " \
		"env set first_try false; " \
	"fi; " \
	"bootm ${kerneladdr}"

/*
 * Command: fastbootcmd
 */

#define COMMAND_FASTBOOT \
	"fastboot " FASTBOOT_DEVICE

#endif /* TELECHIPS_BOOT_FIT_CONFIG_H */
