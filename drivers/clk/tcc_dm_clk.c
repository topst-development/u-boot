// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <clk-uclass.h>
#include <clk.h>
#include <dm.h>
#include <linux/arm-smccc.h>
#include <mach/clock.h>
#include <mach/smc.h>
#include <linux/sizes.h>
#include <linux/io.h>
#include <dm/uclass-internal.h>

// clock fbus
struct tcc_ckc_desc {
	char *name;
	unsigned int id;
	unsigned int flags;
	struct clk clk_desc;
};

struct tcc_ckc_fbus_priv {
	unsigned long num_fbus_clk;
	struct tcc_ckc_desc *fbus_clks;
};

static int tcc_ckc_fbus_probe(struct udevice *dev)
{
	(void)dev;
	/*
	 * Register operations only
	 */
	return 0;
}

static unsigned long tcc_ckc_fbus_set_rate(struct clk *pclk,
					   unsigned long rate)
{
	struct arm_smccc_res res;
	unsigned long ret;

	arm_smccc_smc(SIP_CLK_SET_CLKCTRL, pclk->id, 1,
		      rate, 0, 0, 0, 0, &res);
	ret = res.a0;

	return ret;
}

static unsigned long tcc_ckc_fbus_get_rate(struct clk *pclk)
{
	struct arm_smccc_res res;
	unsigned long ret;

	arm_smccc_smc(SIP_CLK_GET_CLKCTRL, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);
	ret = res.a0;

	return ret;
}

