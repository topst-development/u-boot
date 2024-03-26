// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <pwm.h>
#include <asm/io.h>
#include <clk.h>
#include <mach/clock.h>

#define NUM_PWM 4

/* Debugging stuff */
static int debug = 1;
#define dprintf(msg...)	if (debug) pr_force("pwm-tcc: " msg)

#define PWMEN				0x4
#define PWMMODE				0x8
#define PWMPSTN1(CH)		(0x10 + (0x10 * (CH)))
#define PWMPSTN2(CH)		(0x14 + (0x10 * (CH)))
#define PWMPSTN3(CH)		(0x18 + (0x10 * (CH)))
#define PWMPSTN4(CH)		(0x1c + (0x10 * (CH)))

#define PWMOUT1(CH)			(0x50 + (0x10* (CH)))
#define PWMOUT2(CH)			(0x54 + (0x10* (CH)))
#define PWMOUT3(CH)			(0x58 + (0x10* (CH)))
#define PWMOUT4(CH)			(0x5c + (0x10* (CH)))


#define PHASE_MODE			(1)
#define REGISTER_OUT_MODE	(2)

#define PWM_DIVID_MAX	3 // clock divide max value 3(divide 16)
#define PWM_PERI_CLOCK	(400 * 1000 * 1000) // 400Mhz

#if defined(CONFIG_TCC802X) || defined(CONFIG_TCC803X) \
	|| defined(CONFIG_TCC805X) || defined(CONFIG_TCC807X)
#define TCC_USE_GFB_PORT
#endif

#define pwm_writel writel
#define pwm_readl readl

static int pwm_register_mode_set(struct udevice *dev,
		int pwm_id, unsigned int regist_value);
static int pwm_wait(struct udevice *dev, int pwm_id);


struct tcc_chip {
	void __iomem		*pwm_base;
	void __iomem		*io_pwm_base;

	struct clk		pwm_pclk;
	struct clk		pwm_ioclk;
	unsigned int		port[4];
#ifdef TCC_USE_GFB_PORT
	void __iomem		*io_pwm_port_base;
	unsigned int gfb_port[4];
#endif
};

