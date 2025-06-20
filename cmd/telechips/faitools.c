// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022 Telechips, Inc
 *
 */

#include <common.h>
#include <command.h>
#include <usb.h>
#include <mmc.h>
#include <fs.h>
#include <div64.h>
#include <linux/math64.h>
#include <malloc.h>
#include <memalign.h>
#include <mapmem.h>
#include <u-boot/sha256.h>

#define SIZE_FAI_HEADER		0x60
#define SIZE_ALIGN		0x200UL

#ifdef CONFIG_SYS_TEXT_BASE
#define UPDATE_BUFFER_SIZE CONFIG_FAITOOLS_BUFFER_SIZE
#else
#define UPDATE_BUFFER_SIZE 16*1024*1024
#endif


typedef struct tag {
	char  tag[8];
	unsigned long value;
} __packed tag_t;

typedef struct fai_header {
	char hd_tag[8];
	__le32 hd_size;
	__le32 hd_crc;
	char image_type[16];
	char version[16];
	char area_name[16];
	tag_t t[2];
} __packed fai_header_t;

typedef struct chunk_header {
	__le64 des_addr;
	__le64 chunk_sz;
	unsigned long offset;
} __packed chunk_header_t;

unsigned long fai_read(const char *filename, void *buf, const char *interface, const char *devpart,
		unsigned long bytes, unsigned long pos)
{
	loff_t len_read = 0;

	if (fs_set_blk_dev(interface, devpart, FS_TYPE_ANY))
		return 0;

	if(fs_read(filename, (ulong)buf, (loff_t)pos, (loff_t)bytes, (loff_t*)&len_read) < 0) {
		printf("Invalidated FAI header\n");
		return 0;
	}

	return (unsigned long)len_read;
}

unsigned long chunk_write(const char *filename, struct mmc *mmc, char *buf,
		size_t buf_size, const char *src_interface,
		const char *src_devpart, chunk_header_t *chunk_hd)
{
	loff_t len_read = 0;
	unsigned long offset = chunk_hd->offset;
	unsigned long unit_size = 0;
	loff_t total_len_read = 0;
	sha256_context ctx0, ctx1;
	unsigned char output0[32];
	unsigned char output1[32];
	u32 n;
	unsigned long granules = 0;

	sha256_starts(&ctx0);
	sha256_starts(&ctx1);

	printf("[fai offset 0x%lx, dest_addr 0x%llx, chunk_size : 0x%llx]\n",
			chunk_hd->offset, chunk_hd->des_addr, chunk_hd->chunk_sz);
	if (chunk_hd->chunk_sz == 0)
		return 0;

	while (offset < chunk_hd->offset + chunk_hd->chunk_sz) {
		granules = offset % SIZE_ALIGN;
		if (fs_set_blk_dev(src_interface, src_devpart, FS_TYPE_ANY))
			return -1;

		if (offset + buf_size <= chunk_hd->offset + chunk_hd->chunk_sz + 512*(!!granules)) {
			unit_size = buf_size;
		} else {
			unit_size = chunk_hd->offset + chunk_hd->chunk_sz - offset + 512*(!!granules);
		}

		debug("read offset %lx unit_size %lx, granules %lx\n", offset&~(SIZE_ALIGN-1), unit_size, granules);
		if (fs_read(filename, (ulong)buf, (loff_t)offset&~(SIZE_ALIGN-1), (loff_t)unit_size, (loff_t*)&len_read) < 0) {
			printf("read failed\n");
			return -1;
		}

		memmove(buf, buf + granules, unit_size - granules);

		n = blk_dwrite(mmc_get_blk_desc(mmc), chunk_hd->des_addr/512, unit_size/512 -1*(!!granules), buf);
		if (n == 0) {
			printf("blk write error\n");
			return -1;
		}

		sha256_update(&ctx0, buf, unit_size - 512*(!!granules));
		memset(buf, 0, buf_size);

		n = blk_dread(mmc_get_blk_desc(mmc), chunk_hd->des_addr/512, unit_size/512 -1*(!!granules), buf);
		if (n == 0) {
			printf("blk read error\n");
			return -1;
		}
		sha256_update(&ctx1, buf, unit_size - 512*(!!granules));

		offset += unit_size - 512*(!!granules);
		chunk_hd->des_addr += unit_size - 512*(!!granules);
		total_len_read += len_read - 512*(!!granules);
	}

	sha256_finish(&ctx0, output0);
	sha256_finish(&ctx1, output1);
	if (strncmp(output0, output1, 32) != 0) {
		printf("\n verify error \n");
		printf("sha256 read : ");
		for (int i = 0; i < 32; i++) {
			printf("%02x", output0[i]);
		}
		printf("\n");
		printf("sha256 write : ");
		for (int i = 0; i < 32; i++) {
			printf("%02x", output1[i]);
		}
		printf("\n");
	}

	debug("total len read %llu\n", total_len_read);
	return (unsigned long)total_len_read;
}

