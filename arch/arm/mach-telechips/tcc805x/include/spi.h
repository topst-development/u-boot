/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef DEV_SPI_H
#define DEV_SPI_H
#include <spi.h>
#include <common.h>

#define BITSET_GPSB(X, MASK)\
	(writel(((readl(X)) | ((u32)(MASK))), X))
#define BITCLR_GPSB(X, MASK)\
	(writel(((readl(X)) & ~((u32)(MASK))), X))
#define BITCSET_GPSB(X, CMASK, SMASK)\
	(writel((((readl(X)) & ~((u32)(CMASK))) | ((u32)(SMASK))), X))

/*
 * GPSB Registers
 */
#define GPSB_PORT    0x00
#define GPSB_STAT    0x04
#define GPSB_INTEN   0x08
#define GPSB_MODE    0x0C
#define GPSB_CTRL    0x10
#define GPSB_EVTCTRL 0x14
#define GPSB_CCV     0x18
#define GPSB_TXBASE  0x20
#define GPSB_RXBASE  0x24
#define GPSB_PACKET  0x28
#define GPSB_DMACTR  0x2C
#define GPSB_DMASTR  0x30
#define GPSB_DMAICR  0x34

/*
 * GPSB STAT Register
 */
#define GPSB_STAT_ERR          ((u32)0x1FU << 5U)
#define GPSB_STAT_SERR          BIT(9)
#define GPSB_STAT_WOR           BIT(8)
#define GPSB_STAT_RUR           BIT(7)
#define GPSB_STAT_WUR           BIT(6)
#define GPSB_STAT_ROR           BIT(5)
#define GPSB_STAT_RF            BIT(4)
#define GPSB_STAT_WE            BIT(3)
#define GPSB_STAT_RNE           BIT(2)
#define GPSB_STAT_WTH           BIT(1)
#define GPSB_STAT_RTH           BIT(0)

/*
 * GPSB INTEN Register
 */
#define GPSB_INTEN_DW          BIT(31)
#define GPSB_INTEN_DR          BIT(30)
#define GPSB_INTEN_SHT         BIT(27)
#define GPSB_INTEN_SBT         BIT(26)
#define GPSB_INTEN_SHR         BIT(25)
#define GPSB_INTEN_SBR         BIT(24)
#define GPSB_INTEN_CFGWTH(x)   ((u32)((x) & 0x7U) << 20U)
#define GPSB_INTEN_CFGWTH_MASK (0x7U << 20U)
#define GPSB_INTEN_CFGRTH(x)   ((u32)((x) & 0x7U) << 16U)
#define GPSB_INTEN_CFGRTH_MASK (0x7U << 16U)
#define GPSB_INTEN_RC          BIT(15)

/*
 * GPSB MODE Register
 */
#define GPSB_MODE_DIVLDV(x)   ((u32)((x) & 0xFFU) << 24U)
#define GPSB_MODE_DIVLDV_MASK (0xFFU << 24U)
#define GPSB_MODE_TRE         BIT(23)
#define GPSB_MODE_THL         BIT(22)
#define GPSB_MODE_TSU         BIT(21)
#define GPSB_MODE_PCS         BIT(20)
#define GPSB_MODE_PCD         BIT(19)
#define GPSB_MODE_PWD         BIT(18)
#define GPSB_MODE_PRD         BIT(17)
#define GPSB_MODE_PCK         BIT(16)
#define GPSB_MODE_CRF         BIT(15)
#define GPSB_MODE_CWF         BIT(14)
#define GPSB_MODE_BWS(x)      (((x) & 0x1FU) << 8U)
#define GPSB_MODE_BWS_MASK    (0x1FU << 8U)
#define GPSB_MODE_SD          BIT(7)
#define GPSB_MODE_LB          BIT(6)
#define GPSB_MODE_SDO         BIT(5)
#define GPSB_MODE_CTF         BIT(4)
#define GPSB_MODE_EN          BIT(3)
#define GPSB_MODE_SLV         BIT(2)
#define GPSB_MODE_MD(x)       ((u32)((x) & 0x3U))
#define GPSB_MODE_MD_MASK     (BIT(1) | BIT(0))

/*
 * GPSB CTRL Register
 */
#define GPSB_CTRL_LCW         BIT(31)
#define GPSB_CTRL_LCR         BIT(30)
#define GPSB_CTRL_CMDEND(x)   ((u32)((x) & 0x1FUL) << 24U)
#define GPSB_CTRL_CMDSTART(x) ((u32)((x) & 0x1FUL) << 16U)
#define GPSB_CTRL_RDSTART     ((u32)((x) & 0x1FUL) << 8U)
#define GPSB_CTRL_PLW         BIT(7)
#define GPSB_CTRL_PSW         ((u32)((x) & 0x1FUL))

/*
 * GPSB EVTCTRL Register
 */
