// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <linux/io.h>
#include <mach/tcc_secureboot.h>
#include <hsm_mbox.h>
#include <mach/reboot.h>
#include <mmc.h>

#define GET_MCERT_ENC_MODE(x) ((readl(x + 0x20) >> 24) & 0xFF)

#define MMC_MCERT_BLK_START (0x01u)
#define MMC_MCERT_BLK_CNT (0x1u)
#define MMC_HSM_BLK_START (0x08u)
#define MMC_HSM_BLK_CNT (0x40u)

#define IMAGEID_HSM (0x1u)
#define IMAGEID_MCERT (0x2u)

#define MCERT_SIZE (0x200u)
#define HSM_SIZE (0x8000u)

#define INITDAT_PMU01 (0x144001C0u)
#define SB_ENABLE_BITS (0xFF0000u)

uint32_t tc_check_secureboot(void)
{
	/* INITDAT_PMU01 - [00] sfEnableCECodeVerification */
	if ((readl(INITDAT_PMU01) & SB_ENABLE_BITS) == SB_ENABLE_BITS) {
		return TC_SB_ENABLE;
	} else {
		return TC_SB_DISABLE;
	}
}

static int32_t
secureboot_mfcmd(uint32_t cmd, uint32_t x1, uint32_t x2, uint32_t x3)
{
	ap_send_mf_cmd(cmd, x1, x2, x3);

	return ap_recv_from_hsm();
}

static int32_t secureboot_mmc_read(
	int32_t bootid, uint32_t blkstart, uint32_t blkcount, uintptr_t bufaddr)
{
	struct mmc *mmc = find_mmc_device(0);

	if (mmc_init(mmc) != 0) {
		pr_force("failed to init mmc device\n");
		return -1;
	}
	if (blk_select_hwpart_devnum(IF_TYPE_MMC, 0, (bootid + 1)) != 0) {
		pr_force("failed to select h/w partiton #%d\n", bootid);
		return -1;
	}
	if (blk_dread(
		mmc_get_blk_desc(mmc), blkstart, blkcount, (char *)bufaddr)
		== blkcount) {
		return 0;
	}
	return -1;
}

static int32_t secureboot_mmc_write(
	int32_t bootid, uint32_t blkstart, uint32_t blkcount, uintptr_t bufaddr)
{
	struct mmc *mmc = find_mmc_device(0);

	if (mmc_init(mmc) != 0) {
		pr_force("failed to init mmc device\n");
		return -1;
	}
	if (blk_select_hwpart_devnum(IF_TYPE_MMC, 0, (bootid + 1)) != 0) {
		pr_force("failed to select h/w partiton #%d\n", bootid);
		return -1;
	}
	if (blk_dwrite(
		mmc_get_blk_desc(mmc), blkstart, blkcount, (char *)bufaddr)
		== blkcount) {
		return 0;
	}
	return -1;
}

static int32_t secureboot_keypackage_load(int32_t bootid, uintptr_t bufaddr)
{
	if (secureboot_mmc_read(
		bootid, MMC_MCERT_BLK_START, MMC_MCERT_BLK_CNT, bufaddr)
		== 0) {
		if (memcmp((void *)bufaddr, "MCERT", 5) == 0) {
			return 0;
		}
	}
	return -1;
}

static int secureboot_keypackage_store(int bootid, uintptr_t bufaddr)
{
	if (secureboot_mmc_write(
		bootid, MMC_MCERT_BLK_START, MMC_MCERT_BLK_CNT, bufaddr)
		== 0) {
		return 0;
	}
	return -1;
}

static int secureboot_hsm_load(int bootid, uintptr_t bufaddr)
{
	if (secureboot_mmc_read(
		bootid, MMC_HSM_BLK_START, MMC_HSM_BLK_CNT, bufaddr)
		== 0) {
		return 0;
	}
	return -1;
}

static int secureboot_hsm_store(int bootid, uintptr_t bufaddr)
{
	if (secureboot_mmc_write(
		bootid, MMC_HSM_BLK_START, MMC_HSM_BLK_CNT, bufaddr)
		== 0) {
		return 0;
	}
	return -1;
}

