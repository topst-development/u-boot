/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCC807X_CONFIG_H
#define TCC807X_CONFIG_H

/*
 * Address Map (DRAM: Up to 16 GiB)
 */

#define DEVICE_BASE		(0x10000000U)
#define DEVICE_SIZE		(0x10000000U)

#define DRAM_BASE		(0x20000000U)
#define DRAM_SIZE		(0xA0000000U)
#define DRAM_BASE_EXT		(0x1A0000000U)
#define DRAM_SIZE_EXT		(0x360000000U)

/*
 * Image Load Address & Partition
 */

#define MAINCORE_ADDR_KERNEL	0x24000000
#define MAINCORE_ADDR_FDT	0x28000000

#define MAINCORE_PART_KERNEL	"boot"
#define MAINCORE_PART_FDT	"dtb"
#define MAINCORE_PART_SYSTEM	"system"
#define MAINCORE_PART_MISC	"misc"

#define SUBCORE_ADDR_KERNEL	0x44000000
#define SUBCORE_ADDR_FDT	0x48000000

#define SUBCORE_PART_KERNEL	"subcore_boot"
#define SUBCORE_PART_FDT	"subcore_dtb"
#define SUBCORE_PART_SYSTEM	"subcore_root"
#define SUBCORE_PART_MISC	"subcore_misc"

#if defined(CONFIG_USE_MAINCORE)
#define ADDR_KERNEL		MAINCORE_ADDR_KERNEL
#define ADDR_FDT		MAINCORE_ADDR_FDT

#define PART_KERNEL		MAINCORE_PART_KERNEL
#define PART_FDT		MAINCORE_PART_FDT
#define PART_SYSTEM		MAINCORE_PART_SYSTEM
#define PART_MISC		MAINCORE_PART_MISC
#else
#define ADDR_KERNEL		SUBCORE_ADDR_KERNEL
#define ADDR_FDT		SUBCORE_ADDR_FDT

#define PART_KERNEL		SUBCORE_PART_KERNEL
#define PART_FDT		SUBCORE_PART_FDT
#define PART_SYSTEM		SUBCORE_PART_SYSTEM
#define PART_MISC		SUBCORE_PART_MISC
#endif

#define ARCH_EXTRA_ENV_SETTINGS \
	"bootargs=" "kvm-arm.mode=nvhe" "\0"

#include "telechips/common.h"
#include "telechips/boot.h"
#include "telechips/config_fallbacks.h"
#include "telechips/display.h"
#endif	/* TCC807X_CONFIG_H */
