// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <asm/io.h>

#if defined(CONFIG_TCC_MAINCORE)
#  define CPU_MBOX (0x17A00000U)	/* CPU MBOX 6 */
#else
#  define CPU_MBOX (0x17B00000U)	/* CPU MBOX 7 */
#endif

#define MBOX_SCMD(base, n)	((base) + 0x00U + ((n) << 2))
#define MBOX_RCMD(base, n)	((base) + 0x20U + ((n) << 2))
#define MBOX_CTRL(base)		((base) + 0x40U)
#define MBOX_STATCMD(base)	((base) + 0x44U)

#define CMD_NOTIFY 0
#define CMD_WAIT 1

static s32 do_sync_notify(u32 id)
{
	u32 ctrl, statcmd;
	void *reg;

	flush_dcache_all();

	/* Set OEN to low */
	reg = phys_to_virt(MBOX_CTRL(CPU_MBOX));
	ctrl = readl(reg);
	writel(ctrl & ~((u32)1U << 5U), reg);

	/* Write data to command FIFO */
	reg = phys_to_virt(MBOX_SCMD(CPU_MBOX, 0U));
	writel(id, reg);

	/* Set OEN to high */
	reg = phys_to_virt(MBOX_CTRL(CPU_MBOX));
	ctrl = readl(reg);
	writel(ctrl | (1U << 5U), reg);

	pr_info("[INFO][cmd:sync] Notification sent. (0x%08x)\n", id);

	/* Wait for mailbox being empty */
	reg = phys_to_virt(MBOX_STATCMD(CPU_MBOX));
	do {
		mdelay(10);

		statcmd = readl(reg);
		statcmd &= (u32)1U;
	} while (statcmd == 0U);

	return (s32)CMD_RET_SUCCESS;
}

static s32 do_sync_wait(u32 id)
{
	u32 statcmd, recv;
	void *reg;

	/* Wait for data reception */
	reg = phys_to_virt(MBOX_STATCMD(CPU_MBOX));
	do {
		mdelay(10);

		statcmd = readl(reg);
		statcmd &= (u32)1U << 16;
	} while (statcmd != 0U);

	/* Read data from command FIFO */
	reg = phys_to_virt(MBOX_RCMD(CPU_MBOX, 0U));
	recv = readl(reg);

	pr_info("[INFO][cmd:sync] Message received. (0x%08x)\n", recv);

	if (recv == id)
		return (s32)CMD_RET_SUCCESS;

	pr_err("[ERR][cmd:sync] Unexpected message. (expected: 0x%08x)\n", id);

	return (s32)CMD_RET_FAILURE;
}

static inline s32 parse_cmd(char *const cmd)
{
	s32 ret;

	switch ((u8)cmd[0]) {
	case (u8)'n':
		ret = strncmp(cmd, "notify", (size_t)7U);
		ret = (ret == 0) ? CMD_NOTIFY : (s32)CMD_RET_USAGE;
		break;
	case (u8)'w':
		ret = strncmp(cmd, "wait", (size_t)5U);
		ret = (ret == 0) ? CMD_WAIT : (s32)CMD_RET_USAGE;
		break;
	default:
		ret = (s32)CMD_RET_USAGE;
		break;
	}

	return ret;
}

static int do_sync(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	s32 ret;
	u32 id;

	if (argc < 3)
		return (s32)CMD_RET_USAGE;

	ret = parse_cmd(argv[1]);
	id = (u32)strtoul(argv[2], NULL, 16);

	switch (ret) {
	case CMD_NOTIFY:
		ret = do_sync_notify(id);
		break;
	case CMD_WAIT:
		ret = do_sync_wait(id);
		break;
	default:
		ret = (s32)CMD_RET_USAGE;
		break;
	}

	return ret;
}

U_BOOT_CMD(
	sync, 3, 0, do_sync,
	"Sync main/subcore for Telechips platform",
	"Usage: sync [notify|wait] #id"
)
