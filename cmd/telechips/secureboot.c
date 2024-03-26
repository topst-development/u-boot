// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <command.h>
#include <common.h>
#include <console.h>
#include <mach/reboot.h>
#include <mach/secureboot.h>

static int do_secureboot_enable(struct cmd_tbl *cmdtp, int flag, int argc,
				char * const argv[])
{
	int ret;
	int simmode = 0;
	int reboot = 1;
	int i;

	for (i = 1; i < argc; i++) {
		if (strncmp(argv[i], "-d", 2) == 0) {
			simmode = 1;
			pr_force("OTP Writing will be omited for simulation\n");
		} else if (strncmp(argv[i], "-n", 2) == 0) {
			reboot = 0;
			pr_force("No reboot after secureboot enable\n");
		}
	}

	ret = tc_secureboot_enable(simmode, reboot);
	if (ret != 0) {
		if (ret == -EINVAL) {
			pr_force("Secure boot is already enabled.\n");
#if !defined(CONFIG_TCC807X)
		} else if (ret == -EACCES) {
			if (simmode == 0) {
				pr_force("Reboot and Encrypt images in SNOR for secure boot enable by MCU...\n");
				set_reboot_reason(BOOT_SECUREBOOT_ENABLE);
			} else {
				pr_force("Reboot but secure boot is not enabled\n");
				pr_force("Because simulation is not supported on SNOR\n");
			}
			set_reboot_type(REBOOT_TYPE_COLD);
			do_reset(NULL, 0, 0, NULL);
#endif
		} else {
			pr_force("Failed to enable secure boot (%d)\n", ret);
		}
		ret = CMD_RET_FAILURE;
	}

	return ret;
}

U_BOOT_CMD(
	secureboot_enable, 3, 0, do_secureboot_enable,
	"secureboot_enable [-d] [-n]",
	""
);
