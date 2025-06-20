// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

/*
 * Designware ethernet IP driver for U-Boot
 */

#include <common.h>
#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/devres.h>
#include <errno.h>
#include <miiphy.h>
#include <malloc.h>
#include <pci.h>
#include <reset.h>
#include <linux/compiler.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <power/regulator.h>
#include <linux/delay.h>
#include <asm/system.h>

#include "dwmac4.h"

struct dw_eth_dev *global_dw_eth_dev;
struct eth_pdata * global_eth_pdata;
struct dmamacdescr *tx_mac_descrtable;
struct dmamacdescr *rx_mac_descrtable;

void tcc_inval_buffer(void *buf, size_t size)
{
	unsigned long start = rounddown((unsigned long)buf, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)buf + size,
								ARCH_DMA_MINALIGN);
	invalidate_dcache_range(start, end);
}

void tcc_inval_desc(void *desc)
{
#ifndef CONFIG_SYS_NONCACHED_MEMORY
	unsigned long start = rounddown((unsigned long)desc, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)desc + 16,
								ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
#endif
}

void tcc_flush_buffer(void *buf, size_t size)
{
	unsigned long start = rounddown((unsigned long)buf, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)buf + size,
								ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
}

void tcc_flush_desc(void *desc)
{
#ifndef CONFIG_SYS_NONCACHED_MEMORY
	unsigned long start = rounddown((unsigned long)desc, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)desc + 16,
								ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
#endif
}

void tcc_clr_buffer(void *addr, u32 size)
{
	memset(addr, 0x0, size);
	tcc_flush_buffer(addr, size);
}

static int dw_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	u32 val;
	u32 start;
	u32 miiaddr = MII_BUSY;
	struct dw_eth_dev *priv = bus->priv;

	miiaddr |= ((addr << ETH_MAC_MDIO_ADDR_SHIFT) & ETH_MAC_MDIO_ADDR_MASK);
	miiaddr |= ((reg << ETH_MAC_MDIO_REG_SHIFT) & ETH_MAC_MDIO_REG_MASK);
	// miiaddr |= ((CSR_150_250M << ETH_MAC_MDIO_CSR_SHIFT) & ETH_MAC_MDIO_CSR_MASK);
	miiaddr |= ((CSR_250_300M << ETH_MAC_MDIO_CSR_SHIFT) & ETH_MAC_MDIO_CSR_MASK);
	miiaddr |= MII_GMAC4_READ;

	writel(miiaddr, priv->mac_base + ETH_MAC_MDIO_ADDR);

	start = get_timer(0);
	while (get_timer(start) < CONFIG_MDIO_TIMEOUT) {
		if (!(readl(priv->mac_base + ETH_MAC_MDIO_ADDR) & MII_BUSY)) {
			val = readl(priv->mac_base + ETH_MAC_MDIO_DATA);
			return val;
		}
		udelay(10);
	};

	return -ETIMEDOUT;
}

static int dw_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
						 u16 val)
{
	u32 start;
	int ret = -ETIMEDOUT;
	u32 miiaddr = MII_BUSY;
	struct dw_eth_dev *priv = bus->priv;

	miiaddr |= ((addr << ETH_MAC_MDIO_ADDR_SHIFT) & ETH_MAC_MDIO_ADDR_MASK);
	miiaddr |= ((reg << ETH_MAC_MDIO_REG_SHIFT) & ETH_MAC_MDIO_REG_MASK);
	// miiaddr |= ((CSR_150_250M << ETH_MAC_MDIO_CSR_SHIFT) & ETH_MAC_MDIO_CSR_MASK);
	miiaddr |= ((CSR_250_300M << ETH_MAC_MDIO_CSR_SHIFT) & ETH_MAC_MDIO_CSR_MASK);
	miiaddr |= MII_GMAC4_WRITE;

	writel(val, priv->mac_base + ETH_MAC_MDIO_DATA);
	writel(miiaddr, priv->mac_base + ETH_MAC_MDIO_ADDR);
	start = get_timer(0);
	while (get_timer(start) < CONFIG_MDIO_TIMEOUT) {
		if (!(readl(priv->mac_base + ETH_MAC_MDIO_ADDR) & MII_BUSY)) {
			ret = 0;
			break;
		}
		udelay(10);
	}

	return ret;
}

