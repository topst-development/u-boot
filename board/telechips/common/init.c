// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <mach/board.h>
#include <mach/reboot.h>

int board_early_init_f(void)
{
	int ret;

	reboot_init();
	clock_init_early();

	ret = board_device_init_early();

	return ret;
}

int board_init(void)
{
	int ret;

	clock_init();

	ret = board_device_init();

	return ret;
}

int last_stage_init(void)
{
	int ret;

	reboot_set_env();
	ret = board_device_init_late();

	return ret;
}
