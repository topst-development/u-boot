// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <mach/chipinfo.h>

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
#if !defined(CONFIG_BOOT_SUBCORE_BY_MAINCORE)
		/* Add dummy timestamp for "reset subcore" stage */
		add_boot_time_stamp_dummy();
#endif
		add_boot_time_stamp();
		break;
	default:
		/* We only cares 6 bootstages above */
		break;
	}
}
