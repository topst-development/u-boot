// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <mapmem.h>
#include <mmc.h>
#include <malloc.h>
#include <sdhci.h>
#include <dm.h>
#include <rand.h>
//#include <mach/tcc_sdmmc.h>
#include <asm/cache.h>

/* Default size of a block */
#define TCC_MMC_TUNING_BLOCK_SZ		(0x200u)
/* Number of transfer for one case */
#define TCC_MMC_TUNING_TEST_ITER_NUM	(0x50)
#define TCC_MMC_TUNING_TEST_BLK_NUM	(0x100)
#define DO_TCC_MMC_TUNING_NUM_ARGUMENT	(10)
#define TCC_SDHC_MAX_TAP		(0x1f)
#define TCC_SDHC_HS400_RX_MAX_TAP	(0xF)

#define DRV_NAME "tcc_mmc_tune"

struct tcc_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	void *base;
	void *chctrl_base;
	void *chclkdly_base;
	void *chcddly_base;
	uint32_t max_clk;
	uint32_t peri_clk;
	int32_t index;
	int32_t bus_width;

	uint32_t taps[4];
	uint32_t hs400_pos_tap;
	uint32_t hs400_neg_tap;

	bool increase_skew;
};

struct tcc_tune {
	int curr_dev;
	void *addr;
	unsigned long offset;
	unsigned long cnt;
	unsigned long iter_num;
#define PATT_MANUAL 0UL
#define PATT_RANDOM 1UL
#define PATT_WORST  2UL
	unsigned long rnd;
	uint32_t tx_only;
	uint32_t min_tap1;
	uint32_t max_tap1;
	uint32_t min_tap2;
	uint32_t max_tap2;
	char name_tap1[20];
	char name_tap2[20];
	int (*change_config_func)(unsigned int, unsigned int, struct mmc *);
};

static void advance_cursor(void)
{
	static int pos;
	char cursor[4] = {'/', '-', '\\', '|'};

	pr_force("%c\b", cursor[pos]);
	pos = (pos+1) % 4;
}

static int tcc_do_test(struct tcc_tune *tune,
		lbaint_t start, void *write_pattern, void *read_pattern,
		uint32_t tap1, uint32_t tap2,
		uint32_t pattern_size)
{
	struct mmc *mmc;
	int ret = 0, tmp;
	unsigned int i, rand_n, j;
	unsigned long n;
	const char *name = DRV_NAME;
	lbaint_t size, addr;

	size = (lbaint_t) pattern_size / TCC_MMC_TUNING_BLOCK_SZ;

	/* Init mmc */
	mmc = find_mmc_device(tune->curr_dev);
	mmc->has_init = 0;

	tmp = sdhci_probe(mmc->dev);
	if (tmp != 0) {
		pr_err("[ERROR] %s: failed to probe\n", name);
		return 0;
	}
	tune->change_config_func(tap1, tap2, mmc);

	tmp = mmc_init(mmc);
	if (tmp != 0) {
		pr_err("[ERROR] %s: failed mmc init\n", name);
		return 0;
	}

	addr = start;
	for (i = 0; i < tune->iter_num; i++) {
		advance_cursor();

		/* Random Pattern */
		if (tune->rnd == PATT_RANDOM) {
			srand(0xDEADBEEF);

			for (j = 0; j < pattern_size;
				j += sizeof(unsigned int)) {
				rand_n = rand();
				memcpy((write_pattern + j), &rand_n,
					sizeof(unsigned int));
			}
		}

		n = blk_dwrite(mmc_get_blk_desc(mmc),
				addr, size, write_pattern);
		if (n != size) {
			pr_err("failed to write\n");
			return ret;
		}

		if (tune->tx_only == 0u) {
			memset(read_pattern, 0x0, pattern_size);
			n = blk_dread(mmc_get_blk_desc(mmc),
					addr, size, read_pattern);
			if (n != size) {
				pr_err("failed to read\n");
				return ret;
			}

			if (memcmp(read_pattern,
				write_pattern, pattern_size) != 0) {
				pr_err("failed to compare data\n");
				return ret;
			}
		}

		addr = start + i;
	}
	ret = 1;

	return ret;
}

