/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <command.h>
#include <common.h>
#include <linux/io.h>
#include <asm/arch/pmu.h>
#include <asm/arch/secureboot.h>
#include <asm/dma-mapping.h>
#include <mach/reboot.h>
#include <mach/secureboot.h>
#include <tcc_hsm_fw.h>
#include <mmc.h>
#if defined(CONFIG_UFS_BOOT)
#include <scsi.h>
#endif

#if defined(CONFIG_UFS_BOOT)
#define BLK_ALIGN             (0x1000U)
#else
#define BLK_ALIGN             (0x200U)
#endif

#define MCERT_BLK_START       (MCERT_OFFSET / BLK_ALIGN)
#define MCERT_BLK_CNT         ((MCERT_SIZE + BLK_ALIGN - 1U) / BLK_ALIGN)
#define MCERT_BLK_OFFSET      (MCERT_OFFSET & (BLK_ALIGN - 1U))
#define FBOOT_BLK_START       (FBOOT_OFFSET / BLK_ALIGN)
#define FBOOT_BLK_CNT         (FBOOT_MAX_SIZE / BLK_ALIGN)

#if CONFIG_IS_ENABLED(TCC_SECUREBOOT_ENABLE) || CONFIG_IS_ENABLED(TCC_MCERT_UNIQUEKEY_ENC)
static int32_t secureboot_read(
	int32_t bootid, lbaint_t blkstart, ulong blkcount, uintptr_t buf)
{
#if defined(CONFIG_UFS_BOOT)
	ulong n;

	n = blk_read_devnum(
		IF_TYPE_SCSI, bootid, blkstart, blkcount, (void *)buf);
	flush_dcache_range(
		(unsigned long)buf,
		(unsigned long)(buf + (BLK_ALIGN * blkcount)));

	return n == blkcount ? 0 : 1;
#else
	struct mmc *mmc = find_mmc_device(0);
	struct blk_desc *desc;

	if (mmc_init(mmc) != 0) {
		pr_force("failed to init mmc device\n");
		return -1;
	}

	desc = mmc_get_blk_desc(mmc);
	if (blk_dselect_hwpart(desc, (bootid + 1)) != 0) {
		pr_force("failed to select h/w partiton #%d\n", bootid);
		return -1;
	}
	if (blk_dread(desc, blkstart, blkcount, (void *)buf)
		== blkcount) {
		return 0;
	}
	return -1;
#endif
}

static int32_t secureboot_write(
	int32_t bootid, lbaint_t blkstart, ulong blkcount, uintptr_t buf)
{
#if defined(CONFIG_UFS_BOOT)
	ulong n;

	n = blk_write_devnum(
		IF_TYPE_SCSI, bootid, blkstart, blkcount, (void *)buf);
	flush_dcache_range(
		(unsigned long)buf,
		(unsigned long)(buf + (BLK_ALIGN * blkcount)));

	return n == blkcount ? 0 : 1;
#else
	struct mmc *mmc = find_mmc_device(0);
	struct blk_desc *desc;

	if (mmc_init(mmc) != 0) {
		pr_force("failed to init mmc device\n");
		return -1;
	}

	desc = mmc_get_blk_desc(mmc);
	if (blk_dselect_hwpart(desc, (bootid + 1)) != 0) {
		pr_force("failed to select h/w partiton #%d\n", bootid);
		return -1;
	}
	if (blk_dwrite(
		mmc_get_blk_desc(mmc), blkstart, blkcount, (void *)buf)
		== blkcount) {
		return 0;
	}
	return -1;
#endif
}

static int32_t secureboot_mcert_load(int32_t bootid, uintptr_t buf)
{
	if (secureboot_read(
		bootid, MCERT_BLK_START, MCERT_BLK_CNT, buf) == 0) {
		if (memcmp(
			(void *)(buf + MCERT_BLK_OFFSET),
			MCERT_TAG_NAME, MCERT_TAG_SIZE) == 0) {
			return 0;
		}
	}
	return -1;
}

static int secureboot_mcert_store(int bootid, uintptr_t buf)
{
	if (secureboot_write(
		bootid, MCERT_BLK_START, MCERT_BLK_CNT, buf) == 0) {
		return 0;
	}
	return -1;
}
#endif

