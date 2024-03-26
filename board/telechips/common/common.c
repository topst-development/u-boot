// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <bootstage.h>
#include <fastboot.h>
#include <dm/uclass.h>
#include <mach/board.h>
#include <mach/chipinfo.h>
#include <mach/reboot.h>

static void pr_uclass_probe_err(enum uclass_id id, int ret)
{
	const char *name;

	name = uclass_get_name(id);
	pr_err("Failed to probe %s devices: %d\n", name, ret);
}

void uclass_probe(enum uclass_id id)
{
	int ret;

	ret = uclass_probe_all(id);
	if (ret != 0) {
		pr_uclass_probe_err(id, ret);
	}
}

void uclass_misc_probe(const char *name)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_MISC, name, &dev);
	if (ret != 0) {
		pr_uclass_probe_err(UCLASS_MISC, ret);
	}
}

#if CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
void show_boot_progress(int val)
{
	switch ((enum bootstage_id)val) {
	case BOOTSTAGE_ID_START_UBOOT_F:
	case BOOTSTAGE_ID_RELOCATION:
	case BOOTSTAGE_ID_START_UBOOT_R:
	case BOOTSTAGE_ID_BOOTM_HANDOFF:
	case BOOTSTAGE_ID_MAIN_LOOP:
		add_boot_time_stamp();
		break;
	case BOOTSTAGE_ID_BOOTM_START:
#if defined(CONFIG_ARCH_SUPPORT_BOOT_SUBCORE_BY_MAINCORE) && \
    !defined(CONFIG_BOOT_SUBCORE_BY_MAINCORE)
		/* Add dummy timestamp for subcore boot event */
		add_boot_time_stamp_dummy();
#endif
		add_boot_time_stamp();
		break;
	default:
		/* We only cares 6 bootstages above */
		break;
	}
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

	/* BOOT_MODE_MASK is used for indicating unknown reason */
	if (reboot_reason != BOOT_MODE_MASK) {
		set_reboot_reason(reboot_reason);
		ret = 0;
	}

	return ret;
}
#endif