static void print_result(int **result,
		unsigned int min_x, unsigned int max_x,
		unsigned int min_y, unsigned int max_y)
{
	unsigned int x, y;

	pr_force("    ");
	for (x = min_x ; x < max_x ; x++) {
		if (x < 10u)
			pr_force("%d  ", x);
		else
			pr_force("%d ", x);
	}
	pr_force("\n");

	for (y = min_y ; y < max_y ; y++) {
		if (y < 10u)
			pr_force("%d | ", y);
		else
			pr_force("%d| ", y);

		for (x = min_x ; x < max_x ; x++) {
			if (result[x][y] == 0) {
				pr_force("-  ");
			} else {
				pr_force("0  ");
			}
		}
		pr_force("\n");
	}
	pr_force("    ");
	for (x = min_x ; x < max_x ; x++) {
		if (x < 10u)
			pr_force("%d  ", x);
		else
			pr_force("%d ", x);
	}
	pr_force("\n");
}

static int tcc_tx_tap_change_func(
		unsigned int otap, unsigned int clk_tap, struct mmc *mmc)
{
	struct tcc_sdhci_plat *plat = dev_get_plat(mmc->dev);

	plat->taps[0] = otap;
	plat->taps[3] = clk_tap;

	return 0;
}

static int tcc_hs400_rx_tap_change_func(
		unsigned int pos_tap, unsigned int neg_tap, struct mmc *mmc)
{
	struct tcc_sdhci_plat *plat = dev_get_plat(mmc->dev);

	plat->hs400_pos_tap = pos_tap;
	plat->hs400_neg_tap = neg_tap;

	return 0;
}

