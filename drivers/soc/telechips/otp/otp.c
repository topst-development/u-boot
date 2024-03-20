// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include <fs.h>
#include <linux/arm-smccc.h>
#include <mach/smc.h>
#include <otp.h>
#include <cpu_func.h>

static uint32_t check_otp_image_header(uint8_t *buf, uint32_t len)
{
	struct FWDN_FILE_H_T *fh;
	struct FWDN_IMG_H_T *ih;

	fh = (struct FWDN_FILE_H_T *)buf;
	buf += sizeof(struct FWDN_FILE_H_T);
	ih = (struct FWDN_IMG_H_T *)buf;
	buf += sizeof(struct FWDN_IMG_H_T);

	if (memcmp(fh->ucMarker, "[HEADER]", 8)) {
		pr_force("OTP File Marker Error\n");
		return FALSE;
	}
	if (memcmp(ih->ucMarker, "RAW_IMAGE", 9)) {
		pr_force("OTP Image Marker Error\n");
		return FALSE;
	}
	if (memcmp(ih->ucType, "OTP     ", 8)) {
		pr_force("OTP Image Type Error\n");
		return FALSE;
	}
	if (ih->ulSize > (len + sizeof(struct FWDN_FILE_H_T)
				+ sizeof(struct FWDN_IMG_H_T))) {
		pr_force("File Size Error\n");
		return FALSE;
	}
	return ih->ulSize;
}

int fwdn_drv_otp_write(unsigned char *oBuff, unsigned int ulTotalSize)
{
	int __result = TRUE;
	int ret;
	struct arm_smccc_res res;

	while (ulTotalSize > 0) {
#if defined(OTP_UID_INCLUDE)
		struct secure_uid_box *otpbox = (struct secure_uid_box *)oBuff;

		if (!memcmp(otpbox->uid_tag, "OTPSTORE", 8)) {
			unsigned int OTPSector = 0;
			char *wOTPBuf;

			if ((otpbox->uid_flags == 1)
			    && (otpbox->uid_size != 0)) {
				dpr_force(
					INFO,
					"[OTP_UID		] START OTP_UID Write !!\n");
				if (tzow_api(
					    0, oBuff + otpbox->uid_offset,
					    otpbox->uid_size)
				    != 0) {
					dpr_force(
						INFO,
						"[OTP_UID		] tzow_api write fail !!\n");
					__result = -1;
				}
			}
			break;
		}
#endif
		struct OTP_DATA_HEADER *h = (struct OTP_DATA_HEADER *)oBuff;

		oBuff += sizeof(struct OTP_DATA_HEADER);
		ulTotalSize -= 12;

		if (!memcmp("TZOW", h->cTag, 4)) {
			/* Move into ARM Trusted Firmware */
		} else if (!memcmp("OTPD", h->cTag, 4)) {
			flush_dcache_range((unsigned long)oBuff,
					(unsigned long)oBuff + h->ulLen);
			arm_smccc_smc(
				SIP_TCSB_OTP_WRITE, (unsigned long)oBuff,
				(unsigned long)h->ulLen, 0, 0, 0, 0, 0, &res);
			ret = res.a0;
			if (ret != 0) {
				pr_force("OTP fuse error! (0x%x)\n", ret);
				__result = FALSE;
				break;
			}
			pr_force("OTP fuse OK!\n");
		} else {
			break;
		}
		oBuff += h->ulLen;
		ulTotalSize -= h->ulLen;
	}

	return __result;
}

/**
 * This function writes OTP values to OTP areas. OTP image includes information
 * of OTP areas and values. \param[in] buf physical address of buffer where OTP
 * image was loaded. \param[in] len length of buffer where OTP image was loaded.
 * \return error status.
 */
int write_otp_by_otp_image(uint8_t *buf, uint32_t ilen)
{
	uint32_t len;

	len = check_otp_image_header(buf, ilen);
	if (len == FALSE)
		return FALSE;

	buf += sizeof(struct FWDN_FILE_H_T);
	buf += sizeof(struct FWDN_IMG_H_T);

	return fwdn_drv_otp_write(buf, len);
}

/**
 * This function writes OTP value to OTP area.
 * \param[in] offset offset of OTP area where OTP value will be stored.
 * \param[in] len length of OTP area where OTP value will be stored.
 * \param[in] buf buffer stored in value.
 * \return error status.
 */
int write_otp(uint32_t offset, uint32_t len, uint8_t *buf)
{
	struct arm_smccc_res res;

	flush_dcache_range((unsigned long)buf, (unsigned long)buf + len);
	arm_smccc_smc(
		SIP_WRITE_OTP, (unsigned long)offset, (unsigned long)len,
		(unsigned long)buf, 0, 0, 0, 0, &res);
	return res.a0;
}

