// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <part.h>
#ifdef CONFIG_JPEG_LOGO_IMAGE_SUPPORT
#include <lcd.h>
#include <tcc_jpu_c6.h>
#endif //CONFIG_JPEG_LOGO_IMAGE_SUPPORT
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

#ifdef CONFIG_JPEG_LOGO_IMAGE_SUPPORT
int splash_image_decode_jpeg(unsigned int input_stream_size_,
			     jpu_dec_outputs_t *jpu_out);
int jpu_c6_decoder(jpu_dec_inits_t *init_jpu, jpu_dec_outputs_t *out_jpu);
extern int _vmem_is_cma_allocated_virt_region(unsigned int start_virtaddr,
					      unsigned int length);

int jpu_c6_decoder(jpu_dec_inits_t *init_jpu, jpu_dec_outputs_t *out_jpu)
{
	unsigned int ret = 0;
	int dec_handle = 0;
	int idx;
	codec_addr_t *pAddrCur;

	jpu_dec_init_t dec_init = {
		0,
	};
	jpu_dec_initial_info_t dec_initial_info = {
		0,
	};
	jpu_dec_input_t dec_input = {
		0,
	};
	jpu_dec_output_t dec_output = {
		0,
	};
	jpu_dec_buffer_t dec_buffer = { 0 };
	codec_addr_t frame_buf_addr[2] = {
		0,
	};
	codec_addr_t bitstream_buf_addr[2] = {
		0,
	};

	dec_init.m_iBitstreamBufSize = 0x100000;
	dec_init.m_RegBaseVirtualAddr = init_jpu->register_base_virtual_address;
	dec_init.m_BitstreamBufAddr[PA] =
		init_jpu->memory_base_physical_address;
	dec_init.m_BitstreamBufAddr[VA] = init_jpu->memory_base_virtual_address;
	dec_init.m_iCbCrInterleaveMode = 0;
	dec_init.m_uiDecOptFlags = 0;

	dec_init.m_Memcpy = init_jpu->m_Memcpy;
	dec_init.m_Memset = init_jpu->m_Memset;
	dec_init.m_Interrupt = init_jpu->m_Interrupt;
	dec_init.m_reg_read = init_jpu->m_reg_read;
	dec_init.m_reg_write = init_jpu->m_reg_write;

	ret = TCC_JPU_DEC(JPU_DEC_INIT, &dec_handle, &dec_init, NULL);
	if (ret != JPG_RET_SUCCESS) {
		dprintf("[dec_test] JPU_DEC_INIT failed Error code is 0x%x \r\n",
			ret);
		goto ERR_DEC_OPEN;
	}

	ret = TCC_JPU_DEC(JPU_DEC_SEQ_HEADER, &dec_handle,
			  (void *)init_jpu->input_stream_size,
			  &dec_initial_info);
	if (ret != JPG_RET_SUCCESS) {
		dprintf("[dec_test] JPU_DEC_SEQ_HEADER failed Error code is 0x%x, error reason is 0x%x \r\n",
			ret, dec_initial_info.m_iErrorReason);
		goto ERR_DEC_OPEN;
	}

	dprintf("dec_initial_info.m_iSourceFormat = %d\n",
		dec_initial_info.m_iSourceFormat);
	out_jpu->m_iYUVFormat = dec_initial_info.m_iSourceFormat;

	// frame_buf_addr[PA] = init_jpu->memory_base_physical_address + 0x100000;
	frame_buf_addr[PA] = CONFIG_FB_ADDR;
	if (frame_buf_addr[PA] == 0) {
		dprintf("frame_buf_addr[PA] malloc() failed \r\n");
		goto ERR_DEC_OPEN;
	}
	//PRINTF("frame_buf_addr[PA] = 0x%x\r\n", (codec_addr_t)frame_buf_addr[PA]);

	// frame_buf_addr[VA] = init_jpu->memory_base_virtual_address+0x100000;
	frame_buf_addr[VA] = CONFIG_FB_ADDR;
	if (frame_buf_addr[VA] == 0) {
		dprintf("frame_buf_addr[VA] malloc() failed \r\n");
		goto ERR_DEC_OPEN;
	}
	//PRINTF("frame_buf_addr[VA] = 0x%x\r\n", (codec_addr_t)frame_buf_addr[VA]);

	dec_buffer.m_FrameBufferStartAddr[PA] = frame_buf_addr[PA];
	dec_buffer.m_FrameBufferStartAddr[VA] = frame_buf_addr[VA];
	dec_buffer.m_iFrameBufferCount =
		dec_initial_info.m_iMinFrameBufferCount;
	dec_buffer.m_iJPGScaleRatio = 0;

	ret = TCC_JPU_DEC(JPU_DEC_REG_FRAME_BUFFER, &dec_handle, &dec_buffer,
			  0);
	if (ret != JPG_RET_SUCCESS) {
		dprintf("JPU_DEC_REG_FRAME_BUFFER failed Error code is 0x%x \r\n\n",
			ret);
		goto ERR_DEC_OPEN;
	}
	//PRINTF("TCC_JPU_DEC JPU_DEC_REG_FRAME_BUFFER OK!\r\n");

	dec_input.m_BitstreamDataAddr[PA] =
		init_jpu->memory_base_physical_address;
	dec_input.m_BitstreamDataAddr[VA] =
		init_jpu->memory_base_virtual_address;
	dec_input.m_iBitstreamDataSize = init_jpu->input_stream_size;

	ret = TCC_JPU_DEC(JPU_DEC_DECODE, &dec_handle, &dec_input, &dec_output);
	if (ret == JPG_RET_FAILURE) {
		/* JPU Decorder error */
		goto ERR_DEC_OPEN;
	}
	if (ret == JPG_RET_CODEC_EXIT) {
		/* JPU Decorder error */
		goto ERR_DEC_OPEN;
	}

	out_jpu->m_iWidth = dec_output.m_DecOutInfo.m_iWidth;
	out_jpu->m_iHeight = dec_output.m_DecOutInfo.m_iHeight;
	out_jpu->m_pCurrOut[0][0] = dec_output.m_pCurrOut[0][0];
	out_jpu->m_pCurrOut[0][1] = dec_output.m_pCurrOut[0][1];
	out_jpu->m_pCurrOut[0][2] = dec_output.m_pCurrOut[0][2];
	out_jpu->m_pCurrOut[1][0] = dec_output.m_pCurrOut[1][0];
	out_jpu->m_pCurrOut[1][1] = dec_output.m_pCurrOut[1][1];
	out_jpu->m_pCurrOut[1][2] = dec_output.m_pCurrOut[1][2];

ERR_DEC_OPEN:
	// Now that we are done with decoding, close the open instance.
	ret = TCC_JPU_DEC(JPU_DEC_CLOSE, &dec_handle, 0, 0);

	return ret;
}

