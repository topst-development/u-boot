// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <errno.h>
#include <clk.h>
#include <dm.h>
#include <sdhci.h>
#include <pwrseq.h>
#include <linux/delay.h>
#include <mach/clock.h>
#include "mmc_private.h"

#define ENABLE  (1)
#define DISABLE (0)

/* SD Host Controller Registers */
#define SDHC_BTCONT           0x070
#define SDHC_PRESET4          0x074
#define SDHC_VENDOR           0x078
 #define SDHC_VENDOR_ENHANCED_STROBE    (0x1)
 #define SDHC_VENDOR_DAT_DLY            ((0x0 & 0x3) << 12)
 #define SDHC_VENDOR_CMD_DLY            ((0x0 & 0x3) << 8)

/* Channel Control Registers */
/* Channel Control Offset */
#define SDHC_CHCTRL_TAPDLY              0x00
 #define SDHC_CHCTRL_OTAP_MASK          0x1F
 #define SDHC_CHCTRL_OTAP_SHIFT         8
 #define SDHC_CHCTRL_OTAP_EN_SHIFT      13
#define SDHC_CHCTRL_CAP_0               0x04
#define SDHC_CHCTRL_CAP_1               0x08
#define SDHC_CHCTRL_PRESET_0            0x0C
#define SDHC_CHCTRL_RESET_1             0x10
#define SDHC_CHCTRL_PRESET_2            0x14
#define SDHC_CHCTRL_PRESET_3            0x18
#define SDHC_CHCTRL_PRESET_4            0x1C
#define SDHC_CHCTRL_MAXCRNT             0x20
#define SDHC_CHCTRL_CQ                  0x24
#define SDHC_CHCTRL_DBG_0               0x3C
#define SDHC_CHCTRL_DBG_1               0x40
#define SDHC_CHCTRL_DBG_2               0x44
#define SDHC_CHCTRL_MON                 0x48
#define SDHC_CHCTRL_CD_WP               0x4C
/* Channel Clock Delay Offset */
#define SDHC_CHCTRL_AUTO_TUNE_RTL       0x00
#define SDHC_CHCTRL_SMPL_SFT_CLK_DIV    0x04
#define SDHC_CHCTRL_SMPL_SFT_CLK_CTL    0x08
#define SDHC_CHCTRL_SMPL_SFT_TIM_CTL    0x0C
 #define SDHC_CHCTL_SMPL_SFT_TAP_MASK           0x0F
 #define SDHC_CHCTL_SMPL_SFT_POS_TAP_SHIFT      4
 #define SDHC_CHCTL_SMPL_SFT_NEG_TAP_SHIFT      20
#define SDHC_CHCTRL_TX_CLK_DLY_0_1      0x10
#define SDHC_CHCTRL_TX_CLK_DLY_2        0x14
#define SDHC_CHCTRL_SD0_DQS_DLY         0x18
#define SDHC_CHCTRL_RX_CLK_DLY_FINE_VAL 0x2C
/* Channel CMD and Data Delay Offset */
#define SDHC_CHCTRL_SD_CMD_DLY          0x00
#define SDHC_CHCTRL_SD_DATA0_DLY        0x04
#define SDHC_CHCTRL_SD_DATA1_DLY        0x08
#define SDHC_CHCTRL_SD_DATA2_DLY        0x0C
#define SDHC_CHCTRL_SD_DATA3_DLY        0x10
#define SDHC_CHCTRL_SD_DATA4_DLY        0x14
#define SDHC_CHCTRL_SD_DATA5_DLY        0x18
#define SDHC_CHCTRL_SD_DATA6_DLY        0x1C
#define SDHC_CHCTRL_SD_DATA7_DLY        0x20

#define TCC_SDHC_MAX_CONT_NUM 2

struct tcc_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	void *base;
	void *chctrl_base;
	void *chclkdly_base;
	void *chcddly_base;
	uint32_t max_clk;
	uint32_t peri_clk;
	int32_t index;
	int32_t bus_width;

	uint32_t taps[4];
	uint32_t hs400_pos_tap;
	uint32_t hs400_neg_tap;

	bool increase_skew;
};

