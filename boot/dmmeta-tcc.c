// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <command.h>
#include <common.h>
#include <dm.h>
#include <image.h>
#include <crypto/ecdsa-uclass.h>
#include <dmmeta-tcc.h>
#include "dmmeta-tcc-key.h"

/* #define DMMETA_DEBUG */
#if defined(DMMETA_DEBUG)
static void hxout(const char *buf, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		printf("%02X", buf[i]);
	}
	printf("\n");
}
#endif

static int asn1_publickey(const char *buf, int len, const char **rawkey)
{
	int s, l;

	while (len > 2) {
		if (buf[1] == 0x82) {
			s = 3;
			l = (int)((buf[1] << 8) | buf[2]);
		} else {
			s = 2;
			l = (int)buf[1];
		}

		if (buf[0] == 0x30) {
			if (asn1_publickey(&buf[s], l, rawkey) == 0) {
				return 0;
			}
		} else if (buf[0] == 0x03) {
			if (buf[1] == 0x42) {
				*rawkey = &buf[s+2];
			} else {
				break;
			}
			return 0;
		}
		buf += (s + l);
		len -= (s + l);
	}
	return -1;
}

static int asn1_signature(const char *der, char *sig)
{
	if (der[0] != 0x30) {
		return -1;
	}
	der += 2;

	if (der[0] != 0x02) {
		return -1;
	} else if (der[1] == 0x20) {
		memcpy(&sig[0], &der[2], 32);
		der += 0x22;
	} else if (der[1] == 0x21) {
		memcpy(&sig[0], &der[3], 32);
		der += 0x23;
	} else {
		return -1;
	}

	if (der[0] != 0x02) {
		return -1;
	} else if (der[1] == 0x20) {
		memcpy(&sig[32], &der[2], 32);
	} else if (der[1] == 0x21) {
		memcpy(&sig[32], &der[3], 32);
	} else {
		return -1;
	}

	return 0;
}

static int dmmeta_verify(const char *buf, int len, const char *der)
{
	struct udevice *dev;
	struct image_region reg;
	char sig[64];
	char hash[32];
	struct ecdsa_public_key key = {
		.curve_name = "prime256v1",
		.size_bits = 256,
	};
	int ret = 0;
	const char *rawkey;
	const struct ecdsa_ops *ops;

	ret = asn1_publickey(dmmeta_tcc_key_der, dmmeta_tcc_key_der_len, &rawkey);
	if (ret == 0) {
		key.x = &rawkey[0];
		key.y = &rawkey[32];
		reg.data = buf;
		reg.size = len;

		ret = asn1_signature(der, sig);
#if defined(DMMETA_DEBUG)
		hxout(rawkey, 32);
		hxout(rawkey+32, 32);
		hxout(sig, 32);
		hxout(sig+32, 32);
#endif
	}

	if (ret == 0) {
		memset(hash, 0, 32);
		ret = hash_calculate("sha256", &reg, 1, hash);
	}
	if (ret == 0) {
		uclass_first_device_err(UCLASS_ECDSA, &dev);
		ops = device_get_ops(dev);
		ret = ops->verify(dev, &key, hash, 32, sig, 64);
	}
	return ret;
}

uint64_t dmmeta_size(struct dmmeta *meta)
{
	uint32_t blocks;
	uint32_t total;

	blocks = meta->data_blocks;
	total = 1;
	while (blocks > 1) {
		blocks = ((blocks * 32) + meta->hash_block_size - 1) \
			/ meta->hash_block_size;
		total += blocks;
	}
	return total * meta->hash_block_size;
}

int dmmeta_setup(const char *dev, struct dmmeta *meta, char *sig)
{
	char dmsetup[512] = { 0, };
	char strsalt[128] = { 0, };
	char strhash[128] = { 0, };
	int i;

	memcpy(strhash, sig, 64);
	sig += 64;

	if (dmmeta_verify(strhash, 64, sig) == 0) {
		for (i = 0; i < 32; i++) {
			sprintf(&strsalt[i<<1], "%02x", meta->salt[i]);
		}

		sprintf(dmsetup, \
			"dm-mod.create=\"verity,,,ro,0 %lld verity 1 " \
			"%s %s %d %d %lld %lld %s %s %s\"",
			meta->data_blocks * meta->data_block_size / 512,
			dev,
			dev,
			meta->data_block_size,
			meta->hash_block_size,
			meta->data_blocks,
			meta->data_blocks + 1,
			meta->algorithm,
			strhash,
			strsalt);
		env_set("dmsetup", dmsetup);
		return 0;
	}
	pr_err("Verify Error\n");
	return -1;
}
