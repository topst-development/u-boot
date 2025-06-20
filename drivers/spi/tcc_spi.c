// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/arch/spi.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <memalign.h>
#include <cpu_func.h>
#include <clk.h>
#include <spi.h>
#include <linux/io.h>
#include <asm/dma-mapping.h>
#include <mach/clock.h>
#include <dm/pinctrl.h>

#define TCC897X ((u8)0U)
#define TCC803X ((u8)1U)
#define TCC805X ((u8)2U)
#define TCC750X ((u8)3U)
#define TCC807X ((u8)4U)

static void bitset_gpsb(void __iomem* regs, uint32_t mask)
{
	uint32_t val;
	val = readl(regs);
	val = val | mask;
	writel(val, regs);
}

static void bitclr_gpsb(void __iomem* regs, uint32_t mask)
{
	uint32_t val;
	val = readl(regs);
	val = val & ~mask;
	writel(val, regs);
}

static void bitcset_gpsb(void __iomem* regs, uint32_t cmask, uint32_t smask)
{
	uint32_t val;
	val = readl(regs);
	val = val & ~cmask;
	val = val | smask;
	writel(val, regs);
}

static void tcc_spi_dump(const struct udevice *bus)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);

	(void)pr_debug("[DEBUG][SPI] GPSB REGS DUMP [CH: %d]\n", tccspi->channel);
	(void)pr_debug("[DEBUG][SPI] STAT\t: %#X\n",
			readl(tccspi->regs + GPSB_STAT));
	(void)pr_debug("[DEBUG][SPI] INTEN\t: %#X\n",
			readl(tccspi->regs + GPSB_INTEN));
	(void)pr_debug("[DEBUG][SPI] MODE\t: %#X\n",
			readl(tccspi->regs + GPSB_MODE));
	(void)pr_debug("[DEBUG][SPI] CTRL\t: %#X\n",
			readl(tccspi->regs + GPSB_CTRL));
	(void)pr_debug("[DEBUG][SPI] EVTCTRL\t: %#X\n",
			readl(tccspi->regs + GPSB_EVTCTRL));
	(void)pr_debug("[DEBUG][SPI] CCV\t: %#X\n",
			readl(tccspi->regs + GPSB_CCV));

	(void)pr_debug("[DEBUG][SPI] TXBASE\t: %#X\n",
			readl(tccspi->regs + GPSB_TXBASE));
	(void)pr_debug("[DEBUG][SPI] RXBASE\t: %#X\n",
			readl(tccspi->regs + GPSB_RXBASE));
	(void)pr_debug("[DEBUG][SPI] PACKET\t: %#X\n",
			readl(tccspi->regs + GPSB_PACKET));
	(void)pr_debug("[DEBUG][SPI] DMACTR\t: %#X\n",
			readl(tccspi->regs + GPSB_DMACTR));
	(void)pr_debug("[DEBUG][SPI] DMASTR\t: %#X\n",
			readl(tccspi->regs + GPSB_DMASTR));
	(void)pr_debug("[DEBUG][SPI] DMAICR\t: %#X\n",
			readl(tccspi->regs + GPSB_DMAICR));
}

static void tcc_spi_reset(const struct udevice *bus)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);

	if ((tccspi->soc_info->id == TCC897X) ||
		(tccspi->soc_info->id == TCC803X) ||
		(tccspi->soc_info->id == TCC805X) ||
		(tccspi->soc_info->id == TCC807X)) {
		/* enable SW RESET */
		(void)tcc_set_iobus_swreset(tccspi->hclk.id, (bool)true);
		(void)tcc_set_iobus_swreset(tccspi->hclk.id, (bool)false);

		/* enable IO BUS */
		(void)tcc_set_iobus_pwdn(tccspi->hclk.id, (bool)false);
	} else if (tccspi->soc_info->id == TCC750X) {
		(void)tcc_set_swreset(tccspi->hclk.id, (bool)true);
		(void)tcc_set_swreset(tccspi->hclk.id, (bool)false);
	}
}

static void tcc_spi_set_packet(const struct udevice *bus, uint32_t packet_size)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);

	/* set packet size */
	bitcset_gpsb(tccspi->regs + GPSB_PACKET,
			0x1FFF,
			GPSB_PACKET_SIZE(packet_size));
	/* set packet count to 0 */
	bitclr_gpsb(tccspi->regs + GPSB_PACKET, GPSB_PACKET_COUNT(0x1FFFUL));

	(void)pr_debug("[DEBUG][SPI]%s: set packet size: %u\n", __func__, packet_size);
}

