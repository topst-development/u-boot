// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <mach/chipinfo.h>

void show_boot_progress(int val)
{
	switch ((enum bootstage_id)val) {
	case BOOTSTAGE_ID_START_UBOOT_F:
	case BOOTSTAGE_ID_RELOCATION:
	case BOOTSTAGE_ID_START_UBOOT_R:
	case BOOTSTAGE_ID_BOOTM_START:
	case BOOTSTAGE_ID_BOOTM_HANDOFF:
	case BOOTSTAGE_ID_MAIN_LOOP:
		add_boot_time_stamp();
		break;
	default:
		/* We only cares 6 bootstages above */
		break;
	}
}

#if defined(CONFIG_MULTI_DTB_FIT)
#define MAX_FDT_NAME 30U

static u32 get_board_rev(void)
{
	void *gpmd_ien = phys_to_virt(TCC_GPIO_BASE + 0x7A4U);
	void *gpmd_dat = phys_to_virt(TCC_GPIO_BASE + 0x780U);

	/* GPMDIEN: Enable input buffer for GPMD09 */
	generic_set_bit(9, gpmd_ien);

	/* GPMDDAT: Read data for GPMD09 */
	return (readl(gpmd_dat) >> 9U) & 0x1U;
}

static void get_board_fdt_name(char *buf)
{
	u32 chip_name, board_rev;
	const char *core_type;

	chip_name = get_chip_name();
	board_rev = get_board_rev();

#if defined(CONFIG_TCC_SUBCORE)
	core_type = "-subcore";
#else
	core_type = "";
#endif

	(void)snprintf(buf, MAX_FDT_NAME, "tcc%04x%s-evb_%s",
		       chip_name & 0xFFFFU,
		       core_type,
		       (board_rev == 1U) ? "sv1.0" : "sv0.1");
}

int board_fit_config_name_match(const char *name)
{
	static char fdt_name[MAX_FDT_NAME] = "xxx";
	static s32 first_try = 1;

	if (first_try == 1) {
		get_board_fdt_name(fdt_name);
		first_try = 0;
	}

	return strncmp(fdt_name, name, sizeof(fdt_name));
}
#endif
