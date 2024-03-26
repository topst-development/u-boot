// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef __CAN_RESET_H
#define __CAN_RESET_H

#include <linux/types.h>
#include <linux/printk.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <test/can_porting_chip.h>

union can_rst_addr {
	uint32_t ui_rst_addr;
	void* v_rst_addr;
};

struct can_rst {
	union can_rst_addr rst_addr;
	uint32_t rst_shift;
};

static struct can_rst clken_tcc750x[] = {
	{ { 0x1E00028C }, 21 },		// CAN0
	{ { 0x1E00028C }, 22 },		// CAN1
};

static struct can_rst brst_tcc750x[] = {
	{ { 0x1E000284 }, 21 },		// CAN0
	{ { 0x1E000284 }, 22 },		// CAN1
};

#if (TCC807X_CAN_TEST_CASE == 0)
static struct can_rst clken_tcc807x[] = {
	{ { 0x16051000 }, 27 },		// CAN0
	{ { 0x16051000 }, 26 },		// CAN1
	{ { 0x16051000 }, 25 },		// CAN2
};

static struct can_rst brst_tcc807x[] = {
	{ { 0x1605100C }, 27 },		// CAN0
	{ { 0x1605100C }, 26 },		// CAN1
	{ { 0x1605100C }, 25 },		// CAN2
};
#elif (TCC807X_CAN_TEST_CASE == 1)
static struct can_rst clken_tcc807x[] = {
	{ { 0x16051000 }, 24 },		// CAN3
	{ { 0x16051000 }, 23 },		// CAN4
	{ { 0x16051000 }, 22 },		// CAN5
};

static struct can_rst brst_tcc807x[] = {
	{ { 0x1605100C }, 24 },		// CAN3
	{ { 0x1605100C }, 23 },		// CAN4
	{ { 0x1605100C }, 22 },		// CAN5
};
#elif (TCC807X_CAN_TEST_CASE == 2)
static struct can_rst clken_tcc807x[] = {
	{ { 0x16051000 }, 21 },		// CAN6
	{ { 0x16051000 }, 19 },		// CAN7
	{ { 0x16051000 }, 18 },		// CAN8
};

static struct can_rst brst_tcc807x[] = {
	{ { 0x1605100C }, 21 },		// CAN6
	{ { 0x1605100C }, 19 },		// CAN7
	{ { 0x1605100C }, 18 },		// CAN8
};
#elif (TCC807X_CAN_TEST_CASE == 3)
static struct can_rst clken_tcc807x[] = {
	{ { 0x16051000 }, 17 },		// CAN9
	{ { 0x16051000 }, 16 },		// CAN10
	{ { 0x16051000 }, 15 },		// CAN11
};

static struct can_rst brst_tcc807x[] = {
	{ { 0x1605100C }, 17 },		// CAN9
	{ { 0x1605100C }, 16 },		// CAN10
	{ { 0x1605100C }, 15 },		// CAN11
};

#endif
void can_set(struct can_rst *can_rst)
{
	uint32_t rst_shift, rst_val;
	void* rst_addr;

	rst_addr = can_rst->rst_addr.v_rst_addr;
	rst_val = readl(rst_addr);
	rst_shift = can_rst->rst_shift;
	rst_val |= (1UL << rst_shift);
	writel(rst_val, rst_addr);
}

void can_unset(struct can_rst *can_rst)
{
	uint32_t rst_shift, rst_val;
	void* rst_addr;

	rst_addr = can_rst->rst_addr.v_rst_addr;
	rst_val = readl(rst_addr);
	rst_shift = can_rst->rst_shift;
	rst_val &= ~(1UL << rst_shift);
	writel(rst_val, rst_addr);
}

void can_reset_channel(uint8_t cChannelHandle)
{
	if (IS_ENABLED(CONIFG_TCC750X)) {
		can_set(&clken_tcc750x[cChannelHandle]);
		can_unset(&brst_tcc750x[cChannelHandle]);
 		mdelay(1);
		can_set(&brst_tcc750x[cChannelHandle]);
	} else if (IS_ENABLED(CONFIG_TCC807X)) {
		can_set(&clken_tcc807x[cChannelHandle]);
		can_unset(&brst_tcc807x[cChannelHandle]);
 		mdelay(1);
		can_set(&brst_tcc807x[cChannelHandle]);
	}
}
#endif