static void tcc_spi_clear_packet_cnt(const struct udevice *bus)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);

	bitset_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_CWF | GPSB_MODE_CRF);
	bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_CWF | GPSB_MODE_CRF);

	bitset_gpsb(tccspi->regs + GPSB_DMACTR, GPSB_DMACTR_PCLR);
	bitclr_gpsb(tccspi->regs + GPSB_DMACTR, GPSB_DMACTR_PCLR);

	(void)pr_debug("[DEBUG][SPI] %s: clear packet count\n", __func__);
}

static int32_t tcc_spi_set_clk(const struct udevice *bus, uint32_t sclk)
{
	struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	ulong peri_clk, temp_ulong;
	int32_t ret = 0;

	/* Check max speed */
	if (tccspi->max_speed < sclk) {
		(void)pr_err("[ERROR][SPI] %s: sclk %u is faster than max speed %u\n",
				__func__, sclk, tccspi->max_speed);
		ret = -EINVAL;
	} else {
		tccspi->sclk = sclk;
		peri_clk = ((ulong)tccspi->sclk * 2UL);

		/* Set DIVLDV to zero */
		bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_DIVLDV(0xFFUL));

		/* Set peri clock */
		if ((tccspi->soc_info->id == TCC803X) ||
			(tccspi->soc_info->id == TCC805X) ||
			(tccspi->soc_info->id == TCC750X) ||
			(tccspi->soc_info->id == TCC807X)) {
			temp_ulong = clk_set_rate(&(tccspi->pclk), peri_clk);
			peri_clk = clk_get_rate(&(tccspi->pclk));
		} else if (tccspi->soc_info->id == TCC897X) {
			temp_ulong = tcc_set_peri(tccspi->pclk.id, CKC_ENABLE, peri_clk, 0);
			peri_clk = tcc_get_peri(tccspi->pclk.id);
		}

		if (temp_ulong < 0xFFFFFFFFU) {
			ret = (s32)temp_ulong;
		}

		if (ret < 0) {
			(void)pr_err("[ERROR][SPI] fail to set peripheral clock\n");
		} else {
			(void)pr_debug("[DEBUG][SPI] %s: sclk %u Hz, peri clk %lu Hz\n",
					__func__, tccspi->sclk, peri_clk);
		}
	}

	return ret;
}

static void tcc_spi_set_bitwidth(const struct udevice *bus)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	uint32_t mode;
	uint8_t bitWidth = 7;

	/* set bws */
	if (tccspi->bpw > 1U) {
		bitWidth = (tccspi->bpw - 1U);
	}

	bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_BWS(31UL));
	bitset_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_BWS(bitWidth));
	mode = readl(tccspi->regs + GPSB_MODE);

	/* polling mode */
	if (!tccspi->dma_mode) {
		bitset_gpsb(tccspi->regs + GPSB_INTEN,
				(GPSB_INTEN_SHT |
				 GPSB_INTEN_SBT |
				 GPSB_INTEN_SHR |
				 GPSB_INTEN_SBR));
	} else {
		/* dma mode */
		if (!tccspi->use_gdma) {
			bitclr_gpsb(tccspi->regs + GPSB_INTEN,
					(GPSB_INTEN_SHT |
					 GPSB_INTEN_SBT |
					 GPSB_INTEN_SHR |
					 GPSB_INTEN_SBR));
			if ((mode & GPSB_MODE_BWS(0x10UL)) != 0u) {
				/* little endian */
				bitclr_gpsb(tccspi->regs + GPSB_DMACTR,
						GPSB_DMACTR_END);
			} else {
				/* Big endian */
				bitset_gpsb(tccspi->regs + GPSB_DMACTR,
						GPSB_DMACTR_END);
				if (bitWidth == 16u) {
					bitset_gpsb(tccspi->regs + GPSB_INTEN,
							(GPSB_INTEN_SBT | GPSB_INTEN_SBR));
				}
			}
		} else {
			bitclr_gpsb(tccspi->regs + GPSB_INTEN,
					(GPSB_INTEN_SHT |
					 GPSB_INTEN_SBT |
					 GPSB_INTEN_SHR |
					 GPSB_INTEN_SBR));
		}
	}
	(void)pr_debug("[DEBUG][SPI] %s: bitwidth %u\n", __func__, bitWidth);
}

static void tcc_spi_set_ctf_mode(const struct udevice *bus, bool on)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);

	if (on) {
		bitset_gpsb(tccspi->regs + GPSB_EVTCTRL,
				GPSB_EVTCTRL_CONTM(0x3UL));
		bitset_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_CTF);
	} else {
		bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_CTF);
	}

	(void)pr_debug("[DEBUG][SPI] %s: set CTF %d\n", __func__, on);
}

