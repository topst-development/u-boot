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

#if defined(CONFIG_TCC807X)
#define NUM_PWM 6
#else
#define NUM_PWM 4
#endif

#define dprintf(msg...)	if (debug_mode) {pr_force("pwm-tcc: " msg);}

/* Debugging stuff */
static bool debug_mode = false;
#define PWMEN				0x4
#define PWMMODE				0x8
#define PWMPSTN1(CH)		(0x10U + (0x10U * (CH)))
#define PWMPSTN2(CH)		(0x14U + (0x10U * (CH)))
#define PWMPSTN3(CH)		(0x18U + (0x10U * (CH)))
#define PWMPSTN4(CH)		(0x1cU + (0x10U * (CH)))

#define PWMOUT1(CH)			(0x50U + (0x10U* (CH)))
#define PWMOUT2(CH)			(0x54U + (0x10U* (CH)))
#define PWMOUT3(CH)			(0x58U + (0x10U* (CH)))
#define PWMOUT4(CH)			(0x5cU + (0x10U* (CH)))
#define PWMOUT5(CH)         (0x30U + (0x10U* (CH)))

#define PHASE_MODE			(1)
#define REGISTER_OUT_MODE	(2)

#define PWM_DIVID_MAX	3 // clock divide max value 3(divide 16)
#define PWM_PERI_CLOCK	(400 * 1000 * 1000) // 400Mhz

#if defined(CONFIG_TCC802X) || defined(CONFIG_TCC803X) \
	|| defined(CONFIG_TCC805X) || defined(CONFIG_TCC807X) \
	|| defined(CONFIG_TCN100X)
#define TCC_USE_GFB_PORT
#endif

#if defined(CONFIG_TCN100X)
#define BIT_SHIFT 0
#else
#define BIT_SHIFT 8
#endif

#define pwm_writel writel
#define pwm_readl readl

static int pwm_register_mode_set(struct udevice *dev,
		int pwm_id, unsigned int regist_value);
static int pwm_wait(const struct udevice *dev, int pwm_id);


struct tcc_chip {
	void __iomem		*pwm_base;
	void __iomem		*io_pwm_base;

	struct clk		pwm_pclk;
	struct clk		pwm_ioclk;
	unsigned int		port[4];
	u32 pwm_num;
	u32 pwm_num_id;
#ifdef TCC_USE_GFB_PORT
	void __iomem		*io_pwm_port_base;
	u32 gfb_port;
#endif
};

static int32_t pwm_hi_out(struct udevice *dev, uint pwm_id, int *flag) 
{
	if (pwm_id < (unsigned int)NUM_PWM) {
		(void)pwm_register_mode_set(dev, (int)pwm_id, 0xFFFFFFFFU);
	}
	(void)pwm_set_enable(dev, pwm_id, 1);
	*flag = 1;
	return 0;
}
static int32_t pwm_low_out(struct udevice *dev, uint pwm_id, int *flag)
{
	if (pwm_id < (unsigned int)NUM_PWM) {
		(void)pwm_register_mode_set(dev, (int)pwm_id, 0x00000000U);
	}
	(void)pwm_set_enable(dev, pwm_id, 1);
	*flag = 1;
	return 0;
}
static int32_t clk_error(unsigned long clk_freq, int *flag)
{
	dprintf("%s ERROR clk_freq:%lu\n", __func__, clk_freq);
	*flag = 1;
	return -1;
}

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
	int ret = 0;
	int flag = 0;
#ifdef TCC_USE_GFB_PORT
	unsigned int gfb_port_value = 0;
