/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#ifndef _DW_ETH_H
#define _DW_ETH_H

#ifdef CONFIG_DM_GPIO
#include <asm-generic/gpio.h>
#endif
#include <asm/cache.h>
#include <net.h>
#include <asm/system.h>

#define CONFIG_GMAC_BASE 0x11C00000

struct dmamacdescr {
	u32 des0;
	u32 des1;
	u32 des2;
	u32 des3;
};

/* ===================== DEBUG ===================== */
#define DEBUG_DESC		0
#define DEBUG_CONF		0
#define DEBUG_COMM		0
#define DEBUG_PKT		0

#define DEBUG_PKT_RX	0
#define DEBUG_PKT_TX	0

/* ===================== Common ===================== */
#define PADDR_BIT_MASK GENMASK(32, 0)

#define CONFIG_MACRESET_TIMEOUT	(3 * CONFIG_SYS_HZ)
#define CONFIG_MDIO_TIMEOUT	(3 * CONFIG_SYS_HZ)

#define ETH_TASK_STACK_SIZE 1024

/* RX Buffer size must be multiple of 4/8/16 bytes */
#define BUF_SIZE_16KiB 16368
#define BUF_SIZE_8KiB 8188
#define BUF_SIZE_4KiB 4096
#define BUF_SIZE_2KiB 2048

#define BUFF_SIZE BUF_SIZE_2KiB
//#define BUFF_SIZE ALIGN(1568, ARCH_DMA_MINALIGN)
#define MAX_PACKET_SIZE	ALIGN(BUFF_SIZE, ARCH_DMA_MINALIGN)
#define DEFAULT_MTU 1500

#define SPEED_1000 1000
#define SPEED_100 100
#define SPEED_10 10

#define ETH_RDES_NUM 32
#define ETH_TDES_NUM 32
#define ETH_DESC_SIZE 4 // 32bit * 4 (desc0~desc3)

#define TX_TOTAL_BUFSIZE	(BUFF_SIZE * ETH_TDES_NUM)
#define RX_TOTAL_BUFSIZE	(BUFF_SIZE * ETH_RDES_NUM)

/* Duplex, half or full. */
#define DUPLEX_HALF 0x00
#define DUPLEX_FULL 0x01

/* These need to be power of two, and >= 4 */
#define DMA_TX_SIZE 8
#define DMA_RX_SIZE 8

/* CSR Frequency Access Defines*/
#define CSR_F_35M 35000000
#define CSR_F_60M 60000000
#define CSR_F_100M 100000000
#define CSR_F_150M 150000000
#define CSR_F_250M 250000000
#define CSR_F_300M 300000000

/* MDC Clock Selection define*/
#define CSR_60_100M 0x0  /* MDC = clk_scr_i/42 */
#define CSR_100_150M 0x1 /* MDC = clk_scr_i/62 */
#define CSR_20_35M 0x2   /* MDC = clk_scr_i/16 */
#define CSR_35_60M 0x3   /* MDC = clk_scr_i/26 */
#define CSR_150_250M 0x4 /* MDC = clk_scr_i/102 */
#define CSR_250_300M 0x5 /* MDC = clk_scr_i/122 */

/* Test Mode index */
#define RGMII_MODE (1)
#define RMII_MODE (4)
#define MII_MODE (0)
#define PHY_INF_SEL RGMII_MODE

#define ETH_MODE_NORMAL 0
#define ETH_MODE_MAC_LOOPBACK 1
#define ETH_MODE_PHY_LOOPBACK 2

/* Flow Control defines */
#define FLOW_OFF 0
#define FLOW_RX 1
#define FLOW_TX 2
#define FLOW_AUTO (FLOW_TX | FLOW_RX)

#define DEFAULT_DMA_PBL 8

/* DMA STORE-AND-FORWARD Operation Mode */
#define SF_DMA_MODE 1

/* HW Info */
#define ETH_PHY_RST GPIO_GPA(17)
#define ETH_MDC GPIO_GPA(1)
#define ETH_MDIO GPIO_GPA(2)

#define ETH_TXCLK GPIO_GPA(0)
#define ETH_TXD0 GPIO_GPA(5)
#define ETH_TXD1 GPIO_GPA(6)
#define ETH_TXD2 GPIO_GPA(11)
#define ETH_TXD3 GPIO_GPA(12)
#define ETH_TXEN GPIO_GPA(7)

#define ETH_RXCLK GPIO_GPA(13)
#define ETH_RXD0 GPIO_GPA(3)
#define ETH_RXD1 GPIO_GPA(4)
#define ETH_RXD2 GPIO_GPA(9)
#define ETH_RXD3 GPIO_GPA(10)
#define ETH_RXDV GPIO_GPA(8)

#define RGMII_PERICLK_RATE 125 * 1000 * 1000
#define RMII_PERICLK_RATE 50 * 1000 * 1000
#define MII_PERICLK_RATE 25 * 1000 * 1000

#define ETH_RDES_NUM 32
#define ETH_TDES_NUM 32
#define ETH_DESC_SIZE 4 // 32bit * 4 (desc0~desc3)


// TCC805x

#define ETH_MTL_BASE 0xc00
#define ETH_MTL_Q0_BASE 0xd00
#define ETH_MTL_Q1_BASE // todo
#define ETH_DMA_BASE 0x1000
#define ETH_DMA_CH0_BASE 0x1100
#define ETH_DLY_BASE 0x2000
#define ETH_CFG_BASE 0x3000

/* MAC registers */
#define ETH_MAC_CONFIG 0x00
#define ETH_MAC_PACKET_FILTER 0x08
#define ETH_MAC_HASH_TAB_0_31 0x10
#define ETH_MAC_HASH_TAB_32_63 0x14
#define ETH_MAC_RX_FLOW_CTRL 0x90
#define ETH_MAC_QX_TX_FLOW_CTRL(x) (0x70 + (x * 4))
#define ETH_MAC_TXQ_PRTY_MAP0 0x98
#define ETH_MAC_TXQ_PRTY_MAP1 0x9C
#define ETH_MAC_RXQ_CTRL0 0xa0
#define ETH_MAC_RXQ_CTRL1 0xa4
#define ETH_MAC_RXQ_CTRL2 0xa8
#define ETH_MAC_RXQ_CTRL3 0xac
#define ETH_MAC_INT_STATUS 0xb0
#define ETH_MAC_INT_EN 0xb4
#define ETH_MAC_1US_TIC_COUNTER 0xdc
#define ETH_MAC_PCS_BASE 0xe0
#define ETH_MAC_PHYIF_CONTROL_STATUS 0xf8
#define ETH_MAC_PMT 0xc0
#define ETH_MAC_VERSION 0x110
#define ETH_MAC_DEBUG 0x114
#define ETH_MAC_HW_FEATURE0 0x11c
#define ETH_MAC_HW_FEATURE1 0x120
#define ETH_MAC_HW_FEATURE2 0x124
#define ETH_MAC_MDIO_ADDR 0x200
#define ETH_MAC_MDIO_DATA 0x204
#define ETH_MAC_MDIO_ADDR_SHIFT (21)
#define ETH_MAC_MDIO_ADDR_MASK (0x3E00000u)
#define ETH_MAC_MDIO_REG_SHIFT (16)
#define ETH_MAC_MDIO_REG_MASK (0x1F0000u)
#define ETH_MAC_MDIO_CSR_SHIFT (8)
#define ETH_MAC_MDIO_CSR_MASK (0xF00u)

