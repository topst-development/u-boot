// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <irq.h>
#include <clk.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/const.h>
#include <asm/arch/clock.h>
#include <asm/arch/timer_reg.h>
#include <asm/arch/timer_api.h>
#include <dm/read.h>

DECLARE_GLOBAL_DATA_PTR;

#define TCC_TIMER_NAME      "tcc_timer"

#define timer_readl         readl
#define timer_writel(v, a)  writel((v), a)

struct tcc_timer_plat {
	fdt_addr_t base;
	s32 id;
	s32 width;
	u32 clock_rate;
};

#if defined(CONFIG_TCC897X) || defined(CONFIG_TCC898X) || \
	defined(CONFIG_TCC802X)
#define TIMER_IRQ_NUM_MAX   1
#else
#define TIMER_IRQ_NUM_MAX   6
#endif
#define TCC_TIMER_MAX       6
#define MAX_TCKSEL          6

#define TIMER_CLK_RATE      (u32)12000000

#define TIMER4_DIV_RATE     (u32)375     // When TCKSEL is 4 and TCLK is 12Mhz
#define TIMER3_DIV_RATE     (u32)3       // When TCKSEL is 1 and TCLK is 12Mhz

// temporary
#define TIMER0_IRQ          (32 + 33)

static s32 tclk_have_set;

static void __maybe_unused tcc_timer_dummy(struct udevice *dev)
{
	struct tcc_timer *priv = dev_get_priv(dev);
	fdt_addr_t reg = priv->platinfo->base;
	u32 val = timer_readl(reg + TIMER_TCFG);

	if ((val & TCFG_EN) == TCFG_EN) {
		return;
	}
	priv->used = 1;
	val = TCFG_TCKSEL(1) | TCFG_EN;
	timer_writel(val, reg + TIMER_TCFG);
}

static void tcc_timer_handler(struct udevice *dev)
{
	struct tcc_timer *priv = dev_get_priv(dev);
	fdt_addr_t reg = priv->platinfo->base & ~(fdt_addr_t)0xFF;
	u32 val;

	/* Do not process watchdog irq source */
	if (priv->id >= TCC_TIMER_MAX) {
		return;
	}

	val = timer_readl(reg + TCC_TIREQ);
	val &= ((u32)1 << (u32)priv->id);

	if (val != (u32)0) {
		if (priv->irqcnt < (U32_MAX - (u32)1)) {
			priv->irqcnt++;
		} else {
			priv->irqcnt = 0;
		}

		val = ((u32)1 << ((u32)8 + (u32)priv->id))
		    | ((u32)1 << (u32)priv->id);

		timer_writel(val, reg + TCC_TIREQ);
		if (priv->handler != NULL) {
			priv->handler(priv->args);
		}
	}
}

static s32 tcc_timer_enable(struct udevice *dev)
{
	struct tcc_timer *priv = dev_get_priv(dev);
	fdt_addr_t reg = priv->platinfo->base;
	u32 val;

	if (tclk_have_set == 0) {
		return -ENODEV;
	}

	priv->irqcnt = 0;

	timer_writel(0x0, reg + TIMER_TCNT);

	val = timer_readl(reg + TIMER_TCFG);
	val |= (TCFG_IEN | TCFG_EN);
	timer_writel(val, reg + TIMER_TCFG);

	return 0;
}

static s32 tcc_timer_disable(struct udevice *dev)
{
	struct tcc_timer *priv = dev_get_priv(dev);
	fdt_addr_t reg = priv->platinfo->base;
	u32 val;

	if (tclk_have_set == 0) {
		return -ENODEV;
	}

	val = timer_readl(reg + TIMER_TCFG);
	val &= ~(TCFG_IEN | TCFG_EN);
	timer_writel(val, reg + TIMER_TCFG);
	timer_writel(0x0, reg + TIMER_TCNT);
	priv->irqcnt = 0;

	return 0;
}

