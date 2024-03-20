// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_CLOCK_H
#define TCC_CLOCK_H

#include <common.h>
#include <asm/telechips/tcc_ckc.h>
#include <clk.h>

enum {
	CKC_OK = 0UL,
	CKC_FAILED = 1UL,
	CKC_INVALIED_SRC = 2UL,
	CKC_NO_OPS_DATA = 10UL,
	CKC_MAKE_ULONG_ENUM = ULONG_MAX, // Make enum size to unsigned long
};

enum {
	PLLDIV_0 = 0UL,	/* Disable */
	PLLDIV_2 = 2UL,	/* 1/2 */
	PLLDIV_3 = 3UL,	/* 1/3 */
	PLLDIV_4 = 4UL,	/* 1/4 */
};

/* Clock flags */
#define CLK_F_SKIP_SSCG		(1 << 28)
#define CLK_F_SRC_CLK_MASK	0x3f
#define CLK_F_SRC_CLK(x)	(((x) & CLK_F_SRC_CLK_MASK) | CLK_F_FIXED)
#define CLK_F_FIXED		(1 << 29)	/* Use fixed source clock */
#define CLK_F_DCO_MODE		(1 << 30)	/* Set DCO mode */
#define CLK_F_DIV_MODE		(1 << 31)	/* Set divide mode */

extern unsigned long tcc_set_dpll_config(unsigned long id,
					 unsigned long modfreq,
					 unsigned long modrate);
extern unsigned long tcc_set_pll(unsigned long id,
				 unsigned long en,
				 unsigned long rate,
				 unsigned long tcc_div);
extern unsigned long tcc_get_pll(unsigned long id);
extern unsigned long tcc_set_clkctrl(unsigned long id,
				     unsigned long en,
				     unsigned long rate);
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
				struct clk *clk);

void clock_init(void);
void clock_init_early(void);

#endif /* TCC_CLOCK_H */
