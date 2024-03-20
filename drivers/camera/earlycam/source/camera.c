// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <debug.h>

#include <asm/telechips/vioc/reg_physical.h>
#include <asm/telechips/vioc/vioc_global.h>
#include <asm/telechips/vioc/vioc_config.h>
#include <asm/telechips/vioc/vioc_vin.h>
#include <asm/telechips/vioc/vioc_viqe.h>
#include <asm/telechips/vioc/vioc_scaler.h>
#include <asm/telechips/vioc/vioc_wmix.h>
#include <asm/telechips/vioc/vioc_wdma.h>
#include <asm/telechips/vioc/vioc_rdma.h>
#include <asm/telechips/vioc/vioc_fifo.h>
#include <asm/telechips/vioc/vioc_disp.h>
#include <dm/device.h>

#include "../include/camera.h"
#include "../include/precision.h"
#include "../include/dev/videosource_if.h"
#include "../include/lcdc.h"

#include <common.h>
#include <asm/telechips/gpio.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <stdint.h>

static void __iomem		*ireqconfig;
static void __iomem		*ddiconfig;
static void __iomem		*pglbase;
static void __iomem		*vinbase;
static void __iomem		*viqebase;
static void __iomem		*scbase;
static void __iomem		*wmixbase;
static void __iomem		*wdmabase;
static void __iomem		*rdmabase;

#define PREVIEW_BUFFER_NUMBER	4U
#define PREVIEW_SIZE_MAX	16384U
static unsigned int prev_buffers[PREVIEW_BUFFER_NUMBER];

static void tcc_cif_dump_registers(void)
{
	struct reg_test {
		unsigned int	*reg;
		unsigned int	cnt;
	};
	int i = 0;
	const struct reg_test reg_list[] = {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		{ (unsigned int *)pglbase,	16 },
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		{ (unsigned int *)vinbase,	16 },
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		{ (unsigned int *)viqebase,	 5 },
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		{ (unsigned int *)scbase,	 8 },
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		{ (unsigned int *)wmixbase,	28 },
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		{ (unsigned int *)wdmabase,	16 },
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		{ (unsigned int *)rdmabase,	16 },
	};
	const unsigned int	*reg_addr;
	unsigned int		idx_loop;
	unsigned int		n_loop;
	unsigned int		idx_reg;
	unsigned int		n_reg;

	for (i = 0; i < 3; i++) {
		logd("\n\n");

		n_loop = (int)ARRAY_SIZE(reg_list);
		for (idx_loop = 0; idx_loop < n_loop; idx_loop++) {
			reg_addr = reg_list[idx_loop].reg;
			n_reg = reg_list[idx_loop].cnt;

			for (idx_reg = 0; idx_reg < n_reg; idx_reg++) {
				if ((idx_reg % 4U) == 0U)
					logd("\n0x%lx: ",
						(uintptr_t)reg_addr + idx_reg);
				logd("%08x ", readl((uintptr_t)reg_addr + idx_reg));
			}
			logd("\n");
		}
		logd("\n\n");
	}
}

