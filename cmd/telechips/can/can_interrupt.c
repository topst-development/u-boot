// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef __CAN_INTERRUPT_H
#define __CAN_INTERRUPT_H

#include <linux/types.h>
#include <linux/printk.h>
#include <test/can_porting_chip.h>

#ifdef TCC750X_ES
static int32_t can_irq_tcc750x[] = {
	181,
	183,
};
#else
static int32_t can_irq_tcc750x[] = {
	244,
	246,
};
#endif

#if (TCC807X_CAN_TEST_CASE == 0)
static int32_t can_irq_tcc807x[] = {
	63,
	71,
	73,
};
#elif (TCC807X_CAN_TEST_CASE == 1)
static int32_t can_irq_tcc807x[] = {
	83,
	85,
	89,
};
#elif (TCC807X_CAN_TEST_CASE == 2)
static int32_t can_irq_tcc807x[] = {
	91,
	93,
	95,
};
#elif (TCC807X_CAN_TEST_CASE == 3)
static int32_t can_irq_tcc807x[] = {
	106,
	109,
	111,
};
#endif

int32_t get_can_irq(uint8_t cChannelHandle)
{
	int32_t ret = 0;

	if (IS_ENABLED(CONFIG_TCC750X))
		ret = can_irq_tcc750x[cChannelHandle] + 32;
	else if (IS_ENABLED(CONFIG_TCC807X))
		ret = can_irq_tcc807x[cChannelHandle] + 32;

	printk("ch : %d interrupt num : %d\n", cChannelHandle, ret);

	return ret;
}
#endif
