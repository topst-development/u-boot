// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <config.h>
#include <common.h>
#include <lcd.h>
#include <linux/compat.h>

#include <linux/io.h>
#include <asm/arch/gpio.h>
#include <asm/telechips/gpio.h>
//#include <asm/telechips/vioc/vioc_pxdemux.h>
#include <asm/telechips/vioc/lvds_wrap.h>
#include <asm/telechips/vioc/lvds_phy.h>
#include <tcc_lvds_ctrl.h>

/* LVDS PHY - Dual output mode
 * PXDEMUX5x1_0/1 -> PXDEMUX_Core ->LVDS PHY Dual Port 2/3
 * LVDS PHY - Single output mode
 * PXDEMUX5x1_2 -> LVDS PHY Single Port 0
 */

#ifdef CONFIG_TCC_TI_SERDES_SUPPORT
#include <i2c.h>
#include <dm.h>

#define LVDS_TI_I2C_PORT	0

#define TI_SER_ADDR		0x0C
#define TI_DES_ADDR		0x2C

#define SER_RESET
#define DES_RESET
//#define SER_PATERN_GEN

struct SER_REG {
	unsigned int devaddr;
	unsigned int regaddr;
	unsigned int value;
};

struct SER_REG stserdes[] = {
	/* DEVADDR, Register Addr, Value */
#ifdef SER_RESET
	{TI_SER_ADDR, 0x01, 0x02}, //Reset
	{0xFF, 0xFF, 0xFF}, //delay
#endif
	{TI_SER_ADDR, 0x03, 0xDA}, //SER_GENERAL_CONFIGURATION I2C Pass-through
	{TI_SER_ADDR, 0x04, 0x90},
	//DE_GATE_RGB enabled 4-bit on: default value 0x80 : Humax setting
	{TI_SER_ADDR, 0x0E, 0x33}, //GPIO1_GPIO2_SER setting for Forward channel
	{TI_SER_ADDR, 0x0F, 0x03}, //GPIO3_SER setting for Forward channel
#ifdef SER_PATERN_GEN
	{TI_SER_ADDR, 0x64, 0x15}, //Patten Generator Control default value 0x10
#endif
#ifdef DES_RESET
	{TI_DES_ADDR, 0x01, 0x02}, //Deserializer register reset
	{0xFF, 0xFF, 0xFF}, //delay
#endif
	{TI_DES_ADDR, 0x1E, 0x55}, // GPIO1_GPIO2_DES for Forward channel
	{TI_DES_ADDR, 0x1F, 0x05}, // GPIO3_DES for Forward channel
	{0, 0, 0}
};
#endif

int tm123xdhp90_panel_init(void);
static int tm123xdhp90_set_power(int on);
static int tm123xdhp90_set_backlight_level(int level);

/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
struct vidinfo tm123xdhp90_panel_info = {
	.name = "TM123XDHP90",
	.manufacturer = "tianma",
	.id = PANEL_ID_TM123XDHP90,
	.vl_col = 1920,
	.vl_row = 720,
	.vl_rot = 0,
	.vl_width = 311,
	.vl_height = 130,
	.vl_bpix = 5, /* bpp = (1 << vl_bpix) */
	.clk_freq = 88200000,
	.clk_div = 0, /* pclk = lclk / (2 * clk_div) */
	.bus_width = 24,
	.lpw = 8,
	.lpc = 1920,
	.lswc = 28,
	.lewc = 28,
	.vdb = 0,
	.vdf = 0,

	.fpw1 = 2,
	.flc1 = 720,
	.fswc1 = 10,
	.fewc1 = 10,
	.fpw2 = 2,
	.flc2 = 720,
	.fswc2 = 10,
	.fewc2 = 10,
	.sync_invert = IV_INVERT | IH_INVERT,
	.init = tm123xdhp90_panel_init,
	.set_power = tm123xdhp90_set_power,
	.set_backlight_level = tm123xdhp90_set_backlight_level,
};