static int32_t tcc_spi_set_mode(struct udevice *bus, uint mode)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);

	if ((mode & (u32)SPI_SLAVE) > 0U) {
		/* slave mode */
		bitset_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_SLV);

		if ((mode == (u32)SPI_MODE_1) || (mode == (u32)SPI_MODE_2)) {
			bitset_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_PCK);
		} else {
			bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_PCK);
		}
		if ((mode & (u32)SPI_CS_HIGH) > 0U) {
			bitset_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_PCD);
		} else {
			bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_PCD);
		}

		(void)pr_debug("[DEBUG][SPI] %s: mode %#X (mode reg: %#X)\n",
				__func__, mode, readl(tccspi->regs + GPSB_MODE));
	} else {
		/* master mode */
		bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_SLV);

		if ((mode & (u32)SPI_CPOL) > 0U) {
			bitset_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_PCK);
		} else {
			bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_PCK);
		}
		if ((mode & (u32)SPI_CPHA) > 0U) {
			bitset_gpsb(tccspi->regs + GPSB_MODE,
					GPSB_MODE_PWD | GPSB_MODE_PRD);
		} else {
			bitclr_gpsb(tccspi->regs + GPSB_MODE,
					GPSB_MODE_PWD | GPSB_MODE_PRD);
		}
		if ((mode & (u32)SPI_CS_HIGH) > 0U) {
			bitset_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_PCS);
		} else {
			bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_PCS);
		}
		if ((mode & (u32)SPI_LSB_FIRST) > 0U) {
			bitset_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_SD);
		} else {
			bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_SD);
		}
		if ((mode & (u32)SPI_LOOP) > 0U) {
			bitset_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_LB);
		} else {
			bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_LB);
		}

		(void)pr_debug("[DEBUG][SPI] %s: mode %#X (mode reg: %#X)\n",
				__func__, mode, readl(tccspi->regs + GPSB_MODE));
	}

	return 0;
}

static int32_t tcc_spi_txrx8(const struct udevice *bus,
		uint32_t bytes,
		const void *dout,
		void *din)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	uint32_t data_num, i, temp_u32;
	ulong cnt;
	const uint8_t *tx = dout;
	uint8_t *rx = din;
	int32_t ret = 0;

	data_num = bytes;

	for (i = 0; i < data_num; i++) {
		if (tx != NULL) {
			writel(((uint32_t)(tx[i]) << 24),
					tccspi->regs + GPSB_PORT);
			(void)pr_debug("[DEBUG][SPI] Tx[%u] - 0x%x\n", i, tx[i]);
		} else {
			writel(0, tccspi->regs + GPSB_PORT);
		}

		/* Check Rx FIFO not empty flag */
		cnt = 0;
		while ((readl(tccspi->regs + GPSB_STAT) & GPSB_STAT_RNE) == 0U) {
			cnt++;
			if (cnt > TCC_SPI_TIMEOUT) {
				(void)pr_err("[ERROR][SPI] %s: spi time out occrur !! (STAT: 0x%X)\n",
						__func__,
						readl(tccspi->regs + GPSB_STAT));
				ret = -ETIME;
				break;
			}
		}

		if (ret == 0) {
			temp_u32 = readl(tccspi->regs + GPSB_PORT);

			if (rx != NULL) {
				rx[i] = (uint8_t)(temp_u32 >> 24U);
				(void)pr_debug("[DEBUG][SPI] Rx[%u] - 0x%x (temp_u32 0x%x)\n",
						i, (uint32_t)rx[i], temp_u32);
			}
		}
	}

	return ret;
}

static int32_t tcc_spi_txrx16(const struct udevice *bus,
		uint32_t bytes,
		const void *dout,
		void *din)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	uint32_t data_num, i, temp_u32;
	ulong cnt;
	const uint16_t *tx = dout;
	uint16_t *rx = din;
	int32_t ret = 0;

	if ((bytes % 2U) != 0U) {
		(void)pr_err("[ERROR][SPI] %s: Non word aligned SPI transfer. bitwidth %u\n",
				__func__, tccspi->bpw);
		ret = -EINVAL;
	} else {
		data_num = bytes / 2U;

		for (i = 0; i < data_num; i++) {
			if (tx != NULL) {
				writel(((uint32_t)(tx[i]) << 16),
						tccspi->regs + GPSB_PORT);
				(void)pr_debug("[DEBUG][SPI] Tx[%u] - 0x%x\n", i, tx[i]);
			} else {
				writel(0, tccspi->regs + GPSB_PORT);
			}

			/* Check Rx FIFO not empty flag */
			cnt = 0;
			while ((readl(tccspi->regs + GPSB_STAT) & GPSB_STAT_RNE) == 0U) {
				cnt++;
				if (cnt > TCC_SPI_TIMEOUT) {
					(void)pr_err("[ERROR][SPI] %s: spi time out occrur !! (STAT: 0x%X)\n",
							__func__,
							readl(tccspi->regs + GPSB_STAT));
					ret = -ETIME;
					break;
				}
			}

			if (ret == 0) {
				temp_u32 = readl(tccspi->regs + GPSB_PORT);

				if (rx != NULL) {
					rx[i] = (uint16_t)(temp_u32 >> 16U);
					(void)pr_debug("[DEBUG][SPI] Rx[%u] - 0x%x (temp_u32 0x%x)\n",
							i, (uint32_t)rx[i], temp_u32);
				}
			}
		}
	}

	return ret;
}

