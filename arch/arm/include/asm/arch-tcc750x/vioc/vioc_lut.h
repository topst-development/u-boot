/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef VIOC_LUT_H
#define	VIOC_LUT_H

//#define VIOC_LUT_DEV0  (0)
//#define VIOC_LUT_DEV1  (1)
//#define VIOC_LUT_DEV2  (2)
//#define VIOC_LUT_COMP0 (3)
//#define VIOC_LUT_COMP1 (4)
//#define VIOC_LUT_COMP2 (5)
//#define VIOC_LUT_COMP3 (6)
//
//#define VIOC_LUT_RDMA_00 (0)
//#define VIOC_LUT_RDMA_01 (1)
//#define VIOC_LUT_RDMA_02 (2)
//#define VIOC_LUT_RDMA_03 (3)
//#define VIOC_LUT_RDMA_04 (4)
//#define VIOC_LUT_RDMA_05 (5)
//#define VIOC_LUT_RDMA_06 (6)
//#define VIOC_LUT_RDMA_07 (7)
//#define VIOC_LUT_RDMA_08 (8)
//#define VIOC_LUT_RDMA_09 (9)
//#define VIOC_LUT_RDMA_10 (10)
//#define VIOC_LUT_RDMA_11 (11)
//#define VIOC_LUT_RDMA_12 (12)
//#define VIOC_LUT_RDMA_13 (13)
//#define VIOC_LUT_RDMA_14 (14)
//#define VIOC_LUT_RDMA_15 (15)
//#define VIOC_LUT_VIN_00  (16)
//#define VIOC_LUT_RDMA_16 (17)
//#define VIOC_LUT_VIN_01  (18)
//#define VIOC_LUT_RDMA_17 (19)
//#define VIOC_LUT_WDMA_00 (20)
//#define VIOC_LUT_WDMA_01 (21)
//#define VIOC_LUT_WDMA_02 (22)
//#define VIOC_LUT_WDMA_03 (23)
//#define VIOC_LUT_WDMA_04 (24)
//#define VIOC_LUT_WDMA_05 (25)
//#define VIOC_LUT_WDMA_06 (26)
//#define VIOC_LUT_WDMA_07 (27)
//#define VIOC_LUT_WDMA_08 (28)


#define VIOC_LUT_DEV0_OFFSET (0x04U)
//#define VIOC_LUT_DEV1_OFFSET (0x08U)
//#define VIOC_LUT_DEV2_OFFSET (0x0CU)
//#define VIOC_LUT_DEV3_OFFSET (0x20U)

#define VIOC_LUT_COMP0_OFFSET (0x10U)
//#define VIOC_LUT_COMP1_OFFSET (0x14U)
//#define VIOC_LUT_COMP2_OFFSET (0x18U)
//#define VIOC_LUT_COMP3_OFFSET (0x1CU)

#define LUT_TABLE_OFFSET (1U)
#define LUT_COLOR_DEPTH (8U)
#define LUT_TABLE_SIZE ((u32)1U << LUT_COLOR_DEPTH)

extern void tcc_set_lut_table(unsigned int lut_n, const unsigned int *table);
extern int tcc_set_lut_plugin(unsigned int lut_n, unsigned int plugComp);
extern int tcc_get_lut_plugin(unsigned int lut_n);
extern void tcc_set_lut_enable(unsigned int lut_n, unsigned int enable);
extern int tcc_get_lut_enable(unsigned int lut_n);

#endif
