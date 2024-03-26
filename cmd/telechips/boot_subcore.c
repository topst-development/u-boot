// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#define pr_fmt(fmt) "boot_subcore: " fmt

#include <common.h>
#include <command.h>
#include <compiler.h>
#include <cpu_func.h>
#include <memalign.h>
#include <asm/io.h>
#include <asm/arch/boot-mode.h>
#include <asm/arch/ca7s.h>
#include <mach/smc.h>
#include <mach/chipinfo.h>

static int do_boot_subcore(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	ulong switches;
	int ret;

	if (argc < 2) {
		switches = ULONG_MAX;
	} else {
		switches = simple_strtoul(argv[1], NULL, 16U);
	}

	switch (switches) {
	case 0:
	case 1:
		ret = run_ca7s_firmware(switches);
		break;
	default:
		ret = (int)CMD_RET_USAGE;
		break;
	}

	return ret;
}

U_BOOT_CMD(
	boot_subcore, 2, 0, do_boot_subcore,
	"start or stop sub-core system",
	"1: start / 0: stop"
)

static enum command_ret_t set_subcore_boot_info(const ulong *size)
{
	int i;
	enum command_ret_t ret = CMD_RET_SUCCESS;

	const struct arm_smccc_res args[6] = {
		{	/* Init boot header */
			SIP_CA7S_INIT_BL33_INFO,
			0, 0, 0
		}, {	/* Set start addr (pc) */
			SIP_CA7S_SET_BL33_ADDR,
			SUBCORE_TARGET_KERNEL, 0, 0
		}, {	/* Set FDT addr (x2) */
			SIP_CA7S_SET_BL33_ARGS,
			0, 0, SUBCORE_TARGET_FDT
		}, {	/* Pass kernel src addr, dest addr, size */
			SIP_CA7S_SET_BL33_ADD_IMG_INFO,
			SUBCORE_ADDR_KERNEL, SUBCORE_TARGET_KERNEL, size[0]
		}, {	/* Pass FDT src addr, dest addr, size */
			SIP_CA7S_SET_BL33_ADD_IMG_INFO,
			SUBCORE_ADDR_FDT, SUBCORE_TARGET_FDT, size[1]
		}, {	/* Pass ramdisk src addr, dest addr, size */
			SIP_CA7S_SET_BL33_ADD_IMG_INFO,
			SUBCORE_ADDR_SYSTEM, SUBCORE_TARGET_SYSTEM, size[2]
		},
	};

	for (i = 0; i < 6; i++) {
		const struct arm_smccc_res *arg = &args[i];
		struct arm_smccc_res res;

		arm_smccc_smc(arg->a0, arg->a1, arg->a2, arg->a3,
			      0, 0, 0, 0, &res);

		if (res.a0 != 0U) {
			pr_err("Failed to set subcore boot info (seq:%d)\n", i);
			ret = CMD_RET_FAILURE;
			break;
		}
	}

	return ret;
}

static int do_prepare_subcore(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	int i;
	enum command_ret_t ret = CMD_RET_USAGE;

	const char *const name[3] = { "boot", "dtb", "root" };
	ulong size[3] = { 0, 0, 0 };
	const ulong limit[3] = {
		SUBCORE_SIZE_KERNEL,
		SUBCORE_SIZE_FDT,
		SUBCORE_SIZE_SYSTEM
	};

	if (argc >= 4) {
		for (i = 0; i < 3; i++) {
			size[i] = simple_strtoul(argv[i + 1], NULL, 16);

			if (size[i] > limit[i]) {
				ret = CMD_RET_FAILURE;
				pr_err("Subcore %s (%lu) is too big! (> %lu)\n",
				       name[i], size[i], limit[i]);
			}
		}

		if (ret != CMD_RET_FAILURE) {
			ret = set_subcore_boot_info(size);
		}
	}

	return (s32)ret;
}

U_BOOT_CMD(
	prepare_subcore, 4, 0, do_prepare_subcore,
	"prepare sub-core system boot",
	"<boot size> <fdt size> <root size>\n"
	" - Pass partition size for each sub-core image"
)
