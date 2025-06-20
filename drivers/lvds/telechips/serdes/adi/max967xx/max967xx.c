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

#define SERDES_I2C_PORT 0

#define MAX96737_SER_ADDR 0x20
#define MAX96752_DES_ADDR 0x48

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
	{MAX96752_DES_ADDR, 0x0002, 0x03}, // VIDEO Off, Reverse Audio Off
	{MAX96752_DES_ADDR, 0x0458, 0x28}, // Phy Opt
	{MAX96752_DES_ADDR, 0x0459, 0x68}, // Phy Opt
	{MAX96752_DES_ADDR, 0x01C8, 0x38}, // HS low
	{MAX96752_DES_ADDR, 0x01C9, 0x39}, // VS Low
	{MAX96752_DES_ADDR, 0x01CA, 0x3A}, // DE low
	{MAX96752_DES_ADDR, 0x01CE, 0x4E}, // oLDI Config
	{0xFF,              0x0000, 0x02}, // 2msec delay tiem
	{MAX96752_DES_ADDR, 0x0140, 0x21}, // Forward Audio En
	{0xFF,              0x0000, 0x01}, // Delay

	{MAX96752_DES_ADDR, 0x0002, 0x43},	// Video On
	{0xFF,              0x0000, 0x32},	 // Delay
	{MAX96752_DES_ADDR, 0x0002, 0x03},  // Video Off
	{MAX96752_DES_ADDR, 0x0D00, 0xF4}, //Video Swap Preventing
	{MAX96752_DES_ADDR, 0x0002, 0x43}, //Video Swap Preventing

	{MAX96752_DES_ADDR, 0x0D00, 0xF5}, //Video Swap Preventing
	{MAX96752_DES_ADDR, 0x01C8, 0x18}, //Video Swap Preventing
	{MAX96752_DES_ADDR, 0x01C9, 0x19}, //Video Swap Preventing
	{MAX96752_DES_ADDR, 0x01CA, 0x1A}, //Video Swap Preventing
	{MAX96752_DES_ADDR, 0x0D03, 0x8B}, //Video Swap Preventing
	{MAX96752_DES_ADDR, 0x0215, 0x04}, //SERDES GPIO tunneling
	{MAX96752_DES_ADDR, 0x0002, 0x43},	// Video On
	{MAX96752_DES_ADDR, 0x0001, 0x12},	// PT1_I2C En
	{MAX96737_SER_ADDR, 0x0001, 0x48},	// PT1_I2C En
	{MAX96737_SER_ADDR, 0x0700, 0x8B},	// Video On, Forward Audio En
	{MAX96737_SER_ADDR, 0x0760, 0x8B},	// Video On, Forward Audio En
	{MAX96737_SER_ADDR, 0x0002, 0x37},	// Video On, Forward Audio En

	/* GPIO Setting*/
	// DISP1_LCD_BL_EN
	// MFP0 -> MFP2 : ID0
	{MAX96737_SER_ADDR, 0x02BE, 0x99},
	{MAX96737_SER_ADDR, 0x02C0, 0x60},
	{MAX96752_DES_ADDR, 0x0206, 0x84},
	{MAX96752_DES_ADDR, 0x0208, 0x40},
	{MAX96737_SER_ADDR, 0x02BE, 0x93},
	// DISP1_LCD_BL_EN
	// DISP1_LCD_On
	// MFP1 -> MFP4 : ID1
	{MAX96737_SER_ADDR, 0x02C1, 0x99},
	{MAX96737_SER_ADDR, 0x02C3, 0x60},
	{MAX96752_DES_ADDR, 0x020C, 0x84},
	{MAX96752_DES_ADDR, 0x020E, 0x41},
	{MAX96737_SER_ADDR, 0x02C1, 0x93},
	// DISP1_LCD_On
	// DISP1_LCD_RST
	// MFP2 -> MFP7 : ID2
	{MAX96737_SER_ADDR, 0x02C4, 0x99},
	{MAX96737_SER_ADDR, 0x02C6, 0x60},
	{MAX96752_DES_ADDR, 0x0215, 0x84},
	{MAX96752_DES_ADDR, 0x0217, 0x42},
	{MAX96737_SER_ADDR, 0x02C4, 0x93},
	// DISP1_LCD_RST
	// DISP1_TSC_IRQ
	// MFP5 <- MFP8 : ID8
	{MAX96752_DES_ADDR, 0x0218, 0x99},
	{MAX96752_DES_ADDR, 0x021A, 0x60},
	{MAX96737_SER_ADDR, 0x02CD, 0x84},
	{MAX96737_SER_ADDR, 0x02CF, 0x68},
	{MAX96752_DES_ADDR, 0x0218, 0x93},
	// DISP1_TSC_IRQ
	// DISP1_TSC_RST#
	// MFP12 -> MFP10 : ID12
	{MAX96737_SER_ADDR, 0x02E2, 0x99},
	{MAX96737_SER_ADDR, 0x02E4, 0x60},
	{MAX96752_DES_ADDR, 0x021E, 0x84},
	{MAX96752_DES_ADDR, 0x0220, 0x4C},
	{MAX96737_SER_ADDR, 0x02E2, 0x93},
	// DISP1_TSC_RST#

	{MAX96737_SER_ADDR, 0x0003, 0x08},
	{MAX96737_SER_ADDR, 0x0560, 0xE0},

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
					      2,
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

		if (pstserdes[loop].devaddr == MAX96737_SER_ADDR) {
			i2c_dev = i2c_serdes->ser;
		} else if (pstserdes[loop].devaddr == MAX96752_DES_ADDR) {
			i2c_dev = i2c_serdes->des;
		} else {
			// If you want to add the time delay, use this space.
			mdelay(pstserdes[loop].value);
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

	int ret =  lvds_set_serdes_busnum(&i2c_serdes.ser, MAX96737_SER_ADDR);
	if ((bool)ret) {
		debug_pr(LOGL_ERR, "[%s:%d] Ser i2c dev set fail\n", __func__, __LINE__);
	} else {
		ret = lvds_set_serdes_busnum(&i2c_serdes.des, MAX96752_DES_ADDR);
		if ((bool)ret) {
			debug_pr(LOGL_ERR, "[%s:%d] Des i2c dev set fail\n", __func__, __LINE__);
		} else {
			lvds_set_serdes_program(&i2c_serdes);
		}
	}

	debug_pr(LOGL_INFO, "[%d][%s] Done\n", __LINE__, __func__);
}

