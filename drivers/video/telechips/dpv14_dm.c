// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <linux/delay.h>
#include <display.h>
#include <dm.h>
#include <bootstage.h>
#include <telechips/fb_dm.h>
#include <telechips/dpv14_ctrl.h>
#include <telechips/dpv14_max968xx.h>
#include <telechips/fb_dm_lcd_interface.h>
#include <dm/uclass-internal.h>
#include <dm/pinctrl.h>
#include <dt-bindings/display/telechips-dispdef.h>

#define DPV14_DM_DRV_MAJOR_VER		1
#define DPV14_DM_DRV_MINOR_VER		0
#define DPV14_DM_DRV_PATCH_VER		3

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
	uint32_t power_type;
	uint32_t phy_model;
	uint32_t uivcp_id[PANEL_DP_MAX];
	uint32_t uivic[PANEL_DP_MAX];
	uint32_t uilcdc_mux_id[PANEL_DP_MAX];

	/*
	 * The AUO LCD panel must transmit the LVDS signal first and then turn
	 * on the backlight approximately 240ms later, when initializing the
	 * panel
	 */
	bool panel_auo;

#if defined(GET_MUXID_FROM_FB)
	struct udevice *pre_pudev[PANEL_DP_MAX];
#endif

	struct udevice *parent_dev;
};

static struct tcc_dpv14_priv *dpv14_privs[PANEL_DP_MAX] = {NULL, NULL, NULL, NULL};


static int32_t tcc_dpv14_get_dd_id(const struct udevice *dev)
{
	int32_t ret = 0;
	uint32_t remote_phandle;
	ofnode ports_ofnode, port_ofnode, sub_ofnode, remote_ofnode;
	struct tcc_fb_dm_priv *p_priv;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);
	struct udevice *p_dev = NULL;

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
				 __func__, __LINE__);
		}
	}

	if(ret == 0) {
		remote_ofnode = ofnode_get_by_phandle(remote_phandle);
		if (!ofnode_valid(remote_ofnode)) {
			debug_pr(LOGL_ERR, "\n[%s:%d]Err: remote ofnode isn't valid\n",
				 __func__, __LINE__);

			ret = -ENODEV;
		}
	}

	if(ret == 0) {
		while((ofnode_valid(remote_ofnode)) && (ret == 0)) {
			remote_ofnode = ofnode_get_parent(remote_ofnode);
			if (!ofnode_valid(remote_ofnode)) {
				debug_pr(LOGL_ERR, "\n[%s:%d](%s)Err: parent remote ofnode isn't valid\n",
					 __func__, __LINE__, dev_read_name(dev));
				ret = -ENODEV;
			} else {
				(void)uclass_find_device_by_ofnode(UCLASS_VIDEO, remote_ofnode, &p_dev);
				if (p_dev != NULL) {
					/*For KCS*/
					break;
				}
			}
		}
	}

	if(p_dev != NULL) {
		priv->parent_dev = p_dev;
		p_priv = (struct tcc_fb_dm_priv *)p_dev->priv_;
		priv->uiDc_idx = get_vioc_index(p_priv->lcd_disp);
		p_priv->lcd_pxdw = priv->uipixel_encoding;

		debug_pr(LOGL_INFO, "[%s:%d]Found UCLASS_VIDEO node(%s) => dc id(%u)\n",
			 __func__, __LINE__, dev_read_name(dev), priv->uiDc_idx);
	}

	return ret;
}

#if defined(GET_MUXID_FROM_FB)
static int32_t tcc_dpv14_get_mux_id(const struct udevice *dev)
{
	const struct udevice *pre_udev;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);
	int32_t ret = 0;
	uint8_t ucdpIdx;

	for (ucdpIdx = 0; ucdpIdx < priv->ucnum_of_dps; ucdpIdx++) {
		pre_udev = priv->pre_pudev[ucdpIdx];
		if (pre_udev == NULL) {
			debug_pr(LOGL_ERR, "[%s:%d]Err: parent node isn't valid for Dp%d\n", __func__, __LINE__, ucdpIdx);
			break;
		}

		ret = ofnode_read_u32(dev_ofnode(pre_udev), "lcd-mux-id", &priv->uilcdc_mux_id[ucdpIdx]);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[%s:%d]Warn: couldn't read lcd-mux-id for Dp%d\n", __func__, __LINE__, ucdpIdx);
		}
	}

	return ret;
}
#endif

