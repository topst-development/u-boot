// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Broadcom Corporation.
 */
/*
 * Modified by Telechips Inc. (date: 2020-09)
 */

#include <config.h>
#include <common.h>
#include <blk.h>
#include <env.h>
#include <fastboot.h>
#include <fastboot-internal.h>
#include <fb_ufs.h>
#include <image-sparse.h>
#include <part.h>
//#include <mmc.h>
#include <div64.h>
#include <linux/compat.h>
#include <android_image.h>

//#define FASTBOOT_MAX_BLK_WRITE_UFS 16384
#define FASTBOOT_MAX_BLK_WRITE_UFS (16U*50U)
#define BOOT_PARTITION_NAME "boot"

struct fb_ufs_sparse {
	struct blk_desc	*dev_desc;
};

static int part_get_info_by_name_or_alias(struct blk_desc *dev_desc,
		const char *name, disk_partition_t *info)
{
	int ret;

	ret = part_get_info_by_name(dev_desc, name, info);
	if (ret < 0) {
		/* strlen("fastboot_partition_alias_") + PART_NAME_LEN + 1 */
		char env_alias_name[25 + PART_NAME_LEN + 1];
		const char *aliased_part_name;

		/* check for alias */
		strcpy(env_alias_name, "fastboot_partition_alias_");
		strncat(env_alias_name, name, PART_NAME_LEN);
		aliased_part_name = env_get(env_alias_name);
		if (aliased_part_name != NULL) {
			ret = part_get_info_by_name(dev_desc,
					aliased_part_name, info);
		}
	}
	return ret;
}

/**
 * fb_ufs_blk_write() - Write/erase UFS in chunks of FASTBOOT_MAX_BLK_WRITE
 *
 * @block_dev: Pointer to block device
 * @start: First block to write/erase
 * @blkcnt: Count of blocks
 * @buffer: Pointer to data buffer for write or NULL for erase
 */
static lbaint_t fb_ufs_blk_write(struct blk_desc *block_dev, lbaint_t start,
				 lbaint_t blkcnt, const void *buffer)
{
	lbaint_t blk = start;
	lbaint_t blks_written;
	lbaint_t cur_blkcnt;
	lbaint_t blks = 0;
#if 0
	lbaint_t blks_reads;
	void *read_buffer = 0x40000000;
	int ret = 0;
#endif
	int i;

	for (i = 0; i < blkcnt; i += (int)FASTBOOT_MAX_BLK_WRITE_UFS) {
		cur_blkcnt = min(((int)blkcnt - i), (int)FASTBOOT_MAX_BLK_WRITE_UFS);
		if (buffer != NULL) {
			if (fastboot_progress_callback != NULL) {
				fastboot_progress_callback("writing");
			}
			blks_written = blk_dwrite(block_dev, blk, cur_blkcnt,
					  buffer + (i * block_dev->blksz));
		} else {
			if (fastboot_progress_callback != NULL) {
				fastboot_progress_callback("erasing");
			}
			blks_written = blk_derase(block_dev, blk, cur_blkcnt);
		}
		blk += blks_written;
		blks += blks_written;
	}
	return blks;
}

static lbaint_t fb_ufs_sparse_write(struct sparse_storage *info,
		lbaint_t blk, lbaint_t blkcnt, const void *buffer)
{
	struct fb_ufs_sparse *sparse = info->priv;
	struct blk_desc *dev_desc = sparse->dev_desc;

	return fb_ufs_blk_write(dev_desc, blk, blkcnt, buffer);
}

static lbaint_t fb_ufs_sparse_reserve(struct sparse_storage *info,
		lbaint_t blk, lbaint_t blkcnt)
{
	(void)blk;
	return blkcnt;
}

static void write_raw_image(struct blk_desc *dev_desc, disk_partition_t *info,
		const char *part_name, void *buffer,
		u32 download_bytes, char *response)
{
	lbaint_t blkcnt;
	lbaint_t blks;

	/* determine number of blocks to write */
	blkcnt = ((download_bytes + (info->blksz - 1U)) & ~(info->blksz - 1U));
	blkcnt = lldiv(blkcnt, info->blksz);

	if (blkcnt > info->size) {
		pr_err("too large for partition: '%s'\n", part_name);
		fastboot_fail("too large for partition", response);
	} else {

		pr_info("Flashing Raw Image\n");

		blks = fb_ufs_blk_write(dev_desc, info->start, blkcnt, buffer);

		if (blks != blkcnt) {
			pr_err("failed writing to device %d\n", dev_desc->devnum);
			fastboot_fail("failed writing to device", response);
		} else {
			pr_info("........ wrote " LBAFU " bytes to '%s'\n", blkcnt * info->blksz,
					part_name);
			fastboot_okay(NULL, response);
		}
	}
}

