// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <memalign.h>
#include <cpu_func.h>
#include <clk.h>
#include <spi.h>
#include <linux/io.h>
#include <asm/dma-mapping.h>
#include <asm/arch/clock.h>
#include <asm/telechips/spi.h>
#include <dm/pinctrl.h>

static void tcc_spi_dump(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);

	pr_debug("[DEBUG][SPI] GPSB REGS DUMP [CH: %d]\n", priv->channel);
	pr_debug("[DEBUG][SPI] STAT\t: %#X\n",
			readl(priv->regs + GPSB_STAT));
	pr_debug("[DEBUG][SPI] INTEN\t: %#X\n",
			readl(priv->regs + GPSB_INTEN));
	pr_debug("[DEBUG][SPI] MODE\t: %#X\n",
			readl(priv->regs + GPSB_MODE));
	pr_debug("[DEBUG][SPI] CTRL\t: %#X\n",
			readl(priv->regs + GPSB_CTRL));
	pr_debug("[DEBUG][SPI] EVTCTRL\t: %#X\n",
			readl(priv->regs + GPSB_EVTCTRL));
	pr_debug("[DEBUG][SPI] CCV\t: %#X\n",
			readl(priv->regs + GPSB_CCV));

	pr_debug("[DEBUG][SPI] TXBASE\t: %#X\n",
			readl(priv->regs + GPSB_TXBASE));
	pr_debug("[DEBUG][SPI] RXBASE\t: %#X\n",
			readl(priv->regs + GPSB_RXBASE));
	pr_debug("[DEBUG][SPI] PACKET\t: %#X\n",
			readl(priv->regs + GPSB_PACKET));
	pr_debug("[DEBUG][SPI] DMACTR\t: %#X\n",
			readl(priv->regs + GPSB_DMACTR));
	pr_debug("[DEBUG][SPI] DMASTR\t: %#X\n",
			readl(priv->regs + GPSB_DMASTR));
	pr_debug("[DEBUG][SPI] DMAICR\t: %#X\n",
			readl(priv->regs + GPSB_DMAICR));
}

static int32_t tcc_spi_reset(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	ulong ret;

	/* enable SW RESET */
	tcc_set_iobus_swreset((int32_t)priv->hclk.id, true);
	tcc_set_iobus_swreset((int32_t)priv->hclk.id, false);

	/* enable IO BUS */
	ret = tcc_set_iobus_pwdn((int32_t)priv->hclk.id, false);
	if (ret != 0UL) {
		pr_err("[ERROR][SPI] %s: iobus not enabled\n", __func__);
		return -EIO;
	}

	return 0;
}

static void tcc_spi_set_packet(struct udevice *bus, uint32_t packet_size)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);

	/* set packet size */
	BITCSET_GPSB(priv->regs + GPSB_PACKET,
			0x1FFF,
			GPSB_PACKET_SIZE(packet_size));
	/* set packet count to 0 */
	BITCLR_GPSB(priv->regs + GPSB_PACKET, GPSB_PACKET_COUNT(0x1FFFUL));

	pr_debug("[DEBUG][SPI]%s: set packet size%d\n", __func__, packet_size);
}

static void tcc_spi_clear_packet_cnt(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);

	BITSET_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_CWF | GPSB_MODE_CRF);
	BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_CWF | GPSB_MODE_CRF);

	BITSET_GPSB(priv->regs + GPSB_DMACTR, GPSB_DMACTR_PCLR);
	BITCLR_GPSB(priv->regs + GPSB_DMACTR, GPSB_DMACTR_PCLR);

	pr_debug("[DEBUG][SPI] %s: clear packet count\n", __func__);
}