void tcc_cif_dump_parameters(void)
{
	const struct vs_format		*vs_info;
	const struct viocmg_info	*viocmg;
	const struct vinpath_info	*vinpath;

	vs_info		= &parameters_data.vsrc->format;
	viocmg		= &parameters_data.m_viocmg_info;
	vinpath	= &viocmg->path_info;

	logd("v_pol: %u\n",		vs_info->v_pol);
	logd("h_pol: %u\n",		vs_info->h_pol);
	logd("p_pol: %u\n",		vs_info->p_pol);
	logd("de_active_low: %u\n",	vs_info->de_active_low);
	logd("field_bfield_low: %u\n",	vs_info->field_bfield_low);
	logd("gen_field_en: %u\n",	vs_info->gen_field_en);
	logd("conv_en: %u\n",		vs_info->conv_en);
	logd("hsde_connect_en: %u\n",	vs_info->hsde_connect_en);
	logd("vs_mask: %u\n",		vs_info->vs_mask);
	logd("data_format: %u\n",	vs_info->data_format);
	logd("data_order: %u\n",	vs_info->data_order);
	logd("interlaced: %u\n",	vs_info->interlaced);
	logd("intpl_en: %u\n",		vs_info->intpl_en);
	logd("width: %u\n",		vs_info->width);
	logd("height: %u\n",		vs_info->height);

	logd("lcdc_addr0: 0x%lx\n",	parameters_data.lcdc_addr0);
	logd("lcdc_addr1: 0x%lx\n",	parameters_data.lcdc_addr1);
	logd("lcdc_addr2: 0x%lx\n",	parameters_data.lcdc_addr2);
	logd("Guide_line_area: 0x%lx\n",parameters_data.PGL_addr);
	logd("viqe_addr: 0x%lx\n",	parameters_data.viqe_addr);

	logd("main_display_id: %u\n",	viocmg->main_display_id);
	logd("main_display_port: %u\n",	viocmg->main_display_port);
	logd("main_display_ovp: %u\n",	viocmg->main_display_ovp);

	logd("enable: %u\n",		vinpath->enable);
	logd("use_viqe: %u\n",		vinpath->use_viqe);
	logd("viqe_mode: %u\n",		vinpath->viqe_mode);
	logd("use_parking_line: %u\n",	vinpath->use_parking_line);

	logd("cifport: %u\n",		vinpath->cifport);
	logd("vin_vin: %u\n",		vinpath->vin_vin);
	logd("vin_rdma: %u\n",		vinpath->vin_rdma);
	logd("vin_wmix: %u\n",		vinpath->vin_wmix);
	logd("vin_wdma: %u\n",		vinpath->vin_wdma);
	logd("vin_scaler: %u\n",	vinpath->vin_scaler);
	logd("display_rdma: %u\n",	vinpath->display_rdma);

	logd("mode: %u\n",		vinpath->mode);
	logd("ovp: %u\n",		vinpath->ovp);

	logd("preview_x: %u\n",		vinpath->preview_x);
	logd("preview_y: %u\n",		vinpath->preview_y);
	logd("preview_width: %u\n",	vinpath->preview_width);
	logd("preview_height: %u\n",	vinpath->preview_height);
	logd("preview_format: %u\n",	vinpath->preview_format);

	logd("preview_add_w: %u\n",	vinpath->preview_add_width);
	logd("preview_add_h: %u\n",	vinpath->preview_add_height);

	logd("pgl_pos_x: %u\n",		vinpath->parking_line_x);
	logd("pgl_pos_y: %u\n",		vinpath->parking_line_y);
	logd("pgl_width: %u\n",		vinpath->parking_line_width);
	logd("pgl_height: %u\n",	vinpath->parking_line_height);
	logd("pgl_format: %u\n",	vinpath->parking_line_format);
}

int tcc_cif_get_camera_type(void)
{
	return parameters_data.vsrc->interface;
}

void tcc_cif_set_viocs_addr(void)
{
	const struct vinpath_info *vinpath;
	vinpath	= &parameters_data.m_viocmg_info.path_info;

	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	ddiconfig	= (void __iomem *)(uintptr_t)HwDDI_CONFIG_BASE;
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	ireqconfig	= (void __iomem *)(uintptr_t)HwVIOC_CONFIG;

	pglbase		= VIOC_RDMA_GetAddress(vinpath->vin_rdma);
	vinbase		= VIOC_VIN_GetAddress(vinpath->vin_vin);
	viqebase	= VIOC_VIQE_GetAddress(vinpath->vin_viqe);
	scbase		= VIOC_SC_GetAddress(vinpath->vin_scaler);
	wmixbase	= VIOC_WMIX_GetAddress(vinpath->vin_wmix);
	wdmabase	= VIOC_WDMA_GetAddress(vinpath->vin_wdma);
	rdmabase	= VIOC_RDMA_GetAddress(vinpath->display_rdma);
}

struct tcc_cif_parameters parameters_data = {
	.cif_port_num	= 0,

	.viqe_addr	= 0,
	.lcdc_addr0	= 0,
	.lcdc_addr1	= 0,
	.lcdc_addr2	= 0,
	.PGL_addr	= 0,

