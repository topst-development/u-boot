// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <mailbox-uclass.h>
#include <linux/io.h>
#include <wait_bit.h>
#include <linux/soc/telechips/tcc803x_multi_mbox.h>

/******************************************************************************
 *    MailBox Register offset
 *****************************************************************************/
#define MBOXTXD(x)		(0x00 + (x) * 4)	/* Transmit cmd fifo */
#define MBOXRXD(x)		(0x20 + (x) * 4)	/* Receive cmd fifo */
#define MBOXCTR			0x40
#define MBOXSTR			0x44
#define MBOX_DT_STR		0x50	/* Transmit data status */
#define MBOX_RT_STR		0x54	/* Receive data status */
#define MBOXDTXD		0x60	/* Transmit data fifo */
#define MBOXRTXD		0x70	/* Receive data fifo */

/******************************************************************************
 *    MailBox CTR Register Field Define
 *****************************************************************************/
/* FLUSH: 6bit, OEN: 5bit, IEN: 4bit, LEVEL: 1~0bit */
#define D_FLUSH_BIT		BIT(7)
#define FLUSH_BIT		BIT(6)
#define OEN_BIT			BIT(5)
#define IEN_BIT			BIT(4)
#define LEVEL0_BIT		BIT(1)
#define LEVEL1_BIT		BIT(0)

#define MEMP_MASK		0x00000001
#define MFUL_MASK		0x00000002
#define MCOUNT_MASK		0x000000F0
#define SEMP_MASK		0x00010000
#define SFUL_MASK		0x00020000
#define SCOUNT_MASK		0x00F00000

#define DATA_MEMP_MASK		0x80000000
#define DATA_MFUL_MASK		0x40000000
#define DATA_MCOUNT_MASK	0x0000FFFF

#define DATA_SEMP_MASK		0x80000000
#define DATA_SFUL_MASK		0x40000000
#define DATA_SCOUNT_MASK	0x0000FFFF

#define TCC_MBOX_CH_UBOOT		0x0

DEFINE_MUTEX(tcc_mbox_lock);	/* mbox mutex */

struct tcc_multi_mbox {
	void __iomem *reg_base;
};

static int tcc_multi_mbox_send(struct mbox_chan *chan, const void *data)
{
	int ret = -ENODEV;

	struct tcc_multi_mbox *tcc = dev_get_platdata(chan->dev);

	mutex_lock(&tcc_mbox_lock);

	if ((tcc != NULL) && (data != NULL)) {
		const struct tcc_mbox_data *mbox_msg =
			(const struct tcc_mbox_data *)data;

		/* check fifo */
		if ((readl(tcc->reg_base + MBOXSTR) & MEMP_MASK) == 0) {
			ret = -EBUSY;
		} else {
			s32 idx = 0;

			/* init mbox */
			writel((readl(tcc->reg_base + MBOXCTR) & ~(IEN_BIT)),
				tcc->reg_base + MBOXCTR);

			/* check data fifo */
			if ((readl(tcc->reg_base + MBOX_DT_STR)
				& DATA_MEMP_MASK)
				== 0) {
				/* flush buffer */
				writel((readl(tcc->reg_base + MBOXCTR) |
					D_FLUSH_BIT), tcc->reg_base + MBOXCTR);
			}

			/* disable data output. */
			writel((readl(tcc->reg_base + MBOXCTR) & ~(OEN_BIT)),
				tcc->reg_base + MBOXCTR);

			/* write data fifo */
			if (mbox_msg->data_len > 0) {
				for (idx = 0; idx < mbox_msg->data_len; idx++) {
					writel(mbox_msg->data[idx],
						tcc->reg_base + MBOXDTXD);
				}
			}

			/* write command fifo */
			writel(TCC_MBOX_CH_UBOOT, tcc->reg_base + MBOXTXD(0));
			writel(mbox_msg->cmd[0], tcc->reg_base + MBOXTXD(1));
			writel(mbox_msg->cmd[1], tcc->reg_base + MBOXTXD(2));
			writel(mbox_msg->cmd[2], tcc->reg_base + MBOXTXD(3));
			writel(mbox_msg->cmd[3], tcc->reg_base + MBOXTXD(4));
			writel(mbox_msg->cmd[4], tcc->reg_base + MBOXTXD(5));
			writel(mbox_msg->cmd[5], tcc->reg_base + MBOXTXD(6));
			writel(mbox_msg->cmd[6], tcc->reg_base + MBOXTXD(7));

			/* enable data output. */
			writel(readl(tcc->reg_base + MBOXCTR) | OEN_BIT,
				tcc->reg_base + MBOXCTR);

			ret = 0;
		}
	}

	mutex_unlock(&tcc_mbox_lock);

	return ret;
}