/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
struct lvds_hw_info_t tm123xdhp90_lvds_info = {
	0, 0, 0, 0, 0, 0, 0, 0, 1200 /* vcm */, 500 /* vsw */,
	/* LVDS lane info*/
	{LVDS_PHY_DATA3_LANE, LVDS_PHY_CLK_LANE, LVDS_PHY_DATA2_LANE,
	 LVDS_PHY_DATA1_LANE, LVDS_PHY_DATA0_LANE},
	{LVDS_PHY_DATA0_LANE, LVDS_PHY_DATA1_LANE, LVDS_PHY_DATA2_LANE,
	 LVDS_PHY_CLK_LANE, LVDS_PHY_DATA3_LANE},
	/* LVDS TXOUT info */
	{
		{TXOUT_G_D(0), TXOUT_R_D(5), TXOUT_R_D(4),
			TXOUT_R_D(3), TXOUT_R_D(2), TXOUT_R_D(1), TXOUT_R_D(0)},
		{TXOUT_B_D(1), TXOUT_B_D(0), TXOUT_G_D(5),
			TXOUT_G_D(4), TXOUT_G_D(3), TXOUT_G_D(2), TXOUT_G_D(1)},
		{TXOUT_DE, TXOUT_VS, TXOUT_HS,
			TXOUT_B_D(5), TXOUT_B_D(4), TXOUT_B_D(3), TXOUT_B_D(2)},
		{TXOUT_DUMMY, TXOUT_B_D(7), TXOUT_B_D(6),
			TXOUT_G_D(7), TXOUT_G_D(6), TXOUT_R_D(7), TXOUT_R_D(6)}
	},
	{
		{TXOUT_G_D(0), TXOUT_R_D(5), TXOUT_R_D(4),
			TXOUT_R_D(3), TXOUT_R_D(2), TXOUT_R_D(1), TXOUT_R_D(0)},
		{TXOUT_B_D(1), TXOUT_B_D(0), TXOUT_G_D(5),
			TXOUT_G_D(4), TXOUT_G_D(3), TXOUT_G_D(2), TXOUT_G_D(1)},
		{TXOUT_DE, TXOUT_DUMMY, TXOUT_DUMMY,
			TXOUT_B_D(5), TXOUT_B_D(4), TXOUT_B_D(3), TXOUT_B_D(2)},
		{TXOUT_DUMMY, TXOUT_B_D(7), TXOUT_B_D(6),
			TXOUT_G_D(7), TXOUT_G_D(6), TXOUT_R_D(7), TXOUT_R_D(6)}
	}
};

#ifdef CONFIG_TCC_TI_SERDES_SUPPORT
static void tcc_lvds_set_i2c(void)
{
	int i2c_port, loop, retry, index, ret, data, i;

	struct udevice *i2c_dev;
	struct udevice *bus_dev = NULL;

	i2c_port = LVDS_TI_I2C_PORT;

	if (i2c_port < 0) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[ERR][I2C] not valid Port\n");
		return;
	}
	ret = uclass_get_device_by_seq(UCLASS_I2C, i2c_port, &bus_dev);
	if (!ret) {
		ret = dm_i2c_set_bus_speed(bus_dev, 100000);
		if (ret) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err(
				"\n[%s:%d]Error: from dm_i2c_set_bus_speed().. Ret(0x%x)\n",
				__func__, __LINE__, ret);
		}
	} else {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err(
			"\n[%s:%d]Error: can't get i2c bus %d\n",
			__func__, __LINE__, i2c_port);
		return;
	}

	for (loop = 0; !(stserdes[loop].devaddr == 0 &&
			stserdes[loop].regaddr == 0 &&
			stserdes[loop].value == 0); loop++) {
		if (stserdes[loop].devaddr == 0xFF &&
			stserdes[loop].regaddr == 0xFF &&
			stserdes[loop].value == 0xFF){
			mdelay(100);
			continue;
		}
		for (retry = 3; retry >= 0 ; retry--) {
			ret = i2c_get_chip_for_busnum(i2c_port,
						      stserdes[loop].devaddr,
						      1,
						      &i2c_dev);
			if (ret) {
				if (retry == 0) {
					/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
					/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
					/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
					pr_err("[%d] Failed to get i2c handle for addr 0x%x\n",
						__LINE__,
						stserdes[loop].devaddr);
					return;
				}
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				pr_err("[%d] Getting i2c handle retry left %d\n",
				       __LINE__, retry);
				mdelay(10);
			} else {
				break;
			}
		}

		for (retry = 3; retry >= 0; retry--) {
			dm_i2c_write(i2c_dev, stserdes[loop].regaddr,
				&stserdes[loop].value, 1);
			mdelay(10);
			if (stserdes[loop].regaddr == 0x1)
				continue;
			dm_i2c_read(i2c_dev, stserdes[loop].regaddr, &data, 1);
			if (stserdes[loop].value != data) {
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				pr_err("I2C Write WARN :\n   I2C DEV : 0x%x\nI2C REG : 0x%x\n   Value : 0x%x\n",
					stserdes[loop].devaddr,
					stserdes[loop].regaddr,
					stserdes[loop].value);
				/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
				pr_err("   Read Back : 0x%x\nRetry...\n",
				       data);
			} else {
				break;
			}
		}
	}
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("[%d][%s] Done\n", __LINE__, __func__);
}
#endif