static struct tcc_timer *tcc_timer_register(struct udevice *dev,
					    u32 usec,
					    interrupt_handler_t *handler,
					    void *data)
{
	s32 k;
	u32 max_ref, req_hz = (u32)(1000 * 1000 * 50) / usec;
	u32 srch_k, srch_err, ref[MAX_TCKSEL + 1];
	struct tcc_timer *priv = dev_get_priv(dev);
	fdt_addr_t reg = priv->platinfo->base;

	if (tclk_have_set == 0) {
		(void)pr_err("[ERROR][%s] %s: Timer has not initialized\n",
			     TCC_TIMER_NAME, __func__);
		return ERR_PTR(-ENODEV);
	}

	max_ref = (priv->platinfo->width == 16) ? (u32)0xFFFF : (u32)0xFFFFF;

	/* Wraparound guard */
	if (priv->platinfo->clock_rate > (U32_MAX / (u32)50)) {
		(void)pr_err("[ERROR][%s] %s: Wrong timer clock has set.\n",
			     TCC_TIMER_NAME, __func__);
		return ERR_PTR(-EDOM);
	}

	/* find divide factor */
	srch_k = 0;
	srch_err = 0xFFFFFFFFU;	/* U32_MAX */

	for (k = 0 ; k <= MAX_TCKSEL ; k++) {
		s32 max_cnt;
		u32 tcksel, tck, ref_1, ref_2, err1, err2;

		max_cnt = (k < 5) ? (k + 1) : (k * 2);
		tcksel = (u32)1 << (u32)max_cnt;

		tck = (priv->platinfo->clock_rate * (u32)50) / tcksel;

		ref_1 = tck / req_hz;
		ref_2 = (tck + req_hz - (u32)1) / req_hz;

		priv->div = tcksel;

		err1 = req_hz - (tck / ref_1);
		err2 = (tck / ref_2) - req_hz;
		if (err1 > err2) {
			ref[k] = ref_2;
			err1 = err2;
		} else {
			ref[k] = ref_1;
		}

		if (ref[k] > max_ref) {
			ref[k] = max_ref;
			err1 = ((tck / max_ref) > req_hz)
				? ((tck / max_ref) - req_hz)
				: (req_hz - (tck / max_ref));
		}

		if (err1 < srch_err) {
			srch_err = err1;
			srch_k = (u32)k;
		}

		if (err1 == (u32)0) {
			break;
		}
	}

	/* cannot found divide factor */
	if (k > MAX_TCKSEL) {
		k = MAX_TCKSEL;
		srch_k = (u32)k;
		ref[srch_k] = max_ref;
		(void)pr_warn("[WARN][%s] %s: cannot get the correct timer.\n",
			      TCC_TIMER_NAME, __func__);
		/* TODO: supplementary setting */
	}

	priv->used = 1;
	priv->irqcnt = 0;
	priv->ref = ref[srch_k];
	priv->mref = 0;
	priv->handler = handler;
	priv->args = data;

	timer_writel(TCFG_TCKSEL(srch_k), reg + TIMER_TCFG);
	timer_writel(0x0, reg + TIMER_TCNT);
	timer_writel(priv->mref, reg + TIMER_TMREF);
	timer_writel(priv->ref, reg + TIMER_TREF);

	irq_install_handler(priv->irq,
			    (interrupt_handler_t *)tcc_timer_handler,
			    (void *)dev);
	irq_unmask(priv->irq);

	return priv;
}

static void tcc_timer_unregister(struct udevice *dev)
{
	struct tcc_timer *priv = dev_get_priv(dev);
	fdt_addr_t reg = priv->platinfo->base;

	if (tclk_have_set == 0) {
		return;
	}
	if (priv->id >= TCC_TIMER_MAX) {
		(void)pr_err("[ERROR][%s] %s Timer ID(%d) is wrong.\n",
			     TCC_TIMER_NAME, __func__, priv->id);
		return;
	}
	if (priv->used == 0) {
		(void)pr_warn("[WARN][%s] %s: id:%d is not registered index.\n",
			      TCC_TIMER_NAME, __func__, priv->id);
	}

	irq_mask(priv->irq);
	irq_free_handler(priv->irq);

	timer_writel(0x0, reg + TIMER_TCFG);
	timer_writel(0x0, reg + TIMER_TCNT);

	priv->used = 0;
	priv->handler = NULL;
	priv->args = NULL;
}

