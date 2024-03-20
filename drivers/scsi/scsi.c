// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <pci.h>
#include <scsi.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <memalign.h>
#include <ufs.h>
#include <cpu_func.h>

#if !defined(CONFIG_DM_SCSI)
# ifdef CONFIG_SCSI_DEV_LIST
#  define SCSI_DEV_LIST CONFIG_SCSI_DEV_LIST
# else
#  ifdef CONFIG_SATA_ULI5288

#   define SCSI_VEND_ID 0x10b9
#   define SCSI_DEV_ID  0x5288

#  elif !defined(CONFIG_SCSI_AHCI_PLAT)
#   error no scsi device defined
#  endif
# define SCSI_DEV_LIST {SCSI_VEND_ID, SCSI_DEV_ID}
# endif
#endif

#if defined(CONFIG_PCI) && !defined(CONFIG_SCSI_AHCI_PLAT) && \
	!defined(CONFIG_DM_SCSI)
const struct pci_device_id scsi_device_list[] = { SCSI_DEV_LIST };
#endif
static struct scsi_cmd tempccb;	/* temporary scsi command buffer */

static unsigned char tempbuff[512]; /* temporary data buffer */

#if !defined(CONFIG_DM_SCSI)
static int scsi_max_devs; /* number of highest available scsi device */

static int scsi_curr_dev; /* current device */

static struct blk_desc scsi_dev_desc[CONFIG_SYS_SCSI_MAX_DEVICE];
#endif

/* almost the maximum amount of the scsi_ext command.. */
#define SCSI_MAX_BLK 0xFFFF
#define SCSI_LBA48_READ	0xFFFFFFF

static void scsi_print_error(struct scsi_cmd *pccb)
{
	/* Dummy function that could print an error for debugging */
}

#ifdef CONFIG_SYS_64BIT_LBA
void scsi_setup_read16(struct scsi_cmd *pccb, lbaint_t start,
		       unsigned long blocks)
{
	pccb->cmd[0] = SCSI_READ16;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmd[2] = (unsigned char)(start >> 56) & 0xff;
	pccb->cmd[3] = (unsigned char)(start >> 48) & 0xff;
	pccb->cmd[4] = (unsigned char)(start >> 40) & 0xff;
	pccb->cmd[5] = (unsigned char)(start >> 32) & 0xff;
	pccb->cmd[6] = (unsigned char)(start >> 24) & 0xff;
	pccb->cmd[7] = (unsigned char)(start >> 16) & 0xff;
	pccb->cmd[8] = (unsigned char)(start >> 8) & 0xff;
	pccb->cmd[9] = (unsigned char)start & 0xff;
	pccb->cmd[10] = 0;
	pccb->cmd[11] = (unsigned char)(blocks >> 24) & 0xff;
	pccb->cmd[12] = (unsigned char)(blocks >> 16) & 0xff;
	pccb->cmd[13] = (unsigned char)(blocks >> 8) & 0xff;
	pccb->cmd[14] = (unsigned char)blocks & 0xff;
	pccb->cmd[15] = 0;
	pccb->cmdlen = 16;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
	debug("scsi_setup_read16: cmd: %02X %02X startblk %02X%02X%02X%02X%02X%02X%02X%02X blccnt %02X%02X%02X%02X\n",
	      pccb->cmd[0], pccb->cmd[1],
	      pccb->cmd[2], pccb->cmd[3], pccb->cmd[4], pccb->cmd[5],
	      pccb->cmd[6], pccb->cmd[7], pccb->cmd[8], pccb->cmd[9],
	      pccb->cmd[11], pccb->cmd[12], pccb->cmd[13], pccb->cmd[14]);
}
#endif

static void scsi_setup_inquiry(struct scsi_cmd *pccb)
{
	pccb->cmd[0] = SCSI_INQUIRY;
#if defined(CONFIG_TCC_UFS) || defined(CONFIG_TCC_SC_UFS)
	pccb->cmd[1] = 0;
#else
	pccb->cmd[1] = pccb->lun << 5;
#endif
	pccb->cmd[2] = 0;
	pccb->cmd[3] = 0;
	if (pccb->datalen > 255)
		pccb->cmd[4] = 255;
	else
		pccb->cmd[4] = (unsigned char)pccb->datalen;
	pccb->cmd[5] = 0;

#if defined(CONFIG_TCC_UFS) || defined(CONFIG_TCC_SC_UFS)
	pccb->cmdlen = 12;
#else
	pccb->cmdlen = 6;
#endif
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
}

