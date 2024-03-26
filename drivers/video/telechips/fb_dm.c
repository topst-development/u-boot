// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <display.h>
#include <clk.h>
#include <mach/clock.h>
#include <fdt_support.h>
#include <video.h>
#include <mapmem.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <lcd.h>
#include <splash.h>
#include <bmp_layout.h>
#if defined(CONFIG_SPLASH_LOGO_SUPPORT)
#include <splashimg.h>
#endif
#include <linux/delay.h>
#include <telechips/fb_dm.h>
#include <telechips/fb_dm_lcd_interface.h>

#if defined(CONFIG_SPLASH_LOGO_SUPPORT)
#ifndef CONFIG_SPLASH_PARTITION_NAME
#error "Wrong configuration error: CONFIG_SPLASH_PARTITION_NAME has been set."
#else
#define SPLASH_PARTITION_NAME CONFIG_SPLASH_PARTITION_NAME
#endif
#endif

static int get_id_from_name(const char * str) {
	int ret = 0;

	if ( str == NULL) {
		ret = -1;
		/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_17_7_violation : FALSE] */
		(void)pr_err("[ERROR](%s) : NULL pointer\n",
					 __func__);
	} else {
		int int_val = (int)str[strlen(str) - 1U];
		if(int_val >= (int)'0') {
			ret = int_val - (int)'0';
		} else {
			ret = -1;
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR](%s) : invalid id = %d\n",
						 __func__, int_val);
		}
	}
	return ret;
}

/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
struct tcc_fb_dm_global_info tcc_dm_global = {
	.tcc_fb_dm_version.date = "20230824",
	.tcc_fb_dm_version.name = "tcc_fb_dm",
	.tcc_fb_dm_version.major = 1,
	.tcc_fb_dm_version.minor = 4,
	.tcc_fb_dm_version.patchlevel = 0,
};

static int tcc_fb_dm_bmp_display(struct udevice *dev, ulong addr);
static int tcc_fb_dm_init_image_buf(struct udevice *dev);
static int tcc_fb_dm_init_image_info_for_splash(struct tcc_fb_dm_image_info *image_info, unsigned int splash_width, unsigned int splash_height);
static int tcc_fb_dm_bind(struct udevice *dev);
static int tcc_fb_dm_ofdata_to_plat(struct udevice *dev);


struct tcc_logo {
	int width;
	int height;
	int bmp_bpix;
	const unsigned char *logo_data_base;
};

static const unsigned char logo_data_telechips[] = {
// #include LOGO_TELECHIPS_HEADER
#include LOGO_TOPST_AI_HEADER
};

static struct tcc_logo tcc_logo_info[FB_BUF_CNT_MAX] = {
	[0] = {
		.logo_data_base = logo_data_telechips,
		/* The values below are optional */
		.width = 640,
		.height = 480,
		.bmp_bpix = 24,
	},
	#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X) || defined(CONFIG_TCC807X)
	[1] = {
		.logo_data_base = logo_data_telechips,
		/* The values below are optional */
		.width = 640,
		.height = 480,
		.bmp_bpix = 24,
	},
	[2] = {
		.logo_data_base = logo_data_telechips,
		/* The values below are optional */
		.width = 640,
		.height = 480,
		.bmp_bpix = 24,
	},
	#if defined(CONFIG_TCC805X) || defined(CONFIG_TCC807X)
	[3] = {
		.logo_data_base = logo_data_telechips,
		/* The values below are optional */
		.width = 640,
		.height = 480,
		.bmp_bpix = 24,
	},
	#if defined(CONFIG_TCC807X)
	[4] = {
		.logo_data_base = logo_data_telechips,
		/* The values below are optional */
		.width = 640,
		.height = 480,
		.bmp_bpix = 24,
	},
	#endif
	#endif
	#endif
};

