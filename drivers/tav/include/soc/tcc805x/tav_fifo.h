// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TAV_FIFO_H__
#define __TAV_FIFO_H__

struct TAV_TEST_CASE_FIFO {
		int id;
		int ch;

		/* wdma rate */
		int wdma_id;
		int rdma_id;

		/* CTRL */
		unsigned int eempty;
		unsigned int efull;
		unsigned int wmt;
		unsigned int nentry;
		unsigned int rmt;

		/* base addr */
		unsigned int base[4];
};
#endif /* __TAV_SCALER_H__ */
