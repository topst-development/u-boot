/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2017 The Android Open Source Project
 */

/*
 * Modified by Telechips Inc. (date: 2020-07)
 */

#ifndef __AB_UPDATE_H
#define __AB_UPDATE_H

#include <common.h>

/* Boot slot names are 'a', 'b', 'c', ... */
#define BOOT_SLOT_NAME(slot_num) ('a' + (slot_num))
#if defined(CONFIG_BL3_AB_UPDATE)
#define BOOT_SLOT_NUM(slot_name) ((slot_name) - 'a')
#endif

/* Number of slots */
#define NUM_SLOTS 2

/**
 * Select the slot where to boot from.
 *
 * Devices with more than one boot slot (multiple copies of the kernel and
 * system images) selects which slot should be used to boot from and registers
 * the boot attempt. This is used in by the new A/B update model where one slot
 * is updated in the background while running from the other slot. If
 * the selected slot did not successfully boot in the past, a boot attempt is
 * registered before returning from this function so it isn't selected
 * indefinitely.
 *
 * @param[in] dev_desc Place to store the device description pointer
 * @param[in] part_info Place to store the partition information
 * @return The slot number (>= 0) on success, or a negative on error
 */
int ab_select_slot(struct blk_desc *dev_desc, disk_partition_t *part_info,
		   bool store);
int ab_set_active_slot(struct blk_desc *dev_desc, disk_partition_t *part_info,
		       int slot);

#endif /* __AB_UPDATE_H */
