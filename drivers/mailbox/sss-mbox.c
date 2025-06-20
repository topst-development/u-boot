// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <mailbox-uclass.h>
#include <wait_bit.h>
#include <linux/soc/telechips/tcc_mbox.h>

#define SSS_MB_STATUS           (0x0000)
#define SSS_MB_CMD              (0x2000)

#define SSS_MBOX_SUCCESS        (0xA1)
#define SSS_MBOX_BUSY           (0x1)

#define SSS_MBOX_CMD_FID_01     (0x01)
#define SSS_MBOX_CMD_FID_02     (0x02)

#define MBOX_CMD_TX_FIFO        (SSS_MB_CMD + 0x10U)
#define MBOX_CMD_RX_FIFO        (SSS_MB_CMD + 0x10U)
#define MBOX_DAT_FIFO_TXD       (SSS_MB_CMD + 0x30U)
#define MBOX_DAT_FIFO_RXD       (SSS_MB_CMD + 0x30U)

#define MBOX_MAX_CMD_LENGTH     (8U)
#define MBOX_MAX_DATA_LENGTH    (128U)

struct sss_mbox {
	void __iomem *reg_base;
};

static inline void sss_mbox_writel(const struct sss_mbox *sss, u32 val, u32 offset)
{
	writel((val), (sss->reg_base + offset));
}

static inline u32 sss_mbox_readl(const struct sss_mbox *sss, u32 offset)
{
	return readl((sss->reg_base + offset));
}

static int sss_mbox_check_tx_done(const struct sss_mbox *sss)
{
	return wait_for_bit_le32(sss->reg_base + SSS_MB_STATUS, SSS_MBOX_BUSY,
			false, 1000U, false);
}

static int sss_mbox_send(struct mbox_chan *chan, const void *data)
{
	const struct tcc_mbox_msg *msg = (const struct tcc_mbox_msg *)data;
	const struct sss_mbox *sss =
			(struct sss_mbox *)dev_get_plat(chan->dev);
	u32 i;
	int ret = -EINVAL;

	if ((msg != NULL) && (sss != NULL)) {
		if (msg->cmd_len > MBOX_MAX_CMD_LENGTH) {
			ret = -EINVAL;
		} else if (msg->data_len > MBOX_MAX_DATA_LENGTH) {
			ret = -EINVAL;
		} else if ((sss_mbox_readl(sss, SSS_MB_STATUS) & SSS_MBOX_BUSY) != 0U) {
			ret = -EBUSY;
		} else {
			/* Write command to fifo */
			for (i = 0U ; i < msg->cmd_len ; i++) {
				sss_mbox_writel(sss, msg->cmd[i],
						(MBOX_CMD_TX_FIFO + (i * 0x4U)));
			}

			/* Write data if exist */
			for (i = 0; i < msg->data_len; i++) {
				sss_mbox_writel(sss, msg->data_buf[i],
						(MBOX_DAT_FIFO_TXD + (i * 0x4U)));
			}

			/* Send message */
			sss_mbox_writel(sss, SSS_MBOX_CMD_FID_02, SSS_MB_CMD);

			ret = sss_mbox_check_tx_done(sss);
		}
	}

	return ret;
}

static int sss_mbox_recv(struct mbox_chan *chan, void *data)
{
	struct tcc_mbox_msg *msg = (struct tcc_mbox_msg *)data;
	const struct sss_mbox *sss =
			(struct sss_mbox *)dev_get_plat(chan->dev);
	u32 i;
	int ret = -EINVAL;

	if ((msg != NULL) && (sss != NULL)) {
		if (msg->cmd_len > MBOX_MAX_CMD_LENGTH) {
			ret = -EINVAL;
		} else if (msg->data_len > MBOX_MAX_DATA_LENGTH) {
			ret = -EINVAL;
		} else if ((sss_mbox_readl(sss, SSS_MB_STATUS) & SSS_MBOX_BUSY) != 0) {
			ret = -EBUSY;
		} else {
			/* Read command from fifo */
			for (i = 0; i < msg->cmd_len; i++) {
				msg->cmd[i] = sss_mbox_readl(sss,
						(MBOX_CMD_RX_FIFO + (i * 0x4U)));
			}

			/* Read data from fifo */
			for (i = 0; i < msg->data_len; i++) {
				msg->data_buf[i] = sss_mbox_readl(sss,
						(MBOX_DAT_FIFO_RXD + (i * 0x4U)));
			}

			ret = 0;
		}
	}

	return ret;
}


static int sss_mbox_ofdata_to_platdata(struct udevice *dev)
{
	struct sss_mbox *sss = (struct sss_mbox *)dev_get_plat(dev);
	fdt_addr_t addr;
	int ret = -EINVAL;

	if (sss != NULL) {
		(void)pr_debug("[DEBUG] %s: %s(dev=%p)\n",
				dev->name, __func__, ((void *)dev));

		addr = dev_read_addr(dev);
		if (addr != FDT_ADDR_T_NONE) {
			sss->reg_base = (void __iomem *)addr;
			ret = 0;
		}
	}

	return ret;
}

static int sss_mbox_probe(struct udevice *dev)
{
	const struct sss_mbox *sss = (struct sss_mbox *)dev_get_plat(dev);
	int ret = -EINVAL;

	if (sss != NULL) {
		(void)pr_debug("[DEBUG] %s: %s(dev=%p)\n",
				dev->name, __func__, (void *)dev);
		ret = 0;
	}

	return ret;
}


static const struct udevice_id sss_mbox_ids[2] = {
	{ .compatible = "telechips,sss-mailbox"},
	{},
};

static struct mbox_ops sss_mbox_ops = {
	.send = sss_mbox_send,
	.recv = sss_mbox_recv,
};

U_BOOT_DRIVER(sss_mbox) = {
	.name = "sss-mbox",
	.id = UCLASS_MAILBOX,
	.of_match = sss_mbox_ids,
	.of_to_plat = sss_mbox_ofdata_to_platdata,
	.probe = sss_mbox_probe,
	.plat_auto = (int) sizeof(struct sss_mbox),
	.ops = &sss_mbox_ops,
};

