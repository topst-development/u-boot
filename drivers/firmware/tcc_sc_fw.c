// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <dm/device.h>
#include <errno.h>
#include <linux/compat.h>
#include <linux/err.h>
#include <mmc.h>
#include <mailbox.h>
#ifdef CONFIG_ARM64
#include <asm/armv8/cpu.h>
#else
#include <asm/armv7/cpu.h>
#endif
#include <linux/soc/telechips/tcc_mbox.h>
#include <linux/soc/telechips/tcc_sc_protocol.h>
#include "tcc_sc_fw.h"
#include <linux/io.h>
#include <scsi.h>

#define TCC_SC_FW_UFS_XFER_DATA_SIZE	6U
#define TCC_SC_FW_MAX_RECV_TRIES	10U

static struct tcc_sc_fw_dev *handle_to_tcc_sc_fw_dev(
			const struct tcc_sc_fw_handle *handle)
{
	if (handle == NULL)
		return NULL;

	return (struct tcc_sc_fw_dev *)handle->priv;
}

static void print_sc_cmd_dump(struct tcc_sc_fw_cmd *cmd, const char *s)
{
	pr_debug("[DEBUG] SC CMD DUMP: %s\n", s);
	pr_debug("[DEBUG] CID: 0x%x | BSID: 0x%x\n",
		cmd->cid, cmd->bsid);
	pr_debug("[DEBUG] CMD: 0x%x | UID: 0x%x\n",
		cmd->cmd, cmd->uid);
	pr_debug("[DEBUG] ARG0: 0x%x | ARG1: 0x%x\n",
		cmd->args[0], cmd->args[1]);
	pr_debug("[DEBUG] ARG2: 0x%x | ARG3: 0x%x\n",
		cmd->args[2], cmd->args[3]);
	pr_debug("[DEBUG] ARG4: 0x%x | ARG5: 0x%x\n",
		cmd->args[4], cmd->args[5]);
}

static int check_response(u32 *req, u32 *res)
{
	int ret = 0;
	struct tcc_sc_fw_cmd *req_cmd;
	struct tcc_sc_fw_cmd *res_cmd;

	req_cmd = (struct tcc_sc_fw_cmd *)req;
	res_cmd = (struct tcc_sc_fw_cmd *)res;

	if (res_cmd->bsid != req_cmd->bsid)
		ret = -1;

	if (res_cmd->cid != TCC_SC_CID_SC)
		ret = -1;

	if (res_cmd->uid != req_cmd->uid)
		ret = -1;

	if (res_cmd->cmd != req_cmd->cmd)
		ret = -1;

	return ret;
}

static int tcc_sc_fw_send_recv(struct tcc_sc_fw_dev *tcc_sc_fw,
	u32 *req, u32 *req_data, u32 req_data_len,
	u32 *res, u32 *res_data, u32 res_data_len)
{
	struct tcc_mbox_msg msg = {0, };
	u32 cmd_len = TCC_SC_FW_CMD_FIXED_LENGTH;
	int ret;
	int retry;


	msg.cmd = req;
	msg.cmd_len = cmd_len;
	msg.data_len = req_data_len;
	msg.data_buf = req_data;
	ret = mbox_send(&tcc_sc_fw->chan, (void *)&msg);
	if (ret != 0) {
		pr_err("[ERROR] %s failed to send mbox message\n", __func__);
		return ret;
	}

	retry = 0;
	do {
		/* receive commnad */
		msg.cmd = res;
		msg.cmd_len = cmd_len;
		msg.data_len = res_data_len;
		msg.data_buf = res_data;
		ret = mbox_recv(&tcc_sc_fw->chan, (void *)&msg,
				TCC_SC_BLK_REQ_TIMEOUT_US);
		if (ret != 0) {
			pr_err("[ERROR] %s failed to receive mbox message\n",
				__func__);
			return ret;
		}

		if (check_response(req, res) == 0) {
			ret = 0;
			break;
		}

		ret = -EIO;
		pr_debug("[debug] %s receive NAK (req 0x%x 0x%x, res 0x%x 0x%x)\n",
		       __func__, req[0], req[1], res[0], res[1]);

		retry++;
	} while (retry < TCC_SC_FW_MAX_RECV_TRIES);

	if (ret != 0) {
		pr_err(
			"[ERROR] %s receive NAK (req 0x%x 0x%x, res 0x%x 0x%x)\n",
			__func__, req[0], req[1], res[0], res[1]);
		print_sc_cmd_dump((struct tcc_sc_fw_cmd *)req, "Request");
		print_sc_cmd_dump((struct tcc_sc_fw_cmd *)res, "Response");
	}

	return ret;
}

