/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <stdint.h>
#include <string.h>
#include <dm.h>
#include <linux/soc/telechips/tcc_mbox.h>
#include <cpu_func.h>
#include <hw_sha.h>
#include <mailbox-uclass.h>
#include <crypto/ecdsa-uclass.h>
#include <tcc_hsm_fw.h>

#if IS_ENABLED(CONFIG_TCC750X)
#define CID                                 (0x53U)
#elif IS_ENABLED(CONFIG_TCC805X)
#if IS_ENABLED(CONFIG_USE_SUBCORE)
#define CID                                 (0x53U)
#else
#define CID                                 (0x72U)
#endif
#elif IS_ENABLED(CONFIG_TCC807X)
#if IS_ENABLED(CONFIG_USE_SUBCORE)
#define CID                                 (0x02U)
#else
#define CID                                 (0x01U)
#endif
#endif

#define BSID                                (0x45U)
#define MAX_CMD                             (8)
#define TCC_HSM_DMA                         (1U)

#define SHA1_HASH_LENGTH                    (20U)
#define SHA256_HASH_LENGTH                  (32U)
#define ECDSA256_SIGNATURE_LENGTH           (64U)
#define ECDSA256_BITS                       (256)
#define ECDSA256_PUBKEY_LENGTH              (64U)

#define ERROR_HSM_MBOX_SUCCESS              (0x00000000U)
#define ERROR_UNKNOWN_CID                   (0x80000001U)
#define ERROR_UNKNOWN_BSID                  (0x80000002U)
#define ERROR_VERIFY_FAIL                   (0x80000003U)
#define ERROR_VERIFY_OK                     (0x80000004U)
#define ERROR_HSMBOOT_FAIL                  (0x80000005U)
#define ERROR_HSMBOOT_OK                    (0x80000006U)
#define ERROR_UNKNOWN_ERROR                 (0x80000000U)

#define REQ_HSM_VERIFY                      (0x00020000U | 0x14U)
#define REQ_HSM_GET_SECURE_IMAGE            (0x00070000U)
#define REQ_HSM_KEY_WRITE                   (0x00080000U)
#define REQ_HSM_SECURE_BOOT_ENABLE          (0x00090000U)

#define REQ_HSM_GEN_SHA                     (0x10210000U | 0x400U)
#define REQ_HSM_RUN_ECDSA_VERIFY            (0x10320000U | 0x400U | 0xB0U)

#define REQ_HSM_GET_VER                     (0x20010000U)

#define TCC_HSM_OID_SHA1_160                (0x00001100U)
#define TCC_HSM_OID_SHA2_256                (0x00002300U)
#define TCC_HSM_OID_ECC_P256                (0x00000013U)

struct tcc_hsm_device {
	struct mbox_chan chan;
	unsigned int major;
	unsigned int minor;
	unsigned int patch;
};

static int hsm_communicate(u32 *cmd, u32 *txbuf, u32 txlen, u32 *rxbuf, u32 rxlen)
{
	struct udevice *dev;
	struct tcc_hsm_device *hsm;
	struct tcc_mbox_msg msg;
	int ret = -1;

	(void)uclass_get_device_by_name(UCLASS_FIRMWARE, "tcc_hsm_fw", &dev);
	hsm = dev_get_priv(dev);

	if (hsm != NULL) {
		msg.cmd_len = MAX_CMD;
		msg.cmd = cmd;
		msg.data_buf = txbuf;
		msg.data_len = txlen;
		ret = mbox_send(&hsm->chan, &msg);
		if (ret == 0) {
			msg.data_buf = rxbuf;
			msg.data_len = rxlen;
			ret = mbox_recv(&hsm->chan, &msg, 5000000);
			if (ret != 0) {
				pr_err("HSM MBOX Recv Error (%d)\n", ret);
			}
		} else {
			pr_err("HSM MBOX Send Error (%d)\n", ret);
		}
	}

	return ret;
}

static int tcc_hsm_get_version(struct tcc_hsm_device *hsm)
{
	u32 cmd[MAX_CMD];
	int ret = -1;

	memset(cmd, 0x0, sizeof(cmd));

	cmd[0] = (CID << 8) & BSID;
	cmd[1] = REQ_HSM_GET_VER;

	if (hsm_communicate(cmd, NULL, 0, NULL, 0) == 0) {
		if (cmd[2] == ERROR_HSM_MBOX_SUCCESS) {
			hsm->major = cmd[4];
			hsm->minor = cmd[5];
			hsm->patch = cmd[6];
			ret = 0;
		}
	}

	return ret;
}

