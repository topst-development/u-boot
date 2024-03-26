// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
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
	struct mmc *sc_mmc;
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
	struct mmc plat_mmc;
};

static int tcc_sc_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
		   struct mmc_data *data)
{
	struct mmc *sc_mmc = mmc_get_mmc_dev(dev);
	const struct tcc_sc_mmc_host *host;
	const struct tcc_sc_fw_handle *handle;
	int ret;

	host = (struct tcc_sc_mmc_host *)sc_mmc->priv;
	if (host == NULL)
		return -EINVAL;

	handle = host->fw_handle;

	if (handle == NULL)
		return -ENODEV;

	if (data != NULL) {
		dcache_disable();
	}

	ret = handle->ops.mmc_ops.request_command(host->fw_handle,
				cmd, data, mmc_get_blk_desc(sc_mmc)->hwpart);

	if (data != NULL) {
		dcache_enable();
	}

	return ret;
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
		const struct tcc_sc_mmc_host *host, u32 max_clk, u32 min_clk)
{
	u32 mode_parse[6] = {
		MMC_LEGACY,
		MMC_MODE_HS | MMC_MODE_HS_52MHz,
		MMC_MODE_DDR_52MHz,
		MMC_MODE_HS200,
		MMC_MODE_HS400,
		MMC_MODE_HS400_ES
	};
	u32 i;

	cfg->name = host->name;
	cfg->f_min = min_clk;
	if (host->mmc_prot_info.clock != 0u) {
		cfg->f_max = host->mmc_prot_info.clock;
	} else {
		cfg->f_max = max_clk;
	}

	cfg->voltages = (uint) MMC_VDD_32_33 | (uint) MMC_VDD_33_34;

	cfg->host_caps = host->caps;

	cfg->host_caps |=
		((unsigned int)MMC_MODE_1BIT <<
			(host->mmc_prot_info.bus_width & 0x3U));
	for (i = 0; i <= host->mmc_prot_info.speed_mode; i++) {
		cfg->host_caps |= mode_parse[i];
	}

	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	(void)pr_info("[INFO] %s: %s\n", host->name, __func__);
}

static int tcc_sc_mmc_probe(struct udevice *dev)
{
	int ret;
	struct tcc_sc_mmc_host *host;
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct tcc_sc_mmc_priv *priv = dev_get_priv(dev);
	struct tcc_sc_mmc_plat *plat = dev_get_plat(dev);
	const struct tcc_sc_fw_handle *handle = tcc_sc_fw_get_handle_by_name();

	if ((upriv == NULL) || (priv == NULL)
		|| (plat == NULL) || (handle == NULL)) {

		(void)pr_err(
			"[ERROR] %s : %s: get failed in udevice (dev=%p)\n"
				, dev->name, __func__, (void *)dev);
		return -1;
	}

	host = &(priv->host);

	ret = handle->ops.mmc_ops.prot_info(handle, &host->mmc_prot_info);
	if (ret != 0) {
		(void)pr_err(
			"[ERROR] %s: faile to get tcc sc fw mmc protocol info\n",
			dev->name);
		return ret;
	}

	(void)pr_debug("[DEBUG] %s: %s (dev=%p)\n"
			, dev->name, __func__, (void *)dev);

	(void)pr_info(
			"[INFO] %s: %s max_segs 0x%x max_seg_len 0x%x blk_size 0x%x max_blk_num 0x%x\n"
		, dev->name, __func__,
		host->mmc_prot_info.max_segs, host->mmc_prot_info.max_seg_len,
		host->mmc_prot_info.blk_size, host->mmc_prot_info.max_blk_num);

	host->fw_handle = handle;
	host->name = dev->name;
	host->sc_mmc = &plat->plat_mmc;
	host->sc_mmc->dev = dev;
	host->sc_mmc->priv = host;
	upriv->mmc = host->sc_mmc;

	tcc_sc_mmc_setup_cfg(&plat->cfg, host, 200 * 1000 * 1000, 400 * 1000);

	return 0;
}

#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
static int tcc_sc_mmc_set_enhanced_strobe(struct udevice *dev)
{
	struct tcc_sc_mmc_plat *plat = dev_get_plat(dev);

	if ((plat->cfg.host_caps & MMC_MODE_HS400_ES) != 0) {
		return 0;
	} else {
		return -ENOTSUPP;
	}
}
#endif

static const struct dm_mmc_ops tcc_sc_mmc_ops = {
	.send_cmd	= tcc_sc_mmc_send_cmd,
	.set_ios	= tcc_sc_mmc_set_ios,
#ifdef MMC_SUPPORTS_TUNING
	.execute_tuning	= tcc_sc_mmc_execute_tuning,
#endif
#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
	.set_enhanced_strobe = tcc_sc_mmc_set_enhanced_strobe,
#endif
};

static int tcc_sc_mmc_bind(struct udevice *dev)
{
	struct tcc_sc_mmc_plat *plat = dev_get_plat(dev);

	return mmc_bind(dev, &plat->plat_mmc, &plat->cfg);
}

static const struct udevice_id tcc_sc_mmc_ids[3] = {
	{ .compatible = "telechips,tcc805x-sc-mmc" },
	{ .compatible = "telechips,tcc807x-sc-mmc" },
	{ }
};

U_BOOT_DRIVER(tcc_sc_mmc_drv) = {
	.name               = "tcc_sc_mmc",
	.id                 = UCLASS_MMC,
	.of_match           = tcc_sc_mmc_ids,
	.bind               = tcc_sc_mmc_bind,
	.ops                = &tcc_sc_mmc_ops,
	.probe              = tcc_sc_mmc_probe,
	.priv_auto          = (int) sizeof(struct tcc_sc_mmc_priv),
	.plat_auto          = (int) sizeof(struct tcc_sc_mmc_plat),
};

