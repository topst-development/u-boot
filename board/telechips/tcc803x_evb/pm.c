// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <mach/chipinfo.h>
#include <power/pmic.h>

void pmic_configure(void)
{
	struct udevice *da9062;
	u32 chip_code;
	s32 ret;

	chip_code = get_chip_code();

	/*
	 * S/W Workaround for TCC803xPE CPU stability issue
	 * - Adjust VBUCK2_A voltage from 0.9V to 0.94V
	 */
	if ((chip_code & PIN2PIN) != 0U) {
		ret = pmic_get("da9062", &da9062);

		if (ret == 0) {
			ret = pmic_reg_write(da9062, 0xA3U, 0x40U);
		}

		if (ret != 0) {
			pr_err("Failed to change VBUCK2_A to 0.94V (err: %d)\n",
			       ret);
		}
	}
}
