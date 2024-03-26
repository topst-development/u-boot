// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#define pr_fmt(fmt)	"GICv3: " fmt

#include <common.h>
#include <dm.h>
#include <dt-bindings/interrupt-controller/arm-gic.h>
#include <linux/io.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <asm/arch/irq.h>

#include "irqchip.h"
#include "arm-gic-v3.h"

struct gic_dist {
	u32 ctlr;		/* 0x0000: distributor control */
	u32 typer;		/* 0x0004: interrupt controller type */
	u32 iidr;		/* 0x0008: distributor implementer identification */
	u32 reserved_0;		/* reserved */

	u32 statusr;		/* 0x0010: error reporting status */
	u32 reserved_1[11];	/* 0x0014--0x003C */

	u32 setspi_nsr;		/* 0x0040: non-secure SPI set */
	u32 reserved_2;		/* reserved */
	u32 clrspi_nsr;		/* 0x0048: non-secure SPI clear */
	u32 reserved_3;		/* reserved */
	u32 setspi_sr;		/* 0x0050: secure SPI set */
	u32 reserved_4;		/* reserved */
	u32 clrspi_sr;		/* 0x0058: secure SPI clear */
	u32 reserved_5[9];	/* 0x005C--0x007C */

	u32 igroupr[32];	/* 0x0080: interrupt group */
	u32 isenabler[32];	/* 0x0100: interrupt set-enable */
	u32 icenabler[32];	/* 0x0180: interrupt clear-enable */
	u32 ispendr[32];	/* 0x0200: interrupt set-pending */
	u32 icpendr[32];	/* 0x0280: interrupt clear-pending */
	u32 isactiver[32];	/* 0x0300: interrupt set-active */
	u32 icactiver[32];	/* 0x0380: interrupt clear-active */
	u32 ipriorityr[256];	/* 0x0400: interrupt priority */
	u32 itargetr[256];	/* 0x0800: interrupt processor targets */

	u32 icfgr[64];		/* 0x0C00: interrupt configuration */
	u32 igrpmodr[32];	/* 0x0D00: interrupt group modifier */
	u32 reserved_6[32];	/* 0x0D80--0x0DFC */

	u32 nsacr[64];		/* 0x0E00: non-secure access control */
	u32 sgir;		/* 0x0F00: software generated interrupt */
	u32 reserved_7[3];	/* 0x0F04--0x0F0C */
	u32 cpendsgir[4];	/* 0x0F10: SGI clear-pending */
	u32 spendsgir[4];	/* 0x0F20: SGI set-pending */
	u32 reserved_8[5236];	/* 0x0F30--0x60FC */

	u64 irouter[988];	/* 0x6100: interrupt routing */
	u32 reserved_9[4104];	/* 0x7FE0--0xBFFC */
};

struct gic_rdist {
	u32 ctlr;		/* 0x0000: redistributor control */
	u32 iidr;		/* 0x0004: implement identification */
	u32 typer;		/* 0x0008: redistributor type */
	u32 reserved_0;		/* reserved */

	u32 statusr;		/* 0x0010: error reporting status(optional) */
	u32 waker;		/* 0x0014: redistributor wake */
	u32 reserved_1[10];	/* 0x0018--0x003C */

	u64 setlpir;		/* 0x0040: set lpi pending */
	u64 clrlpir;		/* 0x0048: clear lpi pending */
	u32 reserved_2[8];	/* 0x0050--0x006C */

	u64 propbaser;		/* 0x0070: redistributor properties base address */
	u64 pendbaser;		/* 0x0078: redistributor lpi pending table base address */

	u32 igroupr[1];		/* 0x0080: interrupt group */
	u32 reserved_3[31];	/* 0x0084--0x00FC */
	u32 isenabler[1];	/* 0x0100: interrupt set-enable */
	u32 reserved_4[31];	/* 0x0104--0x017C */
	u32 icenabler[1];	/* 0x0180: interrupt clear-enable */
	u32 reserved_5[31];	/* 0x0184--0x01FC */
	u32 ispendr[1];		/* 0x0200: interrupt set-pending */
	u32 reserved_6[31];	/* 0x0204--0x027C */
	u32 icpendr[1];		/* 0x0280: interrupt clear-pending */
	u32 reserved_7[31];	/* 0x0284--0x02FC */
	u32 isactiver[1];	/* 0x0300: interrupt set-active */
	u32 reserved_8[31];	/* 0x0304--0x037C */
	u32 icactiver[1];	/* 0x0380: interrupt clear-active */
	u32 reserved_9[31];	/* 0x0384--0x03FC */
	u32 ipriorityr[8];	/* 0x0400: interrupt priority */
	u32 reserved_10[504];	/* 0x0420--0x0BFC */

