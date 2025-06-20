// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */


#include <config.h>
#include <common.h>
#include <linux/compat.h>

#include <dm/uclass.h>

#include <linux/delay.h>
#include <i2c.h>
#include <dm.h>

#include <lvds_serdes.h>

#define SERDES_I2C_PORT 1

#define DS90UB947_SER_ADDR 0x0C
#define DS90UB948_DES_ADDR 0x2C

#define RESET_SERIALIZER
#define RESET_DESERIALIZER
//#define PATTERN_GENARATOR

#define debug_pr(level, fmt...) (CONFIG_LOGLEVEL > (signed)level) ? (void)printf(fmt) : 0;

struct serdes_i2c_dev{
	struct udevice *ser;
	struct udevice *des;
};

struct serdes_reg {
	int devaddr;
	unsigned int regaddr;
	uint8_t value;
};

struct serdes_reg stserdes[] = {
#ifdef RESET_SERIALIZER
	{DS90UB947_SER_ADDR, 0x01, 0x02}, //Reset
	{0xFF, 0xFF, 0xFF}, //delay
#endif
	{DS90UB947_SER_ADDR, 0x03, 0xDA}, //SER_GENERAL_CONFIGURATION I2C Pass-through
	{DS90UB947_SER_ADDR, 0x04, 0x90},
	//DE_GATE_RGB enabled 4-bit on: default value 0x80 : Humax setting
	{DS90UB947_SER_ADDR, 0x0E, 0x33}, //GPIO1_GPIO2_SER setting for Forward channel
	{DS90UB947_SER_ADDR, 0x0F, 0x03}, //GPIO3_SER setting for Forward channel
#ifdef PATTERN_GENARATOR
	{DS90UB947_SER_ADDR, 0x64, 0x15}, //Patten Generator Control default value 0x10
#endif
#ifdef RESET_DESERIALIZER
	{DS90UB948_DES_ADDR, 0x01, 0x02}, //Deserializer register reset
	{0xFF, 0xFF, 0xFF}, //delay
#endif
	{DS90UB948_DES_ADDR, 0x1E, 0x55}, // GPIO1_GPIO2_DES for Forward channel
	{DS90UB948_DES_ADDR, 0x1F, 0x05}, // GPIO3_DES for Forward channel
	{0, 0, 0}
};

int lvds_set_serdes_busnum(struct udevice **i2c_dev, int i2c_addr)
{
	int i2c_port = SERDES_I2C_PORT;
	int retry = 0;
	int ret = 0;

	if (i2c_port < 0) {
		debug_pr(LOGL_ERR, "[ERR][I2C] not valid Port\n");
		return -EINVAL;
	}

	ret = uclass_get_device_by_seq(UCLASS_I2C, i2c_port, i2c_dev);
	if (!ret) {
		ret = dm_i2c_set_bus_speed(*i2c_dev, 100000);
		if (ret) {
			debug_pr(LOGL_ERR,
				"\n[%s:%d]Error: from dm_i2c_set_bus_speed().. Ret(0x%x)\n",
				__func__, __LINE__, ret);
		}
	} else {
		debug_pr(LOGL_ERR,
			"\n[%s:%d]Error: can't get i2c bus %d\n",
			__func__, __LINE__, i2c_port);
		return -EINVAL;
	}
	for (retry = 100; retry >= 0 ; retry--) {
		ret = i2c_get_chip_for_busnum(i2c_port,
					      i2c_addr,
					      1,
					      i2c_dev);
		if (ret) {
			if (retry == 0) {
				debug_pr(LOGL_ERR, "[%d] Failed to get i2c handle for device addr 0x%x\n",
					__LINE__,
					i2c_addr);
				return -EINVAL;
			}
			mdelay(10);
		} else {
			if (retry != 100)
				debug_pr(LOGL_INFO, "retry %d - for addr 0x%x\n", retry,
					i2c_addr);
			break;
		}
	}
	return 0;
}

void lvds_set_serdes_program(struct serdes_i2c_dev *i2c_serdes)
{
	struct serdes_reg *pstserdes = stserdes;
	int loop;

	for (loop = 0; (pstserdes[loop].devaddr != 0); loop++) {

		struct udevice *i2c_dev;
		int ret = 0;

		if (pstserdes[loop].devaddr == DS90UB947_SER_ADDR) {
			i2c_dev = i2c_serdes->ser;
		} else if (pstserdes[loop].devaddr == DS90UB948_DES_ADDR) {
			if (!(bool)i2c_serdes->des) {
				ret = lvds_set_serdes_busnum(&i2c_serdes->des, DS90UB948_DES_ADDR);
				if ((bool)ret) {
					debug_pr(LOGL_ERR, "[%s:%d] Des i2c dev set fail\n", __func__, __LINE__);
					break;
				}
			}
			i2c_dev = i2c_serdes->des;
		} else {
			// If you want to add the time delay, use this space.
			mdelay(100);
			ret = -1;
		}

		if (!(bool)ret) {
			ret = dm_i2c_write(i2c_dev, pstserdes[loop].regaddr, &pstserdes[loop].value, 1);
			if ((bool)ret) {
				debug_pr(LOGL_ERR, "[%s:%d] i2c write fail[slave addr 0x%02x][reg 0x%04x][value 0x%02x]\n",
					__func__, __LINE__, pstserdes[loop].devaddr, pstserdes[loop].regaddr, pstserdes[loop].value);
			}
		}
	}
}

void lvds_set_serdes(void)
{
	struct serdes_i2c_dev i2c_serdes = {NULL,};

	int ret =  lvds_set_serdes_busnum(&i2c_serdes.ser, DS90UB947_SER_ADDR);
	if ((bool)ret) {
		debug_pr(LOGL_ERR, "[%s:%d] Ser i2c dev set fail\n", __func__, __LINE__);
	} else {
		lvds_set_serdes_program(&i2c_serdes);
	}

	debug_pr(LOGL_INFO, "[%d][%s] Done\n", __LINE__, __func__);
}