	.m_viocmg_info	= {
		.main_display_id		= 0,
		.main_display_ovp		= 24,

		.path_info	= {
			.enable			= 1,
			.use_viqe		= 1,
			.viqe_mode		= 1,
			.use_parking_line	= 0,

			.cifport		= 0,
			.vin_vin		= 0,
			.vin_rdma		= 16,
			.vin_wmix		= 5,
			.vin_wdma		= 5,
			.vin_scaler		= 2,
			.vin_viqe		= 1,
			.display_rdma		= 1,
			.display_fifo		= 0,
			.ovp			= 16,
			.preview_x		= 0,
			.preview_y		= 0,

			.preview_crop_x		= 30,
			.preview_crop_y		= 5,

			.preview_width		= 1920,
			.preview_height		= 720,

			.preview_format		= VIOC_IMG_FMT_ARGB8888,

			.preview_add_width	= 30,
			.preview_add_height	= 6,

			.parking_line_x		= 0,
			.parking_line_y		= 0,
			.parking_line_width	= 1920,
			.parking_line_height	= 720,
			/* 12: TCC_LCDC_IMAG_FMT_RGB888 */
			.parking_line_format	= 12,
		},
	},
};

static int tcc_cif_reset_vioc_path(void)
{
	const struct viocmg_info	*viocmg	=
		&parameters_data.m_viocmg_info;
	const struct vinpath_info	*vinpath =
		&viocmg->path_info;
	const unsigned int	viocs[] = {
		((u32)VIOC_RDMA		+ vinpath->vin_rdma),
		((u32)VIOC_VIN		+ vinpath->vin_vin),
		((u32)VIOC_VIQE		+ vinpath->vin_viqe),
		((u32)VIOC_SCALER	+ vinpath->vin_scaler),
		((u32)VIOC_WMIX		+ vinpath->vin_wmix),
		((u32)VIOC_WDMA		+ vinpath->vin_wdma),
		((u32)VIOC_RDMA		+ vinpath->display_rdma),
		((u32)VIOC_FIFO		+ vinpath->display_fifo),
	};
	int			idx_vioc	= 0;
	int			n_vioc		= 0;

	n_vioc = (int)ARRAY_SIZE(viocs);

	// reset
	for (idx_vioc = n_vioc - 1; idx_vioc >= 0; idx_vioc--) {
		VIOC_CONFIG_SWReset(viocs[idx_vioc], VIOC_CONFIG_RESET);
	}

	mdelay(1);

	// reset clear
	for (idx_vioc = 0; idx_vioc < n_vioc; idx_vioc++) {
		/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
		VIOC_CONFIG_SWReset(viocs[idx_vioc], VIOC_CONFIG_CLEAR);
	}

	return 0;
}

int tcc_cif_set_rdma(void __iomem *rdma,
	unsigned int base_addr, unsigned int width, unsigned int height,
	unsigned int fmt)
{
	logd("rdma: 0x%p, base_addr: 0x%08x, width: %u, height: %u\n",
		rdma, base_addr, width, height);

	VIOC_RDMA_SetImageFormat(rdma, fmt);
	VIOC_RDMA_SetImageSize(rdma, width, height);
	VIOC_RDMA_SetImageOffset(rdma, fmt, width);
	VIOC_RDMA_SetImageBase(rdma, base_addr, 0x0, 0x0);
	VIOC_RDMA_SetImageEnable(rdma);

	return 0;
}

static int tcc_cif_set_wmix(void __iomem *wmix,
	unsigned int width, unsigned int height)
{
	const static struct vinpath_info	 *path_info;

	unsigned int		layer		= 0x0;
	unsigned int		key_R		= 0xFF;
	unsigned int		key_G		= 0xFF;
	unsigned int		key_B		= 0xFF;
	unsigned int		key_mask_R	= 0xF8;
	unsigned int		key_mask_G	= 0xF8;
	unsigned int		key_mask_B	= 0xF8;

	path_info = &parameters_data.m_viocmg_info.path_info;

	if ((width > 0U) && (height > 0U)) {
		VIOC_WMIX_SetSize(wmix, width, height);
	} else {
		loge("The size for wmixer is abnormal: must be larger than 0\n");
	}

	if (path_info->use_parking_line == 1U) {
		VIOC_WMIX_SetChromaKey(wmix,
			layer, ON,
			key_R, key_G, key_B,
			key_mask_R, key_mask_G, key_mask_B);

		if (path_info->vin_wmix == 5U) {
			(void)VIOC_CONFIG_WMIXPath((unsigned)WMIX50, 1U);
		} else { // path_info->vin_wmix == 4
			(void)VIOC_CONFIG_WMIXPath((unsigned)WMIX40, 1U);
		}
	} else {
		if (path_info->vin_wmix == 5U) {
			(void)VIOC_CONFIG_WMIXPath((unsigned)WMIX50, 0U);
		} else { // path_info->vin_wmix == 4
			(void)VIOC_CONFIG_WMIXPath((unsigned)WMIX40, 0U);
		}
	}

	VIOC_WMIX_SetUpdate(wmix);

	return 0;
}

