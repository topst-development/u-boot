// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <display.h>
#include <clk.h>
#include <mach/clock.h>
#include <dm.h>
#include <bootstage.h>
#include <dm/pinctrl.h>
#include <dm/uclass-internal.h>
#include <telechips/fb_dm.h>
#include <telechips/dsi_ctrl.h>
#include <telechips/fb_dm_lcd_interface.h>

#include <linux/delay.h>

#define DSI_DM_DRV_MAJOR_VER	1
#define DSI_DM_DRV_MINOR_VER	0
#define DSI_DM_DRV_PATCH_VER	1
struct tcc_dsi_priv {
	uint32_t lcdc_mux_id;
	uint32_t lcdc_id;
	uint32_t sdm_bypass;
	uint32_t trvc_bypass;

	struct udevice *parent_dev;
	struct mipi_dsi_dev dsi_dev;

	/*
	 * The AUO LCD panel must transmit the LVDS signal first and then turn
	 * on the backlight approximately 240ms later, when initializing the
	 * panel
	 */
	bool panel_auo;
};

static uint32_t tcc_dsi_get_panel_id(const struct udevice *dev)
{
	uint32_t panel_id = 0u;
	int ret = ofnode_read_u32(dev_ofnode(dev), "panel-index", &panel_id);
	if ((ret < 0) || (panel_id >= 2u)) {
		/* 0: BOE PANEL, 1: AUO PANEL */
		panel_id = 0u;
	}
	return panel_id;
}

static int32_t tcc_dsi_get_dc_id(const struct udevice *dev)
{
	int32_t ret = 0;
	uint32_t remote_phandle;
	ofnode ports_ofnode, port_ofnode, sub_ofnode, remote_ofnode;
	struct udevice *p_dev;
	struct tcc_fb_dm_priv *p_priv;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);

	ports_ofnode = dev_read_subnode(dev, "ports");
	if (!ofnode_valid(ports_ofnode)) {
		ports_ofnode = dev_ofnode(dev);
	}
	port_ofnode = ofnode_find_subnode(ports_ofnode, "port");
	if (!ofnode_valid(port_ofnode)) {
		debug_pr(LOGL_ERR, "\n[%s:%d](%s)Err: 'port' node not found\n",
							__func__,
							__LINE__,
							dev_read_name(dev));

		ret = -EINVAL;
	}

	if(ret == 0) {
		sub_ofnode = ofnode_first_subnode(port_ofnode);
		ret = ofnode_read_u32(sub_ofnode, "remote-endpoint", &remote_phandle);
		if (ret != 0) {
			debug_pr(LOGL_ERR, "\n[%s:%d]Err: can't find remote-endpoint\n",
								__func__,
								__LINE__);

			ret = -EINVAL;
		}
	}

	if(ret == 0) {
		remote_ofnode = ofnode_get_by_phandle(remote_phandle);
		if (!ofnode_valid(remote_ofnode)) {
			debug_pr(LOGL_ERR, "\n[%s:%d]Err: remote ofnode isn't valid\n",
								__func__,
								__LINE__);

			ret = -EINVAL;
		}
	}

	if(ret == 0) {
		while(ofnode_valid(remote_ofnode) && (ret == 0)) {
			remote_ofnode = ofnode_get_parent(remote_ofnode);
			if (!ofnode_valid(remote_ofnode)) {
				debug_pr(LOGL_ERR, "\n[%s:%d](%s)Err: parent remote ofnode isn't valid\n",
									__func__,
									__LINE__,
									dev_read_name(dev));
				ret = -EINVAL;
			} else {
				(void)uclass_find_device_by_ofnode(UCLASS_VIDEO, remote_ofnode, &p_dev);
				if (p_dev != NULL) {
					/*For KCS*/
					break;
				}
			}
		}
	}

	if(ret == 0) {
		priv->parent_dev = p_dev;

		p_priv = (struct tcc_fb_dm_priv *)p_dev->priv_;
		priv->lcdc_id = get_vioc_index(p_priv->lcd_disp);
		p_priv->lcd_pxdw = PIXEL_ENCODING_RGB;

		debug_pr(LOGL_INFO, "[%s:%d]Found dc id as %u\n",
						__func__,
						__LINE__,
						priv->lcdc_id);
	}

	return ret;
}

