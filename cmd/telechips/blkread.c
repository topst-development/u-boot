// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#define pr_fmt(fmt) "blkread: " fmt

#include <common.h>
#include <android_image.h>
#include <image.h>
#if CONFIG_IS_ENABLED(TCC_BOOT_IMAGE)
#include <image-tcc.h>
#endif
#include <memalign.h>
#include <part.h>
#include <linux/io.h>

#define set_dev_desc(i, d) fn_dev_desc((i), (d))
#define get_dev_desc() fn_dev_desc(NULL, NULL)

static struct blk_desc *fn_dev_desc(const char *ifname, const char *dev_str)
{
	static struct blk_desc *desc;

	if ((ifname != NULL) && (dev_str != NULL)) {
		s32 ret = blk_get_device_by_str(ifname, dev_str, &desc);

		if (ret < 0) {
			pr_err("Failed to set current device\n");
		}
	}

	return desc;
}

static enum command_ret_t print_dev_desc(const struct blk_desc *desc)
{
	enum command_ret_t ret = CMD_RET_SUCCESS;

	if (desc != NULL) {
		const char *iface = blk_get_if_type_name(desc->if_type);
		s32 dev = desc->devnum;

		pr_force("%s %d is current device\n", iface, dev);
		ret = CMD_RET_SUCCESS;
	} else {
		pr_err("No current device is set\n");
		ret = CMD_RET_FAILURE;
	}

	return ret;
}

static s32 get_part_by_name_slot(const char *name, const char *slot,
				 struct blk_desc *desc,
				 struct disk_partition *info)
{
	size_t slot_len = strnlen(slot, 2);
	const char *part = name;
	char partbuf[PART_NAME_LEN];

	if ((slot_len == 1U) && ((u8)slot[0] != (u8)'-')) {
		(void)scnprintf(partbuf, sizeof(partbuf), "%s_%s", name, slot);
		part = partbuf;
	}

	return part_get_info_by_name(desc, part, info);
}

static enum command_ret_t get_part_from_args(bool arg_chk, char *const argv[],
					     struct blk_desc **desc,
					     struct disk_partition *info,
					     s32 *part)
{
	enum command_ret_t ret = CMD_RET_SUCCESS;

	*desc = get_dev_desc();
	if (*desc == NULL) {
		ret = print_dev_desc(*desc);
	}

	if ((ret == CMD_RET_SUCCESS) && !arg_chk) {
		ret = CMD_RET_USAGE;
	}

	if (ret == CMD_RET_SUCCESS) {
		*part = get_part_by_name_slot(argv[1], argv[2], *desc, info);

		if (*part < 0) {
			pr_err("Failed to get partition info\n");
			ret = CMD_RET_FAILURE;
		}
	}

	return ret;
}

/* Copy of android_image_check_header() in common/image-android.c */
static int android_image_check_header_v0(const struct andr_img_hdr *hdr)
{
	return memcmp(ANDR_BOOT_MAGIC, hdr->magic, ANDR_BOOT_MAGIC_SIZE);
}

static void load_img_hdr(struct blk_desc *desc, lbaint_t start, void *dest,
			 lbaint_t *blkcnt)
{
	size_t hdr_size;
	lbaint_t hdrcnt;
	ulong read;
	u32 size = 0;
	const size_t andr_img_hdr_size = sizeof(struct andr_img_hdr);
#if CONFIG_IS_ENABLED(TCC_BOOT_IMAGE)
	const size_t tc_img_hdr_size = sizeof(struct tc_img_hdr);
#endif

	hdr_size = andr_img_hdr_size;
#if CONFIG_IS_ENABLED(TCC_BOOT_IMAGE)
	if (tc_img_hdr_size > andr_img_hdr_size) {
		hdr_size = tc_img_hdr_size;
	}
#endif

	hdrcnt = BLOCK_CNT(hdr_size, desc);
	read = blk_dread(desc, start, hdrcnt, dest);

	if (read == hdrcnt) {
		s32 is_andr_img = android_image_check_header_v0(dest);
#if CONFIG_IS_ENABLED(TCC_BOOT_IMAGE)
		s32 is_tc_img = tc_image_check_header(dest);
#endif
		if (is_andr_img == 0) {
			struct andr_img_hdr *hdr = dest;

			size += ALIGN(andr_img_hdr_size, hdr->page_size);
			size += ALIGN(hdr->kernel_size, hdr->page_size);
#if CONFIG_IS_ENABLED(TCC_BOOT_IMAGE)
		} else if (is_tc_img == 0) {
			struct tc_img_hdr *hdr = dest;

			size += ALIGN(tc_img_hdr_size, desc->blksz);
			size += ALIGN(hdr->img_size, desc->blksz);
#endif
		} else {
			pr_err("Invalid boot image header\n");
		}
	} else {
		pr_err("Failed to load boot image header\n");
	}

	if (blkcnt != NULL) {
		*blkcnt = (size == 0U) ? 0U : BLOCK_CNT(size, desc);
	}
}

