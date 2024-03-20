// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef VIDEOSOURCE_IF_H
#define VIDEOSOURCE_IF_H

#include <asm/gpio.h>
#include <errno.h>

#if defined(CONFIG_DM)
#include <dm/device.h>
#endif/* defined(CONFIG_DM) */

enum camera_type {
	CAM_TYPE_DEFAULT	= 0,
	CAM_TYPE_CVBS		= 0,
	CAM_TYPE_SVIDEO,
	CAM_TYPE_COMPONENT,
	CAM_TYPE_AUX,
	CAM_TYPE_CMMB,
	CAM_TYPE_LVDS,
	CAM_TYPE_MAX
};

enum camera_enc {
	CAM_ENC_DEFAULT		= 0,
	CAM_ENC_NTSC		= 0,
	CAM_ENC_PAL,
	CAM_ENC_MAX
};

#define	ADDR_TERM		0xFF
#define	DATA_TERM		0xFF

struct vs_i2c_reg {
	/* coverity [misra_c_2012_rule_5_9_violation : FALSE] */
	unsigned short		slave_addr;
	unsigned short		data;
};

enum vs_interface {
	VIDEOSOURCE_INTERFACE_CIF,
	VIDEOSOURCE_INTERFACE_MIPI,
};

#ifndef	ON
#define	ON			1
#endif
#ifndef	OFF
#define	OFF			0
#endif

#ifndef	ACT_HIGH
#define	ACT_HIGH		1
#endif
#ifndef	ACT_LOW
#define	ACT_LOW			0
#endif

#ifndef	NEGATIVE_EDGE
#define NEGATIVE_EDGE		1
#endif
#ifndef	POSITIVE_EDGE
#define	POSITIVE_EDGE		0
#endif

struct vs_gpio {
	struct gpio_desc	pwr_port;	/* gpio for power */
	struct gpio_desc	pwd_port;	/* gpio for power-down */
	struct gpio_desc	rst_port;	/* gpio for reset */
};

struct vs_format {
	int			cif_port;

	unsigned int		width;
	unsigned int		height;
	unsigned int		crop_x;
	unsigned int		crop_y;
	unsigned int		crop_w;
	unsigned int		crop_h;
	unsigned int		interlaced;
	unsigned int		v_pol;
	unsigned int		h_pol;
	unsigned int		p_pol;
	unsigned int		data_order;	/* data order for vin */
	unsigned int		data_format;	/* data format for vin */
	unsigned int		bit_per_pixel;	/* 8 bit / 16 bit / 24 bit */
	unsigned int		gen_field_en;
	unsigned int		de_active_low;
	unsigned int		field_bfield_low;
	unsigned int		vs_mask;
	unsigned int		hsde_connect_en;
	unsigned int		intpl_en;
	unsigned int		conv_en;	/* OFF: BT.601 / ON: BT.656 */
	unsigned int		se;
	unsigned int		fvs;
};

struct vs_if {
	int (*vs_open)(struct udevice *dev);
	int (*vs_close)(const struct udevice *dev);
	int (*vs_tune)(struct udevice *dev,
		int type, int camera_encode);
	int (*vs_video_check)(struct udevice *dev);
	int (*vs_dump)(struct udevice *dev,
		int type, int camera_encode);
};

struct videosource {
	int			id;
	int			interface;
	struct vs_format	format;
	struct vs_if		drv;
};

extern struct udevice *g_dev;

#if defined(CONFIG_DM)
extern int videosource_i2c_read_reg(struct udevice *dev,
	uint slave_addr, uint *data, uchar data_bytes);
extern int videosource_i2c_write_reg(struct udevice *dev,
	uint slave_addr, int addr_bytes, uint data, int data_bytes);
extern int videosource_i2c_read_regs(struct udevice *dev,
	const struct vs_i2c_reg *list);
extern int videosource_i2c_write_regs(struct udevice *dev,
	const struct vs_i2c_reg *list);
#endif/* defined(CONFIG_DM) */

extern int videosource_if_init(void);
#if defined(CONFIG_DM)
extern struct udevice *videosource_if_get_videosource(void);
extern int videosource_if_open(struct udevice *dev);
extern int videosource_if_close(const struct udevice *dev);
extern int videosource_if_tune(struct udevice *dev,
	int type, int camera_encode);
extern int videosource_if_video_check(struct udevice *dev);
extern int videosource_if_dump(struct udevice *dev,
	int type, int camera_encode);
#endif/*defined(CONFIG_DM) */

#endif/* VIDEOSOURCE_IF_H */
