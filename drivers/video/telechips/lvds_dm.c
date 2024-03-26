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
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
#include <telechips/lvds_ctrl.h>
#elif defined(CONFIG_TCC897X)
#include <asm/arch/vioc/vioc_lvds.h>
#endif
#include <asm/arch/vioc/vioc_disp.h>
#include <asm/arch/vioc/reg_physical.h>
#include <telechips/fb_dm.h>
#include <telechips/fb_dm_lcd_interface.h>

DECLARE_GLOBAL_DATA_PTR;

#define TCC_LVDS_OUTPUT_VESA24 0
#define TCC_LVDS_OUTPUT_JEIDA24 1
#define TCC_LVDS_OUTPUT_MAX 2

struct tcc_lvds_priv {
	struct udevice *panel; //unimplemented
	struct udevice *fb_dev;
	struct lvds_hw_info_t lvds_info;
	int use_evb_exp_gpio; //for tcc8030 evb exp gpio
};

static unsigned int
lvds_outformat[TCC_LVDS_OUTPUT_MAX][TXOUT_MAX_LINE][TXOUT_DATA_PER_LINE] = {
        /* LVDS vesa-24 format */
	{
		{TXOUT_G_D(0),TXOUT_R_D(5),TXOUT_R_D(4),TXOUT_R_D(3),TXOUT_R_D(2),TXOUT_R_D(1),TXOUT_R_D(0)},
		{TXOUT_B_D(1),TXOUT_B_D(0),TXOUT_G_D(5),TXOUT_G_D(4),TXOUT_G_D(3),TXOUT_G_D(2),TXOUT_G_D(1)},
		{TXOUT_DE    ,TXOUT_VS    ,TXOUT_HS    ,TXOUT_B_D(5),TXOUT_B_D(4),TXOUT_B_D(3),TXOUT_B_D(2)},
		{TXOUT_DUMMY ,TXOUT_B_D(7),TXOUT_B_D(6),TXOUT_G_D(7),TXOUT_G_D(6),TXOUT_R_D(7),TXOUT_R_D(6)}
	},
        /* LVDS jeida-24 format */
	{
		{TXOUT_G_D(2),TXOUT_R_D(7),TXOUT_R_D(6),TXOUT_R_D(5),TXOUT_R_D(4),TXOUT_R_D(3),TXOUT_R_D(2)},
		{TXOUT_B_D(3),TXOUT_B_D(2),TXOUT_G_D(7),TXOUT_G_D(6),TXOUT_G_D(5),TXOUT_G_D(4),TXOUT_G_D(3)},
		{TXOUT_DE	 ,TXOUT_VS	  ,TXOUT_HS    ,TXOUT_B_D(7),TXOUT_B_D(6),TXOUT_B_D(5),TXOUT_B_D(4)},
		{TXOUT_DUMMY ,TXOUT_B_D(1),TXOUT_B_D(0),TXOUT_G_D(1),TXOUT_G_D(0),TXOUT_R_D(1),TXOUT_R_D(0)}
	},
};

static int tcc_lvds_init(struct udevice *dev)
{
	int ret;

	ret = pinctrl_select_state(dev, "default");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
		ret = -EINVAL;
	}

	ret = pinctrl_select_state(dev, "power_off");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
		ret = -EINVAL;
	}

	ret = pinctrl_select_state(dev, "blk_off");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
		ret = -EINVAL;
	}

	return 0;
}

int tcc_lvds_enable(struct udevice *dev, int panel_bpp,
		   const struct display_timing *timings)
{
	struct tcc_lvds_priv *priv = dev_get_priv(dev);
	struct tcc_fb_dm_priv *fb_priv = dev_get_priv(priv->fb_dev);
	int ret = 0;

	fb_priv->lcd_pxdw = PIXEL_ENCODING_RGB;

#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	ret = pinctrl_select_state(dev, "pwr_on_1");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
	}

	udelay(20);

	ret = pinctrl_select_state(dev, "pwr_on_2");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
	}

	lvds_splitter_init(&priv->lvds_info);
	lcdc_mux_select(priv->lvds_info.lcdc_mux_id, fb_priv->lcd_id);
	lcdc_display_device_init(priv->fb_dev);
	lcdc_turn_on_display(priv->fb_dev);
	lvds_phy_init(&priv->lvds_info);

	#if defined(CONFIG_TCC803X)
	if (priv->lvds_info.lvds_type == PANEL_LVDS_DUAL) {
		mdelay(80); //requirement for tcc8030-evb_sv0.1 dual LVDS panel
	}
	#endif

	if (priv->use_evb_exp_gpio != -1) {
		pr_info("%s: exp_set_value\n",__func__);
		tcclcd_gpioexp_set_value(1);
	}

	ret = pinctrl_select_state(dev, "blk_on");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
	}
