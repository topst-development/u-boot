/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef VIOC_DDI_CONFIG_H
#define VIOC_DDI_CONFIG_H

#define LVDS_TX_DATA_MAX (8)
#define LVDS_TX_DATA(x) (((x) > LVDS_TX_DATA_MAX) ? (-x) : (x))

#ifdef CONFIG_VIOC_DOLBY_VISION_EDR
#define DDICFG_TYPE_NG (5)
#define DDICFG_TYPE_DV (8)
#endif

#define DDICFG_TYPE_ISP_AXI (3)
#define DDICFG_TYPE_DP_AXI (2)
#define DDICFG_TYPE_VIOC (0)

/*
 * Register offset
 */
#define DDI_CLKEN		(0x00U)
#define SWRESET		(0x04U)
#define	X2X_SLV0_QCH_CC		(0x80U)
#define	X2X_SLV0_QCH_PC		(0x84U)
#define	X2X_SLV1_QCH_CC		(0x88U)
#define X2X_SLV1_QCH_PC		(0x8CU)
#define X2X_DP_MST_QCH_CC		(0x90U)
#define X2X_DDI_MST_QCH_CC		(0x94U)

/*
 * Clock Enable for DDIBUS
 */
#define CLKEN_LCLK4_SEL_SHIFT			(24U)
#define CLKEN_LCLK3_SEL_SHIFT			(22U)
#define CLKEN_LCLK2_SEL_SHIFT			(20U)
#define CLKEN_LCLK1_SEL_SHIFT			(18U)
#define CLKEN_LCLK0_SEL_SHIFT			(16U)
#define CLKEN_VIOC_SHIFT			(0U)

#define CLKEN_LCLK4_SEL_MASK			((u32)0x1U << CLKEN_LCLK4_SEL_SHIFT)
#define CLKEN_LCLK3_SEL_MASK			((u32)0x1U << CLKEN_LCLK3_SEL_SHIFT)
#define CLKEN_LCLK2_SEL_MASK			((u32)0x1U << CLKEN_LCLK2_SEL_SHIFT)
#define CLKEN_LCLK1_SEL_MASK			((u32)0x1U << CLKEN_LCLK1_SEL_SHIFT)
#define CLKEN_LCLK0_SEL_MASK			((u32)0x1U << CLKEN_LCLK0_SEL_SHIFT)
#define CLKEN_VIOC_MASK			((u32)0x1U << CLKEN_VIOC_SHIFT)

/*
 * SWReset for DDIBUS
 */
#define SWRESET_VIOC_SHIFT		(0U)

#define SWRESET_VIOC_MASK		((u32)0x1U << SWRESET_VIOC_SHIFT)

/*
 * AXI Async Bridge Slave 0 LPI Q-CH Clock Control
 */
#define X2X_SLV0_QCH_CC_DELAY_VALUE_SHIFT	(16U)
#define X2X_SLV0_QCH_CC_QFSM_STATE_SHIFT	(12U)
#define X2X_SLV0_QCH_CC_M_QDENY_SHIFT	(11U)
#define X2X_SLV0_QCH_CC_M_QACCEPTn_SHIFT	(10U)
#define X2X_SLV0_QCH_CC_M_QACTIVE_SHIFT	(9U)
#define X2X_SLV0_QCH_CC_M_QREQn_SHIFT	(8U)
#define X2X_SLV0_QCH_CC_CC_MODE_ACK_SHIFT	(4U)
#define X2X_SLV0_QCH_CC_QACTIVE_POLICY_SHIFT	(2U)
#define X2X_SLV0_QCH_CC_CC_MODE_REQ_SHIFT	(0U)