#ifdef CONFIG_TCC750X
static int32_t tcc_dsi_parse_dt_v1(const struct udevice *dev)
{
	int32_t ret = 0;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);
	unsigned int tmp = 0;

	(void)memset(priv, 0, sizeof(struct tcc_dsi_priv));

	dsih_dpi_video_t *dpi_video = &priv->dsi_dev.dpi_video;

	ret = ofnode_read_u32(dev->node_, "no_of_lane", &dpi_video->no_of_lanes);
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[%s:%d]Err: couldn't read no_of_lane, set to default 2lanes\n",
							__func__,
							__LINE__);
		dpi_video->no_of_lanes = 2;
	}

	ret = ofnode_read_u32(dev->node_, "color_coding", &tmp);
	if(tmp < (unsigned int)COLOR_CODE_MAX) {
		dpi_video->color_coding = (dsih_color_coding_t)tmp;
	} else {
		dpi_video->color_coding = COLOR_CODE_24BIT;	// default setting
	}

	if (ret < 0) {
		debug_pr(LOGL_ERR, "[%s:%d]Err: couldn't read no_of_lane, set to default 24bpp\n",
							__func__,
							__LINE__);
		dpi_video->color_coding = COLOR_CODE_24BIT;
	}
	debug_pr(LOGL_INFO, "[%s:%d] Read DSI DT Done\n", __func__, __LINE__);
	return ret;
}

static void tcc_dsi_dphy_reset(unsigned int reset)
{
	void __iomem *reg =
		(void __iomem *)(uintptr_t)CAM_SWRST_REG;
	unsigned int val;

	if(reset != 0U) { // reset
		val = __raw_readl(reg) & ~(0x1U << MIPI_PHY_M_RESETN);
	} else { // release
		val = __raw_readl(reg) | (0x1U << MIPI_PHY_M_RESETN);
	}

	__raw_writel(val, reg);
	return;
}

static int tcc_dsi_enable_v1(struct udevice *dev,
							int panel_bpp,
							const struct display_timing *timings)
{
	struct tcc_dsi_priv *priv = dev_get_priv(dev);
	unsigned long peri_lcd_id;
	int32_t ret = 0;

	udelay(20);
	ret = pinctrl_select_state(dev, "lcd_on"); // pwr_on gpd 11
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][DSI_pin] failed to select state\n");
	}
	if (priv->panel_auo) {
		mdelay(1);
	}
	ret = pinctrl_select_state(dev, "reset_off"); // reset off  gpd 5
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][DSI_pin] failed to select state\n");
	}
	if (!priv->panel_auo) {
		ret = pinctrl_select_state(dev, "blk_on"); // bl gph - 6
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][DSI_pin] failed to select state\n");
		}
	}

	tcc_fb_dm_get_peri_id(priv->lcdc_id, peri_lcd_id);

	tcc_dsi_dphy_reset(0);

	(void)lcdc_mux_select(priv->lcdc_mux_id, priv->lcdc_id);

	(void)lcdc_display_device_init(priv->parent_dev);

	ret = dsi_enable_v1(&priv->dsi_dev, timings);
	(void)lcdc_turn_on_display(priv->parent_dev);

	debug_pr(LOGL_NOTICE, "\n[%s:%d]Set timing to DSI\n", __func__, __LINE__);
	debug_pr(LOGL_NOTICE, " 	panel bpp = %d \n", panel_bpp);
	debug_pr(LOGL_NOTICE, " 	Pixel clk = %u \n", timings->pixelclock.typ);
	debug_pr(LOGL_NOTICE, " 	flags = 0x%x \n", timings->flags);
	debug_pr(LOGL_NOTICE, " 	H active(%u) x V active(%u) \n", timings->hactive.typ, timings->vactive.typ);
	debug_pr(LOGL_NOTICE, " 	H front porch(%u), V front porch(%u) \n", timings->hfront_porch.typ, timings->vfront_porch.typ);
	debug_pr(LOGL_NOTICE, " 	H back porch(%u), V back porch(%u) \n", timings->hback_porch.typ, timings->hback_porch.typ);
	debug_pr(LOGL_NOTICE, " 	H sync len(%u), V sync len(%u) \n", timings->hsync_len.typ, timings->hsync_len.typ);

	return 0;
}

