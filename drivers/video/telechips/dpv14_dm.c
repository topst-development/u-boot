// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <linux/delay.h>
#include <display.h>
#include <dm.h>
#include <telechips/fb_dm.h>
#include <telechips/dpv14_ctrl.h>
#include <telechips/dpv14_max968xx.h>
#include <dm/pinctrl.h>
#include <dt-bindings/display/telechips-dispdef.h>

#define DPV14_DM_DRV_MAJOR_VER		1
#define DPV14_DM_DRV_MINOR_VER		0
#define DPV14_DM_DRV_PATCH_VER		0

#define GET_MUXID_FROM_FB

struct tcc_dpv14_priv {
	uint8_t ucnum_of_dps;
	uint32_t uidp_id;
	uint32_t uipanel_mode;
	uint32_t uii2c_port;
	uint32_t uiphy_lane_swap;
	uint32_t uisdm_bypass;
	uint32_t uitrvc_bypass;
	uint32_t uipixel_clk;
	uint32_t uiDc_idx;
	uint32_t uimax_rate;
	uint32_t uimax_lane;
	uint32_t uipixel_encoding;
	uint32_t uivcp_id[PANEL_DP_MAX];
	uint32_t uivic[PANEL_DP_MAX];
	uint32_t uilcdc_mux_id[PANEL_DP_MAX];

#if defined(GET_MUXID_FROM_FB)
	struct udevice *pre_pudev[PANEL_DP_MAX];
#endif

	struct udevice *parent_dev;
};


static struct tcc_dpv14_priv *psttcc_dpv14_priv[PANEL_DP_MAX] = {NULL, };

extern int uclass_find_device_by_ofnode(enum uclass_id id, ofnode node,
				 struct udevice **devp);
extern int uclass_find_first_device(enum uclass_id id, struct udevice **devp);
extern int uclass_find_next_device(struct udevice **devp);

extern int lcdc_mux_select(unsigned int mux_id, unsigned int lcdc_id);
extern int lcdc_display_device_init(struct udevice *fb_dev);
extern int lcdc_turn_on_display(struct udevice *fb_dev);


static int32_t tcc_dpv14_get_dd_id(struct udevice *dev)
{
	int32_t ret = 0;
	uint32_t remote_phandle;
	ofnode ports_ofnode, port_ofnode, sub_ofnode, remote_ofnode;
	struct udevice *p_dev;
	struct tcc_fb_dm_priv *p_priv;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);

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
	priv->uiDc_idx = p_priv->lcd_id;
	p_priv->lcd_pxdw = priv->uipixel_encoding;

	pr_info("[%s:%d]Found UCLASS_VIDEO node(%s) => dc id(%u)\n",
					__func__,
					__LINE__,
					dev_read_name(dev),
					priv->uiDc_idx);

return_funcs:
	return ret;
}

#if defined(GET_MUXID_FROM_FB)
static int32_t tcc_dpv14_get_mux_id(struct udevice *dev)
{
	uint8_t ucdpIdx;
	int32_t ret = 0;
	struct udevice *pre_udev;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	for (ucdpIdx = 0; ucdpIdx < priv->ucnum_of_dps; ucdpIdx++) {
		pre_udev = priv->pre_pudev[ucdpIdx];
		if (pre_udev == NULL) {
			pr_err("[%s:%d]Err: parent node isn't valid for Dp%d\n", __func__, __LINE__, ucdpIdx);
			goto return_funcs;
		}

		ret = ofnode_read_u32(pre_udev->node_, "lcd-mux-id", &priv->uilcdc_mux_id[ucdpIdx]);
		if (ret < 0) {
			pr_err("[%s:%d]Warn: couldn't read lcd-mux-id for Dp%d\n", __func__, __LINE__, ucdpIdx);
		}
	}

return_funcs:
	return ret;
}
#endif

