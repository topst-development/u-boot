/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC805X_CONFIG_H
#define TCC805X_CONFIG_H

#include "tcc/common.h"
#include "tcc/boot.h"
#include "tcc/partition.h"
#include "tcc/tcc805x-reg.h"

#define CONFIG_MACH_TYPE		MACH_TYPE_TCC805X

#if defined(CONFIG_SD_BOOT)
#  define BOOTDEV_NUM			0
#elif defined(CONFIG_UFS_BOOT)
#  define BOOTDEV_NUM			2
#else
#  error "Not supported boot media or no boot media is selected"
#endif

/*
 * MMC max number of block count
 */

#define CONFIG_SYS_MMC_MAX_BLK_COUNT	1024

/*
 * Default Configurations for TCC805x
 */

#define DRAM_BASE			(0x20000000U)
#define DRAM_SIZE			(0xA0000000U) /* 2.5 GiB */
#define DRAM_BASE_EXT			(0x1A0000000U)

#define CONFIG_SYS_LOAD_ADDR		(0x24000000)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE + 0x0F000000)

#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_TEXT_BASE)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_TEXT_BASE + 0x10000000)

#define COUNTER_FREQUENCY		(0x1800000) /* 24MHz */

#define INT_GIC_OFFSET			(32U)
#define NR_IRQS				(480U + INT_GIC_OFFSET)

/*
 * Default Load Partition & Address for TCC805x
 */

#define MAINCORE_KERNEL_ADDR		0x24000000
#define MAINCORE_FDT_ADDR		0x23000000
#define MAINCORE_DTBO_ADDR		0x23100000
#define MAINCORE_INITRD_ADDR		0x24000000

#define MAINCORE_KERNEL_PART		"boot"
#define MAINCORE_FDT_PART		"dtb"
#define MAINCORE_DTBO_PART		"dtbo"
#define MAINCORE_INITRD_PART		"root"
#define MAINCORE_SYSTEM_PART		"system"
#define MAINCORE_UPDATE_PART		"update"
#define MAINCORE_MISC_PART		"misc"

#define SUBCORE_KERNEL_ADDR		0x44000000
#define SUBCORE_FDT_ADDR		0x43000000
#define SUBCORE_DTBO_ADDR		0x43100000
#define SUBCORE_INITRD_ADDR		0x5A000000

#define SUBCORE_KERNEL_PART		"subcore_boot"
#define SUBCORE_FDT_PART		"subcore_dtb"
#define SUBCORE_DTBO_PART		"subcore_dtbo"
#define SUBCORE_INITRD_PART		"subcore_root"
#define SUBCORE_SYSTEM_PART		"subcore_root"
#define SUBCORE_UPDATE_PART		"subcore_update"
#define SUBCORE_MISC_PART		"subcore_misc"

#if defined(CONFIG_TCC_MAINCORE)
#  define KERNEL_ADDR			MAINCORE_KERNEL_ADDR
#  define FDT_ADDR			MAINCORE_FDT_ADDR
#  define DTBO_ADDR			MAINCORE_DTBO_ADDR
#  define INITRD_ADDR			MAINCORE_INITRD_ADDR

#  define KERNEL_PART			MAINCORE_KERNEL_PART
#  define FDT_PART			MAINCORE_FDT_PART
#  define DTBO_PART			MAINCORE_DTBO_PART
#  define INITRD_PART			MAINCORE_INITRD_PART
#  define SYSTEM_PART			MAINCORE_SYSTEM_PART
#  define UPDATE_PART			MAINCORE_UPDATE_PART
#  define MISC_PART			MAINCORE_MISC_PART
#else
#  define KERNEL_ADDR			SUBCORE_KERNEL_ADDR
#  define FDT_ADDR			SUBCORE_FDT_ADDR
#  define DTBO_ADDR			SUBCORE_DTBO_ADDR
#  define INITRD_ADDR			SUBCORE_INITRD_ADDR

#  define KERNEL_PART			SUBCORE_KERNEL_PART
#  define FDT_PART			SUBCORE_FDT_PART
#  define DTBO_PART			SUBCORE_DTBO_PART
#  define INITRD_PART			SUBCORE_INITRD_PART
#  define SYSTEM_PART			SUBCORE_SYSTEM_PART
#  define UPDATE_PART			SUBCORE_UPDATE_PART
#  define MISC_PART			SUBCORE_MISC_PART
#endif

