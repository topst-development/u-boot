/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCC_HSM_FIRMWARE_H
#define TCC_HSM_FIRMWARE_H

int tcc_hsm_secureboot_image_verify(
	unsigned int header_addr, unsigned int header_size,
	unsigned int img_addr, unsigned int img_size, unsigned int tccimgid);

int tcc_hsm_secureboot_image_encrypt(
	unsigned int img_addr, unsigned int img_size, unsigned int tccimgid,
	unsigned int bootid);

int tcc_hsm_secureboot_key_fuse(void);

int tcc_hsm_secureboot_enable(void);

#endif /* TCC_HSM_FIRMWARE_H */
