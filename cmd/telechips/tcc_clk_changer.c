// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <command.h>
#include <common.h>
#include <dm/uclass.h>
#include <dm.h>
#include <asm/io.h>
#include <mach/clock.h>
#include <string.h>

#define TARGET_IDX	1UL
#define	FREQ_IDX	2UL
#define MAX_BUS_CLOCKS	16UL
#define	TARGET_ERR	101UL

static void tcc_clocks_status(void)
{
	(void)pr_force("\n----After Clock Change----\n");
	(void)pr_force("CPU_CA72_CCU :%luHz\n", tcc_get_clkctrl(FBUS_CPU0));
	(void)pr_force("CPU_CA53_CCU :%luHz\n", tcc_get_clkctrl(FBUS_CPU1));
	(void)pr_force("FBUS_GBM : %luHz\n", tcc_get_clkctrl(FBUS_GPU));
	(void)pr_force("FBUS_GBG : %luHz\n", tcc_get_clkctrl(FBUS_G2D));
	(void)pr_force("FBUS_CB : %luHz\n", tcc_get_clkctrl(FBUS_CBUS));
	(void)pr_force("FBUS_CMB : %luHz\n", tcc_get_clkctrl(FBUS_CMBUS));
	(void)pr_force("FBUS_HSIOB : %luHz\n", tcc_get_clkctrl(FBUS_HSIO));
	(void)pr_force("FBUS_SMU : %luHz\n", tcc_get_clkctrl(FBUS_SMU));
	(void)pr_force("FBUS_DDI : %luHz\n", tcc_get_clkctrl(FBUS_DDI));
	(void)pr_force("FBUS_IOB : %luHz\n", tcc_get_clkctrl(FBUS_IO));
	(void)pr_force("FBUS_VB : %luHz\n", tcc_get_clkctrl(FBUS_VBUS));
	(void)pr_force("FBUS_CCLK_CODA : %luHz\n", tcc_get_clkctrl(FBUS_CODA));
	(void)pr_force("FBUS_CCLK_HEVCDEC :%luHz\n", tcc_get_clkctrl(FBUS_CHEVCDEC));
	(void)pr_force("FBUS_BCLK_HEVCDEC :%luHz\n", tcc_get_clkctrl(FBUS_BHEVCDEC));
	(void)pr_force("FBUS_CCLK_HEVCENC :%luHz\n", tcc_get_clkctrl(FBUS_CHEVCENC));
	(void)pr_force("FBUS_BCLK_HEVCENC :%luHz\n", tcc_get_clkctrl(FBUS_BHEVCENC));
}

static unsigned long tcc_clk_change_get_target(void)
{
	unsigned long ret = 0, i;
	int slct;

	struct clk_slct {
		int target;
		unsigned long clock_id;
		const char *bus_name;
	};

	const struct clk_slct bus_names[MAX_BUS_CLOCKS] = {
			{(int)'1', FBUS_CPU0, "CPU_CA72_CCU"},
			{(int)'2', FBUS_CPU1, "CPU_CA53_CCU"},
			{(int)'3', FBUS_GPU, "FBUS_GBM"},
			{(int)'4', FBUS_G2D, "FBUS_GBG"},
			{(int)'5', FBUS_CBUS, "FBUS_CB"},
			{(int)'6', FBUS_CMBUS, "FBUS_CMB"},
			{(int)'7', FBUS_HSIO, "FBUS_HSIOB"},
			{(int)'8', FBUS_SMU, "FBUS_SMU"},
			{(int)'9', FBUS_DDI, "FBUS_DDI"},
			{(int)'0', FBUS_IO, "FBUS_IOB"},
			{(int)'a', FBUS_VBUS, "FBUS_VB"},
			{(int)'b', FBUS_CODA, "FBUS_CCLK_CODA"},
			{(int)'c', FBUS_CHEVCDEC, "FBUS_CCLK_HEVCDEC"},
			{(int)'d', FBUS_BHEVCDEC, "FBUS_BCLK_HEVCDEC"},
			{(int)'e', FBUS_CHEVCENC, "FBUS_CCLK_HEVCENC"},
			{(int)'f', FBUS_BHEVCENC, "FBUS_BCLK_HEVCENC"}
	};

	for (i = 1; i <= MAX_BUS_CLOCKS; i++) {
		(void)pr_force("%c. %s \t", bus_names[i-1UL].target,
				  bus_names[i-1UL].bus_name);

		if ((i % 4UL) == 0UL) {
			(void)pr_force("\n");
		}
	}

	(void)pr_force("\nPlease Select proper target : ");
	slct = getchar();

	(void)pr_force("%c", slct);

	if ((((int)'0' <= slct) && (slct <= (int)'9')) ||
			(((int)'a' <= slct) && (slct <= (int)'f'))) {
		for (i = 0; i < MAX_BUS_CLOCKS; i++) {
			if (slct == bus_names[i].target) {
				(void)pr_force("\n'%s' Selected.\n",
					 bus_names[i].bus_name);
				ret = bus_names[i].clock_id;
				break;
			}
		}
	} else {
		ret = TARGET_ERR;
	}

	return ret;
}

