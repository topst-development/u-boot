// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pci.h>
#include <generic-phy.h>
#include <dm/pinctrl.h>
#include <asm/global_data.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <pcie_dw_telechips.h>
#include "pcie_dw_common.h"

/*
 * PCIe controller wrapper link configuration registers
 */
#define PCIE_LINK_CFG04		(0x010U)
#define PCIE_LINK_CFG08		(0x020U)
#define PCIE_LINK_CFG25		(0x064U)
#define PCIE_LINK_CFG26		(0x068U)
#define PCIE_LINK_CFG33		(0x084U)
#define PCIE_LINK_CFG44		(0x0b0U)
#define PCIE_LINK_CFG53		(0x0d4U)
#define PCIE_LINK_CFG89		(0x164U)

/*
 * PCIe controller wrapper clock configuration registers
 */
#define PCIE_CLK_CFG00		(0x00U)
#define PCIE_CLK_CFG04		(0x10U)
#define PCIE_CLK_CFG06		(0x18U)
#define PCIE_CLK_CFG07		(0x1CU)
#define PCIE_CLK_CFG08		(0x20U)

/*
 * PCIe controller wrapper dbi configuration registers
 */
#define PCIE_PORT_LOGIC_OFF      (0x700U)
#define PCIE_GEN3_EQ_CTRL		(0x1A8U + PCIE_PORT_LOGIC_OFF)
#define PCIE_MISC_CONTROL1		(0x1BCU + PCIE_PORT_LOGIC_OFF)
#define PCIE_AMBA_ORDERING_CTRL		(0x1D8U + PCIE_PORT_LOGIC_OFF)
#define PCIE_GEN4_LANE_MARGINING2		(0x480U + PCIE_PORT_LOGIC_OFF)
#define PCIE_GEN3_RELATED_OFF		(0x890U)

#define PCIE_LINK_CFG_APP_LTSSM_ENABLE_MASK		((uint32_t)BIT(2))
#define PCIE_LINK_CFG_RDLH_LINK_UP_MASK		((uint32_t)BIT(22))
#define PCIE_LINK_CFG_SMLH_LINK_UP_MASK		((uint32_t)BIT(17))

#define PCIE_DEVICE_RC		(0x4U)
#define PCIE_DEVICE_EP		(0x0U)
#define LINK_UP		(1)
#define LINK_DW		(0)
#define PCIE_LINK_UP_TIMEOUT_MS		(1000U)
#define LINK_SPEED_GEN_4        0x4

#define PCIE_AXI_SLAVE_IF_OFFSET		(0x500000000ULL)

#define GEN3_EQ_PSET_REQ_VEC_SHIFT		(8U)
#define GEN3_EQ_PSET_REQ_VEC_MASK		(0xFFFFU << GEN3_EQ_PSET_REQ_VEC_SHIFT)

#define AX_MSTR_ORDR_P_EVENT_SEL_SHIFT		(3)
#define AX_MSTR_ORDR_P_EVENT_SEL_MASK		(0x3U << AX_MSTR_ORDR_P_EVENT_SEL_SHIFT)

#define GEN4_RXMARGIN_MAX_VOLTAGE_OFFSET_SHIFT		(24U)
#define GEN4_RXMARGIN_NUM_VOLTAGE_STEPS_SHIFT		(16U)
#define GEN4_RXMARGIN_MAX_TIMING_OFFSET_SHIFT		(8U)
#define GEN4_RXMARGIN_NUM_TIMING_STEPS_SHIFT		(0U)
#define GEN4_RXMARGIN_MAX_VOLTAGE_OFFSET_MASK		((u32)0x3FU << GEN4_RXMARGIN_MAX_VOLTAGE_OFFSET_SHIFT)
#define GEN4_RXMARGIN_NUM_VOLTAGE_STEPS_MASK		((u32)0x7FU << GEN4_RXMARGIN_NUM_VOLTAGE_STEPS_SHIFT)
#define GEN4_RXMARGIN_MAX_TIMING_OFFSET_MASK		((u32)0x3FU << GEN4_RXMARGIN_MAX_TIMING_OFFSET_SHIFT)
#define GEN4_RXMARGIN_NUM_TIMING_STEPS_MASK		((u32)0x3FU << GEN4_RXMARGIN_NUM_TIMING_STEPS_SHIFT)