static int32_t tcc_dpv14_get_num_of_dps(const struct udevice *dev)
{
	const char *pcremote_dev_name;
	const char *string_ptr;
	int32_t ret = 0;
	uint32_t remote_phandle;
	struct udevice *p_udev;
	struct udevice *p_dev = NULL;
	ofnode ports_ofnode, port_ofnode, sub_ofnode, remote_ofnode;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	for ((void)uclass_find_first_device(UCLASS_VIDEO, &p_udev);
	      p_udev != NULL; (void)uclass_find_next_device(&p_udev)) {
		if (!ofnode_valid(dev_ofnode(p_udev))) {
			debug_pr(LOGL_DEBUG, "[%s:%d](%s)'port' subnode isn't valid\n",
				 __func__, __LINE__, dev_read_name(p_udev));
			continue;
		}
		ports_ofnode = dev_read_subnode(p_udev, "ports");
		if (!ofnode_valid(ports_ofnode)) {
			ports_ofnode = dev_ofnode(p_udev);
		}
		port_ofnode = ofnode_find_subnode(ports_ofnode, "port");
		if (!ofnode_valid(port_ofnode)) {
			debug_pr(LOGL_DEBUG, "[%s:%d](%s): 'port' node not found\n",
				 __func__, __LINE__, dev_read_name(p_udev));

			continue;
		}

		sub_ofnode = ofnode_first_subnode(port_ofnode);

		ret = ofnode_read_u32(sub_ofnode, "remote-endpoint", &remote_phandle);
		if (ret != 0) {
			debug_pr(LOGL_DEBUG, "[%s:%d] can't find remote-endpoint\n", __func__, __LINE__);
			continue;
		}

		remote_ofnode = ofnode_get_by_phandle(remote_phandle);
		if (!ofnode_valid(remote_ofnode)) {
			debug_pr(LOGL_DEBUG, "[%s:%d]remote ofnode isn't available\n",
				 __func__, __LINE__);

			continue;
		}

		while((ofnode_valid(remote_ofnode)) && (ret == 0)) {
			remote_ofnode = ofnode_get_parent(remote_ofnode);
			if (!ofnode_valid(remote_ofnode)) {
				debug_pr(LOGL_DEBUG, "[%s:%d](%s)parent remote ofnode isn't valid\n",
					 __func__, __LINE__, dev_read_name(p_udev));
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
			debug_pr(LOGL_DEBUG, "[%s:%d](%s)parent dev isn't valid\n",
				 __func__, __LINE__, dev_read_name(p_udev));
			continue;
		}
		pcremote_dev_name = dev_read_name(p_dev);
		string_ptr = strstr(pcremote_dev_name, "dpv");
		if (string_ptr == NULL) {
			debug_pr(LOGL_INFO, "[%s:%d]not found dp device <-> %s\n",
				 __func__, __LINE__, pcremote_dev_name);
			continue;
		}
		#if defined(GET_MUXID_FROM_FB)
		priv->pre_pudev[priv->ucnum_of_dps] = p_udev;
		#endif
		if (priv->ucnum_of_dps >= 0xFFU) {
			priv->ucnum_of_dps = 0U;
		} else {
			priv->ucnum_of_dps++;
		}
		debug_pr(LOGL_INFO, "\n[%s:%d]Found %u dp node as name(%s)\n",
			 __func__, __LINE__, priv->ucnum_of_dps, pcremote_dev_name);
	}
	return ret;
}

static int32_t tcc_dpv14_parse_dt(struct udevice *dev)
{
	uint8_t ucdp_idx;
	int32_t ret = 0;
	uint32_t uidp_id;
	const struct tcc_dpv14_priv *master_priv;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	(void)memset(priv, 0, sizeof(struct tcc_dpv14_priv));

	for (uidp_id = 0; uidp_id < (uint32_t)PANEL_DP_MAX; uidp_id++) {
		if (dpv14_privs[uidp_id] == NULL) {
			dpv14_privs[uidp_id] = priv;
			break;
		}
	}
	if (uidp_id >= (uint32_t)PANEL_DP_MAX) {
		debug_pr(LOGL_ERR, "\n[%s:%d]Err: Invalid DP id as %u\n",
			 __func__, __LINE__, uidp_id);

		ret = -EINVAL;
	}
	if(ret == 0) {
		switch (uidp_id) {
		case (uint32_t)PANEL_DP0:
			ret = tcc_dpv14_get_num_of_dps(dev);
			if (ret != 0) {
				ret = -EINVAL;
			}
			#if defined(GET_MUXID_FROM_FB)
			if(ret == 0) {
				(void)tcc_dpv14_get_mux_id(dev);
			}
			#endif
			break;
		case (uint32_t)PANEL_DP1:
		case (uint32_t)PANEL_DP2:
		case (uint32_t)PANEL_DP3:
		default:
			master_priv = dpv14_privs[PANEL_DP0];
			(void)memcpy(priv, master_priv, sizeof(struct tcc_dpv14_priv));
			break;
		}
		priv->uidp_id = uidp_id;
	}

	if(ret == 0) {
		if (uidp_id == (uint32_t)PANEL_DP0) {
			ret = ofnode_read_u32(dev_ofnode(dev), "dp-panel-mode",
					&priv->uipanel_mode);
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_WARNING,
					"[%s:%d]Warn: couldn't read panel mode, set to panel mode by default\n",
					__func__, __LINE__);
				priv->uipanel_mode = 1U;
			}

			ret = ofnode_read_u32(dev_ofnode(dev), "dp-serdes-i2c-port", &priv->uii2c_port);
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_WARNING,
					"[%s:%d]Warn: couldn't read i2c port, set to D3(0), D5(4) by default\n",
					__func__, __LINE__);
				priv->uii2c_port = (uint32_t)I2C_PORT_TO_SERDES;
			}
			ret = ofnode_read_u32_array(dev_ofnode(dev), "dp-vic", priv->uivic, (size_t)PANEL_DP_MAX);
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_WARNING,
					"[%s:%d]Warn: couldn't read dp-vic, set to 1(480p) by default\n",
					__func__, __LINE__);
				priv->uivic[0] = 1U;
				priv->uivic[1] = 1U;
				priv->uivic[2] = 1U;
				priv->uivic[3] = 1U;
			}

			ret = ofnode_read_u32_array(dev_ofnode(dev), "dp-vcp_id",
						priv->uivcp_id, (size_t)PANEL_DP_MAX);
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_WARNING,
					"[%s:%d]Warn: couldn't read dp-vcp_id, set to 1, 2, 3 and 4 by default\n",
					__func__, __LINE__);
				priv->uivcp_id[0] = 1U;
				priv->uivcp_id[1] = 2U;
				priv->uivcp_id[2] = 3U;
				priv->uivcp_id[3] = 4U;
			}

			ret = ofnode_read_u32(dev_ofnode(dev), "dp-phy-lane-swap", &priv->uiphy_lane_swap);
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_WARNING,
					"[%s:%d]Warn: couldn't read dp-phy-lane-swap, set to 1 by default\n",
					__func__, __LINE__);
				priv->uiphy_lane_swap = 1U;
			}

			ret = ofnode_read_u32(dev_ofnode(dev), "dp-sdm-bypass", &priv->uisdm_bypass);
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_WARNING,
					"[%s:%d]Warn: couldn't read sdm bypass, set to bypass by default\n",
					__func__, __LINE__);
				priv->uisdm_bypass = 1U;
			}

			ret = ofnode_read_u32(dev_ofnode(dev), "dp-trvc-bypass", &priv->uitrvc_bypass);
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_WARNING,
					"[%s:%d]Warn: couldn't read trvc bypass, set to bypass by default\n",
					__func__, __LINE__);
				priv->uitrvc_bypass = 1U;
			}

			ret = ofnode_read_u32(dev_ofnode(dev), "dp-pixel-encoding", &priv->uipixel_encoding);
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_WARNING,
					"[%s:%d]Warn: couldn't read dp-pixel-encoding, set to RGB by default\n",
					__func__, __LINE__);
				priv->uipixel_encoding = 0U;
			}

			ret = ofnode_read_u32(dev_ofnode(dev), "dp-max-lane", &priv->uimax_lane);
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_WARNING,
					"[%s:%d]Warn: couldn't read dp-max-lane, set to 4 lanes by default\n",
					__func__, __LINE__);
				priv->uimax_lane = 4U;
			}

			ret = ofnode_read_u32(dev_ofnode(dev), "dp-max-rate", &priv->uimax_rate);
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_WARNING,
					"[%s:%d]Warn: couldn't read dp-max-rate, set to HBR3 by default\n",
					__func__, __LINE__);
				priv->uimax_rate = 3U;
			}

			ret = ofnode_read_u32(dev_ofnode(dev), "evb_power_type", &priv->power_type);
			if ((ret < 0) || (priv->power_type >= TCC_EVB_LCD_POW_MAX)) {
				/* For KCS */
				debug_pr(LOGL_WARNING, "[%s:%d]Warn: couldn't read evb power type, set to evb power type by default\n", __func__, __LINE__);
				priv->power_type = TCC_EVB_LCD_FOUR_POW;
			}

			ret = ofnode_read_u32(dev_ofnode(dev), "phy_model", &priv->phy_model);
			if ((ret < 0) || (priv->phy_model >= DPTX_PHY_DEVICE_UNKNOWN)) {
				/* For KCS */
				debug_pr(LOGL_WARNING, "[%s:%d]Warn: couldn't read phy model, set to phy model by default\n", __func__, __LINE__);
				priv->phy_model = DPTX_PHY_DEVICE_SNPS;
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
		}
		if ((priv->uipanel_mode != 0U) && (priv->uivic[uidp_id] == 1028)) {
			priv->panel_auo = (bool)true;
		} else {
			priv->panel_auo = (bool)false;
		}
	}

	return 0;
}

