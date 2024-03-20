/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef VIOC_DUMP_H
#define VIOC_DUMP_H

void vioc_read_and_dump_regs(void __iomem *reg_base,
			     unsigned int *data, unsigned int len,
			     int dump);
#endif
