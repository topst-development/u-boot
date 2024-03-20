/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC805X_BOOT_MODE_H
#define TCC805X_BOOT_MODE_H

#define BOOT_MODE_MASK		(0x7FUL)

#define BOOT_COLD		(0x00UL)

/* Reboot by issue */
#define BOOT_WATCHDOG		(0x01UL)
#define BOOT_PANIC		(0x02UL)

/* Reboot by user request */
#define BOOT_FASTBOOT		(0x50UL)
#define BOOT_NORMAL		(0x51UL)
#define BOOT_RECOVERY		(0x52UL)
#define BOOT_TCUPDATE		(0x53UL)
#define BOOT_DDR_CHECK		(0x54UL)
#define BOOT_FASTBOOTD		(0x55UL)
#define BOOT_SECUREBOOT_ENABLE	(0x56UL)

/* Boot issue */
#define BOOT_FAIL_ON_RESUME	(0x60UL)

/* Blunt set */
#define BOOT_HARD		(0x70UL)

/* Reboot Types */
#define REBOOT_TYPE_CORE	(0x00UL)
#define REBOOT_TYPE_WARM	(0x02UL)
#define REBOOT_TYPE_COLD	(0x03UL)

#endif /* TCC805X_BOOT_MODE_H */
