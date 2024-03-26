/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCC750X_BOOT_MODE_H
#define TCC750X_BOOT_MODE_H

#define BOOT_MODE_MASK		(0x7FUL)

#define BOOT_COLD		(0x00UL)

/* abnormal reboot */
#define BOOT_WATCHDOG		(0x01UL)
#define BOOT_PANIC		(0x02UL)
#define BOOT_TEEOS		(0x03UL)

/* normal reboot */
#define BOOT_FASTBOOT		(0x50UL)
#define BOOT_NORMAL		(0x51UL)
#define BOOT_RECOVERY		(0x52UL)
#define BOOT_TCUPDATE		(0x53UL)
#define BOOT_DDR_CHECK		(0x54UL)
#define BOOT_FASTBOOTD		(0x55UL)
#define BOOT_SECUREBOOT_ENABLE	(0x56UL)

/* blunt */
#define BOOT_HARD		(0x70UL)

/* reboot types */
#define REBOOT_TYPE_CORE	(0x00UL)
#define REBOOT_TYPE_COLD	(0x01UL)

#endif /* TCC750X_BOOT_MODE_H */
