// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <linux/delay.h>
#include <display.h>
#include <dm.h>
#include <telechips/fb_dm.h>
#include <telechips/fb_bootstage.h>
#include <telechips/dpv14_ctrl.h>
#include <telechips/dpv14_max968xx.h>
#include <telechips/fb_dm_lcd_interface.h>
#include "dptx_api.h"
#include <dm/uclass-internal.h>
#include <dm/pinctrl.h>
#include <dt-bindings/display/telechips-dispdef.h>
#include <mach/chipinfo.h>

#define GET_MUXID_FROM_FB

#if defined(CONFIG_TCC805X)
/*
 * For D5 (TCC805x)
 *
 *        +-----------------+
 *        |   pre-emphasis  |
 *        |      0  1  2  3 |
 *        + ----------------+
 * vswing | 0 |  8 10 12 16 |
 *        | 1 | 12 15 18    |
 *        | 2 | 16 20       |
 *        | 3 | 24          |
 *        + ----------------+
 */
static const uint32_t default_d3_main_eq[4][4] = {
	{
		/* vswing 0 */
		0x08, 0x0A, 0x0C, 0x10,
	},
	{
		/* vswing 1 */
		0x0C, 0x0F, 0x12, 0xFF,
	},
	{
		/* vswing 2 */
		0x10, 0x14, 0xFF, 0xFF,
	},
	{
		/* vswing 3 */
		0x18, 0xFF, 0xFF, 0xFF,
	},
};

/*
 * For D5 (TCC807x)
 *
 *        +-----------------+
 *        |   pre-emphasis  |
 *        |      0  1  2  3 |
 *        + ----------------+
 * vswing | 0 |  0  2  4  8 |
 *        | 1 |  0  3  6    |
 *        | 2 |  0  4       |
 *        | 3 |  0          |
 *        + ----------------+
 */
static const uint32_t default_d3_post_eq[4][4] = {
	{
		/* vswing 0 */
		0x00, 0x02, 0x04, 0x08,
	},
	{
		/* vswing 1 */
		0x00, 0x03, 0x06, 0xFF,
	},
	{
		/* vswing 2 */
		0x00, 0x04, 0xFF, 0xFF,
	},
	{
		/* vswing 3 */
		0x00, 0xFF, 0xFF, 0xFF,
	},
};
#endif

#if defined(CONFIG_TCC807X)
/*
 * For D5 (TCC807x)
 *
 *        +-----------------+
 *        |   pre-emphasis  |
 *        |      0  1  2  3 |
 *        + ----------------+
 * vswing | 0 |  1  6 13 13 |
 *        | 1 |  3  6 11    |
 *        | 2 |  5 10       |
 *        | 3 |  9          |
 *        + ----------------+
 */
static const uint32_t default_d5_main_eq[4][4] = {
	{
		/* vswing 0 */
		0x01, 0x06, 0x0D, 0x0D,
	},
	{
		/* vswing 1 */
		0x03, 0x06, 0x0B, 0xFF,
	},
	{
		/* vswing 2 */
		0x05, 0x0A, 0xFF, 0xFF,
	},
	{
		/* vswing 3 */
		0x09, 0xFF, 0xFF, 0xFF,
	},
};


/*
 * For D5 (TCC807x)
 *
 *        +-----------------+
 *        |   pre-emphasis  |
 *        |      0  1  2  3 |
 *        + ----------------+
 * vswing | 0 |  0  6  9 12 |
 *        | 1 |  0  6  9    |
 *        | 2 |  0  6       |
 *        | 3 |  0          |
 *        + ----------------+
 */
