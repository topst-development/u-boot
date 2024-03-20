// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef OTP_H
#define OTP_H

#define FALSE 0
#define TRUE 1

#define READ_OTP_IMAGE_DATA 1
#define READ_OTP_ROM_DATA 0

/* Read Data from OTP Image
 * if it is 128 bit area data, it stores CRC value ot OTP data
 * if it is 32 bit area data, it stores OTP data
 */
struct otp_image_output {
	uint32_t data;
	uint32_t crc;
};

struct OTP_DATA_HEADER {
	uint8_t cTag[4];
	uint32_t ulLen;
	uint32_t ulCRC;
};

struct FWDN_FILE_H_T {
	unsigned char ucMarker[8];
	unsigned int ulImageHeaderLength;
	unsigned int ulCRC;
};

struct FWDN_IMG_H_T {
	unsigned char ucMarker[16];
	unsigned char ucType[16];
	unsigned char ucReserved1[8];
	unsigned int ulSize;
	unsigned char ucReserved2[4];
};

int fwdn_drv_otp_write(unsigned char *oBuff, unsigned int ulTotalSize);
int read_otp(uint32_t offset, uint32_t len, uint8_t *buf);
int write_otp(uint32_t offset, uint32_t len, uint8_t *buf);
int get_crc32_of_otp(uint32_t offset, uint32_t len, uint32_t *crc);
int get_crc32_of_raw(uint8_t *data, uint32_t len, uint8_t *crc);
int read_otp_by_otp_image(
	uint8_t *buf, uint32_t len, struct otp_image_output *output,
	uint32_t dnum, uint32_t flag);
int write_otp_by_otp_image(uint8_t *buf, uint32_t len);

#endif /* OTP_H */
