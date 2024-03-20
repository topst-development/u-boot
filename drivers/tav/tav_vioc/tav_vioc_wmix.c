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

int tcc_tav_vioc_wmix_reset(struct TAV_TEST_CASE_WMIX *test_case_wmix)
{
	int i;

	struct VIOC_PATH_INFO_T wmix_info;

	if (!test_case_wmix) {
		pr_err(
			"[ERR][TAV_WMIX] %s test_case_wmix is NULL\r\n",
			__func__);
		goto error_api;
	}

	if (VIOC_CONFIG_Get_Path_Info(test_case_wmix->id, &wmix_info) < 0) {
		pr_err(
			"[ERR][TAV_WMIX] %s failed to VIOC_CONFIG_Get_Path_Info\n",
			__func__);
		goto error_api;
	}

	/* Disable Display Device */
	if (get_vioc_type(wmix_info.disp) == get_vioc_type(VIOC_DISP)) {
		pr_err(
			"[ERR][TAV_WMIX] %s Turn off DEV_%d\n", __func__,
			get_vioc_index(wmix_info.disp));
		VIOC_DISP_TurnOff(VIOC_DISP_GetAddress(wmix_info.disp));
	}

	/* Disable WDMA */
	for (i = 0; i < 2; i++) {
		if (
			get_vioc_type(wmix_info.wdma[i]) ==
			get_vioc_type(VIOC_WDMA)) {
			pr_err(
				"[ERR][TAV_WMIX] %s Turn off WDMA_%d\n",
				__func__, get_vioc_index(wmix_info.wdma[i]));
			VIOC_WDMA_SetImageDisable(
				VIOC_WDMA_GetAddress(wmix_info.wdma[i]));
		}
	}

	/* Disable RDMA */
	for (i = 0; i < wmix_info.mix; i++) {
		if (
			get_vioc_type(wmix_info.rdma[i]) ==
			get_vioc_type(VIOC_RDMA)) {
			pr_err(
				"[ERR][TAV_WMIX] %s Turn off RDMA_%d\n",
				__func__, get_vioc_index(wmix_info.rdma[i]));
			VIOC_RDMA_SetImageDisable(
				VIOC_RDMA_GetAddress(wmix_info.rdma[i]));
		}
	}

	mdelay(10);

	/* Reset WDMA */
	for (i = 0; i < 2; i++) {
		if (
			get_vioc_type(wmix_info.wdma[i]) ==
			get_vioc_type(VIOC_WDMA)) {
			VIOC_CONFIG_SWReset(
				wmix_info.wdma[i], VIOC_CONFIG_RESET);
		}
	}

	if (get_vioc_type(wmix_info.wmix) == get_vioc_type(VIOC_WMIX))
		VIOC_CONFIG_SWReset(wmix_info.wmix, VIOC_CONFIG_RESET);

	/* Reset RDMA */
	for (i = 0; i < wmix_info.mix; i++) {
		if (
			get_vioc_type(wmix_info.rdma[i]) ==
			get_vioc_type(VIOC_RDMA)) {
			VIOC_CONFIG_SWReset(
				wmix_info.rdma[i], VIOC_CONFIG_RESET);
		}
	}

	/* Reset RDMA */
	for (i = 0; i < wmix_info.mix; i++) {
		if (
			get_vioc_type(wmix_info.rdma[i]) ==
			get_vioc_type(VIOC_RDMA)) {
			VIOC_CONFIG_SWReset(
				wmix_info.rdma[i], VIOC_CONFIG_CLEAR);
		}
	}

	if (get_vioc_type(wmix_info.wmix) == get_vioc_type(VIOC_WMIX))
		VIOC_CONFIG_SWReset(wmix_info.wmix, VIOC_CONFIG_CLEAR);

	/* Reset WDMA */
	for (i = 0; i < 2; i++) {
		if (
			get_vioc_type(wmix_info.wdma[i]) ==
			get_vioc_type(VIOC_WDMA)) {
			VIOC_CONFIG_SWReset(
				wmix_info.wdma[i], VIOC_CONFIG_CLEAR);
		}
	}
error_api:
	return 0;
}

