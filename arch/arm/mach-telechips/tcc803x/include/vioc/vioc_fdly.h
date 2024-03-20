/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef VIOC_FDLY_H
#define VIOC_FDLY_H

/************************************************************************
 *   Frame Delay				(Base Addr = 0x12003900)
 *************************************************************************/
#if 0
struct VIOC_FDLY_CTRL {
	unsigned FMT : 1;
	unsigned reserved1: 15;
	unsigned reserved2: 16;
};

union VIOC_FDLY_CTRL_u {
	unsigned long nREG;
	struct VIOC_FDLY_CTRL bREG;
};

struct VIOC_FDLY_RATE {
	unsigned reserved1: 16;
	unsigned MAXRATE : 8;
	unsigned reserved2: 7;
	unsigned REN : 1;
};

union VIOC_FDLY_RATE_u {
	unsigned long nREG;
	struct VIOC_FDLY_RATE bREG;
};

struct VIOC_FDLY_BG {
	unsigned BG0 : 8;
	unsigned BG1 : 8;
	unsigned BG2 : 8;
	unsigned BG3 : 8;
};

union VIOC_FDLY_BG_u {

	unsigned long nREG;
	struct VIOC_FDLY_BG bREG;
};

struct VIOC_FDLY {
	union VIOC_FDLY_CTRL_u uCTRL; // 0x00  R/W	0x00000000	Frame
				// Delay Control Register
	union VIOC_FDLY_RATE_u uRATE; // 0x04	R/W	0x00000000	Frame
				// Delay Rate Control Register
	unsigned
		uBASE0; // 0x08	R/W	0x00000000	Frame Delay
			// Base Address 0 Register
	unsigned
		uBASE1; // 0x0C	R/W	0x00000000	Frame Delay
			// Base Address 1 Register
	union VIOC_FDLY_BG_u
		uBG;	// 0x10	R/W	0x0x000000	Frame
				// Delay Default Color Register
	unsigned int reserved0[3]; // 5,6,7
};
#endif

/*
 * Register offset
 */
#define FDLYCTRL (0x00U)
#define FDLYRATE (0x04U)
#define FDLYRBASE0 (0x08U)
#define FDLYRBASE1 (0x0CU)
#define FDLYRFLT (0x10U)

/*
 * Frame Delay Control Registers
 */
#define FDLYCTRL_FMT_SHIFT (0U)

#define FDLYCTRL_FMT_MASK ((u32)0x1U << FDLYCTRL_FMT_SHIFT)

/*
 * Frame Delay Rate Control Registers
 */
#define FDLYRATE_REN_SHIFT (31U)
#define FDLYRATE_MAXRATE_SHIFT (16U)

#define FDLYRATE_REN_MASK ((u32)0x1U << FDLYRATE_REN_SHIFT)
#define FDLYRATE_MAXRATE_MASK ((u32)0x7FFFU << FDLYRATE_MAXRATE_SHIFT)

/*
 * Frame Delay Base Address 0 Registers
 */
#define FDLYBASE_BASE0_SHIFT (0)

#define FDLYBASA_BASE0_MASK (0xFFFFFFFFU << FDLYBASE_BASE0_SHIFT)

/*
 * Frame Delay Base Address 1 Registers
 */
#define FDLYBASE_BASE1_SHIFT (0U)

#define FDLYBASA_BASE1_MASK ((u32)0xFFFFFFFFU << FDLYBASE_BASE1_SHIFT)

/*
 * Frame Delay Default Color Resgister
 */
#define FDLYDFLT_BG3_SHIFT (24U)
#define FDLYDFLT_BG2_SHIFT (16U)
#define FDLYDFLT_BG1_SHIFT (8U)
#define FDLYDFLT_BG0_SHIFT (0U)

#define FDLYDFLT_BG3_MASK ((u32)0xFFU << FDLYDFLT_BG3_SHIFT)
#define FDLYDFLT_BG2_MASK ((u32)0xFFU << FDLYDFLT_BG2_SHIFT)
#define FDLYDFLT_BG1_MASK ((u32)0xFFU << FDLYDFLT_BG1_SHIFT)
#define FDLYDFLT_BG0_MASK ((u32)0xFFU << FDLYDFLT_BG0_SHIFT)

/* Interface APIs */
extern void VIOC_FDLY_SetFormat(void __iomem *reg, unsigned int fmt);
extern void VIOC_FDLY_SetRate(void __iomem *reg, unsigned int rate);
extern void VIOC_FDLY_SetRateEnable(void __iomem *reg, unsigned int en);
extern void VIOC_FDLY_SetBase0(void __iomem *reg, unsigned int baseAddress);
extern void VIOC_FDLY_SetBase1(void __iomem *reg, unsigned int baseAddress);
extern void VIOC_FDLY_SetDefaultColor(
	void __iomem *reg, unsigned int bg3, unsigned int bg2,
	unsigned int bg1, unsigned int bg0);
extern void VIOC_FDLY_PlugIn(unsigned int sc, unsigned int en);
extern void VIOC_FDLY_Dump(void __iomem *reg);
extern void __iomem *VIOC_FDLY_GetAddress(void);
#endif
