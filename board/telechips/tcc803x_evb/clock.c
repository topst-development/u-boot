// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/telechips/tcc_ckc.h>
#include <linux/io.h>

#include <linux/arm-smccc.h>
#include <mach/smc.h>
#include <mach/chipinfo.h>

#if defined(CONFIG_POWER_RT5028)
#include <asm/telechips/i2c.h>
#include <power/rt5028.h>
#endif

#ifdef CONFIG_TCC_SDHCI_SUPPORT_HS400
#include <asm/telechips/tcc_sdmmc.h>
#endif

#define VPLL_DIV(x, y)	((unsigned long)((unsigned long)(y) << \
			  8UL) | (unsigned long)(x))

void clock_init(void)
{
	bool wake_from_corerst = core_reset_occurred();
	unsigned long cpu_speed;

#if defined(CONFIG_POWER_RT5028)
	rt5028_init(I2C_CH_MASTER3);
	rt5028_set_voltage(RT5028_ID_BUCK1, 1000);      // PWRCORE
	rt5028_set_voltage(RT5028_ID_BUCK2, 1500);      // MEM
	rt5028_set_voltage(RT5028_ID_BUCK3, 1100);      // PWRCPU
	rt5028_set_voltage(RT5028_ID_BUCK4, 3300);
						// PWR_IO(PWRUSB|PWRLVDS|ETC)

	rt5028_set_voltage(RT5028_ID_LDO2, 1800);       // VD2_CORE
	rt5028_set_voltage(RT5028_ID_LDO2, 1800);       // PWRPLL25|PWROTP25
	rt5028_set_voltage(RT5028_ID_LDO3, 3300);       // WIFI/BT_IO
	rt5028_set_voltage(RT5028_ID_LDO4, 3300);       // WIFI/BT_VBAT
	rt5028_set_voltage(RT5028_ID_LDO5, 3300);       // GPS
	rt5028_set_voltage(RT5028_ID_LDO6, 3300);       // VD1_IO
	rt5028_set_voltage(RT5028_ID_LDO7, 1800);       // VD1_CORE
	rt5028_set_voltage(RT5028_ID_LDO8, 3300);       // VD2_IO
#endif

	cpu_speed = 1450000000UL;
#if defined(CONFIG_CPU_OVER_VOLTAGE)
	if ((get_chip_code() & PIN2PIN) != 0U) { /* TCC803xpe */
		cpu_speed = 1200000000UL;
	}
#endif
	(void)tcc_set_clkctrl(FBUS_CPU0,   CKC_ENABLE,  cpu_speed);
	(void)tcc_set_clkctrl(FBUS_CBUS,   CKC_ENABLE,  400000000UL);

	/* do not control CMBUS clock in REE	*/
	/* tcc_set_clkctrl( FBUS_CMBUS,    CKC_ENABLE,  333333334);	*/

	/* 333MHz. set 250MHz for GMAC */
	(void)tcc_set_clkctrl(FBUS_HSIO,   CKC_ENABLE,  250000000UL);
	(void)tcc_set_clkctrl(FBUS_SMU,    CKC_ENABLE,  200000000UL);
	(void)tcc_set_clkctrl(FBUS_DDI,    CKC_ENABLE,  600000000UL);
	(void)tcc_set_clkctrl(FBUS_GPU,    CKC_ENABLE,  700000000UL);
	(void)tcc_set_clkctrl(FBUS_G2D,    CKC_ENABLE,  700000000UL);

	if (!wake_from_corerst) {
		(void)tcc_set_clkctrl(FBUS_CODA,   CKC_ENABLE,  700000000UL);
		(void)tcc_set_clkctrl(FBUS_CHEVC,  CKC_ENABLE,  800000000UL);
		(void)tcc_set_clkctrl(FBUS_BHEVC,  CKC_ENABLE,  520000000UL);
		(void)tcc_set_clkctrl(FBUS_VBUS,   CKC_ENABLE,  700000000UL);
	}
}

void clock_init_early(void)
{
	struct arm_smccc_res res;

	/* Initialize all peripheral clocks(disable). */
	arm_smccc_smc(SIP_CLK_INIT, 0, 0, 0, 0, 0, 0, 0, &res);

	/* change clock source to XIN before change PLL values. */
	(void)tcc_set_clkctrl(FBUS_IO,   CKC_ENABLE, XIN_CLK_RATE);
	(void)tcc_set_clkctrl(FBUS_SMU,  CKC_ENABLE, XIN_CLK_RATE);
	(void)tcc_set_clkctrl(FBUS_HSIO, CKC_ENABLE, XIN_CLK_RATE);

	/* Modifying frequency of following PLLs(PLL0 ~ PLL4) will affect system
	 * malfunctioning(ex. BUS clock change...)
	 * If you want to change these frequecies, please contact the person
	 * in charge of clock driver.
	 */

	/* tcc_set_pll(PLL_0,  CKC_ENABLE, 1000000000, 3UL); */
	(void)tcc_set_pll(PLL_1,  CKC_ENABLE,  768000000UL, 3UL);
	(void)tcc_set_pll(PLL_2,  CKC_ENABLE, 1188000000UL, 3UL);
	(void)tcc_set_pll(PLL_3,  CKC_ENABLE,  750000000UL, 0UL);

	/* Modifying frequency of following PLLs will affect peripherals
	 * on its dedicated block. The frequency of the PLL can be modified at
	 * the discretion of the peripheral person in charge of the block.
	 * (ex. GPU(= Graphic BUS PLL), Video BUS PLL)
	 */

	/* In TCC803x_PE, PLL_4 is used for dedicated membus PLL. */
	/*tcc_set_pll(PLL_4,  CKC_ENABLE, 1000000000, 0UL);*/
	(void)tcc_set_pll(PLL_VIDEO_0, CKC_ENABLE, 800000000UL, 2UL);
	(void)tcc_set_pll(PLL_VIDEO_1, CKC_ENABLE, 1500000000UL, 2UL);

	(void)tcc_set_peri(PERI_TCT, CKC_ENABLE, 12000000UL, 0UL);
	(void)tcc_set_peri(PERI_TCZ, CKC_ENABLE, 12000000UL, 0UL);
	(void)tcc_set_peri(PERI_CB_WDT, CKC_ENABLE, 12000000UL, 0UL);
	(void)tcc_set_peri(PERI_GMAC, CKC_ENABLE, 125000000UL, 0UL);
#if defined(CONFIG_BOOT_CM4_BY_MAINCORE)
	(void)tcc_set_peri(PERI_MIPI_CSI, CKC_ENABLE, 300000000UL, 0UL);
#endif
	writel(0x1, TCC_TIMER_BASE);		/* Timer0 Enable */
	writel(0x5B, TCC_TIMER_BASE + 0x8U);	/* Timer0 TCNT reg. write */


	/* for setting i2c */
	(void)tcc_set_clkctrl(FBUS_IO,     CKC_ENABLE,  300000000UL);
}