static int tcc_pwm_set_config(struct udevice *dev, uint pwm_id,
			      uint period_ns_, uint duty_ns)
{
	struct tcc_chip *tcc = dev_get_priv(dev);
	unsigned int k = 0, reg = 0, bit_shift = 0;
	unsigned long clk_freq;
	unsigned long divide = 0;
	unsigned int  cal_duty = 0, cal_period = 0;
	unsigned int	hi_cnt = 0, low_cnt = 0;
	uint64_t total_cnt = 0;
	uint64_t clk_period_ns = 0;
	uint64_t period_ns = (uint64_t)period_ns_;
#ifdef TCC_USE_GFB_PORT
	unsigned int gfb_port_value = 0;
#endif

	clk_freq = clk_get_rate(&tcc->pwm_pclk);

	if (clk_freq == 0 || (duty_ns > period_ns))
		goto clk_error;

	clk_period_ns = (1000 * 1000 * 1000) / clk_freq;

	dprintf("%s clk_freq:%lu npwn:%d duty_ns:%d period_ns:%llu hwpwm:%d\n",
			__func__, clk_freq, pwm_id, duty_ns,
			period_ns, NUM_PWM);

#ifdef TCC_USE_GFB_PORT
	gfb_port_value = pwm_readl(tcc->io_pwm_port_base);
	gfb_port_value &= ~(0xFF << (8 * pwm_id));
	gfb_port_value |= (tcc->gfb_port[pwm_id] & 0xFF) << (8 * pwm_id);

	pwm_writel(gfb_port_value, tcc->io_pwm_port_base);
#endif

	if (duty_ns == 0)
		goto pwm_low_out;
	else if (duty_ns == period_ns)
		goto pwm_hi_out;

	while (1) {
		clk_period_ns = clk_period_ns * (2);
		total_cnt = period_ns/ clk_period_ns;

		if (total_cnt <= 1) {
			if (duty_ns > (period_ns/2))
				goto pwm_hi_out;
			else
				goto pwm_low_out;
		}

		if ((k == PWM_DIVID_MAX) || (total_cnt <= 0xFFFFFFFF))
			break;
		k++;
	}

	//prevent over flow.
	for (divide = 1; divide < 0xFFFFFFFF; divide++) {
		// 0xFFFFFFFF > total_cnt * duty / divide
		if ((ULLONG_MAX/ duty_ns) > (total_cnt/divide))
			break;
	}

	cal_duty = duty_ns / divide;
	cal_period = period_ns / divide;

	hi_cnt = (total_cnt * (cal_duty))/ (cal_period);
	low_cnt = total_cnt - hi_cnt;


	dprintf("k:%d clk_p:%llu total: %llu hi:%d low:%d\n",
			k, clk_period_ns, total_cnt, hi_cnt, low_cnt);

	reg = pwm_readl(tcc->pwm_base + PWMMODE);

	bit_shift = 4 * pwm_id;

	if (((reg >> bit_shift) & 0xF) != PHASE_MODE) {
		pwm_set_enable(dev, pwm_id, 0);
		pwm_wait(dev, pwm_id);
	}

	reg = (reg & ~(0xF << bit_shift)) | (PHASE_MODE << bit_shift);
	pwm_writel(reg, tcc->pwm_base + PWMMODE);

	bit_shift = (2 * pwm_id) + 24;
	reg = (reg & ~(0x3 << bit_shift)) | k  << bit_shift;
	pwm_writel(reg, tcc->pwm_base + PWMMODE);  //divide

	pwm_writel(low_cnt, tcc->pwm_base + PWMPSTN1(pwm_id));
	pwm_writel(hi_cnt, tcc->pwm_base + PWMPSTN2(pwm_id));


	reg = pwm_readl(tcc->pwm_base + PWMMODE);
	bit_shift = (1 << pwm_id) + 16;
	reg = (reg & ~(0x1 << bit_shift)) | (0x0 << bit_shift);
	pwm_writel(reg, tcc->pwm_base + PWMMODE);

	pwm_set_enable(dev, pwm_id, 1);

	return 0;

pwm_hi_out:
	pwm_register_mode_set(dev, pwm_id, 0xFFFFFFFF);
	pwm_set_enable(dev, pwm_id, 1);
	return 0;

pwm_low_out:
	pwm_register_mode_set(dev, pwm_id, 0x00000000);
	pwm_set_enable(dev, pwm_id, 1);
	return 0;

clk_error:
	dprintf("%s ERROR clk_freq:%lu\n", __func__, clk_freq);
	return -1;
}

static int pwm_wait(struct udevice *dev, int pwm_id)
{
	struct tcc_chip *tcc = dev_get_priv(dev);
	unsigned int delay_cnt;
	unsigned int busy;

	delay_cnt = 0xFFFFFFF;
	if (pwm_id < 0 || pwm_id >= NUM_PWM) {
		pr_force("%s : invalid pwm_id\n", __func__);
		return -1;
	}

	while (delay_cnt--) {
		busy = pwm_readl(tcc->pwm_base);
		if (!(busy  & (0x1 << pwm_id)))
			break;
	}
	dprintf("%s pwm_id: %d delay_cnt: %d\n", __func__, pwm_id, delay_cnt);

	return 0;
}

static int pwm_register_mode_set(struct udevice *dev,
		int pwm_id, unsigned int regist_value)
{
	struct tcc_chip *tcc = dev_get_priv(dev);
	unsigned int  reg = 0, bit_shift = 0;

	if (pwm_id < 0 || pwm_id >= NUM_PWM) {
		pr_force("%s : invalid pwm_id\n", __func__);
		return -1;
	}

	bit_shift = 4 * pwm_id;
	reg = pwm_readl(tcc->pwm_base + PWMMODE);

	if (((reg >> bit_shift) & 0xF) != REGISTER_OUT_MODE) {
		pwm_set_enable(dev, pwm_id, 0);
		pwm_wait(dev, pwm_id);
	}
	reg = (reg & ~(0xF << bit_shift)) | REGISTER_OUT_MODE << bit_shift;
	pwm_writel(reg, tcc->pwm_base + PWMMODE);  //phase mode

	reg = pwm_readl(tcc->pwm_base + PWMMODE);
	bit_shift = (2 * pwm_id) + 24;
	reg = (reg & ~(0x3 << bit_shift));
	pwm_writel(reg, tcc->pwm_base + PWMMODE);

	pwm_writel(regist_value, tcc->pwm_base + PWMOUT1(pwm_id));

	return 0;
}


