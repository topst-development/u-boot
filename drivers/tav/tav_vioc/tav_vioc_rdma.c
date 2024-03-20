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

int tcc_tav_vioc_rdma_enable(struct TAV_TEST_CASE_RDMA *test_case_rdma)
{
	int ret = -1;
	void __iomem *rdma_base;

	if (!test_case_rdma)
		goto error_api;

	rdma_base = VIOC_RDMA_GetAddress(test_case_rdma->id);
	if (!rdma_base)
		goto error_api;
	if (test_case_rdma->en)
		VIOC_RDMA_SetImageEnable(rdma_base);
	else
		VIOC_RDMA_SetImageDisable(rdma_base);
	return 0;
error_api:
	return ret;
}

int tcc_tav_vioc_rdma_setup(struct TAV_TEST_CASE_RDMA *test_case_rdma)
{
	int ret = -1;
	void __iomem *rdma_base;

	if (!test_case_rdma) {
		pr_err(
			"[ERR][TAV_RDMA] %s test_case_rdma is NULL\r\n",
			__func__);
		goto error_api;
	}

	rdma_base = VIOC_RDMA_GetAddress(test_case_rdma->id);
	if (!rdma_base) {
		pr_err("[ERR][TAV_RDMA] %s rdma_base is NULL\r\n", __func__);
		goto error_api;
	}

	if (VIOC_CONFIG_DMAPath_Support()) {
		VIOC_CONFIG_DMAPath_UnSet(test_case_rdma->id);
		VIOC_CONFIG_DMAPath_Set(test_case_rdma->id, test_case_rdma->id);
	}

	if (test_case_rdma->en == 0) {
		VIOC_RDMA_SetImageSize(rdma_base, 0, 0);
	} else {
		/* Set Mixer bypass */
		VIOC_CONFIG_WMIXPath(
			test_case_rdma->id,
			(test_case_rdma->bypass == 1) ? 0 : 1);

		//VIOC_RDMA_SetImageBfield(rdma_base, test_case_rdma->intl);
		//VIOC_RDMA_SetImageBf(rdma_base;
		VIOC_RDMA_SetImageIntl(rdma_base, test_case_rdma->intl);
		VIOC_RDMA_SetImageFormat(rdma_base, test_case_rdma->fmt);
		VIOC_RDMA_SetImageSize(
			rdma_base, test_case_rdma->width,
			test_case_rdma->height);
		VIOC_RDMA_SetImageOffset(
			rdma_base, test_case_rdma->fmt, test_case_rdma->width);

		/* SWAP */
		VIOC_RDMA_SetImageRGBSwapMode(rdma_base, test_case_rdma->swap);

		/* UVI */
		VIOC_RDMA_SetImageUVIEnable(rdma_base, test_case_rdma->uvi);

		/* ALPHA */
		VIOC_RDMA_SetImageAlpha(
			rdma_base, test_case_rdma->a0_2, test_case_rdma->a1_3);
		VIOC_RDMA_SetImageAlphaEnable(rdma_base, test_case_rdma->aen);
		VIOC_RDMA_SetImageAlphaSelect(rdma_base, test_case_rdma->asel);

		/* Y2R */
		VIOC_RDMA_SetImageY2RMode(rdma_base, test_case_rdma->y2rmd);
		VIOC_RDMA_SetImageY2REnable(rdma_base, test_case_rdma->y2r);

		/* R2Y */
		VIOC_RDMA_SetImageR2YEnable(rdma_base, test_case_rdma->r2y);
		VIOC_RDMA_SetImageR2YMode(rdma_base, test_case_rdma->r2ymd);

		VIOC_RDMA_SetImageUVIEnable(rdma_base, test_case_rdma->uvi);

		VIOC_RDMA_SetImageBase(
			rdma_base, test_case_rdma->base[0],
			test_case_rdma->base[1], test_case_rdma->base[2]);
	}
	VIOC_RDMA_SetImageUpdate(rdma_base);

	return 0;
error_api:
	return ret;
}
