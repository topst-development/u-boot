// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include <fs.h>
#include <stdlib.h>
#include <otp.h>

#define NON_LOCKABLE_32_DATA 0x0000
#define NON_LOCKABLE_32_ADMIN 0x0200
#define LOCKABLE_32_DATA 0x0800
#define LOCKABLE_32_ADMIN 0x0A00
#define LOCKABLE_128_DATA 0x1000
#define LOCKABLE_128_ADMIN 0x2000

int otp_write(unsigned long addr, unsigned int nsize)
{
	struct FWDN_FILE_H_T *fh;
	struct FWDN_IMG_H_T *ih;

	fh = (struct FWDN_FILE_H_T *) addr;
	addr += sizeof(struct FWDN_FILE_H_T);
	ih = (struct FWDN_IMG_H_T *) addr;
	addr += sizeof(struct FWDN_IMG_H_T);

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
	if (ih->ulSize > (nsize + sizeof(struct FWDN_FILE_H_T)
				+ sizeof(struct FWDN_IMG_H_T))) {
		pr_force("File Size Error\n");
		return FALSE;
	}
	return fwdn_drv_otp_write((void *)addr, ih->ulSize);
}

static int do_otp(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	const char *filename;
	char *cmd;
	loff_t nread;
	unsigned long addr = CONFIG_SYS_LOAD_ADDR;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	cmd = argv[1];
	if (!strcmp(cmd, "write")) {
		if (argc != 5)
			return CMD_RET_USAGE;

		if (fs_set_blk_dev(argv[2], argv[3], FS_TYPE_ANY))
			return CMD_RET_FAILURE;

		filename = argv[4];

		ret = fs_read(filename, addr, 0, 0, &nread);
		if (ret < 0) {
			pr_force("Can't not read file\n");
			return CMD_RET_FAILURE;
		}

		if (otp_write(addr, nread) != TRUE) {
			pr_force("OTP write failed\n");
			return CMD_RET_FAILURE;
		}
	}

	return 0;
}

static void copy_otp_image_output(struct otp_image_output *dst,
				  struct otp_image_output *src, uint32_t dnum)
{
	int i;

	for (i = 0; i < dnum; i++) {
		dst[i].data = src[i].data;
		dst[i].crc = src[i].crc;
	}
}

static void print_otp_image_output(struct otp_image_output *src, uint32_t dnum)
{
	int i;

	for (i = 0; i < dnum; i++) {
		if (src[i].data != 0 && src[i].crc != 0) {
			pr_force("Error : OTP Image OUTPUT is Wrong\n");
			return;
		}
		pr_force(" -----------------------------\n");
		if (src[i].crc != 0) {
			pr_force("| [%d] => crc  :  0x%08x   |\n", i,
				 src[i].crc);
		} else {
			pr_force("| [%d] => data :  0x%08x   |\n", i,
				 src[i].data);
		}
	}
	pr_force(" -----------------------------\n");
}

static uint32_t get_dnum_of_otp_image(uint32_t *otp_image)
{
	uint32_t *puiData;
	uint32_t dnum;

	puiData = otp_image;
	dnum = puiData[24];	// number of to write otp data

	return dnum;
}

