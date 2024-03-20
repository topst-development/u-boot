// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#define pr_fmt(fmt) "arm-gic: " fmt

#include <common.h>
#include <dm.h>
#include <irq.h>
#include <linux/io.h>

#include <dt-bindings/interrupt-controller/arm-gic.h>

struct gic_dist {
	u32 ctl;		/* distributor control */
	u32 type;		/* interrupt controller type */
	u32 iidr;		/* distributor implementer identification */
	u32 reserved_0[29];	/* 0x00C--0x07F */
	u32 igroup[32];		/* interrupt group */
	u32 isenable[32];	/* interrupt set-enable */
	u32 icenable[32];	/* interrupt clear-enable */
	u32 ispend[32];		/* interrupt set-pending */
	u32 icpend[32];		/* interrupt clear-pending */
	u32 isactive[32];	/* interrupt set-active */
	u32 icactive[32];	/* interrupt clear-active */
	u32 ipriority[255];	/* interrupt priority */
	u32 reserved_1;		/* 0x7FC--0x7FF */
	u32 itarget[255];	/* interrupt processor targets */
	u32 reserved_2;		/* 0xBFC--0xBFF */
	u32 icfg[64];		/* interrupt configuration */
};

struct gic_cpu {
	u32 ctl;		/* cpu interface control */
	u32 pm;			/* interrupt priority mask */
	u32 bp;			/* binary point */
	u32 ia;			/* interrupt acknowledge */
	u32 eoi;		/* end of interrupt */
	u32 rp;			/* running priority */
	u32 hppi;		/* highest priority pending interrupt */
};

struct irq_handler {
	void *m_data;
	void (*m_func)(void *data);
};

static struct irq_handler irq_handlers[NR_IRQS];

#if defined(CONFIG_ARCH_TELECHIPS)
struct pic_irq_register {
	u32 *base[2];
	fdt_size_t size[2];
};

static void tcc_pic_ofdata_to_platdata(struct udevice *dev,
				       struct pic_irq_register *reg)
{
	ulong addr;

	addr = dev_read_addr_size_index(dev, 4, &reg->size[0]);
	reg->base[0] = phys_to_virt(addr);

	addr = dev_read_addr_size_index(dev, 5, &reg->size[1]);
	reg->base[1] = phys_to_virt(addr);
}

#endif

struct gic_irq_register {
	struct gic_dist *dist;
	struct gic_cpu *cpu;
#if defined(CONFIG_ARCH_TELECHIPS)
	struct pic_irq_register pic;
#endif
};

static int gic_irq_ofdata_to_platdata(struct udevice *dev)
{
	struct gic_irq_register *reg = dev_get_platdata(dev);

	reg->dist = dev_remap_addr_index(dev, 0);
	reg->cpu = dev_remap_addr_index(dev, 1);
#if defined(CONFIG_ARCH_TELECHIPS)
	tcc_pic_ofdata_to_platdata(dev, &reg->pic);
#endif

	return 0;
}

static void gic_irq_dist_init(const struct gic_irq_register *reg)
{
	struct gic_dist *gicd = reg->dist;
	u32 irqs;
	u32 i;
	u32 reg_value;

	gicd->ctl = 0U;

	irqs = ((gicd->type & 0x1FU) + 1U) * 32U;
	irqs = (irqs > 1020U) ? 1020U : irqs;

	for (i = 2U; i < (irqs / 16U); i++) {
		gicd->icfg[i] = 0x0;
	}

	for (i = 0U ; i < 8U ; i++) {
		reg_value = gicd->itarget[i];
		reg_value |= (reg_value >> 16U);
		reg_value |= (reg_value >> 8U);
	}

	reg_value |= (reg_value << 8U);
	reg_value |= (reg_value << 16U);

	for (i = 8U ; i < (irqs / 4U) ; i++) {
		gicd->itarget[i] = reg_value;
	}

	for (i = 8U ; i < (irqs / 4U); i++) {
		gicd->ipriority[i] = 0xA0A0A0A0U;
	}

	for (i = 1U ; i < (irqs / 32U); i++) {
		gicd->icenable[i] = 0xFFFFFFFFU;
	}

	gicd->ctl = 1U;
}

static void gic_irq_cpu_init(const struct gic_irq_register *reg)
{
	struct gic_dist *gicd = reg->dist;
	struct gic_cpu *gicc = reg->cpu;
	u32 i;

	gicd->icenable[0] = 0xFFFF0000U;
	gicd->isenable[0] = 0x0000FFFFU;

	for (i = 0 ; i < 8U ; i++) {
		gicd->ipriority[i] = 0xA0A0A0A0U;
	}

	gicc->pm = 0xF0U;
	gicc->ctl = 0x01U;
}

static int gic_irq_probe(struct udevice *dev)
{
	const struct gic_irq_register *reg = dev_get_platdata(dev);

	gic_irq_dist_init(reg);
	gic_irq_cpu_init(reg);

	return 0;
}

static void gic_irq_install(const struct irq_desc *irq,
			    interrupt_handler_t *handler, void *data)
{
	if ((irq->id < NR_IRQS) && (handler != NULL)) {
		irq_handlers[irq->id].m_data = data;
		irq_handlers[irq->id].m_func = handler;
	}
}

static void gic_irq_release(const struct irq_desc *irq)
{
	if (irq->id < NR_IRQS) {
		irq_handlers[irq->id].m_data = NULL;
		irq_handlers[irq->id].m_func = NULL;
	}
}

