// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef _PWM_H_
#define _PWM_H_

extern void pwm_enable(int channel);
extern void pwm_disable(int channel);
extern int pwm_config(unsigned int channel,
		unsigned int duty_ns, unsigned int period_ns);
extern void init_pwm(void);
extern void deinit_pwm(void);
#endif