#define ETH_MAC_ADDR_HIGH(reg) (0x300 + reg * 8)
#define ETH_MAC_ADDR_LOW(reg) (0x304 + reg * 8)
#define ETH_MAC_TXQ0_OP_MODE 0xd00
#define ETH_MAC_RXQ0_OP_MODE 0xd30

/* MAC config */

#define ETH_MAC_CONFIG_IPC BIT(27)
#define ETH_MAC_CONFIG_2K BIT(22)
#define ETH_MAC_CONFIG_ACS BIT(20)
#define ETH_MAC_CONFIG_BE BIT(18)
#define ETH_MAC_CONFIG_JD BIT(17)
#define ETH_MAC_CONFIG_JE BIT(16)
#define ETH_MAC_CONFIG_PS BIT(15)
#define ETH_MAC_CONFIG_FES BIT(14)
#define ETH_MAC_CONFIG_DM BIT(13)
#define ETH_MAC_CONFIG_LM BIT(12)
#define ETH_MAC_CONFIG_DCRS BIT(9)
#define ETH_MAC_CONFIG_TE BIT(1)
#define ETH_MAC_CONFIG_RE BIT(0)

/* Default operating mode of the MAC */
#define ETH_DEFAULT_CORE_INIT (ETH_MAC_CONFIG_DCRS | ETH_MAC_CONFIG_DM | \
                               ETH_MAC_CONFIG_JD | ETH_MAC_CONFIG_BE |   \
                               ETH_MAC_CONFIG_IPC)

#define ETH_CFG0_OFFSET 0x00
#define ETH_CFG0_OFFSET_H 0x02 // Need fot 16 bit register write.

#define ETH_CFG1_OFFSET 0x04
#define ETH_CFG1_OFFSET_H 0x06 // Need fot 16 bit register write.

// GMAC CFG1
#define ETH_CFG1_H_CE BIT(12)
#define ETH_CFG1_H_PHY_INFSEL_SHIFT (8)
#define ETH_CFG1_H_PHY_INFSEL_MASK (0x7)
#define ETH_CFG1_H_FCTRL_SHIFT (5)
#define ETH_CFG1_H_FCTRL_MASK (0x3 << ETH_CFG1_FCTRL_SHIFT)
#define ETH_CFG1_H_TCO BIT(0)

#define ETH_DLY0_OFFSET (0x00) // TXCLK , TXER , TXEN
#define ETH_DLY1_OFFSET (0x04) // TXD0 ~ TXD3
#define ETH_DLY2_OFFSET (0x08) // TXD4 ~ TXD7
#define ETH_DLY3_OFFSET (0x0C) // RXCLK , RXER , RXDV
#define ETH_DLY4_OFFSET (0x10) // RXD0 ~ RXD3
#define ETH_DLY5_OFFSET (0x14) // RXD4 ~ RXD7
#define ETH_DLY6_OFFSET (0x18) // CRS , COL

#define ETH_DLY_TXC_I_SHIFT (0)
#define ETH_DLY_TXC_I_INV_SHIFT (7)
#define ETH_DLY_TXC_O_SHIFT (8)
#define ETH_DLY_TXC_O_INV_SHIFT (15)
#define ETH_DLY_TXEN_SHIFT (16)
#define ETH_DLY_TXER_SHIFT (24)

#define ETH_DLY_TXD0_SHIFT (0)
#define ETH_DLY_TXD1_SHIFT (8)
#define ETH_DLY_TXD2_SHIFT (16)
#define ETH_DLY_TXD3_SHIFT (24)

#define ETH_DLY_RXC_I_SHIFT (0)
#define ETH_DLY_RXC_I_INV_SHIFT (7)
#define ETH_DLY_RXDV_SHIFT (16)
#define ETH_DLY_RXER_SHIFT (24)

#define ETH_DLY_RXD0_SHIFT (0)
#define ETH_DLY_RXD1_SHIFT (8)
#define ETH_DLY_RXD2_SHIFT (16)
#define ETH_DLY_RXD3_SHIFT (24)

#define ETH_PKT_RECV_ALL BIT(31)

/* ===================== Descriptor ===================== */
/* Normal transmit descriptor defines (without split feature) */

/* TDES2 (read format) */
#define TDES2_BUFFER1_SIZE_MASK GENMASK(13, 0)
#define TDES2_VLAN_TAG_MASK GENMASK(15, 14)
#define TDES2_BUFFER2_SIZE_MASK GENMASK(29, 16)
#define TDES2_BUFFER2_SIZE_MASK_SHIFT 16
#define TDES2_TIMESTAMP_ENABLE BIT(30)
#define TDES2_INTERRUPT_ON_COMPLETION BIT(31)

/* TDES3 (read format) */
#define TDES3_PACKET_SIZE_MASK GENMASK(14, 0)
#define TDES3_CHECKSUM_INSERTION_MASK GENMASK(17, 16)
#define TDES3_CHECKSUM_INSERTION_SHIFT 16
#define TDES3_TCP_PKT_PAYLOAD_MASK GENMASK(17, 0)
#define TDES3_TCP_SEGMENTATION_ENABLE BIT(18)
#define TDES3_HDR_LEN_SHIFT 19
#define TDES3_SLOT_NUMBER_MASK GENMASK(22, 19)
#define TDES3_SA_INSERT_CTRL_MASK GENMASK(25, 23)
#define TDES3_CRC_PAD_CTRL_MASK GENMASK(27, 26)

