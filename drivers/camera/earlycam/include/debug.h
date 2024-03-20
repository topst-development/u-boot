// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef CAM_DEBUG_H
#define CAM_DEBUG_H

#include <common.h>

/* debug levels */
#define CRITICAL		0
#define ALWAYS			0
#define INFO			1
#define SPEW			2

#define LOG_TAG			"RVC"
#define DEBUGLEVEL		CRITICAL//INFO//

/* coverity[cert_arr39_c_violation : FALSE] */
/* coverity[misra_c_2012_rule_20_10_violation : FALSE] */
#define loge(fmt, ...)		{ (void)pr_err("[ERROR][%s] %s - " fmt, LOG_TAG, \
					__func__, ##__VA_ARGS__); }
/* coverity[cert_arr39_c_violation : FALSE] */
/* coverity[misra_c_2012_rule_20_10_violation : FALSE] */
#define logw(fmt, ...)		{ (void)pr_warn("[WARN][%s] %s - " fmt, LOG_TAG, \
					__func__, ##__VA_ARGS__); }
/* coverity[cert_arr39_c_violation : FALSE] */
/* coverity[misra_c_2012_rule_20_10_violation : FALSE] */
#define logd(fmt, ...)		{ (void)pr_debug("[DEBUG][%s] %s - " fmt, LOG_TAG, \
					__func__, ##__VA_ARGS__); }
/* coverity[cert_arr39_c_violation : FALSE] */
/* coverity[misra_c_2012_rule_20_10_violation : FALSE] */
#define logi(fmt, ...)		{ (void)pr_info("[INFO][%s] %s - " fmt, LOG_TAG, \
					__func__, ##__VA_ARGS__); }

#endif//EARLYCAM_DEBUG_H__
