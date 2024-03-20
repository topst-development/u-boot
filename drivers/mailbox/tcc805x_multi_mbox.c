// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <mailbox-uclass.h>
#include <linux/io.h>
#include <wait_bit.h>
#include <linux/soc/telechips/tcc805x_multi_mbox.h>

/******************************************************************************
 *    MailBox Register offset
 *****************************************************************************/
#define MBOXTXD(x)		(0x00 + (x) * 4)	/* Transmit cmd fifo */
#define MBOXRXD(x)		(0x20 + (x) * 4)	/* Receive cmd fifo */
#define MBOXCTR			(0x40)
#define MBOXSTR			(0x44)
#define MBOX_DT_STR		(0x50)	/* Transmit data status */
#define MBOX_RT_STR		(0x54)	/* Receive data status */
#define MBOXDTXD		(0x60)	/* Transmit data fifo */
#define MBOXRTXD		(0x70)	/* Receive data fifo */
#define MBOXCTR_SET		(0x74)
#define MBOXCTR_CLR		(0x78)
#define TRM_STS			(0x7C)

/******************************************************************************
 *    MailBox CTR Register Field Define
 *****************************************************************************/
/* FLUSH: 6bit, OEN: 5bit, IEN: 4bit, LEVEL: 1~0bit */
#define D_FLUSH_BIT		(BIT(7))
#define FLUSH_BIT		(BIT(6))
#define OEN_BIT			(BIT(5))
#define IEN_BIT			(BIT(4))
#define LEVEL0_BIT		(BIT(1))
#define LEVEL1_BIT		(BIT(0))

#define OPP_TMN_STS_BIT	(0x00010000)
#define OWN_TMN_STS_BIT	(0x00000001)

#define MEMP_MASK		(0x00000001)
#define MFUL_MASK		(0x00000002)
#define MCOUNT_MASK		(0x000000F0)
#define SEMP_MASK		(0x00010000)
#define SFUL_MASK		(0x00020000)
#define SCOUNT_MASK		(0x00F00000)

#define DATA_MEMP_MASK		(0x80000000)
#define DATA_MFUL_MASK		(0x40000000)
#define DATA_MCOUNT_MASK	(0x0000FFFF)

#define DATA_SEMP_MASK		(0x80000000)
#define DATA_SFUL_MASK		(0x40000000)
#define DATA_SCOUNT_MASK	(0x0000FFFF)

#define BSID			(0x45)
#define CID_A72			(0x72)
#define CID_A53			(0x53)

#define MBOX_ID0_LEN	(4U)
#define MBOX_ID1_LEN	(2U)
#define MBOX_ID_MAX_LEN		(MBOX_ID0_LEN+MBOX_ID1_LEN)

#ifndef char_t
typedef char char_t;
#endif

struct channel_id0 {
	s8 bsid[1];
	s8 cid[1];
	char_t idName[MBOX_ID1_LEN];
};

struct channel_id1 {
	char_t idName[MBOX_ID0_LEN];
};

union mbox_header0 {
	struct channel_id0 id;
	uint32_t cmd;
};

union mbox_header1 {
	struct channel_id1 id;
	uint32_t cmd;
};

DEFINE_MUTEX(tcc_mbox_lock);	/* mbox mutex */

struct tcc_multi_mbox {
	void __iomem *reg_base;
};

static s32 tcc_multi_mbox_send(struct mbox_chan *chan, const void *data)
{
	s32 ret = -ENODEV;

	if (chan != NULL) {
		struct tcc_multi_mbox *tcc =
			dev_get_platdata(chan->dev);

		mutex_lock(&tcc_mbox_lock);

		if ((tcc != NULL) && (data != NULL)) {
			union mbox_header0 header0;
			union mbox_header1 header1;

			const struct tcc_mbox_data *mbox_msg =
				(const struct tcc_mbox_data *)data;

		#ifdef CONFIG_TCC_SUBCORE
			header0.id.cid[0] = CID_A53;
		#else
			header0.id.cid[0] = CID_A72;
		#endif
			header0.id.bsid[0] = BSID;

			header0.id.idName[0] = mbox_msg->id_name[4];
			header0.id.idName[1] = mbox_msg->id_name[5];

			header1.cmd =
				(((u32)mbox_msg->id_name[0]) |
				((u32)mbox_msg->id_name[1]<<8) |
				((u32)mbox_msg->id_name[2]<<16) |
				((u32)mbox_msg->id_name[3]<<24));

			/* check fifo */
			if ((readl(tcc->reg_base + MBOXSTR)
				& (uint32_t)MEMP_MASK) == 0) {
				ret = -EBUSY;
			} else {
				s32 idx = 0;

				/* init mbox */
				writel((readl(tcc->reg_base + MBOXCTR)
					& ~((uint32_t)IEN_BIT)),
					tcc->reg_base + MBOXCTR);

				/* check data fifo */
				if ((readl(tcc->reg_base + MBOX_DT_STR)
					& (uint32_t)DATA_MEMP_MASK)
					== 0) {
					/* flush buffer */
					writel((readl(tcc->reg_base + MBOXCTR) |
						(uint32_t)D_FLUSH_BIT),
						tcc->reg_base +
						(uint32_t)MBOXCTR);
				}

				/* disable data output. */
				writel((readl(tcc->reg_base + MBOXCTR)
					& ~((uint32_t)OEN_BIT)),
					tcc->reg_base + MBOXCTR);

				/* write data fifo */
				if (mbox_msg->data_len > 0) {
					for (idx = 0;
						idx < mbox_msg->data_len;
						idx++) {
						writel(mbox_msg->data[idx],
						tcc->reg_base + MBOXDTXD);
					}
				}
#if 0
			s32 i;

			pr_err("[DEBUG] %s : cmd[0] = 0x%08x\n",
				__func__,
				header0.cmd);
			for (i = 0; i < 6; i++)	{
				pr_err("[DEBUG] %s : cmd[%d] = 0x%08x\n",
					__func__,
					i+1, mbox_msg->cmd[i]);
			}
			pr_err("[DEBUG] %s : cmd[7] = 0x%08x\n",
				__func__,
				header1.cmd);

#endif
				/* write command fifo */
				writel(header0.cmd, tcc->reg_base + MBOXTXD(0));
				writel(mbox_msg->cmd[0],
					tcc->reg_base + MBOXTXD(1));
				writel(mbox_msg->cmd[1],
					tcc->reg_base + MBOXTXD(2));
				writel(mbox_msg->cmd[2],
					tcc->reg_base + MBOXTXD(3));
				writel(mbox_msg->cmd[3],
					tcc->reg_base + MBOXTXD(4));
				writel(mbox_msg->cmd[4],
					tcc->reg_base + MBOXTXD(5));
				writel(mbox_msg->cmd[5],
					tcc->reg_base + MBOXTXD(6));
				writel(header1.cmd,
					tcc->reg_base + MBOXTXD(7));

				/* enable data output. */
				writel(readl(tcc->reg_base + MBOXCTR)
					| (uint32_t)OEN_BIT,
					tcc->reg_base + MBOXCTR);

				ret = 0;
			}
		}

		mutex_unlock(&tcc_mbox_lock);
	}

	return ret;
}

