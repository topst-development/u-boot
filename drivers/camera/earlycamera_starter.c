// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <earlycamera_starter.h>
#include <earlycamera_main.h>
#include <debug.h>
#include <camera.h>
#include <timer_irq.h>
#ifdef CONFIG_EARLYCAMERA_SOLUTION_OVERLAY_PGL
#include "splashimg.h"
#endif/* CONFIG_EARLYCAMERA_SOLUTION_OVERLAY_PGL */

#define SCRATCHPAD	1024		/* bytes of scratchpad memory */

#ifdef CONFIG_EARLYCAMERA_SOLUTION_OVERLAY_PGL
#define CONFIG_TCC_PARKING_GUIDE_LINE_NAME "parkingline_888"
#ifndef CONFIG_SPLASH_PARTITION_NAME
#error	"Wrong configuration error: CONFIG_SPLASH_PARTITION_NAME has been set."
#else
#define	SPLASH_PARTION_NAME CONFIG_SPLASH_PARTITION_NAME
#endif	/* CONFIG_SPLASH_PARTITION_NAME */
#endif/* CONFIG_EARLYCAMERA_SOLUTION_OVERLAY_PGL */

struct ecam_params	g_params;

void earlycamera_set_parameters(const struct ecam_params *parameters)
{
	struct ecam_params	*params	= NULL;
	int			idx_buf = 0;

	params = &g_params;

	/* update memory address of earlycamera_starter */
	for (idx_buf = 0; idx_buf < PMAP_TYPE_CAM_MAX; idx_buf++) {
		params->memory_addr[idx_buf] =
			parameters->memory_addr[idx_buf];
		logd("buffer type: %d, addr: 0x%08lx\n",
			idx_buf, params->memory_addr[idx_buf]);
	}

	/* update memory address of earlycamera_main */
	parameters_data.viqe_addr		=
		g_params.memory_addr[PMAP_TYPE_CAM_VIQE];
	parameters_data.lcdc_addr0	=
		g_params.memory_addr[PMAP_TYPE_CAM_PREVIEW];
	parameters_data.PGL_addr		=
		g_params.memory_addr[PMAP_TYPE_CAM_PGL];
}

#ifdef CONFIG_EARLYCAMERA_SOLUTION_OVERLAY_PGL
static int load_parking_guideline_image(void)
{
	uintptr_t	pgl_addr	= 0;
	unsigned int	pgl_width	= 0;
	unsigned int	pgl_height	= 0;
	int		ret		= 0;

	pgl_addr = g_params.memory_addr[PMAP_TYPE_CAM_PGL];
	if (pgl_addr == 0x0U) {
		loge("PGL - Address: 0x%08lx is WRONG\n", pgl_addr);
		ret = -1;
	} else {
		/* coverity[misra_c_2012_rule_7_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_11_9_violation : FALSE] */
		ret = splash_image_load_v2((const unsigned char *)SPLASH_PARTION_NAME,
			/* coverity[misra_c_2012_rule_7_4_violation : FALSE] */
			(const unsigned char *)CONFIG_TCC_PARKING_GUIDE_LINE_NAME,
			/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_11_9_violation : FALSE] */
			(void *)pgl_addr, &pgl_width, &pgl_height, 0);
		logd("PGL - Name: %s, Address: 0x%08lx, Width: %u, Height: %u\n",
			CONFIG_TCC_PARKING_GUIDE_LINE_NAME,
			pgl_addr, pgl_width, pgl_height);
	}

	return ret;
}
#endif/* CONFIG_EARLYCAMERA_SOLUTION_OVERLAY_PGL */

int prepareParkingGuideLine(void)
{
	struct vinpath_info	*vinpath	= NULL;
	int			ret		= 0;

#ifdef CONFIG_EARLYCAMERA_SOLUTION_OVERLAY_PGL
	vinpath = &parameters_data.m_viocmg_info.path_info;

	ret = load_parking_guideline_image();
	vinpath->use_parking_line = (ret == 0) ? 1U : 0U;
	(void)fdt_pgl_set((int)vinpath->use_parking_line);
#endif/* CONFIG_EARLYCAMERA_SOLUTION_OVERLAY_PGL */

	return ret;
}

int startEarlyCamera(void)
{
	int		ret	= 0;

	(void)pr_force("## EarlyCamera Solution ##\n");

#if defined(CONFIG_EARLYCAMERA_SOLUTION_CONTROLLED_BY_BOOTLOADER)
	ret = tcc_earlycamera_start();
	if (ret < 0) {
		/* failed to start earlycamera */
		loge("ERROR: tcc_earlycamera_start, ret: %d\n", ret);
	}
#if defined(CONFIG_DM_TIMER_IRQ)
	ret = enableEarlyCameraIrq();
	if (ret < 0) {
		/* request of enabling irq for reverse switch is failed */
		loge("ERROR: request of enabling irq for reverse switch\n");
	}
#endif/* defined(CONFIG_DM_TIMER_IRQ) */
#endif/* defined(CONFIG_EARLYCAMERA_SOLUTION_CONTROLLED_BY_BOOTLOADER) */

	return ret;
}

int stopEarlyCamera(void)
{
	int		ret	= 0;

#if defined(CONFIG_EARLYCAMERA_SOLUTION_CONTROLLED_BY_BOOTLOADER)
	/* stop EarlyCamera */
	ret = tcc_earlycamera_stop();
	if (ret < 0) {
		/* failed to stop earlycamera */
		loge("ERROR: tcc_earlycamera_stop\n");
	}
#if defined(CONFIG_DM_TIMER_IRQ)
	disableEarlyCameraIrq();
#endif/* defined(CONFIG_DM_TIMER_IRQ) */
#endif/* defined(CONFIG_EARLYCAMERA_SOLUTION_CONTROLLED_BY_BOOTLOADER) */

	return ret;
}

int enableEarlyCameraIrq(void)
{
	int		ret	= 0;

	/* enable interrupt */
	ret = tcc_earlycamera_initialize_irq();
	if (ret < 0) {
		/* failed to init irq */
		loge("ERROR: tcc_camera_initialize_irq\n");
	}

	return ret;
}

void disableEarlyCameraIrq(void)
{
	/* disable interrupt */
	tcc_earlycamera_deinitialize_irq();
}
