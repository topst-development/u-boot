// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <irq.h>
#include <irq_func.h>

#include <asm/arch/vioc/vioc_rdma.h>
#include <asm/arch/vioc/vioc_intr.h>
#include <asm/arch/vioc/vioc_config.h>
#include <asm/arch/vioc/vioc_rdma.h>
#include <asm/arch/vioc/vioc_wdma.h>
#include <asm/arch/vioc/vioc_disp.h>
#include <asm/arch/vioc/vioc_vin.h>
#include <asm/arch/vioc/vioc_global.h>

static int vioc_base_irq_num[4] = {0,};

struct vioc_irq_handler {
	int m_irq;
	void *m_data;
	void (*m_func)(void *data);
};

#if defined(CONFIG_TCC_APPB_VERIFICATION)
static struct vioc_irq_handler VIOC_IRQ_HANDLER[VIOC_INTR_NUM];
		// __attribute__ ((section(".data")));
static int vioc_irq_installed[4] = {0,};

static void tcc_vioc_irq_handler(void *data)
{
	int i;
	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	long vioc_irq = (long)data;
	void *tmp_pINTR = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pINTR = data;
	data= tmp_pINTR;

	for (i = 0; i < VIOC_INTR_NUM; i++) {
		if ((VIOC_IRQ_HANDLER[i].m_func != NULL) &&
			(VIOC_IRQ_HANDLER[i].m_irq == vioc_irq)) {
			/* Prevent KCS warning */
			VIOC_IRQ_HANDLER[i].m_func(VIOC_IRQ_HANDLER[i].m_data);
		}
	}
}

int tcc_vioc_irq_install_handler(long vioc_irq, int id,
					void *handle_irq, void *data)
{
	int i;
	int irq_used = 0;
	int ret = -1;

	if (vioc_irq < 0) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_INTR] %s: vioc_irq(%lx) is wrong.\n",
				__func__, vioc_irq);
		ret = -1;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	do {
		if (((unsigned int)vioc_irq < INT_VIOC0)
			|| ((unsigned int)vioc_irq > INT_VIOC3)
			|| (handle_irq == NULL)) {
			/* prevent KCS warning */
			break;
		}

		if ((id >= 0) && (id < (int)VIOC_INTR_NUM)) {
			VIOC_IRQ_HANDLER[id].m_irq = vioc_irq;
			VIOC_IRQ_HANDLER[id].m_data = data;
			VIOC_IRQ_HANDLER[id].m_func = handle_irq;
			ret = 0;
		}

		for (i = 0; i < VIOC_INTR_NUM; i++) {
			if ((VIOC_IRQ_HANDLER[i].m_func != NULL)
			    && (VIOC_IRQ_HANDLER[i].m_irq == vioc_irq)) {
				if (irq_used < VIOC_INTR_NUM) {
					irq_used++;
				}
			}
		}
		if (vioc_irq_installed[(unsigned int)vioc_irq-INT_VIOC0] == 0) {
			//pr_info("[DBG][IRQ] install
			//vioc interrupt handler for irq<%d>\r\n", irq);
			irq_install_handler(vioc_irq,
				(interrupt_handler_t *)tcc_vioc_irq_handler, (void *)vioc_irq);
			irq_unmask(vioc_irq);
			vioc_irq_installed[(unsigned int)vioc_irq-INT_VIOC0] = 1;
		}
	} while ((bool)0);
FUNC_EXIT:
	return ret;
}

void tcc_vioc_irq_free_handler(int vioc_irq, unsigned int id)
{
	int i;
	int irq_used = 0;

	if (vioc_irq < 0) {
		pr_err("[ERR][VIOC_INTR] %s: vioc_irq(%d) is wrong.\n",
				__func__, vioc_irq);
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	if (((unsigned int)vioc_irq < INT_VIOC0) || ((unsigned int)vioc_irq > INT_VIOC3)) {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_INTR] %s: vioc_irq(%d) is wrong.\n",
				__func__, vioc_irq);
	} else {
		VIOC_IRQ_HANDLER[id].m_irq = -1;
		VIOC_IRQ_HANDLER[id].m_data = NULL;
		VIOC_IRQ_HANDLER[id].m_func = NULL;

		for (i = 0; i < VIOC_INTR_NUM; i++) {
			if ((VIOC_IRQ_HANDLER[i].m_func != NULL)
			    && (VIOC_IRQ_HANDLER[i].m_irq == vioc_irq)) {
				/* avoid CERT-C Integers Rule INT32-C */
				if (irq_used < VIOC_INTR_NUM) {
					irq_used++;
				}
			}
		}
		if (irq_used == 0) {
			//pr_info("[DBG][IRQ] free vioc
			//interrupt handler for irq<%d>\r\n", irq);
			irq_mask(vioc_irq);
			irq_free_handler(vioc_irq);
			vioc_irq_installed[(unsigned int)vioc_irq-INT_VIOC0] = 0;
		}
	}

FUNC_EXIT:
	return;
}
#endif