static void scsi_setup_req_sense(struct scsi_cmd *pccb)
{
	pccb->cmd[0] = SCSI_REQ_SENSE;
	pccb->cmd[1] = 0;//pccb->lun << 5;
	pccb->cmd[2] = 0;
	pccb->cmd[3] = 0;
	if (pccb->datalen > 255)
		pccb->cmd[4] = 255;
	else
		pccb->cmd[4] = (unsigned char)pccb->datalen;
	pccb->cmd[5] = 0;
	pccb->cmdlen = 12;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
}
#ifdef CONFIG_BLK
static void scsi_setup_read_ext(struct scsi_cmd *pccb, lbaint_t start,
				unsigned short blocks)
{
	pccb->cmd[0] = SCSI_READ10;
#if defined(CONFIG_TCC_UFS) || defined(CONFIG_TCC_SC_UFS)
	pccb->cmd[1] = 0;
#else
	pccb->cmd[1] = pccb->lun << 5;
#endif
	pccb->cmd[2] = (unsigned char)(start >> 24) & 0xff;
	pccb->cmd[3] = (unsigned char)(start >> 16) & 0xff;
	pccb->cmd[4] = (unsigned char)(start >> 8) & 0xff;
	pccb->cmd[5] = (unsigned char)start & 0xff;
	pccb->cmd[6] = 0;
	pccb->cmd[7] = (unsigned char)(blocks >> 8) & 0xff;
	pccb->cmd[8] = (unsigned char)blocks & 0xff;
	pccb->cmd[6] = 0;
	pccb->cmdlen = 10;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
	debug("scsi_setup_read_ext: cmd: %02X %02X startblk %02X%02X%02X%02X blccnt %02X%02X\n",
	      pccb->cmd[0], pccb->cmd[1],
	      pccb->cmd[2], pccb->cmd[3], pccb->cmd[4], pccb->cmd[5],
	      pccb->cmd[7], pccb->cmd[8]);
}

static void scsi_setup_write_ext(struct scsi_cmd *pccb, lbaint_t start,
				 unsigned short blocks)
{
	pccb->cmd[0] = SCSI_WRITE10;
#if defined(CONFIG_TCC_UFS) || defined(CONFIG_TCC_SC_UFS)
	pccb->cmd[1] = 0;
#else
	pccb->cmd[1] = pccb->lun << 5;
#endif
	pccb->cmd[2] = (unsigned char)(start >> 24) & 0xff;
	pccb->cmd[3] = (unsigned char)(start >> 16) & 0xff;
	pccb->cmd[4] = (unsigned char)(start >> 8) & 0xff;
	pccb->cmd[5] = (unsigned char)start & 0xff;
	pccb->cmd[6] = 0;
	pccb->cmd[7] = ((unsigned char)(blocks >> 8)) & 0xff;
	pccb->cmd[8] = (unsigned char)blocks & 0xff;
	pccb->cmd[9] = 0;
	pccb->cmdlen = 10;
	pccb->msgout[0] = SCSI_IDENTIFY;  /* NOT USED */
	debug("%s: cmd: %02X %02X startblk %02X%02X%02X%02X blccnt %02X%02X\n",
	      __func__,
	      pccb->cmd[0], pccb->cmd[1],
	      pccb->cmd[2], pccb->cmd[3], pccb->cmd[4], pccb->cmd[5],
	      pccb->cmd[7], pccb->cmd[8]);
}