#ifdef CONFIG_ANDROID_BOOT_IMAGE
/**
 * Read Android boot image header from boot partition.
 *
 * @param[in] dev_desc MMC device descriptor
 * @param[in] info Boot partition info
 * @param[out] hdr Where to store read boot image header
 *
 * @return Boot image header sectors count or 0 on error
 */
static lbaint_t fb_ufs_get_boot_header(struct blk_desc *dev_desc,
				       disk_partition_t *info,
				       struct andr_img_hdr *hdr,
				       char *response)
{
	ulong sector_size;		/* boot partition sector size */
	lbaint_t hdr_sectors;		/* boot image header sectors count */
	int res;

	/* Calculate boot image sectors count */
	sector_size = info->blksz;
	hdr_sectors = DIV_ROUND_UP(sizeof(struct andr_img_hdr), sector_size);
	if (hdr_sectors == 0U) {
		pr_err("invalid number of boot sectors: 0\n");
		fastboot_fail("invalid number of boot sectors: 0", response);
	} else {
		/* Read the boot image header */
		res = blk_dread(dev_desc, info->start, hdr_sectors, (void *)hdr);
		if (res != (int)hdr_sectors) {
			pr_err("cannot read header from boot partition\n");
			fastboot_fail("cannot read header from boot partition",
					response);
			hdr_sectors = 0;
		} else {
			/* Check boot header magic string */
			res = android_image_check_header(hdr);
			if (res != 0) {
				pr_err("bad boot image magic\n");
				fastboot_fail("boot partition not initialized", response);
				hdr_sectors = 0;
			}
		}
	}
	return hdr_sectors;
}

/**
 * Write downloaded zImage to boot partition and repack it properly.
 *
 * @param dev_desc MMC device descriptor
 * @param download_buffer Address to fastboot buffer with zImage in it
 * @param download_bytes Size of fastboot buffer, in bytes
 *
 * @return 0 on success or -1 on error
 */
static int fb_ufs_update_zimage(struct blk_desc *dev_desc,
				void *download_buffer,
				u32 download_bytes,
				char *response)
{
	uintptr_t hdr_addr;			/* boot image header address */
	struct andr_img_hdr *hdr;		/* boot image header */
	lbaint_t hdr_sectors;			/* boot image header sectors */
	u8 *ramdisk_buffer;
	u32 ramdisk_sector_start;
	u32 ramdisk_sectors;
	u32 kernel_sector_start;
	u32 kernel_sectors;
	u32 sectors_per_page;
	disk_partition_t info;
	int res;

	pr_info("Flashing zImage\n");

	/* Get boot partition info */
	res = part_get_info_by_name(dev_desc, BOOT_PARTITION_NAME, &info);
	if (res < 0) {
		pr_err("cannot find boot partition\n");
		fastboot_fail("cannot find boot partition", response);
		res = -1;
		goto error;
	}

	/* Put boot image header in fastboot buffer after downloaded zImage */
	hdr_addr = (uintptr_t)download_buffer + ALIGN(download_bytes, PAGE_SIZE);
	hdr = (struct andr_img_hdr *)hdr_addr;

	/* Read boot image header */
	hdr_sectors = fb_ufs_get_boot_header(dev_desc, &info, hdr, response);
	if (hdr_sectors == 0U) {
		pr_err("unable to read boot image header\n");
		fastboot_fail("unable to read boot image header", response);
		res = -1;
		goto error;
	}

	/* Check if boot image has second stage in it (we don't support it) */
	if (hdr->second_size > 0U) {
		pr_err("moving second stage is not supported yet\n");
		fastboot_fail("moving second stage is not supported yet",
			      response);
		res = -1;
		goto error;
	}

	/* Extract ramdisk location */
	sectors_per_page = hdr->page_size / info.blksz;
	ramdisk_sector_start = info.start + sectors_per_page;
	ramdisk_sector_start += DIV_ROUND_UP(hdr->kernel_size, hdr->page_size) *
					     sectors_per_page;
	ramdisk_sectors = DIV_ROUND_UP(hdr->ramdisk_size, hdr->page_size) *
				       sectors_per_page;

	/* Read ramdisk and put it in fastboot buffer after boot image header */
	ramdisk_buffer = (u8 *)hdr + (hdr_sectors * info.blksz);
	res = blk_dread(dev_desc, ramdisk_sector_start, ramdisk_sectors,
			ramdisk_buffer);
	if (res != (int)ramdisk_sectors) {
		pr_err("cannot read ramdisk from boot partition\n");
		fastboot_fail("cannot read ramdisk from boot partition",
			      response);
		res = -1;
		goto error;
	}

	/* Write new kernel size to boot image header */
	hdr->kernel_size = download_bytes;
	res = blk_dwrite(dev_desc, info.start, hdr_sectors, (void *)hdr);
	if (res == 0) {
		pr_err("cannot writeback boot image header\n");
		fastboot_fail("cannot write back boot image header", response);
		res = -1;
		goto error;
	}

	/* Write the new downloaded kernel */
	kernel_sector_start = info.start + sectors_per_page;
	kernel_sectors = DIV_ROUND_UP(hdr->kernel_size, hdr->page_size) *
				      sectors_per_page;
	res = blk_dwrite(dev_desc, kernel_sector_start, kernel_sectors,
			 download_buffer);
	if (res == 0) {
		pr_err("cannot write new kernel\n");
		fastboot_fail("cannot write new kernel", response);
		res = -1;
		goto error;
	}

	/* Write the saved ramdisk back */
	ramdisk_sector_start = info.start + sectors_per_page;
	ramdisk_sector_start += DIV_ROUND_UP(hdr->kernel_size, hdr->page_size) *
					     sectors_per_page;
	res = blk_dwrite(dev_desc, ramdisk_sector_start, ramdisk_sectors,
			 ramdisk_buffer);
	if (res == 0) {
		pr_err("cannot write back original ramdisk\n");
		fastboot_fail("cannot write back original ramdisk", response);
		res = -1;
		goto error;
	}

	pr_info("........ zImage was updated in boot partition\n");
	fastboot_okay(NULL, response);
	res = 0;

error:
	return res;
}
#endif

