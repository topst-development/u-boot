// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC805X_GPIO_H
#define TCC805X_GPIO_H

#define GPIO_REG_SHIFT	((unsigned int)6U)
#define GPIO_REGMASK	((unsigned int)0xFU << GPIO_REG_SHIFT)
#define GPIO_BITMASK	((unsigned int)0x3FU << 0)

#define GPIO_PORTA	((unsigned int)0U << GPIO_REG_SHIFT)
#define GPIO_PORTB	((unsigned int)1U << GPIO_REG_SHIFT)
#define GPIO_PORTC	((unsigned int)2U << GPIO_REG_SHIFT)
#define GPIO_PORTE	((unsigned int)3U << GPIO_REG_SHIFT)
#define GPIO_PORTG	((unsigned int)4U << GPIO_REG_SHIFT)
#define GPIO_PORTH	((unsigned int)5U << GPIO_REG_SHIFT)
#define GPIO_PORTK	((unsigned int)6U << GPIO_REG_SHIFT)
#define GPIO_PORTSD0 ((unsigned int)7U << GPIO_REG_SHIFT)		// MP GPIO Register
#define GPIO_PORTSD1 ((unsigned int)8U << GPIO_REG_SHIFT)		// MP GPIO Register
#define GPIO_PORTSD2 ((unsigned int)9U << GPIO_REG_SHIFT)		// MP GPIO Register
#define GPIO_PORTMA	((unsigned int)10U << GPIO_REG_SHIFT)
#define GPIO_PORTMB	((unsigned int)11U << GPIO_REG_SHIFT)
#define GPIO_PORTMC	((unsigned int)12U << GPIO_REG_SHIFT)
#define GPIO_PORTMD	((unsigned int)13U << GPIO_REG_SHIFT)
#define GPIO_PORTEXT1	((unsigned int)14U << GPIO_REG_SHIFT)
#define GPIO_PORTEXT2	((unsigned int)15U << GPIO_REG_SHIFT)

#define TCC_GPA(x)	(GPIO_PORTA | (x))
#define TCC_GPB(x)	(GPIO_PORTB | (x))
#define TCC_GPC(x)	(GPIO_PORTC | (x))
#define TCC_GPE(x)	(GPIO_PORTE | (x))
#define TCC_GPG(x)	(GPIO_PORTG | (x))
#define TCC_GPH(x)	(GPIO_PORTH | (x))
#define TCC_GPK(x)	(GPIO_PORTK | (x))
#define TCC_GPMA(x)	(GPIO_PORTMA | (x))
#define TCC_GPSD0(x)	(GPIO_PORTSD0 | (x))
#define TCC_GPSD1(x)	(GPIO_PORTSD1 | (x))
#define TCC_GPSD2(x)	(GPIO_PORTSD2 | (x))
#define TCC_GPMB(x)	(GPIO_PORTMB | (x))
#define TCC_GPMC(x)	(GPIO_PORTMC | (x))
#define TCC_GPMD(x)	(GPIO_PORTMD | (x))
#define TCC_GPEXT1(x)	(GPIO_PORTEXT1 | (x))
#define TCC_GPEXT2(x)	(GPIO_PORTEXT2 | (x))

#define GPIO_FN_SHIFT	((unsigned int)27U)
#define GPIO_FN_BITMASK	((unsigned int)0x1FU<<GPIO_FN_SHIFT)
#define GPIO_FN(x)		(((x)+(unsigned int)1U) << GPIO_FN_SHIFT)
#define GPIO_FN0		((unsigned int)1U << GPIO_FN_SHIFT)
#define GPIO_FN1		((unsigned int)2U << GPIO_FN_SHIFT)
#define GPIO_FN2		((unsigned int)3U << GPIO_FN_SHIFT)
#define GPIO_FN3		((unsigned int)4U << GPIO_FN_SHIFT)
#define GPIO_FN4		((unsigned int)5U << GPIO_FN_SHIFT)
#define GPIO_FN5		((unsigned int)6U << GPIO_FN_SHIFT)
#define GPIO_FN6		((unsigned int)7U << GPIO_FN_SHIFT)
#define GPIO_FN7		((unsigned int)8U << GPIO_FN_SHIFT)
#define GPIO_FN8		((unsigned int)9U << GPIO_FN_SHIFT)
#define GPIO_FN9		((unsigned int)10U << GPIO_FN_SHIFT)
#define GPIO_FN10		((unsigned int)11U << GPIO_FN_SHIFT)
#define GPIO_FN11		((unsigned int)12U << GPIO_FN_SHIFT)
#define GPIO_FN12		((unsigned int)13U << GPIO_FN_SHIFT)
#define GPIO_FN13		((unsigned int)14U << GPIO_FN_SHIFT)
#define GPIO_FN14		((unsigned int)15U << GPIO_FN_SHIFT)
#define GPIO_FN15		((unsigned int)16U << GPIO_FN_SHIFT)

#define GPIO_CD_SHIFT	((unsigned int)24U)
#define GPIO_CD_BITMASK	((unsigned int)0x7U<<GPIO_CD_SHIFT)
#define GPIO_CD(x)		(((x)+(unsigned int)1U) << GPIO_CD_SHIFT)
#define GPIO_CD0		((unsigned int)1U << GPIO_CD_SHIFT)
#define GPIO_CD1		((unsigned int)2U << GPIO_CD_SHIFT)
#define GPIO_CD2		((unsigned int)3U << GPIO_CD_SHIFT)
#define GPIO_CD3		((unsigned int)4U << GPIO_CD_SHIFT)

#define TCC_EXT_GPIO_MAX	((unsigned int)2U)

#define GPIO_REG(off)	(TCC_GPIO_BASE + (off))
#define PMGPIO_REG(off)	(TCC_PMGPIO_BASE + (off))

struct gpioregs {
	unsigned int data;		/* data */
	unsigned int out_en;	/* output enable */
	unsigned int out_or;	/* OR fnction on output data */
	unsigned int out_bic;	/* BIC function on output data */
	unsigned int out_xor;	/* XOR function on output data */
	unsigned int strength0;	/* driver strength control 0 */
	unsigned int strength1;	/* driver strength control 1 */
	unsigned int pull_enable;	/* pull-up/down enable */
	unsigned int pull_select;	/* pull-up/down select */
	unsigned int in_en;		/* input enable */
	unsigned int in_type;	/* input type (Shmitt / CMOS) */
	unsigned int slew_rate;	/* slew rate */
	unsigned int func_select0;	/* port configuration 0 */
	unsigned int func_select1;	/* port configuration 1 */
	unsigned int func_select2;	/* port configuration 2 */
	unsigned int func_select3;	/* port configuration 3 */
};

#endif
