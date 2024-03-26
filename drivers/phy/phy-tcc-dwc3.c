// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <irq.h>
#include <irq_func.h>
#include <regmap.h>
#include <dm/device_compat.h>

//#define TCC_USB_VERIFICATION

#define U30_PCFG0               (0x10U)
#define U30_PCFG4               (0x20U)
#define U30_PCFG5               (0x24U)
#define U30_PCFG6               (0x28U)
#define U30_PCFG13              (0x44U)
#define U30_PCFG15              (0x4CU)
#define U30_PINT                (0x7CU)
#define U30_LCFG                (0x80U)
#define U30_FPCFG0              (0xA0U)
#define U30_FPCFG1              (0xA4U)
#define U30_FPCFG2              (0xA8U)
#define U30_FPCFG4              (0xB0U)

struct tcc_dwc3_usb_phy {
	struct device	*dev;
	struct regmap	*reg_map;
#if defined(TCC_USB_VERIFICATION)
	int32_t irq_bc;
	int32_t irq_ls;
#endif /* TCC_USB_VERIFICATION */
};

#if defined(TCC_USB_VERIFICATION)
static void tcc_dwc3_usb_print_pcfg_reset_value(const struct phy *usb_phy)
{
	const struct udevice *dev = usb_phy->dev;
	const struct tcc_dwc3_usb_phy *priv = dev_get_priv(dev);
	uint32_t val;

	(void)regmap_read(priv->reg_map, U30_PCFG13, &val);
	pr_info("[XHCI USB 3.0 PHY PCFG13]\n");
	pr_info("Reset value : 0x%08x\n", val);

	(void)regmap_read(priv->reg_map, U30_PCFG15, &val);
	pr_info("[XHCI USB 3.0 PHY PCFG15]\n");
	pr_info("Reset value : 0x%08x\n", val);

	(void)regmap_read(priv->reg_map, U30_FPCFG1, &val);
	pr_info("[XHCI USB 2.0 PHY FPCFG1]\n");
	pr_info("Reset value : 0x%08x\n", val);
}

static void tcc_dwc3_usb_print_pcfg_set_value(const struct phy *usb_phy)
{
	const struct udevice *dev = usb_phy->dev;
	const struct tcc_dwc3_usb_phy *priv = dev_get_priv(dev);
	uint32_t pcfg13 = 0x31C71457;
	uint32_t pcfg15 = 0xA4C4302AU;
	uint32_t fpcfg1 = 0xE31C243CU;
	uint32_t val;

	(void)regmap_read(priv->reg_map, U30_PCFG13, &val);
	pr_info("[XHCI USB 3.0 PHY PCFG13]\n");
	pr_info("The value we are going to set is 0x%08x\n", pcfg13);
	pr_info("Actually set value : 0x%08x\n", val);

	(void)regmap_read(priv->reg_map, U30_PCFG15, &val);
	pr_info("[XHCI USB 3.0 PHY PCFG15]\n");
	pr_info("The value we are going to set is 0x%08x\n", pcfg15);
	pr_info("Actually set value : 0x%08x\n", val);

	(void)regmap_read(priv->reg_map, U30_FPCFG1, &val);
	pr_info("[XHCI USB 2.0 PHY FPCFG1]\n");
	pr_info("The value we are going to set is 0x%08x\n", fpcfg1);
	pr_info("Actually set value : 0x%08x\n", val);
}

