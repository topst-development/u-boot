/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_TIMER_REG_H
#define TCC_TIMER_REG_H

#define TIMER0              0x00U
#define TIMER1              0x10U
#define TIMER2              0x20U
#define TIMER3              0x30U
#define TIMER4              0x40U
#define TIMER5              0x50U
#define TCC_TIREQ           0x60U

#define TIMER_TC32EN        0x80U
#define TIMER_TC32LDV       0x84U
#define TIMER_TC32CMP0      0x88U
#define TIMER_TC32CMP1      0x8CU
#define TIMER_TC32PCNT      0x90U
#define TIMER_TC32MCNT      0x94U
#define TIMER_TC32IRQ       0x98U

/* registers for 16/20-bit timer */
#define TIMER_TCFG          0x00U
#define TIMER_TCNT          0x04U
#define TIMER_TREF          0x08U
#define TIMER_TMREF         0x0CU
#define TIMER_OFFSET        0x10U

/* registers for 32-bit timer */
#define TC32EN_LDM1         ((u32)0x1 << 29)
#define TC32EN_LDM0         ((u32)0x1 << 28)
#define TC32EN_STOPMODE     ((u32)0x1 << 26)
#define TC32EN_LOADZERO     ((u32)0x1 << 25)
#define TC32EN_EN           ((u32)0x1 << 24)

/* fields for TCFG */
#define TCFG_STOP           ((u32)0x1 << 9)
#define TCFG_CC             ((u32)0x1 << 8)
#define TCFG_POL            ((u32)0x1 << 7)
#define TCFG_TCKSEL(x)      ((((u32)(x)) & (u32)0x7) << (u32)4)
#define TCFG_IEN            ((u32)0x1 << 3)
#define TCFG_PWM            ((u32)0x1 << 2)
#define TCFG_CON            ((u32)0x1 << 1)
#define TCFG_EN             (u32)(0x1)

#endif