	u32 icfgr[2];		/* 0x0C00: interrupt configuration */
	u32 reserved_11[62];	/* 0x0C08--0x0CFC */
	u32 igrpmodr[1];	/* 0x0D00: interrupt group modifier */
	u32 reserved_12[63];	/* 0x0C08--0x0CFC */

	u32 nsacr[1];		/* 0x0E00: non-secure access control */
	u32 reserved_13[11391];	/* 0x0E04--0xBFFC */

	u32 miscstatusr;	/* 0xC000: miscellaneous status */
	u32 reserved_14;	/* reserved */
	u32 ierrvr;		/* 0xC008: interrupt error valid */
	u32 reserved_15;	/* reserved */
	u64 sgidr;		/* 0xC010: sgi default */
	u32 reserved_16[3066];	/* 0xC018--0xEFFC */
	u32 cfgid0;		/* 0xF000: configuration id0 */
	u32 cfgid1;		/* 0xF004: configuration id1 */
};

struct irq_handler {
	void *m_arg;
	void (*m_func)(void *data);
};

static struct irq_handler irq_handlers[NR_IRQS];

#if defined(CONFIG_ARCH_TELECHIPS)
struct pic_irq_register {
	u32 *base[2];
	fdt_size_t size[2];
};

static void tcc_pic_of_to_plat(struct udevice *dev, struct pic_irq_register *reg)
{
	ulong addr;

	addr = dev_read_addr_size_index(dev, 4, &reg->size[0]);
	reg->base[0] = phys_to_virt(addr);

	addr = dev_read_addr_size_index(dev, 5, &reg->size[1]);
	reg->base[1] = phys_to_virt(addr);
}

#endif

struct gicv3_irq_register {
	struct gic_dist *dist;
	struct gic_rdist *rdist;
#if defined(CONFIG_ARCH_TELECHIPS)
	struct pic_irq_register pic;
#endif
};

/*----- START: import kernel driver */
/* Our default, arbitrary priority value. Linux only uses one anyway. */
#define DEFAULT_PMR_VALUE       0xF0U
#define GICD_INT_DEF_PRI_X4     0xA0A0A0A0U
#define GICD_INT_EN_CLR_X32     0xFFFFFFFFU

enum gic_intid_range {
	SGI_RANGE,
	PPI_RANGE,
	SPI_RANGE,
	EPPI_RANGE,
	ESPI_RANGE,
	LPI_RANGE,
	__INVALID_RANGE__
};

static enum gic_intid_range __get_intid_range(ulong hwirq)
{
	switch (hwirq) {
	case 0 ... 15:
		return SGI_RANGE;
	case 16 ... 31:
		return PPI_RANGE;
	case 32 ... 1019:
		return SPI_RANGE;
	case EPPI_BASE_INTID ... (EPPI_BASE_INTID + 63):
		return EPPI_RANGE;
	case ESPI_BASE_INTID ... (ESPI_BASE_INTID + 1023):
		return ESPI_RANGE;
	case 8192 ... GENMASK(23, 0):
		return LPI_RANGE;
	default:
		return __INVALID_RANGE__;
	}
}

static enum gic_intid_range get_intid_range(const struct irq *data)
{
	return __get_intid_range(data->id);
}

static inline bool gic_irq_in_rdist(const struct irq *data)
{
	switch (get_intid_range(data)) {
	case SGI_RANGE:
	case PPI_RANGE:
	case EPPI_RANGE:
		return true;
	default:
		return false;
	}
}

static void gic_do_wait_for_rwp(void __iomem *base, u32 bit)
{
	struct gic_dist *gicd = base;
	u32 count = 1000000;	/* 1s! */

	while (gicd->ctlr & bit) {
		count--;
		if (!count) {
			pr_err("RWP timeout, gone fishing\n");
			return;
		}
		udelay(1);
	};
}

/* Wait for completion of a distributor change */
static void gic_dist_wait_for_rwp(const struct gicv3_irq_register *reg)
{
	gic_do_wait_for_rwp(reg->dist, GICD_CTLR_RWP);
}

/* Wait for completion of a redistributor change */
static void gic_redist_wait_for_rwp(const struct gicv3_irq_register *reg)
{
	gic_do_wait_for_rwp(reg->rdist, GICR_CTLR_RWP);
}

