// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <clk.h>
#include <asm/io.h>
#include <irq.h>
#include <asm/arch/clock.h>
#include <linux/arm-smccc.h>
#include <mach/smc.h>
#include <asm/arch/timer_api.h>

#define	WDT_EN_REG		0x0
#define	WDT_CLR_REG		0x4
#define	WDT_IRQ_CNT_REG		0x8
#define	WDT_RST_CNT_REG		0xC

#define XIN_RATE		24000000

#define MAX_CELL		3

struct tcc_pmu_wdt {
	unsigned int	irq_num;
	unsigned int	wdt_en_reg;
	unsigned int	wdt_clr_reg;
	unsigned int	wdt_irq_cnt_reg;
	unsigned int	wdt_rst_cnt_reg;
};

struct tcc_pmu_wdt_priv {
	unsigned int pretimeout;
	unsigned int timeout;
	unsigned int wdt_rst_req_reg;
	unsigned int pmu_wdt_enable;
	struct tcc_pmu_wdt pmu_wdt;
};

void tcc_pmu_wdt_kick_handler(void *arg);

// By default, during initialization process on tcc watchdog driver, this function will be called.
static int tcc_dm_pmu_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct tcc_pmu_wdt_priv *priv = dev_get_priv(dev);
	unsigned int rst_cnt = priv->timeout * XIN_RATE;
	unsigned int kick_cnt = priv->pretimeout * XIN_RATE;
	unsigned int pmu_wdt_rst_enable = (0x1 << 6);
	struct arm_smccc_res res;
	int ret = 0;

	debug("Set reset count on RST_CNT.\n");
	arm_smccc_smc(SIP_WATCHDOG_SETUP, 0, 0, 0, rst_cnt, 0, 0, 0, &res);
	ret = res.a0;

	if (ret != 0)
		return ret;
	
	debug("Set irq count on IRQ_CNT.\n");
	arm_smccc_smc(SIP_WATCHDOG_SETUP_IRQCNT, 0, kick_cnt, 0, 0, 0, 0, 0, &res);
	ret = res.a0;

	writel(readl(priv->wdt_rst_req_reg) | pmu_wdt_rst_enable, priv->wdt_rst_req_reg);

	return ret;
}

// Disable PMU watchdog.
static int tcc_dm_pmu_wdt_stop(struct udevice *dev)
{
	struct tcc_pmu_wdt_priv *priv = dev_get_priv(dev);
	struct arm_smccc_res res;
	int ret;

	arm_smccc_smc(SIP_WATCHDOG_STOP, 0, priv->pmu_wdt_enable, 0, 0, 0, 0, 0, &res);
	ret = res.a0;
	debug("Stopping PMU WDT.\n");

	return ret;
}

// Watchdog kick function : Interrupt signal from ARM GIC(PMU watchdog interrupt)
void tcc_pmu_wdt_kick_handler(void *arg)
{
	struct arm_smccc_res res;
	unsigned long current_time = timer_get_boot_us()/1000U;

	(void)printf("[tcc_pmu_wdt] %s: current_time(ms): %lu\n", __func__, current_time);
	arm_smccc_smc(SIP_WATCHDOG_PING, 0, 0x1, 0, 0, 0, 0, 0, &res);
	
	return 0;
}

// PMU Watchdog probe.
static int tcc_dm_pmu_wdt_probe(struct udevice *dev)
{
	struct tcc_pmu_wdt_priv *priv = dev_get_priv(dev);
	int i = 0, result = 0, j = 0;
	ofnode node;
	unsigned int reg[MAX_CELL];
	int reg_num = 0;
	int pmu_wdt_frequency = 0;
	
	if (!dev_of_valid(dev)) {
		return -EINVAL;
	}
	
	node = ofnode_by_compatible(ofnode_null(), "telechips,tcc_pmu_wdt");
	
	if (ofnode_read_u32_array(node, "pmu_wdt_enable", reg, 1) == 0) {
		priv->pmu_wdt_enable = (1 << reg[0]);
	}

	if (ofnode_read_u32_array(node, "pretimeout", reg, 1) == 0) {
		priv->pretimeout = reg[0];
	}

	if (ofnode_read_u32_array(node, "timeout", reg, 1) == 0) {
		priv->timeout = reg[0];
	}

	if (ofnode_read_u32_array(node, "interrupts", reg, 3) == 0) {
		priv->pmu_wdt.irq_num = reg[1] + 32;
		debug("%d\n", priv->pmu_wdt.irq_num);
	}

	if (ofnode_read_u32_array(node, "rst_req_en", reg, 2) == 0) {
		priv->wdt_rst_req_reg = reg[0];
	}

	return 0;
}