static int tcc_pwm_set_enable(struct udevice *dev, uint pwm_id, bool enable)
{
	struct tcc_chip *tcc = dev_get_priv(dev);

	if (enable == true) {
		pwm_writel(pwm_readl(tcc->pwm_base+PWMEN) | (0x00010<<(pwm_id)),
				tcc->pwm_base + PWMEN);
		pwm_writel(pwm_readl(tcc->pwm_base+PWMEN) | (0x00011<<(pwm_id)),
				tcc->pwm_base + PWMEN);
		pwm_writel(pwm_readl(tcc->pwm_base+PWMEN) | (0x10011<<(pwm_id)),
				tcc->pwm_base + PWMEN);
		dprintf("%s : pwm_id = %d enable\n", __func__, pwm_id);
	} else {
		pwm_writel(pwm_readl(tcc->pwm_base + PWMEN) & ~(1<<(pwm_id)),
				tcc->pwm_base + PWMEN);
		dprintf("%s : pwm_id = %d disable\n", __func__, pwm_id);
	}

	return 0;
}

static int tcc_pwm_probe(struct udevice *dev)
{
	struct tcc_chip *tcc = dev_get_priv(dev);
	u32 freq;
	int ret;

	tcc->pwm_base = (void __iomem *)dev_read_addr_index(dev, 0);
	tcc->io_pwm_base = (void __iomem *)dev_read_addr_index(dev, 1);

	ret = tcc_clk_get_by_index(dev, 0, &(tcc->pwm_pclk));
	if (ret < 0) {
		pr_err("[Error]%s: failed to get peripheral clock\n", __func__);
		return ret;
	}

	ret = dev_read_u32(dev, "clock-frequency", &freq);
	if (ret < 0) {
		freq = PWM_PERI_CLOCK;
		pr_info("pwm default clock :%d init", PWM_PERI_CLOCK);
	} else {
		pr_info("pwm default clock :%d init", freq);
	}

	ret = tcc_clk_get_by_index(dev, 1, &(tcc->pwm_ioclk));
	if (ret < 0) {
		pr_err("[Error]%s: failed to get pwm io clock.\n", __func__);
		return ret;
	}

#ifdef TCC_USE_GFB_PORT
	/* Get pwm gfb number A(0), B(1), C(2), D(3). */
	ret = dev_read_u32_array(dev, "gfb-port", tcc->gfb_port, NUM_PWM);

	dprintf("%s: pwm[A]:%d pwm[B]:%d pwm[C]:%d pwm[D]:%d\n", __func__,
			tcc->gfb_port[0], tcc->gfb_port[1],
			tcc->gfb_port[2], tcc->gfb_port[3]);

	tcc->io_pwm_port_base = (void __iomem *)dev_read_addr_index(dev, 2);
#endif

	ret = clk_set_rate(&(tcc->pwm_pclk), freq);
	if (ret < 0) {
		pr_err("[Error]%s: faile to set peripheral clock\n", __func__);
		return ret;
	}


	dprintf("%s: peri_clk:%d return:%ld\n", __func__,
			freq, clk_get_rate(&(tcc->pwm_pclk)));
	return 0;

}

static const struct pwm_ops tcc_pwm_ops = {
	.set_config	= tcc_pwm_set_config,
	.set_enable	= tcc_pwm_set_enable,
};

static const struct udevice_id tcc_pwm_ids[] = {
	{ .compatible = "telechips,pwm" },
	{ }
};

U_BOOT_DRIVER(tcc_pwm) = {
	.name	= "tcc_pwm",
	.id	= UCLASS_PWM,
	.of_match = tcc_pwm_ids,
	.ops	= &tcc_pwm_ops,
	.probe	= tcc_pwm_probe,
	.priv_auto = sizeof(struct tcc_chip),
};
