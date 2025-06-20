// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <display.h>
#include <clk.h>
#include <mach/clock.h>
#include <dm.h>
#include <telechips/fb_dm.h>
#include <telechips/dsi_ctrl.h>
#include <dm/pinctrl.h>
#include <linux/delay.h>

struct tcc_dsi_priv {
	uint32_t lcdc_mux_id;
	uint32_t lcdc_id;
	uint32_t sdm_bypass;
	uint32_t trvc_bypass;

	struct udevice *parent_dev;
	struct mipi_dsi_dev dsi_dev;
};

extern int uclass_find_device_by_ofnode(enum uclass_id id, ofnode node,
				 struct udevice **devp);
extern int uclass_find_first_device(enum uclass_id id, struct udevice **devp);
extern int uclass_find_next_device(struct udevice **devp);

extern int lcdc_mux_select(unsigned int mux_id, unsigned int lcdc_id);
extern int lcdc_display_device_init(struct udevice *fb_dev);
extern int lcdc_turn_on_display(struct udevice *fb_dev);


static int32_t tcc_dsi_get_dc_id(struct udevice *dev)
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
		pr_err("\n[%s:%d](%s)Err: 'port' node not found\n",
							__func__,
							__LINE__,
							dev_read_name(dev));

		ret = -EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	sub_ofnode = ofnode_first_subnode(port_ofnode);

	ret = ofnode_read_u32(sub_ofnode, "remote-endpoint", &remote_phandle);
	if (ret != 0) {
		pr_err("\n[%s:%d]Err: can't find remote-endpoint\n",
							__func__,
							__LINE__);

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	remote_ofnode = ofnode_get_by_phandle(remote_phandle);
	if (!ofnode_valid(remote_ofnode)) {
		pr_err("\n[%s:%d]Err: remote ofnode isn't valid\n",
							__func__,
							__LINE__);

		ret = -EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	while(ofnode_valid(remote_ofnode)) {
		remote_ofnode = ofnode_get_parent(remote_ofnode);
		if (!ofnode_valid(remote_ofnode)) {
			pr_err("\n[%s:%d](%s)Err: parent remote ofnode isn't valid\n",
								__func__,
								__LINE__,
								dev_read_name(dev));
			ret = -EINVAL;

			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto return_funcs;
		}

		uclass_find_device_by_ofnode(UCLASS_VIDEO, remote_ofnode, &p_dev);
		if (p_dev != NULL) {
			/*For KCS*/
			break;
		}
	}

	priv->parent_dev = p_dev;

	p_priv = (struct tcc_fb_dm_priv *)p_dev->priv_;
	priv->lcdc_id = p_priv->lcd_id;
	p_priv->lcd_pxdw = PIXEL_ENCODING_RGB;

	pr_info("[%s:%d]Found dc id as %u\n",
					__func__,
					__LINE__,
					priv->lcdc_id);

return_funcs:
	return ret;
}

#ifdef CONFIG_TCC750X
static int32_t tcc_dsi_parse_dt_v1(struct udevice *dev)
{
	int32_t ret = 0;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);

	(void)memset(priv, 0, sizeof(struct tcc_dsi_priv));

	dsih_dpi_video_t *dpi_video = &priv->dsi_dev.dpi_video;

	ret = ofnode_read_u32(dev->node_, "no_of_lane", &dpi_video->no_of_lanes);
	if (ret < 0) {
		pr_err("[%s:%d]Err: couldn't read no_of_lane, set to default 2lanes\n",
							__func__,
							__LINE__);
		dpi_video->no_of_lanes = 2;
	}

	ret = ofnode_read_u32(dev->node_, "color_coding", &dpi_video->color_coding);
	if (ret < 0) {
		pr_err("[%s:%d]Err: couldn't read no_of_lane, set to default 24bpp\n",
							__func__,
							__LINE__);
		dpi_video->color_coding = 5;
	}

	ret = ofnode_read_u32(dev->node_, "ser_des_mode", &dpi_video->ser_des_mode);
	if (ret < 0) {
		pr_err("[%s:%d]Err: couldn't read ser_des_mode, set to default normal\n",
							__func__,
							__LINE__);
		dpi_video->ser_des_mode = 0;
	}
	pr_err("[%s:%d]Setting ser_des_mode[%d]\n", __func__, __LINE__, dpi_video->ser_des_mode);

	pr_info("[%s:%d] Read DSI DT Done\n", __func__, __LINE__);
	return ret;
}

