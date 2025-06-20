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
	{ BOOT_FASTBOOTD	, "fastbootd"	},
	{ BOOT_SECUREBOOT_ENABLE, "secureboot"	},
	/* blunt */
	{ BOOT_HARD		, "hard"	},
	/* sentinel */
	{ 0, NULL }
};

const struct boot_reason_hash *get_boot_reason_hash(void)
{
	return boot_reasons;
}

void arch_reboot_init(void)
{
	struct arm_smccc_res res;
	bool corerst = core_reset_occurred();

	if (corerst) {
		arm_smccc_smc(SIP_POST_CORE_RESET, 0, 0, 0, 0, 0, 0, 0, &res);
	}

#if defined(CONFIG_USE_CORE_RESET)
	set_reboot_type(REBOOT_TYPE_CORE);
#else
	set_reboot_type(REBOOT_TYPE_COLD);
#endif
}