int tcc_hsm_secureboot_image_verify(
	unsigned int header_addr, unsigned int header_size,
	unsigned int img_addr, unsigned int img_size, unsigned int tccimgid)
{
	u32 cmd[MAX_CMD];
	int ret = -1;

	memset(cmd, 0x0, sizeof(cmd));

	cmd[0] = (CID << 8) & BSID;
	cmd[1] = REQ_HSM_VERIFY;
	cmd[2] = header_addr;
	cmd[3] = header_size;
	cmd[4] = img_addr;
	cmd[5] = img_size;
	cmd[6] = tccimgid;

	if (hsm_communicate(cmd, NULL, 0, NULL, 0) == 0) {
		if (cmd[2] == ERROR_HSM_MBOX_SUCCESS) {
			ret = 0;
		}
	}

	return ret;
}

int tcc_hsm_secureboot_image_encrypt(
	unsigned int img_addr, unsigned int img_size, unsigned int tccimgid,
	unsigned int bootid)
{
	u32 cmd[MAX_CMD];
	int ret = -1;

	memset(cmd, 0x0, sizeof(cmd));

	cmd[0] = (CID << 8) & BSID;
	cmd[1] = REQ_HSM_GET_SECURE_IMAGE;
	cmd[2] = img_addr;
	cmd[3] = img_size;
	cmd[4] = tccimgid;

	if (hsm_communicate(cmd, NULL, 0, NULL, 0) == 0) {
		if (cmd[2] == ERROR_HSM_MBOX_SUCCESS) {
			ret = 0;
		}
	}

	return ret;
}

int tcc_hsm_secureboot_key_fuse(void)
{
	u32 cmd[MAX_CMD];
	int ret = -1;

	memset(cmd, 0x0, sizeof(cmd));

	cmd[0] = (CID << 8) & BSID;
	cmd[1] = REQ_HSM_KEY_WRITE;

	if (hsm_communicate(cmd, NULL, 0, NULL, 0) == 0) {
		if (cmd[2] == ERROR_HSM_MBOX_SUCCESS) {
			ret = 0;
		}
	}

	return ret;
}

int tcc_hsm_secureboot_enable(void)
{
	u32 cmd[MAX_CMD];
	int ret = -1;

	memset(cmd, 0x0, sizeof(cmd));

	cmd[0] = (CID << 8) & BSID;
	cmd[1] = REQ_HSM_SECURE_BOOT_ENABLE;

	if (hsm_communicate(cmd, NULL, 0, NULL, 0) == 0) {
		if (cmd[2] == ERROR_HSM_MBOX_SUCCESS) {
			ret = 0;
		}
	}

	return ret;
}

static int tcc_hsm_sha1(uintptr_t src, uint32_t size, char *hash)
{
	struct udevice *dev;
	struct tcc_hsm_device *hsm;
	u32 cmd[MAX_CMD];
	u32 txdata[6];
	u32 rxdata[10];
	u32 txsize = 0U;
	int ret = -1;

	(void)uclass_get_device_by_name(UCLASS_FIRMWARE, "tcc_hsm_fw", &dev);
	hsm = dev_get_priv(dev);

	flush_dcache_range(src, src + size);

	memset(&cmd, 0x0, sizeof(cmd));

	if (hsm->major != 0U) {
		txdata[txsize] = 0U;
		txsize++;
	}
	txdata[txsize] = TCC_HSM_OID_SHA1_160;
	txsize++;
	txdata[txsize] = TCC_HSM_DMA;
	txsize++;
	txdata[txsize] = size; /* srcsize */
	txsize++;
	txdata[txsize] = src; /* src */
	txsize++;
	txdata[txsize] = SHA1_HASH_LENGTH;
	txsize++;

	cmd[0] = (CID << 8) & BSID;
	cmd[1] = REQ_HSM_GEN_SHA | (txsize << 2U);

	if (hsm_communicate(cmd, txdata, txsize, rxdata, 10) == 0) {
		memcpy(hash, &rxdata[2], SHA1_HASH_LENGTH);
		ret = 0;
	}

	return ret;
}

static int tcc_hsm_sha256(uintptr_t src, uint32_t size, char *hash)
{
	struct udevice *dev;
	struct tcc_hsm_device *hsm;
	u32 cmd[MAX_CMD];
	u32 txdata[6];
	u32 rxdata[10];
	u32 txsize = 0U;
	int ret = -1;

	(void)uclass_get_device_by_name(UCLASS_FIRMWARE, "tcc_hsm_fw", &dev);
	hsm = dev_get_priv(dev);

	flush_dcache_range(src, src + size);

	memset(&cmd, 0x0, sizeof(cmd));

	if (hsm->major != 0U) {
		txdata[txsize] = 0U;
		txsize++;
	}
	txdata[txsize] = TCC_HSM_OID_SHA2_256;
	txsize++;
	txdata[txsize] = TCC_HSM_DMA;
	txsize++;
	txdata[txsize] = size;
	txsize++;
	txdata[txsize] = src;
	txsize++;
	txdata[txsize] = SHA256_HASH_LENGTH;
	txsize++;

	cmd[0] = (CID << 8) & BSID;
	cmd[1] = REQ_HSM_GEN_SHA | (txsize << 2U);

	if (hsm_communicate(cmd, txdata, txsize, rxdata, 10) == 0) {
		memcpy(hash, &rxdata[2], SHA256_HASH_LENGTH);
		ret = 0;
	}

	return ret;
}