static int32_t tcc_spi_txrx32(const struct udevice *bus,
		uint32_t bytes,
		const void *dout,
		void *din)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	uint32_t data_num, i, temp_u32;
	ulong cnt;
	const uint32_t *tx = dout;
	uint32_t *rx = din;
	int32_t ret = 0;

	if ((bytes % 4U) != 0U) {
		(void)pr_err("[ERROR][SPI] %s: Non word aligned SPI transfer. bitwidth %u\n",
				__func__, tccspi->bpw);
		ret = -EINVAL;
	} else {
		data_num = bytes / 4U;

		for (i = 0; i < data_num; i++) {
			if (tx != NULL) {
				writel(tx[i], tccspi->regs + GPSB_PORT);
				(void)pr_debug("[DEBUG][SPI] Tx[%u] - 0x%X\n", i, tx[i]);
			} else {
				writel(0, tccspi->regs + GPSB_PORT);
			}

			/* Check Rx FIFO not empty flag */
			cnt = 0;
			while ((readl(tccspi->regs + GPSB_STAT) & GPSB_STAT_RNE) == 0U) {
				cnt++;
				if (cnt > TCC_SPI_TIMEOUT) {
					(void)pr_err("[ERROR][SPI] %s: spi time out occrur !! (STAT: 0x%X)\n",
							__func__,
							readl(tccspi->regs + GPSB_STAT));
					ret = -ETIME;
					break;
				}
			}

			if (ret == 0) {
				temp_u32 = readl(tccspi->regs + GPSB_PORT);

				if (rx != NULL) {
					rx[i] = temp_u32;
					(void)pr_debug("[DEBUG][SPI] Rx[%u] - 0x%X (temp_u32 0x%X)\n",
							i, rx[i], temp_u32);
				}
			}
		}
	}

	return ret;
}

static int32_t tcc_spi_xfer_pio(const struct udevice *bus,
		uint32_t bytes,
		const void *dout,
		void *din)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	int32_t ret = 0;

	(void)pr_debug("[DEBUG][SPI] %s: bytes %u dout 0x%p dint 0x%p bitwidth %u\n",
			__func__, bytes, dout, din, tccspi->bpw);

	tcc_spi_clear_packet_cnt(bus);
	/* enable GPSB operation */
	bitset_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_EN);

	if (tccspi->bpw == 8U) {
		ret = tcc_spi_txrx8(bus, bytes, dout, din);
	} else if (tccspi->bpw == 16U) {
		ret = tcc_spi_txrx16(bus, bytes, dout, din);
	} else if (tccspi->bpw == 32U) {
		ret = tcc_spi_txrx32(bus, bytes, dout, din);
	} else {
		(void)pr_err("[ERROR][SPI] Not supported bpw %u\n", tccspi->bpw);
		ret = -EINVAL;
	}

	if (ret == 0) {
		/* disable GPSB operation */
		bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_EN);
	}

	return ret;
}

static void tcc_spi_stop_dma(const struct udevice *bus)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);

	/* Disable GPSB DMA operation */
	bitclr_gpsb(tccspi->regs + GPSB_DMACTR, GPSB_DMACTR_EN);

	/* Clear DMA done and packet interrupt status */
	bitset_gpsb(tccspi->regs + GPSB_DMAICR,
			GPSB_DMAICR_ISD | GPSB_DMAICR_ISP);

	/* disable DMA Request */
	bitclr_gpsb(tccspi->regs + GPSB_DMACTR,
			GPSB_DMACTR_DTE | GPSB_DMACTR_DRE);
	bitclr_gpsb(tccspi->regs + GPSB_INTEN,
			GPSB_INTEN_DR | GPSB_INTEN_DW);
}

