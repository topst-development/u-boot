// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <tav.h>

DECLARE_GLOBAL_DATA_PTR;

static void print_do_irq_help(void)
{
	pr_force(
	"Interrupt Test..!\r\n");
	pr_force(
	"tav irq <n> - Run VIOC interrupt Test\r\n"
	"         -n 0: Disable RU on Display0 interrupt\r\n"
	"            1: Enable RU on Display0 interrupt\r\n");
}

static int do_tavirq(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int enable;

	if (argc != 2) {
		print_do_irq_help();
		return 0;
	}

	if (!strcmp(argv[1], "help") && argc == 2) {
		print_do_irq_help();
	} else {
		enable = simple_strtoul(argv[1], NULL, 10);
		tcc_tav_irq(enable);
	}

	return 0;
}

static void print_db15_help(void)
{
	pr_force(
	"DB-15, VIOC Timer\r\n");
	pr_force(
	"tav db15 <n> - Run Display Bus no.15 Test\r\n"
	"          n -  1: 100us utimer\r\n"
	"               2: 200us utimer\r\n"
	"               3: timer irq - timer0 <1000Hz>, timer1 <500Hz>\r\n"
	"               4: timer irq - timer0 <2000Hz>, timer1 <500Hz>\r\n"
	"               5: timer irqe - tireq0 <1000 counts>, tireq1 <2000 counts>\r\n"
	);
}

static int do_db15(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int test_case;

	if (argc != 2) {
		print_db15_help();
		return 0;
	}

	if (!strcmp(argv[1], "help") && argc == 2) {
		print_db15_help();
	} else {
		test_case = simple_strtoul(argv[1], NULL, 10);
		tcc_tav_db15(test_case);
	}

	return 0;
}

static void print_db12_help(void)
{
	pr_force(
	"DB-12, Asynchronous frame fifo\r\n");
	pr_force(
	"tav db12 <n> - Run Display Bus Asynchronous frame fifo\r\n"
	"          n -  0: Run M2M path\r\n"
	"          n -  1: Run frame fifo\r\n");
}

static int do_db12(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int test_case;

	if (argc != 2) {
		print_db12_help();
		return 0;
	}

	if (!strcmp(argv[1], "help") && argc == 2) {
		print_db12_help();
	} else {
		test_case = simple_strtoul(argv[1], NULL, 10);
		tcc_tav_db12(test_case);
	}

	return 0;
}

static void print_db08_help(void)
{
	pr_force(
	"DB-07, Scaler\r\n");
	pr_force(
		"tav db08 <n> - Run Frame delay Test\r\n"
		"          n -  1: Run Frame delay\r\n");
}

static int do_db08(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int test_case;

	if (argc != 2) {
		print_db08_help();
		return 0;
	}

	if (!strcmp(argv[1], "help") && argc == 2) {
		print_db08_help();
	} else {
		test_case = simple_strtoul(argv[1], NULL, 10);
		tcc_tav_db08(test_case);
	}

	return 0;
}

static void print_db07_help(void)
{
	pr_force(
	"DB-07, Scaler\r\n");
	pr_force(
	"tav db07 <n> - Run Display Bus no.05 Test\r\n"
	"          n -  1: Scale Down\r\n"
	"               2: Scale Up\r\n"
	"               3: Max Resolution \r\n");
}

static int do_db07(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int test_case;

	if (argc != 2) {
		print_db07_help();
		return 0;
	}

	if (!strcmp(argv[1], "help") && argc == 2) {
		print_db07_help();
	} else {
		test_case = simple_strtoul(argv[1], NULL, 10);
		tcc_tav_db07(test_case);
	}

	return 0;
}

static void print_db06_help(void)
{
	pr_force(
	"DB-06, WDMA\r\n");
	pr_force(
	"tav db06 <n> - Run Display Bus no.06 Test\r\n"
	"          n -  0: Buffer Initialize\r\n"
	"               1: Color Conversion(YUV2RGB) Test\r\n"
	"               2: Color Conversion(RGB2YUV) Test\r\n"
	"               3: Output Test\r\n"
	"               4: Y2R Test\r\n"
	"               5: RGB Swap Test\r\n"
	"               6: Continuous/frame-by-frame Test\r\n");
}

static int do_db06(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int test_case;

	if (argc != 2) {
		print_db06_help();
		return 0;
	}

	if (!strcmp(argv[1], "help") && argc == 2) {
		print_db06_help();
	} else {
		test_case = simple_strtoul(argv[1], NULL, 10);
		tcc_tav_db06(test_case);
	}

	return 0;
}

