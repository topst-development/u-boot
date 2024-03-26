/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCC807X_SECUREBOOT_H
#define TCC807X_SECUREBOOT_H

#define FBOOT_NAME                  "HSM"

#define MCERT_NAME                  "MCERT"
#define MCERT_TAG_NAME              "MCERT"
#define MCERT_TAG_SIZE              (5U)

#define GET_MCERT_ENC_MODE(x)       ((readl(x + 0x20U) >> 24) & 0xFFU)
#define MCERT_MF_FLAG_BIT           (0x80U)

#define IMAGEID_FBOOT               (0x1U)
#define FBOOT_OFFSET                (0x1000U)
#define FBOOT_MAX_SIZE              (0x8000U)
#define FBOOT_SIZE(x)               (FBOOT_MAX_SIZE)

#define IMAGEID_MCERT               (0x2U)
#define MCERT_OFFSET                (0x200U)
#define MCERT_SIZE                  (0x200U)

#define BOOTMODE_USB20_FWDN         (0x0U)
#define BOOTMODE_SNOR               (0x1U)
#define BOOTMODE_SNOR_eMMC          (0x2U)
#define BOOTMODE_GPSB_FWDN          (0x4U)
#define BOOTMODE_eMMC_Only          (0x5U)
#define BOOTMODE_SD                 (0x7U)
#define BOOTMODE_USB30_FWDN         (0x8U)
#define BOOTMODE_SNOR_UFS           (0xAU)
#define BOOTMODE_UFS_Only           (0xDU)
#define BOOTMODE_SNOR_GPSB          (0xEU)

inline static bool mcert_controlled_by_AP(void)
{
	switch (readl(PMU_CONFIG) & PMU_BM_MASK) {
	case BOOTMODE_SNOR:
	case BOOTMODE_SNOR_eMMC:
	case BOOTMODE_SNOR_UFS:
	case BOOTMODE_SNOR_GPSB:
		return false;
	}
	return true;
}

inline static int manufacture_status(void)
{
	return 0;
}

#endif /* TCC807X_SECUREBOOT_H */
