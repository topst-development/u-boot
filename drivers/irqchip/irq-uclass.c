// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#define pr_fmt(fmt) "irq: " fmt

#include <common.h>
#include <dm.h>
#include <irq.h>

#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static struct udevice *cur_irq_controller;
static const struct irq_ops *ops;
static u32 cells;

static s32 arch_interrupt_cells_init(void)
{
	s32 ret = dev_read_u32(cur_irq_controller, "#interrupt-cells", &cells);

	if (ret != 0) {
		const char *devname = dev_read_name(cur_irq_controller);

		pr_err("Failed to get '#interrupt-cells' from %s\n", devname);
	}

	if ((ret == 0) && (cells == 0U)) {
		pr_err("Invalid '#interrupt-cells'\n");
		ret = -EINVAL;
	}

	return ret;
}

s32 arch_interrupt_init(void)
{
	struct udevice *dev;
	s32 ret = -EAGAIN;

	cur_irq_controller = NULL;
	ops = NULL;
	cells = 0U;

	if (gd->dm_root != NULL) {
		ret = uclass_first_device_err(UCLASS_IRQ, &dev);
	}

	while ((ret == 0) && (dev != NULL)) {
		const void *prop;

		prop = dev_read_prop(dev, "interrupt-controller", NULL);

		if (prop != NULL) {
			/* Must be called only once */
			cur_irq_controller = dev;
			ops = irq_get_ops(dev);
			ret = arch_interrupt_cells_init();
			break;
		}

		ret = uclass_next_device_err(&dev);
	}

	return ret;
}

s32 irq_get_by_index(struct udevice *dev, struct irq_desc *irq, u32 index)
{
	const fdt32_t *argv;
	u32 range;
	s32 ret;

	if ((cells == 0U) || (ops == NULL) || (ops->get == NULL)) {
		pr_err("IRQ subsystem is not initialized\n");
		ret = -EINVAL;
	} else if ((dev == NULL) || (irq == NULL)) {
		pr_err("Invalid arguments\n");
		ret = -EINVAL;
	} else {
		s32 len = 0;

		irq->dev = cur_irq_controller;
		irq->id = 0;
		irq->type = 0;

		argv = dev_read_prop(dev, "interrupts", &len);
		if ((argv == NULL) || (len <= 0)) {
			const char *devname = dev_read_name(dev);

			pr_err("No interrupts property in %s\n", devname);
			ret = -EINVAL;
		} else {
			range = (u32)len >> 2U;
			range /= cells;

			if ((range <= index) || ((UINT_MAX / cells) < index)) {
				pr_err("Index out of range\n");
				ret = -ENOENT;
			} else {
				argv = &argv[index * cells];
				ret = ops->get(irq, argv, cells);
			}
		}
	}

	return ret;
}

void irq_install_handler(int vec, interrupt_handler_t *handler, void *arg)
{
	if ((vec >= 0) && (ops != NULL) && (ops->install != NULL)) {
		struct irq_desc irq;

		irq.dev = cur_irq_controller;
		irq.id = (u32)vec;
		irq.type = 0;

		ops->install(&irq, handler, arg);
	}
}

void irq_free_handler(int vec)
{
	if ((vec >= 0) && (ops != NULL) && (ops->release != NULL)) {
		struct irq_desc irq;

		irq.dev = cur_irq_controller;
		irq.id = (u32)vec;
		irq.type = 0;

		ops->release(&irq);
	}
}

void irq_mask(s32 vec)
{
	if ((vec >= 0) && (ops != NULL) && (ops->mask != NULL)) {
		struct irq_desc irq;

		irq.dev = cur_irq_controller;
		irq.id = (u32)vec;
		irq.type = 0;

		ops->mask(&irq);
	}
}

void irq_unmask(s32 vec)
{
	if ((vec >= 0) && (ops != NULL) && (ops->unmask != NULL)) {
		struct irq_desc irq;

		irq.dev = cur_irq_controller;
		irq.id = (u32)vec;
		irq.type = 0;

		ops->unmask(&irq);
	}
}

void irq_set_type(s32 vec, u32 type)
{
	if ((vec >= 0) && (ops != NULL) && (ops->set_type != NULL)) {
		struct irq_desc irq;

		irq.dev = cur_irq_controller;
		irq.id = (u32)vec;
		irq.type = type;

		ops->set_type(&irq);
	}
}

#if defined(CONFIG_ARM64)
void do_irq(struct pt_regs *pt_regs, unsigned int esr)
#elif defined(CONFIG_ARM)
void do_irq(struct pt_regs *pt_regs)
#else
#  error Currently IRQ driver model is only available for ARM architectures.
#endif
{
	if ((ops != NULL) && (ops->handle != NULL)) {
		ops->handle(cur_irq_controller);
	}
}

UCLASS_DRIVER(gic) = {
	.name = "interrupt controller",
	.id = UCLASS_IRQ,
};
