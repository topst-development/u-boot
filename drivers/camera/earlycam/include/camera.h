// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <dm/device.h>

#include "dev/videosource_if.h"

struct vinpath_info {
	/* feature */
	unsigned int enable;
	unsigned int use_viqe;
	/* 0: VIOC_VIQE_DEINTL_MODE_BYPASS
	 * 1: VIOC_VIQE_DEINTL_MODE_2D
	 * 2: VIOC_VIQE_DEINTL_MODE_3D
	 */
	unsigned int viqe_mode;
	unsigned int use_parking_line;

	/* VIOC Components */
	unsigned int cifport;
	unsigned int vin_rdma;
	unsigned int vin_vin;
	unsigned int vin_viqe;
	unsigned int vin_scaler;
	unsigned int vin_wmix;
	unsigned int vin_wdma;
	unsigned int display_rdma;
	unsigned int display_fifo;

	/* Early cam status */
	/* 0: normal mode
	 * 1: rear cam mode
	 */
	unsigned int mode;
	unsigned int ovp;

	/* Early cam preview */
	unsigned int preview_x;
	unsigned int preview_y;

	unsigned int preview_crop_x;
	unsigned int preview_crop_y;

	unsigned int preview_width;
	unsigned int preview_height;
	unsigned int preview_format;

	unsigned int preview_add_width;
	unsigned int preview_add_height;

	/* Early cam parking line */
	unsigned int parking_line_x;
	unsigned int parking_line_y;
	unsigned int parking_line_width;
	unsigned int parking_line_height;
	unsigned int parking_line_format;
};

struct viocmg_info {
	/* fbdisplay */
	unsigned int main_display_id;
	unsigned int main_display_port;
	unsigned int main_display_ovp;	/* default ovp */

	/* ecam_info */
	struct vinpath_info		path_info;
};

struct tcc_cif_parameters {
	struct videosource	*vsrc;

	int			cif_port_num;

	uintptr_t viqe_addr;
	uintptr_t lcdc_addr0;
	uintptr_t lcdc_addr1;
	uintptr_t lcdc_addr2;
	uintptr_t PGL_addr;

	struct viocmg_info m_viocmg_info;
};

extern struct tcc_cif_parameters parameters_data;

extern void tcc_cif_set_viocs_addr(void);
extern void tcc_cif_start_stream(void);
extern void tcc_cif_stop_stream(void);
extern int tcc_cif_check_video_input_path(void);
extern void tcc_cif_recovery_video_input_path(void);
extern void tcc_cif_recovery(void);
extern int tcc_cif_get_camera_type(void);
extern void tcc_cif_dump_parameters(void);
extern int tcc_cif_set_rdma(void __iomem *rdma,
	unsigned int base_addr, unsigned int width, unsigned int height,
	unsigned int fmt);

#endif
