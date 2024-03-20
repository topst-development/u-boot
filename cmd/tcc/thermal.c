// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <console.h>
#include <asm/io.h>
#include <mach/smc.h>
#include <asm/system.h>
#include <irq.h>

#define TSENSOR_BASE 0x14700000
#define TSENSOR_IRQ  0x14700038

#define MAIN 0
#define PROBE0 1
#define PROBE1 2
#define PROBE2 3
#define PROBE3 4
#define PROBE4 5
#define ALL 6

#define EN_VRT 4
#define EN_VBT 3
#define EN_CH  2
#define TSENSOR_MODE   0

#define HIGH_TEMP 2552
#define LOW_TEMP  1596

#define TSENSOR_IRQ_NUM (40 + INT_GIC_OFFSET)

int irq_counter, irq_counter2 = 0;

#if defined(CONFIG_TCC805X)
static int code_to_temp(int tsensor_data, unsigned int otp_temp_low,
			unsigned int otp_temp_high)
{
	int temp = 0;

	if (tsensor_data >= otp_temp_low) {
		temp = ((tsensor_data - otp_temp_low) * 60) /
				(otp_temp_high - otp_temp_low) + 25;
	} else if (tsensor_data < otp_temp_low) {
		temp = ((((tsensor_data - otp_temp_low) * 60) /
				(otp_temp_high - otp_temp_low))) *
					(65 / (57 + (6))) + 25;
	} else {
	}
	return temp;
}

static int temp_to_code(int tsensor_data, unsigned int otp_temp_low,
			unsigned int otp_temp_high)
{
	int temp = 0;

	pr_force("[TEMP_TO_CODE] tsensor data = %d\n", tsensor_data);
	if (tsensor_data >= 25) {
		temp = (((tsensor_data - 25) / (6)) *
			(otp_temp_high - otp_temp_low) / 10) + otp_temp_low;
	} else if (tsensor_data < 25) {
		temp = (tsensor_data - 25) * ((57 + (6)) / 65) *
			(otp_temp_high - otp_temp_low) / (85 - 25)
						+ otp_temp_low;
	} else {
	}
	return temp;
}

static int do_read_tsensor(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	uintptr_t tsensor_base_reg = TSENSOR_BASE;
	unsigned int tsensor_data = 0;
	int cal_tsensor_data = 0;
	int i, k = 0;

	writel(0, tsensor_base_reg); //tsensor_disable
	/*need threshold setup*/
	writel((0 << (PROBE4 * 4) | 0 << (PROBE3 * 4) | 0 << (PROBE2 * 4) |
		0 << (PROBE1 * 4) | 0 << (PROBE0 * 4) | 0 << MAIN),
		tsensor_base_reg + 0x2C); //TSENSOR all interrupt non-masking
	writel(1, tsensor_base_reg); // Tsensor enable
	udelay(1000);
	for (i = 0; i < 6; i++) {
		k = i - 1;
		tsensor_data = readl(tsensor_base_reg + 0xD0 + (0x4 * i));
		cal_tsensor_data = (code_to_temp(tsensor_data,
					LOW_TEMP, HIGH_TEMP)) & 0xFFF;
		if (i == 0)
			pr_force("TSENSOR Main Probe = %d\n",
				 cal_tsensor_data);
		else
			pr_force("TSENSOR Probe %d   = %d\n", k,
				 cal_tsensor_data);
	}
	return 0;
}

static int do_tsensor_probe_select(cmd_tbl_t *cmdtp, int flag, int argc,
				   char * const argv[])
{
	uintptr_t tsensor_base_reg = TSENSOR_BASE;
	unsigned int tsensor_probe[6] = {0, };
	int i = 0;

	for (i = 0; i < 6; i++) {
		tsensor_probe[i] = simple_strtoul(argv[i + 1], NULL, 10);
		if ((tsensor_probe[i] != 0 && tsensor_probe[i] != 1)) {
			pr_force("Please input 1 or 0\n");
			pr_force("1 : Enable\n");
			pr_force("0 : Disable\n");
			return -1;
		}
	}

	writel(0, tsensor_base_reg); //tsensor_disable
	writel((tsensor_probe[5] << PROBE4 | tsensor_probe[4] << PROBE3 |
		tsensor_probe[3] << PROBE2 | tsensor_probe[2] << PROBE1 |
		tsensor_probe[1] << PROBE0 | tsensor_probe[0]),
		tsensor_base_reg + 0xC); //tsensor probe select
	pr_force("TSENSOR Probe select = MAIN : %d  PROBE0 : %d PROBE1 : %d",
		 tsensor_probe[0], tsensor_probe[1], tsensor_probe[2]);
	pr_force(" PROBE2 : %d PROBE3 : %d PROBE4 : %d\n",
		 tsensor_probe[3], tsensor_probe[4], tsensor_probe[5]);
	return 0;
}

