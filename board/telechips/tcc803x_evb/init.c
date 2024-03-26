// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <mach/board.h>
#include <mach/reboot.h>

static void init_subcore_serial(void)
{
#if defined(CONFIG_INIT_SUBCORE_SERIAL_BY_MAINCORE)
	bool wake_from_corerst = core_reset_occurred();

	if (!wake_from_corerst) {
		uclass_probe(UCLASS_SP_SERIAL);
	}
#endif
}

int board_device_init_early(void)
{
	init_subcore_serial();

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
	reg = phys_to_virt(0x141000F4);
	writel(0x000C03E0, reg);
}

int board_device_init_late(void)
{
	pic_init_polarity();

	return 0;
}