#ifdef CONFIG_ARM64
static u64 __maybe_unused gic_read_iar(void)
{
	u64 irqstat;

	irqstat = read_sysreg_s(SYS_ICC_IAR1_EL1);
	dsb();

	return irqstat;
}
#endif

static void gic_enable_redist(bool enable, struct gic_rdist *gicr)
{
	u32 count = 1000000;	/* 1s! */
	u32 val;

	val = gicr->waker;
	if (enable)
		/* Wake up this CPU redistributor */
		val &= ~GICR_WAKER_ProcessorSleep;
	else
		val |= GICR_WAKER_ProcessorSleep;
	gicr->waker = val;

	if (!enable) {		/* Check that GICR_WAKER is writeable */
		val = gicr->waker;
		if (!(val & GICR_WAKER_ProcessorSleep))
			return;	/* No PM support in this redistributor */
	}

	while (--count) {
		val = gicr->waker;
		if (enable ^ (bool)(val & GICR_WAKER_ChildrenAsleep))
			break;
		udelay(1);
	};
	if (!count)
		pr_err("redistributor failed to %s...\n",
			enable ? "wakeup" : "sleep");
}

/*
 * Routines to disable, enable, EOI and route interrupts
 */
static u32 convert_offset_index(const struct irq *data, u32 offset, u32 *index)
{
	switch (get_intid_range(data)) {
	case SGI_RANGE:
	case PPI_RANGE:
	case SPI_RANGE:
		*index = data->id;
		return offset;
	case EPPI_RANGE:
		/*
		 * Contrary to the ESPI range, the EPPI range is contiguous
		 * to the PPI range in the registers, so let's adjust the
		 * displacement accordingly. Consistency is overrated.
		 */
		*index = data->id - EPPI_BASE_INTID + 32;
		return offset;
	case ESPI_RANGE:
		*index = data->id - ESPI_BASE_INTID;
		switch (offset) {
		case GICD_ISENABLER:
			return GICD_ISENABLERnE;
		case GICD_ICENABLER:
			return GICD_ICENABLERnE;
		case GICD_ISPENDR:
			return GICD_ISPENDRnE;
		case GICD_ICPENDR:
			return GICD_ICPENDRnE;
		case GICD_ISACTIVER:
			return GICD_ISACTIVERnE;
		case GICD_ICACTIVER:
			return GICD_ICACTIVERnE;
		case GICD_IPRIORITYR:
			return GICD_IPRIORITYRnE;
		case GICD_ICFGR:
			return GICD_ICFGRnE;
		case GICD_IROUTER:
			return GICD_IROUTERnE;
		default:
			break;
		}
		break;
	default:
		break;
	}

	WARN_ON(1);
	*index = data->id;
	return offset;
}

static int __maybe_unused gic_peek_irq(const struct irq *data, u32 offset)
{
	const struct gicv3_irq_register *reg = dev_get_plat(data->dev);
	void __iomem *base;
	u32 index, mask;

	offset = convert_offset_index(data, offset, &index);
	mask = 1 << (index % 32);

	if (gic_irq_in_rdist(data))
		base = reg->rdist + 0x10000U;
	else
		base = reg->dist;

	return !!(readl_relaxed(base + offset + (index / 32) * 4) & mask);
}

static void gic_poke_irq(const struct irq *data, u32 offset)
{
	const struct gicv3_irq_register *reg = dev_get_plat(data->dev);
	void (*rwp_wait)(const struct gicv3_irq_register *);
	void __iomem *base;
	u32 index, mask;

	offset = convert_offset_index(data, offset, &index);
	mask = 1 << (index % 32);

	if (gic_irq_in_rdist(data)) {
		base = reg->rdist + 0x10000U;
		rwp_wait = gic_redist_wait_for_rwp;
	} else {
		base = reg->dist;
		rwp_wait = gic_dist_wait_for_rwp;
	}

	writel_relaxed(mask, base + offset + (index / 32) * 4);
	rwp_wait(reg);
}

static void gic_mask_irq(const struct irq *data)
{
	gic_poke_irq(data, GICD_ICENABLER);
}

static void gic_unmask_irq(const struct irq *data)
{
	gic_poke_irq(data, GICD_ISENABLER);
}

static void gic_eoimode1_eoi_irq(u32 irq)
{
	/*
	 * No need to deactivate an LPI, or an interrupt that
	 * is is getting forwarded to a vcpu.
	 */
	if (irq >= 8192U)
		return;
	gic_write_dir(irq);
}
/*----- END: import kernel driver */

