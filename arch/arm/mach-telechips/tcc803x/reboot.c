// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <linux/arm-smccc.h>
#include <linux/io.h>
#include <mach/reboot.h>
#include <mach/smc.h>

#if defined(CONFIG_TCC_CORE_RESET)
#  define REBOOT_TYPE_DEFAULT	(REBOOT_TYPE_CORE)
#else
#  define REBOOT_TYPE_DEFAULT	(REBOOT_TYPE_COLD)
#endif

#define REBOOT_SUB_TYPE_RESTORE_SUBCORE_IMAGE_BY_MAINCORE (0x10)
#define REBOOT_SUB_TYPE_RESTORE_SUBCORE_IMAGE_BY_SUBCORE (0x20)

static const struct boot_reason_hash boot_reasons[] = {
	{ BOOT_COLD,		 "cold"		},
	{ BOOT_WATCHDOG,	 "watchdog"	},
	{ BOOT_PANIC,		 "panic"	},
	{ BOOT_FASTBOOT,	 "fastboot"	},
	{ BOOT_NORMAL,		 "normal"	},
	{ BOOT_RECOVERY,	 "recovery"	},
	{ BOOT_FASTBOOTD,	 "fastbootd"	},
	{ BOOT_HARD,		 "hard"		},
};

static ulong corerst = FLAG_NOT_SET;
static ulong boot_reason = FLAG_NOT_SET;
static ulong reboot_type = FLAG_NOT_SET;

void reboot_init(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_GET_CORE_RESET_FLAG, 0, 0, 0, 0, 0, 0, 0, &res);
	corerst = res.a0;

	if (corerst == 1UL) {
		arm_smccc_smc(SIP_POST_CORE_RESET, 0, 0, 0, 0, 0, 0, 0, &res);
	}

	boot_reason = get_reboot_reason();
	set_reboot_reason(BOOT_HARD);

	set_reboot_type(REBOOT_TYPE_DEFAULT);
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
	return reboot_type;
}

void set_reboot_type(ulong type)
{
	struct arm_smccc_res res;

#if defined(CONFIG_RESTORE_SUBCORE_IMAGE_BY_MAINCORE)
	type |= REBOOT_SUB_TYPE_RESTORE_SUBCORE_IMAGE_BY_MAINCORE;
#elif defined(CONFIG_BOOT_SUBCORE_SELF_LOAD_MODE)
	type |= REBOOT_SUB_TYPE_RESTORE_SUBCORE_IMAGE_BY_SUBCORE;
#endif
	reboot_type = type;

	arm_smccc_smc(SIP_PREPARE_CORE_RESET, type, 0, 0, 0, 0, 0, 0, &res);
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