int vioc_intr_enable(int vioc_irq, int id, unsigned int mask)
{
	void __iomem *reg;
	int sub_id;
	unsigned int type_clr_offset;
	int ret = -1;
	u32 val = 0U;

	if ((id < 0) || (id > VIOC_INTR_NUM)) {
		ret = -1;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	switch (id) {
	case VIOC_INTR_DEV0:
	case VIOC_INTR_DEV1:
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_DEV2:
#ifdef CONFIG_TCC805X
	case VIOC_INTR_DEV3:
		if (id == VIOC_INTR_DEV3) {
			/* Prevent KCS warning */
			sub_id = id - VIOC_INTR_DISP_OFFSET - VIOC_INTR_DEV0;
		} else {
			/* Prevent KCS warning */
			sub_id = id - VIOC_INTR_DEV0;
		}

		reg = VIOC_DISP_GetAddress((unsigned int)sub_id);
		if (reg == NULL) {
			ret = -1;
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg + DIM);
		__raw_writel((val &
			      ~(mask & VIOC_DISP_INT_MASK)),
			     reg + DIM);
		ret = 0;
		break;
#endif
#endif
#if !defined(CONFIG_TCC805X)
		sub_id = id - VIOC_INTR_DEV0;

		reg = VIOC_DISP_GetAddress((unsigned int)sub_id);
		if (reg == NULL) {
			ret = -1;
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg + DIM);
		__raw_writel((val &
			      ~(mask & VIOC_DISP_INT_MASK)),
			     reg + DIM);
		ret = 0;
		break;
#endif
#if defined(CONFIG_VIOC_DOLBY_VISION_EDR)
	case VIOC_INTR_V_DV:
	{
		void __iomem *pDV_Cfg =
			VIOC_DV_VEDR_GetAddress(VDV_CFG);
		VIOC_V_DV_SetInterruptEnable(pDV_Cfg,
					     (mask & VIOC_V_DV_INT_MASK), 1);
		//dprintk_dv_sequence("### V_DV INT On\n");
	}
	ret = 0;
	break;
#endif
	case VIOC_INTR_RD0:
	case VIOC_INTR_RD1:
	case VIOC_INTR_RD2:
	case VIOC_INTR_RD3:
	case VIOC_INTR_RD4:
	case VIOC_INTR_RD5:
	case VIOC_INTR_RD6:
	case VIOC_INTR_RD7:
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_RD8:
	case VIOC_INTR_RD9:
	case VIOC_INTR_RD10:
	case VIOC_INTR_RD11:
#endif
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_RD12:
	case VIOC_INTR_RD13:
#endif
		/* clera irq status */
		sub_id = id - VIOC_INTR_RD0;
		reg = VIOC_RDMA_GetAddress((unsigned int)sub_id) + RDMASTAT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((mask & VIOC_RDMA_INT_MASK), reg);

		/* enable irq */
		reg = VIOC_RDMA_GetAddress((unsigned int)sub_id) + RDMAIRQMSK;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val & ~(mask & VIOC_RDMA_INT_MASK), reg);
		ret = 0;
		break;
	case VIOC_INTR_WD0:
	case VIOC_INTR_WD1:
	case VIOC_INTR_WD2:
	case VIOC_INTR_WD3:
	case VIOC_INTR_WD4:
	case VIOC_INTR_WD5:
	case VIOC_INTR_WD6:
	case VIOC_INTR_WD7:
	case VIOC_INTR_WD8:
		sub_id = id - VIOC_INTR_WD0;

		/* clera irq status */
		reg = VIOC_WDMA_GetAddress((unsigned int)sub_id) +
		      WDMAIRQSTS_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((mask & VIOC_WDMA_INT_MASK), reg);

		/* enable irq */
		reg = VIOC_WDMA_GetAddress((unsigned int)sub_id) +
		      WDMAIRQMSK_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val & ~(mask & VIOC_WDMA_INT_MASK), reg);
		ret = 0;
		break;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_WD9:
	case VIOC_INTR_WD10:
	case VIOC_INTR_WD11:
	case VIOC_INTR_WD12:
		sub_id = id - VIOC_INTR_WD_OFFSET - VIOC_INTR_WD0;

		/* clera irq status */
		reg = VIOC_WDMA_GetAddress((unsigned int)sub_id) +
				WDMAIRQSTS_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((mask & VIOC_WDMA_INT_MASK), reg);

		/* enable irq */
		reg = VIOC_WDMA_GetAddress((unsigned int)sub_id) +
				WDMAIRQMSK_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val & ~(mask & VIOC_WDMA_INT_MASK), reg);
		ret = 0;
		break;
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_WD13:
		sub_id = id - VIOC_INTR_WD_OFFSET -
			VIOC_INTR_WD_OFFSET2 - VIOC_INTR_WD0;

		/* clera irq status */
		reg = VIOC_WDMA_GetAddress((unsigned int)sub_id) +
				WDMAIRQSTS_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((mask & VIOC_WDMA_INT_MASK), reg);

		/* enable irq */
		reg = VIOC_WDMA_GetAddress((unsigned int)sub_id) +
				WDMAIRQMSK_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val & ~(mask & VIOC_WDMA_INT_MASK), reg);
		ret = 0;
		break;
#endif

#endif
	/*
	 *	VIN_INT[31]: Not Used
	 *	VIN_INT[19]: Enable interrupt if 1 / Disable interrupt if 0
	 *	VIN_INT[18]: Enable interrupt if 1 / Disable interrupt if 0
	 *	VIN_INT[17]: Enable interrupt if 1 / Disable interrupt if 0
	 *	VIN_INT[16]: Enable interrupt if 1 / Disable interrupt if 0
	 */
	case VIOC_INTR_VIN0:
	case VIOC_INTR_VIN1:
	case VIOC_INTR_VIN2:
	case VIOC_INTR_VIN3:
		sub_id = id - VIOC_INTR_VIN0;

		reg = VIOC_VIN_GetAddress((unsigned int)sub_id * 2U) + VIN_INT;
		/* clera irq status */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel((val | (mask & VIOC_VIN_INT_MASK)), reg);

		/* enable irq */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val | ((mask & VIOC_VIN_INT_MASK) << 16U), reg);
		ret = 0;
		break;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_VIN4:
	case VIOC_INTR_VIN5:
	case VIOC_INTR_VIN6:
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_VIN7:
#endif//defined(CONFIG_TCC805X)
		sub_id = id - VIOC_INTR_VIN_OFFSET - VIOC_INTR_VIN0;

		reg = VIOC_VIN_GetAddress((unsigned int)sub_id * 2U) + VIN_INT;

		/* clera irq status */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel((val | (mask & VIOC_VIN_INT_MASK)), reg);

		/* enable irq */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val | ((mask & VIOC_VIN_INT_MASK) << 16U), reg);
		ret = 0;
		break;
#endif//defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_INTR] %s: id(%d) is wrong.\n",
		       __func__, id);
		break;
	}

