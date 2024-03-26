/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef MACH_SECUREBOOT_H
#define MACH_SECUREBOOT_H

#define TC_SB_ENABLE (0x1)
#define TC_SB_DISABLE (0x0)

void tc_secureboot_init(void);
int tc_secureboot_state(void);
#if CONFIG_IS_ENABLED(TCC_SECUREBOOT_ENABLE)
int tc_secureboot_enable(int simmode, int reboot);
#endif

#endif /* MACH_SECUREBOOT_H */