static int32_t tcc_dsi_ofdata_to_platdata(struct udevice *dev)
{
	int32_t ret = 0;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);

	(void)tcc_dsi_parse_dt_v1(dev);
	(void)tcc_dsi_get_dc_id(dev);

	if (tcc_dsi_get_panel_id(dev) == 1u) {
		priv->panel_auo = (bool)true;
	}
	debug_pr(LOGL_NOTICE, "\n[%s:%d]dc id : %u\n",
				__func__,
				__LINE__,
				(uint32_t)priv->lcdc_id);

	return ret;
}
#endif

#ifdef CONFIG_TCC807X
static int32_t tcc_dsi_parse_dt_v2(const struct udevice *dev)
{
	int32_t ret = 0;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_dev *dsi_dev;

	const struct tcc_fb_dm_priv *fb_priv;
	struct udevice *fb_dev;
	ofnode ports, port, node, remote;
	u32 remote_phandle;

	(void)memset(priv, 0, sizeof(struct tcc_dsi_priv));

	dsi_dev = &priv->dsi_dev;

	ret = ofnode_read_u32(dev->node_, "dsi-port", &dsi_dev->port);
	if (ret < 0) {
		/* For KCS */
		debug_pr(LOGL_ERR, "[%s:%d]Err: couldn't read dsi-port err no: %d\n", __func__, __LINE__, ret);
	}

	if(dsi_dev->port == 0U) {
		dsi_dev->core_addr = (void __iomem *)(uintptr_t)DSI0_CORE_ADDR;
		dsi_dev->phy_addr = (void __iomem *)(uintptr_t)DSI0_PHY_ADDR;
		dsi_dev->port = 0U;
	} else if (dsi_dev->port == 1U) {
		dsi_dev->core_addr = (void __iomem *)(uintptr_t)DSI1_CORE_ADDR;
		dsi_dev->phy_addr = (void __iomem *)(uintptr_t)DSI1_PHY_ADDR;
		dsi_dev->port = 1U;
	} else {
		// error
	}

	ret = ofnode_read_u32(dev->node_, "no_of_lane", &dsi_dev->phy_cfg.phy_lanes);
	if (ret < 0) {
		/* For KCS */
		debug_pr(LOGL_ERR, "[%s:%d]Err: couldn't read no_of_lane, Set to default 2 lanes\n", __func__, __LINE__);
		dsi_dev->phy_cfg.phy_lanes = 2;
	}

	ret = ofnode_read_u32(dev->node_, "auto-mode", &dsi_dev->main_cfg.manual_mode_en);
	if (ret < 0) {
		/* For KCS */
		debug_pr(LOGL_ERR, "[%s:%d]Err: couldn't read auto-mode, Set to default Automode\n", __func__, __LINE__);
		dsi_dev->main_cfg.manual_mode_en = 0;
	}

	ret = ofnode_read_u32(dev->node_, "dsi-sdm-bypass", &priv->sdm_bypass);
	if (ret < 0) {
		/* For KCS */
		debug_pr(LOGL_ERR, "[%s:%d]Err: couldn't read dsi-sdm-bypass, Set to default bypass\n", __func__, __LINE__);
		priv->sdm_bypass = 1;
	}

	ret = ofnode_read_u32(dev->node_, "dsi-trvc-bypass", &priv->trvc_bypass);
	if (ret < 0) {
		/* For KCS */
		debug_pr(LOGL_ERR, "[%s:%d]Err: couldn't read dsi-trvc-bypass, Set to default bypass\n", __func__, __LINE__);
		priv->trvc_bypass = 1;
	}
	debug_pr(LOGL_INFO, "[%s:%d] Read DSI DT Done\n", __func__, __LINE__);

	/* find fb_priv */
	ports = dev_read_subnode(dev, "ports");
	if (!ofnode_valid(ports)) {
		ports = dev_ofnode(dev);
	}
	port = ofnode_find_subnode(ports, "port");
	if (!ofnode_valid(port)) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : 'port' subnode not found\n",
			__func__, dev_read_name(dev));
		ret = -EINVAL;
	}

	if(ret == 0) {
		node = ofnode_first_subnode(port);
		ret = ofnode_read_u32(node, "remote-endpoint", &remote_phandle);
		if (ret != 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : ret = %d\n",
				__func__, dev_read_name(dev), ret);
		}
	}

	if(ret == 0) {
		remote = ofnode_get_by_phandle(remote_phandle);
		if(!ofnode_valid(remote)) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : node not valid\n",
				__func__, dev_read_name(dev));
			ret = -EINVAL;
		}
	}

	if(ret == 0) {
		while(ofnode_valid(remote) && (ret==0)) {
			remote = ofnode_get_parent(remote);
			if (!ofnode_valid(remote)) {
				debug_pr(LOGL_ERR, "[ERROR][%s](%s) : no UCLASS_VIDEO for remote-endpoint\n",
					__func__, dev_read_name(dev));
				ret = -EINVAL;
			} else {
				(void)uclass_find_device_by_ofnode(UCLASS_VIDEO, remote, &fb_dev);
				if (fb_dev != NULL) {
					break;
				}
			}
		}
	}

	if(ret == 0) {
		priv->parent_dev = fb_dev;
		fb_priv = (struct tcc_fb_dm_priv *)fb_dev->priv_;
		if( fb_priv != NULL) {
			priv->lcdc_mux_id = fb_priv->lcd_mux_id;
		} else {
			debug_pr(LOGL_ERR, "[%s:%d]Err: fb_priv is NULL, couldn't read dsi-lcd-mux-sel, Set to default 2\n", __func__, __LINE__);
			priv->lcdc_mux_id = 2U;
		}
	}

	return ret;
}

