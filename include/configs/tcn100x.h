/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCN100X_CONFIG_H
#define TCN100X_CONFIG_H

/*
 * Address Map (DRAM: Up to 8 GiB)
 */

#define DEVICE_BASE		(0x40000000U)
#define DEVICE_SIZE		(0x10000000U)

#define DRAM_BASE		(0x100000000U)
#define DRAM_SIZE		(0x200000000U)
#define DRAM_BASE_EXT		(0x0)
#define DRAM_SIZE_EXT		(0x0)

/*
 * Image Load Address & Partition
 */

#define MAINCORE_ADDR_KERNEL	0x184000000
#define MAINCORE_ADDR_FDT	0x188000000

#define MAINCORE_PART_KERNEL	"boot"
#define MAINCORE_PART_FDT	"dtb"
#define MAINCORE_PART_SYSTEM	"system"
#define MAINCORE_PART_MISC	"misc"

#define ADDR_KERNEL		MAINCORE_ADDR_KERNEL
#define ADDR_FDT		MAINCORE_ADDR_FDT

#define PART_KERNEL		MAINCORE_PART_KERNEL
#define PART_FDT		MAINCORE_PART_FDT
#define PART_SYSTEM		MAINCORE_PART_SYSTEM
#define PART_MISC		MAINCORE_PART_MISC

/*
 * Arch-specific Environments
 */

#define ARCH_EXTRA_ENV_SETTINGS \
	"bootargs=" "kvm-arm.mode=nvhe" "\0"

#include "telechips/common.h"
#include "telechips/boot.h"
#include "telechips/config_fallbacks.h"

#endif	/* TCN100X_CONFIG_H */
