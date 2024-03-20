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

int tcc_tav_vioc_fdly_setup(struct TAV_TEST_CASE_FDLY *test_case_fdly)
{
	int ret = -1;
	void __iomem *fdly_base;

	if (!test_case_fdly) {
		pr_err(
			"[ERR][TAV_FIFO] %s test_case_fdly is NULL\r\n",
			__func__);
		goto error_api;
	}

	fdly_base = VIOC_FDLY_GetAddress();
	if (!fdly_base) {
		pr_err("[ERR][TAV_FIFO] %s fdly_base is NULL\r\n", __func__);
		goto error_api;
	} else {
		pr_info("fdly Base Addr = %p\n", fdly_base);
	}

	VIOC_FDLY_SetBase0(fdly_base, test_case_fdly->base0);
	VIOC_FDLY_SetBase1(fdly_base, test_case_fdly->base1);
	VIOC_FDLY_SetFormat(fdly_base, 1);
	VIOC_FDLY_SetDefaultColor(
		fdly_base, test_case_fdly->bg3, test_case_fdly->bg2,
		test_case_fdly->bg1, test_case_fdly->bg0);

	VIOC_FDLY_PlugIn(test_case_fdly->scaler_id, 1);
	return 0;

error_api:
	return ret;
}
