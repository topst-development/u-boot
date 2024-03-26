/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCC803X_CONFIG_H
#define TCC803X_CONFIG_H

/*
 * Address Map (DRAM: Up to 4 GiB)
 */

#define DEVICE_BASE		(0x10000000U)
#define DEVICE_SIZE		(0x10000000U)

#define DRAM_BASE		(0x20000000U)
#define DRAM_SIZE		(0xA0000000U)
#define DRAM_BASE_EXT		(0x1A0000000U)
#define DRAM_SIZE_EXT		(0x060000000U)

/*
 * Image Load Address & Partition
 */

#define MAINCORE_ADDR_KERNEL	0x24000000
#define MAINCORE_ADDR_FDT	0x28000000

#define MAINCORE_PART_KERNEL	"boot"
#define MAINCORE_PART_FDT	"dtb"
#define MAINCORE_PART_SYSTEM	"system"
#define MAINCORE_PART_MISC	"misc"

#define SUBCORE_ADDR_KERNEL	0xA0000000
#define SUBCORE_ADDR_FDT	0xA0A00000
#define SUBCORE_ADDR_SYSTEM	0xA0B00000

#define SUBCORE_SIZE_KERNEL	0x00A00000
#define SUBCORE_SIZE_FDT	0x00100000
#define SUBCORE_SIZE_SYSTEM	0x0F500000

#define SUBCORE_TARGET_KERNEL	0x80008000
#define SUBCORE_TARGET_FDT	0x83000000
#define SUBCORE_TARGET_SYSTEM	0x87000000

#define SUBCORE_PART_KERNEL	"a7s_boot"
#define SUBCORE_PART_FDT	"a7s_dtb"
#define SUBCORE_PART_SYSTEM	"a7s_root"

#define ADDR_KERNEL		MAINCORE_ADDR_KERNEL
#define ADDR_FDT		MAINCORE_ADDR_FDT

#define PART_KERNEL		MAINCORE_PART_KERNEL
#define PART_FDT		MAINCORE_PART_FDT
#define PART_SYSTEM		MAINCORE_PART_SYSTEM
#define PART_MISC		MAINCORE_PART_MISC

/*
 * Arch-specific Commands
 */

#if defined(CONFIG_BOOT_SUBCORE_BY_MAINCORE)
/* sc_loadimg */
#define COMMAND_SUBCORE_LOADIMG \
	"blkread part ${sc_bootpart} ${slot} ${sc_kerneladdr} sc_kernel_size; " \
	"blkread part ${sc_rootpart} ${slot} ${sc_root_addr} sc_root_size"

/* sc_loadfdt */
#define COMMAND_SUBCORE_LOADFDT \
	"blkread fdt ${sc_fdtpart} ${slot} ${sc_fdt_addr}"

/* sc_setupfdt */
#define COMMAND_SUBCORE_SETUPFDT \
	"fdt addr ${sc_fdt_addr}; fdt header get sc_fdt_size totalsize"

/* sc_bootkernel */
#define COMMAND_SUBCORE_BOOTKERNEL \
	"prepare_subcore ${sc_kernel_size} ${sc_fdt_size} ${sc_root_size}; " \
	"boot_subcore 0; boot_subcore 1"

/* sc_bootcmd */
#define COMMAND_SUBCORE_BOOT \
	"if test ${corerst} = 0; then " \
		"run sc_loadimg sc_loadfdt sc_setupfdt sc_bootkernel; " \
	"fi; " "\0" \
	"sc_bootpart=" __stringify(SUBCORE_PART_KERNEL) "\0" \
	"sc_fdtpart=" __stringify(SUBCORE_PART_FDT) "\0" \
	"sc_rootpart=" __stringify(SUBCORE_PART_SYSTEM) "\0" \
	"sc_kerneladdr=" __stringify(SUBCORE_ADDR_KERNEL) "\0" \
	"sc_fdt_addr=" __stringify(SUBCORE_ADDR_FDT) "\0" \
	"sc_root_addr=" __stringify(SUBCORE_ADDR_SYSTEM) "\0" \
	"sc_loadimg=" COMMAND_SUBCORE_LOADIMG "\0" \
	"sc_loadfdt=" COMMAND_SUBCORE_LOADFDT "\0" \
	"sc_setupfdt=" COMMAND_SUBCORE_SETUPFDT "\0" \
	"sc_bootkernel=" COMMAND_SUBCORE_BOOTKERNEL

#define ARCH_EXTRA_ENV_SETTINGS \
	"preload=" "run sc_bootcmd" "\0" \
	"sc_bootcmd=" COMMAND_SUBCORE_BOOT "\0"
#endif

#include "telechips/common.h"
#include "telechips/boot.h"
#include "telechips/config_fallbacks.h"
#include "telechips/display.h"

#endif	/* TCC803X_CONFIG_H */