static int32_t tcc_dpv14_get_num_of_dps(struct udevice *dev)
{
	const char *pcremote_dev_name;
	char *string_ptr;
	int32_t ret = 0;
	uint32_t remote_phandle;
	struct udevice *p_udev;
	struct udevice *p_dev;
	ofnode ports_ofnode, port_ofnode, sub_ofnode, remote_ofnode;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	for (uclass_find_first_device(UCLASS_VIDEO, &p_udev);
			p_udev;
			uclass_find_next_device(&p_udev)) {
		if (!ofnode_valid(p_udev->node_)) {
			pr_debug("[%s:%d](%s)'port' subnode isn't valid\n",
								__func__,
								__LINE__,
								dev_read_name(p_udev));
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
			continue;
		}

		remote_ofnode = ofnode_get_by_phandle(remote_phandle);
		if (!ofnode_valid(remote_ofnode)) {
			pr_debug("[%s:%d]remote ofnode isn't available\n", __func__, __LINE__);

			continue;
		}

		while(ofnode_valid(remote_ofnode)) {
			remote_ofnode = ofnode_get_parent(remote_ofnode);
			if (!ofnode_valid(remote_ofnode)) {
				pr_debug("[%s:%d](%s)parent remote ofnode isn't valid\n",
									__func__,
									__LINE__,
									dev_read_name(p_udev));
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
			continue;
		}

		pcremote_dev_name = dev_read_name(p_dev);

		string_ptr = strstr(pcremote_dev_name, "dpv");

		if (string_ptr == NULL) {
			pr_info("[%s:%d]not found dp device <-> %s\n", __func__, __LINE__, pcremote_dev_name);
			continue;
		}

#if defined(GET_MUXID_FROM_FB)
		priv->pre_pudev[priv->ucnum_of_dps] = p_udev;
#endif

		priv->ucnum_of_dps++;

		pr_info("\n[%s:%d]Found %u dp node as name(%s)\n", __func__, __LINE__, priv->ucnum_of_dps, pcremote_dev_name);
	}

	return ret;
}

static int32_t tcc_dpv14_parse_dt(struct udevice *dev)
{
	uint8_t ucdp_idx;
	int32_t ret = 0;
	uint32_t uidp_id;
	struct tcc_dpv14_priv *master_priv;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	(void)memset(priv, 0, sizeof(struct tcc_dpv14_priv));

	for (uidp_id = 0; uidp_id < PANEL_DP_MAX; uidp_id++) {
		if (psttcc_dpv14_priv[uidp_id] == NULL) {
			psttcc_dpv14_priv[uidp_id] = priv;
			break;
		}
	}

	if (uidp_id >= (uint32_t)PANEL_DP_MAX) {
		pr_err("\n[%s:%d]Err: Invalid DP id as %u\n", __func__, __LINE__, uidp_id);

		ret = -EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	switch (uidp_id) {
	case (uint32_t)PANEL_DP0:
		ret = tcc_dpv14_get_num_of_dps(dev);
		if (ret != 0) {
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto return_funcs;
		}
#if defined(GET_MUXID_FROM_FB)
		(void)tcc_dpv14_get_mux_id(dev);
#endif
		break;
	case (uint32_t)PANEL_DP1:
	case (uint32_t)PANEL_DP2:
	case (uint32_t)PANEL_DP3:
	default:
		master_priv = psttcc_dpv14_priv[PANEL_DP0];
		(void)memcpy(priv, master_priv, sizeof(struct tcc_dpv14_priv));
		break;
	}

	priv->uidp_id = uidp_id;

	if (uidp_id != (uint32_t)PANEL_DP0) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	ret = ofnode_read_u32(dev->node_, "dp-panel-mode", &priv->uipanel_mode);
	if (ret < 0) {
		/* For KCS */
		pr_warn("[%s:%d]Warn: couldn't read panel mode, set to panel mode by default\n", __func__, __LINE__);
		priv->uipanel_mode = 1U;
	}

	ret = ofnode_read_u32(dev->node_, "dp-serdes-i2c-port", &priv->uii2c_port);
	if (ret < 0) {
		/* For KCS */
		pr_warn("[%s:%d]Warn: couldn't read i2c port, set to D3(0), D5(4) by default\n", __func__, __LINE__);
		priv->uii2c_port = (uint32_t)I2C_PORT_TO_SERDES;
	}

	ret = ofnode_read_u32_array(dev->node_, "dp-vic", priv->uivic, (size_t)PANEL_DP_MAX);
	if (ret < 0) {
		/* For KCS */
		pr_warn("[%s:%d]Warn: couldn't read dp-vic, set to 1(480p) by default\n", __func__, __LINE__);
		priv->uivic[0] = 1U;
		priv->uivic[1] = 1U;
		priv->uivic[2] = 1U;
		priv->uivic[3] = 1U;
	}

	ret = ofnode_read_u32_array(dev->node_, "dp-vcp_id", priv->uivcp_id, (size_t)PANEL_DP_MAX);
	if (ret < 0) {
		/* For KCS */
		pr_warn("[%s:%d]Warn: couldn't read dp-vcp_id, set to 1, 2, 3 and 4 by default\n", __func__, __LINE__);
		priv->uivcp_id[0] = 1U;
		priv->uivcp_id[1] = 2U;
		priv->uivcp_id[2] = 3U;
		priv->uivcp_id[3] = 4U;
	}

	ret = ofnode_read_u32(dev->node_, "dp-phy-lane-swap", &priv->uiphy_lane_swap);
	if (ret < 0) {
		/* For KCS */
		pr_warn("[%s:%d]Warn: couldn't read dp-phy-lane-swap, set to 1 by default\n", __func__, __LINE__);
		priv->uiphy_lane_swap = 1U;
	}

	ret = ofnode_read_u32(dev->node_, "dp-sdm-bypass", &priv->uisdm_bypass);
	if (ret < 0) {
		/* For KCS */
		pr_warn("[%s:%d]Warn: couldn't read sdm bypass, set to bypass by default\n", __func__, __LINE__);
		priv->uisdm_bypass = 1U;
	}

	ret = ofnode_read_u32(dev->node_, "dp-trvc-bypass", &priv->uitrvc_bypass);
	if (ret < 0) {
		/* For KCS */
		pr_warn("[%s:%d]Warn: couldn't read trvc bypass, set to bypass by default\n", __func__, __LINE__);
		priv->uitrvc_bypass = 1U;
	}

	ret = ofnode_read_u32(dev->node_, "dp-pixel-encoding", &priv->uipixel_encoding);
	if (ret < 0) {
		/* For KCS */
		pr_warn("[%s:%d]Warn: couldn't read dp-pixel-encoding, set to RGB by default\n", __func__, __LINE__);
		priv->uipixel_encoding = 0U;
	}

	ret = ofnode_read_u32(dev->node_, "dp-max-lane", &priv->uimax_lane);
	if (ret < 0) {
		/* For KCS */
		pr_warn("[%s:%d]Warn: couldn't read dp-max-lane, set to 4 lanes by default\n", __func__, __LINE__);
		priv->uimax_lane = 4U;
	}

	ret = ofnode_read_u32(dev->node_, "dp-max-rate", &priv->uimax_rate);
	if (ret < 0) {
		/* For KCS */
		pr_warn("[%s:%d]Warn: couldn't read dp-max-rate, set to HBR3 by default\n", __func__, __LINE__);
		priv->uimax_rate = 3U;
	}

	for (ucdp_idx = 0; ucdp_idx < priv->ucnum_of_dps; ucdp_idx++) {
		if (priv->uilcdc_mux_id[ucdp_idx] == (uint32_t)LCD_MUX2){
			break;
		}
	}
	if (ucdp_idx == priv->ucnum_of_dps) {
		priv->uisdm_bypass = 0U;
	}

	for (ucdp_idx = 0; ucdp_idx < priv->ucnum_of_dps; ucdp_idx++) {
		if (priv->uilcdc_mux_id[ucdp_idx] == (uint32_t)LCD_MUX3){
			break;
		}
	}
	if (ucdp_idx == priv->ucnum_of_dps) {
		priv->uitrvc_bypass = 0U;
	}

return_funcs:
	return 0;
}

static int tcc_dpv14_init_pinctrl(struct udevice *dev)
{
	int ret = 0;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	ret = pinctrl_select_state(dev, "default");
	if (ret < 0) {
		/* For KCS */
		pr_err("\n[%s:%d]Err: failed to select default state\n", __func__, __LINE__);
	}

	ret = pinctrl_select_state(dev, "power_off");
	if (ret < 0) {
		pr_err("\n[%s:%d]Err: failed to select power_off state\n", __func__, __LINE__);
	}

	ret = pinctrl_select_state(dev, "blk_off");
	if (ret < 0) {
		/* For KCS */
		pr_err("\n[%s:%d]Err: failed to select blk_off state\n", __func__, __LINE__);
	}

	if (priv->uidp_id == (uint32_t)PANEL_DP0) {
		ret = pinctrl_select_state(dev, "dp_hpd");
		if (ret < 0) {
			/* For KCS */
			pr_err("\n[%s:%d]Err: failed to select dp_hpd state\n", __func__, __LINE__);
		}

		ret = pinctrl_select_state(dev, "serdes_intb");
		if (ret < 0) {
			/* For KCS */
			pr_err("\n[%s:%d]Err: failed to select serdes_intb state\n", __func__, __LINE__);
		}

		ret = pinctrl_select_state(dev, "serdes_lock");
		if (ret < 0) {
			/* For KCS */
			pr_err("\n[%s:%d]Err: failed to select serdes_lock state\n", __func__, __LINE__);
		}
	}

	return 0;
}

int tcc_dpv14_enable(struct udevice *dev,
							int panel_bpp,
							const struct display_timing *edid)
{
	uint8_t ucdp_idx;
	int ret = 0;
	uint32_t uip_clk;
	uint32_t auiperi_pclk[PANEL_DP_MAX] = { 0, };
	struct tcc_dpv14_priv *priv_list;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	if (priv->uipanel_mode != 0U) {
		ret = pinctrl_select_state(dev, "pwr_on_1");
		if (ret < 0) {
			pr_err("[%s:%d]Err: failed to select state\n", __func__, __LINE__);
		}

		udelay(20);

		ret = pinctrl_select_state(dev, "pwr_on_2");
		if (ret < 0) {
			pr_err("[%s:%d]Err: failed to select state\n", __func__, __LINE__);
		}

		ret = pinctrl_select_state(dev, "blk_on");
		if (ret < 0) {
			pr_err("[%s:%d]Err: failed to select state\n", __func__, __LINE__);
		}
	}

	uip_clk = (edid->pixelclock.typ / 1000U);
	priv->uipixel_clk = uip_clk;

	lcdc_mux_select(priv->uilcdc_mux_id[priv->uidp_id], priv->uiDc_idx);

	lcdc_display_device_init(priv->parent_dev);

	pr_info("\n[%s:%d]Enable dptx with VIC(%u) :\n", __func__, __LINE__, priv->uivic[priv->uidp_id]);
	pr_info(" 	Display controller %u -> Mux %u\n", priv->uiDc_idx, priv->uilcdc_mux_id[priv->uidp_id]);
	pr_info(" 	Pixel clk = %u\n", edid->pixelclock.typ);
	pr_info(" 	flags = 0x%x \n", edid->flags);
	pr_info(" 	H active(%u) x V active(%u)\n", edid->hactive.typ, edid->vactive.typ);
	pr_info(" 	H front porch(%u), V front porch(%u)\n", edid->hfront_porch.typ, edid->vfront_porch.typ);
	pr_info(" 	H back porch(%u), V back porch(%u)\n", edid->hback_porch.typ, edid->hback_porch.typ);
	pr_info(" 	H sync len(%u), V sync len(%u)\n", edid->hsync_len.typ, edid->hsync_len.typ);

	if ((priv->uidp_id + 1U) != priv->ucnum_of_dps) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	for (ucdp_idx = 0; ucdp_idx < priv->ucnum_of_dps; ucdp_idx++) {
		priv_list = psttcc_dpv14_priv[ucdp_idx];

		auiperi_pclk[ucdp_idx] = priv_list->uipixel_clk;
	}

	ret = dpv14_enable(auiperi_pclk);

	for (ucdp_idx = 0; ucdp_idx < priv->ucnum_of_dps; ucdp_idx++) {
		priv_list = psttcc_dpv14_priv[ucdp_idx];

		lcdc_turn_on_display(priv_list->parent_dev);
	}

 return_funcs:
	return 0;
}

int tcc_dpv14_read_timing(struct udevice *dev, struct display_timing *timing)
{
	uint8_t ucdp_id;
	int32_t ret = 0;
	enum DPV14_DTD_TYPE edtd_type;
	struct dpv14_display_timing stdisplay_timing;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);
	struct tcc_fb_dm_priv *p_priv  = dev_get_priv(priv->parent_dev);

	ucdp_id = (uint8_t)(priv->uidp_id & 0xFFU);

	if ((priv->uipanel_mode != PANEL_MODE_OFF) && (priv->uivic[ucdp_id] == VIC_TO_READ_EDID)) {
		pr_err("[%s:%d]Err: SerDes doesn't support EDID\n", __func__, __LINE__);

		ret = -EINVAL;

		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	edtd_type = (priv->uivic[ucdp_id] == (uint32_t)VIC_TO_READ_EDID) ? (uint8_t)DTD_TYPE_READ_EDID : (uint8_t)DTD_TYPE_CEA861_VIC;

	ret = dpv14_get_dtd(ucdp_id, priv->uivic[ucdp_id], edtd_type, &stdisplay_timing);

	timing->pixelclock.typ = stdisplay_timing.uipixelclock;

	timing->hactive.typ = stdisplay_timing.uihactive;
	timing->hfront_porch.typ = stdisplay_timing.uihfront_porch;
	timing->hback_porch.typ = stdisplay_timing.uihback_porch;
	timing->hsync_len.typ = stdisplay_timing.uihsync_len;

	timing->vactive.typ = stdisplay_timing.uivactive;
	timing->vfront_porch.typ = stdisplay_timing.uivfront_porch;
	timing->vback_porch.typ = stdisplay_timing.uivback_porch;
	timing->vsync_len.typ = stdisplay_timing.uivsync_len;

	timing->flags = stdisplay_timing.flags;

	p_priv->interlaced = stdisplay_timing.uiinterlaced;
	p_priv->pixel_repetition_input = stdisplay_timing.uipixel_repetition;

	pr_debug("\n[%s:%d]VIC(%d) : \n", __func__, __LINE__, priv->uivic[ucdp_id]);
	pr_debug(" Pixel clk = %u \n", timing->pixelclock.typ);
	pr_debug(" Interlace = %u \n", p_priv->interlaced);
	pr_debug(" Pixel repetition = %u \n", p_priv->pixel_repetition_input);
	pr_debug(" flags = 0x%x \n", timing->flags);
	pr_debug(" H active(%u) x V active(%u) \n", timing->hactive.typ, timing->vactive.typ);
	pr_debug(" H front porch(%u), V front porch(%u) \n", timing->hfront_porch.typ, timing->vfront_porch.typ);
	pr_debug(" H back porch(%u), V back porch(%u) \n", timing->hback_porch.typ, timing->hback_porch.typ);
	pr_debug(" H sync len(%u), V sync len(%u) \n", timing->hsync_len.typ, timing->hsync_len.typ);

return_funcs:
	return ret;
}

static int32_t tcc_dpv14_of_to_plat(struct udevice *dev)
{
	int32_t ret = 0;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	(void)tcc_dpv14_parse_dt(dev);

	(void)tcc_dpv14_get_dd_id(dev);

	pr_debug("\n[%s:%d]DP %u of %u DPs...\n",
				__func__,
				__LINE__,
				(uint32_t)priv->uidp_id,
				(uint32_t)priv->ucnum_of_dps);
	pr_debug("                 Display Controller %u -> Mux %u\n",
				priv->uiDc_idx,
				priv->uilcdc_mux_id[priv->uidp_id]);
	pr_debug("                 DP %s\n",
				(priv->uipanel_mode != 0) ? "Panel mode" : "Monitor mode");
	pr_debug("                 I2C Port as %u\n", priv->uii2c_port);
	pr_debug("                 SDM Bypass %s, SRVC Bypass %s\n",
				(priv->uisdm_bypass != 0U) ? "On":"Off",
				(priv->uitrvc_bypass != 0U) ? "On" : "Off");
	pr_debug("                 VIC : %u %u %u %u\n",
				priv->uivic[0],
				priv->uivic[1],
				priv->uivic[2],
				priv->uivic[3]);
	pr_debug("                 VCP id : %u %u %u %u\n",
				priv->uivcp_id[0],
				priv->uivcp_id[1],
				priv->uivcp_id[2],
				priv->uivcp_id[3]);

	return ret;
}

int tcc_dpv14_probe(struct udevice *dev)
{
	uint32_t udtb;
	uint8_t ucDpIdx;
	int ret = 0;
	enum DPV14_RESET_TYPE ereset_type;
	struct dpv14_drv_params stdpv14_drv_params = {0, };
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	if (priv->uipanel_mode != 0U) {
		(void)tcc_dpv14_init_pinctrl(dev);
	}

	if (priv->uidp_id != (uint32_t)PANEL_DP0) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto return_funcs;
	}

	stdpv14_drv_params.ucnum_of_dps = (uint8_t)priv->ucnum_of_dps;
	stdpv14_drv_params.ucdp_id = (uint8_t)priv->uidp_id;
	stdpv14_drv_params.bpanel_mode = (bool)priv->uipanel_mode;
	stdpv14_drv_params.bphy_lane_swap = (bool)priv->uiphy_lane_swap;
	stdpv14_drv_params.bsdm_bypass = (bool)priv->uisdm_bypass;
	stdpv14_drv_params.btrvc_bypass = (bool)priv->uitrvc_bypass;
	stdpv14_drv_params.uci2c_port = (uint8_t)priv->uii2c_port;
	stdpv14_drv_params.ucpixel_encoding = (uint8_t)priv->uipixel_encoding;
	stdpv14_drv_params.ucmax_lane = (uint8_t)priv->uimax_lane;
	stdpv14_drv_params.ucmax_rate = (uint8_t)priv->uimax_rate;

	for (ucDpIdx = 0; ucDpIdx < (uint8_t)PANEL_DP_MAX; ucDpIdx++) {
		stdpv14_drv_params.uidd_mux_id[ucDpIdx] = priv->uilcdc_mux_id[ucDpIdx];
		stdpv14_drv_params.uivic[ucDpIdx] = priv->uivic[ucDpIdx];
		stdpv14_drv_params.ucvcp_id[ucDpIdx] = (uint8_t)priv->uivcp_id[ucDpIdx];
	}

	(void)dpv14_get_reset_type(&ereset_type);

	if ((!stdpv14_drv_params.btrvc_bypass) && (ereset_type == RESET_TYPE_CORE)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_info("\n[%s:%d]DP %d -> Core reset with T-RVC Bypass\n",
							__func__,
							__LINE__,
							stdpv14_drv_params.ucdp_id);

		stdpv14_drv_params.bskip_dp_init = (bool)true;
	}

	stdpv14_drv_params.bphy_lane_swap =  ofnode_read_bool(dev->node_, "phy_lane_swap");
	ret = ofnode_read_u32(dev->node_, "evb_power_type", &udtb);
	if ((ret < 0) || (udtb >= TCC_EVB_LCD_POW_MAX)) {
		/* For KCS */
		pr_warn("[%s:%d]Warn: couldn't read evb power type, set to evb power type by default\n", __func__, __LINE__);
		udtb = TCC_EVB_LCD_FOUR_POW;
		stdpv14_drv_params.bphy_lane_swap = (bool)true;
	}
	stdpv14_drv_params.ucevb_power_type = (uint8_t)udtb;
	ret = ofnode_read_u32(dev->node_, "phy_model", &udtb);
	if ((ret < 0) || udtb >= DPTX_PHY_DEVICE_UNKNOWN) {
		/* For KCS */
		pr_warn("[%s:%d]Warn: couldn't read phy model, set to phy model by default\n", __func__, __LINE__);
		udtb = DPTX_PHY_DEVICE_SNPS;
	}
	stdpv14_drv_params.ucphy_model = (uint8_t)udtb;

	ret = dpv14_Init(&stdpv14_drv_params);

	pr_info("\n[%s:%d]tcc_dpv14_dm Ver : %d.%d.%d -> DP %u probe\n",
					__func__,
					__LINE__,
					DPV14_DM_DRV_MAJOR_VER,
					DPV14_DM_DRV_MINOR_VER,
					DPV14_DM_DRV_PATCH_VER,
					stdpv14_drv_params.ucdp_id);
	pr_debug(" Num of DPs : %u", stdpv14_drv_params.ucnum_of_dps);
	pr_debug(" DP %s\n", (stdpv14_drv_params.bpanel_mode) ? "Panel mode" : "Monitor mode");
	if (stdpv14_drv_params.bpanel_mode) {
		/* For KCS */
		pr_debug(" I2C Port : %u\n", stdpv14_drv_params.uci2c_port);
	}
	pr_debug(" SDM Bypass %s, SRVC Bypass %s\n",
				(stdpv14_drv_params.bsdm_bypass) ? "On" : "Off",
				(stdpv14_drv_params.btrvc_bypass) ? "On" : "Off");
	pr_debug(" Phy lane swap = %s\n",
				(stdpv14_drv_params.bphy_lane_swap) ? "On" : "Off");
	pr_debug(" Encoding type: %s\n",
				(stdpv14_drv_params.ucpixel_encoding == (uint8_t)0U) ? "RGB" :
				(stdpv14_drv_params.ucpixel_encoding == (uint8_t)1U) ? "YCbCr222" :"YCbCr444");
	pr_debug(" Max rate: %s, Max lane: %s\n",
				(stdpv14_drv_params.ucmax_rate == (uint8_t)0U) ? "RBR" :
				(stdpv14_drv_params.ucmax_rate == (uint8_t)1U) ? "HBR" :
				(stdpv14_drv_params.ucmax_rate == (uint8_t)2U) ? "HBR2" :"HBR3",
				(stdpv14_drv_params.ucmax_lane == (uint8_t)1U) ? "1 lane" :
				(stdpv14_drv_params.ucmax_lane == (uint8_t)2U) ? "2 lanes" :"4 lanes");
	pr_debug(" Vcp id : %u %u %u %u\n",
				stdpv14_drv_params.ucvcp_id[0],
				stdpv14_drv_params.ucvcp_id[1],
				stdpv14_drv_params.ucvcp_id[2],
				stdpv14_drv_params.ucvcp_id[3]);
	pr_debug(" VIC : %u %u %u %u\n",
				stdpv14_drv_params.uivic[0],
				stdpv14_drv_params.uivic[1],
				stdpv14_drv_params.uivic[2],
				stdpv14_drv_params.uivic[3]);
	pr_debug(" Mux id : %u %u %u %u\n",
				stdpv14_drv_params.uidd_mux_id[0],
				stdpv14_drv_params.uidd_mux_id[1],
				stdpv14_drv_params.uidd_mux_id[2],
				stdpv14_drv_params.uidd_mux_id[3]);

return_funcs:
	return ret;
}


static const struct dm_display_ops dpv14_tcc_ops = {
	.read_timing = tcc_dpv14_read_timing,
	.enable = tcc_dpv14_enable,
};

static const struct udevice_id tcc_dpv14_ids[] = {
	{ .compatible = "telechips,fb-dpv14-panel" },
	{ }
};

U_BOOT_DRIVER(dpv14_tcc) = {
	.name = "dpv14_tcc",
	.id = UCLASS_DISPLAY,
	.of_match = tcc_dpv14_ids,
	.ops = &dpv14_tcc_ops,
	.of_to_plat = tcc_dpv14_of_to_plat,
	.probe = tcc_dpv14_probe,
	.priv_auto = sizeof(struct tcc_dpv14_priv),
};
