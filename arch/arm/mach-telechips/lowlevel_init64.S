/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Coperation
 */

#include <config.h>
#include <linux/linkage.h>

ENTRY(lowlevel_init)
	mrs x0, hcr_el2
	orr x0, x0, #(1<<4)	/* Enable hcr_el2.imo (Physical IRQ Routing) */
	msr hcr_el2, x0
	ret
ENDPROC(lowlevel_init)

