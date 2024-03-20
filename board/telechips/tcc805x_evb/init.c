// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <init.h>
#include <dm.h>
#include <dm/uclass.h>
#include <mach/chipinfo.h>
#include <mach/clock.h>
#include <mach/gpu.h>
#include <mach/reboot.h>
#include <mach/soc.h>
#include <mach/tcc_secureboot.h>
#include <power/pmic.h>
#include <power/da9131.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/telechips/gpio.h>
#include <asm/mach-types.h>

static inline void device_probe(enum uclass_id id)
{
	struct udevice *dev;
	s32 ret = uclass_first_device_err(id, &dev);

	while ((ret == 0) && (dev != NULL)) {
		ret = uclass_next_device_err(&dev);
	}
}

int board_early_init_f(void)
{
#if defined(CONFIG_TCC_MAINCORE)
	/*
	 * To prevent the chipset from entering the test mode,
	 * we write the test mode password into OTP.
	 */
	testmode_password_enable();
#endif

	reboot_init();
	clock_init_early();

	device_probe(UCLASS_CLK);
	device_probe(UCLASS_MAILBOX);
	device_probe(UCLASS_FIRMWARE);

	return 0;
}

int board_init(void)
{
	clock_init();

	gpio_config(TCC_GPA(30), GPIO_FN0|GPIO_INPUT);
	if(gpio_get(TCC_GPA(30))) {
		printf("[%s:%d] Device mode is on \n", __func__, __LINE__);
		gpio_config(TCC_GPMC(6), GPIO_FN0|GPIO_OUTPUT);
		gpio_set( TCC_GPMC(6), 0);
	}
	else  { 
		printf("[%s:%d] HOST mode \n", __func__, __LINE__);
		gpio_config(TCC_GPMC(6), GPIO_FN0|GPIO_OUTPUT);
		gpio_set( TCC_GPMC(6), 1);
	}

    /* PERI_PWR_EN on TCC8053_SVM_DVRS_DSM_MAIN Board */
    printf("[%s:%d] PERI_PWR_EN is on \n", __func__, __LINE__);
    gpio_config( TCC_GPMB(5), GPIO_FN0|GPIO_OUTPUT|GPIO_PULLUP|GPIO_HIGH );
    gpio_set( TCC_GPMB(5), 1 );

    /* CAM_PWR_EN on TCC8053_SVM_DVRS_DSM_MAIN Board */
    printf("[%s:%d] CAM_PWR_PWR_EN is on \n", __func__, __LINE__);
    gpio_config( TCC_GPMB(17), GPIO_FN0|GPIO_OUTPUT|GPIO_PULLUP|GPIO_HIGH );
    gpio_set( TCC_GPMB(17), 1 );

    /* Raspberry PI CAM */
    //printf("[%s:%d] MIPI1_GPIO1 is on \n", __func__, __LINE__);
    gpio_config( TCC_GPB(15), GPIO_FN0|GPIO_OUTPUT|GPIO_PULLUP|GPIO_HIGH );
    gpio_set( TCC_GPB(15), 1 );

	gpu_init();

	device_probe(UCLASS_POWER_DOMAIN);
	device_probe(UCLASS_I2C);
	device_probe(UCLASS_DISPLAY);

	return 0;
}

static inline void board_set_serialno(void)
{
	const char *val = env_get("serial#");

	/*
	 * The default implementation of this function is only for reference,
	 * and users must modify here as they need.
	 */
	if (val == NULL) {
		u64 uid = get_chip_id();
		char serialno[12];

		uid &= 0xFFFFFFFFFFFULL;

		(void)scnprintf(serialno, sizeof(serialno), "%011llX", uid);
		(void)env_set("serial#", serialno);
	}
}

#if defined(CONFIG_ANDROID_DT_OVERLAY)
static inline void board_set_id_rev(void)
{
	ofnode root = np_to_ofnode(gd->of_root);
	u32 config;

	config = ofnode_read_u32_default(root, "board-id", 0x0);
	(void)env_set_hex("board_id", (ulong)config);

	config = ofnode_read_u32_default(root, "board-rev", 0x0);
	(void)env_set_hex("board_rev", (ulong)config);
}
#else
#  define board_set_id_rev()
#endif

static inline void power_domain_off_control(void)
{
	struct udevice *dev;

	(void)uclass_get_device_by_name(UCLASS_MISC, "power_domain_off", &dev);
}

static inline void pmic_gbus_control(void)
{
	struct udevice *dev;
	s32 ret;

	ret = pmic_get("da9131", &dev);
	if (ret == 0) {
		(void)pmic_reg_write(dev, DA9131_BUCK2_5, DA9131_BUCK2_5_VAL);
	}
}

int board_late_init(void)
{
	board_set_serialno();
	board_set_id_rev();
	power_domain_off_control();
	pmic_gbus_control();
	tc_secureboot_individualize();
	reboot_set_env();

	return 0;
}
