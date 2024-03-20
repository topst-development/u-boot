// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <debug.h>
#include "../include/switch.h"
#include "../include/camera.h"
#include "../include/cam_ipc.h"
#include "../include/dev/videosource_if.h"
#include "../include/lcdc.h"
#include "../include/earlycamera_main.h"

#include <asm/io.h>

static void check_video_input_path(void)
{
	int		idx_try;
	int		n_try;
	int		path_status;

	n_try		= 5;

	for (idx_try = 0; idx_try < n_try; idx_try++) {
		path_status = tcc_cif_check_video_input_path();
		if (path_status != 0) {
			loge("vin path is NOT working\n");
			loge("vin path will be recovered\n");
			tcc_cif_recovery_video_input_path();
		} else {
			logd("vin path is working\n");
			break;
		}
	}
}

static void check_videosource_if(struct udevice *vs_dev)
{
	int		n_delay			= 50;
	int		idx_try;
	int		n_stable;
	int		idx_stable;
	int		n_try;
	int		ret;

	n_delay		= 50;
	n_try		= 700 / n_delay;
	idx_stable	= 0;
	n_stable	= 1;
	for (idx_try = 0; idx_try < n_try; idx_try++) {
		ret = videosource_if_video_check(vs_dev);
		if (ret == 0) {
			loge("videosource is NOT working\n");
			idx_stable = 0;
			mdelay((unsigned int)n_delay);
		} else {
			logd("videosource is working\n");
			idx_stable++;
			if (n_stable <= idx_stable) {
				logd("videosource is stable\n");
				break;
			}
		}
	}
}

static void init_vsrc(struct udevice **vs_dev)
{
	// init video source
	(void)videosource_if_init();
	*vs_dev = videosource_if_get_videosource();
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	parameters_data.vsrc = dev_get_priv(*vs_dev);

	// print all params
	tcc_cif_dump_parameters();

	// index vioc components
	tcc_cif_set_viocs_addr();
}

static int open_videosource(struct udevice *vs_dev)
{
	return (int)((int)(
		(videosource_if_open(vs_dev) == 0) &&
		(videosource_if_tune(vs_dev,
			(int)CAM_TYPE_DEFAULT, (int)CAM_ENC_DEFAULT) == 0)) == 0);
}

static void start_stream(struct udevice *vs_dev, int switch_state)
{
	check_videosource_if(vs_dev);
	// start preview
	tcc_cif_start_stream();
	check_video_input_path();
	// display as the init state
	if ((switch_state == 0) || (switch_state == 1)) {
		tcc_lcd_rear_camera_display((unsigned char)switch_state & 0xFFU);
	}
}

static int prepare_preview(struct udevice *vs_dev, const struct udevice *switch_dev)
{
	int		switch_state		= 0;
	int		ret			= 0;

	logi("video-input path was NOT working\n");

	switch_state = switch_if_get_status(switch_dev);
	if (switch_state == 1) {
		if (open_videosource(vs_dev) == 0) {
			start_stream(vs_dev, switch_state);
		} else {
			ret = -1;
		}
	}

	return ret;
}

int tcc_earlycamera_start(void)
{
	const struct udevice	*switch_dev;
	struct udevice	*vs_dev;
	int		ret			= 0;

	(void)init_cam_ipc();
	// parse and update switch
	(void)switch_if_init();
	switch_dev = switch_if_get_switch();
	if (switch_dev == NULL) {
		loge("switch device has not been probed.\n");
		ret = -1;
	} else {
		init_vsrc(&vs_dev);
		if (tcc_cif_check_video_input_path() != 0) {
			ret = prepare_preview(vs_dev, switch_dev);
		} else {
			logi("video-input path is already working\n");
			logi("Handover - Skip to initialize vs and video-input path\n");
		}
	}

	return ret;
}

int tcc_earlycamera_stop(void)
{
	const struct udevice	*dev;
	int			ret		= 0;

	// display as the init state
	tcc_lcd_rear_camera_display(0);

	dev = videosource_if_get_videosource();
	ret = videosource_if_close(dev);
	if (ret != 0) {
		// failed to close videosource
		loge("videosource_if_close\n");
	}

	return ret;
}
