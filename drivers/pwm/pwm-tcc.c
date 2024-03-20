// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/telechips/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/telechips/vioc/reg_physical.h>
#include <asm/arch/clock.h>

#define REG_PDM_CFG 0x1605104C
#define REG_PDM_PCFG 0x16051094
#define REG_PDM_BASE 0x16030000
#define NUM_PWM 4


/* Debugging stuff */
static int debug = 1;
#define dprintf(msg...)	if (debug) pr_force("\x1b[33m pwm-tcc: \x1b[0m" msg)


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

#define PWM_DIVID_MAX		3	// clock divide max value 3(divide 16)
#define PWM_PERI_CLOCK		(400 * 1000 * 1000)

#define pwm_writel writel
#define pwm_readl readl

static int pwm_wait(int pwm_id);
static int pwm_register_mode_set(int pwm_id, unsigned int regist_value);
static int do_pwm_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]);
static int tcc_pwm_atoi(const char *s);

struct tcc_chip {
	void __iomem *reg_pwm_base;
	void __iomem *reg_pwm_cfg;

	unsigned int pwm_peri_clock;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	void __iomem *reg_pwm_pcfg;
	unsigned int pcfg_port[NUM_PWM];
#endif
};

struct tcc_chip pwm_chip;

int pwm_enable(int pwm_id)
{
	struct tcc_chip *pwm = &pwm_chip;

	if (pwm_id < 0 || pwm_id >= NUM_PWM) {
		pr_force("%s : invalid pwm_id\n", __func__);
		return -1;
	}

	dprintf("%s : pwm_id = %d\n", __func__, pwm_id);

	pwm_writel(pwm_readl(pwm->reg_pwm_base + PWMEN) | (0x00010<<(pwm_id)),
			pwm->reg_pwm_base + PWMEN);
	pwm_writel(pwm_readl(pwm->reg_pwm_base + PWMEN) | (0x00011<<(pwm_id)),
			pwm->reg_pwm_base + PWMEN);
	pwm_writel(pwm_readl(pwm->reg_pwm_base + PWMEN) | (0x10011<<(pwm_id)),
			pwm->reg_pwm_base + PWMEN);

	pwm_writel(0x000, pwm->reg_pwm_base);

	return 0;
}

int pwm_disable(int pwm_id)
{
	struct tcc_chip *pwm = &pwm_chip;

	if (pwm_id < 0 || pwm_id >= NUM_PWM) {
		pr_force("%s : invalid pwm_id\n", __func__);
		return -1;
	}

	dprintf("%s : pwm_id = %d\n", __func__, pwm_id);

	pwm_writel(pwm_readl(pwm->reg_pwm_base + PWMEN) & ~(1<<(pwm_id)),
			pwm->reg_pwm_base + PWMEN);
	pwm_writel(pwm_readl(pwm->reg_pwm_base + PWMEN) | (0x10010<<(pwm_id)),
			pwm->reg_pwm_base + PWMEN);

	return 0;
}

static int pwm_wait(int pwm_id)
{
	struct tcc_chip *pwm = &pwm_chip;
	unsigned int delay_cnt;
	unsigned int busy;

	delay_cnt = 0xffffffff;

	if (pwm_id < 0 || pwm_id >= NUM_PWM) {
		pr_force("%s : invalid pwm_id\n", __func__);
		return -1;
	}

	while (delay_cnt--) {
		busy = pwm_readl(pwm->reg_pwm_base);
		if (!(busy  & (0x1 << pwm_id)))
			break;
	}
	dprintf("%s : pwm_id = %d, delay_cnt = %d\n",
			__func__, pwm_id, delay_cnt);

	return 0;
}

static int pwm_register_mode_set(int pwm_id, unsigned int regist_value)
{
	struct tcc_chip *pwm = &pwm_chip;

	if (pwm_id < 0 || pwm_id >= NUM_PWM) {
		pr_force("%s : invalid pwm_id\n", __func__);
		return -1;
	}

	unsigned int  reg = 0, bit_shift = 0;

	bit_shift = 4 * pwm_id;
	reg = pwm_readl(pwm->reg_pwm_base + PWMMODE);

	if (((reg >> bit_shift) & 0xF) != REGISTER_OUT_MODE) {
		pwm_disable(pwm_id);
		pwm_wait(pwm_id);
	}
	reg = (reg & ~(0xF << bit_shift)) | REGISTER_OUT_MODE << bit_shift;
	pwm_writel(reg, pwm->reg_pwm_base + PWMMODE);  //phase mode

	reg = pwm_readl(pwm->reg_pwm_base + PWMMODE);
	bit_shift = (2 * pwm_id) + 24;
	reg = (reg & ~(0x3 << bit_shift));
	pwm_writel(reg, pwm->reg_pwm_base + PWMMODE);

	pwm_writel(regist_value, pwm->reg_pwm_base + PWMOUT1(pwm_id));

	return 0;
}

