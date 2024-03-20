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
#include <configs/tcc/tcc805x-reg.h>

/* CPU Bus Watchdog 0 ~ 3 : Cortex-A72 */
/* CPU Bus Watchdog 4 ~ 7 : Cortex-A53 */

#define	WDT_EN_REG		0x0
#define	WDT_CLR_REG		0x4
#define	WDT_IRQ_CNT_REG		0x8
#define	WDT_RST_CNT_REG		0xC

#define	WDT_RST_DIS_REG		(0x17000000 + 0x38)
#define	WDT_DIS_CA72_RST	0x1
#define	WDT_DIS_CA53_RST	0x2
#define	WDT_DISABLE		0x0

#define	WDT_EN_BIT		0x00000001

#define	WDT_RST_EN_CPU0		0
#define	WDT_RST_EN_CPU1		1
#define	WDT_RST_EN_CPU2		2
#define	WDT_RST_EN_CPU3		3
#define	WDT_RST_EN_CPU4		4
#define	WDT_RST_EN_CPU5		8
#define	WDT_RST_EN_CPU6		9
#define	WDT_RST_EN_CPU7		10

#define	WDT_IRQ_CA72_WDT0_BUS_HANGUP	367
#define	WDT_IRQ_CA72_WDT0_RST_RELEASE	368
#define	WDT_IRQ_CA72_WDT0_SYS_HANGUP	369
#define	WDT_IRQ_CA53_WDT4_BUS_HANGUP	370
#define	WDT_IRQ_CA53_WDT4_RST_RELEASE	371
#define	WDT_IRQ_CA53_WDT4_SYS_HANGUP	372

#define	WDT_RST_DIS_REG		0x70
#define	WDT_CA72_WDT_DIS_REG	TCC_CA72MP_BUS_BASE + WDT_RST_DIS_REG
#define	WDT_CA53_WDT_DIS_REG	TCC_CA72MP_BUS_BASE + WDT_RST_DIS_REG
#define	WDT_CA72_RST_DIS	0x1
#define	WDT_CA53_RST_DIS	0x2
/* 0x1(reset value) : CA72 watchdog reset disable */
/* 0x1(reset value) : CA53 watchdog reset disable */
/* if value is 0, watchdog reset is enabled by corresponding core. */

/*
 * IRQ Number
 * 186 ~ 182 : CPU Bus WDT4 ~ WDT0
 * 164 ~ 162 : CPU Bus WDT7 ~ WDT5
 */

void tcc_cpu_wdt0_bus_hangup_handler(void *args);
void tcc_cpu_wdt0_rst_release_handler(void *args);
void tcc_cpu_wdt0_sys_hangup_handler(void *args);

void tcc_cpu_wdt4_bus_hangup_handler(void *args);
void tcc_cpu_wdt4_rst_release_handler(void *args);
void tcc_cpu_wdt4_sys_hangup_handler(void *args);

#define	CPU_WDT_MAX	8

struct tcc_cb_wdt {
	unsigned int	irq_num;
	unsigned int	wdt_en_reg;
	unsigned int	wdt_clr_reg;
	unsigned int	wdt_irq_cnt_reg;
	unsigned int	wdt_rst_cnt_reg;
};

struct tcc_cpu_wdt_priv {
	unsigned int cpu_wdt_num;
	unsigned int pretimeout;
	unsigned int timeout;
	unsigned int wdt_frequency;
	struct clk wdt_clk;
	struct tcc_cb_wdt *cpu_wdt;
};

void tcc_wdt_kick_handler(void *arg);

// By default, during initialization process on tcc watchdog driver, this function will be called.
static int tcc_dm_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	unsigned int i = 0;
	struct tcc_cpu_wdt_priv *priv = dev_get_priv(dev);

	for (i = 0; i < priv->cpu_wdt_num; i++) {
		printf("Starting CPU WDT : %d\n", i);
		writel(0x1, priv->cpu_wdt[i].wdt_clr_reg);
	}

	return 0;
}

// Disable CPU Bus watchdog.
static int tcc_dm_wdt_stop(struct udevice *dev)
{
	int i = 0;
	struct tcc_cpu_wdt_priv *priv = dev_get_priv(dev);

	for (i = 0; i < priv->cpu_wdt_num; i++) {
		unsigned int reg_val = readl(priv->cpu_wdt[i].wdt_en_reg) & ~(WDT_EN_BIT);
		writel(reg_val, priv->cpu_wdt[i].wdt_en_reg);
		debug("Stopping CPU WDT : %d\n", i);
	}

	return 0;
}

