// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TAV_RDMA_H__
#define __TAV_RDMA_H__

struct TAV_TEST_CASE_RDMA {
	int id;
	/* CTRL */
	unsigned int intl;
	unsigned int bfmd;
	unsigned int bf;
	unsigned int fb;
	unsigned int asel;
	unsigned int uvi;
	unsigned int r2ymd;
	unsigned int r2y;
	unsigned int pd;
	unsigned int swap;
	unsigned int aen;
	unsigned int y2rmd;
	unsigned int y2r;
	unsigned int fmt;
	unsigned int fmt10_format_type;
	unsigned int fmt10_mode;
	unsigned int en;

	/* SIZE */
	unsigned int height;
	unsigned int width;

	/* ALPHA */
	unsigned int a1_3;
	unsigned int sel;
	unsigned int a0_2;

	unsigned int base[3];

	/* WMIX mixing */
	unsigned int bypass;
};
#endif /* __TAV_RDMA_H__ */