static void tcc_dsi_ch_select(unsigned int port, unsigned int lcdc_mux_id,
							unsigned int sdm_bypass, unsigned int trvc_bypass)
{
	volatile unsigned int* reg = (volatile unsigned int*)(uintptr_t)(MIPI_DSI_CH_SEL_OFFSET);
	unsigned int val;

	if(port == 0U){
		val = __raw_readl(reg) & ~(0x7U << S0_MUX_SEL_SHIFT);
		val |= lcdc_mux_id << (S0_MUX_SEL_SHIFT);
	} else {
		val = __raw_readl(reg) & ~(0x7U << S1_MUX_SEL_SHIFT);
		val |= lcdc_mux_id << (S1_MUX_SEL_SHIFT);
	}

	val &= ~((unsigned int)0x3U << TRVC_DIS_SHIFT);
	val |= (sdm_bypass << SDM_DIS_SHIFT) | (trvc_bypass << TRVC_DIS_SHIFT);

	__raw_writel(val, reg);
}

static int tcc_dsi_enable_v2(struct udevice *dev,
							int panel_bpp,
							const struct display_timing *timings)
{
	int32_t ret = 0;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);
	unsigned long peri_lcd_id;

	ret = pinctrl_select_state(dev, "ser_pwdn_on"); // pwr_on gpd 11
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][DSI_pin] failed to select state ser_pwdn_on\n");
	}

	udelay(20);
	ret = pinctrl_select_state(dev, "lcd_on"); // pwr_on gpd 11
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][DSI_pin] failed to select state lcd_on\n");
	}
	if (priv->panel_auo) {
		mdelay(1);
	}
	ret = pinctrl_select_state(dev, "reset_off"); // reset off  gpd 5
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][DSI_pin] failed to select state reset_off\n");
	}

	if (!priv->panel_auo) {
		ret = pinctrl_select_state(dev, "blk_on"); // bl gph - 6
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][DSI_pin] failed to select state blk_on\n");
		}
	}

	tcc_fb_dm_get_peri_id(priv->lcdc_id, peri_lcd_id);

	priv->dsi_dev.pclk = tcc_get_peri(peri_lcd_id) / 1000U;
	debug_pr(LOGL_INFO, "Real PCLK to DSI : %ldkHz\n", priv->dsi_dev.pclk);
	tcc_dsi_ch_select(priv->dsi_dev.port, priv->lcdc_mux_id, priv->sdm_bypass, priv->trvc_bypass);

	priv->dsi_dev.sys_clk = 293000; // 293Mhz

	(void)lcdc_mux_select(priv->lcdc_mux_id, priv->lcdc_id);

	(void)lcdc_display_device_init(priv->parent_dev);

	ret = dsi_enable_v2(&priv->dsi_dev, timings);
	(void)lcdc_turn_on_display(priv->parent_dev);

	debug_pr(LOGL_NOTICE, "\n[%s:%d]Set timing to DSI\n", __func__, __LINE__);
	debug_pr(LOGL_NOTICE, " 	panel bpp = %d \n", panel_bpp);
	debug_pr(LOGL_NOTICE, " 	Pixel clk = %u \n", timings->pixelclock.typ);
	debug_pr(LOGL_NOTICE, " 	flags = 0x%x \n", timings->flags);
	debug_pr(LOGL_NOTICE, " 	H active(%u) x V active(%u) \n", timings->hactive.typ, timings->vactive.typ);
	debug_pr(LOGL_NOTICE, " 	H front porch(%u), V front porch(%u) \n", timings->hfront_porch.typ, timings->vfront_porch.typ);
	debug_pr(LOGL_NOTICE, " 	H back porch(%u), V back porch(%u) \n", timings->hback_porch.typ, timings->hback_porch.typ);
	debug_pr(LOGL_NOTICE, " 	H sync len(%u), V sync len(%u) \n", timings->hsync_len.typ, timings->hsync_len.typ);

	return 0;
}