static const uint32_t default_d5_post_eq[4][4] = {
	{
		/* vswing 0 */
		0x00, 0x06, 0x09, 0x0C,
	},
	{
		/* vswing 1 */
		0x00, 0x06, 0x09, 0xFF,
	},
	{
		/* vswing 2 */
		0x00, 0x06, 0xFF, 0xFF,
	},
	{
		/* vswing 3 */
		0x00, 0xFF, 0xFF, 0xFF,
	},
};
#endif

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
	uint32_t uivic[PANEL_DP_MAX][VIC_CFG_NUM];
	uint32_t uilcdc_mux_select[PANEL_DP_MAX];

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
	struct tcc_dpv14_priv *priv = (struct tcc_dpv14_priv *)dev_get_priv(dev);
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

	if (ret == 0) {
		sub_ofnode = ofnode_first_subnode(port_ofnode);
		ret = ofnode_read_u32(sub_ofnode, "remote-endpoint", &remote_phandle);
		if (ret != 0) {
			debug_pr(LOGL_ERR, "\n[%s:%d]Err: can't find remote-endpoint\n",
				 __func__, __LINE__);
		}
	}

	if (ret == 0) {
		remote_ofnode = ofnode_get_by_phandle(remote_phandle);
		if (!ofnode_valid(remote_ofnode)) {
			debug_pr(LOGL_ERR, "\n[%s:%d]Err: remote ofnode isn't valid\n",
				 __func__, __LINE__);

			ret = -ENODEV;
		}
	}

	if (ret == 0) {
		while ((ofnode_valid(remote_ofnode)) && (ret == 0)) {
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

	if (p_dev != NULL) {
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
static int32_t tcc_dpv14_get_mux_info(const struct udevice *dev)
{
	const struct udevice *pre_udev;
	struct tcc_dpv14_priv *priv = (struct tcc_dpv14_priv *)dev_get_priv(dev);
	int32_t ret = 0;
	uint8_t ucdpIdx;

	for (ucdpIdx = 0; ucdpIdx < priv->ucnum_of_dps; ucdpIdx++) {
		pre_udev = priv->pre_pudev[ucdpIdx];
		if (pre_udev == NULL) {
			debug_pr(LOGL_ERR, "[%s:%d]Err: parent node isn't valid for Dp%d\n", __func__, __LINE__, ucdpIdx);
			break;
		}

		ret = ofnode_read_u32(dev_ofnode(pre_udev), "lcd-mux-select", &priv->uilcdc_mux_select[ucdpIdx]);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[%s:%d]Warn: couldn't read lcd-mux-select for Dp%d\n", __func__, __LINE__, ucdpIdx);
		}

		if (priv->uilcdc_mux_select[ucdpIdx] == SDM_MUX_SELECT) {
			ret = ofnode_read_u32(dev_ofnode(pre_udev), "lcd-mux-bypass", &priv->uisdm_bypass);
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_WARNING,
					"[%s:%d]Warn: couldn't read sdm bypass, set to bypass by default\n",
					__func__, __LINE__);
				priv->uisdm_bypass = 1U;
			}
		}

		if (priv->uilcdc_mux_select[ucdpIdx] == TRVC_MUX_SELECT) {
			ret = ofnode_read_u32(dev_ofnode(pre_udev), "lcd-mux-bypass", &priv->uitrvc_bypass);
			if (ret < 0) {
				/* For KCS */
				debug_pr(LOGL_WARNING,
					"[%s:%d]Warn: couldn't read trvc bypass, set to bypass by default\n",
					__func__, __LINE__);
				priv->uitrvc_bypass = 1U;
			}
		}
	}

	return ret;
}
#endif

static bool tcc_dpv14_get_dp_id(uint32_t *dp_id, const char *dev_name)
{
	bool ret = false;
	char temp_name[64];
	char *endptr = NULL, *rest = temp_name;
	const char *last_sep = NULL, *sep = NULL;

	(void)strncpy(temp_name, dev_name, sizeof(temp_name));
	temp_name[sizeof(temp_name) - 1U] = '\0';


	sep = strsep(&rest, "_");
	while (sep != NULL) {
		last_sep = sep;
		sep = (const char *)strsep(&rest, "_");
	}

	if (last_sep != NULL) {
		unsigned long val = strtoul(last_sep, &endptr, 10);
		if ((*endptr == '\0') && (endptr != last_sep) &&
			(val < (unsigned long)PANEL_DP_MAX)) {
			*dp_id = (unsigned int)val;
			ret = true;
		} else {
			*dp_id = UINT_MAX;
		}
	} else {
		*dp_id = UINT_MAX;
	}

	return ret;
}

