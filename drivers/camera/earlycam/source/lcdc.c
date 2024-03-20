// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>

#include <tcc_fb.h>
#include <asm/telechips/vioc/reg_physical.h>
#include <asm/telechips/vioc/vioc_config.h>
#include <asm/telechips/vioc/vioc_wmix.h>
#include <asm/telechips/vioc/vioc_rdma.h>

#include "camera.h"

#include "tcc_lcd_interface.h"
#include "lcdc.h"

static void __iomem	*rdmabsse;
static void __iomem	*sg_wmixbase;

static int cache_disp_busy_state = -1;

int tcc_lcd_is_disp_busy(void)
{
	const struct viocmg_info	*viocmg;
	const struct vinpath_info	*vinpath;

	int display_rdma = 0;
	int ret;

	if (cache_disp_busy_state == -1) {
		viocmg	= &parameters_data.m_viocmg_info;
		vinpath	= &viocmg->path_info;

		/* coverity[misra_c_2012_rule_10_8_violation : FALSE] */
		if (vinpath->display_rdma < (unsigned int)INT_MAX) {
			display_rdma = (int)vinpath->display_rdma;
		}

		ret = lcdc_is_display_device_ready(display_rdma);
		cache_disp_busy_state = ret;
	} else {
		ret = cache_disp_busy_state;
	}

	return ret;
}

static void tcc_lcd_set_rdma_fmt(unsigned int fmt)
{
	if ((fmt >= (unsigned int)TCC_LCDC_IMG_FMT_UYVY) &&
		(fmt <= (unsigned int)TCC_LCDC_IMG_FMT_YUV422ITL1)) {
		VIOC_RDMA_SetImageR2YEnable(rdmabsse, 0U);
		VIOC_RDMA_SetImageY2REnable(rdmabsse, 1U);
	} else {
		VIOC_RDMA_SetImageR2YEnable(rdmabsse, 0U);
		VIOC_RDMA_SetImageY2REnable(rdmabsse, 0U);
	}

	VIOC_RDMA_SetImageFormat(rdmabsse, fmt);
}

int tcc_lcd_set_rdma(unsigned int addr_y,
	unsigned int width, unsigned int height, unsigned int fmt)
{
	const struct viocmg_info	*viocmg;
	const struct vinpath_info	*vinpath;

	viocmg		= &parameters_data.m_viocmg_info;
	vinpath		= &parameters_data.m_viocmg_info.path_info;

	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	rdmabsse	= (void __iomem *)((uintptr_t)((unsigned long)HwVIOC_RDMA00
		+ ((unsigned)RDMA_OFFSET * (unsigned long)vinpath->display_rdma)));
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	sg_wmixbase	= (void __iomem *)((uintptr_t)((unsigned long)HwVIOC_WMIX0
		+ ((unsigned)RDMA_OFFSET * (unsigned long)viocmg->main_display_id)));

	VIOC_RDMA_SetImageY2RMode(rdmabsse, 0);

	tcc_lcd_set_rdma_fmt(fmt);

	VIOC_RDMA_SetImageOffset(rdmabsse, fmt, width);
	VIOC_RDMA_SetImageSize(rdmabsse, width, height);

	// image address
	VIOC_RDMA_SetImageBase(rdmabsse, addr_y, 0, 0);

	return 0;
}

void tcc_lcd_rear_camera_display(unsigned char on_off)
{
	const struct viocmg_info	*viocmg;
	struct vinpath_info		*vinpath	= NULL;
	static int			initialized	= 0;

	viocmg		= &parameters_data.m_viocmg_info;
	vinpath	= &parameters_data.m_viocmg_info.path_info;

	if (initialized == 0) {
		initialized = tcc_lcd_is_disp_busy();
	}

	if (initialized == 1) {
		if (on_off == 1U) {	// display on.
			VIOC_WMIX_SetOverlayPriority(sg_wmixbase, vinpath->ovp);
			VIOC_RDMA_SetImageSize(rdmabsse,
				vinpath->preview_width,
				vinpath->preview_height);
			VIOC_RDMA_SetImageEnable(rdmabsse);
			vinpath->mode = 2;

		} else {
			VIOC_WMIX_SetOverlayPriority(sg_wmixbase,
				viocmg->main_display_ovp);
			VIOC_RDMA_SetImageDisable(rdmabsse);
			vinpath->mode = 0;
		}

		VIOC_WMIX_SetUpdate(sg_wmixbase);
	}
}

void tcc_lcd_rear_camera_wmix_channel_prioty(void)
{
#if 0
	void __iomem	*wmix_alpha = &sg_wmixbase->uROPC0;

	VIOC_WMIX_ALPHA_SetAlphaSelection(wmix_alpha, 0x3);
	VIOC_WMIX_ALPHA_SetROPMode(wmix_alpha, 0x3);
#endif
	VIOC_WMIX_SetOverlayPriority(sg_wmixbase, 24);
	VIOC_WMIX_SetUpdate(sg_wmixbase);
}
