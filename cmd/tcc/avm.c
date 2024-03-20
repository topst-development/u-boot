// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include <memalign.h>
#include <cpu_func.h>
#include <compiler.h>
#include <auth_mod.h>

#include <asm/io.h>
#include <asm/telechips/boot-mode.h>
#include <linux/arm-smccc.h>
#include <linux/types.h>

#include <mach/smc.h>
#include <mach/ca7s.h>
#include <mach/chipinfo.h>

static int start_avm(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	void *ap_irqo_en;
	ulong switches;
	s32 ret;

	if (argc < 2)
		return (s32)CMD_RET_USAGE;

	switches = simple_strtoul(argv[1], NULL, 16U);

	switch (switches) {
	case 0:
		/* Disable CMB_MBOX0 interrupts to Cortex-A7S */
		ap_irqo_en = phys_to_virt(0x1460000C);
		writel(0xFFFFFBFFU, ap_irqo_en);

		/* Disable mali0, gc300, g2d interrupts to Cortex-A7S */
		ap_irqo_en = phys_to_virt(0x14600018);
		writel(0xFFFFFFC0U, ap_irqo_en);

		ret = run_ca7s_firmware(0);
		break;
	case 1:
		ret = run_ca7s_firmware(1);
		break;
	default:
		ret = (s32)CMD_RET_USAGE;
		break;
	}

	return ret;
}

U_BOOT_CMD(
	avm_start, 2, 0, start_avm,
	"run or stop AVM system on subcore (Cortex-A7)",
	"1: start / 0: stop"
)

/*
 * Currently, command 'prepare_avm' only is valid (or meaningful) **only when**
 * core reset feature is enabled.
 */
static int prepare_avm(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *name[3] = { "boot", "dtb", "root" };
	u32 limit[3] = { SUBCORE_BAK_KERNEL_MAX_SZ,
		SUBCORE_BAK_DTB_MAX_SZ,
		SUBCORE_BAK_INITRD_MAX_SZ };
	u32 size[3] = { 0, 0, 0 };
	s32 i;
	struct arm_smccc_res res;

	if (argc < 4)
		return (s32)CMD_RET_USAGE;

	for (i = 0; i < 3; i++) {
		size[i] = (u32)simple_strtoul(argv[i + 1], NULL, 16);

		if (size[i] > limit[i]) {
			pr_err("subcore %s partition is too big! ", name[i]);
			pr_err("(size: %u bytes, limit: %u bytes)\n",
			       size[i], limit[i]);
			pr_err("subcore core reset will not work properly.\n");
		}
	}

#ifdef CONFIG_BOOT_SUBCORE_SELF_LOAD_MODE
	/* load backup image only */
	pr_info("Subcore boot mode - self load mode\n");

	/* Init Sub-core Boot Header */
	arm_smccc_smc(SIP_CA7S_INIT_BL33_INFO, 0, 0, 0, 0, 0, 0, 0, &res);
	if(res.a0 != 0 ) {
		pr_err("failed to init ca7 bl33 info\n");
		return (s32)CMD_RET_FAILURE;
	}

	/* Set BL33 PC, ARGS*/
	arm_smccc_smc(SIP_CA7S_SET_BL33_ADDR,
			SUBCORE_KERNEL_ADDR, 0, 0, 0, 0, 0, 0, &res);
	if(res.a0 != 0 ) {
		pr_err("failed to set ca7 bl33 pc\n");
		return (s32)CMD_RET_FAILURE;
	}
	arm_smccc_smc(SIP_CA7S_SET_BL33_ARGS,
			0, 0, SUBCORE_FDT_ADDR, 0, 0, 0, 0, &res);
	if(res.a0 != 0 ) {
		pr_err("failed to set ca7 bl33 args\n");
		return (s32)CMD_RET_FAILURE;
	}

	/* Set Image Info */
	arm_smccc_smc(SIP_CA7S_SET_BL33_ADD_IMG_INFO,
			SUBCORE_BAK_KERNEL_ADDR, SUBCORE_KERNEL_ADDR,
			size[0], 0, 0, 0, 0, &res);
	if(res.a0 != 0 ) {
		pr_err("failed to set ca7 kernel image info\n");
		return (s32)CMD_RET_FAILURE;
	}
	arm_smccc_smc(SIP_CA7S_SET_BL33_ADD_IMG_INFO,
			SUBCORE_BAK_FDT_ADDR, SUBCORE_FDT_ADDR,
			size[1], 0, 0, 0, 0, &res);
	if(res.a0 != 0 ) {
		pr_err("failed to set ca7 kernel dtb info\n");
		return (s32)CMD_RET_FAILURE;
	}
	arm_smccc_smc(SIP_CA7S_SET_BL33_ADD_IMG_INFO,
			SUBCORE_BAK_INITRD_ADDR, SUBCORE_INITRD_ADDR,
			size[2], 0, 0, 0, 0, &res);
	if(res.a0 != 0 ) {
		pr_err("failed to set ca7 ramdisk dtb info\n");
		return (s32)CMD_RET_FAILURE;
	}
#else
	pr_info("Subcore boot mode - legacy mode\n");
	pr_info("WARN: not support secure boot for sub-corea\n");
	arm_smccc_smc(SIP_SET_BOOT_INFO, size[0], size[1], size[2],
		      0, 0, 0, 0, &res);

	if (res.a0 != 0) {
		pr_err("failed to set ca7 boot info.\n");
	}
#endif


	return (s32)CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	avm_prepare, 4, 0, prepare_avm,
	"perform preparations before start AVM system",
	"<boot size> <fdt size> <root size>\n"
	" - Store boot/fdt/root partition size for core reset feature"
)
