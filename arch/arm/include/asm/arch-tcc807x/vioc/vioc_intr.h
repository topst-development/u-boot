/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef VIOC_DISP_INTR__H
#define VIOC_DISP_INTR__H

struct vioc_intr_type {
	int id;
	unsigned int bits;
};

enum {
	VIOC_INTR_DEV0 = 0,
	VIOC_INTR_DEV1 = 1,
	VIOC_INTR_DEV2 = 2,
	VIOC_INTR_DEV3 = 3,
	VIOC_INTR_DEV4 = 4,
	VIOC_INTR_RESERVED_5,
	VIOC_INTR_RESERVED_6,
	VIOC_INTR_RESERVED_7,
	VIOC_INTR_RD0 = 8,
	VIOC_INTR_RD1 = 9,
	VIOC_INTR_RD2 = 10,
	VIOC_INTR_RD3 = 11,
	VIOC_INTR_RD4 = 12,
	VIOC_INTR_RD5 = 13,
	VIOC_INTR_RD6 = 14,
	VIOC_INTR_RD7 = 15,
	VIOC_INTR_RD8 = 16,
	VIOC_INTR_RD9 = 17,
	VIOC_INTR_RD10 = 18,
	VIOC_INTR_RD11 = 19,
	VIOC_INTR_RD12 = 20,
	VIOC_INTR_RD13 = 21,
	VIOC_INTR_RD14 = 22,
	VIOC_INTR_RD15 = 23,
	VIOC_INTR_MC0 = 24,
	VIOC_INTR_RESERVED_25,
	VIOC_INTR_RESERVED_26,
	VIOC_INTR_RESERVED_27,
	VIOC_INTR_FIFO0 = 28,
	VIOC_INTR_FIFO1 = 29,
	VIOC_INTR_RESERVED_30,
	VIOC_INTR_RESERVED_31,
	VIOC_INTR_WD0 = 32,
	VIOC_INTR_WD1 = 33,
	VIOC_INTR_WD2 = 34,
	VIOC_INTR_WD3 = 35,
	VIOC_INTR_WD4 = 36,
	VIOC_INTR_WD5 = 37,
	VIOC_INTR_RESERVED_38,
	VIOC_INTR_RESERVED_39,
	VIOC_INTR_TIMER = 40,
	VIOC_INTR_RESERVED_41,
	VIOC_INTR_RESERVED_42,
	VIOC_INTR_RESERVED_43,
	VIOC_INTR_RESERVED_44,
	VIOC_INTR_RESERVED_45,
	VIOC_INTR_RESERVED_46,
	VIOC_INTR_RESERVED_47,
	VIOC_INTR_WMIX0 = 48,
	VIOC_INTR_WMIX1 = 49,
	VIOC_INTR_WMIX2 = 50,
	VIOC_INTR_WMIX3 = 51,
	VIOC_INTR_RESERVED_52,
	VIOC_INTR_RESERVED_53,
	VIOC_INTR_RESERVED_54,
	VIOC_INTR_RESERVED_55,
	VIOC_INTR_RESERVED_56,
	VIOC_INTR_RESERVED_57,
	VIOC_INTR_RESERVED_58,
	VIOC_INTR_VIQE0 = 59,
	VIOC_INTR_SC0 = 60,
	VIOC_INTR_SC1 = 61,
	VIOC_INTR_SC2 = 62,
	VIOC_INTR_SC3 = 63,
	VIOC_INTR_AFBCDEC0 = 64,
	VIOC_INTR_AFBCDEC1 = 65,
	VIOC_INTR_ADAE0 = 66,
	VIOC_INTR_ADAE1 = 67,
	VIOC_INTR_ADCS0 = 68,
	VIOC_INTR_ADCS1 = 69,
	VIOC_INTR_ADE0 = 70,
	VIOC_INTR_ADE1 = 71,
	VIOC_INTR_ADDE0 = 72,
	VIOC_INTR_ADDE1 = 73,
	VIOC_INTR_ADSIE0 = 74,
	VIOC_INTR_ADSIE1 = 75,
	VIOC_INTR_NUM = VIOC_INTR_ADSIE1
};