static void tcc_cif_set_vin_size(unsigned int width, unsigned int height)
{
	VIOC_VIN_SetImageSize(vinbase, width, height);
	VIOC_VIN_SetImageOffset(vinbase, 0, 0, 0);
	VIOC_VIN_SetImageCropSize(vinbase, width, height);
	VIOC_VIN_SetImageCropOffset(vinbase, 0, 0);
}

static void tcc_cif_set_vin_fmt(const struct vinpath_info *vinpath,
	unsigned int data_format,
	unsigned int interlaced, unsigned int intpl_en)
{
	VIOC_VIN_SetY2RMode(vinbase, 2);
	logd("data_format: 0x%08x, YUV422_16BIT: 0x%08x, YUV422_8BIT: 0x%08x\n",
		data_format, (unsigned int)FMT_YUV422_16BIT,
		(unsigned int)FMT_YUV422_8BIT);
	logd("preview_format: 0x%x, VIOC_IMG_FMT_ARGB8888: 0x%x\n",
		vinpath->preview_format,
		(unsigned int)VIOC_IMG_FMT_ARGB8888);
	if (((data_format == (unsigned int)FMT_YUV422_16BIT) ||
		(data_format == (unsigned int)FMT_YUV422_8BIT)) &&
		(vinpath->preview_format == (unsigned int)VIOC_IMG_FMT_ARGB8888)) {
		logd("interlaced: 0x%08x\n", interlaced);
		if (!((interlaced == (unsigned int)ON) &&
			(vinpath->use_viqe == 1U))) {
			VIOC_VIN_SetY2REnable(vinbase, (unsigned int)ON);
		}
	} else {
		VIOC_VIN_SetY2REnable(vinbase, (unsigned int)OFF);
	}

	VIOC_VIN_SetInterlaceMode(vinbase, interlaced, intpl_en);
}

static void tcc_cif_set_vin(void)
{
	const struct vs_format		*vs_info;
	const struct vinpath_info	*vinpath;

	unsigned int	data_order		= 0;
	unsigned int	data_format		= 0;
	unsigned int	gen_field_en		= 0;
	unsigned int	de_active_low		= 0;
	unsigned int	field_bfield_low	= 0;
	unsigned int	vs_active_low		= 0;
	unsigned int	hs_active_low		= 0;
	unsigned int	pxclk_pol		= 0;
	unsigned int	vs_mask			= 0;
	unsigned int	hsde_connect_en		= 0;
	unsigned int	intpl_en		= 0;
	unsigned int	conv_en			= 0;
	unsigned int	interlaced		= 0;
	unsigned int	width			= 0;
	unsigned int	height			= 0;

	vs_info		= &parameters_data.vsrc->format;
	vinpath		= &parameters_data.m_viocmg_info.path_info;

	data_order		= vs_info->data_order;
	data_format		= vs_info->data_format;
	gen_field_en		= vs_info->gen_field_en;
	de_active_low		= vs_info->de_active_low;
	field_bfield_low	= vs_info->field_bfield_low;
	vs_active_low		= vs_info->v_pol;
	hs_active_low		= vs_info->h_pol;
	pxclk_pol		= vs_info->p_pol;
	vs_mask			= vs_info->vs_mask;
	hsde_connect_en		= vs_info->hsde_connect_en;
	intpl_en		= vs_info->intpl_en;
	conv_en			= vs_info->conv_en;
	interlaced		= vs_info->interlaced;
	width			= vs_info->width;
	if (interlaced >= PRECISION(UINT_MAX)) {
		height		= vs_info->height;
	} else {
		height		= vs_info->height >> interlaced;
	}

	VIOC_VIN_SetSyncPolarity(vinbase,
		hs_active_low, vs_active_low, field_bfield_low,
		de_active_low, gen_field_en, pxclk_pol);
	VIOC_VIN_SetCtrl(vinbase,
		conv_en, hsde_connect_en, vs_mask, data_format, data_order);
	tcc_cif_set_vin_size(width, height);
	tcc_cif_set_vin_fmt(vinpath, data_format, interlaced, intpl_en);

	VIOC_VIN_SetLUTEnable(vinbase, OFF, OFF, OFF);

	VIOC_VIN_SetEnable(vinbase, ON);
}