static int get_tcc_fb_dm_logo_info(unsigned int *addr, unsigned int idx)
{
	int ret = 0;


	if (addr == NULL) {
		pr_err("[%s] : NULL addr\n", __func__);
		ret = -EINVAL;
	} else {
		if (tcc_logo_info[idx].logo_data_base != 0) {
			*addr = (unsigned int)((unsigned long)tcc_logo_info[idx].logo_data_base & 0xFFFFFFFFU);
		} else {
			pr_err("[%s] : no image data found. Please check init_log_arr()\n",
			       __func__);
			ret = -EINVAL;
		}
	}
	return ret;
};

/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static int tcc_fb_dm_bind(struct udevice *dev)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	//if size == 0, u-boot will not allocate memory. It's not error situation.
	plat->size = 0;

	return 0;
}

/* coverity[HIS_metric_violation : FALSE] */
static int tcc_display_init(struct udevice *dev, ofnode ep_node)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	const struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	struct tcc_fb_dm_priv *priv = dev_get_priv(dev);
	struct udevice *output_media;
	int ret = 0;
	u32 remote_phandle;
	const struct display_plat *output_media_uc_plat;
	ofnode remote;
	ulong flag_clk_en = CKC_ENABLE;
	ulong lclk = 0;
	ulong peri_lcd_id;
	uint32_t splash_width = 0, splash_height = 0, splash_format = 0;

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	pr_info("[%s](%s) fb_base_addr = 0x%08x\n", __func__, dev_read_name(dev),
		(unsigned int)(plat->base & UINT_MAX));

	ret = ofnode_read_u32(ep_node, "remote-endpoint", &remote_phandle);
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		(void)pr_err("[ERROR][%s](%s) : remote-endpoint not found. ret = %d\n",
		       __func__, dev_read_name(dev), ret);
		priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
	}

	if (ret == 0) {
		remote = ofnode_get_by_phandle(remote_phandle);
		if (!ofnode_valid(remote)) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) : remote node not valid\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		output_media = NULL;
		/* coverity[misra_c_2012_rule_13_5_violation : FALSE] */
		while ((output_media == NULL) && ofnode_valid(remote)) {
			remote = ofnode_get_parent(remote);
			if (!ofnode_valid(remote)) {
				/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
				/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
				/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				(void)pr_err("[ERROR][%s](%s): no UCLASS_DISPLAY for remote-endpoint\n",
							 __func__, dev_read_name(dev));
				priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
				ret = -EINVAL;
			} else {
				uclass_find_device_by_ofnode(UCLASS_DISPLAY, remote, &output_media);
			}
		}
	}

	if (ret == 0) {
		priv->output_media = output_media;
		output_media_uc_plat = dev_get_uclass_plat(output_media);

		ret = device_probe(output_media);
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) display won't be probe ret=%d\n",
						 __func__, dev_read_name(dev), ret);
			priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
		}
	}

	if (ret == 0) {
		ret = display_read_timing(output_media, &priv->pan_timing);
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) fail to read display timing=%d\n",
						 __func__, dev_read_name(dev), ret);
			priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
		}
	}

	if (ret == 0) {
	/* Check if there is a component that complicts with other fb device */

		#if defined(CONFIG_TCC805X)
		/* display reset */
		(void)lcdc_display_device_reset(dev);
		#endif

		/* init video buffer */
		ret = tcc_fb_dm_init_image_buf(dev);
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) can't init image buf ret=%d\n",
						 __func__, dev_read_name(dev), ret);
			priv->fb_stat = TCC_FBDM_STAT_FB_BUF_ERR;
		}
	}

	if (priv->fb_draw_type == (u32)FB_DRAW_TYPE_BITMAP_HEADER) {
		if (ret == 0) {
			ret = get_tcc_fb_dm_logo_info(&priv->image_info.logo_base, priv->fb_buf_id);
			if (ret < 0) {
				/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
				/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
				/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				(void)pr_err("[ERROR][%s](%s) couldn't get logo data ret=%d\n",
							 __func__, dev_read_name(dev), ret);
				priv->fb_stat = TCC_FBDM_STAT_FB_BUF_ERR;
			}
		}

		/* bmp_display */
		if (ret == 0) {
			if (tcc_dm_global.logo_flag[priv->fb_buf_id] == 0U) {
				ret = tcc_fb_dm_bmp_display(dev, priv->image_info.logo_base);
				if (ret < 0) {
					/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
					/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
					/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
					/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
					/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
					/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
					(void)pr_err("[ERROR][%s](%s) bmp_display failed ret=%d\n",
								 __func__, dev_read_name(dev), ret);
					priv->fb_stat = TCC_FBDM_STAT_FB_BUF_ERR;
				}
				tcc_dm_global.logo_flag[priv->fb_buf_id] = 1;
			}
		}
	#if defined(CONFIG_SPLASH_LOGO_SUPPORT)
	} else if (priv->fb_draw_type == (u32)FB_DRAW_TYPE_SPLASH) {
		if (ret == 0) {
			/* coverity[cert_int36_c_violation : FALSE] */
			/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_7_4_violation : FALSE] */
			ret = splash_image_load_v2((unsigned char const *)SPLASH_PARTITION_NAME, priv->splash_img_name,
									   (void *)plat->base, &splash_width, &splash_height, &splash_format);
			if (ret < 0) {
				/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
				/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
				/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				(void)pr_err("[ERROR][%s](%s) splash_image_load_v2 failed ret=%d\n",
							 __func__, dev_read_name(dev), ret);
				priv->fb_stat = TCC_FBDM_STAT_FB_BUF_ERR;
			}
		}

		if (ret == 0) {
			ret = tcc_fb_dm_init_image_info_for_splash(&priv->image_info, splash_width, splash_height);
			if (ret < 0) {
				/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
				/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
				/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
				/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				(void)pr_err("[ERROR][%s](%s) splash_display failed ret=%d\n",
							 __func__, dev_read_name(dev), ret);
				priv->fb_stat = TCC_FBDM_STAT_FB_BUF_ERR;
			}
		}
	#endif
	} else {
		if (ret == 0) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) : not support bootlogo output type = %d\n",
						 __func__, dev_read_name(dev),
						 priv->fb_draw_type);
		}
	}

	if (ret == 0) {
		tcc_fb_dm_get_peri_id(priv->lcd_id, peri_lcd_id);

		if (peri_lcd_id == 0xFFFFUL) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s): invalid lcd_id[%u]\n",
						 __func__, dev_read_name(dev), priv->lcd_id);
			priv->fb_stat = TCC_FBDM_STAT_FB_PERI_ERR;
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		/* dm clock for peri_lcd is not ready */
		(void)tcc_set_peri(peri_lcd_id, flag_clk_en, priv->pan_timing.pixelclock.typ, 0);
		lclk = tcc_get_peri(peri_lcd_id);

		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_17_7_violation : FALSE] */
		pr_info("[%s] : reqeuest clk_rate = %u, set clk_rate = %lu\n",
				__func__, priv->pan_timing.pixelclock.typ, lclk);

		priv->pan_timing.pixelclock.typ = (u32)(lclk & UINT_MAX);
		priv->image_info.buf_addr0 = (u32)(plat->base & UINT_MAX);
		priv->image_info.enable = 1;
		tcclcd_image_ch_set(priv->lcd_id, &priv->image_info);

		ret = display_enable(output_media, 24, &priv->pan_timing);
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) fail to display enable = %d\n",
						 __func__, dev_read_name(dev), ret);
			priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
			ret = -EINVAL;
		}
		else {
			priv->fb_stat = TCC_FBDM_STAT_ENABLED;
		}
	}

	return ret;
}