static void tcc_spi_start_dma(const struct udevice *bus)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);

	/* Set GPSB DMA address mode (Multiple address mode) */
	bitclr_gpsb(tccspi->regs + GPSB_DMACTR, (u32)0x3C000U);

	/* Disable DMA packet interrupt */
	bitclr_gpsb(tccspi->regs + GPSB_DMAICR, GPSB_DMAICR_IEP);

	/* Enable DMA done interrupt */
	bitset_gpsb(tccspi->regs + GPSB_DMAICR, GPSB_DMAICR_IED);

	/* Set DMA Receiving interrupt */
	bitclr_gpsb(tccspi->regs + GPSB_DMAICR, GPSB_DMAICR_IRQS);

	/* enable DMA Request */
	bitset_gpsb(tccspi->regs + GPSB_DMACTR,
			GPSB_DMACTR_DTE | GPSB_DMACTR_DRE);
	bitset_gpsb(tccspi->regs + GPSB_INTEN,
			GPSB_INTEN_DR | GPSB_INTEN_DW);

	/*enable DMA */
	bitset_gpsb(tccspi->regs + GPSB_DMACTR, GPSB_DMACTR_EN);
}

static int32_t tcc_spi_check_status(const struct udevice *bus)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	uint32_t status = readl(tccspi->regs + GPSB_STAT);
	int32_t ret = 0;

	if ((status & GPSB_STAT_ERR) > 0U) {
		(void)pr_err("[ERROR][SPI] %s: ERROR FLAG (STATUS: %#X)\n",
				__func__, status);
		tcc_spi_dump(bus);
		ret = -EIO;
	}

	return ret;
}

static int32_t tcc_spi_dma_handler(const struct udevice *bus)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	int32_t ret = 0;

	/* disable DMA */
	tcc_spi_stop_dma(bus);
	/* check error flag */
	ret = tcc_spi_check_status(bus);
	/* clear error flag */
	bitclr_gpsb(tccspi->regs + GPSB_STAT, GPSB_STAT_ERR);

	return ret;
}

static int32_t tcc_spi_alloc_dma_buf(const struct udevice *bus)
{
	struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	int32_t ret = 0;
	const void* dma_buf = NULL;

	tccspi->dma_buf_size = GPSB_PACKET_MAX_SIZE;

	/* allocate DMA buffer */
	dma_buf = dma_alloc_coherent(tccspi->dma_buf_size, ((ulong*)tccspi->tx));
	if (dma_buf == NULL) {
		(void)pr_err("[ERROR][SPI] %s: ch %d, allocate DMA buffer error for tx\n",
				__func__, tccspi->channel);
		ret = -ENOMEM;
	} else {
		/* set TX BASE */
		writel((size_t)tccspi->tx, tccspi->regs + GPSB_TXBASE);
	}

	if (ret == 0) {
		dma_buf = dma_alloc_coherent(tccspi->dma_buf_size, ((ulong*)tccspi->rx));
		if (dma_buf == NULL) {
			(void)pr_err("[ERROR][SPI] %s: ch %d, allocate DMA buffer error for rx\n",
					__func__, tccspi->channel);
			ret = -ENOMEM;
		} else {
			/* set RX BASE */
			writel((size_t)tccspi->rx, tccspi->regs + GPSB_RXBASE);
		}
	}

	return ret;
}

static void tcc_spi_deinit_dma_buf(const struct udevice *bus)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);

	if (tccspi->tx != NULL) {
		(void)dma_free_coherent(tccspi->tx);
	}
	if (tccspi->rx != NULL) {
		(void)dma_free_coherent(tccspi->rx);
	}
}

