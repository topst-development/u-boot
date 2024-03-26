// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <adc.h>
#include <clk.h>
#include <asm/arch/adc.h>
#include <linux/io.h>
#include <mach/clock.h>
#include <linux/delay.h>

struct tcc_adc {
	void __iomem	*regs;		/* base address */
	void __iomem	*clk_regs;	/* clock address */
	struct clk      hclk;

	uint32_t	adc_cycle_time;	/* ADC CKIN cycle */
};


static void tcc_adc_power_control(const struct tcc_adc *adc, bool power_on)
{
#if defined(CONFIG_TCC807X)
	if (power_on) {
		(void)tcc_set_iobus_pwdn(adc->hclk.id, (bool)false);
	} else {
		(void)tcc_set_iobus_pwdn(adc->hclk.id, (bool)true);
	}
#else
	uint32_t con;

	con = readl(adc->clk_regs + PCLK_CFG_ADC_CTRL);
	if (power_on) {
		/* enable clock for adc*/
		con |= ADC_CLK_OUT_EN;
		con |= ADC_CLK_DIV_EN;
	} else {
		/* disable clock for adc*/
		con &= ~(u32)(ADC_CLK_OUT_EN | ADC_CLK_DIV_EN);
	}
	writel(con, adc->clk_regs + PCLK_CFG_ADC_CTRL);
#endif
}

static int32_t tcc_adc_start_channel(struct udevice *dev, int32_t channel)
{
	const struct tcc_adc *adc = dev_get_priv(dev);
	uint32_t con;
	int32_t ret = 0;

	if ((channel >= ADC_CHANNEL_START) &&
	    (channel <= ADC_CHANNEL_END)) {
		con = ADCCMD_SMP_CMD((uint32_t)channel);
		writel(con, (adc->regs + ADCCMD));
	} else {
		ret = -EINVAL;
	}

	return ret;
}

static int32_t tcc_adc_channel_data(struct udevice *dev,
		int32_t channel, uint32_t *data)
{
	const struct tcc_adc *adc = dev_get_priv(dev);
	uint32_t con;
	int32_t cnt, ret = 0;

	if ((channel < ADC_CHANNEL_START) || (channel > ADC_CHANNEL_END)) {
		ret = -EINVAL;
	}

	if (ret == 0) {
		/* check sampling channel */
		con = readl(adc->regs + ADCCMD);
		if ((con & ADCCMD_SMP_CMD((uint32_t)channel)) == 0U) {
			(void)pr_err("[ERROR][ADC]channel %d is not active. (0x%x)\n",
					channel, con);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		/* wait for sampling completion */
		cnt = 0;
		do {
			udelay(adc->adc_cycle_time);
			con = readl(adc->regs + ADCCMD);
			if (cnt > ADC_CONV_CYCLE) {
				(void)pr_err("[ERROR][ADC] not complete sampling (reg: 0x%x)",
						con);
				ret = -EIO;
				break;
			}
			cnt++;
		} while ((con & ADCCMD_DONE) == 0U);
	}

	if (ret == 0) {
		*data = readl(adc->regs + ADCDATA0) & ADCDATA_MASK;
	}

	return ret;
}

static int32_t tcc_adc_channels_data(struct udevice *dev, uint32_t channel_mask,
		struct adc_channel *channels)
{
	int32_t ret = 0;
	int32_t channel = 0;
	uint32_t channel_bits = 0;

	if (channels == NULL) {
		ret = -ENOMEM;
	} else {
		for (channel = ADC_CHANNEL_START; channel <= ADC_CHANNEL_END; channel++) {
			channel_bits = ((u32)1 << (u32)channel);
			if ((channel_bits & channel_mask) != 0U) {
				channels[channel].id = channel;
				ret = tcc_adc_start_channel(dev, channel);
				if (ret == 0) {
					ret = tcc_adc_channel_data(dev, channel, &channels[channel].data);
				}

				if (ret < 0) {
					break;
				}
			}
		}
	}

	return ret;
}

static int32_t tcc_adc_stop(struct udevice *dev)
{
	const struct tcc_adc *adc = dev_get_priv(dev);

	tcc_adc_power_control(adc, false);

	return 0;
}

static int32_t tcc_adc_probe(struct udevice *dev)
{
	struct tcc_adc *adc = dev_get_priv(dev);
	int32_t ret = 0;

	if (adc == NULL) {
		(void)pr_err("[ERROR][ADC] %s: tcc_adc is null!\n", __func__);
		ret = -ENOMEM;
	} else {
		/* base, clock control unit address */
		adc->regs = (void *)dev_read_addr_index(dev, 0);
#if defined(CONFIG_TCC807X)
		ret = tcc_clk_get_by_index(dev, 0, &adc->hclk);
#else
		adc->clk_regs = (void *)dev_read_addr_index(dev, 1);
#endif
		if (ret == 0) {
			/* enable adc peri clk */
			tcc_adc_power_control(adc, true);

			/* (1 / (ADC_CKIN_CLOCK / 1 cycles)) (us) */
			adc->adc_cycle_time = (1U / (ADC_CKIN_CLOCK / 1000000U));
			if (adc->adc_cycle_time == 0U) {
				adc->adc_cycle_time = 1U;
			}
		}
	}

	return ret;
}

static int32_t tcc_adc_ofdata_to_platdata(struct udevice *dev)
{
	struct adc_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);

	uc_pdata->data_mask = ADCDATA_MASK;

	uc_pdata->channel_mask = ((u32)1U << TCC_ADC_CHANNELS) - 1U;

	return 0;
}

static const struct adc_ops tcc_adc_ops = {
	.start_channel = tcc_adc_start_channel,
	.channel_data = tcc_adc_channel_data,
	.channels_data = tcc_adc_channels_data,
	.stop = tcc_adc_stop,
};

static const struct udevice_id tcc_adc_ids[] = {
	{ .compatible = "telechips,tcc803x-adc" },
	{ .compatible = "telechips,tcc805x-adc" },
	{ .compatible = "telechips,tcc807x-adc" },
	{ }
};

U_BOOT_DRIVER(tcc_adc) = {
	.name		= "tcc-adc",
	.id		= UCLASS_ADC,
	.of_match	= tcc_adc_ids,
	.ops		= &tcc_adc_ops,
	.probe		= tcc_adc_probe,
	.priv_auto 	= (int)sizeof(struct tcc_adc),
	.of_to_plat	= tcc_adc_ofdata_to_platdata,
};