static int tcc_sc_fw_send_recv_cmd_only(
			struct tcc_sc_fw_dev *tcc_sc_fw, u32 *req, u32 *res)
{
	struct tcc_mbox_msg msg = {0, };
	u32 cmd_len = TCC_SC_FW_CMD_FIXED_LENGTH;
	int ret;
	int retry;

	msg.cmd = req;
	msg.cmd_len = cmd_len;
	msg.data_len = 0;
	msg.data_buf = NULL;
	ret = mbox_send(&tcc_sc_fw->chan, (void *)&msg);
	if (ret != 0) {
		pr_err("[ERROR] %s failed to send mbox message\n", __func__);
		return ret;
	}

	retry = 0;
	do {
		/* receive commnad */
		msg.cmd = res;
		msg.cmd_len = cmd_len;
		msg.data_len = 0;
		msg.data_buf = NULL;
		ret = mbox_recv(&tcc_sc_fw->chan, (void *)&msg,
				TCC_SC_BLK_REQ_TIMEOUT_US);
		if (ret != 0) {
			pr_err("[ERROR] %s failed to receive mbox message\n",
				__func__);
			return ret;
		}

		if (check_response(req, res) == 0) {
			ret = 0;
			break;
		}

		ret = -EIO;
		pr_debug("[debug] %s receive NAK (req 0x%x 0x%x, res 0x%x 0x%x)\n",
				__func__, req[0], req[1], res[0], res[1]);

		retry++;
	} while (retry < TCC_SC_FW_MAX_RECV_TRIES);

	if (ret != 0) {
		pr_err(
			"[ERROR] %s receive NAK (req 0x%x 0x%x, res 0x%x 0x%x)\n",
			__func__, req[0], req[1], res[0], res[1]);
		print_sc_cmd_dump((struct tcc_sc_fw_cmd *)req, "Request");
		print_sc_cmd_dump((struct tcc_sc_fw_cmd *)res, "Response");
	}

	return ret;
}

static int tcc_sc_get_fw_version(const struct tcc_sc_fw_handle *handle,
				struct tcc_sc_fw_version *fw_info)
{
	struct tcc_sc_fw_dev *tcc_sc_fw = NULL;
	struct tcc_sc_fw_cmd req_cmd = {0, };
	struct tcc_sc_fw_cmd res_cmd = {0, };
	int ret;

	if (handle == NULL)
		return -EINVAL;

	tcc_sc_fw = handle_to_tcc_sc_fw_dev(handle);

	if (tcc_sc_fw == NULL) {
		return -EINVAL;
	}

	if (fw_info == NULL)
		return -EINVAL;

	/* send command */
	req_cmd.bsid = tcc_sc_fw->bsid;
	req_cmd.cid = tcc_sc_fw->cid;
	req_cmd.uid = tcc_sc_fw->uid;
	req_cmd.cmd = TCC_SC_CMD_FW_INFO;

	tcc_sc_fw->uid += 1U;

	ret = tcc_sc_fw_send_recv_cmd_only(tcc_sc_fw,
				(u32 *)&req_cmd, (u32 *)&res_cmd);
	if (ret != 0) {
		return ret;
	}

	ret = 0;
	memcpy((void *)fw_info, &res_cmd.args[0],
	       sizeof(struct tcc_sc_fw_version));

	return ret;
}

static int tcc_sc_request_mmc_prot_info(const struct tcc_sc_fw_handle *handle,
					struct tcc_sc_fw_prot_mmc *mmc_info)
{
	struct tcc_sc_fw_dev *tcc_sc_fw = NULL;
	struct tcc_sc_fw_cmd req_cmd = {0, };
	struct tcc_sc_fw_cmd res_cmd = {0, };
	int ret;

	if (handle == NULL)
		return -EINVAL;

	tcc_sc_fw = handle_to_tcc_sc_fw_dev(handle);

	if (tcc_sc_fw == NULL)
		return -EINVAL;

	if (mmc_info == NULL)
		return -EINVAL;

