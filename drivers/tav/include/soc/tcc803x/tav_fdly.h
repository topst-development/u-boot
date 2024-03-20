// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TAV_FDLY_H__
#define __TAV_FDLY_H__

struct TAV_TEST_CASE_FDLY {
	/* FDLYCTRL */
	int scaler_id;
	int fmt;
	/* CTRL */
	unsigned int ren;

	/* base addr */
	unsigned int base0;
	unsigned int base1;

	/* default color */
	unsigned int bg3;
	unsigned int bg2;
	unsigned int bg1;
	unsigned int bg0;

};
#endif /* __TAV_FDLY_H__ */
