/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <mach/smc.h>
#include <linux/arm-smccc.h>
#include <tcc_hsm_fw.h>

int tcc_hsm_secureboot_image_verify(
	unsigned int header_addr, unsigned int header_size,
	unsigned int img_addr, unsigned int img_size, unsigned int tccimgid)
{
	struct arm_smccc_res res;

	if ((header_addr + header_size) != img_addr) {
		return -1;
	}

	arm_smccc_smc(
		SIP_TCSB_IMAGE_VERIFY, (ulong)header_addr,
		(ulong)(header_size + img_size), tccimgid, 0, 0, 0, 0, &res);
	if (res.a0 == 0) {
		return 0;
	}
	return -1;
}

int tcc_hsm_secureboot_image_encrypt(
	unsigned int img_addr, unsigned int img_size,
	unsigned int tccimgid, unsigned int bootid)
{
	struct arm_smccc_res res;
	arm_smccc_smc(
		SIP_TCSB_MFCMD_GET_SECUREIMAGE, bootid, tccimgid,
		img_addr, img_size, 0, 0, 0, &res);
	return res.a0;
}

int tcc_hsm_secureboot_key_fuse(void)
{
	struct arm_smccc_res res;
	arm_smccc_smc(
		SIP_TCSB_MFCMD_SECUREBOOT_KEY_WRITE, 0, 0, 0, 0, 0, 0, 0, &res);
	return res.a0;
}

int tcc_hsm_secureboot_enable(void)
{
	struct arm_smccc_res res;
	arm_smccc_smc(
		SIP_TCSB_MFCMD_SECUREBOOT_ENABLE, 0, 0, 0, 0, 0, 0, 0, &res);
	return res.a0;
}
