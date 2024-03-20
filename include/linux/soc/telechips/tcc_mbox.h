// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_MBOX
#define TCC_MBOX

struct tcc_mbox_msg {
	u32 cmd_len;
	u32 *cmd;
	u32 data_len;
	u32 *data_buf;
};

#endif /* TCC_MBOX */