/*
 * Default Boot Command for TCC805x
 */

#if defined(CONFIG_EARLYCAMERA_SOLUTION)
#  define INIT_EARLYCAMERA_SOLUTION(fdt_addr)		\
	"prepare_earlycamera dtb ${" #fdt_addr "}; "	\
	"load_pgl; "
#  define START_EARLYCAMERA_SOLUTION		\
	"start_earlycamera; "
#endif

#if defined(CONFIG_TCC_MAINCORE) && defined(CONFIG_BOOT_SUBCORE_BY_MAINCORE)
#  define BOOT_SUBCORE_COMMAND \
	"fdt load - ${bootdevnum}#" SUBCORE_FDT_PART \
		" ${slot} ${subcore_fdt_addr}; " \
	"blkread image " SUBCORE_KERNEL_PART \
		" ${slot} ${subcore_kerneladdr}; " \
	"blkread part " SUBCORE_INITRD_PART \
		" ${slot} ${subcore_root_addr}; " \
	"sync notify 0xD0B007; "
#endif

#if defined(CONFIG_TCC_SUBCORE) && defined(CONFIG_BOOT_SUBCORE_BY_MAINCORE)
#  define LOAD_IMAGE_COMMAND \
	"sync wait 0xD0B007; "
#elif defined(CONFIG_CMD_BOOT_ANDROID)
#  define LOAD_IMAGE_COMMAND \
	"run loadfdt; run setupfdt; "
#else
#  define LOAD_IMAGE_COMMAND \
	"blkread info ${rootpart} ${slot} root_devnum root_partnum; " \
	"blkread image ${bootpart} ${slot} ${kerneladdr}; " \
	"run loadfdt; run setupfdt; "
#endif

#if defined(CONFIG_CMD_BOOT_ANDROID)
#elif defined(CONFIG_TCC_SUBCORE)
#if defined(CONFIG_UFS_BOOT)
#  define LINUX_ROOT_BOOTARGS \
	"setenv bootargs ${bootargs} root=/dev/sdc${root_partnum} ro " \
	"rootfstype=ext4 rootwait; "
#else
#  define LINUX_ROOT_BOOTARGS \
	"setenv bootargs ${bootargs} " \
	"root=/dev/mmcblk${root_devnum}p${root_partnum} ro rootfstype=ext4 " \
	"rootwait; "
#endif
#else
#if defined(CONFIG_UFS_BOOT)
#  define LINUX_ROOT_BOOTARGS \
	"setenv bootargs ${bootargs} partition.system=/dev/sdc${root_partnum}; "
#else
#  define LINUX_ROOT_BOOTARGS \
	"setenv bootargs ${bootargs} partition.system=" \
	"/dev/mmcblk${root_devnum}p${root_partnum}; "
#endif
#endif

#if defined(CONFIG_CMD_BOOT_ANDROID)
#  define BOOT_KERNEL_COMMAND \
	AVB_VERIFY_CHECK \
	"boot_android - ${bootdevnum}#${miscpart} ${slot} ${kerneladdr}"
#else
#  define BOOT_KERNEL_COMMAND \
	LINUX_ROOT_BOOTARGS \
	"bootm ${kerneladdr} - ${fdt_addr}"
#endif

#if defined(CONFIG_QNX)
#  define CONFIG_BOOTCOMMAND \
	"blkread part ${bootpart} - 0x30000000; " \
	"go 0x30000000"
#else
#  define CONFIG_BOOTCOMMAND \
	BOOT_SUBCORE_COMMAND \
	LOAD_IMAGE_COMMAND \
	INIT_EARLYCAMERA_SOLUTION(fdt_addr) \
	START_EARLYCAMERA_SOLUTION \
	BOOT_KERNEL_COMMAND
#endif

/*
 * Default U-Boot Command (rather than bootcmd) for TCC805x
 */

#if defined(CONFIG_CMD_BOOT_ANDROID)
#  define USBBOOT_COMMAND \
	"usb start; " \
	"load usb 0 ${kerneladdr} /boot/boot.img; " \
	"load usb 0 ${fdt_addr} /boot/dt.dtb; " \
	"bootm ${kerneladdr} ${initrdaddr} ${fdt_addr}"
#else
#  define USBBOOT_COMMAND \
	"help usb"
#endif