/* coverity[HIS_metric_violation : FALSE] */
static int tcc_fb_dm_probe(struct udevice *dev)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	struct tcc_fb_dm_priv *priv = dev_get_priv(dev);
	ofnode ports, port, node;
	int ret = 0;

	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	pr_info("[%s](%s) : version : %d.%d.%d %s\n", __func__, dev_read_name(dev),
			tcc_dm_global.tcc_fb_dm_version.major,
			tcc_dm_global.tcc_fb_dm_version.minor,
			tcc_dm_global.tcc_fb_dm_version.patchlevel,
			tcc_dm_global.tcc_fb_dm_version.date);

	if (priv->fb_stat == TCC_FBDM_STAT_FB_PARSE_ERR) {
		/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_17_7_violation : FALSE] */
		(void)pr_err("[%s](%s) : probe failed due to device tree error\n",
			 __func__, dev_read_name(dev));
		ret = -EINVAL;
	}

	if (ret == 0) {
		/* Before relocation we don't need to do anything */
		/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
		if (!(gd->flags & GD_FLG_RELOC)) {
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_17_7_violation : FALSE] */
			pr_force("%s no GD_FLG_RELOC\r\n", __func__);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		/* individual display */
		if (priv->fb_buf_id > FB_BUF_CNT_MAX) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[%s](%s) invalid fb_buf_id[%d]. valid range[0-%d]\n",
						 __func__, dev_read_name(dev), priv->fb_buf_id, FB_BUF_CNT_MAX);
			priv->fb_stat = TCC_FBDM_STAT_FB_BUF_ERR;
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		plat->base = priv->fb_buf_addr;

		ports = dev_read_subnode(dev, "ports");
		if (!ofnode_valid(ports)) {
			ports = dev_ofnode(dev);
		}
		port = ofnode_find_subnode(ports, "port");
		if (!ofnode_valid(port)) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[%s](%s): 'port' subnode not found\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		/* coverity[misra_c_2012_rule_14_2_violation : FALSE] */
		for (node = ofnode_first_subnode(port);
			 ofnode_valid(node);
			 node = dev_read_next_subnode(node)) {
			ret = tcc_display_init(dev, node);
		}
	}

	return 0; //return value should be 0 for next fb device
}