DECLARE_GLOBAL_DATA_PTR;

struct tcc_sdhci_priv {
	struct sdhci_host host;
};

static int tcc_sdhci_platform_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct tcc_sdhci_plat *plat = dev_get_plat(mmc->dev);
	struct sdhci_host *host = mmc->priv;
	struct mmc_cmd cmd;
	struct mmc_data data;
	uint32_t reg_val;
	uint16_t w_reg_val;
	int tuning_count;
	uint32_t rx_auto_tune_dis, rx_tune_rst;

	reg_val = readl(plat->chctrl_base + SDHC_CHCTRL_TAPDLY);
	rx_auto_tune_dis = (reg_val & 0x00000020u);
	if (rx_auto_tune_dis != 0u) {
		pr_warn("[WARN] %s: Rx auto-tuning is disabled.\n", host->name);
		return 0;
	}

	w_reg_val = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	w_reg_val |= (unsigned int)SDHCI_CTRL_EXEC_TUNING;
	sdhci_writew(host, w_reg_val, SDHCI_HOST_CONTROL2);

	sdhci_writel(host, SDHCI_INT_DATA_AVAIL, SDHCI_INT_ENABLE);
	sdhci_writel(host, SDHCI_INT_DATA_AVAIL, SDHCI_SIGNAL_ENABLE);

	for (tuning_count = 0; tuning_count < 40; tuning_count++) {
		cmd.cmdidx = opcode;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 0;

		data.blocksize = 64;
		data.blocks = 1;
		data.flags = MMC_DATA_READ;

		if ((cmd.cmdidx == (ushort)MMC_CMD_SEND_TUNING_BLOCK_HS200) &&
				(mmc->bus_width == 8u))
			data.blocksize = 128;

		sdhci_writew(host,
			(u16)SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
				data.blocksize),
			SDHCI_BLOCK_SIZE);
		sdhci_writew(host, (u16)data.blocks, SDHCI_BLOCK_COUNT);
		sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE);

		mmc_send_cmd(mmc, &cmd, NULL);

		w_reg_val = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		if ((w_reg_val & (u16)SDHCI_CTRL_EXEC_TUNING) == (u16)0) {
			break;
		}
	}

	if ((w_reg_val & (u16)SDHCI_CTRL_TUNED_CLK) == (u16)0) {
		pr_err("[ERROR] %s: Tuning procedure has failed.\n",
				host->name);
		sdhci_writew(host, w_reg_val, SDHCI_HOST_CONTROL2);
		return -EIO;
	}

	sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK,
		     SDHCI_INT_ENABLE);
	sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);

	reg_val = readl(plat->chclkdly_base + SDHC_CHCTRL_AUTO_TUNE_RTL);
	if (((reg_val >> ((unsigned int)host->index * 8u)) & 0x20u) == 0u) {
		pr_err("[ERROR] %s: rx auto tuning is not enabled\n",
				host->name);
		return -1;
	}

	rx_tune_rst = (reg_val >> ((unsigned int)host->index * 8u)) & 0x1Fu;
	pr_debug("[DEBUG] %s: rx auto tuning result: %#02x\n",
			host->name, rx_tune_rst);

	return 0;
}

static int tcc_sdhci_set_enhanced_strobe(struct sdhci_host *host) {
	struct mmc *mmc = host->mmc;
	u32 vendor;

	vendor = sdhci_readl(host, SDHC_VENDOR);
	if (mmc->selected_mode == MMC_HS_400_ES) {
		vendor |= SDHC_VENDOR_ENHANCED_STROBE;
	} else {
		vendor &= ~SDHC_VENDOR_ENHANCED_STROBE;
	}
	sdhci_writel(host, vendor, SDHC_VENDOR);

	return 0;
}

