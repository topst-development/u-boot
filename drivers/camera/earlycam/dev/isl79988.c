// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <debug.h>
#include <i2c.h>
#include <asm/arch/gpio.h>
#include <asm/telechips/gpio.h>
#include <asm/telechips/vioc/vioc_vin.h>
#include <videosource_if.h>
#include <isl79988.h>
#include <dm/device.h>
#include <dm/read.h>

#define I2C_CH_NO	0	/* I2C Master Number */
#define I2C_ADDR	0x88	/* I2C Slave Address */

#define WIDTH		720
#define HEIGHT		480

static struct vs_i2c_reg vs_i2c_reg_initialize[] = {
// ISL_79888 init
	{0xff, 0x00},		// page 0
	{0x02, 0x00},
	{0x03, 0x00},		// Disable Tri-state
	{0x04, 0x0A},		// Invert Clock (Normal Clock: 0x08

	{0xff, 0x01},		// page 1
	{0x1C, 0x07},		// auto dection single ended
	{0x37, 0x06},
	{0x39, 0x18},
	{0x33, 0x85},		// Free-run 60Hz
	{0x2f, 0xe6},		// auto blue screen

	{0xff, 0x00},		// page 0
	{0x07, 0x00},		// 1 ch mode
	{0x09, 0x4f},		// PLL=27MHz
	{0x0B, 0x42},		// PLL=27MHz

	{0Xff, 0x05},		// page 5
	{0x05, 0x42},		// byte interleave
	{0x06, 0x61},		// byte interleave
	{0x0E, 0x00},
	{0x11, 0xa0},		// Packet cnt = 1440 (EVB only)
	{0x13, 0x1B},
	{0x33, 0x40},
	{0x34, 0x18},		// PLL normal
	{0x00, 0x02},		// MIPI on

	{ADDR_TERM, DATA_TERM}
};

static struct vs_i2c_reg *vs_i2c_reg_list_table[] = {
	vs_i2c_reg_initialize,
};

static int open(struct udevice *dev)
{
	struct vs_gpio		*plat	= dev_get_platdata(dev);
	int			busnum;
	int			chip_addr;
	uint			offset_len;
	int			ret	= 0;

	/* power-up sequence */
	ret = dm_gpio_set_value(&plat->rst_port, 0);
	mdelay(50);

	ret = dm_gpio_set_value(&plat->rst_port, 1);
	mdelay(10);

	/* probe */
	busnum		= I2C_CH_NO;
	chip_addr	= (int)(I2C_ADDR >> 1);
	offset_len	= 1;
	ret = i2c_get_chip_for_busnum(busnum, chip_addr, offset_len, &dev);
	if (ret != 0) {
		/* failed to probe i2c chip */
		loge("i2c_get_chip_for_busnum(0x%x), ret: %d\n", I2C_ADDR, ret);
	}

	mdelay(550);
	return ret;
}

static int close(struct udevice *dev)
{
	struct vs_gpio		*plat	= dev_get_platdata(dev);
	int			ret	= 0;

	/* power-down sequence */
	ret = dm_gpio_set_value(&plat->rst_port, 0);
	mdelay(20);

	return ret;
}

static int tune(struct udevice *dev, int camera_type, int mode)
{
	struct vs_i2c_reg	*list;
	int			entry	= 0;
	int			ret	= 0;

	entry	= sizeof(vs_i2c_reg_list_table) /
		sizeof(vs_i2c_reg_list_table[0]);
	if (entry <= 0 || mode < 0 || entry <= mode) {
		loge("entry(%d) or mode(%d) is wrong\n", entry, mode);
		ret = -1;
	} else {
		list = vs_i2c_reg_list_table[mode];
		ret = videosource_i2c_write_regs(dev, list);
		mdelay(100);
	}

	return ret;
}

static int dump(struct udevice *dev, int camera_type, int mode)
{
	struct vs_i2c_reg	*list;
	int			entry	= 0;
	int			ret	= 0;

	entry	= sizeof(vs_i2c_reg_list_table) /
		sizeof(vs_i2c_reg_list_table[0]);
	if (entry <= 0 || mode < 0 || entry <= mode) {
		loge("entry(%d) or mode(%d) is wrong\n", entry, mode);
		ret = -1;
	} else {
		list = vs_i2c_reg_list_table[mode];
		ret = videosource_i2c_read_regs(dev, list);
		mdelay(1);
	}

	return ret;
}

struct videosource videosource_isl79988 = {
	.interface	= VIDEOSOURCE_INTERFACE_CIF,

	.format		= {
		.width			= WIDTH,
		.height			= HEIGHT,
		.crop_x			= 30,
		.crop_y			= 5,
		.crop_w			= 30,
		.crop_h			= 5,
		.interlaced		= ON,
		.v_pol			= 0,
		.h_pol			= 0,
		.p_pol			= 0,
		.data_order		= ORDER_RGB,
		.data_format		= FMT_YUV422_8BIT,
		.bit_per_pixel		= 8,
		.gen_field_en		= OFF,
		.de_active_low		= ACT_LOW,
		.field_bfield_low	= OFF,
		.vs_mask		= OFF,
		.hsde_connect_en	= OFF,
		.intpl_en		= OFF,
		.conv_en		= ON,	/* OFF: BT.601 / ON: BT.656 */
		.se			= OFF,
		.fvs			= OFF,
	},

	.driver		= {
		.open			= open,
		.close			= close,
		.tune			= tune,
		.video_check		= NULL,
		.dump			= dump,
	},
};
