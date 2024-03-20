// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <tee.h>
#include <linux/libfdt.h>
#include <linux/io.h>

#define TA_RPBM_UUID { 0x8ffd1621, 0x1f38, 0x4a51, { \
		      0xbb, 0x93, 0x49, 0xdc, 0xa9, 0xcd, 0x6a, 0x07 } }

/* Erase whole RPMB area */
#define PTA_RPMB_ERASE_FS	0

/* Print list of RPMB FS */
#define PTA_RPMB_LIST_FS	1

/* Print list of RPMB FS */
#define PTA_RPMB_ZEROKEY	2

/* RPMB file name */
#define PTA_RPMB_NAME_LENGTH	8	/* A/0001, B/0001, B/0002, ... */

#define PTA_RPMB_MEMDUMP_SIZE	0x1000

struct rpmb_fs_info {
	u32 size;
	u32 flags;
	char name[PTA_RPMB_NAME_LENGTH];
};

static int optee_rpmb(char const *cmd)
{
	struct udevice *tee = NULL;
	const struct tee_optee_ta_uuid uuid = TA_RPBM_UUID;
	struct tee_open_session_arg open_arg;
	struct tee_invoke_arg invoke_arg;
	struct tee_param param;
	struct rpmb_fs_info *fs = NULL;
	int rc, param_num, fs_num;
	struct tee_shm *shm_buf = NULL;

	tee = tee_find_device(tee, NULL, NULL, NULL);
	if (!tee) {
		pr_err("Cannot found tee driver\n");
		goto err;
	}

	if (!strcmp(cmd, "erase")) {
		invoke_arg.func = PTA_RPMB_ERASE_FS;
		param_num = 0;
	} else if (!strcmp(cmd, "print")) {
		invoke_arg.func = PTA_RPMB_LIST_FS;
		param_num = 1;
		memset((void *)&param, 0, sizeof(struct tee_param));

		if (tee_shm_alloc(tee, PTA_RPMB_MEMDUMP_SIZE,
				  TEE_SHM_ALLOC, &shm_buf))
			goto err;
		param.attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
		param.u.memref.shm = shm_buf;
		param.u.memref.size = PTA_RPMB_MEMDUMP_SIZE;
	} else if (!strcmp(cmd, "zerokey")) {
		invoke_arg.func = PTA_RPMB_ZEROKEY;
		param_num = 0;
	} else {
		goto err;
	}

	memset(&open_arg, 0, sizeof(open_arg));
	tee_optee_ta_uuid_to_octets(open_arg.uuid, &uuid);
	rc = tee_open_session(tee, &open_arg, 0, NULL);
	if (rc) {
		pr_err("Cannot open TA\n");
		goto err;
	}

	invoke_arg.session = open_arg.session;
	if (tee_invoke_func(tee, &invoke_arg, param_num, &param)) {
		pr_err("Failed to call invoke command\n");
		goto err_with_close;
	}
	if (invoke_arg.ret) {
		pr_err("Failed to invoke cmd:0x%x, ret:0x%x\n",
		       invoke_arg.func, invoke_arg.ret);
		goto err_with_close;
	}

	if (invoke_arg.func == PTA_RPMB_ERASE_FS) {
		pr_force("RPMB partition erased\n");
	} else if (invoke_arg.func == PTA_RPMB_LIST_FS) {
		if (param.u.memref.size >= sizeof(struct rpmb_fs_info)) {
			fs = (struct rpmb_fs_info *)
				(uintptr_t)param.u.memref.shm->addr;
			fs_num = param.u.memref.size /
				sizeof(struct rpmb_fs_info);
			while (fs_num) {
				pr_force("flags: 0x%02x++0x%04x, name '%s'\n",
					 fs->flags, fs->size, fs->name);
				fs++;
				fs_num--;
			}
		} else {
			pr_force("--- No Data ---\n");
		}
		tee_shm_free(shm_buf);
	} else if (invoke_arg.func == PTA_RPMB_ZEROKEY) {
		pr_force("Done.\n");
	}

	tee_close_session(tee, open_arg.session);
	return 0;

err_with_close:
	tee_close_session(tee, open_arg.session);
err:
	if (shm_buf)
		tee_shm_free(shm_buf);

	if (fs)
		free(fs);
	fs = NULL;
	return CMD_RET_FAILURE;
}

static int optee_version(void)
{
	return 0;
}

static int do_optee(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *cmd;

	if (!working_fdt) {
		pr_warn("Please set fdt before\n");
		//fdt_parse_optee(working_fdt);
		return CMD_RET_FAILURE;
	}

	if (argc < 2)
		return CMD_RET_USAGE;

	cmd = argv[1];
	if (!strcmp(cmd, "rpmb") && argc >= 3)
		return optee_rpmb(argv[2]);
	else if (!strcmp(cmd, "version") && (argc >= 2))
		return optee_version();

	return CMD_RET_USAGE;
}

U_BOOT_CMD(optee, 3, 0, do_optee,
	   "OPTEE management in bootloader",
	   "optee <func> <cmd>\n"
	   "      rpmb <cmd>\n"
	   "           print   - print RPMB flie lists.\n"
	   "           zerokey - derivate secure key by zero.\n"
	   "      version      - pirnt optee version"
);