#define X2X_SLV0_QCH_CC_DELAY_VALUE_MASK	((u32)0xFFFFU << X2X_SLV0_QCH_CC_DELAY_VALUE_SHIFT)
#define X2X_SLV0_QCH_CC_QFSM_STATE_MASK	((u32)0x7U << X2X_SLV0_QCH_CC_QFSM_STATE_SHIFT)
#define X2X_SLV0_QCH_CC_M_QDENY_MASK	((u32)0x1U << X2X_SLV0_QCH_CC_M_QDENY_SHIFT)
#define X2X_SLV0_QCH_CC_M_QACCEPTn_MASK	((u32)0x1U << X2X_SLV0_QCH_CC_M_QACCEPTn_SHIFT)
#define X2X_SLV0_QCH_CC_M_QACTIVE_MASK	((u32)0x1U << X2X_SLV0_QCH_CC_M_QACTIVE_SHIFT)
#define X2X_SLV0_QCH_CC_M_QREQn_MASK	((u32)0x1U << X2X_SLV0_QCH_CC_M_QREQn_SHIFT)
#define X2X_SLV0_QCH_CC_CC_MODE_ACK_MASK	((u32)0x3U << X2X_SLV0_QCH_CC_CC_MODE_ACK_SHIFT)
#define X2X_SLV0_QCH_CC_QACTIVE_POLICY_MASK	((u32)0x1U << X2X_SLV0_QCH_CC_QACTIVE_POLICY_SHIFT)
#define X2X_SLV0_QCH_CC_CC_MODE_REQ_MASK	((u32)0x3U << X2X_SLV0_QCH_CC_CC_MODE_REQ_SHIFT)

/*
 * AXI Async Bridge Slave 0 LPI Q-CH Power Control Register
 */
#define X2X_SLV0_QCH_PC_DELAY_VALUE_SHIFT	(16U)
#define X2X_SLV0_QCH_PC_QFSM_STATE_SHIFT	(12U)
#define X2X_SLV0_QCH_PC_M_QDENY_SHIFT	(11U)
#define X2X_SLV0_QCH_PC_M_QACCEPTn_SHIFT	(10U)
#define X2X_SLV0_QCH_PC_M_QACTIVE_SHIFT	(9U)
#define X2X_SLV0_QCH_PC_M_QREQn_SHIFT	(8U)
#define X2X_SLV0_QCH_PC_CC_MODE_ACK_SHIFT	(4U)
#define X2X_SLV0_QCH_PC_PWRQ_PERMIT_DENY_SHIFT	(3U)
#define X2X_SLV0_QCH_PC_QACTIVE_POLICY_SHIFT	(2U)
#define X2X_SLV0_QCH_PC_CC_MODE_REQ_SHIFT	(0U)

#define X2X_SLV0_QCH_PC_DELAY_VALUE_MASK	((u32)0xFFFFU << X2X_SLV0_QCH_PC_DELAY_VALUE_SHIFT)
#define X2X_SLV0_QCH_PC_QFSM_STATE_MASK	((u32)0x7U << X2X_SLV0_QCH_PC_QFSM_STATE_SHIFT)
#define X2X_SLV0_QCH_PC_M_QDENY_MASK	((u32)0x1U << X2X_SLV0_QCH_PC_M_QDENY_SHIFT)
#define X2X_SLV0_QCH_PC_M_QACCEPTn_MASK	((u32)0x1U << X2X_SLV0_QCH_PC_M_QACCEPTn_SHIFT)
#define X2X_SLV0_QCH_PC_M_QACTIVE_MASK	((u32)0x1U << X2X_SLV0_QCH_PC_M_QACTIVE_SHIFT)
#define X2X_SLV0_QCH_PC_M_QREQn_MASK	((u32)0x1U << X2X_SLV0_QCH_PC_M_QREQn_SHIFT)
#define X2X_SLV0_QCH_PC_CC_MODE_ACK_MASK	((u32)0x3U << X2X_SLV0_QCH_PC_CC_MODE_ACK_SHIFT)
#define X2X_SLV0_QCH_PC_PWRQ_PERMIT_DENY_MASK	((u32)0x1U << X2X_SLV0_QCH_PC_PWRQ_PERMIT_DENY_SHIFT)
#define X2X_SLV0_QCH_PC_QACTIVE_POLICY_MASK	((u32)0x1U << X2X_SLV0_QCH_PC_QACTIVE_POLICY_SHIFT)
#define X2X_SLV0_QCH_PC_CC_MODE_REQ_MASK	((u32)0x3U << X2X_SLV0_QCH_PC_CC_MODE_REQ_SHIFT)

