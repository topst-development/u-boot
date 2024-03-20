// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <adc.h>
#include <asm/telechips/adc.h>
#include <linux/io.h>

struct tcc_adc {
	void __iomem	*regs;		/* base address */
	void __iomem	*clk_regs;	/* clock address */

	uint32_t	adc_cycle_time;	/* ADC CKIN cycle */
};

static void tcc_adc_power_control(struct tcc_adc *adc, bool power_on)
{
	uint32_t con;

	con = readl(adc->clk_regs + PCLK_CFG_ADC_CTRL);
	if (power_on) {
		/* enable clock for r5 adc*/
		con |= ADC_CLK_OUT_EN;
		con |= ADC_CLK_DIV_EN;
	} else {
		/* disable clock for r5 adc*/
		con &= ~(ADC_CLK_OUT_EN | ADC_CLK_DIV_EN);
	}
	writel(con, adc->clk_regs + PCLK_CFG_ADC_CTRL);

	pr_debug("[DEBUG][ADC] %s: ADC PCLK (0x%x)\n",
			__func__,
			readl(adc->clk_regs + PCLK_CFG_ADC_CTRL));
}

int32_t tcc_adc_start_channel(struct udevice *dev, int32_t channel)
{
	struct tcc_adc *adc = dev_get_priv(dev);
	uint32_t con;

	if ((channel < ADC_CHANNEL_START) ||
			(channel > ADC_CHANNEL_END)) {
		pr_err("[ERR][ADC]Input channel invalid : %d\n", channel);
		return -EINVAL;
	}

	con = ADCCMD_SMP_CMD((uint32_t)channel);
	writel(con, adc->regs + ADCCMD);

	pr_debug("[DEBUG][ADC] %s: ADC CMD (0x%x)\n",
			__func__,
			readl(adc->regs + ADCCMD));

	return 0;
}

int32_t tcc_adc_channel_data(struct udevice *dev,
		int32_t channel, uint32_t *data)
{
	struct tcc_adc *adc = dev_get_priv(dev);
	uint32_t con;
	int32_t cnt;

	if ((channel < ADC_CHANNEL_START) || (channel > ADC_CHANNEL_END)) {
		pr_err("[ERROR][ADC] channel %d is not supported\n", channel);
		return -EINVAL;
	}

	/* check sampling channel */
	con = readl(adc->regs + ADCCMD);
	if ((con & ADCCMD_SMP_CMD((uint32_t)channel)) == 0U) {
		pr_err("[ERROR][ADC]channel %d is not active. (0x%x)\n",
				channel, con);
		return -EINVAL;
	}

	/* wait for sampling completion */
	cnt = 0;
	do {
		udelay(adc->adc_cycle_time);
		con = readl(adc->regs + ADCCMD);
		if (cnt > ADC_CONV_CYCLE) {
			pr_err("[ERROR][ADC] not complete sampling (reg: 0x%x)",
					con);
			return -EIO;
		}
		cnt++;
	} while ((con & ADCCMD_DONE) == 0U);

	*data = readl(adc->regs + ADCDATA0) & ADCDATA_MASK;

	pr_debug("[DEBUG][ADC] %s: ADC ch %d - data 0x%x\n",
			__func__, channel, *data);
	return 0;
}

int32_t tcc_adc_channels_data(struct udevice *dev, uint32_t channel_mask,
		struct adc_channel *channels)
{
	int32_t ch, ret;

	if (channels == NULL) {
		return -ENOMEM;
	}

	for (ch = ADC_CHANNEL_START; ch <= ADC_CHANNEL_END; ch++) {
		if (((ADC_CHANNEL((uint32_t)ch)) & channel_mask) != 0U) {
			channels[ch].id = ch;
			ret = tcc_adc_start_channel(dev, ch);
			if (ret < 0) {
				return ret;
			}
			ret = tcc_adc_channel_data(dev, ch, &channels[ch].data);
			if (ret < 0) {
				return ret;
			}
		}
	}

	return 0;
}

int32_t tcc_adc_stop(struct udevice *dev)
{
	struct tcc_adc *adc = dev_get_priv(dev);

	tcc_adc_power_control(adc, false);

	return 0;
}

int32_t tcc_adc_probe(struct udevice *dev)
{
	struct tcc_adc *adc = dev_get_priv(dev);

	if (adc == NULL) {
		pr_err("[ERROR][ADC] %s: tcc_adc is null!\n", __func__);
		return -ENOMEM;
	}

	/* base, clock control unit address */
	adc->regs = (void *)dev_read_addr_index(dev, 0);
	adc->clk_regs = (void *)dev_read_addr_index(dev, 1);

	/* enable adc peri clk */
	tcc_adc_power_control(adc, true);

	/* (1 / (ADC_CKIN_CLOCK / 1 cycles)) (us) */
	adc->adc_cycle_time = (1U / (ADC_CKIN_CLOCK / 1000000U));

	return 0;
}

int32_t tcc_adc_ofdata_to_platdata(struct udevice *dev)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);

	uc_pdata->data_mask = ADCDATA_MASK;

	uc_pdata->channel_mask = (1U << TCC_ADC_CHANNELS) - 1U;

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
	{ }
};

U_BOOT_DRIVER(tcc_adc) = {
	.name		= "tcc-adc",
	.id		= UCLASS_ADC,
	.of_match	= tcc_adc_ids,
	.ops		= &tcc_adc_ops,
	.probe		= tcc_adc_probe,
	.priv_auto_alloc_size = sizeof(struct tcc_adc),
	.ofdata_to_platdata = tcc_adc_ofdata_to_platdata,
};
