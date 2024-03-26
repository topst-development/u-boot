// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/system.h>
#include <mach/reboot.h>
#include <mach/smc.h>

void armv8_el2_to_aarch32(u64 args, u64 mach_nr, u64 fdt_addr,
			  u64 arg4, u64 entry_point)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CORE_EL2_AARCH64_TO_32,
		      entry_point, mach_nr, fdt_addr, 0, 0, 0, 0, &res);
}

void reset_misc(void)
{
	ulong ret = get_reboot_reason();

	if (ret == BOOT_HARD) {
		/* Set reboot reason as "normal" if not set before */
		set_reboot_reason(BOOT_NORMAL);
	}

	dcache_disable();
	icache_disable();
}
