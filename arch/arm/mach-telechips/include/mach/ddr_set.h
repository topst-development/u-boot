// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __DRAM_TEST_H__
#define __DRAM_TEST_H__

#include <common.h>
#include <asm/io.h>

#define ddr_writel(a, v)	mmio_write_32((uintptr_t)a, (uint32_t)v)
#define ddr_readl(a)		mmio_read_32((uintptr_t)a)

#define ZQ_CON6 0x3E0
#define ZQ_DS1_TERM 11
#define ZQ_DS0_TERM 3

#define ZQ_CON3 0x3D4
#define ZQ_DS1_DDS 11
#define ZQ_DS1_PDDS 8
#define ZQ_DS0_DDS 3
#define ZQ_DS0_PDDS 0

static inline void mmio_write_32(uintptr_t addr, uint32_t value)
{
	*(volatile uint32_t*)addr = value;
}

static inline uint32_t mmio_read_32(uintptr_t addr)
{
	return *(volatile uint32_t*)addr;
}

#ifdef CONFIG_CMD_TCC_MEM
static void mr_write(unsigned char address, unsigned char data)
{
	uintptr_t contrl_reg = 0x13300000;

	ddr_writel(contrl_reg + 0x4, (3 << 28) | (address << 16) | data);
	ddr_writel(contrl_reg + 0xc, 0x2);

}

static unsigned char mr_read(unsigned char address, int ch, int rank)
{
	uintptr_t contrl_reg = 0x13300000;
	unsigned int temp = 0;
	unsigned char lane[2], ret = 0;

	contrl_reg += ch * 0x100000;
	ddr_writel(contrl_reg + 0x4, ((rank + 1) << 28) | (address << 16) | 0);
	ddr_writel(contrl_reg + 0xc, 0x1);
	temp = ddr_readl(contrl_reg + 0x8);
	lane[0] = temp & 0xFF;
	lane[1] = (temp & 0xFF00) >> 8;

	if (lane[0] == lane[1]) { // check byte swap
		if (lane[0] == 0) {
			ret = 0;
			/**/
		} else {
			ret = lane[0];
			/**/
		}
	} else {
		if (lane[0] == 0) {
			ret = lane[1];
			/**/
		} else if (lane[1] == 0) {
			ret = lane[0];
			/**/
		} else {
			/**/
			/**/
		}
	}

	return ret;
}

static int soc_vref[64] = {326, 322, 317, 313, 309, 305, 300, 296,
	292, 287, 283, 279, 274, 270, 266, 261,
	257, 253, 249, 244, 240, 236, 231, 227,
	223, 218, 214, 210, 206, 201, 197, 193,
	189, 184, 180, 176, 172, 167, 163, 159,
	155, 150, 146, 142, 138, 134, 129, 125,
	339, 343, 348, 352, 356, 361, 365, 369,
	374, 378, 382, 387, 391, 399, 412, 425};

#endif
#endif