static int tcc_dpv14_init_pinctrl(struct udevice *dev)
{
	int ret = 0;
	const struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	if (priv->uipanel_mode != 0U) {
		ret = pinctrl_select_state(dev, "default");
		if (ret < 0) {
			/* For KCS */
			debug_pr(LOGL_ERR,
				"\n[%s:%d]Err: failed to select default state\n",
				__func__, __LINE__);
		}
		ret = pinctrl_select_state(dev, "blk_off");
		if (ret < 0) {
			/* For KCS */
			debug_pr(LOGL_ERR,
				"\n[%s:%d]Err: failed to select blk_off state\n",
				__func__, __LINE__);
		}
		ret = pinctrl_select_state(dev, "lcd_off");
		if (ret < 0) {
			debug_pr(LOGL_ERR,
				"\n[%s:%d]Err: failed to select lcd_off state\n",
				__func__, __LINE__);
		}
		ret = pinctrl_select_state(dev, "reset_on");
		if (ret < 0) {
			debug_pr(LOGL_ERR,
				"\n[%s:%d]Err: failed to select reset_on state\n",
				__func__, __LINE__);
		}

		if (priv->uidp_id == (uint32_t)PANEL_DP0) {
			ret = pinctrl_select_state(dev, "serdes_intb");
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_ERR,
					"\n[%s:%d]Err: failed to select serdes_intb state\n",
					__func__, __LINE__);
			}

			ret = pinctrl_select_state(dev, "serdes_lock");
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_ERR,
					"\n[%s:%d]Err: failed to select serdes_lock state\n",
					__func__, __LINE__);
			}
		}
	}
	if (priv->uidp_id == (uint32_t)PANEL_DP0) {
		ret = pinctrl_select_state(dev, "dp_hpd");
		if (ret < 0) {
			/* For KCS */
			debug_pr(LOGL_ERR,
				"\n[%s:%d]Err: failed to select dp_hpd state\n",
				__func__, __LINE__);
		}
	}
	return 0;
}

