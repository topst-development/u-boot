/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef VIOC_GLOBAL_H
#define VIOC_GLOBAL_H

/* VIOC Overall Register Map.
 * ex)
 *  upper 2bits are component device [3:2]
 *  lower 2bits are component device number [1:0]
 *  0x0000 => RDMA[0]
 *  0x0001 => RDMA[1]
 *  0x0101 => WDMA[0]
 *  ...
 *  WARNING : DO NOT add 0x00XX type. it will be potential problem.
 */

#define ALIGN_BIT (0x8 - 1)
#define BIT_0 3
#define GET_ADDR_YUV42X_spY(Base_addr) \
	(((((unsigned int)Base_addr) + ALIGN_BIT) >> BIT_0) << BIT_0)
#define GET_ADDR_YUV42X_spU(Yaddr, x, y) \
	(((((unsigned int)Yaddr + (x * y)) + ALIGN_BIT) >> BIT_0) << BIT_0)
#define GET_ADDR_YUV422_spV(Uaddr, x, y) \
	(((((unsigned int)Uaddr + (x * y / 2)) + ALIGN_BIT) >> BIT_0) << BIT_0)
#define GET_ADDR_YUV420_spV(Uaddr, x, y) \
	(((((unsigned int)Uaddr + (x * y / 4)) + ALIGN_BIT) >> BIT_0) << BIT_0)

#define get_vioc_type(x)		((x) >> 8U)
#define get_vioc_index(x)		((x) & 0xFFU)

/* DISP : 0x01XX */
#define VIOC_DISP			(0x0100U)
#define VIOC_DISP0			(0x0100U)
#define VIOC_DISP1			(0x0101U)
#define VIOC_DISP2			(0x0102U)
#define VIOC_DISP3			(0x0103U)
#define VIOC_DISP_MAX			(0x0004U)

/* RDMA : 0x02XX */
#define VIOC_RDMA			(0x0200U)
#define VIOC_RDMA00			(0x0200U)
#define VIOC_RDMA01			(0x0201U)
#define VIOC_RDMA02			(0x0202U)
#define VIOC_RDMA03			(0x0203U)
#define VIOC_RDMA04			(0x0204U)
#define VIOC_RDMA05			(0x0205U)
#define VIOC_RDMA06			(0x0206U)
#define VIOC_RDMA07			(0x0207U)
#define VIOC_RDMA08			(0x0208U)
#define VIOC_RDMA09			(0x0209U)
#define VIOC_RDMA10			(0x020AU)
#define VIOC_RDMA11			(0x020BU)
#define VIOC_RDMA12			(0x020CU)
#define VIOC_RDMA13			(0x020DU)
#define VIOC_RDMA14			(0x020EU)
#define VIOC_RDMA15			(0x020FU)
#define VIOC_RDMA16			(0x0210U)
#define VIOC_RDMA17			(0x0211U)
#define VIOC_RDMA_MAX		(0x0012U)

/* MC : 0x03XX */
#define VIOC_MC				(0x0300U)
#define VIOC_MC0			(0x0300U)
#define VIOC_MC1			(0x0301U)
#define VIOC_MC_MAX			(0x0002U)

/* WMIXER : 0x04XX */
#define VIOC_WMIX			(0x0400U)
#define VIOC_WMIX0			(0x0400U)
#define VIOC_WMIX1			(0x0401U)
#define VIOC_WMIX2			(0x0402U)
#define VIOC_WMIX3			(0x0403U)
#define VIOC_WMIX4			(0x0404U)
#define VIOC_WMIX5			(0x0405U)
#define VIOC_WMIX6			(0x0406U)
#define VIOC_WMIX_MAX			(0x0007U)

/* Scaler : 0x05XX */
#define VIOC_SCALER			(0x0500U)
#define VIOC_SCALER0			(0x0500U)
#define VIOC_SCALER1			(0x0501U)
#define VIOC_SCALER2			(0x0502U)
#define VIOC_SCALER3			(0x0503U)
#define VIOC_SCALER4			(0x0504U)
#define VIOC_SCALER5			(0x0505U)
#define VIOC_SCALER6			(0x0506U)
#define VIOC_SCALER7			(0x0507U)
#define VIOC_SCALER_MAX			(0x0008U)

