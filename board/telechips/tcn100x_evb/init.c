// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <mach/board.h>

int board_device_init_early(void)
{
	return 0;
}

int board_device_init(void)
{
	pmic_configure();

	return 0;
}

static void pic_init_polarity(void)
{
}

int board_device_init_late(void)
{
	pic_init_polarity();

	return 0;
}
