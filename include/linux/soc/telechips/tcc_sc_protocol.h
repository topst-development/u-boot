// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_SC_PROTOCOL
#define TCC_SC_PROTOCOL

#include <mmc.h>
#include <scsi.h>
#include <ufs.h>

struct tcc_sc_fw_handle;
#define TCC_SC_FW_INFO_DESC_SIZE	16 /* in byte */

struct tcc_sc_fw_prot_mmc {
	u32 max_segs;
	u32 max_seg_len;
	u32 blk_size;
	u32 max_blk_num;
	u8 speed_mode;
	u8 bus_width;
	u16 reserved;
	u32 clock;
} __attribute__ ((__packed__));

struct tcc_sc_fw_version {
	u16 major;
	u16 minor;
	u32 patch;
	char desc[TCC_SC_FW_INFO_DESC_SIZE];
};

struct tcc_sc_reg_config {
	u32 address;
	u32 bit_shift;
	u32 width;
	u32 value;
};

struct tcc_sc_reg_req_data {
	u8 count;
	u8 cur_index;
	struct tcc_sc_reg_config *config;
};

struct tcc_sc_fw_mmc_proxy_ops {
	int (*request_command)(const struct tcc_sc_fw_handle *handle,
			struct mmc_cmd *cmd, struct mmc_data *data,
			unsigned char part_num);
	int (*prot_info)(const struct tcc_sc_fw_handle *handle,
			struct tcc_sc_fw_prot_mmc *mmc_info);
};

struct tcc_sc_fw_ufs_proxy_ops {
	int (*request_command)(const struct tcc_sc_fw_handle *handle,
				struct scsi_cmd *cmd);
	int (*request_query)(const struct tcc_sc_fw_handle *handle,
				struct ufs_query_sc *q);
};

struct tcc_sc_fw_reg_proxy_ops {
	int (*request_reg_set)(const struct tcc_sc_fw_handle *handle,
				uint32_t address, uint32_t bit_number,
				uint32_t width, uint32_t value);
	int (*request_reg_set_multi)(const struct tcc_sc_fw_handle *handle,
				struct tcc_sc_reg_req_data *reg_req_data);
};

struct tcc_sc_fw_ops {
	struct tcc_sc_fw_mmc_proxy_ops mmc_ops;
	struct tcc_sc_fw_ufs_proxy_ops ufs_ops;
	struct tcc_sc_fw_reg_proxy_ops reg_ops;
};

struct tcc_sc_fw_handle {
	struct tcc_sc_fw_version version;
	struct tcc_sc_fw_ops ops;
	void *priv;
};

const struct tcc_sc_fw_handle *tcc_sc_fw_get_handle(struct udevice *dev);
const struct tcc_sc_fw_handle *tcc_sc_fw_get_handle_by_name(void);

#endif /* TCC_SC_PROTOCOL */