static void tcc_sdhci_set_control_reg(struct sdhci_host *host)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	uint32_t reg;

	if (IS_SD(host->mmc) &&
		((uint)mmc->signal_voltage == (uint)MMC_SIGNAL_VOLTAGE_180)) {
		reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		reg |= (uint)SDHCI_CTRL_VDD_180;
		sdhci_writew(host, (ushort)reg, SDHCI_HOST_CONTROL2);
	}

	sdhci_set_uhs_timing(host);

#if defined(CONFIG_MMC_HS400_ES_SUPPORT)
	/*
	 * Reinitializing the device tries to set it to lower-speed modes
	 * first, which fails if the Enhanced Strobe bit is set, making
	 * the device impossible to use. Set the correct value here to
	 * let reinitialization attempts succeed.
	 */
	(void)tcc_sdhci_set_enhanced_strobe(host);
#endif
}

static int tcc_sdhci_set_tapdelay(struct sdhci_host *host)
{
	struct tcc_sdhci_plat *plat = dev_get_plat(host->mmc->dev);
	struct mmc *mmc = (struct mmc *)host->mmc;
	uint32_t reg = 0;
	uint32_t pos_tap, neg_tap;

	/* Output Tap Delay */
	reg = readl(plat->chctrl_base + SDHC_CHCTRL_TAPDLY);
	reg &= ~(SDHC_CHCTRL_OTAP_MASK << SDHC_CHCTRL_OTAP_SHIFT);
	reg |= (plat->taps[0] << SDHC_CHCTRL_OTAP_SHIFT) |
		(0x1u << SDHC_CHCTRL_OTAP_EN_SHIFT);
	writel(reg, plat->chctrl_base + SDHC_CHCTRL_TAPDLY);

	/* CMD Tap Delay */
	reg = (plat->taps[1] << 16) | (plat->taps[1] << 8) | 0xF;
	writel(reg, plat->chcddly_base + SDHC_CHCTRL_SD_CMD_DLY);

	/* Data Tap Delay */
	reg = (plat->taps[2] << 16) | (plat->taps[2] << 8) | 0xF;
	writel(reg, plat->chcddly_base + SDHC_CHCTRL_SD_DATA0_DLY);
	writel(reg, plat->chcddly_base + SDHC_CHCTRL_SD_DATA1_DLY);
	writel(reg, plat->chcddly_base + SDHC_CHCTRL_SD_DATA2_DLY);
	writel(reg, plat->chcddly_base + SDHC_CHCTRL_SD_DATA3_DLY);
	writel(reg, plat->chcddly_base + SDHC_CHCTRL_SD_DATA4_DLY);
	writel(reg, plat->chcddly_base + SDHC_CHCTRL_SD_DATA5_DLY);
	writel(reg, plat->chcddly_base + SDHC_CHCTRL_SD_DATA6_DLY);
	writel(reg, plat->chcddly_base + SDHC_CHCTRL_SD_DATA7_DLY);

	/*
	 * RX Tuning succeeds, but Read fails, so DAT1 output delay is changed
	 * See TMSVT-1482 TCS for details (TCC750x onlay)
	 */
	if (plat->increase_skew) {
		pr_debug("[DEBUG] %s: Change DAT1 Tap Delay(0x4) \n", __func__);
		reg = readl(plat->chcddly_base + SDHC_CHCTRL_SD_DATA1_DLY);
		reg |= (uint32_t)0x4;
		writel(reg, plat->chcddly_base + SDHC_CHCTRL_SD_DATA1_DLY);
	}

	/* Clk TX Tap Delay */
	if (host->index < 2) {
		reg = plat->taps[3] << ((unsigned int)host->index << 4u);
		writel(reg, plat->chclkdly_base + SDHC_CHCTRL_TX_CLK_DLY_0_1);
	} else {
		reg = ((plat->taps[3] << 15) | plat->taps[3]) & 0x000F0001u;
		writel(reg, plat->chclkdly_base + SDHC_CHCTRL_TX_CLK_DLY_2);
	}

	if ((mmc->selected_mode == MMC_HS_400) ||
			(mmc->selected_mode == MMC_HS_400_ES)) {
		reg = 0x0001000F;
		writel(reg, plat->chclkdly_base + SDHC_CHCTRL_SD0_DQS_DLY);
		pos_tap = plat->hs400_pos_tap;
		neg_tap = plat->hs400_neg_tap;
		reg = (0x2u << 28u) |
			((neg_tap & 0xFu)<< 20u) |
			((pos_tap & 0xFu) << 4u);
		writel(reg, plat->chclkdly_base + SDHC_CHCTRL_SMPL_SFT_TIM_CTL);
	}

	return 0;
}