static void set_viqe_y2r(void)
{
	VIOC_VIQE_SetImageY2RMode(viqebase, 2);
	VIOC_VIQE_SetImageY2REnable(viqebase, ON);
}

static void tcc_cif_set_viqe(unsigned int is_viqe_used)
{
	const struct vs_format		*vs_info;
	const struct vinpath_info	*vinpath;

	unsigned int		type		= 0;
	unsigned int		plugin_val	= 0;

	vs_info		= &parameters_data.vsrc->format;
	vinpath		= &parameters_data.m_viocmg_info.path_info;

	type		= (unsigned int)VIOC_VIQE + vinpath->vin_viqe;
	plugin_val	= (unsigned int)VIOC_VIN + vinpath->vin_vin;

	if ((plugin_val >= (unsigned int)VIOC_VIN) &&
		(type >= (unsigned int)VIOC_VIQE) && (is_viqe_used == 1U)) {
		unsigned int	interlaced	= vs_info->interlaced;
		unsigned int	width		= vs_info->width;
		unsigned int	height;
		if (interlaced >= PRECISION(UINT_MAX)) {
			height		= vs_info->height;
		} else {
			height		= vs_info->height >> interlaced;
		}

		unsigned int	viqe_width	= 0;
		unsigned int	viqe_height	= 0;
		unsigned int	format		= FMT_FC_YUV422;
		unsigned int	offset		= 0;
		if ((width < PREVIEW_SIZE_MAX) && (height < PREVIEW_SIZE_MAX)) {
			offset			= width * height * 2U * 2U;
		}
		unsigned int	deintl_base0	= 0;
		if (parameters_data.viqe_addr < UINT_MAX) {
			deintl_base0 = (unsigned int)parameters_data.viqe_addr;
		}

		unsigned int	deintl_base1	= deintl_base0 + offset;
		unsigned int	deintl_base2	= deintl_base1 + offset;
		unsigned int	deintl_base3	= deintl_base2 + offset;
		enum VIOC_VIQE_DEINTL_MODE bypass_deintl
						= VIOC_VIQE_DEINTL_MODE_3D;

		unsigned int	cdf_lut_en	= OFF;
		unsigned int	his_en		= OFF;
		unsigned int	gamut_en	= OFF;
		unsigned int	d3d_en		= OFF;
		unsigned int	deintl_en	= ON;

		(void)VIOC_CONFIG_PlugIn(type, plugin_val);

		if (((vs_info->data_format == (unsigned)FMT_YUV422_16BIT) ||
			(vs_info->data_format == (unsigned)FMT_YUV422_8BIT)) &&
			(vinpath->preview_format == (unsigned)VIOC_IMG_FMT_ARGB8888)) {
			set_viqe_y2r();
		}
		VIOC_VIQE_SetControlRegister(viqebase,
			viqe_width, viqe_height, format);
		VIOC_VIQE_SetDeintlRegister(viqebase,
			format, OFF, viqe_width, viqe_height, bypass_deintl,
			(unsigned int)deintl_base0, (unsigned int)deintl_base1,
			(unsigned int)deintl_base2, (unsigned int)deintl_base3);
		VIOC_VIQE_SetControlEnable(viqebase,
			cdf_lut_en, his_en, gamut_en, d3d_en, deintl_en);
		VIOC_VIQE_SetDeintlModeWeave(viqebase);
	}
}