static int32_t secureboot_manufacture_process(void)
{
	uintptr_t bufaddr = CONFIG_SYS_LOAD_ADDR;

	pr_force("secure boot enable...\n");

	if (secureboot_keypackage_load(0, bufaddr) != 0) {
		pr_force("Failed to load MCERT from mmc\n");
		return -1;
	}
	if (GET_MCERT_ENC_MODE(bufaddr) != 0x80) {
		pr_force("MCERT is not for manufacture\n");
		return -1;
	}

	if (secureboot_mfcmd(
		CMD_HSM_GET_SECURE_IMAGE, bufaddr, MCERT_SIZE,
		IMAGEID_MCERT) != 0) {
		pr_force("Failed to change MF-MCERT to secure image\n");
		return -1;
	}

	pr_force("[SECUREBOOT-MF] STEP1 - 2ND MCERT Encryption\n");
	if (secureboot_keypackage_store(1, bufaddr) != 0) {
		pr_force("Failed to store MCERT to mmc\n");
		return -1;
	}

	pr_force("[SECUREBOOT-MF] STEP2 - 2ND HSM Encryption\n");
	if (secureboot_hsm_load(0, bufaddr) != 0) {
		pr_force("Failed to load HSM from emmc\n");
		return -1;
	}

	if (secureboot_mfcmd(
		CMD_HSM_GET_SECURE_IMAGE, bufaddr, HSM_SIZE, IMAGEID_HSM)
		!= 0) {
		pr_force("Failed to change HSM to secure image\n");
		return -1;
	}
	if (secureboot_hsm_store(1, bufaddr) != 0) {
		pr_force("Failed to store HSM to mmc\n");
		return -1;
	}

	pr_force("[SECUREBOOT-MF] STEP3 - Secure Boot Key Writing\n");
	if (secureboot_mfcmd(CMD_HSM_KEY_WRITE, 0, 0, 0) != 0) {
		pr_force("Failed to store Secure Boot Key\n");
		return -1;
	}

	pr_force("[SECUREBOOT-MF] STEP4 - 1ST MCERT Encryption\n");
	if (secureboot_keypackage_load(1, bufaddr) != 0) {
		pr_force("Failed to load MCERT from mmc\n");
		return -1;
	}
	if (secureboot_keypackage_store(0, bufaddr) != 0) {
		pr_force("Failed to store MCERT to mmc\n");
		return -1;
	}

	pr_force("[SECUREBOOT-MF] STEP5 - 1ST HSM Encryption\n");
	if (secureboot_hsm_load(1, bufaddr) != 0) {
		pr_force("Failed to load HSM from mmc\n");
		return -1;
	}
	if (secureboot_hsm_store(0, bufaddr) != 0) {
		pr_force("Failed to store HSM to mmc\n");
		return -1;
	}

	pr_force("[SECUREBOOT-MF] STEP6 - Secure Boot Enable\n");
	if (secureboot_mfcmd(CMD_HSM_SECURE_BOOT_ENABLE, 0, 0, 0) != 0) {
		pr_force("Failed to enable Secure Boot\n");
		return -1;
	}

	set_reboot_type(REBOOT_TYPE_COLD);
	do_reset(NULL, 0, 0, NULL);

	return 0;
}

static int secureboot_mcu_manufacture(void)
{
	pr_force(
		"Reboot and Encrypt images in SNOR for secure boot enable by MCU...\n");
	set_reboot_reason(BOOT_SECUREBOOT_ENABLE);
	set_reboot_type(REBOOT_TYPE_COLD);
	do_reset(NULL, 0, 0, NULL);
	return 0;
}

#define BOOTMODE_SNOR 0x1      // SNOR back-up mode (LEGACY)
#define BOOTMODE_SNOR_eMMC 0x2 // DIE1 Normal Boot
#define BOOTMODE_SNOR_UFS 0xA  // DIE1 Normal Boot
#define BOOTMODE_SNOR_GPSB 0xE // DIE2 Normal Boot

static uint32_t check_snor_boot(void)
{
	if ((readl(TCC_PMU_BASE + 0x14) & 0xF) == BOOTMODE_SNOR) {
		return BOOTMODE_SNOR;
	} else if ((readl(TCC_PMU_BASE + 0x14) & 0xF) == BOOTMODE_SNOR_eMMC) {
		return BOOTMODE_SNOR_eMMC;
	} else if ((readl(TCC_PMU_BASE + 0x14) & 0xF) == BOOTMODE_SNOR_UFS) {
		return BOOTMODE_SNOR_UFS;
	} else if ((readl(TCC_PMU_BASE + 0x14) & 0xF) == BOOTMODE_SNOR_GPSB) {
		return BOOTMODE_SNOR_GPSB;
	}
	return 0;
}

int32_t tc_secureboot_enable(void)
{
	if (tc_check_secureboot() == 0) {
		if (check_snor_boot() != 0u) {
			if (secureboot_mcu_manufacture() == 0) {
				return 0;
			}
		} else {
			if (secureboot_manufacture_process() == 0) {
				return 0;
			}
		}
	} else {
		pr_force("Secure boot has already been enabled\n");
	}
	return -1;
}

void tc_secureboot_individualize(void)
{
	uintptr_t bufaddr = CONFIG_SYS_LOAD_ADDR;
	int bootid;

	if (tc_check_secureboot() == 0) {
		return;
	}
	if (check_snor_boot() != 0u) {
		return;
	}
	for (bootid = 0; bootid < 2; bootid++) {
		if (secureboot_keypackage_load(bootid, bufaddr) != 0) {
			break;
		}
		if (GET_MCERT_ENC_MODE(bufaddr) == 0) {
			if (secureboot_mfcmd(
				CMD_HSM_GET_SECURE_IMAGE, bufaddr,
				MCERT_SIZE, IMAGEID_MCERT) != 0) {
				pr_force(
					"Failed to individualize MCERT(bootid=%d)\n",
					bootid);
				break;
			}
			if (secureboot_keypackage_store(bootid, bufaddr) != 0) {
				pr_force(
					"Failed to store keypackage to mmc(bootid=%d)\n",
					bootid);
				break;
			}
			pr_force(
				"Individualize the MCERT(bootid=%d)\n", bootid);
		}
	}
}
