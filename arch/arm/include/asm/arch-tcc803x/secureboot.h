/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCC803X_SECUREBOOT_H
#define TCC803X_SECUREBOOT_H

#define FBOOT_NAME                  "BL1"

#define MCERT_NAME                  "key package"
#define MCERT_TAG_NAME              "KEY PACKAGES"
#define MCERT_TAG_SIZE              (12U)

#define GET_MCERT_ENC_MODE(x)       (readl(x + 0xCU) & 0x1FFFFU)
#define MCERT_MF_FLAG_BIT           (0x00010000U)

#define IMAGEID_FBOOT               (0x2U)
#define FBOOT_OFFSET                (0x0U)
#define FBOOT_MAX_SIZE              (0x20000U)
#define FBOOT_SIZE(x)               (0x200U + readl((x) + 0x4U))

#define IMAGEID_MCERT               (0x7U)
#define MCERT_OFFSET                (0x3E000U)
#define MCERT_SIZE                  (0x200U)

#define BOOTMODE_USB                (0x0U)
#define BOOTMODE_I2CM               (0x1U)
#define BOOTMODE_SFMC_3B            (0x2U)
#define BOOTMODE_SFMC_4B            (0x3U)
#define BOOTMODE_NAND               (0x4U)
#define BOOTMODE_eMMC_SD0           (0x5U)
#define BOOTMODE_eMMC_SD1           (0x6U)

inline static bool mcert_controlled_by_AP(void)
{
	switch (readl(PMU_CONFIG) & PMU_BM_MASK) {
	case BOOTMODE_SFMC_3B:
	case BOOTMODE_SFMC_4B:
		return false;
	}
	return true;
}

inline static int manufacture_status(void)
{
	return (readl(PMU_SECURE_INF01) & 0x1FFF0000);
}

#endif /* TCC803X_SECUREBOOT_H */
