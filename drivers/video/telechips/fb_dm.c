// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <display.h>
#include <clk.h>
#include <mach/clock.h>
#include <mach/reboot.h>
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
#include <telechips/fb_bootstage.h>
#include <telechips/fb_dm_lcd_interface.h>

#include <asm/arch/vioc/vioc_rdma.h>

#if defined(CONFIG_SPLASH_LOGO_SUPPORT)
#ifndef CONFIG_SPLASH_PARTITION_NAME
#error "Wrong configuration error: CONFIG_SPLASH_PARTITION_NAME has been set."
#else
#define SPLASH_PARTITION_NAME CONFIG_SPLASH_PARTITION_NAME
#endif
#endif

static int get_id_from_name(const char *str)
{
	int ret = 0;

	if (str == NULL) {
		ret = -1;
		debug_pr(LOGL_ERR, "[ERROR](%s) : NULL pointer\n",
					 __func__);
	} else {
		int int_val = (int)str[strlen(str) - 1U];

		if (int_val >= (int)'0') {
			ret = int_val - (int)'0';
		} else {
			ret = -1;
			debug_pr(LOGL_ERR, "[ERROR](%s) : invalid id = %d\n",
						 __func__, int_val);
		}
	}
	return ret;
}

struct tcc_fb_dm_global_info tcc_dm_global = {
	.tcc_fb_dm_version.date = "20250313",
	.tcc_fb_dm_version.name = "tcc_fb_dm",
	.tcc_fb_dm_version.major = 1,
	.tcc_fb_dm_version.minor = 7,
	.tcc_fb_dm_version.patchlevel = 0,
};

static int tcc_fb_dm_bmp_display(struct udevice *dev, ulong addr);
static int tcc_fb_dm_init_image_buf(struct udevice *dev);
#if defined(CONFIG_SPLASH_LOGO_SUPPORT)
static int tcc_fb_dm_init_image_info_for_splash(struct tcc_fb_dm_image_info *image_info, unsigned int splash_width, unsigned int splash_height);
#endif

struct tcc_logo {
	int width;
	int height;
	int bmp_bpix;
	const unsigned char *logo_data_base;
};

static const unsigned char logo_data_telechips[] = {
#include LOGO_TELECHIPS_HEADER
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
		debug_pr(LOGL_ERR, "[%s] : NULL addr\n", __func__);
		ret = -EINVAL;
	} else {
		if (tcc_logo_info[idx].logo_data_base != NULL) {
			*addr = (unsigned int)((unsigned long)tcc_logo_info[idx].logo_data_base & 0xFFFFFFFFU);
		} else {
			debug_pr(LOGL_ERR, "[%s] : no image data found. Please check init_log_arr()\n",
			       __func__);
			ret = -EINVAL;
		}
	}
	return ret;
};

static int tcc_fb_dm_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	//if size == 0, u-boot will not allocate memory. It's not error situation.
	plat->size = 0;

	return 0;
}

static unsigned long tcc_fb_dm_last_timestamp(void)
{
	const struct tcc_fb_dm_priv *priv;
	const struct udevice *dev;
	struct uclass *uc;

	unsigned long prev_timestamp, timestamp = 0u;

	uclass_id_foreach_dev(UCLASS_VIDEO, dev, uc) {
		if (device_active(dev)) {
			priv = dev_get_priv(dev);
			if (priv != NULL) {
				prev_timestamp = timestamp;
				timestamp = priv->timestamp;

				if (timestamp < prev_timestamp) {
					timestamp = prev_timestamp;
				}
			}
		}
	}
	return timestamp;
}