void tcc_dsi_dphy_reset(unsigned int reset)
{
	void __iomem *reg =
		(void __iomem *)CAM_SWRST_REG;
	unsigned int val;

	if(reset) { // reset
		val = __raw_readl(reg) & ~(0x1 << MIPI_PHY_M_RESETN);
	} else { // release
		val = __raw_readl(reg) | (0x1 << MIPI_PHY_M_RESETN);
	}

	__raw_writel(val, reg);
	return;
}

int tcc_dsi_enable_v1(struct udevice *dev,
							int panel_bpp,
							const struct display_timing *timings)
{
	int32_t ret = 0;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);
	unsigned long peri_lcd_id;

	udelay(20);
	/*
	ret = pinctrl_select_state(dev, "pwr_on_1"); // pwr_on gpd 11
	if (ret < 0) {
		pr_err("[ERROR][DSI_pin] failed to select state\n");
	}

	ret = pinctrl_select_state(dev, "pwr_on_2"); // reset off  gpd 5
	if (ret < 0) {
		pr_err("[ERROR][DSI_pin] failed to select state\n");
	}

	ret = pinctrl_select_state(dev, "blk_on"); // bl gph - 6
	if (ret < 0) {
		pr_err("[ERROR][DSI_pin] failed to select state\n");
	}
	*/
	tcc_fb_dm_get_peri_id(priv->lcdc_id, peri_lcd_id);

	tcc_dsi_dphy_reset(0);

	lcdc_mux_select(priv->lcdc_mux_id, priv->lcdc_id);

	lcdc_display_device_init(priv->parent_dev);

	ret = dsi_enable_v1(&priv->dsi_dev, timings);
	lcdc_turn_on_display(priv->parent_dev);

	pr_notice("\n[%s:%d]Set timing to DSI\n", __func__, __LINE__);
	pr_notice(" 	Pixel clk = %u \n", timings->pixelclock.typ);
	pr_notice(" 	flags = 0x%x \n", timings->flags);
	pr_notice(" 	H active(%u) x V active(%u) \n", timings->hactive.typ, timings->vactive.typ);
	pr_notice(" 	H front porch(%u), V front porch(%u) \n", timings->hfront_porch.typ, timings->vfront_porch.typ);
	pr_notice(" 	H back porch(%u), V back porch(%u) \n", timings->hback_porch.typ, timings->hback_porch.typ);
	pr_notice(" 	H sync len(%u), V sync len(%u) \n", timings->hsync_len.typ, timings->hsync_len.typ);

	return 0;
}

static int32_t tcc_dsi_ofdata_to_platdata(struct udevice *dev)
{
	int32_t ret = 0;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);

	(void)tcc_dsi_parse_dt_v1(dev);
	(void)tcc_dsi_get_dc_id(dev);

	pr_notice("\n[%s:%d]dc id : %u\n",
				__func__,
				__LINE__,
				(uint32_t)priv->lcdc_id);

	return ret;
}
#endif

