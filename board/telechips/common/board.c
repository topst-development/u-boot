// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <mach/reboot.h>

#if defined(CONFIG_FASTBOOT)
#  include <fastboot.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

ulong board_get_usable_ram_top(ulong total_size)
{
	s32 cfg = fdt_path_offset(gd->fdt_blob, "/config");
	u32 size;
	u32 ramtop;

	size = fdtdec_get_uint(gd->fdt_blob, cfg, "u-boot,reloc-off", SZ_256M);

	if ((UINT_MAX - (u32)CONFIG_SYS_TEXT_BASE) < size) {
		/* Use default offset in case the size is too big */
		size = SZ_256M;
	}
	ramtop = (u32)CONFIG_SYS_TEXT_BASE + size;

	return (ulong)ramtop;
}

#if defined(CONFIG_ARM64)
u8 tcc_image_get_arch(void)
{
	const char *s = env_get("kernelarch");
	u8 ret = IH_ARCH_ARM64;

	if (s != NULL) {
		ulong arch = simple_strtoul(s, NULL, 16);

		ret = (arch == 0x32UL) ? (u8)IH_ARCH_ARM : (u8)IH_ARCH_ARM64;
	}

	return ret;
}
#endif

#if defined(CONFIG_FASTBOOT)
int fastboot_set_reboot_flag(enum fastboot_reboot_reason reason)
{
	ulong reboot_reason;
	s32 ret = -ENOTSUPP;

	switch (reason) {
	case FASTBOOT_REBOOT_REASON_BOOTLOADER:
		reboot_reason = BOOT_FASTBOOT;
		break;
	case FASTBOOT_REBOOT_REASON_FASTBOOTD:
		reboot_reason = BOOT_FASTBOOTD;
		break;
	case FASTBOOT_REBOOT_REASON_RECOVERY:
		reboot_reason = BOOT_RECOVERY;
		break;
	default:
		reboot_reason = BOOT_MODE_MASK;
		break;
	}

	/* BOOT_MODE_MASK is used for indicating 'default' case */
	if (reboot_reason != BOOT_MODE_MASK) {
		set_reboot_reason(reboot_reason);
		ret = 0;
	}

	return ret;
}
#endif
