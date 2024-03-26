// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 */

#include <stdint.h>
#include <string.h>
#include <cpu_func.h>
#include <time.h>
#include <linux/printk.h>
#include <mach/secureboot.h>
#include <tcc_hsm_fw.h>
#include <image-tcc.h>

static int image_verify(
	unsigned int header_addr, unsigned int header_size,
	unsigned int img_addr, unsigned int img_size, unsigned int tccimgid)
{
	return tcc_hsm_secureboot_image_verify(
		header_addr, header_size, img_addr, img_size, tccimgid);
}

int tc_image_check_header(const struct tc_img_hdr *hdr)
{
	return memcmp(TC_IMG_MAGIC, hdr->cert_magic, 4);
}

/*
 * Authenticate a certificate/image
 *
 * Return: 0 = success, Otherwise = error
 */
int tc_image_verify(uintptr_t hdr_ptr)
{
	int ret;
	uint64_t base_time;
	uint64_t verification_time;
	uintptr_t img_ptr;
	uint32_t img_len, is_sboot;
	struct tc_img_hdr *tc_h;
	int32_t non_sboot_hdr;

	tc_h = (struct tc_img_hdr *)hdr_ptr;
	img_ptr = hdr_ptr + IMAGE_HDR_SIZE;
	img_len = tc_h->img_size;

	non_sboot_hdr = tc_image_check_header(tc_h);
	is_sboot = tc_secureboot_state();

	if (is_sboot == TC_SB_DISABLE) {
		(void)pr_force("Non-secure boot (secure boot flag is clear)\n");
		if (non_sboot_hdr != 0) { // non tc image header
			return NON_TC_IMG;
		}
	}

	pr_force(
		"[Secure Boot] img_id(0x%04x), hdr_ptr(0x%lx), img_ptr(0x%lx)\n",
		0, hdr_ptr, img_ptr);

	base_time = get_timer_us(0);
	ret = image_verify(hdr_ptr, IMAGE_HDR_SIZE, img_ptr, img_len, 0);
	verification_time = get_timer_us(base_time);
	invalidate_dcache_range(
		(unsigned long)hdr_ptr, (unsigned long)img_ptr + img_len);

	if (ret == 0) {
		pr_force(
			"[Secure Boot] SUCCESS (verification time : %llu ms)\n",
			(verification_time / 1000));
		return VERIFY_OK;
	}
	pr_force("[Secure Boot] FAIL (%d)\n", ret);
	return VERIFY_FAIL;
}