static int tcc_fb_dm_backlight(struct udevice *dev, bool blen)
{
	const struct tcc_fb_dm_priv *priv = dev_get_priv(dev);
	unsigned long last_timestamp, spent_time = 0u;

	char *fb_stage_name;

	uint32_t req_wait_time = 0u;
	uint32_t wait_time = 0u;
	int ret = 0;

	(void)blen;

	if (priv->output_media != NULL) {
		last_timestamp = tcc_fb_dm_last_timestamp();

		req_wait_time = display_backlight_wait_time(priv->output_media);

		if ((req_wait_time > 0u) && (req_wait_time < 5000u)) {
			req_wait_time *= 1000u; /* ms to us */
			spent_time = timer_get_boot_us() - last_timestamp;
			if (spent_time < (unsigned long)req_wait_time) {
				wait_time = req_wait_time - (uint32_t)spent_time;
				udelay(wait_time);
			}
		}
		ret = display_backlight(priv->output_media, (bool)true);
		debug_pr(LOGL_DEBUG, "%s req_wait_time = %d, spend_time = %lu, wait_time = %d\n",
		      __func__, req_wait_time, spent_time, wait_time);
		fb_stage_name = malloc(32);
		if (fb_stage_name != NULL) {
			/**
			 * Warning: bootstage_mark_name() stores the name as a pointer,
			 * not a copy.
			 * Make sure the string remains valid for the rest of boot time.
			 * If using malloc, do not free it.
			 */
			snprintf(fb_stage_name, 32, "fb_dm@%d backlight", dev_seq(dev));
			BOOTSTASGE_NAME(fb_stage_name);
		}
	} else {
		debug_pr(LOGL_DEBUG, "%s output media is NULL\n", __func__);
	}

	return ret;
}