#if defined(CONFIG_TCC899X) || defined(CONFIG_TCC803X) || \
defined(CONFIG_TCC901X) || defined(CONFIG_TCC805X)
	if (vioc_irq == vioc_base_irq_num[0]) {
		/* Prevnet KCS warning */
		type_clr_offset = IRQMASKCLR0_0_OFFSET;
	} else if (vioc_irq == vioc_base_irq_num[1]) {
		/* Prevnet KCS warning */
		type_clr_offset = IRQMASKCLR1_0_OFFSET;
	} else if (vioc_irq == vioc_base_irq_num[2]) {
		/* Prevnet KCS warning */
		type_clr_offset = IRQMASKCLR2_0_OFFSET;
	} else if (vioc_irq == vioc_base_irq_num[3]) {
		/* Prevnet KCS warning */
		type_clr_offset = IRQMASKCLR3_0_OFFSET;
	} else {
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_INTR] %s-%d :: vioc_irq(%d) is wierd.\n",
			__func__, __LINE__, vioc_irq);
		ret = -1;
	}
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}
#else
	type_clr_offset = IRQMASKCLR0_0_OFFSET;
#endif

	reg = VIOC_IREQConfig_GetAddress();
#if defined(CONFIG_TCC897X)
	if (id < 32) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(1U << id, reg + type_clr_offset);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(1U << (id - 32), reg + type_clr_offset + 0x4);
	}
#else
	if (id >= 64) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((u32)1U << ((unsigned int)id - 64U), reg + type_clr_offset + 0x8U);
	} else if (id >= 32) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((u32)1U << ((unsigned int)id - 32U), reg + type_clr_offset + 0x4U);
	} else {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((u32)1U << (unsigned int)id, reg + type_clr_offset);
	}
#endif
	ret = 0;

FUNC_EXIT:
	return ret;

}

int vioc_intr_disable(int vioc_irq, int id, unsigned int mask)
{
	void __iomem *reg;
	int sub_id;
	unsigned int do_irq_mask = 1U;
	unsigned int type_set_offset;
	int ret = -1;
	u32 val = 0U;

	if ((id < 0) || (id > VIOC_INTR_NUM)) {
		ret = -1;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	switch (id) {
	case VIOC_INTR_DEV0:
	case VIOC_INTR_DEV1:
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_DEV2:
#ifdef CONFIG_TCC805X
	case VIOC_INTR_DEV3:
		if (id == VIOC_INTR_DEV3) {
		/* Prevent KCS warning */
			sub_id = id - VIOC_INTR_DISP_OFFSET - VIOC_INTR_DEV0;
		} else {
			/* Prevent KCS warning */
			sub_id = id - VIOC_INTR_DEV0;
		}

		reg = VIOC_DISP_GetAddress((unsigned int)sub_id) + DIM;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val | (mask & VIOC_DISP_INT_MASK), reg);
		if ((__raw_readl(reg) & VIOC_DISP_INT_MASK) != VIOC_DISP_INT_MASK) {
			/* Prevent KCS warning */
			do_irq_mask = 0;
		}
		ret = 0;
		break;
#endif
#endif
#if !defined(CONFIG_TCC805X)
		sub_id = id - VIOC_INTR_DEV0;

		reg = VIOC_DISP_GetAddress((unsigned int)sub_id) + DIM;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val | (mask & VIOC_DISP_INT_MASK), reg);
		if ((__raw_readl(reg) & VIOC_DISP_INT_MASK) != VIOC_DISP_INT_MASK) {
			/* Prevent KCS warning */
			do_irq_mask = 0;
		}
		ret = 0;
		break;
#endif
#if defined(CONFIG_VIOC_DOLBY_VISION_EDR)
	case VIOC_INTR_V_DV: {
		void __iomem *pDV_Cfg =
			VIOC_DV_VEDR_GetAddress(VDV_CFG);
		VIOC_V_DV_SetInterruptEnable(pDV_Cfg,
					     (mask & VIOC_V_DV_INT_MASK), 0);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(pDV_Cfg) & (mask & VIOC_V_DV_INT_MASK)) == 0U) {
			/* Prevent KCS warning */
			do_irq_mask = 0;
		}
		//dprintk_dv_sequence("### V_DV INT Off\n");
	}
	ret = 0;
	break;
#endif
	case VIOC_INTR_RD0:
	case VIOC_INTR_RD1:
	case VIOC_INTR_RD2:
	case VIOC_INTR_RD3:
	case VIOC_INTR_RD4:
	case VIOC_INTR_RD5:
	case VIOC_INTR_RD6:
	case VIOC_INTR_RD7:
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_RD8:
	case VIOC_INTR_RD9:
	case VIOC_INTR_RD10:
	case VIOC_INTR_RD11:
#endif
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_RD12:
	case VIOC_INTR_RD13:
#endif
		sub_id = id - VIOC_INTR_RD0;
		reg = VIOC_RDMA_GetAddress((unsigned int)sub_id) + RDMAIRQMSK;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val | (mask & VIOC_RDMA_INT_MASK), reg);
		if ((__raw_readl(reg) & VIOC_RDMA_INT_MASK) != VIOC_RDMA_INT_MASK) {
			/* Prevent KCS warning */
			do_irq_mask = 0;
		}
		ret = 0;
		break;
	case VIOC_INTR_WD0:
	case VIOC_INTR_WD1:
	case VIOC_INTR_WD2:
	case VIOC_INTR_WD3:
	case VIOC_INTR_WD4:
	case VIOC_INTR_WD5:
	case VIOC_INTR_WD6:
	case VIOC_INTR_WD7:
	case VIOC_INTR_WD8:
		sub_id = id - VIOC_INTR_WD0;
		reg = VIOC_WDMA_GetAddress((unsigned int)sub_id) + WDMAIRQMSK_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val | (mask & VIOC_WDMA_INT_MASK), reg);
		if ((__raw_readl(reg) & VIOC_WDMA_INT_MASK) != VIOC_WDMA_INT_MASK) {
			/* Prevent KCS warning */
			do_irq_mask = 0;
		}
		ret = 0;
		break;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_WD9:
	case VIOC_INTR_WD10:
	case VIOC_INTR_WD11:
	case VIOC_INTR_WD12:
		sub_id = id - VIOC_INTR_WD_OFFSET - VIOC_INTR_WD0;
		reg = VIOC_WDMA_GetAddress((unsigned int)sub_id) + WDMAIRQMSK_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val | (mask & VIOC_WDMA_INT_MASK), reg);
		if ((__raw_readl(reg) & VIOC_WDMA_INT_MASK) != VIOC_WDMA_INT_MASK) {
			/* Prevent KCS warning */
			do_irq_mask = 0;
		}
		ret = 0;
		break;
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_WD13:
		sub_id = id - VIOC_INTR_WD_OFFSET - VIOC_INTR_WD_OFFSET2
			- VIOC_INTR_WD0;
		reg = VIOC_WDMA_GetAddress((unsigned int)sub_id) + WDMAIRQMSK_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val | (mask & VIOC_WDMA_INT_MASK), reg);
		if ((__raw_readl(reg) & VIOC_WDMA_INT_MASK) != VIOC_WDMA_INT_MASK) {
			/* Prevent KCS warning */
			do_irq_mask = 0;
		}
		ret = 0;
		break;
