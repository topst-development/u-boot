// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014, Staubli Faverges
 * Pierre Aubert
 *
 * eMMC- Replay Protected Memory Block
 * According to JEDEC Standard No. 84-A441
 */
/*
 * Modified by Telechips Inc. (date: 2021-08)
 */
/*
 * UFS- Replay Protected Memory Block
 */

#include <config.h>
#include <common.h>
#include <linux/unaligned/access_ok.h>
#include <memalign.h>
#include <scsi.h>
#include <u-boot/sha256.h>
#include "ufs.h"

/* Request codes */
#define RPMB_REQ_KEY		1
#define RPMB_REQ_WCOUNTER	2
#define RPMB_REQ_WRITE_DATA	3
#define RPMB_REQ_READ_DATA	4
#define RPMB_REQ_STATUS		5

/* Response code */
#define RPMB_RESP_KEY		0x0100
#define RPMB_RESP_WCOUNTER	0x0200
#define RPMB_RESP_WRITE_DATA	0x0300
#define RPMB_RESP_READ_DATA	0x0400

/* Error codes */
#define RPMB_OK			0
#define RPMB_ERR_GENERAL	1
#define RPMB_ERR_AUTH	2
#define RPMB_ERR_COUNTER	3
#define RPMB_ERR_ADDRESS	4
#define RPMB_ERR_WRITE		5
#define RPMB_ERR_READ		6
#define RPMB_ERR_KEY		7
#define RPMB_ERR_CNT_EXPIRED	0x80
#define RPMB_ERR_MSK		0x7

/* Sizes of RPMB data frame */
#define RPMB_SZ_STUFF		196
#define RPMB_SZ_MAC		32
#define RPMB_SZ_DATA		256
#define RPMB_SZ_NONCE		16

#define SHA256_BLOCK_SIZE	64

#define SEC_PROTOCOL_UFS 0xEC
#define SEC_SPECIFIC_UFS_RPMB 0x0001

static struct scsi_cmd tempccb; /* temporary scsi command buffer */

/* Error messages */
static const char * const rpmb_err_msg[] = {
	"",
	"General failure",
	"Authentication failure",
	"Counter failure",
	"Address failure",
	"Write failure",
	"Read failure",
	"Authentication key not yet programmed",
};


/* Structure of RPMB data frame. */
struct s_rpmb {
	unsigned char stuff[RPMB_SZ_STUFF];
	unsigned char mac[RPMB_SZ_MAC];
	unsigned char data[RPMB_SZ_DATA];
	unsigned char nonce[RPMB_SZ_NONCE];
	unsigned int write_counter;
	unsigned short address;
	unsigned short block_count;
	unsigned short result;
	unsigned short request;
};