static int32_t tcc_spi_set_clk(struct udevice *bus, uint32_t sclk)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	ulong peri_clk;
	int32_t ret = -1;

	/* Check max speed */
	if (priv->max_speed < sclk) {
		pr_err("[ERROR][SPI] %s: sclk %d is faster than max speed %d\n",
				__func__, sclk, priv->max_speed);
		return ret;
	}

	priv->sclk = sclk;
	peri_clk = ((ulong)priv->sclk * 2UL);

	/* Set DIVLDV to zero */
	BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_DIVLDV(0xFFUL));

	/* Set peri clock */
	ret = (int32_t)clk_set_rate(&(priv->pclk), peri_clk);
	if (ret < 0) {
		pr_err("[ERROR][SPI] fail to set peripheral clock\n");
		return ret;
	}

	pr_debug("[DEBUG][SPI] %s: sclk %d Hz, peri clk %lu Hz\n",
			__func__, priv->sclk, clk_get_rate(&(priv->pclk)));

	return 0;
}

static uint8_t tcc_spi_set_bitwidth(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	uint32_t mode;
	uint8_t bitWidth;

	/* set bws */
	bitWidth = (priv->bpw - 1U);
	BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_BWS(31UL));
	BITSET_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_BWS(bitWidth));
	mode = readl(priv->regs + GPSB_MODE);

	/* polling mode */
	if (!priv->dma_mode) {
		BITSET_GPSB(priv->regs + GPSB_INTEN,
				(GPSB_INTEN_SHT |
				 GPSB_INTEN_SBT |
				 GPSB_INTEN_SHR |
				 GPSB_INTEN_SBR));
	} else {
		/* dma mode */
		if (!priv->use_gdma) {
			BITCLR_GPSB(priv->regs + GPSB_INTEN,
					(GPSB_INTEN_SHT |
					 GPSB_INTEN_SBT |
					 GPSB_INTEN_SHR |
					 GPSB_INTEN_SBR));
			if ((mode & GPSB_MODE_BWS(0x10UL)) != 0u) {
				/* little endian */
				BITCLR_GPSB(priv->regs + GPSB_DMACTR,
						GPSB_DMACTR_END);
			} else {
				/* Big endian */
				BITSET_GPSB(priv->regs + GPSB_DMACTR,
						GPSB_DMACTR_END);
				if (bitWidth == 16u) {
					BITSET_GPSB(priv->regs + GPSB_INTEN,
							(GPSB_INTEN_SBT |
							 GPSB_INTEN_SBR)
						   );
				}
			}
		} else {
			BITCLR_GPSB(priv->regs + GPSB_INTEN,
					(GPSB_INTEN_SHT |
					 GPSB_INTEN_SBT |
					 GPSB_INTEN_SHR |
					 GPSB_INTEN_SBR));
		}
	}
	pr_debug("[DEBUG][SPI] %s: bitwidth %d\n", __func__, bitWidth);

	return bitWidth;
}

static void tcc_spi_set_ctf_mode(struct udevice *bus, bool on)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);

	if (on) {
		BITSET_GPSB(priv->regs + GPSB_EVTCTRL,
				GPSB_EVTCTRL_CONTM(0x3UL));
		BITSET_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_CTF);
	} else {
		BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_CTF);
	}

	pr_debug("[DEBUG][SPI] %s: set CTF %d\n", __func__, on);
}

