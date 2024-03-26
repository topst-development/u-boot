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
#include <mach/reboot.h>
#include <mach/chipinfo.h>

struct clkctrl {
	unsigned long channel;
	unsigned long rate;
	unsigned long flags;
};

static void set_bus_clocks(struct clkctrl const *clk_list, size_t length)
{
	uint32_t i;

	for (i = 0; i < length; i++) {
		if (clk_list[i].rate != 0UL) {
			(void)tcc_set_clkctrl(clk_list[i].channel,
					CKC_ENABLE, clk_list[i].rate, clk_list[i].flags);
		}
	}
}

static void clock_init_early_main(void);
/* Remove below comment if sub cluster need to set clock */
/* static void clock_init_early_sub(void); */
static void clock_init_main(void);
static void clock_init_sub(void);

void clock_init(void)
{
	struct boot_info binfo;
	get_boot_info(&binfo);

	if (!core_reset_occurred()) {
		if (is_main_core(binfo.coreid)) {
			clock_init_main();
		} else {
			clock_init_sub();
		}
	}
}

static void clock_init_main(void)
{
	const struct clkctrl clk_list[16] = {

		{FBUS_GPU,	980000000UL, 0UL},
		{FBUS_G2D,	700000000UL, 0UL},

		{FBUS_DDI_AXI,	400000000UL, 0UL},
		{FBUS_DDI_AUX,	 16000000UL, 0UL},
		{FBUS_DDI_APB,	200000000UL, 0UL},

		{FBUS_CODA,	700000000UL, 0UL},
		{FBUS_CHEVCDEC,	800000000UL, 0UL},
		{FBUS_BHEVCDEC,	640000000UL, 0UL},
		{FBUS_CHEVCENC,	800000000UL, 0UL},
		{FBUS_BHEVCENC,	800000000UL, 0UL},
		{FBUS_VBUS,	700000000UL, 0UL},

		{FBUS_AUDIO,	373000000UL, 0UL}, /* 372.556... MHz */

		{FBUS_CAM_APB,	183000000UL, 0UL},
		{FBUS_CAM_AXI,	732000000UL, 0UL},
		{FBUS_CAM_PIX,	732000000UL, 0UL},
		{FBUS_CAM_SYS,	292800000UL, 0UL} /* Actual clock 292.8 */

	};


	/* VIDEO CLOCK */
	(void)tcc_set_clkctrl(FBUS_CODA,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(VPU_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_CHEVCDEC,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(VPU_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_BHEVCDEC,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(VPU_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_CHEVCENC,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(VPU_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_BHEVCENC,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(VPU_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_VBUS,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(VPU_CLKCTRL_SEL_XIN));

	(void)tcc_set_pll(PLL_VIDEO_0, CKC_ENABLE, 3200000000UL, 0);
	(void)tcc_set_pll_div(PLL_VIDEO_0_DIV_0, 4); // 800 MHz
	(void)tcc_set_pll_div(PLL_VIDEO_0_DIV_1, 5); // 640 MHz
	(void)tcc_set_pll(PLL_VIDEO_1, CKC_ENABLE, 2800000000UL, 0);
	(void)tcc_set_pll_div(PLL_VIDEO_1_DIV_0, 4); // 700 MHz

	/* DISPALY CLOCK */
	writel((0x1), (uintptr_t)0x124D0000UL); // Enable DP Access

	(void)tcc_set_clkctrl(FBUS_DDI_AXI,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(DDI_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_DDI_AUX,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(DDI_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_DDI_APB,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(DDI_CLKCTRL_SEL_XIN));

	(void)tcc_set_pll(PLL_DDI, CKC_ENABLE, 800000000UL, 0);
	(void)tcc_set_pll_div(PLL_DDI_DIV_0, 2); // 400 MHz
	(void)tcc_set_pll_div(PLL_DDI_DIV_1, 50);//  16 MHz
	(void)tcc_set_pll_div(PLL_DDI_DIV_2, 4); // 200 MHz


	/* CAM CLOCK */
	(void)tcc_set_clkctrl(FBUS_CAM_APB,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(CAM_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_CAM_AXI,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(CAM_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_CAM_SYS,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(CAM_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_CAM_PIX,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(CAM_CLKCTRL_SEL_XIN));

	(void)tcc_set_pll(PLL_CAM, CKC_ENABLE, 1464000000UL, 0);
	(void)tcc_set_pll_div(PLL_CAM_DIV_0, 8); // APB 183 MHz
	(void)tcc_set_pll_div(PLL_CAM_DIV_1, 2); // AXI 732 MHz
	(void)tcc_set_pll_div(PLL_CAM_DIV_2, 5); // SYS 292.8 MHz
	(void)tcc_set_pll_div(PLL_CAM_DIV_3, 2); // PIX 732 MHz

	/* AUDIO CLOCK */
	(void)tcc_set_clkctrl(FBUS_AUDIO, CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(AUDIO_CLKCTRL_SEL_XIN));

	(void)tcc_set_pll(PLL_AUDIO_0, CKC_ENABLE, 2801664000UL, 0UL);
	(void)tcc_set_pll_div(PLL_AUDIO_0, 8);
	(void)tcc_set_pll(PLL_AUDIO_1, CKC_ENABLE, 2980454400UL, 0UL);
	(void)tcc_set_pll_div(PLL_AUDIO_1, 8);


	set_bus_clocks(clk_list, ARRAY_SIZE(clk_list));
	writel((0x0), (uintptr_t)0x124D0000UL); // Disable DP Access
}

static void clock_init_sub(void)
{
	const struct clkctrl clk_list[1] = {
		{FBUS_NPU,	1000000000UL, 0UL}
	};

	/* NPU CLOCK */
	(void)tcc_set_clkctrl(FBUS_NPU,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(NPU_CLKCTRL_SEL_XIN));

	(void)tcc_set_pll(PLL_NPU, CKC_ENABLE, 2000000000UL, 0);
	(void)tcc_set_pll_div(PLL_NPU_DIV, 2); // 1000 MHz

	set_bus_clocks(clk_list, ARRAY_SIZE(clk_list));
}

void clock_init_early(void)
{
	struct boot_info binfo;
	struct arm_smccc_res res;
	get_boot_info(&binfo);

	/* Initialize TF-A clocks drivers */
	tcc_clk_init();

	if (!core_reset_occurred()) {
		if (is_main_core(binfo.coreid)) {
			clock_init_early_main();
			if (is_dual_boot(binfo.bootsel)) {
				arm_smccc_smc(SIP_CLK_SYNC, true, 0, 0, 0, 0, 0, 0, &res);
			}
		} else {
			do {
				arm_smccc_smc(SIP_CLK_SYNC, false, 0, 0, 0, 0, 0, 0, &res);
			} while (res.a3 != CKC_OK);
			/* Remove comments if sub cluster need to set clock */
			/* clock_init_early_sub(); */
		}
	}

	if (is_main_core(binfo.coreid)) {
		(void)tcc_set_peri(PERI_TCT, CKC_ENABLE, 12000000UL, 0UL);
#ifdef CONFIG_PCIE_TELECHIPS
		(void)tcc_set_peri(PERI_PCIE0_PHY_CR_CLK, CKC_ENABLE, 100000000UL, 0UL);
#endif
	}

}

/*
 * Modifying frequency of following PLLs(PLL0 ~ PLL4) will affect system
 * malfunctioning(ex. BUS clock change...)
 * If you want to change these frequecies, please contact the person
 * in charge of clock driver.
 */
static void clock_init_early_main(void)
{
	const struct clkctrl clk_list[9] = {
		{FBUS_TAW,	1000000000UL, 0UL},
		{FBUS_CPU,	 400000000UL, 0UL},
		{FBUS_IO,	 297000000UL, 0UL},
		{FBUS_SMU,	 200000000UL, 0UL},
		{FBUS_HSIO,	 297000000UL, 0UL},
		{FBUS_HSM,	 400000000UL, 0UL},
		{FBUS_DDI,	 700000000UL, 0UL},
#ifdef CONFIG_PCIE_TELECHIPS
		{FBUS_PCIe0,	320000000UL, 0UL},
		{FBUS_PCIe1,	320000000UL, 0UL},
#else
		{FBUS_PCIe0,	XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(SMU_CLKCTRL_SEL_XIN)},
		{FBUS_PCIe1,	XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(SMU_CLKCTRL_SEL_XIN)}
#endif
	};

	/* change clock source to XIN before change PLL values. */
	(void)tcc_set_clkctrl(FBUS_CPU,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(SMU_CLKCTRL_SEL_XIN));
	/* Remove comments if sub cluster need to set clock */
	(void)tcc_set_clkctrl(FBUS_IO,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(SMU_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_SMU,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(SMU_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_HSIO,CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(SMU_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_HSM,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(SMU_CLKCTRL_SEL_XIN));
	(void)tcc_set_clkctrl(FBUS_TAW,	CKC_ENABLE, XIN_CLK_RATE/2UL, CLK_F_SRC_CLK(SMU_CLKCTRL_SEL_XIN));

	(void)tcc_set_pll(PLL_0, CKC_ENABLE, 2000000000UL, 0UL); /* 2000 MHz */
	(void)tcc_set_pll(PLL_1, CKC_ENABLE, 1400000000UL, 0UL); /* 1400 MHz */
	(void)tcc_set_pll(PLL_2, CKC_ENABLE, 3200000000UL, 0UL); /* 3200 MHz */
	(void)tcc_set_pll(PLL_3, CKC_ENABLE, 1188000000UL, 0UL); /* 1188 MHz - for 4K dp out (SDK default environment) */
	(void)tcc_set_pll(PLL_4, CKC_ENABLE, 2160000000UL, 0UL); /* 2160 MHz - for MIPI DSI 90 MHz + UART 48 MHz(BT) */
	/* PLL_4 is not in use for now. */
	// (void)tcc_set_pll(PLL_4, CKC_ENABLE, 1000000000UL, 0UL);

	(void)tcc_set_pll_div(PLL_0_DIV, 10UL); // 200 MHz
	(void)tcc_set_pll_div(PLL_1_DIV, 10UL); // 140 MHz
	(void)tcc_set_pll_div(PLL_2_DIV, 10UL); // 320 MHz
	(void)tcc_set_pll_div(PLL_4_DIV, 10UL); // 216 MHz

	set_bus_clocks(clk_list, ARRAY_SIZE(clk_list));
}

/* Remove below comment if sub cluster need to set clock */
/*
static void clock_init_early_sub(void)
{
	struct clkctrl clk_list[1] = {
		{FBUS_SP_CA55,	1000000000UL, 0UL},
	};

	set_bus_clocks(clk_list, ARRAY_SIZE(clk_list));
}
*/
