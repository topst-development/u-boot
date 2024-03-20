// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef GPU_H
#define GPU_H

typedef enum {
	CORE_SUB	= 10,
	CORE_MAIN	= 20
}CORE_ID;

/* Firmware's connection state */
typedef enum
{
	RGXFW_CONNECTION_FW_OFFLINE = 0,    /*!< Firmware is offline */
	RGXFW_CONNECTION_FW_READY,          /*!< Firmware is initialised */
	RGXFW_CONNECTION_FW_ACTIVE,         /*!< Firmware connection is fully established */
	RGXFW_CONNECTION_FW_OFFLOADING,     /*!< Firmware is clearing up connection data */
	RGXFW_CONNECTION_FW_STATE_COUNT
} RGXFWIF_CONNECTION_FW_STATE;

#define RGX_VIRTUALIZATION_REG_BASE		(0x10100000)
#define RGX_VIRTUALISATION_REG_SIZE_PER_OS		(0x10000)
#define RGX_VIRTUALISATION_REG_OFFSET_FOR_BACKUP		(7 * RGX_VIRTUALISATION_REG_SIZE_PER_OS)

#define RGX_CR_OS0_SCRATCH0		(0x0880)
#define RGX_CR_OS0_SCRATCH1		(0x0888)
#define RGX_CR_OS0_SCRATCH2		(0x0890)
#define RGX_CR_OS0_SCRATCH3		(0x0898)

/* OS7's register are used; but use the definitions of OS0, to obtain the register offsets within a register bank */
#define SUBCORE_FLAG_REG         RGX_CR_OS0_SCRATCH0
#define MAINCORE_FLAG_REG        RGX_CR_OS0_SCRATCH1
#define TURN_REG                 RGX_CR_OS0_SCRATCH2
#define HOST_CORE_ID_REG         RGX_CR_OS0_SCRATCH3

#define GPU_VZ_WATCHDOG_INTERVAL		(300)
#define GPU_VZ_WATCHDOG_TIMEOUT		(3 * GPU_VZ_WATCHDOG_INTERVAL)

#ifdef CONFIG_TCC_MAINCORE
#define GPU_CORE_ID		(CORE_MAIN)
#else
#define GPU_CORE_ID		(CORE_SUB)
#endif

void gpu_init(void);

#endif