static int ufs_rpmb_request(struct udevice *scsi_dev, const struct s_rpmb *s,
			    unsigned int count)
{
	struct scsi_cmd *pccb;
	u32 trans_len = count * sizeof(struct s_rpmb);
	unsigned char *buf;
	int ret = 0;

	pccb = &tempccb;
	memset(pccb, 0, sizeof(struct scsi_cmd));

	buf = memalign(4096, count * sizeof(struct s_rpmb));
	memcpy(buf, s, trans_len);

	pccb->lun = UFS_UPIU_RPMB_WLUN;
	pccb->cmd[0] = SCSI_SECURITY_PROTOCOL_OUT;
	pccb->cmd[1] = SEC_PROTOCOL_UFS;
	put_unaligned_be16(SEC_SPECIFIC_UFS_RPMB,
		pccb->cmd + 2);
	pccb->cmd[4] = 0; /* inc_512 bit 7 set to 0 */
	put_unaligned_be32(trans_len, pccb->cmd + 6); /* transfer length */
	pccb->pdata = (unsigned char *)buf;
	pccb->dma_dir = DMA_TO_DEVICE;
	pccb->datalen = trans_len;
	pccb->cmdlen = 12;

	ret = scsi_exec(scsi_dev, pccb);

#if 0
	free(pccb);
#endif
	free(buf);

	if (ret) {
		printf("%s:scsi_exec()-> %d\n", __func__, ret);
	}

	return ret;
}
static int ufs_rpmb_response(struct udevice *scsi_dev, struct s_rpmb *s,
			     unsigned int count, unsigned short expected)
{
	struct scsi_cmd *pccb;
	u32 trans_len = count * sizeof(struct s_rpmb);
	unsigned char *buf;
	int ret = 0;

	pccb = &tempccb;
	memset(pccb, 0, sizeof(struct scsi_cmd));

	buf = memalign(4096, count * sizeof(struct s_rpmb));
	memcpy(buf, s, trans_len);

	pccb->lun = UFS_UPIU_RPMB_WLUN;
	pccb->cmd[0] = SCSI_SECURITY_PROTOCOL_IN;
	pccb->cmd[1] = SEC_PROTOCOL_UFS;
	put_unaligned_be16(SEC_SPECIFIC_UFS_RPMB,
		pccb->cmd + 2);
	pccb->cmd[4] = 0; /* inc_512 bit 7 set to 0 */
	put_unaligned_be32(trans_len, pccb->cmd + 6); /* transfer length */
	pccb->pdata = (unsigned char *)buf;
	pccb->dma_dir = DMA_FROM_DEVICE;
	pccb->datalen = trans_len;
	pccb->cmdlen = 12;

	ret = scsi_exec(scsi_dev, pccb);

	memcpy(s, pccb->pdata, trans_len);

	if (ret) {
		pr_err("%s:scsi_exec()-> %d\n", __func__, ret);
	} else {
		if (expected == 0)
			goto out;

		/* Check the response and the status */
		if (be16_to_cpu(s->request) != expected) {
			printf("%s:response= %x, result= %x\n", __func__,
					be16_to_cpu(s->request), be16_to_cpu(s->result));
			ret = -1;
			goto out;
		}

		ret = be16_to_cpu(s->result);
		if (ret) {
			pr_err("%s %s\n", rpmb_err_msg[ret & RPMB_ERR_MSK],
					(ret & RPMB_ERR_CNT_EXPIRED) ?
					"Write counter has expired" : "");
		}
	}
out:
	pr_info("%s:response= %x, result= %x\n", __func__,
		be16_to_cpu(s->request), be16_to_cpu(s->result));

#if 0
	free(pccb);
#endif
	free(buf);

	/* Return the status of the command */
	return ret;
}
static int ufs_rpmb_status(struct udevice *scsi_dev, unsigned short expected)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);

	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_STATUS);
	if (ufs_rpmb_request(scsi_dev, rpmb_frame, 1))
		return -1;

	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	/* Read the result */
	return ufs_rpmb_response(scsi_dev, rpmb_frame, 1, expected);
}
static void rpmb_hmac(unsigned char *key, unsigned char *buff, int len,
		      unsigned char *output)
{
	sha256_context ctx;
	int i;
	unsigned char k_ipad[SHA256_BLOCK_SIZE];
	unsigned char k_opad[SHA256_BLOCK_SIZE];

	sha256_starts(&ctx);

	/* According to RFC 4634, the HMAC transform looks like:
	   SHA(K XOR opad, SHA(K XOR ipad, text))

	   where K is an n byte key.
	   ipad is the byte 0x36 repeated blocksize times
	   opad is the byte 0x5c repeated blocksize times
	   and text is the data being protected.
	*/

	for (i = 0; i < RPMB_SZ_MAC; i++) {
		k_ipad[i] = key[i] ^ 0x36;
		k_opad[i] = key[i] ^ 0x5c;
	}
	/* remaining pad bytes are '\0' XOR'd with ipad and opad values */
	for ( ; i < SHA256_BLOCK_SIZE; i++) {
		k_ipad[i] = 0x36;
		k_opad[i] = 0x5c;
	}
	sha256_update(&ctx, k_ipad, SHA256_BLOCK_SIZE);
	sha256_update(&ctx, buff, len);
	sha256_finish(&ctx, output);

	/* Init context for second pass */
	sha256_starts(&ctx);

	/* start with outer pad */
	sha256_update(&ctx, k_opad, SHA256_BLOCK_SIZE);

	/* then results of 1st hash */
	sha256_update(&ctx, output, RPMB_SZ_MAC);

	/* finish up 2nd pass */
	sha256_finish(&ctx, output);
}

