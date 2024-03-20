// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef LVDS_WRAP_H
#define LVDS_WRAP_H

/*
 * Register offset
 */
#define TS_CFG (0x040)
#define DISP_MUX_EN (0x100)
#define DISP_MUX_SEL (0x104)
#define TS_MUX_EN0 (0x110)
#define TS_MUX_SEL0 (0x114)
#define TXOUT_SEL0_0 (0x118)
#define TXOUT_SEL1_0 (0x11C)
#define TXOUT_SEL2_0 (0x120)
#define TXOUT_SEL3_0 (0x124)
#define TXOUT_SEL4_0 (0x128)
#define TXOUT_SEL5_0 (0x12C)
#define TXOUT_SEL6_0 (0x130)
#define TXOUT_SEL7_0 (0x134)
#define TXOUT_SEL8_0 (0x138)
#define TS_MUX_EN1 (0x13C)
#define TS_MUX_SEL1 (0x140)
#define TXOUT_SEL0_1 (0x144)
#define TXOUT_SEL1_1 (0x148)
#define TXOUT_SEL2_1 (0x14C)
#define TXOUT_SEL3_1 (0x150)
#define TXOUT_SEL4_1 (0x154)
#define TXOUT_SEL5_1 (0x158)
#define TXOUT_SEL6_1 (0x15C)
#define TXOUT_SEL7_1 (0x160)
#define TXOUT_SEL8_1 (0x164)
#define TS_MUX_EN2 (0x168)
#define TS_MUX_SEL2 (0x16C)
#define TXOUT_SEL0_2 (0x170)
#define TXOUT_SEL1_2 (0x174)
#define TXOUT_SEL2_2 (0x178)
#define TXOUT_SEL3_2 (0x17C)
#define TXOUT_SEL4_2 (0x180)
#define TXOUT_SEL5_2 (0x184)
#define TXOUT_SEL6_2 (0x188)
#define TXOUT_SEL7_2 (0x18C)
#define TXOUT_SEL8_2 (0x190)
#define TS_MUX_EN3 (0x194)
#define TS_MUX_SEL3 (0x198)
#define TXOUT_SEL0_3 (0x19C)
#define TXOUT_SEL1_3 (0x1A0)
#define TXOUT_SEL2_3 (0x1A4)
#define TXOUT_SEL3_3 (0x1A8)
#define TXOUT_SEL4_3 (0x1AC)
#define TXOUT_SEL5_3 (0x1B0)
#define TXOUT_SEL6_3 (0x1B4)
#define TXOUT_SEL7_3 (0x1B8)
#define TXOUT_SEL8_3 (0x1BC)
#define CRC_VRF_CNT (0x1C0)
#define SAL_0 (0x1C8)
#define REF_CLK_CNT (0x1CC)
#define CMP_CLK_CNT (0x1D0)
#define HSYNC_CFG_0 (0x1D4)
#define HSYNC_CFG_1 (0x1D8)
#define HSYNC_CFG_2 (0x1DC)
#define VSYNC_CFG_0 (0x1E0)
#define VSYNC_CFG_1 (0x1E4)
#define VSYNC_CFG_2 (0x1E8)
#define CFG_PROTECT (0x1EC)
#define TS_IREQ_CLEAR (0x1F0)
#define TS_IREQ_STATUS (0x1F4)
#define SAL_1 (0x1F8)
#define TS_SWRESET (0x1FC)
#define LVDS_IREQ_CLEAR (0x200)
#define LVDS_IREQ_STATUS (0x204)
#define TS_IREQ_MASK (0x208)
#define LVDS_IREQ_MASK (0x20C)
#define SM_BYPASS (0x214) /* TCC805X CS only */

/*
 * Tx splitter configuration Register
 */
#define TS_CFG_WIDTH_SHIFT (16)
#define TS_CFG_SWAP3_SHIFT (10)
#define TS_CFG_SWAP2_SHIFT (8)
#define TS_CFG_SWAP1_SHIFT (6)
#define TS_CFG_SWAP0_SHIFT (4)
#define TS_CFG_MODE_SHIFT (2)
#define TS_CFG_LR_SHIFT (1)
#define TS_CFG_BP_SHIFT (0)

#define TS_CFG_WIDTH_MASK (0xFFF << TS_CFG_WIDTH_SHIFT)
#define TS_CFG_SWAP3_MASK (0x3 << TS_CFG_SWAP3_SHIFT)
#define TS_CFG_SWAP2_MASK (0x3 << TS_CFG_SWAP2_SHIFT)
#define TS_CFG_SWAP1_MASK (0x3 << TS_CFG_SWAP1_SHIFT)
#define TS_CFG_SWAP0_MASK (0x3 << TS_CFG_SWAP0_SHIFT)
#define TS_CFG_MODE_MASK (0x1 << TS_CFG_MODE_SHIFT)
#define TS_CFG_LR_MASK (0x1 << TS_CFG_LR_SHIFT)
#define TS_CFG_BP_MASK (0x1 << TS_CFG_BP_SHIFT)

/*
 * DISP MUX Enable Register
 */