static int tcc_cif_set_scaler(unsigned int width, unsigned int height)
{
	const struct vinpath_info	*vinpath;

	unsigned int		type		= 0;
	unsigned int		plugin_val	= 0;
	unsigned int		dstW		= 0;
	unsigned int		dstH		= 0;
	unsigned int		dstH_adj	= 0;

	vinpath	= &parameters_data.m_viocmg_info.path_info;

	if ((UINT_MAX - (unsigned int)VIOC_SCALER) > vinpath->vin_scaler) {
		type	= (unsigned int)VIOC_SCALER + vinpath->vin_scaler;
	}
	if ((UINT_MAX - plugin_val) > vinpath->vin_vin) {
		plugin_val = (unsigned int)VIOC_VIN + vinpath->vin_vin;
	}
	if ((UINT_MAX - width) > vinpath->preview_add_width) {
		dstW = width  + vinpath->preview_add_width;
	}
	if ((UINT_MAX - height) > vinpath->preview_add_height) {
		dstH = height + vinpath->preview_add_height;
	}

	(void)VIOC_CONFIG_PlugIn(type, plugin_val);
	VIOC_SC_SetBypass(scbase, OFF);

	dstH_adj = dstH + 1U;
	if (dstH_adj > dstH) {
		VIOC_SC_SetDstSize(scbase, dstW, (dstH + 1U));
	} else {
		loge("Precision error!\n");
	}

	VIOC_SC_SetOutPosition(scbase,
		vinpath->preview_crop_x,
		vinpath->preview_crop_y);
	VIOC_SC_SetOutSize(scbase, width, height);
	VIOC_SC_SetUpdate(scbase);

	return 0;
}

static void tcc_cif_set_wdma(void __iomem *wdma,
	unsigned int base_addr, unsigned int width, unsigned int height,
	unsigned int fmt)
{
	VIOC_WDMA_SetImageFormat(wdma, fmt);
	VIOC_WDMA_SetImageSize(wdma, width, height);
	VIOC_WDMA_SetImageOffset(wdma, fmt, width);

	//set base address for write DMA.
	VIOC_WDMA_SetImageBase(wdma, base_addr, 0, 0);
	VIOC_WDMA_SetImageEnable(wdma, ON);

	// wait for 4 frames because of VIQE 3D mode operation
	mdelay(16 * 3);
}

#if 0
static void tcc_cif_enable_fifo(unsigned int *buf, unsigned int wdma,
	unsigned int rdma0, unsigned int rdma1, unsigned int rdma2)
{
	void __iomem	*pFIFO = (void __iomem *)HwVIOC_FIFO;

	logd("wdma: %u, rdma0: %u, rdma1: %u, rdma2: %u\n",
		wdma, rdma0, rdma1, rdma2);

#if 0
	VIOC_FIFO_ConfigDMA(pFIFO, vinpath_info->display_fifo,
		wdma, rdma0, rdma1, rdma2, 0);
#endif
	VIOC_FIFO_ConfigEntry(pFIFO, buf);
	VIOC_FIFO_SetEnable(pFIFO, 1, 1, 0, 0);
}

static void tcc_cif_disable_fifo(void)
{
	void __iomem	*pFIFO	= NULL;
	uintptr_t	addr	= (uintptr_t)HwVIOC_FIFO;

	pFIFO = (void __iomem *)addr;

	VIOC_FIFO_SetEnable(pFIFO, 0, 0, 0, 0);
}
#endif

static void tcc_cif_set_cifport(struct vinpath_info *vinpath,
	const struct vs_format *vs_info)
{
	void __iomem	*reg = NULL;
	unsigned long		val = 0;

	// set cif port
	/* coverity[misra_c_2012_rule_10_8_violation : FALSE] */
	if (vs_info->cif_port < (INT_MAX / 4)) {
		vinpath->cifport = (unsigned int)vs_info->cif_port;
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		reg = (void __iomem *)((uintptr_t)ddiconfig + 0x0008U);

		if (((unsigned long)vinpath->vin_vin * 4UL) >=
			PRECISION(UINT_MAX)) {
			logd("Wrong precision with vin_vin value: %u\n",
				vinpath->vin_vin);
		} else {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[cert_dcl37_c_violation : FALSE] */
			writel(vinpath->cifport <<
				(vinpath->vin_vin * 4U), reg);
		}

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		/* coverity[cert_dcl37_c_violation : FALSE] */
		/* coverity[cert_int36_c_violation : FALSE] */
		val = readl(reg);

		logd("CIF Port Mapping - 0x%p: 0x%08lx\n", reg, val);
	} else {
		loge("Wrong precision with cif_port: %d\n", vs_info->cif_port);
	}
}

static void tcc_cif_set_pgl(const struct vinpath_info *vinpath)
{
	// set parking guide line RDMA
	if (vinpath->use_parking_line == 1U) {
		/* Global alpha settings of parking guide-line*/
		VIOC_RDMA_SetImageAlphaEnable(pglbase, 1);
		VIOC_RDMA_SetImageAlpha(pglbase, 0xff, 0xff);

		if (parameters_data.PGL_addr < UINT_MAX) {
			(void)tcc_cif_set_rdma(pglbase,
				(unsigned int)parameters_data.PGL_addr,
				vinpath->parking_line_width,
				vinpath->parking_line_height,
				vinpath->parking_line_format);
		} else {
			loge("Set rdma failed due to PGL address\n");
		}
	}
}

