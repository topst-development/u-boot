// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <tcc_fb.h>
#include <asm/io.h>
#include <mach/clock.h>
#include <asm/telechips/tcc_ckc.h>
#include <asm/telechips/vioc/vioc_timer.h>
#include <asm/telechips/vioc/vioc_global.h>
#include <asm/telechips/vioc/reg_physical.h>

void VIOC_TIMER_SetUsecEnable(void __iomem *reg, int en, unsigned int utime)
{
	unsigned int xin_mhz;
	u32 value;

	//pr_info("%s XIN_CLK_RATE is %u\r\n", XIN_CLK_RATE);
	xin_mhz = ((((u32)XIN_CLK_RATE + 1000000U) / 1000000U) - 1U);

	utime &= (USEC_UINTDIV_MASK >> USEC_UINTDIV_SHIFT);
	if (utime == 0U) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto out;
	}

	/* UNITDIV */
	if (en == 1) {
		(void)tcc_set_peri(PERI_LCDTIMER, CKC_ENABLE, XIN_CLK_RATE, 0);
		value = ((u32)1U << USEC_EN_SHIFT);
		value |= ((utime - 1U) << USEC_UINTDIV_SHIFT);
		value |= (xin_mhz & 0xFFFFU);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = __raw_readl(reg + CURTIME);
		value &= ~USEC_EN_MASK;
		(void)tcc_set_peri(PERI_LCDTIMER, CKC_DISABLE, XIN_CLK_RATE, 0);
	}

	if (reg == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_TIMER] %s reg is NULL\r\n", __func__);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, reg + USEC);
	}
out:
	return;
}

unsigned int VIOC_TIMER_GetCurTime(void __iomem *reg)
{
	u32 value = 0;
	void *tmp_pTIMER = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pTIMER = reg;
	reg = tmp_pTIMER;

	if (reg != NULL) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = __raw_readl(reg + CURTIME);
	}
	return value;
}

void VIOC_TIMER_SetTimer(void __iomem *reg,
						enum VIOC_TIMER_IRQ_ID id,
						int en, unsigned int count)
{
	unsigned long timer_offset;
	u32 value;
	int ret = -1;

	switch (id) {
	case VIOC_TIMER_IRQ_TIMER0:
		timer_offset = TIMER0;
		ret = 0;
		break;
	case VIOC_TIMER_IRQ_TIMER1:
		timer_offset = TIMER1;
		ret = 0;
		break;
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][TIMER] %s id(%u) is wrong\n", __func__, (unsigned int)id);
		ret = -1;
		break;
	}
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	count &= (TIMER_COUNTER_MASK >> TIMER_COUNTER_SHIFT);

	if (en == 1) {
		value = ((u32)1U << TIMER_EN_SHIFT);
		value |= (count << TIMER_COUNTER_SHIFT);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = __raw_readl(reg + timer_offset);
		value &= ~TIMER_COUNTER_MASK;
	}

	if (reg == NULL) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_TIMER] %s reg is NULL\r\n", __func__);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, reg + timer_offset);
	}

FUNC_EXIT:
	return;
}

void VIOC_TIMER_SetTiReq(void __iomem *reg,
					enum VIOC_TIMER_IRQ_ID id,
					unsigned int en, unsigned int time_unit)
{
	u32 value;
	unsigned long tireq_offset;
	int ret = -1;

	switch (id) {
	case VIOC_TIMER_IRQ_TIREQ0:
		tireq_offset = TIREQ0;
		ret = 0;
		break;
	case VIOC_TIMER_IRQ_TIREQ1:
		tireq_offset = TIREQ1;
		ret = 0;
		break;
	default:
		ret = -1;
		break;
	}
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	time_unit &= (TIREQ_TIME_MASK >>  TIREQ_TIME_SHIFT);
	if (en == 1U) {
		value = ((u32)1U << TIREQ_EN_SHIFT);
		value |= (time_unit << TIREQ_TIME_SHIFT);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = __raw_readl(reg + tireq_offset);
		value &= ~TIREQ_EN_MASK;
	}
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(value, reg + tireq_offset);

FUNC_EXIT:
	return;
}

void VIOC_TIMER_SetIrqMask(void __iomem *reg,
					enum VIOC_TIMER_IRQ_ID id,
					unsigned int mask)
{
	u32 value;

	if (id > VIOC_TIMER_IRQ_TIMER1) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][TIMER] %s\n", __func__);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = __raw_readl(reg + IRQMASK);
		value &= ~((u32)1U << (unsigned int)id);
		value |= (((mask != 0U) ? (u32)1U : (u32)0U) << (unsigned int)id);

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, reg + IRQMASK);
	}
}

unsigned long VIOC_TIMER_GetIrqStatus(void __iomem *reg)
{
	u32 value;
	void *tmp_pTIMER = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pTIMER = reg;
	reg = tmp_pTIMER;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	value = __raw_readl(reg + IRQSTAT);
	return value;
}

void VIOC_TIMER_ClearIrqStatus(void __iomem *reg,
					unsigned int mask)
{
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(mask, reg + IRQSTAT);
}

void __iomem *VIOC_TIMER_GetAddress(void)
{
	return phys_to_virt(HwVIOC_TIMER);
}
/* EOF */
