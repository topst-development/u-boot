/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TELECHIPS_BOOT_TELECHIPS_CONFIG_H
#define TELECHIPS_BOOT_TELECHIPS_CONFIG_H

/* loadimg */
#define COMMAND_LOADIMG \
	"blkread info ${rootpart} ${slot} root_devnum root_partnum; " \
	"if test $? -eq 0; then " \
		"setenv bootargs ${bootargs} root=" ROOT_DEV "; " \
	"fi; " \
	"blkread image ${bootpart} ${slot} ${kerneladdr}"

/* loadfdt */
#define COMMAND_LOADFDT \
	"blkread fdt ${fdtpart} ${slot} ${fdt_addr}"

/* setupfdt */
#define COMMAND_SETUPFDT \
	"fdt addr ${fdt_addr}; fdt resize"

/* bootkernel */
#define COMMAND_BOOTKERNEL \
	"bootm ${kerneladdr} - ${fdt_addr}"

/* bootcmd */
#define COMMAND_BOOT \
	"run preload loadimg loadfdt setupfdt postload bootkernel" "\0" \
	"preload=" "true" "\0" \
	"postload=" "true" "\0" \
	"loadimg=" COMMAND_LOADIMG "\0" \
	"loadfdt=" COMMAND_LOADFDT "\0" \
	"setupfdt=" COMMAND_SETUPFDT "\0" \
	"bootkernel=" COMMAND_BOOTKERNEL

#endif /* TELECHIPS_BOOT_TELECHIPS_CONFIG_H */
