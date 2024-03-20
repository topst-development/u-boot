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
#include <asm/telechips/vioc/vioc_wdma.h>
#include <asm/telechips/vioc/vioc_wmix.h>
#include <asm/telechips/vioc/vioc_scaler.h>
#include <asm/telechips/vioc/vioc_config.h>
#include <asm/telechips/vioc/vioc_fifo.h>

int tcc_tav_vioc_fifo_setup(struct TAV_TEST_CASE_FIFO *test_case_fifo)
{
	int ret = -1;
	void __iomem *fifo_base;
	void __iomem *wdma_base_src;

	if (!test_case_fifo) {
		pr_err(
			"[ERR][TAV_FIFO] %s test_case_fifo is NULL\r\n",
			__func__);
		goto error_api;
	}

	fifo_base = VIOC_FIFO_GetAddress(test_case_fifo->id);
	wdma_base_src =
		VIOC_WDMA_GetAddress(test_case_fifo->wdma_id);

	if (!fifo_base) {
		pr_err("[ERR][TAV_FIFO] %s fifo_base is NULL\r\n", __func__);
		goto error_api;
	} else {
		pr_info("fifo Base Addr = %p\n", fifo_base);
	}

	if (!wdma_base_src) {
		pr_err("[ERR][TAV_FIFO] %s wdma_base is NULL\r\n", __func__);
		goto error_api;
	} else {
		pr_info("wdma Base Addr = %p\n", fifo_base);
	}

	VIOC_FIFO_ConfigDMA(
		fifo_base, test_case_fifo->ch, test_case_fifo->wdma_id,
		test_case_fifo->rdma_id, 0, 0, 0);

	VIOC_FIFO_ConfigEntry(fifo_base, test_case_fifo->base);

	VIOC_FIFO_SetEnable(fifo_base, 1, 1, 0, 0);
	return 0;
error_api:
	return ret;
}