static void tcc_dwc3_usb_phy_irq_handler(void *arg)
{
	const struct udevice *dev = arg;
	const struct tcc_dwc3_usb_phy *priv = dev_get_priv(dev);
	int32_t count = 3;
	uint32_t val;

	(void)regmap_update_bits(priv->reg_map, U30_PINT,
			(uint)BIT(22), (uint)BIT(22));
	udelay(1);
	(void)regmap_update_bits(priv->reg_map, U30_PINT,
			(uint)BIT(22), 0);

	while (count > 0) {
		(void)regmap_read(priv->reg_map, U30_FPCFG2, &val);
		if ((val & BIT(22)) != 0U) {
			break;
		}

		mdelay(1);
		count--;
	}

	if (count == 0) {
		pr_err("[%s] Failed to detect charger!!\n", __func__);
	} else {
		(void)regmap_read(priv->reg_map, U30_FPCFG2, &val);
		pr_notice("[%s] Charger detection interrupt!\n", __func__);
		pr_notice("Charger detection IRQ bit(22): %d (FPCFG2: 0x%08x)\n",
				((val & BIT(22)) != 0U) ? 1 : 0, val);

		(void)regmap_update_bits(priv->reg_map, U30_FPCFG2,
				(uint)BIT(9), (uint)BIT(9));
		mdelay(100);
		(void)regmap_update_bits(priv->reg_map, U30_FPCFG2,
				(uint)(BIT(8) | BIT(9)), 0);
	}

	(void)regmap_update_bits(priv->reg_map, U30_FPCFG4,
			(uint)BIT(31), (uint)BIT(31));
	udelay(10);
	(void)regmap_update_bits(priv->reg_map, U30_FPCFG4,
			(uint)BIT(31), 0);

	pr_notice("[%s] Enable charger detection!!!\n", __func__);
}

static void tcc_dwc3_usb_line_state_irq_handler(void *arg)
{
	const struct udevice *dev = arg;
	const struct tcc_dwc3_usb_phy *priv = dev_get_priv(dev);
	uint32_t val;

	(void)regmap_read(priv->reg_map, U30_FPCFG0, &val);
	pr_notice("[%s] Line state interrupt!\n", __func__);
	pr_notice("INTR_RSTn: %d, WKUP_RSTn: %d (FPCFG0: 0x%08x)\n",
			((val & BIT(23)) != 0U) ? 1 : 0,
			((val & BIT(22)) != 0U) ? 1 : 0, val);
	pr_notice("Wakeup source: %d\n",
			(((*(volatile uint32_t *)0x14400020) & BIT(7)) != 0U) ?
			1 : 0);

	(void)regmap_update_bits(priv->reg_map, U30_FPCFG0,
			(uint)BIT(23), 0);
	udelay(5);
	(void)regmap_update_bits(priv->reg_map, U30_FPCFG0,
			(uint)BIT(23), (uint)BIT(23));
}
#endif /* TCC_USB_VERIFICATION */