#if CONFIG_IS_ENABLED(ECDSA_VERIFY)
static int tcc_hsm_ecdsa256_verify(struct udevice *dev,
			       const struct ecdsa_public_key *pubkey,
			       const void *hash, size_t hash_len,
			       const void *signature, size_t sig_len)
{
	u32 cmd[MAX_CMD];
	u32 txdata[44];
	u32 rxdata[2];
	int ret = -1;

	if (sig_len != ECDSA256_SIGNATURE_LENGTH || hash_len != SHA256_HASH_LENGTH || pubkey->size_bits != ECDSA256_BITS)
		return -EINVAL;

	if (strcmp(pubkey->curve_name, "prime256v1") != 0)
		return -EINVAL;

	memset(&cmd, 0x0, sizeof(cmd));

	cmd[0] = (CID << 8) & BSID;
	cmd[1] = REQ_HSM_RUN_ECDSA_VERIFY;

	txdata[0] = TCC_HSM_OID_ECC_P256;
	txdata[1] = ECDSA256_PUBKEY_LENGTH;
	memcpy(&txdata[2], pubkey->x, (ECDSA256_PUBKEY_LENGTH >> 1));
	memcpy(&txdata[10], pubkey->y, (ECDSA256_PUBKEY_LENGTH >> 1));
	txdata[18] = hash_len;
	memcpy(&txdata[19], hash, hash_len);
	txdata[27] = sig_len;
	memcpy(&txdata[28], signature, sig_len);

	if (hsm_communicate(cmd, txdata, 44, rxdata, 1) == 0) {
		ret = rxdata[0];
	}

	return ret;
}
#endif

static int tcc_hsm_probe(struct udevice *dev)
{
	struct tcc_hsm_device *hsm = dev_get_priv(dev);
	int ret = -1;

	pr_debug("%s\n", __func__);

	if (hsm != NULL) {
		ret = mbox_get_by_index(dev, 0, &hsm->chan);
		if (tcc_hsm_get_version(hsm) == 0) {
			pr_debug("HSM F/W Version: %d.%d.%d\n",
					hsm->major, hsm->minor, hsm->patch);
		} else {
			pr_err("Version Error\n");
		}
	}

	return ret;
}

static int tcc_hsm_remove(struct udevice *dev)
{
	struct tcc_hsm_device *hsm = dev_get_priv(dev);
	int ret = -1;

	pr_debug("%s\n", __func__);

	if (hsm != NULL) {
		ret = mbox_free(&hsm->chan);
	}

	return ret;
}

static const struct udevice_id tcc_hsm_ids[] = {
	{ .compatible = "telechips,tcc_hsm_fw", },
	{}
};

U_BOOT_DRIVER(tcc_hsm) = {
	.name = "tcc_hsm_fw",
	.id = UCLASS_FIRMWARE,
	.of_match = tcc_hsm_ids,
	.probe = tcc_hsm_probe,
	.remove = tcc_hsm_remove,
	.priv_auto = sizeof(struct tcc_hsm_device),
};

#if CONFIG_IS_ENABLED(ECDSA_VERIFY)
static const struct ecdsa_ops ecdsa_ops = {
	.verify = tcc_hsm_ecdsa256_verify,
};

U_BOOT_DRIVER(tcc_hsm_ecdsa) = {
	.name = "tcc_hsm_ecdsa",
	.id = UCLASS_ECDSA,
	.ops = &ecdsa_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRVINFO(tcc_hsm_ecdsa) = {
	.name = "tcc_hsm_ecdsa",
};
#endif

void hw_sha1(const uchar *in_addr, uint buflen, uchar *out_addr,
	     uint chunk_size)
{
	if (tcc_hsm_sha1((uintptr_t)in_addr, buflen, out_addr) != 0) {
		memset(out_addr, 0, SHA1_HASH_LENGTH);
	}
}

void hw_sha256(const uchar *in_addr, uint buflen, uchar *out_addr,
	       uint chunk_size)
{
	if (tcc_hsm_sha256((uintptr_t)in_addr, buflen, out_addr) != 0) {
		memset(out_addr, 0, SHA256_HASH_LENGTH);
	}
}