#if CONFIG_IS_ENABLED(TCC_SECUREBOOT_ENABLE)
static int secureboot_fboot_load(int bootid, uintptr_t buf)
{
	if (secureboot_read(
		bootid, FBOOT_BLK_START, FBOOT_BLK_CNT, buf) == 0) {
		return 0;
	}
	return -1;
}

static int secureboot_fboot_store(int bootid, uintptr_t buf)
{
	if (secureboot_write(
		bootid, FBOOT_BLK_START, FBOOT_BLK_CNT, buf) == 0) {
		return 0;
	}
	return -1;
}

static int secureboot_enable(int simmode, int reboot)
{
	uintptr_t bufaddr;
	uintptr_t mcertaddr;
	char *buf;

	pr_force("secure boot enable...\n");

	buf = dma_alloc_coherent(
		(BLK_ALIGN * FBOOT_BLK_CNT) + BLK_ALIGN, &bufaddr);
	bufaddr += BLK_ALIGN - (bufaddr % BLK_ALIGN);

	if (manufacture_status() == 0) {
		if (secureboot_mcert_load(0, bufaddr) != 0) {
			pr_force("Failed to load %s from mmc\n", MCERT_NAME);
			dma_free_coherent(buf);
			return -EPERM;
		}

		mcertaddr = bufaddr + MCERT_BLK_OFFSET;

		if (GET_MCERT_ENC_MODE(mcertaddr) != MCERT_MF_FLAG_BIT) {
			pr_force("%s is not for manufacture\n", MCERT_NAME);
			dma_free_coherent(buf);
			return -EPERM;
		}

		if (tcc_hsm_secureboot_image_encrypt(
			mcertaddr, MCERT_SIZE, IMAGEID_MCERT, 0) != 0) {
			pr_force("Failed to change MF-%s to secure image\n",
				MCERT_NAME);
			dma_free_coherent(buf);
			return -EPERM;
		}

		pr_force("[SECUREBOOT-MF] STEP1 - 2ND %s Encryption\n",
			MCERT_NAME);
		if (secureboot_mcert_store(1, bufaddr) != 0) {
			pr_force("Failed to store %s to mmc\n", MCERT_NAME);
			dma_free_coherent(buf);
			return -EPERM;
		}

		pr_force("[SECUREBOOT-MF] STEP2 - 2ND %s Encryption\n",
			FBOOT_NAME);
		if (secureboot_fboot_load(0, bufaddr) != 0) {
			pr_force("Failed to load %s from emmc\n", FBOOT_NAME);
			dma_free_coherent(buf);
			return -EPERM;
		}

		if (tcc_hsm_secureboot_image_encrypt(
			bufaddr, FBOOT_SIZE(bufaddr), IMAGEID_FBOOT, 0) != 0) {
			pr_force("Failed to change %s to secure image\n",
				FBOOT_NAME);
			dma_free_coherent(buf);
			return -EPERM;
		}

		if (secureboot_fboot_store(1, bufaddr) != 0) {
			pr_force("Failed to store %s to mmc\n", FBOOT_NAME);
			dma_free_coherent(buf);
			return -EPERM;
		}

		pr_force("[SECUREBOOT-MF] STEP3 - Secure Boot Key Writing\n");
		if (simmode == 0) {
			if (tcc_hsm_secureboot_key_fuse() != 0) {
				pr_force("Failed to store Secure Boot Key\n");
				dma_free_coherent(buf);
				return -EPERM;
			}
		} else {
			pr_force("STEP3 was omitted.\n");
		}
	} else {
		pr_force("[SECUREBOOT-MF] STEP 1 to 3 have already been completed\n");
	}

	pr_force("[SECUREBOOT-MF] STEP4 - 1ST %s Encryption\n", MCERT_NAME);
	if (secureboot_mcert_load(1, bufaddr) != 0) {
		pr_force("Failed to load %s from mmc\n", MCERT_NAME);
		dma_free_coherent(buf);
		return -EPERM;
	}
	if (secureboot_mcert_store(0, bufaddr) != 0) {
		pr_force("Failed to store %s to mmc\n", MCERT_NAME);
		dma_free_coherent(buf);
		return -EPERM;
	}

	pr_force("[SECUREBOOT-MF] STEP5 - 1ST %s Encryption\n", FBOOT_NAME);
	if (secureboot_fboot_load(1, bufaddr) != 0) {
		pr_force("Failed to load %s from mmc\n", FBOOT_NAME);
		dma_free_coherent(buf);
		return -EPERM;
	}
	if (secureboot_fboot_store(0, bufaddr) != 0) {
		pr_force("Failed to store %s to mmc\n", FBOOT_NAME);
		dma_free_coherent(buf);
		return -EPERM;
	}

	pr_force("[SECUREBOOT-MF] STEP6 - Secure Boot Enable\n");
	if (simmode == 0) {
		if (tcc_hsm_secureboot_enable() != 0) {
			pr_force("Failed to enable Secure Boot\n");
			dma_free_coherent(buf);
			return -EPERM;
		}
	} else {
		pr_force("STEP6 was omitted.\n");
	}

	dma_free_coherent(buf);
#if defined(CONFIG_UFS_BOOT)
	scsi_ufs_start_stop();
#endif
	if (reboot != 0) {
		set_reboot_type(REBOOT_TYPE_COLD);
		do_reset(NULL, 0, 0, NULL);
	}

	return 0;
}
#endif

