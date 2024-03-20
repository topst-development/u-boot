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
#include <adv7182.h>
#include <dm/device.h>
#include <dm/read.h>

#define I2C_CH_NO	7	/* I2C Master Number */
#define I2C_ADDR	0x42	/* I2C Slave Address */

#define WIDTH		720
#define HEIGHT		480

static struct vs_i2c_reg vs_i2c_reg_initialize[] = {
	{0x0f, 0x00},	/* 42 0F 00 ; Exit Power Down Mode */
	{0x00, 0x00},	/* 42 00 00 ; INSEL = CVBS in on Ain 1 */
	{0x03, 0x0c},	/* 42 03 0C ; Enable Pixel & Sync output drivers */
	{0x04, 0x17},	/* 42 04 17 ; Power-up INTRQ pad & Enable SFL */
	{0x13, 0x00},	/* 42 13 00 ; Enable INTRQ output driver */
	{0x17, 0x41},	/* 42 17 41 ; select SH1 */
	{0x1d, 0x40},	/* 42 1D 40 ; Enable LLC output driver */
	{0x52, 0xcb},	/* 42 52 CB ; ADI Recommended Writes */
	{0x0e, 0x80},	/* 42 0E 80 ; ADI Recommended Writes */
	{0xd9, 0x44},	/* 42 D9 44 ; ADI Recommended Writes */
	{0x0e, 0x00},	/* 42 0E 00 ; ADI Recommended Writes */
	{0x0e, 0x40},	/* 42 0E 40 ; Select User Sub Map 2 */
	{0xe0, 0x01},	/* 42 E0 01 ; Select fast Switching Mode */
	{0x0e, 0x00},	/* 42 0E 00 ; Select User Map */

	{ADDR_TERM, DATA_TERM}
};

static struct vs_i2c_reg *vs_i2c_reg_list_table[] = {
	vs_i2c_reg_initialize,
};

static int open(struct udevice *dev)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	const struct vs_gpio	*plat	= dev_get_platdata(dev);
	int			busnum;
	unsigned int		slave_addr;
	int			chip_addr;
	uint			offset_len;
	int			ret	= 0;

	/* power-up sequence */
	ret = dm_gpio_set_value(&plat->rst_port, 0);
	mdelay(20);

	ret = dm_gpio_set_value(&plat->rst_port, 1);
	mdelay(20);

	/* probe */
	busnum		= I2C_CH_NO;
	slave_addr	= (unsigned int)I2C_ADDR >> 1U;
	chip_addr	= (int)slave_addr;
	offset_len	= 1;
	ret = i2c_get_chip_for_busnum(busnum, chip_addr, offset_len, &dev);
	if (ret != 0) {
		/* failed to probe i2c chip */
		loge("i2c_get_chip_for_busnum(0x%x), ret: %d\n",
			(unsigned int)I2C_ADDR, ret);
	}

	return ret;
}

static int close(const struct udevice *dev)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	const struct vs_gpio	*plat	= dev_get_platdata(dev);
	int			ret	= 0;

	/* power-down sequence */
	ret = dm_gpio_set_value(&plat->rst_port, 0);
	mdelay(20);

	return ret;
}

static int tune(struct udevice *dev, int type, int mode)
{
	const struct vs_i2c_reg	*list;
	int			entry	= 0;
	int			ret	= 0;

	(void)type;
	entry	= (int)(sizeof(vs_i2c_reg_list_table) /
		sizeof(vs_i2c_reg_list_table[0]));
	if ((entry <= 0) || (mode < 0) || (entry <= mode)) {
		loge("entry(%d) or mode(%d) is wrong\n", entry, mode);
		ret = -1;
	} else {
		list = vs_i2c_reg_list_table[mode];
		ret = videosource_i2c_write_regs(dev, list);
		mdelay(100);
	}

	return ret;
}

static int dump(struct udevice *dev, int type, int mode)
{
	const struct vs_i2c_reg	*list;
	int			entry	= 0;
	int			ret	= 0;

	(void)type;
	entry	= (int)(sizeof(vs_i2c_reg_list_table) /
		sizeof(vs_i2c_reg_list_table[0]));
	if ((entry <= 0) || (mode < 0) || (entry <= mode)) {
		loge("entry(%d) or mode(%d) is wrong\n", entry, mode);
		ret = -1;
	} else {
		list = vs_i2c_reg_list_table[mode];
		ret = videosource_i2c_read_regs(dev, list);
		mdelay(1);
	}

	return ret;
}

struct videosource videosource_adv7182 = {
	.interface	= (int)VIDEOSOURCE_INTERFACE_CIF,

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

	.drv		= {
		.vs_open		= open,
		.vs_close		= close,
		.vs_tune		= tune,
		.vs_video_check		= NULL,
		.vs_dump		= dump,
	},
};