static int
do_otp_image_verify(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc < 5)
		return CMD_RET_USAGE;

	const char *filename;
	char *cmd;
	loff_t nread;
	int err;
	uint32_t dnum;

	struct otp_image_output *output;
	unsigned long data_addr = CONFIG_SYS_LOAD_ADDR;

	/* File read from dev */
	if (fs_set_blk_dev(argv[2], argv[3], FS_TYPE_ANY))
		return CMD_RET_FAILURE;

	filename = argv[4];
	err = fs_read(filename, data_addr, 0, 0, &nread);
	if (err < 0) {
		pr_force("Can't not read file\n");
		return CMD_RET_FAILURE;
	}

	cmd = argv[1];
	if (!strcmp(cmd, "verify")) {
		dnum = get_dnum_of_otp_image(
			(uint32_t *) data_addr); // number of to write otp data

		output = (struct otp_image_output *)(CONFIG_SYS_LOAD_ADDR
				+ (unsigned long)(nread * 2));

		memset((void *)output, 0,
				sizeof(struct otp_image_output) * dnum);

		/* Prepare OTP Image OUTPUT buffer */
		struct otp_image_output *readImage, *readOTP;

		readImage = (struct otp_image_output *)
			malloc(sizeof(struct otp_image_output) * dnum);
		readOTP = (struct otp_image_output *)
			malloc(sizeof(struct otp_image_output) * dnum);

		/* Read OTP data(CRC) from Image (set flag = 1) */
		err = read_otp_by_otp_image((uint8_t *) data_addr, nread,
				output, dnum, READ_OTP_IMAGE_DATA);
		if (err != TRUE) {
			pr_force("CRC OTP Image error(0x%x)\n", err);
			free(readImage);
			free(readOTP);
			return 0;
		}
		copy_otp_image_output(readImage, output, dnum);

		memset((void *)output, 0,
				sizeof(struct otp_image_output) * dnum);

		/* File read from dev */
		if (fs_set_blk_dev(argv[2], argv[3], FS_TYPE_ANY)) {
			free(readImage);
			free(readOTP);
			return CMD_RET_FAILURE;
		}

		filename = argv[4];
		err = fs_read(filename, data_addr, 0, 0, &nread);
		if (err < 0) {
			pr_force("Can't not read file\n");
			free(readImage);
			free(readOTP);
			return CMD_RET_FAILURE;
		}

		/* Read OTP data(CRC) from OTP (set flag = 0) */
		err = read_otp_by_otp_image((uint8_t *) data_addr, nread,
				output, dnum, READ_OTP_ROM_DATA);
		if (err != TRUE) {
			pr_force("CRC OTP Image error(0x%x)\n", err);
			free(readImage);
			free(readOTP);
			return 0;
		}
		copy_otp_image_output(readOTP, output, dnum);

		pr_force("\nOTP Data in \"%s\" Image :\n", filename);
		print_otp_image_output(readImage, dnum);
		pr_force("\nOTP Data of OTP ROM :\n");
		print_otp_image_output(readOTP, dnum);

		free(readImage);
		free(readOTP);
	} else if (!strcmp(cmd, "write")) {
		if (write_otp_by_otp_image((uint8_t *) data_addr, nread)
		    != TRUE) {
			pr_force("OTP write failed\n");
			return CMD_RET_FAILURE;
		}
	}
	pr_force("\n");

	return 0;
}

static int do_otp_read(cmd_tbl_t *cmdtp, int flag, int argc,
		char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	unsigned long addr = CONFIG_SYS_LOAD_ADDR;
	uint32_t offset, size, err;
	uint32_t crc, buf;

	offset = simple_strtoul(argv[1], NULL, 16);

	if ((offset >= NON_LOCKABLE_32_ADMIN && offset < LOCKABLE_32_DATA)
	    || (offset >= LOCKABLE_32_ADMIN && offset < LOCKABLE_128_DATA)
	    || (offset >= LOCKABLE_128_ADMIN)) {
		pr_force("Offset Error : Inaccessible area\n");
		return 0;
	}

	if (offset < LOCKABLE_128_DATA) {
		size = 4;
	} else if (offset < LOCKABLE_128_ADMIN) {
		size = 16;
	} else {
		pr_force("Offset Error : Inaccessible area\n");
		return 0;
	}

	memset((void *)addr, 0, size);

	err = read_otp(offset, size, (uint8_t *) addr);

	if (err != 0 && err != 0xFFFF) {
		pr_force("read OTP error(0x%x)\n", err);
	} else {
		pr_force("\nData(%d-bit) read from OTP ROM 0x%x\n", (size * 8),
			 offset);
		pr_force(" ===================================\n");
		pr_force("|    OTP Data     |    CRC Value    |\n");

		if (err == 0xFFFF) {
			/*
			 * 128-bit area get CRC
			 * Can not access area, return 0xffff
			 */
			err = get_crc32_of_otp(offset, size, (uint32_t *) addr);
			if (err != 0) {
				pr_force("Get CRC error(0x%x)\n", err);
				return 0;
			}
			memcpy(&crc, (void *)addr, 4);
			pr_force(" ===================================\n");
			pr_force("|        -        |    0x%08x   |\n", crc);
		} else {
			memcpy(&buf, (void *)addr, size);
			pr_force(" ===================================\n");
			pr_force("|    0x%08x   |        -        |\n", buf);
		}
		pr_force(" ===================================\n\n");
	}

	return 0;
}

