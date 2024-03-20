// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <config.h>
#include <common.h>
#include <lcd.h>
#include <linux/compat.h>
#include <asm/telechips/vioc/lvds_wrap.h>
#include <asm/telechips/vioc/lvds_phy.h>
#include <tcc_lvds_ctrl.h>
#include <asm/telechips/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>

static int fld0800_panel_init(void);
static int fld0800_set_power(int on);
static int fld0800_set_backlight_level(int level);

/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
struct vidinfo fld0800_panel_info = {
	.name				= "FLD0800",
	.manufacturer			= "innolux",
	.id				= PANEL_ID_FLD0800,
	.vl_col				= 1024,
	.vl_row				= 600,
	.vl_rot				= 0,
	.vl_width			= 153,
	.vl_height			= 90,
	.vl_bpix			= 5, /* bpp = (1 << vl_bpix) */
	.clk_freq			= 51200000,
	.clk_div      = 0, /* pclk = lclk / (2 * clk_div) */
	.bus_width			= 24,
	.lpw				= 19,
	.lpc				= 1024,
	.lswc				= 147,
	.lewc				= 147,
	.vdb				= 0,
	.vdf				= 0,
	.fpw1				= 2,
	.flc1				= 600,
	.fswc1				= 10,
	.fewc1				= 25,
	.fpw2				= 2,
	.flc2				= 600,
	.fswc2				= 10,
	.fewc2				= 25,
	.sync_invert			= IV_INVERT | IH_INVERT,
	.init				= fld0800_panel_init,
	.set_power			= fld0800_set_power,
	.set_backlight_level = fld0800_set_backlight_level,
};

/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
struct lvds_hw_info_t fld0800_lvds_info = {
	0, 0, 0, 0, 0, 0, 0, 0, 1200/* vcm */, 400/* vsw */,
	/* LVDS lane info*/
	{
		LVDS_PHY_DATA0_LANE, LVDS_PHY_DATA1_LANE, LVDS_PHY_DATA2_LANE,
		LVDS_PHY_CLK_LANE, LVDS_PHY_DATA3_LANE},
	{
		0, 0, 0, 0, 0},
	/* LVDS TXOUT info */
	{
		{
			TXOUT_G_D(0), TXOUT_R_D(5), TXOUT_R_D(4), TXOUT_R_D(3),
			TXOUT_R_D(2), TXOUT_R_D(1), TXOUT_R_D(0)},
		{
			TXOUT_B_D(1), TXOUT_B_D(0), TXOUT_G_D(5), TXOUT_G_D(4),
			TXOUT_G_D(3), TXOUT_G_D(2), TXOUT_G_D(1)},
		{
			TXOUT_DE, TXOUT_VS, TXOUT_HS, TXOUT_B_D(5),
			TXOUT_B_D(4), TXOUT_B_D(3), TXOUT_B_D(2)},
		{
			TXOUT_DUMMY, TXOUT_B_D(7), TXOUT_B_D(6), TXOUT_G_D(7),
			TXOUT_G_D(6), TXOUT_R_D(7), TXOUT_R_D(6)}
	},
};

static unsigned int fld0800_lvds_stbyb;
static int fld0800_panel_init(void)
{
	const struct lcd_platform_data *pdata = &fld0800_panel_info.dev;

	tcclcd_gpio_config(pdata->display_on, GPIO_OUTPUT | GPIO_FN0);
	tcclcd_gpio_config(pdata->bl_on, GPIO_OUTPUT | GPIO_FN0);
	tcclcd_gpio_config(pdata->reset, GPIO_OUTPUT | GPIO_FN0);
	tcclcd_gpio_config(pdata->power_on, GPIO_OUTPUT | GPIO_FN0);
	tcclcd_gpio_config(fld0800_lvds_stbyb, GPIO_OUTPUT | GPIO_FN0);

	tcclcd_gpio_set_value(pdata->display_on, 0);
	tcclcd_gpio_set_value(pdata->bl_on, 0);
	tcclcd_gpio_set_value(pdata->reset, 0);
	tcclcd_gpio_set_value(pdata->power_on, 0);
	tcclcd_gpio_set_value(fld0800_lvds_stbyb, 0);

	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("%s : %d\n", __func__, 0);

	return 0;
}

static int fld0800_set_power(int on)
{
	struct lcd_platform_data *pdata = &fld0800_panel_info.dev;
	int ret = 0;

	pdata->lcdc_hw =
		(void *)lvds_register_hw_info(
			&fld0800_lvds_info,
			pdata->platform_id, pdata->output_port1,
			pdata->output_port2, fld0800_panel_info.clk_freq,
			pdata->lcdc_select, pdata->lcdc_bypass,
			fld0800_panel_info.vl_col);

	if (pdata->lcdc_hw == NULL) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("%s : invalid lcdc_hw ptr. display[%u] not initialized.\n",
			__func__, pdata->lcdc_id);
		ret = -EINVAL;
	}

	if (ret == 0) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("%s : %d\n", __func__, on);
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("%s :lvds_main = %u, lvds_sub = %u ts_mux = %u\n",
			__func__,
			fld0800_lvds_info.port_main, fld0800_lvds_info.port_sub,
			fld0800_lvds_info.ts_mux_id);
		if (on != 0) {
			tcclcd_gpio_set_value(pdata->power_on, 1);
			udelay(20);

			tcclcd_gpio_set_value(fld0800_lvds_stbyb, 1);
			tcclcd_gpio_set_value(pdata->reset, 1);
			tcclcd_gpio_set_value(pdata->display_on, 1);

			lvds_splitter_init(&fld0800_lvds_info);

			(void)lcdc_mux_select(pdata->lcdc_select, pdata->lcdc_id);
			lcdc_initialize(pdata->lcdc_id, &fld0800_panel_info);

			lvds_phy_init(&fld0800_lvds_info);

		} else {
			/*
			 * LVDS Port0 and LVDS Port1 is not controlled separately,
			 * LVDS power off will work only for LVDS port2
			 */
			LVDS_WRAP_ResetPHY(fld0800_lvds_info.ts_mux_id, 1);
			lcdc_deinitialize(pdata->lcdc_id);
			tcclcd_gpio_set_value(pdata->power_on, 0);
		}
	}

	return ret;
}

static int fld0800_set_backlight_level(int level)
{
	const struct lcd_platform_data *pdata = &fld0800_panel_info.dev;

	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("%s : %d\n", __func__, level);

	if (level == 0) {
		tcclcd_gpio_set_value(pdata->bl_on, 0);
	} else {
		tcclcd_gpio_set_value(pdata->bl_on, 1);
	}
	return 0;
}

void fld0800_get_panel_info(struct vidinfo *vid)
{
	//pr_info("%s\n", __func__);
	(void)memcpy(vid, &fld0800_panel_info, sizeof(struct vidinfo));
}
EXPORT_SYMBOL(fld0800_get_panel_info);

void fld0800_set_panel_info(const struct vidinfo *vid)
{
	//pr_info("%s\n", __func__);
	(void)memcpy(&fld0800_panel_info, vid, sizeof(struct vidinfo));
}
EXPORT_SYMBOL(fld0800_set_panel_info);