#ifdef CONFIG_TCC807X
static int32_t tcc_dsi_parse_dt_v2(struct udevice *dev)
{
	int32_t ret = 0;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);
	struct mipi_dsi_dev *dsi_dev;

	struct tcc_fb_dm_priv *fb_priv;
	struct udevice *fb_dev;
	ofnode ports, port, node, remote;
	u32 remote_phandle;

	(void)memset(priv, 0, sizeof(struct tcc_dsi_priv));

	dsi_dev = &priv->dsi_dev;

	ret = ofnode_read_u32(dev->node_, "dsi-port", &dsi_dev->port);
	if (ret < 0) {
		/* For KCS */
		pr_err("[%s:%d]Err: couldn't read dsi-port err no: %d\n", __func__, __LINE__, ret);
	}

	if(dsi_dev->port == 0) {
		dsi_dev->core_addr = (void __iomem *)DSI0_CORE_ADDR;
		dsi_dev->phy_addr = (void __iomem *)DSI0_PHY_ADDR;
		dsi_dev->port = 0;
	} else if (dsi_dev->port == 1) {
		dsi_dev->core_addr = (void __iomem *)DSI1_CORE_ADDR;
		dsi_dev->phy_addr = (void __iomem *)DSI1_PHY_ADDR;
		dsi_dev->port = 1;
	} else {
		// error
	}

	ret = ofnode_read_u32(dev->node_, "no_of_lane", &dsi_dev->phy_cfg.phy_lanes);
	if (ret < 0) {
		/* For KCS */
		pr_err("[%s:%d]Err: couldn't read no_of_lane, Set to default 2 lanes\n", __func__, __LINE__);
		dsi_dev->phy_cfg.phy_lanes = 2;
	}

	ret = ofnode_read_u32(dev->node_, "auto-mode", &dsi_dev->main_cfg.manual_mode_en);
	if (ret < 0) {
		/* For KCS */
		pr_err("[%s:%d]Err: couldn't read auto-mode, Set to default Automode\n", __func__, __LINE__);
		dsi_dev->main_cfg.manual_mode_en = 0;
	}

	ret = ofnode_read_u32(dev->node_, "dsi-sdm-bypass", &priv->sdm_bypass);
	if (ret < 0) {
		/* For KCS */
		pr_err("[%s:%d]Err: couldn't read dsi-sdm-bypass, Set to default bypass\n", __func__, __LINE__);
		priv->sdm_bypass = 1;
	}

	ret = ofnode_read_u32(dev->node_, "dsi-trvc-bypass", &priv->trvc_bypass);
	if (ret < 0) {
		/* For KCS */
		pr_err("[%s:%d]Err: couldn't read dsi-trvc-bypass, Set to default bypass\n", __func__, __LINE__);
		priv->trvc_bypass = 1;
	}
	pr_info("[%s:%d] Read DSI DT Done\n", __func__, __LINE__);

	/* find fb_priv */
	ports = dev_read_subnode(dev, "ports");
	if (!ofnode_valid(ports)) {
		ports = dev_ofnode(dev);
	}
	port = ofnode_find_subnode(ports, "port");
	if (!ofnode_valid(port)) {
		pr_err("[ERROR][%s](%s) : 'port' subnode not found\n",
			__func__, dev_read_name(dev));
		return -EINVAL;
	}

	node = ofnode_first_subnode(port);
	ret = ofnode_read_u32(node, "remote-endpoint", &remote_phandle);
	if (ret) {
		pr_err("[ERROR][%s](%s) : ret = %d\n",
			__func__, dev_read_name(dev), ret);
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
			pr_err("[ERROR][%s](%s) : no UCLASS_VIDEO for remote-endpoint\n",
				__func__, dev_read_name(dev));
		return -EINVAL;
		}

		uclass_find_device_by_ofnode(UCLASS_VIDEO, remote, &fb_dev);
		if (fb_dev) {
			break;
		}
	}
	priv->parent_dev = fb_dev;
	fb_priv = (struct tcc_fb_dm_priv *)fb_dev->priv_;
	priv->lcdc_mux_id = fb_priv->lcd_mux_id;
	if (ret < 0) {
		/* For KCS */
		pr_err("[%s:%d]Err: couldn't read dsi-lcd-mux-sel, Set to default 2\n", __func__, __LINE__);
		priv->lcdc_mux_id = 2;
	}

	return ret;
}

void tcc_dsi_ch_select(unsigned int port, unsigned int lcdc_mux_id,
							unsigned int sdm_bypass, unsigned int trvc_bypass)
{
	void __iomem *reg =
		(void __iomem *)CAM_CFG_REG + MIPI_DSI_CH_SEL_OFFSET;
	unsigned int val;

	if(port == 0){
		val = __raw_readl(reg) & ~(0x7 << S0_MUX_SEL_SHIFT);
		val |= lcdc_mux_id << (S0_MUX_SEL_SHIFT);
	} else {
		val = __raw_readl(reg) & ~(0x7 << S1_MUX_SEL_SHIFT);
		val |= lcdc_mux_id << (S1_MUX_SEL_SHIFT);
	}

	val &= ~(0x3 << TRVC_DIS_SHIFT);
	val |= (sdm_bypass << SDM_DIS_SHIFT) | (trvc_bypass << TRVC_DIS_SHIFT);

	__raw_writel(val, reg);
}