void tx_descs_init(struct dw_eth_dev *priv)
{
	struct dmamacdescr *tx_desc;
	
	for (int i = 0; i < ETH_TDES_NUM; i++) {
		tx_desc = tx_mac_descrtable + i;
		tx_desc->des0 = (u32)((u64)priv->txbuffs[i * BUFF_SIZE] & PADDR_BIT_MASK);
		tx_desc->des1 = 0;
		tx_desc->des2 = 0;
		tx_desc->des3 = 0;
		tcc_flush_desc(tx_desc);
		debug_cond(DEBUG_DESC, "[%s] desc_start : %p ,buff addr : %p , idx :%d \n",
				   __func__, tx_desc, &priv->txbuffs[i * BUFF_SIZE], i);
	}
	tcc_clr_buffer(priv->txbuffs , BUFF_SIZE * ETH_TDES_NUM);
	priv->tx_currdescnum = 0;
}

void rx_descs_init(struct dw_eth_dev *priv)
{
	struct dmamacdescr *rx_descs = rx_mac_descrtable;
	struct dmamacdescr *rx_desc;

	/* Before passing buffers to GMAC we need to make sure zeros
	 * written there right after "priv" structure allocation were
	 * flushed into RAM.
	 * Otherwise there's a chance to get some of them flushed in RAM when
	 * GMAC is already pushing data to RAM via DMA. This way incoming from
	 * GMAC data will be corrupted.
	 */
	for (int i = 0; i < ETH_RDES_NUM; i++) {
		rx_desc = rx_descs + i;
		rx_desc->des0 = (u32)((u64)&priv->rxbuffs[i * BUFF_SIZE] & PADDR_BIT_MASK);
		rx_desc->des1 = 0;
		rx_desc->des2 = 0;
		rx_desc->des3 = RDES3_OWN | RDES3_BUFFER1_VALID_ADDR;
		debug_cond(DEBUG_DESC, "[%s] desc_start : %p ,buff addr : %p , idx :%d \n",
				   __func__, rx_desc, &priv->rxbuffs[i * BUFF_SIZE], i);
		tcc_flush_desc(rx_desc);
	}
	tcc_clr_buffer((void *)priv->rxbuffs, BUFF_SIZE * ETH_RDES_NUM);
	priv->rx_currdescnum =0;
}

static void dw_dma_rst(struct dw_eth_dev *priv)
{
	u32 value;
    u32 limit;
	
    limit = 100;
	value = readl(priv->mac_base + DMA_BUS_MODE);
	/* DMA SW reset */
    value |= DMA_BUS_MODE_SFT_RESET;
	writel(value, priv->mac_base + DMA_BUS_MODE);

    while (limit--){
        if (!(readl(priv->mac_base + DMA_BUS_MODE) & DMA_BUS_MODE_SFT_RESET)){
            break;
        }
		udelay(10);
    }
	debug_cond(DEBUG_CONF, "[%s]dma sw reset %s! \n",
			   __func__, (limit == 0 ? "fail" : "success"));
}

static void dw_dma_init(struct dw_eth_dev *priv)
{
    u32 value;

	//value = readl(priv->mac_base + DMA_SYS_BUS_MODE);
	value = DMA_AXI_BLEN4 | DMA_AXI_BLEN8 | DMA_AXI_BLEN16;
	value |= DMA_SYS_ENAME;
	value |= 2 << DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT;

	writel(value, priv->mac_base + DMA_SYS_BUS_MODE);
}

static void dw_dma_channel_init(struct dw_eth_dev *priv)
{
	//TODO: DMA_BUS_MODE_PBL�� �´°ǰ�?
	writel(DMA_BUS_MODE_PBL, priv->mac_base + DMA_CHAN_CONTROL(0));
}

static void dw_core_init(struct dw_eth_dev *priv, u32 mode)
{
    u32 value;
	phys_addr_t mac_base = priv->mac_base;

    value = ETH_DEFAULT_CORE_INIT;
    if (mode == ETH_MODE_MAC_LOOPBACK) {
        value |= ETH_MAC_CONFIG_LM;
    }

	writel(value, mac_base);
	debug_cond(DEBUG_CONF, "[%s] mac core : %08x\n", __func__, readl(mac_base));
}