static void tcc_dsi_get_num_of_dsis(const struct udevice *dev)
{
	const char *pcremote_dev_name;
	const char *string_ptr;
	int32_t ret = 0;
	uint32_t lane = 0;
	uint32_t remote_phandle;
	struct udevice *p_udev;
	struct udevice *p_dev = NULL;
	ofnode ports_ofnode, port_ofnode, sub_ofnode, remote_ofnode;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);

	for ((void)uclass_find_first_device(UCLASS_VIDEO, &p_udev) ;
			p_udev != NULL ;
			(void)uclass_find_next_device(&p_udev)) {
		if (!ofnode_valid(p_udev->node_)) {
			debug_pr(LOGL_DEBUG, "[%s:%d](%s)'port' subnode isn't valid\n",
								__func__,
								__LINE__,
								dev_read_name(p_udev));
								debug_pr(LOGL_ERR, "%s %d\n", __func__, __LINE__);
			continue;
		}
		ports_ofnode = dev_read_subnode(p_udev, "ports");
		if (!ofnode_valid(ports_ofnode)) {
			ports_ofnode = dev_ofnode(p_udev);
		}
		port_ofnode = ofnode_find_subnode(ports_ofnode, "port");
		if (!ofnode_valid(port_ofnode)) {
			debug_pr(LOGL_DEBUG, "[%s:%d](%s): 'port' node not found\n",
								__func__,
								__LINE__,
								dev_read_name(p_udev));
			continue;
		}
		sub_ofnode = ofnode_first_subnode(port_ofnode);

		ret = ofnode_read_u32(sub_ofnode, "remote-endpoint", &remote_phandle);
		if (ret != 0) {
			debug_pr(LOGL_DEBUG, "[%s:%d] can't find remote-endpoint\n", __func__, __LINE__);
			debug_pr(LOGL_ERR, "%s %d\n", __func__, __LINE__);
			continue;
		}
		remote_ofnode = ofnode_get_by_phandle(remote_phandle);
		if (!ofnode_valid(remote_ofnode)) {
			debug_pr(LOGL_DEBUG, "[%s:%d]remote ofnode isn't available\n", __func__, __LINE__);
			debug_pr(LOGL_ERR, "%s %d\n", __func__, __LINE__);

			continue;
		}
		while(ofnode_valid(remote_ofnode) && (ret==0)) {
			remote_ofnode = ofnode_get_parent(remote_ofnode);
			if (!ofnode_valid(remote_ofnode)) {
				debug_pr(LOGL_DEBUG, "[%s:%d](%s)parent remote ofnode isn't valid\n",
									__func__,
									__LINE__,
									dev_read_name(p_udev));
									debug_pr(LOGL_ERR, "%s %d\n", __func__, __LINE__);
				ret = -EINVAL;
			} else {
				(void)uclass_find_device_by_ofnode(UCLASS_DISPLAY, remote_ofnode, &p_dev);
				if (p_dev != NULL) {
					/*For KCS*/
					break;
				}
			}
		}
		if (p_dev == NULL) {
			debug_pr(LOGL_DEBUG, "[%s:%d](%s)parent dev isn't valid\n", __func__, __LINE__, dev_read_name(p_udev));
			debug_pr(LOGL_ERR, "%s %d\n", __func__, __LINE__);
			continue;
		}

		pcremote_dev_name = dev_read_name(p_dev);

		string_ptr = strstr(pcremote_dev_name, "dsi");

		if (string_ptr == NULL) {
			debug_pr(LOGL_INFO, "[%s:%d]not found dsi device <-> %s\n", __func__, __LINE__, pcremote_dev_name);
			continue;
		}

		ret = ofnode_read_u32(remote_ofnode, "no_of_lane", &lane);
		if (ret < 0) {
			/* For KCS */
			debug_pr(LOGL_ERR, "[%s:%d]Err: couldn't read no_of_lane, Set to default 2 lanes\n", __func__, __LINE__);
			lane = 2;
		}
		if(priv->dsi_dev.num_of_dsi == 0U) {
			priv->dsi_dev.lane0 = lane;
		} else {
			priv->dsi_dev.lane1 = lane;
		}
		priv->dsi_dev.num_of_dsi++;

		debug_pr(LOGL_INFO, "num of dsi : %d lane : %d\n", priv->dsi_dev.num_of_dsi, lane);
	}
}

