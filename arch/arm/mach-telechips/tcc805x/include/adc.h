/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_ADC_H
#define TCC_ADC_H
#include <adc.h>

#define TCC_ADC_CORE_NUM	1
#define TCC_ADC_DAT_ERR		0xFFFFU

/* TSADC Clock Setting */
#define ADC_PERI_CLOCK	24000000U	// 24 MHz
#define ADC_CKIN_CLOCK	2000000U	// 2 MHz

#define ADC_CONV_CYCLE	21

/*
 * TCC R5 ADC Register
 */
#define ADCCMD			0x00UL
#define ADCIRQ			0x04UL
#define ADCCTRL			0x08UL
#define ADCDATA0		0x80UL
#define PCLK_CFG_ADC_CTRL	0x74UL

#define ADCCMD_DONE		BIT(31)
#define ADCCMD_SMP_CMD(x)	BIT(x)

#define ADCDATA_MASK		0xFFFUL

#define ADC_CLK_OUT_EN		BIT(30)
#define ADC_CLK_DIV_EN		BIT(29)

/* ADC Channel */
#define ADC_CHANNEL_START	0
#define ADC_CHANNEL_END		11
#define TCC_ADC_CHANNELS	12U

#endif /* TCC_ADC_H */
