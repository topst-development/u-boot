// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __SERIAL_H
#define __SERIAL_H

#include <dm/ofnode.h>

#if defined(CONFIG_TCC805X)
#define TCC_SERIAL_MAX_CH 9
#elif defined(CONFIG_TCC803X)
#define TCC_SERIAL_MAX_CH 5
#endif

void tcc_serial_pre_init(ofnode cur_serial_node);

#endif