/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
/* coverity[HIS_metric_violation : FALSE] */
static int tcc_fb_dm_ofdata_to_plat(struct udevice *dev)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	struct tcc_fb_dm_priv *priv = dev_get_priv(dev);
	int ret = 0;
	int ret_warn = 0;
	unsigned int of_lcd_id;
	int fb_id;

	priv->fb_stat = TCC_FBDM_STAT_DISABLED;
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	pr_info("[%s](%s)\n", __func__, dev_read_name(dev));

	fb_id = get_id_from_name(dev_read_name(dev));
	if (fb_id < 0) {
		/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		(void)pr_err("[ERROR][%s](%s) could't get id from the device\n",
		       __func__, dev_read_name(dev));
		priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		ret = -EINVAL;
	}
	if (ret == 0) {
		priv->id = (unsigned int)fb_id;

		ret = ofnode_read_u32(dev->node_, "lcd-id", &of_lcd_id);
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) could't get lcd-id\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
	}

	if (ret == 0) {
		priv->lcd_id = get_vioc_index(of_lcd_id);

		if (priv->lcd_id >= VIOC_DISP_MAX) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) lcd_id invalid[%d]\n",
						 __func__, dev_read_name(dev), priv->lcd_id);
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		ret_warn = ofnode_read_u32(dev->node_, "lcd-layer", &priv->image_info.lcd_layer);
		if (ret_warn < 0) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_warn("[WARN][%s](%s) could't get lcd-layer, set default layer to 0.\n",
					__func__, dev_read_name(dev));
			priv->image_info.lcd_layer = 0;
		}

		ret_warn = ofnode_read_u32(dev->node_, "lcd-ovp", &priv->ovp);
		if (ret_warn < 0) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_warn("[WARN][%s](%s) could't get lcd-ovp, set default ovp(Overlay priority) to 24.\n",
					__func__, dev_read_name(dev));
			priv->ovp = 24;
		}

		ret = ofnode_read_u32(dev->node_, "clk-div", &priv->clk_div);
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) could't get clk-div\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
	}

	if (ret == 0) {
		ret = ofnode_read_u32(dev->node_, "fb-draw-type", &priv->fb_draw_type);
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) could't get fb-draw-type\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
	}

	if (ret == 0) {
		ret = ofnode_read_u32(dev->node_, "fb-buf-id", &priv->fb_buf_id);
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_17_7_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) could't get fb-buf-id\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
	}

	if (ret == 0) {
		unsigned int buf_addr;

		ret = ofnode_read_u32(dev->node_, "fb-buf-addr", &buf_addr);
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_17_7_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) could't get fb-buf-addr\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
		priv->fb_buf_addr = (unsigned long)buf_addr;
	}

	if (ret == 0) {
		ret = ofnode_read_u32(dev->node_, "bpix", &priv->target_bpix);
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) could't get bpix\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
	}

	if (ret == 0) {
		if (priv->fb_draw_type == (u32)FB_DRAW_TYPE_SPLASH) {
			priv->splash_img_name = ofnode_read_string(dev->node_, "splash-img-name");
			if (priv->splash_img_name == NULL) {
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				(void)pr_warn("[WARN][%s](%s) splash-img-name is not defined\n",
							  __func__, dev_read_name(dev));
				/* coverity[misra_c_2012_rule_7_4_violation : FALSE] */
				priv->splash_img_name = FB_DM_DEFAULT_BOOTLOGO_NAME;
			}
		}

		ret = ofnode_read_u32(dev->node_, "lcd-mux-id", &priv->lcd_mux_id);
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) could't get lcd-mux-id\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
	}
	if (ret == 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_info("[%s] tcc_fb_dm_priv : fb id = %d\n", __func__, priv->id);
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_info("[%s] tcc_fb_dm_priv : lcd_id = %d\n", __func__, priv->lcd_id);
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_info("[%s] tcc_fb_dm_priv : lcd_layer = %d\n", __func__, priv->image_info.lcd_layer);
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_info("[%s] tcc_fb_dm_priv : ovp = %d\n", __func__, priv->ovp);
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_info("[%s] tcc_fb_dm_priv : clk_div = %d\n", __func__, priv->clk_div);
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_info("[%s] tcc_fb_dm_priv : fb_draw_type = %d\n", __func__, priv->fb_draw_type);
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_info("[%s] tcc_fb_dm_priv : fb_buf_id = %d\n", __func__, priv->fb_buf_id);
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_info("[%s] tcc_fb_dm_priv : target_bpix = %d\n", __func__, priv->target_bpix);
		if (priv->splash_img_name != NULL) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_info("[%s] tcc_fb_dm_priv : splash_img_name = %s\n", __func__, priv->splash_img_name);
		}
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_info("[%s] tcc_fb_dm_priv : lcd_mux_id = %d\n", __func__, priv->lcd_mux_id);
	}

	return ret;
}

