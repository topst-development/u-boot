// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TAV_SCALER_H__
#define __TAV_SCALER_H__

struct TAV_TEST_CASE_SCALER {
		int id;
		int plugin;

		/* CTRL */
		unsigned int bypass;

		/* SRC SIZE */
		unsigned int src_height;
		unsigned int src_width;

		/* DST SIZE */
		unsigned int dst_height;
		unsigned int dst_width;

		/* OUTPUT POSITION */
		unsigned int ypos;
		unsigned int xpos;

		/* OUTPUT SIZE */
		unsigned int out_height;
		unsigned int out_width;
};
#endif /* __TAV_SCALER_H__ */
