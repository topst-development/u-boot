// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 */

#include <env.h>
#include <image-android-dt.h>
#include <common.h>

enum cmd_dtimg_info {
	CMD_DTIMG_START = 0,
	CMD_DTIMG_SIZE,
};

static int do_dtimg_dump(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	char *endp;
	ulong hdr_addr;

	if (argc != 2)
		return CMD_RET_USAGE;

	hdr_addr = simple_strtoul(argv[1], &endp, 16);
	if (*endp != '\0') {
		printf("Error: Wrong image address\n");
		return CMD_RET_FAILURE;
	}

	if (!android_dt_check_header(hdr_addr)) {
		printf("Error: DT image header is incorrect\n");
		return CMD_RET_FAILURE;
	}

	android_dt_print_contents(hdr_addr);

	return CMD_RET_SUCCESS;
}

static int do_dtimg_load(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	char *endp;
	ulong addr;
	struct blk_desc *dev_desc;
	disk_partition_t part_info;
	char part_name[PART_NAME_LEN];
	int ret;

	if (argc != 5)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[4], &endp, 16);
	if (*endp != '\0') {
		printf("Error: Wrong address\n");
		return CMD_RET_FAILURE;
	}

	if (strcmp(argv[3], "-")) {
		char slot[3];

		slot[0] = '_';
		slot[1] = argv[3][0];
		slot[2] = '\0';
		strncpy(part_name, argv[2], sizeof(part_name) - 2);
		strcat(part_name, slot);
	} else {
		strncpy(part_name, argv[2], sizeof(part_name));
	}

	if (part_get_info_by_dev_and_name_or_num(argv[1], part_name,
						 &dev_desc, &part_info) < 0)
		return CMD_RET_FAILURE;

	ret = android_dt_load(dev_desc, &part_info, addr);
	if (ret < 0) {
		printf("Error loading dtb/dtbo in Android image\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_dtimg_index(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	ulong hdr_addr;
	u32 id, rev;
	int index;
	char *endp;
	char buf[32];

	if (argc != 5)
		return CMD_RET_USAGE;

	hdr_addr = simple_strtoul(argv[1], &endp, 16);
	id = simple_strtoul(argv[2], &endp, 16);
	rev = simple_strtoul(argv[3], &endp, 16);

	index = android_dt_get_index(hdr_addr, id, rev);
	if (index < 0)
		return CMD_RET_FAILURE;

	snprintf(buf, sizeof(buf), "%d", index);
	env_set(argv[4], buf);

	return CMD_RET_SUCCESS;
}

static int dtimg_get_fdt(int argc, char * const argv[], enum cmd_dtimg_info cmd)
{
	ulong hdr_addr;
	u32 index;
	char *endp;
	ulong fdt_addr;
	u32 fdt_size;
	char buf[65];

	if (argc != 4)
		return CMD_RET_USAGE;

	hdr_addr = simple_strtoul(argv[1], &endp, 16);
	if (*endp != '\0') {
		printf("Error: Wrong image address\n");
		return CMD_RET_FAILURE;
	}

	if (!android_dt_check_header(hdr_addr)) {
		printf("Error: DT image header is incorrect\n");
		return CMD_RET_FAILURE;
	}

	index = simple_strtoul(argv[2], &endp, 0);
	if (*endp != '\0') {
		printf("Error: Wrong index\n");
		return CMD_RET_FAILURE;
	}

	if (!android_dt_get_fdt_by_index(hdr_addr, index, &fdt_addr, &fdt_size))
		return CMD_RET_FAILURE;

	switch (cmd) {
	case CMD_DTIMG_START:
		snprintf(buf, sizeof(buf), "%lx", fdt_addr);
		break;
	case CMD_DTIMG_SIZE:
		snprintf(buf, sizeof(buf), "%x", fdt_size);
		break;
	default:
		printf("Error: Unknown cmd_dtimg_info value: %d\n", cmd);
		return CMD_RET_FAILURE;
	}

	env_set(argv[3], buf);

	return CMD_RET_SUCCESS;
}

static int do_dtimg_start(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	return dtimg_get_fdt(argc, argv, CMD_DTIMG_START);
}

static int do_dtimg_size(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	return dtimg_get_fdt(argc, argv, CMD_DTIMG_SIZE);
}

static cmd_tbl_t cmd_dtimg_sub[] = {
	U_BOOT_CMD_MKENT(dump, 2, 0, do_dtimg_dump, "", ""),
	U_BOOT_CMD_MKENT(load, 5, 0, do_dtimg_load, "", ""),
	U_BOOT_CMD_MKENT(index, 5, 0, do_dtimg_index, "", ""),
	U_BOOT_CMD_MKENT(start, 4, 0, do_dtimg_start, "", ""),
	U_BOOT_CMD_MKENT(size, 4, 0, do_dtimg_size, "", ""),
};

static int do_dtimg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *cp;

	cp = find_cmd_tbl(argv[1], cmd_dtimg_sub, ARRAY_SIZE(cmd_dtimg_sub));

	/* Strip off leading 'dtimg' command argument */
	argc--;
	argv++;

	if (!cp || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(dtimg, CONFIG_SYS_MAXARGS, 0, do_dtimg,
	   "manipulate dtb/dtbo Android image",
	   "dump <addr>\n"
	   "    - parse specified image and print its structure info\n"
	   "      <addr>: image address in RAM, in hex\n"
	   "dtimg load <interface> <dev[:part|#part_name]> <slot> <addr>\n"
	   "    - load dtb/dtbo in Android image from the partition 'part' on\n"
	   "    - device type 'interface' instance 'dev' to address 'addr'.\n"
	   "dtimg index <addr> <id> <rev> <varname>\n"
	   "    - get decimal index of FDT in the image\n"
	   "      <addr>: image address in RAM, in hex\n"
	   "      <id>: id of the board, in hex\n"
	   "      <rev>: revision of the board, in hex\n"
	   "      <varname>: name of variable where to store index of FDT\n"
	   "dtimg start <addr> <index> <varname>\n"
	   "    - get address (hex) of FDT in the image, by index\n"
	   "      <addr>: image address in RAM, in hex\n"
	   "      <index>: index of desired FDT in the image\n"
	   "      <varname>: name of variable where to store address of FDT\n"
	   "dtimg size <addr> <index> <varname>\n"
	   "    - get size (hex, bytes) of FDT in the image, by index\n"
	   "      <addr>: image address in RAM, in hex\n"
	   "      <index>: index of desired FDT in the image\n"
	   "      <varname>: name of variable where to store size of FDT"
);
