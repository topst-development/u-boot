// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/da9131.h>

static int da9131_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret != 0) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
		return ret;
	}

	return 0;
}

static int da9131_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret != 0) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
		return ret;
	}

	return 0;
}

static struct dm_pmic_ops da9131_ops = {
	.read = da9131_read,
	.write = da9131_write,
};

static const struct udevice_id da9131_ids[] = {
	{ .compatible = "dlg,da9131" },
	{ }
};

U_BOOT_DRIVER(pmic_da9131) = {
	.id		= UCLASS_PMIC,
	.name		= "pmic_da9131",
	.of_match	= da9131_ids,
	.ops		= &da9131_ops,
};
