// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 */

#ifndef SECURE_BOOT_H
#define SECURE_BOOT_H

#define IMAGE_CERT_SIZE (0x200u)

#define NON_TC_IMG (0)
#define VERIFY_OK (1)
#define VERIFY_FAIL (-1)

int32_t tc_verify_img(uintptr_t cert_ptr);

#endif /* SECURE_BOOT_H */
