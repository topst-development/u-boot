// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#define pr_fmt(fmt) "tcc: pd: " fmt

#include <common.h>
#include <dm.h>
#include <dt-bindings/power-domain/telechips-power-domain.h>
#include <mach/smc.h>
#include <power-domain.h>
#include <power-domain-uclass.h>

static int tcc_power_domain_request(struct power_domain *power_domain)
{
	ulong id = power_domain->id;
	s32 ret = 0;

	if (id >= PD_ID_MAX) {
		pr_err("Invalid power domain id %lu\n", id);
		ret = -EINVAL;
	}

	return ret;
}

static int tcc_power_domain_free(struct power_domain *power_domain)
{
	return 0;
}

static inline int tcc_power_domain_set_state(ulong id, ulong state)
{
	const char *onoff = (state == PD_PWR_UP) ? "on" : "off";
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_SET_PD_PWR_STS, id, state, 0, 0, 0, 0, 0, &res);
	if (res.a0 == SMC_OK) {
		if (res.a1 == state) {
			pr_info("Power domain %lu is powered %s.\n", id, onoff);
			ret = 0;
		} else {
			pr_err("Failed to power %s domain %lu.\n", onoff, id);
			ret = -EPERM;
		}
	} else {
		pr_err("Invalid power control for domain %lu.\n", id);
		ret = -EINVAL;
	}

	return ret;
}

static int tcc_power_domain_on(struct power_domain *power_domain)
{
	return tcc_power_domain_set_state(power_domain->id, PD_PWR_UP);
}

static int tcc_power_domain_off(struct power_domain *power_domain)
{
	return tcc_power_domain_set_state(power_domain->id, PD_PWR_DOWN);
}

static int tcc_power_domain_bind(struct udevice *dev)
{
	return 0;
}

static int tcc_power_domain_probe(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id tcc_power_domain_ids[] = {
	{ .compatible = "telechips,power-domain" },
	{ /* sentinel */ }
};

static struct power_domain_ops tcc_power_domain_ops = {
	.request = tcc_power_domain_request,
	.free = tcc_power_domain_free,
	.on = tcc_power_domain_on,
	.off = tcc_power_domain_off,
};

U_BOOT_DRIVER(tcc_power_domain) = {
	.name = "tcc_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = tcc_power_domain_ids,
	.bind = tcc_power_domain_bind,
	.probe = tcc_power_domain_probe,
	.ops = &tcc_power_domain_ops,
};

/*
 * Power off listed domain on "telechips,power-domain-off" DT node,
 * when tcc_power_domain_off driver is probed.
 */

static int tcc_power_domain_off_probe(struct udevice *dev)
{
	return dev_power_domain_off(dev);
}

static const struct udevice_id tcc_power_domain_off_ids[] = {
	{ .compatible = "telechips,power-domain-off" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(tcc_power_domain_off) = {
	.name = "tcc_power_domain_off",
	.id = UCLASS_MISC,
	.of_match = tcc_power_domain_off_ids,
	.probe = tcc_power_domain_off_probe,
	.flags = DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
