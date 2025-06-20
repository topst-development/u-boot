/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef VIOC_CPU_IF_H
#define VIOC_CPU_IF_H

/************************************************************************
 *	CPU Interface			(Base Addr = 0x12100000)
 ************************************************************************/
struct VIOC_CPUIF_CTRL {
	unsigned RD_HLD : 3;
	unsigned RD_PW : 9;
	unsigned RD_STP : 3;
	unsigned RD_B16 : 1;
	unsigned WR_HLD : 3;
	unsigned WR_PW : 9;
	unsigned WR_STP : 3;
	unsigned WR_B16 : 1;
};

union VIOC_CPUIF_CTRL_u {
	unsigned long nREG;
	struct VIOC_CPUIF_CTRL bREG;
};

struct VIOC_CPUIF_BSWAP {
	unsigned XA00_RDBS : 8;
	unsigned XA00_WRBS : 8;
	unsigned XA01_RDBS : 8;
	unsigned XA01_WRBS : 8;
	unsigned XA10_RDBS : 8;
	unsigned XA10_WRBS : 8;
	unsigned XA11_RDBS : 8;
	unsigned XA11_WRBS : 8;
};

union VIOC_CPUIF_BSWAP_u {
	unsigned long nREG[2];
	struct VIOC_CPUIF_BSWAP bREG;
};

struct VIOC_CPUIF_TYPE {
	unsigned int MODE68 : 1;
	unsigned int reserved : 31;
};

union VIOC_CPUIF_TYPE_u {
	unsigned long nREG;
	struct VIOC_CPUIF_TYPE bREG;
};

union VIOC_CPUIF_AREA_u {
	unsigned char b08[16];
	unsigned short b16[8];
	unsigned int b32[4];
};

struct VIOC_CPUIF_CHANNEL {
	union VIOC_CPUIF_CTRL_u uCS0_CMD0_CTRL;
	// 0x00	0xA0229011	CS0 CMD0 (XA[1]==0) Control Register
	union VIOC_CPUIF_CTRL_u uCS0_DAT0_CTRL;
	// 0x04	0xA0429021	CS0 DAT0 (XA[1]==0) Control Register
	union VIOC_CPUIF_CTRL_u uCS0_CMD1_CTRL;
	// 0x08	0xA0229011	CS0 CMD0 (XA[1]==0) Control Register
	union VIOC_CPUIF_CTRL_u uCS0_DAT1_CTRL;
	// 0x0C	0xA0429021	CS0 DAT0 (XA[1]==0) Control Register
	union VIOC_CPUIF_CTRL_u uCS1_CMD0_CTRL;
	// 0x10	0xA0129009	CS1 CMD0 (XA[1]==0) Control Register
	union VIOC_CPUIF_CTRL_u uCS1_DAT0_CTRL;
	// 0x14	0xA0229011	CS1 DAT0 (XA[1]==0) Control Register
	union VIOC_CPUIF_CTRL_u uCS1_CMD1_CTRL;
	// 0x18	0xA0129009	CS1 CMD1 (XA[1]==0) Control Register
	union VIOC_CPUIF_CTRL_u uCS1_DAT1_CTRL;
	// 0x1C	0xA0229011	CS1 DAT1 (XA[1]==0) Control Register
	union VIOC_CPUIF_BSWAP_u uCS0_BSWAP;
	// 0x20~0x24	 0xE4E4E4E4	CS0 Byte Swap 0 register
	union VIOC_CPUIF_BSWAP_u uCS1_BSWAP;
	// 0x28~0x2C 0xE4E4E4E4	CS0 Byte Swap 0 register
	unsigned int reserved0[2];
	// 0x30~0x34
	union VIOC_CPUIF_TYPE_u uTYPE;
	// 0x38	0x00000000	CPUIF Type Register
	unsigned int reserved1;
	// 0x3C
	union VIOC_CPUIF_AREA_u uCS0_CMD0;
	// 0x40~0x4C
	union VIOC_CPUIF_AREA_u uCS0_CMD1;
	// 0x50~0x5C
	union VIOC_CPUIF_AREA_u uCS0_DAT0;
	// 0x60~0x6C
	union VIOC_CPUIF_AREA_u uCS0_DAT1;
	// 0x70~0x7C
	union VIOC_CPUIF_AREA_u uCS1_CMD0;
	// 0x80~0x8C
	union VIOC_CPUIF_AREA_u uCS1_CMD1;
	// 0x90~0x9C
	union VIOC_CPUIF_AREA_u uCS1_DAT0;
	// 0xA0~0xAC
	union VIOC_CPUIF_AREA_u uCS1_DAT1;
	// 0xB0~0xBC
	unsigned int reserved2[16];
};
#endif
