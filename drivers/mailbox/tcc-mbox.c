// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <mailbox-uclass.h>
#include <linux/io.h>
#include <wait_bit.h>
#include <linux/soc/telechips/tcc_mbox.h>

#define MBOX_CMD_TX_FIFO		(0x0U)
#define MBOX_CMD_RX_FIFO		(0x20U)
#define MBOX_CTRL			(0x40U)
#define MBOX_CMD_FIFO_STS		(0x44U)
#define MBOX_DAT_FIFO_TX_STS		(0x50U)
#define MBOX_DAT_FIFO_RX_STS		(0x54U)
#define MBOX_DAT_FIFO_TXD		(0x60U)
#define MBOX_DAT_FIFO_RXD		(0x70U)
#define MBOX_CTRL_SET			(0x74U)
#define MBOX_CTRL_CLR			(0x78U)
#define MBOX_OPPOSITE_STS		(0x7CU)

#define MBOX_CMD_RX_FIFO_COUNT_MASK		(0xFU)
#define MBOX_CMD_RX_FIFO_COUNT			(20U)
#define MBOX_CMD_RX_FIFO_FULL			(17U)
#define MBOX_CMD_RX_FIFO_EMPTY			(16U)
#define MBOX_CMD_TX_FIFO_COUNT_MASK		(0xFU)
#define MBOX_CMD_TX_FIFO_COUNT			(4U)
#define MBOX_CMD_TX_FIFO_FULL			(1U)
#define MBOX_CMD_TX_FIFO_EMPTY			(0U)
#define MBOX_CMD_FIFO_MAX_COUNT			(8U)

#define MBOX_DAT_TX_FIFO_COUNT_MASK		(0xFFFFU)
#define MBOX_DAT_TX_FIFO_COUNT			(0U)
#define MBOX_DAT_TX_FIFO_FULL			(30U)
#define MBOX_DAT_TX_FIFO_EMPTY			(31U)

#define MBOX_DAT_RX_FIFO_COUNT_MASK		(0xFFFFU)
#define MBOX_DAT_RX_FIFO_COUNT			(0U)
#define MBOX_DAT_RX_FIFO_FULL			(30U)
#define MBOX_DAT_RX_FIFO_EMPTY			(31U)

#define MBOX_DAT_FIFO_MAX_COUNT		(128U)

#define MBOX_CTRL_TEST			(31U)
#define MBOX_CTRL_ICLR_WRITE		(21U)
#define MBOX_CTRL_IEN_WRITE		(20U)
#define MBOX_CTRL_DF_FLUSH		(7U)
#define MBOX_CTRL_CF_FLUSH		(6U)
#define MBOX_CTRL_OEN			(5U)
#define MBOX_CTRL_IEN_READ		(4U)
#define MBOX_CTRL_ILEVEL		(0U)

#define MBOX_ILEVEL_NEMP		(0x0U)
#define MBOX_ILEVEL_GT2			(0x1U)
#define MBOX_ILEVEL_GT4			(0x2U)
#define MBOX_ILEVEL_FULL		(0x3U)

#define MBOX_OP_POLL			(1 << 0)
#define MBOX_OP_IRQ				(1 << 1)
#define MBOX_OP_IRQ_SHIFT		(0x4U)
#define MBOX_OP_IRQ_MASK		(0x3U)
#define MBOX_OP_IRQ_LEVEL(X)	(((X) & MBOX_OP_IRQ_MASK) << MBOX_OP_IRQ_SHIFT)
/* indicate SSS MBOX channel */
#define MBOX_OP_SSS			(1 << 2)

#define MBOX_MAX_CMD_LENGTH		(8U)
#define MBOX_MAX_DATA_LENGTH		(128U)

#ifndef CONFIG_TCC_MBOX_FIFO_WAIT_TIME
/* in msec */
#define CONFIG_TCC_MBOX_FIFO_WAIT_TIME	(1000U)
#endif

struct tcc_mbox {
	void __iomem *reg_base;
};

