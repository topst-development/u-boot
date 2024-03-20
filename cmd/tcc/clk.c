// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <command.h>
#include <common.h>
#include <linux/io.h>
#include <asm/arch/clock.h>
#include <linux/arm-smccc.h>
#include <asm/telechips/tcc_ckc.h>
#include <mach/smc.h>

static int do_tcc_clk_show(cmd_tbl_t *cmdtp, int flag,
			   int argc, char * const argv[])
{
	int i;

	pr_force("\n------------------- PLL Clocks --------------------\n");
	for (i = 0; i < MAX_TCC_PLL; i++) {
		pr_force("%29s%d : %15llu Hz\n", "PLL", i, tcc_get_pll(i));
	}

	pr_force("%30s : %15llu Hz\n", "VIDEO PLL0", tcc_get_pll(PLL_VIDEO_0));
	pr_force("%30s : %15llu Hz\n", "VIDEO PLL1", tcc_get_pll(PLL_VIDEO_1));

	pr_force("\n------------------- Bus Clocks --------------------\n");
	pr_force("%30s : %15llu Hz\n", "CPU (Cortex-A72)",
	       tcc_get_clkctrl(FBUS_CPU0));
	pr_force("%30s : %15llu Hz\n", "CPU (Cortex-A53)",
	       tcc_get_clkctrl(FBUS_CPU1));
	pr_force("%30s : %15llu Hz\n", "CPU BUS",
	       tcc_get_clkctrl(FBUS_CBUS));
	pr_force("%30s : %15llu Hz\n", "CM BUS",
	       tcc_get_clkctrl(FBUS_CMBUS));
	pr_force("%30s : %15llu Hz\n", "SMU BUS",
	       tcc_get_clkctrl(FBUS_SMU));
	pr_force("%30s : %15llu Hz\n", "IO BUS",
	       tcc_get_clkctrl(FBUS_IO));
	pr_force("%30s : %15llu Hz\n", "HSIO BUS",
	       tcc_get_clkctrl(FBUS_HSIO));
	pr_force("%30s : %15llu Hz\n", "Display BUS",
	       tcc_get_clkctrl(FBUS_DDI));
	pr_force("%30s : %15llu Hz\n", "Graphic 3D",
	       tcc_get_clkctrl(FBUS_GPU));
	pr_force("%30s : %15llu Hz\n", "Video BUS",
	       tcc_get_clkctrl(FBUS_VBUS));
	pr_force("%30s : %15llu Hz\n", "CODA",
	       tcc_get_clkctrl(FBUS_CODA));
	pr_force("%30s : %15llu Hz\n", "HEVC_C(Decoder)",
	       tcc_get_clkctrl(FBUS_CHEVCDEC));
	pr_force("%30s : %15llu Hz\n", "HEVC_B(Decoder)",
	       tcc_get_clkctrl(FBUS_BHEVCDEC));
	pr_force("%30s : %15llu Hz\n", "HEVC_C(Encoder)",
	       tcc_get_clkctrl(FBUS_CHEVCENC));
	pr_force("%30s : %15llu Hz\n", "HEVC_B(Encoder)",
	       tcc_get_clkctrl(FBUS_BHEVCENC));
	pr_force("%30s : %15llu Hz\n", "MEM BUS Core Clock",
	       tcc_get_clkctrl(FBUS_MEM));
	pr_force("%30s : %15llu Hz\n", "MEM BUS Sub System Clock",
	       tcc_get_clkctrl(FBUS_MEM_SUB));
	pr_force("%30s : %15llu Hz\n", "LPDDR4 PHY PLL Clock",
	       tcc_get_clkctrl(FBUS_MEM_LPDDR4));

	return 0;
}

U_BOOT_CMD(
	   clock, 2, 1, do_tcc_clk_show, "Telechips Clock Information tool", ""
	  );
