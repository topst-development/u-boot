// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <command.h>
#include <common.h>
#include <dm/uclass.h>
#include <dm.h>
#include <linux/io.h>
#include <irq.h>
#include <mach/clock.h>
#include <asm/arch/timer_api.h>

static int do_tcc_wdt_start(cmd_tbl_t *cmdtp, int flag,
			    int argc, char * const argv[])
{
	struct udevice *dev;

	pr_info("cmd - tcc_cb_wdt_start :: start CPU Watchdog\n");
	uclass_foreach_dev_probe(UCLASS_WDT, dev);
	tcc_wdt_initiation();
}

static int do_tcc_wdt_stop(cmd_tbl_t *cmdtp, int flag,
			   int argc, char * const argv[])
{
	pr_info("cmd - tcc_cb_wdt_stop :: stop CPU Watchdog\n");
	tcc_wdt_halt();
}

U_BOOT_CMD(
	   tcc_cb_wdt_start, 2, 1, do_tcc_wdt_start, "watchdog start", ""
	  );

U_BOOT_CMD(
	   tcc_cb_wdt_stop, 2, 1, do_tcc_wdt_stop, "watchdog stop", ""
	  );


// for PMU Watchdog verification
static int do_tcc_pmu_wdt_start(void)
{
	pr_info("cmd - tcc_pmu_wdt_start :: start PMU Watchdog\n");
	tcc_pmu_wdt_initiation();
}

static int do_tcc_pmu_wdt_stop(void)
{
	pr_info("cmd - tcc_pmu_wdt_stop :: stop PMU Watchdog\n");
	tcc_pmu_wdt_halt();
}

static int do_tcc_pmu_wdt_reset(void)
{
	pr_info("cmd - tcc_pmu_wdt_reset :: reset PMU Watchdog\n");
	tcc_pmu_wdt_resetting();
}

U_BOOT_CMD(
	   tcc_pmu_wdt_start, 2, 1, do_tcc_pmu_wdt_start,
	   "PMU watchdog start", ""
	  );

U_BOOT_CMD(
	   tcc_pmu_wdt_stop, 2, 1, do_tcc_pmu_wdt_stop, "PMU watchdog stop", ""
	  );

U_BOOT_CMD(
	   tcc_pmu_wdt_resetting, 2, 1, do_tcc_pmu_wdt_reset,
	   "PMU watchdog reset", ""
	  );