static int tcc_dpv14_enable(struct udevice *dev, int panel_bpp,
			    const struct display_timing *edid)
{
	uint8_t ucdp_idx;
	int ret = 0;
	uint32_t uip_clk;
	uint32_t auiperi_pclk[PANEL_DP_MAX] = { 0, };
	const struct tcc_dpv14_priv *priv_list;
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	if (priv->uipanel_mode != 0U) {
		if (!priv->panel_auo) {
			ret = pinctrl_select_state(dev, "lcd_on");
			if (ret < 0) {
				debug_pr(LOGL_ERR, "[%s:%d]Err: failed to select state\n", __func__, __LINE__);
			}
			udelay(20);
			ret = pinctrl_select_state(dev, "reset_off");
			if (ret < 0) {
				debug_pr(LOGL_ERR, "[%s:%d]Err: failed to select state\n", __func__, __LINE__);
			}

			ret = pinctrl_select_state(dev, "blk_on");
			if (ret < 0) {
				debug_pr(LOGL_ERR, "[%s:%d]Err: failed to select state\n", __func__, __LINE__);
			}
		}
	}

	uip_clk = (edid->pixelclock.typ / 1000U);
	priv->uipixel_clk = uip_clk;

	(void)lcdc_mux_select(priv->uilcdc_mux_id[priv->uidp_id], priv->uiDc_idx);

	(void)lcdc_display_device_init(priv->parent_dev);

	debug_pr(LOGL_INFO, "\n[%s:%d]Enable dptx with VIC(%u) :\n", __func__, __LINE__, priv->uivic[priv->uidp_id]);
	debug_pr(LOGL_INFO, " 	panel bpp = %u\n", panel_bpp);
	debug_pr(LOGL_INFO, " 	Display controller %u -> Mux %u\n", priv->uiDc_idx, priv->uilcdc_mux_id[priv->uidp_id]);
	debug_pr(LOGL_INFO, " 	Pixel clk = %u\n", edid->pixelclock.typ);
	debug_pr(LOGL_INFO, " 	flags = 0x%x \n", edid->flags);
	debug_pr(LOGL_INFO, " 	H active(%u) x V active(%u)\n", edid->hactive.typ, edid->vactive.typ);
	debug_pr(LOGL_INFO, " 	H front porch(%u), V front porch(%u)\n", edid->hfront_porch.typ, edid->vfront_porch.typ);
	debug_pr(LOGL_INFO, " 	H back porch(%u), V back porch(%u)\n", edid->hback_porch.typ, edid->hback_porch.typ);
	debug_pr(LOGL_INFO, " 	H sync len(%u), V sync len(%u)\n", edid->hsync_len.typ, edid->hsync_len.typ);

	if ((priv->uidp_id + 1U) == priv->ucnum_of_dps) {
		for (ucdp_idx = 0; ucdp_idx < priv->ucnum_of_dps; ucdp_idx++) {
			priv_list = dpv14_privs[ucdp_idx];

			auiperi_pclk[ucdp_idx] = priv_list->uipixel_clk;
		}

		ret = dpv14_enable(auiperi_pclk);

		for (ucdp_idx = 0; ucdp_idx < priv->ucnum_of_dps; ucdp_idx++) {
			priv_list = dpv14_privs[ucdp_idx];

			/* The AUO panel must be reset before signal input. */
			if (priv_list->panel_auo) {
				struct tcc_fb_dm_priv *fb_priv = dev_get_priv(priv_list->parent_dev);

				ret = pinctrl_select_state(fb_priv->output_media, "lcd_on");
				if (ret < 0) {
					debug_pr(LOGL_ERR,
						"\n[%s:%d]Err: failed to select lcd_off state\n",
						__func__, __LINE__);
				}
				mdelay(1);
				ret = pinctrl_select_state(fb_priv->output_media, "reset_off");
				if (ret < 0) {
					debug_pr(LOGL_ERR, "[%s:%d]Err: failed to select state\n", __func__, __LINE__);
				}
			}

			(void)lcdc_turn_on_display(priv_list->parent_dev);
		}
	}

	return ret;
}

