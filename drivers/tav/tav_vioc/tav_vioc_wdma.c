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

int tcc_tav_vioc_wdma_enable(struct TAV_TEST_CASE_WDMA *test_case_wdma)
{
	int ret = -1;
	void __iomem *wdma_base;

	if (!test_case_wdma)
		goto error_api;

	wdma_base = VIOC_WDMA_GetAddress(test_case_wdma->id);
	if (!wdma_base)
		goto error_api;
	VIOC_WDMA_SetImageEnable(wdma_base, test_case_wdma->cont);
	return 0;
error_api:
	return ret;
}

int tcc_tav_vioc_wdma_setup(struct TAV_TEST_CASE_WDMA *test_case_wdma)
{
	int ret = -1;
	void __iomem *wdma_base;

	if (!test_case_wdma) {
		pr_err(
			"[ERR][TAV_WDMA] %s test_case_wdma is NULL\r\n",
			__func__);
		goto error_api;
	}

	wdma_base = VIOC_WDMA_GetAddress(test_case_wdma->id);
	if (!wdma_base) {
		pr_err("[ERR][TAV_WDMA] %s wdma_base is NULL\r\n", __func__);
		goto error_api;
	}
	#if defined(TCC899X) || defined(TCC901X)
	VIOC_WDMA_SetDataFormat(
		wdma_base, test_case_wdma->fmt10_format_type,
		test_case_wdma->fmt10_mode);
	#endif
	VIOC_WDMA_SetImageFormat(wdma_base, test_case_wdma->fmt);
	VIOC_WDMA_SetImageSize(
		wdma_base, test_case_wdma->width, test_case_wdma->height);
	VIOC_WDMA_SetImageOffset(
		wdma_base, test_case_wdma->fmt, test_case_wdma->width);
	VIOC_WDMA_SetImageBase(
		wdma_base, test_case_wdma->base[0], test_case_wdma->base[1],
		test_case_wdma->base[2]);

	VIOC_WDMA_SetImageR2YEnable(wdma_base, test_case_wdma->r2y);
	if (test_case_wdma->r2y == 1)
		VIOC_WDMA_SetImageR2YMode(wdma_base, test_case_wdma->r2ymd);
	VIOC_WDMA_SetImageY2REnable(wdma_base, test_case_wdma->y2r);
	if (test_case_wdma->y2r == 1)
		VIOC_WDMA_SetImageY2RMode(wdma_base, test_case_wdma->y2rmd);
	VIOC_WDMA_SetImageInterlaced(wdma_base, test_case_wdma->intl);

	VIOC_WDMA_SetImageUpdate(wdma_base);
	return 0;
error_api:
	return ret;
}
