/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef VIOC_BLOCK_H
#define VIOC_BLOCK_H

// RDMA and VRDMA
#define TVC_RDMA(x) (x)
#define TVC_RDMA_N (0x20)

// WDMA
#define TVC_WDMA(x) (TVC_RDMA(TVC_RDMA_N) + x)
#define TVC_WDMA_N (0x20)

// WMIXER
#define TVC_WMIX(x) (TVC_WDMA(TVC_WDMA_N) + x)
#define TVC_WMIX_N (0x10)

// SCALER
#define TVC_SCALER(x) (TVC_WMIX(TVC_WMIX_N) + x)
#define TVC_SCALER_N (0x10)

// LUT UP TABLE
#define TVC_LUT(x) (TVC_SCALER(TVC_SCALER_N) + x)
#define TVC_LUT_N (0x10)

// VIN
#define TVC_VIN(x) (TVC_LUT(TVC_LUT_N) + x)
#define TVC_VIN_N (0x10)

// VIQE
#define TVC_VIQE(x) (TVC_VIN(TVC_VIN_N) + x)
#define TVC_VIQE_N (0x8)

// DEINTLS
#define TVC_DEINTLS(x) (TVC_VIQE(TVC_VIQE_N) + x)
#define TVC_DEINTLS_N (0x8)

// MAP CONVERTER
#define TVC_MC(x) (TVC_DEINTLS(TVC_DEINTLS_N) + x)
#define TVC_MC_N (0x10)

// DEC
#define TVC_DEC(x) (TVC_MC(TVC_MC_N) + x)
#define TVC_DEC_N (0x10)

// DTRC
#define TVC_DTRC(x) (TVC_DEC(TVC_DEC_N) + x)
#define TVC_DTRC_N (0x10)

#define TVC_DISP_N (0x3)

#define TVC_ERROR_NUM (0xFFFF)
#endif /* __VIOC_BLOCK_H__ */
