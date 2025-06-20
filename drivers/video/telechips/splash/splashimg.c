// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <part.h>
#include <malloc.h>
#include <memalign.h>
#include <vsprintf.h>

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

static int get_splash_index(const SPLASH_IMAGE_Header_info_t *splash_hdr,
			    const unsigned char *pucImageName)
{
	unsigned int i = 0;
	int ret = -1;

	for (i = 0; i < splash_hdr->uiNumber; i++) {
		if (strcmp((const char *)splash_hdr->SPLASH_IMAGE[i].ucImageName,
			   (const char *)pucImageName) == 0) {
			ret = (int)i;
			break;
		}
	}

	return ret;
}

static SPLASH_IMAGE_Header_info_t *alloc_splash_header(void)
{
	SPLASH_IMAGE_Header_info_t *pHeader;

	// allocate memory for splash header
	pHeader = memalign(ARCH_DMA_MINALIGN, FLASH_PAGE_SIZE);

	if (pHeader != NULL) {
		(void)memset(pHeader, 0, sizeof(SPLASH_IMAGE_Header_info_t));
	}

	return pHeader;
}

static int get_splash_partition_info(struct blk_desc **dev_desc, lbaint_t *uiStartBlock,
				     SPLASH_IMAGE_Header_info_t *splash_hdr,
				     const unsigned char *pucPartitionName)
{
	unsigned int idxPartition = 0, nPartition = 50;
	int uiPartition;
	char strPartition[4] = {
		0,
	};
	struct disk_partition d_part_info;
	unsigned long uiBlockCount;
	int ret = 0;

	for (idxPartition = 1; idxPartition < nPartition; idxPartition++) {
		(void)sprintf((char *)&strPartition, SPLASH_PARTITION_PRESTR,
			      idxPartition); // device:partition_index
		uiPartition = blk_get_device_part_str(SPLASH_PARTITION_BLKDEV, strPartition,
						      dev_desc, &d_part_info, 1);
		if (uiPartition < 0) {
			debug_pr(LOGL_ERR, "ERROR : no splash partition found!\n");
			break;
		}

		// read splash image header
		*uiStartBlock = d_part_info.start;

		uiBlockCount = blk_dread(*dev_desc, *uiStartBlock,
					 BLOCK_CNT(FLASH_PAGE_SIZE, (*dev_desc)),
					 (void *)splash_hdr);
		if (uiBlockCount == 0u) {
			debug_pr(LOGL_INFO, "ERROR : splash header read fail!\n");
			continue; // return -1;
		}

		if (strcmp((const char *)splash_hdr->ucPartition, (const char *)pucPartitionName) !=
		    0) {
			if (strstr((char *)splash_hdr->ucPartition, "splash") != NULL) {
				debug_pr(LOGL_ERR, "[ERROR] Invalid splash partition name: ");
				debug_pr(LOGL_ERR, "expect: %s", pucPartitionName);
				debug_pr(LOGL_ERR, ", actual: %s\n", splash_hdr->ucPartition);
			}
			continue; // return -1;
		}

		if (idxPartition == (nPartition - 1u)) {
			debug_pr(LOGL_ERR, "ERROR: failed to get the splash partition\n");
			ret = -1;
			break;
		} else {
			debug_pr(LOGL_INFO, "The splash partition index is %d\n", idxPartition);
			break;
		}
	}

	if (splash_hdr->uiNumber > 255u) {
		debug_pr(LOGL_ERR, "ERROR : there is no data in splash partition\n");
		ret = -1;
	}

	return ret;
}

static void read_splash_data(const struct SPLASH_IMAGE_INFO *p_header_info,
			     struct blk_desc *dev_desc, lbaint_t uiStartBlock, unsigned int *width,
			     unsigned int *height, void *pvBuffer)
{
	if (width != NULL) {
		/* UI image width */
		*width = p_header_info->uiImageWidth;
		debug_pr(LOGL_INFO, "splash width = %d\n", *width);
	}

	if (height != NULL) {
		/* UI image height */
		*height = p_header_info->uiImageHeight;
		debug_pr(LOGL_INFO, "splash height = %d\n", *height);
	}

	debug_pr(LOGL_INFO, "uiImageWidth = %d\n", p_header_info->uiImageWidth);
	debug_pr(LOGL_INFO, "uiImageHeight = %d\n", p_header_info->uiImageHeight);
	debug_pr(LOGL_INFO, "splash image size = %d\n", p_header_info->uiImageSize);
	debug_pr(LOGL_INFO, "splash image name = %s\n", p_header_info->ucImageName);
	debug_pr(LOGL_INFO, "splash image addr = 0x%08x\n", p_header_info->uiImageAddr);

	if (strcmp((const char *)p_header_info->ucFmt, SPLASH_IMAGE_FMT_JPEG) == 0) {
		debug_pr(
			LOGL_INFO,
			"JPEG file is used but the bootloader is not supported. Wrong compilation!\n");
	} else {
		// By default, there are no other cases except for BMP and JPEG formats
		debug_pr(LOGL_INFO, "splash image format is BMP\n");
		(void)blk_dread(dev_desc,
				uiStartBlock + BLOCK_CNT(p_header_info->uiImageAddr, dev_desc),
				BLOCK_CNT(p_header_info->uiImageSize, dev_desc), pvBuffer);
	}
}

int splash_image_load_v2(const unsigned char *pucPartitionName, const unsigned char *pucImageName,
			 void *pvBuffer, unsigned int *width, unsigned int *height,
			 const unsigned int *fmt)
{
	SPLASH_IMAGE_Header_info_t *splash_hdr = NULL;
	struct blk_desc *dev_desc = NULL;
	lbaint_t uiStartBlock = 0;
	int uiImageIndex = 0;
	int ret = 0;

	const struct SPLASH_IMAGE_INFO *p_header_info;

	(void)fmt;

	debug_pr(LOGL_INFO, "Splash Image name : %s\n", pucImageName);

	splash_hdr = alloc_splash_header();

	if (splash_hdr == NULL) {
		debug_pr(LOGL_ERR, "splash_hdr allocate fail\n");
		ret = -1;
	}

	if (ret != -1) {
		ret = get_splash_partition_info(&dev_desc, &uiStartBlock, splash_hdr,
						pucPartitionName);
	}

	// get logo index
	if (ret != -1) {
		uiImageIndex = get_splash_index(splash_hdr, pucImageName);
		debug_pr(LOGL_INFO, "splash image index = %d\n", uiImageIndex);

		if (uiImageIndex < 0) {
			debug_pr(LOGL_ERR, "ERROR : there is no image from [%s]\n", pucImageName);
			ret = -1;
		}
	}

	if (ret != -1) {
		p_header_info = &(splash_hdr->SPLASH_IMAGE[uiImageIndex]);

		read_splash_data(p_header_info, dev_desc, uiStartBlock, width, height, pvBuffer);
	}

	if ((ret == -1) && (splash_hdr != NULL)) {
		free(splash_hdr);
	}

	return ret;
}