	/* send command */
	req_cmd.bsid = tcc_sc_fw->bsid;
	req_cmd.cid = tcc_sc_fw->cid;
	req_cmd.uid = tcc_sc_fw->uid;
	req_cmd.cmd = TCC_SC_CMD_PROT_INFO;
	req_cmd.args[0] = TCC_SC_CMD_PROT_MMC;

	tcc_sc_fw->uid += 1U;

	ret = tcc_sc_fw_send_recv_cmd_only(tcc_sc_fw,
			(u32 *)&req_cmd, (u32 *)&res_cmd);
	if (ret != 0) {
		return ret;
	}

	ret = 0;
	memcpy((void *)mmc_info, &res_cmd.args[0],
	       sizeof(struct tcc_sc_fw_prot_mmc));

	return ret;
}

static int tcc_sc_request_mmc_cmd(const struct tcc_sc_fw_handle *handle,
				struct mmc_cmd *cmd, struct mmc_data *data,
				unsigned char part_num)
{
	struct tcc_sc_fw_dev *tcc_sc_fw = NULL;
	struct tcc_sc_fw_cmd req_cmd = {0, };
	struct tcc_sc_fw_cmd res_cmd = {0, };
	unsigned int i, desc_count, xfer_byte;
	unsigned int seg_size = 65536;
	unsigned int addr;
	int ret;

	if (handle == NULL)
		return -EINVAL;

	tcc_sc_fw = handle_to_tcc_sc_fw_dev(handle);

	if (tcc_sc_fw == NULL)
		return -EINVAL;

	req_cmd.bsid = tcc_sc_fw->bsid;
	req_cmd.cid = tcc_sc_fw->cid;
	req_cmd.uid = tcc_sc_fw->uid;
	req_cmd.cmd = TCC_SC_CMD_MMC_CMD;
	req_cmd.args[0] = cmd->cmdidx;
	req_cmd.args[1] = cmd->cmdarg;
	req_cmd.args[2] = cmd->resp_type;
	req_cmd.args[3] = part_num;
	req_cmd.args[4] = 0;
	req_cmd.args[5] = 0;

	tcc_sc_fw->uid += 1U;

	if (data != NULL) {
		tcc_sc_fw->req_data[0] = data->blocksize;
		tcc_sc_fw->req_data[1] = data->blocks;
		tcc_sc_fw->req_data[2] = data->flags;

		xfer_byte = data->blocksize * data->blocks;
		desc_count = ((xfer_byte + seg_size) - 1U) / seg_size;
		addr = (phys_addr_t)data->dest;

		tcc_sc_fw->req_data[3] = desc_count;
		tcc_sc_fw->data_len = (u32)(4U + (tcc_sc_fw->req_data[3] * 2U));
		for (i = 0U ; i < (desc_count - 1U) ; i++) {
			tcc_sc_fw->req_data[4U + (i * 2U)] = addr;
			tcc_sc_fw->req_data[5U + (i * 2U)] = seg_size;
			xfer_byte = xfer_byte - seg_size;
			addr = addr + seg_size;
		}
		tcc_sc_fw->req_data[4U + (i * 2U)] = addr;
		tcc_sc_fw->req_data[5U + (i * 2U)] = xfer_byte;

		ret = tcc_sc_fw_send_recv(tcc_sc_fw,
				(u32 *)&req_cmd, tcc_sc_fw->req_data,
				tcc_sc_fw->data_len,
				(u32 *)&res_cmd, NULL, 0);
	} else {
		ret = tcc_sc_fw_send_recv_cmd_only(tcc_sc_fw,
					(u32 *)&req_cmd, (u32 *)&res_cmd);
	}

	if (ret == 0) {
		cmd->response[0] = res_cmd.args[0];
		cmd->response[1] = res_cmd.args[1];
		cmd->response[2] = res_cmd.args[2];
		cmd->response[3] = res_cmd.args[3];
		ret = (int)res_cmd.args[4];

		if ((data != NULL) && (ret == 0)) {
			ret = (int)res_cmd.args[5];
		}
	}

	return ret;
}

