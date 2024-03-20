// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TAV_WDMA_H__
#define __TAV_WDMA_H__

struct TAV_TEST_CASE_WDMA {
	int id;
	/* CTRL */
	unsigned int intl;
	unsigned int fu;
	unsigned int cont;
	unsigned int y2rmd;
	unsigned int y2r;
	unsigned int swap;
	unsigned int r2ymd;
	unsigned int r2y;
	unsigned int fmt10_format_type;
	unsigned int fmt10_mode;
	unsigned int fmt;

	/* SIZE */
	unsigned int height;
	unsigned int width;

	/* Background */
	unsigned int bg3;
	unsigned int bg2;
	unsigned int bg1;
	unsigned int bg0;

	/* Base Address */
	unsigned int base[3];
};
#endif /* __TAV_WDMA_H__ */