#endif
	clk_freq = clk_get_rate(&tcc->pwm_pclk);
	if ((clk_freq == (unsigned int)0) || (duty_ns > period_ns)) {
		ret = clk_error(clk_freq, &flag);
	}
	if (flag == 0 && (pwm_id == tcc->pwm_num)) {
		clk_period_ns = (1000ULL * 1000ULL * 1000ULL) / clk_freq;

		dprintf("%s clk_freq:%lu npwn:%d duty_ns:%d period_ns:%llu hwpwm:%d\n",
				__func__, clk_freq, pwm_id, duty_ns,
				period_ns, NUM_PWM);
#ifdef TCC_USE_GFB_PORT
		gfb_port_value = pwm_readl(tcc->io_pwm_port_base);
		if (pwm_id < (unsigned int)NUM_PWM){
			gfb_port_value &= ~((unsigned int)0xFF << ((unsigned int)BIT_SHIFT * tcc->pwm_num_id));
			gfb_port_value |= (tcc->gfb_port & (unsigned int)0xFF) << ((unsigned int)BIT_SHIFT * tcc->pwm_num_id);
		}
		pwm_writel(gfb_port_value, tcc->io_pwm_port_base);
#endif

		if (duty_ns == (unsigned int)0) {
			ret = pwm_low_out(dev, pwm_id, &flag);
		}
		if (duty_ns == period_ns){
			ret = pwm_hi_out(dev, pwm_id, &flag);
		}
		if (flag == 0) { 
			while (true) {
				if (clk_period_ns <= (ULLONG_MAX / 2ULL)) {
					clk_period_ns = clk_period_ns * (2ULL);
				}
				total_cnt = period_ns/ clk_period_ns;

				if (total_cnt <= (unsigned int)1) {
					if (duty_ns > (period_ns/(unsigned int)2)) {
						ret = pwm_hi_out(dev, pwm_id, &flag);
					}
					else {
						ret = pwm_low_out(dev, pwm_id, &flag);
					}
				}
				if ((flag == 1) || (k == (unsigned int)PWM_DIVID_MAX) || (total_cnt <= 0xFFFFFFFFU)) {
					break;
				}
				k++;
			}
			if(flag == 0) {
				//prevent over flow.
				for (divide = 1; divide < 0xFFFFFFFFU; divide++) {
					// 0xFFFFFFFF > total_cnt * duty / divide
					if ((ULLONG_MAX/ duty_ns) > (total_cnt/divide)){
						break;
					}
				}

				cal_duty = duty_ns / divide;
				cal_period = (unsigned int)period_ns / (unsigned int)divide;

				hi_cnt = (unsigned int)((total_cnt * (cal_duty))/ (cal_period));
				low_cnt = (unsigned int)total_cnt - hi_cnt;


				dprintf("k:%d clk_p:%llu total: %llu hi:%d low:%d\n",
						k, clk_period_ns, total_cnt, hi_cnt, low_cnt);

				reg = pwm_readl(tcc->pwm_base + PWMMODE);

				bit_shift = (unsigned int)4 * tcc->pwm_num_id;
				if(bit_shift < (unsigned int)32) {
					if (((reg >> bit_shift) & (unsigned int)0xF) != (unsigned int)PHASE_MODE) {
						(void)pwm_set_enable(dev, pwm_id, 0);
						(void)pwm_wait(dev, (int)pwm_id);
					}
					reg = (reg & ~(((unsigned int)0xF) << bit_shift)) | ((unsigned int)PHASE_MODE << bit_shift);
				}
				pwm_writel(reg, tcc->pwm_base + PWMMODE);

				bit_shift = ((unsigned int)2 * tcc->pwm_num_id) + (unsigned int)24;
				if (bit_shift < (unsigned int)32) {
					reg = (reg & ~((unsigned int)0x3 << bit_shift)) | (k  << bit_shift);
				}
				pwm_writel(reg, tcc->pwm_base + PWMMODE);  //divide
				pwm_writel(low_cnt, tcc->pwm_base + PWMPSTN1(tcc->pwm_num_id));
				pwm_writel(hi_cnt, tcc->pwm_base + PWMPSTN2(tcc->pwm_num_id));
				reg = pwm_readl(tcc->pwm_base + PWMMODE);
				if (pwm_id < (unsigned int)32) {
					bit_shift = ((unsigned int)1 << (unsigned int)tcc->pwm_num_id) + (unsigned int)16;
				}
				if (bit_shift < (unsigned int)32) {
					reg = (reg & ~((unsigned int)0x1 << bit_shift)) | ((unsigned int)0x0 << bit_shift);
				}
				pwm_writel(reg, tcc->pwm_base + PWMMODE);

				(void)pwm_set_enable(dev, pwm_id, 1);
			}
		}
	}
	return ret;
}

static int pwm_wait(const struct udevice *dev, int pwm_id)
{
	const struct tcc_chip *tcc = dev_get_priv(dev);
	unsigned int delay_cnt;
	unsigned int busy;
	int ret = 0;

	delay_cnt = 0xFFFFFFF;
	if (tcc->pwm_num != pwm_id) {
		pr_err("[Error]%s: PWM channels do not match. pwm_id : %u pwm_num : %u\n", __func__, pwm_id, tcc->pwm_num);
		ret = -1;
	} else if ((pwm_id < 0) || (pwm_id >= NUM_PWM)) {
		pr_force("%s : invalid pwm_id\n", __func__);
		ret = -1;
	}
	else {
		while (delay_cnt > (unsigned int)0) {
			busy = pwm_readl(tcc->pwm_base);
			if ((unsigned int)0 == (busy  & ((unsigned int)0x1 << (unsigned int)tcc->pwm_num_id))) {
				break;
			}
			delay_cnt--;
		}
		dprintf("%s pwm_id: %d delay_cnt: %d\n", __func__, pwm_id, delay_cnt);
	}
	return ret;
}

