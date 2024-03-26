// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <mach/reboot.h>
#include <mach/smc.h>

static struct boot_reason_hash boot_reasons[] = {
	{ BOOT_COLD		, "cold"	},
	/* abnormal reboot */
	{ BOOT_WATCHDOG		, "watchdog"	},
	{ BOOT_PANIC		, "panic"	},
	{ BOOT_TEEOS		, "teeos"	},
	/* normal reboot*/
	{ BOOT_FASTBOOT		, "fastboot"	},
	{ BOOT_NORMAL		, "normal"	},
	{ BOOT_RECOVERY		, "recovery"	},
	{ BOOT_TCUPDATE		, "tcupdate"	},
	{ BOOT_DDR_CHECK	, "ddrcheck"	},
	{ BOOT_FASTBOOTD	, "fastbootd"	},
	{ BOOT_SECUREBOOT_ENABLE, "secureboot"	},
	/* boot failure */
	{ BOOT_FAIL_ON_RESUME	, "fail,resume"	},
	/* blunt */
	{ BOOT_HARD		, "hard"	},
	/* sentinel */
	{ 0, NULL }
};

const struct boot_reason_hash *get_boot_reason_hash(void)
{
	return boot_reasons;
}
