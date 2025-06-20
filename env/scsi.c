// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008-2011 Freescale Semiconductor, Inc.
 */
/*
 * Modified by Telechips Inc. (date: 2021-03)
 */

/* #define DEBUG */

#include <common.h>
#include <asm/global_data.h>

#include <command.h>
#include <env.h>
#include <env_internal.h>
#include <fdtdec.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <scsi.h>
#include <memalign.h>
#include <part.h>
#include <search.h>
#include <errno.h>
#include <dm/ofnode.h>

#define __STR(X) #X
#define STR(X) __STR(X)

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(OF_CONTROL)
static inline int scsi_offset_try_partition(const char *str, int copy, s64 *val)
{
	struct disk_partition info;
	struct blk_desc *desc;
	int len, i, ret;

	ret = blk_get_device_by_str("scsi",
			STR(CONFIG_SYS_SCSI_ENV_DEV), &desc);

	if (ret < 0)
		return (ret);

	for (i = 1;;i++) {
		ret = part_get_info(desc, i, &info);
		if (ret < 0)
			return ret;

		if (!strncmp((const char *)info.name, str, sizeof(info.name)))
			break;
	}

	/* round up to info.blksz */
	len = DIV_ROUND_UP(CONFIG_ENV_SIZE, info.blksz);

	/* use the top of the partion for the environment */
	*val = (info.start + info.size - (1 + copy) * len) * info.blksz;

	return 0;
}

static inline s64 scsi_offset(int copy)
{
	const struct {
		const char *offset_redund;
		const char *partition;
		const char *offset;
	} dt_prop = {
		.offset_redund = "u-boot,scsi-env-offset-redundant",
		.partition = "u-boot,scsi-env-partition",
		.offset = "u-boot,scsi-env-offset",
	};
	s64 val = 0, defvalue;
	const char *propname;
	const char *str;
	int err;

	/* look for the partition in scsi CONFIG_SYS_SCSI_ENV_DEV */
	str = ofnode_conf_read_str(dt_prop.partition);
	if (str) {
		/* try to place the environment at end of the partition */
		err = scsi_offset_try_partition(str, copy, &val);
		if (!err)
			return val;
	}

	defvalue = CONFIG_ENV_OFFSET;
	propname = dt_prop.offset;

#if defined(CONFIG_ENV_OFFSET_REDUND)
	if (copy) {
		defvalue = CONFIG_ENV_OFFSET_REDUND;
		propname = dt_prop.offset_redund;
	}
#endif
	return ofnode_conf_read_int(propname, defvalue);
}
#else
static inline s64 scsi_offset(int copy)
{
	s64 offset = CONFIG_ENV_OFFSET;

#if defined(CONFIG_ENV_OFFSET_REDUND)
	if (copy)
		offset = CONFIG_ENV_OFFSET_REDUND;
#endif
	return offset;
}
#endif

__weak int scsi_get_env_addr(int copy, u32 *env_addr)

{
	s64 offset = scsi_offset(copy);

	*env_addr = offset;

	return 0;
}

__weak int scsi_get_env_dev(void)
{
	return CONFIG_SYS_SCSI_ENV_DEV;
}


#if defined(CONFIG_CMD_SAVEENV) && !defined(CONFIG_SPL_BUILD)
static inline int write_env(unsigned long size,
			    unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;
	int dev = scsi_get_env_dev();
	struct blk_desc *desc;

	desc = blk_get_devnum_by_type(IF_TYPE_SCSI, dev);

	if (!desc) {
		return -1;
	}

	blk_start	= ALIGN(offset, 4096) / 4096;
	blk_cnt		= ALIGN(size, 4096) / 4096;

	n = blk_dwrite(desc, blk_start, blk_cnt, (u_char *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

static int env_scsi_save(void)
{
	//ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	ALLOC_ALIGN_BUFFER(env_t, env_new, 1, 4096);
	u32	offset;
	int	ret, copy = 0;

	ret = env_export(env_new);
	if (ret)
		goto fini;

#ifdef CONFIG_ENV_OFFSET_REDUND
	if (gd->env_valid == ENV_VALID)
		copy = 1;
#endif

	if (scsi_get_env_addr(copy, &offset)) {
		ret = 1;
		goto fini;
	}

	pr_info("Writing to %sSCSI... ", copy ? "redundant " : "");
	if (write_env(CONFIG_ENV_SIZE, offset, (u_char *)env_new)) {
		puts("failed\n");
		ret = 1;
		goto fini;
	}

	ret = 0;

#ifdef CONFIG_ENV_OFFSET_REDUND
	gd->env_valid = gd->env_valid == ENV_REDUND ? ENV_VALID : ENV_REDUND;
#endif

fini:
	return ret;
}

static inline int erase_env(u64 size, u64 offset)
{
	uint blk_start, blk_cnt, n;
	int dev = scsi_get_env_dev();
	struct blk_desc *desc;

	desc = blk_get_devnum_by_type(IF_TYPE_SCSI, dev);

	if (!desc) {
		return -1;
	}

	blk_start	= ALIGN(offset, 4096) / 4096;
	blk_cnt		= ALIGN(size, 4096) / 4096;

	n = blk_derase(desc, blk_start, blk_cnt);
	printf("%d blocks erased: %s\n", n, (n == blk_cnt) ? "OK" : "ERROR");

	return (n == blk_cnt) ? 0 : 1;
}

static int env_scsi_erase(void)
{
	int	ret, copy = 0;
	u32	offset;

	if (scsi_get_env_addr(copy, &offset))
		return CMD_RET_FAILURE;

	ret = erase_env(CONFIG_ENV_SIZE, offset);

#ifdef CONFIG_ENV_OFFSET_REDUND
	copy = 1;

	if (scsi_get_env_addr(copy, &offset))
		return CMD_RET_FAILURE;

	ret |= erase_env(CONFIG_ENV_SIZE, offset);
#endif

	return ret;
}
#endif /* CONFIG_CMD_SAVEENV && !CONFIG_SPL_BUILD */

static inline int read_env(unsigned long size,
			   unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;
	int dev = scsi_get_env_dev();
	struct blk_desc *desc;

	desc = blk_get_devnum_by_type(IF_TYPE_SCSI, dev);

	if (!desc) {
		return -1;
	}

	blk_start	= ALIGN(offset, 4096) / 4096;
	blk_cnt		= ALIGN(size, 4096) / 4096;

	n = blk_dread(desc, blk_start, blk_cnt, (uchar *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

static int env_scsi_load(void)
{
#if !defined(ENV_IS_EMBEDDED)
	//ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	ALLOC_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE, 4096);
	u32 offset;
	int ret;
	const char *errmsg;
	env_t *ep = NULL;

	if (scsi_get_env_addr(0, &offset)) {
		ret = -EIO;
		goto fini;
	}

	if (read_env(CONFIG_ENV_SIZE, offset, buf)) {
		errmsg = "!read failed";
		ret = -EIO;
		goto fini;
	}

	ret = env_import(buf, 1, H_EXTERNAL);
	if (!ret) {
		ep = (env_t *)buf;
		gd->env_addr = (ulong)&ep->data;
	}

fini:
	if (ret)
		env_set_default(errmsg, 0);
#endif
	return ret;
}

U_BOOT_ENV_LOCATION(scsi) = {
	.location	= ENVL_SCSI,
	ENV_NAME("SCSI")
	.load		= env_scsi_load,
#ifndef CONFIG_SPL_BUILD
	.save		= env_save_ptr(env_scsi_save),
	.erase		= ENV_ERASE_PTR(env_scsi_erase),
#endif
};
