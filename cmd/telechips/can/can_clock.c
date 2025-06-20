// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef __CAN_CLOCK_H
#define __CAN_CLOCK_H

#include <linux/types.h>
#include <linux/printk.h>

#include <mach/clock.h>
#include <test/can_porting_chip.h>

#if defined(CONFIG_TCC807X)
#if (TCC807X_CAN_TEST_CASE == 0)
unsigned long can_pclk_tcc807x[] = {
	PERI_CAN0,
	PERI_CAN1,
	PERI_CAN2,
};
#elif (TCC807X_CAN_TEST_CASE == 1)
unsigned long can_pclk_tcc807x[] = {
	PERI_CAN3,
	PERI_CAN4,
	PERI_CAN5,
};
#elif (TCC807X_CAN_TEST_CASE == 2)
unsigned long can_pclk_tcc807x[] = {
	PERI_CAN6,
	PERI_CAN7,
	PERI_CAN8,
};
#elif (TCC807X_CAN_TEST_CASE == 3)
unsigned long can_pclk_tcc807x[] = {
	PERI_CAN9,
	PERI_CAN10,
	PERI_CAN11,
};
#endif

#elif defined(CONFIG_TCC750X)
unsigned long can_pclk_tcc750x[] = {
	PERI_CAN0,
	PERI_CAN1,
};
#endif

unsigned long get_can_pclk(uint8_t cChannelHandle)
{
	unsigned long ret;

#if defined(CONFIG_TCC807X)
	ret = can_pclk_tcc807x[cChannelHandle];
#elif defined(CONFIG_TCC750X)
	ret = can_pclk_tcc750x[cChannelHandle];
#endif
	printk("ch : %d pclk num : 0x%lx\n", cChannelHandle, ret);

	return ret;
}
#endif
