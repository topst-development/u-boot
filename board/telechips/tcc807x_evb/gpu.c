// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <mach/gpu.h>
#include <mach/reboot.h>

void gpu_init(void)
{
#if !defined(CONFIG_MALI_VZ) || \
	(defined(CONFIG_MALI_VZ) && defined(CONFIG_USE_SUBCORE))
	unsigned long reg = GPU_3D_CFG_REG;
	unsigned long val;

        bool wake_from_corerst = core_reset_occurred();
        if(!wake_from_corerst)
        {

                /* bus AXI-B */
                writel(GPU_3DENGINE_DEBUG_SYS_ASSIGN_ENABLE_SB, reg + GPU_3DENGINE_DEBUG);
                val = readl(reg + GPU_3DENGINE_SWRESET);
                writel((val &~ GPU_3DENGINE_SWRESET_3D_MASK), reg + GPU_3DENGINE_SWRESET);
                writel(GPU_3DENGINE_SWRESET_FULL_MASK, reg + GPU_3DENGINE_SWRESET);
        }
#endif
}
