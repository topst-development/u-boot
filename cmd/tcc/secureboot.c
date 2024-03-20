// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#include <common.h>
#include <console.h>
#include <mach/tcc_secureboot.h>

static int do_secureboot_enable(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	if (tc_secureboot_enable() == 0) {
		return 0;
	}
	return -1;
}

U_BOOT_CMD(
	secureboot_enable, 1, 1, do_secureboot_enable,
	"secureboot_enable",
	""
);