/* DTRC : 0x06XX */
//#define VIOC_DTRC (0x0600)

/* WDMA : 0x07XX */
#define VIOC_WDMA			(0x0700U)
#define VIOC_WDMA00			(0x0700U)
#define VIOC_WDMA01			(0x0701U)
#define VIOC_WDMA02			(0x0702U)
#define VIOC_WDMA03			(0x0703U)
#define VIOC_WDMA04			(0x0704U)
#define VIOC_WDMA05			(0x0705U)
#define VIOC_WDMA06			(0x0706U)
#define VIOC_WDMA07			(0x0707U)
#define VIOC_WDMA08			(0x0708U)
#define VIOC_WDMA09			(0x0709U)
#define VIOC_WDMA10			(0x070AU)
#define VIOC_WDMA11			(0x070BU)
#define VIOC_WDMA12			(0x070CU)
#define VIOC_WDMA13			(0x070DU)
#define VIOC_WDMA_MAX			(0x000EU)

/* DEINTL_S : 0x08XX */
#define VIOC_DEINTLS			(0x0800U)
#define VIOC_DEINTLS0			(0x0800U)
#define VIOC_DEINTLS_MAX		(0x0001U)

/* Async FIFO : 0x09XX */
#define VIOC_FIFO			(0x0900U)
#define VIOC_FIFO0			(0x0900U)
#define VIOC_FIFO_MAX			(0x0001U)

/* VIDEO IN : 0x0AXX */
#define VIOC_VIN			(0x0A00U)
#define VIOC_VIN00			(0x0A00U)
#define VIOC_VIN01			(0x0A01U)
#define VIOC_VIN10			(0x0A02U)
#define VIOC_VIN11			(0x0A03U)
#define VIOC_VIN20			(0x0A04U)
#define VIOC_VIN21			(0x0A05U)
#define VIOC_VIN30			(0x0A06U)
#define VIOC_VIN31			(0x0A07U)
#define VIOC_VIN40			(0x0A08U)
#define VIOC_VIN41			(0x0A09U)
#define VIOC_VIN50			(0x0A0AU)
#define VIOC_VIN51			(0x0A0BU)
#define VIOC_VIN60			(0x0A0CU)
#define VIOC_VIN61			(0x0A0DU)
#define VIOC_VIN70			(0x0A0EU)
#define VIOC_VIN71			(0x0A0FU)
#define VIOC_VIN_MAX			(0x0010U)

/* LUT DEV : 0x0BXX */
#define VIOC_LUT			(0x0B00U)
#define VIOC_LUT_DEV0			(0x0B00U)
#define VIOC_LUT_DEV1			(0x0B01U)
#define VIOC_LUT_DEV2			(0x0B02U)
#define VIOC_LUT_COMP0			(0x0B03U)
#define VIOC_LUT_COMP1			(0x0B04U)
#define VIOC_LUT_COMP2			(0x0B05U)
#define VIOC_LUT_COMP3			(0x0B06U)
#define VIOC_LUT_DEV3 (0x0B07U)
#define VIOC_LUT_MAX (0x0008U)

/* LUT TABLE : 0x0CXX */
#define VIOC_LUT_TABLE			(0x0C00U)
#define VIOC_LUT_TABLE0			(0x0C00U)
#define VIOC_LUT_TABLE1			(0x0C01U)
#define VIOC_LUT_TABLE_MAX		(0x0002U)

/* VIOC CONFIG : 0x0DXX */
#define VIOC_CONFIG			(0x0D00U)

/* VIN DEMUX : 0x0EXX */
#define VIOC_VIN_DEMUX			(0x0E00U)

/* TIMER : 0x0FXX */
#define VIOC_TIMER			(0x0F00U)

/* VIQE : 0x10XX */
#define VIOC_VIQE			(0x1000U)
#define VIOC_VIQE0			(0x1000U)
#define VIOC_VIQE1			(0x1001U)
#define VIOC_VIQE_MAX			(0x0002U)

/* VIDEO MARK : 0x11XX */
//#define VIOC_VIDEOMARK		(0x1100)

