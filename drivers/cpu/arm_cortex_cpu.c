// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#define pr_fmt(fmt) "arm-cortex: " fmt

#include <common.h>
#include <dm.h>
#include <cpu.h>
#include <asm/armv8/cpu.h>

#define ARM_CPU_DESC_LEN (40)

static int arm_cortex_cpu_probe(struct udevice *dev)
{
	return 0;
}

static int arm_cortex_cpu_get_desc(const struct udevice *dev, char *buf, int size)
{
	u32 midr = read_midr();

	u32 partnum = (midr & 0x0000FFF0U) >> 4;
	u32 variant = (midr & 0x00F00000U) >> 20;
	u32 revision = midr & 0x0000000FU;

	const char *processor;
	int ret = -ENOSPC;

	switch (partnum) {
	case MIDR_PARTNUM_CORTEX_A35:
		processor = "Cortex-A35";
		break;
	case MIDR_PARTNUM_CORTEX_A53:
		processor = "Cortex-A53";
		break;
	case MIDR_PARTNUM_CORTEX_A55:
		processor = "Cortex-A55";
		break;
	case MIDR_PARTNUM_CORTEX_A72:
		processor = "Cortex-A72";
		break;
	case MIDR_PARTNUM_CORTEX_A76:
		processor = "Cortex-A76";
		break;
	default:
		processor = "Unknown";
		break;
	}

	if (size >= ARM_CPU_DESC_LEN) {
		size_t len = (size_t)size;

		(void)scnprintf(buf, len, "ARM %s Processor, Rev: r%up%u",
				processor, variant, revision);
		ret = 0;
	}

	return ret;
}

static const struct cpu_ops arm_cortex_cpu_ops = {
	.get_desc = arm_cortex_cpu_get_desc,
};

static const struct udevice_id arm_cortex_cpu_ids[] = {
	{ .compatible = "arm,cortex-a35" },
	{ .compatible = "arm,cortex-a53" },
	{ .compatible = "arm,cortex-a55" },
	{ .compatible = "arm,cortex-a72" },
	{ .compatible = "arm,cortex-a76" },
	{ .compatible = NULL }
};

U_BOOT_DRIVER(cpu_arm_cortex) = {
	.name = "cpu_arm_cortex",
	.id = UCLASS_CPU,
	.of_match = of_match_ptr(arm_cortex_cpu_ids),
	.probe = arm_cortex_cpu_probe,
	.ops = &arm_cortex_cpu_ops,
	.flags = DM_FLAG_PRE_RELOC
};
