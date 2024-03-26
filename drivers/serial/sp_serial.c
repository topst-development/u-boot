// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <dm/platform_data/serial_pl01x.h>
#include "serial_pl01x_internal.h"

DECLARE_GLOBAL_DATA_PTR;

static int sp_serial_probe(struct udevice *dev)
{
	ofnode sp_serial_node;
	ofnode uart_node;
	u32 uart_phandle = 0;
	struct udevice *udev;

	gd->baudrate = CONFIG_BAUDRATE;
	sp_serial_node = dev_ofnode(dev);
	ofnode_read_u32(sp_serial_node, "serial_sel", &uart_phandle);
	uart_node = ofnode_get_by_phandle(uart_phandle);
	uclass_get_device_by_ofnode(UCLASS_SERIAL, uart_node, &udev);

	pl01x_serial_setbrg(udev, 115200);

	return 0;
}

UCLASS_DRIVER(sp_serial) = {
	.name	= "sp_serial",
	.id		= UCLASS_SP_SERIAL,
};

static const struct udevice_id sp_serial_ids[] = {
	{ .compatible = "sp-serial" },
	{ }
};

U_BOOT_DRIVER(sp_serial) = {
	.name		= "sp_serial",
	.id		= UCLASS_SP_SERIAL,
	.of_match	= of_match_ptr(sp_serial_ids),
	.probe		= sp_serial_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};
