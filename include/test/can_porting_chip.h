// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef __CAN_GPIO_H
#define __CAN_GPIO_H

#include <asm/io.h>
#include <linux/types.h>
#include <linux/errno.h>

#if defined(CONFIG_TCC750X)

#define CAN_CONTROLLER_NUMBER           (2U)
#define CAN_NON_CACHE_MEMORY_START      (0x20000000UL) /* DRAM, the memory space for kernel */

#define CAN_BASE_ADDR			(0x18400000UL)
#define CAN_BASE_PORT			(0)
#define CAN_CHANNEL_ADDR_OFFSET         (0x00010000UL)    /* 0x18400000 + 0x10000 */
#define CAN_CONFIG_ADDR                 (0x18420000UL)
#define CAN_CONFIG_BASE_ADDR            (0x1842000CUL)
#define CAN_CONFIG_EXTS_CTRL0_ADDR      (0x1842002CUL)
#define CAN_CONFIG_EXTS_CTRL1_ADDR      (0x18420030UL)
#define CAN_CONFIG_WR_PW_ADDR           (0x18420038UL)
#define CAN_CONFIG_WR_LOCK_ADDR         (0x1842003CUL)
#define CAN_TSCFG_ADDR         			(0x18400164UL)

#elif defined(CONFIG_TCC807X)

#define CAN_CONTROLLER_NUMBER           (3U)
#define CAN_NON_CACHE_MEMORY_START      (0x40000000UL) /* DRAM, the memory space for kernel */

#define TCC807X_CAN_TEST_CASE 0
/*
 * CASE 0 : CAN0 ~ CAN2
 * CASE 1 : CAN3 ~ CAN5
 * CASE 2 : CAN6 ~ CAN8
 * CASE 3 : CAN9 ~ CAN11
 * */


#if (TCC807X_CAN_TEST_CASE == 0)
#define CAN_BASE_ADDR			(0x16500000UL)
#define CAN_BASE_PORT			(0)
#elif (TCC807X_CAN_TEST_CASE == 1)
#define CAN_BASE_ADDR			(0x16530000UL)
#define CAN_BASE_PORT			(3)
#elif (TCC807X_CAN_TEST_CASE == 2)
#define CAN_BASE_ADDR			(0x16560000UL)
#define CAN_BASE_PORT			(6)
#elif (TCC807X_CAN_TEST_CASE == 3)
#define CAN_BASE_ADDR			(0x16590000UL)
#define CAN_BASE_PORT			(9)
#endif

#define CAN_CHANNEL_ADDR_OFFSET         (0x00010000UL)    /* 0x16500000 + 0x10000 */
#define CAN_CONFIG_ADDR                 (0x165C0000UL)
#define CAN_CONFIG_BASE_ADDR            (0x165C000CUL)
#define CAN_CONFIG_EXTS_CTRL0_ADDR      (0x165C003CUL)
#define CAN_CONFIG_EXTS_CTRL1_ADDR      (0x165C0040UL)
#define CAN_CONFIG_WR_PW_ADDR           (0x165C0048UL)
#define CAN_CONFIG_WR_LOCK_ADDR         (0x165C004CUL)
#define CAN_TSCFG_ADDR         			(0x16500164UL)

#endif

union gpio_addr {
	uint32_t ui_addr;
	void *v_addr;
};

struct can_gpio {
	uint32_t g_val;
	union gpio_addr g_addr;
	/*
	 * excess :
	 * configure pin functoins in consecutively seperated register.
	 */
	uint32_t excess;
	uint32_t ex_val;
};

int32_t can_gpio_setting(void);
int32_t get_can_irq(uint8_t cChannelHandle);
unsigned long get_can_pclk(uint8_t cChannelHandle);
void can_reset_channel(uint8_t cChannelHandle);
#endif
