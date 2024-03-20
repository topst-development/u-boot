// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2017 The Android Open Source Project
 * Copyright (C) 2019, 2020 Telechips Inc.
 */

#include <ab_update.h>
#include <command.h>

static int do_ab_select(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret;
	struct blk_desc *dev_desc;
	disk_partition_t part_info;
	char slot[2];

	if (argc != 4)
		return CMD_RET_USAGE;

	/* Lookup the "misc" partition from argv[2] and argv[3] */
	if (part_get_info_by_dev_and_name_or_num(argv[2], argv[3],
						 &dev_desc, &part_info) < 0) {
		return CMD_RET_FAILURE;
	}

#if defined(CONFIG_BL3_AB_UPDATE)
    ret = ab_select_slot(dev_desc, &part_info, false);
#else
	ret = ab_select_slot(dev_desc, &part_info, true);
#endif

	if (ret < 0) {
		printf("AB: failed to select slot, error %d.\n", ret);
		return CMD_RET_FAILURE;
	}

	/* Slot names are 'a', 'b', ... */
	slot[0] = BOOT_SLOT_NAME(ret);
	slot[1] = '\0';
	env_set(argv[1], slot);
	printf("AB: Booting slot: %s\n", slot);
	return CMD_RET_SUCCESS;
}

static int do_ab_set(cmd_tbl_t *cmdtp, int flag, int argc,
		     char * const argv[])
{
	struct blk_desc *dev_desc;
	disk_partition_t part_info;
	int slot;
	int ret;

	if (argc != 4)
		return CMD_RET_USAGE;

	if (part_get_info_by_dev_and_name_or_num(argv[2], argv[3],
						 &dev_desc, &part_info) < 0)
		return CMD_RET_FAILURE;

	slot = *argv[1] - 'a';
	ret = ab_set_active_slot(dev_desc, &part_info, slot);
	if (ret < 0) {
		printf("Couldn't set active slot\n");
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

static cmd_tbl_t cmd_ab[] = {
	U_BOOT_CMD_MKENT(select, 4, 1, do_ab_select, "", ""),
	U_BOOT_CMD_MKENT(set, 4, 1, do_ab_set, "", ""),
};

static int do_ab(cmd_tbl_t *cmdtp, int flag, int argc,
		 char * const argv[])
{
	cmd_tbl_t *cp;

	cp = find_cmd_tbl(argv[1], cmd_ab, ARRAY_SIZE(cmd_ab));

	argc--;
	argv++;

	if (!cp || argc > cp->maxargs)
		return CMD_RET_USAGE;

	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	ab,	CONFIG_SYS_MAXARGS,	0,	do_ab,
	"manipulate AB slots",
	"select <slot_var_name> <interface> <dev[:part|#part_name]>\n"
	"    - Select the slot used to boot from and register the boot attempt.\n"
	"    - Load the slot metadata from the partition 'part' on\n"
	"      device type 'interface' instance 'dev' and store the active\n"
	"      slot in the 'slot_var_name' variable. This also updates the\n"
	"      Android slot metadata with a boot attempt, which can cause\n"
	"      successive calls to this function to return a different result\n"
	"      if the returned slot runs out of boot attempts.\n"
	"    - If 'part_name' is passed, preceded with a # instead of :, the\n"
	"      partition name whose label is 'part_name' will be looked up in\n"
	"      the partition table. This is commonly the \"misc\" partition.\n"
	"ab set <slot_name> <interface> <dev:[part|#part_name]>\n"
	"    - Set the active slot to the 'slot_name' from the partition 'part'\n"
	"      on device type 'interface' instance 'dev'."
);
