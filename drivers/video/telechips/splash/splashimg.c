// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <part.h>
#include <malloc.h>
#include <memalign.h>

#include "splashimg.h"

#if IS_ENABLED(CONFIG_UFS_BOOT)
#define FLASH_PAGE_SIZE 4096
#define SPLASH_PARTITION_PRESTR "2:%x"
#define SPLASH_PARTITION_BLKDEV "scsi"
#else
#define FLASH_PAGE_SIZE 512
#define SPLASH_PARTITION_PRESTR "0:%x"
#define SPLASH_PARTITION_BLKDEV "mmc"
#endif

// #define DGB_SPLASH

#if defined(DGB_SPLASH)
#define dprintf pr_debug
#else
#define dprintf(fmt, ...)
#endif

int get_splash_index(SPLASH_IMAGE_Header_info_t *splash_hdr,
		     const unsigned char *pucImageName)
{
	unsigned int i = 0;

	for (i = 0; i < splash_hdr->uiNumber; i++) {
		if (!strcmp((char *)(splash_hdr->SPLASH_IMAGE[i].ucImageName),
			    (char *)(pucImageName))) {
			return i;
		}
	}

	return -1;
}

void print_splash_header(SPLASH_IMAGE_Header_info_t *splash_hdr)
{
	unsigned int i;

	dprintf("\x1b[1;33m[%s:%d] ++\x1b[0m\n", __func__, __LINE__);

	dprintf("\x1b[1;33m[%s:%d] splash tag : %s \x1b[0m\n", __func__,
		__LINE__, splash_hdr->ucPartition);
	dprintf("\x1b[1;33m[%s:%d] Image num : %d \x1b[0m\n", __func__,
		__LINE__, splash_hdr->uiNumber);

	for (i = 0; i < splash_hdr->uiNumber; i++) {
		dprintf("\x1b[1;31m[%s:%d] image index  : %d \x1b[0m\n",
			__func__, __LINE__, i);
		dprintf("\x1b[1;32m[%s:%d] image name   : %s \x1b[0m\n",
			__func__, __LINE__,
			splash_hdr->SPLASH_IMAGE[i].ucImageName);
		dprintf("\x1b[1;32m[%s:%d] Start addr   : %d \x1b[0m\n",
			__func__, __LINE__,
			splash_hdr->SPLASH_IMAGE[i].uiImageAddr);
		dprintf("\x1b[1;32m[%s:%d] image size   : %d \x1b[0m\n",
			__func__, __LINE__,
			splash_hdr->SPLASH_IMAGE[i].uiImageSize);
		dprintf("\x1b[1;32m[%s:%d] image width  : %d \x1b[0m\n",
			__func__, __LINE__,
			splash_hdr->SPLASH_IMAGE[i].uiImageWidth);
		dprintf("\x1b[1;32m[%s:%d] image height : %d \x1b[0m\n",
			__func__, __LINE__,
			splash_hdr->SPLASH_IMAGE[i].uiImageHeight);
	}

	dprintf("\x1b[1;33m[%s:%d] --\x1b[0m\n", __func__, __LINE__);
}

int splash_image_load_v2(const unsigned char *pucPartitionName,
			 const unsigned char *pucImageName, void *pvBuffer,
			 unsigned int *width, unsigned int *height,
			 unsigned int *fmt)
{
	SPLASH_IMAGE_Header_info_t *splash_hdr;
	struct blk_desc *dev_desc;
	struct disk_partition part_info;
	unsigned int idxPartition = 0, nPartition = 50;
	char strPartition[4] = {
		0,
	};
	int uiPartition;
	int uiStartBlock = 0;
	int uiBlockCount;
	int uiImageIndex = 0;

	struct SPLASH_IMAGE_INFO *p_header_info;

	dprintf("Splash Image name : %s\n", pucImageName);

	// allocate memory for splash header
	splash_hdr = memalign(ARCH_DMA_MINALIGN, FLASH_PAGE_SIZE);

	// get splash partition info
	for (idxPartition = 1; idxPartition < nPartition; idxPartition++) {
		sprintf((char *)&strPartition, SPLASH_PARTITION_PRESTR,
			idxPartition); // device:partition_index
		uiPartition = blk_get_device_part_str(SPLASH_PARTITION_BLKDEV,
			strPartition, &dev_desc, &part_info, 1);
		if (uiPartition < 0) {
			dprintf("ERROR : no splash partition found!\n");
			break;
		}

		// read splash image header
		uiStartBlock = part_info.start;

		uiBlockCount = blk_dread(dev_desc, uiStartBlock,
					 BLOCK_CNT(FLASH_PAGE_SIZE, dev_desc),
					 (void *)splash_hdr);
		if (!uiBlockCount) {
			dprintf("ERROR : splash header read fail!\n");
			continue; //return -1;
		}

		if (strcmp((char *)splash_hdr->ucPartition,
			   (char *)pucPartitionName)) {
			if (strstr((char *)splash_hdr->ucPartition,
				   "splash") != NULL) {
				pr_err("[ERROR] Invalid splash partition name: ");
				pr_err("expect: %s", pucPartitionName);
				pr_err(", actual: %s\n",
				       splash_hdr->ucPartition);
			}
			continue; //return -1;
		}

		if (idxPartition == (nPartition - 1)) {
			dprintf("ERROR: failed to get the splash partition\n");
			return -1;
		} else {
			dprintf("The splash partition index is %d\n",
				idxPartition);
			break;
		}
	}

	if (splash_hdr->uiNumber < 0 || splash_hdr->uiNumber > 255) {
		dprintf("ERROR : there is no data in splash partition\n");
		return -1;
	}

	// get logo index
	uiImageIndex = get_splash_index(splash_hdr, pucImageName);
	dprintf("splash image index = %d\n", uiImageIndex);

	if (uiImageIndex < 0) {
		dprintf("ERROR : there is no image from [%s]\n", pucImageName);
		return -1;
	}

	p_header_info = &(splash_hdr->SPLASH_IMAGE[uiImageIndex]);

	if (width) {
		/* UI image width */
		*width = p_header_info->uiImageWidth;
	}

	if (height) {
		/* UI image height */
		*height = p_header_info->uiImageHeight;
	}

	dprintf("uiImageWidth = %d\n", p_header_info->uiImageWidth);
	dprintf("uiImageHeight = %d\n", p_header_info->uiImageHeight);
	dprintf("splash width = %d, height = %d\n", *width, *height);
	dprintf("splash image size = %d\n", p_header_info->uiImageSize);
	dprintf("splash image name = %s\n", p_header_info->ucImageName);
	dprintf("splash image addr = 0x%08x\n", p_header_info->uiImageAddr);

	if (!strcmp((const char *)p_header_info->ucFmt,
		    SPLASH_IMAGE_FMT_JPEG)) {
		dprintf("JPEG file is used but the bootloader is not supported. Wrong compilation!\n");
	} else {
		// By default, there are no other cases except for BMP and JPEG formats
		dprintf("splash image format is BMP\n");
		blk_dread(dev_desc,
			  uiStartBlock + BLOCK_CNT(p_header_info->uiImageAddr,
						   dev_desc),
			  BLOCK_CNT(p_header_info->uiImageSize, dev_desc),
			  pvBuffer);
	}

	return 0;
}