static int tcc_display_init(struct udevice *dev, ofnode ep_node)
{
	const struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct tcc_fb_dm_priv *priv = dev_get_priv(dev);
	struct udevice *output_media;
	int ret = 0;
	u32 remote_phandle;
	const struct display_plat *output_media_uc_plat;
	ofnode remote;
	ulong flag_clk_en = CKC_ENABLE;
	ulong lclk = 0;
	ulong peri_lcd_id;
	int lcd_mux_select_idx;
#if defined(CONFIG_SPLASH_LOGO_SUPPORT)
	uint32_t splash_width = 0, splash_height = 0, splash_format = 0;
#endif

	debug_pr(LOGL_INFO, "[%s](%s) fb_base_addr = 0x%08x\n", __func__, dev_read_name(dev),
		(unsigned int)(plat->base & UINT_MAX));

	ret = ofnode_read_u32(ep_node, "remote-endpoint", &remote_phandle);
	if (ret < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : remote-endpoint not found. ret = %d\n",
		       __func__, dev_read_name(dev), ret);
		priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
	}

	if (ret == 0) {
		remote = ofnode_get_by_phandle(remote_phandle);
		if (!ofnode_valid(remote)) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : remote node not valid\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		output_media = NULL;
		while ((output_media == NULL) && ofnode_valid(remote)) {
			remote = ofnode_get_parent(remote);
			if (!ofnode_valid(remote)) {
				debug_pr(LOGL_ERR, "[ERROR][%s](%s): no UCLASS_DISPLAY for remote-endpoint\n",
							 __func__, dev_read_name(dev));
				priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
				ret = -EINVAL;
			} else {
				(void)uclass_find_device_by_ofnode(UCLASS_DISPLAY, remote, &output_media);
			}
		}
	}

	if (ret == 0) {
		priv->output_media = output_media;
		output_media_uc_plat = dev_get_uclass_plat(output_media);

		ret = device_probe(output_media);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) display won't be probe ret=%d\n",
						 __func__, dev_read_name(dev), ret);
			priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
		}
	}

	if (ret == 0) {
		ret = display_read_timing(output_media, &priv->pan_timing);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) fail to read display timing=%d\n",
						 __func__, dev_read_name(dev), ret);
			priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
		}
	}

	if (ret == 0) {
		/* Check if there is a component that conflicts with other fb device */

		/* display reset */
		(void)lcdc_display_device_reset(dev);

		/* init video buffer */
		ret = tcc_fb_dm_init_image_buf(dev);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) can't init image buf ret=%d\n",
						 __func__, dev_read_name(dev), ret);
			priv->fb_stat = TCC_FBDM_STAT_FB_BUF_ERR;
		}
	}

	if (priv->fb_draw_type == (u32)FB_DRAW_TYPE_BITMAP_HEADER) {
		if (ret == 0) {
			ret = get_tcc_fb_dm_logo_info(&priv->image_info.logo_base, priv->fb_buf_id);
			if (ret < 0) {
				debug_pr(LOGL_ERR, "[ERROR][%s](%s) couldn't get logo data ret=%d\n",
							 __func__, dev_read_name(dev), ret);
				priv->fb_stat = TCC_FBDM_STAT_FB_BUF_ERR;
			}
		}

		/* bmp_display */
		if (ret == 0) {
			if (tcc_dm_global.logo_flag[priv->fb_buf_id] == 0U) {
				ret = tcc_fb_dm_bmp_display(dev, priv->image_info.logo_base);
				if (ret < 0) {
					debug_pr(LOGL_ERR, "[ERROR][%s](%s) bmp_display failed ret=%d\n",
								 __func__, dev_read_name(dev), ret);
					priv->fb_stat = TCC_FBDM_STAT_FB_BUF_ERR;
				}
				tcc_dm_global.logo_flag[priv->fb_buf_id] = 1;
			}
		}
	#if defined(CONFIG_SPLASH_LOGO_SUPPORT)
	} else if (priv->fb_draw_type == (u32)FB_DRAW_TYPE_SPLASH) {
		if (ret == 0) {
			ret = splash_image_load_v2((const unsigned char *)SPLASH_PARTITION_NAME, priv->splash_img_name,
									   (void *)(uintptr_t)plat->base, &splash_width, &splash_height, &splash_format);
			if (ret < 0) {
				debug_pr(LOGL_ERR, "[ERROR][%s](%s) splash_image_load_v2 failed ret=%d\n",
							 __func__, dev_read_name(dev), ret);
				priv->fb_stat = TCC_FBDM_STAT_FB_BUF_ERR;
			}
		}

		if (ret == 0) {
			ret = tcc_fb_dm_init_image_info_for_splash(&priv->image_info, splash_width, splash_height);
			if (ret < 0) {
				debug_pr(LOGL_ERR, "[ERROR][%s](%s) splash_display failed ret=%d\n",
							 __func__, dev_read_name(dev), ret);
				priv->fb_stat = TCC_FBDM_STAT_FB_BUF_ERR;
			}
		}
	#endif
	} else if (priv->fb_draw_type == (u32)FB_DRAW_TYPE_DISABLE) {
		if (ret == 0) {
			debug_pr(LOGL_INFO, "[INFO][%s](%s) : disable bootlogo output type = %d\n",
						 __func__, dev_read_name(dev),
						 priv->fb_draw_type);
		}
	} else {
		if (ret == 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : not support bootlogo output type = %d\n",
						 __func__, dev_read_name(dev),
						 priv->fb_draw_type);
		}
	}
	if (ret == 0) {
		#if defined(CONFIG_TCC807X)
		if ((priv->svsc_trvc_en == 1u) || (priv->svsc_sdm_en == 1u)) {
			(void)lcdc_set_svsc_clk(dev);
			(void)lcdc_set_svsc_evs(dev);
		}
		#endif

		tcc_fb_dm_get_peri_id(get_vioc_index(priv->lcd_disp), peri_lcd_id);

		if (peri_lcd_id == 0xFFFFUL) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s): invalid lcd_disp[%u]\n",
						__func__, dev_read_name(dev),
						get_vioc_index(priv->lcd_disp));
			priv->fb_stat = TCC_FBDM_STAT_FB_PERI_ERR;
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		/* dm clock for peri_lcd is not ready */
		(void)tcc_set_peri(peri_lcd_id, flag_clk_en, priv->pan_timing.pixelclock.typ, 0);
		lclk = tcc_get_peri(peri_lcd_id);

		debug_pr(LOGL_INFO, "[%s] : reqeuest clk_rate = %u, set clk_rate = %lu\n",
				__func__, priv->pan_timing.pixelclock.typ, lclk);

		priv->pan_timing.pixelclock.typ = (u32)(lclk & UINT_MAX);
		priv->image_info.buf_addr0 = (u32)(plat->base & UINT_MAX);
		priv->image_info.enable = 1;
		tcclcd_image_ch_set(priv);

		ret = display_enable(output_media, 24, &priv->pan_timing);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) fail to display enable = %d\n",
						__func__, dev_read_name(dev), ret);
			priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
			ret = -EINVAL;
		} else {
			for (lcd_mux_select_idx = 0; lcd_mux_select_idx < (priv->lcd_mux_select_num - 1); lcd_mux_select_idx++) {
				ret |= lcdc_mux_select(priv->ext_lcd_mux_select[lcd_mux_select_idx],
					 get_vioc_index(priv->lcd_disp));
			}
			priv->fb_stat = TCC_FBDM_STAT_ENABLED;
			priv->timestamp = timer_get_boot_us();
		}
	}
	return ret;
}

