// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef IRQCHIP_H
#define IRQCHIP_H

#include <dm/device.h>
#include <irq.h>
#include <irq_func.h>

struct irqchip_ops {
	void (*install)(const struct irq *data,
			interrupt_handler_t *handler, void *arg);
	void (*release)(const struct irq *data);
	void (*mask)(const struct irq *data);
	void (*unmask)(const struct irq *data);
	void (*set_type)(struct irq *data);
	void (*handle)(const struct udevice *dev);
	int  (*get)(struct irq *data, const fdt32_t *argv, u32 argc);
};

#define irqchip_get_ops(dev) (const struct irqchip_ops *)((dev)->driver->ops)

#endif /* IRQCHIP_H */
