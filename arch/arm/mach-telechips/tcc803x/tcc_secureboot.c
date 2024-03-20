// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <linux/io.h>
#include <linux/arm-smccc.h>
#include <mach/reboot.h>
#include <mach/smc.h>
#include <mach/tcc_secureboot.h>
#include <mmc.h>

#define MFCMD_GET_SECUREIMAGE (1)
#define MFCMD_SECUREBOOT_KEY_FUSE (2)
#define MFCMD_SECUREBOOT_ENABLE_FUSE (3)

#define IMAGEID_BL1 (2)
#define IMAGEID_KEYPACKAGE (7)

#define GET_KEYPACKAGE_MFFLAGS(x) ((readl(x + 0xC) >> 16) & 0xFFFF)
#define GET_KEYPACKAGE_REENC(x) (readl(x + 0xC))
#define KEYPACKAGE_SIZE (0x200)
#define BL1_HEADER_SIZE (0x200)

#define MMC_KEYPACKAGE_BLK_START (0x1F0)
#define MMC_KEYPACKAGE_BLK_CNT (0x1)
#define MMC_BL1_BLK_START (0x0)
#define MMC_BL1_BLK_CNT (0x100)

#define INITDAT_PMU01 (0x144001C0u)
#define SB_ENABLE_BITS (0x1u)

uint32_t tc_check_secureboot(void)
{
	/* INITDAT_PMU01 - [00] sfEnableCECodeVerification */
	if ((readl(INITDAT_PMU01) & SB_ENABLE_BITS) == SB_ENABLE_BITS) {
		return TC_SB_ENABLE;
	} else {
		return TC_SB_DISABLE;
	}
}

static int secureboot_mfcmd(int cmd, int x1, int x2, int x3, int x4)
{
	struct arm_smccc_res res;

	arm_smccc_smc(cmd, x1, x2, x3, x4, 0, 0, 0, &res);
	return res.a0;
}

static int secureboot_mmc_read(
	int bootid, unsigned int blkstart, unsigned int blkcount,
	uintptr_t bufaddr)
{
	struct mmc *mmc = find_mmc_device(0);
	unsigned int res;
	int current_part = mmc_get_blk_desc(mmc)->hwpart;

	if (mmc_init(mmc) != 0) {
		pr_err("failed to init mmc device\n");
		return -1;
	}
	if (mmc_switch_part(mmc, bootid + 1) != 0) {
		pr_err("failed to switch partiton #%d\n", bootid);
		return -1;
	}
	res = blk_dread(
		mmc_get_blk_desc(mmc), blkstart, blkcount, (char *)bufaddr);
	if (mmc_switch_part(mmc, current_part) != 0) {
		pr_err("failed to switch partiton #0\n");
		return -1;
	}
	if (res == blkcount) {
		return 0;
	}
	return -1;
}

static int secureboot_mmc_write(
	int bootid, unsigned int blkstart, unsigned int blkcount,
	uintptr_t bufaddr)
{
	struct mmc *mmc = find_mmc_device(0);
	unsigned int res;
	int current_part = mmc_get_blk_desc(mmc)->hwpart;

	if (mmc_init(mmc) != 0) {
		pr_err("failed to init mmc device\n");
		return -1;
	}
	if (mmc_switch_part(mmc, bootid + 1) != 0) {
		pr_err("failed to switch partiton #%d\n", bootid);
		return -1;
	}
	res = blk_dwrite(
		mmc_get_blk_desc(mmc), blkstart, blkcount, (char *)bufaddr);
	if (mmc_switch_part(mmc, current_part) != 0) {
		pr_err("failed to switch partiton #0\n");
		return -1;
	}
	if (res == blkcount) {
		return 0;
	}
	return -1;
}

static int secureboot_keypackage_load(int bootid, uintptr_t bufaddr)
{
	if (secureboot_mmc_read(
		bootid, MMC_KEYPACKAGE_BLK_START, MMC_KEYPACKAGE_BLK_CNT,
		bufaddr) == 0) {
		if (memcmp((void *)bufaddr, "KEY PACKAGES", 12) == 0) {
			return 0;
		}
	}
	return -1;
}

static int secureboot_keypackage_store(int bootid, uintptr_t bufaddr)
{
	if (secureboot_mmc_write(
		bootid, MMC_KEYPACKAGE_BLK_START, MMC_KEYPACKAGE_BLK_CNT,
		bufaddr) == 0) {
		return 0;
	}
	return -1;
}

static int secureboot_bl1_load(int bootid, uintptr_t bufaddr)
{
	if (secureboot_mmc_read(
		bootid, MMC_BL1_BLK_START, MMC_BL1_BLK_CNT,
		bufaddr) == 0) {
		return 0;
	}
	return -1;
}

static int secureboot_bl1_store(int bootid, uintptr_t bufaddr)
{
	if (secureboot_mmc_write(
		bootid, MMC_BL1_BLK_START, MMC_BL1_BLK_CNT,
		bufaddr) == 0) {
		return 0;
	}
	return -1;
}

