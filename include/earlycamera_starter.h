// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef CAM_ST_H
#define	CAM_ST_H

enum {
	PMAP_TYPE_CAM_PGL = 0,
	PMAP_TYPE_CAM_VIQE,
	PMAP_TYPE_CAM_PREVIEW,
	PMAP_TYPE_CAM_MAX,
};

struct ecam_params {
	/* coverity[cert_dcl37_c_violation : FALSE] */
	uintptr_t	memory_addr[PMAP_TYPE_CAM_MAX];
};

extern struct ecam_params g_params;

extern void earlycamera_set_parameters(const struct ecam_params *parameters);
extern int prepareParkingGuideLine(void);
extern int startEarlyCamera(void);
extern int fdt_pgl_set(int enable);
extern int stopEarlyCamera(void);
#if defined(CONFIG_EARLYCAMERA_SOLUTION_CONTROLLED_BY_BOOTLOADER)
extern int enableEarlyCameraIrq(void);
extern void disableEarlyCameraIrq(void);
#endif//defined(CONFIG_EARLYCAMERA_SOLUTION_CONTROLLED_BY_BOOTLOADER)

#endif//EARLYCAMERA_STARTER_H
