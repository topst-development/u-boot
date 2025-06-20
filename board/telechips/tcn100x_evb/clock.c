// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <mach/board.h>
#include <mach/clock.h>

void clock_init_early(void)
{
        tcc_clk_init();

	/* All clocks set in MCU BL1 and TF-A BL1 */
}

void clock_init(void)
{

}