/*
 * AXI Async Bridge Slave 1 LPI Q-Ch Clock Control Register
 */
#define X2X_SLV1_QCH_CC_DELAY_VALUE_SHIFT	(16U)
#define X2X_SLV1_QCH_CC_QFSM_STATE_SHIFT	(12U)
#define X2X_SLV1_QCH_CC_M_QDENY_SHIFT	(11U)
#define X2X_SLV1_QCH_CC_M_QACCEPTn_SHIFT	(10U)
#define X2X_SLV1_QCH_CC_M_QACTIVE_SHIFT	(9U)
#define X2X_SLV1_QCH_CC_M_QREQn_SHIFT	(8U)
#define X2X_SLV1_QCH_CC_CC_MODE_ACK_SHIFT	(4U)
#define X2X_SLV1_QCH_CC_QACTIVE_POLICY_SHIFT	(2U)
#define X2X_SLV1_QCH_CC_CC_MODE_REQ_SHIFT	(0U)

#define X2X_SLV1_QCH_CC_DELAY_VALUE_MASK	((u32)0xFFFFU << X2X_SLV1_QCH_CC_DELAY_VALUE_SHIFT)
#define X2X_SLV1_QCH_CC_QFSM_STATE_MASK	((u32)0x7U << X2X_SLV1_QCH_CC_QFSM_STATE_SHIFT)
#define X2X_SLV1_QCH_CC_M_QDENY_MASK	((u32)0x1U << X2X_SLV1_QCH_CC_M_QDENY_SHIFT)
#define X2X_SLV1_QCH_CC_M_QACCEPTn_MASK	((u32)0x1U << X2X_SLV1_QCH_CC_M_QACCEPTn_SHIFT)
#define X2X_SLV1_QCH_CC_M_QACTIVE_MASK	((u32)0x1U << X2X_SLV1_QCH_CC_M_QACTIVE_SHIFT)
#define X2X_SLV1_QCH_CC_M_QREQn_MASK	((u32)0x1U << X2X_SLV1_QCH_CC_M_QREQn_SHIFT)
#define X2X_SLV1_QCH_CC_CC_MODE_ACK_MASK	((u32)0x3U << X2X_SLV1_QCH_CC_CC_MODE_ACK_SHIFT)
#define X2X_SLV1_QCH_CC_QACTIVE_POLICY_MASK	((u32)0x1U << X2X_SLV1_QCH_CC_QACTIVE_POLICY_SHIFT)
#define X2X_SLV1_QCH_CC_CC_MODE_REQ_MASK	((u32)0x3U << X2X_SLV1_QCH_CC_CC_MODE_REQ_SHIFT)

/*
 * AXI Async Bridge Slave 1 LPI Q-CH Power Control Register
 */
#define X2X_SLV1_QCH_PC_DELAY_VALUE_SHIFT	(16U)
#define X2X_SLV1_QCH_PC_QFSM_STATE_SHIFT	(12U)
#define X2X_SLV1_QCH_PC_M_QDENY_SHIFT	(11U)
#define X2X_SLV1_QCH_PC_M_QACCEPTn_SHIFT	(10U)
#define X2X_SLV1_QCH_PC_M_QACTIVE_SHIFT	(9U)
#define X2X_SLV1_QCH_PC_M_QREQn_SHIFT	(8U)
#define X2X_SLV1_QCH_PC_CC_MODE_ACK_SHIFT	(4U)
#define X2X_SLV1_QCH_PC_PWRQ_PERMIT_DENY_SHIFT	(3U)
#define X2X_SLV1_QCH_PC_QACTIVE_POLICY_SHIFT	(2U)
#define X2X_SLV1_QCH_PC_CC_MODE_REQ_SHIFT	(0U)

