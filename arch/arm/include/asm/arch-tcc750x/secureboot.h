/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCC750X_SECUREBOOT_H
#define TCC750X_SECUREBOOT_H

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

#define BOOTMODE_USB                (0x0U)
#define BOOTMODE_SNOR               (0x1U)
#define BOOTMODE_EMMC               (0x2U)
#define BOOTMODE_EMMC_SIC           (0x3U)

inline static bool mcert_controlled_by_AP(void)
{
	switch (readl(PMU_CONFIG) & PMU_BM_MASK) {
	case BOOTMODE_EMMC_SIC:
		return false;
	}
	return true;
}

inline static int manufacture_status(void)
{
	return 0;
}

#endif /* TCC750X_SECUREBOOT_H */