static int tcc_fb_dm_bmp_display(struct udevice *dev, ulong addr)
{
	int ret = 0;
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
	struct bmp_image *bmp = (struct bmp_image *)map_sysmem(addr, 0);
	void *bmp_alloc_addr = NULL;
	bool align = (bool)false;
	unsigned long len;
	int x = 0, y = 0;

	#if defined(CONFIG_SPLASH_SCREEN_ALIGN)
	x = BMP_ALIGN_CENTER;
	y = BMP_ALIGN_CENTER;
	align = (bool)true;
	#endif

	if (!((bmp->header.signature[0] == 'B') &&
	      (bmp->header.signature[1] == 'M'))) {
		bmp = gunzip_bmp(addr, &len, &bmp_alloc_addr);
	}

	if (bmp == NULL) {
		/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_17_7_violation : FALSE] */
		(void)pr_err("[ERROR][%s](%s) : There is no valid bmp file at the given address\n",
					 __func__, dev_read_name(dev));
		ret = -EINVAL;
	}

	if ( ret == 0) {
		addr = map_to_sysmem(bmp);
		ret = video_bmp_display(dev, addr, x, y, align);
	}

	if (bmp_alloc_addr != NULL) {
		/* coverity[misra_c_2012_rule_21_3_violation : FALSE] */
		free(bmp_alloc_addr);
	}

	return ret;
}