/**
 * struct pcie_dw_tcc - Telechips DW PCIe controller state
 *
 * @pci: The common PCIe DW structure
 * @app_base: The base address of application register space
 */
struct pcie_dw_tcc {
	/* Must be first member of the struct */
	struct pcie_dw dw;
	struct phy phy;
	void *link_base;
	void *clk_base;
	int32_t max_link_speed;
	int32_t pms;
};

enum pcie_dw_tcc_reg_type {
	PCIE_DW_TCC_REG_UNKNOWN,
	PCIE_DW_TCC_REG_DBI,
	PCIE_DW_TCC_REG_CLK,
	PCIE_DW_TCC_REG_LINK,
};

/* Global Variables used to support PCIe SQ tuning */
static struct pcie_dw_tcc *g_pDevPriv;

static uint32_t pcie_dw_tcc_readl(const struct pcie_dw_tcc *pci,
		enum pcie_dw_tcc_reg_type type,
		u32 offset)
{
	uint32_t val = 0U;

	if (pci != NULL) {
		switch(type) {
		case PCIE_DW_TCC_REG_DBI:
			val = readl(pci->dw.dbi_base + offset);
			break;
		case PCIE_DW_TCC_REG_CLK:
			val = readl(pci->clk_base + offset);
			break;
		case PCIE_DW_TCC_REG_LINK:
			val = readl(pci->link_base + offset);
			break;
		case PCIE_DW_TCC_REG_UNKNOWN:
		default:
			(void)printf("ERROR: Invalid reg type (%d)", type);
			break;
		}
	}

	return val;
}

static void pcie_dw_tcc_writel(const struct pcie_dw_tcc *pci,
		enum pcie_dw_tcc_reg_type type,
		u32 reg, u32 val, u32 mask)
{
	if (pci != NULL) {
		switch(type) {
		case PCIE_DW_TCC_REG_DBI:
			writel(((readl(pci->dw.dbi_base + reg) & ~mask) | val),
					pci->dw.dbi_base + reg);
			break;
		case PCIE_DW_TCC_REG_CLK:
			writel(((readl(pci->clk_base + reg) & ~mask) | val),
					pci->clk_base + reg);
			break;
		case PCIE_DW_TCC_REG_LINK:
			writel(((readl(pci->link_base + reg) & ~mask) | val),
					pci->link_base + reg);
			break;
		case PCIE_DW_TCC_REG_UNKNOWN:
		default:
			(void)printf("ERROR: Invalid reg type (%d)", type);
			break;
		}
	}
}

