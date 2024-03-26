// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2016 Xilinx, Inc
 * Written by Michal Simek
 *
 * Based on ahci-uclass.c
 */

#define LOG_CATEGORY UCLASS_SCSI

#include <common.h>
#include <dm.h>
#include <scsi.h>
#if defined(CONFIG_TCC_UFS) || defined(CONFIG_TCC_SC_UFS)
#include <blk.h>

#if CONFIG_IS_ENABLED(BLK)
struct udevice *find_scsi_device(int dev_num)
{
	struct udevice *dev, *scsi_dev;
	int ret;

	ret = blk_find_device(IF_TYPE_SCSI, dev_num, &dev);

	if (ret) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		printf("SCSI Device %d not found\n", dev_num);
#endif
		return NULL;
	}

	scsi_dev = dev_get_parent(dev);

	return scsi_dev;
}
#endif
#endif

int scsi_exec(struct udevice *dev, struct scsi_cmd *pccb)
{
	struct scsi_ops *ops = scsi_get_ops(dev);

	if (!ops->exec)
		return -ENOSYS;

	return ops->exec(dev, pccb);
}

int scsi_bus_reset(struct udevice *dev)
{
	struct scsi_ops *ops = scsi_get_ops(dev);

	if (!ops->bus_reset)
		return -ENOSYS;

	return ops->bus_reset(dev);
}

#if defined(CONFIG_TCC_UFS) || defined(CONFIG_TCC_SC_UFS)
int scsi_ufs_query(struct udevice *dev, struct ufs_query_sc *q)
{
	struct scsi_ops *ops = scsi_get_ops(dev);

	if (!ops->ufs_query)
		return -ENOSYS;

	return ops->ufs_query(dev, q);
}
#endif

UCLASS_DRIVER(scsi) = {
	.id		= UCLASS_SCSI,
	.name		= "scsi",
	.per_device_plat_auto	= sizeof(struct scsi_plat),
};