/*
 * This function is useful when configuring some registers after sw reset.
 * sdhci_probe()
 *     sdhci_init()
 *         sdhci_reset(host, SDHCI_RESET_ALL)
 * sdhci_deferred_probe()
 * Send first CMD ...
 */
static int tcc_sdhci_deferred_probe(struct sdhci_host *host)
{
#if defined(CONFIG_TCC807X)
	uint32_t reg_val;

	/* For chip boundary issue */
	reg_val = sdhci_readl(host, SDHC_VENDOR);
	reg_val &= (~0x00003300);
	reg_val |= (SDHC_VENDOR_DAT_DLY | SDHC_VENDOR_CMD_DLY);
	sdhci_writel(host, reg_val, SDHC_VENDOR);
#endif

	return 0;
}

const struct sdhci_ops tcc_sdhci_ops = {
	.platform_execute_tuning = &tcc_sdhci_platform_execute_tuning,
	.set_control_reg = tcc_sdhci_set_control_reg,
	.set_delay = tcc_sdhci_set_tapdelay,
	.set_enhanced_strobe = tcc_sdhci_set_enhanced_strobe,
	.deferred_probe = tcc_sdhci_deferred_probe,
};

static int tcc_sdhci_clk_init(struct udevice *dev)
{
	struct tcc_sdhci_plat *plat = dev_get_plat(dev);
	int ret, peri_id = 0, iobus_id = 0;
#if defined(CONFIG_TCC750X)
	int pswrst_id = 0;
#endif
	int div;
	u32 get_peri;
	u32 reg_val;
	/* set clock flags */
	int enable_clk_conf = 1;
	int use_high_peri = 0;
	int use_iobus = true;

#if defined(CONFIG_TCC803X)
	if ((plat->index == 0) &&
			((plat->cfg.host_caps & MMC_MODE_HS400) ||
			(plat->cfg.host_caps & MMC_MODE_HS400_ES))) {
		use_high_peri = 1;
	}

	iobus_id = (int)IOBUS_SDMMC0 + plat->index;
	peri_id = (int)PERI_SDMMC0 + plat->index;
#elif defined(CONFIG_TCC750X)
	peri_id = PERI_SDMMC;
	pswrst_id = SYSPERI_SDMMC;
	use_iobus = 0;
	use_high_peri = 1;
#else /* TCC805x, TCC807x */
	if (plat->index == 0) {
		enable_clk_conf = 0;
		use_high_peri = 1;
	}

	iobus_id = (int)IOBUS_SDMMC1 + plat->index - 1;
	peri_id = (int)PERI_SDMMC1 + plat->index - 1;
#endif

	if (enable_clk_conf != 0) {
		if (use_iobus == true ) {
			(void)tcc_set_iobus_swreset(iobus_id, ENABLE);
			(void)tcc_set_iobus_swreset(iobus_id, DISABLE);

			(void)tcc_set_iobus_pwdn(iobus_id, DISABLE);

			(void)tcc_set_peri(peri_id, ENABLE, plat->peri_clk, 0);
		} else { /* tcc750x*/
			//(void)tcc_ckc_swreset(pswrst_id, CKC_DISABLE);

			(void)tcc_set_peri(peri_id, DISABLE, plat->peri_clk, 0);
			(void)tcc_set_peri(peri_id, ENABLE, plat->peri_clk, 0);

			//(void)tcc_ckc_swreset(pswrst_id, CKC_ENABLE);
		}
	}

	if (use_high_peri != 0) {
		if (enable_clk_conf != 0) {
			/*
			 * Prepare clock configuration
			 *   disable dividor
			 *   enable clk gating
			 */
			reg_val = 0x00000002;
			writel(reg_val,
				plat->chclkdly_base +
				SDHC_CHCTRL_SMPL_SFT_CLK_DIV);
			reg_val = 0x00000001;
			writel(reg_val,
				plat->chclkdly_base +
				SDHC_CHCTRL_SMPL_SFT_CLK_CTL);
			(void)tcc_set_peri(peri_id, ENABLE, plat->peri_clk, 0);

			get_peri = tcc_get_peri(peri_id);
			(void)tcc_set_peri(peri_id, DISABLE, plat->peri_clk, 0);
		} else {
			get_peri = plat->peri_clk;
		}

		reg_val = 0x00000001;
		writel(reg_val,
			plat->chclkdly_base + SDHC_CHCTRL_SMPL_SFT_CLK_CTL);

		div = ((get_peri + plat->max_clk - 1) / plat->max_clk) - 1;
		reg_val = (div << 8);
		writel(reg_val,
			plat->chclkdly_base + SDHC_CHCTRL_SMPL_SFT_CLK_DIV);

		reg_val = (div << 8) | (0x00000002);
		writel(reg_val,
			plat->chclkdly_base + SDHC_CHCTRL_SMPL_SFT_CLK_DIV);

		reg_val = (div << 8) | (0x00000003);
		writel(reg_val,
			plat->chclkdly_base + SDHC_CHCTRL_SMPL_SFT_CLK_DIV);

		reg_val = 0x00010001;
		writel(reg_val,
			plat->chclkdly_base + SDHC_CHCTRL_SMPL_SFT_CLK_CTL);

		reg_val = 0x00010000;
		writel(reg_val,
			plat->chclkdly_base + SDHC_CHCTRL_SMPL_SFT_CLK_CTL);

		if (enable_clk_conf != 0) {
			(void)tcc_set_peri(peri_id, ENABLE, plat->peri_clk, 0);
		}
	}

	if (enable_clk_conf != 0) {
		pr_debug("[DEBUG] %s: actual peri clock: %lu\n",
				dev->name, tcc_get_peri(peri_id));
	}

	ret = 0;

	return ret;
}

