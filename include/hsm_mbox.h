// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 */

#ifndef HSM_MBOX_H
#define HSM_MBOX_H

#define ERROR_UNKNOWN_CID 0x80000001
#define ERROR_UNKNOWN_BSID 0x80000002
#define ERROR_VERIFY_FAIL 0x80000003
#define ERROR_VERIFY_OK 0x80000004
#define ERROR_HSMBOOT_FAIL 0x80000005
#define ERROR_HSMBOOT_OK 0x80000006
#define ERROR_UNKNOWN_ERROR 0x80000000

#define ERROR_HSM_MBOX_SUCCESS 0x0u

#define CMD_HSM_GET_SECURE_IMAGE 0x00070000u
#define CMD_HSM_KEY_WRITE 0x00080000u
#define CMD_HSM_SECURE_BOOT_ENABLE 0x00090000u

unsigned int ap_recv_from_hsm(void);
void ap_send_to_hsm(
	unsigned int header_addr, unsigned int header_size,
	unsigned int img_addr, unsigned int img_size, unsigned int tccimgid);

void ap_send_mf_cmd(
	unsigned int cmd, unsigned int img_addr,
	unsigned int img_size, unsigned int tccimgid);

#endif /* HSM_MBOX_H */