#define DISP_MUX_EN_EN_SHIFT (0)
#define DISP_MUX_EN_EN_MASK (0x1 << DISP_MUX_EN_EN_SHIFT)

/*
 * DISP MUX Select Register
 */
#define DISP_MUX_SEL_SEL_SHIFT (0)
#define DISP_MUX_SEL_SEL_MASK (0x3 << DISP_MUX_SEL_SEL_SHIFT)

/*
 * Tx splitter MUX Enable Register
 */
#define TS_MUX_EN_EN_SHIFT (0)
#define TS_MUX_EN_EN_MASK (0x1 << TS_MUX_EN_EN_SHIFT)

/*
 * Tx splitter MUX Select Register
 */
#define TS_MUX_SEL_SEL_SHIFT (0)
#define TS_MUX_SEL_SEL_MASK (0x7 << TS_MUX_SEL_SEL_SHIFT)

/*
 * Tx splitter TXOUT select0 Register
 */
#define TXOUT_SEL0_SEL03_SHIFT (24)
#define TXOUT_SEL0_SEL02_SHIFT (16)
#define TXOUT_SEL0_SEL01_SHIFT (8)
#define TXOUT_SEL0_SEL00_SHIFT (0)

#define TXOUT_SEL0_SEL03_MASK (0x1F << TXOUT_SEL0_SEL03_SHIFT)
#define TXOUT_SEL0_SEL02_MASK (0x1F << TXOUT_SEL0_SEL02_SHIFT)
#define TXOUT_SEL0_SEL01_MASK (0x1F << TXOUT_SEL0_SEL01_SHIFT)
#define TXOUT_SEL0_SEL00_MASK (0x1F << TXOUT_SEL0_SEL00_SHIFT)

/*
 * Tx splitter TXOUT select1 Register
 */
#define TXOUT_SEL1_SEL07_SHIFT (24)
#define TXOUT_SEL1_SEL06_SHIFT (16)
#define TXOUT_SEL1_SEL05_SHIFT (8)
#define TXOUT_SEL1_SEL04_SHIFT (0)

#define TXOUT_SEL1_SEL07_MASK (0x1F << TXOUT_SEL1_SEL07_SHIFT)
#define TXOUT_SEL1_SEL06_MASK (0x1F << TXOUT_SEL1_SEL06_SHIFT)
#define TXOUT_SEL1_SEL05_MASK (0x1F << TXOUT_SEL1_SEL05_SHIFT)
#define TXOUT_SEL1_SEL04_MASK (0x1F << TXOUT_SEL1_SEL04_SHIFT)

/*
 * Tx splitter TXOUT select2 Register
 */
#define TXOUT_SEL2_SEL11_SHIFT (24)
#define TXOUT_SEL2_SEL10_SHIFT (16)
#define TXOUT_SEL2_SEL09_SHIFT (8)
#define TXOUT_SEL2_SEL08_SHIFT (0)

#define TXOUT_SEL2_SEL11_MASK (0x1F << TXOUT_SEL2_SEL11_SHIFT)
#define TXOUT_SEL2_SEL10_MASK (0x1F << TXOUT_SEL2_SEL10_SHIFT)
#define TXOUT_SEL2_SEL09_MASK (0x1F << TXOUT_SEL2_SEL09_SHIFT)
#define TXOUT_SEL2_SEL08_MASK (0x1F << TXOUT_SEL2_SEL08_SHIFT)

/*
 * Tx splitter TXOUT select3 Register
 */
#define TXOUT_SEL3_SEL15_SHIFT (24)
#define TXOUT_SEL3_SEL14_SHIFT (16)
#define TXOUT_SEL3_SEL13_SHIFT (8)
#define TXOUT_SEL3_SEL12_SHIFT (0)

#define TXOUT_SEL3_SEL15_MASK (0x1F << TXOUT_SEL3_SEL15_SHIFT)
#define TXOUT_SEL3_SEL14_MASK (0x1F << TXOUT_SEL3_SEL14_SHIFT)
#define TXOUT_SEL3_SEL13_MASK (0x1F << TXOUT_SEL3_SEL13_SHIFT)
#define TXOUT_SEL3_SEL12_MASK (0x1F << TXOUT_SEL3_SEL12_SHIFT)

/*
 * Tx splitter TXOUT select4 Register
 */
#define TXOUT_SEL4_SEL19_SHIFT (24)
#define TXOUT_SEL4_SEL18_SHIFT (16)
#define TXOUT_SEL4_SEL17_SHIFT (8)
#define TXOUT_SEL4_SEL16_SHIFT (0)

#define TXOUT_SEL4_SEL19_MASK (0x1F << TXOUT_SEL4_SEL19_SHIFT)
#define TXOUT_SEL4_SEL18_MASK (0x1F << TXOUT_SEL4_SEL18_SHIFT)
#define TXOUT_SEL4_SEL17_MASK (0x1F << TXOUT_SEL4_SEL17_SHIFT)
#define TXOUT_SEL4_SEL16_MASK (0x1F << TXOUT_SEL4_SEL16_SHIFT)

/*
 * Tx splitter TXOUT select5 Register
 */