static int do_find_window(struct tcc_tune *tune)
{
	int i, j, cmd_ret, tmp;
	uint32_t tap1, tap2;
	int **result;
	lbaint_t start, blk_num;
	uint32_t pattern_size;
	void *write_pattern, *read_pattern;
	unsigned long spend_time, total_time;
	const char *name = DRV_NAME;

	start = tune->offset;
	blk_num = tune->cnt;
	pattern_size = TCC_MMC_TUNING_BLOCK_SZ * (unsigned int)blk_num;
	cmd_ret = (int)CMD_RET_FAILURE;
	total_time = 0;

	result = (int **)memalign(ARCH_DMA_MINALIGN,sizeof(int *) * tune->max_tap1);
	if (!result) {
		pr_err("[ERROR] %s: failed to alloc mem - result\n", name);
		return (int)CMD_RET_FAILURE;
	}

	for (i = 0; (unsigned int)i < tune->max_tap1; i++) {
		result[i] = (int *)memalign(ARCH_DMA_MINALIGN ,sizeof(int) * tune->max_tap2);
		j = i;
		if (!result[j]) {
			pr_err("[ERROR] %s: failed to alloc mem - result[%d]\n",
					name, j);
			for (j--; j >= 0; j--)
				free(result[j]);
			goto error2;
		}
	}

	write_pattern = tune->addr;
	if (!write_pattern) {
		pr_err(
			"[ERROR] %s: failed to alloc mem - write_pattern\n",
			name);
		goto error;
	}
	read_pattern = tune->addr + pattern_size;
	if (!read_pattern) {
		pr_err(
			"[ERROR] %s: failed to alloc mem - read_pattern\n",
			name);
		goto error;
	}

	pr_force("##### Window Test Start ######\n");
	pr_force("Tap1(X) (%s) min %d max %d\n",
			tune->name_tap1, tune->min_tap1, tune->max_tap1);
	pr_force("Tap2(Y) (%s) min %d max %d\n",
			tune->name_tap2, tune->min_tap2, tune->max_tap2);
	if (tune->rnd == PATT_RANDOM)
		pr_force("Random pattern test\n");
	else if (tune->rnd == PATT_WORST)
		pr_force("Worst pattern test\n");
	else
		pr_force("Manual pattern test\n");
	pr_force("lba start 0x%lx iter for one test 0x%lx tx only %d\n",
			tune->offset, tune->iter_num, tune->tx_only);
	pr_force(
		"write pattern addr 0x%p read pattern addr 0x%p pattern size 0x%x\n",
			write_pattern, read_pattern, pattern_size);

	for (tap1 = tune->min_tap1; tap1 < tune->max_tap1; tap1++) {
		for (tap2 = tune->min_tap2; tap2 < tune->max_tap2; tap2++) {
			pr_force("TEST [%s %d %s %d]\n", tune->name_tap1, tap1,
					tune->name_tap2, tap2);

			spend_time = get_timer(0);
			result[tap1][tap2] = tcc_do_test(tune, start,
					write_pattern, read_pattern,
					tap1, tap2,
					pattern_size);
			spend_time = get_timer(spend_time);
			if (spend_time > 0UL) {
				pr_force("Time: %lu msec\n", spend_time);
				total_time += spend_time;
			}

			if (result[tap1][tap2] != 0)
				pr_force(": success\n\n");
			else
				pr_force("\n");

			tmp = ctrlc();
			if (tmp == 1) {
				cmd_ret = (int)CMD_RET_SUCCESS;
				goto error;
			}
		}
	}

	pr_force("##### Window Test Finish ######\n");
	pr_force("Test time: %lu msec\n", total_time);
	pr_force("Tap1(X) (%s) min %d max %d\n",
			tune->name_tap1, tune->min_tap1, tune->max_tap1-1u);
	pr_force("Tap2(Y) (%s) min %d max %d\n",
			tune->name_tap2, tune->min_tap2, tune->max_tap2-1u);
	if (tune->rnd == PATT_RANDOM)
		pr_force("Random pattern test\n");
	else if (tune->rnd == PATT_WORST)
		pr_force("Worst pattern test\n");
	else
		pr_force("Manual pattern test\n");
	pr_force("lba start 0x%lx iter for one test 0x%lx tx only %x\n",
			tune->offset, tune->iter_num, tune->tx_only);
	pr_force(
		"write pattern addr 0x%p read pattern addr 0x%p pattern size 0x%x\n",
			write_pattern, read_pattern, pattern_size);
	print_result(result, tune->min_tap1, tune->max_tap1,
			tune->min_tap2, tune->max_tap2);

	cmd_ret = (int)CMD_RET_SUCCESS;

error:
	for (i = 0; (unsigned int)i < tune->max_tap1; i++)
		free(result[i]);
error2:
	free(result);

	return cmd_ret;
}

static int do_tx_tuning(struct cmd_tbl *cmdtp, int flag,
		int argc, char * const argv[])
{
	struct tcc_tune tune;
	const char *tmp_tap1 = "otapdelay";
	const char *tmp_tap2 = "tx clk";
	const char *name = DRV_NAME;

	if (argc == 5) {
		tune.addr = (void *)simple_strtoul(argv[1], NULL, 16);
		tune.offset = simple_strtoul(argv[2], NULL, 16);
		tune.cnt = simple_strtoul(argv[3], NULL, 16);
		tune.iter_num = simple_strtoul(argv[4], NULL, 16);
		tune.rnd = 0;
	} else if (argc == 6) {
		tune.addr = (void *)simple_strtoul(argv[1], NULL, 16);
		tune.offset = simple_strtoul(argv[2], NULL, 16);
		tune.cnt = simple_strtoul(argv[3], NULL, 16);
		tune.iter_num = simple_strtoul(argv[4], NULL, 16);
		tune.rnd = simple_strtoul(argv[5], NULL, 16);
	} else {
		return CMD_RET_USAGE;
	}

	tune.curr_dev = 0;
	tune.min_tap1 = 0;
	tune.max_tap1 = TCC_SDHC_MAX_TAP + 1;
	strncpy(tune.name_tap1, tmp_tap1, 9);

	tune.min_tap2 = 0;
	tune.max_tap2 = TCC_SDHC_MAX_TAP + 1;

	strncpy(tune.name_tap2, tmp_tap2, 6);
	tune.tx_only = 0;
	tune.change_config_func = tcc_tx_tap_change_func;
	if (tune.iter_num == 0UL) {
		pr_warn("[WARN] %s: iter_num zero. default is one\n", name);
		tune.iter_num = 1;
	}

	return do_find_window(&tune);
}

