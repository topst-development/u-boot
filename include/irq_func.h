/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Header file for interrupt functions
 *
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Modified by Copyright Telechips Inc.
 * Modified date : 2022-06
 */

#ifndef __IRQ_FUNC_H
#define __IRQ_FUNC_H

struct pt_regs;

typedef void (interrupt_handler_t)(void *arg);

int interrupt_init(void);
void timer_interrupt(struct pt_regs *regs);
void external_interrupt(struct pt_regs *regs);
void irq_install_handler(int vec, interrupt_handler_t *handler, void *arg);
void irq_free_handler(int vec);
void reset_timer(void);

void enable_interrupts(void);
int disable_interrupts(void);

#if defined(CONFIG_USE_IRQ)
s32 arch_interrupt_init(void);

void irq_mask(int vec);
void irq_unmask(int vec);
void irq_set_type(int vec, u32 type);
#endif

#endif