#define GPSB_EVTCTRL_TXCRX    BIT(31)
#define GPSB_EVTCTRL_TXCREP   BIT(30)
#define GPSB_EVTCTRL_EXTEN    BIT(29)
#define GPSB_EVTCTRL_EXTDCHK  BIT(28)
#define GPSB_EVTCTRL_EXTDPOL  BIT(27)
#define GPSB_EVTCTRL_MDLYE    BIT(26)
#define GPSB_EVTCTRL_SDOE     BIT(25)
#define GPSB_EVTCTRL_RDPS     BIT(24)
#define GPSB_EVTCTRL_CONTM(x) ((u32)((x) & 0x3U) << 22U)
#define GPSB_EVTCTRL_MDLYS    BIT(21)
#define GPSB_EVTCTRL_TXCV(x)  (u32)((x) & 0xFFFFU)

/*
 * GPSB CCV Register
 */
#define GPSB_CCV_FSDI         BIT(31)
#define GPSB_CCV_TXC(x)       (u32)((x) & 0xFFFFU)

/*
 * GPSB PACKET Register
 */
#define GPSB_PACKET_COUNT(x)   ((u32)((x) & 0x1FFFU) << 16U)
#define GPSB_PACKET_SIZE(x)    (u32)((x) & 0x1FFFU)
#define GPSB_PACKET_MAX_COUNT  0x1FFF
#define GPSB_PACKET_MAX_SIZE   0x1FFCU

/*
 * GPSB DMA Control Register
 */
#define GPSB_DMACTR_DTE        BIT(31)
#define GPSB_DMACTR_DRE        BIT(30)
#define GPSB_DMACTR_CT         BIT(29)
#define GPSB_DMACTR_END        BIT(28)
#define GPSB_DMACTR_TXAM(x)    ((u32)((x) & 0x3U) << 16U)
#define GPSB_DMACTR_RXAM(x)    ((u32)((x) & 0x3U) << 14U)
#define GPSB_DMACTR_MD(x)      ((u32)((x) & 0x3U) << 4U)
#define GPSB_DMACTR_PCLR       BIT(2)
#define GPSB_DMACTR_EN         BIT(0)

/*
 * GPSB DMA Status Register
 */
#define GPSB_DMASTR_RXPCNT(x)  ((u32)((x) & 0x1FFFU) << 17U)
#define GPSB_DMASTR_TXPCNT(x)  (u32)((x) & 0x1FFFU)

/*
 * GPSB DMA Interrupt Control Register
 */
#define GPSB_DMAICR_ISD        BIT(29)
#define GPSB_DMAICR_ISP        BIT(28)
#define GPSB_DMAICR_IRQS       BIT(20)
#define GPSB_DMAICR_IED        BIT(17)
#define GPSB_DMAICR_IEP        BIT(16)
#define GPSB_DMAICR_IRQPCNT(x) (u32)((x) & 0x1FFFU)

/*
 * GPSB Port Selection Reigster
 */
#define GPSB_PORT_SEL0 0x00U
#define GPSB_PORT_SEL1 0x04U
#define GPSB_ISTS      0x0CU

/*
 * GPSB Access Control Register
 */
#define GPSB_AC0_START 0x00U
#define GPSB_AC0_LIMIT 0x04U
#define GPSB_AC1_START 0x08U
#define GPSB_AC1_LIMIT 0x0CU
#define GPSB_AC2_START 0x10U
#define GPSB_AC2_LIMIT 0x14U
#define GPSB_AC3_START 0x18U
#define GPSB_AC3_LIMIT 0x1CU

/* SPI driver name */
#define TCC_SPI_DRV_NAME	"tcc-spi"
#define GPSB_CH_NUM          6

/* interrupt timeout count */
#define TCC_SPI_TIMEOUT	           100000UL

/* Structure for telechips spi private data */
struct tcc_spi_priv {
	struct spi_slave slave;
	void __iomem  *regs;		/* GPSB base address */
	void __iomem  *pcfg;		/* GPSB port configuration address */
	void __iomem  *ac;		/* DMA access control address */
	int32_t       channel;		/* GPSB channel number */
	struct clk    hclk;		/* GPSB IOBUS */
	struct clk    pclk;		/* Peri. Clock */
	uint32_t  irq_id;		/* Interrupt ID */
	uint32_t  mode;			/* SPI MODE */
	uint32_t  max_speed;		/* Default SCLK in Hz*/
	uint32_t  sclk;			/* Default SCLK in Hz*/
	uint32_t  cs;			/* CS */
	uint32_t  port;			/* GPSB port number */
	bool      ctf;			/* 1: CTF mode 0: Normal mode */
	uint8_t   bpw;			/* Bit width */

	/* DMA */
	bool      dma_mode;
	bool      use_gdma;
	uint32_t  dma_buf_size;
	uint32_t  *tx;
	uint32_t  *rx;
	uint32_t  remain_bytes;
};

#endif /* DEV_SPI_H */