static ulong scsi_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
		       void *buffer)
{
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
	struct udevice *bdev = dev->parent;
	struct scsi_platdata *uc_plat = dev_get_uclass_platdata(bdev);
	lbaint_t start, blks, max_blks;
	uintptr_t buf_addr;
	unsigned short smallblks = 0;
	struct scsi_cmd *pccb = (struct scsi_cmd *)&tempccb;

	/* Setup device */
	pccb->target = block_dev->target;
	pccb->lun = block_dev->lun;
	buf_addr = (unsigned long)buffer;
	start = blknr;
	blks = blkcnt;
	if (uc_plat->max_bytes_per_req)
		max_blks = uc_plat->max_bytes_per_req / block_dev->blksz;
	else
		max_blks = SCSI_MAX_BLK;

	debug("\nscsi_read: dev %d startblk " LBAF
	      ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, (unsigned long)buffer);
	do {
		pccb->pdata = (unsigned char *)buf_addr;
		pccb->dma_dir = DMA_FROM_DEVICE;
#ifdef CONFIG_SYS_64BIT_LBA
		if (start > SCSI_LBA48_READ) {
			unsigned long blocks;
			blocks = min_t(lbaint_t, blks, max_blks);
			pccb->datalen = block_dev->blksz * blocks;
			scsi_setup_read16(pccb, start, blocks);
			start += blocks;
			blks -= blocks;
		} else
#endif
		if (blks > max_blks) {
			pccb->datalen = block_dev->blksz * max_blks;
			smallblks = max_blks;
			scsi_setup_read_ext(pccb, start, smallblks);
			start += max_blks;
			blks -= max_blks;
		} else {
			pccb->datalen = block_dev->blksz * blks;
			smallblks = (unsigned short)blks;
			scsi_setup_read_ext(pccb, start, smallblks);
			start += blks;
			blks = 0;
		}
		debug("scsi_read_ext: startblk " LBAF
		      ", blccnt %x buffer %lX\n",
		      start, smallblks, buf_addr);
		if (scsi_exec(bdev, pccb)) {
			scsi_print_error(pccb);
			blkcnt -= blks;
			break;
		}
		buf_addr += pccb->datalen;
	} while (blks != 0);
	debug("scsi_read_ext: end startblk " LBAF
	      ", blccnt %x buffer %lX\n", start, smallblks, buf_addr);
	return blkcnt;
}

/*******************************************************************************
 * scsi_write
 */