#define TXOUT_SEL5_SEL23_SHIFT (24)
#define TXOUT_SEL5_SEL22_SHIFT (16)
#define TXOUT_SEL5_SEL21_SHIFT (8)
#define TXOUT_SEL5_SEL20_SHIFT (0)

#define TXOUT_SEL5_SEL23_MASK (0x1F << TXOUT_SEL5_SEL23_SHIFT)
#define TXOUT_SEL5_SEL22_MASK (0x1F << TXOUT_SEL5_SEL22_SHIFT)
#define TXOUT_SEL5_SEL21_MASK (0x1F << TXOUT_SEL5_SEL21_SHIFT)
#define TXOUT_SEL5_SEL20_MASK (0x1F << TXOUT_SEL5_SEL20_SHIFT)

/*
 * Tx splitter TXOUT select6 Register
 */
#define TXOUT_SEL6_SEL27_SHIFT (24)
#define TXOUT_SEL6_SEL26_SHIFT (16)
#define TXOUT_SEL6_SEL25_SHIFT (8)
#define TXOUT_SEL6_SEL24_SHIFT (0)

#define TXOUT_SEL6_SEL27_MASK (0x1F << TXOUT_SEL6_SEL27_SHIFT)
#define TXOUT_SEL6_SEL26_MASK (0x1F << TXOUT_SEL6_SEL26_SHIFT)
#define TXOUT_SEL6_SEL25_MASK (0x1F << TXOUT_SEL6_SEL25_SHIFT)
#define TXOUT_SEL6_SEL24_MASK (0x1F << TXOUT_SEL6_SEL24_SHIFT)

/*
 * Tx splitter TXOUT select7 Register
 */
#define TXOUT_SEL7_SEL31_SHIFT (24)
#define TXOUT_SEL7_SEL30_SHIFT (16)
#define TXOUT_SEL7_SEL29_SHIFT (8)
#define TXOUT_SEL7_SEL28_SHIFT (0)

#define TXOUT_SEL7_SEL31_MASK (0x1F << TXOUT_SEL7_SEL31_SHIFT)
#define TXOUT_SEL7_SEL30_MASK (0x1F << TXOUT_SEL7_SEL30_SHIFT)
#define TXOUT_SEL7_SEL29_MASK (0x1F << TXOUT_SEL7_SEL29_SHIFT)
#define TXOUT_SEL7_SEL28_MASK (0x1F << TXOUT_SEL7_SEL28_SHIFT)

/*
 * Tx splitter TXOUT select8 Register
 */
#define TXOUT_SEL8_SEL34_SHIFT (16)
#define TXOUT_SEL8_SEL33_SHIFT (8)
#define TXOUT_SEL8_SEL32_SHIFT (0)

#define TXOUT_SEL8_SEL34_MASK (0x1F << TXOUT_SEL8_SEL34_SHIFT)
#define TXOUT_SEL8_SEL33_MASK (0x1F << TXOUT_SEL8_SEL33_SHIFT)
#define TXOUT_SEL8_SEL32_MASK (0x1F << TXOUT_SEL8_SEL32_SHIFT)

/*
 * CRC VRF Count register
 */
#define CRC_VRF_CNT_CMP1_SHIFT (24)
#define CRC_VRF_CNT_CMP0_SHIFT (16)
#define CRC_VRF_CNT_REF1_SHIFT (8)
#define CRC_VRF_CNT_REF0_SHIFT (0)

#define CRC_VRF_CNT_CMP1_MASK (0xF << CRC_VRF_CNT_CMP1_SHIFT)
#define CRC_VRF_CNT_CMP0_MASK (0xF << CRC_VRF_CNT_CMP0_SHIFT)
#define CRC_VRF_CNT_REF1_MASK (0xF << CRC_VRF_CNT_REF1_SHIFT)
#define CRC_VRF_CNT_REF0_MASK (0xF << CRC_VRF_CNT_REF0_SHIFT)

/*
 * Tx splitter sync signal polarity
 */
#define SAL_DE_SHIFT (2)
#define SAL_HS_SHIFT (1)
#define SAL_VS_SHIFT (0)

#define SAL_DE_MASK (0x1 << SAL_DE_SHIFT)
#define SAL_HS_MASK (0x1 << SAL_HS_SHIFT)
#define SAL_VS_MASK (0x1 << SAL_VS_SHIFT)

/*
 * Clock checker reference clock count
 */
#define REF_CLK_CNT_SHIFT (0)
#define REF_CLK_CNT_MAX (0xFFFF << REF_CLK_CNT_SHIFT)

/*
 * Clock checker compare clock count
 */
#define CMP_CLK_MIN_SHIFT (16)
#define CMP_CLK_MAX_SHIFT (0)

#define CMP_CLK_MIN_MASK (0xFFFF << CMP_CLK_CNT_MIN_SHIFT)
#define CMP_CLK_MAX_MASK (0xFFFF << CMP_CLK_CNT_MAX_SHIFT)

/*
 * Hsync checker configuration 0
 */
#define HSYNC_CFG_0_HSYNC_SHIFT (16)
#define HSYNC_CFG_0_HFP_SHIFT (0)

#define HSYNC_CFG_0_HSYNC_MASK (0xFFFF << HSYNC_CFG_0_HSYNC_SHIFT)
#define HSYNC_CFG_0_HFP_MASK (0xFFFF << HSYNC_CFG_0_HFP_SHIFT)

