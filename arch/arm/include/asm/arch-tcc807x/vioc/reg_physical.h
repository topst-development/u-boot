/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef PLATFORM_REG_PHYSICAL_H
#define PLATFORM_REG_PHYSICAL_H

/*
 * VIOC register 4K align remap enable
 * -----------------------------------
 * The base address of the VIOC Component register is remapped according to
 * the VIOC_REMAP.REMAP_EN setting.
 *
 * VIOC_REMAP.REMAP_EN is set, the register address of the VIOC components
 * is remapped to 4K byte align.
 * This means that the address of the VIOC components is at least 4K byte apart.
 * Refer to VIOC Overall Register Map for details.
 */
#include <linux/bitops.h>
#include <asm/arch/vioc/vioc_ddicfg.h>

#define is_VIOC_REMAP 0U

#if 0
static inline unsigned int is_VIOC_REMAP(void) {
	return VIOC_DDICONFIG_GetViocRemap();
}
#endif

struct TCC_DEF16BIT_IDX_TYPE {
	unsigned VALUE : 16;
};

union TCC_DEF16BIT_TYPE {
	unsigned short nREG;
	struct TCC_DEF16BIT_IDX_TYPE bREG;
};

struct TCC_DEF32BIT_IDX_TYPE {
	unsigned VALUE : 32;
};

union TCC_DEF32BIT_TYPE {
	unsigned long nREG;
	struct TCC_DEF32BIT_IDX_TYPE bREG;
};

/******************************************************************
 *
 *	TCC805X DataSheet PART 7 DISPLAY BUS
 *
 ******************************************************************/
#define HwVIOC_BASE (unsigned long)(0x12000000U)

/* DISP */
#define DISP_OFFSET (0x3000UL)
#define HwVIOC_DISP0 (HwVIOC_BASE + 0x00000UL)
#define HwVIOC_DISP1 (HwVIOC_BASE + 0x03000UL)
#define HwVIOC_DISP2 (HwVIOC_BASE + 0x06000UL)
#define HwVIOC_DISP3 (HwVIOC_BASE + 0x09000UL)
#define HwVIOC_DISP4 (HwVIOC_BASE + 0x0C000UL)
#define TCC_VIOC_DISP_BASE(x) ((unsigned long)HwVIOC_DISP0 + ((unsigned long)DISP_OFFSET * (x)))

/* RDMA */
#define RDMA_OFFSET (0x1000UL)
#define HwVIOC_RDMA00 (HwVIOC_BASE + 0x20000UL)
#define HwVIOC_RDMA01 (HwVIOC_BASE + 0x21000UL)
#define HwVIOC_RDMA02 (HwVIOC_BASE + 0x22000UL)
#define HwVIOC_RDMA03 (HwVIOC_BASE + 0x23000UL)
#define HwVIOC_RDMA04 (HwVIOC_BASE + 0x24000UL)
#define HwVIOC_RDMA05 (HwVIOC_BASE + 0x25000UL)
#define HwVIOC_RDMA06 (HwVIOC_BASE + 0x26000UL)
#define HwVIOC_RDMA07 (HwVIOC_BASE + 0x27000UL)
#define HwVIOC_RDMA08 (HwVIOC_BASE + 0x28000UL)
#define HwVIOC_RDMA09 (HwVIOC_BASE + 0x29000UL)
#define HwVIOC_RDMA10 (HwVIOC_BASE + 0x2A000UL)
#define HwVIOC_RDMA11 (HwVIOC_BASE + 0x2B000UL)
#define HwVIOC_RDMA12 (HwVIOC_BASE + 0x2C000UL)
#define HwVIOC_RDMA13 (HwVIOC_BASE + 0x2D000UL)
#define HwVIOC_RDMA14 (HwVIOC_BASE + 0x2E000UL)
#define HwVIOC_RDMA15 (HwVIOC_BASE + 0x2F000UL)
#define TCC_VIOC_RDMA_BASE(x) (HwVIOC_RDMA00 + (RDMA_OFFSET * (x)))

