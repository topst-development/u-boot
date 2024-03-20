// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC803x_MULTI_MBOX
#define TCC803x_MULTI_MBOX

#define TCC_MBOX_SUCCESS            0
#define TCC_MBOX_ERR_COMMON         -1
#define TCC_MBOX_ERR_ARG_ERROR      -2
#define TCC_MBOX_ERR_BUSY           -3
#define TCC_MBOX_ERR_TIMEOUT        -4

#define MBOX_CMD_FIFO_SIZE		(6U)
#define MBOX_DATA_FIFO_SIZE		(128U)

#define MBOX_ID_LEN		(6U)

struct tcc_mbox_data {
	u32	cmd[MBOX_CMD_FIFO_SIZE];
	u32	data[MBOX_DATA_FIFO_SIZE];
	u32	data_len;
	char id_name[MBOX_ID_LEN];
};

#endif
