// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <lcd.h>
#include <display.h>
#include <dm.h>
#include <edid.h>
#include <panel.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/pinctrl.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <asm/arch/vioc/vioc_disp.h>
#include <asm/arch/vioc/reg_physical.h>
#include <telechips/fb_dm.h>
#include <hdmiv20_ctrl.h>

DECLARE_GLOBAL_DATA_PTR;

struct tcc_hdmi_priv {
	struct udevice *panel; //unimplemented
	struct udevice *fb_dev;
	unsigned int hdmi_vic;
};

static int tcc_hdmi_init(struct udevice *dev)
{
	struct tcc_hdmi_priv *priv = dev_get_priv(dev);
	struct tcc_fb_dm_priv *fb_priv = dev_get_priv(priv->fb_dev);
	int ret = 0;

	#if defined(CONFIG_HDMI_PANEL_MODE)
	ret = pinctrl_select_state(dev, "default");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
		ret = -EINVAL;
	}
	/* power off, reset on, blk off */
	ret = pinctrl_select_state(dev, "power_off");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
		ret = -EINVAL;
	}
	#endif

	hdmiv20_panel_init(fb_priv->lcd_id, priv->hdmi_vic);

	return ret;
}

int tcc_hdmi_enable(struct udevice *dev, int panel_bpp,
		   const struct display_timing *timings)
{
	struct tcc_hdmi_priv *priv = dev_get_priv(dev);
	struct tcc_fb_dm_priv *fb_priv = dev_get_priv(priv->fb_dev);
	int enable = 1;
	int ret = 0;

	#if defined(CONFIG_HDMI_PANEL_MODE)
	ret = pinctrl_select_state(dev, "pwr_on_1");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
		ret = -EINVAL;
	}

	udelay(20);

	ret = pinctrl_select_state(dev, "pwr_on_2");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
		ret = -EINVAL;
	}
	#endif

	hdmiv20_set_power(enable, fb_priv->lcd_id,
		fb_priv->image_info.image_width,
		fb_priv->image_info.image_height);

	//panel chip setting
	#if defined(CONFIG_HDMI_PANEL_MODE)
	mdelay(50); //for evb panel
	hdmiv20_set_panel_ctrl(enable);

	ret = pinctrl_select_state(dev, "blk_on");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
		ret = -EINVAL;
	}
	#endif

	return 0;
}

int tcc_hdmi_read_timing(struct udevice *dev, struct display_timing *timing)
{
	int ret = 0;

	//used for image size
	timing->hactive.typ = 640;
	timing->vactive.typ = 480;

	#if defined(CONFIG_TCC_FB_USES_SCALER)
	timing->hactive.typ = 1920;
	timing->vactive.typ = 1080;
	#endif
	return ret;
}

static int tcc_hdmi_of_to_plat(struct udevice *dev)
{
	struct tcc_hdmi_priv *priv = dev_get_priv(dev);
	struct tcc_fb_dm_priv *fb_priv;
	struct udevice *fb_dev;
	int vic;
	int ret = 0;

	u32 remote_phandle;
	ofnode ports, port, node, remote;

	/* video code */
	ret = ofnode_read_u32(dev->node_, "vic", &vic);
	if (ret < 0) {
		pr_err("[ERR][%s](%s) :couldn't read lvds_type[mode]\n",
		       __func__, dev_read_name(dev));
		return -EINVAL;
	}
	priv->hdmi_vic = vic;

	/* find fb_priv */
	ports = dev_read_subnode(dev, "ports");
	if (!ofnode_valid(ports)) {
		ports = dev_ofnode(dev);
	}
	port = ofnode_find_subnode(ports, "port");
	if (!ofnode_valid(port)) {
		pr_err("[ERROR][%s](%s): 'port' subnode not found\n",
		      __func__, dev_read_name(dev));
		return -EINVAL;
	}

	node = ofnode_first_subnode(port);
	ret = ofnode_read_u32(node, "remote-endpoint", &remote_phandle);
	if (ret) {
		pr_err("[ERROR][%s](%s) : ret = %d\n",
		       __func__,dev_read_name(dev), ret);
		return ret;
	}

	remote = ofnode_get_by_phandle(remote_phandle);

	if(!ofnode_valid(remote)) {
		pr_err("[ERROR][%s](%s) : node not valid\n",
		       __func__, dev_read_name(dev));
		return -EINVAL;
	}

	while(ofnode_valid(remote)) {
		remote = ofnode_get_parent(remote);
		if (!ofnode_valid(remote)) {
			pr_err("[ERROR][%s](%s): no UCLASS_VIDEO for remote-endpoint\n",
			      __func__, dev_read_name(dev));
			return -EINVAL;
		}
		uclass_find_device_by_ofnode(UCLASS_VIDEO, remote, &fb_dev);
		if (fb_dev) {
			break;
		}
	}
	priv->fb_dev = fb_dev;
	fb_priv = (struct tcc_fb_dm_priv *)fb_dev->priv_;
	fb_priv->media_id = PANEL_HDMI;

	return ret;
}

int tcc_hdmi_probe(struct udevice *dev)
{
	/* struct tcc_hdmi_priv *priv = dev_get_priv(dev); */
	int ret;

	ret = tcc_hdmi_init(dev);

	return ret;
}

static const struct dm_display_ops hdmi_tcc_ops = {
	.read_timing = tcc_hdmi_read_timing,
	.enable = tcc_hdmi_enable,
};

static const struct udevice_id tcc_hdmi_ids[] = {
	{.compatible = "telechips,fb-hdmi_v20"},
	{}
};

U_BOOT_DRIVER(hdmi_tcc) = {
	.name	= "hdmi_tcc",
	.id	= UCLASS_DISPLAY,
	.of_match = tcc_hdmi_ids,
	.ops	= &hdmi_tcc_ops,
	.of_to_plat	= tcc_hdmi_of_to_plat,
	.probe	= tcc_hdmi_probe,
	.priv_auto	= sizeof(struct tcc_hdmi_priv),
};
