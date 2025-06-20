// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <mach/chipinfo.h>
#include <mach/fdt.h>

static struct reserved_memory_config configurable[] = {
#if defined(CONFIG_USE_MAINCORE)
	/* memory for sub-core kernel if using dual-boot system */
	{ "sub_cluster", true, false },
#endif
	/* sentinel */
	{ NULL, false, false }
};

const struct reserved_memory_config *get_reserved_memory_config(void)
{
	struct boot_info binfo;

	get_boot_info(&binfo);

	configurable[0].enable = is_dual_boot(binfo.bootsel) &&
				 is_main_core(binfo.coreid);

	return configurable;
}