static void gic_irq_mask(const struct irq_desc *irq)
{
	const struct gic_irq_register *reg = dev_get_platdata(irq->dev);
	struct gic_dist *gicd = reg->dist;

	u32 mask = (u32)1U << (irq->id & 31U);
	u32 offs = irq->id / 32U;

	gicd->icenable[offs] = mask;
}

static void gic_irq_unmask(const struct irq_desc *irq)
{
	const struct gic_irq_register *reg = dev_get_platdata(irq->dev);
	struct gic_dist *gicd = reg->dist;

	u32 mask = (u32)1U << (irq->id & 31U);
	u32 offs = irq->id / 32U;

	gicd->isenable[offs] = mask;
}

#if defined(CONFIG_ARCH_TELECHIPS)
static inline void tcc_pic_irq_set_type(const struct pic_irq_register *pic,
					u32 id, u32 type)
{
	u32 *addr = pic->base[0];
	fdt_size_t thres = (pic->size[0] / 4U) * 32U;
	u32 mask;

	if (id >= thres) {
		addr = pic->base[1];
		id -= (u32)thres;
	}

	mask = (u32)1U << (id & 0xFU);

	switch (type) {
	case IRQ_TYPE_EDGE_FALLING:
	case IRQ_TYPE_LEVEL_LOW:
		addr[id / 32U] |= mask;
		break;
	case IRQ_TYPE_EDGE_RISING:
	case IRQ_TYPE_LEVEL_HIGH:
		addr[id / 32U] &= ~mask;
		break;
	default:
		pr_err("Invalid IRQ type (%u)\n", type);
		break;
	}
}
#endif

static inline void gic_irq_configure(struct gic_dist *gicd, u32 id, u32 type)
{
	u32 mask = ((u32)2U) << ((id & 0xFU) << 1U);

	switch (type) {
	case IRQ_TYPE_EDGE_FALLING:
	case IRQ_TYPE_EDGE_RISING:
	case IRQ_TYPE_EDGE_BOTH:
		gicd->icfg[id / 16U] |= mask;
		break;
	case IRQ_TYPE_LEVEL_LOW:
	case IRQ_TYPE_LEVEL_HIGH:
		gicd->icfg[id / 16U] &= ~mask;
		break;
	default:
		pr_err("Invalid IRQ type (%u)\n", type);
		break;
	}
}

static void gic_irq_set_type(struct irq_desc *irq)
{
	const struct gic_irq_register *reg = dev_get_platdata(irq->dev);

	if (irq->id >= 16U) {
#if defined(CONFIG_ARCH_TELECHIPS)
		if (irq->id >= 32U) {
			tcc_pic_irq_set_type(&reg->pic, irq->id - 32U,
					     irq->type);
		}

		if ((irq->type == (u32)IRQ_TYPE_LEVEL_LOW) ||
		    (irq->type == (u32)IRQ_TYPE_EDGE_FALLING)) {
			/* LEVEL_LOW (8) -> LEVEL_HIGH (4) */
			/* EDGE_FALLING (2) -> EDGE_RISING (1) */
			irq->type >>= 1U;
		}
#endif
		if ((irq->id < 32U) ||
		    (irq->type == (u32)IRQ_TYPE_LEVEL_HIGH) ||
		    (irq->type == (u32)IRQ_TYPE_EDGE_RISING)) {
			gic_irq_configure(reg->dist, irq->id, irq->type);
		}
	}
}

static void gic_irq_handle(const struct udevice *dev)
{
	const struct gic_irq_register *reg = dev_get_platdata(dev);
	struct gic_cpu *gicc = reg->cpu;
	u32 id;

	id = gicc->ia & 0x3FFU;

	if (id < NR_IRQS) {
		if (irq_handlers[id].m_func != NULL) {
			irq_handlers[id].m_func(irq_handlers[id].m_data);
		}

		gicc->eoi = id;
	}
}

static int gic_irq_get(struct irq_desc *irq, const fdt32_t *argv, u32 argc)
{
	s32 ret = -EINVAL;

	if (argc == 3U) {
		u32 priv = fdt32_to_cpu(argv[0]);

		irq->id = fdt32_to_cpu(argv[1]);
		irq->type = fdt32_to_cpu(argv[2]);

		if ((UINT_MAX - 32U) < irq->id) {
			irq->id = 0;
			irq->type = 0;
		} else {
			irq->id += ((priv == 1U) ? 16U : 32U);
			ret = (s32)irq->id;
		}
	}

	return ret;
}

static const struct irq_ops gic_irq_ops = {
	.install = gic_irq_install,
	.release = gic_irq_release,
	.mask = gic_irq_mask,
	.unmask = gic_irq_unmask,
	.set_type = gic_irq_set_type,
	.handle = gic_irq_handle,
	.get = gic_irq_get,
};

static const struct udevice_id arm_gic_ids[] = {
	{ .compatible = "arm,gic-400" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(arm_gic) = {
	.name = "arm-gic",
	.id = UCLASS_IRQ,
	.of_match = of_match_ptr(arm_gic_ids),
	.probe = gic_irq_probe,
	.ofdata_to_platdata = gic_irq_ofdata_to_platdata,
	.platdata_auto_alloc_size = (int)sizeof(struct gic_irq_register),
	.ops = &gic_irq_ops,
};