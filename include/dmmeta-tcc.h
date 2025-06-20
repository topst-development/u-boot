// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef DMMETA_TCC_H
#define DMMETA_TCC_H

struct dmmeta {
	uint8_t signature[8];
	uint32_t version;
	uint32_t hash_type;
	uint8_t uuid[16];
	uint8_t algorithm[32];
	uint32_t data_block_size;
	uint32_t hash_block_size;
	uint64_t data_blocks;
	uint16_t salt_size;
	uint8_t _pad1[6];
	uint8_t salt[256];
	uint8_t _pad2[168];
};

uint64_t dmmeta_size(struct dmmeta *meta);
int dmmeta_setup(const char *dev, struct dmmeta *meta, char *sig);

#endif /* DMMETA_TCC_H */
