// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <debug.h>
#include <videosource_if.h>
#include <i2c.h>
#include <adv7182.h>
#include <isl79988.h>
#include <max9276.h>
#if defined(CONFIG_DM)
#include <dm/device.h>
#include <dm/uclass.h>
#include <dm/pinctrl.h>
#include <dm/read.h>
#endif/* defined(CONFIG_DM) */
#include <asm/gpio.h>
#include <precision.h>

struct udevice	*g_dev;

int videosource_i2c_read_reg(struct udevice *dev,
	uint slave_addr, uint *data, uchar data_bytes)
{
	uchar			buf[4]	= {0,};
	uchar			idx_buf	= 0;
	uchar			shift	= 0;
	uint			udata	= 0;
	int			ret	= 0;

	ret = dm_i2c_read(dev, slave_addr, buf, (int)data_bytes);
	if (ret != 0) {
		loge("i2c device name: %s, slave_addr: 0x%08x, read error\n",
			dev_read_name(dev), slave_addr);
	}

	/* convert data to big / little endia */
	*data = 0;
	for (idx_buf = 0; idx_buf < data_bytes; idx_buf++) {
		/* rebuild i2c data */
		udata = buf[idx_buf];
		shift = data_bytes - idx_buf;

		if (shift > 0U) {
			shift = shift - 1U;

			if (shift < PRECISION(255U)) {
				*data |= (udata << shift);
			}
		}
	}

	logi("slave_addr: 0x%08x, data: 0x%08x\n", slave_addr, *data);

	return ret;
}

int videosource_i2c_write_reg(struct udevice *dev,
	uint slave_addr, int addr_bytes, uint data, int data_bytes)
{
	uchar			buf[8]	= {0,};
	uchar			*p_buf	= NULL;
	unsigned		shift	= 0;
	int			idx_buf	= 0;
	int			ret	= 0;
	int			t_diff	= 0;

	logd("slave_addr: 0x%08x, addr_bytes: %d, data: 0x%08x\n",
		slave_addr, addr_bytes, data);

	/* convert slave_addr to i2c byte stream */
	p_buf = buf;
	for (idx_buf = 0; idx_buf < data_bytes; idx_buf++) {
		/* rebuild i2c data */
		t_diff = data_bytes - idx_buf;
		if ((t_diff <= 31) && (t_diff >= 0)) {
			shift = (unsigned)t_diff;
		} else {
			loge("precision error on setting shift variable.\n");
			break;
		}

		if (shift > 0U) {
			shift = shift - 1U;

			if (shift < PRECISION(255U)) {
				*p_buf = (uchar)((data >> shift) & 0xFFU);
			}

			p_buf++;
		}

	}

	ret = dm_i2c_write(dev, slave_addr, buf, data_bytes);
	if (ret != 0) {
		loge("i2c device name: %s, slave_addr: 0x%08x, write error!!!!\n",
			dev_read_name(dev), slave_addr);
		ret = -1;
	}

	return ret;
}

int videosource_i2c_read_regs(struct udevice *dev,
	const struct vs_i2c_reg *list)
{
	uint			data		= 0;
	uchar			data_bytes	= 0;
	int			ret		= 0;

	data_bytes	= 1;
	while (!((list->slave_addr == (unsigned)ADDR_TERM)
		&& (list->data == (unsigned)DATA_TERM))) {
		ret = videosource_i2c_read_reg(dev,
			list->slave_addr, &data, data_bytes);
		if (ret == 1) {
			loge("i2c device name: %s, read error!!!!\n",
				dev_read_name(dev));
			break;
		}
		list++;
	}

	return 0;
}

int videosource_i2c_write_regs(struct udevice *dev,
	const struct vs_i2c_reg *list)
{
	int			addr_bytes	= 0;
	int			data_bytes	= 0;
	int			ret		= 0;

	addr_bytes	= 1;
	data_bytes	= 1;
	while (!((list->slave_addr == (unsigned)ADDR_TERM)
		&& (list->data == (unsigned)DATA_TERM))) {
		ret = videosource_i2c_write_reg(dev,
			list->slave_addr, addr_bytes, list->data, data_bytes);
		if (ret == 1) {
			loge("i2c device name: %s, write error!!!!\n",
				dev_read_name(dev));
			break;
		}
		list++;
	}

	return 0;
}

int videosource_if_init(void)
{
	struct udevice		*dev	= NULL;

	/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_11_9_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_2_violation : FALSE] */
	/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	uclass_foreach_dev_probe(UCLASS_I2C_GENERIC, dev);

	return 0;
}

struct udevice *videosource_if_get_videosource(void)
{
	return g_dev;
}

int videosource_if_open(struct udevice *dev)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	const struct videosource	*priv	= dev_get_priv(dev);
	const struct vs_if		*drv	= &priv->drv;
	int				ret	= 0;

	if (drv->vs_open == NULL) {
		loge("drv->open is NULL\n");
		ret = -1;
	} else {
		/* drv->open is available */
		ret = drv->vs_open(dev);
	}

	return ret;
}

