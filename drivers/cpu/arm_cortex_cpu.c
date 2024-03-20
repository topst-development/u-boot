// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#define pr_fmt(fmt) "arm-cortex: " fmt

#include <common.h>
#include <dm.h>
#include <cpu.h>

#if defined(CONFIG_ARM64)
#  include <asm/armv8/cpu.h>
#else
#  include <asm/armv7/cpu.h>
#endif

static int arm_cortex_cpu_probe(struct udevice *dev)
{
	return 0;
}

#define ARM_CPU_DESC_LEN (40)

static int arm_cortex_cpu_get_desc(struct udevice *dev, char *buf, int size)
{
	u32 midr = read_midr();

	u32 partnum = (midr & 0x0000FFF0U) >> 4;
	u32 variant = (midr & 0x00F00000U) >> 20;
	u32 revision = midr & 0x0000000FU;

	const char *processor;
	s32 ret = -ENOSPC;

	switch (partnum) {
#if !defined(CONFIG_ARM64)
	case MIDR_PARTNUM_CORTEX_A7:
		processor = "Cortex-A7";
		break;
	case MIDR_PARTNUM_CORTEX_A15:
		processor = "Cortex-A15";
		break;
#endif
	case MIDR_PARTNUM_CORTEX_A35:
		processor = "Cortex-A35";
		break;
	case MIDR_PARTNUM_CORTEX_A53:
		processor = "Cortex-A53";
		break;
	case MIDR_PARTNUM_CORTEX_A72:
		processor = "Cortex-A72";
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
#if !defined(CONFIG_ARM64)
	{ .compatible = "arm,cortex-a7"  },
	{ .compatible = "arm,cortex-a15" },
#endif
	{ .compatible = "arm,cortex-a35" },
	{ .compatible = "arm,cortex-a53" },
	{ .compatible = "arm,cortex-a72" },
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
