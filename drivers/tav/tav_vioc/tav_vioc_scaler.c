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
#include <asm/telechips/vioc/vioc_wdma.h>
#include <asm/telechips/vioc/vioc_wmix.h>
#include <asm/telechips/vioc/vioc_scaler.h>
#include <asm/telechips/vioc/vioc_config.h>

int tcc_tav_vioc_scaler_setup(struct TAV_TEST_CASE_SCALER *test_case_scaler)
{
	int ret = -1;
	void __iomem *scaler_base;

	if (!test_case_scaler) {
		pr_err(
			"[ERR][TAV_WDMA] %s test_case_scaler is NULL\r\n",
			__func__);
		goto error_api;
	}

	scaler_base = VIOC_SC_GetAddress(test_case_scaler->id);
	if (!scaler_base) {
		pr_err("[ERR][TAV_WDMA] %s wdma_base is NULL\r\n", __func__);
		goto error_api;
	} else {
		pr_info("scaler Base Addr = %4p\n", scaler_base);
	}

	VIOC_SC_SetBypass(scaler_base, test_case_scaler->bypass);

	VIOC_SC_SetDstSize(
		scaler_base,
		(test_case_scaler->dst_width - test_case_scaler->xpos),
		(test_case_scaler->dst_height - test_case_scaler->ypos));
	VIOC_SC_SetOutSize(
		scaler_base,
		(test_case_scaler->dst_width - test_case_scaler->xpos),
		(test_case_scaler->dst_height - test_case_scaler->ypos));
	VIOC_SC_SetOutPosition(scaler_base, 0, 0);
	VIOC_CONFIG_PlugIn(test_case_scaler->id, test_case_scaler->plugin);
	VIOC_SC_SetUpdate(scaler_base);
	return 0;
error_api:
	return ret;
}