static ulong scsi_write(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
			const void *buffer)
{
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
	struct udevice *bdev = dev->parent;
	struct scsi_platdata *uc_plat = dev_get_uclass_platdata(bdev);
	lbaint_t start, blks, max_blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	struct scsi_cmd *pccb = (struct scsi_cmd *)&tempccb;

	/* Setup device */
	pccb->target = block_dev->target;
	pccb->lun = block_dev->lun;
	buf_addr = (unsigned long)buffer;
	start = blknr;
	blks = blkcnt;
	if (uc_plat->max_bytes_per_req)
		max_blks = uc_plat->max_bytes_per_req / block_dev->blksz;
	else
		max_blks = SCSI_MAX_BLK;

	debug("\n%s: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      __func__, block_dev->devnum, start, blks, (unsigned long)buffer);
	do {
		pccb->pdata = (unsigned char *)buf_addr;
		pccb->dma_dir = DMA_TO_DEVICE;
		if (blks > max_blks) {
			pccb->datalen = block_dev->blksz * max_blks;
			smallblks = max_blks;
			scsi_setup_write_ext(pccb, start, smallblks);
			start += max_blks;
			blks -= max_blks;
		} else {
			pccb->datalen = block_dev->blksz * blks;
			smallblks = (unsigned short)blks;
			scsi_setup_write_ext(pccb, start, smallblks);
			start += blks;
			blks = 0;
		}
		debug("%s: startblk " LBAF ", blccnt %x buffer %lx\n",
		      __func__, start, smallblks, buf_addr);
		if (scsi_exec(bdev, pccb)) {
			scsi_print_error(pccb);
			blkcnt -= blks;
			break;
		}
		buf_addr += pccb->datalen;
	} while (blks != 0);
	debug("%s: end startblk " LBAF ", blccnt %x buffer %lX\n",
	      __func__, start, smallblks, buf_addr);
	return blkcnt;
}

/*******************************************************************************
 * scsi_erase
 */

static ulong scsi_erase(struct udevice *dev, lbaint_t start, lbaint_t blkcnt)
{
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
	struct udevice *bdev = dev->parent;
	struct scsi_unmap_parameter *unmap_parameter;
	uintptr_t buf_addr;
	struct scsi_cmd *pccb = (struct scsi_cmd *)&tempccb;
	int ret;
#if DEBUG
	char *tmp;
#endif

	pccb->target = block_dev->target;
	pccb->lun = block_dev->lun;
	buf_addr = (unsigned long)memalign(4096,
				sizeof(struct scsi_unmap_parameter));

	memset((void *)buf_addr, 0, sizeof(struct scsi_unmap_parameter));
	memset(pccb->cmd, 0, sizeof(pccb->cmd));
	unmap_parameter = (struct scsi_unmap_parameter *)buf_addr;

	pccb->cmd[0] = SCSI_UNMAP;
	pccb->cmd[8] = sizeof(struct scsi_unmap_parameter);
	pccb->cmdlen = 10;

	unmap_parameter->data_length =
		cpu_to_be16(sizeof(struct scsi_unmap_parameter) - 0x2);
	unmap_parameter->desc_data_length =
		cpu_to_be16(sizeof(struct scsi_unmap_parameter) - 0x8);
	unmap_parameter->blk_desc.lba =
		cpu_to_be64(start);
	unmap_parameter->blk_desc.cnt =
		cpu_to_be32(blkcnt);

#if DEBUG
	tmp = (char *)buf_addr;

	pr_info("%s: pccb->cmd = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			__func__, pccb->cmd[0], pccb->cmd[1],
			pccb->cmd[2], pccb->cmd[3], pccb->cmd[4],
			pccb->cmd[5], pccb->cmd[6], pccb->cmd[7],
			pccb->cmd[8], pccb->cmd[9]);

	pr_info("%s: buf = %02x %02x %02x %02x\n", __func__,
		tmp[0], tmp[1], tmp[2], tmp[3]);
	pr_info("%s: buf = %02x %02x %02x %02x\n", __func__,
		tmp[4], tmp[5], tmp[6], tmp[7]);
	pr_info("%s: buf = %02x %02x %02x %02x\n", __func__,
		tmp[8], tmp[9], tmp[10], tmp[11]);
	pr_info("%s: buf = %02x %02x %02x %02x\n", __func__,
		tmp[12], tmp[13], tmp[14], tmp[15]);
	pr_info("%s: buf = %02x %02x %02x %02x\n", __func__,
		tmp[16], tmp[17], tmp[18], tmp[19]);
	pr_info("%s: buf = %02x %02x %02x %02x\n", __func__,
		tmp[20], tmp[21], tmp[22], tmp[23]);
#endif

	pccb->pdata = (unsigned char *)buf_addr;
	pccb->dma_dir = DMA_TO_DEVICE;

	ret = scsi_exec(bdev, pccb);
	if (ret) {
		scsi_print_error(pccb);
		ret = -EIO;
	} else {
		ret = blkcnt;
	}

	free((void *)buf_addr);

	return ret;
}

#endif

#if defined(CONFIG_PCI) && !defined(CONFIG_SCSI_AHCI_PLAT) && \
	!defined(CONFIG_DM_SCSI)
void scsi_init(void)
{
	int busdevfunc = -1;
	int i;
	/*
	 * Find a device from the list, this driver will support a single
	 * controller.
	 */
	for (i = 0; i < ARRAY_SIZE(scsi_device_list); i++) {
		/* get PCI Device ID */
#ifdef CONFIG_DM_PCI
		struct udevice *dev;
		int ret;

		ret = dm_pci_find_device(scsi_device_list[i].vendor,
					 scsi_device_list[i].device, 0, &dev);
		if (!ret) {
			busdevfunc = dm_pci_get_bdf(dev);
			break;
		}
#else
		busdevfunc = pci_find_device(scsi_device_list[i].vendor,
					     scsi_device_list[i].device,
					     0);
#endif
		if (busdevfunc != -1)
			break;
	}

	if (busdevfunc == -1) {
		printf("Error: SCSI Controller(s) ");
		for (i = 0; i < ARRAY_SIZE(scsi_device_list); i++) {
			printf("%04X:%04X ",
			       scsi_device_list[i].vendor,
			       scsi_device_list[i].device);
		}
		printf("not found\n");
		return;
	}
#ifdef DEBUG
	else {
		printf("SCSI Controller (%04X,%04X) found (%d:%d:%d)\n",
		       scsi_device_list[i].vendor,
		       scsi_device_list[i].device,
		       (busdevfunc >> 16) & 0xFF,
		       (busdevfunc >> 11) & 0x1F,
		       (busdevfunc >> 8) & 0x7);
	}
#endif
	bootstage_start(BOOTSTAGE_ID_ACCUM_SCSI, "ahci");
	scsi_low_level_init(busdevfunc);
	scsi_scan(true);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_SCSI);
}
#endif

/* copy src to dest, skipping leading and trailing blanks
 * and null terminate the string
 */
static void scsi_ident_cpy(unsigned char *dest, unsigned char *src,
			   unsigned int len)
{
	int start, end;

	start = 0;
	while (start < len) {
		if (src[start] != ' ')
			break;
		start++;
	}
	end = len-1;
	while (end > start) {
		if (src[end] != ' ')
			break;
		end--;
	}
	for (; start <= end; start++)
		*dest ++= src[start];
	*dest = '\0';
}

