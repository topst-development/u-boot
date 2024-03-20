// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Eugeniu Rosca <rosca.eugeniu@gmail.com>
 * Copyright (C) 2020 Telechips Inc.
 *
 * Command to read/modify/write Android BCB fields
 */

#include <android_bootloader_message.h>
#include <command.h>
#include <common.h>

enum bcb_cmd {
	BCB_CMD_LOAD,
	BCB_CMD_FIELD_SET,
	BCB_CMD_FIELD_CLEAR,
	BCB_CMD_FIELD_TEST,
	BCB_CMD_FIELD_DUMP,
	BCB_CMD_STORE,
};

static int bcb_dev = -1;
static int bcb_part = -1;
static struct bootloader_message bcb __aligned(4096) = { { 0 } };

static int bcb_cmd_get(char *cmd)
{
	if (!strcmp(cmd, "load"))
		return BCB_CMD_LOAD;
	if (!strcmp(cmd, "set"))
		return BCB_CMD_FIELD_SET;
	if (!strcmp(cmd, "clear"))
		return BCB_CMD_FIELD_CLEAR;
	if (!strcmp(cmd, "test"))
		return BCB_CMD_FIELD_TEST;
	if (!strcmp(cmd, "store"))
		return BCB_CMD_STORE;
	if (!strcmp(cmd, "dump"))
		return BCB_CMD_FIELD_DUMP;
	else
		return -1;
}

static int bcb_is_misused(int argc, char *const argv[])
{
	int cmd = bcb_cmd_get(argv[0]);

	switch (cmd) {
	case BCB_CMD_LOAD:
	case BCB_CMD_FIELD_SET:
		if (argc != 3)
			goto err;
		break;
	case BCB_CMD_FIELD_TEST:
		if (argc != 4)
			goto err;
		break;
	case BCB_CMD_FIELD_CLEAR:
		if (argc != 1 && argc != 2)
			goto err;
		break;
	case BCB_CMD_STORE:
		if (argc != 1)
			goto err;
		break;
	case BCB_CMD_FIELD_DUMP:
		if (argc != 2)
			goto err;
		break;
	default:
		printf("Error: 'bcb %s' not supported\n", argv[0]);
		return -1;
	}

	if (cmd != BCB_CMD_LOAD && (bcb_dev < 0 || bcb_part < 0)) {
		printf("Error: Please, load BCB first!\n");
		return -1;
	}

	return 0;
err:
	printf("Error: Bad usage of 'bcb %s'\n", argv[0]);

	return -1;
}

static int bcb_field_get(char *name, char **fieldp, int *sizep)
{
	if (!strcmp(name, "command")) {
		*fieldp = bcb.command;
		*sizep = sizeof(bcb.command);
	} else if (!strcmp(name, "status")) {
		*fieldp = bcb.status;
		*sizep = sizeof(bcb.status);
	} else if (!strcmp(name, "recovery")) {
		*fieldp = bcb.recovery;
		*sizep = sizeof(bcb.recovery);
	} else if (!strcmp(name, "stage")) {
		*fieldp = bcb.stage;
		*sizep = sizeof(bcb.stage);
	} else if (!strcmp(name, "reserved")) {
		*fieldp = bcb.reserved;
		*sizep = sizeof(bcb.reserved);
	} else {
		printf("Error: Unknown bcb field '%s'\n", name);
		return -1;
	}

	return 0;
}

static int do_bcb_load(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	struct blk_desc *desc;
	disk_partition_t info;
	u64 cnt;
	int part, ret;

	if (argc != 3)
		return CMD_RET_USAGE;

	part = part_get_info_by_dev_and_name_or_num(argv[1], argv[2], &desc,
						    &info);
	if (part < 0)
		return CMD_RET_FAILURE;

	cnt = DIV_ROUND_UP(sizeof(struct bootloader_message), info.blksz);
	if (cnt > info.size)
		goto err_too_small;

	if (blk_dread(desc, info.start, cnt, &bcb) != cnt) {
		ret = -EIO;
		goto err_read_fail;
	}

	bcb_dev = desc->devnum;
	bcb_part = part;
	debug("%s: Loaded from mmc %d:%d\n", __func__, bcb_dev, bcb_part);

	return CMD_RET_SUCCESS;
err_read_fail:
	printf("Error: %s %s read failed (%d)\n", argv[1], argv[2], ret);
	goto err;
err_too_small:
	printf("Error: %s %s too small!", argv[1], argv[2]);
	goto err;
err:
	bcb_dev = -1;
	bcb_part = -1;

	return CMD_RET_FAILURE;
}

static int do_bcb_set(cmd_tbl_t *cmdtp, int flag, int argc,
		      char * const argv[])
{
	int size, len;
	char *field, *str, *found;

	if (bcb_field_get(argv[1], &field, &size))
		return CMD_RET_FAILURE;

	len = strlen(argv[2]);
	if (len >= size) {
		printf("Error: sizeof('%s') = %d >= %d = sizeof(bcb.%s)\n",
		       argv[2], len, size, argv[1]);
		return CMD_RET_FAILURE;
	}
	str = argv[2];

	field[0] = '\0';
	while ((found = strsep(&str, ":"))) {
		if (field[0] != '\0')
			strcat(field, "\n");
		strcat(field, found);
	}

	return CMD_RET_SUCCESS;
}