#endif
#endif
	/*
	 *	VIN_INT[31]: Not Used
	 *	VIN_INT[19]: Enable interrupt if 1 / Disable interrupt if 0
	 *	VIN_INT[18]: Enable interrupt if 1 / Disable interrupt if 0
	 *	VIN_INT[17]: Enable interrupt if 1 / Disable interrupt if 0
	 *	VIN_INT[16]: Enable interrupt if 1 / Disable interrupt if 0
	 */
	case VIOC_INTR_VIN0:
	case VIOC_INTR_VIN1:
	case VIOC_INTR_VIN2:
	case VIOC_INTR_VIN3:
		sub_id = id - VIOC_INTR_VIN0;
		reg = VIOC_VIN_GetAddress((unsigned int)sub_id * 2U) + VIN_INT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val & ~((mask & VIOC_VIN_INT_MASK) << 16U), reg);
//		if ((readl(reg) & VIOC_VIN_INT_MASK) != VIOC_VIN_INT_MASK) {
//			do_irq_mask = 0;
//		}
		ret = 0;
		break;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_VIN4:
	case VIOC_INTR_VIN5:
	case VIOC_INTR_VIN6:
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_VIN7:
#endif //defined(CONFIG_TCC805X)
		sub_id = id - VIOC_INTR_VIN_OFFSET - VIOC_INTR_VIN0;
		reg = VIOC_VIN_GetAddress((unsigned int)sub_id * 2U) + VIN_INT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel(val & ~((mask & VIOC_VIN_INT_MASK) << 16U), reg);
//		if ((readl(reg) & VIOC_VIN_INT_MASK) != VIOC_VIN_INT_MASK) {
//			do_irq_mask = 0;
//		}
		ret = 0;
		break;
#endif//defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_INTR] %s: id(%d) is wrong.\n",
		       __func__, id);
		ret = 0;
		break;
	}

	if (do_irq_mask == 1U) {
#if defined(CONFIG_TCC899X) || defined(CONFIG_TCC803X) || \
defined(CONFIG_TCC901X) || defined(CONFIG_TCC805X)
		if (vioc_irq == vioc_base_irq_num[0]) {
			/* Prevent KCS warning */
			type_set_offset = IRQMASKSET0_0_OFFSET;
		} else if (vioc_irq == vioc_base_irq_num[1]) {
			/* Prevent KCS warning */
			type_set_offset = IRQMASKSET1_0_OFFSET;
		} else if (vioc_irq == vioc_base_irq_num[2]) {
			/* Prevent KCS warning */
			type_set_offset = IRQMASKSET2_0_OFFSET;
		}  else if (vioc_irq == vioc_base_irq_num[3]) {
			/* Prevent KCS warning */
			type_set_offset = IRQMASKSET3_0_OFFSET;
		} else {
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			pr_err("[ERR][VIOC_INTR] %s-%d :: vioc_irq(%d) is wierd.\n",
				__func__, __LINE__, vioc_irq);
			ret = -1;
		}
		if (ret < 0) {
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
#else
		type_set_offset = IRQMASKSET0_0_OFFSET;
#endif

		reg = VIOC_IREQConfig_GetAddress();
#if defined(CONFIG_TCC897X)
		if (id < 32) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			__raw_writel((u32)1U << id, reg + type_set_offset);
		} else {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			__raw_writel((u32)1U << ((unsigned int)id - 32U), reg + type_set_offset + 0x4U);
		}
#else
		if (id >= 64) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			__raw_writel((u32)1U << ((unsigned int)id - 64U), reg + type_set_offset + 0x8U);
		} else if (id >= 32) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			__raw_writel((u32)1U << ((unsigned int)id - 32U), reg + type_set_offset + 0x4U);
		} else {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			__raw_writel((u32)1U << (unsigned int)id, reg + type_set_offset);
		}
#endif
	}
	ret = 0;

FUNC_EXIT:
	return ret;

}

