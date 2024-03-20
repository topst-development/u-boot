// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/serial.h>
#include <asm/gpio.h>
#include <asm/telechips/gpio.h>
#include <asm/mach-types.h>
#include <power/pmic.h>

#include <lcd.h>

DECLARE_GLOBAL_DATA_PTR;
struct lcd_platform_data power_pins[PANEL_LCD_POWER_MAX];

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)

#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC0)
const unsigned char aucDC0_LogoData[] = {
#include DC0_TCCLOGO_HEADER
};
#endif

#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC1)
const unsigned char aucDC1_LogoData[] = {
#include DC1_TCCLOGO_HEADER
};
#endif

#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC2)
const unsigned char aucDC2_LogoData[] = {
#include DC2_TCCLOGO_HEADER
};
#endif

#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC3)
const unsigned char aucDC3_LogoData[] = {
#include DC3_TCCLOGO_HEADER
};
#endif

#else
const unsigned char logo_data[] = {
#include TCCLOGO_HEADER
};
#endif /* #if defined( CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH ) */

void init_panel_info(struct vidinfo *vid)
{
	vid->dev.power_on = power_pins[vid->dev.lcd_power].power_on;
	vid->dev.display_on = power_pins[vid->dev.lcd_power].display_on;
	vid->dev.bl_on = power_pins[vid->dev.lcd_power].bl_on;
	vid->dev.reset = power_pins[vid->dev.lcd_power].reset;

#ifdef CONFIG_BOOT_LOGO_LEGACY

#if defined(CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH)

	switch (vid->dev.uiDisplayPathNum) {
	case DISPLAY_CONTROLLER_0:
#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC0)
		vid->dev.logo_data = (unsigned long)aucDC0_LogoData;
#endif
		break;
	case DISPLAY_CONTROLLER_1:
#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC1)
		vid->dev.logo_data = (unsigned long)aucDC1_LogoData;
#endif
		break;
	case DISPLAY_CONTROLLER_2:
#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC2)
		vid->dev.logo_data = (unsigned long)aucDC2_LogoData;
#endif
		break;
	case DISPLAY_CONTROLLER_3:
	default:
#if defined(CONFIG_DISPLAY_OUTPUT_PATH_DC3)
		vid->dev.logo_data = (unsigned long)aucDC3_LogoData;
#endif
		break;
	}
#else
	vid->dev.logo_data = logo_data;
#endif /* #if defined( CONFIG_LCD_INDIVIDUAL_BOOTLOGO_PER_PATH ) */

#elif CONFIG_BOOT_LOGO_SPLASH
	vid->dev.logo_data = NULL;
#endif
}

void init_lcd_power(unsigned int board_type)
{
	pr_info("[%s] : board type = [%s]\n", __func__,
		(board_type == 0) ? "tcc8059" : "tcc8050");
	switch (board_type) {
	case 0: /*tcc8059*/
		power_pins[PANEL_LCD_POWER0].power_on = TCC_GPB(17);
		power_pins[PANEL_LCD_POWER0].display_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER0].bl_on = TCC_GPMA(26);
		power_pins[PANEL_LCD_POWER0].reset = TCC_GPB(18);

		power_pins[PANEL_LCD_POWER1].power_on = TCC_GPC(8);
		power_pins[PANEL_LCD_POWER1].display_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER1].bl_on = TCC_GPMA(27);
		power_pins[PANEL_LCD_POWER1].reset = TCC_GPC(9);

		power_pins[PANEL_LCD_POWER2].power_on = TCC_GPSD1(10);
		power_pins[PANEL_LCD_POWER2].display_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER2].bl_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER2].reset = GPIO_NC;

		power_pins[PANEL_LCD_POWER3].power_on =
			GPIO_NC; /* no power port for TCC8059 EVB*/
		power_pins[PANEL_LCD_POWER3].display_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER3].bl_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER3].reset = GPIO_NC;

		power_pins[PANEL_LCD_NC].power_on = GPIO_NC;
		power_pins[PANEL_LCD_NC].display_on = GPIO_NC;
		power_pins[PANEL_LCD_NC].bl_on = GPIO_NC;
		power_pins[PANEL_LCD_NC].reset = GPIO_NC;
		break;
	case 1: /*tcc8050*/
		power_pins[PANEL_LCD_POWER0].power_on = TCC_GPMC(19);
		power_pins[PANEL_LCD_POWER0].display_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER0].bl_on = TCC_GPH(6);
		power_pins[PANEL_LCD_POWER0].reset = TCC_GPMC(20);

		power_pins[PANEL_LCD_POWER1].power_on = TCC_GPC(8);
		power_pins[PANEL_LCD_POWER1].display_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER1].bl_on = TCC_GPH(7);
		power_pins[PANEL_LCD_POWER1].reset = TCC_GPC(9);

		power_pins[PANEL_LCD_POWER2].power_on = TCC_GPMC(2);
		power_pins[PANEL_LCD_POWER2].display_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER2].bl_on = TCC_GPE(15);
		power_pins[PANEL_LCD_POWER2].reset = TCC_GPMC(18);

		power_pins[PANEL_LCD_POWER3].power_on = TCC_GPMB(25);
		power_pins[PANEL_LCD_POWER3].display_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER3].bl_on = TCC_GPMC(21);
		power_pins[PANEL_LCD_POWER3].reset = TCC_GPMB(19);

		power_pins[PANEL_LCD_NC].power_on = GPIO_NC;
		power_pins[PANEL_LCD_NC].display_on = GPIO_NC;
		power_pins[PANEL_LCD_NC].bl_on = GPIO_NC;
		power_pins[PANEL_LCD_NC].reset = GPIO_NC;
		break;
	case 2: /*EMS board*/
		power_pins[PANEL_LCD_POWER0].power_on = TCC_GPA(21);
		power_pins[PANEL_LCD_POWER0].display_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER0].bl_on = TCC_GPH(6);
		power_pins[PANEL_LCD_POWER0].reset = TCC_GPG(6);

		power_pins[PANEL_LCD_POWER1].power_on = TCC_GPMB(28);
		power_pins[PANEL_LCD_POWER1].display_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER1].bl_on = TCC_GPH(7);
		power_pins[PANEL_LCD_POWER1].reset = TCC_GPC(12);

		power_pins[PANEL_LCD_POWER2].power_on = TCC_GPC(9);
		power_pins[PANEL_LCD_POWER2].display_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER2].bl_on = TCC_GPMB(29);
		power_pins[PANEL_LCD_POWER2].reset = TCC_GPC(8);

		power_pins[PANEL_LCD_POWER3].power_on = TCC_GPMC(23);
		power_pins[PANEL_LCD_POWER3].display_on = GPIO_NC;
		power_pins[PANEL_LCD_POWER3].bl_on = TCC_GPMC(21);
		power_pins[PANEL_LCD_POWER3].reset = TCC_GPMC(25);
		break;
	default:
		pr_err("[%s] : not valid board type", __func__);
		break;
	}
}