// Watchdog kick function : Interrupt signal from ARM GIC(CPU Bus watchdog interrupt)
void tcc_wdt_kick_handler(void *arg)
{
	int target = *(int *)arg;
	struct udevice *dev;
	struct tcc_cpu_wdt_priv *priv;
	int i = 0, ret = 0;

	/* Find watchdog device. */
	ret = uclass_find_device(UCLASS_WDT, i, &dev);
	for (i = 1; ret != 0; i++) {
		if (strcmp(dev->name, "tcc_cb_wdt@17010000") == 0) {
			debug("Found device : %s\n", dev->name);
			break;
		}
		ret = uclass_find_device(UCLASS_WDT, i, &dev);
	}

	priv = dev_get_priv(dev);
	printf("cpu wdt %d kicked\n", target);

	writel(0x1, priv->cpu_wdt[target].wdt_clr_reg);
}

// Watchdog probe.
int tcc_dm_wdt_probe(struct udevice *dev)
{
	struct tcc_cpu_wdt_priv *priv = dev_get_priv(dev);
	int i = 0, result = 0, j = 0;
	ofnode node;
	unsigned int regs[CPU_WDT_MAX * 3];
	int reg_num = 0;
	int cb_wdt_frequency = 0;

	if (!dev_of_valid(dev)) {
		return -EINVAL;
	}

	priv->cpu_wdt = kzalloc(CPU_WDT_MAX * sizeof(struct tcc_cb_wdt), GFP_KERNEL);
	node = ofnode_by_compatible(ofnode_null(), "telechips,tcc_cb_wdt");

	if (ofnode_read_u32_array(node, "num_cpu_wdt", regs, 1) == 0) {
		printf("num_cpu_wdt : %d\n", regs[0]);
		priv->cpu_wdt_num = regs[0];
	}

	if (ofnode_read_u32_array(node, "pretimeout", regs, 1) == 0) {
		printf("pretimeout : %d\n", regs[0]);
		priv->pretimeout = regs[0];
	}

	if (ofnode_read_u32_array(node, "timeout", regs, 1) == 0) {
		printf("timeout : %d\n", regs[0]);
		priv->timeout = regs[0];
	}

	if (ofnode_read_u32_array(node, "clock-frequency", regs, 1) == 0) {
		printf("CPU Bus watchdog frequency : %d\n", regs[0]);
		cb_wdt_frequency = regs[0];
	}

	/* Set CPU Bus Watchdog clock frequency. */
//	tcc_clk_get_by_index(dev, 0, &(priv->wdt_clk));
//	clk_set_rate(&(priv->wdt_clk), cb_wdt_frequency);
	priv->wdt_clk.rate = cb_wdt_frequency;

	reg_num = priv->cpu_wdt_num * 2;
	if (ofnode_read_u32_array(node, "reg", regs, reg_num) == 0) {
		for (j=0;j<reg_num;j++) {
			if ((j % 2) == 0) {
				unsigned int base_addr = regs[j];
				priv->cpu_wdt[j/2].wdt_en_reg = base_addr + WDT_EN_REG;
				priv->cpu_wdt[j/2].wdt_clr_reg = base_addr + WDT_CLR_REG;
				priv->cpu_wdt[j/2].wdt_irq_cnt_reg = base_addr + WDT_IRQ_CNT_REG;
				priv->cpu_wdt[j/2].wdt_rst_cnt_reg = base_addr + WDT_RST_CNT_REG;
				printf("%p %p %p %p\n", priv->cpu_wdt[j/2].wdt_en_reg, priv->cpu_wdt[j/2].wdt_clr_reg, priv->cpu_wdt[j/2].wdt_irq_cnt_reg, priv->cpu_wdt[j/2].wdt_rst_cnt_reg);
			}
		}
	}

	reg_num = priv->cpu_wdt_num * 3;
	if (ofnode_read_u32_array(node, "interrupts", regs, reg_num) == 0) {
		for (j=0;j<reg_num;j++) {
			if ((j % 3) == 1) {
				priv->cpu_wdt[j/3].irq_num = regs[j] + 32;
				printf("%d\n", priv->cpu_wdt[j/3].irq_num);
			}
		}
	}

	/* write IRQ_CNT, RST_CNT Only */
	for (i = 0; i < priv->cpu_wdt_num; i++) {
		printf("CPU WDT %d init.\n", i);
		writel(priv->timeout * cb_wdt_frequency, priv->cpu_wdt[i].wdt_rst_cnt_reg);
		writel(priv->pretimeout * cb_wdt_frequency, priv->cpu_wdt[i].wdt_irq_cnt_reg);
	}

	return 0;
}

