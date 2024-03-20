// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef GPIO_H
#define GPIO_H

#ifndef GPIO_INPUT
#define GPIO_INPUT      (0x0002U)
#endif

#ifndef GPIO_OUTPUT
#define GPIO_OUTPUT     (0x0001U)
#endif

#define GPIO_LEVEL      (0x0000U)
#define GPIO_EDGE       (0x0010U)

#define GPIO_RISING     (0x0020U)
#define GPIO_FALLING    (0x0040U)

#define GPIO_HIGH       (0x0020U)
#define GPIO_LOW        (0x0040U)

#define GPIO_PULLUP     (0x0100U)
#define GPIO_PULLDOWN   (0x0200U)
#define GPIO_PULLDISABLE (0x400U)

#ifndef GPIO_NC
#define GPIO_NC		(0xFFFFFFFFU)
#endif

#ifndef GPIO_EXP
#define GPIO_EXP	(0xFFFFFFF0U)
#endif

struct ext_gpio;

struct gpio_cfg {
	const char *str_gpio;
	u32 function;
	u32 direction;
	u32 pull;
};

s32 gpio_config(u32 n, u32 flags);
void gpio_set(u32 n, u32 on);
s32 gpio_get(u32 n);
s32 register_ext_gpios(u32 gpio_id, struct ext_gpio *gpios);

#endif