/* NEXGUARD : 0x12XX */
//#define VIOC_NEXGUARD			(0x1200)

/* OUTCFG : 0x13XX */
#define VIOC_OUTCFG			(0x1300U)

/* V_EDR : 0x14XX */
//#define VIOC_V_EDR			(0x1400)

/* V_PANEL : 0x15XX */
//#define VIOC_V_PANEL			(0x1500)

/* V_PANEL_LUT : 0x16XX */
//#define VIOC_V_PANEL_LUT		(0x1600)

/* V_DV_CFG : 0x17XX */
//#define VIOC_V_DV_CFG			(0x1700)

/* AFBCDEC : 0x18XX */
#define VIOC_FBCDEC			(0x1800U)
#define VIOC_FBCDEC0			(0x1800U)
#define VIOC_FBCDEC1			(0x1801U)
#define VIOC_FBCDEC_MAX		(0x0002U)

/* VIOC Interrupt sources */
#define VIOC0_IRQI			(119U)
#define VIOC1_IRQI			(120U)
#define VIOC2_IRQI			(121U)
#define VIOC3_IRQI			(122U)
#define VIOC_IRQ_MAX		(0x4U)

/* Display Device : PXDW FORMAT */
#define VIOC_PXDW_FMT_04_STN (0)
#define VIOC_PXDW_FMT_08_STN (1)
#define VIOC_PXDW_FMT_08_RGB_STRIPE (2)
#define VIOC_PXDW_FMT_16_RGB565 (3)
#define VIOC_PXDW_FMT_15_RGB555 (4)
#define VIOC_PXDW_FMT_18_RGB666 (5)
#define VIOC_PXDW_FMT_08_UY (6)
#define VIOC_PXDW_FMT_08_VY (7)
#define VIOC_PXDW_FMT_16_YU (8)
#define VIOC_PXDW_FMT_16_YV (9)
#define VIOC_PXDW_FMT_08_RGB_DELTA0 (10)
#define VIOC_PXDW_FMT_08_RGB_DELTA1 (11)
#define VIOC_PXDW_FMT_24_RGB888 (12)
#define VIOC_PXDW_FMT_08_RGBD (13)
#define VIOC_PXDW_FMT_16_RGB666 (14)
#define VIOC_PXDW_FMT_16_RGB888 (15)
#define VIOC_PXDW_FMT_10_RGB_STRIPE_RGB (16)
#define VIOC_PXDW_FMT_10_RGB_DELTA_RGB_GBR (17)
#define VIOC_PXDW_FMT_10_RGB_DELTA_GBR_RGB (18)
#define VIOC_PXDW_FMT_10_YCBCR_CBYCRY (19)
#define VIOC_PXDW_FMT_10_YCBCR_CRYCBY (20)
#define VIOC_PXDW_FMT_20_YCBCR_YCBCR (21)
#define VIOC_PXDW_FMT_20_YCBCR_YCRCB (22)
#define VIOC_PXDW_FMT_30_RGB101010 (23)
#define VIOC_PXDW_FMT_10_RGB_DUMMY (24)
#define VIOC_PXDW_FMT_20_RGB101010 (25)
#define VIOC_PXDW_FMT_24_YCBCR420 (26)
#define VIOC_PXDW_FMT_30_YCBCR420 (27)

/* RDMA/WDMA : RGB Swap */
#define VIOC_SWAP_RGB (0)
#define VIOC_SWAP_RBG (1)
#define VIOC_SWAP_GRB (2)
#define VIOC_SWAP_GBR (3)
#define VIOC_SWAP_BRG (4)
#define VIOC_SWAP_BGR (5)
#define VIOC_SWAP_MAX (6)

/* RDMA/WDMA : Image Format */
#define VIOC_IMG_FMT_BPP1 (0)	// 1bit
#define VIOC_IMG_FMT_BPP2 (1)	// 2bits
#define VIOC_IMG_FMT_BPP4 (2)	// 4bits
#define VIOC_IMG_FMT_BPP8 (3)	// 1byte
#define VIOC_IMG_FMT_RGB332 (8)      // 1byte
#define VIOC_IMG_FMT_ARGB4444 (9)    // 2bytes
#define VIOC_IMG_FMT_RGB565 (10)     // 2bytes
#define VIOC_IMG_FMT_ARGB1555 (11)   // 2bytes
#define VIOC_IMG_FMT_ARGB8888 (12)   // 4bytes
#define VIOC_IMG_FMT_ARGB6666_4 (13) // 4bytes
#define VIOC_IMG_FMT_RGB888 (14)
	// 3bytes	: newly supported : 3 bytes format