// [For verification] watchdog initiation function
int tcc_wdt_initiation(void)
{
	static int numbering[CPU_WDT_MAX];
	struct udevice *dev;
	struct tcc_cpu_wdt_priv *cpu_wdt_priv;
	int i = 0, ret = 0;

	printf("CPU Bus Watchdog Initiation\n");

//	writel(0x0000037F, 0x1b936254);		// WDTRST mask register -- MICOM -> CA72/53.
//	writel(0x00003FFF, 0x14400130);		// set register field to propagate wdt reset into TOP reset.

	/* Register Interrupt Handler for Sys/Bus Hangup */
	irq_install_handler(WDT_IRQ_CA72_WDT0_BUS_HANGUP + 32, tcc_cpu_wdt0_bus_hangup_handler, NULL);
	irq_install_handler(WDT_IRQ_CA72_WDT0_RST_RELEASE + 32, tcc_cpu_wdt0_rst_release_handler, NULL);
	irq_install_handler(WDT_IRQ_CA72_WDT0_SYS_HANGUP + 32, tcc_cpu_wdt0_sys_hangup_handler, NULL);

	irq_install_handler(WDT_IRQ_CA53_WDT4_BUS_HANGUP + 32, tcc_cpu_wdt4_bus_hangup_handler, NULL);
	irq_install_handler(WDT_IRQ_CA53_WDT4_RST_RELEASE + 32, tcc_cpu_wdt4_rst_release_handler, NULL);
	irq_install_handler(WDT_IRQ_CA53_WDT4_SYS_HANGUP + 32, tcc_cpu_wdt4_sys_hangup_handler, NULL);
	
	/* Find cpu bus watchdog driver. */
	ret = uclass_find_device(UCLASS_WDT, i, &dev);
	printf("return value : %d\n", ret);
	for (i = 1; ret == 0; i++) {
		printf("dev name : %s\n", dev->name);
		if (strcmp(dev->name, "tcc_cb_wdt@17010000") == 0) {
			printf("Found device : %s\n", dev->name);
			break;
		}
		ret = uclass_find_device(UCLASS_WDT, i, &dev);
	}

	cpu_wdt_priv = dev_get_priv(dev);
	printf("# of CPU Watchdogs : %d\n", cpu_wdt_priv->cpu_wdt_num);
	printf("CPU Watchdogs clock : %d\n", cpu_wdt_priv->wdt_clk.rate);

	for (i=0;i<cpu_wdt_priv->cpu_wdt_num;i++) {
		printf("en reg : %p, irq num : %p\n", cpu_wdt_priv->cpu_wdt[i].wdt_en_reg, cpu_wdt_priv->cpu_wdt[i].irq_num);
	}

	printf("Addr : %p\n", cpu_wdt_priv->cpu_wdt[i].wdt_en_reg);
	/* For all CPU Watchdogs,  */
	/* Register IRQ Handlers, enable, kick */
	for (i = 0; i < cpu_wdt_priv->cpu_wdt_num; i++) {
		unsigned int reg_val = readl(cpu_wdt_priv->cpu_wdt[i].wdt_en_reg) | WDT_EN_BIT;
		unsigned int shifting = 0;
		unsigned int enab = 0x1;

		/* Set Reset enable bit on Watchdog Reset Request enable register. */
		if (i > 4) {
			shifting = i + 3;
		}
		else {
			shifting = i;
		}
		enab = enab << shifting;
		writel(readl(0x144000BC) | enab, 0x144000BC);

		/* Set value for print log. */
		numbering[i] = i;
		printf("numbering : %d\n", i);

		/* Register IRQ Handler for CPU Watchdog. */
		irq_install_handler(cpu_wdt_priv->cpu_wdt[i].irq_num, tcc_wdt_kick_handler, &numbering[i]);
		irq_unmask(cpu_wdt_priv->cpu_wdt[i].irq_num);

		/* Kick dog and enable watchdog. */
		writel(0x1, cpu_wdt_priv->cpu_wdt[i].wdt_clr_reg);
		writel(reg_val, cpu_wdt_priv->cpu_wdt[i].wdt_en_reg);
	}

	tcc_dm_wdt_start(dev, 0, 0);

	return 0;
}

// [For verification] watchdog disable function
int tcc_wdt_halt(void)
{
	struct udevice *dev;
	const struct wdt_ops *tcc_ops;
	int i = 0, ret = 0;

	printf("CPU Bus Watchdog Halt.\n");

	ret = uclass_find_device(UCLASS_WDT, i, &dev);
	for (i = 1; ret != 0; i++) {
		if (strcmp(dev->name, "tcc_cb_wdt@17010000") == 0) {
			debug("Found device : %s\n", dev->name);
			break;
		}
		ret = uclass_find_device(UCLASS_WDT, i, &dev);
	}
	if (ret != 0)
		return ret;

	tcc_dm_wdt_stop(dev);
	return 0;
}