static int tcc_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct tcc_sdhci_plat *plat = dev_get_plat(dev);
	struct mmc_config *cfg = &plat->cfg;
	int ret;

	plat->base = (void *)dev_read_addr_index(dev, 0);
	ret = (int)IS_ERR(plat->base);
	if (ret != 0) {
		pr_err("[ERROR] %s: Cannot get base address\n", dev->name);
		return (int)PTR_ERR(plat->base);
	}
	plat->chctrl_base = (void *)dev_read_addr_index(dev, 1);
	ret = (int)IS_ERR(plat->chctrl_base);
	if (ret != 0) {
		pr_err("[ERROR] %s: Cannot get chctrl_base address\n",
				dev->name);
		return (int)PTR_ERR(plat->chctrl_base);
	}
	plat->chclkdly_base = (void *)dev_read_addr_index(dev, 2);
	ret = (int)IS_ERR(plat->chclkdly_base);
	if (ret != 0) {
		pr_err("[ERROR] %s: Cannot get chclkdly_base address\n",
				dev->name);
		return (int)PTR_ERR(plat->chclkdly_base);
	}
	plat->chcddly_base = (void *)dev_read_addr_index(dev, 3);
	ret = (int)IS_ERR(plat->chcddly_base);
	if (ret != 0) {
		pr_err("[ERROR] %s: Cannot get chcddly_base address\n",
				dev->name);
		return (int)PTR_ERR(plat->chcddly_base);
	}

	plat->index = dev_read_u32_default(dev, "controller-id", 0);
	if (plat->index > TCC_SDHC_MAX_CONT_NUM) {
		pr_err("[ERROR] %s: invalid controller-id %d\n", dev->name,
				plat->index);
		return -EINVAL;
	}

	ret = dev_read_u32_array(dev, "tcc-mmc-taps", plat->taps, 4);
	if (ret != 0) {
		plat->taps[0] = 0xF;
		plat->taps[1] = 0xF;
		plat->taps[2] = 0xF;
		plat->taps[3] = 0xF;
	}

	if (device_is_compatible(dev, "telechips,tcc750x-sdhci"))
		plat->increase_skew = true;

	plat->hs400_pos_tap =
		(uint)dev_read_u32_default(dev, "tcc-mmc-hs400-pos-tap", 0);
	plat->hs400_pos_tap =
		plat->hs400_pos_tap & SDHC_CHCTL_SMPL_SFT_TAP_MASK;
	plat->hs400_neg_tap =
		(uint)dev_read_u32_default(dev, "tcc-mmc-hs400-neg-tap", 0);
	plat->hs400_neg_tap =
		plat->hs400_neg_tap & SDHC_CHCTL_SMPL_SFT_TAP_MASK;

	plat->bus_width = dev_read_u32_default(dev, "bus-width", 4);

	plat->max_clk = (uint)dev_read_u32_default(dev, "max-frequency",
			50000000);
	plat->peri_clk = (uint)dev_read_u32_default(dev, "peri-clock-frequency",
			(s32)plat->max_clk);

	ret = mmc_of_parse(dev, cfg);

	pr_debug("[DEBUG] %s: base              : 0x%p\n",
			dev->name, plat->base);
	pr_debug("[DEBUG] %s: chctrl_base       : 0x%p\n",
			dev->name, plat->chctrl_base);
	pr_debug("[DEBUG] %s: chclkbly_base     : 0x%p\n",
			dev->name, plat->chclkdly_base);
	pr_debug("[DEBUG] %s: chcdbly_base      : 0x%p\n",
			dev->name, plat->chcddly_base);
	pr_debug("[DEBUG] %s: bus_width         : %d\n",
			dev->name, plat->bus_width);
	pr_debug("[DEBUG] %s: index             : %d\n",
			dev->name, plat->index);
	pr_debug("[DEBUG] %s: max_clk           : %d\n",
			dev->name, plat->max_clk);
	pr_debug("[DEBUG] %s: peri_clk          : %u\n",
			dev->name, plat->peri_clk);
	pr_debug("[DEBUG] %s: otap delay        : 0x%08x\n",
			dev->name, plat->taps[0]);
	pr_debug("[DEBUG] %s: cmd delay         : 0x%08x\n",
			dev->name, plat->taps[1]);
	pr_debug("[DEBUG] %s: data delay        : 0x%08x\n",
			dev->name, plat->taps[2]);
	pr_debug("[DEBUG] %s: tx_clk delay      : 0x%08x\n",
			dev->name, plat->taps[3]);
	pr_debug("[DEBUG] %s: hs400 pos/neg taps: 0x%08x / 0x%08x\n",
			dev->name, plat->hs400_pos_tap, plat->hs400_neg_tap);
	pr_debug("[DEBUG] %s: cfg->host_caps    : %#08x\n",
			dev->name, cfg->host_caps);

	return 0;
}