static void load_fdt_hdr(struct blk_desc *desc, lbaint_t start, void *dest,
			 lbaint_t *blkcnt, lbaint_t *blkoff)
{
	size_t hdr_size;
	lbaint_t hdrcnt;
	ulong read;
	u32 size = 0;
	u32 offs = 0;
	const size_t andr_img_hdr_size = sizeof(struct andr_img_hdr);
	const size_t fdt_hdr_size = sizeof(struct fdt_header);

	if (fdt_hdr_size < andr_img_hdr_size) {
		hdr_size = andr_img_hdr_size;
	} else {
		hdr_size = fdt_hdr_size;
	}

	hdrcnt = BLOCK_CNT(hdr_size, desc);
	read = blk_dread(desc, start, hdrcnt, dest);

	if (read == hdrcnt) {
		s32 is_andr_img = android_image_check_header_v0(dest);
		s32 is_fdt = fdt_check_header(dest);

		if (is_andr_img == 0) {
			struct andr_img_hdr *hdr = dest;

			size += ALIGN(hdr->dtb_size, hdr->page_size);
			offs += ALIGN(andr_img_hdr_size, hdr->page_size);
			offs += ALIGN(hdr->kernel_size, hdr->page_size);
			offs += ALIGN(hdr->ramdisk_size, hdr->page_size);
			offs += ALIGN(hdr->second_size, hdr->page_size);
			offs += ALIGN(hdr->recovery_dtbo_size, hdr->page_size);
		} else if (is_fdt == 0) {
			struct fdt_header *hdr = dest;

			size = fdt_totalsize(hdr);
		} else {
			pr_err("Invalid fdt header\n");
		}
	} else {
		pr_err("Failed to load fdt header\n");
	}

	if (blkcnt != NULL) {
		*blkcnt = (size == 0U) ? 0U : BLOCK_CNT(size, desc);
	}

	if (blkoff != NULL) {
		*blkoff = (offs == 0U) ? 0U : BLOCK_CNT(offs, desc);
	}
}

static int sub_do_dev(struct cmd_tbl *cmd, int flags, int argc,
		      char *const argv[])
{
	enum command_ret_t ret;
	const struct blk_desc *desc;

	switch (argc) {
	case 3:
		desc = set_dev_desc(argv[1], argv[2]);
		ret = print_dev_desc(desc);
		break;
	case 1:
		desc = get_dev_desc();
		ret = print_dev_desc(desc);
		break;
	default:
		ret = CMD_RET_USAGE;
		break;
	}

	return (s32)ret;
}

static int sub_do_img(struct cmd_tbl *cmd, int flags, int argc,
		      char *const argv[])
{
	enum command_ret_t ret = CMD_RET_SUCCESS;
	bool arg_chk = (argc == cmd->maxargs);

	struct blk_desc *desc;
	struct disk_partition info;
	s32 part;
	ulong addr;

	ret = get_part_from_args(arg_chk, argv, &desc, &info, &part);

	if (ret == CMD_RET_SUCCESS) {
		s32 err = strict_strtoul(argv[3], 16, &addr);

		if (err < 0) {
			pr_err("Invalid address given\n");
			ret = CMD_RET_FAILURE;
		}
	}

	if (ret == CMD_RET_SUCCESS) {
		lbaint_t blkcnt;
		void *dest = phys_to_virt(addr);

		load_img_hdr(desc, info.start, dest, &blkcnt);

		if (blkcnt == 0U) {
			ret = CMD_RET_FAILURE;
		} else {
			ulong read;

			read = blk_dread(desc, info.start, blkcnt, dest);

			pr_info("Load boot image (%lu blocks)\n", blkcnt);

			if (read != blkcnt) {
				pr_err("Failed to load boot image\n");
				ret = CMD_RET_FAILURE;
			}
		}
	}

	return (s32)ret;
}

