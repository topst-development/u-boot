// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TCC_VERIFICATION_TOOLS_H__
#define __TCC_VERIFICATION_TOOLS_H__
extern int vioc_irq_init;
int tcc_tav_db15(int test_case);
int tcc_tav_db05(int test_case);
int tcc_tav_db06(int test_case);
int tcc_tav_db07(int test_case);
int tcc_tav_db12(int test_case);
int tcc_tav_db08(int test_case);
int tcc_tav_irq(int enable);
#endif /* __TCC_VERIFICATION_TOOLS_H__ */