/**
 * fastboot_ufs_get_part_info() - Lookup ufs partion by name
 *
 * @part_name: Named partition to lookup
 * @dev_desc: Pointer to returned blk_desc pointer
 * @part_info: Pointer to returned disk_partition_t
 * @response: Pointer to fastboot response buffer
 */
int fastboot_ufs_get_part_info(const char *part_name,
			       struct blk_desc **dev_desc,
			       disk_partition_t *part_info, char *response)
{
	int r;

	*dev_desc = blk_get_dev("scsi", CONFIG_FASTBOOT_FLASH_UFS_DEV);
	if (*dev_desc == NULL) {
		fastboot_fail("block device not found", response);
		r = -ENOENT;
	} else {
		if ((part_name == NULL) || (strcmp(part_name, "") == 0)) {
			fastboot_fail("partition not given", response);
			r = -ENOENT;
		} else {
			r = part_get_info_by_name_or_alias(*dev_desc, part_name, part_info);
			if (r < 0) {
				fastboot_fail("partition not found", response);
			}
		}
	}

	return r;
}

/**
 * fastboot_ufs_flash_write() - Write image to ufs for fastboot
 *
 * @cmd: Named partition to write image to
 * @download_buffer: Pointer to image data
 * @download_bytes: Size of image data
 * @response: Pointer to fastboot response buffer
 */
