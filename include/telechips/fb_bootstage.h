/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef FB_BOOTSTAGE_H
#define FB_BOOTSTAGE_H

#include <bootstage.h>

#if CONFIG_IS_ENABLED(FB_BOOTSTAGE)
#define BOOTSTASGE_NAME(name) \
	bootstage_mark_name(BOOTSTAGE_ID_ALLOC, (name));
#else
#define BOOTSTASGE_NAME(name) (void)(name);
#endif

#endif