static int32_t tcc_spi_set_mode(struct udevice *bus, uint mode)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);

	if ((mode & SPI_SLAVE) > 0U) {
		/* slave mode */
		BITSET_GPSB(priv->regs+GPSB_MODE, GPSB_MODE_SLV);

		if ((mode == SPI_MODE_1) || (mode == SPI_MODE_2)) {
			BITSET_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_PCK);
		} else {
			BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_PCK);
		}
		if ((mode & SPI_CS_HIGH) > 0U) {
			BITSET_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_PCD);
		} else {
			BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_PCD);
		}

		pr_debug("[DEBUG][SPI] %s: mode %#X (mode reg: %#X)\n",
				__func__, mode, readl(priv->regs + GPSB_MODE));

		return 0;
	}
	/* master mode */
	BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_SLV);

	if ((mode & SPI_CPOL) > 0U) {
		BITSET_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_PCK);
	} else {
		BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_PCK);
	}
	if ((mode & SPI_CPHA) > 0U) {
		BITSET_GPSB(priv->regs + GPSB_MODE,
				GPSB_MODE_PWD | GPSB_MODE_PRD);
	} else {
		BITCLR_GPSB(priv->regs + GPSB_MODE,
				GPSB_MODE_PWD | GPSB_MODE_PRD);
	}
	if ((mode & SPI_CS_HIGH) > 0U) {
		BITSET_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_PCS);
	} else {
		BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_PCS);
	}
	if ((mode & SPI_LSB_FIRST) > 0U) {
		BITSET_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_SD);
	} else {
		BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_SD);
	}
	if ((mode & SPI_LOOP) > 0U) {
		BITSET_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_LB);
	} else {
		BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_LB);
	}

	pr_debug("[DEBUG][SPI] %s: mode %#X (mode reg: %#X)\n",
			__func__, mode, readl(priv->regs + GPSB_MODE));

	return 0;
}

static int32_t tcc_spi_txrx8(struct udevice *bus,
		uint32_t bytes,
		const void *dout,
		void *din)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	uint32_t data_num, i, dummy;
	ulong cnt;
	const uint8_t *tx = dout;
	uint8_t *rx = din;

	data_num = bytes;

	for (i = 0; i < data_num; i++) {
		if (tx != NULL) {
			writel(((uint32_t)(tx[i]) << 24),
					priv->regs + GPSB_PORT);
			pr_debug("[DEBUG][SPI] Tx[%d] - 0x%x\n", i, tx[i]);
		} else {
			writel(0, priv->regs + GPSB_PORT);
		}

		/* Check Rx FIFO not empty flag */
		cnt = 0;
		while ((readl(priv->regs + GPSB_STAT) & GPSB_STAT_RNE) == 0U) {
			cnt++;
			if (cnt > TCC_SPI_TIMEOUT) {
				pr_err("[ERROR][SPI] %s: spi time out occrur !! (STAT: 0x%X)\n",
						__func__,
						readl(priv->regs + GPSB_STAT));
				return -ETIME;
			}
		}

		dummy = readl(priv->regs + GPSB_PORT);

		if (rx != NULL) {
			rx[i] = (uint8_t)(dummy >> 24U);
			pr_debug("[DEBUG][SPI] Rx[%d] - 0x%x (dummy 0x%x)\n",
					i, (uint32_t)rx[i], dummy);
		}
	}

	return 0;
}

static int32_t tcc_spi_txrx16(struct udevice *bus,
		uint32_t bytes,
		const void *dout,
		void *din)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	uint32_t data_num, i, dummy;
	ulong cnt;
	const uint16_t *tx = dout;
	uint16_t *rx = din;

	if ((bytes % 2U) != 0U) {
		pr_err("[ERROR][SPI] %s: Non word aligned SPI transfer. bitwidth %d\n",
				__func__, priv->bpw);
		return -EINVAL;
	}

	data_num = bytes / 2U;

	for (i = 0; i < data_num; i++) {
		if (tx != NULL) {
			writel(((uint32_t)(tx[i]) << 16),
					priv->regs + GPSB_PORT);
			pr_debug("[DEBUG][SPI] Tx[%d] - 0x%x\n", i, tx[i]);
		} else {
			writel(0, priv->regs + GPSB_PORT);
		}

		/* Check Rx FIFO not empty flag */
		cnt = 0;
		while ((readl(priv->regs + GPSB_STAT) & GPSB_STAT_RNE) == 0U) {
			cnt++;
			if (cnt > TCC_SPI_TIMEOUT) {
				pr_err("[ERROR][SPI] %s: spi time out occrur !! (STAT: 0x%X)\n",
						__func__,
						readl(priv->regs + GPSB_STAT));
				return -ETIME;
			}
		}

		dummy = readl(priv->regs + GPSB_PORT);

		if (rx != NULL) {
			rx[i] = (uint16_t)(dummy >> 16U);
			pr_debug("[DEBUG][SPI] Rx[%d] - 0x%x (dummy 0x%x)\n",
					i, (uint32_t)rx[i], dummy);
		}
	}

	return 0;
}

