// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <fdt_support.h>
#include <mach/fdt.h>

static int ft_enable_match(void *blob, int parent, const char *seek, bool exact)
{
	int node;
	int ret = 0;

	fdt_for_each_subnode(node, blob, parent) {
		const char *name;
		size_t len;
		int cmp;

		name = fdt_get_name(blob, node, NULL);
		len = strnlen(seek, 31);

		cmp = memcmp(seek, name, len);

		if ((cmp == 0) && (!(exact) || (name[len] == '\0'))) {
			ret = fdt_delprop(blob, node, "status");
			ret = (ret != -FDT_ERR_NOTFOUND) ? ret : 0;

			if (exact || (ret != 0)) {
				break;
			}
		}
	}

	return ret;
}

static int ft_reserved_mem_setup(void *blob, int node)
{
	const struct reserved_memory_config *cfg = get_reserved_memory_config();
	int ret = 0;

	for_each_reserved_memory_config(cfg) {
		if (!cfg->enable) {
			continue;
		}

		ret = ft_enable_match(blob, node, cfg->name, cfg->exact_match);
		if (ret != 0) {
			break;
		}
	}

	if (ret != 0) {
		pr_err("ERROR: failed to set /reserved-memory/%s\n", cfg->name);
	}

	return ret;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	int ret = fdt_check_header(blob);

	if (ret == 0) {
		int reserved_mem = fdt_path_offset(blob, "/reserved-memory");

		if (reserved_mem >= 0) {
			ret = ft_reserved_mem_setup(blob, reserved_mem);
		}
	} else {
		pr_err("ERROR: invalid fdt format\n");
	}

	return ret;
}
