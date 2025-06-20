// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <android_image.h>
#include <cpu_func.h>
#include <image.h>
#include <asm/arch/ca7s.h>
#include <linux/io.h>
#include <mach/chipinfo.h>
#include <mach/clock.h>
#include <mach/smc.h>

int run_ca7s_firmware(uint32_t start)
{
	struct arm_smccc_res res;
	int ret = 0;
	void *ap_irqo_en;

	switch (start) {
	case 0U:
		/* Disable CMB_MBOX0 interrupts to Cortex-A7S */
		ap_irqo_en = phys_to_virt(0x1460000C);
		writel(0xFFFFFBFFU, ap_irqo_en);

		/* Disable mali0, gc300, g2d interrupts to Cortex-A7S */
		ap_irqo_en = phys_to_virt(0x14600018);
		writel(0xFFFFFFC0U, ap_irqo_en);

		arm_smccc_smc(SIP_CORE_END_SUBCORE, 0, 0, 0, 0, 0, 0, 0, &res);
		break;
	case 1U:
		add_boot_time_stamp();
		arm_smccc_smc(SIP_CORE_PWUP, 0, 0, 0, 0, 0, 0, 0, &res);

		/* Configure A7S CPU clock aftbr resetting A7S */
		(void)tcc_set_clkctrl(FBUS_CPU1, CKC_ENABLE, 1100000000UL, 0UL);
		break;
	default:
		ret = 1;
		break;
	}

	return ret;
}