void update_descs(struct dw_eth_dev *priv)
{
	u32 rx_desc_addr = (u32)((u64)rx_mac_descrtable & PADDR_BIT_MASK);
	u32 tx_desc_addr = (u32)((u64)tx_mac_descrtable & PADDR_BIT_MASK);
	u32 rx_tail_desc_addr = rx_desc_addr + (ETH_RDES_NUM * sizeof(struct dmamacdescr));

	/* Set Tx/Rx Descriptor Base Address*/
    writel(rx_desc_addr, priv->mac_base + DMA_CHAN_RX_BASE_ADDR(0));
    writel(tx_desc_addr, priv->mac_base + DMA_CHAN_TX_BASE_ADDR(0));

    /* Set Tx/Rx Ring Length */
    writel((ETH_RDES_NUM - 1), priv->mac_base + DMA_CHAN_RX_RING_LEN(0));
    writel((ETH_TDES_NUM - 1), priv->mac_base + DMA_CHAN_TX_RING_LEN(0));

	/* Set Rx Tail Descriptor Address */
	/*
	 * Point RX tail pointer at last descriptor. Ideally, we'd point at the
	 * first descriptor, implying all descriptors were available. However,
	 * that's not distinguishable from none of the descriptors being
	 * available.
	 */
    writel(rx_tail_desc_addr, priv->mac_base + DMA_CHAN_RX_END_ADDR(0));

	debug_cond(DEBUG_DESC, "[%s] DMA_CHAN_RX_BASE_ADDR  : 0x%x\n", __func__, rx_desc_addr);
	debug_cond(DEBUG_DESC, "[%s] DMA_CHAN_TX_BASE_ADDR  : 0x%x\n", __func__, tx_desc_addr);
	debug_cond(DEBUG_DESC, "[%s] DMA_CHAN_RX_END_ADDR 	: 0x%x\n", __func__, rx_tail_desc_addr);
}

void dw_desc_init(struct dw_eth_dev *priv)
{
	rx_descs_init(priv);
	tx_descs_init(priv);
	update_descs(priv);
}

static void dw_set_op_mode(struct dw_eth_dev *priv)
{
	u32 rq_op_val;
	u32 tq_op_val;
    u32 tx_fifo_sz, rx_fifo_sz;
	u32 tqs, rqs;
	u32 pbl;
	u32 val;

	val = readl(priv->mac_base + ETH_MAC_HW_FEATURE1);

	tx_fifo_sz = (val & GMAC_HW_TXFIFOSIZE_MASK) >> GMAC_HW_TXFIFOSIZE_SHIFT;
	rx_fifo_sz = (val & GMAC_HW_RXFIFOSIZE_MASK) >> GMAC_HW_RXFIFOSIZE_SHIFT;
	tqs = (128 << tx_fifo_sz) / 256 - 1;
	rqs = (128 << rx_fifo_sz) / 256 - 1;

    rq_op_val = 0;
    rq_op_val |= (rqs << MTL_OP_MODE_RQS_SHIFT) & MTL_OP_MODE_RQS_MASK;
    rq_op_val |= MTL_OP_MODE_RSF;
	writel(rq_op_val, priv->mac_base + ETH_MAC_RXQ0_OP_MODE);

    tq_op_val = 0;
    tq_op_val |= (tqs << MTL_OP_MODE_TQS_SHIFT) & MTL_OP_MODE_TQS_MASK;
    tq_op_val |= MTL_OP_MODE_TXQEN;
    tq_op_val |= MTL_OP_MODE_TSF;
    writel(tq_op_val, priv->mac_base + ETH_MAC_TXQ0_OP_MODE);

	/* Flow control used only if each channel gets 4KB or more FIFO */
	if(rqs >= ((4096 / 256) - 1)) {
		u32 rfd, rfa;
		rq_op_val |= MTL_OP_MODE_EHFC;
		/*
		 * Set Threshold for Activating Flow Contol space for min 2
		 * frames ie, (1500 * 1) = 1500 bytes.
		 *
		 * Set Threshold for Deactivating Flow Contol for space of
		 * min 1 frame (frame size 1500bytes) in receive fifo
		 */
		if (rqs == ((4096 / 256) - 1)) {
			/*
			 * This violates the above formula because of FIFO size
			 * limit therefore overflow may occur inspite of this.
			 */
			rfd = 0x3; /* Full-3K */
			rfa = 0x1; /* Full-1.5K */
		} else if (rqs == ((8192 / 256) - 1)) {
			rfd = 0x6; /* Full-4K */
			rfa = 0xa; /* Full-6K */
		} else if (rqs == ((16384 / 256) - 1)) {
			rfd = 0x6;	/* Full-4K */
			rfa = 0x12; /* Full-10K */
		} else {
			rfd = 0x6;	/* Full-4K */
			rfa = 0x1E; /* Full-16K */
		}

		rq_op_val |= (rfd << MTL_OP_MODE_RFD_SHIFT) & MTL_OP_MODE_RFD_MASK;
    	rq_op_val |= (rfa << MTL_OP_MODE_RFA_SHIFT) & MTL_OP_MODE_RFA_MASK;
		writel(rq_op_val, priv->mac_base + ETH_MAC_RXQ0_OP_MODE);
	}
	// DMA Channel operation mode
    // Set PBL
	pbl = tqs + 1;
	if (pbl > 32) {
		pbl = 32;
	}
	writel((pbl << DMA_BUS_MODE_PBL_SHIFT), priv->mac_base + DMA_CHAN_TX_CONTROL(0));
	writel((DEFAULT_DMA_PBL << DMA_BUS_MODE_RPBL_SHIFT), priv->mac_base + DMA_CHAN_RX_CONTROL(0));
	
	// RXQ0-2EN ( EN_AV )
    writel(0x2,  priv->mac_base + ETH_MAC_RXQ_CTRL0);

	debug_cond(DEBUG_CONF, "[%s] ETH_MAC_RXQ0_OP_MODE	: 0x%x\n",
			   __func__, readl(priv->mac_base + ETH_MAC_RXQ0_OP_MODE));
	debug_cond(DEBUG_CONF, "[%s] ETH_MAC_TXQ0_OP_MODE	: 0x%x\n",
			   __func__, readl(priv->mac_base + ETH_MAC_TXQ0_OP_MODE));
	debug_cond(DEBUG_CONF, "[%s] ETH_MAC_RXQ_CTRL0		: 0x%x\n",
			   __func__, readl(priv->mac_base + ETH_MAC_RXQ_CTRL0));
	debug_cond(DEBUG_CONF, "[%s] DMA_CHAN_TX_CONTROL	: 0x%x\n",
			   __func__, readl(priv->mac_base + DMA_CHAN_TX_CONTROL(0)));
	debug_cond(DEBUG_CONF, "[%s] DMA_CHAN_RX_CONTROL	: 0x%x\n",
			   __func__, readl(priv->mac_base + DMA_CHAN_RX_CONTROL(0)));
}

