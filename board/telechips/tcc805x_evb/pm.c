// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <mach/chipinfo.h>
#include <power/pmic.h>

struct regcfg {
	u32 reg;
	u32 sel;
	u32 val;
};

static void pmic_regcfg(struct udevice *dev, const struct regcfg *cfg, s32 num)
{
	if (dev != NULL) {
		s32 i;

		for (i = 0; i < num; i++) {
			const struct regcfg *c = &cfg[i];
			s32 ret;

			ret = pmic_clrsetbits(dev, c->reg, c->sel, c->val);
			if (ret != 0) {
				pr_err("Failed to config %s (err: %d)\n",
				       dev->name, ret);
				break;
			}
		}
	}
}

void pmic_configure(void)
{
	struct udevice *da9062;
	struct udevice *da9131_d0;
	struct udevice *da9131_d2;
	u32 chip_name, reg_value;
	s32 ret;

	const struct regcfg da9062_cfg[3] = {
		{ 0x97, 0xff, 0x96 }, /* WAIT         */
		{ 0x92, 0xff, 0x03 }, /* ID_32_31     */
		{ 0x0c, 0x10, 0x00 }, /* IRQ_MASK_C   */
	};

	const struct regcfg da9131_cfg[6] = {
		{ 0x20, 0xff, 0x49 }, /* BUCK_BUCK1_0 */
		{ 0x21, 0xff, 0x49 }, /* BUCK_BUCK1_1 */
		{ 0x28, 0xff, 0x49 }, /* BUCK_BUCK2_0 */
		{ 0x29, 0xff, 0x49 }, /* BUCK_BUCK2_1 */
		{ 0x13, 0xff, 0x08 }, /* SYS_GPIO_1_1 */
		{ 0x15, 0xff, 0x08 }, /* SYS_GPIO_2_1 */
	};

	const struct regcfg da9131_sel_cfg[1] = {
		{ 0x24, 0x10, 0x00},
	};

	chip_name = get_chip_name();

	/*
	 * Some EVBs may not have da9062 and/or da9131 depending on which
	 * PMIC sub-board it is using.
	 *
	 * Ignore return value for such environment.
	 */
	(void)pmic_get("da9062", &da9062);
	(void)pmic_get("da9131_d0", &da9131_d0);
	(void)pmic_get("da9131_d2", &da9131_d2);

	/*
	 * S/W Workaround for GB_0P8 spec-out issue (for TCC8050/3)
	 * - Adjust GB_0P8 voltage from 0.8V to 0.83V
	 */
	if ((da9131_d0 != NULL) && (chip_name != 0x8059U)) {
		ret = pmic_reg_write(da9131_d0, 0x2dU, 0x53U);
		if (ret != 0) {
			pr_err("Failed to change GP_0P8 to 0.83V (err: %d)\n",
			       ret);
		}
	}

	/* DA9131_D2 regulator BUCK 1 configuration based on channel A */
	if (da9131_d2 != NULL) {
		ret = pmic_reg_read(da9131_d2, 0x26U);
		if (ret > 0) {
			reg_value = (u32)ret;
			/* Copy the voltage settings of channel B to channel A */
			ret = pmic_reg_write(da9131_d2, 0x25U, reg_value);
			if (ret == 0) {
				/* Select channel A */
				pmic_regcfg(da9131_d2,
					da9131_sel_cfg,
					ARRAY_SIZE(da9131_sel_cfg));
			} else {
				pr_err("Failed to set da9131_d2 channel A voltage\n");
			}
		} else {
			pr_err("Failed to get da9131_d2 channel B voltage\n");
		}
	}

	/*
	 * S/W workaround for power sequence issue (for DA9062)
	 * - Delay CORE_0P8 power off timing for 16.4 msec
	 * - Unmask SYS_EN IRQ to get wake-up signal
	 */
	pmic_regcfg(da9062, da9062_cfg, ARRAY_SIZE(da9062_cfg));

	/*
	 * S/W workaround for power sequence issue (for DA9131 OTP-42/43 v1)
	 * - Adjust voltage slew rate correctly
	 * - Enable GPIO1/2 pull-up/pull-down
	 */
	pmic_regcfg(da9131_d0, da9131_cfg, ARRAY_SIZE(da9131_cfg));
	pmic_regcfg(da9131_d2, da9131_cfg, ARRAY_SIZE(da9131_cfg));
}
