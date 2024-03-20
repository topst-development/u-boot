// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <dm/uclass.h>
#include <irq.h>
#include <asm/arch/timer_api.h>

DECLARE_GLOBAL_DATA_PTR;

#define TIMER_UC_NAME       "timer_irq"
#define TCC_TIMER_MAX       6

static const struct timer_irq_ops *timer_irq_dev_ops(struct udevice *dev)
{
	return (const struct timer_irq_ops *)dev->driver->ops;
}

s32 timer_irq_init(void)
{
	struct udevice *dev = NULL;

	if (gd->dm_root == NULL) {
		return -EAGAIN;
	}

	uclass_foreach_dev_probe(UCLASS_TIMER_IRQ, dev);

	return 0;
}

static struct udevice *timer_get_dev(struct tcc_timer *timer)
{
	struct tcc_timer *priv;
	struct udevice *dev;
	s32 ret;

	for (ret = uclass_first_device_err(UCLASS_TIMER_IRQ, &dev);
		dev != NULL;
		ret = uclass_next_device_err(&dev)) {

		if (ret == 0) {
			priv = dev_get_priv(dev);
			if (priv->id == timer->id) {
				break;
			}
		}
	}

	return dev;
}

struct tcc_timer *timer_register(u32 usec,
				 interrupt_handler_t *handler,
				 void *data)
{
	const struct timer_irq_ops *ops;
	struct tcc_timer *priv;
	struct udevice *dev;
	s32 ret;

	for (ret = uclass_first_device_err(UCLASS_TIMER_IRQ, &dev);
		dev != NULL;
		ret = uclass_next_device_err(&dev)) {
		if (ret == 0) {
			priv = dev_get_priv(dev);
			if ((priv->used == 0) && (priv->reserved == 0)) {
				break;
			}
		}
	}

	if (dev == NULL) {
		(void)pr_warn("[WARN] %s: Failed to get timer.\n", __func__);
		return NULL;
	}

	ops = timer_irq_dev_ops(dev);
	if (ops != NULL) {
		return ops->register_timer(dev, usec, handler, data);
	} else {
		return NULL;
	}
}

void timer_unregister(struct tcc_timer *timer)
{
	const struct timer_irq_ops *ops;
	struct udevice *dev;

	dev = timer_get_dev(timer);
	if (dev == NULL) {
		(void)pr_err("[ERROR] %s: Failed to get timer.\n", __func__);
		return;
	}
	ops = timer_irq_dev_ops(dev);
	if (ops != NULL) {
		ops->unregister_timer(dev);
	}
}

s32 timer_enable(struct tcc_timer *timer)
{
	const struct timer_irq_ops *ops;
	struct udevice *dev;
	s32 ret = -1;

	dev = timer_get_dev(timer);
	if (dev == NULL) {
		(void)pr_err("[ERROR] %s: Failed to get timer.\n", __func__);
		return ret;
	}
	ops = timer_irq_dev_ops(dev);
	if (ops != NULL) {
		ret = ops->enable_timer(dev);
	}
	return ret;
}

s32 timer_disable(struct tcc_timer *timer)
{
	const struct timer_irq_ops *ops;
	struct udevice *dev;
	s32 ret = -1;

	dev = timer_get_dev(timer);
	if (dev == NULL) {
		(void)pr_err("[ERROR] %s: Failed to get timer.\n", __func__);
		return ret;
	}
	ops = timer_irq_dev_ops(dev);
	if (ops != NULL) {
		ret = ops->disable_timer(dev);
	}

	return ret;
}

UCLASS_DRIVER(timer_irq) = {
	.name = "timer_irq",
	.id = UCLASS_TIMER_IRQ,
};