static void dw_set_filter(struct dw_eth_dev *priv)
{
	writel(ETH_PKT_RECV_ALL, priv->mac_base + ETH_MAC_PACKET_FILTER);
	debug_cond(DEBUG_CONF, "[%s] ETH_MAC_PACKET_FILTER	: 0x%x\n",
			   __func__, readl(priv->mac_base + ETH_MAC_PACKET_FILTER));
}

static void dw_mac_enable(struct dw_eth_dev *priv)
{
	u32 value;

	value = readl(priv->mac_base + ETH_MAC_CONFIG);
	value |= (ETH_MAC_CONFIG_RE) | (ETH_MAC_CONFIG_TE);

	writel(value, priv->mac_base + ETH_MAC_CONFIG);
	debug_cond(DEBUG_CONF, "[%s] ETH_MAC_CONFIG	: 0x%x\n",
			   __func__, readl(priv->mac_base + ETH_MAC_CONFIG));
}



void dw_dma_start_tx(struct dw_eth_dev *priv, u32 chan)
{
    u32 value;

    value = readl(priv->mac_base + DMA_CHAN_TX_CONTROL(chan));
    value |= DMA_CONTROL_ST;
    writel(value, priv->mac_base + DMA_CHAN_TX_CONTROL(chan));

    value = readl(priv->mac_base + DMA_CHAN_INTR_ENA(chan));
    value |= DMA_CHAN_INTR_DEFAULT_MASK_4_10;
    writel(value, priv->mac_base + DMA_CHAN_INTR_ENA(chan));
	debug_cond(DEBUG_CONF, "[%s] DMA_CHAN_INTR_ENA(%d)	: 0x%x\n",
			   __func__, chan, readl(priv->mac_base + DMA_CHAN_INTR_ENA(chan)));
}

void dw_dma_start_rx(struct dw_eth_dev *priv, u32 chan)
{
	u32 value;

	// Start RX DMA
	value = readl(priv->mac_base + DMA_CHAN_RX_CONTROL(chan));
	value |= DMA_CONTROL_SR;
	writel(value, priv->mac_base + DMA_CHAN_RX_CONTROL(chan));
}

static int dw_mdio_init(const char *name, void *priv)
{
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printk("[%s] Failed to allocate MDIO bus\n", __func__);
		return -ENOMEM;
	}

	bus->read = dw_mdio_read;
	bus->write = dw_mdio_write;

	snprintf(bus->name, sizeof(bus->name), "%s", name);

	bus->priv = priv;

	return mdio_register(bus);
}

static int _dw_write_hwaddr(struct dw_eth_dev *priv, u8 *mac_id)
{
	u32 macid_lo, macid_hi;

	macid_lo = mac_id[0] + (mac_id[1] << 8) + (mac_id[2] << 16) +
			   (mac_id[3] << 24);
	macid_hi = mac_id[4] + (mac_id[5] << 8);

	writel(macid_hi, priv->mac_base + ETH_MAC_ADDR_HIGH(0));
	writel(macid_lo, priv->mac_base + ETH_MAC_ADDR_LOW(0));

	return 0;
}