static int tcc_mbox_send(struct mbox_chan *chan, const void *data);
static int tcc_mbox_recv(struct mbox_chan *chan, void *data);

static inline void tcc_mbox_writel(struct tcc_mbox *tcc, u32 val, u32 reg)
{
	writel((val), (tcc->reg_base + reg));
}

static inline u32 tcc_mbox_readl(struct tcc_mbox *tcc, u32 reg)
{
	return readl((tcc->reg_base + reg));
}

static int tcc_mbox_check_tx_done(struct tcc_mbox *tcc)
{
	int ret;
	u32 mask;

	if (tcc == NULL)
		return -1;

	mask = ((u32) 0x1U << (u32) MBOX_CMD_TX_FIFO_EMPTY);
	ret = wait_for_bit_le32((tcc->reg_base + MBOX_CMD_FIFO_STS),
			mask, true, CONFIG_TCC_MBOX_FIFO_WAIT_TIME, false);
	if (ret == 0) {
		mask = ((u32) 0x1U << (u32) MBOX_DAT_TX_FIFO_EMPTY);
		ret = wait_for_bit_le32((tcc->reg_base + MBOX_DAT_FIFO_TX_STS),
				mask, true,
				CONFIG_TCC_MBOX_FIFO_WAIT_TIME, false);
	}

	return ret;
}

static int tcc_mbox_send(struct mbox_chan *chan, const void *data)
{
	const struct tcc_mbox_msg *msg = (const struct tcc_mbox_msg *)data;
	struct tcc_mbox *tcc = dev_get_platdata(chan->dev);
	u32 i, val;

	/* check message */
	if ((msg->cmd_len > MBOX_MAX_CMD_LENGTH) || (msg->cmd_len == 0x0U)
		|| (msg->cmd == NULL))
		return -EINVAL;

	if (msg->data_len > MBOX_MAX_DATA_LENGTH)
		return -EINVAL;

	if ((msg->data_len != 0U) && (msg->data_buf == NULL))
		return -EINVAL;

	if (tcc_mbox_check_tx_done(tcc) != 0)
		return -EBUSY;

	/* Write command to fifo */
	for (i = 0U ; i < msg->cmd_len ; i++) {
		tcc_mbox_writel(tcc, msg->cmd[i],
				(MBOX_CMD_TX_FIFO + (i * 0x4U)));
	}

	/* Write data if exist */
	if (msg->data_buf != NULL) {
		for (i = 0; i < msg->data_len; i++) {
			tcc_mbox_writel(tcc, msg->data_buf[i],
					MBOX_DAT_FIFO_TXD);
		}
	}

	/* Send message */
	val = tcc_mbox_readl(tcc, MBOX_CTRL);
	val |= ((u32) 0x1U << (u32) MBOX_CTRL_OEN);
	tcc_mbox_writel(tcc, val, MBOX_CTRL);

	if (tcc_mbox_check_tx_done(tcc) != 0)
		return -ETIMEDOUT;

	/* Finish */
	val = tcc_mbox_readl(tcc, MBOX_CTRL);
	val &= ~(0x1U << MBOX_CTRL_OEN);
	val |= ((0x1U << MBOX_CTRL_CF_FLUSH) |
		(0x1U << MBOX_CTRL_DF_FLUSH) |
		(MBOX_ILEVEL_FULL << MBOX_CTRL_ILEVEL));
	tcc_mbox_writel(tcc, val, MBOX_CTRL);

	return 0;
}