unsigned int vioc_intr_get_status(int id)
{
	void __iomem *reg = NULL;
	unsigned int ret = 0U;
	void *tmp_pINTR = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pINTR = reg;
	reg = tmp_pINTR;

	if ((id < 0) || (id > VIOC_INTR_NUM)) {
		ret = 0U;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	switch (id) {
	case VIOC_INTR_DEV0:
	case VIOC_INTR_DEV1:
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_DEV2:
#ifdef CONFIG_TCC805X
	case VIOC_INTR_DEV3:
		if (id == VIOC_INTR_DEV3) {
			/* Prevent KCS warning */
			id -= VIOC_INTR_DISP_OFFSET - VIOC_INTR_DEV0;
		}
		reg = VIOC_DISP_GetAddress((unsigned int)id) + DSTATUS;

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		ret = (__raw_readl(reg) & VIOC_DISP_INT_MASK);
		break;
#endif
#endif
#if !defined(CONFIG_TCC805X)
		reg = VIOC_DISP_GetAddress((unsigned int)id) + DSTATUS;

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		ret = (__raw_readl(reg) & VIOC_DISP_INT_MASK);
		break;
#endif
	case VIOC_INTR_RD0:
	case VIOC_INTR_RD1:
	case VIOC_INTR_RD2:
	case VIOC_INTR_RD3:
	case VIOC_INTR_RD4:
	case VIOC_INTR_RD5:
	case VIOC_INTR_RD6:
	case VIOC_INTR_RD7:
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_RD8:
	case VIOC_INTR_RD9:
	case VIOC_INTR_RD10:
	case VIOC_INTR_RD11:
#endif
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_RD12:
	case VIOC_INTR_RD13:
#endif
		id -= VIOC_INTR_RD0;
		reg = VIOC_RDMA_GetAddress((unsigned int)id) + RDMASTAT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		ret = (__raw_readl(reg) & VIOC_RDMA_INT_MASK);
		break;
	case VIOC_INTR_WD0:
	case VIOC_INTR_WD1:
	case VIOC_INTR_WD2:
	case VIOC_INTR_WD3:
	case VIOC_INTR_WD4:
	case VIOC_INTR_WD5:
	case VIOC_INTR_WD6:
	case VIOC_INTR_WD7:
	case VIOC_INTR_WD8:
		id -= VIOC_INTR_WD0;
		reg = VIOC_WDMA_GetAddress((unsigned int)id) + WDMAIRQSTS_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		ret = (__raw_readl(reg) & VIOC_WDMA_INT_MASK);
		break;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_WD9:
	case VIOC_INTR_WD10:
	case VIOC_INTR_WD11:
	case VIOC_INTR_WD12:
		id -= (VIOC_INTR_WD_OFFSET + VIOC_INTR_WD0);
		reg = VIOC_WDMA_GetAddress((unsigned int)id) + WDMAIRQSTS_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		ret = (__raw_readl(reg) & VIOC_WDMA_INT_MASK);
		break;
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_WD13:
		id -= (VIOC_INTR_WD_OFFSET -
			VIOC_INTR_WD_OFFSET2 - VIOC_INTR_WD0);
		reg = VIOC_WDMA_GetAddress((unsigned int)id) + WDMAIRQSTS_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		ret = (__raw_readl(reg) & VIOC_WDMA_INT_MASK);
		break;
#endif
#endif
	case VIOC_INTR_VIN0:
	case VIOC_INTR_VIN1:
	case VIOC_INTR_VIN2:
	case VIOC_INTR_VIN3:
		id -= VIOC_INTR_VIN0;
		reg = VIOC_VIN_GetAddress((unsigned int)id * 2U) + VIN_INT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		ret = (__raw_readl(reg) & VIOC_VIN_INT_MASK);
		break;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_VIN4:
	case VIOC_INTR_VIN5:
	case VIOC_INTR_VIN6:
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_VIN7:
#endif//defined(CONFIG_TCC805X)
		id -= (VIOC_INTR_VIN_OFFSET + VIOC_INTR_VIN0);
		reg = VIOC_VIN_GetAddress((unsigned int)id * 2U) + VIN_INT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		ret = (__raw_readl(reg) & VIOC_VIN_INT_MASK);
		break;
#endif//defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_INTR] %s: id(%d) is wrong.\n",
		       __func__, id);
		ret = 0U;
		break;
	}

FUNC_EXIT:
	return ret;
}

bool check_vioc_irq_status(void __iomem *reg, int id)
{
	unsigned int flag;
	bool ret = false;
	void *tmp_pINTR = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pINTR = reg;
	reg = tmp_pINTR;

	if ((id < 0) || (id > VIOC_INTR_NUM)) {
		/* Prevent KCS warning */
		ret = false;
	} else {
		if (id < 32) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			flag = ((__raw_readl(reg + IRQSELECT0_0_OFFSET) & ((u32)1U << (unsigned int)id)) != 0U) ?
				(__raw_readl(reg + SYNCSTATUS0_OFFSET) & ((u32)1U << (unsigned int)id)) :
				(__raw_readl(reg + RAWSTATUS0_OFFSET) & ((u32)1U << (unsigned int)id));
		} else if (id < 64) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
			flag = ((__raw_readl(reg + IRQMASKCLR0_1_OFFSET)
				& ((u32)1U << ((unsigned int)id - 32U))) != 0U) ?
				(__raw_readl(reg + SYNCSTATUS1_OFFSET)
				& ((u32)1U << ((unsigned int)id - 32U))) :
				(__raw_readl(reg + RAWSTATUS1_OFFSET)
				& ((u32)1U << ((unsigned int)id - 32U)));
		}
#if !defined(CONFIG_TCC897X)
		else {
			flag = ((__raw_readl(reg + IRQMASKCLR0_2_OFFSET)
				& ((u32)1U << ((unsigned int)id - 64U))) != 0U) ?
				(__raw_readl(reg + SYNCSTATUS2_OFFSET)
				& ((u32)1U << ((unsigned int)id - 64U)))
				: (__raw_readl(reg + RAWSTATUS2_OFFSET)
				& ((u32)1U << ((unsigned int)id - 64U)));
		}
#endif
		if (flag != 0U) {
			/* Prevent KCS warning */
			ret = true;
		} else {
			/* Prevent KCS warning */
			ret = false;
		}
	}
	return ret;
}

bool is_vioc_intr_activatied(int id, unsigned int mask)
{
	void __iomem *reg = NULL;
	bool ret = false;
	void *tmp_pINTR = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pINTR = reg;
	reg = tmp_pINTR;

	if ((id < 0) || (id > VIOC_INTR_NUM)) {
		ret = false;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	switch (id) {
	case VIOC_INTR_DEV0:
	case VIOC_INTR_DEV1:
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_DEV2:
#ifdef CONFIG_TCC805X
	case VIOC_INTR_DEV3:
		if (id == VIOC_INTR_DEV3) {
			/* Prevent KCS warning */
			id -= VIOC_INTR_DISP_OFFSET - VIOC_INTR_DEV0;
		}
		reg = VIOC_DISP_GetAddress((unsigned int)id);
		if (reg == NULL) {
			ret = false;
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg + DSTATUS) & (mask & VIOC_DISP_INT_MASK)) != 0U) {
		    /* Prevent KCS warning */
			ret = true;
		} else {
			/* Prevent KCS warning */
			ret = false;
		}
		break;
#endif
#endif
#if !defined(CONFIG_TCC805X)
		reg = VIOC_DISP_GetAddress((unsigned int)id);
		if (reg == NULL) {
			ret = false;
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg + DSTATUS) & (mask & VIOC_DISP_INT_MASK)) != 0U) {
			/* Prevent KCS warning */
			ret = true;
		} else {
			/* Prevent KCS warning */
			ret = false;
		}
		break;
