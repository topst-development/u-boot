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
	u32 mc_cpu_uv = 0, sc_cpu_uv = 0, npu_uv = 0;

	struct udevice *mpq2977 = NULL;
	struct udevice *mpq7932 = NULL;
	struct udevice *mpq2286 = NULL;

	/* Maincore CPU Voltage Setting */
	/* Rail1 (VBUCK) voltage tuning */
	u8 mpq2977_data[2] = {0x00, 0x00};

	/* Subcore CPU Voltage Setting */
	struct regcfg mpq7932_cfg[3] = {
		{0x10, 0xff, 0x20}, /* Release Write Protect */
		{0x00, 0xff, 0x03}, /* Page 3(VOUT4) Select  */
		{0x21, 0xff, 0x00}, /* VOUT4 voltage tuning  */
	};

	/* NPU Voltage Setting */
	struct regcfg mpq2286_cfg[1] = {
		{ 0x21, 0xFF, 0x00}, /* VOUT voltage tuning */
	};

	(void)pmic_get("mpq2977", &mpq2977);
	(void)pmic_get("mpq7932", &mpq7932);
	(void)pmic_get("mpq2286", &mpq2286);

	if (mpq2977 != NULL) {
		mc_cpu_uv = dev_read_u32_default(mpq2977, "mc-cpu-mv", 0x00) * 1000;
	}

	if (mpq7932 != NULL) {
		sc_cpu_uv = dev_read_u32_default(mpq7932, "sc-cpu-mv", 0x00) * 1000;
	}

	if (mpq2286 != NULL) {
		npu_uv    = dev_read_u32_default(mpq2286, "npu-mv",    0x00) * 1000;
	}

	if (mc_cpu_uv != 0) {
		/* (reg_val * 0.0025 = Out_mV) */
		mpq2977_data[0] = ((u32)(mc_cpu_uv / 2500) >> 0) & 0xff;
		mpq2977_data[1] = ((u32)(mc_cpu_uv / 2500) >> 8) & 0xff;
		pmic_write(mpq2977, 0x21, mpq2977_data, 2);

		pr_info("Maincore cpu voltage set to %d mv\n", mc_cpu_uv);
	}

	if (sc_cpu_uv != 0) {
		/* (reg_val * 6.25 + 206.25) * 1 = Out_mV */
		mpq7932_cfg[2].val = (u8)((sc_cpu_uv - 206250) / 6250);
		pmic_regcfg(mpq7932, mpq7932_cfg, 3);

		pr_info("Subcore cpu voltage set to %d mv\n", sc_cpu_uv);
	}

	if (npu_uv != 0) {
		/* (reg_val * 6.25 + 206.25) * 1 = Out_mV */
		mpq2286_cfg[0].val = (u8)((npu_uv - 206250) / 6250);
		pmic_regcfg(mpq2286, mpq2286_cfg, 1);

		pr_info("NPU voltage set to %d mv\n", npu_uv);
	}
}