#define VIOC_IMG_FMT_ARGB6666_3 (15)
	// 3bytes	: newly supported : 3 bytes format
#define VIOC_IMG_FMT_COMP (16) // 4bytes
#define VIOC_IMG_FMT_DECOMP (VIOC_IMG_FMT_COMP)
#define VIOC_IMG_FMT_444SEP (21) // 3bytes
#define VIOC_IMG_FMT_UYVY (22)
	// 2bytes	: LSB [Y/U/Y/V] MSB : newly supported : 2 bytes format
#define VIOC_IMG_FMT_VYUY (23)
	// 2bytes	: LSB [Y/V/Y/U] MSB : newly supported : 2 bytes format
#define VIOC_IMG_FMT_YUV420SEP (24) // 1,1byte
#define VIOC_IMG_FMT_YUV422SEP (25) // 1,1byte
#define VIOC_IMG_FMT_YUYV (26)
	// 2bytes	: LSB [Y/U/Y/V] MSB : previous SEQ
#define VIOC_IMG_FMT_YVYU (27)
	// 2bytes	: LSB [Y/V/Y/U] MSB : newly supported : 2 bytes format
#define VIOC_IMG_FMT_YUV420IL0 (28) // 1,2byte
#define VIOC_IMG_FMT_YUV420IL1 (29) // 1,2byte
#define VIOC_IMG_FMT_YUV422IL0 (30) // 1,2bytes
#define VIOC_IMG_FMT_YUV422IL1 (31) // 1,2bytes

/* Configuration & Interrupt */
#define VIOC_EDR_WMIX0 (0)
#define VIOC_EDR_EDR (1)

#define VIOC_SC_RDMA_00 (0x00)
#define VIOC_SC_RDMA_01 (0x01)
#define VIOC_SC_RDMA_02 (0x02)
#define VIOC_SC_RDMA_03 (0x03)
#define VIOC_SC_RDMA_04 (0x04)
#define VIOC_SC_RDMA_05 (0x05)
#define VIOC_SC_RDMA_06 (0x06)
#define VIOC_SC_RDMA_07 (0x07)
#define VIOC_SC_RDMA_08 (0x08)
#define VIOC_SC_RDMA_09 (0x09)
#define VIOC_SC_RDMA_10 (0x0A)
#define VIOC_SC_RDMA_11 (0x0B)
#define VIOC_SC_RDMA_12 (0x0C)
#define VIOC_SC_RDMA_13 (0x0D)
#define VIOC_SC_RDMA_14 (0x0E)
#define VIOC_SC_RDMA_15 (0x0F)
#define VIOC_SC_VIN_00 (0x10)
#define VIOC_SC_RDMA_16 (0x11)
#define VIOC_SC_VIN_01 (0x12)
#define VIOC_SC_RDMA_17 (0x13)
#define VIOC_SC_WDMA_00 (0x14)
#define VIOC_SC_WDMA_01 (0x15)
#define VIOC_SC_WDMA_02 (0x16)
#define VIOC_SC_WDMA_03 (0x17)
#define VIOC_SC_WDMA_04 (0x18)
#define VIOC_SC_WDMA_05 (0x19)
#define VIOC_SC_WDMA_06 (0x1A)
#define VIOC_SC_WDMA_07 (0x1B)
#define VIOC_SC_WDMA_08 (0x1C)
#define VIOC_SC_WDMA_13 (0x20)

