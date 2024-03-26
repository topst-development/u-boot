// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/arm-smccc.h>
#include <mach/clock.h>
#include <mach/smc.h>
#include <mach/chipinfo.h>
#include <mach/reboot.h>

void clock_init(void)
{
	struct clkctrl {
		unsigned long channel;
		unsigned long rate;
		unsigned long flags;
	};
	struct clkctrl initial_clkctrl[24] = {
		/* System Bus  */
#if 0
		/* Below BUS clocks already set in TF-A BL2 */
		{FBUS_CPUBUS,	    400000000UL,   0UL}, /* 400 MHz */
		{FBUS_OIC,	    930000000UL,   0UL}, /* 930 MHz */
		{FBUS_OMC_ACLK,	    930000000UL,   0UL}, /* 930 MHz */
		{FBUS_OIC_CR,	    465000000UL,   0UL}, /* 465 MHz */
		{FBUS_IMEM,	    434000000UL,   0UL}, /* 433.333... MHz */
		{FBUS_HSM,	    400000000UL,   0UL}, /* 400 MHz */
		{FBUS_DMA,	    300000000UL,   0UL}, /* 300 MHz */
		{FBUS_GMAC,	    300000000UL,   0UL}, /* 300 MHz */
		{FBUS_SFMC,	    300000000UL,   0UL}, /* 300 MHz */
		{FBUS_SDMMC,	    400000000UL,   0UL}, /* 400 MHz */
		{FBUS_UART,	    300000000UL,   0UL}, /* 300 MHz */
		{FBUS_SYS,	    200000000UL,   0UL}, /* 200 MHz */
		{FBUS_MEMCFG,	    465000000UL,   0UL}, /* 465 MHz */
		{FBUS_OTP,	    465000000UL,   0UL}, /* 465 MHz */
		{FBUS_TZCFG,	    465000000UL,   0UL}, /* 465 MHz */
		{FBUS_OMC_PCLK,	    465000000UL,   0UL}, /* 465 MHz */
		{FBUS_SIL,	    400000000UL,   0UL}, /* 400 MHz */
		{FBUS_OIC_SAFE_DAP, 750000000UL,   0UL}, /* 750 MHz */
		{FBUS_OIC_PCDV,	  930000000UL,   0UL}, /* 930 MHz */
		{FBUS_OIC_NPU0,	  930000000UL,   0UL}, /* 930 MHz */
		{FBUS_OIC_NPU1,	  930000000UL,   0UL}, /* 930 MHz */
#endif
#ifdef CONFIG_PCIE_TELECHIPS
		{FBUS_PCIe,	  310000000UL,   0UL}, /* 310 MHz */
#endif
		{FBUS_CAM,	  600000000UL,   0UL}, /* 600 MHz */
		{FBUS_DDI,	  600000000UL,   0UL}, /* 600 MHz */
		{FBUS_CBC,	  465000000UL,   0UL}, /* 465 MHz */
		{FBUS_DMA,	  300000000UL,   0UL}, /* 300 MHz */
		{FBUS_CAN,	  300000000UL,   0UL}, /* 300 MHz */
		{FBUS_GPSB,	  200000000UL,   0UL}, /* 200 MHz */
		{FBUS_I2C,	  200000000UL,   0UL}, /* 200 MHz */
		{FBUS_LIDAR_PROC, 100000000UL,   0UL}, /* 100 MHz */
		{FBUS_LIDAR_CTRL, 100000000UL,   0UL}, /* 100 MHz */
		/* Camera Subsystem Bus */
		{FBUS_CAM_AXI,	  550000000UL,   0UL}, /* 550 MHz */
		{FBUS_CAM_APB,	  550000000UL,   0UL}, /* 550 MHz */
		{FBUS_CAM_PIX,	  550000000UL,   0UL}, /* 550 MHz */
		/* NPU Subsystem Bus */
		{FBUS_NPU_0_ACLK,    800000000UL,   0UL}, /*  800 MHz */
		{FBUS_NPU_0_CCLK,   1000000000UL,   0UL}, /* 1000 MHz */
		{FBUS_NPU_0_CPUCLK,  800000000UL,   0UL}, /*  800 MHz */
		{FBUS_NPU_1_ACLK,    800000000UL,   0UL}, /*  800 MHz */
		{FBUS_NPU_1_CCLK,   1000000000UL,   0UL}, /* 1000 MHz */
		{FBUS_NPU_1_CPUCLK,  800000000UL,   0UL}, /*  800 MHz */
		{FBUS_NPU_PCLK,	     200000000UL,   0UL}, /*  200 MHz */
		/* Video Subsystem Bus */
		{FBUS_VBUS,	  750000000UL,   0UL}, /* 750 MHz */
		{FBUS_CHEVCENC,	  700000000UL,   0UL}, /* 700 MHz */
		{FBUS_BHEVCENC,	  500000000UL,   0UL}, /* 500 MHz */
	};
	unsigned long i;

	if (!core_reset_occurred()) {
		for (i = 0; i < ARRAY_SIZE(initial_clkctrl); i++) {
			(void)tcc_set_clkctrl(  initial_clkctrl[i].channel,
					CKC_ENABLE,
					initial_clkctrl[i].rate,
					initial_clkctrl[i].flags);
		}
	}
}