#define X2X_SLV1_QCH_PC_DELAY_VALUE_MASK	((u32)0xFFFFU << X2X_SLV1_QCH_PC_DELAY_VALUE_SHIFT)
#define X2X_SLV1_QCH_PC_QFSM_STATE_MASK	((u32)0x7U << X2X_SLV1_QCH_PC_QFSM_STATE_SHIFT)
#define X2X_SLV1_QCH_PC_M_QDENY_MASK	((u32)0x1U << X2X_SLV1_QCH_PC_M_QDENY_SHIFT)
#define X2X_SLV1_QCH_PC_M_QACCEPTn_MASK	((u32)0x1U << X2X_SLV1_QCH_PC_M_QACCEPTn_SHIFT)
#define X2X_SLV1_QCH_PC_M_QACTIVE_MASK	((u32)0x1U << X2X_SLV1_QCH_PC_M_QACTIVE_SHIFT)
#define X2X_SLV1_QCH_PC_M_QREQn_MASK	((u32)0x1U << X2X_SLV1_QCH_PC_M_QREQn_SHIFT)
#define X2X_SLV1_QCH_PC_CC_MODE_ACK_MASK	((u32)0x3U << X2X_SLV1_QCH_PC_CC_MODE_ACK_SHIFT)
#define X2X_SLV1_QCH_PC_PWRQ_PERMIT_DENY_MASK	((u32)0x1U << X2X_SLV1_QCH_PC_PWRQ_PERMIT_DENY_SHIFT)
#define X2X_SLV1_QCH_PC_QACTIVE_POLICY_MASK	((u32)0x1U << X2X_SLV1_QCH_PC_QACTIVE_POLICY_SHIFT)
#define X2X_SLV1_QCH_PC_CC_MODE_REQ_MASK	((u32)0x3U << X2X_SLV1_QCH_PC_CC_MODE_REQ_SHIFT)

/*
 * AXI Async Bridge DP Master LPI Q-CH Clock Control Register
 */
#define X2X_DP_MST_QCH_CC_DELAY_VALUE_SHIFT	(16U)
#define X2X_DP_MST_QCH_CC_QFSM_STATE_SHIFT	(12U)
#define X2X_DP_MST_QCH_CC_M_QDENY_SHIFT	(11U)
#define X2X_DP_MST_QCH_CC_M_QACCEPTn_SHIFT	(10U)
#define X2X_DP_MST_QCH_CC_M_QACTIVE_SHIFT	(9U)
#define X2X_DP_MST_QCH_CC_M_QREQn_SHIFT	(8U)
#define X2X_DP_MST_QCH_CC_CC_MODE_ACK_SHIFT	(4U)
#define X2X_DP_MST_QCH_CC_QACTIVE_POLICY_SHIFT	(2U)
#define X2X_DP_MST_QCH_CC_CC_MODE_REQ_SHIFT	(0U)

#define X2X_DP_MST_QCH_CC_DELAY_VALUE_MASK	((u32)0xFFFFU << X2X_DP_MST_QCH_CC_DELAY_VALUE_SHIFT)
#define X2X_DP_MST_QCH_CC_QFSM_STATE_MASK	((u32)0x7U << X2X_DP_MST_QCH_CC_QFSM_STATE_SHIFT)
#define X2X_DP_MST_QCH_CC_M_QDENY_MASK	((u32)0x1U << X2X_DP_MST_QCH_CC_M_QDENY_SHIFT)
#define X2X_DP_MST_QCH_CC_M_QACCEPTn_MASK	((u32)0x1U << X2X_DP_MST_QCH_CC_M_QACCEPTn_SHIFT)
#define X2X_DP_MST_QCH_CC_M_QACTIVE_MASK	((u32)0x1U << X2X_DP_MST_QCH_CC_M_QACTIVE_SHIFT)
#define X2X_DP_MST_QCH_CC_M_QREQn_MASK	((u32)0x1U << X2X_DP_MST_QCH_CC_M_QREQn_SHIFT)
#define X2X_DP_MST_QCH_CC_CC_MODE_ACK_MASK	((u32)0x3U << X2X_DP_MST_QCH_CC_CC_MODE_ACK_SHIFT)
#define X2X_DP_MST_QCH_CC_QACTIVE_POLICY_MASK	((u32)0x1U << X2X_DP_MST_QCH_CC_QACTIVE_POLICY_SHIFT)
#define X2X_DP_MST_QCH_CC_CC_MODE_REQ_MASK	((u32)0x3U << X2X_DP_MST_QCH_CC_CC_MODE_REQ_SHIFT)