/* TDES3 (write back format) */
#define TDES3_IP_HDR_ERROR BIT(0)
#define TDES3_DEFERRED BIT(1)
#define TDES3_UNDERFLOW_ERROR BIT(2)
#define TDES3_EXCESSIVE_DEFERRAL BIT(3)
#define TDES3_COLLISION_COUNT_MASK GENMASK(7, 4)
#define TDES3_COLLISION_COUNT_SHIFT 4
#define TDES3_EXCESSIVE_COLLISION BIT(8)
#define TDES3_LATE_COLLISION BIT(9)
#define TDES3_NO_CARRIER BIT(10)
#define TDES3_LOSS_CARRIER BIT(11)
#define TDES3_PAYLOAD_ERROR BIT(12)
#define TDES3_PACKET_FLUSHED BIT(13)
#define TDES3_JABBER_TIMEOUT BIT(14)
#define TDES3_ERROR_SUMMARY BIT(15)
#define TDES3_TIMESTAMP_STATUS BIT(17)
#define TDES3_TIMESTAMP_STATUS_SHIFT 17

/* TDES3 context */
#define TDES3_CTXT_TCMSSV BIT(26)

/* TDES3 Common */
#define TDES3_RS1V BIT(26)
#define TDES3_RS1V_SHIFT 26

// When this bit is set, it indicates that the buffer contains the last segment of the packet.
// When this bit is set, the B1L or B2L field should have a non-zero value.
#define TDES3_LAST_DESCRIPTOR BIT(28)
#define TDES3_LAST_DESCRIPTOR_SHIFT 28

// When this bit is set, it indicates that the buffer contains the first segment of a packet.
#define TDES3_FIRST_DESCRIPTOR BIT(29)
#define TDES3_CONTEXT_TYPE BIT(30)
#define TDES3_CONTEXT_TYPE_SHIFT 30

/* TDS3 use for both format (read and write back) */
#define TDES3_OWN BIT(31)
#define TDES3_OWN_SHIFT 31

/* Normal receive descriptor defines (without split feature) */

/* RDES0 (write back format) */
#define RDES0_VLAN_TAG_MASK GENMASK(15, 0)

/* RDES1 (write back format) */
#define RDES1_IP_PAYLOAD_TYPE_MASK GENMASK(2, 0)
#define RDES1_IP_HDR_ERROR BIT(3)
#define RDES1_IPV4_HEADER BIT(4)
#define RDES1_IPV6_HEADER BIT(5)
#define RDES1_IP_CSUM_BYPASSED BIT(6)
#define RDES1_IP_CSUM_ERROR BIT(7)
#define RDES1_PTP_MSG_TYPE_MASK GENMASK(11, 8)
#define RDES1_PTP_PACKET_TYPE BIT(12)
#define RDES1_PTP_VER BIT(13)
#define RDES1_TIMESTAMP_AVAILABLE BIT(14)
#define RDES1_TIMESTAMP_AVAILABLE_SHIFT 14
#define RDES1_TIMESTAMP_DROPPED BIT(15)
#define RDES1_IP_TYPE1_CSUM_MASK GENMASK(31, 16)

/* RDES2 (write back format) */
#define RDES2_L3_L4_HEADER_SIZE_MASK GENMASK(9, 0)
#define RDES2_VLAN_FILTER_STATUS BIT(15)
#define RDES2_SA_FILTER_FAIL BIT(16)
#define RDES2_DA_FILTER_FAIL BIT(17)
#define RDES2_HASH_FILTER_STATUS BIT(18)
#define RDES2_MAC_ADDR_MATCH_MASK GENMASK(26, 19)
#define RDES2_HASH_VALUE_MATCH_MASK GENMASK(26, 19)
#define RDES2_L3_FILTER_MATCH BIT(27)
#define RDES2_L4_FILTER_MATCH BIT(28)
#define RDES2_L3_L4_FILT_NB_MATCH_MASK GENMASK(27, 26)
#define RDES2_L3_L4_FILT_NB_MATCH_SHIFT 26

/* RDES3 (write back format) */
//#define RDES3_PACKET_SIZE_MASK GENMASK(14, 0)
#define RDES3_PACKET_SIZE_MASK 0x7FFFu
#define RDES3_ERROR_SUMMARY BIT(15)
#define RDES3_PACKET_LEN_TYPE_MASK GENMASK(18, 16)
#define RDES3_DRIBBLE_ERROR BIT(19)
#define RDES3_RECEIVE_ERROR BIT(20)
#define RDES3_OVERFLOW_ERROR BIT(21)
#define RDES3_RECEIVE_WATCHDOG BIT(22)
#define RDES3_GIANT_PACKET BIT(23)
#define RDES3_CRC_ERROR BIT(24)
#define RDES3_RDES0_VALID BIT(25)
#define RDES3_RDES1_VALID BIT(26)
#define RDES3_RDES2_VALID BIT(27)
#define RDES3_LAST_DESCRIPTOR BIT(28)
#define RDES3_FIRST_DESCRIPTOR BIT(29)
#define RDES3_CONTEXT_DESCRIPTOR BIT(30)
#define RDES3_CONTEXT_DESCRIPTOR_SHIFT 30

/* RDES3 (read format) */
#define RDES3_BUFFER1_VALID_ADDR BIT(24)
#define RDES3_BUFFER2_VALID_ADDR BIT(25)
#define RDES3_INT_ON_COMPLETION_EN BIT(30)

/* TDS3 use for both format (read and write back) */
#define RDES3_OWN BIT(31)

/* ============================================================ */
/* RX Queues Routing */
#define GMAC_RXQCTRL_AVCPQ_MASK GENMASK(2, 0)
#define GMAC_RXQCTRL_AVCPQ_SHIFT 0
#define GMAC_RXQCTRL_PTPQ_MASK GENMASK(6, 4)
#define GMAC_RXQCTRL_PTPQ_SHIFT 4
#define GMAC_RXQCTRL_DCBCPQ_MASK GENMASK(10, 8)
#define GMAC_RXQCTRL_DCBCPQ_SHIFT 8
#define GMAC_RXQCTRL_UPQ_MASK GENMASK(14, 12)
#define GMAC_RXQCTRL_UPQ_SHIFT 12
#define GMAC_RXQCTRL_MCBCQ_MASK GENMASK(18, 16)
#define GMAC_RXQCTRL_MCBCQ_SHIFT 16
#define GMAC_RXQCTRL_MCBCQEN BIT(20)
#define GMAC_RXQCTRL_MCBCQEN_SHIFT 20
#define GMAC_RXQCTRL_TACPQE BIT(21)
#define GMAC_RXQCTRL_TACPQE_SHIFT 21