static int32_t tcc_spi_txrx32(struct udevice *bus,
		uint32_t bytes,
		const void *dout,
		void *din)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	uint32_t data_num, i, dummy;
	ulong cnt;
	const uint32_t *tx = dout;
	uint32_t *rx = din;

	if ((bytes % 4U) != 0U) {
		pr_err("[ERROR][SPI] %s: Non word aligned SPI transfer. bitwidth %d\n",
				__func__, priv->bpw);
		return -EINVAL;
	}

	data_num = bytes / 4U;

	for (i = 0; i < data_num; i++) {
		if (tx != NULL) {
			writel(tx[i], priv->regs + GPSB_PORT);
			pr_debug("[DEBUG][SPI] Tx[%d] - 0x%X\n", i, tx[i]);
		} else {
			writel(0, priv->regs + GPSB_PORT);
		}

		/* Check Rx FIFO not empty flag */
		cnt = 0;
		while ((readl(priv->regs + GPSB_STAT) & GPSB_STAT_RNE) == 0U) {
			cnt++;
			if (cnt > TCC_SPI_TIMEOUT) {
				pr_err("[ERROR][SPI] %s: spi time out occrur !! (STAT: 0x%X)\n",
						__func__,
						readl(priv->regs + GPSB_STAT));
				return -ETIME;
			}
		}

		dummy = readl(priv->regs + GPSB_PORT);

		if (rx != NULL) {
			rx[i] = dummy;
			pr_debug("[DEBUG][SPI] Rx[%d] - 0x%X (dummy 0x%X)\n",
					i, rx[i], dummy);
		}
	}

	return 0;
}

static int32_t tcc_spi_xfer_pio(struct udevice *bus,
		uint32_t bytes,
		const void *dout,
		void *din)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	int32_t ret = 0;

	pr_debug("[DEBUG][SPI] %s: bytes %d dout 0x%lx dint 0x%lx bitwidth %d\n",
			__func__, bytes, (ulong)dout, (ulong)din, priv->bpw);

	tcc_spi_clear_packet_cnt(bus);
	/* enable GPSB operation */
	BITSET_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_EN);

	if (priv->bpw == 8U) {
		ret = tcc_spi_txrx8(bus, bytes, dout, din);
	} else if (priv->bpw == 16U) {
		ret = tcc_spi_txrx16(bus, bytes, dout, din);
	} else if (priv->bpw == 32U) {
		ret = tcc_spi_txrx32(bus, bytes, dout, din);
	} else {
		pr_err("[ERROR][SPI] Not supported bpw %d\n", priv->bpw);
		return -EINVAL;
	}

	/* disable GPSB operation */
	BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_EN);

	return ret;
}

static void tcc_spi_stop_dma(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);

	/* Disable GPSB DMA operation */
	BITCLR_GPSB(priv->regs + GPSB_DMACTR, GPSB_DMACTR_EN);

	/* Clear DMA done and packet interrupt status */
	BITSET_GPSB(priv->regs + GPSB_DMAICR,
			GPSB_DMAICR_ISD | GPSB_DMAICR_ISP);

	/* disable DMA Request */
	BITCLR_GPSB(priv->regs + GPSB_DMACTR,
			GPSB_DMACTR_DTE | GPSB_DMACTR_DRE);
	BITCLR_GPSB(priv->regs + GPSB_INTEN,
			GPSB_INTEN_DR | GPSB_INTEN_DW);
}

