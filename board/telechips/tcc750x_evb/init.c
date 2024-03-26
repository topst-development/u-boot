// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <linux/io.h>
#include <mach/board.h>
#include <mach/chipinfo.h>

int board_device_init_early(void)
{
	uint32_t chip_rev = get_chip_rev();

	if (chip_rev == 0) {
		/*
		 * This is workaround for ES dma.
		 * Please refer to TCS:CD750XL-143.
		 */
		(void)readl(0x18350000);
	}

	return 0;
}

int board_device_init(void)
{
	pmic_configure();

	return 0;
}

static void pic_init_polarity(void)
{
	void *reg;

	/* set default for active-low irq signals */
	reg = phys_to_virt(0x10000044);
	writel(0x0E006000, reg);
	reg = phys_to_virt(0x10000048);
	writel(0x02000000, reg);
}

int board_device_init_late(void)
{
	pic_init_polarity();

	return 0;
}
