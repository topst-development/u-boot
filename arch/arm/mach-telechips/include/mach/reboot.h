/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_REBOOT_H
#define TCC_REBOOT_H

#include <asm/telechips/boot-mode.h>
#include <linux/types.h>

#define FLAG_NOT_SET (ULONG_MAX)

/**
 * struct boot_reason_hash - Hashmap for boot reason and its unique ID
 * @id: Unique ID to indicate a boot reason
 * @name: Human-readable boot reason in string
 */
struct boot_reason_hash {
	ulong id;
	const char *name;
};

/**
 * reboot_init() - Initialize data to use for below functions
 *
 * This function must be called **only once** and before calling other
 * functions below. Reboot reason is set as "hard", meaning cold reset
 * by unspecified reason, at the end of the function for the next boot.
 */
void reboot_init(void);

/**
 * reboot_set_env() - Set U-Boot environment variable for reboot data
 *
 * This function sets env "corerst" which can be 1 or 0 if core reset
 * was occurred or not, and "boot_reason" which contains boot reason in
 * string.
 */
void reboot_set_env(void);

/**
 * core_reset_occurred() - Check if core reset has occurred
 *
 * Return:
 * * true  - Current boot is triggered by core reset
 * * false - Current boot is not triggered by core reset
 */
bool core_reset_occurred(void);

/**
 * get_reboot_reason() - Get reboot resaon ID set for next reboot
 *
 * Return:
 * One of boot modes defined in asm/telechips/boot-mode.h
 */
ulong get_reboot_reason(void);

/**
 * set_reboot_reason() - Set reboot reason ID for next reboot
 * @mode: One of boot modes defined in asm/telechips/boot-mode.h
 */
void set_reboot_reason(ulong id);

/**
 * get_reboot_type() - Get reboot type set for next reboot
 *
 * Return:
 * One of reboot types defined in asm/telechips/boot-mode.h
 */
ulong get_reboot_type(void);

/**
 * set_reboot_type() - Set reboot type for next reboot
 * @type: One of reboot types defined in asm/telechips/boot-mode.h
 */
void set_reboot_type(ulong type);

/**
 * get_boot_reason() - Get current boot reason ID
 *
 * Return:
 * One of reason IDs listed on boot_reasons[] array
 */
ulong get_boot_reason(void);

/**
 * read_boot_reason() - Get current boot reason in string
 *
 * Return:
 * One of reasons in string listed on reboot_reasons[] array
 */
const char *read_boot_reason(void);

#endif /* TCC_REBOOT_H */