static void wdt_timer_test(void)
{
	unsigned long current_time = timer_get_boot_us()/1000U;

	pr_info("[tcc_strg_wdt] timer operating. current_time(ms):
	       %lu\n", current_time);
}

void tcc_strg_wdt_kick_handler(void *arg)
{
	unsigned long current_time = timer_get_boot_us()/1000U;

	pr_info("[tcc_strg_wdt] Kicked. current_time(ms): %lu\n", current_time);
	writel(0x1, 0x1D2C3004);
}
// for verification
static int do_tcc_strg_reset_cmd(void)
{
	struct tcc_timer *tt_timer;

	writel(0x85808A85, 0x1d2c1010);
	/*set bus clock into 380 Mhz*/
	writel(0xb18fb386, 0x1d2c1030);

	/* PMU access register -> access only CA53, CA72*/
	writel(0x37f, 0x1b936254);
	/* disable wdt reset mask */
	writel(0x0, 0x14400130);
	// rst cnt
	writel(0x1C9C3800, 0x1d2c300c);
	// irq cnt
	writel(0x07270E00, 0x1d2c3008);
	// reset req enable
	writel(0x800, 0x144000BC);
	// enable
	writel(0x1, 0x1d2c3000);

	tt_timer = timer_register(1000000, (void)*wdt_timer_test, NULL);
	timer_enable(tt_timer);
}

static int do_tcc_strg_kick_cmd(void)
{
	writel(0x85808A85, 0x1d2c1010);
	/*set bus clock into 380 Mhz*/
	writel(0xb18fb386, 0x1d2c1030);

	writel(0x37f, 0x1b936254);
	writel(0x0, 0x14400130);
	// rst cnt
	writel(0x1C9C3800, 0x1d2c300c);
	// irq cnt
	writel(0x07270E00, 0x1d2c3008);
	// reset req enable
	writel(0x800, 0x144000BC);
	irq_install_handler(327 + 32, tcc_strg_wdt_kick_handler, NULL);
	irq_unmask(327 + 32);
	// enable
	writel(0x1, 0x1d2c3000);
}

U_BOOT_CMD(
	   tcc_strg_reset_cmd, 2, 1, do_tcc_strg_reset_cmd, "strg wdt reset", ""
	  );

U_BOOT_CMD(
	   tcc_strg_kick_cmd, 2, 1, do_tcc_strg_kick_cmd, "strg wdt kick", ""
	  );

static void cbus_wdt_timer_test(void)
{
	unsigned long current_time = timer_get_boot_us()/1000U;

	pr_info("[tcc_cbus_wdt] timer operating. current_time(ms): %lu\n",
	       current_time);
}


// for verification
static int do_tcc_cbus_wdt_reset_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
				     char * const argv[])
{
	int i;
	unsigned int cwdt_base = 0x17010000;
	unsigned int cwdt_offset = 0x10000;
	unsigned int req_en_bit = 0x1;
	struct tcc_timer *tt_timer;

	if (argc < 2) {
		pr_err("input argument.\n");
		return -1;
	}

	tcc_set_clkctrl(FBUS_CBUS, CKC_ENABLE, 400000000);
	tcc_set_peri(PERI_CB_WDT, CKC_ENABLE, 400000000, 0);

	i = (*argv[1] - '0');

	if (i >= 5)
		req_en_bit = req_en_bit << (i+3);
	else
		req_en_bit = req_en_bit << i;
	/* PMU access register -> access only CA53, CA72*/
	writel(0x37f, 0x1b936254);
	/* disable wdt reset mask */
	writel(0x0, 0x14400130);

	// rst cnt
	writel(0xEE6B2800, 0x1701000c + (i * cwdt_offset));
	// irq cnt
	writel(0x47868C00, 0x17010008 + (i * cwdt_offset));
	// reset req enable
	writel(req_en_bit, 0x144000BC);
	// enable
	writel(0x1, cwdt_base + (i * cwdt_offset));

	tt_timer = timer_register(1000000, (void)*cbus_wdt_timer_test, NULL);
	timer_enable(tt_timer);
}

void tcc_cbus_wdt_kick_handler(void *arg)
{
	unsigned int cwdt_base = 0x17010000;
	unsigned int cwdt_offset = 0x10000;
	int val = *(int *)arg;
	unsigned int rl = 0x1;
	unsigned long current_time = timer_get_boot_us()/1000U;

	pr_info("CBUS WDT Kicked %d. current time : %lu\n", val, current_time);
	writel(0x1, cwdt_base + (cwdt_offset * val) + 0x4);

	writel(readl(0x1700001C) & ~(rl << val), 0x1700001C);
}

static int do_tcc_cbus_wdt_kick_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
				    char * const argv[])
{
	// WDT0 test
	static int i;
	unsigned int cwdt_base = 0x17010000;
	unsigned int cwdt_offset = 0x10000;
	unsigned int cwdt_irq[8] = {182, 183, 184, 185, 186, 162, 163, 164};
	unsigned int req_en_bit = 0x1;

	if (argc < 2) {
		pr_err("input argument.\n");
		return -1;
	}
	i = (*argv[1] - '0');

	if (i >= 5)
		req_en_bit = req_en_bit << (i+3);
	else
		req_en_bit = req_en_bit << i;

	// set peri clock
	tcc_set_clkctrl(FBUS_CBUS, CKC_ENABLE, 400000000);
	tcc_set_peri(PERI_CB_WDT, CKC_ENABLE, 400000000, 0);

	writel(0x37f, 0x1b936254);
	writel(0x0, 0x14400130);
	// rst cnt - 10s
	writel(0xEE6B2800, 0x1701000c + (i * cwdt_offset));
	// irq cnt - 3s
	writel(0x47868C00, 0x17010008 + (i * cwdt_offset))
	pr_info("cbus %d wdt reg write completed.\n", i);

	req_en_bit = req_en_bit << i;
	// reset req enable
	writel(req_en_bit, 0x144000BC);
	irq_install_handler(cwdt_irq[i] + 32,
			    tcc_cbus_wdt_kick_handler, (void)*(&i));
	irq_unmask(cwdt_irq[i] + 32);
	// kick
	writel(0x1, cwdt_base + (i * cwdt_offset) + 0x4);
	// enable
	writel(0x1, cwdt_base + (i * cwdt_offset));
}