#define VIOC_VIQE_RDMA_00 (0x00)
#define VIOC_VIQE_RDMA_01 (0x01)
#define VIOC_VIQE_RDMA_02 (0x02)
#define VIOC_VIQE_RDMA_03 (0x03)
#define VIOC_VIQE_RDMA_06 (0x04)
#define VIOC_VIQE_RDMA_07 (0x05)
#define VIOC_VIQE_RDMA_10 (0x06)
#define VIOC_VIQE_RDMA_11 (0x07)
#define VIOC_VIQE_RDMA_12 (0x08)
#define VIOC_VIQE_RDMA_14 (0x09)
#define VIOC_VIQE_VIN_00 (0x0A)
#define VIOC_VIQE_RDMA_16 (0x0B)
#define VIOC_VIQE_VIN_01 (0x0C)
#define VIOC_VIQE_RDMA_17 (0x0D)

#define VIOC_DEINTLS_RDMA_00 (0x00)
#define VIOC_DEINTLS_RDMA_01 (0x01)
#define VIOC_DEINTLS_RDMA_02 (0x02)
#define VIOC_DEINTLS_RDMA_03 (0x03)
#define VIOC_DEINTLS_RDMA_06 (0x04)
#define VIOC_DEINTLS_RDMA_07 (0x05)
#define VIOC_DEINTLS_RDMA_10 (0x06)
#define VIOC_DEINTLS_RDMA_11 (0x07)
#define VIOC_DEINTLS_RDMA_12 (0x08)
#define VIOC_DEINTLS_RDMA_14 (0x09)
#define VIOC_DEINTLS_VIN_00 (0x0A)
#define VIOC_DEINTLS_RDMA_16 (0x0B)
#define VIOC_DEINTLS_VIN_01 (0x0C)
#define VIOC_DEINTLS_RDMA_17 (0x0D)

#define VIOC_VM_RDMA_00 (0x00)
#define VIOC_VM_RDMA_01 (0x01)
#define VIOC_VM_RDMA_02 (0x02)
#define VIOC_VM_RDMA_03 (0x03)
#define VIOC_VM_RDMA_04 (0x04)
#define VIOC_VM_RDMA_05 (0x05)
#define VIOC_VM_RDMA_06 (0x06)
#define VIOC_VM_RDMA_07 (0x07)
#define VIOC_VM_RDMA_08 (0x08)
#define VIOC_VM_RDMA_09 (0x09)
#define VIOC_VM_RDMA_10 (0x0A)
#define VIOC_VM_RDMA_11 (0x0B)
#define VIOC_VM_RDMA_12 (0x0C)
#define VIOC_VM_RDMA_13 (0x0D)
#define VIOC_VM_RDMA_14 (0x0E)
#define VIOC_VM_RDMA_15 (0x0F)
#define VIOC_VM_VIN_00 (0x10)
#define VIOC_VM_RDMA_16 (0x11)
#define VIOC_VM_VIN_01 (0x12)
#define VIOC_VM_RDMA_17 (0x13)
#define VIOC_VM_WDMA_00 (0x14)
#define VIOC_VM_WDMA_01 (0x15)
#define VIOC_VM_WDMA_02 (0x16)
#define VIOC_VM_WDMA_03 (0x17)
#define VIOC_VM_WDMA_04 (0x18)
#define VIOC_VM_WDMA_05 (0x19)
#define VIOC_VM_WDMA_06 (0x1A)
#define VIOC_VM_WDMA_07 (0x1B)
#define VIOC_VM_WDMA_08 (0x1C)

#define VIOC_NG_RDMA_00 (0x00)
#define VIOC_NG_RDMA_01 (0x01)
#define VIOC_NG_RDMA_02 (0x02)
#define VIOC_NG_RDMA_03 (0x03)
#define VIOC_NG_RDMA_04 (0x04)
#define VIOC_NG_RDMA_05 (0x05)
#define VIOC_NG_RDMA_06 (0x06)
#define VIOC_NG_RDMA_07 (0x07)
#define VIOC_NG_RDMA_08 (0x08)
#define VIOC_NG_RDMA_09 (0x09)
#define VIOC_NG_RDMA_10 (0x0A)
#define VIOC_NG_RDMA_11 (0x0B)
#define VIOC_NG_RDMA_12 (0x0C)
#define VIOC_NG_RDMA_13 (0x0D)
#define VIOC_NG_RDMA_14 (0x0E)
#define VIOC_NG_RDMA_15 (0x0F)
#define VIOC_NG_VIN_00 (0x10)
#define VIOC_NG_RDMA_16 (0x11)
#define VIOC_NG_VIN_01 (0x12)
#define VIOC_NG_RDMA_17 (0x13)
#define VIOC_NG_WDMA_00 (0x14)
#define VIOC_NG_WDMA_01 (0x15)
#define VIOC_NG_WDMA_02 (0x16)
#define VIOC_NG_WDMA_03 (0x17)
#define VIOC_NG_WDMA_04 (0x18)
#define VIOC_NG_WDMA_05 (0x19)
#define VIOC_NG_WDMA_06 (0x1A)
#define VIOC_NG_WDMA_07 (0x1B)
#define VIOC_NG_WDMA_08 (0x1C)