static unsigned int tm123xdhp90_lvds_stbyb = GPIO_NC;
int tm123xdhp90_panel_init(void)
{
	const struct lcd_platform_data *pdata = &(tm123xdhp90_panel_info.dev);

	tcclcd_gpio_config(pdata->display_on, GPIO_OUTPUT | GPIO_FN0);
	tcclcd_gpio_config(pdata->bl_on, GPIO_OUTPUT | GPIO_FN0);
	tcclcd_gpio_config(pdata->reset, GPIO_OUTPUT | GPIO_FN0);
	tcclcd_gpio_config(pdata->power_on, GPIO_OUTPUT | GPIO_FN0);
	tcclcd_gpio_config(tm123xdhp90_lvds_stbyb, GPIO_OUTPUT | GPIO_FN0);

	tcclcd_gpio_set_value(pdata->display_on, 0);
	tcclcd_gpio_set_value(pdata->bl_on, 0);
	tcclcd_gpio_set_value(pdata->reset, 0);
	tcclcd_gpio_set_value(pdata->power_on, 0);
	tcclcd_gpio_set_value(tm123xdhp90_lvds_stbyb, 0);

	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("%s : %d\n", __func__, 0);

	return 0;
}

static int tm123xdhp90_set_power(int on)
{
	struct lcd_platform_data *pdata = &(tm123xdhp90_panel_info.dev);
	int ret = 0;

	pdata->lcdc_hw = (void *)lvds_register_hw_info(
		&tm123xdhp90_lvds_info, pdata->platform_id, pdata->output_port1,
		pdata->output_port2, tm123xdhp90_panel_info.clk_freq,
		pdata->lcdc_select, pdata->lcdc_bypass,
		tm123xdhp90_panel_info.vl_col);

	if (pdata->lcdc_hw == NULL) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("%s : invalid lcdc_hw ptr\n", __func__);
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
		pr_info("%s :lvds_main = %u, lvds_sub = %u ts_mux = %u\n", __func__,
			tm123xdhp90_lvds_info.port_main, tm123xdhp90_lvds_info.port_sub,
			tm123xdhp90_lvds_info.ts_mux_id);
		if (on != 0) {
			tcclcd_gpio_set_value(pdata->power_on, 1);
			udelay(20);

			tcclcd_gpio_set_value(pdata->reset, 1);
			tcclcd_gpio_set_value(pdata->display_on, 1);
			tcclcd_gpio_set_value(tm123xdhp90_lvds_stbyb, 1);

			lvds_splitter_init(&tm123xdhp90_lvds_info);

			(void)lcdc_mux_select(pdata->lcdc_select, pdata->lcdc_id);
			lcdc_initialize(pdata->lcdc_id, &tm123xdhp90_panel_info);

			lvds_phy_init(&tm123xdhp90_lvds_info);
#ifdef CONFIG_TCC_TI_SERDES_SUPPORT
			tcc_lvds_set_i2c();
#endif
		} else {
			/*Dual port SW reset*/
			LVDS_WRAP_ResetPHY(TS_MUX_IDX0, 1); // Dual port control
			lcdc_deinitialize(pdata->lcdc_id);
			tcclcd_gpio_set_value(pdata->power_on, 0);
		}
	}

	return ret;
}

static int tm123xdhp90_set_backlight_level(int level)
{
	const struct lcd_platform_data *pdata = &(tm123xdhp90_panel_info.dev);

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

void tm123xdhp90_get_panel_info(struct vidinfo *vid)
{
	// pr_info("%s\n", __func__);
	(void)memcpy(vid, &tm123xdhp90_panel_info, sizeof(struct vidinfo));
}
EXPORT_SYMBOL(tm123xdhp90_get_panel_info);

void tm123xdhp90_set_panel_info(const struct vidinfo *vid)
{
	// pr_info("%s\n", __func__);
	(void)memcpy(&tm123xdhp90_panel_info, vid, sizeof(struct vidinfo));
}
EXPORT_SYMBOL(tm123xdhp90_set_panel_info);
