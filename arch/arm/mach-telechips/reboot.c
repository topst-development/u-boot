// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <env.h>
#include <mach/reboot.h>
#include <mach/smc.h>

void reboot_init(void)
{
#if defined(CONFIG_ARCH_REBOOT_INIT)
	arch_reboot_init();
#endif
	set_reboot_reason(BOOT_HARD);
}

void reboot_set_env(void)
{
	ulong corerst_flag = get_core_reset_flag();
	const char *boot_reason_str = read_boot_reason();

	(void)env_set_ulong("corerst", corerst_flag);
	(void)env_set("boot_reason", boot_reason_str);
}

ulong get_core_reset_flag(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_GET_CORE_RESET_FLAG, 0, 0, 0, 0, 0, 0, 0, &res);

	return res.a0;
}

bool core_reset_occurred(void)
{
	ulong flag = get_core_reset_flag();

	return (flag != 0U);
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
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_GET_BOOT_REASON, 0, 0, 0, 0, 0, 0, 0, &res);

	return res.a0;
}

const char *read_boot_reason(void)
{
	const struct boot_reason_hash *reasons = get_boot_reason_hash();
	ulong boot_reason = get_boot_reason();
	const char *reason_str = reasons->name;

	for ( ; reasons->name != NULL; reasons++) {
		if (boot_reason == reasons->id) {
			reason_str = reasons->name;
			break;
		}
	}

	return reason_str;
}