static int do_tsensor_mode_select(cmd_tbl_t *cmdtp, int flag, int argc,
				  char * const argv[])
{
	uintptr_t tsensor_base_reg = TSENSOR_BASE;
	unsigned int operate_mode = 0;
	unsigned int original_mode = 0;

	operate_mode = simple_strtoul(argv[1], NULL, 10);
	if (operate_mode != 0 && operate_mode != 1) {
		pr_force("Please input 1 or 0\n");
		pr_force("1: Continuous mode\n0: One-shot mode");
		return -1;
	}
	original_mode = readl(tsensor_base_reg + 0x4);
	writel(original_mode | operate_mode, tsensor_base_reg + 0x4);
	if (operate_mode == 0)
		pr_force("Operation mode - One-shot mode\n");
	else
		pr_force("Operation mode - Continuos mode\n");

	return 0;
}

static int do_tsensor_threshold_set(cmd_tbl_t *cmdtp, int flag, int argc,
				    char * const argv[])
{
	uintptr_t tsensor_base_reg = TSENSOR_BASE;
	unsigned int tsensor_threshold_sel = 0;
	int tsensor_threshold_up = 0;
	int tsensor_threshold_down = 0;
	int i, t = 0;

	tsensor_threshold_sel = simple_strtoul(argv[1], NULL, 10);
	tsensor_threshold_up = simple_strtoul(argv[2], NULL, 10);
	tsensor_threshold_down = simple_strtoul(argv[3], NULL, 10);

	if (tsensor_threshold_sel > 6 || tsensor_threshold_sel < 0) {
		pr_force("Please input probe number 0 - 6\n0 : Main\n");
		pr_force("1~5 : Probe0~4\n6 : All probe\n");
		return -1;
	}

	tsensor_threshold_up =
		temp_to_code(tsensor_threshold_up, LOW_TEMP, HIGH_TEMP);
	tsensor_threshold_down =
		temp_to_code(tsensor_threshold_down, LOW_TEMP, HIGH_TEMP);

	writel(0, tsensor_base_reg); //tsensor_disable
	writel((0 << (PROBE4 * 4) | 0 << (PROBE3 * 4) | 0 << (PROBE2 * 4) |
		0 << (PROBE1 * 4) | 0 << (PROBE0 * 4) | 0 << MAIN),
		tsensor_base_reg + 0x24); //tsensor all interrupt disable
	writel((7 << (PROBE4 * 4) | 7 << (PROBE3 * 4) | 7 << (PROBE2 * 4) |
		7 << (PROBE1 * 4) | 7 << (PROBE0 * 4) | 7 << MAIN),
		tsensor_base_reg + 0x4C); //tsensor all interrupt clear
	switch (tsensor_threshold_sel) {
	case MAIN:
		writel(tsensor_threshold_up, tsensor_base_reg + 0x50);
		writel(tsensor_threshold_down, tsensor_base_reg + 0x54);
	case PROBE0:
		writel(tsensor_threshold_up, tsensor_base_reg + 0x58);
		writel(tsensor_threshold_down, tsensor_base_reg + 0x5C);
	case PROBE1:
		writel(tsensor_threshold_up, tsensor_base_reg + 0x60);
		writel(tsensor_threshold_down, tsensor_base_reg + 0x64);
	case PROBE2:
		writel(tsensor_threshold_up, tsensor_base_reg + 0x68);
		writel(tsensor_threshold_down, tsensor_base_reg + 0x6C);
	case PROBE3:
		writel(tsensor_threshold_up, tsensor_base_reg + 0x70);
		writel(tsensor_threshold_down, tsensor_base_reg + 0x74);
	case PROBE4:
		writel(tsensor_threshold_up, tsensor_base_reg + 0x78);
		writel(tsensor_threshold_down, tsensor_base_reg + 0x7C);
	case ALL:
		for (i = 0; i < 6; i++) {
			writel(tsensor_threshold_up,
			       tsensor_base_reg + 0x50 + 0x8 * i);
			writel(tsensor_threshold_down,
			       tsensor_base_reg + 0x54 + 0x8 * i);
		}
	default:
		for (i = 0; i < 6; i++) {
			writel(tsensor_threshold_up,
			       tsensor_base_reg + 0x50 + 0x8 * i);
			writel(tsensor_threshold_down,
			       tsensor_base_reg + 0x54 + 0x8 * i);
		}
	}

	for (i = 0; i < 6; i++) {
		t = i - 1;
		if (i == 0) {
			pr_force("TSENSOR PROBE Threshold = MAIN PROBE Up : %d",
				 tsensor_threshold_up);
			pr_force(" MAIN PROBE Down %d\n",
				 tsensor_threshold_down);
		} else {
			pr_force("TSENSOR PROBE Threshold = PROBE[%d]  Up : %d",
				 t, tsensor_threshold_up);
			pr_force(" PROBE[%d]  Down %d\n",
				 t, tsensor_threshold_down);
		}
	}
	return 0;
}