void fastboot_ufs_flash_write(const char *cmd, void *download_buffer,
			      u32 download_bytes, char *response)
{
	struct blk_desc *dev_desc;
	disk_partition_t info;

	dev_desc = blk_get_dev("scsi", CONFIG_FASTBOOT_FLASH_UFS_DEV);
	if ((dev_desc == NULL) || (dev_desc->type == (unsigned char)DEV_TYPE_UNKNOWN)) {
		pr_err("invalid ufs device\n");
		fastboot_fail("invalid ufs device", response);
		goto out;
	}

#if CONFIG_IS_ENABLED(EFI_PARTITION)
	if (strcmp(cmd, CONFIG_FASTBOOT_GPT_NAME) == 0) {
		pr_info("%s: updating MBR, Primary and Backup GPT(s)\n",
		       __func__);
		if (is_valid_gpt_buf(dev_desc, download_buffer) != 0) {
			pr_info("%s: invalid GPT - refusing to write to flash\n",
			       __func__);
			fastboot_fail("invalid GPT partition", response);
			goto out;
		}
		if (write_mbr_and_gpt_partitions(dev_desc, download_buffer) != 0) {
			pr_info("%s: writing GPT partitions failed\n", __func__);
			fastboot_fail("writing GPT partitions failed",
				      response);
			goto out;
		}
		pr_info("........ success\n");
		fastboot_okay(NULL, response);
		goto out;
	}
#endif

#if CONFIG_IS_ENABLED(DOS_PARTITION)
	if (strcmp(cmd, CONFIG_FASTBOOT_MBR_NAME) == 0) {
		pr_info("%s: updating MBR\n", __func__);
		if (is_valid_dos_buf(download_buffer) != 0) {
			pr_info("%s: invalid MBR - refusing to write to flash\n",
			       __func__);
			fastboot_fail("invalid MBR partition", response);
			goto out;
		}
		if (write_mbr_partition(dev_desc, download_buffer) != 0) {
			pr_info("%s: writing MBR partition failed\n", __func__);
			fastboot_fail("writing MBR partition failed",
				      response);
			goto out;
		}
		pr_info("........ success\n");
		fastboot_okay(NULL, response);
		goto out;
	}
#endif

#ifdef CONFIG_ANDROID_BOOT_IMAGE
	if (strncasecmp(cmd, "zimage", 6) == 0) {
		fb_ufs_update_zimage(dev_desc, download_buffer,
				     download_bytes, response);
		goto out;
	}
#endif

	if (part_get_info_by_name_or_alias(dev_desc, cmd, &info) < 0) {
		pr_err("cannot find partition: '%s'\n", cmd);
		fastboot_fail("cannot find partition", response);
		goto out;
	}

	if (is_sparse_image(download_buffer) != 0) {
		struct fb_ufs_sparse sparse_priv;
		struct sparse_storage sparse;
		int err;

		sparse_priv.dev_desc = dev_desc;

		sparse.blksz = info.blksz;
		sparse.start = info.start;
		sparse.size = info.size;
		sparse.write = fb_ufs_sparse_write;
		sparse.reserve = fb_ufs_sparse_reserve;
		sparse.mssg = fastboot_fail;

		pr_info("Flashing sparse image at offset " LBAFU "\n",
		       sparse.start);

		sparse.priv = &sparse_priv;
		err = write_sparse_image(&sparse, cmd, download_buffer,
					 response);
		if (err != 0) {
			fastboot_okay(NULL, response);
		}
	} else {
		write_raw_image(dev_desc, &info, cmd, download_buffer,
				download_bytes, response);
	}

out:
	return;
}

/**
 * fastboot_ufs_flash_erase() - Erase ufs for fastboot
 *
 * @cmd: Named partition to erase
 * @response: Pointer to fastboot response buffer
 */
void fastboot_ufs_erase(const char *cmd, char *response)
{
	int ret;
	struct blk_desc *dev_desc;
	disk_partition_t info;
	lbaint_t blks, blks_start, blks_size/*, grp_size*/;
#if 0
	struct ufs *mmc = find_mmc_device(CONFIG_FASTBOOT_FLASH_MMC_DEV);

	if (mmc == NULL) {
		pr_err("invalid mmc device\n");
		fastboot_fail("invalid mmc device", response);
		return;
	}
#endif
	dev_desc = blk_get_dev("scsi", CONFIG_FASTBOOT_FLASH_UFS_DEV);
	if ((dev_desc == NULL) || (dev_desc->type == (unsigned char)DEV_TYPE_UNKNOWN)) {
		pr_err("invalid ufs device\n");
		fastboot_fail("invalid mmc device", response);
	} else {
		ret = part_get_info_by_name_or_alias(dev_desc, cmd, &info);
		if (ret < 0) {
			pr_err("cannot find partition: '%s'\n", cmd);
			fastboot_fail("cannot find partition", response);
		} else {
			/* Align blocks to erase group size to avoid erasing other partitions */
			//grp_size = 1;//mmc->erase_grp_size;
			blks_start = info.start;
			blks_size = info.size;

			pr_info("Erasing blocks " LBAFU " to " LBAFU " due to alignment\n",
					blks_start, blks_start + blks_size);

			blks = fb_ufs_blk_write(dev_desc, blks_start, blks_size, NULL);

			if (blks != blks_size) {
				pr_err("failed erasing from device %d\n", dev_desc->devnum);
				fastboot_fail("failed erasing from device", response);
			} else {
				pr_info("........ erased " LBAFU " bytes from '%s'\n",
						blks_size * info.blksz, cmd);

				fastboot_okay(NULL, response);
			}
		}
	}
}