#if defined(CONFIG_CMD_BOOT_ANDROID)
#elif defined(CONFIG_TCC_SUBCORE)
#if defined(CONFIG_UFS_BOOT)
#  define LINUX_UPDATE_BOOTARGS \
	"setenv bootargs ${bootargs} root=/dev/sdc${update_partnum} ro " \
	"rootfstype=ext4 rootwait; "
#else
#  define LINUX_UPDATE_BOOTARGS \
	"setenv bootargs ${bootargs} " \
	"root=/dev/mmcblk${update_devnum}p${update_partnum} ro rootfstype=ext4 " \
	"rootwait; "
#endif
#else
#if defined(CONFIG_UFS_BOOT)
#  define LINUX_UPDATE_BOOTARGS \
	"setenv bootargs ${bootargs} partition.update=/dev/sdc${update_partnum}; "
#else
#  define LINUX_UPDATE_BOOTARGS \
	"setenv bootargs ${bootargs} partition.update=" \
	"/dev/mmcblk${update_devnum}p${update_partnum}; "
#endif
#endif

#if defined(CONFIG_AB_UPDATE)
#  define RECOVERY_COMMAND \
	"setenv bootargs " RECOVERY_BOOTARGS "; " \
	"blkread part ${bootpart} ${slot} ${kerneladdr}; " \
	"run loadfdt; run setupfdt; " \
	"bootm ${kerneladdr} ${initrdaddr} ${fdt_addr}"
#else
#  define RECOVERY_COMMAND \
	"setenv bootargs " RECOVERY_BOOTARGS "; " \
	"blkread info ${updatepart} ${slot} update_devnum update_partnum; " \
	"blkread part recovery - ${kerneladdr}; " \
	"fdt load - ${bootdevnum}#dtb_rec - ${fdt_addr}; " \
	"run setupfdt; " \
	LINUX_UPDATE_BOOTARGS \
	"bootm ${kerneladdr} ${initrdaddr} ${fdt_addr}"
#endif

#if defined(CONFIG_CMD_AVB)
#  define FASTBOOT_COMMAND \
	"run loadfdt; run setupfdt; fdt optee; " \
	"avb init ${bootdev} ${bootdevnum}; " \
	"fastboot " __stringify(CONFIG_FASTBOOT_USB_DEV)
#else
#  define FASTBOOT_COMMAND \
	"fastboot " __stringify(CONFIG_FASTBOOT_USB_DEV)
#endif

#if CONFIG_IS_ENABLED(CMD_BCB)
#  define FASTBOOTD_COMMAND \
	"if bcb load - ${bootdevnum}#${miscpart}; then " \
		"bcb set command boot-fastboot; bcb store; " \
		"setenv bootargs " RECOVERY_BOOTARGS "; " \
		"run bootcmd; " \
	"fi; "
#else
#  define FASTBOOTD_COMMAND \
	"echo ERROR: Enable CMD_BCB config for fastbootd support."
#endif

/*
 * Default U-Boot Environment for TCC805x
 */

#if defined(CONFIG_TCC_MAINCORE) && defined(CONFIG_BOOT_SUBCORE_BY_MAINCORE)
#  define SUBCORE_ENV_SETTINGS \
	"subcore_kerneladdr=" __stringify(SUBCORE_KERNEL_ADDR) "\0" \
	"subcore_fdt_addr=" __stringify(SUBCORE_FDT_ADDR) "\0" \
	"subcore_root_addr=" __stringify(SUBCORE_INITRD_ADDR) "\0"
#endif

#if defined(CONFIG_ANDROID_SYSTEM_AS_ROOT)
#  define MISC_ENV_SETTINGS \
	"android_rootdev=" \
	"/dev/mmcblk${android_root_devnum}p${android_root_partnum}\0"
#endif

#define LOADFDT_COMMAND \
	"fdt load - ${bootdevnum}#${bootpart} ${slot} ${fdt_addr}; " \
	"if test $? -ne 0; then " \
		"fdt load - ${bootdevnum}#${fdtpart} ${slot} ${fdt_addr}; " \
		"if test $? -ne 0; then " \
			"echo Error loading a device tree blob; " \
			"exit; " \
		"fi; " \
	"fi"

#define SETUPFDT_COMMAND \
	"fdt addr ${fdt_addr}; run applydto; fdt resize"