int tcc_dsi_enable_v2(struct udevice *dev,
							int panel_bpp,
							const struct display_timing *timings)
{
	int32_t ret = 0;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);
	unsigned long peri_lcd_id;

	ret = pinctrl_select_state(dev, "ser_pwdn_on"); // pwr_on gpd 11
	if (ret < 0) {
		pr_err("[ERROR][DSI_pin] failed to select state ser_pwdn_on\n");
	}

	udelay(20);
	ret = pinctrl_select_state(dev, "pwr_on_1"); // pwr_on gpd 11
	if (ret < 0) {
		pr_err("[ERROR][DSI_pin] failed to select state pwr_on_1\n");
	}

	ret = pinctrl_select_state(dev, "pwr_on_2"); // reset off  gpd 5
	if (ret < 0) {
		pr_err("[ERROR][DSI_pin] failed to select state pwr_on_2\n");
	}

	ret = pinctrl_select_state(dev, "blk_on"); // bl gph - 6
	if (ret < 0) {
		pr_err("[ERROR][DSI_pin] failed to select state blk_on\n");
	}

	tcc_fb_dm_get_peri_id(priv->lcdc_id, peri_lcd_id);

	priv->dsi_dev.pclk = tcc_get_peri(peri_lcd_id) / 1000;
	pr_info("Real PCLK to DSI : %ldkHz\n", priv->dsi_dev.pclk);
	tcc_dsi_ch_select(priv->dsi_dev.port, priv->lcdc_mux_id, priv->sdm_bypass, priv->trvc_bypass);

	priv->dsi_dev.sys_clk = 293000; // 293Mhz

	lcdc_mux_select(priv->lcdc_mux_id, priv->lcdc_id);

	lcdc_display_device_init(priv->parent_dev);

	ret = dsi_enable_v2(&priv->dsi_dev, timings);
	lcdc_turn_on_display(priv->parent_dev);

	pr_notice("\n[%s:%d]Set timing to DSI\n", __func__, __LINE__);
	pr_notice(" 	Pixel clk = %u \n", timings->pixelclock.typ);
	pr_notice(" 	flags = 0x%x \n", timings->flags);
	pr_notice(" 	H active(%u) x V active(%u) \n", timings->hactive.typ, timings->vactive.typ);
	pr_notice(" 	H front porch(%u), V front porch(%u) \n", timings->hfront_porch.typ, timings->vfront_porch.typ);
	pr_notice(" 	H back porch(%u), V back porch(%u) \n", timings->hback_porch.typ, timings->hback_porch.typ);
	pr_notice(" 	H sync len(%u), V sync len(%u) \n", timings->hsync_len.typ, timings->hsync_len.typ);

	return 0;
}


static void tcc_dsi_get_num_of_dsis(struct udevice *dev)
{
	const char *pcremote_dev_name;
	char *string_ptr;
	int32_t ret = 0;
	int32_t lane = 0;
	uint32_t remote_phandle;
	struct udevice *p_udev;
	struct udevice *p_dev;
	ofnode ports_ofnode, port_ofnode, sub_ofnode, remote_ofnode;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);

	for (uclass_find_first_device(UCLASS_VIDEO, &p_udev);
			p_udev;
			uclass_find_next_device(&p_udev)) {
		if (!ofnode_valid(p_udev->node_)) {
			pr_debug("[%s:%d](%s)'port' subnode isn't valid\n",
								__func__,
								__LINE__,
								dev_read_name(p_udev));
								pr_err("%s %d\n", __func__, __LINE__);
			continue;
		}
		ports_ofnode = dev_read_subnode(p_udev, "ports");
		if (!ofnode_valid(ports_ofnode)) {
			ports_ofnode = dev_ofnode(p_udev);
		}
		port_ofnode = ofnode_find_subnode(ports_ofnode, "port");
		if (!ofnode_valid(port_ofnode)) {
			pr_debug("[%s:%d](%s): 'port' node not found\n",
								__func__,
								__LINE__,
								dev_read_name(p_udev));
			continue;
		}
		sub_ofnode = ofnode_first_subnode(port_ofnode);

		ret = ofnode_read_u32(sub_ofnode, "remote-endpoint", &remote_phandle);
		if (ret != 0) {
			pr_debug("[%s:%d] can't find remote-endpoint\n", __func__, __LINE__);
			pr_err("%s %d\n", __func__, __LINE__);
			continue;
		}
		remote_ofnode = ofnode_get_by_phandle(remote_phandle);
		if (!ofnode_valid(remote_ofnode)) {
			pr_debug("[%s:%d]remote ofnode isn't available\n", __func__, __LINE__);
			pr_err("%s %d\n", __func__, __LINE__);

			continue;
		}
		while(ofnode_valid(remote_ofnode)) {
			remote_ofnode = ofnode_get_parent(remote_ofnode);
			if (!ofnode_valid(remote_ofnode)) {
				pr_debug("[%s:%d](%s)parent remote ofnode isn't valid\n",
									__func__,
									__LINE__,
									dev_read_name(p_udev));
									pr_err("%s %d\n", __func__, __LINE__);
				break;
			}
			uclass_find_device_by_ofnode(UCLASS_DISPLAY, remote_ofnode, &p_dev);
			if (p_dev != NULL) {
				/*For KCS*/
				break;
			}
		}
		if (p_dev == NULL) {
			pr_debug("[%s:%d](%s)parent dev isn't valid\n", __func__, __LINE__, dev_read_name(p_udev));
			pr_err("%s %d\n", __func__, __LINE__);
			continue;
		}

		pcremote_dev_name = dev_read_name(p_dev);

		string_ptr = strstr(pcremote_dev_name, "dsi");

		if (string_ptr == NULL) {
			pr_info("[%s:%d]not found dsi device <-> %s\n", __func__, __LINE__, pcremote_dev_name);
			continue;
		}

		ret = ofnode_read_u32(remote_ofnode, "no_of_lane", &lane);
		if (ret < 0) {
			/* For KCS */
			pr_err("[%s:%d]Err: couldn't read no_of_lane, Set to default 2 lanes\n", __func__, __LINE__);
			lane = 2;
		}
		if(priv->dsi_dev.num_of_dsi == 0)
			priv->dsi_dev.lane0 = lane;
		else
			priv->dsi_dev.lane1 = lane;
		priv->dsi_dev.num_of_dsi++;

		pr_info("num of dsi : %d lane : %d\n", priv->dsi_dev.num_of_dsi, lane);
	}
}

