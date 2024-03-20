// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_BOOT_CONFIG_H
#define TCC_BOOT_CONFIG_H

/*
 * Kernel Boot Configuration
 */

#if defined(CONFIG_CMD_AVB)
#  define AVB_VERIFY_CHECK \
	"fdt optee; "  \
	"if run avb_verify; then " \
		"echo AVB verification OK.;" \
		"setenv android_rootdev;" \
		"setenv bootargs \"$bootargs $avb_bootargs\";" \
	"else " \
		"echo AVB verification failed.;" \
		"exit;" \
	"fi;"
#  define AVB_VERIFY_CMD_SETTINGS \
	"avb_verify=avb init ${bootdev} ${bootdevnum}; avb verify ${slot};\0"
#endif

#if defined(CONFIG_ANDROID_DT_OVERLAY)
#  define APPLY_DTO_COMMAND \
	"if test ${board_id} != 0x0; then " \
		"dtimg load - ${bootdevnum}#dtbo ${slot} ${dtboaddr}; " \
		"dtimg index ${dtboaddr} ${board_id} ${board_rev} dtboidx; " \
		"dtimg start ${dtboaddr} ${dtboidx} fdtovaddr; " \
		"dtimg size ${dtboaddr} ${dtboidx} fdtovsize; " \
		"fdt resize ${fdtovsize}; " \
		"fdt apply ${fdtovaddr}; " \
	"fi; "
#endif

#endif /* TCC_BOOT_CONFIG_H */