static int
do_otp_write(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc < 4)
		return CMD_RET_USAGE;

	unsigned long crc_addr = CONFIG_SYS_LOAD_ADDR;
	unsigned long value_addr = CONFIG_SYS_LOAD_ADDR + 4;
	int err;
	uint32_t offset, size, crc;
	uint32_t value[4] = { 0 };

	offset = simple_strtoul(argv[1], NULL, 16);
	size = simple_strtoul(argv[2], NULL, 10) / 8;

	if ((offset >= NON_LOCKABLE_32_ADMIN && offset < LOCKABLE_32_DATA)
	    || (offset >= LOCKABLE_32_ADMIN && offset < LOCKABLE_128_DATA)
	    || (offset >= LOCKABLE_128_ADMIN)) {
		pr_force("Offset Error : Inaccessible area\n");
		return 0;
	}

	if (size != 4 && size != 16) {
		pr_force("Size Error : size must be 32-bit or 128-bit\n");
		return 0;
	}

	if ((offset < LOCKABLE_128_DATA) && (size != 4)) {
		pr_force("Size Error : 0x%x offset size must be 32-bit\n",
			 offset);
		return 0;
	}
	if ((offset >= LOCKABLE_128_DATA && offset < LOCKABLE_128_ADMIN)
	    && (size != 16)) {
		pr_force("Size Error : 0x%x offset size must be 128-bit\n",
			 offset);
		return 0;
	}

	value[0] = simple_strtoul(argv[3], NULL, 16);
	if (size > 4) {
		value[1] = simple_strtoul(argv[4], NULL, 16);
		value[2] = simple_strtoul(argv[5], NULL, 16);
		value[3] = simple_strtoul(argv[6], NULL, 16);
	}

	memset((void *)crc_addr, 0, 4);
	memset((void *)value_addr, 0, size);
	memcpy((void *)value_addr, value, size);

	err = get_crc32_of_raw((uint8_t *) value_addr, size,
			(uint8_t *) crc_addr);
	if (err != 0) {
		pr_force("Get CRC error(0x%x)\n", err);
		return 0;
	}
	memcpy(&crc, (void *)crc_addr, 4);
	pr_force("\nCRC of the write value : 0x%x\n", crc);

	err = write_otp(offset, size, (uint8_t *) value_addr);
	if (err != 0) {
		pr_force("write OTP error(0x%x)\n", err);
		return 0;
	}
	pr_force("Write value to OTP ROM 0x%x (%d-bit)\n", offset, (size * 8));
	pr_force("Write OTP successfully\n\n");

	return 0;
}

