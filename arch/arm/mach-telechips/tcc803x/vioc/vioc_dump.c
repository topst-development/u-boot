// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) Telechips Inc.
 */
#include <common.h>
#include <linux/io.h>
#include <asm/telechips/vioc/vioc_dump.h>

void vioc_read_and_dump_regs(void __iomem *reg_base,
			     unsigned int *data, unsigned int len,
			     int dump)
{
	unsigned int i;

	if (data != NULL) {
		for (i = 0U; i < (len >> 2U); i++) {
			if ((dump != 0) && ((i % 4U) == 0U)) {
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				(void)pr_force("\r\n%p: ", reg_base + (i << 2U));
			}
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			data[i] = __raw_readl(reg_base + (i << 2U));
			if (dump != 0) {
				/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
				(void)pr_force(" %02x", (data[i] >> 24U) & 0xFFU);
				(void)pr_force(" %02x", (data[i] >> 16U) & 0xFFU);
				(void)pr_force(" %02x", (data[i] >> 8U) & 0xFFU);
				(void)pr_force(" %02x", data[i] & 0xFFU);
			}
		}
		if (dump != 0) {
			/* Prevent KCS warning */
			(void)pr_force("\r\n");
		}
	}
}
