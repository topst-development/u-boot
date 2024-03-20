// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 */

#include <stdint.h>
#include <string.h>
#include <mach/ddr_set.h>
#include <cpu_func.h>
#include <hsm_mbox.h>

/* Samsung SSS mbox */
#define HSM_S_MB_DATA00 0x1E012000
#define HSM_NS_MB_DATA00 0x1E022000
#define HSM_S_MB_STATUS 0x1E010000
#define HSM_NS_MB_STATUS 0x1E020000

#define SSS_MBOX_SUCCESS 0xA1

#define SSS_MBOX_BUSY 0x1
#define SSS_SRAM_VERIFY_SUCCESS 0x1

#define SSS_MBOX_CMD_FID_01 0x01
#define SSS_MBOX_CMD_FID_02 0x02

#define SSS_MBOX_CMD_ROM_GET_INFO 0x101

/* TCC mbox */
#ifdef CONFIG_TCC_MAINCORE
#define MBOX_HSM 0x1E100000
#else
#define MBOX_HSM 0x1E110000
#endif

#define MBOX_CMD_TX_FIFO 0x0U
#define MBOX_CMD_RX_FIFO 0x20U
#define MBOX_CTRL 0x40U
#define MBOX_CMD_FIFO_STS 0x44U
#define MBOX_DAT_FIFO_TX_STS 0x50U
#define MBOX_DAT_FIFO_RX_STS 0x54U
#define MBOX_DAT_FIFO_TXD 0x60U
#define MBOX_DAT_FIFO_RXD 0x70U
#define MBOX_CTRL_SET 0x74U
#define MBOX_CTRL_CLR 0x78U
#define MBOX_OPPOSITE_STS 0x7CU

#define MBOX_CTRL_TEST 31U
#define MBOX_CTRL_ICLR_WRITE 21U
#define MBOX_CTRL_IEN_WRITE 20U
#define MBOX_CTRL_DF_FLUSH 7U
#define MBOX_CTRL_CF_FLUSH 6U
#define MBOX_CTRL_OEN 5U
#define MBOX_CTRL_IEN_READ 4U
#define MBOX_CTRL_ILEVEL 0U

#define MBOX_ILEVEL_NEMP 0x0U
#define MBOX_ILEVEL_GT2 0x1U
#define MBOX_ILEVEL_GT4 0x2U
#define MBOX_ILEVEL_FULL 0x3U

/*
 * <HSM F/W Mailbox Protocal>
 *  - 0x00 - 0x00: BSID
 *  - 0x01 - 0x01: CID
 *  - 0x02 - 0x03: Reserved
 *  - 0x04 - 0x07: CMD
 *  - 0x08 - 0x23: ARG
 */
#define BSID_BL1 0x43U

#define CID_A72 0x72U
#define CID_A53 0x53U
#define CID_HSM 0xA0U

#ifdef CONFIG_TCC_MAINCORE
#define CID_AP CID_A72
#else
#define CID_AP CID_A53
#endif

#define CMD_HSM_VERIFY 0x00020014UL
#define CMD_HSM_SETREADY 0x00030000UL

struct mbox_cmd_t {
	unsigned int info;
	unsigned int cmd;
	unsigned int args[6];
};

#define MBOX_READ32(a) mmio_read_32(a)
#define MBOX_WRITE32(a, v) mmio_write_32(a, v)

