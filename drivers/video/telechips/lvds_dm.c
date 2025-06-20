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
#include <bootstage.h>
#include <linux/delay.h>
#include <dm/pinctrl.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <telechips/lvds_ctrl.h>
#include <asm/arch/vioc/vioc_disp.h>
#include <asm/arch/vioc/reg_physical.h>
#include <telechips/fb_dm.h>
#include <telechips/fb_dm_lcd_interface.h>

#define LVDS_DRV_DATE	"20250313"
#define LVDS_DRV_MAJOR	1
#define LVDS_DRV_MINOR	1
#define LVDS_DRV_PATCH	0

DECLARE_GLOBAL_DATA_PTR;

#define TCC_LVDS_OUTPUT_VESA24 0
#define TCC_LVDS_OUTPUT_JEIDA24 1
#define TCC_LVDS_OUTPUT_MAX 2

struct tcc_lvds_priv {
	struct udevice *panel; //unimplemented
	struct udevice *fb_dev;
	struct lvds_hw_info_t lvds_info;
	int use_evb_exp_gpio; //for tcc8030 evb exp gpio

	/*
	 * The AUO LCD panel must transmit the LVDS signal first and then turn
	 * on the backlight approximately 240ms later, when initializing the
	 * panel
	 */
	bool panel_auo;
};

static unsigned int
lvds_outformat[TCC_LVDS_OUTPUT_MAX][TXOUT_MAX_LINE][TXOUT_DATA_PER_LINE] = {
	/* LVDS vesa-24 format */
	{
		{TXOUT_G_D(0), TXOUT_R_D(5), TXOUT_R_D(4), TXOUT_R_D(3), TXOUT_R_D(2), TXOUT_R_D(1), TXOUT_R_D(0)},
		{TXOUT_B_D(1), TXOUT_B_D(0), TXOUT_G_D(5), TXOUT_G_D(4), TXOUT_G_D(3), TXOUT_G_D(2), TXOUT_G_D(1)},
		{TXOUT_DE,     TXOUT_VS,     TXOUT_HS,     TXOUT_B_D(5), TXOUT_B_D(4), TXOUT_B_D(3), TXOUT_B_D(2)},
		{TXOUT_DUMMY,  TXOUT_B_D(7), TXOUT_B_D(6), TXOUT_G_D(7), TXOUT_G_D(6), TXOUT_R_D(7), TXOUT_R_D(6)}
	},
	/* LVDS jeida-24 format */
	{
		{TXOUT_G_D(2), TXOUT_R_D(7), TXOUT_R_D(6), TXOUT_R_D(5), TXOUT_R_D(4), TXOUT_R_D(3), TXOUT_R_D(2)},
		{TXOUT_B_D(3), TXOUT_B_D(2), TXOUT_G_D(7), TXOUT_G_D(6), TXOUT_G_D(5), TXOUT_G_D(4), TXOUT_G_D(3)},
		{TXOUT_DE,     TXOUT_VS,     TXOUT_HS,     TXOUT_B_D(7), TXOUT_B_D(6), TXOUT_B_D(5), TXOUT_B_D(4)},
		{TXOUT_DUMMY,  TXOUT_B_D(1), TXOUT_B_D(0), TXOUT_G_D(1), TXOUT_G_D(0), TXOUT_R_D(1), TXOUT_R_D(0)}
	},
};

static int tcc_lvds_init(struct udevice *dev)
{
	int ret;

	ret = pinctrl_select_state(dev, "default");
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : failed to select state\n",
		       __func__, dev_read_name(dev));
	}

	ret = pinctrl_select_state(dev, "blk_off");
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : failed to select state\n",
		       __func__, dev_read_name(dev));
	}

	ret = pinctrl_select_state(dev, "lcd_off");
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : failed to select state\n",
		       __func__, dev_read_name(dev));
	}

	ret = pinctrl_select_state(dev, "reset_on");
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : failed to select state\n",
		       __func__, dev_read_name(dev));
	}

	return 0;
}

static int tcc_lvds_enable(struct udevice *dev, int panel_bpp,
		   const struct display_timing *timings)
{
	struct tcc_lvds_priv *priv = dev_get_priv(dev);
	struct tcc_fb_dm_priv *fb_priv = dev_get_priv(priv->fb_dev);
	int ret = 0;
	fb_priv->lcd_pxdw = PIXEL_ENCODING_RGB;

	(void)panel_bpp;
	(void)timings;

	ret = pinctrl_select_state(dev, "lcd_on");
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : failed to select state\n",
		       __func__, dev_read_name(dev));
	}
	if (priv->panel_auo) {
		mdelay(1);
	} else {
		udelay(20);
	}
	ret = pinctrl_select_state(dev, "reset_off");
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : failed to select state\n",
		       __func__, dev_read_name(dev));
	}

