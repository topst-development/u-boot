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
	VIOC_INTR_TIMER = 3,
	VIOC_INTR_RD0 = 4,
	VIOC_INTR_RD1 = 5,
	VIOC_INTR_RD2 = 6,
	VIOC_INTR_RD3 = 7,
	VIOC_INTR_RD4 = 8,
	VIOC_INTR_RD5 = 9,
	VIOC_INTR_RD6 = 10,
	VIOC_INTR_RD7 = 11,
	VIOC_INTR_RD8 = 12,
	VIOC_INTR_RD9 = 13,
	VIOC_INTR_RD10 = 14,
	VIOC_INTR_RD11 = 15,
	VIOC_INTR_RD12 = 16,
	VIOC_INTR_RD13 = 17,
	VIOC_INTR_RD14 = 18,
	VIOC_INTR_RD15 = 19,
	VIOC_INTR_RD16 = 20,
	VIOC_INTR_RD17 = 21,
	VIOC_INTR_MC0 = 22,
	VIOC_INTR_MC1 = 23,
	VIOC_INTR_RESERVED_24,
	VIOC_INTR_RESERVED_25,
	VIOC_INTR_RESERVED_26,
	VIOC_INTR_RESERVED_27,
	VIOC_INTR_FIFO0 = 28,
	VIOC_INTR_FIFO1 = 29,
	VIOC_INTR_DEV3 = 30,
	VIOC_INTR_RESERVED_31,
	VIOC_INTR_WD0 = 32,
	VIOC_INTR_WD1 = 33,
	VIOC_INTR_WD2 = 34,
	VIOC_INTR_WD3 = 35,
	VIOC_INTR_WD4 = 36,
	VIOC_INTR_WD5 = 37,
	VIOC_INTR_WD6 = 38,
	VIOC_INTR_WD7 = 39,
	VIOC_INTR_WD8 = 40,
	VIOC_INTR_RESERVED_41,
	VIOC_INTR_RESERVED_42,
	VIOC_INTR_SC4 = 43,
	VIOC_INTR_SC5 = 44,
	VIOC_INTR_VIQE1 = 45,
	VIOC_INTR_RESERVED_46,
	VIOC_INTR_RESERVED_47,
	VIOC_INTR_WMIX0 = 48,
	VIOC_INTR_WMIX1 = 49,
	VIOC_INTR_WMIX2 = 50,
	VIOC_INTR_WMIX3 = 51,
	VIOC_INTR_WMIX4 = 52,
	VIOC_INTR_WMIX5 = 53,
	VIOC_INTR_WMIX6 = 54,
	VIOC_INTR_VIN0 = 55,
	VIOC_INTR_VIN1 = 56,
	VIOC_INTR_VIN2 = 57,
	VIOC_INTR_VIN3 = 58,
	VIOC_INTR_VIQE0 = 59,
	VIOC_INTR_SC0 = 60,
	VIOC_INTR_SC1 = 61,
	VIOC_INTR_SC2 = 62,
	VIOC_INTR_SC3 = 63,
	VIOC_INTR_AFBCDEC0 = 64,
	VIOC_INTR_AFBCDEC1 = 65,
	VIOC_INTR_RESERVED_66,
	VIOC_INTR_RESERVED_67,
	VIOC_INTR_RESERVED_68,
	VIOC_INTR_RESERVED_69,
	VIOC_INTR_RESERVED_70,
	VIOC_INTR_RESERVED_71,
	VIOC_INTR_WD9 = 72,
	VIOC_INTR_WD10 = 73,
	VIOC_INTR_WD11 = 74,
	VIOC_INTR_WD12 = 75,
	VIOC_INTR_VIN4 = 76,
	VIOC_INTR_VIN5 = 77,
	VIOC_INTR_VIN6 = 78,
	VIOC_INTR_VIN7 = 79,
	VIOC_INTR_SC6 = 80,
	VIOC_INTR_SC7 = 81,
	VIOC_INTR_WD13 = 82,
	VIOC_INTR_NUM = VIOC_INTR_WD13
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
#define VIOC_INTR_DISP_OFFSET (VIOC_INTR_DEV3 - (VIOC_INTR_DEV2 + 1))
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
	VIOC_WDMA_INTR_RESERVED,
	VIOC_WDMA_INTR_MAX
};
#define VIOC_WDMA_INT_MASK (((u32)1U << VIOC_WDMA_INTR_MAX) - 1U)

#define VIOC_INTR_WD_OFFSET (VIOC_INTR_WD9 - (VIOC_INTR_WD8 + 1))
#define VIOC_INTR_WD_OFFSET2 \
	(VIOC_INTR_WD13 - (VIOC_INTR_WD12 + 1)) // for WDMA13

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

#if defined(CONFIG_VIOC_DOLBY_VISION_EDR)
/* VIOC V_DV irqs */
enum vioc_v_dv_intr_src {
	FALL_HDMITX_HDE = 0,
	FALL_HDMITX_VS = 3,
	RISE_HDMITX_VDE = 17,
	VIOC_V_DV_INTR_MAX = 31
};
#define VIOC_V_DV_INTR_DISPLAY (((u32)1U << FALL_HDMITX_VS) | ((u32)1U << RISE_HDMITX_VDE))
#define VIOC_V_DV_INT_MASK (0xFFFFFFFFU)
#endif

#define INT_VIOC0       (119U + INT_GIC_OFFSET)
#define INT_VIOC3       (122U + INT_GIC_OFFSET)

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