#if CONFIG_IS_ENABLED(TCC_MCERT_UNIQUEKEY_ENC)
static void secureboot_mcert_individualize(void)
{
	uintptr_t bufaddr;
	uintptr_t mcertaddr;
	char *buf;
	int bootid;

	buf = dma_alloc_coherent(
		(BLK_ALIGN * MCERT_BLK_CNT) + BLK_ALIGN, &bufaddr);
	bufaddr += BLK_ALIGN - (bufaddr % BLK_ALIGN);
	mcertaddr = bufaddr + MCERT_BLK_OFFSET;

	for (bootid = 0; bootid < 2; bootid++) {
		if (secureboot_mcert_load(bootid, bufaddr) != 0) {
			break;
		}
		if (GET_MCERT_ENC_MODE(mcertaddr) == 0) {
			if (tcc_hsm_secureboot_image_encrypt(
				mcertaddr, MCERT_SIZE,
				IMAGEID_MCERT, bootid) != 0) {
				pr_force(
					"Failed to re-encrypt %s (bootid=%d)\n",
					MCERT_NAME, bootid);
				break;
			}
			if (secureboot_mcert_store(bootid, bufaddr) != 0) {
				pr_force(
					"Failed to store %s (bootid=%d)\n",
					MCERT_NAME, bootid);
				break;
			}
			pr_force(
				" %s (bootid=%d) Individualization Success\n",
				MCERT_NAME, bootid);
		}
	}

	dma_free_coherent(buf);
}
#endif

void tc_secureboot_init(void)
{
#if CONFIG_IS_ENABLED(TCC_SECUREBOOT_ENABLE)
	if (get_boot_reason() == BOOT_SECUREBOOT_ENABLE) {
		(void)tc_secureboot_enable(0, 1);
	}
#endif
#if CONFIG_IS_ENABLED(TCC_MCERT_UNIQUEKEY_ENC)
	if (tc_secureboot_state() == TC_SB_ENABLE) {
		if (mcert_controlled_by_AP() == true) {
			secureboot_mcert_individualize();
		}
	}
#endif
}

int tc_secureboot_state(void)
{
	int ret = TC_SB_ENABLE;

	if ((pmu_readl(PMU_SECURE_INF01) & PMU_SB_ENABLE_MASK) == 0) {
		ret = TC_SB_DISABLE;
	}

	return ret;
}

#if CONFIG_IS_ENABLED(TCC_SECUREBOOT_ENABLE)
int tc_secureboot_enable(int simmode, int reboot)
{
	int ret = -EINVAL;

	if (tc_secureboot_state() == TC_SB_DISABLE) {
		if (mcert_controlled_by_AP() == true) {
			ret = secureboot_enable(simmode, reboot);
		} else {
			ret = -EACCES;
		}
	}

	return ret;
}
#endif
