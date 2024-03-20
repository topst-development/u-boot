// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <debug.h>
#include "../include/switch.h"

#include <dm/uclass.h>
#include <dm/pinctrl.h>
#include <dm/read.h>

#include <asm/io.h>

struct udevice	*g_switch_dev;

int switch_if_init(void)
{
	struct udevice		*dev		= NULL;

	/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_11_9_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_2_violation : FALSE] */
	/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	uclass_foreach_dev_probe(UCLASS_GPIO, dev);

	return 0;
}

struct udevice *switch_if_get_switch(void)
{
	return g_switch_dev;
}

int switch_if_get_status(const struct udevice *dev)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	const struct switch_gpio	*plat		= dev_get_platdata(dev);

	int			gpio_value	= -1;
	int			gpio_active	= -1;
	int			status		= -1;

	/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
	gpio_value	= (int)!!dm_gpio_get_value(&plat->switch_gpio_desc);
	/* coverity[misra_c_2012_rule_10_8_violation : FALSE] */
	if (plat->switch_active < (unsigned int)INT_MAX) {
		gpio_active	= (int)plat->switch_active;
	}

	status = (int)(gpio_value == gpio_active);
	logd("value: %d, active: %d, status: %d\n",
		gpio_value, gpio_active, status);

	return status;
}

#if defined(CONFIG_DM)
static const struct udevice_id switch_ids[] = {
	{
		.compatible = "telechips,switch",
	},
	{ }
};

static int switch_probe(struct udevice *dev)
{
	int			ret		= 0;

	// reverse sw handle
	g_switch_dev = dev;

	return ret;
}

/* coverity[misra_c_2012_rule_2_7_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int switch_remove(struct udevice *dev)
{
	int			ret		= 0;

	return ret;
}

static int switch_ofdata_to_platdata(struct udevice *dev)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	struct switch_gpio	*plat		= dev_get_platdata(dev);
	int			ret		= 0;

	// gpio port configuration
	/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
	(void)gpio_request_by_name(dev,
		"switch-gpios", 0, &plat->switch_gpio_desc, GPIOD_IS_IN);
	ret = dev_read_u32(dev, "switch-active", &plat->switch_active);
	if (ret < 0) {
		// device node "switch-active" does not exist
		plat->switch_active = 0;
	}

	return ret;
}

/* coverity[cert_dcl37_c_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
/* coverity[misra_c_2012_rule_21_2_violation : FALSE] */
U_BOOT_DRIVER(sw) = {
	/* coverity[cert_str30_c_violation : FALSE] */
	.name				= "switch",
	.id				= UCLASS_GPIO,
	.of_match			= switch_ids,
	.probe				= switch_probe,
	.remove				= switch_remove,
	.ofdata_to_platdata		= switch_ofdata_to_platdata,
	.platdata_auto_alloc_size	= (s32)sizeof(struct switch_gpio),
};
#endif//defined(CONFIG_DM)
