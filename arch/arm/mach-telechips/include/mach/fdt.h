/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef MACH_FDT_H
#define MACH_FDT_H

struct reserved_memory_config {
	const char *const path;
	bool enable;
};

const struct reserved_memory_config *get_reserved_memory_config(void);

#endif /* MACH_FDT_H */