loff_t get_fai_size(const char *filename, const char *interface,
		const char *devpart)
{
	loff_t size;

	if (fs_set_blk_dev(interface, devpart, FS_TYPE_ANY))
		return 1;

	if (fs_size(filename, &size) < 0)
		return CMD_RET_FAILURE;

	return size;
}

static int parse_fai_chunk_header(const char *filename, const char *interface,
		const char *devpart, chunk_header_t *chunk_hd)
{
	chunk_hd->offset += chunk_hd->chunk_sz;
	if (fai_read(filename, chunk_hd, interface, devpart, 16, chunk_hd->offset)) {
		chunk_hd->offset += 0x10;
		return 0;
	}
	return -1;
}

static int fai_write(const char *filename, const char *src_interface,
		const char *src_devpart, int dest_dev, fai_header_t *fai_hd)
{
	loff_t file_size = get_fai_size(filename, src_interface, src_devpart);
	chunk_header_t chunk_hd;
	unsigned char *buf = NULL;
	size_t buf_size = UPDATE_BUFFER_SIZE;
	struct mmc *mmc;
	int ret = 0;

	memset(&chunk_hd, 0, sizeof(chunk_header_t));
	chunk_hd.offset = fai_hd->hd_size;
	printf("hd_size %x, chunk_sz %llx\n", fai_hd->hd_size, chunk_hd.chunk_sz);

	mmc = find_mmc_device(dest_dev);
	if (!mmc) {
		printf("Not found mmc device\n");
		return -1;
	}
	if(mmc_init(mmc)) {
		printf("mmc init failed\n");
		return -1;
	}

	buf = memalign(CONFIG_SYS_CACHELINE_SIZE, buf_size);
	if(!buf)
		return -ENOMEM;

	while (chunk_hd.offset + chunk_hd.chunk_sz < file_size) {
		ret = parse_fai_chunk_header(filename, src_interface, src_devpart, &chunk_hd);
		if (ret < 0) {
			printf("Can't not parse fai chunk header\n");
			free(buf);
			return -1;
		}

		ret = chunk_write(filename, mmc, buf, buf_size, src_interface, src_devpart, &chunk_hd);
		if (ret < 0) {
			printf("chunk write error\n");
			free(buf);
			return -1;
		}
	}
	free(buf);
	return ret;
}

static int fai_info(const char *filename, const char *src_interface,
		const char *src_devpart, fai_header_t *fai_hd)
{
	loff_t file_size = get_fai_size(filename, src_interface, src_devpart);
	chunk_header_t chunk_hd;
	int ret = 0;
	int cnt_chunk = 0;

	memset(&chunk_hd, 0, sizeof(chunk_header_t));
	chunk_hd.offset = fai_hd->hd_size;
	printf("hd_size %x, chunk_sz %llx\n", fai_hd->hd_size, chunk_hd.chunk_sz);

	while (chunk_hd.offset + chunk_hd.chunk_sz < file_size) {
		ret = parse_fai_chunk_header(filename, src_interface, src_devpart, &chunk_hd);
		if (ret < 0) {
			printf("Can't not parse fai chunk header\n");
			return -1;
		}

		printf("[chunk number %d][fai offset 0x%lx, dest_addr 0x%llx, chunk_size : 0x%llx]\n",
				cnt_chunk++, chunk_hd.offset, chunk_hd.des_addr, chunk_hd.chunk_sz);
	}
	return ret;
}