static void tcc_cif_set_vioc(const struct vinpath_info *vinpath,
	const struct vs_format *vs_info)
{
	// set VIN
	tcc_cif_set_vin();

	// set Deinterlacer
	if (vs_info->interlaced == 1U) {
		tcc_cif_set_viqe(vinpath->use_viqe);
	}

	// set SCaler
	if ((vinpath->preview_width != vs_info->width) ||
		(vinpath->preview_height != vs_info->height)) {
		(void)tcc_cif_set_scaler(vinpath->preview_width,
			vinpath->preview_height);
	}

	// set WMIXer
	if (vinpath->vin_vin < 3U) {
		(void)tcc_cif_set_wmix(wmixbase,
			vinpath->preview_width,
			vinpath->preview_height);
	}

	// set WDMA
	if (parameters_data.lcdc_addr0 < UINT_MAX) {
		tcc_cif_set_wdma(wdmabase,
			(unsigned int)parameters_data.lcdc_addr0,
			vinpath->preview_width,
			vinpath->preview_height,
			vinpath->preview_format);

		if (tcc_lcd_is_disp_busy() == 1) {
			// set Display
			(void)tcc_lcd_set_rdma(
				(unsigned int)parameters_data.lcdc_addr0,
				vinpath->preview_width,
				vinpath->preview_height,
				vinpath->preview_format);

#if 0
			// set AsyncFIFO
			tcc_cif_enable_fifo(prev_buffers,
				vinpath->vin_wdma,
				vinpath->display_rdma,
				vinpath->display_rdma,
				vinpath->display_rdma);
#endif
		}
	}



}

void tcc_cif_start_stream(void)
{
	const struct vs_format		*vs_info;
	struct viocmg_info		*viocmg;
	struct vinpath_info		*vinpath;

	unsigned int		offset = 0;
	unsigned int		idx_buf = 0;

	vs_info		= &parameters_data.vsrc->format;
	viocmg		= &parameters_data.m_viocmg_info;
	vinpath		= &viocmg->path_info;

	// allocate preview memory
	if ((vinpath->preview_width <= PREVIEW_SIZE_MAX) &&
		(vinpath->preview_height <= PREVIEW_SIZE_MAX)) {
		offset = vinpath->preview_width *
			vinpath->preview_height * PREVIEW_BUFFER_NUMBER;
	} else {
		offset = 0;
	}

	for (idx_buf = 0; idx_buf < PREVIEW_BUFFER_NUMBER; idx_buf++) {
		if (parameters_data.lcdc_addr0 < UINT_MAX) {
			prev_buffers[idx_buf] =
				(unsigned int)parameters_data.lcdc_addr0;
		}
		if ((offset * idx_buf) < (UINT_MAX - prev_buffers[idx_buf])) {
			prev_buffers[idx_buf] += (offset * idx_buf);
		}
		logd("prev_buffers[%u] = 0x%08x\n",
			idx_buf, prev_buffers[idx_buf]);
	}

	// clear 1st memory.
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	(void)memset((void *)parameters_data.lcdc_addr0, 0,
			((unsigned long)vinpath->preview_width *
				(unsigned long)vinpath->preview_height * 4UL));


	// reset vioc component
	(void)tcc_cif_reset_vioc_path();

	tcc_cif_set_cifport(vinpath, vs_info);
	tcc_cif_set_pgl(vinpath);
	tcc_cif_set_vioc(vinpath, vs_info);
}

static void disable_wdma(void)
{
	int			idx_loop	= 0;
	unsigned int		status		= 0;
	unsigned int 		baddr;
	unsigned int 		caddr;
	int			eofr_mask;

	VIOC_WDMA_SetIreqMask(wdmabase, VIOC_WDMA_IREQ_ALL_MASK, ON);
	VIOC_WDMA_SetImageDisable(wdmabase);

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	/* coverity[cert_dcl37_c_violation : FALSE] */
	/* coverity[cert_int36_c_violation : FALSE] */
	baddr	= readl((void *)((uintptr_t)wdmabase +
				(unsigned)WDMABASE0_OFFSET));
	caddr	= VIOC_WDMA_Get_CAddress(wdmabase);
	/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
	eofr_mask	= VIOC_WDMA_IREQ_EOFR_MASK;
	if (baddr != caddr) {
		for (idx_loop = 0; idx_loop < 7; idx_loop++) {
			VIOC_WDMA_GetStatus(wdmabase, &status);
			if ((status & (unsigned int)eofr_mask) > 0U) {
				// eof interrupt occurs
				break;
			}

			logi("[%02d] WDMA: %p, STATUS: 0x%08x\n",
				idx_loop, wdmabase, status);
			mdelay(10);
		}
	}
}

