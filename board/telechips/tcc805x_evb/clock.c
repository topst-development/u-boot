// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/telechips/tcc_ckc.h>
#include <asm/io.h>
#include <linux/arm-smccc.h>
#include <mach/clock.h>
#include <mach/smc.h>
#include <mach/chipinfo.h>

#define VPLL_DIV(x, y)	((unsigned long)((unsigned long)(y) << (unsigned long)8) | (unsigned long)(x))

void clock_init(void)
{
#if defined(CONFIG_TCC_MAINCORE)
        u32 name = get_chip_name ();
	u32 rev = get_chip_rev(); /* ES:0x00, CS:0x01, BX:0x02 */
	struct clkctrl {
		unsigned long channel;
		unsigned long rate;
	};
	struct clkctrl initial_clkctrl[12] = {
		{FBUS_CBUS,	400000000},
		{FBUS_HSIO,	250000000}, /* 333MHz. set 250MHz for GMAC */
		{FBUS_SMU,	200000000},
		{FBUS_DDI,	620000000},
		{FBUS_GPU,	600000000},
		{FBUS_G2D,	700000000},
		{FBUS_CODA,	700000000},
		{FBUS_CHEVCDEC,	780000000},
		{FBUS_BHEVCDEC,	520000000},
		{FBUS_CHEVCENC,	780000000},
		{FBUS_BHEVCENC,	520000000},
		{FBUS_VBUS,	700000000},
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(initial_clkctrl); i++) {
		struct clkctrl *ctrl = &initial_clkctrl[i];
		if(((name == 0x8059) || (name == 0x8053)) &&
				(ctrl->channel == FBUS_GPU)) {
                       ctrl->rate = 420000000;
		}

                if(((name == 0x8050) && (rev ==0x2)) &&
                                (ctrl->channel == FBUS_GPU)) {
                       ctrl->rate = 700000000;
		}

		(void)tcc_set_clkctrl(ctrl->channel, CKC_ENABLE, ctrl->rate);
	}
#endif
}

void clock_init_early(void)
{
	struct arm_smccc_res res;

	/* Initialize all peripheral clocks(disable). */
	arm_smccc_smc(SIP_CLK_INIT, 0, 0, 0, 0, 0, 0, 0, &res);

#if defined(CONFIG_TCC_MAINCORE)
	/* change clock source to XIN before change PLL values. */
	(void)tcc_set_clkctrl(FBUS_IO, CKC_ENABLE, XIN_CLK_RATE);
	(void)tcc_set_clkctrl(FBUS_SMU,CKC_ENABLE, XIN_CLK_RATE);
	(void)tcc_set_clkctrl(FBUS_HSIO, CKC_ENABLE, XIN_CLK_RATE);

	/* Modifying frequency of following PLLs(PLL0 ~ PLL4) will affect system
	 * malfunctioning(ex. BUS clock change...)
	 * If you want to change these frequecies, please contact the person
	 * in charge of clock driver.
	 */
	(void)tcc_set_pll(PLL_1, CKC_ENABLE, 1200000000UL, 3UL);
	(void)tcc_set_pll(PLL_2, CKC_ENABLE, 1240000000UL, 3UL); /* for supporting component/composite */
	(void)tcc_set_pll(PLL_3, CKC_ENABLE, 1188000000UL, 0UL);
	(void)tcc_set_pll(PLL_4, CKC_ENABLE, 1000000000UL, 0UL); /* for cm bus */

	/* Modifying frequency of following PLLs will affect peripherals
	 * on its dedicated block. The frequency of the PLL can be modified at
	 * the discretion of the peripheral person in charge of the block.
	 * (ex. GPU(= Graphic BUS PLL), Video BUS PLL)
	 */
	(void)tcc_set_pll(PLL_VIDEO_0, CKC_ENABLE, 3120000000UL, VPLL_DIV(4, 6));
	(void)tcc_set_pll(PLL_VIDEO_1, CKC_ENABLE, 2800000000UL, VPLL_DIV(4, 1));

	(void)tcc_set_peri(PERI_TCT, CKC_ENABLE, 12000000UL, 0UL);

	(void)tcc_set_clkctrl(FBUS_IO, CKC_ENABLE, 300000000UL); /* for setting i2c */

#endif
}