static void mbox_send(unsigned int sendto, struct mbox_cmd_t *mbox_tx_cmd)
{
	MBOX_WRITE32(
		sendto + MBOX_CTRL,
		(MBOX_READ32(sendto + MBOX_CTRL) & ~(0x1 << MBOX_CTRL_OEN)));
	MBOX_WRITE32(
		sendto + MBOX_CTRL,
		(MBOX_READ32(sendto + MBOX_CTRL)
		 | (0x1 << MBOX_CTRL_CF_FLUSH)));
	MBOX_WRITE32(
		sendto + MBOX_CTRL,
		(MBOX_READ32(sendto + MBOX_CTRL)
		 | (0x1 << MBOX_CTRL_DF_FLUSH)));
	MBOX_WRITE32(
		sendto + MBOX_CTRL,
		(MBOX_READ32(sendto + MBOX_CTRL)
		 | (MBOX_ILEVEL_FULL << MBOX_CTRL_ILEVEL)));

	MBOX_WRITE32(sendto + MBOX_CMD_TX_FIFO, mbox_tx_cmd->info);

	MBOX_WRITE32(sendto + MBOX_CMD_TX_FIFO + 0x04, mbox_tx_cmd->cmd);
	MBOX_WRITE32(sendto + MBOX_CMD_TX_FIFO + 0x08, mbox_tx_cmd->args[0]);
	MBOX_WRITE32(sendto + MBOX_CMD_TX_FIFO + 0x0c, mbox_tx_cmd->args[1]);
	MBOX_WRITE32(sendto + MBOX_CMD_TX_FIFO + 0x10, mbox_tx_cmd->args[2]);
	MBOX_WRITE32(sendto + MBOX_CMD_TX_FIFO + 0x14, mbox_tx_cmd->args[3]);
	MBOX_WRITE32(sendto + MBOX_CMD_TX_FIFO + 0x18, mbox_tx_cmd->args[4]);
	MBOX_WRITE32(sendto + MBOX_CMD_TX_FIFO + 0x1c, mbox_tx_cmd->args[5]);

	MBOX_WRITE32(
		sendto + MBOX_CTRL,
		(MBOX_READ32(sendto + MBOX_CTRL)
		 & ~(0x1 << MBOX_CTRL_IEN_WRITE)));
	MBOX_WRITE32(
		sendto + MBOX_CTRL,
		(MBOX_READ32(sendto + MBOX_CTRL)
		 & ~(0x1 << MBOX_CTRL_IEN_READ)));
	// MBOX_WRITE32(sendto + MBOX_CTRL, (MBOX_READ32(sendto + MBOX_CTRL) |
	// (MBOX_ILEVEL_FULL << MBOX_CTRL_ILEVEL)));

	/* Check Samusng SSS mailbox status */
	if (sendto == MBOX_HSM) {
		while (MBOX_READ32(HSM_NS_MB_STATUS) & SSS_MBOX_BUSY)
			;
	}

	MBOX_WRITE32(
		sendto + MBOX_CTRL,
		(MBOX_READ32(sendto + MBOX_CTRL) | (0x1 << MBOX_CTRL_OEN)));

	/* Send message to Samusng SSS mailbox */
	if (sendto == MBOX_HSM) {
		MBOX_WRITE32(HSM_NS_MB_DATA00, SSS_MBOX_CMD_FID_02);
	}

	while ((MBOX_READ32(sendto + MBOX_CMD_FIFO_STS) & (0xF)) != 1)
		;

	MBOX_WRITE32(
		sendto + MBOX_CTRL,
		(MBOX_READ32(sendto + MBOX_CTRL) & ~(0x1 << MBOX_CTRL_OEN)));
}