#if defined(CONFIG_TCC897X)
	lvds_ddi_config_init(&priv->lvds_info);
	(void)lcdc_mux_select(priv->lvds_info.lcdc_mux_select, get_vioc_index(fb_priv->lcd_disp));
	(void)lcdc_display_device_init(priv->fb_dev);
	(void)lcdc_turn_on_display(priv->fb_dev);
	if ((bool)ofnode_device_is_compatible(dev_ofnode(dev), "telechips,fb-fld0800")) {
		mdelay(120);	// B/L timing requirement of FLD-0800 LVDS panel
	}
#else
	lvds_splitter_init(&priv->lvds_info);
	(void)lcdc_mux_select(priv->lvds_info.lcdc_mux_select, get_vioc_index(fb_priv->lcd_disp));
	(void)lcdc_display_device_init(priv->fb_dev);
	(void)lcdc_turn_on_display(priv->fb_dev);
	lvds_phy_init(&priv->lvds_info);
#endif

#if defined(CONFIG_TELECHIPS_LVDS_SERDES)
	lvds_set_serdes();
#endif

#if defined(CONFIG_TCC803X)
	if (priv->lvds_info.lvds_type == (u32)PANEL_LVDS_DUAL) {
		mdelay(80); //requirement for tcc8030-evb_sv0.1 dual LVDS panel
	}
#endif

	if (priv->use_evb_exp_gpio != -1) {
		debug_pr(LOGL_INFO, "[INFO][%s](%s) : exp_set_value\n",
				__func__, dev_read_name(dev));
		tcclcd_gpioexp_set_value(1);
	}
	if (!priv->panel_auo) {
		ret = pinctrl_select_state(dev, "blk_on");
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : failed to select state\n",
					__func__, dev_read_name(dev));
		}
	}
	return 0;
}

static int tcc_lvds_read_timing(
	struct udevice *dev, struct display_timing *disp_timing)
{
	struct tcc_lvds_priv *priv = dev_get_priv(dev);
	int ret;

	ret = ofnode_decode_display_timing(dev_ofnode(dev), 0, disp_timing);

	priv->lvds_info.xres = disp_timing->hactive.typ;
	priv->lvds_info.p_clk = disp_timing->pixelclock.typ;

	return ret;
}

static int tcc_lvds_find_remote_phandle(
	const struct udevice *dev, u32 *remote_phandle)
{
	ofnode ports, port, node;
	int ret = 0;

	ports = dev_read_subnode(dev, "ports");
	if (!ofnode_valid(ports)) {
		ports = dev_ofnode(dev);
	}

	port = ofnode_find_subnode(ports, "port");
	if (!ofnode_valid(port)) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : 'port' subnode not found\n",
		      __func__, dev_read_name(dev));
		ret = -EINVAL;
	} else {
		node = ofnode_first_subnode(port);
		ret = ofnode_read_u32(node, "remote-endpoint", remote_phandle);
		if ((bool)ret) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : ret = %d\n",
				__func__, dev_read_name(dev), ret);
		}
	}

	return ret;
}


static int tcc_lvds_set_panel_id(
	const struct udevice *dev, struct tcc_lvds_priv *priv)
{
	struct tcc_fb_dm_priv *fb_priv;
	struct udevice *fb_dev = NULL;
	u32 remote_phandle;
	ofnode remote;
	int ret = tcc_lvds_find_remote_phandle(dev, &remote_phandle);

	if (ret == 0) {
		remote = ofnode_get_by_phandle(remote_phandle);
		if (!ofnode_valid(remote)) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : node not valid\n",
				__func__, dev_read_name(dev));
			ret = -EINVAL;
		}

		while (ret == 0) {
			remote = ofnode_get_parent(remote);
			if (!ofnode_valid(remote)) {
				debug_pr(LOGL_ERR, "[ERROR][%s](%s) : no UCLASS_VIDEO for remote-endpoint\n",
					__func__, dev_read_name(dev));
				ret = -EINVAL;
			} else {
				(void)uclass_find_device_by_ofnode(UCLASS_VIDEO, remote, &fb_dev);
				if (fb_dev != NULL) {
					priv->fb_dev = fb_dev;
					fb_priv = (struct tcc_fb_dm_priv *)fb_dev->priv_;
					fb_priv->media_id = priv->lvds_info.lvds_type;
					priv->lvds_info.lcdc_mux_select = fb_priv->lcd_mux_select;
					priv->lvds_info.lcdc_mux_bypass = fb_priv->lcd_mux_bypass;
					debug_pr(LOGL_INFO, "[INFO][%s](%s) : lcd_mux_select = %u\n", __func__,
						dev_read_name(dev), priv->lvds_info.lcdc_mux_select);
					break;
				}
			}
		}
	}

	return ret;
}