static int tcc_dpv14_read_timing(struct udevice *dev,
				 struct display_timing *out_timing)
{
	uint8_t ucdp_id;
	int32_t ret = 0;
	enum DPV14_DTD_TYPE edtd_type;
	struct dpv14_display_timing stdisplay_timing;
	const struct tcc_dpv14_priv *priv = dev_get_priv(dev);
	struct tcc_fb_dm_priv *p_priv  = dev_get_priv(priv->parent_dev);

	ucdp_id = (uint8_t)(priv->uidp_id & 0xFFU);

	if ((priv->uipanel_mode != PANEL_MODE_OFF) && (priv->uivic[ucdp_id] == VIC_TO_READ_EDID)) {
		debug_pr(LOGL_ERR, "[%s:%d]Err: SerDes doesn't support EDID\n", __func__, __LINE__);

		ret = -EINVAL;
	}

	if(ret == 0) {
		edtd_type = (priv->uivic[ucdp_id] == (uint32_t)VIC_TO_READ_EDID) ? DTD_TYPE_READ_EDID : DTD_TYPE_CEA861_VIC;

		(void)memset(&stdisplay_timing, 0, sizeof(stdisplay_timing));
		ret = dpv14_get_dtd(ucdp_id, priv->uivic[ucdp_id], edtd_type, &stdisplay_timing);

		out_timing->pixelclock.typ = stdisplay_timing.uipixelclock;

		out_timing->hactive.typ = stdisplay_timing.uihactive;
		out_timing->hfront_porch.typ = stdisplay_timing.uihfront_porch;
		out_timing->hback_porch.typ = stdisplay_timing.uihback_porch;
		out_timing->hsync_len.typ = stdisplay_timing.uihsync_len;

		out_timing->vactive.typ = stdisplay_timing.uivactive;
		out_timing->vfront_porch.typ = stdisplay_timing.uivfront_porch;
		out_timing->vback_porch.typ = stdisplay_timing.uivback_porch;
		out_timing->vsync_len.typ = stdisplay_timing.uivsync_len;

		if(stdisplay_timing.flags <= (unsigned)DISPLAY_FLAGS_DOUBLECLK) {
			out_timing->flags = (enum display_flags)stdisplay_timing.flags;
		}

		p_priv->interlaced = stdisplay_timing.uiinterlaced;
		p_priv->pixel_repetition_input = stdisplay_timing.uipixel_repetition;

		debug_pr(LOGL_DEBUG, "\n[%s:%d]VIC(%d) : \n", __func__, __LINE__, priv->uivic[ucdp_id]);
		debug_pr(LOGL_DEBUG, " Pixel clk = %u \n", out_timing->pixelclock.typ);
		debug_pr(LOGL_DEBUG, " Interlace = %u \n", p_priv->interlaced);
		debug_pr(LOGL_DEBUG, " Pixel repetition = %u \n", p_priv->pixel_repetition_input);
		debug_pr(LOGL_DEBUG, " flags = 0x%x \n", out_timing->flags);
		debug_pr(LOGL_DEBUG, " H active(%u) x V active(%u) \n", out_timing->hactive.typ, out_timing->vactive.typ);
		debug_pr(LOGL_DEBUG, " H front porch(%u), V front porch(%u) \n", out_timing->hfront_porch.typ, out_timing->vfront_porch.typ);
		debug_pr(LOGL_DEBUG, " H back porch(%u), V back porch(%u) \n", out_timing->hback_porch.typ, out_timing->hback_porch.typ);
		debug_pr(LOGL_DEBUG, " H sync len(%u), V sync len(%u) \n", out_timing->hsync_len.typ, out_timing->hsync_len.typ);
	}