static int tcc_sc_request_ufs_cmd(
		const struct tcc_sc_fw_handle *handle, struct scsi_cmd *cmd)
{
	struct tcc_sc_fw_dev *tcc_sc_fw = NULL;
	struct tcc_sc_fw_cmd req_cmd = {0, };
	struct tcc_sc_fw_cmd res_cmd = {0, };
	int ret = 0;
	uint32_t direction, datsz, lba;

	if (handle == NULL)
		return -EINVAL;
	tcc_sc_fw = handle_to_tcc_sc_fw_dev(handle);

	if (tcc_sc_fw == NULL)
		return -EINVAL;

	/* direction( 0 = write, 1 = read, 2 = none */
	if (cmd->dma_dir == DMA_FROM_DEVICE) {
		direction = 1;
	} else if (cmd->dma_dir == DMA_TO_DEVICE) {
		direction = 0;
	} else if( cmd->dma_dir == 0xE) {
		direction = cmd->dma_dir;
	} else {
		direction = -1;
	}

	datsz = cmd->datalen;

	if (cmd->priv == 0xf) {
		lba = cmd->contr_stat;
	} else {
		lba = cmd->cmd[2] << 24 |
			cmd->cmd[3] << 16 |
			cmd->cmd[4] << 8 |
			cmd->cmd[5] << 0;
	}
#if DEBUG
	//memcpy(&lba, &cmd->cmd[2], sizeof(uint32_t));
	pr_debug("%s:%d - lba = %08x\n", __func__, __LINE__, lba);
	pr_debug("%s: Lun=%d(op=%x / 0x%x / 0x%x / 0x%x / 0x%x / 0x%x) LBACNT = 0x%x / 0x%x\n",
		__func__, cmd->lun, cmd->cmd[0], cmd->cmd[1], cmd->cmd[2],
		cmd->cmd[3], cmd->cmd[4], cmd->cmd[5],
		cmd->cmd[7], cmd->cmd[8]);
	pr_debug("cmd->pdata = %08x, cmd->datalen = %08x\n",
		cmd->pdata, cmd->datalen);
#endif
	req_cmd.bsid = tcc_sc_fw->bsid;
	req_cmd.cid = tcc_sc_fw->cid;
	req_cmd.uid = tcc_sc_fw->uid;
	req_cmd.cmd = TCC_SC_CMD_UFS_CMD;
	req_cmd.args[0] = datsz; //datsz
	req_cmd.args[1] = 0; //(uint32_t)cmd->cmd; //blocks
	req_cmd.args[2] = lba; //lba
	req_cmd.args[3] = cmd->lun; //partition number

	if (cmd->priv == 0xf) {
		req_cmd.args[4] = cmd->trans_bytes;
		req_cmd.args[5] = 0xf;
	} else {
		req_cmd.args[4] = 0; //tag
		req_cmd.args[5] = direction; //direction
	}

	memcpy(&tcc_sc_fw->req_data[0], &cmd->cmd[0], 4);
	memcpy(&tcc_sc_fw->req_data[1], &cmd->cmd[4], 4);
	memcpy(&tcc_sc_fw->req_data[2], &cmd->cmd[8], 4);
	memcpy(&tcc_sc_fw->req_data[3], &cmd->cmd[12], 4);

	tcc_sc_fw->req_data[4] = (phys_addr_t)cmd->pdata;
	tcc_sc_fw->req_data[5] = cmd->datalen;
	tcc_sc_fw->data_len = TCC_SC_FW_UFS_XFER_DATA_SIZE;
	tcc_sc_fw->uid += 1U;

	ret = tcc_sc_fw_send_recv(tcc_sc_fw,
				(u32 *)&req_cmd, tcc_sc_fw->req_data,
				tcc_sc_fw->data_len,
				(u32 *)&res_cmd, NULL, 0);
	if (ret != 0) {
		pr_err("[ERROR] %s tcc_sc_fw_send_recv() is failed\n",
			__func__);
		return ret;
	}

	return ret;
}

