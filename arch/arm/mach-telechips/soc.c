// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <mach/reboot.h>
#include <mach/smc.h>

#if !defined(CONFIG_ARM64)
/*
 * For AArch32 build, enable_caches() should be implemented in platform code.
 * For AArch64 build, follow default enable_caches() implementation.
 */
void enable_caches(void)
{
	uint32_t reg = get_cr();

	if ((reg & CR_I) == 0) {
		invalidate_icache_all();
		icache_enable();
	}

	if ((reg & CR_C) == 0) {
		invalidate_dcache_all();
		dcache_enable();
	}
}
#endif

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

#if defined(CONFIG_ARM64)
void armv8_el2_to_aarch32(u64 args, u64 mach_nr, u64 fdt_addr,
			  u64 arg4, u64 entry_point)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CORE_EL2_AARCH64_TO_32,
		      entry_point,
		      mach_nr,
		      fdt_addr,
		      0, 0, 0, 0, &res);
}
#endif

#if defined(CONFIG_TCC_MAINCORE)
#include <mach/chipinfo.h>
#include <mach/soc.h>

#define PMU_SECURE_INF02 (TCC_PMU_BASE + 0x1C4U)

void testmode_password_enable(void)
{
	const static uint8_t password[] = {
		0x4F, 0x54, 0x50, 0x32, 0x03, 0x00, 0x00, 0x00,
		0x22, 0x50, 0xFA, 0x2E, 0x32, 0x7D, 0xD0, 0x6C,
		0x69, 0x92, 0xE7, 0x6E, 0xFD, 0x1A, 0x09, 0xC2,
		0xFD, 0x43, 0x64, 0x03, 0x9D, 0xCF, 0x6C, 0x0A,
		0xD8, 0x97, 0x53, 0x2E, 0xBB, 0x59, 0x82, 0x67,
		0x35, 0xB4, 0xB6, 0x70, 0x5E, 0x81, 0x61, 0xF5,
		0xF1, 0x92, 0x6B, 0x4B, 0x1B, 0x98, 0x16, 0x80,
		0x7A, 0xB5, 0x36, 0x61, 0x21, 0x9B, 0x42, 0xB0,
		0xA4, 0x7B, 0xF3, 0x13, 0x7D, 0x80, 0xFA, 0x8A,
		0x0B, 0xBD, 0xB9, 0xAE, 0xF9, 0x11, 0xEA, 0xC7,
		0xD3, 0x1B, 0xA6, 0x38, 0x7C, 0xE9, 0x07, 0x4D,
		0x66, 0xFC, 0x9E, 0xCE, 0x65, 0xA9, 0x36, 0x18,
		0xD8, 0x37, 0xF3, 0xE6, 0xCC, 0x84, 0x17, 0x03,
		0x74, 0xFD, 0x4D, 0x5D,
	};

	const ulong pwaddr = CONFIG_SYS_LOAD_ADDR;
	void *const pwaddr_p = phys_to_virt(pwaddr);
	const u32 family = get_chip_family();

	struct arm_smccc_res res;

	if ((family == 0x8050U) || (family == 0x8030U)) {
		u32 info = readl(PMU_SECURE_INF02);

		if ((info & (u32)BIT(26)) == 0U) {
			(void)memcpy(pwaddr_p, password, sizeof(password));
			arm_smccc_smc(SIP_TCSB_OTP_WRITE, pwaddr,
				      sizeof(password), 0, 0, 0, 0, 0, &res);
		}
	}
}
#endif
