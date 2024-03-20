/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_PARTITION_CONFIG_H
#define TCC_PARTITION_CONFIG_H

#if defined(CONFIG_CMD_BOOT_ANDROID)

#if CONFIG_IS_ENABLED(LIBAVB)
#  define TAMPER_EVIDENCE_PART \
	"name=tamper_evidence,size=8192KiB,uuid=${uuid_gpt_tamper_evidence};"

#  if CONFIG_IS_ENABLED(AB_UPDATE)
#    define VBMETA_PART \
	"name=vbmeta_a,size=1024KiB,uuid=${uuid_gpt_vbmeta_a};" \
	"name=vbmeta_b,size=1024KiB,uuid=${uuid_gpt_vbmeta_b};"
#  else
#    define VBMETA_PART \
	"name=vbmeta,size=1024KiB,uuid=${uuid_gpt_vbmeta};"
#  endif
#endif

#if CONFIG_IS_ENABLED(AB_UPDATE)
#  define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	BL3_PART \
	"name=secure,size=102400KiB,uuid=${uuid_gpt_secure};" \
	VBMETA_PART \
	TAMPER_EVIDENCE_PART \
	"name=env,size=1MiB,uuid=${uuid_gpt_reserved};" \
	"name=dtb_a,size=2048KiB,uuid=${uuid_gpt_fdt_a};" \
	"name=dtb_b,size=2048KiB,uuid=${uuid_gpt_fdt_b};" \
	"name=dtb_rec,size=2048KiB,uuid=${uuid_gpt_fdt};" \
	"name=dtbo_a,size=8192KiB,uuid=${uuid_gpt_fdt_a};" \
	"name=dtbo_b,size=8192KiB,uuid=${uuid_gpt_fdt_b};" \
	"name=boot_a,size=30720KiB,bootable,uuid=${uuid_gpt_boot_a};" \
	"name=boot_b,size=30720KiB,bootable,uuid=${uuid_gpt_boot_b};" \
	"name=recovery,size=32768KiB,uuid=${uuid_gpt_recovery};" \
	"name=splash,size=5120KiB,uuid=${uuid_gpt_splash};" \
	"name=misc,size=1024KiB,uuid=${uuid_gpt_misc};" \
	"name=tcc,size=1024KiB,uuid=${uuid_gpt_reserved};" \
	"name=cache,size=153600KiB,uuid=${uuid_gpt_cache};" \
	"name=system_a,size=2097152KiB,uuid=${uuid_gpt_system_a};" \
	"name=system_b,size=2097152KiB,uuid=${uuid_gpt_system_b};" \
	"name=vendor_a,size=153600KiB,uuid=${uuid_gpt_vendor_a};" \
	"name=vendor_b,size=153600KiB,uuid=${uuid_gpt_vendor_b};" \
	"name=sest,size=8192KiB,uuid=${uuid_gpt_sest};" \
	"name=userdata,size=-,uuid=${uuid_gpt_userdata}"
#else /* CONFIG_AB_UPDATE */
#  define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	BL3_PART \
	"name=secure,size=102400KiB,uuid=${uuid_gpt_secure};" \
	VBMETA_PART \
	TAMPER_EVIDENCE_PART \
	"name=uboot-env,size=1MiB,uuid=${uuid_gpt_reserved};" \
	"name=misc,size=1024KiB,uuid=${uuid_gpt_misc};" \
	"name=dtb,size=2048KiB,uuid=${uuid_gpt_fdt};" \
	"name=dtb_rec,size=2048KiB,uuid=${uuid_gpt_fdt};" \
	"name=dtbo,size=8192KiB,uuid=${uuid_gpt_fdt};" \
	"name=boot,size=30720KiB,bootable,uuid=${uuid_gpt_boot};" \
	"name=recovery,size=32768KiB,uuid=${uuid_gpt_recovery};" \
	"name=splash,size=5120KiB,uuid=${uuid_gpt_splash};" \
	"name=tcc,size=1024KiB,uuid=${uuid_gpt_reserved};" \
	"name=cache,size=153600KiB,uuid=${uuid_gpt_cache};" \
	"name=system,size=2097152KiB,uuid=${uuid_gpt_system};" \
	"name=vendor,size=153600KiB,uuid=${uuid_gpt_vendor};" \
	"name=sest,size=8192KiB,uuid=${uuid_gpt_sest};" \
	"name=userdata,size=-,uuid=${uuid_gpt_userdata}"
#endif /* !CONFIG_AB_UPDATE */

#else /* CONFIG_CMD_BOOT_ANDROID */

#if CONFIG_IS_ENABLED(AB_UPDATE)
#  define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	BL3_PART \
	"name=secure,size=102400KiB,uuid=${uuid_gpt_secure};" \
	"name=env,size=1MiB,uuid=${uuid_gpt_reserved};" \
	"name=boot_a,size=30720KiB,bootable,uuid=${uuid_gpt_boot_a};" \
	"name=boot_b,size=30720KiB,bootable,uuid=${uuid_gpt_boot_b};" \
	"name=dtb_a,size=2048KiB,uuid=${uuid_gpt_fdt_a};" \
	"name=dtb_b,size=2048KiB,uuid=${uuid_gpt_fdt_b};" \
	"name=splash,size=5120KiB,uuid=${uuid_gpt_splash};" \
	"name=misc,size=1024KiB,uuid=${uuid_gpt_misc};" \
	"name=rootfs_a,size=524288KiB,uuid=${uuid_gpt_rootfs_a};" \
	"name=rootfs_b,size=524288KiB,uuid=${uuid_gpt_rootfs_b};" \
	"name=log,size=65536KiB,uuid=${uuid_gpt_log};" \
	"name=data,size=-,uuid=${uuid_gpt_data}"
#else /* CONFIG_AB_UPDATE */
#  define PARTS_DEFAULT \
	"uuid_disk=${uuid_gpt_disk};" \
	BL3_PART \
	"name=secure,size=102400KiB,uuid=${uuid_gpt_secure};" \
	"name=env,size=1MiB,uuid=${uuid_gpt_reserved};" \
	"name=boot,size=30720KiB,bootable,uuid=${uuid_gpt_boot};" \
	"name=dtb,size=2048KiB,uuid=${uuid_gpt_fdt};" \
	"name=splash,size=5120KiB,uuid=${uuid_gpt_splash};" \
	"name=rootfs,size=524288KiB,uuid=${uuid_gpt_rootfs};" \
	"name=log,size=65536KiB,uuid=uuid=${uuid_gpt_log};" \
	"name=data,size=-,uuid=${uuid_gpt_data}"
#endif /* !CONFIG_AB_UPDATE */

#endif /* CONFIG_CMD_BOOT_ANDROID */
#endif /* TCC_PARTITION_CONFIG_H */