static int secureboot_manufacture_process(void)
{
	uintptr_t bufaddr = CONFIG_SYS_LOAD_ADDR;
	unsigned int bl1size;

	pr_info("secure boot enable...\n");

	if ((readl(INITDAT_PMU01) & 0x1FFF0000) == 0) {
		if (secureboot_keypackage_load(0, bufaddr) != 0) {
			pr_err("Failed to load key package from mmc\n");
			return -1;
		}
		if ((GET_KEYPACKAGE_MFFLAGS(bufaddr) & 0x1) != 0x1) {
			pr_err("Key Package is not for manufacture\n");
			return -1;
		}
		if (secureboot_mfcmd(
			SIP_TCSB_MFCMD_GET_SECUREIMAGE, 0,
			IMAGEID_KEYPACKAGE, bufaddr,
			KEYPACKAGE_SIZE) != 0) {
			pr_err("Failed to change Key Package to secure image\n");
			return -1;
		}

		pr_info("[SECUREBOOT-MF] STEP1 - 2ND KEY PACKAGE Encryption\n");
		if (secureboot_keypackage_store(1, bufaddr) != 0) {
			pr_err("Failed to store keypackage to mmc\n");
			return -1;
		}

		pr_info("[SECUREBOOT-MF] STEP2 - 2ND BL1 Encryption\n");
		if (secureboot_bl1_load(0, bufaddr) != 0) {
			pr_err("Failed to load bl1 from emmc\n");
			return -1;
		}
		bl1size = readl(bufaddr + 0x4) + BL1_HEADER_SIZE;
		if (secureboot_mfcmd(
			SIP_TCSB_MFCMD_GET_SECUREIMAGE, 0, IMAGEID_BL1,
			bufaddr, bl1size) != 0) {
			return -1;
		}
		if (secureboot_bl1_store(1, bufaddr) != 0) {
			pr_err("Failed to store BL1 to mmc\n");
			return -1;
		}

		pr_info("[SECUREBOOT-MF] STEP3 - Secure Boot Key Writing\n");
		if (secureboot_mfcmd(
			SIP_TCSB_MFCMD_SECUREBOOT_KEY_WRITE, 0, 0, 0, 0) != 0) {
			pr_err("Failed to store Secure Boot Key\n");
			return -1;
		}
	} else {
		pr_info("[SECUREBOOT-MF] STEP 1 to 3 have already been completed\n");
	}

	pr_info("[SECUREBOOT-MF] STEP4 - 1ST KEY PACKAGE Encryption\n");
	if (secureboot_keypackage_load(1, bufaddr) != 0) {
		pr_err("Failed to load keypackage from mmc\n");
		return -1;
	}
	if (secureboot_keypackage_store(0, bufaddr) != 0) {
		pr_err("Failed to store keypackage to mmc\n");
		return -1;
	}

	pr_info("[SECUREBOOT-MF] STEP5 - 1ST BL1 Encryption\n");
	if (secureboot_bl1_load(1, bufaddr) != 0) {
		pr_err("Failed to load keypackage from mmc\n");
		return -1;
	}
	if (secureboot_bl1_store(0, bufaddr) != 0) {
		pr_err("Failed to store keypackage to mmc\n");
		return -1;
	}

	pr_info("[SECUREBOOT-MF] STEP6 - Secure Boot Enable\n");
	if (secureboot_mfcmd(SIP_TCSB_MFCMD_SECUREBOOT_ENABLE, 0, 0, 0, 0)
		!= 0) {
		pr_err("Failed to enable Secure Boot\n");
		return -1;
	}

	set_reboot_reason(REBOOT_TYPE_COLD);
	do_reset(NULL, 0, 0, NULL);
	return 0;
}

static int secureboot_mcu_manufacture(void)
{
	pr_info("Reboot and Encrypt images in SNOR for secure boot enable by MCU...\n");
	set_reboot_reason(BOOT_SECUREBOOT_ENABLE);
	set_reboot_type(REBOOT_TYPE_COLD);
	do_reset(NULL, 0, 0, NULL);
	return 0;
}

int32_t tc_secureboot_enable(void)
{
	if ((readl(TCC_PMU_BASE + 0x1C0) & 0x1) == 0) {
		if ((readl(TCC_PMU_BASE + 0x14) & 0xE) == 0x2) {
			if (secureboot_mcu_manufacture() == 0) {
				return 0;
			}
		} else {
			if (secureboot_manufacture_process() == 0) {
				return 0;
			}
		}
	} else {
		pr_info("Secure boot has already been enabled\n");
	}
	return -1;
}

void tc_secureboot_individualize(void)
{
	uintptr_t bufaddr = CONFIG_SYS_LOAD_ADDR;
	int bootid;

	if ((readl(TCC_PMU_BASE + 0x1C0) & 0x1) == 0) {
		return;
	}
	if ((readl(TCC_PMU_BASE + 0x14) & 0xE) == 0x2) {
		return;
	}
	for (bootid = 0; bootid < 2; bootid++) {
		if (secureboot_keypackage_load(bootid, bufaddr) != 0) {
			break;
		}
		if (GET_KEYPACKAGE_REENC(bufaddr) == 0) {
			if (secureboot_mfcmd(
				SIP_TCSB_MFCMD_GET_SECUREIMAGE, bootid,
				IMAGEID_KEYPACKAGE, bufaddr,
				KEYPACKAGE_SIZE) != 0) {
				pr_err("Failed to individualize keypackage(bootid=%d)\n",
						bootid);
				break;
			}
			if (secureboot_keypackage_store(bootid, bufaddr) != 0) {
				pr_err("Failed to store keypackage to mmc(bootid=%d)\n",
						bootid);
				break;
			}
			pr_info("Individualize the key package(bootid=%d)\n",
				bootid);
		}
	}
}
