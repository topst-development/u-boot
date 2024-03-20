// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#define pr_fmt(fmt) "blkread: " fmt

#include <common.h>
#include <blk.h>
#include <command.h>
#include <memalign.h>
#include <linux/types.h>
#include <compiler.h>
#include <part.h>
#include <image.h>
#include <linux/io.h>
#include <linux/libfdt_env.h>
#include <fdt.h>
#include <div64.h>

#include <android_image.h>

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
				 struct blk_desc *desc, disk_partition_t *info)
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

static enum command_ret_t sub_do_common(bool arg_chk, char *const argv[],
					struct blk_desc **desc,
					disk_partition_t *info, s32 *part)
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

static int sub_do_dev(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
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

static int sub_do_img(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	enum command_ret_t ret = CMD_RET_SUCCESS;
	bool arg_chk = (argc == cmdtp->maxargs);

	struct blk_desc *desc;
	disk_partition_t info;
	s32 part;
	ulong addr;

	ret = sub_do_common(arg_chk, argv, &desc, &info, &part);

	if (ret == CMD_RET_SUCCESS) {
		s32 err = strict_strtoul(argv[3], 16, &addr);

		if (err < 0) {
			pr_err("Invalid address given\n");
			ret = CMD_RET_FAILURE;
		}
	}

	if (ret == CMD_RET_SUCCESS) {
		long size = android_image_load(desc, &info, addr, ULONG_MAX);

		if (size < 0) {
			pr_err("Failed to load boot image\n");
			ret = CMD_RET_FAILURE;
		}
	}

	return (s32)ret;
}

static int sub_do_part(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	enum command_ret_t ret = CMD_RET_SUCCESS;
	bool arg_chk = ((cmdtp->maxargs > 0) && (argc >= (cmdtp->maxargs - 1)));

	struct blk_desc *desc;
	disk_partition_t info;
	s32 part;
	ulong addr;

	ret = sub_do_common(arg_chk, argv, &desc, &info, &part);

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

static int sub_do_info(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	enum command_ret_t ret = CMD_RET_SUCCESS;
	bool arg_chk = (argc == cmdtp->maxargs);

	struct blk_desc *desc;
	disk_partition_t info;
	s32 part;

	ret = sub_do_common(arg_chk, argv, &desc, &info, &part);

	if ((ret == CMD_RET_SUCCESS) && (desc->devnum >= 0) && (part >= 0)) {
		(void)env_set_ulong(argv[3], (ulong)desc->devnum);
		(void)env_set_ulong(argv[4], (ulong)part);
	}

	return (s32)ret;
}

U_BOOT_CMD_WITH_SUBCMDS(
	blkread, "read from block device",

	/* help message for command usage */
	"[dev|kernel|part|info] ...\n\n"

	"blkread dev [<interface> <dev>]\n"
	"    - show or set current device interface\n"

	"blkread image <part> <slot> <addr>\n"
	"    - read a boot image from the partition `part' in `slot' to\n"
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
	U_BOOT_SUBCMD_MKENT(part, 5, 0, sub_do_part),
	U_BOOT_SUBCMD_MKENT(info, 5, 0, sub_do_info),
);
