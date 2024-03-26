/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2020 ARM Ltd.
 *
 * Modified by Copyright Telechips Inc.
 * Import this file in the kernel from kernel-5.4.y:
 *   - arch/arm/include/asm/vdos/cp15.h
 */
#ifndef ASM_SYSREG_H
#ifndef __ASSEMBLY__

#include <linux/stringify.h>

#define __ACCESS_CP15(CRn, Op1, CRm, Op2)	\
	"mrc", "mcr", __stringify(p15, Op1, %0, CRn, CRm, Op2), u32
#define __ACCESS_CP15_64(Op1, CRm)		\
	"mrrc", "mcrr", __stringify(p15, Op1, %Q0, %R0, CRm), u64

#define __read_sysreg(r, w, c, t) ({				\
	t __val;						\
	asm volatile(r " " c : "=r" (__val));			\
	__val;							\
})
#define read_sysreg(...)		__read_sysreg(__VA_ARGS__)

#define __write_sysreg(v, r, w, c, t)	asm volatile(w " " c : : "r" ((t)(v)))
#define write_sysreg(v, ...)		__write_sysreg(v, __VA_ARGS__)

#define BPIALL				__ACCESS_CP15(c7, 0, c5, 6)
#define ICIALLU				__ACCESS_CP15(c7, 0, c5, 0)

#define CNTVCT				__ACCESS_CP15_64(1, c14)

#endif /* __ASSEMBLY__ */

#endif	/* ASM_SYSREG_H */