static void tcc_spi_start_dma(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);

	/* Set GPSB DMA address mode (Multiple address mode) */
	BITCLR_GPSB(priv->regs + GPSB_DMACTR, 0x3C000UL);

	/* Disable DMA packet interrupt */
	BITCLR_GPSB(priv->regs + GPSB_DMAICR, GPSB_DMAICR_IEP);

	/* Enable DMA done interrupt */
	BITSET_GPSB(priv->regs + GPSB_DMAICR, GPSB_DMAICR_IED);

	/* Set DMA Receiving interrupt */
	BITCLR_GPSB(priv->regs + GPSB_DMAICR, GPSB_DMAICR_IRQS);

	/* enable DMA Request */
	BITSET_GPSB(priv->regs + GPSB_DMACTR,
			GPSB_DMACTR_DTE | GPSB_DMACTR_DRE);
	BITSET_GPSB(priv->regs + GPSB_INTEN,
			GPSB_INTEN_DR | GPSB_INTEN_DW);

	/*enable DMA */
	BITSET_GPSB(priv->regs + GPSB_DMACTR, GPSB_DMACTR_EN);
}

static int32_t tcc_spi_check_status(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	uint32_t status = readl(priv->regs + GPSB_STAT);

	if ((status & GPSB_STAT_ERR) > 0U) {
		pr_err("[ERROR][SPI] %s: ERROR FLAG (STATUS: %#X)\n",
				__func__, status);
		tcc_spi_dump(bus);
		return -EIO;
	}
	return 0;
}

static int32_t tcc_spi_dma_handler(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	int32_t ret;

	/* disable DMA */
	tcc_spi_stop_dma(bus);
	/* check error flag */
	ret = tcc_spi_check_status(bus);
	/* clear error flag */
	BITCLR_GPSB(priv->regs + GPSB_STAT, GPSB_STAT_ERR);

	return ret;
}

static void tcc_spi_set_txrx_addr(struct udevice *bus,
		uint32_t *tx_addr,
		uint32_t *rx_addr)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);

	BITCSET_GPSB(priv->regs + GPSB_TXBASE, 0xFFFFFFFFU, (ulong)tx_addr);
	BITCSET_GPSB(priv->regs + GPSB_RXBASE, 0xFFFFFFFFU, (ulong)rx_addr);
}

static int32_t tcc_spi_alloc_dma_buf(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);

	priv->dma_buf_size = GPSB_PACKET_MAX_SIZE;

	/* allocate DMA buffer */
	(void)dma_alloc_coherent(priv->dma_buf_size, (ulong *)&priv->tx);
	if (priv->tx == NULL) {
		pr_err("[ERROR][SPI] %s: ch %d, allocate DMA buffer error for tx\n",
				__func__, priv->channel);
		return -ENOMEM;
	}

	(void)dma_alloc_coherent(priv->dma_buf_size, (ulong *)&priv->rx);
	if (priv->rx == NULL) {
		pr_err("[ERROR][SPI] %s: ch %d, allocate DMA buffer error for rx\n",
				__func__, priv->channel);
		return -ENOMEM;
	}

	/* set TX/RX BASE */
	tcc_spi_set_txrx_addr(bus, priv->tx, priv->rx);

	return 0;
}

static void tcc_spi_deinit_dma_buf(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);

	if (priv->tx != NULL) {
		(void)free(priv->tx);
	}
	if (priv->rx != NULL) {
		(void)free(priv->rx);
	}
}

