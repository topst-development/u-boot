// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <mach/board.h>
#include <mach/gpu.h>


int board_device_init_early(void)
{
	return 0;
}

int board_device_init(void)
{
	pmic_configure();
	gpu_init();

	return 0;
}

static void pic_init_polarity(void)
{
	void *reg;

	/* set default for active-low irq signals */
	reg = phys_to_virt(0x141000F4);
	writel(0x080C03E0, reg);
	reg = phys_to_virt(0x14600100);
	writel(0x00000007, reg);
	reg = phys_to_virt(0x14600114);
	writel(0x00200000, reg);
	reg = phys_to_virt(0x14600120);
	writel(0x01FF8000, reg);
}

int board_device_init_late(void)
{
	pic_init_polarity();

	return 0;
}