static int32_t tcc_spi_xfer_dma(const struct udevice *bus,
		uint32_t bytes,
		const void *dout,
		void *din)
{
	struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	uint32_t len;
	const void *dst;
	void *src;
	ulong tmo = 0;
	int32_t ret, time_error = 0;

	tccspi->remain_bytes = bytes;
	dst = dout;
	src = din;
	(void)memset(tccspi->tx, 0, tccspi->dma_buf_size);
	(void)memset(tccspi->rx, 0, tccspi->dma_buf_size);

	while (tccspi->remain_bytes > 0U) {
		/* set packet size */
		if (tccspi->remain_bytes >= GPSB_PACKET_MAX_SIZE) {
			len = GPSB_PACKET_MAX_SIZE;
		} else {
			len = tccspi->remain_bytes;
		}
		tcc_spi_set_packet(bus, len);

		/* copy dst to GPSB TX buffer */
		if (dst != NULL) {
			(void)memcpy((void*)tccspi->tx, dst, len);
		}

		/* clean and invalidate the area of cache */
		(void)flush_cache((ulong)tccspi->tx, len);
		(void)flush_cache((ulong)tccspi->rx, len);

		/* Clear FIFO/PCNT */
		tcc_spi_clear_packet_cnt(bus);

		/* TODO: Setup GDMA, if use */

		/* enable DMA */
		tcc_spi_start_dma(bus);

		tcc_spi_dump(bus);
		/* enable GPSB operation */
		bitset_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_EN);

		/* check IRQ status for Done Interrupt */
		tmo = get_timer(0UL);
		while ((readl(tccspi->regs + GPSB_DMAICR)
					& GPSB_DMAICR_ISD) == 0U) {
			if (get_timer(tmo) >= TCC_SPI_TIMEOUT) {
				(void)pr_err("[ERROR][SPI] DMA Interrupt Timeout!\n");
				tcc_spi_dump(bus);
				 time_error = -ETIMEDOUT;
				break;
			}
		}

		/* disable GPSB operation */
		bitclr_gpsb(tccspi->regs + GPSB_MODE, GPSB_MODE_EN);

		ret = tcc_spi_dma_handler(bus);
		if ((ret < 0) || (time_error < 0)) {
			(void)pr_err("[ERROR][SPI] %s: return %d (bytes: %u remain bytes: %u)\n",
					__func__,
					ret,
					bytes,
					tccspi->remain_bytes);

			ret = (time_error < 0) ? time_error : ret;
			break;
		}

		if (ret == 0) {
			/* copy GPSB RX buffer to src */
			if (src != NULL) {
				(void)memcpy(src, tccspi->rx, len);
			}
			tccspi->remain_bytes -= len;
			dst += len;
			src += len;
		}
	}

	return ret;
}

static int32_t tcc_spi_xfer(struct udevice *dev, uint32_t bitlen,
		const void *dout, void *din, ulong flags)
{
	const struct udevice *bus = dev->parent;
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	uint32_t bytelen;
	int32_t ret = 0;

	(void)flags;

	if (bitlen == 0U) {
		(void)pr_warn("[WARN][SPI] %s: No data to transfer. bitlen %u\n",
				__func__, bitlen);
	} else {
		if ((bitlen % 8U) != 0U) {
			(void)pr_err("[ERROR][SPI] %s: Non byte aligned - bitlen %u\n",
					__func__, bitlen);
			ret = -EINVAL;
		}

		if (ret == 0) {
			bytelen = bitlen / 8U;

			if (tccspi->dma_mode) {
				ret = tcc_spi_xfer_dma(bus, bytelen, dout, din);
			} else {
				ret = tcc_spi_xfer_pio(bus, bytelen, dout, din);
			}
		}

		if (ret < 0) {
			(void)pr_err("[ERROR][SPI] %s: xfer failed!! return %d\n",
					__func__, ret);
		}
	}

	return ret;
}

static int32_t tcc_spi_set_speed(struct udevice *bus, uint speed)
{
	return tcc_spi_set_clk(bus, speed);
}

static int32_t tcc_spi_set_port(const struct udevice *bus)
{
	const struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	int32_t ch = tccspi->channel;
	uint32_t port = tccspi->port;
	uint32_t pcfg_offset, pcfg_val;
	uint32_t offset;
	int32_t i, ret = 0;

	/* check channel and port */
	if (ch < 0) {
		(void)pr_err("[ERROR][SPI] %s: channel(%d), port(%u) is wrong number !\n",
				__func__, ch, port);
		ret = -EINVAL;
	} else {
		/* check port conflict */
		for (i = 0; i < GPSB_CH_NUM; i++) {
			if (i < 4) {
				pcfg_offset = GPSB_PORT_SEL0;
				pcfg_val = readl(tccspi->pcfg + GPSB_PORT_SEL0);
				offset = (u32)i << 3;
			} else {
				pcfg_offset = GPSB_PORT_SEL1;
				pcfg_val = readl(tccspi->pcfg + GPSB_PORT_SEL1);
				offset = ((u32)i - 4U) << 3;
			}

			if (offset <= 24U) {
				if (((pcfg_val >> offset) & 0xFFU) == port) {
					bitset_gpsb(tccspi->pcfg + pcfg_offset,
							((u32)0xFFU << offset));
					(void)pr_warn("[WARN][SPI] port: %u conflict - Channel %d is clear\n",
							port, i);
				}
			}
		}

		/* set port */
		if (ch < 4) {
			pcfg_offset = GPSB_PORT_SEL0;
			offset = (u32)ch << 3;
		} else {
			pcfg_offset = GPSB_PORT_SEL1;
			offset = ((u32)ch - 4U) << 3;
		}

		if (offset <= 24U) {
			bitcset_gpsb(tccspi->pcfg + pcfg_offset,
					((u32)0xFFU << offset),
					(port << offset));
		}

		(void)pr_debug("[DEBUG][SPI] %s: channel %d sets port %u\n",
				__func__, ch, port);
	}

	return ret;
}