static int tcc_lvds_ofdata_to_platdata(struct udevice *dev)
{
	struct tcc_lvds_priv *priv = dev_get_priv(dev);
	int ret;
	const char *mapping;
	unsigned int lvds_format = TCC_LVDS_OUTPUT_VESA24;
#if defined(CONFIG_TCC897X) || defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	unsigned int read_dt_arr[5];
	unsigned int dt_lvds_type;
	unsigned int use_exp_gpio;
	size_t num_val = 0u;
	size_t idx;
#endif

	if ((bool)ofnode_device_is_compatible(dev_ofnode(dev), "telechips,fb-auoc123han06")) {
		priv->panel_auo = (bool)true;
	}

	mapping = ofnode_read_string(dev_ofnode(dev), "data-mapping");
	if (mapping == NULL) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : couldn't read data-mapping\n",
			__func__, dev_read_name(dev));
	} else if (!(bool)strncmp(mapping, "jeida-24", 8u)) {
		lvds_format = TCC_LVDS_OUTPUT_JEIDA24;
	} else {
	/*
		MISRA C-2012 Control Flow(Rule 15.7)
		: Empty else without comment
	*/
	}
	debug_pr(LOGL_INFO, "[INFO][%s](%s) : lvds_format = %u\n",
		__func__, dev_read_name(dev), lvds_format);

	(void)memcpy(priv->lvds_info.txout_main, lvds_outformat[lvds_format],
		sizeof(priv->lvds_info.txout_main));

	ret = ofnode_read_u32(dev_ofnode(dev), "mode", &dt_lvds_type);
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : couldn't read lvds_type[mode]\n",
			__func__, dev_read_name(dev));
	}

	if (dt_lvds_type == (u32)LVDS_DUAL) {
		/* Optional property, 0(def) - odd/even dual, 1 - left/right dual */
		ret = ofnode_read_u32(dev_ofnode(dev), "split-mode", &priv->lvds_info.split_mode);
		if (ret < 0) {
			debug_pr(LOGL_INFO, "[INFO][%s](%s) : no split_mode exists. set default(odd/even)\n",
				__func__, dev_read_name(dev));
			priv->lvds_info.split_mode = LVDS_WRAP_CORE_SPLIT_O_E;
		}
		if (priv->lvds_info.split_mode >= (u32)LVDS_WRAP_CORE_SPLIT_MAX) {
			priv->lvds_info.split_mode = LVDS_WRAP_CORE_SPLIT_O_E;
		}
		debug_pr(LOGL_INFO, "[INFO][%s](%s) : split-mode=%u\n",
			__func__, dev_read_name(dev), priv->lvds_info.split_mode);
	}

#if !defined(CONFIG_TCC897X)
	if (dt_lvds_type == (u32)LVDS_DUAL) {
		priv->lvds_info.lvds_type = PANEL_LVDS_DUAL;
		num_val = 2;
	} else