/*
 * Hsync checker configuration 1
 */
#define HSYNC_CFG_1_HACTIVE_SHIFT (16)
#define HSYNC_CFG_1_HBP_SHIFT (0)

#define HSYNC_CFG_1_HACTIVE_MASK (0xFFFF << HSYNC_CFG_1_HACTIVE_SHIFT)
#define HSYNC_CFG_1_HBP_MASK (0xFFFF << HSYNC_CFG_1_HBP_SHIFT)

/*
 * Hsync checker configuration 2
 */
#define HSYNC_CFG_2_HACTIVEM_SHIFT (24)
#define HSYNC_CFG_2_HBPM_SHIFT (16)
#define HSYNC_CFG_2_HSYNCM_SHIFT (8)
#define HSYNC_CFG_2_HFPM_SHIFT (0)

#define HSYNC_CFG_2_HACTIVE_MASK (0xF << HSYNC_CFG_2_HACTIVEM_SHIFT)
#define HSYNC_CFG_2_HBPM_MASK (0xF << HSYNC_CFG_2_HBPM_SHIFT)
#define HSYNC_CFG_2_HSYNCM_MASK (0xF << HSYNC_CFG_2_HSYNCM_SHIFT)
#define HSYNC_CFG_2_HFPM_MASK (0xF << HSYNC_CFG_2_HBPM_SHIFT)

/*
 * VSYNC checker configuration 0
 */
#define VSYNC_CFG_0_VSYNC_SHIFT (16)
#define VSYNC_CFG_0_VFP_SHIFT (0)

#define VSYNC_CFG_0_VSYNC_MASK (0xFFFF << VSYNC_CFG_0_VSYNC_SHIFT)
#define VSYNC_CFG_0_VFP_MASK (0xFFFF << VSYNC_CFG_0_VFP_SHIFT)

/*
 * Vsync checker configuration 1
 */
#define VSYNC_CFG_1_VACTIVE_SHIFT (16)
#define VSYNC_CFG_1_VBP_SHIFT (0)

#define VSYNC_CFG_1_VACTIVE_MASK (0xFFFF << VSYNC_CFG_1_VACTIVE_SHIFT)
#define VSYNC_CFG_1_VBP_MASK (0xFFFF << VSYNC_CFG_1_VBP_SHIFT)

/*
 * Vsync checker configuration 2
 */
#define VSYNC_CFG_2_VACTIVEM_SHIFT (24)
#define VSYNC_CFG_2_VBPM_SHIFT (16)
#define VSYNC_CFG_2_VSYNCM_SHIFT (8)
#define VSYNC_CFG_2_VFPM_SHIFT (0)

#define VSYNC_CFG_2_VACTIVE_MASK (0xF << VSYNC_CFG_2_VACTIVEM_SHIFT)
#define VSYNC_CFG_2_VBPM_MASK (0xF << VSYNC_CFG_2_VBPM_SHIFT)
#define VSYNC_CFG_2_VSYNCM_MASK (0xF << VSYNC_CFG_2_VSYNCM_SHIFT)
#define VSYNC_CFG_2_VFPM_MASK (0xF << VSYNC_CFG_2_VFPM_SHIFT)

/*
 * LVDS WRAP configuration protect
 */
#define CFG_PROTECT_SHIFT (0)

#define CFG_PROTECT_MASK (0xFFFFFFFFU << CFG_PROTECT_SHIFT)

/*
 * Tx splitter interrupt clear
 */
#define TS_IREQ_CLEAR_HSYNC1_TM_SHIFT (12)
#define TS_IREQ_CLEAR_HSYNC0_TM_SHIFT (11)
#define TS_IREQ_CLEAR_VSYNC1_TM_SHIFT (10)
#define TS_IREQ_CLEAR_VSYNC0_TM_SHIFT (9)
#define TS_IREQ_CLEAR_CLK1_TM_SHIFT (8)
#define TS_IREQ_CLEAR_CLK0_TM_SHIFT (7)
#define TS_IREQ_CLEAR_HSYNC1_ERR_SHIFT (6)
#define TS_IREQ_CLEAR_HSYNC0_ERR_SHIFT (5)
#define TS_IREQ_CLEAR_VSYNC1_ERR_SHIFT (4)
#define TS_IREQ_CLEAR_VSYNC0_ERR_SHIFT (3)
#define TS_IREQ_CLEAR_CLK1_ERR_SHIFT (2)
#define TS_IREQ_CLEAR_CLK0_ERR_SHIFT (1)
#define TS_IREQ_CLEAR_CRC_FAIL_SHIFT (0)

