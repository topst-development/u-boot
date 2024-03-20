// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <debug.h>
#include <linux/io.h>
#include <mailbox.h>
#include <linux/soc/telechips/tcc805x_multi_mbox.h>
#include <cam_ipc.h>
#include <common.h>
#include <dm.h>

#define MBOX_ID_CAMIPC 0

static unsigned int is_drv_probed;

int init_cam_ipc(void)
{
	struct udevice	*dev	= NULL;
	int ret = 0;

	/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_11_9_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_2_violation : FALSE] */
	/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	uclass_foreach_dev_probe(UCLASS_MAILBOX, dev);

	if (is_drv_probed != 1U) {
		(void)pr_force("[INFO] CAM_IPC is not initialized!\n");
		(void)pr_force("[INFO] Please check cam_ipc of device tree\n");
		(void)pr_force("[INFO] TRVC could be frozen during handover\n");

		ret = -ENODEV;
	} else {
		(void)pr_info("cam ipc is initialized successfully.\n");
	}

	return ret;
}

static struct mbox_chan *request_mbox_channel(struct cam_ipc_device *dev,
					      int channel_id)
{
	struct mbox_chan *chan = &dev->mbox_ch;
	struct mbox_chan *ret;

	if ((IS_ERR_OR_NULL(dev) == false) &&
		(mbox_get_by_index(dev->udev, channel_id, chan) >= 0)) {
		(void)pr_debug("succeed to request mailbox !\n");
		ret = chan;
	} else {
		(void)pr_err("Failed to request mbox (id: %d)\n", channel_id);
		ret = NULL;
	}

	return ret;
}

static void send_mbox_message(struct cam_ipc_device *dev,
				const struct tcc_mbox_data *payload)
{
	if (IS_ERR_OR_NULL(dev) == true) {
		/* failure of sending a msg */
		(void)pr_err("Failed on sending a message\n");
	} else {
		if (mbox_send(&dev->mbox_ch, (const void *)payload) < 0) {
			/* success of sending a msg */
			(void)pr_err("Succeed to send\n");
		}
	}
}

static int initialize(struct cam_ipc_device *dev)
{
	int ret = 0;
	int temp = 0;
	struct tcc_mbox_data data;

	mutex_init(&dev->rx.lock);
	atomic_set(&dev->rx.seq, 0);

	(void)request_mbox_channel(dev, MBOX_ID_CAMIPC);
	if (IS_ERR(&dev->mbox_ch) > 0L) {
		(void)pr_err("Fail to_request a channel\n");
		ret = -1;
	} else {
		(void)memset(&data, 0x0, sizeof(struct tcc_mbox_data));
		temp = atomic_read(&dev->tx.seq);
		if (temp >= 0) {
			data.cmd[0] = (u32)temp;
		}
		data.cmd[1] = ((u32)((unsigned int)CAM_IPC_CMD_EARLY & 0xFFFFU) << 16U)
			| (u32)CAM_IPC_MGR_SEND;

		/* FIXME Later, following copy function should be replaced to strscpy
		* when the boot-loader code is updated.
		*/
		(void)strncpy(data.id_name, "CAMIPC", MBOX_ID_LEN);

		send_mbox_message(dev, &data);

		is_drv_probed = 1;
	}

	return ret;
}

static const struct udevice_id cam_ipc_ids[] = {
	{
		.compatible = "telechips,cam_ipc",
	},
	{}
};

static int cam_ipc_probe(struct udevice *dev)
{
	struct cam_ipc_device	*cam_dev = NULL;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	cam_dev = (struct cam_ipc_device *)dev_get_priv(dev);
	cam_dev->udev = dev;

	return initialize(cam_dev);
}

/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int cam_ipc_remove(struct udevice *dev)
{
	(void)dev;
	return 0;
}

/* coverity[cert_dcl37_c_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
U_BOOT_DRIVER(cam_ipc) = {
	/* coverity[cert_str30_c_violation : FALSE] */
	.name = "cam_ipc",
	.id = UCLASS_MAILBOX,
	.of_match = cam_ipc_ids,
	.probe = cam_ipc_probe,
	.remove = cam_ipc_remove,
	.priv_auto_alloc_size = (int)sizeof(struct cam_ipc_device),
};
