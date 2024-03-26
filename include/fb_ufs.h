/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Broadcom Corporation.
 */
/*
 * Modified by Telechips Inc. (date: 2021-03)
 */

#ifndef FB_UFS_H_
#define FB_UFS_H_
#include <part.h>
/**
 * fastboot_ufs_get_part_info() - Lookup UFS partion by name
 *
 * @part_name: Named partition to lookup
 * @dev_desc: Pointer to returned blk_desc pointer
 * @disk_part_info: Pointer to returned struct disk_partition
 * @response: Pointer to fastboot response buffer
 */
int fastboot_ufs_get_part_info(const char *part_name,
			       struct blk_desc **dev_desc,
			       struct disk_partition *disk_part_info, char *response);

/**
 * fastboot_ufs_flash_write() - Write image to UFS for fastboot
 *
 * @cmd: Named partition to write image to
 * @download_buffer: Pointer to image data
 * @download_bytes: Size of image data
 * @response: Pointer to fastboot response buffer
 */
void fastboot_ufs_flash_write(const char *cmd, void *download_buffer,
			      u32 download_bytes, char *response);
/**
 * fastboot_ufs_flash_erase() - Erase UFS for fastboot
 *
 * @cmd: Named partition to erase
 * @response: Pointer to fastboot response buffer
 */
void fastboot_ufs_erase(const char *cmd, char *response);
#endif
