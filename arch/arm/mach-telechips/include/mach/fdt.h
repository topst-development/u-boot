/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef MACH_FDT_H
#define MACH_FDT_H

struct reserved_memory_config {
	const char *const name;
	bool enable;
	bool exact_match;
};

const struct reserved_memory_config *get_reserved_memory_config(void);

#define reserved_memory_config_end(cfg) \
	(((cfg) == NULL) || ((cfg)->name == NULL))

#define for_each_reserved_memory_config(cfg) \
	for ( ; !reserved_memory_config_end(cfg); (cfg)++)

#endif /* MACH_FDT_H */
