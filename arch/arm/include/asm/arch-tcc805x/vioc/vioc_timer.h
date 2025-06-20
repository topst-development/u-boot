/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef VIOC_TIMER_H
#define	VIOC_TIMER_H

/*
 * Register offset
 */
#define USEC		(0x00U)
#define CURTIME		(0x04U)
#define TIMER0		(0x08U)
#define TIMER1		(0x0CU)
#define TIREQ0		(0x10U)
#define TIREQ1		(0x14U)
#define IRQMASK		(0x18U)
#define IRQSTAT		(0x1CU)

/*
 * Micro-second Control Register
 */
#define USEC_EN_SHIFT			(31U)
#define USEC_UINTDIV_SHIFT		(8U)
#define USEC_USECDIV_SHIFT		(0U)

#define USEC_EN_MASK ((u32)0x1U << USEC_EN_SHIFT)
#define USEC_UINTDIV_MASK ((u32)0xFFU << USEC_UINTDIV_SHIFT)
#define USEC_USECDIV_MASK ((u32)0xFFU << USEC_USECDIV_SHIFT)

/*
 * Current Time Register
 */
#define CURTIME_CURTIME_SHIFT		(0U)

#define CURTIME_CURTIME_MASK		((u32)0xFFFFFFFFU << CURTIME_CURTIME_SHIFT)

/*
 * Timer Core k Register
 */
#define TIMER_EN_SHIFT			(31U)
#define TIMER_COUNTER_SHIFT		(0U)

#define TIMER_EN_MASK ((u32)0x1U << TIMER_EN_SHIFT)
#define TIMER_COUNTER_MASK ((u32)0xFFFFU << TIMER_COUNTER_SHIFT)

/*
 * Timer Interrupt k Register
 */
#define TIREQ_EN_SHIFT (31)
#define TIREQ_TIME_SHIFT (0)

#define TIREQ_EN_MASK ((u32)0x1U << TIREQ_EN_SHIFT)
#define TIREQ_TIME_MASK (0xFFFFU << TIREQ_TIME_SHIFT)

/*
 * Interrupt Mask Register
 */
#define IRQMASK_TIREQ1_SHIFT	(3U)
#define IRQMASK_TIREQ0_SHIFT	(2U)
#define IRQMASK_TIMER1_SHIFT	(1U)
#define IRQMASK_TIMER0_SHIFT	(0U)

#define IRQMASK_TIREQ1_MASK		((u32)0x1U << IRQMASK_TIREQ1_SHIFT)
#define IRQMASK_TIREQ0_MASK		((u32)0x1U << IRQMASK_TIREQ0_SHIFT)
#define IRQMASK_TIMER1_MASK		((u32)0x1U << IRQMASK_TIMER1_SHIFT)
#define IRQMASK_TIMER0_MASK		((u32)0x1U << IRQMASK_TIMER0_SHIFT)

/*
 * Interrupt Status Register
 */
#define IRQSTAT_TIREQ1_SHIFT	(3U)
#define IRQSTAT_TIREQ0_SHIFT	(2U)
#define IRQSTAT_TIMER1_SHIFT	(1U)
#define IRQSTAT_TIMER0_SHIFT	(0U)

#define IRQSTAT_TIREQ1_MASK		((u32)0x1U << IRQSTAT_TIREQ1_SHIFT)
#define IRQSTAT_TIREQ0_MASK		((u32)0x1U << IRQSTAT_TIREQ0_SHIFT)
#define IRQSTAT_TIMER1_MASK		((u32)0x1U << IRQSTAT_TIMER1_SHIFT)
#define IRQSTAT_TIMER0_MASK		((u32)0x1U << IRQSTAT_TIMER0_SHIFT)

enum VIOC_TIMER_IRQ_ID {
	VIOC_TIMER_IRQ_TIMER0 = 0,
	VIOC_TIMER_IRQ_TIMER1,
	VIOC_TIMER_IRQ_TIREQ0,
	VIOC_TIMER_IRQ_TIREQ1
};

void VIOC_TIMER_SetUsecEnable(void __iomem *reg, int en, unsigned int utime);
unsigned int VIOC_TIMER_GetCurTime(void __iomem *reg);
void VIOC_TIMER_SetTimer(void __iomem *reg,
		enum VIOC_TIMER_IRQ_ID id, int en, unsigned int count);
void VIOC_TIMER_SetTiReq(void __iomem *reg,
		enum VIOC_TIMER_IRQ_ID id, unsigned int en, unsigned int time_unit);
void VIOC_TIMER_SetIrqMask(void __iomem *reg,
		enum VIOC_TIMER_IRQ_ID id, unsigned int mask);
unsigned long VIOC_TIMER_GetIrqStatus(void __iomem *reg);
void VIOC_TIMER_ClearIrqStatus(void __iomem *reg,  unsigned int mask);
void __iomem *VIOC_TIMER_GetAddress(void);

#endif
