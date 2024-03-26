/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * arch/arm64/include/asm/arch_gicv3.h
 *
 * Copyright (C) 2015 ARM Ltd.
 *
 * Modified by Copyright Telechips Inc.
 * Import this file in the kernel from kernel-5.4.y:
 *   - arch/arm64/include/asm/arch_gicv3.h
 */
#ifndef ASM_ARCH_GICV3_64_H
#define ASM_ARCH_GICV3_64_H

#include <asm/sysreg_64.h>

#ifndef __ASSEMBLY__
#include <linux/stringify.h>
#include <asm/barriers.h>
#define read_gicreg(r)			read_sysreg_s(SYS_ ## r)
#define write_gicreg(v, r)		write_sysreg_s(v, SYS_ ## r)

/*
 * Low-level accessors
 *
 * These system registers are 32 bits, but we make sure that the compiler
 * sets the GP register's most significant bits to 0 with an explicit cast.
 */

static inline void gic_write_eoir(u32 irq)
{
	write_sysreg_s(irq, SYS_ICC_EOIR1_EL1);
	isb();
}

static inline void gic_write_dir(u32 irq)
{
	write_sysreg_s(irq, SYS_ICC_DIR_EL1);
	isb();
}

static inline u64 gic_read_iar_common(void)
{
	u64 irqstat;

	irqstat = read_sysreg_s(SYS_ICC_IAR1_EL1);
	dsb();
	return irqstat;
}

static inline void gic_write_ctlr(u32 val)
{
	write_sysreg_s(val, SYS_ICC_CTLR_EL1);
	isb();
}

static inline u32 gic_read_ctlr(void)
{
	return read_sysreg_s(SYS_ICC_CTLR_EL1);
}

static inline void gic_write_grpen1(u32 val)
{
	write_sysreg_s(val, SYS_ICC_IGRPEN1_EL1);
	isb();
}

static inline void gic_write_sgi1r(u64 val)
{
	write_sysreg_s(val, SYS_ICC_SGI1R_EL1);
}

static inline u32 gic_read_sre(void)
{
	return read_sysreg_s(SYS_ICC_SRE_EL1);
}

static inline void gic_write_sre(u32 val)
{
	write_sysreg_s(val, SYS_ICC_SRE_EL1);
	isb();
}

static inline void gic_write_bpr1(u32 val)
{
	write_sysreg_s(val, SYS_ICC_BPR1_EL1);
}

static inline u32 gic_read_pmr(void)
{
	return read_sysreg_s(SYS_ICC_PMR_EL1);
}

static __always_inline void gic_write_pmr(u32 val)
{
	write_sysreg_s(val, SYS_ICC_PMR_EL1);
}

static inline u32 gic_read_rpr(void)
{
	return read_sysreg_s(SYS_ICC_RPR_EL1);
}

#define gic_read_typer(c)		readq_relaxed(c)
#define gic_write_irouter(v, c)		writeq_relaxed(v, c)
#define gic_read_lpir(c)		readq_relaxed(c)
#define gic_write_lpir(v, c)		writeq_relaxed(v, c)

#define gic_flush_dcache_to_poc(a,l)	__flush_dcache_area((a), (l))

#define gits_read_baser(c)		readq_relaxed(c)
#define gits_write_baser(v, c)		writeq_relaxed(v, c)

#define gits_read_cbaser(c)		readq_relaxed(c)
#define gits_write_cbaser(v, c)		writeq_relaxed(v, c)

#define gits_write_cwriter(v, c)	writeq_relaxed(v, c)

#define gicr_read_propbaser(c)		readq_relaxed(c)
#define gicr_write_propbaser(v, c)	writeq_relaxed(v, c)

#define gicr_write_pendbaser(v, c)	writeq_relaxed(v, c)
#define gicr_read_pendbaser(c)		readq_relaxed(c)

#define gits_write_vpropbaser(v, c)	writeq_relaxed(v, c)

#define gits_write_vpendbaser(v, c)	writeq_relaxed(v, c)
#define gits_read_vpendbaser(c)		readq_relaxed(c)

static inline void gic_arch_enable_irqs(void)
{
	asm volatile ("msr daifclr, #2" : : : "memory");
}

#endif /* __ASSEMBLY__ */
#endif /* __ASM_ARCH_GICV3_H */