static int tcc_fb_dm_probe(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct tcc_fb_dm_priv *priv = dev_get_priv(dev);
	char *fb_stage_name;
	ofnode ports, port, node;
	int ret = 0;

	fb_stage_name = malloc(32);
	if (fb_stage_name != NULL) {
		/**
		 * Warning: bootstage_mark_name() stores the name as a pointer,
		 * not a copy.
		 * Make sure the string remains valid for the rest of boot time.
		 * If using malloc, do not free it.
		 */
		snprintf(fb_stage_name, 32, "fb_dm@%d probe", dev_seq(dev));
		BOOTSTASGE_NAME(fb_stage_name);
	}

	if (get_boot_reason() == BOOT_PANIC) {
		/*
		 * If the boot reason is BOOT_PANIC, it generally means that the
		 * kernel has stopped while the display output remains active.
		 * In this case, the following condition check is implemented
		 * to prevent further display configuration changes.
		 */
		(void)pr_force("## %s: skipped due to BOOT_PANIC\r\n", dev_read_name(dev));
		ret = -ENODEV;
	} else {
		debug_pr(LOGL_INFO, "[%s](%s) : version : %d.%d.%d %s\n", __func__, dev_read_name(dev),
				tcc_dm_global.tcc_fb_dm_version.major,
				tcc_dm_global.tcc_fb_dm_version.minor,
				tcc_dm_global.tcc_fb_dm_version.patchlevel,
				tcc_dm_global.tcc_fb_dm_version.date);

		if (priv->fb_stat == TCC_FBDM_STAT_FB_PARSE_ERR) {
			debug_pr(LOGL_ERR, "[%s](%s) : probe failed due to device tree error\n",
				__func__, dev_read_name(dev));
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		/* Before relocation we don't need to do anything */
		if ((gd->flags & (unsigned)GD_FLG_RELOC) == 0U) {
			(void)pr_force("%s no GD_FLG_RELOC\r\n", __func__);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		/* individual display */
		if (priv->fb_buf_id > FB_BUF_CNT_MAX) {
			debug_pr(LOGL_ERR, "[%s](%s) invalid fb_buf_id[%d]. valid range[0-%d]\n",
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
			debug_pr(LOGL_ERR, "[%s](%s:%d): 'port' subnode not found\n",
						 __func__, dev_read_name(dev), __LINE__);
			priv->fb_stat = TCC_FBDM_STAT_REMOTE_ERR;
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		for (node = ofnode_first_subnode(port);
			 ofnode_valid(node);
			 node = dev_read_next_subnode(node)) {
			(void)tcc_display_init(dev, node);
		}
	}
	fb_stage_name = malloc(32);
	if (fb_stage_name != NULL) {
		/**
		 * Warning: bootstage_mark_name() stores the name as a pointer,
		 * not a copy.
		 * Make sure the string remains valid for the rest of boot time.
		 * If using malloc, do not free it.
		 */
		snprintf(fb_stage_name, 32, "fb_dm@%d probe:done", dev_seq(dev));
		BOOTSTASGE_NAME(fb_stage_name);
	}
	return 0; //return value should be 0 for next fb device
}

static int tcc_fb_dm_ofdata_to_plat(struct udevice *dev)
{
	struct tcc_fb_dm_priv *priv = dev_get_priv(dev);
	int ret = 0;
	int ret_warn = 0;
	unsigned int of_vioc_id;
	int fb_id;
	uint32_t tmp_lcd_mux_select[PANEL_LCD_MUX_MAX];

	priv->fb_stat = TCC_FBDM_STAT_DISABLED;
	debug_pr(LOGL_INFO, "[%s](%s)\n", __func__, dev_read_name(dev));

	fb_id = get_id_from_name(dev_read_name(dev));
	if (fb_id < 0) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) could't get id from the device\n",
		       __func__, dev_read_name(dev));
		priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		ret = -EINVAL;
	}

	/* RDMA */
	if (ret == 0) {
		priv->id = (unsigned int)fb_id;

		ret = ofnode_read_u32(dev->node_, "lcd-rdma", &of_vioc_id);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) could't get lcd-rdma\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
	}

	if (ret == 0) {
		priv->lcd_rdma = of_vioc_id;

		if (get_vioc_index(priv->lcd_rdma) >= VIOC_RDMA_MAX) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) lcd-rdma invalid[%d]\n",
						 __func__, dev_read_name(dev),
						 get_vioc_index(priv->lcd_rdma));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
			ret = -EINVAL;
		}
	}

	/* WMIX */
	if (ret == 0) {
		if (VIOC_RDMA_HAS_WMIX(priv->lcd_rdma) == 0) {
			of_vioc_id = 0u;
		} else {
			ret = ofnode_read_u32(dev->node_, "lcd-wmix", &of_vioc_id);
			if (ret < 0) {
				debug_pr(LOGL_ERR, "[ERROR][%s](%s) lcd-wmix invalid[%d]\n",
							__func__, dev_read_name(dev),
							get_vioc_index(priv->lcd_wmix));
				priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
			}
		}
	}

	if (ret == 0) {
		priv->lcd_wmix = of_vioc_id;

		if (get_vioc_index(priv->lcd_wmix) >= VIOC_WMIX_MAX) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) lcd-wmix invalid[%d]\n",
						 __func__, dev_read_name(dev),
						 get_vioc_index(priv->lcd_wmix));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
			ret = -EINVAL;
		}
	}

	/* DISP */
	if (ret == 0) {
		ret = ofnode_read_u32(dev->node_, "lcd-disp", &of_vioc_id);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) could't get lcd-disp\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
	}

	if (ret == 0) {
		priv->lcd_disp = of_vioc_id;

		if (get_vioc_index(priv->lcd_disp) >= VIOC_DISP_MAX) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) lcd-disp invalid[%d]\n",
						 __func__, dev_read_name(dev), get_vioc_index(priv->lcd_disp));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		ret_warn = ofnode_read_u32(dev->node_, "lcd-ovp", &priv->ovp);
		if (ret_warn < 0) {
			debug_pr(LOGL_WARNING, "[WARN][%s](%s) could't get lcd-ovp, set default ovp(Overlay priority) to 24.\n",
					__func__, dev_read_name(dev));
			priv->ovp = 24;
		}

		ret = ofnode_read_u32(dev->node_, "clk-div", &priv->clk_div);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) could't get clk-div\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
	}

	if (ret == 0) {
		ret = ofnode_read_u32(dev->node_, "fb-draw-type", &priv->fb_draw_type);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) could't get fb-draw-type\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
	}

	if (ret == 0) {
		ret = ofnode_read_u32(dev->node_, "fb-buf-id", &priv->fb_buf_id);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) could't get fb-buf-id\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
	}

	if (ret == 0) {
		unsigned int buf_addr;

		ret = ofnode_read_u32(dev->node_, "fb-buf-addr", &buf_addr);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) could't get fb-buf-addr\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
		priv->fb_buf_addr = (unsigned long)buf_addr;
	}

	if (ret == 0) {
		ret = ofnode_read_u32(dev->node_, "bpix", &priv->target_bpix);
		if (ret < 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) could't get bpix\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
		}
	}

	if (ret == 0) {
		if (priv->fb_draw_type == (u32)FB_DRAW_TYPE_SPLASH) {
			priv->splash_img_name = ofnode_read_string(dev->node_, "splash-img-name");
			if (priv->splash_img_name == NULL) {
				debug_pr(LOGL_WARNING, "[WARN][%s](%s) splash-img-name is not defined, set default splash-img-name to '%s'.\n",
							  __func__, dev_read_name(dev), FB_DM_DEFAULT_BOOTLOGO_NAME);
				priv->splash_img_name = (const unsigned char *)FB_DM_DEFAULT_BOOTLOGO_NAME;
			}
		}

		(void)ofnode_get_property(dev->node_, "lcd-mux-select", &priv->lcd_mux_select_num);
		priv->lcd_mux_select_num /= (int)sizeof(uint32_t);

		if (priv->lcd_mux_select_num <= 0) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) could't get lcd-mux-select\n",
						 __func__, dev_read_name(dev));
			priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
			ret = -EINVAL;
		} else {
			ret = ofnode_read_u32_array(dev->node_, "lcd-mux-select", tmp_lcd_mux_select, (size_t)priv->lcd_mux_select_num);

			if (ret < 0) {
				debug_pr(LOGL_ERR, "[ERROR][%s](%s) could't get lcd-mux-select\n",
							 __func__, dev_read_name(dev));
				priv->fb_stat = TCC_FBDM_STAT_FB_PARSE_ERR;
			} else if (priv->lcd_mux_select_num == 1) {
				priv->lcd_mux_select = tmp_lcd_mux_select[0];
			} else {
				priv->lcd_mux_select = tmp_lcd_mux_select[0];
				(void)memcpy(priv->ext_lcd_mux_select, &tmp_lcd_mux_select[1],
				   (sizeof(uint32_t) * (unsigned int)(priv->lcd_mux_select_num - 1)));
			}
		}
	}

	if (ret == 0) {
		if ((SDM_MUX_SELECT != NO_COMPONENT_SDM_TRVC) || (TRVC_MUX_SELECT != NO_COMPONENT_SDM_TRVC)) {
			ret_warn = ofnode_read_u32(dev->node_, "lcd-mux-bypass", &priv->lcd_mux_bypass);
			if (ret_warn < 0) {
				debug_pr(LOGL_WARNING, "[WARN][%s](%s) there is no lcd-mux-bypass, default set bypass\n",
							 __func__, dev_read_name(dev));
				priv->lcd_mux_bypass = 1u;
			}
			#if defined(CONFIG_TCC807X)
			ret_warn = ofnode_read_u32(dev->node_, "svsc-trvc-en", &priv->svsc_trvc_en);
			if (ret_warn < 0) {
				debug_pr(LOGL_DEBUG, "[DEBUG][%s](%s) there is no trvc-en, default set disabled\n",
							 __func__, dev_read_name(dev));
				priv->svsc_trvc_en = 0u;
			}

			ret_warn = ofnode_read_u32(dev->node_, "svsc-sdm-en", &priv->svsc_sdm_en);
			if (ret_warn < 0) {
				debug_pr(LOGL_DEBUG, "[DEBUG][%s](%s) there is no sdm-en, default set disabled\n",
							 __func__, dev_read_name(dev));
				priv->svsc_sdm_en = 0u;
			}

			if ((priv->lcd_mux_bypass == 1u) && (priv->svsc_sdm_en == 1u) && (priv->lcd_mux_select == SDM_MUX_SELECT)) {
				debug_pr(LOGL_WARNING, "[WARN][%s](%s) SVSC cannot be enabled in the mux-bypass setting, Change SVSC to disabled\n",
							 __func__, dev_read_name(dev));
				priv->svsc_sdm_en = 0u;
			}
			if ((priv->lcd_mux_bypass == 1u) && (priv->svsc_trvc_en == 1u) && (priv->lcd_mux_select == TRVC_MUX_SELECT)) {
				debug_pr(LOGL_WARNING, "[WARN][%s](%s) SVSC cannot be enabled in the mux-bypass setting, Change SVSC to disabled\n",
							 __func__, dev_read_name(dev));
				priv->svsc_trvc_en = 0u;
			}
			#endif
		}
	}

	if (ret == 0) {
		ofnode virtual_config_node;
		virtual_config_node = dev_read_subnode(dev, "virtual-config");

		if ((ofnode_valid(virtual_config_node)) && ofnode_is_enabled(virtual_config_node)) {
			ret = ofnode_read_u32(virtual_config_node, "width", &priv->image_info.virtual_width);
			if (ret == 0) {
				ret = ofnode_read_u32(virtual_config_node, "height", &priv->image_info.virtual_height);
			}
			if (ret == 0) {
				ret = ofnode_read_u32(virtual_config_node, "offset-x", &priv->image_info.virtual_offset_x);
			}
			if (ret == 0) {
				ret = ofnode_read_u32(virtual_config_node, "offset-y", &priv->image_info.virtual_offset_y);
			}
			debug_pr(LOGL_INFO, "[%s:%d] virtual width[%d] height[%d] offset_x[%d] offset_y[%d]\n", __func__, __LINE__
					, priv->image_info.virtual_width, priv->image_info.virtual_height
					, priv->image_info.virtual_offset_x, priv->image_info.virtual_offset_y);
		}
	}

	if (ret == 0) {
		debug_pr(LOGL_INFO, "[%s] tcc_fb_dm_priv : fb id = %d\n", __func__, priv->id);
		debug_pr(LOGL_INFO, "[%s] tcc_fb_dm_priv : ovp = %d\n", __func__, priv->ovp);
		debug_pr(LOGL_INFO, "[%s] tcc_fb_dm_priv : clk_div = %d\n", __func__, priv->clk_div);
		debug_pr(LOGL_INFO, "[%s] tcc_fb_dm_priv : fb_draw_type = %d\n", __func__, priv->fb_draw_type);
		debug_pr(LOGL_INFO, "[%s] tcc_fb_dm_priv : fb_buf_id = %d\n", __func__, priv->fb_buf_id);
		debug_pr(LOGL_INFO, "[%s] tcc_fb_dm_priv : target_bpix = %d\n", __func__, priv->target_bpix);
		if (priv->splash_img_name != NULL) {
			debug_pr(LOGL_INFO, "[%s] tcc_fb_dm_priv : splash_img_name = %s\n", __func__, priv->splash_img_name);
		}
		debug_pr(LOGL_INFO, "[%s] tcc_fb_dm_priv : lcd_mux_select = %d\n", __func__, priv->lcd_mux_select);
		debug_pr(LOGL_INFO, "[%s] tcc_fb_dm_priv : lcd_mux_bypass = %d\n", __func__, priv->lcd_mux_bypass);
		#if defined(CONFIG_TCC807X)
		debug_pr(LOGL_INFO, "[%s] tcc_fb_dm_priv : svsc_sdm_en = %d\n", __func__, priv->svsc_sdm_en);
		debug_pr(LOGL_INFO, "[%s] tcc_fb_dm_priv : svsc_trvc_en = %d\n", __func__, priv->svsc_trvc_en);
		#endif
	}

	return ret;
}

