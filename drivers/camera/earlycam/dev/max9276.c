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
#include <max9276.h>
#include <dm/device.h>
#include <dm/read.h>

#define I2C_CH_NO	7	/* I2C Master Number */
#define DES_ADDR	0x94	/* I2C Slave Address */
#define SER_ADDR	0x80
#define ISP_ADDR	0x30

#define DATA_TERM			(0xFFFFFFFF)

#define WIDTH	1920
#define HEIGHT	(1080 - 1)

struct max9276_i2c_reg_t {
	unsigned int saddr;
	unsigned int delay;
	unsigned int addr;
	unsigned int data[];
} max9276_i2c_reg_t;

static max9276_i2c_reg_t vs_i2c_reg_list_des_enable_local_ack = {
	.addr	= 0x1C,
	.data	= { 0xB6, DATA_TERM },
};

static max9276_i2c_reg_t vs_i2c_reg_list_des_boost_mode = {
	.addr	= 0x14,
	.data	= { 0x08, DATA_TERM },
};

static max9276_i2c_reg_t vs_i2c_reg_list_ser_config_mode = {
	.saddr	= SER_ADDR,
	.delay	= 0x7F,
	.addr	= 0x04,
	.data	= { 0x43, DATA_TERM },
};

static max9276_i2c_reg_t vs_i2c_reg_list_isp_init = {
	.saddr	= ISP_ADDR,
	.addr	= 0x0A,
	.data	= { 0x01, 0x07, 0x02, 0x01, 0x00,
		0x00, 0x30, 0x80, 0xC5, DATA_TERM },
};

static max9276_i2c_reg_t vs_i2c_reg_list_ser_stream_mode = {
	.saddr	= SER_ADDR,
	.delay	= 0x7F,
	.addr	= 0x04,
	.data	= { 0x83, DATA_TERM },
};

static max9276_i2c_reg_t vs_i2c_reg_list_des_disable_local_ack = {
	.addr	= 0x1C,
	.data	= { 0x36, DATA_TERM },
};

static max9276_i2c_reg_t *vs_i2c_reg_list_table[] = {
	&vs_i2c_reg_list_des_enable_local_ack,
	&vs_i2c_reg_list_des_boost_mode,
	&vs_i2c_reg_list_ser_config_mode,
	&vs_i2c_reg_list_isp_init,
	&vs_i2c_reg_list_ser_stream_mode,
	&vs_i2c_reg_list_des_disable_local_ack,
};

static int max9276_i2c_write_regs(struct udevice *dev,
	const max9276_i2c_reg_t *list)
{
	unsigned short		slave_addr	= DES_ADDR;
	int			idx_data	= 0;
	unsigned char		data[1024]	= {0,};
	int			data_bytes	= 0;
	int			ret		= 0;

	/* assign an i2c slave address */
	slave_addr = (list->saddr == 0) ? DES_ADDR : list->saddr;

	/* convert to a i2c config buffer */
	idx_data	= 0;
	data_bytes	= 0;
	/* add data */
	while (list->data[idx_data] != DATA_TERM) {
		/* rebuild config data */
		data[data_bytes++] = list->data[idx_data++];
	}

	/* i2c send */
	logd("slave address: 0x%x, reg addr: 0x%08x, data_bytes: %d\n",
		slave_addr, list->addr, data_bytes);
	ret = dm_i2c_write(dev, list->addr, data, data_bytes);
	if (ret != 0) {
		loge("i2c device name: %s, write error!!!!\n",
			dev_read_name(dev));
		goto end;
	}

	/* delay for applying */
	if (list->delay != 0) {
		logd("slave address: 0x%x, delay %dms\n",
			slave_addr, list->delay);
		mdelay(list->delay);
	}

end:
	return 0;
}

static int open(struct udevice *dev)
{
	struct vs_gpio		*plat		= dev_get_platdata(dev);
	int			ret		= 0;

	/* power-up sequence */
	ret = dm_gpio_set_value(&plat->rst_port, 0);
	mdelay(20);

	ret = dm_gpio_set_value(&plat->rst_port, 1);
	mdelay(20);

	return ret;
}