/* It must be called after clock_init_early() */
static s32 tcc_timer_irq_probe(struct udevice *dev)
{
	struct tcc_timer_plat *plat = dev_get_platdata(dev);
	struct tcc_timer *priv = dev_get_priv(dev);
	s32 ret;

	priv->id = plat->id;
	priv->platinfo = plat;
	priv->name = dev_read_name(dev);
	priv->irq = TIMER0_IRQ + priv->id;
	priv->ref = (priv->platinfo->width == 16) ? (u32)0xFFFF : (u32)0xFFFFF;
	priv->used = 0;
	priv->reserved = 0;
	priv->irqcnt = 0;
	priv->handler = NULL;
	priv->args = NULL;
	priv->mref = 0;
	priv->div = 0;

	if (tclk_have_set == 0) {
		ret = (s32)tcc_set_peri(PERI_TCT, CKC_ENABLE,
					plat->clock_rate, UL(0));
		if (ret != 0) {
			(void)pr_err(
				     "[ERROR][%s] %s: Failed to set timer clock.\n",
				     TCC_TIMER_NAME, __func__);
			return ret;
		}
		#if !defined(CONFIG_TCC_SUBCORE)
		tcc_timer_dummy(dev);
		#endif
		tclk_have_set = 1;
	}

	return 0;
}

static s32 tcc_timer_ofdata_to_platdata(struct udevice *dev)
{
	struct tcc_timer_plat *plat = dev_get_platdata(dev);
	s32 ret;

	plat->base = dev_read_addr(dev);
	ret = dev_read_s32(dev, "id", &plat->id);
	if (ret != 0) {
		(void)pr_err("[ERROR][%s] %s: Failed to read id.\n",
			     TCC_TIMER_NAME, __func__);
		return ret;
	}
	ret = dev_read_s32(dev, "width", &plat->width);
	if (ret != 0) {
		(void)pr_err("[ERROR][%s] %s: Failed to read width.\n",
			     TCC_TIMER_NAME, __func__);
		return ret;
	}
	ret = dev_read_u32(dev, "clock-frequency", (u32 *)&plat->clock_rate);
	if (ret != 0) {
		(void)pr_err(
			     "[ERROR][%s] %s: Failed to read clock-frequency.\n",
			     TCC_TIMER_NAME, __func__);
		return ret;
	}

	if (plat->clock_rate == (u32)0) {
		plat->clock_rate = TIMER_CLK_RATE;
	}

	return 0;
}

static const struct timer_irq_ops tcc_timer_irq_ops = {
	.register_timer = tcc_timer_register,
	.unregister_timer = tcc_timer_unregister,
	.enable_timer = tcc_timer_enable,
	.disable_timer = tcc_timer_disable,
};

static const struct udevice_id timer_irq_ids[] = {
	{ .compatible = "telechips,timer" },
	{ NULL }
};

U_BOOT_DRIVER(timer) = {
	.name		= (char *)"tcc_timer_irq",
	.id		= UCLASS_TIMER_IRQ,
	.of_match	= of_match_ptr(timer_irq_ids),
	.probe		= tcc_timer_irq_probe,
	.ofdata_to_platdata = tcc_timer_ofdata_to_platdata,
	.priv_auto_alloc_size = (s32)sizeof(struct tcc_timer),
	.platdata_auto_alloc_size = (s32)sizeof(struct tcc_timer_plat),
	.ops = (const void *)&tcc_timer_irq_ops,

};