static int tcc_sc_request_ufs_query(
		const struct tcc_sc_fw_handle *handle, struct ufs_query_sc *q)
{
	struct tcc_sc_fw_dev *tcc_sc_fw = NULL;
	struct tcc_sc_fw_cmd req_cmd = {0, };
	struct tcc_sc_fw_cmd res_cmd = {0, };
	int ret = 0;
	uint32_t datsz;

	if (handle == NULL)
		return -EINVAL;
	tcc_sc_fw = handle_to_tcc_sc_fw_dev(handle);

	if (tcc_sc_fw == NULL)
		return -EINVAL;

	datsz = q->datsz;

	req_cmd.bsid = tcc_sc_fw->bsid;
	req_cmd.cid = tcc_sc_fw->cid;
	req_cmd.uid = tcc_sc_fw->uid;
	req_cmd.cmd = TCC_SC_CMD_UFS_CMD;
	req_cmd.args[0] = datsz; //datsz
	req_cmd.args[1] = q->ts.desc.opcode;
	req_cmd.args[2] = q->ts.desc.idn;
	req_cmd.args[3] = q->ts.desc.index;
	req_cmd.args[4] = (u32)(q->legacy_buf);
	req_cmd.args[5] = 0xe; //direction

	tcc_sc_fw->data_len = TCC_SC_FW_UFS_XFER_DATA_SIZE;
	tcc_sc_fw->uid += 1U;

	ret = tcc_sc_fw_send_recv(tcc_sc_fw,
				(u32 *)&req_cmd, tcc_sc_fw->req_data,
				tcc_sc_fw->data_len,
				(u32 *)&res_cmd, NULL, 0);
	if (ret != 0) {
		pr_err("[ERROR] %s tcc_sc_fw_send_recv() is failed\n",
			__func__);
		return ret;
	}

	return ret;
}

static int tcc_sc_request_reg_set_cmd(const struct tcc_sc_fw_handle *handle,
		uint32_t address, uint32_t bit_number,
		uint32_t width, uint32_t value)
{
	struct tcc_sc_fw_dev *tcc_sc_fw = NULL;
	struct tcc_sc_fw_cmd req_cmd = {0, };
	struct tcc_sc_fw_cmd res_cmd = {0, };
	int ret;

	if (handle == NULL)
		return -EINVAL;

	tcc_sc_fw = handle_to_tcc_sc_fw_dev(handle);

	if (tcc_sc_fw == NULL)
		return -EINVAL;

	req_cmd.bsid = tcc_sc_fw->bsid;
	req_cmd.cid = tcc_sc_fw->cid;
	req_cmd.uid = tcc_sc_fw->uid;
	req_cmd.cmd = TCC_SC_CMD_REG_SET;

	req_cmd.args[0] = address + 0x60000000U;
	req_cmd.args[1] = bit_number;
	req_cmd.args[2] = width;
	req_cmd.args[3] = value;
	req_cmd.args[4] = 0;
	req_cmd.args[5] = 0;

	tcc_sc_fw->uid += 1U;

	ret = tcc_sc_fw_send_recv_cmd_only(tcc_sc_fw,
				(u32 *)&req_cmd, (u32 *)&res_cmd);

	if (ret != 0) {
		return ret;
	}

	return ret;
}

static int tcc_sc_request_reg_set_cmd_multi(
				const struct tcc_sc_fw_handle *handle,
				struct tcc_sc_reg_req_data *reg_req_data)
{
	struct tcc_sc_fw_dev *tcc_sc_fw = NULL;
	struct tcc_sc_fw_cmd req_cmd = {0, };
	struct tcc_sc_fw_cmd res_cmd = {0, };
	int ret;
	unsigned int i;

	if (handle == NULL)
		return -EINVAL;

	tcc_sc_fw = handle_to_tcc_sc_fw_dev(handle);

	if (tcc_sc_fw == NULL)
		return -EINVAL;

	req_cmd.bsid = tcc_sc_fw->bsid;
	req_cmd.cid = tcc_sc_fw->cid;
	req_cmd.uid = tcc_sc_fw->uid;
	req_cmd.cmd = TCC_SC_CMD_REG_SET_MULTI;
	req_cmd.args[0] = reg_req_data->count;

	tcc_sc_fw->data_len = (u32) reg_req_data->count * 4U;
	for (i = 0U; i < ((u32) reg_req_data->count / 4U); i++) {
		tcc_sc_fw->req_data[(i * 4U)]
			= reg_req_data->config[i].address + 0x60000000U;
		tcc_sc_fw->req_data[(i * 4U) + 1U]
			= reg_req_data->config[i].bit_shift;
		tcc_sc_fw->req_data[(i * 4U) + 2U]
			= reg_req_data->config[i].width;
		tcc_sc_fw->req_data[(i * 4U) + 3U]
			= reg_req_data->config[i].value;
	}

	tcc_sc_fw->uid += 1U;

