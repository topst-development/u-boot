/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef VIOC_LUT_H
#define VIOC_LUT_H

#define LUT_TABLE_OFFSET 1U
#define LUT_COLOR_DEPTH 8U
#define LUT_TABLE_SIZE ((u32)1U << LUT_COLOR_DEPTH)

extern void tcc_set_lut_table(unsigned int lut_n, const unsigned int *table);
extern int tcc_set_lut_plugin(unsigned int lut_n, unsigned int plugComp);
extern int tcc_get_lut_plugin(unsigned int lut_n);
extern void tcc_set_lut_enable(unsigned int lut_n, unsigned int enable);
extern int tcc_get_lut_enable(unsigned int lut_n);

#endif
