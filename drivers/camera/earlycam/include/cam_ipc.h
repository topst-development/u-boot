// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef CAM_IPC_H
#define CAM_IPC_H

#include <mailbox.h>
#include <asm-generic/atomic.h>
#include <linux/compat.h>

#define CAM_IPC_MGR_SEND	0x1
#define CAM_IPC_MGR_ACK		0x10
#define CAM_IPC_MGR_STATUS	0x11

enum {
	CAM_IPC_CMD_NULL = 0,
	CAM_IPC_CMD_OVP,
	CAM_IPC_CMD_POS,
	CAM_IPC_CMD_RESET,
	CAM_IPC_CMD_READY,
	CAM_IPC_CMD_STATUS,
	CAM_IPC_CMD_EARLY,
	CAM_IPC_CMD_MAX,
};

struct cam_ipc_tx {
	struct mutex lock;
	atomic_t seq;
};

struct cam_ipc_rx {
	struct mutex lock;
	atomic_t seq;
};

struct cam_ipc_device {
	struct udevice *udev;
	const char *name;
	const char *mbox_name;
	const char *mbox_id;
	struct mbox_chan mbox_ch;

	atomic_t status;

	struct cam_ipc_tx tx;
	struct cam_ipc_rx rx;
};

extern int init_cam_ipc(void);

#endif