static int dw_adjust_link(struct dw_eth_dev *priv, struct phy_device *phydev)
{
	u32 ctrl = readl(priv->mac_base + MAC_CTRL_REG);

	if (!phydev->link) {
		printk("[%s] %s: No link.\n", __func__, phydev->dev->name);
		return 0;
	}

	switch (phydev->speed) {
		case SPEED_1000:
			ctrl &= ~(ETH_MAC_CONFIG_FES | ETH_MAC_CONFIG_PS);
			break;
		case SPEED_100:
			ctrl |= (ETH_MAC_CONFIG_FES | ETH_MAC_CONFIG_PS);
			break;
		case SPEED_10:
			ctrl |= ETH_MAC_CONFIG_PS;
			break;
	}

	if (phydev->duplex) {
		ctrl |= ETH_MAC_CONFIG_DM;
	} else {
		ctrl &= ~ETH_MAC_CONFIG_DM;
	}

	writel(ctrl, priv->mac_base + MAC_CTRL_REG);

	debug_cond(DEBUG_CONF, "[%s] MAC Control Register 0x%x\n", __func__, ctrl);
	debug_cond(DEBUG_CONF, "[%s] Speed: %d, %s duplex%s\n",
			   __func__,
			   phydev->speed,
			   (phydev->duplex) ? "full" : "half",
			   (phydev->port == PORT_FIBRE) ? ", fiber mode" : "");

	return 0;
}

static void dw_mac_disable(struct dw_eth_dev *priv)
{
    u32 value;
	
    value = readl(priv->mac_base + ETH_MAC_CONFIG);
    value &= ~((ETH_MAC_CONFIG_RE) | (ETH_MAC_CONFIG_TE));

    writel(value, priv->mac_base + ETH_MAC_CONFIG);
}

static void dw_dma_stop_tx(struct dw_eth_dev *priv, u32 chan)
{
    u32 value;

    // Stop TX DMA
    value = readl(priv->mac_base + DMA_CHAN_TX_CONTROL(chan));
    value &= ~DMA_CONTROL_ST;
    writel(value, priv->mac_base + DMA_CHAN_TX_CONTROL(chan));

    // Interrupt Disable
    value = 0;
    writel(value, priv->mac_base + DMA_CHAN_INTR_ENA(chan));
}

static void dw_dma_stop_rx(struct dw_eth_dev *priv, u32 chan)
{
    u32 value;

    // Stop RX DMA
	value = readl(priv->mac_base + DMA_CHAN_RX_CONTROL(chan));
	value &= ~DMA_CONTROL_SR;
	writel(value, priv->mac_base + DMA_CHAN_RX_CONTROL(chan));
}

static void dw_drain_all_rx_packet(struct dw_eth_dev *priv, u32 chan)
{
	int i;

	/* Wait for all RX packets to drain out of MTL */
	for (i = 0; i < 1000000; i++) {
		u32 val = readl(priv->mac_base + MTL_RXQ0_MTR0D);
		u32 prxq = (val >> MTL_RXQ0_DEBUG_PRXQ_SHIFT) &
				   MTL_RXQ0_DEBUG_PRXQ_MASK;
		u32 rxqsts = (val >> MTL_RXQ0_DEBUG_RXQSTS_SHIFT) &
					 MTL_RXQ0_DEBUG_RXQSTS_MASK;
		if ((!prxq) && (!rxqsts)) {
			break;
		}
	}
}

static void dw_drain_all_tx_packet(struct dw_eth_dev *priv, u32 chan)
{
	int i;
	/* Wait for TX all packets to drain out of MTL */
	for (i = 0; i < 1000000; i++) {
		u32 val = readl(priv->mac_base + MTL_TXQ0_MTT0D);
		u32 trcsts = (val >> MTL_TXQ0_DEBUG_TRCSTS_SHIFT) &
					 MTL_TXQ0_DEBUG_TRCSTS_MASK;
		u32 txqsts = val & MTL_TXQ0_DEBUG_TXQSTS;
		if ((trcsts != 1) && (!txqsts)) {
			break;
		}
	}
}

static void _dw_eth_halt(struct dw_eth_dev *priv)
{
	/* Disable DMA Rx/Tx */
	dw_mac_disable(priv);

	/* Stop DMA Tx */
	dw_dma_stop_tx(priv, 0);

	/* Drain All Tx Packet */
	dw_drain_all_tx_packet(priv,0);

	/* Stop DMA Rx */
	dw_dma_stop_rx(priv, 0);

	/* Drain All Rx Packet */
	dw_drain_all_rx_packet(priv,0);

	/* Shutdown Phy */
	phy_shutdown(priv->phydev);
}

