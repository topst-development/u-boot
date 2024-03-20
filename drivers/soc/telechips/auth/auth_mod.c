// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 */

#include <stdint.h>
#include <string.h>
#include <mach/ddr_set.h>
#include <cpu_func.h>
#include <time.h>
#include <hsm_mbox.h>
#include <auth_mod.h>
#include <linux/arm-smccc.h>
#include <mach/smc.h>
#include <image.h>
#include <mach/tcc_secureboot.h>

#if !defined(CONFIG_TCC803X)
#define SUPPORT_HSM
#endif

static unsigned int image_verify(
	unsigned int header_addr, unsigned int header_size,
	unsigned int img_addr, unsigned int img_size, unsigned int tccimgid)
{
#if defined(SUPPORT_HSM)
	ap_send_to_hsm(header_addr, header_size, img_addr, img_size, tccimgid);
	return ap_recv_from_hsm();
#else
	if ((header_addr + header_size) != img_addr) {
		return ERROR_UNKNOWN_ERROR;
	}

	struct arm_smccc_res res;

	arm_smccc_smc(
		SIP_TCSB_IMAGE_VERIFY, (ulong)header_addr,
		(ulong)(header_size + img_size), tccimgid, 0, 0, 0, 0, &res);
	if (res.a0 == 0) {
		return ERROR_VERIFY_OK;
	}
	return ERROR_VERIFY_FAIL;
#endif
}

/*
 * Authenticate a certificate/image
 *
 * Return: 0 = success, Otherwise = error
 */
int32_t tc_verify_img(uintptr_t cert_ptr)
{
	unsigned int ret;
	uint64_t base_time;
	uint64_t verification_time;
	uintptr_t img_ptr;
	uint32_t img_len, is_sboot;
	struct tc_img_hdr *tc_h;
	int32_t non_sboot_hdr;

	tc_h = (struct tc_img_hdr *)cert_ptr;
	img_ptr = cert_ptr + IMAGE_CERT_SIZE;
	img_len = tc_h->img_size;

	non_sboot_hdr = tc_image_check_header(tc_h);
	is_sboot = tc_check_secureboot();

	if (is_sboot == TC_SB_DISABLE) {
		(void)pr_force("Non-secure boot (secure boot flag is clear)\n");
		if (non_sboot_hdr != 0) { // non tc image header
			return NON_TC_IMG;
		}
	}

	pr_force(
		"[Secure Boot] img_id(0x%04x), cert_ptr(0x%lx), img_ptr(0x%lx)\n",
		0, cert_ptr, img_ptr);
	base_time = get_timer_us(0);
	ret = image_verify(cert_ptr, IMAGE_CERT_SIZE, img_ptr, img_len, 0);
	verification_time = get_timer_us(base_time);
	invalidate_dcache_range(
		(unsigned long)cert_ptr, (unsigned long)img_ptr + img_len);

	if (ret == ERROR_VERIFY_OK) {
		pr_force(
			"[Secure Boot] SUCCESS (verification time : %llu ms)\n",
			(verification_time / 1000));
		return VERIFY_OK;
	}
	pr_force("[Secure Boot] FAIL (0x%08x)\n", ret);
	return VERIFY_FAIL;
}