static int sub_do_fdt(struct cmd_tbl *cmd, int flags, int argc,
		      char *const argv[])
{
	enum command_ret_t ret = CMD_RET_SUCCESS;
	bool arg_chk = (argc == cmd->maxargs);

	struct blk_desc *desc;
	struct disk_partition info;
	s32 part;
	ulong addr;

	ret = get_part_from_args(arg_chk, argv, &desc, &info, &part);

	if (ret == CMD_RET_SUCCESS) {
		s32 err = strict_strtoul(argv[3], 16, &addr);

		if (err < 0) {
			pr_err("Invalid address given\n");
			ret = CMD_RET_FAILURE;
		}
	}

	if (ret == CMD_RET_SUCCESS) {
		lbaint_t blkcnt;
		lbaint_t blkoff;
		void *dest = phys_to_virt(addr);

		load_fdt_hdr(desc, info.start, dest, &blkcnt, &blkoff);

		if (blkcnt == 0U) {
			ret = CMD_RET_FAILURE;
		} else {
			ulong read;

			if (blkoff <= (UINT64_MAX - info.start)) {
				read = blk_dread(desc, info.start + blkoff,
						 blkcnt, dest);

				pr_info("Load fdt (%lu blocks)\n", blkcnt);
			}

			if (read != blkcnt) {
				pr_err("Failed to load fdt\n");
				ret = CMD_RET_FAILURE;
			}
		}
	}

	return (s32)ret;
}

static int sub_do_part(struct cmd_tbl *cmd, int flags, int argc,
		       char *const argv[])
{
	enum command_ret_t ret = CMD_RET_SUCCESS;
	bool arg_chk = ((cmd->maxargs > 0) && (argc >= (cmd->maxargs - 1)));

	struct blk_desc *desc;
	struct disk_partition info;
	s32 part;
	ulong addr;

	if (argc == 5) {
		(void)env_set_hex(argv[4], 0U);
	}

	ret = get_part_from_args(arg_chk, argv, &desc, &info, &part);

	if (ret == CMD_RET_SUCCESS) {
		s32 err = strict_strtoul(argv[3], 16, &addr);

		if (err < 0) {
			pr_err("Invalid address given\n");
			ret = CMD_RET_FAILURE;
		}
	}

	if (ret == CMD_RET_SUCCESS) {
		void *dest = phys_to_virt(addr);
		ulong size = blk_dread(desc, info.start, info.size, dest);

		if (size != info.size) {
			pr_err("Failed to read partition\n");
			ret = CMD_RET_FAILURE;
		}

		if ((ret == CMD_RET_SUCCESS) && (argc == 5)) {
			size *= info.blksz;
			(void)env_set_hex(argv[4], size);
		}
	}

	return (s32)ret;
}

static int sub_do_info(struct cmd_tbl *cmd, int flags, int argc,
		       char *const argv[])
{
	enum command_ret_t ret = CMD_RET_SUCCESS;
	bool arg_chk = (argc == cmd->maxargs);

	struct blk_desc *desc;
	struct disk_partition info;
	s32 part;

	ret = get_part_from_args(arg_chk, argv, &desc, &info, &part);

	if ((ret == CMD_RET_SUCCESS) && (desc->devnum >= 0) && (part >= 0)) {
		(void)env_set_ulong(argv[3], (ulong)desc->devnum);
		(void)env_set_ulong(argv[4], (ulong)part);
	}

	return (s32)ret;
}

U_BOOT_CMD_WITH_SUBCMDS(
	blkread, "read from block device",

	/* help message for command usage */
	"<dev|image|fdt|part|info> ...\n\n"

	"blkread dev [<interface> <dev>]\n"
	"    - show or set current device interface\n"

	"blkread image <part> <slot> <addr>\n"
	"    - read a boot image from the partition `part' in `slot' to\n"
	"      memory address `addr'\n"

	"blkread fdt <part> <slot> <addr>\n"
	"    - read a FDT image from the partition `part' in `slot' to\n"
	"      memory address `addr'\n"

	"blkread part <part> <slot> <addr> [size_var]\n"
	"    - read a partition `part' in `slot' to memory address `addr' and\n"
	"      return loaded amounts (in byte) in `size_var' if given\n"

	"blkread info <part> <slot> <devnum_var> <partnum_var>\n"
	"    - get device number and partition number of a partition `part'\n"
	"      in `slot', and store each in `devnum_var' and `partnum_var'\n",

	/* sub-command entry list */
	U_BOOT_SUBCMD_MKENT(dev, 3, 0, sub_do_dev),
	U_BOOT_SUBCMD_MKENT(image, 4, 0, sub_do_img),
	U_BOOT_SUBCMD_MKENT(fdt, 4, 0, sub_do_fdt),
	U_BOOT_SUBCMD_MKENT(part, 5, 0, sub_do_part),
	U_BOOT_SUBCMD_MKENT(info, 5, 0, sub_do_info),
);