static void tsensor_irq_request(void *arg)
{
	uintptr_t tsensor_base_reg = TSENSOR_BASE;
	int reg;
	/*set irq*/
	irq_counter = irq_counter + 1;
	if (irq_counter == 7) {
		writel(0, tsensor_base_reg);
		writel(0x777777, tsensor_base_reg + 0x2C);
		irq_counter = 0;
	}
	reg = readl(tsensor_base_reg + 0x34);
	pr_force("0x%x\n", reg);
}

static void tsensor_irq_request2(void *arg)
{
	uintptr_t tsensor_base_reg = TSENSOR_BASE;
	int reg;
	/*set irq*/
	irq_counter2 = irq_counter2 + 1;
	reg = readl(tsensor_base_reg + 0x34);
	pr_force("0x%x\n", reg);
	if (irq_counter2 == 1) {
		writel(0x777777, tsensor_base_reg + 0x2C);
		irq_counter = 0;
	}
}

static int do_tsensor_irq_set(cmd_tbl_t *cmdtp, int flag, int argc,
			      char * const argv[])
{
	uintptr_t tsensor_base_reg = TSENSOR_BASE;
	int en = 0;
	unsigned int tsensor_probe[6] = {0, };
	int i = 0;

	for (i = 0; i < 6; i++) {
		tsensor_probe[i] = simple_strtoul(argv[i + 1], NULL, 10);
		if (tsensor_probe[i] > 7) {
			pr_force("Please input 0 to 7\n");
			return -1;
		}
		if (tsensor_probe[i] != 0)
			en = en + 1;
	}
	writel(0, tsensor_base_reg); // Tsensor disable
	writel(0, tsensor_base_reg + 0x24); //tsensor interrupt all disable
	writel((7 << (PROBE4 * 4) | 7 << (PROBE3 * 4) | 7 << (PROBE2 * 4) |
		7 << (PROBE1 * 4) | 7 << (PROBE0 * 4) | 7 << MAIN),
		tsensor_base_reg + 0x44); //tsensor all interrupt clear
	writel((tsensor_probe[5] << (PROBE4 * 4) |
		tsensor_probe[4] << (PROBE3 * 4) |
		tsensor_probe[3] << (PROBE2 * 4) |
		tsensor_probe[2] << (PROBE1 * 4) |
		tsensor_probe[1] << (PROBE0 * 4) |
		tsensor_probe[0]), tsensor_base_reg + 0x24);
	writel(((~(tsensor_probe[5]) & 0x7) << (PROBE4 * 4) |
		(~(tsensor_probe[4]) & 0x7) << (PROBE3 * 4) |
		(~(tsensor_probe[3]) & 0x7) << (PROBE2 * 4) |
		(~(tsensor_probe[2]) & 0x7) << (PROBE1 * 4) |
		(~(tsensor_probe[1]) & 0x7) << (PROBE0 * 4) |
		(~(tsensor_probe[0]) & 0x7) << MAIN), tsensor_base_reg + 0x2C);
	pr_force("tsensor probe irq set = main : %d  probe0 : %d probe1 : %d",
		 tsensor_probe[0], tsensor_probe[1], tsensor_probe[2]);
	pr_force(" probe2 : %d probe3 : %d probe4 : %d\n",
		 tsensor_probe[3], tsensor_probe[4], tsensor_probe[5]);

	if (en != 0) {
		irq_install_handler(TSENSOR_IRQ_NUM,
				    &tsensor_irq_request, NULL);
		irq_unmask(TSENSOR_IRQ_NUM);
		pr_force("TSENSOR Interrupt mode bit : Enable\n");

	} else {
		irq_mask(TSENSOR_IRQ_NUM);
		irq_free_handler(TSENSOR_IRQ_NUM);
		pr_force("TSENSOR Interrupt mode bit : Disable\n");
	}
	return 0;
}

