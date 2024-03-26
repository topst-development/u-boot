// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#define pr_fmt(fmt) "pmic-i2c-simple: " fmt

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>

static int pmic_i2c_simple_read(struct udevice *dev, uint reg, uint8_t *buff,
				int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret != 0) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
	}

	return ret;
}

static int pmic_i2c_simple_write(struct udevice *dev, uint reg,
				 const uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret != 0) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
	}

	return ret;
}

static struct dm_pmic_ops pmic_i2c_simple_ops = {
	.read = pmic_i2c_simple_read,
	.write = pmic_i2c_simple_write,
};

static const struct udevice_id pmic_i2c_simple_ids[] = {
	{ .compatible = "pmic-i2c-simple" },
	{ }
};

U_BOOT_DRIVER(pmic_i2c_simple) = {
	.id		= UCLASS_PMIC,
	.name		= "pmic_i2c_simple",
	.of_match	= pmic_i2c_simple_ids,
	.ops		= &pmic_i2c_simple_ops,
};
