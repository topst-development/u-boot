/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef MACH_BOARD_H
#define MACH_BOARD_H

void uclass_probe(enum uclass_id id);
void uclass_misc_probe(const char *name);

int board_device_init_early(void);
int board_device_init(void);
int board_device_init_late(void);

void clock_init_early(void);
void clock_init(void);

void pmic_configure(void);

#endif /* MACH_BOARD_H */