/* MAC Packet Filtering */
#define GMAC_PACKET_FILTER_PR BIT(0)
#define GMAC_PACKET_FILTER_HMC BIT(2)
#define GMAC_PACKET_FILTER_PM BIT(4)

#define GMAC_MAX_PERFECT_ADDRESSES 128

/* MAC RX Queue Enable */
#define GMAC_RX_QUEUE_CLEAR(queue) ~(GENMASK(1, 0) << ((queue)*2))
#define GMAC_RX_AV_QUEUE_ENABLE(queue) BIT((queue)*2)
#define GMAC_RX_DCB_QUEUE_ENABLE(queue) BIT(((queue)*2) + 1)

/* MAC Flow Control RX */
#define GMAC_RX_FLOW_CTRL_RFE BIT(0)

/* RX Queues Priorities */
#define GMAC_RXQCTRL_PSRQX_MASK(x) GENMASK(7 + ((x)*8), 0 + ((x)*8))
#define GMAC_RXQCTRL_PSRQX_SHIFT(x) ((x)*8)

/* TX Queues Priorities */
#define GMAC_TXQCTRL_PSTQX_MASK(x) GENMASK(7 + ((x)*8), 0 + ((x)*8))
#define GMAC_TXQCTRL_PSTQX_SHIFT(x) ((x)*8)

/* MAC Flow Control TX */
#define GMAC_TX_FLOW_CTRL_TFE BIT(1)
#define GMAC_TX_FLOW_CTRL_PT_SHIFT 16

/*  MAC Interrupt bitmap*/
#define GMAC_INT_RGSMIIS BIT(0)
#define GMAC_INT_PCS_LINK BIT(1)
#define GMAC_INT_PCS_ANE BIT(2)
#define GMAC_INT_PCS_PHYIS BIT(3)
#define GMAC_INT_PMT_EN BIT(4)
#define GMAC_INT_LPI_EN BIT(5)

#define INTR_BASE_NUM 32
#define TCC805X_GMAC_INTR_NUM 139
#define GMAC_INTRRUPT (TCC805X_GMAC_INTR_NUM + INTR_BASE_NUM)

#define GMAC_PCS_IRQ_DEFAULT (GMAC_INT_RGSMIIS | GMAC_INT_PCS_LINK | \
                              GMAC_INT_PCS_ANE)

#define GMAC_INT_DEFAULT_MASK GMAC_INT_PMT_EN
//#define   GMAC_INT_DEFAULT_MASK   (GMAC_INT_RGSMIIS | BIT(14))//GMAC_INT_PMT_EN

/* Energy Efficient Ethernet (EEE) for GMAC4
 *
 * LPI status, timer and control register offset
 */
#define GMAC4_LPI_CTRL_STATUS 0xd0
#define GMAC4_LPI_TIMER_CTRL 0xd4

/* LPI control and status defines */
#define GMAC4_LPI_CTRL_STATUS_LPITCSE BIT(21) /* LPI Tx Clock Stop Enable */
#define GMAC4_LPI_CTRL_STATUS_LPITXA BIT(19)  /* Enable LPI TX Automate */
#define GMAC4_LPI_CTRL_STATUS_PLS BIT(17)     /* PHY Link Status */
#define GMAC4_LPI_CTRL_STATUS_LPIEN BIT(16)   /* LPI Enable */

/* MAC Debug bitmap */
#define GMAC_DEBUG_TFCSTS_MASK GENMASK(18, 17)
#define GMAC_DEBUG_TFCSTS_SHIFT 17
#define GMAC_DEBUG_TFCSTS_IDLE 0
#define GMAC_DEBUG_TFCSTS_WAIT 1
#define GMAC_DEBUG_TFCSTS_GEN_PAUSE 2
#define GMAC_DEBUG_TFCSTS_XFER 3
#define GMAC_DEBUG_TPESTS BIT(16)
#define GMAC_DEBUG_RFCFCSTS_MASK GENMASK(2, 1)
#define GMAC_DEBUG_RFCFCSTS_SHIFT 1
#define GMAC_DEBUG_RPESTS BIT(0)

/* MAC HW features0 bitmap */
#define GMAC_HW_FEAT_ADDMAC BIT(18)
#define GMAC_HW_FEAT_RXCOESEL BIT(16)
#define GMAC_HW_FEAT_TXCOSEL BIT(14)
#define GMAC_HW_FEAT_EEESEL BIT(13)
#define GMAC_HW_FEAT_TSSEL BIT(12)
#define GMAC_HW_FEAT_MMCSEL BIT(8)
#define GMAC_HW_FEAT_MGKSEL BIT(7)
#define GMAC_HW_FEAT_RWKSEL BIT(6)
#define GMAC_HW_FEAT_SMASEL BIT(5)
#define GMAC_HW_FEAT_VLHASH BIT(4)
#define GMAC_HW_FEAT_PCSSEL BIT(3)
#define GMAC_HW_FEAT_HDSEL BIT(2)
#define GMAC_HW_FEAT_GMIISEL BIT(1)
#define GMAC_HW_FEAT_MIISEL BIT(0)

/* MAC HW features1 bitmap */
#define GMAC_HW_FEAT_AVSEL BIT(20)
#define GMAC_HW_TSOEN BIT(18)
#define GMAC_HW_TXFIFOSIZE_MASK GENMASK(10, 6)
#define GMAC_HW_TXFIFOSIZE_SHIFT 6
#define GMAC_HW_RXFIFOSIZE_MASK GENMASK(4, 0)
#define GMAC_HW_RXFIFOSIZE_SHIFT 0

/* MAC HW features2 bitmap */
#define GMAC_HW_FEAT_TXCHCNT GENMASK(21, 18)
#define GMAC_HW_FEAT_RXCHCNT GENMASK(15, 12)
#define GMAC_HW_FEAT_TXQCNT GENMASK(9, 6)
#define GMAC_HW_FEAT_RXQCNT GENMASK(3, 0)

/* MAC HW ADDR regs */
#define GMAC_HI_DCS GENMASK(18, 16)
#define GMAC_HI_DCS_SHIFT 16
#define GMAC_HI_REG_AE BIT(31)

/*  MTL registers */
/* RX/TX Queue Mode */
#define MTL_QUEUE_AVB 0x0
#define MTL_QUEUE_DCB 0x1