static int parse_fai_header(const char *filename, const char *interface, const char *devpart, fai_header_t *fai_hd)
{
	unsigned int offset = 0;

	if (fai_read(filename, fai_hd, interface, devpart, SIZE_FAI_HEADER, offset)) {
		if (memcmp(&fai_hd->hd_tag, "[HEADER]", sizeof(fai_hd->hd_tag)) != 0) {
			printf("fai header is not matched\n");
			return -1;
		}
		if (memcmp(&fai_hd->image_type, "FILESYSTEM", 10) == 0) {
			debug("fai area type is filesystem\n");
		} else if (memcmp(&fai_hd->image_type, "RAW_IMAGE", 9) == 0) {
			debug("fai area type is raw image\n");
		} else {
			printf("unkown filesystem type!\n");
			return -1;
		}

		if (memcmp(&fai_hd->area_name, "SD Data", 7) == 0)
			debug("fai area name is SD Data\n");
	}

	return 0;
}

static int do_faitools_write(struct cmd_tbl *cmdtp, int flag,
		int argc, char * const argv[])
{
	char *src_interface;
	char *src_devpart;
	char *dest_interface;
	int dest_dev;
	fai_header_t fai_hd;
	char *fai_file;
	int ret = 0;

	if (argc < 6)
		return CMD_RET_USAGE;

	src_interface = argv[1];

	if (!strcmp(src_interface, "usb")) {
		src_devpart = argv[2];
		fai_file = argv[3];
		dest_interface = argv[4];
		dest_dev = simple_strtol(argv[5], NULL, 10);

		ret = parse_fai_header(fai_file, src_interface, src_devpart, &fai_hd);
		if (ret < 0) {
			printf("Can't not parse fai header\n");
			return CMD_RET_FAILURE;
		}
		ret = fai_write(fai_file, src_interface, src_devpart, dest_dev, &fai_hd);
		if (ret < 0) {
			printf("Can't not write fai file\n");
			return CMD_RET_FAILURE;
		}
	}

	return CMD_RET_SUCCESS;
}

static int do_faitools_info(struct cmd_tbl *cmdtp, int flag,
		int argc, char * const argv[])
{
	char *src_interface;
	char *src_devpart;
	fai_header_t fai_hd;
	char *fai_file;
	int ret = 0;

	if (argc < 4)
		return CMD_RET_USAGE;

	src_interface = argv[1];

	if (!strcmp(src_interface, "usb")) {
		src_devpart = argv[2];
		fai_file = argv[3];

		ret = parse_fai_header(fai_file, src_interface, src_devpart, &fai_hd);
		if (ret < 0) {
			printf("Can't not parse fai header\n");
			return CMD_RET_FAILURE;
		}
		ret = fai_info(fai_file, src_interface, src_devpart, &fai_hd);
		if (ret < 0) {
			printf("Can't not write fai file\n");
			return CMD_RET_FAILURE;
		}
	}

	return CMD_RET_SUCCESS;
}

static struct cmd_tbl cmd_faitools[] = {
	U_BOOT_CMD_MKENT(write, 7, 0, do_faitools_write, "", ""),
	U_BOOT_CMD_MKENT(info, 5, 0, do_faitools_info, "", ""),
};

static int do_faitoolsops(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_faitools, ARRAY_SIZE(cmd_faitools));

	/* Drop the faitools command */
	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	faitools, 9, 1, do_faitoolsops,
	"faitools",
	"write <src_interface> <src_dev[:part]> <filename> <dest_interface> <dest_dev>\n"
	"         info <src_interface> <src_dev[:part]> <filename>"
);