static int scsi_read_capacity(struct udevice *dev, struct scsi_cmd *pccb,
			      lbaint_t *capacity, unsigned long *blksz)
{
	*capacity = 0;

	memset(pccb->cmd, '\0', sizeof(pccb->cmd));
	pccb->cmd[0] = SCSI_RD_CAPAC10;
#if defined(CONFIG_TCC_UFS) || defined(CONFIG_TCC_SC_UFS)
	pccb->cmd[1] = 0;
#else
	pccb->cmd[1] = pccb->lun << 5;
#endif
	pccb->cmdlen = 10;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */

	pccb->datalen = 8;
	if (scsi_exec(dev, pccb))
		return 1;

	*capacity = ((lbaint_t)pccb->pdata[0] << 24) |
		    ((lbaint_t)pccb->pdata[1] << 16) |
		    ((lbaint_t)pccb->pdata[2] << 8)  |
		    ((lbaint_t)pccb->pdata[3]);

	if (*capacity != 0xffffffff) {
		/* Read capacity (10) was sufficient for this drive. */
		*blksz = ((unsigned long)pccb->pdata[4] << 24) |
			 ((unsigned long)pccb->pdata[5] << 16) |
			 ((unsigned long)pccb->pdata[6] << 8)  |
			 ((unsigned long)pccb->pdata[7]);
		return 0;
	}

	/* Read capacity (10) was insufficient. Use read capacity (16). */
	memset(pccb->cmd, '\0', sizeof(pccb->cmd));
	pccb->cmd[0] = SCSI_RD_CAPAC16;
	pccb->cmd[1] = 0x10;
	pccb->cmdlen = 16;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */

	pccb->datalen = 16;
	pccb->dma_dir = DMA_FROM_DEVICE;
	if (scsi_exec(dev, pccb))
		return 1;

	*capacity = ((uint64_t)pccb->pdata[0] << 56) |
		    ((uint64_t)pccb->pdata[1] << 48) |
		    ((uint64_t)pccb->pdata[2] << 40) |
		    ((uint64_t)pccb->pdata[3] << 32) |
		    ((uint64_t)pccb->pdata[4] << 24) |
		    ((uint64_t)pccb->pdata[5] << 16) |
		    ((uint64_t)pccb->pdata[6] << 8)  |
		    ((uint64_t)pccb->pdata[7]);

	*blksz = ((uint64_t)pccb->pdata[8]  << 56) |
		 ((uint64_t)pccb->pdata[9]  << 48) |
		 ((uint64_t)pccb->pdata[10] << 40) |
		 ((uint64_t)pccb->pdata[11] << 32) |
		 ((uint64_t)pccb->pdata[12] << 24) |
		 ((uint64_t)pccb->pdata[13] << 16) |
		 ((uint64_t)pccb->pdata[14] << 8)  |
		 ((uint64_t)pccb->pdata[15]);

	return 0;
}

#if defined(CONFIG_SUPPORT_UFS_RPMB)
static int scsi_read_capacity_for_ufs_rpmb(struct udevice *dev,
		lbaint_t *capacity, unsigned long *blksz)
{
	struct ufs_query_sc q = {0, };
	*capacity = 0;

	memset(tempbuff, 0, 512);
	ufs_fill_rpmb_unit_desc(dev, &q, (uint32_t *)tempbuff);
	if (scsi_ufs_query(dev, &q))
		return 1;
	*capacity = (((lbaint_t)tempbuff[0xF] << 24) |
		    ((lbaint_t)tempbuff[0x10] << 16) |
		    ((lbaint_t)tempbuff[0x11] << 8)  |
		    ((lbaint_t)tempbuff[0x12])) - 1;
	*blksz = (unsigned long)(1 << tempbuff[0xA]);

	return 0;
}
#else
static int scsi_read_capacity_for_ufs_rpmb(struct udevice *dev,
		lbaint_t *capacity, unsigned long *blksz)
{
	return 0;
}
#endif

/*
 * Some setup (fill-in) routines
 */
static void scsi_setup_test_unit_ready(struct scsi_cmd *pccb)
{
	pccb->cmd[0] = SCSI_TST_U_RDY;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmd[2] = 0;
	pccb->cmd[3] = 0;
	pccb->cmd[4] = 0;
	pccb->cmd[5] = 0;
	pccb->cmdlen = 6;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
}