static int gicv3_irq_of_to_plat(struct udevice *dev)
{
	struct gicv3_irq_register *reg = dev_get_plat(dev);

	reg->dist = dev_remap_addr_index(dev, 0);
	reg->rdist = dev_remap_addr_index(dev, 1);
#if defined(CONFIG_ARCH_TELECHIPS)
	tcc_pic_of_to_plat(dev, &reg->pic);
#endif

	return 0;
}

static void gicv3_irq_dist_init(const struct gicv3_irq_register *reg)
{
	struct gic_dist *gicd = reg->dist;
	u32 irqs;
	u32 i;

	irqs = ((gicd->typer & 0x1FU) + 1U) * 32U;
	irqs = (irqs > 1020U) ? 1020U : irqs;

	/* Disable the distributor */
	gicd->ctlr = 0U;
	gic_dist_wait_for_rwp(reg);

	/*
	 * Configure SPIs as non-secure Group-1. This will only matter
	 * if the GIC only has a single security state. This will not
	 * do the right thing if the kernel is running in secure mode,
	 * but that's not the intended use case anyway.
	 */
	for (i = 1U; i < (irqs / 32U); i++)
		gicd->igroupr[i] = 0xFFFFFFFFU;

	/* Now do the common stuff, and wait for the distributor to drain */
	{
		/*
		 * Set all global interrupts to be level triggered, active high.
		 */
		for (i = 2U; i < (irqs / 16U); i++) {
			gicd->icfgr[i] = 0U;
		}

		/*
		 * Set priority on all global interrupts.
		 */
		for (i = 8U; i < (irqs / 4U); i++) {
			gicd->ipriorityr[i] = GICD_INT_DEF_PRI_X4;
		}

		/*
		 * Deactivate and disable all SPIs. Leave the PPI and SGIs
		 * alone as they are in the redistributor registers on GICv3.
		 */
		for (i = 1U; i < (irqs / 32U); i++) {
			gicd->icenabler[i] = GICD_INT_EN_CLR_X32;
			gicd->icenabler[i] = GICD_INT_EN_CLR_X32;
		}

		gic_dist_wait_for_rwp(reg);
	}

	/* Enable distributor with ARE, Group1 */
	gicd->ctlr = 0x13U;

	/*
	 * Set all global interrupts to the boot CPU only. ARE must be
	 * enabled.
	 */
	for (i = 32U; i < irqs; i++) {
		gicd->irouter[i] = 0ULL;
	}


	for (i = 2U; i < (irqs / 16U); i++) {
		gicd->icfgr[i] = 0x0;
	}
}

static u32 gic_get_pribits(void)
{
	u32 pribits;

	pribits = gic_read_ctlr();
	pribits &= ICC_CTLR_EL1_PRI_BITS_MASK;
	pribits >>= ICC_CTLR_EL1_PRI_BITS_SHIFT;
	pribits++;

	return pribits;
}

static void gicv3_irq_cpu_init(const struct gicv3_irq_register *reg)
{
	struct gic_rdist *gicr = reg->rdist;
	u32 i;

	gic_enable_redist(true, gicr);

	/* Configure SGIs/PPIs as non-secure Group-1 */
	gicr->igroupr[0] = 0XFFFFFFFFU;

	/* gic_cpu_config(): PPI/SGI */
	{
		/*
		 * Deal with the banked PPI and SGI interrupts - disable all
		 * private interrupts. Make sure everything is deactivated.
		 */
		gicr->icactiver[0] = GICD_INT_EN_CLR_X32;
		gicr->icenabler[0] = GICD_INT_EN_CLR_X32;

		/*
		 * Set priority on PPI and SGI interrupts
		 */
		for (i = 0 ; i < 8U ; i++) {
			gicr->ipriorityr[i] = GICD_INT_DEF_PRI_X4;
		}

		gic_redist_wait_for_rwp(reg);
	}

	/* gic_cpu_sys_reg_init(): */
	{
		u32 pribits;

		pribits = gic_get_pribits();

		write_gicreg(DEFAULT_PMR_VALUE, ICC_PMR_EL1);

		//gic_write_bpr1(0);

		/* EOI drops priority only (mode 1) */
		gic_write_ctlr(ICC_CTLR_EL1_EOImode_drop);

		switch(pribits) {
		case 8:
		case 7:
			write_gicreg(0, ICC_AP1R3_EL1);
			write_gicreg(0, ICC_AP1R2_EL1);
			/* Fall through */
		case 6:
			write_gicreg(0, ICC_AP1R1_EL1);
			/* Fall through */
		case 5:
		case 4:
			write_gicreg(0, ICC_AP1R0_EL1);
		}
		isb();

		/* ... and let's hit the road... */
		gic_write_grpen1(1);
	}
}