#define MTL_OPERATION_MODE 0x00000c00
#define MTL_OPERATION_SCHALG_MASK GENMASK(6, 5)
#define MTL_OPERATION_SCHALG_WRR (0x0 << 5)
#define MTL_OPERATION_SCHALG_WFQ (0x1 << 5)
#define MTL_OPERATION_SCHALG_DWRR (0x2 << 5)
#define MTL_OPERATION_SCHALG_SP (0x3 << 5)
#define MTL_OPERATION_RAA BIT(2)
#define MTL_OPERATION_RAA_SP (0x0 << 2)
#define MTL_OPERATION_RAA_WSP (0x1 << 2)

#define MTL_INT_STATUS 0x00000c20
#define MTL_INT_QX(x) BIT(x)

#define MTL_RXQ_DMA_MAP0 0x00000c30 /* queue 0 to 3 */
#define MTL_RXQ_DMA_MAP1 0x00000c34 /* queue 4 to 7 */
#define MTL_RXQ_DMA_Q04MDMACH_MASK GENMASK(3, 0)
#define MTL_RXQ_DMA_Q04MDMACH(x) ((x) << 0)
#define MTL_RXQ_DMA_QXMDMACH_MASK(x) GENMASK(11 + (8 * ((x)-1)), 8 * (x))
#define MTL_RXQ_DMA_QXMDMACH(chan, q) ((chan) << (8 * (q)))

#define MTL_CHAN_BASE_ADDR 0x00000d00
#define MTL_CHAN_BASE_OFFSET 0x40
#define MTL_CHANX_BASE_ADDR(x) (MTL_CHAN_BASE_ADDR + \
                                (x * MTL_CHAN_BASE_OFFSET))

#define MTL_CHAN_TX_OP_MODE(x) MTL_CHANX_BASE_ADDR(x)
#define MTL_CHAN_TX_DEBUG(x) (MTL_CHANX_BASE_ADDR(x) + 0x8)
#define MTL_CHAN_INT_CTRL(x) (MTL_CHANX_BASE_ADDR(x) + 0x2c)
#define MTL_CHAN_RX_OP_MODE(x) (MTL_CHANX_BASE_ADDR(x) + 0x30)
#define MTL_CHAN_RX_DEBUG(x) (MTL_CHANX_BASE_ADDR(x) + 0x38)

#define MTL_OP_MODE_RSF BIT(5)
#define MTL_OP_MODE_TXQEN BIT(3)
#define MTL_OP_MODE_TSF BIT(1)

#define MTL_OP_MODE_TQS_MASK (0x1FF0000) // GENMASK(24, 16)
#define MTL_OP_MODE_TQS_SHIFT 16

#define MTL_OP_MODE_TTC_MASK 0x70
#define MTL_OP_MODE_TTC_SHIFT 4

#define MTL_OP_MODE_TTC_32 0
#define MTL_OP_MODE_TTC_64 (1 << MTL_OP_MODE_TTC_SHIFT)
#define MTL_OP_MODE_TTC_96 (2 << MTL_OP_MODE_TTC_SHIFT)
#define MTL_OP_MODE_TTC_128 (3 << MTL_OP_MODE_TTC_SHIFT)
#define MTL_OP_MODE_TTC_192 (4 << MTL_OP_MODE_TTC_SHIFT)
#define MTL_OP_MODE_TTC_256 (5 << MTL_OP_MODE_TTC_SHIFT)
#define MTL_OP_MODE_TTC_384 (6 << MTL_OP_MODE_TTC_SHIFT)
#define MTL_OP_MODE_TTC_512 (7 << MTL_OP_MODE_TTC_SHIFT)

#define MTL_OP_MODE_RQS_MASK (0x3FF00000) // GENMASK(29, 20)
#define MTL_OP_MODE_RQS_SHIFT 20

#define MTL_OP_MODE_RFD_MASK (0xFC000) // GENMASK(19, 14)
#define MTL_OP_MODE_RFD_SHIFT 14

#define MTL_OP_MODE_RFA_MASK (0x3F00)// GENMASK(13, 8)
#define MTL_OP_MODE_RFA_SHIFT 8

#define MTL_OP_MODE_EHFC BIT(7)

#define MTL_OP_MODE_RTC_MASK 0x18
#define MTL_OP_MODE_RTC_SHIFT 3

#define MTL_OP_MODE_RTC_32 (1 << MTL_OP_MODE_RTC_SHIFT)
#define MTL_OP_MODE_RTC_64 0
#define MTL_OP_MODE_RTC_96 (2 << MTL_OP_MODE_RTC_SHIFT)
#define MTL_OP_MODE_RTC_128 (3 << MTL_OP_MODE_RTC_SHIFT)

/* MTL ETS Control register */
#define MTL_ETS_CTRL_BASE_ADDR 0x00000d10
#define MTL_ETS_CTRL_BASE_OFFSET 0x40
#define MTL_ETSX_CTRL_BASE_ADDR(x) (MTL_ETS_CTRL_BASE_ADDR + \
                                    ((x)*MTL_ETS_CTRL_BASE_OFFSET))

#define MTL_ETS_CTRL_CC BIT(3)
#define MTL_ETS_CTRL_AVALG BIT(2)

/* MTL Queue Quantum Weight */
#define MTL_TXQ_WEIGHT_BASE_ADDR 0x00000d18
#define MTL_TXQ_WEIGHT_BASE_OFFSET 0x40
#define MTL_TXQX_WEIGHT_BASE_ADDR(x) (MTL_TXQ_WEIGHT_BASE_ADDR + \
                                      ((x)*MTL_TXQ_WEIGHT_BASE_OFFSET))
#define MTL_TXQ_WEIGHT_ISCQW_MASK GENMASK(20, 0)

/* MTL sendSlopeCredit register */
#define MTL_SEND_SLP_CRED_BASE_ADDR 0x00000d1c
#define MTL_SEND_SLP_CRED_OFFSET 0x40
#define MTL_SEND_SLP_CREDX_BASE_ADDR(x) (MTL_SEND_SLP_CRED_BASE_ADDR + \
                                         ((x)*MTL_SEND_SLP_CRED_OFFSET))

#define MTL_SEND_SLP_CRED_SSC_MASK GENMASK(13, 0)

/* MTL hiCredit register */
#define MTL_HIGH_CRED_BASE_ADDR 0x00000d20
#define MTL_HIGH_CRED_OFFSET 0x40
#define MTL_HIGH_CREDX_BASE_ADDR(x) (MTL_HIGH_CRED_BASE_ADDR + \
                                     ((x)*MTL_HIGH_CRED_OFFSET))
