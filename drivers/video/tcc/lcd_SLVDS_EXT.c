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

static int slvds_ext_panel_init(void);
static int slvds_ext_set_power(int on);
static int slvds_ext_set_backlight_level(int level);

struct vidinfo slvds_ext_panel_info = {
	.name				= "SLVDS_EXT",
	.manufacturer			= "innolux",
	.id				= PANEL_ID_SLVDS_EXT,
	.vl_col				= 1024,
	.vl_row				= 600,
	.vl_rot				= 0,
	.vl_width			= 153,
	.vl_height			= 90,
	.vl_bpix			= 5, /* bpp = (1 << vl_bpix) */
	.clk_freq			= 51200000,
	.clk_div			= 0, /* pclk = lclk / (2 * clk_div) */
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
	.init				= slvds_ext_panel_init,
	.set_power			= slvds_ext_set_power,
	.set_backlight_level = slvds_ext_set_backlight_level,
};

struct lvds_hw_info_t slvds_ext_lvds_info = {
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

static unsigned int lvds_stbyb;
static int slvds_ext_panel_init(void)
{
	struct lcd_platform_data *pdata = &slvds_ext_panel_info.dev;

	tcclcd_gpio_config(pdata->display_on, GPIO_OUTPUT | GPIO_FN0);
	tcclcd_gpio_config(pdata->bl_on, GPIO_OUTPUT | GPIO_FN0);
	tcclcd_gpio_config(pdata->reset, GPIO_OUTPUT | GPIO_FN0);
	tcclcd_gpio_config(pdata->power_on, GPIO_OUTPUT | GPIO_FN0);
	tcclcd_gpio_config(lvds_stbyb, GPIO_OUTPUT | GPIO_FN0);

	tcclcd_gpio_set_value(pdata->display_on, 0);
	tcclcd_gpio_set_value(pdata->bl_on, 0);
	tcclcd_gpio_set_value(pdata->reset, 0);
	tcclcd_gpio_set_value(pdata->power_on, 0);
	tcclcd_gpio_set_value(lvds_stbyb, 0);

	pr_info("%s : %d\n", __func__, 0);

	return 0;
}

static int slvds_ext_set_power(int on)
{
	struct lcd_platform_data *pdata = &slvds_ext_panel_info.dev;

	pdata->lcdc_hw = (void *)lvds_register_hw_info(
		&slvds_ext_lvds_info, pdata->platform_id, pdata->output_port1,
		pdata->output_port2, slvds_ext_panel_info.clk_freq,
		pdata->lcdc_select, pdata->lcdc_bypass,
		slvds_ext_panel_info.vl_col);

	if (!pdata->lcdc_hw) {
		pr_err(
			"%s : invalid lcdc_hw ptr. display[%d] not initialized.\n",
			__func__, pdata->lcdc_id);
		return -1;
	}

	pr_info("%s : %d\n", __func__, on);
	pr_info(
		"%s :lvds_main = %d, lvds_sub = %d ts_mux = %d\n", __func__,
		slvds_ext_lvds_info.port_main, slvds_ext_lvds_info.port_sub,
		slvds_ext_lvds_info.ts_mux_id);
	if (on) {
		tcclcd_gpio_set_value(pdata->power_on, 1);
		udelay(20);

		tcclcd_gpio_set_value(lvds_stbyb, 1);
		tcclcd_gpio_set_value(pdata->reset, 1);
		tcclcd_gpio_set_value(pdata->display_on, 1);

		lvds_splitter_init(&slvds_ext_lvds_info);

		lcdc_mux_select(pdata->lcdc_select, pdata->lcdc_id);
		lcdc_initialize(pdata->lcdc_id, &slvds_ext_panel_info);

		lvds_phy_init(&slvds_ext_lvds_info);

	} else {
		/* LVDS Port0 and LVDS Port1 are controlled at once */
		LVDS_WRAP_ResetPHY(slvds_ext_lvds_info.ts_mux_id, 1);
		lcdc_deinitialize(pdata->lcdc_id);
		tcclcd_gpio_set_value(pdata->power_on, 0);
	}

	return 0;
}

static int slvds_ext_set_backlight_level(int level)
{
	struct lcd_platform_data *pdata = &slvds_ext_panel_info.dev;

	pr_info("%s : %d\n", __func__, level);

	if (level == 0)
		tcclcd_gpio_set_value(pdata->bl_on, 0);
	else
		tcclcd_gpio_set_value(pdata->bl_on, 1);
	return 0;
}

void slvds_ext_get_panel_info(struct vidinfo *vid)
{
	//pr_info("%s\n", __func__);
	memcpy(vid, &slvds_ext_panel_info, sizeof(struct vidinfo));
}
EXPORT_SYMBOL(slvds_ext_get_panel_info);

void slvds_ext_set_panel_info(struct vidinfo *vid)
{
	//pr_info("%s\n", __func__);
	memcpy(&slvds_ext_panel_info, vid, sizeof(struct vidinfo));
}
EXPORT_SYMBOL(slvds_ext_set_panel_info);