/* coverity[HIS_metric_violation : FALSE] */
static int tcc_fb_dm_init_image_buf(struct udevice *dev)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	const struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	struct tcc_fb_dm_priv *priv = dev_get_priv(dev);
	int ret = 0;
	int fb_size;

	video_set_flush_dcache(dev, 1);

	if ((priv->pan_timing.hactive.typ == 0U) || (priv->pan_timing.vactive.typ == 0U)) {
		/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		(void)pr_err("[ERROR][%s](%s) : invalid_size value hactive[%d],vactive[%d]\n",
		       __func__, dev_read_name(dev),
		       priv->pan_timing.hactive.typ, priv->pan_timing.vactive.typ);
		ret = -EINVAL;
	}

	if (ret == 0) {
		if (priv->pan_timing.hactive.typ > MAX_INDIVIDUAL_IMAGE_WIDTH) {
			uc_priv->xsize = MAX_INDIVIDUAL_IMAGE_WIDTH;
			priv->image_info.image_width = MAX_INDIVIDUAL_IMAGE_WIDTH;
			priv->image_info.frame_width = MAX_INDIVIDUAL_IMAGE_WIDTH;
			priv->image_info.offset_x = (priv->pan_timing.hactive.typ - MAX_INDIVIDUAL_IMAGE_WIDTH) / 2U;

			/* coverity[misra_c_2012_rule_17_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_notice("[%s:%d]Panel W is larger than 1920 as %u, position offset x(%u)\n",
					  __func__,
					  __LINE__,
					  uc_priv->xsize,
					  priv->image_info.offset_x);
		} else {
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			uc_priv->xsize = (ushort)priv->pan_timing.hactive.typ;
			priv->image_info.image_width = uc_priv->xsize;
			priv->image_info.frame_width = uc_priv->xsize;
			priv->image_info.offset_x = 0;
		}

		if (priv->pan_timing.vactive.typ > MAX_INDIVIDUAL_IMAGE_HEIGHT) {
			uc_priv->ysize = MAX_INDIVIDUAL_IMAGE_HEIGHT;
			priv->image_info.image_height = MAX_INDIVIDUAL_IMAGE_HEIGHT;
			priv->image_info.frame_height = MAX_INDIVIDUAL_IMAGE_HEIGHT;
			priv->image_info.offset_y = (priv->pan_timing.vactive.typ - MAX_INDIVIDUAL_IMAGE_HEIGHT) / 2U;

			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
			pr_notice("[%s:%d]Panel H is larger than 1080 as %u, position offset y(%u)\n",
					  __func__,
					  __LINE__,
					  uc_priv->ysize,
					  priv->image_info.offset_y);
		} else {
			uc_priv->ysize = (ushort)(priv->pan_timing.vactive.typ & (u32)INT_MAX);
			priv->image_info.image_height = uc_priv->ysize;
			priv->image_info.frame_height = uc_priv->ysize;
			priv->image_info.offset_y = 0;
		}

		if (priv->target_bpix == (u32)TCC_VIDEO_BPP16) {
			uc_priv->bpix = VIDEO_BPP16;
			priv->image_info.fmt = TCC_LCDC_IMG_FMT_RGB565;
		} else if (priv->target_bpix == (u32)TCC_VIDEO_BPP32) {
			uc_priv->bpix = VIDEO_BPP32;
			priv->image_info.fmt = TCC_LCDC_IMG_FMT_RGB888;
		} else {
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) : target bpix[%u] not supported\n",
						 __func__, dev_read_name(dev), priv->target_bpix);
			ret = -EINVAL;
		}
	}

	if (ret  == 0) {
		if (uc_priv->line_length <= 0) {
			unsigned int nbytes_uint = 1U;
			int nbytes_int = 1;
			if((int)uc_priv->bpix >= 1) {
				nbytes_int = (int)uc_priv->bpix;
				nbytes_uint =  (unsigned int)nbytes_int;
				/* coverity[misra_c_2012_rule_10_6_violation : FALSE] */
				nbytes_uint = 1U << nbytes_uint;
				nbytes_uint /= 8U;
			}
			/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
			/* coverity[misra_c_2012_rule_10_8_violation : FALSE] */
			uc_priv->line_length = (int)(priv->image_info.frame_width * nbytes_uint);
		}

		/* coverity[misra_c_2012_rule_10_8_violation : FALSE] */
		if(uc_priv->line_length >= (int)MAX_LINE_LENGTH) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) : line_length[%d] not supported\n",
						 __func__, dev_read_name(dev), uc_priv->line_length);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		fb_size = (int)(uc_priv->line_length * (int)priv->image_info.frame_height);
		uc_priv->fb_size = fb_size;
		if(uc_priv->fb_size > (MAX_RES_WIDTH * MAX_RES_HEIGHT)) {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			(void)pr_err("[ERROR][%s](%s) : fb_size[%d] not supported\n",
						 __func__, dev_read_name(dev), uc_priv->fb_size);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		uc_priv->fb = map_sysmem(plat->base, (unsigned long)uc_priv->fb_size);

		video_set_default_colors(dev, false);

		if (tcc_dm_global.logo_flag[priv->fb_buf_id] == 0U) {
			/* clear background */
			video_clear(dev);
		}
	}

	return ret;
}

