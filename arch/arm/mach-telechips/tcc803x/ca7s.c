// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/arch/ca7s.h>
#include <asm/arch/clock.h>
#include <android_image.h>
#include <linux/arm-smccc.h>
#include <linux/io.h>
#include <mach/chipinfo.h>
#include <mach/smc.h>
#include <auth_mod.h>
#include <image.h>

s32 run_ca7s_firmware(u32 start)
{
	struct tc_img_hdr *hdr;
	struct arm_smccc_res res;
	s32 ret = 0;
	ulong img_addr;
	u32 img_size;

	img_addr = SUBCORE_KERNEL_ADDR;
	hdr = phys_to_virt(img_addr);
	img_size = hdr->img_size;

	switch (start) {
	case 0U:
		arm_smccc_smc(SIP_CORE_END_SUBCORE, 0, 0, 0, 0, 0, 0, 0, &res);
		break;
	case 1U:

#ifdef CONFIG_BOOT_SUBCORE_SELF_LOAD_MODE
		add_boot_time_stamp();
		arm_smccc_smc(SIP_CORE_PWUP, 0, 0, 0, 0, 0, 0, 0, &res);
#else
		ret = tc_verify_img((uintptr_t)img_addr);
		if (ret == VERIFY_FAIL) {
			break;
		} else if (ret == VERIFY_OK) {
			(void)memcpy(img_addr,
				     img_addr + IMAGE_CERT_SIZE,
				     img_size);
			flush_dcache_range(img_addr, img_addr + img_size +
					   IMAGE_CERT_SIZE);
		}
		add_boot_time_stamp();
		arm_smccc_smc(SIP_CORE_START_SUBCORE,
			      img_addr,
			      SUBCORE_FDT_ADDR,
			      0, 0, 0, 0, 0, &res);
#endif
		/* Configure A7S CPU clock aftbr resetting A7S */
		(void)tcc_set_clkctrl(FBUS_CPU1, true, 1100000000UL);
		break;
	default:
		ret = 1;
		break;
	}

	return ret;
}