static void wdt_timer_test(void)
{
	unsigned long current_time = timer_get_boot_us()/1000U;
	(void)printf("[tcc_pmu_wdt] %s: current_time(ms): %lu\n", __func__, current_time);
}

// [For verification] watchdog initiation function
// Revised on 2020.12.23
int tcc_pmu_wdt_initiation(void)
{
	struct udevice *dev;
	struct arm_smccc_res res;
	int i = 0, ret = 0;
	unsigned int pmu_wdt_enable = (0x1 << 31);
	unsigned int pmu_wdt_rst_enable = (0x1 << 6);

	printf("PMU Watchdog Initiation.\n");

	writel(0x0, 0x1d2c3000);
	writel(0x00000100, 0x14400254);
	writel(0x0, 0x14400130);

	// register irq handler.
	irq_install_handler(190 + 32, tcc_pmu_wdt_kick_handler, NULL);
	irq_unmask(190 + 32);

	// reset enable bit set.
	writel(readl(0x144000BC) | pmu_wdt_rst_enable, 0x144000BC);
	
	writel(24000000*5, 0x144000DC);
	writel(24000000*20, 0x144000E0);

	// pmu watchdog enable bit set.
	writel(0x80000000, 0x144000D8);

	return ret;
}

int tcc_pmu_wdt_resetting(void)
{
	struct udevice *dev;
	struct tcc_pmu_wdt_priv *pmu_wdt_priv;
	struct arm_smccc_res res;
	int i = 0, ret = 0;
	unsigned int pmu_wdt_enable = (0x1 << 31);
	unsigned int pmu_wdt_rst_enable = (0x1 << 6);
	struct tcc_timer *tt_timer;
	
	printf("PMU Watchdog Initiation.\n");

	// set PMU watchdog reset enable bit
        writel(readl(0x144000BC) | pmu_wdt_rst_enable, 0x144000BC);
        printf("reset enable reg : %x\n", readl(0x144000BC));
        printf("access register : %x\n", readl(0x1B936254));
        writel(0x0000037F, 0x1B936254);
 
        // set all watchdog reset mask disable -> yield watchdog reset.
        writel(0, 0x14400130);
        printf("reset req mask register for TOP reg : %x\n", readl(0x14400130));
 
        writel(24000000*5, 0x144000DC);
        writel(24000000*20, 0x144000E0);
 
        writel(0x80000000, 0x144000D8);

	return ret;
}

// [For verification] watchdog disable function
int tcc_pmu_wdt_halt(void)
{
	struct udevice *dev;
	int i = 0, ret = 0;

	printf("PMU Watchdog Halt.\n");

	writel(0x0, 0x144000D8);

	return 0;
}

static const struct wdt_ops tcc_pmu_wdt_ops = {
	.start = tcc_dm_pmu_wdt_start,
	.stop = tcc_dm_pmu_wdt_stop,
};

static const struct udevice_id tcc_pmu_wdt_ids[] = {
	{ .compatible = "telechips,tcc_pmu_wdt" },
	{}
};

U_BOOT_DRIVER(tcc_pmu_wdt) = {
	.name = "tcc_pmu_wdt",
	.id = UCLASS_WDT,
	.of_match = tcc_pmu_wdt_ids,
	.ops = &tcc_pmu_wdt_ops,
	.flags = DM_FLAG_PRE_RELOC,
	.priv_auto_alloc_size = sizeof(struct tcc_pmu_wdt_priv),
	.probe = tcc_dm_pmu_wdt_probe,
};