#define TS_IREQ_CLEAR_HSYNC1_TM_MASK (0x1 << TS_IREQ_CLEAR_HSYNC1_TM_SHIFT)
#define TS_IREQ_CLEAR_HSYNC0_TM_MASK (0x1 << TS_IREQ_CLEAR_HSYNC0_TM_SHIFT)
#define TS_IREQ_CLEAR_VSYNC1_TM_MASK (0x1 << TS_IREQ_CLEAR_VSYNC1_TM_SHIFT)
#define TS_IREQ_CLEAR_VSYNC0_TM_MASK (0x1 << TS_IREQ_CLEAR_VSYNC0_TM_SHIFT)
#define TS_IREQ_CLEAR_CLK1_TM_MASK (0x1 << TS_IREQ_CLEAR_CLK1_TM_SHIFT)
#define TS_IREQ_CLEAR_CLK0_TM_MASK (0x1 << TS_IREQ_CLEAR_CLK0_TM_SHIFT)
#define TS_IREQ_CLEAR_HSYNC1_ERR_MASK (0x1 << TS_IREQ_CLEAR_HSYNC1_ERR_SHIFT)
#define TS_IREQ_CLEAR_HSYNC0_ERR_MASK (0x1 << TS_IREQ_CLEAR_HSYNC0_ERR_SHIFT)
#define TS_IREQ_CLEAR_VSYNC1_ERR_MASK (0x1 << TS_IREQ_CLEAR_VSYNC1_ERR_SHIFT)
#define TS_IREQ_CLEAR_VSYNC0_ERR_MASK (0x1 << TS_IREQ_CLEAR_VSYNC0_ERR_SHIFT)
#define TS_IREQ_CLEAR_CLK1_ERR_MASK (0x1 << TS_IREQ_CLEAR_CLK1_ERR_SHIFT)
#define TS_IREQ_CLEAR_CLK0_ERR_MASK (0x1 << TS_IREQ_CLEAR_CLK0_ERR_SHIFT)
#define TS_IREQ_CLEAR_CRC_FAIL_MASK (0x1 << TS_IREQ_CLEAR_CRC_FAIL_SHIFT)

/*
 * Tx splitter interrupt status
 */
#define TS_IREQ_STATUS_HSYNC1_TM_SHIFT (12)
#define TS_IREQ_STATUS_HSYNC0_TM_SHIFT (11)
#define TS_IREQ_STATUS_VSYNC1_TM_SHIFT (10)
#define TS_IREQ_STATUS_VSYNC0_TM_SHIFT (9)
#define TS_IREQ_STATUS_CLK1_TM_SHIFT (8)
#define TS_IREQ_STATUS_CLK0_TM_SHIFT (7)
#define TS_IREQ_STATUS_HSYNC1_ERR_SHIFT (6)
#define TS_IREQ_STATUS_HSYNC0_ERR_SHIFT (5)
#define TS_IREQ_STATUS_VSYNC1_ERR_SHIFT (4)
#define TS_IREQ_STATUS_VSYNC0_ERR_SHIFT (3)
#define TS_IREQ_STATUS_CLK1_ERR_SHIFT (2)
#define TS_IREQ_STATUS_CLK0_ERR_SHIFT (1)
#define TS_IREQ_STATUS_CRC_FAIL_SHIFT (0)

#define TS_IREQ_STATUS_HSYNC1_TM_MASK (0x1 << TS_IREQ_STATUS_HSYNC1_TM_SHIFT)
#define TS_IREQ_STATUS_HSYNC0_TM_MASK (0x1 << TS_IREQ_STATUS_HSYNC0_TM_SHIFT)
#define TS_IREQ_STATUS_VSYNC1_TM_MASK (0x1 << TS_IREQ_STATUS_VSYNC1_TM_SHIFT)
#define TS_IREQ_STATUS_VSYNC0_TM_MASK (0x1 << TS_IREQ_STATUS_VSYNC0_TM_SHIFT)
#define TS_IREQ_STATUS_CLK1_TM_MASK (0x1 << TS_IREQ_STATUS_CLK1_TM_SHIFT)
#define TS_IREQ_STATUS_CLK0_TM_MASK (0x1 << TS_IREQ_STATUS_CLK0_TM_SHIFT)
#define TS_IREQ_STATUS_HSYNC1_ERR_MASK (0x1 << TS_IREQ_STATUS_HSYNC1_ERR_SHIFT)
#define TS_IREQ_STATUS_HSYNC0_ERR_MASK (0x1 << TS_IREQ_STATUS_HSYNC0_ERR_SHIFT)
#define TS_IREQ_STATUS_VSYNC1_ERR_MASK (0x1 << TS_IREQ_STATUS_VSYNC1_ERR_SHIFT)
#define TS_IREQ_STATUS_VSYNC0_ERR_MASK (0x1 << TS_IREQ_STATUS_VSYNC0_ERR_SHIFT)
#define TS_IREQ_STATUS_CLK1_ERR_MASK (0x1 << TS_IREQ_STATUS_CLK1_ERR_SHIFT)
#define TS_IREQ_STATUS_CLK0_ERR_MASK (0x1 << TS_IREQ_STATUS_CLK0_ERR_SHIFT)
#define TS_IREQ_STATUS_CRC_FAIL_MASK (0x1 << TS_IREQ_STATUS_CRC_FAIL_SHIFT)

/*
 * LVDS WRAP soft reset
 */
#define SWRESET_LVDS_PHY_2PORT_SYS_SHIFT (4)
#define SWRESET_LVDS_PHY_2PORT_APB_SHIFT (2)
#define SWRESET_TS_SHIFT (0)

