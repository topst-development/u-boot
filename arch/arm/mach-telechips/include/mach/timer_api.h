// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_TIMER_API_H
#define TCC_TIMER_API_H

#include "irq.h"

struct tcc_timer {
	s32 id;
	u32 ref;
	u32 mref;
	u32 div;
	void *args;
	s32 used;
	s32 reserved;
	s32 irq;
	u32 irqcnt;
	const char *name;
	interrupt_handler_t *handler;
	struct tcc_timer_plat *platinfo;
};

struct timer_irq_ops {
	struct tcc_timer *(*register_timer)(struct udevice *dev,
					    u32 usec,
					    interrupt_handler_t *handler,
					    void *data);
	void (*unregister_timer)(struct udevice *dev);
	s32 (*enable_timer)(struct udevice *dev);
	s32 (*disable_timer)(struct udevice *dev);
};

s32 timer_enable(struct tcc_timer *timer);
s32 timer_disable(struct tcc_timer *timer);
struct tcc_timer *timer_register(u32 usec,
				 interrupt_handler_t *handler,
				 void *data);
void timer_unregister(struct tcc_timer *timer);

#endif