static int32_t tcc_spi_xfer_dma(struct udevice *bus,
		uint32_t bytes,
		const void *dout,
		void *din)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	uint32_t len;
	const void *dst;
	void *src;
	int32_t time_error = 0;
	ulong tmo = 0;
	int32_t ret = 0;

	priv->remain_bytes = bytes;
	dst = dout;
	src = din;
	(void)memset(priv->tx, 0, priv->dma_buf_size);
	(void)memset(priv->rx, 0, priv->dma_buf_size);

	while (priv->remain_bytes > 0U) {
		/* set packet size */
		if (priv->remain_bytes >= GPSB_PACKET_MAX_SIZE) {
			len = GPSB_PACKET_MAX_SIZE;
		} else {
			len = priv->remain_bytes;
		}
		tcc_spi_set_packet(bus, len);

		/* copy dst to GPSB TX buffer */
		if (dst != NULL) {
			(void)memcpy(priv->tx, dst, len);
		}

		/* clean and invalidate the area of cache */
		(void)flush_cache((ulong)priv->tx, len);
		(void)flush_cache((ulong)priv->rx, len);

		/* Clear FIFO/PCNT */
		tcc_spi_clear_packet_cnt(bus);

		/* TODO: Setup GDMA, if use */

		/* enable DMA */
		tcc_spi_start_dma(bus);

		tcc_spi_dump(bus);
		/* enable GPSB operation */
		BITSET_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_EN);

		/* check IRQ status for Done Interrupt */
		tmo = get_timer(0UL);
		while ((readl(priv->regs + GPSB_DMAICR)
					& GPSB_DMAICR_ISD) == 0U) {
			if (get_timer(tmo) >= TCC_SPI_TIMEOUT) {
				pr_err("[ERROR][SPI] DMA Interrupt Timeout!\n");
				tcc_spi_dump(bus);
				time_error = -ETIMEDOUT;
				break;
			}
		}

		/* disable GPSB operation */
		BITCLR_GPSB(priv->regs + GPSB_MODE, GPSB_MODE_EN);

		ret = tcc_spi_dma_handler(bus);
		if ((ret < 0) || (time_error < 0)) {
			pr_err("[ERROR][SPI] %s: return %d (bytes: %d remain bytes: %d)\n",
					__func__,
					ret,
					bytes,
					priv->remain_bytes);
			return (time_error < 0) ? time_error : ret;
		}

		/* copy GPSB RX buffer to src */
		if (src != NULL) {
			(void)memcpy(src, priv->rx, len);
		}
		priv->remain_bytes -= len;
		dst += len;
		src += len;
	}

	return ret;
}

static int32_t tcc_spi_xfer(struct udevice *dev, uint32_t bitlen,
		const void *dout, void *din, ulong flags)
{
	struct udevice *bus = dev->parent;
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	uint32_t bytelen;
	int32_t ret;

	if (bitlen == 0U) {
		pr_warn("[WARN][SPI] %s: No data to transfer. bitlen %d\n",
				__func__, bitlen);
		return 0;
	}

	if ((bitlen % 8U) != 0U) {
		pr_err("[ERROR][SPI] %s: Non byte aligned - bitlen %d\n",
				__func__, bitlen);
		return -EINVAL;
	}

	bytelen = bitlen / 8U;

	if (priv->dma_mode) {
		ret = tcc_spi_xfer_dma(bus, bytelen, dout, din);
	} else {
		ret = tcc_spi_xfer_pio(bus, bytelen, dout, din);
	}

	if (ret < 0) {
		pr_err("[ERROR][SPI] %s: xfer failed!! return %d\n",
				__func__, ret);
		return ret;
	}

	return ret;
}

static int32_t tcc_spi_set_speed(struct udevice *bus, uint speed)
{
	return tcc_spi_set_clk(bus, speed);
}

