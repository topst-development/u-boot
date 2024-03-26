// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <command.h>
#include <common.h>
#include <dm.h>
#include <pwm.h>
#include <asm/io.h>
#include <linux/delay.h>

static int do_pwm_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
static int tcc_pwm_atoi(const char *s);
static int notrace pwm_test(struct udevice *dev);

static int tcc_pwm_atoi(const char *s)
{
	int ret;
	unsigned int mul;
	unsigned int ch_cnt = 0;

	if (s == NULL) {
		ret = -1;
	} else {
		while (s[ch_cnt] != 0x0) { //count string length
			ch_cnt++;
		}
		mul = 1;
		ret = 0;
		while (ch_cnt != 0) {
			if (s[ch_cnt - 1] < '0' || s[ch_cnt - 1] > '9') {
				ret = -1;
				break;
			}
			ret += (s[ch_cnt - 1] - '0') * mul;
			ch_cnt--;
			mul *= 10;
		}
	}
	return ret;
}

static int notrace pwm_test(struct udevice *dev)
{

	pwm_set_config(dev, 0, 1000000, 0);
	pwm_set_enable(dev, 0, true);
	mdelay(5000);
	pwm_set_enable(dev, 0, false);

	pwm_set_config(dev, 0, 1000000, 250000);
	pwm_set_enable(dev, 0, true);
	mdelay(5000);
	pwm_set_enable(dev, 0, false);

	pwm_set_config(dev, 0, 1000000, 500000);
	pwm_set_enable(dev, 0, true);
	mdelay(5000);
	pwm_set_enable(dev, 0, false);

	pwm_set_config(dev, 0, 1000000, 750000);
	pwm_set_enable(dev, 0, true);
	mdelay(5000);
	pwm_set_enable(dev, 0, false);

	pwm_set_config(dev, 0, 1000000, 1000000);
	pwm_set_enable(dev, 0, true);
	mdelay(5000);
	pwm_set_enable(dev, 0, false);

	return 0;
}

static int do_pwm_cmd(struct cmd_tbl *cmdtp, int flag, int argc,
		char *const argv[])
{
	struct udevice *dev;
	const char *str_cmd;
	int pwm_id;
	unsigned int duty_ns;
	unsigned int period_ns;
	int ret;

	ret = uclass_get_device(UCLASS_PWM, 0, &dev);
	if (ret < 0) {
		pr_err("[Error][PWM] %s faile to probe pwm driver\n", __func__);
		return ret;
	}

	if (argc < 3)
		return CMD_RET_USAGE;

	str_cmd = argv[1];
	argc -= 2;
	argv += 2;

	pwm_id = tcc_pwm_atoi(argv[0]);

	if (!strcmp(str_cmd, "config")) {
		if (argc != 3)
			return CMD_RET_USAGE;

		duty_ns = tcc_pwm_atoi(argv[1]);
		period_ns = tcc_pwm_atoi(argv[2]);

		pwm_set_config(dev, pwm_id, period_ns, duty_ns);

	} else if (!strcmp(str_cmd, "enable")) {
		pwm_set_enable(dev, pwm_id, 1);
	} else if (!strcmp(str_cmd, "disable")) {
		pwm_set_enable(dev, pwm_id, 0);
	} else if (!strcmp(str_cmd, "test")) {
		pwm_test(dev);
	} else {
		return CMD_RET_USAGE;
	}

	return 0;

}

U_BOOT_CMD(pwm, 5, 0, do_pwm_cmd,
		"Verify PWM output",
		"config id duty_ns period_ns\n"
		"pwm enable id\n"
		"pwm disable id\n"
		);