void tcc_cif_stop_stream(void)
{
	const struct vs_format		*vs_info;
	const struct viocmg_info	*viocmg;
	const struct vinpath_info	*vinpath;

	vs_info		= &parameters_data.vsrc->format;
	viocmg		= &parameters_data.m_viocmg_info;
	vinpath		= &viocmg->path_info;

#if 0
	// disable AsyncFIFO
	tcc_cif_disable_fifo();
#endif

	// disable RDMA
	VIOC_RDMA_SetImageDisable(rdmabase);

	// disable WDMA
	disable_wdma();

	// disable WMIX, but don't care

	// disable SCaler
	if ((UINT_MAX - (u32)VIOC_SCALER) >= (vinpath->vin_scaler)) {
		(void)VIOC_CONFIG_PlugOut((u32)VIOC_SCALER + vinpath->vin_scaler);
	}

	// disable Deinterlacer
	if ((vs_info->interlaced == 1U) && (vinpath->use_viqe == 1U)) {
		VIOC_VIQE_SetControlEnable(viqebase, OFF, OFF, OFF, OFF, OFF);
		(void)VIOC_CONFIG_PlugOut((u32)VIOC_VIQE + vinpath->vin_viqe);
	}

	// disable VIN
	VIOC_VIN_SetEnable(vinbase, OFF);
	mdelay(1);

	// disable PGL
	if (vinpath->use_parking_line == 1U) {
		VIOC_RDMA_SetImageDisable(pglbase);
	}

	// reset vioc component
	(void)tcc_cif_reset_vioc_path();
}

int tcc_cif_check_video_input_path(void)
{
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	uintptr_t wdmabase_addr 		= (uintptr_t)wdmabase;
	wdmabase_addr 				= wdmabase_addr + 0x10U;

	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	const void __iomem	*reg		=
		(const void __iomem *)(wdmabase_addr);
	unsigned long		prev_addr	= 0;
	unsigned long		curr_addr	= 0;
	int			n_check		= 0;
	int			idx_check	= 0;
	u64			delay		= 16UL;
	int			ret		= -1;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_11_8_violation : FALSE] */
	/* coverity[cert_dcl37_c_violation : FALSE] */
	/* coverity[cert_int36_c_violation : FALSE] */
	curr_addr = readl(reg);
	mdelay(delay);

	n_check = 3;
	for (idx_check = 0; idx_check < n_check; idx_check++) {
		prev_addr = curr_addr;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		/* coverity[cert_dcl37_c_violation : FALSE] */
		/* coverity[cert_int36_c_violation : FALSE] */
		curr_addr = readl(reg);
		mdelay(delay);

		if (prev_addr != curr_addr) {
			// path is working
			ret = 0;
		}

		// path is not working
		logd("[%d] prev_addr: 0x%08lx, curr_addr: 0x%08lx\n",
			idx_check, prev_addr, curr_addr);
	}
	logd("WDMA Current Address is the same.\n");

	return ret;
}

void tcc_cif_recovery_video_input_path(void)
{
	logd("The video-input path is not working.\n");

	tcc_cif_dump_registers();

	tcc_cif_stop_stream();
	tcc_cif_start_stream();
}

static void init_videosource(void)
{
	struct udevice		*dev		= NULL;

	dev = videosource_if_get_videosource();
	(void)videosource_if_close(dev);
	(void)videosource_if_open(dev);
	(void)videosource_if_tune(dev, tcc_cif_get_camera_type(),
		(int)CAM_ENC_DEFAULT);
}

void tcc_cif_recovery(void)
{
	logd("The video-input path is not working.\n");

	tcc_cif_dump_registers();
	tcc_cif_stop_stream();
	init_videosource();
	tcc_cif_start_stream();
}