#if CONFIG_IS_ENABLED(CMD_BCB)
#  define BCB_COMMAND \
	"if bcb load - ${bootdevnum}#${miscpart}; then " \
		"if bcb test command = bootonce-bootloader; then " \
			"bcb clear command; bcb store; " \
			"run fastbootcmd; " \
			"exit; " \
		"elif bcb test command = boot-recovery; then " \
			"bcb clear command; bcb store; " \
			"run recoverycmd; " \
			"exit; " \
		"fi; " \
	"fi; "
#endif

#if CONFIG_IS_ENABLED(AB_UPDATE)
#  define AB_COMMAND \
	"ab select slot - ${bootdevnum}#${miscpart}; " \
	"if test $? -ne 0; then " \
		"echo Unable to get slot name; " \
		"setenv slot a; " \
	"fi; "
#endif

#if CONFIG_IS_ENABLED(USE_PREBOOT)
#  define PREBOOT_COMMAND \
	"blkread dev ${bootdev} ${bootdevnum}; " \
	AB_COMMAND \
	BCB_COMMAND \
	"if test ${boot_reason} = fastboot; then " \
		"echo Entering fastboot mode...; " \
		"run fastbootcmd; " \
	"elif test ${boot_reason} = fastbootd; then " \
		"echo Entering userspace fastboot mode...; " \
		"run fastbootdcmd; " \
	"elif test ${boot_reason} = recovery; then " \
		"echo Entering recovery mode...; " \
		"run recoverycmd; " \
	"fi"
#endif

#if defined(CONFIG_CMD_BOOT_ANDROID)
#  define BOOTMENU_ENV_SETTINGS \
	"menucmd=bootmenu\0" \
	"bootmenu_0=NORMAL BOOT=boot\0" \
	"bootmenu_1=RECOVERY MODE=run recoverycmd; boot\0" \
	"bootmenu_2=USB BOOT=run usbbootcmd\0" \
	"bootmenu_3=FASTBOOT=run fastbootcmd\0" \
	"bootmenu_4=REBOOT=reset\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"preboot=" PREBOOT_COMMAND "\0" \
	"bootargs=" NORMAL_BOOTARGS "\0" \
	"kerneladdr=" __stringify(KERNEL_ADDR) "\0" \
	"fdt_addr=" __stringify(FDT_ADDR) "\0" \
	"fdt_high=" __stringify(FDT_HIGH) "\0" \
	"dtboaddr=" __stringify(DTBO_ADDR) "\0" \
	"initrdaddr=" __stringify(INITRD_ADDR) "\0" \
	"initrd_high=" __stringify(FDT_ADDR) "\0" \
	"kernelarch=" __stringify(KERNEL_ARCH) "\0" \
	"slot=-\0" \
	"bootdev=" BOOT_DEVICE "\0" \
	"bootdevnum=" __stringify(BOOTDEV_NUM) "\0" \
	"loadfdt=" LOADFDT_COMMAND "\0" \
	"setupfdt=" SETUPFDT_COMMAND "\0" \
	"applydto=" APPLY_DTO_COMMAND "\0" \
	"usbbootcmd=" USBBOOT_COMMAND "\0" \
	"recoverycmd=" RECOVERY_COMMAND "\0" \
	"fastbootcmd=" FASTBOOT_COMMAND "\0" \
	"fastbootdcmd=" FASTBOOTD_COMMAND "\0" \
	"bootpart=" KERNEL_PART "\0" \
	"fdtpart=" FDT_PART "\0" \
	"dtbopart=" DTBO_PART "\0" \
	"rootpart=" SYSTEM_PART "\0" \
	"updatepart=" UPDATE_PART "\0" \
	"miscpart=" MISC_PART "\0" \
	AVB_VERIFY_CMD_SETTINGS \
	SUBCORE_ENV_SETTINGS \
	MISC_ENV_SETTINGS \
	BOOTMENU_ENV_SETTINGS \
	"partitions=" PARTS_DEFAULT "\0"

#define BL3_PART \
	"name=bl3_ca72_a,size=2048KiB,uuid=${uuid_gpt_ca72_bootloader_a};" \
	"name=bl3_ca72_b,size=2048KiB,uuid=${uuid_gpt_ca72_bootloader_b};" \
	"name=bl3_ca53_a,size=2048KiB,uuid=${uuid_gpt_ca53_bootloader_a};" \
	"name=bl3_ca53_b,size=2048KiB,uuid=${uuid_gpt_ca53_bootloader_b};"

#endif	/* TCC805X_CONFIG_H */
