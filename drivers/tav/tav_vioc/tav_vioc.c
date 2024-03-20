// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <tav_vioc.h>

#include <lcd.h>
#include <asm/io.h>
#include <asm/telechips/vioc/vioc_global.h>
#include <asm/telechips/vioc/reg_physical.h>
#include <asm/telechips/vioc/vioc_disp.h>
#include <asm/telechips/vioc/vioc_rdma.h>
#include <asm/telechips/vioc/vioc_wmix.h>
#include <asm/telechips/vioc/vioc_scaler.h>
#include <asm/telechips/vioc/vioc_config.h>
#include <asm/telechips/vioc/vioc_fdly.h>

#include <asm/io.h>
#include <asm/telechips/vioc/vioc_rdma.h>
#include <asm/telechips/vioc/vioc_global.h>

int tcc_tav_vioc_setup(struct tav_test_case_t *test_case)
{
	int i;

	if (!test_case) {
		pr_err("[ERR][TAV_VIOC] %s test_case is NULL\r\n", __func__);
		return -1;
	}

	for (i = 0; i < 2; i++) {
		if (test_case->wmix[i].skip_swreset == 0) {
			if (
				get_vioc_type(test_case->wmix[i].id) ==
				get_vioc_type(VIOC_WMIX))
				tcc_tav_vioc_wmix_reset(&test_case->wmix[i]);
		}
	}

	for (i = 0; i < 4; i++) {
		if (
			get_vioc_type(test_case->rdma[i].id) ==
			get_vioc_type(VIOC_RDMA)) {
			pr_info(
				"[INF][TAV_VIOC] %s process RDMA[%d]\r\n",
				__func__,
				get_vioc_index(test_case->rdma[i].id));
			tcc_tav_vioc_rdma_setup(&test_case->rdma[i]);
		}
	}

	for (i = 0; i < 2; i++) {
		if (
			get_vioc_type(test_case->wmix[i].id) ==
			get_vioc_type(VIOC_WMIX)) {
			pr_info(
				"[INF][TAV_VIOC] %s process WMIX[%d]\r\n",
				__func__,
				get_vioc_index(test_case->wmix[i].id));
			tcc_tav_vioc_wmix_setup(&test_case->wmix[i]);
		}
	}

	for (i = 0; i < 2; i++) {
		if (
			get_vioc_type(test_case->wdma[i].id) ==
			get_vioc_type(VIOC_WDMA)) {
			pr_info(
				"[INF][TAV_VIOC] %s process WDMA[%d]\r\n",
				__func__,
				get_vioc_index(test_case->wdma[i].id));
			tcc_tav_vioc_wdma_setup(&test_case->wdma[i]);
		}
	}

	tcc_tav_vioc_scaler_setup(&test_case->scaler);

	/* Enable DMAs */
	for (i = 0; i < 4; i++) {
		if (
			get_vioc_type(test_case->rdma[i].id) ==
			get_vioc_type(VIOC_RDMA)) {
			pr_info(
				"[INF][TAV_VIOC] %s enable RDMA[%d]\r\n",
				__func__,
				get_vioc_index(test_case->rdma[i].id));
			tcc_tav_vioc_rdma_enable(&test_case->rdma[i]);
		}
	}

	for (i = 0; i < 2; i++) {
		if (
			get_vioc_type(test_case->wdma[i].id) ==
			get_vioc_type(VIOC_WDMA)) {
			pr_info(
				"[INF][TAV_VIOC] %s enable WDMA[%d]\r\n",
				__func__,
				get_vioc_index(test_case->wdma[i].id));
			tcc_tav_vioc_wdma_enable(&test_case->wdma[i]);
		}
	}
	return 0;
}