static int32_t pcie_dw_tcc_set_reset_gpio(struct udevice *dev, int32_t value)
{
	int32_t err = 0;

	if (dev != NULL) {
		if (value != 0) {
			err = pinctrl_select_state(dev, "pcie_rst_on");
		} else {
			err = pinctrl_select_state(dev, "pcie_rst_off");
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_is_link_up(const struct pcie_dw_tcc *pci)
{
	int32_t err = LINK_DW;

	if (pci != NULL) {
		uint32_t val, mask;

		mask = PCIE_LINK_CFG_RDLH_LINK_UP_MASK;
		val = pcie_dw_tcc_readl(pci, PCIE_DW_TCC_REG_LINK, PCIE_LINK_CFG26) & mask;
		if (val != 0U) {
			mask = PCIE_LINK_CFG_SMLH_LINK_UP_MASK;
			val = pcie_dw_tcc_readl(pci, PCIE_DW_TCC_REG_LINK, PCIE_LINK_CFG33) & mask;
			if (val != 0U) {
				err = LINK_UP;
			}
		}
	}

	return err;
}

static int32_t pcie_dw_tcc_check_link_up(const struct pcie_dw_tcc *pci)
{
	int32_t err = LINK_DW;

	if (pci != NULL) {
		unsigned long timeout;

		timeout = get_timer(0) + PCIE_LINK_UP_TIMEOUT_MS;
		do {
			err = pcie_dw_tcc_is_link_up(pci);
			if (get_timer(0) > timeout) {
				break;
			} else {
				udelay(100);
			}
		} while(err != LINK_UP);
	}

	return err;
}

static int32_t pcie_dw_tcc_enable_ltssm(const struct pcie_dw_tcc *pci, bool enable)
{
	int32_t err = 0;

	if (pci != NULL) {
		uint32_t val, mask;

		mask = PCIE_LINK_CFG_APP_LTSSM_ENABLE_MASK;
		val = enable ? mask : 0x0U;
		pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_LINK, PCIE_LINK_CFG04,
				val, mask);
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_establish_link(struct pcie_dw_tcc *pci)
{
	int32_t err = 0;

	if (pci != NULL) {
		err = pcie_dw_tcc_enable_ltssm(pci, true);
		if (err == 0) {
			err = pcie_dw_tcc_check_link_up((const struct pcie_dw_tcc *)pci);
			if (err == LINK_DW) {
				(void)printf("PCIE-%d: Link down\n", dev_seq(pci->dw.dev));
			} else {
				(void)printf("PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n", dev_seq(pci->dw.dev),
						pcie_dw_get_link_speed(&pci->dw),
						pcie_dw_get_link_width(&pci->dw),
						pci->dw.first_busno);
			}
		}
	}

	return err;
}

static int32_t pcie_dw_tcc_host_init(struct pcie_dw_tcc *priv)
{
	int32_t err = 0;

	if (priv != NULL) {
		if (pcie_dw_tcc_is_link_up(priv) != 0) {
			(void)printf("PCI Link already up before configuration!\n");
		} else {
			struct pcie_dw *pci = &priv->dw;
			struct udevice *dev = pci->dev;

			/* setup PCIe controller for RC mode */
			pcie_dw_setup_host(pci);

			/* deassert reset gpio */
			err = pcie_dw_tcc_set_reset_gpio(dev, 1);
			if (err == 0) {
				err = pcie_dw_tcc_establish_link(priv);
			}
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_set_layer(struct pcie_dw_tcc *pci)
{
	int32_t err = 0;

	if (pci != NULL) {
		uint32_t val, mask;

		dw_pcie_dbi_write_enable(&pci->dw, true);

		if (pci->max_link_speed >= LINK_SPEED_GEN_2) {
			val = 0x20000U;
			mask = 0x0U;
			pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_DBI,
					PCIE_LINK_WIDTH_SPEED_CONTROL, val, mask);
		}

		if (pci->max_link_speed >= LINK_SPEED_GEN_3) {
			val = 0x800U;
			mask = 0x0U;
			pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_DBI,
					PCIE_GEN3_RELATED_OFF, val, mask);
		}

		dw_pcie_dbi_write_enable(&pci->dw, false);
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_set_max_link_speed(struct pcie_dw_tcc *pci)
{
	int32_t err = 0;

	if (pci != NULL) {
		uint32_t val, mask;

		if (pci->max_link_speed <= LINK_SPEED_GEN_4) {
			val = (u32)pci->max_link_speed;
			mask = TARGET_LINK_SPEED_MASK;
			dw_pcie_dbi_write_enable(&pci->dw, true);
			pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_DBI,
					PCIE_LINK_CTL_2, val, mask);
			dw_pcie_dbi_write_enable(&pci->dw, false);
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_gen4_phy_setup(struct pcie_dw_tcc *priv)
{
	int32_t err = 0;

	if (priv != NULL) {
		struct pcie_dw *pci = &priv->dw;

		if (priv->max_link_speed == LINK_SPEED_GEN_4) {
			uint32_t val, mask;

			dw_pcie_dbi_write_enable(pci, true);

			val = (0x59FU << GEN3_EQ_PSET_REQ_VEC_SHIFT);
			mask = GEN3_EQ_PSET_REQ_VEC_MASK;
			pcie_dw_tcc_writel(priv, PCIE_DW_TCC_REG_DBI,
					PCIE_GEN3_EQ_CTRL, val, mask);

			mask = GEN4_RXMARGIN_MAX_VOLTAGE_OFFSET_MASK |
				GEN4_RXMARGIN_NUM_VOLTAGE_STEPS_MASK |
				GEN4_RXMARGIN_MAX_TIMING_OFFSET_MASK |
				GEN4_RXMARGIN_NUM_TIMING_STEPS_MASK;
			val = ((u32)0x1FU << GEN4_RXMARGIN_NUM_TIMING_STEPS_SHIFT);
			val |= ((u32)0x32U << GEN4_RXMARGIN_MAX_TIMING_OFFSET_SHIFT);
			val |= ((u32)0x7FU << GEN4_RXMARGIN_NUM_VOLTAGE_STEPS_SHIFT);
			val |= ((u32)0x28U << GEN4_RXMARGIN_MAX_VOLTAGE_OFFSET_SHIFT);
			pcie_dw_tcc_writel(priv, PCIE_DW_TCC_REG_DBI,
					PCIE_GEN4_LANE_MARGINING2, val, mask);

			mask = AX_MSTR_ORDR_P_EVENT_SEL_MASK;
			val = (0x1U << AX_MSTR_ORDR_P_EVENT_SEL_SHIFT);
			pcie_dw_tcc_writel(priv, PCIE_DW_TCC_REG_DBI,
					PCIE_AMBA_ORDERING_CTRL, val, mask);

			dw_pcie_dbi_write_enable(pci, false);
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_set_refclk(const struct pcie_dw_tcc *pci)
{
	int32_t err = 0;

	if (pci != NULL) {
		uint32_t val, mask;

		if (pci->pms < 0) {
			err = -ENODEV;
		}

		if (err == 0) {
			if (pci->max_link_speed == LINK_SPEED_GEN_4) {
				mask = 0x1C0U;
				val = mask;
				pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_LINK, PCIE_LINK_CFG08, val, mask);

				val = 0x0U;
				mask = 0x100000U;
				pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_CLK, PCIE_CLK_CFG07, val, mask);

				mask = 0x80000000U;
				pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_CLK, PCIE_CLK_CFG00, val, mask);
				udelay(10);

				mask = 0x40000000U;
				val |= mask;
				pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_CLK, PCIE_CLK_CFG06, val, mask);
				udelay(100);

				mask = 0x80000000U;
				val |= mask;
				pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_CLK, PCIE_CLK_CFG00, val, mask);

				mask = 0x80U;
				do {
					val = pcie_dw_tcc_readl(pci, PCIE_DW_TCC_REG_CLK, PCIE_CLK_CFG08) & mask;
				} while (val == 0x0U);

				mask = 0x40031U;
				val |= mask;
				pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_CLK, PCIE_CLK_CFG04, val, mask);

				mdelay(15);
			} else {
				val = 0x0U;
				mask = 0x80000000U;
				pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_CLK, PCIE_CLK_CFG00, val, mask);

				val = ((u32)pci->pms | 0x80000000U);
				mask = 0xFFFFFFFFU;
				pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_CLK, PCIE_CLK_CFG00, val, mask);

				do {
					val = pcie_dw_tcc_readl(pci, PCIE_DW_TCC_REG_CLK, PCIE_CLK_CFG00);
				} while ((val & 0x800000U) != 0x0U);

				val = 0x40021U;
				mask = 0x0U;
				pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_CLK, PCIE_CLK_CFG04, val, mask);
			}
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_set_mode(const struct pcie_dw_tcc *pci, uint32_t type)
{
	int32_t err = 0;

	if (pci != NULL) {
		uint32_t val, mask;

		val = (type << 6);
		mask = (0xFU << 6);
		pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_LINK, PCIE_LINK_CFG89, val, mask);
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_set_slv_addr_mask(const struct pcie_dw_tcc *pci)
{
	int32_t err = 0;

	if (pci != NULL) {
		uint32_t val, mask;

		val = 0x0U;
		mask = 0xFFFFFFFFU;
		pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_LINK, PCIE_LINK_CFG53, val, mask);
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_add_pcie_port(struct pcie_dw_tcc *pci)
{
	int32_t err = 0;

	if (pci != NULL) {
		err = pcie_dw_tcc_set_slv_addr_mask((const struct pcie_dw_tcc *)pci);
		if (err == 0) {
			/* set PCIe controller mode rc or ep*/
			err = pcie_dw_tcc_set_mode((const struct pcie_dw_tcc *)pci, PCIE_DEVICE_RC);
		}

		if (err == 0) {
			/* set reference clock source */
			err = pcie_dw_tcc_set_refclk(pci);
		}

		if (err == 0) {
			err = pcie_dw_tcc_gen4_phy_setup(pci);
		}

		if (err == 0) {
			/* set PCIe Target Link Speed */
			err = pcie_dw_tcc_set_max_link_speed(pci);
		}

		if (err == 0) {
			err = pcie_dw_tcc_set_layer(pci);
		}

		if (err == 0) {
			err = pcie_dw_tcc_host_init(pci);
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_assert_global_reset(const struct pcie_dw_tcc *pci)
{
	int32_t err = 0;

	if (pci != NULL) {
		uint32_t val, mask;

		val = 0x0U;
		mask = 0x7FU;
		pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_LINK, PCIE_LINK_CFG44, val, mask);

		val = 0x7FU;
		mask = val;
		pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_LINK, PCIE_LINK_CFG44, val, mask);
		udelay(500);
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_clear_interrupt(const struct pcie_dw_tcc *pci)
{
	int32_t err = 0;

	if (pci != NULL) {
		uint32_t val, mask;

		val = 0xFFFFFFFFU;
		mask = val;
		pcie_dw_tcc_writel(pci, PCIE_DW_TCC_REG_LINK, PCIE_LINK_CFG25, val, mask);
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_ctl_setup(struct udevice *dev, struct pcie_dw_tcc *pci)
{
	int32_t err = 0;

	if ((pci != NULL) && (dev != NULL)) {
		/* assert reset gpio */
		err = pcie_dw_tcc_set_reset_gpio(dev, 0);
		if (err == 0) {
			/* clear all interrupt signals */
			err = pcie_dw_tcc_clear_interrupt(pci);
		}

		if (err == 0) {
			/* assert global reset */
			err = pcie_dw_tcc_assert_global_reset(pci);
		}

		if (err == 0) {
			err = generic_phy_reset(&pci->phy);
		}

		if (err == 0) {
			pci->dw.first_busno = dev_seq(dev);
			pci->dw.dev = dev;
			err = pcie_dw_tcc_add_pcie_port(pci);
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static int32_t pcie_dw_tcc_phy_setup(struct udevice *dev, struct phy *phy)
{
	int32_t err = 0;

	if (dev != NULL) {
		err = generic_phy_power_on(phy);
		if (err == 0) {
			err = generic_phy_init(phy);
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

/**
 * pcie_dw_tcc_probe() - Probe the PCIe bus for active link
 *
 * @dev: A pointer to the device being operated on
 *
 * Probe for an active link on the PCIe bus and configure the controller
 * to enable this port.
 *
 * Return: 0 on success, else -ENODEV
 */
static int32_t pcie_dw_tcc_probe(struct udevice *dev)
{
	int32_t err = 0;

	if (dev != NULL) {
		struct pcie_dw_tcc *pci = dev_get_priv(dev);
		struct udevice *ctlr = pci_get_controller(dev);
		struct pci_controller *hose = dev_get_uclass_priv(ctlr);
		int32_t idx;

		for (idx = 0; idx < hose->region_count; idx++) {
			hose->regions[idx].phys_start += PCIE_AXI_SLAVE_IF_OFFSET;
		}

		err = pcie_dw_tcc_phy_setup(dev, &pci->phy);
		if (err == 0) {
			err = pcie_dw_tcc_ctl_setup(dev, pci);
		}

		if (err == LINK_UP) {
			if (pci->dw.mem.size < UINT_MAX) {
				pcie_dw_prog_outbound_atu_unroll(
						&pci->dw, PCIE_ATU_REGION_INDEX0,
						PCIE_ATU_TYPE_MEM,
						pci->dw.mem.phys_start,
						pci->dw.mem.bus_start, (u32)pci->dw.mem.size);
			}

			/* set global variable */
			g_pDevPriv = pci;
			err = 0;
		}
	} else {
		err = -ENODEV;
	}

	return err;
}

/**
 * pcie_dw_tcc_of_to_plat() - Translate from DT to device state
 *
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int32_t pcie_dw_tcc_of_to_plat(struct udevice *dev)
{
	int32_t err = 0;

	if (dev != NULL) {
		struct pcie_dw_tcc *pci = dev_get_priv(dev);
		fdt_addr_t addr;
		fdt_size_t size;

		addr = dev_read_addr_size_name(dev, "dbi", &size);
		if (addr == FDT_ADDR_T_NONE) {
			err = -ENODEV;
		} else {
			pci->dw.dbi_base = (void __iomem *)addr;
		}

		if (err == 0) {
			addr = dev_read_addr_size_name(dev, "config", &pci->dw.cfg_size);
			if (addr == FDT_ADDR_T_NONE) {
				err = -ENODEV;
			} else {
				pci->dw.cfg_base = (void __iomem *)addr;
				pci->dw.cfg_base += PCIE_AXI_SLAVE_IF_OFFSET;
			}
		}

		if (err == 0) {
			addr = dev_read_addr_name(dev, "link");
			if (addr == FDT_ADDR_T_NONE) {
				err = -ENODEV;
			} else {
				pci->link_base = (void __iomem *)addr;
			}
		}

		if (err == 0) {
			addr = dev_read_addr_name(dev, "clk");
			if (addr == FDT_ADDR_T_NONE) {
				err = -ENODEV;
			} else {
				pci->clk_base = (void __iomem *)addr;
			}
		}

		if (err == 0) {
			pci->max_link_speed = dev_read_u32_default(dev, "max_link_speed", 1);
			if  (pci->max_link_speed == 0) {
				pci->max_link_speed = LINK_SPEED_GEN_3;
			}
		}

		if (err == 0) {
			err = generic_phy_get_by_name(dev, "pcie_phy", &pci->phy);
		}

		if (err == 0) {
			pci->pms = dev_read_u32_default(dev, "pms", 0);
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

int32_t pcie_dw_tcc_set_phy_conf(void *params)
{
	int32_t err = 0;

	if ((g_pDevPriv != NULL) && (params != NULL)) {
		struct pcie_dw_tcc *pci = g_pDevPriv;

		if (pcie_dw_tcc_is_link_up(pci) == LINK_UP) {
			err = generic_phy_configure(&pci->phy, params);
		}
	} else {
		err = -EINVAL;
	}

	return err;
}

static const struct dm_pci_ops pcie_dw_tcc_ops = {
	.read_config	= pcie_dw_read_config,
	.write_config	= pcie_dw_write_config,
};

static const struct udevice_id pcie_dw_tcc_ids[] = {
	{ .compatible = "telechips,pcie-rc" },
	{ }
};

U_BOOT_DRIVER(pcie_dw_tcc) = {
	.name			= "tcc-pcie",
	.id			= UCLASS_PCI,
	.of_match		= pcie_dw_tcc_ids,
	.ops			= &pcie_dw_tcc_ops,
	.of_to_plat	= pcie_dw_tcc_of_to_plat,
	.probe			= pcie_dw_tcc_probe,
	.priv_auto	= sizeof(struct pcie_dw_tcc),
};