static int32_t tcc_dsi_ofdata_to_platdata_v2(struct udevice *dev)
{
	struct tcc_dsi_priv *priv = dev_get_priv(dev);
	int32_t ret = 0;

	(void)tcc_dsi_parse_dt_v2(dev);
	if(priv->dsi_dev.port == 0U) {
		tcc_dsi_get_num_of_dsis(dev);
	}
	(void)tcc_dsi_get_dc_id(dev);

	if (tcc_dsi_get_panel_id(dev) == 1u) {
		priv->panel_auo = (bool)true;
	}

	debug_pr(LOGL_NOTICE, "\n[%s:%d]dc id : %u\n",
				__func__,
				__LINE__,
				(uint32_t)priv->lcdc_id);

	return ret;
}
#endif

static int tcc_dsi_init_pinctrl(struct udevice *dev)
{
	int ret = 0;
#ifdef CONFIG_TCC807X
	struct tcc_dsi_priv *priv = dev_get_priv(dev);
#endif

	ret = pinctrl_select_state(dev, "ser_pwdn_on");
	if (ret < 0) {
		debug_pr(LOGL_ERR, "\n[%s:%d]Err: failed to select ser_pwdn_on state\n",
							__func__,
							__LINE__);
	}

	udelay(20);

	ret = pinctrl_select_state(dev, "default");
	if (ret < 0) {
		debug_pr(LOGL_ERR, "\n[%s:%d]Err: failed to select default state\n",
							__func__,
							__LINE__);
	}
#ifdef CONFIG_TCC807X
	if(priv->dsi_dev.port == 0U) {
		ret = pinctrl_select_state(dev, "blk_off_dsi");
		if (ret < 0) {
			debug_pr(LOGL_ERR, "\n[%s:%d]Err: failed to select blk_off state\n",
								__func__,
								__LINE__);
		}
	}
#else
	ret = pinctrl_select_state(dev, "blk_off");
	if (ret < 0) {
		debug_pr(LOGL_ERR, "\n[%s:%d]Err: failed to select blk_off state\n",
							__func__,
							__LINE__);
	}
#endif


	ret = pinctrl_select_state(dev, "lcd_off");
	if (ret < 0) {
		debug_pr(LOGL_ERR, "\n[%s:%d]Err: failed to select lcd_off state\n",
							__func__,
							__LINE__);
	}

	ret = pinctrl_select_state(dev, "reset_on");
	if (ret < 0) {
		debug_pr(LOGL_ERR, "\n[%s:%d]Err: failed to select reset_on state\n",
							__func__,
							__LINE__);
	}



	return 0;
}

