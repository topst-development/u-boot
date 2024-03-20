/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Telechips Corporation.
 */

#ifndef _FB_UFS_H_
#define _FB_UFS_H_

/**
 * fastboot_ufs_get_part_info() - Lookup UFS partion by name
 *
 * @part_name: Named partition to lookup
 * @dev_desc: Pointer to returned blk_desc pointer
 * @part_info: Pointer to returned disk_partition_t
 * @response: Pointer to fastboot response buffer
 */
int fastboot_ufs_get_part_info(const char *part_name,
			       struct blk_desc **dev_desc,
			       disk_partition_t *part_info, char *response);

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
