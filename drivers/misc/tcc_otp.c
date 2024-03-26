// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include <fs.h>
#include <linux/arm-smccc.h>
#include <mach/smc.h>
#include <mach/otp.h>
#include <cpu_func.h>

#if defined(CONFIG_CMD_TCC_OTP3)
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include <stdlib.h>
#endif

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

#if !defined(CONFIG_CMD_TCC_OTP3)
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

#else

static uint32_t dec_otp3_image(uint8_t *buf, uint32_t len, uint8_t *password)
{
	int __result = TRUE;
	uint8_t salt[SALT_LENGTH] = {0};
	uint8_t iv[IV_LENGTH] = {0};
	uint8_t sha[SHA_LENGTH] = {0};
	uint8_t key[AES128_KEY_LENGTH] = {0};
	uint8_t temp[32] = {0};
	uint8_t key_exp[AES256_EXPAND_KEY_LENGTH];
	uint8_t *pArr;
	size_t password_len = strlen(password);
	sha256_context *ctx;

	ctx = malloc(sizeof(sha256_context));
	sha256_starts(ctx);
	sha256_update(ctx, buf, (len - SHA_LENGTH));
	sha256_finish(ctx, temp);
	memcpy((void *)sha, (void *)temp, SHA_LENGTH);
	if (memcmp(sha, (void *)(buf + (len - SHA_LENGTH)), SHA_LENGTH) != 0) {
		pr_force("Integrity Check Error\n");
		__result = FALSE;
	}
	else {
		memcpy((void *)salt, (void *)(buf + (len - SALT_LENGTH - SHA_LENGTH)), SALT_LENGTH);
		memcpy((void *)iv, (void *)(buf + (len - SALT_LENGTH - IV_LENGTH - SHA_LENGTH)), IV_LENGTH);
		pArr = (uint8_t *)malloc(password_len + SALT_LENGTH);

		memcpy((void *)pArr, (void *)(password), password_len);
		memcpy((void *)(pArr + password_len), (void *)salt, SALT_LENGTH);

		sha256_starts(ctx);
		sha256_update(ctx, pArr, (password_len + SALT_LENGTH));
		sha256_finish(ctx, temp);
		memcpy((void *)key, (void *)temp, AES128_KEY_LENGTH);

		aes_expand_key(key, AES128_KEY_LENGTH, key_exp);
		aes_cbc_decrypt_blocks(AES128_KEY_LENGTH, key_exp, iv,
			(buf + sizeof(struct OTPD_HEADER)), (buf + sizeof(struct OTPD_HEADER)),
			(len - sizeof(struct OTPD_HEADER) - SALT_LENGTH - IV_LENGTH - SHA_LENGTH)/AES_BLOCK_LENGTH);

		free(pArr);
	}
	free(ctx);

	return __result;
}

/**
 * This function writes OTP values to OTP areas. OTP image includes information
 * of OTP areas and values. \param[in] buf physical address of buffer where OTP
 * image was loaded. \param[in] len length of buffer where OTP image was loaded.
 * \return error status.
 */
