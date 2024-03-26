// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include <fs.h>
#include <stdlib.h>
#include <mach/otp.h>
#include <asm/dma-mapping.h>

static int do_otp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc < 6)
		return CMD_RET_USAGE;

	const char *filename;
	char *cmd;
	loff_t nread;
	struct OTP_DATA_HEADER *h;
	uintptr_t data_addr;
	char *buf;
	int ret = CMD_RET_FAILURE;

	/* File read from dev */
	if (fs_set_blk_dev(argv[2], argv[3], FS_TYPE_ANY)) {
		return CMD_RET_FAILURE;
	}

	filename = argv[4];
	ret = fs_size(filename, &nread);
	if (ret < 0) {
		pr_force("Can't not read file\n");
		return CMD_RET_FAILURE;
	}
	buf = dma_alloc_coherent(nread, &data_addr);

	/* File read from dev */
	if (fs_set_blk_dev(argv[2], argv[3], FS_TYPE_ANY)) {
		goto out;
	}

	ret = fs_read(filename, data_addr, 0, 0, &nread);
	if (ret < 0) {
		pr_force("Can't not read file\n");
		goto out;
	}

	h = (struct OTP_DATA_HEADER *)data_addr;

	if (memcmp("OTPD", h->cTag, 4) != 0) {
		pr_force("OTP Data Header Marker error!\n");
		goto out;
	}

	cmd = argv[1];
	if (!strcmp(cmd, "verify")) {
		if (read_otp_by_otp3_image((uint8_t *) data_addr, nread, argv[5]) != TRUE) {
			pr_force("OTP verify failed\n");
			goto out;
		}
	} else if (!strcmp(cmd, "write")) {
		if (write_otp_by_otp3_image((uint8_t *) data_addr, argv[5]) != TRUE) {
			pr_force("OTP write failed\n");
			goto out;
		}
	}
	pr_force("\n");
	ret = 0;
out:
	dma_free_coherent(buf);
	return ret;
}

static int do_otp_read(struct cmd_tbl *cmdtp, int flag, int argc,
		char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	uintptr_t addr;
	char *addr_buf;
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

	addr_buf = dma_alloc_coherent(size, &addr);

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
				dma_free_coherent(addr_buf);
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
	dma_free_coherent(addr_buf);

	return 0;
}

static int
do_otp_write(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc < 4)
		return CMD_RET_USAGE;

	uintptr_t crc_addr;
	uintptr_t value_addr;
	int err;
	uint32_t offset, size, crc;
	uint32_t value[4] = { 0 };
	char *crc_buf;
	char *value_buf;

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

	crc_buf = dma_alloc_coherent(4, &crc_addr);
	value_buf = dma_alloc_coherent(size, &value_addr);
	memcpy((void *)value_addr, value, size);

	err = get_crc32_of_raw((uint8_t *) value_addr, size,
			(uint8_t *) crc_addr);
	if (err != 0) {
		pr_force("Get CRC error(0x%x)\n", err);
		dma_free_coherent(crc_buf);
		dma_free_coherent(value_buf);
		return 0;
	}
	memcpy(&crc, (void *)crc_addr, 4);
	pr_force("\nCRC of the write value : 0x%x\n", crc);

	err = write_otp(offset, size, (uint8_t *) value_addr);
	if (err != 0) {
		pr_force("write OTP error(0x%x)\n", err);
		dma_free_coherent(crc_buf);
		dma_free_coherent(value_buf);
		return 0;
	}
	pr_force("Write value to OTP ROM 0x%x (%d-bit)\n", offset, (size * 8));
	pr_force("Write OTP successfully\n\n");

	dma_free_coherent(crc_buf);
	dma_free_coherent(value_buf);

	return 0;
}

static int
do_otp_hsmrb(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	uintptr_t crc_addr;
	uintptr_t value_addr;
	char *cmd;
	uint32_t offset, size, err;
	uint32_t crc, buf;
	uint32_t value[4] = { 0, 0, 0, 0 };
	uint32_t i;
	char *crc_buf;
	char *value_buf;

	cmd = argv[1];

	if (!strcmp(cmd, "read")) {
		for (i = 0; i < 2; i++) {
			offset = (i == 0) ? 0x34 : 0x834;
			size = 4;

			crc_buf = dma_alloc_coherent(size, &crc_addr);
			value_buf = dma_alloc_coherent(size, &value_addr);

			err = read_otp(offset, size, (uint8_t *) value_addr);

			if (err != 0 && err != 0xFFFF) {
				pr_force("read OTP error(0x%x)\n", err);
				goto out;
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
					goto out;
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

			crc_buf = dma_alloc_coherent(size, &crc_addr);
			value_buf = dma_alloc_coherent(size, &value_addr);
			memcpy((void *)value_addr, value, size);

			err = get_crc32_of_raw((uint8_t *) value_addr, size,
					(uint8_t *) crc_addr);
			if (err != 0) {
				pr_force("Get CRC error(0x%x)\n", err);
				goto out;
			}
			memcpy(&crc, (void *)crc_addr, 4);
			pr_force("\nCRC of the write value : 0x%x\n", crc);

			err = write_otp(offset, size, (uint8_t *) value_addr);
			if (err != 0) {
				pr_force("write OTP error(0x%x)\n", err);
				goto out;
			}
		}
		pr_force("Write OTP successfully\n\n");
	} else {
		pr_force("Option error(%s)\n\n", argv[1]);
		return 0;
	}

out:
	dma_free_coherent(crc_buf);
	dma_free_coherent(value_buf);
	return 0;
}

U_BOOT_CMD(otp, 6, 0, do_otp, "OTP sub-system",
	   "\n"
	   "otp verify <interface> [<dev[:part]> <filename> <password>\n"
	   "    - Verify 'filename' from 'dev' on 'interface' OTP Image\n"
	   "otp write <interface> [<dev[:part]> <filename> <password>\n"
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