#elif defined(CONFIG_TCC897X)
	ret = pinctrl_select_state(dev, "stbyb_on");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
	}

	ret = pinctrl_select_state(dev, "pwr_on_2");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
	}

	ret = pinctrl_select_state(dev, "pwr_on_1");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
	}

	lcdc_display_device_init(priv->fb_dev);
	lcdc_turn_on_display(priv->fb_dev);
	//lvds init
	tcc_set_ddi_lvds_reset(1);
	tcc_set_ddi_lvds_data_arrary(priv->lvds_info.txout_main);
	tcc_set_ddi_lvds_config();
	tcc_set_ddi_lvds_pms(fb_priv->lcd_id, priv->lvds_info.p_clk, 1);

	mdelay(1);

	ret = pinctrl_select_state(dev, "blk_on");
	if (ret < 0) {
		pr_err("[ERROR][%s](%s) failed to select state\n",
		       __func__, dev_read_name(dev));
	}
#endif

	return 0;
}

int tcc_lvds_read_timing(struct udevice *dev, struct display_timing *timing)
{
	struct tcc_lvds_priv *priv = dev_get_priv(dev);
	int ret;

	ret = ofnode_decode_display_timing(dev_ofnode(dev), 0, timing);

	priv->lvds_info.xres = timing->hactive.typ;
	priv->lvds_info.p_clk = timing->pixelclock.typ;

	return ret;
}

static int tcc_lvds_ofdata_to_platdata(struct udevice *dev)
{
	struct tcc_lvds_priv *priv = dev_get_priv(dev);
	struct tcc_fb_dm_priv *fb_priv;
	struct udevice *fb_dev;
	int ret;
	const char *mapping;
	unsigned int lvds_format;

#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	int read_dt_arr[5];
	unsigned int dt_lvds_type;
	int num_val;
	int idx;
#endif

	u32 remote_phandle;
	ofnode ports, port, node, remote;

	mapping = ofnode_read_string(dev->node_, "data-mapping");
	if (mapping == NULL) {
		pr_err("[ERR][%s](%s) :couldn't read data-mapping\n",
		       __func__, dev_read_name(dev));
		ret = -EINVAL;
	}

        if (!strcmp(mapping, "vesa-24")) {
                lvds_format = TCC_LVDS_OUTPUT_VESA24;
	}

        memcpy(priv->lvds_info.txout_main, lvds_outformat[lvds_format],
                sizeof(priv->lvds_info.txout_main));

#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	ret = ofnode_read_u32(dev->node_, "mode", &dt_lvds_type);
	if (ret < 0) {
		pr_err("[ERR][%s](%s) :couldn't read lvds_type[mode]\n",
		       __func__, dev_read_name(dev));
	}

	if (dt_lvds_type == LVDS_DUAL) {
		priv->lvds_info.lvds_type = PANEL_LVDS_DUAL;
		num_val = 2;
	} else if (dt_lvds_type == LVDS_SINGLE) {
		priv->lvds_info.lvds_type = PANEL_LVDS_SINGLE;
		num_val = 1;
	} else {
		pr_err("[ERR][%s](%s) : invalid lvds_type = %d\n",
		       __func__, dev_read_name(dev), priv->lvds_info.lvds_type);
	}
	ret = ofnode_read_u32_array(dev->node_, "phy-ports", read_dt_arr, num_val);
	if (ret < 0) {
		pr_err("[ERR][%s](%s) :couldn't read phy-ports [%d]\n",
		       __func__, dev_read_name(dev), ret);
	}

	priv->lvds_info.port_main = read_dt_arr[0];

	if (priv->lvds_info.lvds_type == PANEL_LVDS_DUAL) {
		priv->lvds_info.port_sub = read_dt_arr[1];
		priv->lvds_info.ts_mux_id = 0x3U;
	} else {
		priv->lvds_info.port_sub = LVDS_PHY_PORT_MAX;
		priv->lvds_info.ts_mux_id = (priv->lvds_info.port_main + 2U) % 4U;
	}

	#if defined(CONFIG_TCC805X)
	ret = ofnode_read_u32(dev->node_, "lcdc-mux-bypass", &priv->lvds_info.lcdc_mux_bypass);
	if (ret < 0) {
		pr_err("[ERR][%s](%s) :couldn't read lcdc_mux_bypass\n",
		       __func__, dev_read_name(dev));
	}
	#endif

	num_val = 5;
	ret = ofnode_read_u32_array(dev->node_, "lane-main", read_dt_arr, num_val);
	if (ret < 0) {
		pr_err("[ERR][%s](%s) :couldn't read lane-main\n",
		       __func__, dev_read_name(dev));
	}

	for(idx = 0; idx < num_val; idx++) {
		priv->lvds_info.lane_main[idx] = read_dt_arr[idx];
	}

	if (dt_lvds_type == LVDS_DUAL) {
		ret = ofnode_read_u32_array(dev->node_, "lane-sub", read_dt_arr, num_val);
		if (ret < 0) {
			pr_err("[ERR][%s](%s) :couldn't read lane-sub\n",
			       __func__, dev_read_name(dev));
		}

		for(idx = 0; idx < num_val; idx++) {
			priv->lvds_info.lane_sub[idx] = read_dt_arr[idx];
		}
	}

	ret = ofnode_read_u32(dev->node_, "vcm", &priv->lvds_info.vcm);
	if (ret < 0) {
		pr_err("[ERR][%s](%s) :couldn't read vcm\n",
		       __func__, dev_read_name(dev));
	}

	ret = ofnode_read_u32(dev->node_, "vsw", &priv->lvds_info.vsw);
	if (ret < 0) {
		pr_err("[ERR][%s](%s) :couldn't read vsw\n",
		       __func__, dev_read_name(dev));
	}

        if (priv->lvds_info.lvds_type == PANEL_LVDS_DUAL) {
                memcpy(priv->lvds_info.txout_sub, lvds_outformat[lvds_format],
                        sizeof(priv->lvds_info.txout_sub));
        }

	ret = ofnode_read_u32(dev->node_, "tcc8030-evb-exp", &priv->use_evb_exp_gpio);
	if (ret == 0) {
		pr_info("[INFO][%s](%s) : tcc8030-evb-exp-gpio\n",
			__func__, dev_read_name(dev));
	} else { //exp gpio is not used
		ret = 0;
		priv->use_evb_exp_gpio = -1;
	}
#endif //#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)

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
	fb_priv->media_id = priv->lvds_info.lvds_type;

	priv->lvds_info.lcdc_mux_id = fb_priv->lcd_mux_id;
	if (priv->lvds_info.lcdc_mux_id < 0) {
		pr_err("[ERR][%s](%s) :couldn't read lcd_mux_id\n",
		       __func__, dev_read_name(dev));
	}

	return ret;
}