/**
 * scsi_init_dev_desc_priv - initialize only SCSI specific blk_desc properties
 *
 * @dev_desc: Block device description pointer
 */
static void scsi_init_dev_desc_priv(struct blk_desc *dev_desc)
{
	dev_desc->target = 0xff;
	dev_desc->lun = 0xff;
	dev_desc->log2blksz =
		LOG2_INVALID(typeof(dev_desc->log2blksz));
	dev_desc->type = DEV_TYPE_UNKNOWN;
	dev_desc->vendor[0] = 0;
	dev_desc->product[0] = 0;
	dev_desc->revision[0] = 0;
	dev_desc->removable = false;
}

#if !defined(CONFIG_DM_SCSI)
/**
 * scsi_init_dev_desc - initialize all SCSI specific blk_desc properties
 *
 * @dev_desc: Block device description pointer
 * @devnum: Device number
 */
static void scsi_init_dev_desc(struct blk_desc *dev_desc, int devnum)
{
	dev_desc->lba = 0;
	dev_desc->blksz = 0;
	dev_desc->if_type = IF_TYPE_SCSI;
	dev_desc->devnum = devnum;
	dev_desc->part_type = PART_TYPE_UNKNOWN;

	scsi_init_dev_desc_priv(dev_desc);
}
#endif

/**
 * scsi_detect_dev - Detect scsi device
 *
 * @target: target id
 * @lun: target lun
 * @dev_desc: block device description
 *
 * The scsi_detect_dev detects and fills a dev_desc structure when the device is
 * detected.
 *
 * Return: 0 on success, error value otherwise
 */
static int scsi_detect_dev(struct udevice *dev, int target, int lun,
			   struct blk_desc *dev_desc)
{
	unsigned char perq, modi;
	lbaint_t capacity;
	unsigned long blksz = 0;
	struct scsi_cmd *pccb = (struct scsi_cmd *)&tempccb;
	int count, err;

#if defined(CONFIG_TCC_UFS) || defined(CONFIG_TCC_SC_UFS)
	dev_info(dev, "%s - Req Sense\n", __func__);
	pccb->target = target;
	pccb->lun = lun;
	pccb->pdata = (unsigned char *)&tempbuff;
	pccb->datalen = 18;
	pccb->dma_dir = DMA_FROM_DEVICE;
	scsi_setup_req_sense(pccb);
	if (scsi_exec(dev, pccb)) {
		if (pccb->contr_stat == SCSI_SEL_TIME_OUT) {
			debug("Selection timeout ID %d\n",
			      pccb->target);
			return -ETIMEDOUT;
		}
		scsi_print_error(pccb);
		return -ENODEV;
	}
#endif

	pccb->target = target;
	pccb->lun = lun;
	pccb->pdata = (unsigned char *)&tempbuff;

#if defined(CONFIG_TCC_UFS) || defined(CONFIG_TCC_SC_UFS)
	pccb->datalen = 36;
#else
	pccb->datalen = 512;
#endif
	pccb->dma_dir = DMA_FROM_DEVICE;
	scsi_setup_inquiry(pccb);
	if (scsi_exec(dev, pccb)) {
		if (pccb->contr_stat == SCSI_SEL_TIME_OUT) {
			/*
			  * selection timeout => assuming no
			  * device present
			  */
			debug("Selection timeout ID %d\n",
			      pccb->target);
			return -ETIMEDOUT;
		}
		scsi_print_error(pccb);
		return -ENODEV;
	}
	perq = tempbuff[0];
	modi = tempbuff[1];
	if ((perq & 0x1f) == 0x1f)
		return -ENODEV; /* skip unknown devices */
	if ((modi & 0x80) == 0x80) /* drive is removable */
		dev_desc->removable = true;
	/* get info for this device */
	scsi_ident_cpy((unsigned char *)dev_desc->vendor,
		       &tempbuff[8], 8);
	scsi_ident_cpy((unsigned char *)dev_desc->product,
		       &tempbuff[16], 16);
	scsi_ident_cpy((unsigned char *)dev_desc->revision,
		       &tempbuff[32], 4);
	dev_desc->target = pccb->target;
	dev_desc->lun = pccb->lun;