	ret = tcc_sc_fw_send_recv(tcc_sc_fw,
			(u32 *)&req_cmd, tcc_sc_fw->req_data,
			tcc_sc_fw->data_len,
			(u32 *)&res_cmd, NULL, 0);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

const struct tcc_sc_fw_handle *tcc_sc_fw_get_handle(struct udevice *dev)
{
	if (dev == NULL)
		return ERR_PTR(-EINVAL);

	struct udevice *sc_fw_dev = dev_get_parent(dev);

	if (sc_fw_dev == NULL)
		return ERR_PTR(-EINVAL);

	struct tcc_sc_fw_dev *priv = dev_get_priv(sc_fw_dev);

	if (priv == NULL)
		return ERR_PTR(-EINVAL);

	struct tcc_sc_fw_handle *handle = &priv->handle;

	return handle;
}

const struct tcc_sc_fw_handle *tcc_sc_fw_get_handle_by_name(void)
{
	struct tcc_sc_fw_dev *priv = NULL;
	struct udevice *sc_fw_dev = NULL;

	uclass_get_device_by_name(UCLASS_FIRMWARE, "tcc_sc_fw", &sc_fw_dev);
	priv = dev_get_priv(sc_fw_dev);

	if (priv == NULL)
		return NULL;

	return &priv->handle;
}

static int tcc_sc_fw_probe(struct udevice *dev)
{
	struct tcc_sc_fw_dev *priv = dev_get_priv(dev);
	int ret;

	pr_debug("[DEBUG] %s: %s(dev=%p\n", dev->name, __func__, dev);

	ret = mbox_get_by_index(dev, 0, &priv->chan);
	if (ret != 0) {
		pr_debug("[DEBUG] %s: failed to get mbox channel\n",
				dev->name);
		return ret;
	}

	if (is_cortex_a53() != 0) {
		priv->cid = TCC_SC_CID_CA53;
	} else if (is_cortex_a72() != 0) {
		priv->cid = TCC_SC_CID_CA72;
	} else {
		pr_err("[ERROR] %s: unsupported core\n",
				dev->name);
		return -EPERM;
	}

	priv->data_len = 128;
	priv->req_data = (u32 *)malloc(sizeof(u32) * priv->data_len);
	if (priv->req_data == NULL) {
		return -ENOMEM;
	}
	priv->res_data = (u32 *)malloc(sizeof(u32) * priv->data_len);
	if (priv->res_data == NULL) {
		free(priv->req_data);
		return -ENOMEM;
	}

	priv->bsid = TCC_SC_BSID_BL3;
	priv->uid = 0U;
	pr_debug("[DEBUG] %s: bsid 0x%02x cid 0x%02x\n",
				dev->name, priv->bsid, priv->cid);

	priv->handle.priv = priv;
	priv->handle.ops.mmc_ops.request_command
					= tcc_sc_request_mmc_cmd;
	priv->handle.ops.ufs_ops.request_command
					= tcc_sc_request_ufs_cmd;
	priv->handle.ops.ufs_ops.request_query
					= tcc_sc_request_ufs_query;
	priv->handle.ops.mmc_ops.prot_info
					= tcc_sc_request_mmc_prot_info;
	priv->handle.ops.reg_ops.request_reg_set
					= tcc_sc_request_reg_set_cmd;
	priv->handle.ops.reg_ops.request_reg_set_multi
					= tcc_sc_request_reg_set_cmd_multi;

	/* get firmware version */
	ret = tcc_sc_get_fw_version(&priv->handle, &priv->handle.version);
	if (ret != 0) {
		pr_err("[ERROR] %s: failed to get firmware info\n",
				dev->name);
		free(priv->req_data);
		free(priv->res_data);
	} else {
		pr_info("[INFO] %s: firmware version %d.%d.%d (%s)\n",
		    dev->name, priv->handle.version.major,
		    priv->handle.version.minor, priv->handle.version.patch,
		    priv->handle.version.desc);
	}

	return ret;
}

static const struct udevice_id tcc_sc_fw_ids[2] = {
	{ .compatible = "telechips,tcc805x-sc-fw"},
	{},
};

U_BOOT_DRIVER(tcc_sc_fw) = {
	.name = "tcc_sc_fw",
	.id = UCLASS_FIRMWARE,
	.of_match = tcc_sc_fw_ids,
	.probe	= tcc_sc_fw_probe,
	.priv_auto_alloc_size = (int) sizeof(struct tcc_sc_fw_dev),
};
