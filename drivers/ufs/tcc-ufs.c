// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

/**
 * tcc-ufs.c - Platform driver for Telechips UFSHCI device
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <ufs.h>

#include "ufs.h"
#include "tcc-ufs.h"

//-------------------------------------------------
//Macro Definition
//-------------------------------------------------
//#define reg_setb(x,y) (x|=(1U<<y))
//#define reg_clrb(x,y) (x&=~(1U<<y))
#define reg_rdb(x, y)  (((0U) == ((x) & ((1UL) << (y)))) ? (0U) : (1U))
#define SET_FMP(x)	  ((x) + (0x10000UL))
#define SET_UNIPRO(addr, val) writel((val),\
	(void *)(unsigned int *)(unsigned int)((0x1D270000U) + (addr)))

static int tcc_ufs_smu_setting(struct ufs_hba *hba);

static void tcc_ufs_post_linkup(struct ufs_hba *hba)
{
	//unsigned int wlu_enable = 0;
	//unsigned int wlu_burst_len = 3;
	//unsigned int hci_buffering_enable = 0;
	unsigned int axidma_rwdataburstlen;
	unsigned int no_of_beat_burst = 7;

	// Link Start Up Complete. Now initializing
	// UTP transfer management request list base address
	ufshcd_writel(hba, UTRIACR_VAL, REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL);
	ufshcd_writel(hba, UTMRLBA_LOW_VAL, REG_UTP_TASK_REQ_LIST_BASE_L);
	ufshcd_writel(hba, UTMRLBA_HIGH_VAL, REG_UTP_TASK_REQ_LIST_BASE_H);

	ufshcd_writel(hba, UTRLBA_LOW_VAL, REG_UTP_TRANSFER_REQ_LIST_BASE_L);
	ufshcd_writel(hba, UTRLBA_HIGH_VAL, REG_UTP_TRANSFER_REQ_LIST_BASE_H);
	ufshcd_writel(hba, UTMRLBA_LOW_VAL, REG_UTP_TASK_REQ_LIST_BASE_L);
	ufshcd_writel(hba, UTMRLBA_HIGH_VAL, REG_UTP_TASK_REQ_LIST_BASE_H);

	ufshcd_writel(hba, TXPRDT_ENTRY_SIZE_VAL, HCI_TXPRDT_ENTRY_SIZE);
	ufshcd_writel(hba, RXPRDT_ENTRY_SIZE_VAL, HCI_RXPRDT_ENTRY_SIZE);

	ufshcd_writel(hba, 0x1, REG_UTP_TASK_REQ_LIST_RUN_STOP);
	ufshcd_writel(hba, 0x1, REG_UTP_TRANSFER_REQ_LIST_RUN_STOP);

	// Write Line Unique Feature Enable/Disable
	//if(wlu_enable == 1U) {
	//	dev_dbg(&hba->dev,
	//		"BASIC_LINK_UP::Write Line Unique Feature Enable");
	//	axidma_rwdataburstlen = (1u<<31) | (wlu_burst_len<<27) | 0x3;
	//} else {
		axidma_rwdataburstlen =
	    ((unsigned int)0 << 31) | ((unsigned int)0 << 27) |
	    no_of_beat_burst;
	//}

	//if(hci_buffering_enable == 1) {
	//	dev_dbg(&hba->dev,
	//			"BASIC_LINK_UP::Hci Buffering Feature Enable");
	//	axidma_rwdataburstlen =
	//		(axidma_rwdataburstlen & 0xFFFFFFF0) | no_of_beat_burst;
	//} else {
		axidma_rwdataburstlen =
			(axidma_rwdataburstlen & 0xFFFFFFF0U) | 0x0U;
	//}

	//if((hci_buffering_enable == 1) || (wlu_enable == 1)) {
	//	ufshcd_writel(hba, axidma_rwdataburstlen,
	//			HCI_AXIDMA_RWDATA_BURST_LEN);
	//}
	tcc_ufs_smu_setting(hba);
}

static int tcc_ufs_link_startup_notify(struct ufs_hba *hba,
				       enum ufs_notify_change_status status)
{
	hba->quirks |= (unsigned int)UFSHCD_QUIRK_BROKEN_LCC;
	switch (status) {
	case PRE_CHANGE:
		break;
	case POST_CHANGE:
		tcc_ufs_post_linkup(hba);
		break;
	default:
		break;
	}

	return 0;
}

static void tcc_ufs_pre_init(struct ufs_hba *hba)
{
	ufshcd_writel(hba, 0x0, HCI_CLKSTOP_CTRL);
	ufshcd_writel(hba, 0x1, HCI_GPIO_OUT);
	ufshcd_writel(hba, 0x1, HCI_UFS_ACG_DISABLE);
}

static void tcc_ufs_post_init(struct ufs_hba *hba)
{
	//int res=0;
	unsigned int data = 0;

	data = ufshcd_readl(hba, REG_CONTROLLER_ENABLE);
	while (data != 0x1U) {
		data = ufshcd_readl(hba, REG_CONTROLLER_ENABLE);
	}

	data = ufshcd_readl(hba, REG_CONTROLLER_STATUS);
	while (reg_rdb((data), (3U)) != 1U) {
		data = ufshcd_readl(hba, REG_CONTROLLER_STATUS);
	}

	ufshcd_writel(hba, 0x7FFF, REG_INTERRUPT_ENABLE);

	//Initial Set of some MPHY registers
#if 0
	ufshcd_dme_set(hba,
		UIC_ARG_MIB_SEL(0x9A, UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0)), 0x0F);
	ufshcd_dme_set(hba,
		UIC_ARG_MIB_SEL(0x200, UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)), 0x40);
	ufshcd_dme_set(hba,
		UIC_ARG_MIB_SEL(0x9A, UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0)), 0x0F);
	ufshcd_dme_set(hba,
		UIC_ARG_MIB_SEL(0x9A, UIC_ARG_MPHY_RX_GEN_SEL_INDEX(1)), 0x0F);
	ufshcd_dme_set(hba,
		UIC_ARG_MIB_SEL(0x93, UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0)), 0xFF);
	ufshcd_dme_set(hba,
		UIC_ARG_MIB_SEL(0x93, UIC_ARG_MPHY_RX_GEN_SEL_INDEX(1)), 0xFF);
	ufshcd_dme_set(hba,
		UIC_ARG_MIB_SEL(0x99, UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0)), 0xFF);
	ufshcd_dme_set(hba,
		UIC_ARG_MIB_SEL(0x99, UIC_ARG_MPHY_RX_GEN_SEL_INDEX(1)), 0xFF);
	ufshcd_dme_set(hba,
		UIC_ARG_MIB_SEL(0x8F, UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)), 0x3A);
	ufshcd_dme_set(hba,
		UIC_ARG_MIB_SEL(0x8F, UIC_ARG_MPHY_TX_GEN_SEL_INDEX(1)), 0x3A);

	ufshcd_dme_set(hba,
		UIC_ARG_MIB_SEL(0x200, UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)), 0x0);
#endif
	// Configuration regarding real time specification
	//ufshcd_dme_set(hba,
	//UIC_ARG_MIB_SEL(PA_DBG_CLK_PERIOD, UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)),
	//UNIPRO_PCLK_PERIOD_NS);
	//ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_DBG_AUTOMODE_THLD,
	//UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)), CALCULATED_VALUE);
	//ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(PA_DBG_OPTION_SUITE,
	//UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)), 0x2E8201C3);
	SET_UNIPRO((PA_DBG_CLK_PERIOD), (UNIPRO_PCLK_PERIOD_NS));
	SET_UNIPRO((PA_DBG_AUTOMODE_THLD), (CALCULATED_VALUE));
	SET_UNIPRO((PA_DBG_OPTION_SUITE), (0x2E8201C3U));
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)PA_Local_TX_LCC_Enable,
				       (unsigned int)
				       UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)),
		       (unsigned int)0x0);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)N_DeviceID, (unsigned int)
				       UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)),
		       (unsigned int)N_DEVICE_ID_VAL);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)N_DeviceID_valid,
				       (unsigned int)
				       UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)),
		       (unsigned int)N_DEVICE_ID_VALID_VAL);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)T_ConnectionState,
				       (unsigned int)
				       UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)),
		       (unsigned int)T_CONNECTION_STATE_OFF_VAL);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)T_PeerDeviceID,
				       (unsigned int)
				       UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)),
		       (unsigned int)T_PEER_DEVICE_ID_VAL);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)T_ConnectionState,
				       (unsigned int)
				       UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0)),
		       (unsigned int)T_CONNECTION_STATE_ON_VAL);

	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x200,
				       UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0U)),
		       0x40U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x8f,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0U)),
		       0x3fU);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x8f,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(1U)),
		       0x3fU);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x0f,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0U)),
		       0x00U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x0f,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(1U)),
		       0x00U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x21,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0U)),
		       0x00U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x21,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(1U)),
		       0x00U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x22,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0U)),
		       0x00U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x22,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(1U)),
		       0x00U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x5c,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0U)),
		       0x38U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x5c,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(1U)),
		       0x38U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x62,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0U)),
		       0x97U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x62,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(1U)),
		       0x97U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x63,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0U)),
		       0x70U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x63,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(1U)),
		       0x70U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x65,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0U)),
		       0x01U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x65,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(1U)),
		       0x01U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x69,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(0U)),
		       0x01U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x69,
				       UIC_ARG_MPHY_RX_GEN_SEL_INDEX(1U)),
		       0x01U);
	ufshcd_dme_set(hba,
		       UIC_ARG_MIB_SEL((unsigned int)0x200,
				       UIC_ARG_MPHY_TX_GEN_SEL_INDEX(0U)),
		       0x0U);

	ufshcd_writel(hba, 0xA, HCI_DATA_REORDER);
	ufshcd_writel(hba, 0xDFC2E492, FMP_FMPRSECURITY);
}

static int tcc_ufs_hce_enable_notify(struct ufs_hba *hba,
				     enum ufs_notify_change_status status)
{
	switch (status) {
	case PRE_CHANGE:
		tcc_ufs_pre_init(hba);
		break;
	case POST_CHANGE:
		tcc_ufs_post_init(hba);
		break;
	default:
		break;
	}

	return 0;
}

static void encryption_setting(struct ufs_hba *hba)
{
	ufshcd_writel(hba, 0x0, HCI_CRYPTOCFG__CFGE_CAPIDX_DUSIZE);
	ufshcd_writel(hba, 0x00000003, HCI_CRYPTOCFG_CRYPTOKEY_0);
	ufshcd_writel(hba, 0x0000000F, HCI_CRYPTOCFG_CRYPTOKEY_1);
	ufshcd_writel(hba, 0x00000030, HCI_CRYPTOCFG_CRYPTOKEY_2);
	ufshcd_writel(hba, 0x000000F0, HCI_CRYPTOCFG_CRYPTOKEY_3);
	ufshcd_writel(hba, 0x00000300, HCI_CRYPTOCFG_CRYPTOKEY_4);
	ufshcd_writel(hba, 0x00000F00, HCI_CRYPTOCFG_CRYPTOKEY_5);
	ufshcd_writel(hba, 0x00003000, HCI_CRYPTOCFG_CRYPTOKEY_6);
	ufshcd_writel(hba, 0x0000F000, HCI_CRYPTOCFG_CRYPTOKEY_7);
	ufshcd_writel(hba, 0x00030000, HCI_CRYPTOCFG_CRYPTOKEY_8);
	ufshcd_writel(hba, 0x000F0000, HCI_CRYPTOCFG_CRYPTOKEY_9);
	ufshcd_writel(hba, 0x00300000, HCI_CRYPTOCFG_CRYPTOKEY_A);
	ufshcd_writel(hba, 0x00F00000, HCI_CRYPTOCFG_CRYPTOKEY_B);
	ufshcd_writel(hba, 0x03000000, HCI_CRYPTOCFG_CRYPTOKEY_C);
	ufshcd_writel(hba, 0x0F000000, HCI_CRYPTOCFG_CRYPTOKEY_D);
	ufshcd_writel(hba, 0x30000000, HCI_CRYPTOCFG_CRYPTOKEY_E);
	ufshcd_writel(hba, 0xF0000000, HCI_CRYPTOCFG_CRYPTOKEY_F);
}

static int tcc_ufs_smu_setting(struct ufs_hba *hba)
{
	//unsigned int smu_bypass = 1;
	//unsigned int smu_index = 0;
	unsigned int desc_type = 0;
	unsigned int tid, sw, sr, nsw, nsr, ufk, enc, valid;
	unsigned int protbytzpc, select_inline_enc, fmp_on, unmap_disable,
	    nskeyreg, nssmu, nsuser, use_otp_mask;
	unsigned int fmp_bsector;
	unsigned int fmp_esector;
	unsigned int fmp_lun;
	unsigned int fmp_ctrl;
	unsigned int fmp_security;

	encryption_setting(hba);
	tid = 0;
	//sw    = (smu_bypass)? 1 : 1;
	sw = 1;
	//sr    = (smu_bypass)? 1 : 1;
	sr = 1;
	//nsw   = (smu_bypass)? 1 : 1;
	nsw = 1;
	//nsr   = (smu_bypass)? 1 : 1;
	nsr = 1;
	//ufk   = (smu_bypass)? 1 : 1;
	ufk = 1;
	//enc   = (smu_bypass)? 0 : 1;
	enc = 0;
	//valid = (smu_bypass)? ((smu_index==0)? 1 : 0) : 1;
	valid = 1;
	fmp_bsector = 0x0;
	fmp_esector = 0xFFFFFFFF;
	fmp_lun = 0x7;
	fmp_ctrl = (tid << 8u) |	//tid
	    (sw << 7u) |	//sw
	    (sr << 6u) |	//sr
	    (nsw << 5u) |	//nsw
	    (nsr << 4u) |	//nsr
	    (ufk << 3u) |	//ufk
	    (0x0u << 2u) |	//
	    (enc << 1u) |	//enc
	    (valid << 0u);	//valid
	ufshcd_writel(hba, 0x00000001, FMP_FMPDEK0);
	ufshcd_writel(hba, 0x00000010, FMP_FMPDEK1);
	ufshcd_writel(hba, 0x00000100, FMP_FMPDEK2);
	ufshcd_writel(hba, 0x00001000, FMP_FMPDEK3);
	ufshcd_writel(hba, 0x00010000, FMP_FMPDEK4);
	ufshcd_writel(hba, 0x00100000, FMP_FMPDEK5);
	ufshcd_writel(hba, 0x01000000, FMP_FMPDEK6);
	ufshcd_writel(hba, 0x10000000, FMP_FMPDEK7);
	ufshcd_writel(hba, 0x00000001, FMP_FMPDTK0);
	ufshcd_writel(hba, 0x00000010, FMP_FMPDTK1);
	ufshcd_writel(hba, 0x00000100, FMP_FMPDTK2);
	ufshcd_writel(hba, 0x00001000, FMP_FMPDTK3);
	ufshcd_writel(hba, 0x00010000, FMP_FMPDTK4);
	ufshcd_writel(hba, 0x00100000, FMP_FMPDTK5);
	ufshcd_writel(hba, 0x01000000, FMP_FMPDTK6);
	ufshcd_writel(hba, 0x10000000, FMP_FMPDTK7);
	ufshcd_writel(hba, 0x00000001, FMP_FMPFEKM0);
	ufshcd_writel(hba, 0x00000010, FMP_FMPFEKM1);
	ufshcd_writel(hba, 0x00000100, FMP_FMPFEKM2);
	ufshcd_writel(hba, 0x00001000, FMP_FMPFEKM3);
	ufshcd_writel(hba, 0x00010000, FMP_FMPFEKM4);
	ufshcd_writel(hba, 0x00100000, FMP_FMPFEKM5);
	ufshcd_writel(hba, 0x01000000, FMP_FMPFEKM6);
	ufshcd_writel(hba, 0x10000000, FMP_FMPFEKM7);
	ufshcd_writel(hba, 0x00000001, FMP_FMPFTKM0);
	ufshcd_writel(hba, 0x00000010, FMP_FMPFTKM1);
	ufshcd_writel(hba, 0x00000100, FMP_FMPFTKM2);
	ufshcd_writel(hba, 0x00001000, FMP_FMPFTKM3);
	ufshcd_writel(hba, 0x00010000, FMP_FMPFTKM4);
	ufshcd_writel(hba, 0x00100000, FMP_FMPFTKM5);
	ufshcd_writel(hba, 0x01000000, FMP_FMPFTKM6);
	ufshcd_writel(hba, 0x10000000, FMP_FMPFTKM7);
	ufshcd_writel(hba, 0x00000001, FMP_FMPSCTRL0);
	ufshcd_writel(hba, 0x00000010, FMP_FMPSCTRL1);
	ufshcd_writel(hba, 0x00000100, FMP_FMPSCTRL2);
	ufshcd_writel(hba, 0x00001000, FMP_FMPSCTRL3);
	ufshcd_writel(hba, 0x00010000, FMP_FMPSCTRL4);
	ufshcd_writel(hba, 0x00100000, FMP_FMPSCTRL5);
	ufshcd_writel(hba, 0x01000000, FMP_FMPSCTRL6);
	ufshcd_writel(hba, 0x10000000, FMP_FMPSCTRL7);
#if 0
	switch (smu_index) {
	case 0:{
#endif
			ufshcd_writel(hba, fmp_bsector, FMP_FMPSBEGIN0);
			ufshcd_writel(hba, fmp_esector, FMP_FMPSEND0);
			ufshcd_writel(hba, fmp_lun, FMP_FMPSLUN0);
			ufshcd_writel(hba, fmp_ctrl, FMP_FMPSCTRL0);
			//break;
			//}
#if 0
	case 1:	{
				ufshcd_writel(hba, fmp_bsector, FMP_FMPSBEGIN1);
				ufshcd_writel(hba, fmp_esector, FMP_FMPSEND1);
				ufshcd_writel(hba, fmp_lun, FMP_FMPSLUN1);
				ufshcd_writel(hba, fmp_ctrl, FMP_FMPSCTRL1);
				break;
			}
	case 2:	{
				ufshcd_writel(hba, fmp_bsector, FMP_FMPSBEGIN2);
				ufshcd_writel(hba, fmp_esector, FMP_FMPSEND2);
				ufshcd_writel(hba, fmp_lun, FMP_FMPSLUN2);
				ufshcd_writel(hba, fmp_ctrl, FMP_FMPSCTRL2);
				break;
			}
	case 3:	{
				ufshcd_writel(hba, fmp_bsector, FMP_FMPSBEGIN3);
				ufshcd_writel(hba, fmp_esector, FMP_FMPSEND3);
				ufshcd_writel(hba, fmp_lun, FMP_FMPSLUN3);
				ufshcd_writel(hba, fmp_ctrl, FMP_FMPSCTRL3);
				break;
			}
	case 4:	{
				ufshcd_writel(hba, fmp_bsector, FMP_FMPSBEGIN4);
				ufshcd_writel(hba, fmp_esector, FMP_FMPSEND4);
				ufshcd_writel(hba, fmp_lun, FMP_FMPSLUN4);
				ufshcd_writel(hba, fmp_ctrl, FMP_FMPSCTRL4);
				break;
			}
	case 5:	{
				ufshcd_writel(hba, fmp_bsector, FMP_FMPSBEGIN5);
				ufshcd_writel(hba, fmp_esector, FMP_FMPSEND5);
				ufshcd_writel(hba, fmp_lun, FMP_FMPSLUN5);
				ufshcd_writel(hba, fmp_ctrl, FMP_FMPSCTRL5);
				break;
			}
	case 6:	{
				ufshcd_writel(hba, fmp_bsector, FMP_FMPSBEGIN6);
				ufshcd_writel(hba, fmp_esector, FMP_FMPSEND6);
				ufshcd_writel(hba, fmp_lun, FMP_FMPSLUN6);
				ufshcd_writel(hba, fmp_ctrl, FMP_FMPSCTRL6);
				break;
			}
	case 7:	{
				ufshcd_writel(hba, fmp_bsector, FMP_FMPSBEGIN7);
				ufshcd_writel(hba, fmp_esector, FMP_FMPSEND7);
				ufshcd_writel(hba, fmp_lun, FMP_FMPSLUN7);
				ufshcd_writel(hba, fmp_ctrl, FMP_FMPSCTRL7);
				break;
			}
	default:	{
				ufshcd_writel(hba, fmp_bsector, FMP_FMPSBEGIN7);
				ufshcd_writel(hba, fmp_esector, FMP_FMPSEND7);
				ufshcd_writel(hba, fmp_lun, FMP_FMPSLUN7);
				ufshcd_writel(hba, fmp_ctrl, FMP_FMPSCTRL7);
				break;
			}
		}
#endif
	protbytzpc = 0x0;
	select_inline_enc = 0x1;
	fmp_on = 0x1;
	unmap_disable = 0x1;
	nskeyreg = 0x1;
	nssmu = 0x1;
	nsuser = 0x1;
	use_otp_mask = 0x0;
	fmp_security = (protbytzpc << 31u) |
		((unsigned int)select_inline_enc << 30u) |
		((unsigned int)fmp_on << 29u) |
		((unsigned int)unmap_disable << 28u) |
		((unsigned int)0x3F << 22u) |
		((unsigned int)desc_type << 19u) |
		((unsigned int)0x5 << 16u) |
		((unsigned int)nskeyreg << 15u) |
		((unsigned int)nssmu << 14u) |
		((unsigned int)nsuser << 13u) |
		((unsigned int)use_otp_mask << 12u) |
		((unsigned int)0x5 << 9u) |
		((unsigned int)0x5 << 6u) |
		((unsigned int)0x5 << 3u) | ((unsigned int)0x5 << 0u);
	ufshcd_writel(hba, fmp_security, FMP_FMPRSECURITY);
	return 0;
}

static int tcc_ufs_init(struct ufs_hba *hba)
{
	return 0;
}

static struct ufs_hba_ops tcc_hba_ops = {
	.init = tcc_ufs_init,
	.hce_enable_notify = tcc_ufs_hce_enable_notify,
	.link_startup_notify = tcc_ufs_link_startup_notify,
	.smu_setting = tcc_ufs_smu_setting,
};

static int tcc_ufs_probe(struct udevice *dev)
{
	int err;
	unsigned int val;

	val = readl((void *)0x1d2c0018);
	val &= 0xFFFFEBFF;
	writel((val), ((void *)0x1d2c0018));
	dev_dbg(dev, "%s : UFS Reset(0x%x)\n", __func__, val);
	udelay(100);
	val =
		((unsigned int)readl((void *)0x1d2c0018) |
		 ((unsigned int)1 << 12u) | ((unsigned int)1 << 10u));
	writel((val), ((void *)0x1d2c0018));
	dev_dbg(dev, "%s : Reset(0x%x) done\n", __func__, val);
	//writel((0x2), ((void*)0x1dff000c));
	writel((0x1),
			((void *)((unsigned int *)(unsigned int)((unsigned int)
					0x1d280000 +
					(unsigned int)
					HCI_MPHY_REFCLK_SEL))));
	err = ufshcd_probe(dev, &tcc_hba_ops);
	dev_dbg(dev, "%s\n", __func__);
	if (err != 0)
		dev_err(dev, "ufshcd_probe() failed %d\n", err);

	return err;
}

static int tcc_ufs_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;

	return ufs_scsi_bind(dev, &scsi_dev);
}

static const struct udevice_id tcc_ufs_ids[] = {
	{
		.compatible = "telechips,ufs",
	},
	{},
};

U_BOOT_DRIVER(tcc_ufs_pltfm) = {
	.name = "tcc-ufs",
	.id = UCLASS_UFS,
	.of_match = tcc_ufs_ids,
	.probe = tcc_ufs_probe,
	.bind = tcc_ufs_bind,
};