static int
do_otp_hsmrb(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	unsigned long crc_addr = CONFIG_SYS_LOAD_ADDR;
	unsigned long value_addr = CONFIG_SYS_LOAD_ADDR + 4;
	char *cmd;
	uint32_t offset, size, err;
	uint32_t crc, buf;
	uint32_t value[4] = { 0, 0, 0, 0 };
	uint32_t i;

	cmd = argv[1];

	if (!strcmp(cmd, "read")) {
		for (i = 0; i < 2; i++) {
			offset = (i == 0) ? 0x34 : 0x834;
			size = 4;

			err = read_otp(offset, size, (uint8_t *) value_addr);

			if (err != 0 && err != 0xFFFF) {
				pr_force("read OTP error(0x%x)\n", err);
				return 0;
			}

			pr_force(" ===================================\n");
			if (err == 0xFFFF) {
				/*
				 * 128-bit area get CRC
				 * Can not access area, return 0xffff
				 */
				err = get_crc32_of_otp(offset, size,
						(uint32_t *) crc_addr);
				if (err != 0) {
					pr_force("Get CRC error(0x%x)\n", err);
					return 0;
				}
				memcpy(&crc, (void *)crc_addr, 4);
				pr_force("|  0x%08x  |  0x%08x  |    -    |\n",
					 offset, crc);
			} else {
				memcpy(&buf, (void *)value_addr, size);
				pr_force("|  0x%08x  |  0x%08x  |    -    |\n",
					 offset, buf);
			}
		}
		pr_force(" ===================================\n\n");
	} else if (!strcmp(cmd, "write")) {
		for (i = 0; i < 2; i++) {
			offset = (i == 0) ? 0x34 : 0x834;
			value[0] = (i == 0) ? 0x00001000 : 0x00100000;
			size = 4;

			memset((void *)crc_addr, 0, size);
			memset((void *)value_addr, 0, size);
			memcpy((void *)value_addr, value, size);

			err = get_crc32_of_raw((uint8_t *) value_addr, size,
					(uint8_t *) crc_addr);
			if (err != 0) {
				pr_force("Get CRC error(0x%x)\n", err);
				return 0;
			}
			memcpy(&crc, (void *)crc_addr, 4);
			pr_force("\nCRC of the write value : 0x%x\n", crc);

			err = write_otp(offset, size, (uint8_t *) value_addr);
			if (err != 0) {
				pr_force("write OTP error(0x%x)\n", err);
				return 0;
			}
		}
		pr_force("Write OTP successfully\n\n");
	} else {
		pr_force("Option error(%s)\n\n", argv[1]);
	}

	return 0;
}

U_BOOT_CMD(otp, 5, 0, do_otp, "OTP sub-system",
	   "otp write <interface> [<dev[:part]> <filename>\n"
	   "    - Write 'filename' from 'dev' on 'interface' to OTP\n");

U_BOOT_CMD(otpimage, 5, 0, do_otp_image_verify, "OTP sub-system",
	   "\n"
	   "otpimage verify <interface> [<dev[:part]> <filename>\n"
	   "    - Verify 'filename' from 'dev' on 'interface' OTP Image\n"
	   "otpimage write <interface> [<dev[:part]> <filename>\n"
	   "    - Write 'filename' from 'dev' on 'interface' to OTP\n");

U_BOOT_CMD(otpread, 2, 0, do_otp_read, "OTP sub-system",
	   " [offset(HEX)]\n"
	   "    - Read data from OTP 32-bit data area\n"
	   "    - Get CRC value form OTP 128-bit data area\n");

U_BOOT_CMD(otpwrite, 7, 0, do_otp_write, "OTP sub-system",
	   "\n"
	   "otpwrite [offset(HEX)] [size(32)] [value(HEX)]\n"
	   "otpwrite [offset(HEX)] [size(128)] [value1(HEX)] [value2(HEX)] [value3(HEX)] [value4(HEX)]\n"
	   "    - Write value to OTP ROM and calculate the crc value of the value\n"
	   "    - Size must be 32 or 128\n"
	   "    - If the size is 32, input one value.\n"
	   "    - If the size is 128, input four values.\n");

U_BOOT_CMD(otphsmrb, 2, 0, do_otp_hsmrb, "OTP sub-system",
	   "\n"
	   "[option(read/write)]\n"
	   "    - Write value to OTP ROM to support HSM redundant boot\n");