	for (count = 0; count < 3; count++) {
		pr_info("%s - Test Unit Ready\n", __func__);
		pccb->datalen = 0;
		scsi_setup_test_unit_ready(pccb);
		err = scsi_exec(dev, pccb);
		if (!err)
			break;
	}
	if (err) {
		if (dev_desc->removable) {
			dev_desc->type = perq;
			goto removable;
		}
		scsi_print_error(pccb);
		return -EINVAL;
	}
	if (lun != 0xC4 /*Except for RPMB*/&&
		scsi_read_capacity(dev, pccb, &capacity, &blksz)) {
		scsi_print_error(pccb);
		return -EINVAL;
	} else if (lun == 0xC4) {
		scsi_read_capacity_for_ufs_rpmb(dev, &capacity, &blksz);
	}
	dev_desc->lba = capacity;
	dev_desc->blksz = blksz;
	dev_desc->log2blksz = LOG2(dev_desc->blksz);
	dev_desc->type = perq;
removable:
	return 0;
}

/*
 * (re)-scan the scsi bus and reports scsi device info
 * to the user if mode = 1
 */
#if defined(CONFIG_DM_SCSI)
static int scsi_ufs_refresh_op(struct udevice *dev, uint32_t Req_type,
		uint32_t index, uint32_t direction, uint32_t value)
{
	int *buf_addr;
	struct scsi_cmd *pccb = (struct scsi_cmd *)&tempccb;
	int ret = 0;
	int val;

	val = value;
	buf_addr = (int *)memalign(4096, 256);
	memset(buf_addr, 0, 256);
	memset(pccb, 0, sizeof(struct scsi_cmd));

	/* Setup device */
	pccb->priv = 0xf;
	pccb->datalen = Req_type; //[0] = type
	pccb->contr_stat = index; //[2] = index
	pccb->lun = (unsigned int)buf_addr; //[3] = data buffer
	pccb->trans_bytes = direction; //[4] = direction (in/out)

	if (direction == 1) {
		memcpy(buf_addr, &val, sizeof(uint32_t));
	}

	flush_cache((unsigned long)buf_addr,
			(unsigned long)256);

	if (scsi_exec(dev, pccb)) {
		ret = -ENODEV;
	} else {
		memcpy(&ret, buf_addr, sizeof(uint32_t));
	}

	free((void *)buf_addr);
	memset(pccb, 0, sizeof(struct scsi_cmd));
	return ret;
}

static int do_scsi_scan_one(struct udevice *dev, int id, int lun, bool verbose)
{
	int ret;
	struct udevice *bdev;
	struct blk_desc bd;
	struct blk_desc *bdesc;
	char str[10];

	/*
	 * detect the scsi driver to get information about its geometry (block
	 * size, number of blocks) and other parameters (ids, type, ...)
	 */
	scsi_init_dev_desc_priv(&bd);
	if (scsi_detect_dev(dev, id, lun, &bd))
		return -ENODEV;

	/*
	* Create only one block device and do detection
	* to make sure that there won't be a lot of
	* block devices created
	*/
	snprintf(str, sizeof(str), "id%dlun%d", id, lun);
	ret = blk_create_devicef(dev, "scsi_blk", str, IF_TYPE_SCSI, -1,
			bd.blksz, bd.lba, &bdev);
	if (ret) {
		debug("Can't create device\n");
		return ret;
	}

	bdesc = dev_get_uclass_platdata(bdev);
	bdesc->target = id;
	bdesc->lun = lun;
	bdesc->removable = bd.removable;
	bdesc->type = bd.type;
	memcpy(&bdesc->vendor, &bd.vendor, sizeof(bd.vendor));
	memcpy(&bdesc->product, &bd.product, sizeof(bd.product));
	memcpy(&bdesc->revision, &bd.revision,	sizeof(bd.revision));

	if (verbose) {
		printf("  Device %d: ", bdesc->devnum);
		dev_print(bdesc);
	}
	return 0;
}

int scsi_scan_dev(struct udevice *dev, bool verbose)
{
	struct scsi_platdata *uc_plat; /* scsi controller platdata */
	int ret;
	int i;
	int lun;

	/* probe SCSI controller driver */
	ret = device_probe(dev);
	if (ret)
		return ret;

	/* Get controller platdata */
	uc_plat = dev_get_uclass_platdata(dev);

	for (i = 0; i < uc_plat->max_id; i++)
		for (lun = 0; lun < uc_plat->max_lun; lun++)
			do_scsi_scan_one(dev, i, lun, verbose);

#if defined(CONFIG_SUPPORT_UFS_RPMB)
	/* For RPMB wLUN, LUN = 0xC4 (UFS's RPMB ID) */
	do_scsi_scan_one(dev, 0, 0xC4, verbose);
#endif

	return 0;
}