static int32_t tcc_spi_set_port(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	int32_t ch = priv->channel;
	uint32_t port = priv->port;
	uint32_t pcfg_offset, pcfg_val;
	uint32_t offset;
	int32_t i;

	/* check channel and port */
	if (ch < 0) {
		pr_err("[ERROR][SPI] %s: channel(%d), port(%d) is wrong number !\n",
				__func__, ch, port);
		return -EINVAL;
	}

	/* check port conflict */
	for (i = 0; i < GPSB_CH_NUM; i++) {
		if (i < 4) {
			pcfg_offset = GPSB_PORT_SEL0;
			pcfg_val = readl(priv->pcfg + GPSB_PORT_SEL0);
			offset = (u32)i << 3;
		} else {
			pcfg_offset = GPSB_PORT_SEL1;
			pcfg_val = readl(priv->pcfg + GPSB_PORT_SEL1);
			offset = ((u32)i - 4U) << 3;
		}

		if (((pcfg_val >> offset) & 0xFFU) == port) {
			BITSET_GPSB(priv->pcfg + pcfg_offset,
					((u32)0xFFU << offset));
			pr_warn("[WARN][SPI] port%d conflict - Channel %d is clear\n",
					port, i);
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
	BITCSET_GPSB(priv->pcfg + pcfg_offset,
			((u32)0xFFU << offset),
			(port << offset));

	pr_debug("[DEBUG][SPI] %s: channel %d sets port %d\n",
			__func__, ch, port);
	pr_debug("[DEBUG][SPI] %s: port_sel0 -> 0x%X , port_sel1 -> 0x%X\n",
			__func__,
			readl(priv->pcfg + GPSB_PORT_SEL0),
			readl(priv->pcfg + GPSB_PORT_SEL0));

	return 0;
}

static int32_t tcc_spi_parse_dt(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	uint32_t ac_val[2] = {0,};
	int32_t ret;

	/* base, port configuration, access control address */
	priv->regs = (void *)dev_read_addr_index(bus, 0);
	priv->pcfg = (void *)dev_read_addr_index(bus, 1);
	priv->ac = (void *)dev_read_addr_index(bus, 2);

	/* channel */
	ret = dev_read_alias_seq(bus, &priv->channel);
	if (ret < 0) {
		pr_err("[ERROR][SPI] %s: failed to get alias id\n", __func__);
		return -EINVAL;
	}
	if (priv->channel < 0) {
		pr_err("[ERROR][SPI] %s: wrong spi channel\n", __func__);
		return -EINVAL;
	}

	/* get peripheral clock , iobus clock */
	ret = tcc_clk_get_by_index(bus, 0, &(priv->pclk));
	if (ret < 0) {
		pr_err("[ERROR][SPI] failed to get peripheral clock\n");
		return ret;
	}
	ret = tcc_clk_get_by_index(bus, 1, &(priv->hclk));
	if (ret < 0) {
		pr_err("[ERROR][SPI] fail to get iobus clock\n");
		return ret;
	}

	/* port number */
	ret = dev_read_u32(bus, "gpsb-port", &priv->port);
	if (ret < 0) {
		pr_err("[ERROR][SPI] There is no port information.\n");
		return -EINVAL;
	}
	ret = pinctrl_select_state(bus, "active");
	if (ret < 0) {
		pr_err("[ERROR][SPI] fail to get pinctrl (active)\n");
		return -EINVAL;
	}

	/* get max speed, if no value, default 10MHz */
	priv->max_speed = (uint32_t)dev_read_u32_default(bus,
			"spi-max-frequency",
			10000000);
	priv->bpw = (uint8_t)dev_read_u32_default(bus, "bpw", 32);
	if (priv->bpw > 32U) {
		pr_err("[ERROR][SPI] %s: not supported bpw %d\n",
				__func__, priv->bpw);
		return -EINVAL;
	}
	priv->ctf = !(dev_read_bool(bus, "ctf-mode-disable"));
	priv->dma_mode = dev_read_bool(bus, "dma-mode");

	priv->use_gdma = false;
	if (priv->dma_mode && (priv->channel > 2)) {
		priv->use_gdma = true;
		pr_err("[WARN][SPI] %s: not ready to support GPSB ch %d with dma mode\n",
					 __func__, priv->channel);
		priv->dma_mode = false;
	}

	/* access control : Configure the AHB Access Filter */
	if (priv->dma_mode) {
		ret = dev_read_u32_array(bus, "access-control0", ac_val, 2);
		if (ret == 0) {
			pr_debug("[DEBUG][SPI] access-control0 start:%#x limit: %#x\n",
					ac_val[0], ac_val[1]);
			writel(ac_val[0], priv->ac + GPSB_AC0_START);
			writel(ac_val[1], priv->ac + GPSB_AC0_LIMIT);
		}

		ret = dev_read_u32_array(bus, "access-control1", ac_val, 2);
		if (ret == 0) {
			pr_debug("[DEBUG][SPI] access-control1 start:%#x limit:%#x\n",
					ac_val[0], ac_val[1]);
			writel(ac_val[0], priv->ac + GPSB_AC1_START);
			writel(ac_val[1], priv->ac + GPSB_AC1_LIMIT);
		}

		ret = dev_read_u32_array(bus, "access-control2", ac_val, 2);
		if (ret == 0) {
			pr_debug("[DEBUG][SPI] access-control2 start:%#x limit:%#x\n",
					ac_val[0], ac_val[1]);
			writel(ac_val[0], priv->ac + GPSB_AC2_START);
			writel(ac_val[1], priv->ac + GPSB_AC2_LIMIT);
		}

		ret = dev_read_u32_array(bus, "access-control3", ac_val, 2);
		if (ret == 0) {
			pr_debug("[DEBUG][SPI] access-control3 start:%#x limit:%#x\n",
					ac_val[0], ac_val[1]);
			writel(ac_val[0], priv->ac + GPSB_AC3_START);
			writel(ac_val[1], priv->ac + GPSB_AC3_LIMIT);
		}
	}

	return 0;
}

static int32_t tcc_spi_probe(struct udevice *bus)
{
	struct tcc_spi_priv *priv = dev_get_priv(bus);
	int32_t ret;

	ret = tcc_spi_parse_dt(bus);
	if (ret < 0) {
		return ret;
	}

	/* reset GPSB */
	ret = tcc_spi_reset(bus);
	if (ret != 0) {
		return ret;
	}

	/* port configuration */
	ret = tcc_spi_set_port(bus);
	if (ret < 0) {
		return ret;
	}

	/* Check Continuous mode */
	tcc_spi_set_ctf_mode(bus, priv->ctf);

	/* Set bit-width */
	tcc_spi_set_bitwidth(bus);

	if (priv->dma_mode) {
		/* allocate DMA buffer, dma_buf_size is GPSB_PACKET_MAX_SIZE*/
		ret = tcc_spi_alloc_dma_buf(bus);
		if (ret < 0) {
			tcc_spi_deinit_dma_buf(bus);
			return ret;
		}
	}

	if (priv->use_gdma) {
		/* TODO: configure GDMA */
		pr_warn("[WARN][SPI] ch %d dma is not supported\n",
				priv->channel);
	}

	return 0;
}

static int32_t tcc_spi_remove(struct udevice *bus)
{
	int32_t ret;

	/* pin control - idle state */
	ret = pinctrl_select_state(bus, "idle");
	if (ret < 0) {
		pr_err("[ERROR][SPI] fail to get pinctrl (idle)\n");
		return ret;
	}

	/* de-allocate DMA buffer */
	tcc_spi_deinit_dma_buf(bus);

	return 0;
}

static const struct dm_spi_ops tcc_spi_ops = {
	.set_speed = tcc_spi_set_speed,
	.set_mode  = tcc_spi_set_mode,
	.xfer      = tcc_spi_xfer,
};

static const struct udevice_id tcc_spi_ids[] = {
	{ .compatible = "telechips,tcc803x-spi" },
	{ .compatible = "telechips,tcc805x-spi" },
	{ }
};

U_BOOT_DRIVER(tcc_spi) = {
	.name	= TCC_SPI_DRV_NAME,
	.id	= UCLASS_SPI,
	.of_match = tcc_spi_ids,
	.ops	= &tcc_spi_ops,
	.priv_auto_alloc_size = sizeof(struct tcc_spi_priv),
	.probe	= tcc_spi_probe,
	.remove = tcc_spi_remove,
};