static int32_t tcc_dwc3_usb_phy_init(struct phy *usb_phy)
{
	const struct udevice *dev;
	const struct tcc_dwc3_usb_phy *priv;
	int32_t ret = 0;
	int32_t i = 0;
	uint32_t val;
	uint32_t fpcfg1 = 0xE31C243CU;
	uint32_t pcfg13 = 0x31C71457;
	uint32_t pcfg15 = 0xA4C4302AU;

	if (usb_phy == NULL) {
		pr_err("[%s] Failed to find USB PHY\n", __func__);
		ret = -ENODEV;
	} else {
		dev = usb_phy->dev;
		priv = dev_get_priv(dev);

#if defined(TCC_USB_VERIFICATION)
		tcc_dwc3_usb_print_pcfg_reset_value(usb_phy);
#endif /* TCC_USB_VERIFICATION */

#ifdef CONFIG_TCC807X
		// Set USB 3.0 High-speed PHY Reference Clock Frequency
		(void)regmap_update_bits(priv->reg_map, U30_FPCFG0,
				(uint)(BIT(2) | BIT(0)), 0);
		(void)regmap_update_bits(priv->reg_map, U30_FPCFG0,
				(uint)(BIT(3) | BIT(1)),
				(uint)(BIT(3) | BIT(1)));

		// Set USB 3.0 PHY Reference Clock Frequency
		(void)regmap_update_bits(priv->reg_map, U30_PCFG5,
				(uint)BIT(20), 0);
		(void)regmap_update_bits(priv->reg_map, U30_PCFG5,
				(uint)(BIT(21) | BIT(17)),
				(uint)(BIT(21) | BIT(17)));

		(void)regmap_update_bits(priv->reg_map, U30_PCFG6,
				(uint)(BIT(22) | BIT(21) | BIT(18) | BIT(17)
					| BIT(9) | BIT(6) | BIT(5)),
				(uint)(BIT(22) | BIT(21) | BIT(18) | BIT(17)
					| BIT(9) | BIT(6) | BIT(5)));
#endif

		/* PHY POR */
		(void)regmap_update_bits(priv->reg_map, U30_PCFG4,
				(uint)BIT(0), (uint)BIT(0));
		(void)regmap_update_bits(priv->reg_map, U30_LCFG,
				(uint)BIT(31), 0);
		(void)regmap_update_bits(priv->reg_map, U30_PCFG0,
				(uint)BIT(30), 0);
		(void)regmap_update_bits(priv->reg_map, U30_FPCFG0,
				(uint)BIT(31), (uint)BIT(31));

		/* Turn On SS Circuits */
		(void)regmap_update_bits(priv->reg_map, U30_PCFG0,
				(uint)(BIT(24) | BIT(25)), 0);
		udelay(5);
		(void)regmap_update_bits(priv->reg_map, U30_PCFG0,
				(uint)BIT(30), (uint)BIT(30));

		/* External SRAM Init Done Wait */
		(void)regmap_read(priv->reg_map, U30_PCFG0, &val);
		while ((val & BIT(5)) == 0U) {
			udelay(1);
			(void)regmap_read(priv->reg_map, U30_PCFG0, &val);
		}

		/* External SRAM LD Done SET */
		(void)regmap_update_bits(priv->reg_map, U30_PCFG0,
				(uint)BIT(3), (uint)BIT(3));

		/* Set TXVRT to 0xA */
		(void)regmap_write(priv->reg_map, U30_FPCFG1, fpcfg1);

		/* Set Tx vboost level to 0x7 */
		(void)regmap_write(priv->reg_map, U30_PCFG13, pcfg13);

		/* Set Tx iboost level to 0xA */
		(void)regmap_write(priv->reg_map, U30_PCFG15, pcfg15);

#if defined(TCC_USB_VERIFICATION)
		tcc_dwc3_usb_print_pcfg_set_value(usb_phy);
#endif /* TCC_USB_VERIFICATION */

		/* USB 2.0 PHY POR Release */
		(void)regmap_update_bits(priv->reg_map, U30_FPCFG0,
				(uint)BIT(24), 0);
		(void)regmap_update_bits(priv->reg_map, U30_FPCFG0,
				(uint)BIT(31), 0);

		/* LINK Reset Release */
		(void)regmap_update_bits(priv->reg_map, U30_LCFG,
				(uint)BIT(31), (uint)BIT(31));

		(void)regmap_read(priv->reg_map, U30_PCFG0, &val);
		while (i < 10000) {
			if ((val & BIT(2)) != 0U) {
				pr_info("USB 3.0 PHY Check Valid!\n");
				pr_info("PHY Valid Bit(2): %d (PCFG0: 0x%08x)\n",
						((val & BIT(2)) != 0U) ? 1 : 0,
						val);
				break;
			}

			i++;
			udelay(5);
			(void)regmap_read(priv->reg_map, U30_PCFG0, &val);
		}

		(void)regmap_update_bits(priv->reg_map, U30_LCFG,
				(uint)(BIT(27) | BIT(26) | BIT(19) |
					BIT(18) | BIT(17) | BIT(7)),
				(uint)(BIT(27) | BIT(26) | BIT(19) |
					BIT(18) | BIT(17) | BIT(7)));

#if defined(TCC_USB_VERIFICATION)
		if (priv->irq_bc != 0) {
			/* Clear IRQ */
			(void)regmap_update_bits(priv->reg_map, U30_FPCFG4,
					(uint)BIT(31), (uint)BIT(31));

			/* Disable PHY-Valid Detect */
			(void)regmap_update_bits(priv->reg_map, U30_FPCFG4,
					(uint)BIT(30), 0);

			/* Clear IRQ */
			(void)regmap_update_bits(priv->reg_map, U30_FPCFG4,
					(uint)BIT(31), 0);
			udelay(1);
			(void)regmap_update_bits(priv->reg_map, U30_FPCFG2,
					(uint)(BIT(8) | BIT(10)),
					(uint)(BIT(8) | BIT(10)));
			udelay(1);

			/* Enable CHG_DET interrupt */
			(void)regmap_update_bits(priv->reg_map, U30_FPCFG4,
					(uint)BIT(28), (uint)BIT(28));

			(void)regmap_update_bits(priv->reg_map, U30_PINT,
					(uint)BIT(6), 0);
			(void)regmap_update_bits(priv->reg_map, U30_PINT,
					(uint)(BIT(0) | BIT(1) | BIT(2) |
						BIT(3) | BIT(4) | BIT(5) |
						BIT(7) | BIT(31)),
					(uint)(BIT(1) | BIT(2) | BIT(3) |
						BIT(4) | BIT(5) | BIT(7) |
						BIT(31)));

			irq_unmask(priv->irq_bc);
		}

		if (priv->irq_ls != 0) {
			(void)regmap_update_bits(priv->reg_map, U30_FPCFG0,
					(uint)(BIT(22) | BIT(23)),
					(uint)(BIT(22) | BIT(23)));

			irq_unmask(priv->irq_ls);
		}
#endif /* TCC_USB_VERIFICATION */
	}

	return ret;
}