int tcc_lvds_probe(struct udevice *dev)
{
	/* struct tcc_lvds_priv *priv = dev_get_priv(dev); */
	int ret;
	#if defined(CONFIG_TCC803X) && defined(CONFIG_R5_LVDS_CTRL)
	struct mbox_chan mbox_dm_ch;
	#endif

	/*
	ret = uclass_get_device_by_phandle(UCLASS_PANEL, dev, "tcc,panel",
					   &priv->panel);

	if (ret) {
		debug("%s: Cannot find panel for '%s' (ret=%d)\n", __func__,
		      dev->name, ret);
		return ret;
	}

	*/
	#if defined(CONFIG_TCC803X) && defined(CONFIG_R5_LVDS_CTRL)
	memset(&mbox_dm_ch, 0, sizeof(mbox_dm_ch));
	ret = mbox_get_by_index(dev, 0, &mbox_dm_ch);
	tcc_lvds_ctrl_register_mbox(&mbox_dm_ch);
	if (ret != 0) {
		pr_err("[ERR] %s failed to get mbox\n", dev->name);
		return ret;
	}
	#endif
	ret = tcc_lvds_init(dev);

	return ret;
}

static const struct dm_display_ops lvds_tcc_ops = {
	.read_timing = tcc_lvds_read_timing,
	.enable = tcc_lvds_enable,
};

static const struct udevice_id tcc_lvds_ids[] = {
	{.compatible = "telechips,fb-tm123xdhp90"},
	{.compatible = "telechips,fb-fld0800"},
	{}
};

U_BOOT_DRIVER(lvds_tcc) = {
	.name	= "lvds_tcc",
	.id	= UCLASS_DISPLAY,
	.of_match = tcc_lvds_ids,
	.ops	= &lvds_tcc_ops,
	.of_to_plat	= tcc_lvds_ofdata_to_platdata,
	.probe	= tcc_lvds_probe,
	.priv_auto	= sizeof(struct tcc_lvds_priv),
};