int scsi_refresh()
{
	struct uclass *uc;
	struct udevice *dev; /* SCSI controller */
	int ret;

	ret = uclass_get(UCLASS_SCSI, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		if (!env_get("ufs_refresh")) {
#ifdef CONFIG_TCC_MAINCORE
			printf("UFS Needs to REFRESH!\n");
			ret = scsi_ufs_refresh_op(
				dev, 0x1, 0x2F, 0x1, 0x1); //bRefreshMethod = 0x1;
			ret = scsi_ufs_refresh_op(
				dev, 0x1, 0x2E, 0x1, 0x1); //bRefreshUnit = 0x1;
			ret = scsi_ufs_refresh_op(
				dev, 0x2, 0x7, 0x1, 0x1); //fRefreshEnable = 0x1;
			ret = scsi_ufs_refresh_op(
				dev, 0x1, 0x2C, 0x0, 0);
			printf("Waiting to REFRESH ");
			while(ret == 0x1) { //Get bRefreshStatus
				printf(".", ret);
				mdelay(1000);
				ret = scsi_ufs_refresh_op(
					dev, 0x1, 0x2C, 0x0, 0);
			}
			printf("\nREFRESH Complete!\n");
			ret = env_set("ufs_refresh", "1");
			ret = env_save();
#else
			printf("Wait for maincore's Refresh OP\n");
			mdelay(2000);
			ret = scsi_ufs_refresh_op(
				dev, 0x1, 0x2C, 0x0, 0);
			while (ret == 0x1) {
				printf(".", ret);
				mdelay(1000);
				ret = scsi_ufs_refresh_op(
					dev, 0x1, 0x2C, 0x0, 0);
			}
			ret = env_set("ufs_refresh", "1");
			ret = env_save();
#endif
		}
	}

	return ret;
}

int scsi_scan(bool verbose)
{
	struct uclass *uc;
	struct udevice *dev; /* SCSI controller */
	int ret;

	if (verbose)
		printf("scanning bus for devices...\n");

	blk_unbind_all(IF_TYPE_SCSI);

	ret = uclass_get(UCLASS_SCSI, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		ret = scsi_scan_dev(dev, verbose);
		if (ret)
			return ret;
	}

	return 0;
}
#else
int scsi_scan(bool verbose)
{
	unsigned char i, lun;
	int ret;

	if (verbose)
		printf("scanning bus for devices...\n");
	for (i = 0; i < CONFIG_SYS_SCSI_MAX_DEVICE; i++)
		scsi_init_dev_desc(&scsi_dev_desc[i], i);

	scsi_max_devs = 0;
	for (i = 0; i < CONFIG_SYS_SCSI_MAX_SCSI_ID; i++) {
		for (lun = 0; lun < CONFIG_SYS_SCSI_MAX_LUN; lun++) {
			struct blk_desc *bdesc = &scsi_dev_desc[scsi_max_devs];

			ret = scsi_detect_dev(NULL, i, lun, bdesc);
			if (ret)
				continue;
			part_init(bdesc);

			if (verbose) {
				printf("  Device %d: ", bdesc->devnum);
				dev_print(bdesc);
			}
			scsi_max_devs++;
		} /* next LUN */
	}
	if (scsi_max_devs > 0)
		scsi_curr_dev = 0;
	else
		scsi_curr_dev = -1;

	printf("Found %d device(s).\n", scsi_max_devs);
#ifndef CONFIG_SPL_BUILD
	env_set_ulong("scsidevs", scsi_max_devs);
#endif
	return 0;
}
#endif

#ifdef CONFIG_BLK
static const struct blk_ops scsi_blk_ops = {
	.read	= scsi_read,
	.write	= scsi_write,
	.erase	= scsi_erase,
};

U_BOOT_DRIVER(scsi_blk) = {
	.name		= "scsi_blk",
	.id		= UCLASS_BLK,
	.ops		= &scsi_blk_ops,
};
#else
U_BOOT_LEGACY_BLK(scsi) = {
	.if_typename	= "scsi",
	.if_type	= IF_TYPE_SCSI,
	.max_devs	= CONFIG_SYS_SCSI_MAX_DEVICE,
	.desc		= scsi_dev_desc,
};
#endif