int pwm_config(int pwm_id, int duty_ns, int period_ns_)
{
	struct tcc_chip *pwm = &pwm_chip;

	if (pwm_id < 0 || pwm_id >= NUM_PWM) {
		pr_force("%s : invalid pwm_id\n", __func__);
		return -1;
	}
	unsigned int k = 0, reg = 0, bit_shift = 0;
	unsigned long clk_freq;
	unsigned long divide = 0;
	unsigned int  cal_duty = 0, cal_period = 0;
	unsigned int hi_cnt = 0, low_cnt = 0;
	uint64_t total_cnt = 0;
	uint64_t clk_period_ns = 0;
	uint64_t period_ns = (uint64_t)period_ns_;

	clk_freq = tcc_get_peri(PERI_PDM);


	if (clk_freq == 0 || (duty_ns > period_ns))
		goto clk_error;

	clk_period_ns = (1000 * 1000 * 1000)/clk_freq;
	//clk_period_ns = div_u64((1000 * 1000 * 1000), clk_freq);

	dprintf("%s clk_freq:%lu npwn:%d duty_ns:%d period_ns:%llu hwpwm:%d\n",
			__func__, clk_freq, pwm_id, duty_ns,
			period_ns, NUM_PWM);

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


	dprintf("k: %d clk_p: %llu total:%llu hi:%d low:%d\n",
			k, clk_period_ns, total_cnt, hi_cnt, low_cnt);

	reg = pwm_readl(pwm->reg_pwm_base + PWMMODE);

	bit_shift = 4 * pwm_id;

	if (((reg >> bit_shift) & 0xF) != PHASE_MODE) {
		pwm_disable(pwm_id);
		pwm_wait(pwm_id);
	}

	reg = (reg & ~(0xF << bit_shift)) | (PHASE_MODE << bit_shift);
	pwm_writel(reg, pwm->reg_pwm_base + PWMMODE);

	bit_shift = (2 * pwm_id) + 24;
	reg = (reg & ~(0x3 << bit_shift)) | k  << bit_shift;
	pwm_writel(reg, pwm->reg_pwm_base + PWMMODE);  //divide

	pwm_writel(low_cnt, pwm->reg_pwm_base + PWMPSTN1(pwm_id));
	pwm_writel(hi_cnt, pwm->reg_pwm_base + PWMPSTN2(pwm_id));


	reg = pwm_readl(pwm->reg_pwm_base + PWMMODE);
	bit_shift = (1 << pwm_id) + 16;
	reg = (reg & ~(0x1 << bit_shift)) | (0x0 << bit_shift);
	pwm_writel(reg, pwm->reg_pwm_base + PWMMODE);

	pwm_enable(pwm_id);

	return 0;

pwm_hi_out:
	pwm_register_mode_set(pwm_id, 0xFFFFFFFF);
	pwm_enable(pwm_id);
	return 0;

pwm_low_out:
	pwm_register_mode_set(pwm_id, 0x00000000);
	pwm_enable(pwm_id);
	return 0;

clk_error:
	dprintf("%s ERROR clk_freq:%lu\n", __func__, clk_freq);
	return -1;
}


int pwm_init(int pwm_id, int pdm_out)
{
	struct tcc_chip *pwm = &pwm_chip;
	unsigned int reg;
	int idx;

	if (pwm_id < 0 || pwm_id >= NUM_PWM) {
		pr_force("%s : invalid pwm_id\n", __func__);
		return -1;
	}

	dprintf("%s : pdm_out ", __func__, pdm_out);
	tcc_set_peri(PERI_PDM, ENABLE, PWM_PERI_CLOCK, 0);

	pwm->reg_pwm_base = (void __iomem *)REG_PDM_BASE;
	pwm->reg_pwm_pcfg = (void __iomem *)REG_PDM_PCFG;
	pwm->reg_pwm_cfg = (void __iomem *)REG_PDM_CFG;
	pwm->pcfg_port[pwm_id] = pdm_out;
	reg = 0;
	for (idx = 0; idx < NUM_PWM; idx++)
		reg |= (pwm->pcfg_port[idx] << (idx * 8));
	pwm_writel(reg, pwm->reg_pwm_pcfg);
	dprintf("%s id:%d, pdm_out:%d peri_clk:%lu pcfg: 0x%08x\n",
			__func__, pwm_id, pdm_out,
			tcc_get_peri(PERI_PDM),
			pwm_readl(pwm->reg_pwm_pcfg));

	return 0;
}
