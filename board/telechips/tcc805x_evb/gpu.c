// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <linux/io.h>
#include <mach/gpu.h>
#include <mach/reboot.h>

static void wait_for_gpu_ready(void)
{
	unsigned long reg = RGX_VIRTUALIZATION_REG_BASE;
	unsigned int count = 0;

	if(GPU_CORE_ID == CORE_MAIN)
	{
		reg += RGX_VIRTUALISATION_REG_SIZE_PER_OS;
	}

	if(readl(reg + RGX_CR_OS0_SCRATCH3) > RGXFW_CONNECTION_FW_READY)
	{
		while(readl(reg + RGX_CR_OS0_SCRATCH3) > RGXFW_CONNECTION_FW_READY)
		{
			mdelay(1);
			count++;
			if(count == GPU_VZ_WATCHDOG_TIMEOUT)
			{
				break;
			}
		}
		pr_err("%s: time spent waiting for GPU FW to reset FW connection %umsec",
				__func__, count);
	}
}

void gpu_init(void)
{
	bool wake_from_corerst = core_reset_occurred();
	if(wake_from_corerst)
	{
#ifdef CONFIG_POWERVR_VZ
		wait_for_gpu_ready();
#endif
	}
}