#define SWRESET_LVDS_PHY_2PORT_SYS_MASK \
	(0x1 << SWRESET_LVDS_PHY_2PORT_SYS_SHIFT)
#define SWRESET_LVDS_PHY_2PORT_APB_MASK \
	(0x1 << SWRESET_LVDS_PHY_2PORT_APB_SHIFT)
#define SWRESET_TS_MASK (0x1 << SWRESET_TS_SHIFT)

/*
 * LVDS Phy open short interrupt clear
 */
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D4P_SHIFT (29)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D4M_SHIFT (28)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D3P_SHIFT (27)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D3M_SHIFT (26)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D2P_SHIFT (25)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D2M_SHIFT (24)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D1P_SHIFT (23)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D1M_SHIFT (22)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D0P_SHIFT (21)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D0M_SHIFT (20)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D4P_SHIFT (19)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D4M_SHIFT (18)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D3P_SHIFT (17)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D3M_SHIFT (16)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D2P_SHIFT (15)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D2M_SHIFT (14)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D1P_SHIFT (13)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D1M_SHIFT (12)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D0P_SHIFT (11)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D0M_SHIFT (10)

#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D4P_SHIFT (9)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D4M_SHIFT (8)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D3P_SHIFT (7)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D3M_SHIFT (6)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D2P_SHIFT (5)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D2M_SHIFT (4)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D1P_SHIFT (3)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D1M_SHIFT (2)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D0P_SHIFT (1)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D0M_SHIFT (0)

#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D4P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS1_OS_DET_D4P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D4M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS1_OS_DET_D4M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D3P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS1_OS_DET_D3P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D3M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS1_OS_DET_D3M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D2P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS1_OS_DET_D2P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D2M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS1_OS_DET_D2M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D1P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS1_OS_DET_D1P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D1M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS1_OS_DET_D1M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D0P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS1_OS_DET_D0P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS1_OS_DET_D0M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS1_OS_DET_D0M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D4P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS0_OS_DET_D4P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D4M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS0_OS_DET_D4M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D3P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS0_OS_DET_D3P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D3M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS0_OS_DET_D3M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D2P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS0_OS_DET_D2P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D2M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS0_OS_DET_D2M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D1P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS0_OS_DET_D1P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D1M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS0_OS_DET_D1M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D0P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS0_OS_DET_D0P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS0_OS_DET_D0M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS0_OS_DET_D0M_SHIFT)

#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D4P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS2_OS_DET_D4P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D4M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS2_OS_DET_D4M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D3P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS2_OS_DET_D3P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D3M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS2_OS_DET_D3M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D2P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS2_OS_DET_D2P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D2M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS2_OS_DET_D2M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D1P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS2_OS_DET_D1P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D1M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS2_OS_DET_D1M_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D0P_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS2_OS_DET_D0P_SHIFT)
#define LVDS_IREQ_CLEAR_LVDS2_OS_DET_D0M_MASK \
	(0x1 << LVDS_IREQ_CLEAR_LVDS2_OS_DET_D0M_SHIFT)

/*
 * LVDS Phy open short interrupt status
 */
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D4P_SHIFT (29)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D4M_SHIFT (28)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D3P_SHIFT (27)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D3M_SHIFT (26)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D2P_SHIFT (25)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D2M_SHIFT (24)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D1P_SHIFT (23)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D1M_SHIFT (22)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D0P_SHIFT (21)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D0M_SHIFT (20)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D4P_SHIFT (19)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D4M_SHIFT (18)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D3P_SHIFT (17)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D3M_SHIFT (16)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D2P_SHIFT (15)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D2M_SHIFT (14)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D1P_SHIFT (13)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D1M_SHIFT (12)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D0P_SHIFT (11)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D0M_SHIFT (10)

#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D4P_SHIFT (9)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D4M_SHIFT (8)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D3P_SHIFT (7)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D3M_SHIFT (6)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D2P_SHIFT (5)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D2M_SHIFT (4)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D1P_SHIFT (3)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D1M_SHIFT (2)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D0P_SHIFT (1)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D0M_SHIFT (0)

#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D4P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS1_OS_DET_D4P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D4M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS1_OS_DET_D4M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D3P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS1_OS_DET_D3P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D3M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS1_OS_DET_D3M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D2P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS1_OS_DET_D2P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D2M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS1_OS_DET_D2M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D1P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS1_OS_DET_D1P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D1M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS1_OS_DET_D1M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D0P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS1_OS_DET_D0P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS1_OS_DET_D0M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS1_OS_DET_D0M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D4P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS0_OS_DET_D4P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D4M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS0_OS_DET_D4M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D3P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS0_OS_DET_D3P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D3M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS0_OS_DET_D3M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D2P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS0_OS_DET_D2P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D2M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS0_OS_DET_D2M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D1P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS0_OS_DET_D1P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D1M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS0_OS_DET_D1M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D0P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS0_OS_DET_D0P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS0_OS_DET_D0M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS0_OS_DET_D0M_SHIFT)

