// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_FDT_H
#define TCC_FDT_H

struct reserved_memory_config {
	const char *const path;
	bool enable;
};

const struct reserved_memory_config *get_reserved_memory_config(void);

#endif /* TCC_FDT_H */