#endif
	case VIOC_INTR_RD0:
	case VIOC_INTR_RD1:
	case VIOC_INTR_RD2:
	case VIOC_INTR_RD3:
	case VIOC_INTR_RD4:
	case VIOC_INTR_RD5:
	case VIOC_INTR_RD6:
	case VIOC_INTR_RD7:
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_RD8:
	case VIOC_INTR_RD9:
	case VIOC_INTR_RD10:
	case VIOC_INTR_RD11:
#endif
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_RD12:
	case VIOC_INTR_RD13:
#endif
		id -= VIOC_INTR_RD0;
		reg = VIOC_RDMA_GetAddress((unsigned int)id) + RDMASTAT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg) & (mask & VIOC_RDMA_INT_MASK)) != 0U) {
			/* Prevent KCS warning */
			ret = true;
		} else {
			/* Prevent KCS warning */
			ret = false;
		}
		break;
	case VIOC_INTR_WD0:
	case VIOC_INTR_WD1:
	case VIOC_INTR_WD2:
	case VIOC_INTR_WD3:
	case VIOC_INTR_WD4:
	case VIOC_INTR_WD5:
	case VIOC_INTR_WD6:
	case VIOC_INTR_WD7:
	case VIOC_INTR_WD8:
		id -= VIOC_INTR_WD0;
		reg = VIOC_WDMA_GetAddress((unsigned int)id) + WDMAIRQSTS_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg) & (mask & VIOC_WDMA_INT_MASK)) != 0U) {
			/* Prevent KCS warning */
			ret = true;
		} else {
			/* Prevent KCS warning */
			ret = false;
		}
		break;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_WD9:
	case VIOC_INTR_WD10:
	case VIOC_INTR_WD11:
	case VIOC_INTR_WD12:
		id -= (VIOC_INTR_WD_OFFSET + VIOC_INTR_WD0);
		reg = VIOC_WDMA_GetAddress((unsigned int)id) + WDMAIRQSTS_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg) & (mask & VIOC_WDMA_INT_MASK)) != 0U) {
			/* Prevent KCS warning */
			ret = true;
		} else {
			ret = false;
		}
		break;
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_WD13:
		id -= (VIOC_INTR_WD_OFFSET - VIOC_INTR_WD_OFFSET2
		       - VIOC_INTR_WD0);
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		reg = VIOC_WDMA_GetAddress((unsigned int)id) + WDMAIRQSTS_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg) & (mask & VIOC_WDMA_INT_MASK)) != 0U) {
			/* Prevent KCS warning */
			ret = true;
		} else {
			/* Prevent KCS warning */
			ret = false;
		}
		break;
#endif
#endif
	case VIOC_INTR_VIN0:
	case VIOC_INTR_VIN1:
	case VIOC_INTR_VIN2:
	case VIOC_INTR_VIN3:
		id -= VIOC_INTR_VIN0;
		reg = VIOC_VIN_GetAddress((unsigned int)id * 2U) + VIN_INT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg) & (mask & VIOC_VIN_INT_MASK)) != 0U) {
			/* Prevent KCS warning */
			ret = true;
		} else {
			/* Prevent KCS warning */
			ret = false;
		}
		break;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_VIN4:
	case VIOC_INTR_VIN5:
	case VIOC_INTR_VIN6:
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_VIN7:
#endif//defined(CONFIG_TCC805X)
		id -= (VIOC_INTR_VIN_OFFSET + VIOC_INTR_VIN0);
		reg = VIOC_VIN_GetAddress((unsigned int)id * 2U) + VIN_INT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg) & (mask & VIOC_VIN_INT_MASK)) != 0U) {
			/* Prevent KCS warning */
			ret = true;
		} else {
		/* Prevent KCS warning */
			ret = false;
		}
		break;
#endif//defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_INTR] %s: id(%d) is wrong.\n",
		       __func__, id);
		ret = false;
		break;
	}

FUNC_EXIT:
	return ret;
}

bool is_vioc_intr_unmasked(int id, unsigned int mask)
{
	void __iomem *reg = NULL;
	bool ret = false;
	void *tmp_pINTR = NULL; /* avoid MISRA C-2012 Rule 8.13 */

	/* avoid MISRA C-2012 Rule 8.13 */
	tmp_pINTR = reg;
	reg = tmp_pINTR;

	if ((id < 0) || (id > VIOC_INTR_NUM)) {
		ret = false;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	switch (id) {
	case VIOC_INTR_DEV0:
	case VIOC_INTR_DEV1:
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_DEV2:
#ifdef CONFIG_TCC805X
	case VIOC_INTR_DEV3:
		if (id == VIOC_INTR_DEV3) {
			/* Prevent KCS warning */
			id -= VIOC_INTR_DISP_OFFSET - VIOC_INTR_DEV0;
		}

		reg = VIOC_DISP_GetAddress((unsigned int)id);
		if (reg == NULL) {
			ret = false;
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg + DIM) & (mask & VIOC_DISP_INT_MASK)) != 0U) {
			/* Prevent KCS warning */
			ret = false;
		} else {
			/* Prevent KCS warning */
			ret = true;
		}
		break;
#endif
#endif
#if !defined(CONFIG_TCC805X)
		reg = VIOC_DISP_GetAddress((unsigned int)id);
		if (reg == NULL) {
			ret = false;
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg + DIM) & (mask & VIOC_DISP_INT_MASK)) != 0U) {
			/* Prevent KCS warning */
			ret = false;
		} else {
			/* Prevent KCS warning */
			ret = true;
		}
		break;
#endif
	case VIOC_INTR_RD0:
	case VIOC_INTR_RD1:
	case VIOC_INTR_RD2:
	case VIOC_INTR_RD3:
	case VIOC_INTR_RD4:
	case VIOC_INTR_RD5:
	case VIOC_INTR_RD6:
	case VIOC_INTR_RD7:
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_RD8:
	case VIOC_INTR_RD9:
	case VIOC_INTR_RD10:
	case VIOC_INTR_RD11:
#endif
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_RD12:
	case VIOC_INTR_RD13:
#endif
		id -= VIOC_INTR_RD0;
		reg = VIOC_RDMA_GetAddress((unsigned int)id) + RDMAIRQMSK;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg) & (mask & VIOC_RDMA_INT_MASK)) != 0U ) {
			/* Prevent KCS warning */
			ret = false;
		} else {
			/* Prevent KCS warning */
			ret = true;
		}
		break;
	case VIOC_INTR_WD0:
	case VIOC_INTR_WD1:
	case VIOC_INTR_WD2:
	case VIOC_INTR_WD3:
	case VIOC_INTR_WD4:
	case VIOC_INTR_WD5:
	case VIOC_INTR_WD6:
	case VIOC_INTR_WD7:
	case VIOC_INTR_WD8:
		id -= VIOC_INTR_WD0;
		reg = VIOC_WDMA_GetAddress((unsigned int)id) + WDMAIRQMSK_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg) & (mask & VIOC_WDMA_INT_MASK)) != 0U) {
			/* Prevent KCS warning */
			ret = false;
		} else {
			/* Prevent KCS warning */
			ret = true;
		}
		break;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_WD9:
	case VIOC_INTR_WD10:
	case VIOC_INTR_WD11:
	case VIOC_INTR_WD12:
		id -= (VIOC_INTR_WD_OFFSET + VIOC_INTR_WD0);
		reg = VIOC_WDMA_GetAddress((unsigned int)id) + WDMAIRQMSK_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg) & (mask & VIOC_WDMA_INT_MASK)) != 0U) {
			/* Prevent KCS warning */
			ret = false;
		} else {
			/* Prevent KCS warning */
			ret = true;
		}
		break;
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_WD13:
		id -= (VIOC_INTR_WD_OFFSET - VIOC_INTR_WD_OFFSET2
		       - VIOC_INTR_WD0);
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		reg = VIOC_WDMA_GetAddress((unsigned int)id) + WDMAIRQMSK_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg) & (mask & VIOC_WDMA_INT_MASK)) != 0U) {
			/* Prevent KCS warning */
			ret = false;
		} else {
			/* Prevent KCS warning */
			ret = true;
		}
		break;
#endif
#endif
	/*
	 *	VIN_INT[31]: Not Used
	 *	VIN_INT[19]: Enable interrupt if 1 / Disable interrupt if 0
	 *	VIN_INT[18]: Enable interrupt if 1 / Disable interrupt if 0
	 *	VIN_INT[17]: Enable interrupt if 1 / Disable interrupt if 0
	 *	VIN_INT[16]: Enable interrupt if 1 / Disable interrupt if 0
	 */
	case VIOC_INTR_VIN0:
	case VIOC_INTR_VIN1:
	case VIOC_INTR_VIN2:
	case VIOC_INTR_VIN3:
		id -= VIOC_INTR_VIN0;
		reg = VIOC_VIN_GetAddress((unsigned int)id * 2U) + VIN_INT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg) & ((mask & VIOC_VIN_INT_MASK) << 16U)) != 0U) {
			/* Prevent KCS warning */
			ret = true;
		} else {
			/* Prevent KCS warning */
			ret = false;
		}
		break;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_VIN4:
	case VIOC_INTR_VIN5:
	case VIOC_INTR_VIN6:
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_VIN7:
#endif//defined(CONFIG_TCC805X)
		id -= (VIOC_INTR_VIN_OFFSET + VIOC_INTR_VIN0);
		reg = VIOC_VIN_GetAddress((unsigned int)id * 2U) + VIN_INT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		if ((__raw_readl(reg) & ((mask & VIOC_VIN_INT_MASK) << 16U)) != 0U) {
			/* Prevent KCS warning */
			ret = true;
		} else {
			/* Prevent KCS warning */
			ret = false;
		}
#endif//defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	break;
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_INTR] %s: id(%d) is wrong.\n",
		       __func__, id);
		ret = false;
		break;
	}

FUNC_EXIT:
	return ret;
}