#define MTL_HIGH_CRED_HC_MASK GENMASK(28, 0)

/* MTL loCredit register */
#define MTL_LOW_CRED_BASE_ADDR 0x00000d24
#define MTL_LOW_CRED_OFFSET 0x40
#define MTL_LOW_CREDX_BASE_ADDR(x) (MTL_LOW_CRED_BASE_ADDR + \
                                    ((x)*MTL_LOW_CRED_OFFSET))

#define MTL_HIGH_CRED_LC_MASK GENMASK(28, 0)

/*  MTL debug */
#define MTL_DEBUG_TXSTSFSTS BIT(5)
#define MTL_DEBUG_TXFSTS BIT(4)
#define MTL_DEBUG_TWCSTS BIT(3)

/* MTL debug: Tx FIFO Read Controller Status */
#define MTL_DEBUG_TRCSTS_MASK GENMASK(2, 1)
#define MTL_DEBUG_TRCSTS_SHIFT 1
#define MTL_DEBUG_TRCSTS_IDLE 0
#define MTL_DEBUG_TRCSTS_READ 1
#define MTL_DEBUG_TRCSTS_TXW 2
#define MTL_DEBUG_TRCSTS_WRITE 3
#define MTL_DEBUG_TXPAUSED BIT(0)

/* MAC debug: GMII or MII Transmit Protocol Engine Status */
#define MTL_DEBUG_RXFSTS_MASK GENMASK(5, 4)
#define MTL_DEBUG_RXFSTS_SHIFT 4
#define MTL_DEBUG_RXFSTS_EMPTY 0
#define MTL_DEBUG_RXFSTS_BT 1
#define MTL_DEBUG_RXFSTS_AT 2
#define MTL_DEBUG_RXFSTS_FULL 3
#define MTL_DEBUG_RRCSTS_MASK GENMASK(2, 1)
#define MTL_DEBUG_RRCSTS_SHIFT 1
#define MTL_DEBUG_RRCSTS_IDLE 0
#define MTL_DEBUG_RRCSTS_RDATA 1
#define MTL_DEBUG_RRCSTS_RSTAT 2
#define MTL_DEBUG_RRCSTS_FLUSH 3
#define MTL_DEBUG_RWCSTS BIT(0)

/*  MTL interrupt */
#define MTL_RX_OVERFLOW_INT_EN BIT(24)
#define MTL_RX_OVERFLOW_INT BIT(16)

/* To dump the core regs excluding  the Address Registers */
#define GMAC_REG_NUM 132

/*  MTL debug */
#define MTL_DEBUG_TXSTSFSTS BIT(5)
#define MTL_DEBUG_TXFSTS BIT(4)
#define MTL_DEBUG_TWCSTS BIT(3)

/* MTL debug: Tx FIFO Read Controller Status */
#define MTL_DEBUG_TRCSTS_MASK GENMASK(2, 1)
#define MTL_DEBUG_TRCSTS_SHIFT 1
#define MTL_DEBUG_TRCSTS_IDLE 0
#define MTL_DEBUG_TRCSTS_READ 1
#define MTL_DEBUG_TRCSTS_TXW 2
#define MTL_DEBUG_TRCSTS_WRITE 3
#define MTL_DEBUG_TXPAUSED BIT(0)

/* MAC debug: GMII or MII Transmit Protocol Engine Status */
#define MTL_DEBUG_RXFSTS_MASK GENMASK(5, 4)
#define MTL_DEBUG_RXFSTS_SHIFT 4
#define MTL_DEBUG_RXFSTS_EMPTY 0
#define MTL_DEBUG_RXFSTS_BT 1
#define MTL_DEBUG_RXFSTS_AT 2
#define MTL_DEBUG_RXFSTS_FULL 3
#define MTL_DEBUG_RRCSTS_MASK GENMASK(2, 1)
#define MTL_DEBUG_RRCSTS_SHIFT 1
#define MTL_DEBUG_RRCSTS_IDLE 0
#define MTL_DEBUG_RRCSTS_RDATA 1
#define MTL_DEBUG_RRCSTS_RSTAT 2
#define MTL_DEBUG_RRCSTS_FLUSH 3
#define MTL_DEBUG_RWCSTS BIT(0)

#define MTL_TXQ0_MTT0D 0xd08
#define MTL_TXQ0_DEBUG_TXQSTS BIT(4)
#define MTL_TXQ0_DEBUG_TRCSTS_SHIFT 1
#define MTL_TXQ0_DEBUG_TRCSTS_MASK 3

#define MTL_RXQ0_MTR0D 0xd38
#define MTL_RXQ0_DEBUG_PRXQ_SHIFT 16
#define MTL_RXQ0_DEBUG_PRXQ_MASK 0x7fff
#define MTL_RXQ0_DEBUG_RXQSTS_SHIFT 4
#define MTL_RXQ0_DEBUG_RXQSTS_MASK 3

/* Common MAC defines */
#define MAC_CTRL_REG 0x00000000  /* MAC Control */
#define MAC_ENABLE_TX 0x00000008 /* Transmitter Enable */
#define MAC_ENABLE_RX 0x00000004 /* Receiver Enable */

// Register
#define DWMAC_MAC_CONFIG (0x0)
#define DWMAC_MAC_PF (0x8)
#define DWMAC_MAC_L3_L4_CTRL (0x900)

/* ===================== core ===================== */

#define MMC_GMAC4_OFFSET 0x700
#define PTP_GMAC4_OFFSET 0xb00

#define MMC_BASE_ADDR ETH_MAC_BASE + MMC_GMAC4_OFFSET
#define PTP_BASE_ADDR ETH_MAC_BASE + PTP_GMAC4_OFFSET

/* MAC Management Counters register offset */

#define MMC_CNTRL 0x00        /* MMC Control */
#define MMC_RX_INTR 0x04      /* MMC RX Interrupt */
#define MMC_TX_INTR 0x08      /* MMC TX Interrupt */
#define MMC_RX_INTR_MASK 0x0c /* MMC Interrupt Mask */
#define MMC_TX_INTR_MASK 0x10 /* MMC Interrupt Mask */
#define MMC_DEFAULT_MASK 0xffffffff