U_BOOT_CMD(
	   tcc_cbus_wdt_reset_cmd, 2, 1, do_tcc_cbus_wdt_reset_cmd,
	   "cbus wdt reset", ""
	  );

U_BOOT_CMD(
	   tcc_cbus_wdt_kick_cmd, 2, 1, do_tcc_cbus_wdt_kick_cmd,
	   "cbus wdt kick", ""
	  );

static int do_tcc_cbus_wdt4_cluster_reset(void)
{
	unsigned int cpu_wdt_rst_dis_base = 0x17000000 + 0x38;
	unsigned int wdt4_base = 0x17050000;
	unsigned int wdt4_monitor_base = wdt4_base + 0x10;
	unsigned int wdt4_halt_val_base = wdt4_base + 0x18;
	unsigned int wdt4_release_val_base = wdt4_base + 0x1c;
	/*
	 *  writel(0x37f, 0x1b936254);
	 *  writel(0x0, 0x14400130);
	 *  writel(0x10, 0x144000BC);
	 */

	tcc_set_clkctrl(FBUS_CBUS, CKC_ENABLE, 400000000);
	tcc_set_peri(PERI_CB_WDT, CKC_ENABLE, 400000000, 0);

	/*set WDTRSTDIS[1] = 0*/
	writel(readl(cpu_wdt_rst_dis_base)
	       & ~(0x2), cpu_wdt_rst_dis_base);
	/* set WDT HALT VALUE into 0xFFFF */
	writel(0xFFFF, wdt4_halt_val_base);
	/* set WDT MONITOR 'BUS_STOP' field = 1 */
	writel(0x3, wdt4_monitor_base);
	/* set release value(CVALUE) into 0xFF */
	writel(0xFF, wdt4_release_val_base);
	/* set WDT RST CVALUE */
	writel(0x00028000, wdt4_base + 0xC);
	/* set WDT CVALUE */
	writel(0x0000C000, wdt4_base + 0x8);
	/* set WDT ENABLE bit */
	writel(0x1, wdt4_base);

	// write to set only register.
	writel(0x53, 0x17000000 + 0x18);
}

U_BOOT_CMD(
	   tcc_cbus_wdt4_cluster_reset_cmd, 2, 1,
	   do_tcc_cbus_wdt4_cluster_reset, "cbus wdt4 cluster reset", ""
	  );


static int do_tcc_cbus_wdt0_cluster_reset(void)
{
	unsigned int cpu_wdt_rst_dis_base = 0x17000000 + 0x38;
	unsigned int wdt0_base = 0x17010000;
	unsigned int wdt0_monitor_base = wdt0_base + 0x10;
	unsigned int wdt0_halt_val_base = wdt0_base + 0x18;
	unsigned int wdt0_release_val_base = wdt0_base + 0x1c;
	/*
	 * writel(0x37f, 0x1b936254);
	 * writel(0x0, 0x14400130);
	 * writel(0x1, 0x144000BC);
	 */

	tcc_set_clkctrl(FBUS_CBUS, CKC_ENABLE, 400000000);
	tcc_set_peri(PERI_CB_WDT, CKC_ENABLE, 400000000, 0);

	writel(readl(cpu_wdt_rst_dis_base) & ~(0x1), cpu_wdt_rst_dis_base);
	writel(0xFFFF, wdt0_halt_val_base);
	writel(0x3, wdt0_monitor_base);
	writel(0xFF, wdt0_release_val_base);

	writel(0x00028000, wdt0_base + 0xC);
	writel(0x0000C000, wdt0_base + 0x8);
	writel(0x1, wdt0_base);

	// write to set only register.
	writel(0x72, 0x17000000 + 0x18);
}

U_BOOT_CMD(
	   tcc_cbus_wdt0_cluster_reset_cmd, 2, 1,
	   do_tcc_cbus_wdt0_cluster_reset, "cbus wdt0 cluster reset", ""
	  );

void tcc_cbus_wdt4_rst_release_ca72_handler(void *arg)
{
	pr_info("Called : %s\n", __func__);
	/* write signal on WDT_SYSACK in CA53 */
	writel(0x1, 0x17050000 + 0x14);
}