int vioc_intr_clear(int id, unsigned int mask)
{
	void __iomem *reg;
	int ret = -1;
	u32 val = 0U;

	if ((id < 0) || (id > VIOC_INTR_NUM)) {
		ret = -1;
		/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
		goto FUNC_EXIT;
	}

	switch (id) {
	case VIOC_INTR_DEV0:
	case VIOC_INTR_DEV1:
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_DEV2:
#ifdef CONFIG_TCC805X
	case VIOC_INTR_DEV3:
		if (id == VIOC_INTR_DEV3) {
			/* Prevent KCS warning */
			id -= VIOC_INTR_DISP_OFFSET - VIOC_INTR_DEV0;
		}
		reg = VIOC_DISP_GetAddress((unsigned int)id);
		if (reg == NULL) {
			ret = -1;
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((mask & VIOC_DISP_INT_MASK), reg + DSTATUS);
		ret = 0;
		break;
#endif
#endif
#if !defined(CONFIG_TCC805X)
		reg = VIOC_DISP_GetAddress((unsigned int)id);
		if (reg == NULL) {
			ret = -1;
			/* coverity[misra_c_2012_rule_15_1_violation : FALSE] */
			goto FUNC_EXIT;
		}
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((mask & VIOC_DISP_INT_MASK), reg + DSTATUS);
		ret = 0;
		break;
#endif
	case VIOC_INTR_RD0:
	case VIOC_INTR_RD1:
	case VIOC_INTR_RD2:
	case VIOC_INTR_RD3:
	case VIOC_INTR_RD4:
	case VIOC_INTR_RD5:
	case VIOC_INTR_RD6:
	case VIOC_INTR_RD7:
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_RD8:
	case VIOC_INTR_RD9:
	case VIOC_INTR_RD10:
	case VIOC_INTR_RD11:
#endif
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_RD12:
	case VIOC_INTR_RD13:
#endif
		id -= VIOC_INTR_RD0;
		reg = VIOC_RDMA_GetAddress((unsigned int)id) + RDMASTAT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((mask & VIOC_RDMA_INT_MASK), reg);
		ret = 0;
		break;
	case VIOC_INTR_WD0:
	case VIOC_INTR_WD1:
	case VIOC_INTR_WD2:
	case VIOC_INTR_WD3:
	case VIOC_INTR_WD4:
	case VIOC_INTR_WD5:
	case VIOC_INTR_WD6:
	case VIOC_INTR_WD7:
	case VIOC_INTR_WD8:
		id -= VIOC_INTR_WD0;
		reg = VIOC_WDMA_GetAddress((unsigned int)id) + WDMAIRQSTS_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((mask & VIOC_WDMA_INT_MASK), reg);
		ret = 0;
		break;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_WD9:
	case VIOC_INTR_WD10:
	case VIOC_INTR_WD11:
	case VIOC_INTR_WD12:
		id -= (VIOC_INTR_WD_OFFSET + VIOC_INTR_WD0);
		reg = VIOC_WDMA_GetAddress((unsigned int)id) + WDMAIRQSTS_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((mask & VIOC_WDMA_INT_MASK), reg);
		ret = 0;
		break;
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_WD13:
		id -= (VIOC_INTR_WD_OFFSET - VIOC_INTR_WD_OFFSET2
		       - VIOC_INTR_WD0);
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		reg = VIOC_WDMA_GetAddress((unsigned int)id) + WDMAIRQSTS_OFFSET;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel((mask & VIOC_WDMA_INT_MASK), reg);
		ret = 0;
		break;
#endif
#endif
	case VIOC_INTR_VIN0:
	case VIOC_INTR_VIN1:
	case VIOC_INTR_VIN2:
	case VIOC_INTR_VIN3:
		id -= VIOC_INTR_VIN0;
		reg = VIOC_VIN_GetAddress((unsigned int)id * 2U) + VIN_INT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel((val | (mask & VIOC_VIN_INT_MASK)), reg);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		ret = 0;
		break;
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	case VIOC_INTR_VIN4:
	case VIOC_INTR_VIN5:
	case VIOC_INTR_VIN6:
#if defined(CONFIG_TCC805X)
	case VIOC_INTR_VIN7:
#endif//defined(CONFIG_TCC805X)
		id -= (VIOC_INTR_VIN_OFFSET + VIOC_INTR_VIN0);
		reg = VIOC_VIN_GetAddress((unsigned int)id * 2U) + VIN_INT;
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(reg);
		__raw_writel((val | (mask & VIOC_VIN_INT_MASK)), reg);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		ret = 0;
		break;
#endif//defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
	default:
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		pr_err("[ERR][VIOC_INTR] %s: id(%d) is wrong.\n",
		       __func__, id);
		ret = -1;
		break;
	}

FUNC_EXIT:
	return ret;
}

void vioc_intr_initialize(void)
{
	void __iomem *reg = VIOC_IREQConfig_GetAddress();
	unsigned int i = 0U;

	/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
	/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
	__raw_writel(0xffffffff, reg + IRQMASKCLR0_0_OFFSET);
	__raw_writel(0xffffffff, reg + IRQMASKCLR0_1_OFFSET);
#if !defined(CONFIG_TCC897X)
	__raw_writel(0xffffffff, reg + IRQMASKCLR0_2_OFFSET);
#endif

	/* disp irq mask & status clear */
#if defined(CONFIG_TCC805X)
	for (i = 0;
	     i < ((unsigned int)VIOC_INTR_DEV3
		 	- ((unsigned int)VIOC_INTR_DISP_OFFSET
		 	+ (unsigned int)VIOC_INTR_DEV0));
	     i++) {
#else
	for (i = 0; i < ((unsigned int)VIOC_INTR_DEV2
			- (unsigned int)VIOC_INTR_DEV0); i++) {
#endif
		reg = VIOC_DISP_GetAddress(i);
		if (reg == NULL) {
			/* Prevent KCS warning */
			continue;
		}
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(VIOC_DISP_INT_MASK, reg + DIM);
		__raw_writel(VIOC_DISP_INT_MASK, reg + DSTATUS);
	}

	/* rdma irq mask & status clear */
	for (i = 0; i < ((unsigned int)VIOC_INTR_RD11
			- (unsigned int)VIOC_INTR_RD0); i++) {
		reg = VIOC_RDMA_GetAddress(i);
		if (reg == NULL) {
			/* Prevent KCS warning */
			continue;
		}
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(VIOC_RDMA_INT_MASK, reg + RDMAIRQMSK);
		__raw_writel(VIOC_RDMA_INT_MASK, reg + RDMASTAT);
	}

	/* wdma irq mask & status clear */
#if defined(CONFIG_TCC803X) || defined(CONFIG_TCC805X)
#if defined(CONFIG_TCC805X)
	for (i = 0; i < ((unsigned int)VIOC_INTR_WD13 -
			((unsigned int)VIOC_INTR_WD_OFFSET +
			(unsigned int)VIOC_INTR_WD_OFFSET2 + (unsigned int)VIOC_INTR_WD0)); i++) {
#else
	for (i = 0; i < ((unsigned int)VIOC_INTR_WD12 -
			((unsigned int)VIOC_INTR_WD_OFFSET + (unsigned int)VIOC_INTR_WD0)); i++) {
#endif
		reg = VIOC_WDMA_GetAddress(i);
		if (reg == NULL) {
			/* Prevent KCS warning */
			continue;
		}
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(VIOC_WDMA_INT_MASK, reg + WDMAIRQMSK_OFFSET);
		__raw_writel(VIOC_WDMA_INT_MASK, reg + WDMAIRQSTS_OFFSET);
	}
#else
	for (i = 0; i < (VIOC_INTR_WD8 - VIOC_INTR_WD0); i++) {
		reg = VIOC_WDMA_GetAddress(i);
		if (reg == NULL) {
			/* Prevent KCS warning*/
			continue;
		}
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(VIOC_WDMA_INT_MASK, reg + WDMAIRQMSK_OFFSET);
		__raw_writel(VIOC_WDMA_INT_MASK, reg + WDMAIRQSTS_OFFSET);
	}
#endif
}