static int do_hs400_rx(struct cmd_tbl *cmdtp, int flag,
		       int argc, char * const argv[])
{
	struct tcc_tune tune;
	const char *tmp_tap1 = "pos";
	const char *tmp_tap2 = "neg";

	if (argc == 5) {
		tune.addr = (void *)simple_strtoul(argv[1], NULL, 16);
		tune.offset = simple_strtoul(argv[2], NULL, 16);
		tune.cnt = simple_strtoul(argv[3], NULL, 16);
		tune.iter_num = simple_strtoul(argv[4], NULL, 16);
		tune.rnd = 0;
	} else if (argc == 6) {
		tune.addr = (void *)simple_strtoul(argv[1], NULL, 16);
		tune.offset = simple_strtoul(argv[2], NULL, 16);
		tune.cnt = simple_strtoul(argv[3], NULL, 16);
		tune.iter_num = simple_strtoul(argv[4], NULL, 16);
		tune.rnd = simple_strtoul(argv[5], NULL, 16);
	} else {
		return CMD_RET_USAGE;
	}

	tune.curr_dev = 0;
	tune.min_tap1 = 0;
	tune.max_tap1 = TCC_SDHC_HS400_RX_MAX_TAP + 1;

	strncpy(tune.name_tap1, tmp_tap1, 3);

	tune.min_tap2 = 0;
	tune.max_tap2 = TCC_SDHC_HS400_RX_MAX_TAP + 1;

	strncpy(tune.name_tap2, tmp_tap2, 3);
	tune.tx_only = 0;
	tune.change_config_func = tcc_hs400_rx_tap_change_func;
	if (tune.iter_num == 0ul) {
		pr_warn("warning! iter_num zero. default is one\n");
		tune.iter_num = 1;
	}

	return do_find_window(&tune);
}

static struct cmd_tbl cmd_tcc_mmc_tune[] = {
	U_BOOT_CMD_MKENT(hs400_rx, 7, 0, do_hs400_rx, "", ""),
	U_BOOT_CMD_MKENT(tx, 7, 0, do_tx_tuning, "", ""),
};

static int do_tcc_mmc_tuning_ops(struct cmd_tbl *cmdtp, int flag,
				int argc, char * const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_tcc_mmc_tune,
			ARRAY_SIZE(cmd_tcc_mmc_tune));

	/* Drop the tcc mmc tune command */
	argc--;
	argv++;

	if ((cp == NULL) || (argc > cp->maxargs))
		return (int)CMD_RET_USAGE;

	if (!cmd_is_repeatable(cp) && (flag == CMD_FLAG_REPEAT))
		return (int)CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
tcc_mmc_tuning,	DO_TCC_MMC_TUNING_NUM_ARGUMENT,	0, do_tcc_mmc_tuning_ops,
"tcc_mmc_tuning - find tx/rx tap delay window of telechip mmc controller",
"function list\n"
"hs400_rx <addr> <blk> <cnt> <iter_num> <rnd_test>: find tcc803x hs400 rx sampling timing window\n"
"tx <addr> <blk> <cnt> <iter_num> <rnd_test>: find tx delay tap window\n"
);