int write_otp_by_otp3_image(uint8_t *buf, uint8_t *password)
{
	int __result = FALSE;
	int ret, i, j;
	struct OTP_DATA_HEADER *otp_data_h;
	struct OTPD_HEADER *otpd_h;
	uint32_t offset, size;
	uint8_t *value_addr;

	otp_data_h = (struct OTP_DATA_HEADER *)buf;
	buf += sizeof(struct OTP_DATA_HEADER);
	otpd_h = (struct OTPD_HEADER *)buf;

	if (!memcmp("OTPD", otp_data_h->cTag, 4) && !memcmp("OTP3", otpd_h->cTag, 4)) {
		dec_otp3_image(buf, otp_data_h->ulLen, password);
		buf += sizeof(struct OTPD_HEADER);

		for (i = 0; i < otpd_h->count; i++) {
			offset = *(uint32_t *)buf;
			size = *(uint32_t *)(buf + 4);
			buf += 8;
			if (size == 4) {
				value_addr = buf;
				ret = write_otp(offset, size, (uint8_t *)value_addr);
				if (ret != 0) {
					pr_force("write OTP error(0x%x)\n", ret);
					goto out;
				}
				pr_force("Write[0x%08X] to OTP ROM offset[0x%04X]\n",
					*(uint32_t *)value_addr, offset);
				buf += size;
			}
			else if (size == 16) {
				value_addr = buf;
				ret = write_otp(offset, size, (uint8_t *)value_addr);
				if (ret != 0) {
					pr_force("write OTP error(0x%x)\n", ret);
					goto out;
				}
				pr_force("Write[0x%08X 0x%08X 0x%08X 0x%08X] to OTP ROM offset[0x%04X]\n",
					*(uint32_t *)(value_addr), *(uint32_t *)(value_addr + 4),
					*(uint32_t *)(value_addr + 8), *(uint32_t *)(value_addr + 12), offset);
				buf += size;
			}
			else if (size > 16) {
				if ((size % 16) == 0) {
					for (j = 0; j < (size / 16); j++) {
						value_addr = buf;
						ret = write_otp((offset + (j * 16)), 16, (uint8_t *)value_addr);
						if (ret != 0) {
							pr_force("write OTP error(0x%x)\n", ret);
							goto out;
						}
						pr_force("Write[0x%08X 0x%08X 0x%08X 0x%08X] to OTP ROM offset[0x%04X]\n",
							*(uint32_t *)(value_addr), *(uint32_t *)(value_addr + 4),
							*(uint32_t *)(value_addr + 8), *(uint32_t *)(value_addr + 12),
							(offset + (j * 16)));
						buf += 16;
					}
				}
				else {
					pr_force("OTP data size error!\n");
					goto out;
				}
			}
			else {
				pr_force("OTP data size error!\n");
				goto out;
			}
			pr_force("\n");
		}
		pr_force("OTP fuse OK!\n");
		__result = TRUE;
	}
out:
	return __result;
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
int read_otp_by_otp3_image(uint8_t *buf, uint32_t len, uint8_t *password)
{
	int __result = FALSE;
	int ret, i, j;
	struct OTP_DATA_HEADER *otp_data_h;
	struct OTPD_HEADER *otpd_h;
	uint32_t offset, size, temp_data;
	unsigned long data = CONFIG_SYS_LOAD_ADDR + len;
	uint8_t *value_addr;
	int flag = 0;

	otp_data_h = (struct OTP_DATA_HEADER *)buf;
	buf += sizeof(struct OTP_DATA_HEADER);
	otpd_h = (struct OTPD_HEADER *)buf;

	if (!memcmp("OTPD", otp_data_h->cTag, 4) && !memcmp("OTP3", otpd_h->cTag, 4)) {
		dec_otp3_image(buf, otp_data_h->ulLen, password);
		buf += sizeof(struct OTPD_HEADER);

		for (i = 0; i < otpd_h->count; i++) {
			offset = *(uint32_t *)buf;
			size = *(uint32_t *)(buf + 4);
			buf += 8;
			if (size == 4) {
				value_addr = buf;
				memset((void *)data, 0, 4);
				ret = read_otp(offset, size, (uint8_t *)data);
				if (ret != 0) {
					pr_force("Read OTP error(0x%x)\n", ret);
					goto out;
				}

				pr_force("DATA[0x%08X] from OTP ROM offset[0x%04X]\n",
						*(uint32_t *)data, offset);
				pr_force("DATA[0x%08X] from OTP Image(#%d)[0x%04X]\n",
						*(uint32_t *)value_addr, i, offset);

				if ((*(uint32_t *)data & *(uint32_t *)value_addr) != *(uint32_t *)value_addr) {
					flag = 1;
				}

				buf += size;
			}
			else if (size == 16) {
				value_addr = buf;
				memset((void *)data, 0, 4);
				ret = get_crc32_of_otp(offset, size, (uint32_t *)data);
				if (ret != 0) {
					pr_force("Read OTP error(0x%x)\n", ret);
					goto out;
				}

				pr_force("CRC[0x%08X] from OTP ROM offset[0x%04X]\n",
						*(uint32_t *)data, offset);

				temp_data = *(uint32_t *)data;

				memset((void *)data, 0, 4);

				ret = get_crc32_of_raw(value_addr, size, (uint8_t *)data);
				if (ret != 0) {
					pr_force("Read OTP error(0x%x)\n", ret);
					goto out;
				}

				pr_force("CRC[0x%08X] from OTP Image(#%d)[0x%04X]\n",
						*(uint32_t *)data, i, offset);

				if (*(uint32_t *)data != temp_data) {
					flag = 1;
				}

				buf += size;
			}
			else if (size > 16) {
				if ((size % 16) == 0) {
					for (j = 0; j < (size / 16); j++) {
						value_addr = buf;
						memset((void *)data, 0, 4);
						ret = get_crc32_of_otp((offset + (j * 16)), 16, (uint32_t *)data);
						if (ret != 0) {
							pr_force("Read OTP error(0x%x)\n", ret);
							goto out;
						}

						pr_force("CRC[0x%08X] from OTP ROM offset[0x%04X]\n",
								*(uint32_t *)data, offset);

						temp_data = *(uint32_t *)data;

						memset((void *)data, 0, 4);

						ret = get_crc32_of_raw(value_addr, 16, (uint8_t *)data);
						if (ret != 0) {
							pr_force("Read OTP error(0x%x)\n", ret);
							goto out;
						}

						pr_force("CRC[0x%08X] from OTP Image(#%d-%d)[0x%04X]\n",
								*(uint32_t *)data, i, j, offset);

						if (*(uint32_t *)data != temp_data) {
							flag = 1;
						}
						buf += 16;
					}
				}
				else {
					pr_force("OTP data size error!\n");
					goto out;
				}
			}
			else {
				pr_force("OTP data size error!\n");
				goto out;
			}
			pr_force("\n");
		}
		if (flag != 0) {
			goto out;
		}
		pr_force("OTP verify OK!\n");
		__result = TRUE;
	}
out:
	return __result;
}
#endif