#define VIOC_RDMA_RDMA02 (0x02)
#define VIOC_RDMA_RDMA03 (0x03)
#define VIOC_RDMA_RDMA06 (0x06)
#define VIOC_RDMA_RDMA07 (0x07)
#define VIOC_RDMA_RDMA11 (0x0B)
#define VIOC_RDMA_RDMA12 (0x0C)
#define VIOC_RDMA_RDMA13 (0x0D)
#define VIOC_RDMA_RDMA14 (0x0E)
#define VIOC_RDMA_RDMA16 (0x10)
#define VIOC_RDMA_RDMA17 (0x11)

#define VIOC_MC_RDMA02 (0x02)
#define VIOC_MC_RDMA03 (0x03)
#define VIOC_MC_RDMA06 (0x06)
#define VIOC_MC_RDMA07 (0x07)
#define VIOC_MC_RDMA11 (0x0B)
#define VIOC_MC_RDMA12 (0x0C)
#define VIOC_MC_RDMA13 (0x0D)
#define VIOC_MC_RDMA14 (0x0E)
#define VIOC_MC_RDMA16 (0x10)
#define VIOC_MC_RDMA17 (0x11)

#define VIOC_DTRC_RDMA02 (0x02)
#define VIOC_DTRC_RDMA03 (0x03)
#define VIOC_DTRC_RDMA06 (0x06)
#define VIOC_DTRC_RDMA07 (0x07)
#define VIOC_DTRC_RDMA11 (0x0B)
#define VIOC_DTRC_RDMA12 (0x0C)
#define VIOC_DTRC_RDMA13 (0x0D)
#define VIOC_DTRC_RDMA14 (0x0E)
#define VIOC_DTRC_RDMA16 (0x10)
#define VIOC_DTRC_RDMA17 (0x11)

#define VIOC_AD_RDMA_00 (0x00)
#define VIOC_AD_RDMA_01 (0x01)
#define VIOC_AD_RDMA_02 (0x02)
#define VIOC_AD_RDMA_03 (0x03)
#define VIOC_AD_RDMA_04 (0x04)
#define VIOC_AD_RDMA_05 (0x05)
#define VIOC_AD_RDMA_06 (0x06)
#define VIOC_AD_RDMA_07 (0x07)
#define VIOC_AD_RDMA_08 (0x08)
#define VIOC_AD_RDMA_09 (0x09)
#define VIOC_AD_RDMA_10 (0x0A)
#define VIOC_AD_RDMA_11 (0x0B)
#define VIOC_AD_RDMA_12 (0x0C)
#define VIOC_AD_RDMA_13 (0x0D)
#define VIOC_AD_RDMA_14 (0x0E)
#define VIOC_AD_RDMA_15 (0x0F)
#define VIOC_AD_RDMA_16 (0x11)
#define VIOC_AD_RDMA_17 (0x13)