static int do_tcc_cbus_ca72_register_handler(void)
{
	irq_install_handler(371 + 32,
			    tcc_cbus_wdt4_rst_release_ca72_handler, NULL);
	irq_unmask(371 + 32);
	pr_info("Register ca72 handler completed.\n");
}

static int do_tcc_cbus_wdt4_cluster_reset_handling(void)
{
	unsigned int cpu_wdt_rst_dis_base = 0x17000000 + 0x38;
	unsigned int wdt4_base = 0x17050000;
	unsigned int wdt4_monitor_base = wdt4_base + 0x10;
	unsigned int wdt4_halt_val_base = wdt4_base + 0x18;
	unsigned int wdt4_release_val_base = wdt4_base + 0x1c;

	tcc_set_clkctrl(FBUS_CBUS, CKC_ENABLE, 400000000);
	tcc_set_peri(PERI_CB_WDT, CKC_ENABLE, 400000000, 0);

	writel(readl(cpu_wdt_rst_dis_base) & ~(0x2), cpu_wdt_rst_dis_base);
	writel(0xFFFF, wdt4_halt_val_base);
	/* bus stop, system stop */
	writel(0x3, wdt4_monitor_base);
	writel(0xFF, wdt4_release_val_base);

	writel(0x00028000, wdt4_base + 0xC);
	writel(0x0000C000, wdt4_base + 0x8);
	writel(0x1, wdt4_base);

	// write to set only register.
	writel(0x53, 0x17000000 + 0x18);
}

U_BOOT_CMD(
	   tcc_cbus_wdt4_cluster_rst_handler, 2, 1,
	   do_tcc_cbus_ca72_register_handler,
	   "cbus wdt4 cluster reset handling", ""
	  );

U_BOOT_CMD(
	   tcc_cbus_wdt4_cluster_rst, 2, 1,
	   do_tcc_cbus_wdt4_cluster_reset_handling,
	   "cbus wdt4 cluster reset handling", ""
	  );

void tcc_cbus_wdt0_rst_release_ca53_handler(void *arg)
{
	pr_info("Called : %s\n", __func__);
	/* write signal on WDT_SYSACK in CA72 */
	writel(0x1, 0x17010000 + 0x14);
}

static int do_tcc_cbus_ca53_register_handler(void)
{
	irq_install_handler(368 + 32,
			    tcc_cbus_wdt0_rst_release_ca53_handler, NULL);
	irq_unmask(368 + 32);
	pr_info("Register ca53 handler completed.\n");
}

static int do_tcc_cbus_wdt0_cluster_reset_handling(void)
{
	unsigned int cpu_wdt_rst_dis_base = 0x17000000 + 0x38;
	unsigned int wdt0_base = 0x17010000;
	unsigned int wdt0_monitor_base = wdt0_base + 0x10;
	unsigned int wdt0_halt_val_base = wdt0_base + 0x18;
	unsigned int wdt0_release_val_base = wdt0_base + 0x1c;

	tcc_set_clkctrl(FBUS_CBUS, CKC_ENABLE, 400000000);
	tcc_set_peri(PERI_CB_WDT, CKC_ENABLE, 400000000, 0);

	writel(readl(cpu_wdt_rst_dis_base) & ~(0x1), cpu_wdt_rst_dis_base);
	writel(0xFFFF, wdt0_halt_val_base);
	writel(0x3, wdt0_monitor_base);
	writel(0xFF, wdt0_release_val_base);

	writel(0x00028000, wdt0_base + 0xC);
	writel(0x0000C000, wdt0_base + 0x8);
	writel(0x1, wdt0_base);

	// write to set only register.
	writel(0x72, 0x17000000 + 0x18);
}

U_BOOT_CMD(
	   tcc_cbus_wdt0_cluster_rst_handler, 2, 1,
	   do_tcc_cbus_ca53_register_handler,
	   "cbus wdt0 cluster reset handling", ""
	  );

U_BOOT_CMD(
	   tcc_cbus_wdt0_cluster_rst, 2, 1,
	   do_tcc_cbus_wdt0_cluster_reset_handling,
	   "cbus wdt0 cluster reset handling", ""
	  );