static int32_t tcc_dsi_ofdata_to_platdata_v2(struct udevice *dev)
{
	int32_t ret = 0;
	struct tcc_dsi_priv *priv = dev_get_priv(dev);

	(void)tcc_dsi_parse_dt_v2(dev);
	if(priv->dsi_dev.port == 0)
		tcc_dsi_get_num_of_dsis(dev);
	(void)tcc_dsi_get_dc_id(dev);

	pr_notice("\n[%s:%d]dc id : %u\n",
				__func__,
				__LINE__,
				(uint32_t)priv->lcdc_id);

	return ret;
}
#endif

static int tcc_dsi_init_pinctrl(struct udevice *dev)
{
	int ret = 0;
#if 0
	ret = pinctrl_select_state(dev, "ser_pwdn_on");
	if (ret < 0) {
		pr_err("\n[%s:%d]Err: failed to select ser_pwdn_on state\n",
							__func__,
							__LINE__);
	}

	udelay(20);

	ret = pinctrl_select_state(dev, "default");
	if (ret < 0) {
		pr_err("\n[%s:%d]Err: failed to select default state\n",
							__func__,
							__LINE__);
	}

	ret = pinctrl_select_state(dev, "power_off");
	if (ret < 0) {
		pr_err("\n[%s:%d]Err: failed to select power_off state\n",
							__func__,
							__LINE__);
	}

	ret = pinctrl_select_state(dev, "blk_off");
	if (ret < 0) {
		pr_err("\n[%s:%d]Err: failed to select blk_off state\n",
							__func__,
							__LINE__);
	}
#endif
	return ret;
}

int tcc_dsi_read_timing(struct udevice *dev, struct display_timing *timing)
{
	int32_t ret = 0;
	ret = ofnode_decode_display_timing(dev_ofnode(dev), 0, timing);

	pr_notice("\n[%s:%d]Get timing from DSI\n", __func__, __LINE__);
	pr_notice("		Pixel clk = %u \n", timing->pixelclock.typ);
	pr_notice("		flags = 0x%x \n", timing->flags);
	pr_notice("		H active(%u) x V active(%u) \n", timing->hactive.typ, timing->vactive.typ);
	pr_notice("		H front porch(%u), V front porch(%u) \n", timing->hfront_porch.typ, timing->vfront_porch.typ);
	pr_notice("		H back porch(%u), V back porch(%u) \n", timing->hback_porch.typ, timing->hback_porch.typ);
	pr_notice("		H sync len(%u), V sync len(%u) \n", timing->hsync_len.typ, timing->hsync_len.typ);

	return ret;
}


int tcc_dsi_probe(struct udevice *dev)
{
	int ret = 0;

	(void)tcc_dsi_init_pinctrl(dev);

	return ret;
}


static const struct dm_display_ops dsi_tcc_ops = {
	.read_timing = tcc_dsi_read_timing,
#ifdef CONFIG_TCC750X
	.enable = tcc_dsi_enable_v1,
#else
	.enable = tcc_dsi_enable_v2,
#endif
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
	.priv_auto = sizeof(struct tcc_dsi_priv),
};