/* MMC control register */
/* When set, all counter are reset */
#define MMC_CNTRL_COUNTER_RESET 0x1
/* When set, do not roll over zero after reaching the max value*/
#define MMC_CNTRL_COUNTER_STOP_ROLLOVER 0x2
#define MMC_CNTRL_RESET_ON_READ 0x4   /* Reset after reading */
#define MMC_CNTRL_COUNTER_FREEZER 0x8 /* Freeze counter values to the current value.*/
#define MMC_CNTRL_PRESET 0x10
#define MMC_CNTRL_FULL_HALF_PRESET 0x20

/* IPC*/
#define MMC_RX_IPC_INTR_MASK 0x100
#define MMC_RX_IPC_INTR 0x108

/* ===================== dma ===================== */

/* Define the max channel number used for tx (also rx).
 * dwmac4 accepts up to 8 channels for TX (and also 8 channels for RX
 */
#define DMA_CHANNEL_NB_MAX 1

#define DMA_BUS_MODE 0x00001000
#define DMA_SYS_BUS_MODE 0x00001004
#define DMA_STATUS 0x00001008
#define DMA_DEBUG_STATUS_0 0x0000100c
#define DMA_DEBUG_STATUS_1 0x00001010
#define DMA_DEBUG_STATUS_2 0x00001014
#define DMA_AXI_BUS_MODE 0x00001028

/* DMA Bus Mode bitmap */
#define DMA_BUS_MODE_SFT_RESET BIT(0)

/* DMA SYS Bus Mode bitmap */
#define DMA_BUS_MODE_SPH BIT(24)
#define DMA_BUS_MODE_PBL BIT(16)
#define DMA_BUS_MODE_PBL_SHIFT 16
#define DMA_BUS_MODE_RPBL_SHIFT 16
#define DMA_BUS_MODE_MB BIT(14)
#define DMA_BUS_MODE_FB BIT(0)

/* DMA Interrupt top status */
#define DMA_STATUS_MAC BIT(17)
#define DMA_STATUS_MTL BIT(16)
#define DMA_STATUS_CHAN7 BIT(7)
#define DMA_STATUS_CHAN6 BIT(6)
#define DMA_STATUS_CHAN5 BIT(5)
#define DMA_STATUS_CHAN4 BIT(4)
#define DMA_STATUS_CHAN3 BIT(3)
#define DMA_STATUS_CHAN2 BIT(2)
#define DMA_STATUS_CHAN1 BIT(1)
#define DMA_STATUS_CHAN0 BIT(0)

/* DMA debug status bitmap */
#define DMA_DEBUG_STATUS_TS_MASK 0xf
#define DMA_DEBUG_STATUS_RS_MASK 0xf

/* DMA AXI bitmap */
#define DMA_AXI_EN_LPI BIT(31)
#define DMA_AXI_LPI_XIT_FRM BIT(30)
#define DMA_AXI_WR_OSR_LMT GENMASK(27, 24)
#define DMA_AXI_WR_OSR_LMT_SHIFT 24
#define DMA_AXI_RD_OSR_LMT GENMASK(19, 16)
#define DMA_AXI_RD_OSR_LMT_SHIFT 16

#define DMA_AXI_OSR_MAX 0xf
#define DMA_AXI_MAX_OSR_LIMIT ((DMA_AXI_OSR_MAX << DMA_AXI_WR_OSR_LMT_SHIFT) | \
                               (DMA_AXI_OSR_MAX << DMA_AXI_RD_OSR_LMT_SHIFT))

#define DMA_SYS_BUS_MB BIT(14)
#define DMA_AXI_1KBBE BIT(13)
#define DMA_SYS_BUS_AAL BIT(12)
#define DMA_SYS_ENAME BIT(11)
#define DMA_AXI_BLEN256 BIT(7)
#define DMA_AXI_BLEN128 BIT(6)
#define DMA_AXI_BLEN64 BIT(5)
#define DMA_AXI_BLEN32 BIT(4)
#define DMA_AXI_BLEN16 BIT(3)
#define DMA_AXI_BLEN8 BIT(2)
#define DMA_AXI_BLEN4 BIT(1)
#define DMA_SYS_BUS_FB BIT(0)
#define DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT		16

#define DMA_BURST_LEN_DEFAULT (DMA_AXI_BLEN256 | DMA_AXI_BLEN128 | \
                               DMA_AXI_BLEN64 | DMA_AXI_BLEN32 |   \
                               DMA_AXI_BLEN16 | DMA_AXI_BLEN8 |    \
                               DMA_AXI_BLEN4)

#define DMA_AXI_BURST_LEN_MASK 0x000000FE

/* Following DMA defines are chanels oriented */
#define DMA_CHAN_BASE_ADDR 0x00001100
#define DMA_CHAN_BASE_OFFSET 0x80
#define DMA_CHANX_BASE_ADDR(x) (DMA_CHAN_BASE_ADDR + \
                                (x * DMA_CHAN_BASE_OFFSET))
#define DMA_CHAN_REG_NUMBER 17

#define DMA_CHAN_CONTROL(x) DMA_CHANX_BASE_ADDR(x)
#define DMA_CHAN_TX_CONTROL(x) (DMA_CHANX_BASE_ADDR(x) + 0x4)
#define DMA_CHAN_RX_CONTROL(x) (DMA_CHANX_BASE_ADDR(x) + 0x8)
#define DMA_CHAN_TX_BASE_ADDR(x) (DMA_CHANX_BASE_ADDR(x) + 0x14)
#define DMA_CHAN_RX_BASE_ADDR(x) (DMA_CHANX_BASE_ADDR(x) + 0x1c)
#define DMA_CHAN_TX_END_ADDR(x) (DMA_CHANX_BASE_ADDR(x) + 0x20)
#define DMA_CHAN_RX_END_ADDR(x) (DMA_CHANX_BASE_ADDR(x) + 0x28)
#define DMA_CHAN_TX_RING_LEN(x) (DMA_CHANX_BASE_ADDR(x) + 0x2c)
#define DMA_CHAN_RX_RING_LEN(x) (DMA_CHANX_BASE_ADDR(x) + 0x30)
#define DMA_CHAN_INTR_ENA(x) (DMA_CHANX_BASE_ADDR(x) + 0x34)
#define DMA_CHAN_RX_WATCHDOG(x) (DMA_CHANX_BASE_ADDR(x) + 0x38)
#define DMA_CHAN_SLOT_CTRL_STATUS(x) (DMA_CHANX_BASE_ADDR(x) + 0x3c)
#define DMA_CHAN_CUR_TX_DESC(x) (DMA_CHANX_BASE_ADDR(x) + 0x44)
#define DMA_CHAN_CUR_RX_DESC(x) (DMA_CHANX_BASE_ADDR(x) + 0x4c)
#define DMA_CHAN_CUR_TX_BUF_ADDR(x) (DMA_CHANX_BASE_ADDR(x) + 0x54)
#define DMA_CHAN_CUR_RX_BUF_ADDR(x) (DMA_CHANX_BASE_ADDR(x) + 0x5c)
#define DMA_CHAN_STATUS(x) (DMA_CHANX_BASE_ADDR(x) + 0x60)

