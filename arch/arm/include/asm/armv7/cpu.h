/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

/*
 * Modified by Telechips Inc. (date: 2020-04)
 */

#define MIDR_PARTNUM_CORTEX_A7	0xC07
#define MIDR_PARTNUM_CORTEX_A15	0xC0F
#define MIDR_PARTNUM_CORTEX_A35	0xD04
#define MIDR_PARTNUM_CORTEX_A53	0xD03
#define MIDR_PARTNUM_CORTEX_A72	0xD08
#define MIDR_PARTNUM_SHIFT	0x4
#define MIDR_PARTNUM_MASK	(0xFFF << 0x4)

static inline unsigned int read_midr(void)
{
	unsigned long val;

	asm volatile ("mrc p15, 0, %0, c0, c0, 0" : "=r" (val));

	return val;
}

#define is_cortex_a35() (((read_midr() & MIDR_PARTNUM_MASK) >> \
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A35)
#define is_cortex_a53() (((read_midr() & MIDR_PARTNUM_MASK) >> \
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A53)
#define is_cortex_a72() (((read_midr() & MIDR_PARTNUM_MASK) >>\
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A72)