	return ret;
}

static int32_t tcc_dpv14_of_to_plat(struct udevice *dev)
{
	int32_t ret = 0;
	const struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	(void)tcc_dpv14_parse_dt(dev);

	(void)tcc_dpv14_get_dd_id(dev);

	debug_pr(LOGL_DEBUG, "\n[%s:%d]DP %u of %u DPs...\n",
		 __func__, __LINE__, (uint32_t)priv->uidp_id,
		 (uint32_t)priv->ucnum_of_dps);
	debug_pr(LOGL_DEBUG, "                 Display Controller %u -> Mux %u\n",
		 priv->uiDc_idx, priv->uilcdc_mux_id[priv->uidp_id]);
	debug_pr(LOGL_DEBUG, "                 DP %s\n",
		 (priv->uipanel_mode != 0U) ? "Panel mode" : "Monitor mode");
	debug_pr(LOGL_DEBUG, "                 I2C Port as %u\n",
		 priv->uii2c_port);
	debug_pr(LOGL_DEBUG, "                 SDM Bypass %s, SRVC Bypass %s\n",
		 (priv->uisdm_bypass != 0U) ? "On":"Off",
		 (priv->uitrvc_bypass != 0U) ? "On" : "Off");
	debug_pr(LOGL_DEBUG, "                 VIC : %u %u %u %u\n",
		 priv->uivic[0], priv->uivic[1], priv->uivic[2], priv->uivic[3]);
	debug_pr(LOGL_DEBUG, "                 VCP id : %u %u %u %u\n",
		 priv->uivcp_id[0], priv->uivcp_id[1],
		 priv->uivcp_id[2], priv->uivcp_id[3]);

	return ret;
}

