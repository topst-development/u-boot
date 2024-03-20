// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

/**
 * \brief This function checks the secure boot flag in OTP-ROM..
 */

#define TC_SB_ENABLE (0x1u)
#define TC_SB_DISABLE (0x0u)

uint32_t tc_check_secureboot(void);

int32_t tc_secureboot_enable(void);
void tc_secureboot_individualize(void);