static int tcc_fb_dm_bmp_display(struct udevice *dev, ulong addr)
{
	int ret = 0;
	const struct bmp_image *bmp = (struct bmp_image *)map_sysmem(addr, 0);
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
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : There is no valid bmp file at the given address\n",
					 __func__, dev_read_name(dev));
		ret = -EINVAL;
	}

	if (ret == 0) {
		addr = map_to_sysmem(bmp);
		ret = video_bmp_display(dev, addr, x, y, align);
	}

	if (bmp_alloc_addr != NULL) {
		free(bmp_alloc_addr);
	}

	return ret;
}

static int tcc_fb_dm_init_image_buf(struct udevice *dev)
{
	const struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct tcc_fb_dm_priv *priv = dev_get_priv(dev);
	int ret = 0;
	int fb_size;

	video_set_flush_dcache(dev, 1);

	if ((priv->pan_timing.hactive.typ == 0U) || (priv->pan_timing.vactive.typ == 0U)) {
		debug_pr(LOGL_ERR, "[ERROR][%s](%s) : invalid_size value hactive[%d],vactive[%d]\n",
		       __func__, dev_read_name(dev),
		       priv->pan_timing.hactive.typ, priv->pan_timing.vactive.typ);
		ret = -EINVAL;
	}

	if (ret == 0) {
		if ((priv->image_info.virtual_width != 0u) || (priv->image_info.virtual_height != 0u)) {
			priv->image_info.image_width = priv->image_info.virtual_width;
			uc_priv->xsize = (unsigned short)(priv->image_info.virtual_width & 0xFFFFU);
			priv->image_info.offset_x = priv->image_info.virtual_offset_x;

			priv->image_info.image_height = priv->image_info.virtual_height;
			uc_priv->ysize = (unsigned short)(priv->image_info.virtual_height & 0xFFFFU);
			priv->image_info.offset_y = priv->image_info.virtual_offset_y;

		} else {
			if (priv->pan_timing.hactive.typ > MAX_INDIVIDUAL_IMAGE_WIDTH) {
				priv->image_info.image_width = MAX_INDIVIDUAL_IMAGE_WIDTH;
				priv->image_info.offset_x = (priv->pan_timing.hactive.typ - MAX_INDIVIDUAL_IMAGE_WIDTH) / 2U;

				debug_pr(LOGL_WARNING, "[%s:%d]Panel W is larger than 1920 as %u, position offset x(%u)\n",
						  __func__,
						  __LINE__,
						  uc_priv->xsize,
						  priv->image_info.offset_x);
				uc_priv->xsize = MAX_INDIVIDUAL_IMAGE_WIDTH;
			} else {
				uc_priv->xsize = (ushort)priv->pan_timing.hactive.typ;
				priv->image_info.image_width = uc_priv->xsize;
				priv->image_info.offset_x = 0;
			}

			if (priv->pan_timing.vactive.typ > MAX_INDIVIDUAL_IMAGE_HEIGHT) {
				uc_priv->ysize = MAX_INDIVIDUAL_IMAGE_HEIGHT;
				priv->image_info.image_height = MAX_INDIVIDUAL_IMAGE_HEIGHT;
				priv->image_info.offset_y = (priv->pan_timing.vactive.typ - MAX_INDIVIDUAL_IMAGE_HEIGHT) / 2U;

				debug_pr(LOGL_WARNING, "[%s:%d]Panel H is larger than 1080 as %u, position offset y(%u)\n",
						  __func__,
						  __LINE__,
						  uc_priv->ysize,
						  priv->image_info.offset_y);
			} else {
				uc_priv->ysize = (ushort)(priv->pan_timing.vactive.typ & 0xFFFFU);
				priv->image_info.image_height = uc_priv->ysize;
				priv->image_info.offset_y = 0;
			}
		}

		if (priv->target_bpix == (u32)TCC_VIDEO_BPP16) {
			uc_priv->bpix = VIDEO_BPP16;
			priv->image_info.fmt = TCC_LCDC_IMG_FMT_RGB565;
		} else if (priv->target_bpix == (u32)TCC_VIDEO_BPP32) {
			uc_priv->bpix = VIDEO_BPP32;
			priv->image_info.fmt = TCC_LCDC_IMG_FMT_RGB888;
		} else {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : target bpix[%u] not supported\n",
						 __func__, dev_read_name(dev), priv->target_bpix);
			ret = -EINVAL;
		}
	}

	if (ret  == 0) {
		if (uc_priv->line_length <= 0) {
			unsigned int nbytes_uint = 1U;
			int nbytes_int = 1;

			if ((int)uc_priv->bpix >= 1) {
				nbytes_int = (int)uc_priv->bpix;
				nbytes_uint =  (unsigned int)nbytes_int;
				nbytes_uint = ((unsigned int)1U << nbytes_uint);
				nbytes_uint /= 8U;
			}

			if (priv->image_info.image_width <= (UINT_MAX / nbytes_uint)) {
				uc_priv->line_length = (signed)(priv->image_info.image_width * nbytes_uint);
			} else {
				uc_priv->line_length = 0;
			}
		}

		if (uc_priv->line_length >= MAX_LINE_LENGTH) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : line_length[%d] not supported\n",
						 __func__, dev_read_name(dev), uc_priv->line_length);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		if ((unsigned int)uc_priv->line_length <= (UINT_MAX / priv->image_info.image_height)) {
			fb_size = (signed)((unsigned int)uc_priv->line_length * priv->image_info.image_height);
		} else {
			fb_size = 0;
		}

		uc_priv->fb_size = fb_size;

		if (uc_priv->fb_size > (MAX_RES_WIDTH * MAX_RES_HEIGHT)) {
			debug_pr(LOGL_ERR, "[ERROR][%s](%s) : fb_size[%d] not supported\n",
						 __func__, dev_read_name(dev), uc_priv->fb_size);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		uc_priv->fb = map_sysmem(plat->base, (unsigned long)uc_priv->fb_size);

		video_set_default_colors(dev, false);
		if (tcc_dm_global.logo_flag[priv->fb_buf_id] == 0U) {
			/* clear background */
			(void)video_clear(dev);
		}
	}

	return ret;
}

