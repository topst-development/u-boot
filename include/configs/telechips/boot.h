/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TELECHIPS_BOOT_CONFIG_H
#define TELECHIPS_BOOT_CONFIG_H

#if defined(CONFIG_FIT)
#include "boot_fit.h"
#else
#include "boot_telechips.h"
#endif

#endif /* TELECHIPS_BOOT_CONFIG_H */