static int tcc_multi_mbox_recv(struct mbox_chan *chan, void *data)
{
	int ret = -ENODEV;

	struct tcc_mbox_data *mbox_msg = (struct tcc_mbox_data *)data;
	struct tcc_multi_mbox *tcc = dev_get_platdata(chan->dev);

	if ((mbox_msg != NULL) && (tcc != NULL)) {
		u32 chan_ID;
		s32 idx;

		ret = -ENODATA;
		/* check receive fifo */
		if (((readl(tcc->reg_base + MBOXSTR) >> 20) & 0xF)
			== 0x8) {
			mbox_msg->data_len =
				readl(tcc->reg_base + MBOX_RT_STR) &
				DATA_SCOUNT_MASK;

			for (idx = 0; idx < mbox_msg->data_len; idx++) {
				mbox_msg->data[idx] =
					readl(tcc->reg_base + MBOXRTXD);
			}

			chan_ID = readl(tcc->reg_base + MBOXRXD(0));
			mbox_msg->cmd[0] =
				readl(tcc->reg_base + MBOXRXD(1));
			mbox_msg->cmd[1] =
				readl(tcc->reg_base + MBOXRXD(2));
			mbox_msg->cmd[2] =
				readl(tcc->reg_base + MBOXRXD(3));
			mbox_msg->cmd[3] =
				readl(tcc->reg_base + MBOXRXD(4));
			mbox_msg->cmd[4] =
				readl(tcc->reg_base + MBOXRXD(5));
			mbox_msg->cmd[5] =
				readl(tcc->reg_base + MBOXRXD(6));
			mbox_msg->cmd[6] =
				readl(tcc->reg_base + MBOXRXD(7));

			if (chan_ID == TCC_MBOX_CH_UBOOT) {
				ret = 0;
			}
		}
	}

	return ret;
}

static int tcc_multi_mbox_ofdata_to_platdata(struct udevice *dev)
{
	struct tcc_multi_mbox *tcc = dev_get_platdata(dev);
	fdt_addr_t addr;
	int ret = -ENODEV;

	if (tcc != NULL) {
		pr_debug("[DEBUG] %s: %s(dev=%p)\n",
				dev->name, __func__, dev);

		addr = dev_read_addr(dev);
		if (addr == FDT_ADDR_T_NONE) {
			ret = -EINVAL;
		} else {
			tcc->reg_base = (void __iomem *)addr;
			ret = 0;
		}
	}

	return ret;
}

static int tcc_multi_mbox_probe(struct udevice *dev)
{
	struct tcc_multi_mbox *tcc = dev_get_platdata(dev);
	int ret = -ENODEV;

	if (tcc != NULL) {

		pr_debug("[DEBUG] %s: %s(dev=%p)\n",
				dev->name, __func__, dev);

		/*receive disable */
		writel((readl(tcc->reg_base + MBOXCTR) & ~(IEN_BIT)),
			tcc->reg_base + MBOXCTR);

		/* disable data output. */
		writel((readl(tcc->reg_base + MBOXCTR) & ~(OEN_BIT)),
			tcc->reg_base + MBOXCTR);

		/* flush cmd fifo & data fifo*/
		writel((readl(tcc->reg_base + MBOXCTR)
			| D_FLUSH_BIT | FLUSH_BIT),
			tcc->reg_base + MBOXCTR);

		ret = 0;
	}

	return ret;
}

static const struct udevice_id tcc_multi_mbox_ids[] = {
	{ .compatible = "telechips,multichannel-mailbox"},
	{},
};

static struct mbox_ops tcc_multi_mbox_ops = {
	.send = tcc_multi_mbox_send,
	.recv = tcc_multi_mbox_recv,
};

U_BOOT_DRIVER(tcc_multi_mbox) = {
	.name = "tcc-multi-mbox",
	.id = UCLASS_MAILBOX,
	.of_match = tcc_multi_mbox_ids,
	.ofdata_to_platdata = tcc_multi_mbox_ofdata_to_platdata,
	.probe = tcc_multi_mbox_probe,
	.platdata_auto_alloc_size = (int) sizeof(struct tcc_multi_mbox),
	.ops = &tcc_multi_mbox_ops,
};