int tcc_tav_vioc_wmix_setup(struct TAV_TEST_CASE_WMIX *test_case_wmix)
{
	int i, j, ret = -1;
	unsigned int pa, pb, psel0, psel1;
	unsigned int aa, ab, asel0, asel1;
	void __iomem *wmix_base;

	struct	VIOC_PATH_INFO_T wmix_info;

	if (!test_case_wmix) {
		pr_err(
			"[ERR][TAV_WMIX] %s test_case_wmix is NULL\r\n",
			__func__);
		goto error_api;
	}

	wmix_base = VIOC_WMIX_GetAddress(test_case_wmix->id);
	if (!wmix_base) {
		pr_err(
			"[ERR][TAV_WMIX] %s wmix_base is NULL\r\n", __func__);
		goto error_api;
	}

	if (VIOC_CONFIG_Get_Path_Info(test_case_wmix->id, &wmix_info) < 0) {
		pr_err(
			"[ERR][TAV_WMIX] %s failed to VIOC_CONFIG_Get_Path_Info\n",
			__func__);
		goto error_api;
	}

	VIOC_WMIX_SetOverlayPriority(wmix_base, test_case_wmix->ovp);

	pr_err(
		"[INF][TAV_WMIX] %s size %dx%d, mix = %d\r\n", __func__,
		test_case_wmix->width, test_case_wmix->height, wmix_info.mix);
	VIOC_WMIX_SetSize(
		wmix_base, test_case_wmix->width, test_case_wmix->height);
	VIOC_WMIX_SetBGColor(
		wmix_base, test_case_wmix->bg[0], test_case_wmix->bg[1],
		test_case_wmix->bg[2], test_case_wmix->bg[3]);

	for (i = 0; i < wmix_info.mix; i++) {
		if (
			get_vioc_type(wmix_info.rdma[i]) ==
			get_vioc_type(VIOC_RDMA)) {
			VIOC_WMIX_SetPosition(
				wmix_base, i, test_case_wmix->pos[i].xpos,
				test_case_wmix->pos[i].ypos);
		}
	}

	/* Chroma */
	for (i = 0; i < (wmix_info.mix - 1); i++) {
		if (test_case_wmix->mix[i].key.key_en == 1) {
			VIOC_WMIX_SetChromaKey(
				wmix_base, 0,
				test_case_wmix->mix[i].key.key_en,
				test_case_wmix->mix[i].key.key_r,
				test_case_wmix->mix[i].key.key_g,
				test_case_wmix->mix[i].key.key_b,
				test_case_wmix->mix[i].key.mask_r,
				test_case_wmix->mix[i].key.mask_g,
				test_case_wmix->mix[i].key.mask_b);
		}
	}

	/* Alpha */
	for (i = 0; i < (wmix_info.mix - 1); i++) {
		for (j = 0; j < 4; j++) {

			if (test_case_wmix->mix[i].region[j].valid != 1)
				continue;
			pa = test_case_wmix->mix[i].region[j].pixel_result_a;
			pb = test_case_wmix->mix[i].region[j].pixel_result_b;
			psel0 = test_case_wmix->mix[i].region[j].pixel_sel0_out;
			psel1 = test_case_wmix->mix[i].region[j].pixel_sel1_out;

			aa = test_case_wmix->mix[i].region[j].alpha_result_a;
			ab = test_case_wmix->mix[i].region[j].alpha_result_b;
			asel0 = test_case_wmix->mix[i].region[j].alpha_sel0_out;
			asel1 = test_case_wmix->mix[i].region[j].alpha_sel1_out;

			VIOC_WMIX_ALPHA_SetColorControl(
				wmix_base, i, j,
				((pa & 3) << 2) | (psel0 & 3),
				((pb & 3) << 2) | (psel1 & 3));
			VIOC_WMIX_ALPHA_SetAlphaValueControl(
				wmix_base, i, j,
				((aa & 3) << 1) | (asel0 & 1),
				((ab & 3) << 1) | (asel1 & 1));
		}
		if (test_case_wmix->mix[i].rop.valid == 1) {
			VIOC_WMIX_ALPHA_SetROPMode(
				wmix_base, i,
				test_case_wmix->mix[i].rop.ropmode);
			VIOC_WMIX_ALPHA_SetAlphaSelection(
				wmix_base, i, test_case_wmix->mix[i].rop.asel);
			VIOC_WMIX_ALPHA_SetAlphaValue(
				wmix_base, i,
				test_case_wmix->mix[i].rop.alpha0,
				test_case_wmix->mix[i].rop.alpha1);
		}
	}

	VIOC_WMIX_SetUpdate(wmix_base);

	ret = 0;
error_api:
	return ret;
}
