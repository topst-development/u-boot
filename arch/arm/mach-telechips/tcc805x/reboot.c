// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <linux/arm-smccc.h>
#include <linux/io.h>
#include <mach/reboot.h>
#include <mach/smc.h>

static const struct boot_reason_hash boot_reasons[] = {
	{ BOOT_COLD,		 "cold"		},
	{ BOOT_WATCHDOG,	 "watchdog"	},
	{ BOOT_PANIC,		 "panic"	},
	{ BOOT_FASTBOOT,	 "fastboot"	},
	{ BOOT_NORMAL,		 "normal"	},
	{ BOOT_RECOVERY,	 "recovery"	},
	{ BOOT_DDR_CHECK,	 "ddrcheck"	},
	{ BOOT_FASTBOOTD,	 "fastbootd"	},
	{ BOOT_FAIL_ON_RESUME,	 "fail,resume"	},
	{ BOOT_HARD,		 "hard"		},
};

static ulong corerst = FLAG_NOT_SET;
static ulong boot_reason = FLAG_NOT_SET;

#define PMU_RST_STS2 (TCC_PMU_BASE + 0x16C)

void reboot_init(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_GET_CORE_RESET_FLAG, 0, 0, 0, 0, 0, 0, 0, &res);
	corerst = res.a0;

	boot_reason = get_reboot_reason();
	set_reboot_reason(BOOT_HARD);
}

void reboot_set_env(void)
{
	const char *boot_reason_str = read_boot_reason();

	(void)env_set_ulong("corerst", corerst);
	(void)env_set("boot_reason", boot_reason_str);
}

bool core_reset_occurred(void)
{
	return (corerst != 0U);
}

ulong get_reboot_reason(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_GET_RESET_REASON, 0, 0, 0, 0, 0, 0, 0, &res);

	return res.a0;
}

void set_reboot_reason(ulong id)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_SET_RESET_REASON, id, 0, 0, 0, 0, 0, 0, &res);
}

ulong get_reboot_type(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_GET_RESET_TYPE, 0, 0, 0, 0, 0, 0, 0, &res);

	return res.a0;
}

void set_reboot_type(ulong type)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_SET_RESET_TYPE, type, 0, 0, 0, 0, 0, 0, &res);
}

ulong get_boot_reason(void)
{
	return boot_reason;
}

const char *read_boot_reason(void)
{
	const char *reason_str = boot_reasons[0].name;
	u32 i;

	for (i = 0; i < ARRAY_SIZE(boot_reasons); i++) {
		if (boot_reason == boot_reasons[i].id) {
			reason_str = boot_reasons[i].name;
			break;
		}
	}

	return reason_str;
}