static unsigned long tcc_clk_change_get_frequency(void)
{
	int ch;
	unsigned long req_freq = 0;

	(void)pr_force("\nPlease input target frequency");
	(void)pr_force(" (ex. 1G(g), 30M(m), 300K(k)) : ");

	ch = getchar();
	while (ch != (int)'\r') {
		(void)pr_force("%c", ch);
		if (((int)'0' <= ch) && (ch <= (int)'9')) {
			/* only number will be calculated. */
			if (req_freq != 0UL) {
				req_freq *= 10UL;
			}

			ch -= (int)'0';
			req_freq += (unsigned long)ch;
		}
		if ((ch == (int)'G') || (ch == (int)'g')) {
			req_freq = req_freq * 1000UL * 1000UL * 1000UL;
		} else if ((ch == (int)'M') || (ch == (int)'m')) {
			req_freq = req_freq * 1000UL * 1000UL;
		} else if ((ch == (int)'K') || (ch == (int)'k')) {
			req_freq = req_freq * 1000UL;
		} else {
			// do nothing
		}

		ch = getchar();
	}
	(void)pr_force("\n");

	return req_freq;
}

static int do_tcc_clock_changer(struct cmd_tbl *cmdtp, int flag,
				int argc, char * const argv[])
{
	unsigned long target_id = 0x1000, freq = 0;
	int ret = 0, ch;

	(void)argv;
	(void)flag;
	(void)cmdtp;

	/* usage : 'tcc_clock_changer <TARGET> <FREQ.>' */
	if (argc < 1) {
		(void)pr_force("Refer following information.\n");
		ret = -1;
	} else {

		/* Get input from user. */
		target_id = tcc_clk_change_get_target();
		(void)pr_force("Target clock id : %lu\n", target_id);

		if (target_id == TARGET_ERR) {
			(void)pr_force("Unexpected input from user.\n");
			ret = -1;
		} else {
			/* Get frequency from user */
			freq = tcc_clk_change_get_frequency();
			(void)pr_force("Target frequency : %lu\n", freq);

			/* Warning message for changing clock frequency. */
			(void)pr_force("[WARNING] Changing sub-system or CPU Core clock might\n");
			(void)pr_force(" yield unexpected operation on other core.\n");
			(void)pr_force("Continue? (Y/N)");

			ch = getchar();
			if (ch == (int)'N') {
				(void)pr_force("\nTerminated without any configurations.\n");
				ret = -1;
			} else {
				/* Check Target. */
				(void)tcc_set_clkctrl(target_id, CKC_ENABLE, freq, 0UL);
				tcc_clocks_status();
			}
		}
	}

	return ret;
}

U_BOOT_CMD(
	tcc_clock_changer, 10, 1, do_tcc_clock_changer,
	"Clock Changer for sub-systems and CPU core clock",
	"\n- Description : This command allows to change clock frequency of\n"
	" Sub-systems and CPU Core clock.\n"
	"- Usage : # tcc_clock_changer [target] [frequency]\n"
	"- target : CPU_CA72_CCU, CPU_CA53_CCU, FBUS_GBM, FBUS_GBG\n"
	"  FBUS_CB, FBUS_CMB, FBUS_HSIOB, FBUS_SMU, FBUS_DDI, FBUS_IOB\n"
	"  FBUS_VB, FBUS_CCLK_CODA, FBUS_CCLK_HEVCDEC, FBUS_BCLK_HEVCDEC\n"
	"  FBUS_CCLK_HEVCENC, FBUS_BCLK_HEVCENC\n"
	"For more information about targets, please Refer TCC805x Full\n"
	"Specification 'Part2. SMU/PMU - 3.2 Bus Clock Generation' and\n"
	"'Part10. CPU BUS - 3.1 CKC Register Description'"
);