int videosource_if_close(const struct udevice *dev)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	const struct videosource	*priv	= dev_get_priv(dev);
	const struct vs_if		*drv	= &priv->drv;
	int				ret	= 0;

	if (drv->vs_close == NULL) {
		loge("drv->close is NULL\n");
		ret = -1;
	} else {
		/* driver->close is available */
		ret = drv->vs_close(dev);
	}

	return ret;
}

int videosource_if_tune(struct udevice *dev, int type, int camera_encode)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	const struct videosource	*priv	= dev_get_priv(dev);
	const struct vs_if		*drv	= &priv->drv;
	int				ret	= 0;

	if (drv->vs_tune == NULL) {
		ret = 1;
	} else {
		/* drv->tune is available */
		ret = drv->vs_tune(dev, type, camera_encode);
	}

	return ret;
}

int videosource_if_video_check(struct udevice *dev)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	const struct videosource	*priv	= dev_get_priv(dev);
	const struct vs_if		*drv	= &priv->drv;
	int				ret	= 0;

	if (drv->vs_video_check == NULL) {
		logd("drv->vs_video_check is NULL\n");
		ret = -1;
	} else {
		ret = drv->vs_video_check(dev);
		if (ret == 1) {
			/* vs is working */
			logd("videosource is working\n");
		}
	}

	return ret;
}

int videosource_if_dump(struct udevice *dev, int type, int camera_encode)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	const struct videosource	*priv	= dev_get_priv(dev);
	const struct vs_if		*drv	= &priv->drv;
	int				ret	= 0;

	if (drv->vs_dump == NULL) {
		loge("drv->vs_dump is NULL\n");
		ret = -1;
	} else {
		/* drv->vs_dump is available */
		ret = drv->vs_dump(dev, type, camera_encode);
	}

	return ret;
}

#if defined(CONFIG_DM)
static const struct udevice_id videosource_ids[] = {
#if defined(CONFIG_EARLYCAMERA_SOLUTION_VIDEODECODER_ANALOGDEVICES_ADV7182)
	{
		.compatible = "analogdevices,adv7182",
		.data		= (ulong)&videosource_adv7182,
	},
#elif defined(CONFIG_EARLYCAMERA_SOLUTION_VIDEODECODER_INTERSIL_ISL79988)
	{
		.compatible = "intersil,isl79988",
		.data		= (ulong)&videosource_isl79988,
	},
#elif defined(CONFIG_EARLYCAMERA_SOLUTION_DESERIALIZER_MAXIM_MAX9276)
	{
		.compatible	= "maxim,max9276",
		.data		= (ulong)&videosource_max9276,
	},
#endif
	{ }
};

static int videosource_probe(struct udevice *dev)
{
	struct videosource	*priv	= NULL;
	int			ret	= 0;

	/* set private data */
	uintptr_t data_addr = (uintptr_t)dev->driver->of_match->data;
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	dev->priv = (void *)data_addr;

	/* cif port configuration */
	ret = pinctrl_select_state(dev, "active");
	if (ret < 0) {
		loge("[ERROR][VS]: failed to get pinctrl (active)\n");
		ret = -EINVAL;
	}

	/* cif port mapping */
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	priv	= dev_get_priv(dev);
	/* coverity[misra_c_2012_rule_11_3_violation : FALSE] */
	(void)dev_read_u32(dev, "cifport", (u32 *)&priv->format.cif_port);

	/* reverse videosource handle */
	g_dev = dev;

	return ret;
}

static int videosource_remove(struct udevice *dev)
{
	int			ret	= 0;

	ret = pinctrl_select_state(dev, "idle");
	if (ret < 0) {
		loge("failed to get pinctrl(idle)\n");
		ret = -EINVAL;
	}

	return ret;
}

static int videosource_ofdata_to_platdata(struct udevice *dev)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	struct vs_gpio		*plat	= dev_get_platdata(dev);
	int			ret	= 0;

	/* gpio port configuration for power-up/down sequence */
	/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
	(void)gpio_request_by_name(dev,
		"pwr-gpios", 0, &plat->pwr_port, GPIOD_IS_OUT);
	/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
	(void)gpio_request_by_name(dev,
		"pwd-gpios", 0, &plat->pwd_port, GPIOD_IS_OUT);
	/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
	(void)gpio_request_by_name(dev,
		"rst-gpios", 0, &plat->rst_port, GPIOD_IS_OUT);

	return ret;
}

/* coverity[cert_dcl37_c_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
/* coverity[misra_c_2012_rule_21_2_violation : FALSE] */
U_BOOT_DRIVER(videosource) = {
	/* coverity[cert_str30_c_violation : FALSE] */
	.name				= "videosource",
	.id				= UCLASS_I2C_GENERIC,
	.of_match			= videosource_ids,
	.probe				= videosource_probe,
	.remove				= videosource_remove,
	.ofdata_to_platdata		= videosource_ofdata_to_platdata,
	.priv_auto_alloc_size		= (int)sizeof(struct videosource),
	.platdata_auto_alloc_size	= (int)sizeof(struct vs_gpio),
};
#endif/* defined(CONFIG_DM) */
