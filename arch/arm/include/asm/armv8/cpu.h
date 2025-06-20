/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 * Copyright 2023 Telechips Inc.
 */

/*
 * Modified by Telechips Inc. (date: 2023-04)
 */

#define MIDR_PARTNUM_CORTEX_A35		(0xD04U)
#define MIDR_PARTNUM_CORTEX_A53		(0xD03U)
#define MIDR_PARTNUM_CORTEX_A55		(0xD05U)
#define MIDR_PARTNUM_CORTEX_A72		(0xD08U)
#define MIDR_PARTNUM_CORTEX_A76		(0xD0BU)
#define MIDR_PARTNUM_CORTEX_A65AE	(0xD43U)
#define MIDR_PARTNUM_SHIFT		(0x4U)
#define MIDR_PARTNUM_MASK		(0xFFFU << 0x4U)

static inline unsigned int read_midr(void)
{
	unsigned long val;

	asm volatile("mrs %0, midr_el1" : "=r" (val));

	return val;
}

#define is_cortex_a35() (((read_midr() & MIDR_PARTNUM_MASK) >> \
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A35)
#define is_cortex_a53() (((read_midr() & MIDR_PARTNUM_MASK) >> \
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A53)
#define is_cortex_a72() (((read_midr() & MIDR_PARTNUM_MASK) >>\
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A72)
