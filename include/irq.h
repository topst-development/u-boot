// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef IRQ_H
#define IRQ_H

#include <dm/device.h>
#include <irq_func.h>

struct irq_desc {
	struct udevice *dev;
	u32 id;
	u32 type;
};

struct irq_ops {
	void (*install)(const struct irq_desc *irq,
			interrupt_handler_t *handler, void *data);
	void (*release)(const struct irq_desc *irq);
	void (*mask)(const struct irq_desc *irq);
	void (*unmask)(const struct irq_desc *irq);
	void (*set_type)(struct irq_desc *irq);
	void (*handle)(const struct udevice *dev);
	int  (*get)(struct irq_desc *irq, const fdt32_t *argv, u32 argc);
};

#define irq_get_ops(dev) (const struct irq_ops *)((dev)->driver->ops)

s32 arch_interrupt_init(void);

s32 irq_get_by_index(struct udevice *dev, struct irq_desc *irq, u32 index);

void irq_mask(s32 vec);
void irq_unmask(s32 vec);
void irq_set_type(s32 vec, u32 type);

#endif /* IRQ_H */