#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D4P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS2_OS_DET_D4P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D4M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS2_OS_DET_D4M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D3P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS2_OS_DET_D3P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D3M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS2_OS_DET_D3M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D2P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS2_OS_DET_D2P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D2M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS2_OS_DET_D2M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D1P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS2_OS_DET_D1P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D1M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS2_OS_DET_D1M_SHIFT)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D0P_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS2_OS_DET_D0P_SHIFT)
#define LVDS_IREQ_STATUS_LVDS2_OS_DET_D0M_MASK \
	(0x1 << LVDS_IREQ_STATUS_LVDS2_OS_DET_D0M_SHIFT)

/*
 * Tx splitter interrupt mask
 */
#define TS_IREQ_MASK_HSYNC1_TM_SHIFT (12)
#define TS_IREQ_MASK_HSYNC0_TM_SHIFT (11)
#define TS_IREQ_MASK_VSYNC1_TM_SHIFT (10)
#define TS_IREQ_MASK_VSYNC0_TM_SHIFT (9)
#define TS_IREQ_MASK_CLK1_TM_SHIFT (8)
#define TS_IREQ_MASK_CLK0_TM_SHIFT (7)
#define TS_IREQ_MASK_HSYNC1_ERR_SHIFT (6)
#define TS_IREQ_MASK_HSYNC0_ERR_SHIFT (5)
#define TS_IREQ_MASK_VSYNC1_ERR_SHIFT (4)
#define TS_IREQ_MASK_VSYNC0_ERR_SHIFT (3)
#define TS_IREQ_MASK_CLK1_ERR_SHIFT (2)
#define TS_IREQ_MASK_CLK0_ERR_SHIFT (1)
#define TS_IREQ_MASK_CRC_FAIL_SHIFT (0)

#define TS_IREQ_MASK_HSYNC1_TM_MASK (0x1 << TS_IREQ_MASK_HSYNC1_TM_SHIFT)
#define TS_IREQ_MASK_HSYNC0_TM_MASK (0x1 << TS_IREQ_MASK_HSYNC0_TM_SHIFT)
#define TS_IREQ_MASK_CLK1_TM_MASK (0x1 << TS_IREQ_MASK_CLK1_TM_SHIFT)
#define TS_IREQ_MASK_CLK0_TM_MASK (0x1 << TS_IREQ_MASK_CLK0_TM_SHIFT)
#define TS_IREQ_MASK_HSYNC1_ERR_MASK (0x1 << TS_IREQ_MASK_HSYNC1_ERR_SHIFT)
#define TS_IREQ_MASK_HSYNC0_ERR_MASK (0x1 << TS_IREQ_MASK_HSYNC0_ERR_SHIFT)
#define TS_IREQ_MASK_VSYNC1_ERR_MASK (0x1 << TS_IREQ_MASK_VSYNC1_ERR_SHIFT)
#define TS_IREQ_MASK_VSYNC0_ERR_MASK (0x1 << TS_IREQ_MASK_VSYNC0_ERR_SHIFT)
#define TS_IREQ_MASK_CLK1_ERR_MASK (0x1 << TS_IREQ_MASK_CLK1_ERR_SHIFT)
#define TS_IREQ_MASK_CLK0_ERR_MASK (0x1 << TS_IREQ_MASK_CLK0_ERR_SHIFT)
#define TS_IREQ_MASK_CRC_FAIL_MASK (0x1 << TS_IREQ_MASK_CRC_FAIL_SHIFT)

/*
 * LVDS Phy open short interrupt mask
 */
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D4P_SHIFT (29)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D4M_SHIFT (28)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D3P_SHIFT (27)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D3M_SHIFT (26)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D2P_SHIFT (25)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D2M_SHIFT (24)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D1P_SHIFT (23)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D1M_SHIFT (22)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D0P_SHIFT (21)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D0M_SHIFT (20)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D4P_SHIFT (19)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D4M_SHIFT (18)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D3P_SHIFT (17)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D3M_SHIFT (16)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D2P_SHIFT (15)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D2M_SHIFT (14)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D1P_SHIFT (13)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D1M_SHIFT (12)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D0P_SHIFT (11)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D0M_SHIFT (10)

#define LVDS_IREQ_MASK_LVDS2_OS_DET_D4P_SHIFT (9)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D4M_SHIFT (8)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D3P_SHIFT (7)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D3M_SHIFT (6)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D2P_SHIFT (5)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D2M_SHIFT (4)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D1P_SHIFT (3)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D1M_SHIFT (2)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D0P_SHIFT (1)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D0M_SHIFT (0)

#define LVDS_IREQ_MASK_LVDS1_OS_DET_D4P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS1_OS_DET_D4P_SHIFT)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D4M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS1_OS_DET_D4M_SHIFT)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D3P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS1_OS_DET_D3P_SHIFT)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D3M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS1_OS_DET_D3M_SHIFT)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D2P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS1_OS_DET_D2P_SHIFT)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D2M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS1_OS_DET_D2M_SHIFT)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D1P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS1_OS_DET_D1P_SHIFT)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D1M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS1_OS_DET_D1M_SHIFT)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D0P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS1_OS_DET_D0P_SHIFT)
#define LVDS_IREQ_MASK_LVDS1_OS_DET_D0M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS1_OS_DET_D0M_SHIFT)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D4P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS0_OS_DET_D4P_SHIFT)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D4M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS0_OS_DET_D4M_SHIFT)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D3P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS0_OS_DET_D3P_SHIFT)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D3M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS0_OS_DET_D3M_SHIFT)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D2P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS0_OS_DET_D2P_SHIFT)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D2M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS0_OS_DET_D2M_SHIFT)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D1P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS0_OS_DET_D1P_SHIFT)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D1M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS0_OS_DET_D1M_SHIFT)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D0P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS0_OS_DET_D0P_SHIFT)
#define LVDS_IREQ_MASK_LVDS0_OS_DET_D0M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS0_OS_DET_D0M_SHIFT)

