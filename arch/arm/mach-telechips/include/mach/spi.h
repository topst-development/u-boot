/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef DEV_SPI_H
#define DEV_SPI_H
#include <spi.h>
#include <common.h>

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
#define GPSB_STAT_ERR           ((u32)0x1FU << 5U)
#define GPSB_STAT_SERR          (u32)BIT(9)
#define GPSB_STAT_WOR           (u32)BIT(8)
#define GPSB_STAT_RUR           (u32)BIT(7)
#define GPSB_STAT_WUR           (u32)BIT(6)
#define GPSB_STAT_ROR           (u32)BIT(5)
#define GPSB_STAT_RF            (u32)BIT(4)
#define GPSB_STAT_WE            (u32)BIT(3)
#define GPSB_STAT_RNE           (u32)BIT(2)
#define GPSB_STAT_WTH           (u32)BIT(1)
#define GPSB_STAT_RTH           (u32)BIT(0)

/*
 * GPSB INTEN Register
 */
#define GPSB_INTEN_DW          (u32)BIT(31)
#define GPSB_INTEN_DR          (u32)BIT(30)
#define GPSB_INTEN_SHT         (u32)BIT(27)
#define GPSB_INTEN_SBT         (u32)BIT(26)
#define GPSB_INTEN_SHR         (u32)BIT(25)
#define GPSB_INTEN_SBR         (u32)BIT(24)
#define GPSB_INTEN_CFGWTH(x)   ((u32)((x) & (u32)0x7U) << 20U)
#define GPSB_INTEN_CFGWTH_MASK (0x7U << 20U)
#define GPSB_INTEN_CFGRTH(x)   ((u32)((x) & (u32)0x7U) << 16U)
#define GPSB_INTEN_CFGRTH_MASK (0x7U << 16U)
#define GPSB_INTEN_RC          (u32)BIT(15)

/*
 * GPSB MODE Register
 */
#define GPSB_MODE_DIVLDV(x)   ((u32)((x) & 0xFFU) << 24U)
#define GPSB_MODE_DIVLDV_MASK ((u32)0xFFU << 24U)
#define GPSB_MODE_TRE         (u32)BIT(23)
#define GPSB_MODE_THL         (u32)BIT(22)
#define GPSB_MODE_TSU         (u32)BIT(21)
#define GPSB_MODE_PCS         (u32)BIT(20)
#define GPSB_MODE_PCD         (u32)BIT(19)
#define GPSB_MODE_PWD         (u32)BIT(18)
#define GPSB_MODE_PRD         (u32)BIT(17)
#define GPSB_MODE_PCK         (u32)BIT(16)
#define GPSB_MODE_CRF         (u32)BIT(15)
#define GPSB_MODE_CWF         (u32)BIT(14)
#define GPSB_MODE_BWS(x)      (u32)(((x) & (u32)0x1FU) << 8U)
#define GPSB_MODE_BWS_MASK    (u32)(0x1FU << (u32)8U)
#define GPSB_MODE_SD          (u32)BIT(7)
#define GPSB_MODE_LB          (u32)BIT(6)
#define GPSB_MODE_SDO         (u32)BIT(5)
#define GPSB_MODE_CTF         (u32)BIT(4)
#define GPSB_MODE_EN          (u32)BIT(3)
#define GPSB_MODE_SLV         (u32)BIT(2)
#define GPSB_MODE_MD(x)       ((u32)((x) & (u32)0x3U))
#define GPSB_MODE_MD_MASK     ((u32)BIT(1) | (u32)BIT(0))

/*
 * GPSB CTRL Register
 */
#define GPSB_CTRL_LCW         (u32)(u32)BIT(31)
#define GPSB_CTRL_LCR         (u32)BIT(30)
#define GPSB_CTRL_CMDEND(x)   ((u32)((x) & (u32)0x1FUL) << 24U)
#define GPSB_CTRL_CMDSTART(x) ((u32)((x) & (u32)0x1FUL) << 16U)
#define GPSB_CTRL_RDSTART     ((u32)((x) & (u32)0x1FUL) << 8U)
#define GPSB_CTRL_PLW         (u32)BIT(7)
#define GPSB_CTRL_PSW         ((u32)((x) & (u32)0x1FUL))

/*
 * GPSB EVTCTRL Register
 */