static int tcc_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct tcc_sdhci_plat *plat = dev_get_plat(dev);
	struct tcc_sdhci_priv *priv = dev_get_priv(dev);
	struct sdhci_host *host = &(priv->host);
	int ret;
#ifdef CONFIG_PWRSEQ
	struct udevice *pwr_dev;
#endif
	/* Initialize clocks */
	ret = tcc_sdhci_clk_init(dev);
	if (ret != 0) {
		return ret;
	}

	if ((plat->cfg.host_caps & MMC_CAP_NONREMOVABLE) != 0u) {
		/* Set CAP register */
		writel(0x0DFFE470, plat->chctrl_base + SDHC_CHCTRL_CAP_0);
		if (((plat->cfg.host_caps & MMC_MODE_HS400) != 0u) ||
			(plat->cfg.host_caps & MMC_MODE_HS400_ES) != 0u) {
			writel(0x0000000F,
				plat->chctrl_base + SDHC_CHCTRL_CAP_1);
		} else {
			writel(0x00000007,
				plat->chctrl_base + SDHC_CHCTRL_CAP_1);
		}

		/* Set WP/CD */
		writel(0x00000000, plat->chctrl_base + SDHC_CHCTRL_CD_WP);
	} else {
		/* Set CAP register */
		writel(0x05FFE470, plat->chctrl_base + SDHC_CHCTRL_CAP_0);
		writel(0x00000007, plat->chctrl_base + SDHC_CHCTRL_CAP_1);

		if ((plat->cfg.host_caps & MMC_CAP_NEEDS_POLL) != 0u) {
			/* Set WP/CD */
			writel(0x00000000,
				plat->chctrl_base + SDHC_CHCTRL_CD_WP);
		} else {
			/* Set WP/CD */
			writel(0x00000001,
				plat->chctrl_base + SDHC_CHCTRL_CD_WP);
		}
	}

