// SPDX-License-Identifier: (GPL-2.0-or-later OR MIT)
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef CKC_FLAGS_H
#define CKC_FLAGS_H

#define U32(x)		((uint32_t)(x))

#ifndef BIT
#define BIT(nr)			(U32(1) << (nr))
#endif

#define XIN_CLK_RATE	(24000000UL)	// 24 MHz
#define XTIN_CLK_RATE	(32768UL)	// 32.768 KHz

#define CKC_ENABLE	1U
#define CKC_DISABLE	0U

#define CKC_RESUME	0U
#define CKC_SUSPEND	1U

/* Clock flags */
#define CLK_F_SRC_CLK_MASK	0x3FU
#define CLK_F_SRC_CLK_SHIFT	16UL
#define CLK_F_SRC_CLK(x)	(((x) & CLK_F_SRC_CLK_MASK) | CLK_F_FIXED)
#define CLK_F_GET_SRC_CLK(x)	((x) & CLK_F_SRC_CLK_MASK)
#define CLK_F_FIXED		(U32(1U) << 29)	/* Use fixed source clock */
#define CLK_F_DCO_MODE		(U32(1U) << 30)	/* Set DCO mode */
#define CLK_F_DIV_MODE		(U32(1U) << 31)	/* Set divide mode */

#define TC_CLK_F_SRC CLK_F_SRC_CLK

#endif
