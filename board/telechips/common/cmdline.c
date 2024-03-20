// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <malloc.h>
#include <android_bootloader.h>
#include <asm/setup.h>
#include <mach/reboot.h>

typedef const char *(*cmdline_get_fn)(void);

/**
 * struct cmdline_hash - Hashmap for kernel cmdline parameter (or bootargs)
 * @key: The key part string
 * @get: The getter function returning the value part
 */
struct cmdline_hash {
	const char *key;
	cmdline_get_fn get;
};

/**
 * cmdline_hash_get_pair() - Return key=value formed string for a given hash
 * @hashmap: struct cmdline_hash to pair key, value
 *
 * Return:
 * * key=value in string, if hashmap->key != NULL and hashmap->get() != NULL
 * * value in string, if hashmap->key == NULL (cf. for boolean type parameter)
 * * NULL, for other cases
 */
static const char *cmdline_hash_get_pair(const struct cmdline_hash *hashmap)
{
	const char *pair = NULL;

	if ((hashmap != NULL) && (hashmap->get != NULL)) {
		const char *val = hashmap->get();

		if ((hashmap->key != NULL) && (val != NULL)) {
			/* Return key=value in string */
			const char *chunks[3] = { hashmap->key, val, NULL };

			pair = strjoin(chunks, '=');
		}

		if ((hashmap->key == NULL) && (val != NULL)) {
			/* Return value in string */
			pair = val;
		}
	}

	return pair;
}

/**
 * cmdline_get_xxx() - Return value of cmdline hash.
 *
 * This functions return the value part of cmdline hash for parameter xxx
 * which mostly constists as key=value formed string. The function must be
 * in form of cmdline_get_fn, and registered in cmdline_hash_tbl[].
 *
 * Note that the default implementation of each cmdline_get_xxx is only for
 * reference, and users must modify it as they need.
 *
 * Return:
 * The value part of kernel cmdline parameter
 */
static const char *cmdline_get_default(void)
{
	return env_get("bootargs");
}

static const char *cmdline_get_hardware(void)
{
	return CONFIG_SYS_SOC;
}

static const char *cmdline_get_serialno(void)
{
	return env_get("serial#");
}

static const char *cmdline_get_wifimac(void)
{
	return NULL;
}

static const char *cmdline_get_btaddr(void)
{
	return NULL;
}

static const char *cmdline_get_bootreason(void)
{
	return read_boot_reason();
}

static const char *cmdline_get_dtboidx(void)
{
	return env_get("dtboidx");
}

static struct cmdline_hash cmdline_hash_tbl[] = {
	{ NULL, cmdline_get_default },
	{ "androidboot.hardware", cmdline_get_hardware },
	{ "androidboot.serialno", cmdline_get_serialno },
	{ "androidboot.wifimac", cmdline_get_wifimac },
	{ "androidboot.btaddr", cmdline_get_btaddr },
	{ "androidboot.bootreason", cmdline_get_bootreason },
	{ "androidboot.dtbo_idx", cmdline_get_dtboidx },
};

void setup_board_cmdline(void)
{
	const char *chunks[16], **current_chunk = chunks;
	const char *cmdline;
	ulong i;
	s32 ret;

	for (i = 0UL; i < ARRAY_SIZE(cmdline_hash_tbl); i++) {
		const struct cmdline_hash *hash = &cmdline_hash_tbl[i];
		const char *chunk = cmdline_hash_get_pair(hash);

		if (chunk != NULL) {
			*current_chunk = chunk;
			++current_chunk;
		}
	};

	*current_chunk = NULL;

	cmdline = strjoin(chunks, ' ');
	ret = env_set("bootargs", cmdline);

	if (ret != 0) {
		/* Print error, but continue booting anyway */
		pr_err("Can't set bootargs on %s (ret: %d)\n", __func__, ret);
	}
}
