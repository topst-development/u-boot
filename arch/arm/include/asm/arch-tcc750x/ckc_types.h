// SPDX-License-Identifier: (GPL-2.0-or-later OR MIT)
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef CKC_TYPES_H
#define CKC_TYPES_H

enum {
	CKC_OK			= 0,
	CKC_FAILED		= 1,
	CKC_INVALID_SRC		= 2,
	CKC_INVALID_ID		= 4,
	CKC_INVALID_SIP		= 8,
	CKC_NO_OPS		= 16,
	CKC_NO_OPS_FN		= 32,
	CKC_NO_OPS_DATA		= 64,
};


typedef struct {
	uint32_t	clk_version_major;
	uint32_t	clk_version_minor;
	uint32_t	clk_version_patch;
} tCLK_VERSION_INFO;

typedef struct {
	uint32_t	status;
	uint32_t	is_main;
} tCLK_SYNC_INFO;

typedef struct {
	uint64_t	id;
	uint32_t	offset;
	bool		status;
	bool		inv_status;
} tCLK_RST_INFO;

typedef struct {
	uintptr_t	pllpms;
	uintptr_t	pllcon;
	uintptr_t	pllmon;
} tPLL_ADDR;

/* PLL clock info structure */
typedef struct {
	uint64_t	id;
	/* common bits */
	uint64_t	rate;
	uint32_t	en;
	uint32_t	p;
	uint32_t	m;
	uint32_t	s;
	uint32_t	src;
	uint32_t	bypass;
	/* dither only */
	uint32_t	mfr;
	uint32_t	mrr;
	uint32_t	sscg_en;
	uint32_t	sel_pf;

	int16_t		k; /* k can have negative value */
} tPLLCLKCTRL;

/* BUS clock info structure */
typedef struct {
	uint64_t	id;
	uint32_t	flags;
	uint64_t	rate;
	uint32_t	en;
	uint32_t	config;
	uint32_t	sel;
} tBCLKCTRL;

/* Peripheral clock info structure */
typedef struct {
	uint64_t	id;
	uint32_t	flags;
	uint64_t	rate;
	uint32_t	md;	// For backward compatibility (DCO peri register)
	uint32_t	en;
	uint32_t	sel;
	uint32_t	divider;
} tPCLKCTRL;

/* Divider clock info structure */
typedef struct {
	uint64_t	id;
	uint32_t	src_id;
	uint32_t	src_type;
	uint32_t	en;
	uint32_t	en_offset;
	uint32_t	divider;
	uint32_t	div_offset;
	uint32_t	div_mask;
	uint64_t	rate;
} tDCLKCTRL;

/* Gate clock info structure */
typedef tCLK_RST_INFO tGCLKCTRL;


/* External clock info structure */
typedef struct {
	uint64_t	id;
	uint32_t	en;
	uint32_t	rate;
} tEXTCLKCTRL;


/*
 * Per-core clock info structure for managing max clock frequency.
 * check_dsu_clk_nned_change() & update_max_clk() will use this structure.
 *
 * In ES, we handle MP_CA76, MP_CA55, SP_CA55 core clocks as a group.
 * In CS, we need to handle per-core clocks of MP_CA76, MP_CA55, SP_CA55.
 * It is inefficient read per-core clocks everytime when change each core clock.
 * So we'll caching each core clocks by using struct core_clk_info.
 *
 * Because each clusters (main / sub) will not change other cluster's core clock,
 * we don't need to worry about clock info syncronization.
 */
struct core_clk_info {
	uint64_t	id;
	uint64_t 	rate;
};

typedef struct {
	uint64_t	id;
	uint64_t 	*clk_src;
	uint32_t 	length;
	uint32_t 	config_min;
	uint32_t 	config_max;
} tSRCINFO;

#endif
