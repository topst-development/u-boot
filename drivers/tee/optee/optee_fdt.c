// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2020 Telechips Inc.
 */

#include <common.h>
#include <linux/arm-smccc.h>
#include <linux/libfdt.h>
#include <cpu_func.h>

#define TEE_OS_FDT_ADDR		(0xB200FFF2)

int fdt_parse_optee(void *fdt)
{
	int offset;
	struct arm_smccc_res res;

	/* Send fdt address to OP-TEE OS */
	flush_dcache_all();
	arm_smccc_smc(TEE_OS_FDT_ADDR, env_get_hex("fdtaddr", 0),
		      0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == 0) {
		pr_force("OPTEE is enabled\n");

		/* enable reserved memory */
		offset = fdt_node_offset_by_compatible(fdt, -1, "optee.os");
		if (offset > 0) {
			(void)fdt_setprop_string(fdt, offset, "status", "okay");
		} else {
			panic("optee.os is not defined in device tree\n");
		}
	} else {
		pr_err("OPTEE may not be stored in storage.\n");
	}

	return 0;
}