/* [For verification] bus hangup monitoring function for MP core(CA72) */
void tcc_cpu_wdt0_bus_hangup_handler(void *args) {
	unsigned int reg_val = 0;
	debug("[DEBUG][tcc_cpu_wdt0][%s] CA72 WDT0 Bus Hangup Handler.\n", __func__);
	reg_val = readl(WDT_CA72_WDT_DIS_REG);
	if (reg_val & WDT_CA72_RST_DIS) {
		printf("[DEBUG][tcc_cpu_wdt0][%s] CA72 WDT Reset Disabled.\n", __func__);
	}
	else {
		printf("[DEBUG][tcc_cpu_wdt0][%s] CA72 WDT Reset Enabled.\n", __func__);
	}
}

/* [For verification] sys hangup monitoring function for MP core(CA72) */
void tcc_cpu_wdt0_sys_hangup_handler(void *args) {
	unsigned int reg_val = 0;
	debug("[DEBUG][tcc_cpu_wdt0][%s] CA72 WDT0 System Hangup Handler.\n", __func__);
	reg_val = readl(WDT_CA72_WDT_DIS_REG);

	if (reg_val & WDT_CA72_RST_DIS) {
		printf("[DEBUG][tcc_cpu_wdt0][%s] CA72 WDT Reset Disabled.\n", __func__);
	}
	else {
		printf("[DEBUG][tcc_cpu_wdt0][%s] CA72 WDT Reset Enabled.\n", __func__);
	}
}

/* [For verification] reset release handler function for MP core(CA72) */
void tcc_cpu_wdt0_rst_release_handler(void *args) {
	unsigned int reg_val = 0;
	debug("[DEBUG][tcc_cpu_wdt0][%s] CA72 WDT0 Reset Release Handler.\n", __func__);
	reg_val = readl(WDT_CA72_WDT_DIS_REG);

	if (reg_val & WDT_CA72_RST_DIS) {
		printf("[DEBUG][tcc_cpu_wdt0][%s] CA72 WDT Reset Disabled.\n", __func__);
	}
	else {
		printf("[DEBUG][tcc_cpu_wdt0][%s] CA72 WDT Reset Enabled.\n", __func__);
	}
}

/* [For verification] bus hangup monitoring function for AP core(CA53) */
void tcc_cpu_wdt4_bus_hangup_handler(void *args) {
	unsigned int reg_val = 0;
	debug("[DEBUG][tcc_cpu_wdt0][%s] CA53 WDT4 Bus Hangup Handler.\n", __func__);
	reg_val = readl(WDT_CA53_WDT_DIS_REG);

	if (reg_val & WDT_CA72_RST_DIS) {
		printf("[DEBUG][tcc_cpu_wdt4][%s] CA53 WDT Reset Disabled.\n", __func__);
	}
	else {
		printf("[DEBUG][tcc_cpu_wdt4][%s] CA53 WDT Reset Enabled.\n", __func__);
	}
}

/* [For verification] sys hangup monitoring function for AP core(CA53) */
void tcc_cpu_wdt4_sys_hangup_handler(void *args) {
	unsigned int reg_val = 0;
	debug("[%s][%s][%s] CA53 WDT4 System Hangup Handler.\n");
	reg_val = readl(WDT_CA53_WDT_DIS_REG);

	if (reg_val & WDT_CA72_RST_DIS) {
		printf("[DEBUG][tcc_cpu_wdt4][%s] CA53 WDT Reset Disabled.\n", __func__);
	}
	else {
		printf("[DEBUG][tcc_cpu_wdt4][%s] CA53 WDT Reset Enabled.\n", __func__);
	}
}

/* [For verification] reset release handler function for AP core(CA53) */
void tcc_cpu_wdt4_rst_release_handler(void *args) {
	unsigned int reg_val = 0;
	debug("[%s][%s][%s] CA53 WDT4 Reset Release Handler.\n");
	reg_val = readl(WDT_CA53_WDT_DIS_REG);

	if (reg_val & WDT_CA72_RST_DIS) {
		printf("[DEBUG][tcc_cpu_wdt4][%s] CA53 WDT Reset Disabled.\n", __func__);
	}
	else {
		printf("[DEBUG][tcc_cpu_wdt4][%s] CA53 WDT Reset Enabled.\n", __func__);
	}
}

static const struct wdt_ops tcc_wdt_ops = {
	.start = tcc_dm_wdt_start,
	.stop = tcc_dm_wdt_stop,
};

static const struct udevice_id tcc_wdt_ids[] = {
	{ .compatible = "telechips,tcc_cb_wdt" },
	{}
};

U_BOOT_DRIVER(tcc_cpu_wdt) = {
	.name = "tcc_cpu_wdt",
	.id = UCLASS_WDT,
	.of_match = tcc_wdt_ids,
	.ops = &tcc_wdt_ops,
	.flags = DM_FLAG_PRE_RELOC,
	.priv_auto_alloc_size = sizeof(struct tcc_cpu_wdt_priv),
	.probe = tcc_dm_wdt_probe,
};
