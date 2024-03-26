// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <mach/fdt.h>

static struct reserved_memory_config configurable[] = {
#if defined(CONFIG_BOOT_SUBCORE_BY_MAINCORE)
	/* memory for subcore kernel if using dual-boot system */
	{ "/reserved-memory/a7s_avm", true },
	{ "/reserved-memory/a7s_avm_backup", true },
#endif
	/* sentinel */
	{ NULL, false }
};

const struct reserved_memory_config *get_reserved_memory_config(void)
{
	return configurable;
}