int splash_image_decode_jpeg(unsigned int input_stream_size_,
			     jpu_dec_outputs_t *jpu_out)
{
	int ret = 0;

	jpu_dec_inits_t jpu_init = {
		0,
	};
	//	jpu_dec_outputs_t jpu_out = {0,};
	codec_addr_t register_base_physical_address = TCC_JPEG_EDCODER_BASE;
	codec_addr_t register_base_virtual_address = TCC_JPEG_EDCODER_BASE;
	codec_addr_t memory_base_physical_address =
		TCC_JPEG_DECODE_BASE; // 0x8cc00000;
	codec_addr_t memory_base_virtual_address =
		TCC_JPEG_DECODE_BASE; // 0x8cc00000;
	int input_stream_size =
		(int)input_stream_size_; // decoding CO jpeg stream size

	// dprintf(INFO, "==== %s() in\n", __func__);

	memset(jpu_out, 0, sizeof(jpu_dec_outputs_t));

	jpu_init.input_stream_size = input_stream_size;
	jpu_init.register_base_virtual_address = register_base_virtual_address;
	jpu_init.memory_base_physical_address = memory_base_physical_address;
	jpu_init.memory_base_virtual_address = memory_base_virtual_address;
	jpu_init.m_Memset =
		vetc_memset; // (void  (*) ( void*, int, unsigned int ))sys_memset;
	jpu_init.m_Memcpy =
		vetc_memcpy; // (void* (*) ( void*, const void*, unsigned int ))sys_memcpy;
	jpu_init.m_reg_read = vetc_reg_read;
	jpu_init.m_reg_write = vetc_reg_write;
	jpu_init.m_Interrupt = NULL; // (int  (*) ( void ))sys_test;

	dprintf("input stream size = %d\n", input_stream_size);
	dprintf("==== register_base_virtual_address(0x%x)\n==== memory_base_physical_address(0x%x)\n",
		jpu_init.register_base_virtual_address,
		jpu_init.memory_base_physical_address);

	ret = jpu_c6_decoder(&jpu_init, jpu_out);
	if (ret != 0)
		dprintf("jpu error : %d\n", ret);

	dprintf("==== width x height(%dx%d), YUV format(%d), m_pCurrOut(0x%x)\n",
		jpu_out->m_iWidth, jpu_out->m_iHeight, jpu_out->m_iYUVFormat,
		jpu_out->m_pCurrOut[0][0]);
	return ret;
}
#endif //CONFIG_JPEG_LOGO_IMAGE_SUPPORT

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
	disk_partition_t part_info;
	unsigned int idxPartition = 0, nPartition = 50;
	char strPartition[4] = {
		0,
	};
	int uiPartition;
	int uiStartBlock = 0;
	int uiBlockCount;
	int uiImageIndex = 0;

	struct SPLASH_IMAGE_INFO *p_header_info;
#ifdef CONFIG_JPEG_LOGO_IMAGE_SUPPORT
	jpu_dec_outputs_t jpu_out = {
		0,
	};
#endif //CONFIG_JPEG_LOGO_IMAGE_SUPPORT

	dprintf("Splash Image name : %s\n", pucImageName);

	// allocate memory for splash header
	splash_hdr = malloc(FLASH_PAGE_SIZE);

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
#ifdef CONFIG_JPEG_LOGO_IMAGE_SUPPORT
		dprintf("splash image format is JPEG\n");
		blk_dread(dev_desc,
			  uiStartBlock + BLOCK_CNT(p_header_info->uiImageAddr,
						   dev_desc),
			  BLOCK_CNT(p_header_info->uiImageSize, dev_desc),
			  TCC_JPEG_DECODE_BASE);
		splash_image_decode_jpeg(p_header_info->uiImageSize, &jpu_out);

		// set yuv-format of decoded image
		if (fmt) {
			switch (jpu_out.m_iYUVFormat) {
			case 0: //yuv420
				*fmt = TCC_LCDC_IMG_FMT_YUV420SP;
				break;
			case 1: //yuv422
				*fmt = TCC_LCDC_IMG_FMT_YUV422SP;
				break;
			case 3: // yuv444
				*fmt = TCC_LCDC_IMG_FMT_444SEP;
				break;
			default:
				dprintf("unknown format: %d\n",
				       jpu_out.m_iYUVFormat);
			}
		}
#else
		dprintf("JPEG file is used but the bootloader is not supported. Wrong compilation!\n");
#endif
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
