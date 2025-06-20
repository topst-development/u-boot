/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCC_CLOCK_H
#define TCC_CLOCK_H

#include <common.h>
#include <asm/arch/tcc_ckc.h>
#include <clk.h>

#if 0
enum {
	CKC_OK = 0UL,
	CKC_FAILED = 1UL,
	CKC_INVALIED_SRC = 2UL,
	CKC_NO_OPS_DATA = 10UL,
	CKC_MAKE_ULONG_ENUM = ULONG_MAX, // Make enum size to unsigned long
};
#endif

enum {
	PLLDIV_0 = 0UL,	/* Disable */
	PLLDIV_2 = 2UL,	/* 1/2 */
	PLLDIV_3 = 3UL,	/* 1/3 */
	PLLDIV_4 = 4UL,	/* 1/4 */
};

extern unsigned long tcc_set_swreset(unsigned long id,
				     unsigned long op);

extern void tcc_clk_init(void);

extern unsigned long tcc_set_dpll_config(unsigned long id,
					 unsigned long modfreq,
					 unsigned long modrate,
					 unsigned long sel_pf);

extern unsigned long tcc_set_pll(unsigned long id,
				 unsigned long en,
				 unsigned long rate,
				 unsigned long tcc_div);

extern unsigned long tcc_set_pll_div(unsigned long id,
				     unsigned long tcc_div);

extern unsigned long tcc_get_pll(unsigned long id);

extern unsigned long tcc_set_clkctrl(unsigned long id,
				     unsigned long en,
				     unsigned long rate,
				     unsigned long flags);

extern unsigned long tcc_get_clkctrl(unsigned long id);

extern unsigned long tcc_set_peri(unsigned long id,
				  unsigned long en,
				  unsigned long rate,
				  unsigned long flags);

extern unsigned long tcc_get_peri(unsigned long id);

extern unsigned long tcc_set_iobus_pwdn(unsigned long id,
					bool pwdn);

extern unsigned long tcc_set_iobus_swreset(unsigned long id,
					   bool reset);

extern unsigned long tcc_set_hsiobus_pwdn(unsigned long id,
					  bool pwdn);

extern unsigned long tcc_set_hsiobus_swreset(unsigned long id,
					     bool reset);

extern unsigned long tcc_set_cpubus_pwdn(unsigned long id,
					 bool pwdn);

extern unsigned long tcc_set_cpubus_swreset(unsigned long id,
					    bool reset);

extern int tcc_clk_get_by_index(struct udevice *dev,
				int index,
				struct clk *pclk);

void clock_init(void);
void clock_init_early(void);

#endif /* TCC_CLOCK_H */