int designware_eth_init(struct dw_eth_dev *priv, u8 *enetaddr)
{
	int ret = 0;

	memset(tx_mac_descrtable, 0, sizeof(struct dmamacdescr)*DWMAC_DESCRIPTORS_SIZE);

	/* Initialize DMA */
	dw_dma_rst(priv);
	dw_dma_init(priv);
	dw_dma_channel_init(priv);

	/* Initialize MAC Core */
	dw_core_init(priv, ETH_MODE_NORMAL);

	/* Initialize MAC Rx/Tx Descriptor */
	dw_desc_init(priv);

	/* Set Rx/Tx MTL Operation Mode */
    dw_set_op_mode(priv);

	/* Set Packet Filter */
    dw_set_filter(priv);

	/* Enable DMA Rx/Tx */
	dw_mac_enable(priv);

	/* Start DMA Rx/Tx */
    dw_dma_start_tx(priv, 0);
    dw_dma_start_rx(priv, 0);

	/* Start up the PHY */
	ret = phy_startup(priv->phydev);
	if (ret) {
		printf("Could not initialize PHY %s\n",
		       priv->phydev->dev->name);
		return ret;
	}

	ret = dw_adjust_link(priv, priv->phydev);


	return ret;
}

int designware_eth_enable(struct dw_eth_dev *priv)
{
	if (!priv->phydev->link){
		return -EIO;
	}
	/* Enable MAC */	
    dw_mac_enable(priv);
	return 0;
}

static int designware_eth_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct dw_eth_dev *priv = dev_get_priv(dev);
	int ret;

	ret = designware_eth_init(priv, pdata->enetaddr);
	if (ret)
		return ret;
	ret = designware_eth_enable(priv);
	if (ret)
		return ret;

	return 0;
}

#define ETH_ZLEN	60

void dump_packet(uint8_t *buf, uint32_t len, uint32_t dir)
{
    uint32_t cnt;

	printk("=====================%s==================\n", dir == DEBUG_PKT_RX ? "RX" : "TX");
	printk("%s , len = %d byte, buf addr: %p , %s \n",
		   dir == 0 ? "rx" : "tx", len, buf, dir == 0 ? "rx" : "tx");
	printk("\n");
	for (cnt = 0; cnt < len; cnt++) {
		if ((cnt % 16) == 0) {
			printk("\n %03x:", cnt);
		}
		printk(" %02x", buf[cnt]);
	}
	printk("\n");
	printk("=========================================\n");
}

static int _dw_eth_send(struct dw_eth_dev *priv, void *packet, int length)
{
	u32 desc_num = priv->tx_currdescnum;	
	struct dmamacdescr *tx_desc;
	void *buffer;
	int i;

	/* Set Current Descriptor Address */
	tx_desc = tx_mac_descrtable + desc_num;
	/* Set Current Tx Buffer Address */
	buffer = (void *)(&priv->txbuffs[desc_num * BUFF_SIZE]);
	tcc_inval_desc(tx_desc);

	if (tx_desc->des3 & TDES3_OWN) {
		printk("CPU not owner of tx frame\n");
		return -EPERM;
	}

	tcc_inval_buffer(buffer, length);

	memcpy(buffer, packet, length);
	if (length < ETH_ZLEN) {
		memset(&((char *)buffer)[length], 0, ETH_ZLEN - length);
		length = ETH_ZLEN;
	}

	/* Flush data to be sent */
	tcc_flush_buffer(buffer, length);

	tx_desc->des0 = (ulong)buffer;
	tx_desc->des1 = 0;
	tx_desc->des2 = length;
	mb();
	tx_desc->des3 = TDES3_OWN | TDES3_FIRST_DESCRIPTOR | TDES3_LAST_DESCRIPTOR;
	tx_desc->des3 |= length; 

	/* Flush modified buffer descriptor */
	tcc_flush_desc(tx_desc);

	if (++desc_num >= ETH_TDES_NUM) { 
		desc_num = 0;
	}
	priv->tx_currdescnum = desc_num;

#if DEBUG_PKT
	dump_packet((uint8_t *)buffer, length, DEBUG_PKT_TX);
#endif
	/* Start the transmission */
	debug_cond(DEBUG_COMM, "[%s] length : %d\n", __func__, length);
	writel((ulong)(tx_desc + 1), priv->mac_base + DMA_CHAN_TX_END_ADDR(0));

	for (i = 0; i < 1000000; i++) {
		tcc_inval_desc(tx_desc);
		if (!(readl(&tx_desc->des3) & TDES3_OWN))
			return 0;
		udelay(1);
	}
	debug_cond(DEBUG_COMM, "[%s] TX timeout!\n", __func__);

	return -ETIMEDOUT;
}

