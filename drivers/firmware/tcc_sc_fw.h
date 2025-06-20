/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCC_SC_FW
#define TCC_SC_FW

#if IS_ENABLED(CONFIG_TCC805X)
#define TCC_SC_CID_MAIN	0x72U
#define TCC_SC_CID_SUB	0x53U
#define TCC_SC_CID_SC	0xD3U
#else /* TCC807x */
#define TCC_SC_CID_MAIN	0x01U
#define TCC_SC_CID_SUB	0x02U
#define TCC_SC_CID_SC	0xD0U
#endif
#define TCC_SC_CID_HSM	0xA0U

#define TCC_SC_BSID_BL0	0x42U
#define TCC_SC_BSID_BL1	0x43U
#define TCC_SC_BSID_BL2	0x44U
#define TCC_SC_BSID_BL3	0x45U

#define TCC_SC_CMD_FW_INFO		0x00000000U
#define TCC_SC_CMD_PROT_INFO	0x00000001U
 #define TCC_SC_CMD_PROT_MMC	0x2
#define TCC_SC_CMD_MMC_CMD		0x00000005U
#define TCC_SC_CMD_UFS_CMD		0x00000006U
#define TCC_SC_CMD_REG_SET		0x00000010U
#define TCC_SC_CMD_REG_SET_MULTI	0x00000011U

#define TCC_SC_BLK_REQ_TIMEOUT_US	(1000U * 1000U * 1000U)

#define TCC_SC_FW_CMD_FIXED_LENGTH	8U

struct tcc_sc_fw_cmd {
	u8	bsid;
	u8	cid;
	u16	uid;
	u32	cmd;
	u32 args[6];
};

struct tcc_sc_fw_dev {
	struct tcc_sc_fw_handle handle;
	struct mbox_chan chan;
	u8 bsid;
	u8 cid;
	u16 uid;

	u32 *req_data;
	u32 *res_data;
	u32 data_len;
};

#endif /* TCC_SC_FW */