#ifdef CONFIG_PWRSEQ
	ret = uclass_get_device_by_phandle(UCLASS_PWRSEQ, dev, "mmc-pwrseq",
			&pwr_dev);
	if (ret == 0) {
		ret = pwrseq_set_power(pwr_dev, true);
		if (ret != 0) {
			return ret;
		}
	}
#endif

	/* Host Configurtaion */
	host->name = dev->name;
	host->ops = &tcc_sdhci_ops;
	host->ioaddr = plat->base;
	host->index = plat->index;
	host->bus_width = plat->bus_width;
	host->max_clk = plat->max_clk;
	host->quirks = SDHCI_QUIRK_32BIT_DMA_ADDR;
	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;
	ret = sdhci_setup_cfg(&plat->cfg, host, plat->max_clk, 400000);
	if (ret != 0) {
		return ret;
	}
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int tcc_sdhci_bind(struct udevice *dev)
{
	struct tcc_sdhci_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id tcc_sdhci_ids[5] = {
	{ .compatible = "telechips,tcc803x-sdhci" },
	{ .compatible = "telechips,tcc805x-sdhci" },
	{ .compatible = "telechips,tcc807x-sdhci" },
	{ .compatible = "telechips,tcc750x-sdhci" },
	{ }
};

U_BOOT_DRIVER(tcc_sdhci_drv) = {
	.name               = "sdhci-tcc",
	.id                 = UCLASS_MMC,
	.of_match           = tcc_sdhci_ids,
	.bind               = tcc_sdhci_bind,
	.of_to_plat         = tcc_sdhci_ofdata_to_platdata,
	.ops                = &sdhci_ops,
	.probe              = tcc_sdhci_probe,
	.priv_auto          = sizeof(struct tcc_sdhci_priv),
	.plat_auto          = sizeof(struct tcc_sdhci_plat),
};

#ifdef CONFIG_PWRSEQ
static int tcc_mmc_pwrseq_set_power(struct udevice *dev, bool enable)
{
	struct gpio_desc reset;
	int ret;

	ret = gpio_request_by_name(dev, "reset-gpios", 0, &reset, GPIOD_IS_OUT);
	if (ret != 0) {
		pr_err("[ERROR] %s: failed to get reset-gpios\n", dev->name);
		return ret;
	}

	ret = dm_gpio_set_value(&reset, 1);
	udelay(1);
	ret = dm_gpio_set_value(&reset, 0);
	udelay(200);

	pr_debug("[DEBUG] %s: set reset-gpios\n", dev->name);

	return 0;
}

static const struct pwrseq_ops tcc_mmc_pwrseq_ops = {
	.set_power = tcc_mmc_pwrseq_set_power,
};

static const struct udevice_id tcc_mmc_pwrseq_ids[2] = {
	{ .compatible = "mmc-pwrseq-simple" },
	{ }
};

U_BOOT_DRIVER(tcc_mmc_pwrseq_drv) = {
	.name     = "mmc-pwrseq-simple",
	.id       = UCLASS_PWRSEQ,
	.of_match = tcc_mmc_pwrseq_ids,
	.ops      = &tcc_mmc_pwrseq_ops,
};
#endif