static int tcc_ckc_fbus_enable(struct clk *pclk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_CLK_ENABLE_CLKCTRL, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0UL) {
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

static int tcc_ckc_fbus_disable(struct clk *pclk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_CLK_DISABLE_CLKCTRL, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0UL) {
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

static const struct clk_ops tcc_ckc_fbus_ops = {
	.set_rate = tcc_ckc_fbus_set_rate,
	.get_rate = tcc_ckc_fbus_get_rate,
	.enable = tcc_ckc_fbus_enable,
	.disable = tcc_ckc_fbus_disable,
};

static const struct udevice_id tcc_ckc_fbus_id[] = {
	{.compatible = "telechips,clk-fbus"},
	{}
};

U_BOOT_DRIVER(tcc_fbus_ckc) = {
	.name = "tcc_ckc_fbus",
	.id = UCLASS_CLK,
	.ops = &tcc_ckc_fbus_ops,
	.of_match = tcc_ckc_fbus_id,
	.probe = tcc_ckc_fbus_probe,
	.flags = DM_FLAG_PRE_RELOC,
	.priv_auto_alloc_size = (int)sizeof(struct tcc_ckc_fbus_priv),
};

// clock peripheral
struct tcc_ckc_peri_priv {
	unsigned long num_peri_clk;
	struct tcc_ckc_desc *peri_clks;
};

static int tcc_ckc_peri_probe(struct udevice *dev)
{
	(void)dev;
	/*
	 * Regiter operations only
	 */
	return 0;
}

static unsigned long tcc_ckc_peri_set_rate(struct clk *pclk,
					   unsigned long rate)
{
	struct arm_smccc_res res;
	unsigned long ret;

	arm_smccc_smc(SIP_CLK_SET_PCLKCTRL, pclk->id,
		      1UL, rate, 0, 0, 0, 0, &res);
	ret = res.a0;

	return ret;
}

static unsigned long tcc_ckc_peri_get_rate(struct clk *pclk)
{
	struct arm_smccc_res res;
	unsigned long ret;

	arm_smccc_smc(SIP_CLK_GET_PCLKCTRL, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);
	ret = res.a0;

	return ret;
}

static int tcc_ckc_peri_enable(struct clk *pclk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_CLK_ENABLE_PERI, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0UL) {
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

static int tcc_ckc_peri_disable(struct clk *pclk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_CLK_DISABLE_PERI, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0UL) {
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

static int tcc_ckc_peri_request(struct clk *pclk)
{
	struct udevice *target_dev;
	int i = 0, ret = 0;

	while ((ret == 0) && (i < __INT_MAX__)) {
		ret = uclass_find_device(UCLASS_CLK, i, &target_dev);
		if (ret == 0) {
			if (strncmp("telechips,clk-peri", target_dev->name, 18) == 0) {
				pclk->dev = target_dev;
				break;
			}
			i++;
		}
	}

	return ret;
}

static const struct clk_ops tcc_ckc_peri_ops = {
	.set_rate = tcc_ckc_peri_set_rate,
	.get_rate = tcc_ckc_peri_get_rate,
	.request = tcc_ckc_peri_request,
	.enable = tcc_ckc_peri_enable,
	.disable = tcc_ckc_peri_disable,
};

static const struct udevice_id tcc_ckc_peri_id[] = {
	{.compatible = "telechips,clk-peri"},
	{}
};

U_BOOT_DRIVER(tcc_peri_ckc) = {
	.name = "tcc_ckc_peri",
	.id = UCLASS_CLK,
	.ops = &tcc_ckc_peri_ops,
	.of_match = tcc_ckc_peri_id,
	.probe = tcc_ckc_peri_probe,
	.flags = DM_FLAG_PRE_RELOC,
	.priv_auto_alloc_size = (int)sizeof(struct tcc_ckc_peri_priv),
};

// clock iobus
struct tcc_ckc_iobus_priv {
	unsigned long num_iobus_clk;
	struct tcc_ckc_desc *iobus_clks;
};

static int tcc_ckc_iobus_probe(struct udevice *dev)
{
	(void)dev;
	/*
	 * Register operations only
	 */
	return 0;
}

static int tcc_ckc_iobus_enable(struct clk *pclk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_CLK_ENABLE_IOBUS, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0UL) {
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

static int tcc_ckc_iobus_disable(struct clk *pclk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_CLK_DISABLE_IOBUS, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0UL) {
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

static int tcc_ckc_iobus_request(struct clk *pclk)
{
	struct udevice *target_dev;
	int i = 0, ret = 0;

	while ((ret == 0) && (i < __INT_MAX__)) {
		ret = uclass_find_device(UCLASS_CLK, i, &target_dev);
		if (ret == 0) {
			if (strncmp("telechips,clk-iobus", target_dev->name, 19) == 0) {
				pclk->dev = target_dev;
				break;
			}
			i++;
		}
	}

	return ret;
}

static const struct clk_ops tcc_ckc_iobus_ops = {
	.enable = tcc_ckc_iobus_enable,
	.disable = tcc_ckc_iobus_disable,
	.request = tcc_ckc_iobus_request,
};

static const struct udevice_id tcc_ckc_iobus_id[] = {
	{.compatible = "telechips,clk-iobus"},
	{}
};

U_BOOT_DRIVER(tcc_iobus_ckc) = {
	.name = "tcc_ckc_iobus",
	.id = UCLASS_CLK,
	.ops = &tcc_ckc_iobus_ops,
	.of_match = tcc_ckc_iobus_id,
	.probe = tcc_ckc_iobus_probe,
	.flags = DM_FLAG_PRE_RELOC,
	.priv_auto_alloc_size = (int)sizeof(struct tcc_ckc_iobus_priv),
};

// clock hsio bus
struct tcc_ckc_hsiobus_priv {
	unsigned long num_hsiobus_clk;
	struct tcc_ckc_desc *hsiobus_clks;
};

static int tcc_ckc_hsiobus_probe(struct udevice *dev)
{
	(void)dev;
	/*
	 * Register operations only
	 */
	return 0;
}

static int tcc_ckc_hsiobus_enable(struct clk *pclk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_CLK_ENABLE_HSIOBUS, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0UL) {
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

static int tcc_ckc_hsiobus_disable(struct clk *pclk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_CLK_DISABLE_HSIOBUS, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0UL) {
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

static int tcc_ckc_hsiobus_request(struct clk *pclk)
{
	struct udevice *target_dev;
	int i = 0, ret = 0;

	while ((ret == 0) && (i < __INT_MAX__)) {
		ret = uclass_find_device(UCLASS_CLK, i, &target_dev);
		if (ret == 0) {
			if (strncmp("telechips,clk-hsiobus", target_dev->name, 21) == 0) {
				pclk->dev = target_dev;
				break;
			}
			i++;
		}
	}

	return ret;
}

static const struct clk_ops tcc_ckc_hsiobus_ops = {
	.enable = tcc_ckc_hsiobus_enable,
	.disable = tcc_ckc_hsiobus_disable,
	.request = tcc_ckc_hsiobus_request,
};

static const struct udevice_id tcc_ckc_hsiobus_id[] = {
	{.compatible = "telechips,clk-hsiobus"},
	{}
};

U_BOOT_DRIVER(tcc_hsiobus_ckc) = {
	.name = "tcc_ckc_hsiobus",
	.id = UCLASS_CLK,
	.ops = &tcc_ckc_hsiobus_ops,
	.of_match = tcc_ckc_hsiobus_id,
	.probe = tcc_ckc_hsiobus_probe,
	.flags = DM_FLAG_PRE_RELOC,
	.priv_auto_alloc_size = (int)sizeof(struct tcc_ckc_hsiobus_priv),
};

// clock display bus
struct tcc_ckc_ddibus_priv {
	unsigned long num_ddibus_clk;
	struct tcc_ckc_desc *ddibus_clks;
};

static int tcc_ckc_ddibus_probe(struct udevice *dev)
{
	(void)dev;
	/*
	 * Register operations only
	 */
	return 0;
}

static int tcc_ckc_ddibus_enable(struct clk *pclk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_CLK_ENABLE_ISODDI, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0UL) {
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

static int tcc_ckc_ddibus_disable(struct clk *pclk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_CLK_DISABLE_ISODDI, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0UL) {
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

static int tcc_ckc_ddibus_request(struct clk *pclk)
{
	struct udevice *target_dev;
	int i = 0, ret = 0;

	while ((ret == 0) && (i < __INT_MAX__)) {
		ret = uclass_find_device(UCLASS_CLK, i, &target_dev);
		if (ret == 0) {
			if (strncmp("telechips,clk-ddibus", target_dev->name, 20) == 0) {
				pclk->dev = target_dev;
				break;
			}
			i++;
		}
	}

	return ret;
}

static const struct clk_ops tcc_ckc_ddibus_ops = {
	.enable = tcc_ckc_ddibus_enable,
	.disable = tcc_ckc_ddibus_disable,
	.request = tcc_ckc_ddibus_request,
};

static const struct udevice_id tcc_ckc_ddibus_id[] = {
	{.compatible = "telechips,clk-ddibus"},
	{}
};

U_BOOT_DRIVER(tcc_ddibus_ckc) = {
	.name = "tcc_ckc_ddibus",
	.id = UCLASS_CLK,
	.ops = &tcc_ckc_ddibus_ops,
	.of_match = tcc_ckc_ddibus_id,
	.probe = tcc_ckc_ddibus_probe,
	.flags = DM_FLAG_PRE_RELOC,
	.priv_auto_alloc_size = (int)sizeof(struct tcc_ckc_ddibus_priv),
};

// clock vpubus
struct tcc_ckc_vpubus_priv {
	unsigned long num_vpubus_clk;
	struct tcc_ckc_desc *vpubus_clks;
};

static int tcc_ckc_vpubus_probe(struct udevice *dev)
{
	(void)dev;
	/*
	 * Register operations only.
	 */
	return 0;
}

static int tcc_ckc_vpubus_enable(struct clk *pclk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_CLK_ENABLE_VPUBUS, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0UL) {
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

static int tcc_ckc_vpubus_disable(struct clk *pclk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_CLK_DISABLE_VPUBUS, pclk->id,
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0UL) {
		ret = 0;
	} else {
		ret = -1;
	}

	return ret;
}

static int tcc_ckc_vpubus_request(struct clk *pclk)
{
	struct udevice *target_dev;
	int i = 0, ret = 0;

	while ((ret == 0) && (i < __INT_MAX__)) {
		ret = uclass_find_device(UCLASS_CLK, i, &target_dev);
		if (ret == 0) {
			if (strncmp("telechips,clk-vpubus", target_dev->name, 20) == 0) {
				pclk->dev = target_dev;
				break;
			}
			i++;
		}
	}

	return ret;

}

static const struct clk_ops tcc_ckc_vpubus_ops = {
	.enable = tcc_ckc_vpubus_enable,
	.disable = tcc_ckc_vpubus_disable,
	.request = tcc_ckc_vpubus_request,
};

static const struct udevice_id tcc_ckc_vpubus_id[] = {
	{.compatible = "telechips,clk-vpubus"},
	{}
};

U_BOOT_DRIVER(tcc_vpubus_ckc) = {
	.name = "tcc_ckc_vpubus",
	.id = UCLASS_CLK,
	.ops = &tcc_ckc_vpubus_ops,
	.of_match = tcc_ckc_vpubus_id,
	.probe = tcc_ckc_vpubus_probe,
	.flags = DM_FLAG_PRE_RELOC,
	.priv_auto_alloc_size = (int)sizeof(struct tcc_ckc_vpubus_priv),
};

// clock isoip_top
#if 0
struct tcc_ckc_isoip_top_priv {
	unsigned long num_isoip_top_clk;
	struct tcc_ckc_desc *isoip_top_clks;
};

static int tcc_ckc_isoip_top_probe(struct udevice *dev)
{
	/*
	 * Register operations only
	 */
	return 0;
}

static int tcc_ckc_isoip_top_enable(struct clk *clk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc((unsigned long)SIP_CLK_ENABLE_ISOTOP, clk->id,
		      0, 0, 0, 0, 0, 0, &res);
	ret = (int)res.a0;

	return ret;
}

static int tcc_ckc_isoip_top_disable(struct clk *clk)
{
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc((unsigned long)SIP_CLK_DISABLE_ISOTOP, clk->id,
		      0, 0, 0, 0, 0, 0, &res);
	ret = (int)res.a0;

	return ret;
}

static int tcc_ckc_isoip_top_request(struct clk *clk)
{
	struct udevice *target_dev;
	int i = 0, ret = 0;

	ret = uclass_find_device(UCLASS_CLK, i, &target_dev);
	if (ret < 0)
		return ret;

	for (i = 1; i >= 0; i++) {
		if (strncmp("telechips,clk-isoip_top",
			    target_dev->name,
			    23) == 0)
			break;

		ret = uclass_find_device(UCLASS_CLK, i, &target_dev);
		if (ret < 0)
			return ret;
	}
	clk->dev = target_dev;
	return ret;
}

static const struct clk_ops tcc_ckc_isoip_top_ops = {
	.enable = tcc_ckc_isoip_top_enable,
	.disable = tcc_ckc_isoip_top_disable,
	.request = tcc_ckc_isoip_top_request,
};

static const struct udevice_id tcc_ckc_isoip_top_id[] = {
	{.compatible = "telechips,clk-isoip_top"},
	{}
};
#endif

int tcc_clk_get_by_index(struct udevice *dev, int index, struct clk *pclk)
{
	struct ofnode_phandle_args args;
	struct udevice *dev_clk;
	int ret;

	ret = dev_read_phandle_with_args(dev, "clocks",
					 "#clock-cells", 0,
					 index, &args);

	if (ret == 0) {
		(void)uclass_find_device_by_ofnode(UCLASS_CLK, args.node, &dev_clk);

		pclk->id = args.args[0];
		pclk->dev = dev_clk;
	}

	return 0;
}
