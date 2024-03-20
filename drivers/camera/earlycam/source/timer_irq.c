// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <debug.h>
#include <config.h>
#include <common.h>

#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/telechips/gpio.h>
#include <asm/arch/timer_api.h>
#include <timer_irq.h>
#include "camera.h"
#include "lcdc.h"
#include "switch.h"

#if defined(CONFIG_EARLYCAMERA_SOLUTION_TIME_MS) && \
	(CONFIG_EARLYCAMERA_SOLUTION_TIME_MS > 0)
#define REVERSE_GEAR_TIMEMS	(CONFIG_EARLYCAMERA_SOLUTION_TIME_MS * 1000)
#else
#define REVERSE_GEAR_TIMEMS	(0)
#endif

static struct tcc_timer		*earlycamera_timer;
static int			timer_regiter_status;

static inline void check_state_and_do_recovery(void)
{
	int ret = tcc_cif_check_video_input_path();
	if (ret < 0) {
		loge("vin path is NOT working\n");

		tcc_lcd_rear_camera_display(0);
		tcc_cif_recovery_video_input_path();
		ret = tcc_cif_check_video_input_path();
		if (ret < 0) {
			// going to be recovered
			tcc_cif_recovery();
		}
		tcc_lcd_rear_camera_display(1);
	}
}

static inline void preview_if_available(int *disp_initialized)
{
	*disp_initialized = tcc_lcd_is_disp_busy();

	if (*disp_initialized == 0) {
		tcc_lcd_rear_camera_display(1);
		*disp_initialized = 1;
	}
}

static void open_vs_start_stream(struct udevice *dev)
{
	(void)videosource_if_open(dev);
	(void)videosource_if_tune(dev, tcc_cif_get_camera_type(),
		(int)CAM_ENC_DEFAULT);

	tcc_cif_start_stream();
}

static void stop_stream(const struct udevice *dev)
{
	tcc_cif_stop_stream();
	(void)videosource_if_close(dev);
}

static void handle_switch_changed(const int *prev_state, const int *curr_state)
{
	struct udevice	*dev				= NULL;
	int		ret				= 0;
	unsigned char	curr_state_conv 		= 0;

	if ((*curr_state == 0) || (*curr_state == 1)) {
		curr_state_conv = (unsigned char)*curr_state & 0xFFU;
	}

	logd("prev_state: %d, curr_state: %d\n", *prev_state, *curr_state);

	dev = videosource_if_get_videosource();
	ret = tcc_cif_check_video_input_path();

	if ((ret < 0) && (*curr_state == 1)) {
		open_vs_start_stream(dev);
	} else if ((ret == 0) && (*curr_state == 0)) {
		stop_stream(dev);
	} else {
		;	/* nothing to do */
	}

	tcc_lcd_rear_camera_display(curr_state_conv);
}

/* coverity[misra_c_2012_rule_2_7_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_13_violation : FALSE] */
static void tcc_earlycamera_interrupt_handler(void *param)
{
	const struct udevice	*switch_dev;
	static int	prev_state			= -1;
	static int	curr_state			= -1;
	static int	is_display_device_initialized	= 0;

	switch_dev = switch_if_get_switch();

	if (switch_dev == NULL)
		loge("switch device is NULL\n");

	if (curr_state == -1) {
		curr_state = switch_if_get_status(switch_dev);
	}

	prev_state = curr_state;
	curr_state = switch_if_get_status(switch_dev);
	if ((prev_state == curr_state) && (curr_state == 1)) {
		// check if the vioc path is working.
		check_state_and_do_recovery();
		preview_if_available(&is_display_device_initialized);
	} else if (prev_state != curr_state) {
		handle_switch_changed(&prev_state, &curr_state);
	} else {
		/* nothing to do */
		;
	}

}

static inline int check_timer_freq(void)
{
	int ret = 0;
	if (REVERSE_GEAR_TIMEMS == 0) {
		loge("REVERSE_GEAR_TIMEMS is not setting.\n");
		ret = 1;
	}
	return ret;
}

static inline int register_earlycam_timer(void)
{
	int ret = 0;
	// register timer irq handler
	earlycamera_timer = timer_register(REVERSE_GEAR_TIMEMS,
		  (interrupt_handler_t *)tcc_earlycamera_interrupt_handler,
		  NULL);

	if (earlycamera_timer == NULL) {
		timer_regiter_status = 0;
		ret = 1;
		loge("Failed on register tcc_timer\n");
	} else {
		timer_regiter_status = 1;
	}
	return ret;
}

#ifdef CONFIG_TCC805X
int tcc_earlycamera_initialize_irq(void)
{
	int ret = 0;

	logd("initialize irq is called\n");

	/* coverity[misra_c_2012_rule_13_5_violation : FALSE] */
	if ((check_timer_freq() == 0) && (register_earlycam_timer() == 0)) {
		if (timer_enable(earlycamera_timer) != 0) {
			/*
			 * Succeeded to register camera but failed to enable
			 * the timer
			 */
			logw("[WARN] Failed to enable timer.\n");
			ret = -1;
		}
	} else {
		ret = -1;
	}

	return ret;
}

void tcc_earlycamera_deinitialize_irq(void)
{
	if (timer_regiter_status == 1) {
		// disable timer
		(void)timer_disable(earlycamera_timer);

		// release timer irq handler
		timer_unregister(earlycamera_timer);
	}
}
#else
/* In TCC803x, the boot-loader running on main-core delegates a role of
 * camera-preview to sub-core by loading and running the core directly via the
 * boot loader command. This results in an initialization problem since both
 * timer irq from boot-loader in main-core and camera driver in sub-core try to
 * initialize VIOC simultaneously. So, we MUST NOT use timer interrupt in
 * TCC803x.
 */
int tcc_earlycamera_initialize_irq(void)
{
	return 0;
}
void tcc_earlycamera_deinitialize_irq(void)
{
}
#endif
