// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <fdt_support.h>
#include <mach/fdt.h>

static int ft_rsvm_setup(void *blob, const struct reserved_memory_config *cfg)
{
	int ret = 0;

	for ( ; cfg->path != NULL; cfg++) {
		const char *path = cfg->path;
		int node;

		if (cfg->enable) {
			node = fdt_path_offset(blob, path);
			ret = node;

			if (node >= 0) {
				ret = fdt_delprop(blob, node, "status");
				ret = (ret != -FDT_ERR_NOTFOUND) ? ret : 0;
			}

			if (ret != 0) {
				pr_err("Error: failed to enable %s\n", path);
				break;
			}
		}
	}

	return ret;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	int ret = fdt_check_header(blob);

	if (ret != 0) {
		pr_err("ERROR: invalid fdt format\n");
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
