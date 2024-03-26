// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/vioc/vioc_fifo.h>
#include <asm/arch/vioc/vioc_global.h>
#include <asm/arch/vioc/reg_physical.h>
#include <telechips/fb_dm.h>

#define NOP __asm("NOP")

void VIOC_FIFO_ConfigEntry(void __iomem *reg, unsigned int *buf)
{
	unsigned int E_EMPTY = 1U; // emergency empty
	unsigned int E_FULL = 1U;  // emergency full
	unsigned int WMT = 0U;    // wdma mode - time
	unsigned int NENTRY =
		4U;	    // frame memory number  ->  max. frame count is 4.
	unsigned int RMT = 0U; // rdma mode - time
	unsigned int idxBuf;
	u32 val;
	unsigned int *tmp_pFIFO = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pFIFO = buf;
	buf = tmp_pFIFO;

	for (idxBuf = 0U; idxBuf < NENTRY; idxBuf++) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(buf[idxBuf], reg + (CH0_BASE + (idxBuf * 0x04U)));
	}

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + CH0_CTRL0)
	       & ~(CH_CTRL_EEMPTY_MASK | CH_CTRL_EFULL_MASK | CH_CTRL_WMT_MASK
		   | CH_CTRL_NENTRY_MASK | CH_CTRL_RMT_MASK));
	val |= (((E_EMPTY & 0x3U) << CH_CTRL_EEMPTY_SHIFT)
		| ((E_FULL & 0x3U) << CH_CTRL_EFULL_SHIFT)
		| ((WMT & 0x3U) << CH_CTRL_WMT_SHIFT)
		| ((NENTRY & 0x3FU) << CH_CTRL_NENTRY_SHIFT)
		| ((RMT & 0x1U) << CH_CTRL_RMT_SHIFT));
	__raw_writel(val, reg + CH0_CTRL0);
}

void VIOC_FIFO_SetEmergency(void __iomem *reg, unsigned int ch,
		unsigned int eempty, unsigned int efull)
{
	u32 val;

	if (ch > 1U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][FIFO] %s parameter is wrong ch(%u)\n",
				__func__,ch);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + (0x10U * ch) + CH0_CTRL0) &
			~(CH_CTRL_EEMPTY_MASK | CH_CTRL_EFULL_MASK));
		val |= (((eempty & 0x3U) << CH_CTRL_EEMPTY_SHIFT) |
				((efull & 0x3U) << CH_CTRL_EFULL_SHIFT));

		__raw_writel(val, reg + (0x10U * ch) + CH0_CTRL0);
	}
}

void VIOC_FIFO_SetRMT(void __iomem *reg, unsigned int ch, unsigned int rmt)
{
	u32 val;

	if (ch > 1U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][FIFO] %s parameter is wrong ch(%u)\n",
				__func__, ch);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + (0x10U * ch) + CH0_CTRL0) & ~(CH_CTRL_RMT_MASK));
		val |= ((rmt & 0x1U) << CH_CTRL_RMT_SHIFT);

		__raw_writel(val, reg + (0x10U * ch) + CH0_CTRL0);
	}
}

void VIOC_FIFO_SetWMT(void __iomem *reg, unsigned int ch, unsigned int wmt)
{
	u32 val;

	if (ch > 1U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][FIFO] %s parameter is wrong ch(%u)\n",
				__func__, ch);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + (0x10U * ch)+ CH0_CTRL0) & ~(CH_CTRL_WMT_MASK));
		val |= ((wmt & 0x3U) << CH_CTRL_WMT_SHIFT);

		__raw_writel(val, reg + (0x10U * ch) + CH0_CTRL0);
	}
}

void VIOC_FIFO_SetNENTRY(void __iomem *reg, unsigned int ch,
		unsigned int nentry)
{
	u32 val;

	if (ch > 1U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][FIFO] %s parameter is wrong ch(%u)\n",
				__func__, ch);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + (0x10U * ch) + CH0_CTRL0) & ~(CH_CTRL_NENTRY_MASK));
		val |= ((nentry & 0x3FU) << CH_CTRL_NENTRY_SHIFT);

		__raw_writel(val, reg + (0x10U * ch) + CH0_CTRL0);
	}
}

void VIOC_FIFO_ConfigDMA(void __iomem *reg, unsigned int ch, unsigned int nWDMA,
		unsigned int nRDMA0, unsigned int nRDMA1,
		unsigned int nRDMA2, unsigned int fRDMA)
{
	u32 val;

	if (ch > 1U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][FIFO] %s parameter is wrong ch(%u)\n",
				__func__, ch);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + (0x10U * ch) + CH0_CTRL1) &
			~(CH_CTRL1_FRDMA_MASK | CH_CTRL1_RDMA2SEL_MASK |
			CH_CTRL1_RDMA1SEL_MASK | CH_CTRL1_RDMA0SEL_MASK |
			CH_CTRL1_WDMASEL_MASK));
		val |= (((fRDMA & 0x3U) << CH_CTRL1_FRDMA_SHIFT) |
			((nRDMA2 & 0xFU) << CH_CTRL1_RDMA2SEL_SHIFT) |
			((nRDMA1 & 0xFU) << CH_CTRL1_RDMA1SEL_SHIFT) |
			((nRDMA0 & 0xFU) << CH_CTRL1_RDMA0SEL_SHIFT) |
			((nWDMA & 0xFU) << CH_CTRL1_WDMASEL_SHIFT));
		__raw_writel(val, reg + (0x10U * ch) + CH0_CTRL1);
	}
}

void VIOC_FIFO_SetEnable(void __iomem *reg, unsigned int nWDMA,
		unsigned int nRDMA0, unsigned int nRDMA1, unsigned int nRDMA2)
{
	u32 val;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	val = (__raw_readl(reg + CH0_CTRL0) &
	       ~(CH_CTRL_REN2_MASK | CH_CTRL_REN1_MASK |
	       CH_CTRL_REN0_MASK | CH_CTRL_WEN_MASK));
	val |= (((nRDMA2 & 0x1U) << CH_CTRL_REN2_SHIFT) |
		((nRDMA1 & 0x1U) << CH_CTRL_REN1_SHIFT) |
		((nRDMA0 & 0x1U) << CH_CTRL_REN0_SHIFT) |
		((nWDMA & 0x1U) << CH_CTRL_WEN_SHIFT));
	__raw_writel(val, reg + CH0_CTRL0);
}

void VIOC_FIFO_SetBase(void __iomem *reg, unsigned int ch,
		unsigned int baseaddr)
{
	unsigned int val;

	if (ch > 1U) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][FIFO] %s parameter is wrong ch(%u)\n",
				__func__, ch);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = (__raw_readl(reg + (0x40U * ch) + CH0_BASE) & CH_BASE_BASE_MASK);
		val |= ((baseaddr & 0xFFFFU) << CH_BASE_BASE_SHIFT);

		__raw_writel(val, reg + (0x40U * ch) + CH0_BASE);
	}
}

unsigned int VIOC_FIFO_GetStatus(void __iomem *reg)
{
	void *tmp_pFIFO = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pFIFO = reg;
	reg = tmp_pFIFO;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	return __raw_readl(reg + CH0_FIFOSTAT);
}

void __iomem *VIOC_FIFO_GetAddress(unsigned int vioc_id)
{
	/* avoid MISRA C-2012 Rule 2.7 */
	(void)vioc_id;

	return phys_to_virt(HwVIOC_FIFO);
}
