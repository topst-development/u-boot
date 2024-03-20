// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#define DEBUG

#include <bouncebuf.h>
#include <common.h>
#include <cpu_func.h>
#include <errno.h>
#include <malloc.h>
#include <memalign.h>
#include <mmc.h>
#include <wait_bit.h>
#include <dm.h>
#include <linux/soc/telechips/tcc_sc_protocol.h>

struct tcc_sc_mmc_host {
	const char *name;
	struct mmc *mmc;
	unsigned int caps;
	void *priv;
	const struct tcc_sc_fw_handle *fw_handle;
	struct tcc_sc_fw_prot_mmc mmc_prot_info;

	unsigned char part_num;
};

struct tcc_sc_mmc_priv {
	struct tcc_sc_mmc_host host;
};

struct tcc_sc_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static int tcc_sc_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
		   struct mmc_data *data)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	struct tcc_sc_mmc_host *host = mmc->priv;
	const struct tcc_sc_fw_handle *handle;

	if (host == NULL)
		return -EINVAL;

	handle = host->fw_handle;

	if (handle == NULL)
		return -ENODEV;

	if (data != NULL) {
		flush_cache((unsigned long)data->src,
			(unsigned long)data->blocks * data->blocksize);
	}

	return handle->ops.mmc_ops.request_command(host->fw_handle,
				cmd, data, mmc_get_blk_desc(mmc)->hwpart);
}

static int tcc_sc_mmc_execute_tuning(struct udevice *dev, uint opcode)
{
	/* do nothing */
	return 0;
}

static int tcc_sc_mmc_set_ios(struct udevice *dev)
{
	/* do nothing */
	return 0;
}

static void tcc_sc_mmc_setup_cfg(struct mmc_config *cfg,
		struct tcc_sc_mmc_host *host, u32 max_clk, u32 min_clk)
{
	u32 mode_parse[5] = {
		MMC_LEGACY,
		MMC_MODE_HS | MMC_MODE_HS_52MHz,
		MMC_MODE_DDR_52MHz,
		MMC_MODE_HS200,
		MMC_MODE_HS400
	};
	u32 i;

	cfg->name = host->name;
	cfg->f_min = min_clk;
	if (host->mmc_prot_info.clock != 0) {
		cfg->f_max = host->mmc_prot_info.clock;
	} else {
		cfg->f_max = max_clk;
	}

	cfg->voltages = (uint) MMC_VDD_32_33 | (uint) MMC_VDD_33_34;

	cfg->host_caps = host->caps;

	cfg->host_caps |=
		(MMC_MODE_1BIT << (host->mmc_prot_info.bus_width & 0x3));
	for (i = 0; i <= host->mmc_prot_info.speed_mode; i++) {
		cfg->host_caps |= mode_parse[i];
	}

	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	pr_debug("[DEBUG] %s: %s\n", host->name, __func__);
}

static int tcc_sc_mmc_probe(struct udevice *dev)
{
	int ret;
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct tcc_sc_mmc_priv *priv = dev_get_priv(dev);
	struct tcc_sc_mmc_plat *plat = dev_get_platdata(dev);
	struct tcc_sc_mmc_host *host = &(priv->host);
	const struct tcc_sc_fw_handle *handle = tcc_sc_fw_get_handle(dev);

	ret = handle->ops.mmc_ops.prot_info(handle, &host->mmc_prot_info);
	if (ret != 0) {
		pr_err(
			"[ERROR] %s: faile to get tcc sc fw mmc protocol info\n",
			dev->name);
		return ret;
	}

	pr_debug("[DEBUG] %s: %s (dev=%p)\n", dev->name, __func__, dev);
	pr_info("[INFO] %s: %s max_segs 0x%x max_seg_len 0x%x blk_size 0x%x max_blk_num 0x%x\n",
		dev->name, __func__,
		host->mmc_prot_info.max_segs, host->mmc_prot_info.max_seg_len,
		host->mmc_prot_info.blk_size, host->mmc_prot_info.max_blk_num);

	host->fw_handle = handle;
	host->name = dev->name;
	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;
	upriv->mmc = host->mmc;

	tcc_sc_mmc_setup_cfg(&plat->cfg, host, 200 * 1000 * 1000, 400 * 1000);

	return 0;
}

static const struct dm_mmc_ops tcc_sc_mmc_ops = {
	.send_cmd	= tcc_sc_mmc_send_cmd,
	.set_ios	= tcc_sc_mmc_set_ios,
#ifdef MMC_SUPPORTS_TUNING
	.execute_tuning	= tcc_sc_mmc_execute_tuning,
#endif
};

static int tcc_sc_mmc_bind(struct udevice *dev)
{
	struct tcc_sc_mmc_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id tcc_sc_mmc_ids[2] = {
	{ .compatible = "telechips,tcc805x-sc-mmc" },
	{ }
};

U_BOOT_DRIVER(tcc_sc_mmc_drv) = {
	.name               = "tcc_sc_mmc",
	.id                 = UCLASS_MMC,
	.of_match           = tcc_sc_mmc_ids,
	.bind               = tcc_sc_mmc_bind,
	.ops                = &tcc_sc_mmc_ops,
	.probe              = tcc_sc_mmc_probe,
	.priv_auto_alloc_size     = (int) sizeof(struct tcc_sc_mmc_priv),
	.platdata_auto_alloc_size = (int) sizeof(struct tcc_sc_mmc_plat),
};