static int tcc_dpv14_probe(struct udevice *dev)
{
	uint8_t ucDpIdx;
	int ret = 0;
	enum DPV14_RESET_TYPE ereset_type;
	struct dpv14_drv_params stdpv14_drv_params = {0, };
	const struct tcc_dpv14_priv *priv = dev_get_priv(dev);

	(void)tcc_dpv14_init_pinctrl(dev);

	if (priv->uidp_id == (uint32_t)PANEL_DP0) {
		stdpv14_drv_params.ucnum_of_dps = (uint8_t)priv->ucnum_of_dps;
		stdpv14_drv_params.ucdp_id = (uint8_t)priv->uidp_id;
		stdpv14_drv_params.bpanel_mode = (bool)priv->uipanel_mode;
		stdpv14_drv_params.bphy_lane_swap = priv->uiphy_lane_swap;
		stdpv14_drv_params.bsdm_bypass = (bool)priv->uisdm_bypass;
		stdpv14_drv_params.btrvc_bypass = (bool)priv->uitrvc_bypass;
		stdpv14_drv_params.uci2c_port = (uint8_t)(priv->uii2c_port & 0xFFu);
		stdpv14_drv_params.ucpixel_encoding = (uint8_t)(priv->uipixel_encoding & 0xFFU);
		stdpv14_drv_params.ucmax_lane = (uint8_t)(priv->uimax_lane & 0xFFu);
		stdpv14_drv_params.ucmax_rate = (uint8_t)(priv->uimax_rate & 0xFFu);
		stdpv14_drv_params.ucevb_power_type = (uint8_t)(priv->power_type & 0xFFu);
		stdpv14_drv_params.ucphy_model = (uint8_t)(priv->phy_model & 0xFFu);

		for (ucDpIdx = 0; ucDpIdx < (uint8_t)PANEL_DP_MAX; ucDpIdx++) {
			stdpv14_drv_params.uidd_mux_id[ucDpIdx] = priv->uilcdc_mux_id[ucDpIdx];
			stdpv14_drv_params.uivic[ucDpIdx] = priv->uivic[ucDpIdx];
			stdpv14_drv_params.ucvcp_id[ucDpIdx] = (uint8_t)(priv->uivcp_id[ucDpIdx] & 0xFFU);
		}

		(void)dpv14_get_reset_type(&ereset_type);

		if ((!stdpv14_drv_params.btrvc_bypass) && (ereset_type == RESET_TYPE_CORE)) {
			debug_pr(LOGL_INFO, "\n[%s:%d]DP %d -> Core reset with T-RVC Bypass\n",
								__func__,
								__LINE__,
								stdpv14_drv_params.ucdp_id);

			stdpv14_drv_params.bskip_dp_init = (bool)true;
		}

		ret = dpv14_Init(&stdpv14_drv_params);

		debug_pr(LOGL_INFO, "\n[%s:%d]tcc_dpv14_dm Ver : %d.%d.%d -> DP %u probe\n",
						__func__,
						__LINE__,
						DPV14_DM_DRV_MAJOR_VER,
						DPV14_DM_DRV_MINOR_VER,
						DPV14_DM_DRV_PATCH_VER,
						stdpv14_drv_params.ucdp_id);
		debug_pr(LOGL_DEBUG, " Num of DPs : %u", stdpv14_drv_params.ucnum_of_dps);
		debug_pr(LOGL_DEBUG, " DP %s\n", (stdpv14_drv_params.bpanel_mode) ? "Panel mode" : "Monitor mode");
		if (stdpv14_drv_params.bpanel_mode) {
			/* For KCS */
			debug_pr(LOGL_DEBUG, " I2C Port : %u\n", stdpv14_drv_params.uci2c_port);
		}
		debug_pr(LOGL_DEBUG, " SDM Bypass %s, SRVC Bypass %s\n",
					(stdpv14_drv_params.bsdm_bypass) ? "On" : "Off",
					(stdpv14_drv_params.btrvc_bypass) ? "On" : "Off");
		debug_pr(LOGL_DEBUG, " Phy lane swap = %s\n",
					(stdpv14_drv_params.bphy_lane_swap) ? "On" : "Off");
		debug_pr(LOGL_DEBUG, " Encoding type: %s\n",
					(stdpv14_drv_params.ucpixel_encoding == (uint8_t)0U) ? "RGB" :
					(stdpv14_drv_params.ucpixel_encoding == (uint8_t)1U) ? "YCbCr222" :"YCbCr444");
		debug_pr(LOGL_DEBUG, " Max rate: %s, Max lane: %s\n",
					(stdpv14_drv_params.ucmax_rate == (uint8_t)0U) ? "RBR" :
					(stdpv14_drv_params.ucmax_rate == (uint8_t)1U) ? "HBR" :
					(stdpv14_drv_params.ucmax_rate == (uint8_t)2U) ? "HBR2" :"HBR3",
					(stdpv14_drv_params.ucmax_lane == (uint8_t)1U) ? "1 lane" :
					(stdpv14_drv_params.ucmax_lane == (uint8_t)2U) ? "2 lanes" :"4 lanes");
		debug_pr(LOGL_DEBUG, " Vcp id : %u %u %u %u\n",
					stdpv14_drv_params.ucvcp_id[0],
					stdpv14_drv_params.ucvcp_id[1],
					stdpv14_drv_params.ucvcp_id[2],
					stdpv14_drv_params.ucvcp_id[3]);
		debug_pr(LOGL_DEBUG, " VIC : %u %u %u %u\n",
					stdpv14_drv_params.uivic[0],
					stdpv14_drv_params.uivic[1],
					stdpv14_drv_params.uivic[2],
					stdpv14_drv_params.uivic[3]);
		debug_pr(LOGL_DEBUG, " Mux id : %u %u %u %u\n",
					stdpv14_drv_params.uidd_mux_id[0],
					stdpv14_drv_params.uidd_mux_id[1],
					stdpv14_drv_params.uidd_mux_id[2],
					stdpv14_drv_params.uidd_mux_id[3]);
	}
	return ret;
}

