// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#include <common.h>
#include <command.h>
#include <test/can_test.h>


static int do_can_test(struct cmd_tbl *cmd, int flag, int argc,
		      char *const argv[])
{


    CANTestMain();

    return 0;

}





/***************************************************/

U_BOOT_CMD(
	can_test,	1,	1,	do_can_test,
	"print console devices and information",
	""
);