static int tcc_dsi_read_timing(struct udevice *dev, struct display_timing *timing)
{
	uint32_t panel_id;
	int ret = 0;

	panel_id = tcc_dsi_get_panel_id(dev);

	ret = ofnode_decode_display_timing(dev_ofnode(dev), (int)panel_id, timing);

	debug_pr(LOGL_NOTICE, "\n[%s:%d]Get timing from DSI\n", __func__, __LINE__);
	debug_pr(LOGL_NOTICE, "		Pixel clk = %u \n", timing->pixelclock.typ);
	debug_pr(LOGL_NOTICE, "		flags = 0x%x \n", timing->flags);
	debug_pr(LOGL_NOTICE, "		H active(%u) x V active(%u) \n", timing->hactive.typ, timing->vactive.typ);
	debug_pr(LOGL_NOTICE, "		H front porch(%u), V front porch(%u) \n", timing->hfront_porch.typ, timing->vfront_porch.typ);
	debug_pr(LOGL_NOTICE, "		H back porch(%u), V back porch(%u) \n", timing->hback_porch.typ, timing->hback_porch.typ);
	debug_pr(LOGL_NOTICE, "		H sync len(%u), V sync len(%u) \n", timing->hsync_len.typ, timing->hsync_len.typ);

	return ret;
}

static int tcc_dsi_probe(struct udevice *dev)
{
	int ret = 0;

	(void)tcc_dsi_init_pinctrl(dev);

	debug_pr(LOGL_INFO, "\n[%s:%d]DSI Display dm Ver : %d.%d.%d probe\n",
					__func__,
					__LINE__,
					DSI_DM_DRV_MAJOR_VER,
					DSI_DM_DRV_MINOR_VER,
					DSI_DM_DRV_PATCH_VER);
	return ret;
}

static uint32_t tcc_dsi_backlight_wait_time(struct udevice *dev)
{
	struct tcc_dsi_priv *priv = dev_get_priv(dev);
	uint32_t wait_time = 0u;

	if (priv->panel_auo) {
		/* AUO Panel */
		wait_time = 240u;
	}
	return wait_time;
}

static int tcc_dsi_backlight(struct udevice *dev, bool blen)
{
	struct tcc_dsi_priv *priv = dev_get_priv(dev);
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

static const struct dm_display_ops dsi_tcc_ops = {
	.read_timing = tcc_dsi_read_timing,
#ifdef CONFIG_TCC750X
	.enable = tcc_dsi_enable_v1,
#else
	.enable = tcc_dsi_enable_v2,
#endif
	.backlight_wait_time_fn = tcc_dsi_backlight_wait_time,
	.backlight_fn = tcc_dsi_backlight,
};

static const struct udevice_id tcc_dsi_ids[] = {
	{ .compatible = "telechips,fb-dsi-panel" },
	{ }
};


U_BOOT_DRIVER(dsi_tcc) = {
	.name = "dsi_tcc",
	.id = UCLASS_DISPLAY,
	.of_match = tcc_dsi_ids,
	.probe = tcc_dsi_probe,
	.ops = &dsi_tcc_ops,
#ifdef CONFIG_TCC750X
	.of_to_plat = tcc_dsi_ofdata_to_platdata,
#else
	.of_to_plat = tcc_dsi_ofdata_to_platdata_v2,
#endif
	.priv_auto = (int)sizeof(struct tcc_dsi_priv),
};
