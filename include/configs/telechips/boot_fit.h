/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TELECHIPS_BOOT_FIT_CONFIG_H
#define TELECHIPS_BOOT_FIT_CONFIG_H

/* loadimg */
#define COMMAND_LOADIMG \
	"blkread info ${rootpart} ${slot} root_devnum root_partnum; " \
	"if test $? -eq 0; then " \
		"setenv bootargs ${bootargs} root=" ROOT_DEV "; " \
	"fi; " \
	"blkread fdt ${bootpart} ${slot} ${kerneladdr}"

/* bootkernel */
#define COMMAND_BOOTKERNEL \
	"bootm ${kerneladdr}"

/* bootcmd */
#define COMMAND_BOOT \
	"run preload loadimg postload bootkernel" "\0" \
	"preload=" "true" "\0" \
	"postload=" "true" "\0" \
	"loadimg=" COMMAND_LOADIMG "\0" \
	"bootkernel=" COMMAND_BOOTKERNEL

#endif /* TELECHIPS_BOOT_FIT_CONFIG_H */