////////////////////////////////////////////////////////////////////////////////
//
//	CPUIF FORMAT
//
#define CPUIF_OFMT_08_TYPE0 0 // LCDC : 8-8-8, SI(08bits) 8-8-8
#define CPUIF_OFMT_24_TYPE0 4 // LCDC : 8-8-8, SI(16bits) 9-9
#define CPUIF_OFMT_24_TYPE1 5
#define CPUIF_OFMT_24_TYPE2 6
#define CPUIF_OFMT_24_TYPE3 7
#define CPUIF_OFMT_18_TYPE0 8
#define CPUIF_OFMT_18_TYPE1 9
#define CPUIF_OFMT_18_TYPE2 10
#define CPUIF_OFMT_18_TYPE3 11
#define CPUIF_OFMT_16_TYPE0 12
#define CPUIF_OFMT_16_TYPE1 13
#define CPUIF_OFMT_16_TYPE2 14
//
//	CPUIF FORMAT
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//	WDMA SYNC SELECTION
//
#define VIOC_WDMA_SYNC_ABSOLUTE (0U << 0U)    // SYNCMD_ADDR
#define VIOC_WDMA_SYNC_RELATIVE (1U << 0U)    // SYNCMD_ADDR
#define VIOC_WDMA_SYNC_START_EDGE (0U << 0U)  // SYNCMD_SENS
#define VIOC_WDMA_SYNC_START_LEVEL (1U << 0U) // SYNCMD_SENS

#define VIOC_WDMA_SYNC_RDMA00 (0)
#define VIOC_WDMA_SYNC_RDMA01 (1)
#define VIOC_WDMA_SYNC_RDMA02 (2)
#define VIOC_WDMA_SYNC_RDMA03 (3)
#define VIOC_WDMA_SYNC_RDMA04 (4)
#define VIOC_WDMA_SYNC_RDMA05 (5)
#define VIOC_WDMA_SYNC_RDMA06 (6)
#define VIOC_WDMA_SYNC_RDMA07 (7)
#define VIOC_WDMA_SYNC_RDMA08 (8)
#define VIOC_WDMA_SYNC_RDMA09 (9)
#define VIOC_WDMA_SYNC_RDMA10 (10)
#define VIOC_WDMA_SYNC_RDMA11 (11)
#define VIOC_WDMA_SYNC_RDMA12 (12)
#define VIOC_WDMA_SYNC_RDMA13 (13)
#define VIOC_WDMA_SYNC_RDMA14 (14)
#define VIOC_WDMA_SYNC_RDMA15 (15)
#define VIOC_WDMA_SYNC_RDMA16 (16)
#define VIOC_WDMA_SYNC_RDMA17 (17)
//
//	WDMA SYNC SELECTION
//
////////////////////////////////////////////////////////////////////////////////

/* VIOC DRIVER STATUS TYPE */
#define VIOC_DEVICE_INVALID (-2)
#define VIOC_DEVICE_BUSY (-1)
#define VIOC_DEVICE_CONNECTED (0)

/* VIOC DRIVER ERROR TYPE */
#define VIOC_DRIVER_ERR_INVALID (-3)
#define VIOC_DRIVER_ERR_BUSY (-2)
#define VIOC_DRIVER_ERR (-1)
#define VIOC_DRIVER_NOERR (0)

/* VIOC PATH STATUS TYPE */
#define VIOC_PATH_DISCONNECTED		(0U)
#define VIOC_PATH_CONNECTING		(1U)
#define VIOC_PATH_CONNECTED			(2U)
#define VIOC_PATH_DISCONNECTING		(3U)

#define FBX_MODE(x)			(x)
#define FBX_SINGLE			FBX_MODE(0x0U)
#define FBX_DOUBLE			FBX_MODE(0x1U)
#define FBX_TRIPLE			FBX_MODE(0x2U)

#define FBX_UPDATE(x) (x)
#define FBX_RDMA_UPDATE FBX_UPDATE(0x0U)
#define FBX_M2M_RDMA_UPDATE FBX_UPDATE(0x1U)
#define FBX_ATTACH_UPDATE FBX_UPDATE(0x2U)
#define FBX_OVERLAY_UPDATE FBX_UPDATE(0x3U)
#define FBX_NOWAIT_UPDATE FBX_UPDATE(0x4U)

#define FBX_DEVICE(x)		(x)
#define FBX_DEVICE_NONE			FBX_DEVICE(0x0U)
#define FBX_DEVICE_LVDS			FBX_DEVICE(0x1U)
#define FBX_DEVICE_HDMI			FBX_DEVICE(0x2U)
#define FBX_DEVICE_COMPOSITE		FBX_DEVICE(0x3U)
#define FBX_DEVICE_COMPONENT		FBX_DEVICE(0x4U)
#endif