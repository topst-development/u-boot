// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <config.h>
#include <linux/linkage.h>

ENTRY(lowlevel_init)
#if defined(CONFIG_ARM64)
	mrs	x0, hcr_el2
	orr	x0, x0, #(1<<4)
	msr	hcr_el2, x0
	ret
#else
	stmfd	sp!, {r0-r3, lr}
	ldmfd	sp!, {r0-r3, pc}
#endif
END(lowlevel_init)