#define LVDS_IREQ_MASK_LVDS2_OS_DET_D4P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS2_OS_DET_D4P_SHIFT)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D4M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS2_OS_DET_D4M_SHIFT)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D3P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS2_OS_DET_D3P_SHIFT)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D3M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS2_OS_DET_D3M_SHIFT)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D2P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS2_OS_DET_D2P_SHIFT)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D2M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS2_OS_DET_D2M_SHIFT)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D1P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS2_OS_DET_D1P_SHIFT)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D1M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS2_OS_DET_D1M_SHIFT)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D0P_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS2_OS_DET_D0P_SHIFT)
#define LVDS_IREQ_MASK_LVDS2_OS_DET_D0M_MASK \
	(0x1 << LVDS_IREQ_MASK_LVDS2_OS_DET_D0M_SHIFT)

/*
 * SM_BYPASS(SDM/SRVC Bypass)
 */
#define SHIFT_SDC 0 /* SDM bypass configuration */
#define SHIFT_SRC 1 /* SRVC bypass configuration */
#define SHIFT_SDB 2 /* SDM bypass status */
#define SHIFT_SRB 3 /* SRVC bypass status */
#define SHIFT_SDO 4 /* SDM bypass OTP */
#define SHIFT_SRO 5 /* SRVC bypass OTP */

enum {
	TS_TXOUT_SEL0 = 0,
	TS_TXOUT_SEL1,
	TS_TXOUT_SEL2,
	TS_TXOUT_SEL3,
	TS_TXOUT_SEL4,
	TS_TXOUT_SEL5,
	TS_TXOUT_SEL6,
	TS_TXOUT_SEL7,
	TS_TXOUT_SEL8,
	TS_TXOUT_SEL_MAX
}; //TS_TXOUT_SEL

enum {
	TS_SWAP_CH0 = 0,
	TS_SWAP_CH1,
	TS_SWAP_CH2,
	TS_SWAP_CH3,
	TS_SWAP_CH_MAX
}; //TS_SWAP_CH

enum { DISP_MUX_TYPE = 0, TS_MUX_TYPE, MUX_TYPE_MAX }; //MUX_TYPE

enum {
	TS_MUX_PATH_DISP0 = 0,
	TS_MUX_PATH_DISP1,
	TS_MUX_PATH_DISP2,
	TS_MUX_PATH_DISP3,
	TS_MUX_PATH_CORE,
	TS_MUX_PATH_MAX
}; //TS_MUX_PATH

enum {
	TS_MUX_IDX0 = 0,
	TS_MUX_IDX1,
	TS_MUX_IDX2,
	TS_MUX_IDX3,
	TS_MUX_IDX_MAX,
}; //TS_MUX_IDX

enum {
	LCDC_MUX_IDX0 = 0,
	LCDC_MUX_IDX1,
	LCDC_MUX_IDX2,
	LCDC_MUX_IDX_MAX,
}; //LCDC_MUX_IDX

#define TXOUT_DUMMY (0x1F)
#define TXOUT_DE (24)
#define TXOUT_HS (25)
#define TXOUT_VS (26)
#define TXOUT_R_D(x) ((x) + 0x10)
#define TXOUT_G_D(x) ((x) + 0x8)
#define TXOUT_B_D(x) (x)

#define TXOUT_MAX_LINE 4
#define TXOUT_DATA_PER_LINE 7
#define TXOUT_GET_DATA(i)                                        \
	((TXOUT_DATA_PER_LINE - 1) - ((i) % TXOUT_DATA_PER_LINE) \
	 + (TXOUT_DATA_PER_LINE * ((i) / TXOUT_DATA_PER_LINE)))

extern void LVDS_WRAP_SetDataArray(
	unsigned int ch,
	unsigned int data[TXOUT_MAX_LINE][TXOUT_DATA_PER_LINE]);
extern void LVDS_WRAP_SetConfigure(
	unsigned int lr, unsigned int bypass, unsigned int width);
extern void LVDS_WRAP_SetDataSwap(unsigned int ch, unsigned int set);
extern void LVDS_WRAP_SetMuxOutput(
	unsigned int mux, unsigned int ch, unsigned int select,
	unsigned int enable);
extern void LVDS_WRAP_SetAccessCode(void);
extern void LVDS_WRAP_ResetPHY(unsigned int port, unsigned int reset);
extern void LVDS_WRAP_SetSyncPolarity(unsigned int sync);
void LVDS_WRAP_SM_Bypass(unsigned int lcdc_mux_id, unsigned int en);
#endif