static int close(struct udevice *dev)
{
	struct vs_gpio		*plat		= dev_get_platdata(dev);
	int			ret		= 0;

	/* power-down sequence */
	ret = dm_gpio_set_value(&plat->rst_port, 0);
	mdelay(20);

	return ret;
}

static int tune(struct udevice *dev, int camera_type, int mode)
{
	struct udevice		*dev_des	= NULL;
	struct udevice		*dev_ser	= NULL;
	struct udevice		*dev_isp	= NULL;
	unsigned short		slave_addr	= DES_ADDR;
	int			entry		= 0;
	max9276_i2c_reg_t	*list		= NULL;
	int			ldx_list	= 0;
	int			ret		= 0;

	entry = sizeof(vs_i2c_reg_list_table) /
		sizeof(vs_i2c_reg_list_table[0]);
	if (entry <= 0 || mode < 0 || entry <= mode) {
		loge("entry(%d) or mode(%d) is wrong\n", entry, mode);
		return -1;
	}

	/* probe */
	dev_des = dev;
	ret = i2c_get_chip_for_busnum(I2C_CH_NO,
		(int)(DES_ADDR >> 1), (uint)1, &dev_des);
	if (ret != 0) {
		/* failed to probe i2c chip */
		loge("i2c_get_chip_for_busnum(0x%x), ret: %d\n", DES_ADDR, ret);
	}

	/* probe */
	dev_ser = dev;
	ret = i2c_get_chip_for_busnum(I2C_CH_NO,
		(int)(SER_ADDR >> 1), (uint)1, &dev_ser);
	if (ret != 0) {
		/* failed to probe i2c chip */
		loge("i2c_get_chip_for_busnum(0x%x), ret: %d\n", SER_ADDR, ret);
	}

	/* probe */
	dev_isp = dev;
	ret = i2c_get_chip_for_busnum(I2C_CH_NO,
		(int)(ISP_ADDR >> 1), (uint)1, &dev_isp);
	if (ret != 0) {
		/* failed to probe i2c chip */
		loge("i2c_get_chip_for_busnum(0x%x), ret: %d\n", ISP_ADDR, ret);
	}

	for (ldx_list = 0; ldx_list < entry; ldx_list++) {
		list = vs_i2c_reg_list_table[ldx_list];

		/* assign an i2c slave address */
		slave_addr = (list->saddr == 0) ? DES_ADDR : list->saddr;

		switch (slave_addr) {
		case DES_ADDR:
			ret = max9276_i2c_write_regs(dev_des, list);
			break;
		case SER_ADDR:
			ret = max9276_i2c_write_regs(dev_ser, list);
			break;
		case ISP_ADDR:
			ret = max9276_i2c_write_regs(dev_isp, list);
			break;
		}
	}

	return ret;
}

struct videosource videosource_max9276 = {
	.interface	= VIDEOSOURCE_INTERFACE_CIF,

	.format		= {
		.width			= WIDTH,
		.height			= HEIGHT,
		.crop_x			= 0,
		.crop_y			= 0,
		.crop_w			= 0,
		.crop_h			= 0,
		.interlaced		= OFF,
		.v_pol			= 0,
		.h_pol			= 0,
		.p_pol			= 0,
		.data_order		= ORDER_RGB,
		.data_format		= FMT_YUV422_16BIT,
		.bit_per_pixel		= 8,
		.gen_field_en		= OFF,
		.de_active_low		= ACT_LOW,
		.field_bfield_low	= OFF,
		.vs_mask		= OFF,
		.hsde_connect_en	= ON,
		.intpl_en		= OFF,
		.conv_en		= OFF,	/* OFF: BT.601 / ON: BT.656 */
		.se			= OFF,
		.fvs			= OFF,
	},

	.driver		= {
		.vs_open		= open,
		.vs_close		= close,
		.vs_tune		= tune,
		.vs_video_check		= NULL,
		.vs_dump		= NULL,
	},
};