static int32_t tcc_dpv14_get_num_of_dps(const struct udevice *dev)
{
	const char *pcremote_dev_name;
	const char *string_ptr;
	int32_t ret = 0;
	uint32_t remote_phandle;
	struct udevice *p_udev;
	struct udevice *p_dev = NULL;
	ofnode ports_ofnode, port_ofnode, sub_ofnode, remote_ofnode;
	uint32_t dp_id;
	struct tcc_dpv14_priv *priv = (struct tcc_dpv14_priv *)dev_get_priv(dev);

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

		if (ofnode_read_u32(sub_ofnode, "remote-endpoint", &remote_phandle) != 0) {
			debug_pr(LOGL_DEBUG, "[%s:%d] can't find remote-endpoint\n", __func__, __LINE__);
			continue;
		}

		remote_ofnode = ofnode_get_by_phandle(remote_phandle);
		if (!ofnode_valid(remote_ofnode)) {
			debug_pr(LOGL_DEBUG, "[%s:%d]remote ofnode isn't available\n",
				 __func__, __LINE__);

			continue;
		}

		while ((ofnode_valid(remote_ofnode)) && (ret == 0)) {
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
		if (tcc_dpv14_get_dp_id(&dp_id, dev_read_name(p_dev))) {
			priv->pre_pudev[dp_id] = p_udev;
		} else {
			continue;
		}
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

static int32_t tcc_dpv14_parse_dt(const struct udevice *dev)
{
	uint8_t ucdp_idx;
	int32_t ret = 0;
	uint32_t uidp_id;
	uint32_t dp_bandwidth[DP_BANDWIDTH_CFG_NUM] = {0,};
	struct tcc_dpv14_priv *priv = (struct tcc_dpv14_priv *)dev_get_priv(dev);
	struct ofnode_phandle_args args;
	ofnode settings_node;

	(void)memset(priv, 0, sizeof(struct tcc_dpv14_priv));

	if (!tcc_dpv14_get_dp_id(&uidp_id, dev_read_name(dev))) {
		debug_pr(LOGL_ERR, "\n[%s:%d]Err: Invalid DP id as %u\n",
			 __func__, __LINE__, uidp_id);
		ret = -EINVAL;
	} else if (dpv14_privs[uidp_id] != NULL) {
		debug_pr(LOGL_ERR, "\n[%s:%d]Err: The DP ID(%u) is already in use\n",
			 __func__, __LINE__, uidp_id);
		ret = -EINVAL;
	} else {
		dpv14_privs[uidp_id] = priv;
		priv->uidp_id = uidp_id;
	}

	if (ret == 0) {
		ret = tcc_dpv14_get_num_of_dps(dev);
#if defined(GET_MUXID_FROM_FB)
		(void)tcc_dpv14_get_mux_info(dev);
#endif
	}

	if (ret == 0) {
		if (dev_read_phandle_with_args(dev, "settings", NULL, 0, 0, &args) != 0) {
			debug_pr(LOGL_ERR, "Failed to find common settings node\n");
		}

		settings_node = args.node;

		if (!ofnode_valid(settings_node)) {
			debug_pr(LOGL_ERR, "[%s:%d]settings node isn't vailed\n",
				 __func__, __LINE__);
		}

		ret = ofnode_read_u32(settings_node, "dp-panel-mode",
				&priv->uipanel_mode);
		if (ret < 0) {
			/* For KCS */
			debug_pr(LOGL_WARNING,
				"[%s:%d]Warn: couldn't read panel mode, set to panel mode by default\n",
				__func__, __LINE__);
			priv->uipanel_mode = 1U;
		}

		ret = ofnode_read_u32(settings_node, "dp-serdes-i2c-port", &priv->uii2c_port);
		if (ret < 0) {
			/* For KCS */
			debug_pr(LOGL_WARNING,
				"[%s:%d]Warn: couldn't read i2c port, set to D3(0), D5(4) by default\n",
				__func__, __LINE__);
			priv->uii2c_port = (uint32_t)I2C_PORT_TO_SERDES;
		}

		ret = ofnode_read_u32_array(settings_node, "dp-vic", (uint32_t *)priv->uivic,
		       (size_t)(PANEL_DP_MAX * VIC_CFG_NUM));
		if (ret < 0) {
			/* For KCS */
			debug_pr(LOGL_WARNING,
				"[%s:%d]Warn: couldn't read dp-vic, set to 1(480p) by default\n",
				__func__, __LINE__);
			priv->uivic[0][0] = 0U;
			priv->uivic[1][0] = 0U;
			priv->uivic[2][0] = 0U;
			priv->uivic[3][0] = 0U;
			priv->uivic[0][1] = 1U;
			priv->uivic[1][1] = 1U;
			priv->uivic[2][1] = 1U;
			priv->uivic[3][1] = 1U;
		}

		ret = ofnode_read_u32_array(settings_node, "dp-vcp_id",
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

		ret = ofnode_read_u32(settings_node, "dp-phy-lane-swap", &priv->uiphy_lane_swap);
		if (ret < 0) {
			/* For KCS */
			debug_pr(LOGL_WARNING,
				"[%s:%d]Warn: couldn't read dp-phy-lane-swap, set to 1 by default\n",
				__func__, __LINE__);
			priv->uiphy_lane_swap = 1U;
		}

		ret = ofnode_read_u32(settings_node, "dp-pixel-encoding", &priv->uipixel_encoding);
		if (ret < 0) {
			/* For KCS */
			debug_pr(LOGL_WARNING,
				"[%s:%d]Warn: couldn't read dp-pixel-encoding, set to RGB by default\n",
				__func__, __LINE__);
			priv->uipixel_encoding = 0U;
		}
		ret = ofnode_read_u32_array(settings_node, "dp-max-bandwidth", dp_bandwidth,
		       (size_t)DP_BANDWIDTH_CFG_NUM);
		if (ret < 0) {
			debug_pr(LOGL_WARNING,
				"[%s:%d]Warn: couldn't read dp-max-bandwidth, set to HBR3 and 4 lanes by default\n",
				__func__, __LINE__);
			priv->uimax_lane = 4U;
			priv->uimax_rate = 3U;
		} else {
			priv->uimax_lane = dp_bandwidth[DP_BANDWIDTH_CFG_LANE];
			priv->uimax_rate = dp_bandwidth[DP_BANDWIDTH_CFG_RATE];
		}

		ret = ofnode_read_u32(settings_node, "evb_power_type", &priv->power_type);
		if ((ret < 0) || (priv->power_type >= TCC_EVB_LCD_POW_MAX)) {
			/* For KCS */
			debug_pr(LOGL_WARNING, "[%s:%d]Warn: couldn't read evb power type, set to evb power type by default\n", __func__, __LINE__);
			priv->power_type = TCC_EVB_LCD_FOUR_POW;
		}

		ret = ofnode_read_u32(settings_node, "phy_model", &priv->phy_model);
		if ((ret < 0) || (priv->phy_model >= DPTX_PHY_DEVICE_UNKNOWN)) {
			/* For KCS */
			debug_pr(LOGL_WARNING, "[%s:%d]Warn: couldn't read phy model, set to phy model by default\n", __func__, __LINE__);
			priv->phy_model = DPTX_PHY_DEVICE_SNPS;
		}

		for (ucdp_idx = 0; ucdp_idx < priv->ucnum_of_dps; ucdp_idx++) {
			if (priv->uilcdc_mux_select[ucdp_idx] == (uint32_t)LCD_MUX2) {
				break;
			}
		}
		if (ucdp_idx == priv->ucnum_of_dps) {
			priv->uisdm_bypass = 0U;
		}

		for (ucdp_idx = 0; ucdp_idx < priv->ucnum_of_dps; ucdp_idx++) {
			if (priv->uilcdc_mux_select[ucdp_idx] == (uint32_t)LCD_MUX3) {
				break;
			}
		}
		if (ucdp_idx == priv->ucnum_of_dps) {
			priv->uitrvc_bypass = 0U;
		}

		if ((priv->uipanel_mode != 0U) && (priv->uivic[uidp_id][VIC_CFG_VIDEO_CODE] == 1028U)) {
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
	const struct tcc_dpv14_priv *priv = (struct tcc_dpv14_priv *)dev_get_priv(dev);

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

static bool tcc_dpv14_check_privs(const struct tcc_dpv14_priv *priv)
{
	uint8_t dp_idx = 0;
	bool ret = true;

	for (dp_idx = 0; dp_idx < priv->ucnum_of_dps; dp_idx++) {
		if (dpv14_privs[dp_idx] == NULL) {
			ret = false;

			break;
		}
	}

	return ret;
}

static int tcc_dpv14_enable(struct udevice *dev, int panel_bpp,
			    const struct display_timing *edid)
{
	static uint8_t dpv14_enable_cnt;
	uint8_t ucdp_idx;
	int ret = 0;
	uint32_t uip_clk;
	uint32_t auiperi_pclk[PANEL_DP_MAX] = { 0, };
	const struct tcc_dpv14_priv *priv_list;
	struct tcc_dpv14_priv *priv = (struct tcc_dpv14_priv *)dev_get_priv(dev);

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

	(void)lcdc_mux_select(priv->uilcdc_mux_select[priv->uidp_id], priv->uiDc_idx);

	(void)lcdc_display_device_init(priv->parent_dev);

	debug_pr(LOGL_INFO, "\n[%s:%d]Enable dptx with VIC(%u) :\n", __func__, __LINE__,
	  priv->uivic[priv->uidp_id][VIC_CFG_VIDEO_CODE]);
	debug_pr(LOGL_INFO, " 	panel bpp = %u\n", panel_bpp);
	debug_pr(LOGL_INFO, " 	Display controller %u -> Mux %u\n", priv->uiDc_idx, priv->uilcdc_mux_select[priv->uidp_id]);
	debug_pr(LOGL_INFO, " 	Pixel clk = %u\n", edid->pixelclock.typ);
	debug_pr(LOGL_INFO, " 	flags = 0x%x \n", edid->flags);
	debug_pr(LOGL_INFO, " 	H active(%u) x V active(%u)\n", edid->hactive.typ, edid->vactive.typ);
	debug_pr(LOGL_INFO, " 	H front porch(%u), V front porch(%u)\n", edid->hfront_porch.typ, edid->vfront_porch.typ);
	debug_pr(LOGL_INFO, " 	H back porch(%u), V back porch(%u)\n", edid->hback_porch.typ, edid->hback_porch.typ);
	debug_pr(LOGL_INFO, " 	H sync len(%u), V sync len(%u)\n", edid->hsync_len.typ, edid->hsync_len.typ);

	dpv14_enable_cnt = (uint8_t)(((uint16_t)dpv14_enable_cnt + 1u) % 0xFFu);

	if ((dpv14_enable_cnt <= (uint8_t)PANEL_DP_MAX) &&
		(dpv14_enable_cnt == priv->ucnum_of_dps) &&
		tcc_dpv14_check_privs(priv)) {
		for (ucdp_idx = 0; ucdp_idx < priv->ucnum_of_dps; ucdp_idx++) {
			priv_list = dpv14_privs[ucdp_idx];

			auiperi_pclk[ucdp_idx] = priv_list->uipixel_clk;
		}
		BOOTSTASGE_NAME("dp: enable");
		ret = dpv14_enable(auiperi_pclk);
		BOOTSTASGE_NAME("dp: enable:done");
		for (ucdp_idx = 0; ucdp_idx < priv->ucnum_of_dps; ucdp_idx++) {
			priv_list = dpv14_privs[ucdp_idx];

			/* The AUO panel must be reset before signal input. */
			if (priv_list->panel_auo) {
				const struct tcc_fb_dm_priv *fb_priv =
					(struct tcc_fb_dm_priv *)dev_get_priv(priv_list->parent_dev);

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

static bool tcc_dpv14_get_dtd_type(uint8_t dp_id,
				   const uint32_t vic[PANEL_DP_MAX][VIC_CFG_NUM],
				   enum DPV14_DTD_TYPE *dtd_type)
{
	bool ret = true;

	if (vic[dp_id][VIC_CFG_VIDEO_CODE] == (uint32_t)VIC_TO_READ_EDID) {
		*dtd_type = DTD_TYPE_READ_EDID;
	} else {
		switch (vic[dp_id][VIC_CFG_VIDEO_FORMAT]) {
		case VIDEO_FORMAT_CEA_861:
			*dtd_type = DTD_TYPE_CEA861_VIC;
			break;
		case VIDEO_FORMAT_VESA_CVT:
			*dtd_type = DTD_TYPE_VESA_CVT_VIC;
			break;
		case VIDEO_FORMAT_VESA_DMT:
			*dtd_type = DTD_TYPE_VESA_DMT_VIC;
			break;
		default:
			debug_pr(LOGL_ERR, "[%s:%d]Err: Invaild Video format %u\n",
				__func__, __LINE__, vic[dp_id][VIC_CFG_VIDEO_FORMAT]);
			ret = false;
			break;
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
	const struct tcc_dpv14_priv *priv = (struct tcc_dpv14_priv *)dev_get_priv(dev);
	struct tcc_fb_dm_priv *p_priv  = (struct tcc_fb_dm_priv *)dev_get_priv(priv->parent_dev);

	ucdp_id = (uint8_t)(priv->uidp_id & 0xFFU);

	if ((priv->uipanel_mode != PANEL_MODE_OFF) && (priv->uivic[ucdp_id][VIC_CFG_VIDEO_CODE] == VIC_TO_READ_EDID)) {
		debug_pr(LOGL_ERR, "[%s:%d]Err: SerDes doesn't support EDID\n", __func__, __LINE__);

		ret = -EINVAL;
	} else if (!tcc_dpv14_get_dtd_type(ucdp_id, priv->uivic, &edtd_type)) {
		ret = -EINVAL;
	} else {
		(void)memset(&stdisplay_timing, 0, sizeof(stdisplay_timing));
		ret = dpv14_get_dtd(ucdp_id, priv->uivic[ucdp_id][VIC_CFG_VIDEO_CODE], edtd_type, &stdisplay_timing);

		out_timing->pixelclock.typ = stdisplay_timing.uipixelclock;

		out_timing->hactive.typ = stdisplay_timing.uihactive;
		out_timing->hfront_porch.typ = stdisplay_timing.uihfront_porch;
		out_timing->hback_porch.typ = stdisplay_timing.uihback_porch;
		out_timing->hsync_len.typ = stdisplay_timing.uihsync_len;

		out_timing->vactive.typ = stdisplay_timing.uivactive;
		out_timing->vfront_porch.typ = stdisplay_timing.uivfront_porch;
		out_timing->vback_porch.typ = stdisplay_timing.uivback_porch;
		out_timing->vsync_len.typ = stdisplay_timing.uivsync_len;

		if (stdisplay_timing.flags <= (unsigned)DISPLAY_FLAGS_DOUBLECLK) {
			out_timing->flags = (enum display_flags)stdisplay_timing.flags;
		}

		p_priv->interlaced = stdisplay_timing.uiinterlaced;
		p_priv->pixel_repetition_input = stdisplay_timing.uipixel_repetition;

		debug_pr(LOGL_DEBUG, "\n[%s:%d]VIC(%d) : \n", __func__, __LINE__, priv->uivic[ucdp_id][VIC_CFG_VIDEO_CODE]);
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
	const struct tcc_dpv14_priv *priv = (struct tcc_dpv14_priv *)dev_get_priv(dev);

	(void)tcc_dpv14_parse_dt(dev);

	(void)tcc_dpv14_get_dd_id(dev);

	debug_pr(LOGL_DEBUG, "\n[%s:%d]DP %u of %u DPs...\n",
		 __func__, __LINE__, (uint32_t)priv->uidp_id,
		 (uint32_t)priv->ucnum_of_dps);
	debug_pr(LOGL_DEBUG, "                 Display Controller %u -> Mux %u\n",
		 priv->uiDc_idx, priv->uilcdc_mux_select[priv->uidp_id]);
	debug_pr(LOGL_DEBUG, "                 DP %s\n",
		 (priv->uipanel_mode != 0U) ? "Panel mode" : "Monitor mode");
	debug_pr(LOGL_DEBUG, "                 I2C Port as %u\n",
		 priv->uii2c_port);
	debug_pr(LOGL_DEBUG, "                 SDM Bypass %s, SRVC Bypass %s\n",
		 (priv->uisdm_bypass != 0U) ? "On":"Off",
		 (priv->uitrvc_bypass != 0U) ? "On" : "Off");
	debug_pr(LOGL_DEBUG, "                 VIC : %u %u %u %u\n",
		 priv->uivic[0][VIC_CFG_VIDEO_CODE], priv->uivic[1][VIC_CFG_VIDEO_CODE],
		 priv->uivic[2][VIC_CFG_VIDEO_CODE], priv->uivic[3][VIC_CFG_VIDEO_CODE]);
	debug_pr(LOGL_DEBUG, "                 VCP id : %u %u %u %u\n",
		 priv->uivcp_id[0], priv->uivcp_id[1],
		 priv->uivcp_id[2], priv->uivcp_id[3]);

	return ret;
}

static void tcc_dpv14_set_drv_params(const struct tcc_dpv14_priv *priv,
				     struct dpv14_drv_params *drv_params)
{
	uint32_t uitcc80xx_rev;
	uint8_t dp_idx;

	drv_params->num_of_dps = (uint8_t)priv->ucnum_of_dps;
	drv_params->dp_id = (uint8_t)(priv->uidp_id & 0xFFU);
	drv_params->panel_mode = (bool)priv->uipanel_mode;
	drv_params->phy_lane_swap = (priv->uiphy_lane_swap == 0x1U) ? true : false;
	drv_params->sdm_bypass = (bool)priv->uisdm_bypass;
	drv_params->trvc_bypass = (bool)priv->uitrvc_bypass;
	drv_params->i2c_port = (uint8_t)(priv->uii2c_port & 0xFFu);
	drv_params->pixel_encoding = (uint8_t)(priv->uipixel_encoding & 0xFFU);
	drv_params->max_lane = (uint8_t)(priv->uimax_lane & 0xFFu);
	drv_params->max_rate = (uint8_t)(priv->uimax_rate & 0xFFu);
	drv_params->evb_power_type = (uint8_t)(priv->power_type & 0xFFu);
	drv_params->phy_model = (uint8_t)(priv->phy_model & 0xFFu);

	uitcc80xx_rev = get_chip_rev();
	drv_params->chip_rev = (uint8_t)(uitcc80xx_rev & 0xFFU);

	for (dp_idx = 0; dp_idx < (uint8_t)PANEL_DP_MAX; dp_idx++) {
		drv_params->dd_mux_select[dp_idx] = priv->uilcdc_mux_select[dp_idx];
		drv_params->vic[dp_idx][VIC_CFG_VIDEO_CODE] = priv->uivic[dp_idx][VIC_CFG_VIDEO_CODE];
		drv_params->vic[dp_idx][VIC_CFG_VIDEO_FORMAT] = priv->uivic[dp_idx][VIC_CFG_VIDEO_FORMAT];
		drv_params->vcp_id[dp_idx] = (uint8_t)(priv->uivcp_id[dp_idx] & 0xFFU);
	}

	if (!drv_params->phy_lane_swap) {
		drv_params->lane_order[0] = (uint8_t)LINK_LANE_2;
		drv_params->lane_order[1] = (uint8_t)LINK_LANE_3;
		drv_params->lane_order[2] = (uint8_t)LINK_LANE_0;
		drv_params->lane_order[3] = (uint8_t)LINK_LANE_1;
	} else {
		drv_params->lane_order[0] = (uint8_t)LINK_LANE_0;
		drv_params->lane_order[1] = (uint8_t)LINK_LANE_1;
		drv_params->lane_order[2] = (uint8_t)LINK_LANE_2;
		drv_params->lane_order[3] = (uint8_t)LINK_LANE_3;
	}

	if (drv_params->phy_model == DPTX_PHY_DEVICE_SNPS) {
		if (drv_params->chip_rev == (uint8_t)TCC80XX_REV_ES) {
			drv_params->max_rate =
				(drv_params->max_rate > (uint8_t)DPTX_LINK_RATE_HBR2) ?
				(uint8_t)DPTX_LINK_RATE_HBR2 :
				(uint8_t)drv_params->max_rate;

			for (dp_idx = 0; dp_idx < (unsigned)DPTX_INPUT_STREAM_MAX; dp_idx++) {
				drv_params->dd_mux_select[dp_idx] = dp_idx;
			}
		}
	}
}

static void tcc_dpv14_set_default_phy_eq(struct dpv14_drv_params *drv_params)
{
	#if defined(CONFIG_TCC805X)
	(void)memcpy(drv_params->hw_config.phy_eq[0].main_eq, default_d3_main_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[0].post_eq, default_d3_post_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[1].main_eq, default_d3_main_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[1].post_eq, default_d3_post_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[2].main_eq, default_d3_main_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[2].post_eq, default_d3_post_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[3].main_eq, default_d3_main_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[3].post_eq, default_d3_post_eq, sizeof(uint32_t) * 16u);
	#endif
	#if defined(CONFIG_TCC807X)
	(void)memcpy(drv_params->hw_config.phy_eq[0].main_eq, default_d5_main_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[0].post_eq, default_d5_post_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[1].main_eq, default_d5_main_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[1].post_eq, default_d5_post_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[2].main_eq, default_d5_main_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[2].post_eq, default_d5_post_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[3].main_eq, default_d5_main_eq, sizeof(uint32_t) * 16u);
	(void)memcpy(drv_params->hw_config.phy_eq[3].post_eq, default_d5_post_eq, sizeof(uint32_t) * 16u);
	#endif
}

#if defined(DPTX_DUMP_PHY_EQ)
static void tcc_dpv14_dump_hw_config(struct dptx_hw_config *hw_config)
{
	uint32_t vsw_idx, pre_idx;

	pr_force("RBR Main\n");
	for (vsw_idx = 0u; vsw_idx < 4u; vsw_idx++) {
		for (pre_idx = 0u; pre_idx < 4u; pre_idx++) {
			pr_force("0x%02x ", hw_config->phy_eq[0].main_eq[vsw_idx][pre_idx]);
		}
		pr_force("\n");
	}
	pr_force("RBR Post\n");
	for (vsw_idx = 0u; vsw_idx < 4u; vsw_idx++) {
		for (pre_idx = 0u; pre_idx < 4u; pre_idx++) {
			pr_force("0x%02x ", hw_config->phy_eq[0].post_eq[vsw_idx][pre_idx]);
		}
		pr_force("\n");
	}
	pr_force("HBR Main\n");
	for (vsw_idx = 0u; vsw_idx < 4u; vsw_idx++) {
		for (pre_idx = 0u; pre_idx < 4u; pre_idx++) {
			pr_force("0x%02x ", hw_config->phy_eq[1].main_eq[vsw_idx][pre_idx]);
		}
		pr_force("\n");
	}
	pr_force("HBR Post\n");
	for (vsw_idx = 0u; vsw_idx < 4u; vsw_idx++) {
		for (pre_idx = 0u; pre_idx < 4u; pre_idx++) {
			pr_force("0x%02x ", hw_config->phy_eq[1].post_eq[vsw_idx][pre_idx]);
		}
		pr_force("\n");
	}
	pr_force("HBR2 Main\n");
	for (vsw_idx = 0u; vsw_idx < 4u; vsw_idx++) {
		for (pre_idx = 0u; pre_idx < 4u; pre_idx++) {
			pr_force("0x%02x ", hw_config->phy_eq[2].main_eq[vsw_idx][pre_idx]);
		}
		pr_force("\n");
	}
	pr_force("HBR2 Post\n");
	for (vsw_idx = 0u; vsw_idx < 4u; vsw_idx++) {
		for (pre_idx = 0u; pre_idx < 4u; pre_idx++) {
			pr_force("0x%02x ", hw_config->phy_eq[2].post_eq[vsw_idx][pre_idx]);
		}
		pr_force("\n");
	}
	pr_force("HBR3 Main\n");
	for (vsw_idx = 0u; vsw_idx < 4u; vsw_idx++) {
		for (pre_idx = 0u; pre_idx < 4u; pre_idx++) {
			pr_force("0x%02x ", hw_config->phy_eq[3].main_eq[vsw_idx][pre_idx]);
		}
		pr_force("\n");
	}
	pr_force("HBR3 Post\n");
	for (vsw_idx = 0u; vsw_idx < 4u; vsw_idx++) {
		for (pre_idx = 0u; pre_idx < 4u; pre_idx++) {
			pr_force("0x%02x ", hw_config->phy_eq[3].post_eq[vsw_idx][pre_idx]);
		}
		pr_force("\n");
	}
}
#endif

static void tcc_dpv14_parse_hw_config(const struct udevice *dev, struct dpv14_drv_params *drv_params)
{
	uint32_t aux_hysteresis;
	ofnode hw_config_node, eq_node, eq_sub_node;
	int32_t ret = 0;

	/* default value */
	drv_params->hw_config.aux_hysteresis = 1u;
	drv_params->hw_config.support_spread_specturm_clock = 1u;

	hw_config_node = dev_read_subnode(dev, "hw-config");
	if (ofnode_valid(hw_config_node)) {
		ret = ofnode_read_u32(hw_config_node, "aux-hysteresis", &aux_hysteresis);
		if (ret == 0) {
			if (aux_hysteresis <= 3u) {
				drv_params->hw_config.aux_hysteresis = aux_hysteresis;
			}
		}
		eq_node = ofnode_find_subnode(hw_config_node, "eq");
		if (!ofnode_valid(eq_node)) {
			ret = -ENODEV;
		}
		if (ret == 0) {
			if (!ofnode_is_enabled(eq_node)) {
				ret = -ENODEV;
			}
		}
		if (ret == 0) {
			eq_sub_node = ofnode_find_subnode(eq_node, "rbr");
			if (!ofnode_valid(eq_sub_node)) {
				ret = -ENODEV;
			}
		}
		if (ret == 0) {
			ret = ofnode_read_u32_array(eq_sub_node, "main",
						    (uint32_t *)drv_params->hw_config.phy_eq[0].main_eq, 16u);
		}
		if (ret == 0) {
			ret = ofnode_read_u32_array(eq_sub_node, "post",
						    (uint32_t *)drv_params->hw_config.phy_eq[0].post_eq, 16u);
		}
		if (ret == 0) {
			eq_sub_node = ofnode_find_subnode(eq_node, "hbr");
			if (!ofnode_valid(eq_sub_node)) {
				ret = -ENODEV;
			}
		}
		if (ret == 0) {
			ret = ofnode_read_u32_array(eq_sub_node, "main",
						    (uint32_t *)drv_params->hw_config.phy_eq[1].main_eq, 16u);
		}
		if (ret == 0) {
			ret = ofnode_read_u32_array(eq_sub_node, "post",
						    (uint32_t *)drv_params->hw_config.phy_eq[1].post_eq, 16u);
		}
		if (ret == 0) {
			eq_sub_node = ofnode_find_subnode(eq_node, "hbr2");
			if (!ofnode_valid(eq_sub_node)) {
				ret = -ENODEV;
			}
		}
		if (ret == 0) {
			ret = ofnode_read_u32_array(eq_sub_node, "main",
						    (uint32_t *)drv_params->hw_config.phy_eq[2].main_eq, 16u);
		}
		if (ret == 0) {
			ret = ofnode_read_u32_array(eq_sub_node, "post",
						    (uint32_t *)drv_params->hw_config.phy_eq[2].post_eq, 16u);
		}
		if (ret == 0) {
			eq_sub_node = ofnode_find_subnode(eq_node, "hbr3");
			if (!ofnode_valid(eq_sub_node)) {
				ret = -ENODEV;
			}
		}
		if (ret == 0) {
			ret = ofnode_read_u32_array(eq_sub_node, "main",
						    (uint32_t *)drv_params->hw_config.phy_eq[3].main_eq, 16u);
		}
		if (ret == 0) {
			ret = ofnode_read_u32_array(eq_sub_node, "post",
						    (uint32_t *)drv_params->hw_config.phy_eq[3].post_eq, 16u);
		}
		if (ret == 0) {
			drv_params->hw_config.phy_eq_manual_mode = true;
		}
		if (!drv_params->hw_config.phy_eq_manual_mode) {
			tcc_dpv14_set_default_phy_eq(drv_params);
			#if defined(CONFIG_TCC807X)
			drv_params->hw_config.phy_eq_manual_mode = true;
			#endif
		}
		#if defined(DPTX_DUMP_PHY_EQ)
		tcc_dpv14_dump_hw_config(&drv_params->hw_config);
		#endif
	}
	if (!drv_params->panel_mode) {
		drv_params->hw_config.support_sideband_msg = 1u;
	}
}

static int tcc_dpv14_probe(struct udevice *dev)
{
	int ret = 0;
	struct dpv14_drv_params drv_params = {0, };
	const struct tcc_dpv14_priv *priv = (struct tcc_dpv14_priv *)dev_get_priv(dev);

	(void)tcc_dpv14_init_pinctrl(dev);

	if (priv->uidp_id == (uint32_t)PANEL_DP0) {
		BOOTSTASGE_NAME("dp: init");
		tcc_dpv14_set_drv_params(priv, &drv_params);

		tcc_dpv14_parse_hw_config(dev, &drv_params);

		ret = dpv14_Init(&drv_params);

		dpv14_debug_pr_drv_params(LOGL_DEBUG, &drv_params);
		BOOTSTASGE_NAME("dp: init:done");
	}
	return ret;
}

static uint32_t tcc_dpv14_backlight_wait_time(struct udevice *dev)
{
	const struct tcc_dpv14_priv *priv = (const struct tcc_dpv14_priv *)dev_get_priv(dev);
	uint32_t wait_time = 0u;

	if (priv->panel_auo) {
		wait_time = 240u;
	}
	return wait_time;
}

static int tcc_dpv14_backlight(struct udevice *dev, bool blen)
{
	const struct tcc_dpv14_priv *priv = (const struct tcc_dpv14_priv *)dev_get_priv(dev);
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
