// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Amit Singh Tomar <amittomer25@gmail.com>
 *
 * Actions DWMAC specific glue layer
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm.h>
#include <mach/clock.h>
#include <phy.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
// #include "designware.h"
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include "dwmac4.h"

#define  GMAC_CONFIG_INTF_SEL_MASK  (0x7000000u)
#define  GMAC_CONFIG_INTF_RGMII         (0x1000000u)
#define  GMAC_CONFIG_INTF_GMII          (0x0000000u)
#define  GMAC_CONFIG_INTF_RMII          (0x4000000u)
#define  GMAC_CONFIG_INTF_MII           (0x6000000u)
#define  GMAC_CONFIG1_CE                        (0x10000000u)
#define GMAC_SW_CONFIG0_REG                     (0x0u)
#define GMAC_SW_CONFIG1_REG                     (0x4u)

struct dwmac_tcc_delay {
	int txclk_delay;
	int rxclk_delay;
};

struct dwmac_tcc_plat {
	struct dwmac_tcc_delay gmac_delay;
	int phy_interface;
	void *gmac_base;
	void *hsio_base;
	struct clk pclk; // gmac peri
	struct clk fclk; // hsio bus
};

static int dwmac_tcc_probe(struct udevice *dev)
{
	return designware_eth_probe(dev);
	return 0;
}


static void dwmac_tcc_write_reg_16(struct dwmac_tcc_plat *plat, u32 reg, u32 val)
{
	u16 *addr;
	u16 data;

	addr = (void *)(plat->hsio_base + (uintptr_t)reg + 0x2);
	data = (val >> 16) & 0xFFFF;
	*((u16 *)(addr)) = data;
}

static int dwmac_tcc_set_phy_interface(struct udevice *dev, struct dwmac_tcc_plat * plat)
{
	int ret ;
	unsigned int sw_config1 = 0;

	sw_config1 &= ~GMAC_CONFIG_INTF_SEL_MASK;

	ret = 0;
	if (plat->phy_interface == PHY_INTERFACE_MODE_RGMII) {
		ret = pinctrl_select_state(dev, "rgmii");
		if (ret < 0) {
			pr_err("[ERROR][GMAC] %s. pinctrl error\n", __func__);
			return ret;
		} else {
			ret = clk_set_rate(&plat->pclk, 125000000UL);
			if (ret != 0) {
				pr_err("[ERROR][GMAC] %s. set clock failed\n", __func__);
			} else {
				ret = clk_set_rate(&plat->fclk, 300000000UL);
				if (ret != 0 ) {
					pr_err("[ERROR][GMAC] %s. set hsio clock failed",
							__func__);
				} else {
					sw_config1 |= (u32)(GMAC_CONFIG_INTF_RGMII
								& GMAC_CONFIG_INTF_SEL_MASK);
					sw_config1 |= GMAC_CONFIG1_CE;
					dwmac_tcc_write_reg_16(plat, GMAC_SW_CONFIG1_REG, sw_config1);
				}
			}
		}
	} else {
		pr_err("[ERROR][GMAC]%s. invalid phy interface\n", __func__);
		ret = -1;
	}

	return ret;
}

// writel(value, addr);
static void dwmac_tcc_set_delay(struct dwmac_tcc_plat *plat)
{
	int i;

	// reset all delay settings.
	for (i=0; i<7; i++) {
		writel(0x0, plat->gmac_base + 0x2000 + i*4);
	}

	if (plat->gmac_delay.rxclk_delay == -1) {
		pr_err("%s. rxclk_delay invalid\n", __func__);
	} else {
		writel(plat->gmac_delay.rxclk_delay, plat->gmac_base + 0x200C);
	}

	if (plat->gmac_delay.txclk_delay == -1) {
		pr_err("%s. txclk_delay invalid\n", __func__);
	} else {
		writel((plat->gmac_delay.txclk_delay << 8), plat->gmac_base + 0x2000);
	}

}

// const char *dev_read_string(const struct udevice *dev, const char *propname)
// int dev_read_u32(const struct udevice *dev, const char *propname, u32 *outp)
static int dwmac_tcc_of_to_plat(struct udevice *dev)
{
	struct dwmac_tcc_plat *pdata = (struct dwmac_tcc_plat *)dev_get_plat(dev);
	const char * phy_intf_name;
	int ret;

	pdata->gmac_base = (void *)dev_read_addr_index(dev, 0);
	pdata->hsio_base = (void *)dev_read_addr_index(dev, 1);
	phy_intf_name = (char *)dev_read_string(dev, "phy-interface");

	ret = tcc_clk_get_by_index(dev, 0, &pdata->pclk);
	if (ret < 0) {
		(void)pr_err("[ERROR][GMAC] failed to get peripheral clock\n");
	}

	ret = tcc_clk_get_by_index(dev, 1, &pdata->fclk);
	if (ret < 0) {
		(void)pr_err("[ERROR][GMAC] failed to get hsio clock\n");
	}

	ret = dev_read_u32(dev, "txclk_delay", &pdata->gmac_delay.txclk_delay);
	if (ret != 0) {
		pr_err("%s. txclk_delay cannot found\n", __func__);
		pdata->gmac_delay.txclk_delay = -1;
	}

	ret = dev_read_u32(dev, "rxclk_delay", &pdata->gmac_delay.rxclk_delay);
	if (ret != 0) {
		pr_err("%s. rxclk_delay cannot found\n", __func__);
		pdata->gmac_delay.rxclk_delay = -1;
	}

	pdata->phy_interface = phy_get_interface_by_name(phy_intf_name);

	if (pdata->hsio_base == NULL) {
		pr_err("%s. hsio_base is NULL \n", __func__);
		return -EINVAL;
	}
	if ((fdt_addr_t)pdata->hsio_base == FDT_ADDR_T_NONE) {
		pr_err("%s. hsio_base is FDT_ADDR_T_NONEr \n", __func__);
		return -EINVAL;
	}

	if (pdata->gmac_base == NULL) {
		pr_err("%s. gmac_base is NULL \n", __func__);
		return -EINVAL;
	}
	if ((fdt_addr_t)pdata->gmac_base == FDT_ADDR_T_NONE) {
		pr_err("%s. gmac_base is FDT_ADDR_T_NONEr \n", __func__);
		return -EINVAL;
	}

	dwmac_tcc_set_delay(pdata);

	ret = dwmac_tcc_set_phy_interface(dev, pdata);
	if (ret != 0) {
		return -EINVAL;
	}

	// return 0;
	return designware_eth_of_to_plat(dev);
}

static const struct udevice_id dwmac_tcc_ids[] = {
	{.compatible = "telechips,gmac" },
	{ }
};

U_BOOT_DRIVER(dwmac_tcc) = {
	.name   = "dwmac_tcc",
	.id     = UCLASS_ETH,
	.of_match = dwmac_tcc_ids,
	.of_to_plat = dwmac_tcc_of_to_plat,
	.probe  = dwmac_tcc_probe,
	.ops    = &designware_eth_ops,
	.priv_auto	= sizeof(struct dw_eth_dev),
	.plat_auto	= sizeof(struct dwmac_tcc_plat),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