static s32 tcc_multi_mbox_recv(struct mbox_chan *chan, void *data)
{
	s32 ret = -ENODEV;

	if ((chan != NULL) && (data != NULL)) {
		union mbox_header0 header0;
		union mbox_header1 header1;

		struct tcc_mbox_data *mbox_msg = (struct tcc_mbox_data *)data;
		struct tcc_multi_mbox *tcc = dev_get_platdata(chan->dev);

		if ((mbox_msg != NULL) && (tcc != NULL)) {
			s32 idx;
			s32 i;

			ret = -ENODATA;
			/* check receive fifo */
			if (((readl(tcc->reg_base + MBOXSTR) >> 20)
				& (uint32_t)0xF)
				== 0x8) {
				mbox_msg->data_len =
					readl(tcc->reg_base + MBOX_RT_STR) &
					(uint32_t)DATA_SCOUNT_MASK;

				for (idx = 0; idx < mbox_msg->data_len; idx++) {
					mbox_msg->data[idx] =
						readl(tcc->reg_base + MBOXRTXD);
				}

				header0.cmd =
					readl(tcc->reg_base + MBOXRXD(0));
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
				header1.cmd =
					readl(tcc->reg_base + MBOXRXD(7));

				memset(mbox_msg->id_name,
					0x00,
					(uint32_t)MBOX_ID_MAX_LEN);

				for (i = 0; i < (s32)MBOX_ID0_LEN; i++) {
					mbox_msg->id_name[i] =
						header1.id.idName[i];
				}

				mbox_msg->id_name[MBOX_ID0_LEN] =
					header0.id.idName[0];
				mbox_msg->id_name[MBOX_ID0_LEN+1] =
					header0.id.idName[1];
				ret = 0;
#if 0
				pr_err("[DEBUG] %s : id = %s\n",
					__func__,
					mbox_msg->id_name);
				for (i = 0; i < 6; i++) {
					pr_err("[DEBUG] %s : cmd[%d] = 0x%08x\n",
						__func__,
						i+1,
						mbox_msg->cmd[i]);
				}
#endif
			}
		}
	} else {
		ret = -EINVAL;
	}
	return ret;
}

static s32 tcc_multi_mbox_ofdata_to_platdata(struct udevice *dev)
{
	struct tcc_multi_mbox *tcc = dev_get_platdata(dev);
	fdt_addr_t addr;
	s32 ret = -ENODEV;

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

static s32 tcc_multi_mbox_probe(struct udevice *dev)
{
	struct tcc_multi_mbox *tcc = dev_get_platdata(dev);
	s32 ret = -ENODEV;

	if (tcc != NULL) {

		pr_debug("[DEBUG] %s: %s(dev=%p)\n",
				dev->name, __func__, dev);

		/*receive irq disable */
		writel((readl(tcc->reg_base + MBOXCTR) & ~((uint32_t)IEN_BIT)),
			tcc->reg_base + MBOXCTR);

		/* disable data output. */
		writel((readl(tcc->reg_base + MBOXCTR) & ~((uint32_t)OEN_BIT)),
			tcc->reg_base + MBOXCTR);
#if 0
		/* flush cmd fifo & data fifo*/
		writel((readl(tcc->reg_base + MBOXCTR)
			| D_FLUSH_BIT | FLUSH_BIT),
			tcc->reg_base + MBOXCTR);
#endif
		writel(readl(tcc->reg_base + TRM_STS)
			| (uint32_t)OWN_TMN_STS_BIT,
			tcc->reg_base + TRM_STS);

		ret = 0;
	}

	return ret;
}

static s32 tcc_multi_mbox_remove(struct udevice *dev)
{
	struct tcc_multi_mbox *tcc = dev_get_platdata(dev);
	s32 ret = -ENODEV;

	if (tcc != NULL) {

		pr_debug("[DEBUG] %s: %s(dev=%p)\n",
				dev->name, __func__, dev);

		writel(readl(tcc->reg_base + TRM_STS) &
			~(uint32_t)OWN_TMN_STS_BIT, tcc->reg_base + TRM_STS);

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
	.remove = tcc_multi_mbox_remove,
	.platdata_auto_alloc_size = (int32_t) sizeof(struct tcc_multi_mbox),
	.ops = &tcc_multi_mbox_ops,
};