#if 0
void clock_test()
{
	struct arm_smccc_res res;
	int i;

	printf("\n------------------------------ PERI Clock -------------------------------\n");
	for (i = 0; i < PERI_MAX; i++) {
		arm_smccc_smc(SIP_CLK_IS_PERI, i, 0, 0, 0, 0, 0, 0, &res);
		if(res.a0 == 0) {
			tcc_set_peri(i, CKC_ENABLE, 50000000, 0);
		}
		printf("PERI_%03d : %10d     ", i, tcc_get_peri(i));
		if ((i+1) % 3 == 0)
			printf("\n");
	}
	printf("\n");

	printf("\n------------------------------ IOBUS Clock ------------------------------\n");
	for (i = 0; i < IOBUS_MAX; i++) {
		if (i == IOBUS_UART0 || i == IOBUS_UART_SMARTCARD0 || i == IOBUS_UART_SMARTCARD1) {
			printf("IOBUS_%03d : (Skip Console UART)        ", i);
			if ((i+1) % 2 == 0)
				printf("\n");
			continue;
		}
		arm_smccc_smc(SIP_CLK_DISABLE_IOBUS, i, 0, 0, 0, 0, 0, 0, &res);
		arm_smccc_smc(SIP_CLK_IS_IOBUS, i, 0, 0, 0, 0, 0, 0, &res);
		printf("IOBUS_%03d : %s ->  ", i, res.a0?"(PWR OFF)":"(PWR ON)");
		arm_smccc_smc(SIP_CLK_ENABLE_IOBUS, i, 0, 0, 0, 0, 0, 0, &res);
		arm_smccc_smc(SIP_CLK_IS_IOBUS, i, 0, 0, 0, 0, 0, 0, &res);
		printf("%s     ", res.a0?"(PWR OFF)":"(PWR ON)");
		if ((i+1) % 2 == 0)
			printf("\n");
	}
	printf("\n");

	printf("\n------------------------------- HSIO BUS Clock ------------------------------\n");
	for (i = 0; i < HSIOBUS_MAX; i++) {
		arm_smccc_smc(SIP_CLK_DISABLE_HSIOBUS, i, 0, 0, 0, 0, 0, 0, &res);
		arm_smccc_smc(SIP_CLK_IS_HSIOBUS, i, 0, 0, 0, 0, 0, 0, &res);
		printf("HSIOBUS_%03d : %s ->  ", i, res.a0?"(PWR OFF)":"(PWR ON)");
		arm_smccc_smc(SIP_CLK_ENABLE_HSIOBUS, i, 0, 0, 0, 0, 0, 0, &res);
		arm_smccc_smc(SIP_CLK_IS_HSIOBUS, i, 0, 0, 0, 0, 0, 0, &res);
		printf("%s     ", res.a0?"(PWR OFF)":"(PWR ON)");
		if ((i+1) % 2 == 0)
			printf("\n");
	}
	printf("\n");

	printf("\n------------------------------- DDI BUS Clock -----------------------------\n");
	for (i = 0; i < DDIBUS_MAX; i++) {
		arm_smccc_smc(SIP_CLK_DISABLE_DDIBUS, i, 0, 0, 0, 0, 0, 0, &res);
		arm_smccc_smc(SIP_CLK_IS_DDIBUS, i, 0, 0, 0, 0, 0, 0, &res);
		printf("DDIBUS_%03d : %s ->  ", i, res.a0?"(PWR OFF)":"(PWR ON)");
		arm_smccc_smc(SIP_CLK_ENABLE_DDIBUS, i, 0, 0, 0, 0, 0, 0, &res);
		arm_smccc_smc(SIP_CLK_IS_DDIBUS, i, 0, 0, 0, 0, 0, 0, &res);
		printf("%s     ", res.a0?"(PWR OFF)":"(PWR ON)");
		if ((i+1) % 2 == 0)
			printf("\n");
	}
	printf("\n");

	printf("\n------------------------------- VPU BUS Clock -----------------------------\n");
	for (i = 0; i < VIDEOBUS_MAX; i++) {
		arm_smccc_smc(SIP_CLK_DISABLE_VPUBUS, i, 0, 0, 0, 0, 0, 0, &res);
		arm_smccc_smc(SIP_CLK_IS_VPUBUS, i, 0, 0, 0, 0, 0, 0, &res);
		printf("VPUBUS_%03d : %s ->  ", i, res.a0?"(PWR OFF)":"(PWR ON)");
		arm_smccc_smc(SIP_CLK_ENABLE_VPUBUS, i, 0, 0, 0, 0, 0, 0, &res);
		arm_smccc_smc(SIP_CLK_IS_VPUBUS, i, 0, 0, 0, 0, 0, 0, &res);
		printf("%s     ", res.a0?"(PWR OFF)":"(PWR ON)");
		if ((i+1) % 2 == 0)
			printf("\n");
	}
	printf("\n");
}
#endif