#define GPSB_EVTCTRL_TXCRX    (u32)BIT(31)
#define GPSB_EVTCTRL_TXCREP   (u32)BIT(30)
#define GPSB_EVTCTRL_EXTEN    (u32)BIT(29)
#define GPSB_EVTCTRL_EXTDCHK  (u32)BIT(28)
#define GPSB_EVTCTRL_EXTDPOL  (u32)BIT(27)
#define GPSB_EVTCTRL_MDLYE    (u32)BIT(26)
#define GPSB_EVTCTRL_SDOE     (u32)BIT(25)
#define GPSB_EVTCTRL_RDPS     (u32)BIT(24)
#define GPSB_EVTCTRL_CONTM(x) ((u32)((x) & (u32)0x3U) << 22U)
#define GPSB_EVTCTRL_MDLYS    (u32)BIT(21)
#define GPSB_EVTCTRL_TXCV(x)  (u32)((x) & (u32)0xFFFFU)

/*
 * GPSB CCV Register
 */
#define GPSB_CCV_FSDI         (u32)BIT(31)
#define GPSB_CCV_TXC(x)       (u32)((x) & (u32)0xFFFFU)

/*
 * GPSB PACKET Register
 */
#define GPSB_PACKET_COUNT(x)   ((u32)((x) & (u32)0x1FFFU) << 16U)
#define GPSB_PACKET_SIZE(x)    (u32)((x) & (u32)0x1FFFU)
#define GPSB_PACKET_MAX_COUNT  0x1FFF
#define GPSB_PACKET_MAX_SIZE   0x1FFCU

/*
 * GPSB DMA Control Register
 */
#define GPSB_DMACTR_DTE        (u32)BIT(31)
#define GPSB_DMACTR_DRE        (u32)BIT(30)
#define GPSB_DMACTR_CT         (u32)BIT(29)
#define GPSB_DMACTR_END        (u32)BIT(28)
#define GPSB_DMACTR_TXAM(x)    ((u32)((x) & (u32)0x3U) << 16U)
#define GPSB_DMACTR_RXAM(x)    ((u32)((x) & (u32)0x3U) << 14U)
#define GPSB_DMACTR_MD(x)      ((u32)((x) & (u32)0x3U) << 4U)
#define GPSB_DMACTR_PCLR       (u32)BIT(2)
#define GPSB_DMACTR_EN         (u32)BIT(0)

/*
 * GPSB DMA Status Register
 */
#define GPSB_DMASTR_RXPCNT(x)  ((u32)((x) & (u32)0x1FFFU) << 17U)
#define GPSB_DMASTR_TXPCNT(x)  (u32)((x) & (u32)0x1FFFU)

/*
 * GPSB DMA Interrupt Control Register
 */
#define GPSB_DMAICR_ISD        (u32)BIT(29)
#define GPSB_DMAICR_ISP        (u32)BIT(28)
#define GPSB_DMAICR_IRQS       (u32)BIT(20)
#define GPSB_DMAICR_IED        (u32)BIT(17)
#define GPSB_DMAICR_IEP        (u32)BIT(16)
#define GPSB_DMAICR_IRQPCNT(x) (u32)((x) & (u32)0x1FFFU)

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
	void __iomem  *regs;	/* GPSB base address */
	void __iomem  *pcfg;	/* GPSB port configuration address */
	void __iomem  *ac;		/* DMA access control address */
	int32_t 	channel;	/* GPSB channel number */
	struct clk 	hclk;		/* GPSB IOBUS */
	struct clk 	pclk;		/* Peri. Clock */
	uint32_t	irq_id;		/* Interrupt ID */
	uint32_t	mode;			/* SPI MODE */
	uint32_t	max_speed;	/* Default SCLK in Hz*/
	uint32_t	sclk;			/* Default SCLK in Hz*/
	uint32_t	cs;			/* CS */
	uint32_t	port;			/* GPSB port number */
	bool		ctf;			/* 1: CTF mode 0: Normal mode */
	uint8_t		bpw;			/* Bit width */

	/* DMA */
	bool		dma_mode;
	bool		use_gdma;
	uint32_t	dma_buf_size;
	//size_t    tx;
	//size_t    rx;
	void		*tx;
	void		*rx;
	uint32_t	remain_bytes;
};

#endif /* DEV_SPI_H */
