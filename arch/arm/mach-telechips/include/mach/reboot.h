/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef MACH_REBOOT_H
#define MACH_REBOOT_H

#include <asm/arch/boot-mode.h>
#include <linux/types.h>

#define FLAG_NOT_SET (ULONG_MAX)

struct boot_reason_hash {
	ulong id;
	const char *const name;
};

const struct boot_reason_hash *get_boot_reason_hash(void);

void arch_reboot_init(void);
void reboot_init(void);
void reboot_set_env(void);

ulong get_core_reset_flag(void);
bool core_reset_occurred(void);

ulong get_reboot_reason(void);
void set_reboot_reason(ulong id);

ulong get_reboot_type(void);
void set_reboot_type(ulong type);

ulong get_boot_reason(void);
const char *read_boot_reason(void);

#endif /* MACH_REBOOT_H */
