// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <linux/io.h>
#include <mach/cm4.h>

#if defined(CONFIG_TCC_MAINCORE)
#   define CM4_MBOX		(0x19000000U)
#else
#   define CM4_MBOX		(0x19020000U)
#endif

#define MBOX_SCMD(base, n)	((base) + 0x00U + ((n) << 2))
#define MBOX_RCMD(base, n)	((base) + 0x20U + ((n) << 2))
#define MBOX_CTRL(base)		((base) + 0x40U)
#define MBOX_STATCMD(base)	((base) + 0x44U)

#define CM4_BOOT_MAGIC_VAL	(0xB007ECF0U) /* Boot EarlyCam Firmware */

void run_cm4_firmware(void)
{
	u32 ctrl, statcmd;
	void *reg;

	/* Set OEN to low */
	reg = phys_to_virt(MBOX_CTRL(CM4_MBOX));
	ctrl = readl(reg);
	writel(ctrl & ~((u32)1U << 5U), reg);

	/* Write data to command FIFO */
	reg = phys_to_virt(MBOX_SCMD(CM4_MBOX, 0U));
	writel(CM4_BOOT_MAGIC_VAL, reg);

	/* Set OEN to high */
	reg = phys_to_virt(MBOX_CTRL(CM4_MBOX));
	ctrl = readl(reg);
	writel(ctrl | (1U << 5U), reg);

	/* Wait for mailbox being empty */
	reg = phys_to_virt(MBOX_STATCMD(CM4_MBOX));
	do {
		mdelay(10);

		statcmd = readl(reg);
		statcmd &= (u32)1U;
	} while (statcmd == 0U);
}