/* VIOC DEV0/1/2 irqs */
enum vioc_disp_intr_src {
	VIOC_DISP_INTR_FU = 0, /* LCD output fifo under-run */
	VIOC_DISP_INTR_VSR,    /* VS Rising */
	VIOC_DISP_INTR_VSF,    /* VS Falling */
	VIOC_DISP_INTR_RU,     /* Register Update */
	VIOC_DISP_INTR_DD,     /* Disable Done */
	VIOC_DISP_INTR_SREQ,   /* Device Stop Request */
	VIOC_DISP_INTR_MAX
};
#define VIOC_INTR_DISP_OFFSET (VIOC_INTR_DEV4 - (VIOC_INTR_DEV3 + 1))
#define VIOC_DISP_INTR_DISPLAY                                \
	(/*((u32)1U<<VIOC_DISP_INTR_FU)|*/ ((u32)1U << VIOC_DISP_INTR_RU) \
	 | ((u32)1U << VIOC_DISP_INTR_DD))
#define VIOC_DISP_INT_MASK (((u32)1U << VIOC_DISP_INTR_MAX) - 1U)

/* VIOC RDMA irqs */
enum vioc_rdma_intr_src {
	VIOC_RDMA_INTR_CFG = 0, /* Configurate Update */
	VIOC_RDMA_INTR_EOFR,    /* EOF Rising*/
	VIOC_RDMA_INTR_EOFF,    /* EOF Falling */
	VIOC_RDMA_INTR_UPDD,    /* Update Done */
	VIOC_RDMA_INTR_EOFW,    /* EOF-WAIT Rising */
	VIOC_RDMA_INTR_TOPR,    /* TOP Ready */
	VIOC_RDMA_INTR_BOTR,    /* Bottom Ready */
	VIOC_RDMA_INTR_MAX
};
#define VIOC_RDMA_INT_MASK (((u32)1U << VIOC_RDMA_INTR_MAX) - 1U)

/* VIOC WDMA irqs */
enum vioc_wdma_intr_src {
	VIOC_WDMA_INTR_UPD = 0, /* Register Update */
	VIOC_WDMA_INTR_SREQ,    /* VIOC_WDMA_INTR_EOFF, */
	VIOC_WDMA_INTR_ROL,     /* Rolling */
	VIOC_WDMA_INTR_ENR,     /* Synchronized Enable Rising */
	VIOC_WDMA_INTR_ENF,     /* Synchronized Enable Falling */
	VIOC_WDMA_INTR_EOFR,    /* EOF Rising */
	VIOC_WDMA_INTR_EOFF,    /* EOF Falling */
	VIOC_WDMA_INTR_SEOFR,   /* Sync EOF Rising */
	VIOC_WDMA_INTR_SEOFF,   /* Sync EOF Falling */
	VIOC_WDMA_INTR_MAX
};
#define VIOC_WDMA_INT_MASK (((u32)1U << VIOC_WDMA_INTR_MAX) - 1U)

#define VIOC_INTR_WD_OFFSET (VIOC_INTR_WD5 - (VIOC_INTR_WD4 + 1))

#if 0
/* VIOC VIN irqs */
enum vioc_vin_intr_src {
	VIOC_VIN_INTR_UPD = 0,
	VIOC_VIN_INTR_EOF,
	VIOC_VIN_INTR_VS,
	VIOC_VIN_INTR_INVS,
	VIOC_VIN_INTR_MAX
};
#define VIOC_VIN_INT_MASK ((1U << VIOC_VIN_INTR_MAX) - 1U)

#define VIOC_INTR_VIN_OFFSET (VIOC_INTR_VIN4 - (VIOC_INTR_VIN3 + 1))
#endif

#define INT_VIOC0       (139U + INT_GIC_OFFSET)
#define INT_VIOC3       (142U + INT_GIC_OFFSET)

/* VIOC SC irqs */
#define VIOC_SC_INT_MASK 0xF

/* VIOC WMIX irqs */
#define VIOC_WMIX_INT_MASK 0x1F
int tcc_vioc_irq_install_handler (
	long vioc_irq, int id, void *handle_irq, void *data);
void tcc_vioc_irq_free_handler (int vioc_irq, unsigned int id);
int vioc_intr_enable(int vioc_irq, int id, unsigned int mask);
int vioc_intr_disable(int vioc_irq, int id, unsigned int mask);
unsigned int vioc_intr_get_status(int id);
bool check_vioc_irq_status(void __iomem *reg, int id);
bool is_vioc_intr_activatied(int id, unsigned int mask);
int vioc_intr_clear(int id, unsigned int mask);
void vioc_intr_initialize(void);
bool is_vioc_intr_unmasked(int id, unsigned int mask);

#endif /* __VIOC_DISP_INTR__H__ */