int _dw_eth_recv(struct dw_eth_dev *priv, uchar **packetp)
{
	u32 desc_num;
	struct dmamacdescr *rx_desc;
	int length = -EAGAIN;
	void *rx_buff;

	/*Set Current Descriptor Address*/
	desc_num = priv->rx_currdescnum;
	rx_desc = rx_mac_descrtable + desc_num;

	/*Set Current Rx Buffer Address*/
	rx_buff = &priv->rxbuffs[desc_num * BUFF_SIZE];
	
	/* Invalidate Current Descriptor */
	tcc_inval_desc((void *)rx_desc);

	/* Check  if the owner is the CPU */
	if (!(rx_desc->des3 & RDES3_OWN)) {
		length = (rx_desc->des3) & RDES3_PACKET_SIZE_MASK;
		debug_cond(DEBUG_COMM, "[%s] length : %d\n", __func__, length);
		/* Invalidate received data */
		tcc_inval_buffer(rx_buff, length);
#if DEBUG_PKT
		dump_packet(rx_buff, length, DEBUG_PKT_RX);
#endif
		*packetp = (uchar *)(rx_buff);
	}

	return length;
}

static int _dw_free_pkt(struct dw_eth_dev *priv)
{
	u32 desc_num = priv->rx_currdescnum;
	struct dmamacdescr *desc = &rx_mac_descrtable[desc_num];
	void *buff = (void *)(&priv->rxbuffs[desc_num * BUFF_SIZE]);

	/*
	 * Make the current descriptor valid again and go to
	 * the next one
	 */

	tcc_clr_buffer(buff, BUFF_SIZE);
	desc->des0 = (u32)((u64)buff & PADDR_BIT_MASK);
	desc->des1 = 0;
	desc->des2 = 0;
	/*
	 * Make sure that if HW sees the _OWN write below, it will see all the
	 * writes to the rest of the descriptor too.
	 */
	mb();
	desc->des3 = RDES3_OWN | RDES3_BUFFER1_VALID_ADDR;
	tcc_flush_desc((void *)desc);

	if (++desc_num >= ETH_RDES_NUM) {
		desc_num = 0;
	}

	priv->rx_currdescnum = desc_num;
	writel((ulong)desc, priv->mac_base + DMA_CHAN_RX_END_ADDR(0));

	return 0;
}

static int dw_phy_init(struct dw_eth_dev *priv, void *dev)
{
	struct phy_device *phydev;
	int phy_addr = -1, ret;

#ifdef CONFIG_PHY_ADDR
	phy_addr = CONFIG_PHY_ADDR;
#endif

	phydev = phy_connect(priv->bus, phy_addr, dev, priv->interface);
	if (!phydev)
		return -ENODEV;

	phydev->supported &= PHY_GBIT_FEATURES;
	if (priv->max_speed) {
		ret = phy_set_supported(phydev, priv->max_speed);
		if (ret)
			return ret;
	}
	phydev->advertising = phydev->supported;

	priv->phydev = phydev;
	phy_config(phydev);

	return 0;
}

int designware_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct dw_eth_dev *priv = dev_get_priv(dev);
	
	global_eth_pdata = (struct eth_pdata *)pdata;
	global_dw_eth_dev = (struct dw_eth_dev *)priv;

	u32 iobase = pdata->iobase;
	ulong ioaddr;
	int ret, err;
#ifdef CONFIG_CLK
	int i, clock_nb;

	tx_mac_descrtable = (struct dmamacdescr *)noncached_alloc(DWMAC_DESCRIPTORS_SIZE, 64);
	rx_mac_descrtable = tx_mac_descrtable + ETH_TDES_NUM;
// 
	memset(tx_mac_descrtable, 0, sizeof(struct dmamacdescr)*DWMAC_DESCRIPTORS_SIZE);

	priv->clock_count = 0;
	clock_nb = dev_count_phandle_with_args(dev, "clocks", "#clock-cells",
					       0);
	if (clock_nb > 0) {
		priv->clocks = devm_kcalloc(dev, clock_nb, sizeof(struct clk),
					    GFP_KERNEL);
		if (!priv->clocks)
			return -ENOMEM;

		for (i = 0; i < clock_nb; i++) {
			err = clk_get_by_index(dev, i, &priv->clocks[i]);
			if (err < 0)
				break;

			err = clk_enable(&priv->clocks[i]);
			if (err && err != -ENOSYS && err != -ENOTSUPP) {
				pr_err("failed to enable clock %d\n", i);
				clk_free(&priv->clocks[i]);
				goto clk_err;
			}
			priv->clock_count++;
		}
	} else if (clock_nb != -ENOENT) {
		pr_err("failed to get clock phandle(%d)\n", clock_nb);
		return clock_nb;
	}
#endif

	/*
	 * For now, there is no reset scenario for GMAC using reset driver.
	 * Hence, we disable code of parsing reset property from dtb
	 *
	 */
#if 0
	ret = reset_get_bulk(dev, &reset_bulk);
	if (ret)
		pr_err("Can't get reset: %d\n", ret);
	else
		reset_deassert_bulk(&reset_bulk);