static int tcc_mbox_recv(struct mbox_chan *chan, void *data)
{
	struct tcc_mbox_msg *msg = (struct tcc_mbox_msg *)data;
	struct tcc_mbox *tcc = dev_get_platdata(chan->dev);
	u32 i, fifo_count;
	u32 mask;
	int ret;

	/* check message */
	if ((msg->cmd_len > MBOX_MAX_CMD_LENGTH) || (msg->cmd_len == 0U)
		|| (msg->cmd == NULL))
		return -EINVAL;

	if (msg->data_len > MBOX_MAX_DATA_LENGTH)
		return -EINVAL;

	if ((msg->data_len != 0U) && (msg->data_buf == NULL))
		return -EINVAL;

	/* Read command from fifo */
	mask = ((u32) 0x1U << (u32) MBOX_CMD_RX_FIFO_EMPTY);
	ret = wait_for_bit_le32(tcc->reg_base + MBOX_CMD_FIFO_STS,
			mask, false, CONFIG_TCC_MBOX_FIFO_WAIT_TIME, false);
	if (ret == 0) {
		fifo_count = (tcc_mbox_readl(tcc, MBOX_CMD_FIFO_STS)
			>> MBOX_CMD_RX_FIFO_COUNT)
			& MBOX_CMD_RX_FIFO_COUNT_MASK;
	} else {
		return ret;
	}

	for (i = 0 ; i < fifo_count ; i++) {
		/* if buf len is smaller than fifo cnt, do dummy read */
		if (msg->cmd_len < (i + 1U))
			tcc_mbox_readl(tcc, (MBOX_CMD_RX_FIFO + (i * 0x4U)));
		else
			msg->cmd[i] = tcc_mbox_readl(tcc,
					(MBOX_CMD_RX_FIFO + (i * 0x4U)));
	}

	/* Read data from fifo */
	fifo_count = (tcc_mbox_readl(tcc, MBOX_DAT_FIFO_RX_STS)
			>> MBOX_DAT_RX_FIFO_COUNT)
			& MBOX_DAT_RX_FIFO_COUNT_MASK;
	for (i = 0 ; i < fifo_count ; i++) {
		/* if buf len is smaller than fifo cnt, do dummy read */
		if ((msg->data_buf == NULL) || (msg->data_len < (i + 1U)))
			tcc_mbox_readl(tcc, MBOX_DAT_FIFO_RXD);
		else
			msg->data_buf[i] = tcc_mbox_readl(tcc,
						MBOX_DAT_FIFO_RXD);
	}

	return 0;
}


static int tcc_mbox_ofdata_to_platdata(struct udevice *dev)
{
	struct tcc_mbox *tcc = dev_get_platdata(dev);
	fdt_addr_t addr;

	pr_debug("[DEBUG] %s: %s(dev=%p)\n",
			dev->name, __func__, dev);

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	tcc->reg_base = (void __iomem *)addr;

	return 0;
}

static int tcc_mbox_probe(struct udevice *dev)
{
	struct tcc_mbox *tcc = dev_get_platdata(dev);
	u32 val;

	pr_debug("[DEBUG] %s: %s(dev=%p)\n",
			dev->name, __func__, dev);

	val = tcc_mbox_readl(tcc, MBOX_CTRL);
	val &= ~(0x1U << MBOX_CTRL_OEN);
	val &= ~((0x1U << MBOX_CTRL_IEN_READ)
			| (MBOX_ILEVEL_FULL << MBOX_CTRL_ILEVEL));
	val |= ((0x1U << MBOX_CTRL_CF_FLUSH) | (0x1U << MBOX_CTRL_DF_FLUSH));
	tcc_mbox_writel(tcc, val, MBOX_CTRL);

	return 0;
}

static const struct udevice_id tcc_mbox_ids[2] = {
	{ .compatible = "telechips,tcc805x-mailbox"},
	{},
};

static struct mbox_ops tcc_mbox_ops = {
	.send = tcc_mbox_send,
	.recv = tcc_mbox_recv,
};

U_BOOT_DRIVER(tcc_mbox) = {
	.name = "tcc-mbox",
	.id = UCLASS_MAILBOX,
	.of_match = tcc_mbox_ids,
	.ofdata_to_platdata = tcc_mbox_ofdata_to_platdata,
	.probe = tcc_mbox_probe,
	.platdata_auto_alloc_size = (int) sizeof(struct tcc_mbox),
	.ops = &tcc_mbox_ops,
};