#endif
	if (dt_lvds_type == (u32)LVDS_SINGLE) {
		priv->lvds_info.lvds_type = PANEL_LVDS_SINGLE;
		num_val = 1;
	} else {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : invalid lvds_type = %d\n",
			__func__, dev_read_name(dev), priv->lvds_info.lvds_type);
	}
	ret = ofnode_read_u32_array(dev_ofnode(dev), "phy-ports", read_dt_arr, num_val);
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : couldn't read phy-ports [%d]\n",
			__func__, dev_read_name(dev), ret);
	}

	priv->lvds_info.port_main = read_dt_arr[0];

	if (priv->lvds_info.lvds_type == (u32)PANEL_LVDS_DUAL) {
		priv->lvds_info.port_sub = read_dt_arr[1];
		priv->lvds_info.ts_mux_select = 0x3U;
	} else {
		priv->lvds_info.port_sub = LVDS_PHY_PORT_MAX;
		priv->lvds_info.ts_mux_select = (priv->lvds_info.port_main + 2U) % 4U;
	}

	num_val = 5;
	ret = ofnode_read_u32_array(dev_ofnode(dev), "lane-main", read_dt_arr, num_val);
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : couldn't read lane-main\n",
			__func__, dev_read_name(dev));
	}

	for (idx = 0; idx < num_val; idx++) {
		priv->lvds_info.lane_main[idx] = read_dt_arr[idx];
	}

	if (dt_lvds_type == (u32)LVDS_DUAL) {
		ret = ofnode_read_u32_array(dev_ofnode(dev), "lane-sub", read_dt_arr, num_val);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : couldn't read lane-sub\n",
				__func__, dev_read_name(dev));
		}

		for (idx = 0; idx < num_val; idx++) {
			priv->lvds_info.lane_sub[idx] = read_dt_arr[idx];
		}
	}

	ret = ofnode_read_u32(dev_ofnode(dev), "vcm", &priv->lvds_info.vcm);
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : couldn't read vcm\n",
			__func__, dev_read_name(dev));
	}

	ret = ofnode_read_u32(dev_ofnode(dev), "vsw", &priv->lvds_info.vsw);
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : couldn't read vsw\n",
			__func__, dev_read_name(dev));
	}

	if (priv->lvds_info.lvds_type == (u32)PANEL_LVDS_DUAL) {
		(void)memcpy(priv->lvds_info.txout_sub, lvds_outformat[lvds_format],
			sizeof(priv->lvds_info.txout_sub));
	}

	ret = ofnode_read_u32(dev_ofnode(dev), "tcc8030-evb-exp", &use_exp_gpio);
	if (ret == 0) {
		debug_pr(LOGL_INFO, "[INFO][%s](%s) : tcc8030-evb-exp-gpio\n",
			__func__, dev_read_name(dev));
		priv->use_evb_exp_gpio = (use_exp_gpio < (~0U>>1)) ? (int)use_exp_gpio : -1;
	} else { //exp gpio is not used
		priv->use_evb_exp_gpio = -1;
	}
	ret = tcc_lvds_set_panel_id(dev, priv);

	return ret;
}

static int tcc_lvds_probe(struct udevice *dev)
{
	int ret;

#if defined(CONFIG_TCC803X) && defined(CONFIG_R5_LVDS_CTRL)
	struct mbox_chan mbox_dm_ch;

	(void)memset(&mbox_dm_ch, 0, sizeof(mbox_dm_ch));
	ret = mbox_get_by_index(dev, 0, &mbox_dm_ch);
	tcc_lvds_ctrl_register_mbox(&mbox_dm_ch);
	if (ret != 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : failed to get mbox\n", __func__, dev->name);
	} else {
		ret = tcc_lvds_init(dev);
	}
#else
	ret = tcc_lvds_init(dev);
#endif

	debug_pr(LOGL_INFO, "\n[%s:%d](%s) LVDS driver v%d.%d.%d(%s) probe.\n",
						__func__, __LINE__,
						dev->name,
						LVDS_DRV_MAJOR,
						LVDS_DRV_MINOR,
						LVDS_DRV_PATCH,
						LVDS_DRV_DATE);
	return ret;
}


static uint32_t tcc_lvds_backlight_wait_time(struct udevice *dev)
{
	struct tcc_lvds_priv *priv = dev_get_priv(dev);
	uint32_t wait_time = 0u;

	if (priv->panel_auo) {
		wait_time = 240u;
	}
	return wait_time;
}

static int tcc_lvds_backlight(struct udevice *dev, bool blen)
{
	struct tcc_lvds_priv *priv = dev_get_priv(dev);
	int ret = 0;

	if (priv->panel_auo) {
		if (blen) {
			ret = pinctrl_select_state(dev, "blk_on");
		} else {
			ret = pinctrl_select_state(dev, "blk_off");
		}
	}
	return ret;
}

static const struct dm_display_ops lvds_tcc_ops = {
	.read_timing = tcc_lvds_read_timing,
	.enable = tcc_lvds_enable,
	.backlight_wait_time_fn = tcc_lvds_backlight_wait_time,
	.backlight_fn = tcc_lvds_backlight,
};

static const struct udevice_id tcc_lvds_ids[] = {
	{.compatible = "telechips,fb-auoc123han06"},
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
	.priv_auto	= (int)sizeof(struct tcc_lvds_priv),
};