static int32_t tcc_dwc3_usb_phy_exit(struct phy *usb_phy)
{
	const struct udevice *dev;
	const struct tcc_dwc3_usb_phy *priv;
	int32_t ret = 0;

	if (usb_phy == NULL) {
		pr_err("[%s] Failed to find USB PHY\n", __func__);
		ret = -ENODEV;
	} else {
		dev = usb_phy->dev;
		priv = dev_get_priv(dev);

#if defined(TCC_USB_VERIFICATION)
		if (priv->irq_bc != 0) {
			(void)regmap_update_bits(priv->reg_map, U30_FPCFG2,
					(uint)BIT(8), (uint)BIT(8));

			irq_mask(priv->irq_bc);
		}

		if (priv->irq_ls != 0) {
			irq_mask(priv->irq_ls);
		}
#endif /* TCC_USB_VERIFICATION */
	}

	return ret;
}

static struct phy_ops tcc_dwc3_usb_phy_ops = {
	.init = tcc_dwc3_usb_phy_init,
	.exit = tcc_dwc3_usb_phy_exit,
};

static int32_t tcc_dwc3_usb_phy_probe(struct udevice *dev)
{
	struct tcc_dwc3_usb_phy *priv = dev_get_priv(dev);
	int32_t ret;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->reg_map);
	if (ret != 0) {
		pr_err("[%s] Failed to regmap_init_mem()\n", __func__);
	} else {
#if defined(TCC_USB_VERIFICATION)
		if (dev_read_bool(dev, "support-bc12")) {
			priv->irq_bc = 192 + (int32_t)INT_GIC_OFFSET;

			irq_install_handler(priv->irq_bc,
					(interrupt_handler_t *)
					tcc_dwc3_usb_phy_irq_handler, dev);
			irq_mask(priv->irq_bc);
		}

		if (dev_read_bool(dev, "support-line-state")) {
			priv->irq_ls = 193 + (int32_t)INT_GIC_OFFSET;

			irq_install_handler(priv->irq_ls,
					(interrupt_handler_t *)
					tcc_dwc3_usb_line_state_irq_handler,
					dev);
			irq_mask(priv->irq_ls);
		}
#endif /* TCC_USB_VERIFICATION */
	}

	return ret;
}

static const struct udevice_id tcc_dwc3_usb_phy_ids[] = {
	{ .compatible = "telechips,tcc_dwc3_phy" },
	{ },
};

U_BOOT_DRIVER(tcc_dwc3_usb_phy) = {
	.name      = "tcc_dwc3_phy",
	.id        = UCLASS_PHY,
	.of_match  = tcc_dwc3_usb_phy_ids,
	.ops       = &tcc_dwc3_usb_phy_ops,
	.probe     = tcc_dwc3_usb_phy_probe,
	.priv_auto = sizeof(struct tcc_dwc3_usb_phy),
};