static int pwm_register_mode_set(struct udevice *dev,
		int pwm_id, unsigned int regist_value)
{
	struct tcc_chip *tcc = dev_get_priv(dev);
	unsigned int  reg = 0, bit_shift = 0;
	int ret = 0;
	if(tcc->pwm_num != pwm_id) {
		pr_err("[Error]%s: PWM channels do not match. pwm_id : %u pwm_num : %u\n", __func__, pwm_id, tcc->pwm_num);
		ret = -1;
	} else if ((pwm_id < 0) || (pwm_id >= NUM_PWM)) {
		pr_force("%s : invalid pwm_id\n", __func__);
		ret = -1;
	} else {
		bit_shift = (unsigned int)4 * (unsigned int)tcc->pwm_num_id;
		reg = pwm_readl(tcc->pwm_base + PWMMODE);

		if (((reg >> bit_shift) & (unsigned int)0xF) != (unsigned int)REGISTER_OUT_MODE) {
			(void)pwm_set_enable(dev, (unsigned int)pwm_id, 0);
			(void)pwm_wait(dev, pwm_id);
		}
		reg = (reg & ~((unsigned int)0xF << bit_shift)) | ((unsigned int)REGISTER_OUT_MODE << bit_shift);
		pwm_writel(reg, tcc->pwm_base + PWMMODE);  //phase mode

		reg = pwm_readl(tcc->pwm_base + PWMMODE);
		bit_shift = ((unsigned int)2 * (unsigned int)tcc->pwm_num_id) + (unsigned int)24;
		reg = (reg & ~((unsigned int)0x3 << bit_shift));
		pwm_writel(reg, tcc->pwm_base + PWMMODE);
		if(pwm_id > 3) {
			pwm_writel(regist_value, tcc->pwm_base + PWMOUT5(tcc->pwm_num_id));
		} else {
			pwm_writel(regist_value, tcc->pwm_base + PWMOUT1(tcc->pwm_num_id));
		}
	}
	return ret;
}
static int tcc_pwm_set_enable(struct udevice *dev, uint pwm_id, bool enable)
{
	struct tcc_chip *tcc = dev_get_priv(dev);
	int ret = 0;
	if(tcc->pwm_num != pwm_id) {
		pr_err("[Error]%s: PWM channels do not match. pwm_id : %u pwm_num : %u\n", __func__, pwm_id, tcc->pwm_num);
		ret = -1;
	} else if (0 <= pwm_id || pwm_id < (unsigned int)NUM_PWM) {
		if (enable == true) {
			pwm_writel(pwm_readl(tcc->pwm_base+PWMEN) | ((unsigned int)0x00010<<(tcc->pwm_num_id)),
					tcc->pwm_base + PWMEN);
			pwm_writel(pwm_readl(tcc->pwm_base+PWMEN) | ((unsigned int)0x00011<<(tcc->pwm_num_id)),
					tcc->pwm_base + PWMEN);
			pwm_writel(pwm_readl(tcc->pwm_base+PWMEN) | ((unsigned int)0x10011<<(tcc->pwm_num_id)),
					tcc->pwm_base + PWMEN);
			dprintf("%s : pwm_id = %d enable\n", __func__, pwm_id);
		} else {
			pwm_writel(pwm_readl(tcc->pwm_base + PWMEN) & ~((unsigned int)1<<(tcc->pwm_num_id)),
					tcc->pwm_base + PWMEN);
			dprintf("%s : pwm_id = %d disable\n", __func__, pwm_id);
		}
	}
	return ret;
}
static int tcc_pwm_probe(struct udevice *dev)
{
	struct tcc_chip *tcc = dev_get_priv(dev);
	u32 freq;
	int ret = 0;
	volatile uintptr_t result;
	unsigned long temp;

	result = dev_read_addr_index(dev, 0);
	tcc->pwm_base = (void __iomem *)result;
	result = dev_read_addr_index(dev, 1);
	tcc->io_pwm_base = (void __iomem *)result;
	ret = tcc_clk_get_by_index(dev, 0, &(tcc->pwm_pclk));
	if (ret < 0) {
		pr_err("[Error]%s: failed to get peripheral clock\n", __func__);
	}	
	else {
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
		}
		else {
			ret = dev_read_u32(dev, "pwm-num", &tcc->pwm_num);
			tcc->pwm_num_id = tcc->pwm_num % 4;
			if (ret < 0) {
				pr_err("[Error]%s: failed to get pwm_num.\n", __func__);
			} else {
#ifdef TCC_USE_GFB_PORT
				ret = dev_read_u32(dev, "gfb-port", &tcc->gfb_port);

				dprintf("%s: %s: gfb-port[%d]\n", __func__, dev->name, tcc->gfb_port);
				result = dev_read_addr_index(dev, 2);
				tcc->io_pwm_port_base = (void __iomem *)result;
#endif
				temp = clk_set_rate(&(tcc->pwm_pclk), (unsigned long)freq);
				if (temp < (unsigned int)0xFFFFFFFFU) {
					ret = (int)temp;
				}
				if (ret < 0) {
					pr_err("[Error]%s: faile to set peripheral clock\n", __func__);
				}
				else {
					dprintf("%s: %s: peri_clk:%d return:%ld\n", __func__,
							dev->name, freq, clk_get_rate(&(tcc->pwm_pclk)));
				}
			}
		}
	}
	return ret;
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
