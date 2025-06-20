// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_IMAGE_H
#define TCC_IMAGE_H

/* Telechips BOOT Image */

#define TC_IMG_MAGIC "CERT"

#define IMAGE_HDR_SIZE (0x200u)

#define NON_TC_IMG (0)
#define VERIFY_OK (1)
#define VERIFY_FAIL (-1)

struct tc_img_hdr {
	char cert_magic[4];
	char cert[252];

	char header_magic[4];
	unsigned int img_size; /* size in bytes */

	char dummy[248];
};

int tc_image_check_header(const struct tc_img_hdr *hdr);
int tc_image_verify(uintptr_t hdr_ptr);

#endif /* TCC_IMAGE_H */
