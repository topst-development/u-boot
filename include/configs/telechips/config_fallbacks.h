/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TELECHIPS_CONFIG_FALLBACKS_H
#define TELECHIPS_CONFIG_FALLBACKS_H

/*
 * include/configs/telechips/common.h
 */

#ifndef COMMAND_AB
#define COMMAND_AB "true"
#endif

/*
 * include/configs/<CONFIG_SYS_CONFIG_NAME>.h
 */

#ifndef ARCH_EXTRA_ENV_SETTINGS
#define ARCH_EXTRA_ENV_SETTINGS
#endif

#endif /* TELECHIPS_CONFIG_FALLBACKS_H */
