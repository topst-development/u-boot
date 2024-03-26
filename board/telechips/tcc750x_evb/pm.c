// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <mach/chipinfo.h>
#include <power/pmic.h>

#define BD96801_VOLTAGE_TUNE_STEP	10	// mV

/* If the BUCK_INI_VOUT register value is less than 0xc8, it is calculated as 5mv step */
#define BD96801_BUCK_INI_VOUT_5MV_STEP	0xC8

struct regcfg {
	u32 reg;
	u32 sel;
	u32 val;
};

static int pmic_regcfg(struct udevice *dev, const struct regcfg *cfg, s32 num)
{
	int ret = 0;

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
	} else {
		ret = -ENODEV;
	}

	return ret;
}

void pmic_configure(void)
{
	struct udevice *bd96801, *raa271000, *mpq70165fs, *mpq2286;
	int ret = 0;
	int core_v = 0;

	/* TCC750X_LPD4_4X321_V010 (ROHM PMIC) */
	struct regcfg bd96801_cfg[2] = {
		{ 0x04, 0xFF, 0x9D}, /* Release Write Protect				*/
		{ 0x2A, 0x1F, 0x00}, /* BUCK3_VOUT voltage tuning (0.9V) - CORE_OP9	*/
	};

	/* TCC750X_LPD4_4X321_PMIC_R_V010 (Renesas PMIC) */
	const struct regcfg raa271000_cfg[4] = {
		{ 0x70, 0xFF, 0xBE}, /* VOUT1 voltage tuning (0.8V -> 0.9V) - NPU	*/
		{ 0x71, 0xFF, 0x00},
		{ 0xa0, 0xFF, 0xBE}, /* VOUT4_voltage tuning (0.9V) - CORE_OP9		*/
		{ 0xa1, 0xFF, 0x00},
	};

	/* TCC750X_LPD4_4X321_PMIC_M_V010 (MPS PMIC) */
	const struct regcfg mpq2286_cfg[1] = {
		{ 0x21, 0xFF, 0x6d}, /*	VOUT voltage tuning (0.8V -> 0.9V) - NPU	*/
	};

	const struct regcfg mpq70165fs_cfg[2] = {
		{ 0x00, 0xFF, 0x00}, /* Selete VOUT1					*/
		{ 0x21, 0xFF, 0x6f}, /* VOUT1 voltage tuning (0.9V) - CORE_OP9		*/
	};

	(void)pmic_get("raa271000", &raa271000);
	ret = pmic_regcfg(raa271000, raa271000_cfg, 4);

	if (ret == -ENODEV) {
		(void)pmic_get("mpq70165fs", &mpq70165fs);
		ret = pmic_regcfg(mpq70165fs, mpq70165fs_cfg, 2);

		if (ret == 0) {
			(void)pmic_get("mpq2286", &mpq2286);
			ret = pmic_regcfg(mpq2286, mpq2286_cfg, 1);
		}
	}

	if (ret == -ENODEV) {
		(void)pmic_get("bd96801", &bd96801);
		if (bd96801 == NULL) {
			return;
		}

		core_v =  dm_i2c_reg_read(bd96801, 0x23);

		/* Set only if the core voltage is less than 1.5 V. */
		if (core_v < BD96801_BUCK_INI_VOUT_5MV_STEP) {
			/* current core voltage (mV) */
			core_v = (core_v * 5) + 500;

			/* Voltage Tuning
			ex) current core voltage = 750mV
			(900 - 750) / 10 = 15
			core = 0x0F -> +150mV */

			if (core_v != 900) {
				core_v = (900 - core_v) / BD96801_VOLTAGE_TUNE_STEP;
				bd96801_cfg[1].val = core_v;
			}

			ret = pmic_regcfg(bd96801, bd96801_cfg, 2);
		}
	}
}