static uint32_t tcc_dpv14_backlight_wait_time(struct udevice *dev)
{
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);
	uint32_t wait_time = 0u;

	if (priv->panel_auo) {
		wait_time = 240u;
	}
	return wait_time;
}

static int tcc_dpv14_backlight(struct udevice *dev, bool blen)
{
	struct tcc_dpv14_priv *priv = dev_get_priv(dev);
	int ret = 0;

	if (priv->panel_auo) {
		if (blen) {
			ret = pinctrl_select_state(dev, "blk_on");
			if (ret < 0) {
				debug_pr(LOGL_ERR, "[%s:%d]Err: failed to select state\n", __func__, __LINE__);
			}
		} else {
			ret = pinctrl_select_state(dev, "blk_off");
			if (ret < 0) {
				debug_pr(LOGL_ERR, "[%s:%d]Err: failed to select state\n", __func__, __LINE__);
			}
		}
	}
	return ret;
}

static const struct dm_display_ops dpv14_tcc_ops = {
	.read_timing = tcc_dpv14_read_timing,
	.enable = tcc_dpv14_enable,
	.backlight_wait_time_fn = tcc_dpv14_backlight_wait_time,
	.backlight_fn = tcc_dpv14_backlight,
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
	.priv_auto = (int)sizeof(struct tcc_dpv14_priv),
};