static void print_db05_help(void)
{
	pr_force(
	"DB-05, GRDMA and VRDMA\r\n");
	pr_force(
	"tav db05 <n> - Run Display Bus no.05 Test\r\n"
	"          n -  0: Buffer Initialize\r\n"
	"               1: Format Test\r\n"
	"               2: Swap Test\r\n"
	"               3: BR(Bit Reverse) Test\r\n"
	"               4: Y2R Test\r\n"
	"               5: Y2RMD Test\r\n"
	"               6: R2Y Test\r\n"
	"               7: R2YMD Test\r\n");
}

static int do_db05(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int test_case;

	if (argc != 2) {
		print_db05_help();
		return 0;
	}

	if (!strcmp(argv[1], "help") && argc == 2) {
		print_db05_help();
	} else {
		test_case = simple_strtoul(argv[1], NULL, 10);
		tcc_tav_db05(test_case);
	}

	return 0;
}

static void print_do_db01_help(void)
{
	pr_force(
	"DB-01, Top\r\n");
	pr_force(
	"tav db01 <n> - Run Display Bus no.01 Test\r\n");
}

static int do_db01(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 2) {
		print_do_db01_help();
		return 0;
	}

	if (!strcmp(argv[1], "help") && argc == 2)
		print_do_db01_help();
	return 0;
}

/*---------------------------------------------------------------------------*/
static cmd_tbl_t cmd_tav_sub[] = {
	//VIOC
	U_BOOT_CMD_MKENT(db01, 3, 0, do_db01, "", ""),
	U_BOOT_CMD_MKENT(db05, 3, 0, do_db05, "", ""),
	U_BOOT_CMD_MKENT(db06, 3, 0, do_db06, "", ""),
	U_BOOT_CMD_MKENT(db07, 3, 0, do_db07, "", ""),
	U_BOOT_CMD_MKENT(db08, 3, 0, do_db08, "", ""),
	U_BOOT_CMD_MKENT(db12, 3, 0, do_db12, "", ""),
	U_BOOT_CMD_MKENT(db15, 3, 0, do_db15, "", ""),
	U_BOOT_CMD_MKENT(irq, 3, 0, do_tavirq, "", ""),

	//Graphic
	// U_BOOT_CMD_MKENT(gb06, 3, 0, do_gb06, "", ""),
};

static int do_tav(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_tav_sub[0], ARRAY_SIZE(cmd_tav_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

/***************************************************/
static char tav_help_text[] =
"=========================================================================\r\n"
"       TCC8050(Dolphin3) Verification                                    \r\n"
"=========================================================================\r\n"
"       Test Menu(VIOC)                                                   \r\n"
"=========================================================================\r\n"
"       <01> (cmd -> tav db01) DB-01, TOP                                 \r\n"
"       <02> (cmd -> tav db02) DB-02, Display Controller                  \r\n"
"       <03> (cmd -> tav db03) DB-03, VIN                                 \r\n"
"       <04> (cmd -> tav db04) DB-04, VIN_DEMUX                           \r\n"
"       <05> (cmd -> tav db05) DB-05, GRDMA and VRDMA                     \r\n"
"       <06> (cmd -> tav db06) DB-06, WDMA                                \r\n"
"       <07> (cmd -> tav db07) DB-07, Scaler                              \r\n"
"       <08> (cmd -> tav db08) DB-08, Frame delay                         \r\n"
"       <09> (cmd -> tav db09) DB-09, WMIX                                \r\n"
"       <10> (cmd -> tav db10) DB-10, Simple de-interlacer                \r\n"
"       <11> (cmd -> tav db11) DB-11, VIQE                                \r\n"
"       <12> (cmd -> tav db12) DB-12, Asynchronous frame fifo             \r\n"
"       <13> (cmd -> tav db13) DB-13, LUT                                 \r\n"
"       <14> (cmd -> tav db14) DB-14, VIOC Configuration & Interrupt      \r\n"
"       <15> (cmd -> tav db15) DB-15, Timer                               \r\n"
"       <16> (cmd -> tav db16) DB-16, Map Converter                       \r\n"
"       <21> (cmd -> tav db21) DB-21, Display sub-system configuration    \r\n"
"       tav irq                                                           \r\n"
"=========================================================================\r\n"
"       Test Menu(2D Graphic)                                             \r\n"
"=========================================================================\r\n"
"       <06> (cmd -> tav gb06) GB-06, 2D Graphic Engine                   \r\n"
"       <07> (cmd -> tav gb07) GB-07, 2D Graphic Engine Configuration     \r\n"
"       <08> (cmd -> tav gb08) GB-08, 2D Graphic Engine Dedicated CKC     \r\n"
"       <09> (cmd -> tav gb09) GB-09, 2D Graphic Engine Secure Wrapper    \r\n"
"=========================================================================\r\n";
U_BOOT_CMD(
	tav, 7, 1, do_tav,
	"Verify Tools for Telechips Application BSP Team.",
	tav_help_text
);