void clock_init_early(void)
{
	/* Initialize all peripheral clocks(disable). */
	tcc_clk_init();

	if (!core_reset_occurred()) {
		/* Modifying frequency of following PLLs(PLL0 ~ PLL4) will affect system
		 * malfunctioning(ex. BUS clock change...)
		 * If you want to change these frequecies, please contact the person
		 * in charge of clock driver.
		 */
#if 0
		/* change clock source to XIN before change PLL values. */
		(void)tcc_set_clkctrl(FBUS_CPUBUS, CKC_ENABLE, XIN_CLK_RATE/2, CLK_F_CLK_SRC(SYS_CLKCTRL_SEL_XIN));

		/*
		 * PLL0-3 already set in TF-A.
		 * Because of these PLLs are related with memory subsystem,
		 * we are not recommend to change PLL0-3 clocks in U-Boot.
		 */
		(void)tcc_set_pll(PLL_0, CKC_ENABLE, 3000000000UL, 0UL); // 3.0  GHz
		(void)tcc_set_pll(PLL_1, CKC_ENABLE, 1860000000UL, 0UL); // 1.86 GHz
		(void)tcc_set_pll(PLL_2, CKC_ENABLE, 2400000000UL, 0UL); // 2.4  GHz
		(void)tcc_set_pll(PLL_3, CKC_ENABLE, 2600000000UL, 0UL); // 2.6  GHz
#endif
		(void)tcc_set_pll(PLL_4, CKC_ENABLE, 1600000000UL, 0UL); // 1.6  GHz
		(void)tcc_set_pll_div(PLL_4_DIV, 2);

		/* Modifying frequency of following PLLs will affect peripherals
		 * on its dedicated block. The frequency of the PLL can be modified at
		 * the discretion of the peripheral person in charge of the block.
		 * (ex. GPU(= Graphic BUS PLL), Video BUS PLL)
		 */

		(void)tcc_set_pll(PLL_CAM, CKC_ENABLE, 1100000000UL, 0);
		(void)tcc_set_pll_div(PLL_CAM_DIV_0, 2); /* FBUS_CAM_AXI (550 MHz)  */
		(void)tcc_set_pll_div(PLL_CAM_DIV_1, 2); /* FBUS_CAM_APB (550 MHz)  */
		(void)tcc_set_pll_div(PLL_CAM_DIV_2, 2); /* FBUS_CAM_PIX (550 MHz)  */

		(void)tcc_set_pll(PLL_NPU_0, CKC_ENABLE, 1000000000UL, 0);
		(void)tcc_set_pll(PLL_NPU_1, CKC_ENABLE,  800000000UL, 0);
		(void)tcc_set_pll_div(PLL_NPU_1_DIV_1, 4); /* For FBUS_NPU_PCLK */

		(void)tcc_set_pll(PLL_VIDEO_0, CKC_ENABLE, 3000000000UL, 0);
		(void)tcc_set_pll(PLL_VIDEO_1, CKC_ENABLE, 2800000000UL, 0);
		(void)tcc_set_pll_div(PLL_VIDEO_0_DIV_0, 4);
		(void)tcc_set_pll_div(PLL_VIDEO_0_DIV_1, 6);
		(void)tcc_set_pll_div(PLL_VIDEO_1_DIV_0, 4);

		(void)tcc_set_peri(PERI_TCT, CKC_ENABLE, 12000000UL, 0UL);

#ifdef CONFIG_PCIE_TELECHIPS
		(void)tcc_set_peri(PERI_PCIe, CKC_ENABLE, 100000000UL, 0UL);
#endif
	}

}