#endif

	/*
	 * If we are on PCI bus, either directly attached to a PCI root port,
	 * or via a PCI bridge, fill in plat before we probe the hardware.
	 */
	if (IS_ENABLED(CONFIG_PCI) && device_is_on_pci_bus(dev)) {
		dm_pci_read_config32(dev, PCI_BASE_ADDRESS_0, &iobase);
		iobase &= PCI_BASE_ADDRESS_MEM_MASK;
		iobase = dm_pci_mem_to_phys(dev, iobase);

		pdata->iobase = iobase;
		pdata->phy_interface = PHY_INTERFACE_MODE_RMII;
	}

	debug("%s, iobase=%x, priv=%p\n", __func__, iobase, priv);
	ioaddr = iobase;
	priv->mac_base = ioaddr;
	priv->interface = pdata->phy_interface;
	priv->max_speed = pdata->max_speed;

#if IS_ENABLED(CONFIG_DM_MDIO)
	ret = dw_dm_mdio_init(dev->name, dev);
#else
	ret = dw_mdio_init(dev->name, priv);
#endif
	if (ret) {
		err = ret;
		goto mdio_err;
	}
	priv->bus = miiphy_get_dev_by_name(dev->name);

	ret = dw_phy_init(priv, dev);
	debug("%s, ret=%d\n", __func__, ret);
	if (!ret)
		return 0;

	/* continue here for cleanup if no PHY found */
	err = ret;
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);
mdio_err:

#ifdef CONFIG_CLK
clk_err:
	ret = clk_release_all(priv->clocks, priv->clock_count);
	if (ret)
		pr_err("failed to disable all clocks\n");

#endif
	return err;
}

int designware_eth_send(struct udevice *dev, void *packet, int length)
{
	struct dw_eth_dev *priv = dev_get_priv(dev);

	return _dw_eth_send(priv, packet, length);
}

int designware_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct dw_eth_dev *priv = dev_get_priv(dev);

	return _dw_eth_recv(priv, packetp);
}

int designware_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct dw_eth_dev *priv = dev_get_priv(dev);

	return _dw_free_pkt(priv);
}

void designware_eth_stop(struct udevice *dev)
{
	struct dw_eth_dev *priv = dev_get_priv(dev);

	return _dw_eth_halt(priv);
}

int designware_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct dw_eth_dev *priv = dev_get_priv(dev);

	return _dw_write_hwaddr(priv, pdata->enetaddr);
}

const struct eth_ops designware_eth_ops = {
	.start			= designware_eth_start,
	.send			= designware_eth_send,
	.recv			= designware_eth_recv,
	.free_pkt		= designware_eth_free_pkt,
	.stop			= designware_eth_stop,
	.write_hwaddr		= designware_eth_write_hwaddr,
};

// initialize eth_pdata field.
// eth_pdata is necessary to interface between uboot net subsys and device driver.
int designware_eth_of_to_plat(struct udevice *dev)
{
	struct dw_eth_pdata *dw_pdata = dev_get_plat(dev);
	struct eth_pdata *pdata = &dw_pdata->eth_pdata;
	const char *phy_mode;
	int ret = 0;

	pdata->iobase = dev_read_addr(dev);
	pdata->phy_interface = -1;
	phy_mode = dev_read_string(dev, "phy-mode");
	if (phy_mode)
		pdata->phy_interface = phy_get_interface_by_name(phy_mode);
	if (pdata->phy_interface == -1) {
		debug("%s: Invalid PHY interface '%s'\n", __func__, phy_mode);
		return -EINVAL;
	}

	pdata->max_speed = dev_read_u32_default(dev, "max-speed", 0);

	return ret;
}

static int designware_eth_bind(struct udevice *dev)
{
	return 0;
}

static int designware_eth_remove(struct udevice *dev)
{
	struct dw_eth_dev *priv = dev_get_priv(dev);

	free(priv->phydev);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

#ifdef CONFIG_CLK
	return clk_release_all(priv->clocks, priv->clock_count);
#else
	return 0;
#endif
}

// no need. because probe is anycalled by dwmac_tcc
static const struct udevice_id designware_eth_ids[] = {
	{ }
};

U_BOOT_DRIVER(eth_designware) = {
	.name	= "eth_designware",
	.id	= UCLASS_ETH,
	.of_match = designware_eth_ids,
	.of_to_plat = designware_eth_of_to_plat,
	.bind	= designware_eth_bind,
	.probe	= designware_eth_probe,
	.remove	= designware_eth_remove,
	.ops	= &designware_eth_ops,
	.priv_auto	= sizeof(struct dw_eth_dev),
	.plat_auto	= sizeof(struct dw_eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
