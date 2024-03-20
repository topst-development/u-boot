// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#define DEBUG

#include <bouncebuf.h>
#include <common.h>
#include <cpu_func.h>
#include <errno.h>
#include <malloc.h>
#include <memalign.h>
#include <scsi.h>
#include <wait_bit.h>
#include <dm.h>
#include <linux/soc/telechips/tcc_sc_protocol.h>
#include <ufs.h>
#include "ufs.h"

/* Only use one Task Tag for all requests */
#define TASK_TAG	(0)
#define UFS_MAX_BYTES	(128 * 256 * 1024)

struct tcc_sc_ufs_host {
	const char *name;
	uint32_t caps;
	void *priv;
	const struct tcc_sc_fw_handle *fw_handle;

	uint8_t part_num;
	struct ufs_desc_info desc_info;
};

struct tcc_sc_ufs_priv {
	struct tcc_sc_ufs_host host;
};
#if defined (CONFIG_SUPPORT_UFS_RPMB)
int ufs_fill_rpmb_unit_desc(struct udevice *scsi_dev,
		struct ufs_query_sc *q, uint32_t *buf)
{
	q->datsz = 0x23; /* RPMB Unit Desc Size */
	q->legacy_buf = buf;
	q->ts.desc.opcode = UPIU_QUERY_OPCODE_READ_DESC;
	q->ts.desc.idn = QUERY_DESC_IDN_UNIT;
	q->ts.desc.index = 0xC4; /* RPMB Unit Index */
	q->ts.desc.selector = 0x0;

	return 0;
}

static int ufs_get_model_name(struct udevice *scsi_dev, u8 *name, size_t size)
{
	struct blk_desc *dev_desc;

	dev_desc = blk_get_dev("scsi", 0x3/*RPMB*/);
	memcpy(name, dev_desc->product, size);

	return 0;
}


static int ufs_fill_geometry_desc(struct udevice *scsi_dev,
		struct ufs_query_sc *q, uint32_t *buf)
{
	q->datsz = 0x48; /* Geometry Desc Size */
	q->legacy_buf = buf;
	q->ts.desc.opcode = UPIU_QUERY_OPCODE_READ_DESC;
	q->ts.desc.idn = QUERY_DESC_IDN_GEOMETRY;
	q->ts.desc.index = 0x00;
	q->ts.desc.selector = 0x0;

	return 0;
}

static u8 ufs_get_rpmb_size_mult(struct udevice *scsi_dev)
{
	struct blk_desc *dev_desc;
	int byte;
	u8 size_mult;

	dev_desc = blk_get_dev("scsi", 0x3/*RPMB*/);
	byte = dev_desc->lba * dev_desc->blksz;
	size_mult = byte / 1024 * 128;
	/* rpmb size = size_mult * 128KB. - UFS  */

	return size_mult;
}

static u8 ufs_get_rpmb_wr_sec_c(struct udevice *scsi_dev)
{
	struct ufs_query_sc q = {0, };
	char buf[512] = {0, };
	u8 value;

	ufs_fill_geometry_desc(scsi_dev, &q, (uint32_t *)buf);
	scsi_ufs_query(scsi_dev, &q);
	value = (u8)buf[0x17];

	/* rpmb wr_sec_c : max. multi frame count for writing */
	return value;
}

struct ufs_desc_info *ufs_get_desc_info(struct udevice *dev)
{
	struct tcc_sc_ufs_priv *priv = dev_get_priv(dev);
	struct tcc_sc_ufs_host *host = &(priv->host);

	ufs_get_model_name(dev, (unsigned char*)host->desc_info.product,
		sizeof(host->desc_info.product));
	host->desc_info.bRPMB_ReadWriteSize = ufs_get_rpmb_wr_sec_c(dev);
	host->desc_info.bRPMBRegion0Size = ufs_get_rpmb_size_mult(dev);

	return &host->desc_info;
}
#endif

static int32_t tcc_sc_ufs_probe(struct udevice *dev)
{
	struct tcc_sc_ufs_priv *priv = dev_get_priv(dev);
	struct tcc_sc_ufs_host *host = &(priv->host);
	const struct tcc_sc_fw_handle *handle = tcc_sc_fw_get_handle(dev);
	struct scsi_platdata *scsi_plat;
	int32_t res=0;

	if (dev == NULL) {
		res = -ENODEV;
	} else {
		host->fw_handle = handle;
		host->name = dev->name;

		scsi_plat = dev_get_uclass_platdata(dev);
		scsi_plat->max_id = (unsigned long)UFSHCD_MAX_ID;
		scsi_plat->max_lun = UFS_MAX_LUNS;
		scsi_plat->max_bytes_per_req = UFS_MAX_BYTES;
		dev->priv = (void *)host;
	}

	return res;
}

static int32_t tcc_sc_ufs_scsi_exec(struct udevice *scsi_dev,
		struct scsi_cmd *pccb)
{
	const struct tcc_sc_fw_handle *handle =
		tcc_sc_fw_get_handle(scsi_dev);
	const struct tcc_sc_ufs_host *host;
	int32_t ret = 0;

	if((scsi_dev == NULL) || (pccb == NULL)) {
		ret= -ENODEV;
	} else {
		host = scsi_dev->priv;
	dev_dbg(scsi_dev, "%s:%d - origin pccb->pdata = 0x%p\n",
			__func__, __LINE__, pccb->pdata);
	if (pccb->pdata != NULL) {
		flush_cache((unsigned long)pccb->pdata,
			(unsigned long)pccb->datalen);
	}
	flush_cache((unsigned long)pccb->cmd,
		(unsigned long)pccb->cmdlen);

	ret = handle->ops.ufs_ops.request_command(host->fw_handle, pccb);
	}
	return ret;
}



static int32_t tcc_sc_ufs_ufs_query(struct udevice *scsi_dev,
		struct ufs_query_sc *q)
{
	const struct tcc_sc_fw_handle *handle =
		tcc_sc_fw_get_handle(scsi_dev);
	struct tcc_sc_ufs_host *host;
	int32_t ret = 0;

	if (scsi_dev == NULL) {
		return -ENODEV;
	}

	host = scsi_dev->priv;
	dev_dbg(scsi_dev, "%s:%d - origin q->legacy_buf = 0x%p\n",
			__func__, __LINE__, q->legacy_buf);
	if (q->legacy_buf != NULL) {
		flush_cache((unsigned long)q->legacy_buf,
			(unsigned long)q->datsz);
	}

	ret = handle->ops.ufs_ops.request_query(host->fw_handle, q);

	return ret;

}

int ufs_probe_dev(int index)
{
	struct udevice *dev;

	return uclass_get_device(UCLASS_UFS, index, &dev);
}

static struct scsi_ops tcc_sc_ufs_scsi_ops = {
	.exec		= tcc_sc_ufs_scsi_exec,
	.ufs_query	= tcc_sc_ufs_ufs_query,
};

static const struct udevice_id tcc_sc_ufs_ids[2] = {
	{ .compatible = "telechips,tcc805x-sc-ufs" },
	{ }
};

U_BOOT_DRIVER(tcc_sc_ufs_drv) = {
	.name               = "tcc_sc_ufs",
	.id                 = UCLASS_SCSI,
	.of_match           = tcc_sc_ufs_ids,
	.ops                = &tcc_sc_ufs_scsi_ops,
	.probe              = tcc_sc_ufs_probe,
	.priv_auto_alloc_size     = (int32_t) sizeof(struct tcc_sc_ufs_priv),
};

