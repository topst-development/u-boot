// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <fdt_support.h>
#include <mach/fdt.h>

static struct reserved_memory_config configurable[] = {
#if defined(CONFIG_BOOT_SUBCORE_BY_MAINCORE)
	/* memory for subcore kernel if using dual-boot system */
	{ "/reserved-memory/a7s_avm", true },
#endif
#if defined(CONFIG_BOOT_SUBCORE_BY_MAINCORE) && defined(CONFIG_TCC_CORE_RESET)
	/* memory for subcore backup image if using core reset feature */
	{ "/reserved-memory/a7s_avm_backup", true },
#endif
#if defined(CONFIG_TCC_CORE_RESET)
	/* memory for u-boot image if using core reset feature */
	{ "/reserved-memory/u_boot", true },
#endif
	/* sentinel */
	{ NULL, false }
};

const struct reserved_memory_config *get_reserved_memory_config(void)
{
	return configurable;
}