/**
 * This function reads OTP data. It is restricted within 32bit area.
 * \param[in]  offset offset of OTP area where data was stored.
 * \param[in]  len length of buffer.
 * \param[out] buf buffer loaded OTP value.
 * \return error status.
 */
int read_otp(uint32_t offset, uint32_t len, uint8_t *buf)
{
	struct arm_smccc_res res;

	flush_dcache_range((unsigned long)buf, (unsigned long)buf + len);
	arm_smccc_smc(
		SIP_READ_OTP, (unsigned long)offset, (unsigned long)len,
		(unsigned long)buf, 0, 0, 0, 0, &res);
	return res.a0;
}

/**
 * This function verifies data of OTP area.
 * \param[in]  offset offset of OTP area.
 * \param[in]  len length of OTP area.
 * \param[out] crc crc vlue of data stored in OTP area.
 * \return error status
 */
int get_crc32_of_otp(uint32_t offset, uint32_t len, uint32_t *crc)
{
	struct arm_smccc_res res;

	flush_dcache_range((unsigned long)crc, (unsigned long)crc + 4u);
	arm_smccc_smc(
		SIP_GET_OTP_CRC, (unsigned long)offset, (unsigned long)len,
		(unsigned long)crc, 0, 0, 0, 0, &res);
	return res.a0;
}

/**
 * This function verifies data of raw data.
 * \param[in]  data raw data.
 * \param[in]  len length of raw data.
 * \param[out] crc crc vlue of raw data.
 * \return error status
 */
int get_crc32_of_raw(uint8_t *data, uint32_t len, uint8_t *crc)
{
	struct arm_smccc_res res;

	flush_dcache_range((unsigned long)data, (unsigned long)data + len);
	flush_dcache_range((unsigned long)crc, (unsigned long)crc + 4u);
	arm_smccc_smc(
		SIP_GET_RAW_CRC, (unsigned long)data, (unsigned long)len,
		(unsigned long)crc, 0, 0, 0, 0, &res);
	return res.a0;
}

/**
 * This function read OTP value from OTP area(flag=0) or Image(flag=1).
 * \param[in]  buf physical address of buffer where OTP image was loaded.
 * \param[in]  len length of buffer where OTP image was loaded.
 * \param[out] output buffer loaded read data.
 * \param[in]  dnum number of ouput data.
 * \param[in]  flag decide how to read data(0 : from OTP ROM, 1 : from OTP
 * Image). \return error status.
 */
int read_otp_by_otp_image(
	uint8_t *buf, uint32_t len, struct otp_image_output *output,
	uint32_t dnum, uint32_t flag)
{
	int __result = TRUE;
	int ret;
	uint32_t ulTotalSize;
	struct arm_smccc_res res;

	ulTotalSize = check_otp_image_header(buf, len);
	if (ulTotalSize == FALSE)
		__result = FALSE;

	buf += sizeof(struct FWDN_FILE_H_T);
	buf += sizeof(struct FWDN_IMG_H_T);

	while (ulTotalSize > 0) {
		struct OTP_DATA_HEADER *h = (struct OTP_DATA_HEADER *)buf;

		buf += sizeof(struct OTP_DATA_HEADER);
		ulTotalSize -= 12;

		if (!memcmp("TZOW", h->cTag, 4)) {
			/* Move into ARM Trusted Firmware */
		} else if (!memcmp("OTPD", h->cTag, 4)) {
			if (flag == READ_OTP_IMAGE_DATA) {
				flush_dcache_range(
					(unsigned long)buf,
					(unsigned long)buf + h->ulLen);
				flush_dcache_range(
					(unsigned long)output,
					(unsigned long)output
						+ (sizeof(uint32_t) * 2
						   * dnum));
				arm_smccc_smc(
					SIP_READ_OTP_FROM_IMAGE,
					(unsigned long)buf,
					(unsigned long)h->ulLen,
					(unsigned long)output, 0, 0, 0, 0,
					&res);
				ret = res.a0;
			} else if (flag == READ_OTP_ROM_DATA) {
				flush_dcache_range(
					(unsigned long)buf,
					(unsigned long)buf + h->ulLen);
				flush_dcache_range(
					(unsigned long)output,
					(unsigned long)output
						+ (sizeof(uint32_t) * 2
						   * dnum));
				arm_smccc_smc(
					SIP_READ_OTP_BY_IMAGE,
					(unsigned long)buf,
					(unsigned long)h->ulLen,
					(unsigned long)output, 0, 0, 0, 0,
					&res);
				ret = res.a0;
			} else {
				__result = FALSE;
				break;
			}

			if (ret != 0) {
				pr_force("OTP read error! (0x%x)\n", ret);
				__result = FALSE;
				break;
			}
		} else {
			break;
		}
		buf += h->ulLen;
		ulTotalSize -= h->ulLen;
	}

	return __result;
}
