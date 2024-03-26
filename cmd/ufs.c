// SPDX-License-Identifier: GPL-2.0+
/**
 * ufs.c - UFS specific U-boot commands
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - http://www.ti.com
 *
 */
#include <common.h>
#include <command.h>
#include <ufs.h>
#include <console.h>
#include <scsi.h>

#if defined (CONFIG_CMD_UFS_RPMB)
static int confirm_key_prog(void)
{
	puts("Warning: Programming authentication key can be done only once !\n"
	     "         Use this command only if you are sure of what you are doing,\n"
	     "Really perform the key programming? <y/N> ");
	if (confirm_yesno())
		return 1;

	puts("Authentication key programming aborted\n");
	return 0;
}

static int do_ufsrpmb_key(struct cmd_tbl *cmdtp, int flag,
			  int argc, char * const argv[])
{
	void *key_addr;
	struct udevice *scsi = find_scsi_device(0x3/*WLUN*/);

	if (argc != 2)
		return CMD_RET_USAGE;

	key_addr = (void *)simple_strtoul(argv[1], NULL, 16);
	if (!confirm_key_prog())
		return CMD_RET_FAILURE;
	if (ufs_rpmb_set_key(scsi, key_addr)) {
		printf("ERROR - Key already programmed ?\n");
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

static int do_ufsrpmb_read(struct cmd_tbl *cmdtp, int flag,
			   int argc, char * const argv[])
{
	u16 blk, cnt;
	void *addr;
	int n;
	void *key_addr = NULL;
	struct udevice *scsi = find_scsi_device(0x3/*WLUN*/);

	if (argc < 4)
		return CMD_RET_USAGE;

	addr = (void *)simple_strtoul(argv[1], NULL, 16);
	blk = simple_strtoul(argv[2], NULL, 16);
	cnt = simple_strtoul(argv[3], NULL, 16);

	if (argc == 5)
		key_addr = (void *)simple_strtoul(argv[4], NULL, 16);

	printf("\nUFS RPMB read: dev # %d, block # %d, count %d ... ",
	       0x3, blk, cnt);
	n =  ufs_rpmb_read(scsi, addr, blk, cnt, key_addr);

	printf("%d RPMB blocks read: %s\n", n, (n == cnt) ? "OK" : "ERROR");
	if (n != cnt)
		return CMD_RET_FAILURE;
	return CMD_RET_SUCCESS;
}
static int do_ufsrpmb_write(struct cmd_tbl *cmdtp, int flag,
			    int argc, char * const argv[])
{
	u16 blk, cnt;
	void *addr;
	int n;
	void *key_addr;
	struct udevice *scsi = find_scsi_device(0x3/*WLUN*/);

	if (argc != 5)
		return CMD_RET_USAGE;

	addr = (void *)simple_strtoul(argv[1], NULL, 16);
	blk = simple_strtoul(argv[2], NULL, 16);
	cnt = simple_strtoul(argv[3], NULL, 16);
	key_addr = (void *)simple_strtoul(argv[4], NULL, 16);

	printf("\nUFS RPMB write: dev # %d, block # %d, count %d ... ",
	       0x3, blk, cnt);
	n =  ufs_rpmb_write(scsi, addr, blk, cnt, key_addr);

	printf("%d RPMB blocks written: %s\n", n, (n == cnt) ? "OK" : "ERROR");
	if (n != cnt)
		return CMD_RET_FAILURE;
	return CMD_RET_SUCCESS;
}
static int do_ufsrpmb_counter(struct cmd_tbl *cmdtp, int flag,
			      int argc, char * const argv[])
{
	unsigned long counter;
	struct udevice *scsi = find_scsi_device(0x3/*WLUN*/);

	if (ufs_rpmb_get_counter(scsi, &counter))
		return CMD_RET_FAILURE;
	printf("RPMB Write counter= %lx\n", counter);
	return CMD_RET_SUCCESS;
}

static struct cmd_tbl cmd_rpmb[] = {
	U_BOOT_CMD_MKENT(key, 2, 0, do_ufsrpmb_key, "", ""),
	U_BOOT_CMD_MKENT(read, 5, 1, do_ufsrpmb_read, "", ""),
	U_BOOT_CMD_MKENT(write, 5, 0, do_ufsrpmb_write, "", ""),
	U_BOOT_CMD_MKENT(counter, 1, 1, do_ufsrpmb_counter, "", ""),
};

static int do_ufsrpmb(struct cmd_tbl *cmdtp, int flag,
		      int argc, char * const argv[])
{
	struct cmd_tbl *cp;
	int ret;

	cp = find_cmd_tbl(argv[1], cmd_rpmb, ARRAY_SIZE(cmd_rpmb));

	/* Drop the rpmb subcommand */
	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

#if defined (CONFIG_UFS_CORE)
	ufs_probe();
#endif

	ret = cp->cmd(cmdtp, flag, argc, argv);

	return ret;
}
#endif

static int do_ufs_init(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	int dev, ret;

	if (argc >= 1) {
		if (!strcmp(argv[0], "init")) {
			if (argc == 3) {
				dev = simple_strtoul(argv[1], NULL, 10);
				ret = ufs_probe_dev(dev);
				if (ret)
					return CMD_RET_FAILURE;
			} else {
#if defined (CONFIG_UFS_CORE)
				ufs_probe();
#endif
			}

			return CMD_RET_SUCCESS;
		}
	}

	return CMD_RET_USAGE;
}

static struct cmd_tbl cmd_ufs[] = {
	U_BOOT_CMD_MKENT(init, 1, 0, do_ufs_init, "", ""),
#if CONFIG_IS_ENABLED(CMD_UFS_RPMB)
	U_BOOT_CMD_MKENT(rpmb, CONFIG_SYS_MAXARGS, 1, do_ufsrpmb, "", ""),
#endif
};

static int do_ufsops(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_ufs, ARRAY_SIZE(cmd_ufs));

	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(ufs, 29, 1, do_ufsops,
	   "UFS  sub system",
	   "init [dev] - init UFS subsystem\n"
#if CONFIG_IS_ENABLED(CMD_UFS_RPMB)
	    "ufs rpmb read addr blk# cnt [address of auth-key] - block size is 256 bytes\n"
	    "ufs rpmb write addr blk# cnt <address of auth-key> - block size is 256 bytes\n"
	    "ufs rpmb key <address of auth-key> - program the RPMB authentication key.\n"
	    "ufs rpmb counter - read the value of the write counter\n"
#endif
);
