// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TCC_TAV_VIOC_H__
#define __TCC_TAV_VIOC_H__

#include <tav_rdma.h>
#include <tav_wmix.h>
#include <tav_wdma.h>
#include <tav_scaler.h>
#include <tav_fifo.h>
#include <tav_fdly.h>

#define SIZE_OF_TEST_NAME 64

enum e_test_status {
	TEST_STATUS_PASS,               // pass the test case

	TEST_STATUS_RUN_PASS,           // pass run test
	TEST_STATUS_RUN_SKIP,           // skip run test

	TEST_STATUS_NO_INPUT,
	TEST_STATUS_NO_OUTPUT,
	TEST_STATUS_NO_REFERENCE,
	TEST_STATUS_NO_COMPARE,

	/* errors */
	TEST_STATUS_ERR_RUN,
	TEST_STATUS_ERR_OUTPUT,
	TEST_STATUS_ERR_INPUT,
	TEST_STATUS_ERR_REFERENCE,

	/* fail */
	TEST_STATUS_FAIL_COMPARE,
};

struct tav_test_data_t {
	int id;
	unsigned int base[3];
	unsigned int fmt;
	unsigned int width;
	unsigned int height;
};

struct tav_test_case_t {
	int major; /* Major number for Test case */
	int minor; /* Minor number for Test case */
	char test_name[SIZE_OF_TEST_NAME];

	struct TAV_TEST_CASE_WMIX wmix[2];
	struct TAV_TEST_CASE_RDMA rdma[4];
	struct TAV_TEST_CASE_WDMA wdma[2];
	struct TAV_TEST_CASE_SCALER scaler;
	struct TAV_TEST_CASE_FIFO fifo;
	struct TAV_TEST_CASE_FDLY fdly;
};

/* TAV RDMA  */
int tcc_tav_vioc_rdma_enable(struct TAV_TEST_CASE_RDMA *test_case_rdma);
int tcc_tav_vioc_rdma_setup(struct TAV_TEST_CASE_RDMA *test_case_rdma);
int tcc_tav_vioc_setup(struct tav_test_case_t *tast_case);

/* TAV WMIX */
int tcc_tav_vioc_wmix_reset(struct TAV_TEST_CASE_WMIX *test_case_wmix);
int tcc_tav_vioc_wmix_setup(struct TAV_TEST_CASE_WMIX *test_case_wmix);

/* TAV WDMA */
int tcc_tav_vioc_wdma_enable(struct TAV_TEST_CASE_WDMA *test_case_wdma);
int tcc_tav_vioc_wdma_setup(struct TAV_TEST_CASE_WDMA *test_case_wdma);

/* TAV SCALER */
int tcc_tav_vioc_scaler_setup(struct TAV_TEST_CASE_SCALER *test_case_scaler);

/* TAV FIFO */
int tcc_tav_vioc_fifo_setup(struct TAV_TEST_CASE_FIFO *test_case_fifo);

/* TAV FDLY */
int tcc_tav_vioc_fdly_setup(struct TAV_TEST_CASE_FDLY *test_case_fdly);

#endif /* __TCC_TAV_VIOC_H__ */
