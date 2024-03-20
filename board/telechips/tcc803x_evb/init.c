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
#include <mach/cm4.h>
#include <mach/reboot.h>
#include <mach/soc.h>
#include <mach/tcc_secureboot.h>
#include <i2c.h>

#if defined(CONFIG_CPU_OVER_VOLTAGE)

#define I2C_BUS		(1)
#define PMIC_ADDR	(0x58)
#define PMIC_VBUCK2_A	(0xa3)
#define PMIC_VTARGET	(0x40) /* 0.94v */

static s32 set_cpu_voltage(void)
{
	u8 value = PMIC_VTARGET;
	s32 ret = 0;
	u32 chip_code;

	/* CONFIG_CPU_OVER_VOLTAGE is supported at TCC803xpe */
	chip_code = get_chip_code();

	if ((chip_code & PIN2PIN) != 0) {
		struct udevice *dev;

		uclass_foreach_dev_probe(UCLASS_I2C, dev);
		ret = i2c_get_chip_for_busnum(I2C_BUS, PMIC_ADDR, 1, &dev);

		if (ret == 0) {
			value = PMIC_VTARGET;
			ret = dm_i2c_write(dev, PMIC_VBUCK2_A, &value, 1);
		}
	}

	return (ret == 0) ? 0 : -EINVAL;
}
#endif

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
#if defined(CONFIG_CPU_OVER_VOLTAGE)
	(void)set_cpu_voltage();
#endif
	device_probe(UCLASS_I2C);
	device_probe(UCLASS_DISPLAY);

	return 0;
}

int misc_init_f(void)
{
#if defined(CONFIG_INIT_SUBCORE_SERIAL_BY_MAINCORE)
	bool wake_from_corerst = core_reset_occurred();

	if (!wake_from_corerst) {
		device_probe(UCLASS_SP_SERIAL);
	}
#endif
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

int board_late_init(void)
{
#if defined(CONFIG_BOOT_CM4_BY_MAINCORE)
	run_cm4_firmware();
#endif

	board_set_serialno();
	board_set_id_rev();
	tc_secureboot_individualize();
	reboot_set_env();

	return 0;
}