static int tcc_fb_dm_init_image_info_for_splash (struct tcc_fb_dm_image_info *image_info, unsigned int splash_width, unsigned int splash_height){

	int ret = 0;

	if ((splash_width == 0U) || (splash_height == 0U)) {
		/* coverity[misra_c_2012_rule_10_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_10_4_violation : FALSE] */
		/* coverity[misra_c_2012_rule_15_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_17_7_violation : FALSE] */
		(void)pr_err("[ERROR][%s] : invalid_size value splash width[%u], height[%u]\n",
			__func__, splash_width, splash_height);
		ret = -EINVAL;
	}

	if (ret == 0) {
		// Width offset
		if (image_info->frame_width > splash_width)
		{
			image_info->offset_x = ((image_info->frame_width - splash_width) / 2U);
		}

		// Height offset
		if (image_info->frame_height > splash_height)
		{
			image_info->offset_y = ((image_info->frame_height - splash_height) / 2U);
		}

		image_info->frame_width = splash_width;
		image_info->image_width = splash_width;
		image_info->frame_height = splash_height;
		image_info->image_height = splash_height;
	}
	return ret;
}

static const struct udevice_id tcc_fb_dm_ids[] = {
	{ .compatible = "telechips,tcc_fb_dm" },
	{ }
};

/* coverity[cert_dcl37_c_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
/* coverity[misra_c_2012_rule_21_2_violation : FALSE] */
U_BOOT_DRIVER(tcc_fb_dm) = {
	/* coverity[cert_str30_c_violation : FALSE] */
	.name	= "tcc_fb_dm",
	.id	= UCLASS_VIDEO,
	.of_match = tcc_fb_dm_ids,
	.probe	= tcc_fb_dm_probe,
	.bind = tcc_fb_dm_bind,
	.of_to_plat     = tcc_fb_dm_ofdata_to_plat,
	/* coverity[misra_c_2012_rule_10_3_violation : FALSE] */
	.priv_auto = sizeof(struct tcc_fb_dm_priv),
};
