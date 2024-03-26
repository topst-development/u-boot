/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef _UFS_H
#define _UFS_H

struct udevice;

struct query_desc {
    uint8_t     opcode;
    uint8_t     idn;
    uint8_t     index;
    uint8_t     selector;
    uint8_t     reserved0[2];
    uint16_t    length;
    uint32_t    reserved2[2];
};     /* 16 bytes with big endian */

struct query_flag {
    uint8_t     opcode;
    uint8_t     idn;
    uint8_t     index;
    uint8_t     selector;
    uint8_t     reserved0[7];
    uint8_t     value;
    uint32_t    reserved8;
};     /* 16 bytes with big endian */

struct query_attr {
    uint8_t     opcode;
    uint8_t     idn;
    uint8_t     index;
    uint8_t     selector;
    uint8_t     reserved0[4];
    uint32_t    value;  /* little endian */
    uint32_t    reserved4;
};     /* 16 bytes with big endian except for value */

struct ufs_query_sc {
	uint32_t datsz;
	uint32_t *legacy_buf;
	union {
		struct query_desc desc;
		struct query_flag flag;
		struct query_attr attr;
	} ts;
};

struct ufs_desc_info {
	char product[16];
	u8 bRPMB_ReadWriteSize;
	u8 bRPMBRegion0Size;
};

/**
 * ufs_probe() - initialize all devices in the UFS uclass
 *
 * @return 0 if Ok, -ve on error
 */
int ufs_probe(void);

/**
 * ufs_probe_dev() - initialize a particular device in the UFS uclass
 *
 * @index: index in the uclass sequence
 *
 * @return 0 if successfully probed, -ve on error
 */
int ufs_probe_dev(int index);

/*
 * ufs_scsi_bind() - Create a new scsi device as a child of the UFS device and
 *		     bind it to the ufs_scsi driver
 * @ufs_dev: UFS device
 * @scsi_devp: Pointer to scsi device
 *
 * @return 0 if Ok, -ve on error
 */
int ufs_scsi_bind(struct udevice *ufs_dev, struct udevice **scsi_devp);

#if defined(CONFIG_SUPPORT_UFS_RPMB)
struct ufs_desc_info *ufs_get_desc_info(struct udevice *dev);
int ufs_fill_rpmb_unit_desc(struct udevice *scsi_dev,
		struct ufs_query_sc *q, uint32_t *buf);

int ufs_rpmb_get_counter(struct udevice *scsi_dev, unsigned long *pcounter);
int ufs_rpmb_set_key(struct udevice *scsi_dev, void *key);
int ufs_rpmb_read(struct udevice *scsi_dev, void *addr, unsigned short blk,
		  unsigned short cnt, unsigned char *key);
int ufs_rpmb_write(struct udevice *scsi_dev, void *addr, unsigned short blk,
		  unsigned short cnt, unsigned char *key);
int ufs_rpmb_route_frames(struct udevice *scsi_dev, void *req, unsigned long reqlen,
			  void *rsp, unsigned long rsplen);

/* for RPMB info. */
#else
static inline struct ufs_desc_info *ufs_get_desc_info(struct udevice *dev)
{ return NULL; }
static inline int ufs_fill_rpmb_unit_desc(struct udevice *scsi_dev,
		struct ufs_query_sc *q, uint32_t *buf)
{ return 0; }
static inline int ufs_rpmb_get_counter(struct udevice *scsi_dev, unsigned long *pcounter)
{ return 0; }
static inline int ufs_rpmb_set_key(struct udevice *scsi_dev, void *key)
{ return 0; }

static inline int ufs_rpmb_read(struct udevice *scsi_dev, void *addr, unsigned short blk,
		  unsigned short cnt, unsigned char *key)
{ return 0; }
static inline int ufs_rpmb_write(struct udevice *scsi_dev, void *addr, unsigned short blk,
		  unsigned short cnt, unsigned char *key)
{ return 0; }
static inline int ufs_rpmb_route_frames(struct udevice *scsi_dev, void *req, unsigned long reqlen,
			  void *rsp, unsigned long rsplen)
{ return 0; }

#endif

#endif