int ufs_rpmb_get_counter(struct udevice *scsi_dev, unsigned long *pcounter)
{
	int ret;
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);

	/* Fill the request */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_WCOUNTER);

	if (ufs_rpmb_request(scsi_dev, rpmb_frame, 1))
		return -1;

	/* Read the result */
	ret = ufs_rpmb_response(scsi_dev, rpmb_frame, 1, RPMB_RESP_WCOUNTER);
	if (ret)
		return ret;

	*pcounter = be32_to_cpu(rpmb_frame->write_counter);
	return 0;
}
int ufs_rpmb_set_key(struct udevice *scsi_dev, void *key)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);
	/* Fill the request */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_KEY);
	memcpy(rpmb_frame->mac, key, RPMB_SZ_MAC);

	if (ufs_rpmb_request(scsi_dev, rpmb_frame, 1))
		return -1;

	/* read the operation status */
	return ufs_rpmb_status(scsi_dev, RPMB_RESP_KEY);
}
int ufs_rpmb_read(struct udevice *scsi_dev, void *addr, unsigned short blk,
		  unsigned short cnt, unsigned char *key)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);
	int i;

	for (i = 0; i < cnt; i++) {
		/* Fill the request */
		memset(rpmb_frame, 0, sizeof(struct s_rpmb));
		rpmb_frame->address = cpu_to_be16(blk + i);
		rpmb_frame->request = cpu_to_be16(RPMB_REQ_READ_DATA);
		if (ufs_rpmb_request(scsi_dev, rpmb_frame, 1))
			break;

		/* Read the result */
		if (ufs_rpmb_response(scsi_dev, rpmb_frame, 1, RPMB_RESP_READ_DATA))
			break;

		/* Check the HMAC if key is provided */
		if (key) {
			unsigned char ret_hmac[RPMB_SZ_MAC];

			rpmb_hmac(key, rpmb_frame->data, 284, ret_hmac);
			if (memcmp(ret_hmac, rpmb_frame->mac, RPMB_SZ_MAC)) {
				printf("MAC error on block #%d\n", i);
				break;
			}
		}
		/* Copy data */
		memcpy(addr + i * RPMB_SZ_DATA, rpmb_frame->data, RPMB_SZ_DATA);
	}
	return i;
}
int ufs_rpmb_write(struct udevice *scsi_dev, void *addr, unsigned short blk,
		  unsigned short cnt, unsigned char *key)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);
	unsigned long wcount;
	int i;

	for (i = 0; i < cnt; i++) {
		if (ufs_rpmb_get_counter(scsi_dev, &wcount)) {
			printf("Cannot read RPMB write counter\n");
			break;
		}

		/* Fill the request */
		memset(rpmb_frame, 0, sizeof(struct s_rpmb));
		memcpy(rpmb_frame->data, addr + i * RPMB_SZ_DATA, RPMB_SZ_DATA);
		rpmb_frame->address = cpu_to_be16(blk + i);
		rpmb_frame->block_count = cpu_to_be16(1);
		rpmb_frame->write_counter = cpu_to_be32(wcount);
		rpmb_frame->request = cpu_to_be16(RPMB_REQ_WRITE_DATA);
		/* Computes HMAC */
		rpmb_hmac(key, rpmb_frame->data, 284, rpmb_frame->mac);

		if (ufs_rpmb_request(scsi_dev, rpmb_frame, 1))
			break;

		/* Get status */
		if (ufs_rpmb_status(scsi_dev, RPMB_RESP_WRITE_DATA))
			break;
	}
	return i;
}

