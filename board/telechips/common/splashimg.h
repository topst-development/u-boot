// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef _SPLASHIMG_H_ /* 014.12.23 */
#define _SPLASHIMG_H_

#define IMAGE_SIZE_MAX 10
#define SPLASH_TAG "subcore_splash"
#define SPLASH_TAG_SIZE 16

#define SPLASH_IMAGE_WIDTH 800
#define SPLASH_IMAGE_HEIGHT 480

#define DEFAULT_HEADER_SIZE 512
#define DEFAULT_PAGE_SIZE 512

// List of values of supported image formats
#define SPLASH_IMAGE_FMT_BMP "BMP"
#define SPLASH_IMAGE_FMT_JPEG "JPEG"

typedef struct SPLASH_IMAGE_INFO {
	unsigned char ucImageName[16];
	unsigned int uiImageAddr;
	unsigned int uiImageSize;
	unsigned int uiImageWidth;
	unsigned int uiImageHeight;
	unsigned int Padding;
	unsigned char ucFmt[5];
	unsigned char ucRev[7];
} SPLASH_IMAGE_INFO_t;

typedef struct SPLASH_IMAGE_Header_info {
	unsigned char ucPartition[20];
	unsigned int uiNumber;
	unsigned char ucRev[4];
	SPLASH_IMAGE_INFO_t SPLASH_IMAGE[IMAGE_SIZE_MAX];
} SPLASH_IMAGE_Header_info_t;

typedef struct SPLASH_BUFFER {
	unsigned char *data;
	unsigned int size;
} SPLASH_BUFFER_t;

int splash_image_load(unsigned char *pucImageName, void *pvBuffer);
int splash_image_load_v2(const unsigned char *pucPartitionName,
			 const unsigned char *pucImageName, void *pvBuffer,
			 unsigned int *width, unsigned int *height,
			 unsigned int *fmt);

#endif /* _SPLASHIMG_H_ */