static int mbox_recv(unsigned int recvfrom)
{
	unsigned int ret = ERROR_UNKNOWN_ERROR;
	unsigned int bsid;
	unsigned int cid;
	struct mbox_cmd_t mbox_rx_cmd;

	mbox_rx_cmd.info = MBOX_READ32(recvfrom + MBOX_CMD_RX_FIFO);
	mbox_rx_cmd.cmd = MBOX_READ32(recvfrom + MBOX_CMD_RX_FIFO + 0x04);
	mbox_rx_cmd.args[0] = MBOX_READ32(recvfrom + MBOX_CMD_RX_FIFO + 0x08);
	mbox_rx_cmd.args[1] = MBOX_READ32(recvfrom + MBOX_CMD_RX_FIFO + 0x0c);
	mbox_rx_cmd.args[2] = MBOX_READ32(recvfrom + MBOX_CMD_RX_FIFO + 0x10);
	mbox_rx_cmd.args[3] = MBOX_READ32(recvfrom + MBOX_CMD_RX_FIFO + 0x14);
	mbox_rx_cmd.args[4] = MBOX_READ32(recvfrom + MBOX_CMD_RX_FIFO + 0x18);
	mbox_rx_cmd.args[5] = MBOX_READ32(recvfrom + MBOX_CMD_RX_FIFO + 0x1c);

	bsid = (mbox_rx_cmd.info & (0xFFUL << 0UL));
	cid = (mbox_rx_cmd.info & (0xFFUL << 8UL)) >> 0x8;

	if (bsid == BSID_BL1) {
		if (cid != CID_HSM) {
			ret = ERROR_UNKNOWN_CID;
		} else {
			if (mbox_rx_cmd.cmd == CMD_HSM_VERIFY) {
				if (mbox_rx_cmd.args[0] == 0x0) {
					ret = ERROR_VERIFY_OK;
				} else {
					ret = ERROR_VERIFY_FAIL;
				}
			} else {
				if (mbox_rx_cmd.args[0] == 0x0) {
					ret = ERROR_HSM_MBOX_SUCCESS;
				} else {
					ret = ERROR_UNKNOWN_ERROR;
				}
			}
		}
	} else {
		ret = ERROR_UNKNOWN_BSID;
	}

	/* Check Samusng SSS mailbox response */
	if (recvfrom == MBOX_HSM) {
		while (MBOX_READ32(HSM_NS_MB_STATUS) & SSS_MBOX_BUSY)
			;

		if (MBOX_READ32(HSM_NS_MB_DATA00) != SSS_MBOX_SUCCESS) {
			ret = ERROR_VERIFY_FAIL;
		}
	}

	return ret;
}

unsigned int ap_recv_from_hsm(void)
{
	while ((MBOX_READ32(MBOX_HSM + MBOX_CMD_FIFO_STS) & (0x1 << 16UL)) != 0)
		;

	return mbox_recv(MBOX_HSM);
}

void ap_send_to_hsm(
	unsigned int header_addr, unsigned int header_size,
	unsigned int img_addr, unsigned int img_size, unsigned int tccimgid)
{
	struct mbox_cmd_t mbox_tx_cmd;

	memset(&mbox_tx_cmd, 0x0, sizeof(struct mbox_cmd_t));

	mbox_tx_cmd.info = (mbox_tx_cmd.info) | (BSID_BL1 << 0UL);
	mbox_tx_cmd.info = (mbox_tx_cmd.info) | (CID_AP << 8UL);
	mbox_tx_cmd.cmd = CMD_HSM_VERIFY;

	mbox_tx_cmd.args[0] = header_addr;
	mbox_tx_cmd.args[1] = header_size;
	mbox_tx_cmd.args[2] = img_addr;
	mbox_tx_cmd.args[3] = img_size;
	mbox_tx_cmd.args[4] = tccimgid;
	mbox_tx_cmd.args[5] = 0x0;

	mbox_send(MBOX_HSM, &mbox_tx_cmd);
}

void ap_send_mf_cmd(
	unsigned int cmd, unsigned int img_addr,
	unsigned int img_size, unsigned int tccimgid)
{
	struct mbox_cmd_t mbox_tx_cmd;

	memset(&mbox_tx_cmd, 0x0, sizeof(struct mbox_cmd_t));

	mbox_tx_cmd.info = (mbox_tx_cmd.info) | (BSID_BL1 << 0UL);
	mbox_tx_cmd.info = (mbox_tx_cmd.info) | (CID_AP << 8UL);
	mbox_tx_cmd.cmd = cmd;

	mbox_tx_cmd.args[0] = img_addr;
	mbox_tx_cmd.args[1] = img_size;
	mbox_tx_cmd.args[2] = tccimgid;

	mbox_send(MBOX_HSM, &mbox_tx_cmd);
}
