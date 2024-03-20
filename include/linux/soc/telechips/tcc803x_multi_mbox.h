// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC803x_MULTI_MBOX
#define TCC803x_MULTI_MBOX

#define MBOX_CMD_FIFO_SIZE      7U
#define MBOX_DATA_FIFO_SIZE     128U

#define TCC_MBOX_SUCCESS            0
#define TCC_MBOX_ERR_COMMON         -1
#define TCC_MBOX_ERR_ARG_ERROR      -2
#define TCC_MBOX_ERR_BUSY           -3
#define TCC_MBOX_ERR_TIMEOUT        -4

/*
 * MBOX has 8 cmd fifo but user can only use 7 cmd fifo.
 * Cmd fifo[0] is used inside mbox drvier for channel ID.
 * MBOX FIFO--User cmd--remark
 * cmd fifo[0]: channel ID (use mbox drvier only)
 * cmd fifo[1]:cmd[0] user cmd 0 (User ID)
 * cmd fifo[2]:cmd[1] user cmd 1
 * cmd fifo[3]:cmd[2] user cmd 2
 * cmd fifo[4]:cmd[3] user cmd 3
 * cmd fifo[5]:cmd[4] user cmd 4
 * cmd fifo[6]:cmd[5] user cmd 5
 * cmd fifo[7]:cmd[6] user cmd 6
 */

struct tcc_mbox_data {
	unsigned int cmd[MBOX_CMD_FIFO_SIZE];
	unsigned int data[MBOX_DATA_FIFO_SIZE];
	unsigned int data_len;
};

#endif