#if defined(CONFIG_SPLASH_LOGO_SUPPORT)
static int tcc_fb_dm_init_image_info_for_splash(struct tcc_fb_dm_image_info *info, unsigned int splash_width, unsigned int splash_height)
{
	int ret = 0;

	if ((splash_width == 0U) || (splash_height == 0U)) {
		debug_pr(LOGL_ERR, "[ERROR][%s] : invalid_size value splash width[%u], height[%u]\n",
			__func__, splash_width, splash_height);
		ret = -EINVAL;
	}

	if (ret == 0) {
		// Width offset
		if (info->image_width > splash_width) {
			info->offset_x = ((info->image_width - splash_width) / 2U);
		}

		// Height offset
		if (info->image_height > splash_height) {
			info->offset_y = ((info->image_height - splash_height) / 2U);
		}

		info->image_width = splash_width;
		info->image_height = splash_height;
	}
	return ret;
}
#endif

static const struct udevice_id tcc_fb_dm_ids[] = {
	{ .compatible = "telechips,tcc_fb_dm" },
	{ }
};


static struct video_ops tcc_fb_dm_ops = {
	.backlight_fn = tcc_fb_dm_backlight,
};

U_BOOT_DRIVER(tcc_fb_dm) = {
	.name = "tcc_fb_dm",
	.id = UCLASS_VIDEO,
	.ops = &tcc_fb_dm_ops,
	.of_match = tcc_fb_dm_ids,
	.probe = tcc_fb_dm_probe,
	.bind = tcc_fb_dm_bind,
	.of_to_plat = tcc_fb_dm_ofdata_to_plat,
	.priv_auto = (int)sizeof(struct tcc_fb_dm_priv),
};
