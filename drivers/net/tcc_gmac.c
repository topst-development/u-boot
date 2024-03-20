// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>

#include <phy.h>
#include <netdev.h>
#include <asm/telechips/gpio.h>
#include <asm/arch/clock.h>
#include <asm/telechips/tcc_ckc.h>

#define CONFIG_GMAC_BASE 0x11C00000
#define CONFIG_HSIO_BASE 0x11DA0000
#define CONFIG_MAC_ADDR {0x00, 0x12, 0x34, 0x56, 0x78, 0x01}

#define GMACDLY0_OFFSET     (0x2000)
#define GMACDLY1_OFFSET     (0x2004)
#define GMACDLY2_OFFSET     (0x2008)
#define GMACDLY3_OFFSET     (0x200C)
#define GMACDLY4_OFFSET     (0x2010)
#define GMACDLY5_OFFSET     (0x2014)
#define GMACDLY6_OFFSET     (0x2018)

#define GMAC1_CFG1_OFFSET   (0x006C) // used.

// GMAC
#define GPIO_GMAC_TXD0      TCC_GPB(5)	//FUNC 1
#define GPIO_GMAC_TXD1      TCC_GPB(6)	//FUNC 1
#define GPIO_GMAC_TXD2      TCC_GPB(11)	//FUNC 1
#define GPIO_GMAC_TXD3      TCC_GPB(12)	//FUNC 1
#define GPIO_GMAC_TXEN      TCC_GPB(7)	//FUNC 1
#define GPIO_GMAC_TXER      TCC_GPB(15)	//FUNC 1
#define GPIO_GMAC_TXCLK     TCC_GPB(0)	//FUNC 1

#define GPIO_GMAC_RXD0      TCC_GPB(3)	//FUNC 1
#define GPIO_GMAC_RXD1      TCC_GPB(4)	//FUNC 1
#define GPIO_GMAC_RXD2      TCC_GPB(9)	//FUNC 1
#define GPIO_GMAC_RXD3      TCC_GPB(10)	//FUNC 1
#define GPIO_GMAC_RXDV      TCC_GPB(8)	// FUNC 1
#define GPIO_GMAC_RXER		TCC_GPB(14)	// FUNC 1
#define GPIO_GMAC_RXCLK     TCC_GPB(13)	// FUNC 1

#define GPIO_GMAC_COL       TCC_GPB(16)	// FUNC 1
#define GPIO_GMAC_CRS       TCC_GPB(17)	// FUNC 1
#define GPIO_GMAC_MDC       TCC_GPB(1)	//FUNC 1
#define GPIO_GMAC_MDIO      TCC_GPB(2)

#define GPIO_PHY_RST        TCC_GPA(24)	//FUNC 0

extern void gpio_set(unsigned int n, unsigned int on);

void tca_gmac_phy_reset(void)
{
	gpio_set(GPIO_PHY_RST, 0);
	mdelay(10);
	gpio_set(GPIO_PHY_RST, 1);
	mdelay(200);
}

void tca_gmac_timing_tunning(void)
{
	void __iomem *gmac = (void __iomem *)CONFIG_GMAC_BASE;

	writel((18<<8), gmac+GMACDLY0_OFFSET); //delay TXCLK 18 step.
	writel(0x0, gmac+GMACDLY1_OFFSET); //delay 0x0
	writel(0x0, gmac+GMACDLY2_OFFSET); //delay 0x0
	writel(0x0, gmac+GMACDLY3_OFFSET); //delay 0x0
	writel(0x0, gmac+GMACDLY4_OFFSET); //delay 0x0
	writel(0x0, gmac+GMACDLY5_OFFSET); //delay 0x0
	writel(0x0, gmac+GMACDLY6_OFFSET); //delay 0x0
}

void tca_gmac_port_init(void)
{
	void __iomem *hsiobus_cfg = (void __iomem *)CONFIG_HSIO_BASE;

	tca_gmac_timing_tunning();

	// gpio config for GMAC.
	gpio_config(GPIO_GMAC_TXD0, GPIO_FN1 | GPIO_OUTPUT);
	gpio_config(GPIO_GMAC_TXD1, GPIO_FN1 | GPIO_OUTPUT);
	gpio_config(GPIO_GMAC_TXD2, GPIO_FN1 | GPIO_OUTPUT);
	gpio_config(GPIO_GMAC_TXD3, GPIO_FN1 | GPIO_OUTPUT);

	gpio_config(GPIO_GMAC_TXEN, GPIO_FN1 | GPIO_OUTPUT);
	gpio_config(GPIO_GMAC_TXER, GPIO_FN1 | GPIO_OUTPUT);
	gpio_config(GPIO_GMAC_TXCLK, GPIO_FN1 | GPIO_OUTPUT);

	gpio_config(GPIO_GMAC_RXD0, GPIO_FN1);
	gpio_config(GPIO_GMAC_RXD1, GPIO_FN1);
	gpio_config(GPIO_GMAC_RXD2, GPIO_FN1);
	gpio_config(GPIO_GMAC_RXD3, GPIO_FN1);

	gpio_config(GPIO_GMAC_RXDV, GPIO_FN1);
	gpio_config(GPIO_GMAC_RXER, GPIO_FN1);
	gpio_config(GPIO_GMAC_RXCLK, GPIO_FN1);

	gpio_config(GPIO_GMAC_COL, GPIO_FN1);
	gpio_config(GPIO_GMAC_CRS, GPIO_FN1);
	gpio_config(GPIO_GMAC_MDC, GPIO_FN1);
	gpio_config(GPIO_GMAC_MDIO, GPIO_FN1);

	gpio_config(GPIO_PHY_RST, GPIO_FN0|GPIO_OUTPUT);

	gpio_set(GPIO_PHY_RST, 0);

	writel(readl(hsiobus_cfg+GMAC1_CFG1_OFFSET) & ~(1<<31),
			hsiobus_cfg+GMAC1_CFG1_OFFSET); // clock disable.
	writel((1<<18), hsiobus_cfg+GMAC1_CFG1_OFFSET); // RGMII
	writel(readl(hsiobus_cfg+GMAC1_CFG1_OFFSET) |(1<<31),
			hsiobus_cfg+GMAC1_CFG1_OFFSET); // clock enable.
}


#ifdef CONFIG_CMD_NET

int board_eth_init(bd_t *bis)
{
	int ret = 0;

	const uchar enetaddr[7] = CONFIG_MAC_ADDR;

	u32 interface = PHY_INTERFACE_MODE_RGMII;

	eth_env_set_enetaddr("ethaddr", enetaddr);

	tca_gmac_port_init();
	tca_gmac_phy_reset();

	if (designware_initialize(CONFIG_GMAC_BASE, interface) >= 0)
		ret++;

	return ret;
}
#endif

