// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <dm/uclass.h>
#include <irq.h>
#include <mach/timer_api.h>

DECLARE_GLOBAL_DATA_PTR;

#define TIMER_UC_NAME       "timer_irq"
#define TCC_TIMER_MAX       6

static const struct timer_irq_ops *timer_irq_dev_ops(const struct udevice *dev)
{
	return (const struct timer_irq_ops *)dev->driver->ops;
}

s32 timer_irq_init(void)
{
	struct udevice *dev = NULL;
	s32 ret = -1;

	if (gd->dm_root == NULL) {
		ret = -EAGAIN;
	} else {
		ret = uclass_first_device_err(UCLASS_TIMER_IRQ, &dev);
		while ((ret == 0) && (dev != NULL)) {
			ret = uclass_next_device_err(&dev);
		}
		ret = 0;
	}

	return ret;
}

static struct udevice *timer_get_dev(const struct tcc_timer *timer)
{
	const struct tcc_timer *priv;
	struct udevice *dev;
	s32 ret;

	ret = uclass_first_device_err(UCLASS_TIMER_IRQ, &dev);
	while ((ret == 0) && (dev != NULL)) {
		priv = (const struct tcc_timer *)dev_get_priv(dev);
		if (priv->id == timer->id) {
			break;
		}
		ret = uclass_next_device_err(&dev);
	}

	return dev;
}

struct tcc_timer *timer_register(u32 usec,
				 interrupt_handler_t *handler,
				 void *data)
{
	const struct timer_irq_ops *ops;
	const struct tcc_timer *priv;
	struct tcc_timer *ptimer;
	struct udevice *dev;
	s32 ret;

	ret = uclass_first_device_err(UCLASS_TIMER_IRQ, &dev);
	while ((ret == 0) && (dev != NULL)) {
		priv = (const struct tcc_timer *)dev_get_priv(dev);
		if ((priv->used == 0) && (priv->reserved == 0)) {
			break;
		}
		ret = uclass_next_device_err(&dev);
	}

	if (dev == NULL) {
		(void)log_warning("[WARN] %s: Failed to get timer.\n", __func__);
		ptimer = NULL;
	} else {
		ops = timer_irq_dev_ops(dev);
		if (ops != NULL) {
			ptimer = ops->register_timer(dev, usec, handler, data);
		} else {
			ptimer = NULL;
		}
	}

	return ptimer;
}

void timer_unregister(const struct tcc_timer *timer)
{
	const struct timer_irq_ops *ops;
	const struct udevice *dev;

	dev = timer_get_dev(timer);
	if (dev == NULL) {
		(void)log_err("[ERROR] %s: Failed to get timer.\n", __func__);
	} else {
		ops = timer_irq_dev_ops(dev);
		if (ops != NULL) {
			ops->unregister_timer(dev);
		}
	}
}

s32 timer_enable(const struct tcc_timer *timer)
{
	const struct timer_irq_ops *ops;
	const struct udevice *dev;
	s32 ret = -1;

	dev = timer_get_dev(timer);
	if (dev == NULL) {
		(void)log_err("[ERROR] %s: Failed to get timer.\n", __func__);
	} else {
		ops = timer_irq_dev_ops(dev);
		if (ops != NULL) {
			ret = ops->enable_timer(dev);
		}
	}

	return ret;
}

s32 timer_disable(const struct tcc_timer *timer)
{
	const struct timer_irq_ops *ops;
	const struct udevice *dev;
	s32 ret = -1;

	dev = timer_get_dev(timer);
	if (dev == NULL) {
		(void)log_err("[ERROR] %s: Failed to get timer.\n", __func__);
	} else {
		ops = timer_irq_dev_ops(dev);
		if (ops != NULL) {
			ret = ops->disable_timer(dev);
		}
	}

	return ret;
}

UCLASS_DRIVER(timer_irq) = {
	.name = "timer_irq",
	.id = UCLASS_TIMER_IRQ,
};
