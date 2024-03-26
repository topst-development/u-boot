// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <bootstage.h>
#include <mach/timer_api.h>
#include <linux/err.h>
#include <linux/delay.h>

#define TEST_TIMER_NAME  "test_timer"

static unsigned long s_time = 0;

static int do_test_udelay(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int usecs;
	ulong time;
	int i;

	if (argc != 2)
		return CMD_RET_USAGE;

	usecs = simple_strtoul(argv[1], NULL, 0);

	(void)pr_info("[INFO][%s] %s: ######## usec: %d ########\n", TEST_TIMER_NAME, __func__, usecs);

	for(i=0; i<5; i++)
	{
		time = timer_get_boot_us();
		udelay(usecs);
		(void)pr_info("[INFO][%s] %d: udelay(%lu)\n", TEST_TIMER_NAME, i, timer_get_boot_us() - time);
		time = timer_get_boot_us();
		udelay(usecs);
		(void)pr_info("[INFO][%s] %d: udelay(%lu)\n", TEST_TIMER_NAME, i, timer_get_boot_us() - time);
		time = timer_get_boot_us();
		udelay(usecs);
		(void)pr_info("[INFO][%s] %d: udelay(%lu)\n", TEST_TIMER_NAME, i, timer_get_boot_us() - time);
		time = timer_get_boot_us();
		udelay(usecs);
		(void)pr_info("[INFO][%s] %d: udelay(%lu)\n", TEST_TIMER_NAME, i, timer_get_boot_us() - time);
		time = timer_get_boot_us();
		udelay(usecs);
		(void)pr_info("[INFO][%s] %d: udelay(%lu)\n", TEST_TIMER_NAME, i, timer_get_boot_us() - time);

		time = timer_get_boot_us();
		udelay(usecs+1U);
		(void)pr_info("[INFO][%s] %d: udelay(%lu)\n", TEST_TIMER_NAME, i, timer_get_boot_us() - time);
		time = timer_get_boot_us();
		udelay(usecs+1U);
		(void)pr_info("[INFO][%s] %d: udelay(%lu)\n", TEST_TIMER_NAME, i, timer_get_boot_us() - time);
		time = timer_get_boot_us();
		udelay(usecs+2U);
		(void)pr_info("[INFO][%s] %d: udelay(%lu)\n", TEST_TIMER_NAME, i, timer_get_boot_us() - time);
		time = timer_get_boot_us();
		udelay(usecs+2U);
		(void)pr_info("[INFO][%s] %d: udelay(%lu)\n", TEST_TIMER_NAME, i, timer_get_boot_us() - time);
		time = timer_get_boot_us();
		udelay(usecs);
		(void)pr_info("[INFO][%s] %d: udelay(%lu)\n", TEST_TIMER_NAME, i, timer_get_boot_us() - time);

		time = timer_get_boot_us();
		udelay(10000);
		(void)pr_info("[INFO][%s] %d: udelay(%lu)\n", TEST_TIMER_NAME, i, timer_get_boot_us() - time);

		time = timer_get_boot_us();
		udelay(20000);
		(void)pr_info("[INFO][%s] %d: udelay(%lu)\n", TEST_TIMER_NAME, i, timer_get_boot_us() - time);
	}

	return CMD_RET_SUCCESS;
}


static void my_timer_test(void)
{
	unsigned long current_time = timer_get_boot_us()/1000U;
	(void)pr_info("[INFO][%s] %s: current_time(ms): %lu, gap(ms): %lu\n", TEST_TIMER_NAME, __func__, current_time, (s_time!=0U)?current_time-s_time:0U);
	s_time = current_time;
}


static int do_test_timer(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	int res = CMD_RET_SUCCESS;

	static struct tcc_timer *test_timer = NULL;

	if (!strcasecmp("start", argv[1]) && (argc == 3)) {
		if (test_timer == NULL) {
			unsigned long usecs = simple_strtoul(argv[2], NULL, 0);
			s_time = 0;

			test_timer = timer_register(usecs, (void*)my_timer_test, NULL);
			if (test_timer == NULL) {
				(void)pr_warn("[WARN][%s] %s: Failed to register timer.\n", TEST_TIMER_NAME, __func__);
				res = CMD_RET_FAILURE;
			} else {
				if (timer_enable(test_timer) != 0) {
					(void)pr_warn("[WARN][%s] %s: Failed to enable timer.\n", TEST_TIMER_NAME, __func__);
				}
			}
		} else {
			(void)pr_warn("[WARN][%s] %s: Timer test is already in progress.\n", TEST_TIMER_NAME, __func__);
		}
	} else if (!strcasecmp("stop", argv[1]) && (argc == 2)) {
		if (timer_disable(test_timer) != 0) {
			(void)pr_warn("[WARN][%s] %s: Failed to disable timer.\n", TEST_TIMER_NAME, __func__);
		}
		timer_unregister(test_timer);
		s_time = 0;
		test_timer = NULL;
	} else {
		return CMD_RET_USAGE;
	}

	return res;
}

static struct cmd_tbl cmd_timer_test[] = {
	U_BOOT_CMD_MKENT(udelay, 2, 0, do_test_udelay, "", ""),
	U_BOOT_CMD_MKENT(timer, 3, 0, do_test_timer, "", ""),
};

static int do_timer_testops(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_timer_test, ARRAY_SIZE(cmd_timer_test));

	/* Drop the first command */
	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	timer_test, CONFIG_SYS_MAXARGS, 0, do_timer_testops,
	"tcc timer test",
	"udelay <usecs>\n"
	"timer_test timer start <usecs>\n"
	"timer_test timer stop\n"
	);

