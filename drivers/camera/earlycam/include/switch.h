// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef SWITCH_H
#define SWITCH_H

#include <dm/device.h>
#include <asm/gpio.h>

struct switch_gpio {
	struct gpio_desc	switch_gpio_desc;
	unsigned int		switch_active;
};

extern struct udevice *g_switch_dev;

extern int switch_if_init(void);
extern struct udevice *switch_if_get_switch(void);
extern int switch_if_get_status(const struct udevice *dev);


#endif//SWITCH_H