/* WMIX */
#define WMIX_OFFSET (0x1000UL)
#define HwVIOC_WMIX0 (HwVIOC_BASE + 0x60000UL)
#define HwVIOC_WMIX1 (HwVIOC_BASE + 0x61000UL)
#define HwVIOC_WMIX2 (HwVIOC_BASE + 0x62000UL)
#define HwVIOC_WMIX3 (HwVIOC_BASE + 0x63000UL)
#define TCC_VIOC_WMIX_BASE(x) (HwVIOC_WMIX0 + (WMIX_OFFSET * (x)))

/* SCALER */
#define SC_OFFSET (0x1000UL)
#define HwVIOC_SC0 (HwVIOC_BASE + 0x50000UL)
#define HwVIOC_SC1 (HwVIOC_BASE + 0x51000UL)
#define HwVIOC_SC2 (HwVIOC_BASE + 0x52000UL)
#define HwVIOC_SC3 (HwVIOC_BASE + 0x53000UL)
#define TCC_VIOC_SC_BASE(x) (HwVIOC_SC0 + (SC_OFFSET * (x)))

/* WDMA */
#define WDMA_OFFSET (0x1000UL)
#define HwVIOC_WDMA00 (HwVIOC_BASE + 0x40000UL)
#define HwVIOC_WDMA01 (HwVIOC_BASE + 0x41000UL)
#define HwVIOC_WDMA02 (HwVIOC_BASE + 0x42000UL)
#define HwVIOC_WDMA03 (HwVIOC_BASE + 0x43000UL)
#define HwVIOC_WDMA04 (HwVIOC_BASE + 0x44000UL)
#define HwVIOC_WDMA05 (HwVIOC_BASE + 0x45000UL)
#define TCC_VIOC_WDMA_BASE(x) (HwVIOC_WDMA00 + (WDMA_OFFSET * (x)))

/* MC */
#define MC_OFFSET (0x1000UL)
#define HwVIOC_MC0 (HwVIOC_BASE + 0x90000UL)
#define TCC_VIOC_MC_BASE(x) (HwVIOC_MC0 + (MC_OFFSET * (x)))

/* LUT */
#define HwVIOCLUT_BASE (HwVIOC_BASE + 0xB0000UL)

/* 3D_LUT0 */
#define LUT_3D_LUT0_OFFSET (0x1000UL)
#define HwVIOC_3D_LUT0_1 (HwVIOC_BASE + 0x01000UL)
#define HwVIOC_3D_LUT0_0 (HwVIOC_BASE + 0x02000UL)
#define TCC_VIOC_3D_LUT0_BASE(x) (HwVIOC_3D_LUT0_1 + (3D_LUT0_OFFSET * (x)))

/* FIFO */
#define HwVIOC_FIFO (HwVIOC_BASE + 0xA0000UL)

/* VIQE */
#define VIQE_OFFSET (0x1000UL)
#define HwVIOC_VIQE00 (HwVIOC_BASE + 0x70000UL)
#define TCC_VIOC_VIQE_BASE(x) (HwVIOC_VIQE00 + VIQE_OFFSET * (x))

/* CONFIG */
#define HwVIOC_CONFIG (HwVIOC_BASE + 0xF0000UL)

/* TIMER */
#define HwVIOC_TIMER (HwVIOC_BASE + 0xC0000UL)

/************************************************************************
 *   DDI_CONFIG (Base Addr = 0x12380000) R/W
 *************************************************************************/
#define HwDDI_CONFIG_BASE (0x12380000U)

/**************************************************************************
 *
 *	TCC805X DataSheet PART 9 Cortex-M4 BUS
 *
 **************************************************************************/

#define HwCORTEXM4_CODE_MEM_BASE (0x19080000)
#define HwCORTEXM4_CODE_MEM_SIZE (0x00010000)
#define HwCORTEXM4_DATA_MEM_BASE (0x19090000)
#define HwCORTEXM4_DATA_MEM_SIZE (0x00010000)
#define HwCORTEXM4_MAILBOX0_BASE (0x19000000)
#define HwCORTEXM4_MAILBOX1_BASE (0x19010000)

#define HwCORTEXM4_TSD_CFG_BASE (0x19100000)

#endif /* _PLATFORM_REG_PHYSICAL_H_ */