static int do_bcb_clear(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int size;
	char *field;

	if (argc == 1) {
		memset(&bcb, 0, sizeof(bcb));
		return CMD_RET_SUCCESS;
	}

	if (bcb_field_get(argv[1], &field, &size))
		return CMD_RET_FAILURE;

	memset(field, 0, size);

	return CMD_RET_SUCCESS;
}

static int do_bcb_test(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	int size;
	char *field;
	char *op = argv[2];

	if (bcb_field_get(argv[1], &field, &size))
		return CMD_RET_FAILURE;

	if (*op == '=' && *(op + 1) == '\0') {
		if (!strncmp(argv[3], field, size))
			return CMD_RET_SUCCESS;
		else
			return CMD_RET_FAILURE;
	} else if (*op == '~' && *(op + 1) == '\0') {
		if (!strstr(field, argv[3]))
			return CMD_RET_FAILURE;
		else
			return CMD_RET_SUCCESS;
	} else {
		printf("Error: Unknown operator '%s'\n", op);
	}

	return CMD_RET_FAILURE;
}

static int do_bcb_dump(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	int size;
	char *field;

	if (bcb_field_get(argv[1], &field, &size))
		return CMD_RET_FAILURE;

	print_buffer((ulong)field - (ulong)&bcb, (void *)field, 1, size, 16);

	return CMD_RET_SUCCESS;
}

static int do_bcb_store(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	struct blk_desc *desc;
	disk_partition_t info;
	u64 cnt;
	int ret;

#if defined(CONFIG_UFS_BOOT)
	desc = blk_get_devnum_by_type(IF_TYPE_SCSI, bcb_dev);
#else
	desc = blk_get_devnum_by_type(IF_TYPE_MMC, bcb_dev);
#endif
	if (!desc) {
		ret = -ENODEV;
		goto err;
	}

	ret = part_get_info(desc, bcb_part, &info);
	if (ret)
		goto err;

	cnt = DIV_ROUND_UP(sizeof(struct bootloader_message), info.blksz);

	if (blk_dwrite(desc, info.start, cnt, &bcb) != cnt) {
		ret = -EIO;
		goto err;
	}

	return CMD_RET_SUCCESS;
err:
#if defined(CONFIG_UFS_BOOT)
	printf("Error: ufs %d:%d write failed (%d)\n", bcb_dev, bcb_part, ret);
#else
	printf("Error: mmc %d:%d write failed (%d)\n", bcb_dev, bcb_part, ret);
#endif
	return CMD_RET_FAILURE;
}

static cmd_tbl_t cmd_bcb_sub[] = {
	U_BOOT_CMD_MKENT(load, CONFIG_SYS_MAXARGS, 1, do_bcb_load, "", ""),
	U_BOOT_CMD_MKENT(set, CONFIG_SYS_MAXARGS, 1, do_bcb_set, "", ""),
	U_BOOT_CMD_MKENT(clear, CONFIG_SYS_MAXARGS, 1, do_bcb_clear, "", ""),
	U_BOOT_CMD_MKENT(test, CONFIG_SYS_MAXARGS, 1, do_bcb_test, "", ""),
	U_BOOT_CMD_MKENT(dump, CONFIG_SYS_MAXARGS, 1, do_bcb_dump, "", ""),
	U_BOOT_CMD_MKENT(store, CONFIG_SYS_MAXARGS, 1, do_bcb_store, "", ""),
};

static int do_bcb(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_bcb_sub, ARRAY_SIZE(cmd_bcb_sub));
	if (!c)
		return CMD_RET_USAGE;

	if (bcb_is_misused(argc, argv)) {
		/*
		 * We try to improve the user experience by reporting the
		 * root-cause of misusage, so don't return CMD_RET_USAGE,
		 * since the latter prints out the full-blown help text
		 */
		return CMD_RET_FAILURE;
	}

	return c->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(bcb, CONFIG_SYS_MAXARGS, 1, do_bcb,
	   "Load/set/clear/test/dump/store Android BCB fields",
	   "load <interface> <dev[:part|#part_name]\n"
	   "    - load BCB from the partition 'part' on device type 'interface'\n"
	   "      instance 'dev'.\n"
	   "bcb set <field> <val>\n"
	   "    - set BCB <field> to <val>\n"
	   "bcb clear [<field>]\n"
	   "    - clear BCB <field> or all fields\n"
	   "bcb test  <field> <op> <val>\n"
	   "    - test  BCB <field> against <val>\n"
	   "bcb dump  <field>\n"
	   "    - dump  BCB <field>\n"
	   "bcb store\n"
	   "    - store BCB back\n"
	   "\n"
	   "Legend:\n"
	   "<interface> - device type\n"
	   "<dev>       - device index containing the BCB partition\n"
	   "<part>      - partition index or name containing the BCB\n"
	   "<field>     - one of {command,status,recovery,stage,reserved}\n"
	   "<op>        - the binary operator used in 'bcb test':\n"
	   "              '=' returns true if <val> matches the string stored in <field>\n"
	   "              '~' returns true if <val> matches a subset of <field>'s string\n"
	   "<val>       - string/text provided as input to bcb {set,test}\n"
	   "              NOTE: any ':' character in <val> will be replaced by line feed\n"
	   "              during 'bcb set' and used as separator by upper layers\n"
);