static int32_t tcc_spi_parse_dt(struct udevice *bus)
{
	struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	uint32_t ac_val[2] = {0,};
	int32_t ret = 0;

	/* base, port configuration, access control address */
	tccspi->regs = (void *)dev_read_addr_index(bus, 0);
	tccspi->pcfg = (void *)dev_read_addr_index(bus, 1);
	tccspi->ac = (void *)dev_read_addr_index(bus, 2);

	/* channel */
	ret = dev_read_alias_seq(bus, &tccspi->channel);
	if (ret < 0) {
		(void)pr_err("[ERROR][SPI] %s: failed to get alias id\n", __func__);
		ret = -EINVAL;
	}

	if (ret == 0) {
		if (tccspi->channel < 0) {
			(void)pr_err("[ERROR][SPI] %s: wrong spi channel\n", __func__);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		/* get peripheral clock , iobus clock */
		ret = tcc_clk_get_by_index(bus, 0, &(tccspi->pclk));
		if (ret < 0) {
			(void)pr_err("[ERROR][SPI] failed to get peripheral clock\n");
		}
	}

	if (ret == 0) {
		if ((tccspi->soc_info->id == TCC803X) ||
			(tccspi->soc_info->id == TCC805X) ||
			(tccspi->soc_info->id == TCC807X)) {
			ret = tcc_clk_get_by_index(bus, 1, &(tccspi->hclk));
			if (ret < 0) {
				(void)pr_err("[ERROR][SPI] fail to get iobus clock\n");
			}
		} else if ((tccspi->soc_info->id == TCC897X) ||
				   (tccspi->soc_info->id == TCC750X)) {
			tccspi->hclk.id = PSWRST_GPSB_BASE + tccspi->channel;
		}
	}

	if (ret == 0) {
		if ((tccspi->soc_info->id == TCC897X) ||
			(tccspi->soc_info->id == TCC803X) ||
			(tccspi->soc_info->id == TCC805X) ||
			(tccspi->soc_info->id == TCC807X)) {
			/* port number */
			ret = dev_read_u32(bus, "gpsb-port", &tccspi->port);
			if (ret < 0) {
				(void)pr_err("[ERROR][SPI] There is no port information.\n");
				ret = -EINVAL;
			}
		}
	}

	if (ret == 0) {
		ret = pinctrl_select_state(bus, "active");
		if (ret < 0) {
			(void)pr_err("[ERROR][SPI] fail to get pinctrl (active)\n");
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		/* get max speed, if no value, default 10MHz */
		tccspi->max_speed = (uint32_t)dev_read_u32_default(bus,
				"spi-max-frequency",
				10000000);
		tccspi->bpw = (uint8_t)dev_read_u32_default(bus, "bpw", 32);
		if (tccspi->bpw > 32U) {
			(void)pr_err("[ERROR][SPI] %s: not supported bpw %u\n",
					__func__, tccspi->bpw);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		tccspi->ctf = !(dev_read_bool(bus, "ctf-mode-disable"));
		tccspi->dma_mode = dev_read_bool(bus, "dma-mode");
		tccspi->use_gdma = false;

		if ((tccspi->soc_info->id == TCC803X) ||
			(tccspi->soc_info->id == TCC805X)) {
			if (tccspi->dma_mode && (tccspi->channel > 2)) {
				tccspi->use_gdma = true;
				(void)pr_err("[WARN][SPI] %s: not ready to support GPSB ch %d with dma mode\n",
							 __func__, tccspi->channel);
				tccspi->dma_mode = false;
			}
		}

		/* access control : Configure the AHB Access Filter */
		if (tccspi->dma_mode) {
			ret = dev_read_u32_array(bus, "access-control0", ac_val, 2);
			if (ret == 0) {
				(void)pr_debug("[DEBUG][SPI] access-control0 start:%#x limit: %#x\n",
						ac_val[0], ac_val[1]);
				writel(ac_val[0], tccspi->ac + GPSB_AC0_START);
				writel(ac_val[1], tccspi->ac + GPSB_AC0_LIMIT);
			}

			ret = dev_read_u32_array(bus, "access-control1", ac_val, 2);
			if (ret == 0) {
				(void)pr_debug("[DEBUG][SPI] access-control1 start:%#x limit:%#x\n",
						ac_val[0], ac_val[1]);
				writel(ac_val[0], tccspi->ac + GPSB_AC1_START);
				writel(ac_val[1], tccspi->ac + GPSB_AC1_LIMIT);
			}

			ret = dev_read_u32_array(bus, "access-control2", ac_val, 2);
			if (ret == 0) {
				(void)pr_debug("[DEBUG][SPI] access-control2 start:%#x limit:%#x\n",
						ac_val[0], ac_val[1]);
				writel(ac_val[0], tccspi->ac + GPSB_AC2_START);
				writel(ac_val[1], tccspi->ac + GPSB_AC2_LIMIT);
			}

			ret = dev_read_u32_array(bus, "access-control3", ac_val, 2);
			if (ret == 0) {
				(void)pr_debug("[DEBUG][SPI] access-control3 start:%#x limit:%#x\n",
						ac_val[0], ac_val[1]);
				writel(ac_val[0], tccspi->ac + GPSB_AC3_START);
				writel(ac_val[1], tccspi->ac + GPSB_AC3_LIMIT);
			}
		}
	}

	return ret;
}

static int32_t tcc_spi_probe(struct udevice *bus)
{
	struct tcc_spi_priv *tccspi = dev_get_priv(bus);
	int32_t ret = 0;

	tccspi->soc_info = (struct tcc_spi_soc_info*)dev_get_driver_data(bus);

	ret = tcc_spi_parse_dt(bus);

	if (ret == 0) {
		/* reset GPSB */
		tcc_spi_reset(bus);

		if ((tccspi->soc_info->id == TCC897X) ||
		    (tccspi->soc_info->id == TCC803X) ||
		    (tccspi->soc_info->id == TCC805X) ||
		    (tccspi->soc_info->id == TCC807X)) {

			/* port configuration */
			ret = tcc_spi_set_port(bus);
		}
	}

	if (ret == 0) {
		/* Check Continuous mode */
		tcc_spi_set_ctf_mode(bus, tccspi->ctf);

		/* Set bit-width */
		tcc_spi_set_bitwidth(bus);

		if (tccspi->dma_mode) {
			/* allocate DMA buffer, dma_buf_size is GPSB_PACKET_MAX_SIZE*/
			ret = tcc_spi_alloc_dma_buf(bus);
			if (ret < 0) {
				tcc_spi_deinit_dma_buf(bus);
			}
		}
	}

	if (ret == 0) {
		if (tccspi->use_gdma) {
			/* TODO: configure GDMA */
			(void)pr_warn("[WARN][SPI] ch %d dma is not supported\n",
					tccspi->channel);
		}
	}

	return 0;
}

static int32_t tcc_spi_remove(struct udevice *bus)
{
	int32_t ret = 0;

	/* pin control - idle state */
	ret = pinctrl_select_state(bus, "idle");
	if (ret < 0) {
		(void)pr_err("[ERROR][SPI] fail to get pinctrl (idle)\n");
	} else {
		/* de-allocate DMA buffer */
		tcc_spi_deinit_dma_buf(bus);
	}

	return ret;
}

static const struct tcc_spi_soc_info tcc897x_spi_info = {
	.id = TCC897X,
};

static const struct tcc_spi_soc_info tcc803x_spi_info = {
	.id = TCC803X,
};

static const struct tcc_spi_soc_info tcc805x_spi_info = {
	.id = TCC805X,
};

static const struct tcc_spi_soc_info tcc750x_spi_info = {
	.id = TCC750X,
};

static const struct tcc_spi_soc_info tcc807x_spi_info = {
	.id = TCC807X,
};

static const struct dm_spi_ops tcc_spi_ops = {
	.set_speed = tcc_spi_set_speed,
	.set_mode  = tcc_spi_set_mode,
	.xfer      = tcc_spi_xfer,
};

static const struct udevice_id tcc_spi_ids[] = {
	{	.compatible = "telechips,tcc897x-spi",
		.data = (ulong)&tcc897x_spi_info},
	{	.compatible = "telechips,tcc803x-spi",
		.data = (ulong)&tcc803x_spi_info},
	{	.compatible = "telechips,tcc805x-spi",
		.data = (ulong)&tcc805x_spi_info},
	{	.compatible = "telechips,tcc750x-spi",
		.data = (ulong)&tcc750x_spi_info},
	{	.compatible = "telechips,tcc807x-spi",
		.data = (ulong)&tcc807x_spi_info},
	{ }
};

U_BOOT_DRIVER(tcc_spi) = {
	.name      = TCC_SPI_DRV_NAME,
	.id        = UCLASS_SPI,
	.of_match  = tcc_spi_ids,
	.ops       = &tcc_spi_ops,
	.priv_auto = (s32)sizeof(struct tcc_spi_priv),
	.probe     = tcc_spi_probe,
	.remove    = tcc_spi_remove,
};
