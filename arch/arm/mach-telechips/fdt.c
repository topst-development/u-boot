// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <fdt_support.h>
#include <mach/fdt.h>

static int ft_rsvm_setup(void *blob, const struct reserved_memory_config *cfg)
{
	s32 ret = 0;

	for ( ; cfg->path != NULL; cfg++) {
		const char *path = cfg->path;
		s32 node;

		if (!cfg->enable) {
			/* Skip if path is empty string */
			continue;
		}

		node = fdt_path_offset(blob, path);
		ret = node;

		if (node >= 0) {
			ret = fdt_delprop(blob, node, "status");

			if (ret == -FDT_ERR_NOTFOUND) {
				/*
				 * It is okay if there is no 'status' property.
				 * We attempted to remove it anyway.
				 */
				ret = 0;
			}
		}

		if (ret != 0) {
			pr_err("Failed to enable '%s' node.\n", path);
			break;
		}
	}

	return ret;
}

int ft_system_setup(void *blob, bd_t *bd)
{
	s32 ret = fdt_check_header(blob);

	if (ret != 0) {
		pr_err("Failed to check FDT header.\n");
	} else {
		const struct reserved_memory_config *rsvm_cfg;

		rsvm_cfg = get_reserved_memory_config();
		if (rsvm_cfg != NULL) {
			ret = ft_rsvm_setup(blob, rsvm_cfg);
		}

		/* It is okay if there is no FDT node to setup. */
	}

	return ret;
}