/* DMA Control X */
#define DMA_CONTROL_MSS_MASK GENMASK(13, 0)

/* DMA Tx Channel X Control register defines */
#define DMA_CONTROL_TSE BIT(12)
#define DMA_CONTROL_OSP BIT(4)
#define DMA_CONTROL_ST BIT(0)

/* DMA Rx Channel X Control register defines */
#define DMA_CONTROL_SR BIT(0)

/* Interrupt status per channel */
#define DMA_CHAN_STATUS_REB GENMASK(21, 19)
#define DMA_CHAN_STATUS_REB_SHIFT 19
#define DMA_CHAN_STATUS_TEB GENMASK(18, 16)
#define DMA_CHAN_STATUS_TEB_SHIFT 16
#define DMA_CHAN_STATUS_NIS BIT(15)
#define DMA_CHAN_STATUS_AIS BIT(14)
#define DMA_CHAN_STATUS_CDE BIT(13)
#define DMA_CHAN_STATUS_FBE BIT(12)
#define DMA_CHAN_STATUS_ERI BIT(11)
#define DMA_CHAN_STATUS_ETI BIT(10)
#define DMA_CHAN_STATUS_RWT BIT(9)
#define DMA_CHAN_STATUS_RPS BIT(8)
#define DMA_CHAN_STATUS_RBU BIT(7)
#define DMA_CHAN_STATUS_RI BIT(6)
#define DMA_CHAN_STATUS_TBU BIT(2)
#define DMA_CHAN_STATUS_TPS BIT(1)
#define DMA_CHAN_STATUS_TI BIT(0)

/* Interrupt enable bits per channel */
#define DMA_CHAN_INTR_ENA_NIE BIT(16)
#define DMA_CHAN_INTR_ENA_AIE BIT(15)
#define DMA_CHAN_INTR_ENA_NIE_4_10 BIT(15)
#define DMA_CHAN_INTR_ENA_AIE_4_10 BIT(14)
#define DMA_CHAN_INTR_ENA_CDE BIT(13)
#define DMA_CHAN_INTR_ENA_FBE BIT(12)
#define DMA_CHAN_INTR_ENA_ERE BIT(11)
#define DMA_CHAN_INTR_ENA_ETE BIT(10)
#define DMA_CHAN_INTR_ENA_RWE BIT(9)
#define DMA_CHAN_INTR_ENA_RSE BIT(8)
#define DMA_CHAN_INTR_ENA_RBUE BIT(7)
#define DMA_CHAN_INTR_ENA_RIE BIT(6)
#define DMA_CHAN_INTR_ENA_TBUE BIT(2)
#define DMA_CHAN_INTR_ENA_TSE BIT(1)
#define DMA_CHAN_INTR_ENA_TIE BIT(0)

#define DMA_CHAN_INTR_NORMAL (DMA_CHAN_INTR_ENA_NIE | \
                              DMA_CHAN_INTR_ENA_RIE | \
                              DMA_CHAN_INTR_ENA_TIE)

#define DMA_CHAN_INTR_ABNORMAL (DMA_CHAN_INTR_ENA_AIE | \
                                DMA_CHAN_INTR_ENA_FBE)

/* DMA default interrupt mask for 4.00 */
#define DMA_CHAN_INTR_DEFAULT_MASK (DMA_CHAN_INTR_NORMAL | \
                                    DMA_CHAN_INTR_ABNORMAL)

#define DMA_CHAN_INTR_NORMAL_4_10 (DMA_CHAN_INTR_ENA_NIE_4_10 | \
                                   DMA_CHAN_INTR_ENA_RIE |      \
                                   DMA_CHAN_INTR_ENA_TIE)

#define DMA_CHAN_INTR_ABNORMAL_4_10 (DMA_CHAN_INTR_ENA_AIE_4_10 | \
                                     DMA_CHAN_INTR_ENA_FBE)

/* DMA default interrupt mask for 4.10a */
#define DMA_CHAN_INTR_DEFAULT_MASK_4_10 (DMA_CHAN_INTR_NORMAL_4_10 | \
                                         DMA_CHAN_INTR_ABNORMAL_4_10)

/* channel 0 specific fields */
#define DMA_CHAN0_DBG_STAT_TPS GENMASK(15, 12)
#define DMA_CHAN0_DBG_STAT_TPS_SHIFT 12
#define DMA_CHAN0_DBG_STAT_RPS GENMASK(11, 8)
#define DMA_CHAN0_DBG_STAT_RPS_SHIFT 8

#define DWMAC_REG_W(val, reg) writel(val, reg)
#define DWMAC_REG_R(reg) readl(reg)

#define MII_BUSY 0x00000001
#define MII_WRITE 0x00000002

/* GMAC4 defines */
#define MII_GMAC4_GOC_SHIFT		2
#define MII_GMAC4_WRITE			(1 << MII_GMAC4_GOC_SHIFT)
#define MII_GMAC4_READ			(3 << MII_GMAC4_GOC_SHIFT)
#endif

#define DWMAC_DESCRIPTORS_SIZE	ALIGN(32 * \
				      4, ARCH_DMA_MINALIGN)


// int designware_initialize(u32 base_addr, u32 interface);
int designware_eth_probe(struct udevice *dev);
int designware_eth_of_to_plat(struct udevice *dev);
extern const struct eth_ops designware_eth_ops;

struct dw_eth_dev {
	char txbuffs[TX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);
	char rxbuffs[RX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);

	u32 interface;
	u32 max_speed;
	u32 tx_currdescnum;
	u32 rx_currdescnum;

	phys_addr_t mac_base;
#ifndef CONFIG_DM_ETH
	struct eth_device *dev;
#endif
#ifdef CONFIG_DM_GPIO
	struct gpio_desc reset_gpio;
#endif
#ifdef CONFIG_CLK
	struct clk *clocks;	/* clock list */
	int clock_count;	/* number of clock in clock list */
#endif

	struct phy_device *phydev;
	struct mii_dev *bus;
};

struct dw_eth_pdata {
	struct eth_pdata eth_pdata;
};