static int do_tsensor_irq_temp_read(cmd_tbl_t *cmdtp, int flag, int argc,
				    char * const argv[])
{
	uintptr_t tsensor_base_reg = TSENSOR_BASE;
	int i, k, temp = 0;
	int high_temp, low_temp = 0;

	for (i = 0; i < 6; i++) {
		k = i - 1;
		temp = readl(tsensor_base_reg + 0x110 + (0x4 * i));
		high_temp = (temp >> 16) & 0xFFF;
		low_temp = temp & 0xFFF;
		high_temp = code_to_temp(high_temp, LOW_TEMP, HIGH_TEMP);
		low_temp = code_to_temp(high_temp, LOW_TEMP, HIGH_TEMP);
		if (i == 0) {
			pr_force("TSENSOR Main probe IRQ TEMP data");
			pr_force(" HIGH / LOW : %d / %d\n",
				 high_temp, low_temp);
		} else {
			pr_force("TSENSOR Probe %d    IRQ TEMP data", k);
			pr_force(" HIGH / LOW : %d / %d\n",
				 high_temp, low_temp);
		}
	}
	return 0;
}

static int do_tsensor_irq_threshold_test(cmd_tbl_t *cmdtp, int flag, int argc,
					 char * const argv[])
{
	uintptr_t tsensor_base_reg = TSENSOR_BASE;

	writel(0, tsensor_base_reg); // Tsensor enable
	writel(0, tsensor_base_reg + 0x24); //tsensor interrupt all disable
	udelay(1);
	writel((1 << (PROBE4 * 4) | 1 << (PROBE3 * 4) | 1 << (PROBE2 * 4) |
		1 << (PROBE1 * 4) | 1 << (PROBE0 * 4) | 1 << MAIN),
		tsensor_base_reg + 0x44); //tsensor all interrupt clear
	// open irq
	irq_install_handler(TSENSOR_IRQ_NUM, &tsensor_irq_request2, NULL);
	irq_unmask(TSENSOR_IRQ_NUM);
	pr_force("TSENSOR Interrupt handler open\n");
	writel(0, tsensor_base_reg + 0x4);  //One-shot mode
	pr_force("TSENSOR Mode is One-shot\n");
	//Main probe threshold test
	writel((0 << (PROBE4 * 4) | 0 << (PROBE3 * 4) | 0 << (PROBE2 * 4) |
		0 << (PROBE1 * 4) | 0 << (PROBE0 * 4) | 1 << MAIN),
		tsensor_base_reg + 0xC); //Main Probe select
	udelay(1);
	writel((0 << (PROBE4 * 4) | 0 << (PROBE3 * 4) | 0 << (PROBE2 * 4) |
		0 << (PROBE1 * 4) | 0 << (PROBE0 * 4) | 0 << MAIN),
		tsensor_base_reg + 0x2C); //TSENSOR all interrupt non-masking
	udelay(1);
	writel((0 << (PROBE4 * 4) | 0 << (PROBE3 * 4) | 0 << (PROBE2 * 4) |
		0 << (PROBE1 * 4) | 0 << (PROBE0 * 4) | 0 << MAIN),
		tsensor_base_reg + 0x24); //ALL IRQ disable
	udelay(1);
	writel((0 << (PROBE4 * 4) | 0 << (PROBE3 * 4) | 0 << (PROBE2 * 4) |
		0 << (PROBE1 * 4) | 0 << (PROBE0 * 4) | 1 << MAIN),
		tsensor_base_reg + 0x2C); //TSENSOR all interrupt masking
	udelay(1);
	writel((0 << (PROBE4 * 4) | 0 << (PROBE3 * 4) | 0 << (PROBE2 * 4) |
		0 << (PROBE1 * 4) | 0 << (PROBE0 * 4) | 0 << MAIN),
		tsensor_base_reg + 0x2C); //TSENSOR all interrupt non-masking
	udelay(1);
	writel((0 << (PROBE4 * 4) | 0 << (PROBE3 * 4) | 0 << (PROBE2 * 4) |
		0 << (PROBE1 * 4) | 0 << (PROBE0 * 4) | 1 << MAIN),
		tsensor_base_reg + 0x24); //IRQ Enable only Main Probe
	udelay(1);
	writel((0 << (PROBE4 * 4) | 0 << (PROBE3 * 4) | 0 << (PROBE2 * 4) |
		0 << (PROBE1 * 4) | 0 << (PROBE0 * 4) | 1 << MAIN),
		tsensor_base_reg + 0x2C); //TSENSOR all interrupt masking
	udelay(1);
	writel((0 << (PROBE4 * 4) | 0 << (PROBE3 * 4) | 0 << (PROBE2 * 4) |
		0 << (PROBE1 * 4) | 0 << (PROBE0 * 4) | 1 << MAIN),
		tsensor_base_reg + 0x24); //IRQ Enable only Main Probe
	udelay(1);
	return 0;
}
#else