/*
 * AXI Async Bridge DDI Master LPI Q-CH Clock Control Register
 */
#define X2X_DDI_MST_QCH_CC_DELAY_VALUE_SHIFT	(16U)
#define X2X_DDI_MST_QCH_CC_QFSM_STATE_SHIFT	(12U)
#define X2X_DDI_MST_QCH_CC_M_QDENY_SHIFT	(11U)
#define X2X_DDI_MST_QCH_CC_M_QACCEPTn_SHIFT	(10U)
#define X2X_DDI_MST_QCH_CC_M_QACTIVE_SHIFT	(9U)
#define X2X_DDI_MST_QCH_CC_M_QREQn_SHIFT	(8U)
#define X2X_DDI_MST_QCH_CC_CC_MODE_ACK_SHIFT	(4U)
#define X2X_DDI_MST_QCH_CC_QACTIVE_POLICY_SHIFT	(2U)
#define X2X_DDI_MST_QCH_CC_CC_MODE_REQ_SHIFT	(0U)

#define X2X_DDI_MST_QCH_CC_DELAY_VALUE_MASK	((u32)0xFFFFU << X2X_DDI_MST_QCH_CC_DELAY_VALUE_SHIFT)
#define X2X_DDI_MST_QCH_CC_QFSM_STATE_MASK	((u32)0x7U << X2X_DDI_MST_QCH_CC_QFSM_STATE_SHIFT)
#define X2X_DDI_MST_QCH_CC_M_QDENY_MASK	((u32)0x1U << X2X_DDI_MST_QCH_CC_M_QDENY_SHIFT)
#define X2X_DDI_MST_QCH_CC_M_QACCEPTn_MASK	((u32)0x1U << X2X_DDI_MST_QCH_CC_M_QACCEPTn_SHIFT)
#define X2X_DDI_MST_QCH_CC_M_QACTIVE_MASK	((u32)0x1U << X2X_DDI_MST_QCH_CC_M_QACTIVE_SHIFT)
#define X2X_DDI_MST_QCH_CC_M_QREQn_MASK	((u32)0x1U << X2X_DDI_MST_QCH_CC_M_QREQn_SHIFT)
#define X2X_DDI_MST_QCH_CC_CC_MODE_ACK_MASK	((u32)0x3U << X2X_DDI_MST_QCH_CC_CC_MODE_ACK_SHIFT)
#define X2X_DDI_MST_QCH_CC_QACTIVE_POLICY_MASK	((u32)0x1U << X2X_DDI_MST_QCH_CC_QACTIVE_POLICY_SHIFT)
#define X2X_DDI_MST_QCH_CC_CC_MODE_REQ_MASK	((u32)0x3U << X2X_DDI_MST_QCH_CC_CC_MODE_REQ_SHIFT)

extern void VIOC_DDICONFIG_SetSWRESET(unsigned int type, unsigned int set);
extern void VIOC_DDICONFIG_SetPWDN(unsigned int type, unsigned int set);
extern void VIOC_DDICONFIG_SetPeriClock(unsigned int num, unsigned int set);
extern unsigned int VIOC_DDICONFIG_GetViocRemap(void);
extern unsigned int VIOC_DDICONFIG_SetViocRemap(unsigned int enable);
extern void VIOC_DDICONFIG_DUMP(void);
extern void __iomem *VIOC_DDICONFIG_GetAddress(void);
#endif
