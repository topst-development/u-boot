
// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <command.h>
#include <common.h>
#include <linux/io.h>
#include <clk.h>
#include <clk-uclass.h>

// Commands on this file MUST be executed on firmware which is in
// 'prebuilt_wdtrstdis.zip'

/*
 * IP-PV Dedicated CKC(CA72) 2. Reset - SWRESET
 */
static int do_tcc_ca72_ckc_reset(cmd_tbl_t *cmdtp,
				 int flag,
				 int argc,
				 char * const argv[])
{
	unsigned int ca72_cfg_swreset = 0x17100000+0x4;
	unsigned int ca72_ckc_base = 0x17110000;

	pr_force("cmd - tcc_ca72_ckc_reset\n");
	writel(readl(ca72_cfg_swreset) & ~(0x20), ca72_cfg_swreset);

	pr_force("After CA72 CKC Reset\n");
	pr_force("CLKCTRL : %x\n", readl(ca72_ckc_base + 0x0));
	pr_force("PLLPMS : %x\n", readl(ca72_ckc_base + 0x8));
	pr_force("PLLCON : %x\n", readl(ca72_ckc_base + 0xC));
	pr_force("PLLMON : %x\n", readl(ca72_ckc_base + 0x10));
	pr_force("PLLDIVC : %x\n", readl(ca72_ckc_base + 0x14));

	return 0;
}

/*
 * IP-PV Dedicated CKC(CA53) 2. Reset - SWRESET
 */
static int do_tcc_ca53_ckc_reset(cmd_tbl_t *cmdtp,
				 int flag,
				 int argc,
				 char * const argv[])
{
	unsigned int ca53_cfg_swreset = 0x17200000+0x4;
	unsigned int ca53_ckc_base = 0x17210000;

	pr_force("cmd - tcc_ca53_ckc_reset\n");
	writel(readl(ca53_cfg_swreset) & ~(0x20), ca53_cfg_swreset);

	pr_force("After CA53 CKC Reset\n");
	pr_force("CLKCTRL : %x\n", readl(ca53_ckc_base + 0x0));
	pr_force("PLLPMS : %x\n", readl(ca53_ckc_base + 0x8));
	pr_force("PLLCON : %x\n", readl(ca53_ckc_base + 0xC));
	pr_force("PLLMON : %x\n", readl(ca53_ckc_base + 0x10));
	pr_force("PLLDIVC : %x\n", readl(ca53_ckc_base + 0x14));

	return 0;
}

U_BOOT_CMD(
	tcc_ca72_ckc_reset, 2, 1, do_tcc_ca72_ckc_reset, "ca72 ckc reset", ""
	);

U_BOOT_CMD(
	tcc_ca53_ckc_reset, 2, 1, do_tcc_ca53_ckc_reset, "ca53 ckc reset", ""
	);


/*
 * IP-PV Dedicated CKC(CA72) 2. Reset - RSTDIS
 */
static int do_tcc_ca72_ckc_rst_disable(cmd_tbl_t *cmdtp,
				       int flag,
				       int argc,
				       char * const argv[])
{
	unsigned int ca72_rst_disable = 0x17100000 + 0x8;

	writel(readl(ca72_rst_disable) | 0x20, ca72_rst_disable);

	pr_force("cbus0 watchdog start\n");

	return 0;
}

/*
 * IP-PV Dedicated CKC(CA53) 2. Reset - RSTDIS
 */
static int do_tcc_ca53_ckc_rst_disable(cmd_tbl_t *cmdtp,
				       int flag,
				       int argc,
				       char * const argv[])
{
	unsigned int ca53_rst_disable = 0x17200000 + 0x8;

	writel(readl(ca53_rst_disable) | (0x20), ca53_rst_disable);

	pr_force("cbus4 watchdog start\n");

	return 0;
}

U_BOOT_CMD(
	tcc_ca72_ckc_disable_reset, 2, 1, do_tcc_ca72_ckc_rst_disable,
	"ca72 ckc reset disable", ""
	);

U_BOOT_CMD(
	tcc_ca53_ckc_disable_reset, 2, 1, do_tcc_ca53_ckc_rst_disable,
	"ca53 ckc reset disable", ""
	);


/*
 * IP-PV Dedicated CKC(CA72) 2. Reset - WDTRSTDIS
 */
static int do_tcc_ca72_ckc_wdt_rst_disable(cmd_tbl_t *cmdtp,
					   int flag,
					   int argc,
					   char * const argv[])
{
	unsigned int ca72_wdt_rst_disable = 0x17100000 + 0x70;

	 writel(readl(0x17000038) & ~(0x3), 0x17000038);

	writel(0x00008800, 0x1701000c);         // rst cnt for CBUS0 WDT
	writel(0x00000000, 0x17010008);         // irq cnt for CBUS0 WDT

	writel(0x1, 0x144000BC);	 // reset req enable for CBUS0 WDT
	writel(0x1, 0x17010000);         // enable

	pr_force("cbus0 watchdog start\n");

	return 0;
}

/*
 * IP-PV Dedicated CKC(CA53) 2. Reset - WDTRSTDIS
 */
static int do_tcc_ca53_ckc_wdt_rst_disable(cmd_tbl_t *cmdtp,
					   int flag,
					   int argc,
					   char * const argv[])
{
	unsigned int ca53_wdt_rst_disable = 0x17200000 + 0x70;

	writel(readl(0x17000038) & ~(0x3), 0x17000038);

	writel(0x00008800, 0x1705000c);         // rst cnt for CBUS4 WDT
	writel(0x00000440, 0x17050008);         // irq cnt for CBUS4 WDT

	writel(0x10, 0x144000BC);         // reset req enable for CBUS4 WDT
	writel(0x1, 0x17050000);          // enable

	pr_force("cbus4 watchdog start\n");

	return 0;
}

U_BOOT_CMD(
	tcc_ca72_ckc_disable_wdt_reset, 2, 1, do_tcc_ca72_ckc_wdt_rst_disable,
	"ca72 ckc wdt reset disable", ""
	);

U_BOOT_CMD(
	tcc_ca53_ckc_disable_wdt_reset, 2, 1, do_tcc_ca53_ckc_wdt_rst_disable,
	"ca53 ckc wdt reset disable", ""
	);