static int do_read_tsensor(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	uintptr_t tsensor_base_reg = TSENSOR_BASE;
	unsigned int tsensor_data = 0;

	writel(0, tsensor_base_reg); //tsensor_disable
	/*need threshold setup*/
	writel((1 << 1) | (1 << 0), tsensor_base_reg); // Tsensor enable
	udelay(100);
	while (tsensor_data == 0)
		tsensor_data = readl(tsensor_base_reg + 0x30);

	pr_force("TSENSOR Parallel Read = 0x%X\n", tsensor_data);
	return 0;
}

static void tsensor_irq_request(void *arg)
{
	/*set irq*/
	pr_force("TSENSOR irq request\n");
	writel(0x1, 0x14700038);
}

static int do_tsensor_irq_set(cmd_tbl_t *cmdtp, int flag, int argc,
			      char * const argv[])
{
	uintptr_t tsensor_base_reg = TSENSOR_BASE;
	int irq_pol, en = 0;
	int irq_number = TSENSOR_IRQ_NUM;

	en = simple_strtoul(argv[1], NULL, 10);

	writel(0, tsensor_base_reg); // Tsensor enable
	irq_pol = readl(0x141000E4);

	if (en == 1) {
		writel((irq_pol) | (1 << 8), 0x141000E4);
		writel(0x2E, tsensor_base_reg + 0x3C); // Threshold value
		writel((1 << 2), tsensor_base_reg);
		irq_install_handler(TSENSOR_IRQ_NUM,
				    &tsensor_irq_request, NULL);
		irq_unmask(TSENSOR_IRQ_NUM);
		pr_force("TSENSOR Interrupt mode bit : Enable\n");

	} else if (en == 0) {
		writel((irq_pol) | (0 << 8), 0x141000E4);
		writel((0 << 2), tsensor_base_reg);
		irq_mask(TSENSOR_IRQ_NUM);
		irq_free_handler(irq_number);
		pr_force("TSENSOR Interrupt mode bit : Disable\n");
	} else {
		pr_force("TSENSOR Interrupt mode bit setup\n");
		pr_force(" Enable : 1\n Disable : 0\n");
	}
	return 0;
}

#endif

U_BOOT_CMD(tsensor_read, 1, 1, do_read_tsensor,
	   "Read tsensor data",
	   "");
#if defined(CONFIG_TCC805X)
U_BOOT_CMD(tsensor_probe_select, 7, 1, do_tsensor_probe_select,
	   "TSENSOR probe select (1: select 0: non-select) - [ MAIN ] [ PROBE0 ] [ PROBE1 ] [ PROBE2 ] [ PROBE3 ] [ PROBE4 ]",
	   "");

U_BOOT_CMD(tsensor_irq_set, 7, 1, do_tsensor_irq_set,
	   "TSENSOR Probe IRQ enable (1: enable 0: disable) - [ MAIN ] [ PROBE0 ] [ PROBE1 ] [ PROBE2 ] [ PROBE3 ] [ PROBE4 ]",
	   "");

U_BOOT_CMD(tsensor_threshold_set, 4, 1, do_tsensor_threshold_set,
	   "TSENSOR IRQ threshold value setting : [PROBE_SELECT] [ UP THRESHOLD TEMPERATURE ] [ DOWN THRESHOLD TEMPERATURE ]",
	   "");

U_BOOT_CMD(first_irq_occur_temp_read, 1, 1, do_tsensor_irq_temp_read,
	   "Read saved IRQ temperature ",
	   "");

U_BOOT_CMD(tsensor_mode_select, 2, 1, do_tsensor_mode_select,
	   "Set TSENSOR operating mode",
	   "");

U_BOOT_CMD(tsensor_irq_threshold_test, 1, 1, do_tsensor_irq_threshold_test,
	   "TSENSOR IRQ Threshold test",
	   "");
#else

U_BOOT_CMD(tsensor_irq_set, 2, 1, do_tsensor_irq_set,
	   "TSENSOR IRQ enable / disable - Set all PROBE IRQ control register",
	   "");
#endif