static int gicv3_irq_probe(struct udevice *dev)
{
	const struct gicv3_irq_register *reg = dev_get_plat(dev);

	gicv3_irq_dist_init(reg);
	gicv3_irq_cpu_init(reg);

	return 0;
}

static void gic_irq_install(const struct irq *data, interrupt_handler_t *handler, void *arg)
{
	if ((data->id < NR_IRQS) && (handler != NULL)) {
		irq_handlers[data->id].m_arg = arg;
		irq_handlers[data->id].m_func = handler;
	}
}

static void gic_irq_release(const struct irq *data)
{
	if (data->id < NR_IRQS) {
		irq_handlers[data->id].m_arg = NULL;
		irq_handlers[data->id].m_func = NULL;
	}
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
		gicd->icfgr[id / 16U] |= mask;
		break;
	case IRQ_TYPE_LEVEL_LOW:
	case IRQ_TYPE_LEVEL_HIGH:
		gicd->icfgr[id / 16U] &= ~mask;
		break;
	default:
		pr_err("Invalid IRQ type (%u)\n", type);
		break;
	}
}

static void gic_irq_set_type(struct irq *data)
{
	const struct gicv3_irq_register *reg = dev_get_plat(data->dev);

	if (data->id >= 16UL) {
#if defined(CONFIG_ARCH_TELECHIPS)
		if (data->id >= 32UL) {
			tcc_pic_irq_set_type(&reg->pic, data->id - 32UL, data->flags);
		}

		if ((data->flags == (u32)IRQ_TYPE_LEVEL_LOW) ||
		    (data->flags == (u32)IRQ_TYPE_EDGE_FALLING)) {
			/* LEVEL_LOW (8) -> LEVEL_HIGH (4) */
			/* EDGE_FALLING (2) -> EDGE_RISING (1) */
			data->flags >>= 1UL;
		}
#endif
		if ((data->id < 32UL) ||
		    (data->flags == (ulong)IRQ_TYPE_LEVEL_HIGH) ||
		    (data->flags == (ulong)IRQ_TYPE_EDGE_RISING)) {
			gic_irq_configure(reg->dist, data->id, data->flags);
		}
	}
}

static void gic_irq_handle(const struct udevice *dev)
{
	u32 id;

	id = gic_read_iar();

	/* Check for special IDs first */
	if ((id >= 1020U) && (id <= 1023U))
		return;

	gic_write_eoir(id);

	if (id < NR_IRQS) {
		if (irq_handlers[id].m_func != NULL) {
			irq_handlers[id].m_func(irq_handlers[id].m_arg);
		}

		gic_eoimode1_eoi_irq(id);
	}
}

static int gic_irq_get(struct irq *data, const fdt32_t *argv, u32 argc)
{
	s32 ret = -EINVAL;

	if (argc == 3U) {
		u32 priv = fdt32_to_cpu(argv[0]);

		data->id = (ulong)fdt32_to_cpu(argv[1]);
		data->flags = (ulong)fdt32_to_cpu(argv[2]);

		if ((ULONG_MAX - 32UL) < data->id) {
			data->id = 0UL;
			data->flags = 0UL;
		} else {
			data->id += ((priv == 1U) ? 16UL : 32UL);
			if (data->id < INT_MAX) {
				ret = (s32)data->id;
			}
		}
	}

	return ret;
}

static const struct irqchip_ops gicv3_irqchip_ops = {
	.install = gic_irq_install,
	.release = gic_irq_release,
	.mask = gic_mask_irq,
	.unmask = gic_unmask_irq,
	.set_type = gic_irq_set_type,
	.handle = gic_irq_handle,
	.get = gic_irq_get,
};

static const struct udevice_id arm_gicv3_ids[] = {
	{ .compatible = "arm,gic-600" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(arm_gic) = {
	.name = "arm-gicv3",
	.id = UCLASS_IRQ,
	.of_match = of_match_ptr(arm_gicv3_ids),
	.probe = gicv3_irq_probe,
	.of_to_plat = gicv3_irq_of_to_plat,
	.plat_auto = (int)sizeof(struct gicv3_irq_register),
	.ops = &gicv3_irqchip_ops,
};