static int send_write_mult_block(struct udevice *scsi_dev, const struct s_rpmb *frm,
				 unsigned short cnt)
{
	return ufs_rpmb_request(scsi_dev, frm, cnt);
}

static int send_read_mult_block(struct udevice *scsi_dev, struct s_rpmb *frm,
				unsigned short cnt)
{
	return ufs_rpmb_response(scsi_dev, frm, cnt, 0);
}

static int rpmb_route_write_req(struct udevice *scsi_dev, struct s_rpmb *req,
				unsigned short req_cnt, struct s_rpmb *rsp,
				unsigned short rsp_cnt)
{
	int ret;

	/*
	 * Send the write request.
	 */
	ret = send_write_mult_block(scsi_dev, req, req_cnt);
	if (ret)
		return ret;

	memset(rsp, 0, sizeof(*rsp));
	rsp->request = cpu_to_be16(RPMB_REQ_STATUS);
	ret = send_write_mult_block(scsi_dev, rsp, rsp_cnt);
	if (ret)
		return ret;

	return send_read_mult_block(scsi_dev, rsp, 1);
}

static int rpmb_route_read_req(struct udevice *scsi_dev, struct s_rpmb *req,
			       unsigned short req_cnt, struct s_rpmb *rsp,
			       unsigned short rsp_cnt)
{
	int ret;

	ret = send_write_mult_block(scsi_dev, req, req_cnt);
	if (ret)
		return ret;

	/*
	 * Read the result of the request.
	 */
	return send_read_mult_block(scsi_dev, rsp, rsp_cnt);
}

static int rpmb_route_frames(struct udevice *scsi_dev, struct s_rpmb *req,
			     unsigned short req_cnt, struct s_rpmb *rsp,
			     unsigned short rsp_cnt)
{
	unsigned short n;

	/*
	 * If multiple request frames are provided, make sure that all are
	 * of the same type.
	 */
	for (n = 1; n < req_cnt; n++)
		if (req[n].request != req->request)
			return -EINVAL;

	switch (be16_to_cpu(req->request)) {
	case RPMB_REQ_KEY:
		if (req_cnt != 1 || rsp_cnt != 1)
			return -EINVAL;
		return rpmb_route_write_req(scsi_dev, req, req_cnt, rsp, rsp_cnt);

	case RPMB_REQ_WRITE_DATA:
		if (!req_cnt || rsp_cnt != 1)
			return -EINVAL;
		return rpmb_route_write_req(scsi_dev, req, req_cnt, rsp, rsp_cnt);

	case RPMB_REQ_WCOUNTER:
		if (req_cnt != 1 || rsp_cnt != 1)
			return -EINVAL;
		return rpmb_route_read_req(scsi_dev, req, req_cnt, rsp, rsp_cnt);

	case RPMB_REQ_READ_DATA:
		if (req_cnt != 1 || !req_cnt)
			return -EINVAL;
		return rpmb_route_read_req(scsi_dev, req, req_cnt, rsp, rsp_cnt);

	default:
		debug("Unsupported message type: %d\n",
		      be16_to_cpu(req->request));
		return -EINVAL;
	}
}

int ufs_rpmb_route_frames(struct udevice *scsi_dev, void *req, unsigned long reqlen,
			  void *rsp, unsigned long rsplen)
{
	/*
	 * Whoever crafted the data supplied to this function knows how to
	 * format the PRMB frames and which response is expected. If
	 * there's some unexpected mismatch it's more helpful to report an
	 * error immediately than trying to guess what was the intention
	 * and possibly just delay an eventual error which will be harder
	 * to track down.
	 */

	if (reqlen % sizeof(struct s_rpmb) || rsplen % sizeof(struct s_rpmb))
		return -EINVAL;

	return rpmb_route_frames(scsi_dev, req, reqlen / sizeof(struct s_rpmb),
				 rsp, rsplen / sizeof(struct s_rpmb));
}
