// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_UFS_H
#define TCC_UFS_H
//-------------------------------------------------
// HCI Reigster Definition
//-------------------------------------------------
#define HCI_UTRLBA					0x70U
#define HCI_UTRLBAU					0x74U
#define HCI_TXPRDT_ENTRY_SIZE       0x1100U
#define HCI_RXPRDT_ENTRY_SIZE       0x1104U
#define HCI_CLKSTOP_CTRL            0x11B0U
#define HCI_DATA_REORDER            0x1160U
#define HCI_AXIDMA_RWDATA_BURST_LEN 0x116CU
#define HCI_GPIO_OUT                0x1170U
#define HCI_UFS_ACG_DISABLE         0x11FCU
#define HCI_CRYPTOCFG_CRYPTOKEY_0   0x0400U
#define HCI_CRYPTOCFG_CRYPTOKEY_1   0x0404U
#define HCI_CRYPTOCFG_CRYPTOKEY_2   0x0408U
#define HCI_CRYPTOCFG_CRYPTOKEY_3   0x040cU
#define HCI_CRYPTOCFG_CRYPTOKEY_4   0x0410U
#define HCI_CRYPTOCFG_CRYPTOKEY_5   0x0414U
#define HCI_CRYPTOCFG_CRYPTOKEY_6   0x0418U
#define HCI_CRYPTOCFG_CRYPTOKEY_7   0x041cU
#define HCI_CRYPTOCFG_CRYPTOKEY_8   0x0420U
#define HCI_CRYPTOCFG_CRYPTOKEY_9   0x0424U
#define HCI_CRYPTOCFG_CRYPTOKEY_A   0x0428U
#define HCI_CRYPTOCFG_CRYPTOKEY_B   0x042cU
#define HCI_CRYPTOCFG_CRYPTOKEY_C   0x0430U
#define HCI_CRYPTOCFG_CRYPTOKEY_D   0x0434U
#define HCI_CRYPTOCFG_CRYPTOKEY_E   0x0438U
#define HCI_CRYPTOCFG_CRYPTOKEY_F   0x043cU
#define HCI_CRYPTOCFG__CFGE_CAPIDX_DUSIZE 0x0440U
//-------------------------------------------------
// HCI Specific Initialization Defines
//-------------------------------------------------
//UTP Task Management Request List Base Address
#define  CONV_UTMRLBA_LOW_VAL          0x7D303800U
//UTP Transfer Request List Base Address
#define  CONV_UTMRLBA_HIGH_VAL         0x00000000U
#define  UTMRLBA_LOW_VAL               0x1D303800U
#define  UTMRLBA_HIGH_VAL              0x00000000U
#define  CONV_UTRLBA_LOW_VAL           0x7D300000U
#define  CONV_UTRLBA_HIGH_VAL          0x00000000U
#define  UTRLBA_LOW_VAL                0x1D300000U
#define  UTRLBA_HIGH_VAL               0x00000000U
//UTP  Command  Descriptor  Base  Address
#define  CONV_UCDBA_LOW_VAL            0x7D300800U
#define  CONV_UCDBAU_HIGH_VAL          0x00000000U
#define  UCDBA_LOW_VAL                 0x1D300800U
#define  UCDBAU_HIGH_VAL               0x00000000U
//Data  Base  Address
#define  CONV_DBA_LOW_VAL              0x7D301000U
#define  CONV_DBAU_HIGH_VAL            0x00000000U
#define  DBA_LOW_VAL                   0x1D301000U
#define  DBAU_HIGH_VAL                 0x00000000U
#define  DBA_OFFSET                    0x00000000U
//UTP Command Descriptor
#define  UCD_OFFSET                    0x00000000U
//Physical Region Description Table
#define  PRDT_OFFSET                   0x00000200U
#define  RESP_OFFSET                   0x00000100U
#define  RESP_LENGTH                   0x00000040U
#define  UTRL_OFFSET                   0x00000020U
#define  UTMRL_OFFSET                  0x00000050U
#define  AXI_ADDR_WIDTH                0x35U
#define  UTRD_DOORBELL_SIZE            0x16U
#define  UTMRD_DOORBELL_SIZE           0x4U
#define  UTRIACR_VAL                   0x0100060aU
#define  ENABLE                        0x00000001U
#define  DISABLE                       0x00000000U
#define  TXPRDT_ENTRY_SIZE_VAL         0xFFFFFFFFU
#define  RXPRDT_ENTRY_SIZE_VAL         0xFFFFFFFFU
#define  TO_CNT_VAL_1US_VAL            0x00000064U
#define  MISC_VAL                      0x000000f0U
#define  DEFAULT_PAGE_CODE_VALUE       0x0U
#define  SUPPORTED_VPD                 0x0U
#define  MODE_PAGE_VPD                 0x87U
#define  DEFAULT_ALLOCATION_LENGTH     0x24U
#define  DEFAULT_CONTROL_VALUE         0x0U
#define  VIP_LOGICAL_BLOCK_SIZE        0x00001000U
#define  CONTROL                       0x0AU
#define  READ_WRITE_ERROR_RECOVERY     0x01U
#define  CACHING                       0x08U
#define  ALL_PAGES                     0x3FU
#define  DEFAULT_SUBPAGE_CODE          0x0U

//--------------------------------------------------
////Ufs Specific Defines
////------------------------------------------------
#define  N_DeviceID                    0x3000U
#define  N_DeviceID_valid              0x3001U
#define  T_ConnectionState             0x4020U
#define  T_PeerDeviceID                0x4021U

// Define Internal Attributes
//--------------------------------------------------
#define  PA_TX_SKIP_VAL						0x00000002U
#define PA_Local_TX_LCC_Enable				0x0000155EU
#define  PA_DBG_CLK_PERIOD					0x00003850U
#define  PA_DBG_AUTOMODE_THLD				0x000038d8U
#define  PA_DBG_OPTION_SUITE				0x00003990U
#define  DL_FC0_PROTECTION_TIMEOUT_VAL_VALUE 0x00001FFFU
#define  DL_FC1_PROTECTION_TIMEOUT_VAL_VALUE 0x00001FFFU
#define  DME_LINKSTARTUP_REQ				0x00000016U
#define  N_DEVICE_ID_VAL					0x00000000U
#define  N_DEVICE_ID_VALID_VAL				0x00000001U
#define  T_CONNECTION_STATE_OFF_VAL			0x00000000U
#define  T_CONNECTION_STATE_ON_VAL			0x00000001U
#define  T_PEER_DEVICE_ID_VAL				0x00000001U
#define  DME_LINKSTARTUP_CMD				0x00000016U
#define  T_DBG_CPORT_OPTION_SUITE			0x00006890U
#define  T_DBG_OPTION_SUITE_VAL				0x0000000DU
#define  TX_LINERESET_PVALUE_MSB			0x000000abU
#define  TX_LINERESET_PVALUE_LSB			0x000000acU
#define  RX_LINERESET_VALUE_MSB				0x0000001cU
#define  RX_LINERESET_VALUE_LSB				0x0000001dU
#define  RX_OVERSAMPLING_ENABLE				0x00000076U
#define DME_RESET_REQ						0x00007820U
#define DME_RESET_REQ_LEVEL					0x00007824U
#define DME_ENDPOINTRESET_REQ				0x00007840U

#define UNIPRO_PCLK_PERIOD_NS 6U //145 MHz < PCLK_Freq <= 170 MHz
#define CALCULATED_VALUE 0x4E20U //UNIPRO_PCLK_PERIOD_NS * 6 = 120,000

#define UFS_FMP_BASE	0x10000U //UFS_HCI_BASE + 0x10000 //0x1D290000
#define FMP_FMPRCTRL         (UFS_FMP_BASE + 0x0000U)
#define FMP_FMPRSTAT         (UFS_FMP_BASE + 0x0008U)
#define FMP_FMPRSECURITY     (UFS_FMP_BASE + 0x0010U)
#define FMP_FMPVERSION       (UFS_FMP_BASE + 0x001cU)
#define FMP_FMPDEK0          (UFS_FMP_BASE + 0x0020U)
#define FMP_FMPDEK1          (UFS_FMP_BASE + 0x0024U)
#define FMP_FMPDEK2          (UFS_FMP_BASE + 0x0028U)
#define FMP_FMPDEK3          (UFS_FMP_BASE + 0x002cU)
#define FMP_FMPDEK4          (UFS_FMP_BASE + 0x0030U)
#define FMP_FMPDEK5          (UFS_FMP_BASE + 0x0034U)
#define FMP_FMPDEK6          (UFS_FMP_BASE + 0x0038U)
#define FMP_FMPDEK7          (UFS_FMP_BASE + 0x003cU)
#define FMP_FMPDTK0          (UFS_FMP_BASE + 0x0040U)
#define FMP_FMPDTK1          (UFS_FMP_BASE + 0x0044U)
#define FMP_FMPDTK2          (UFS_FMP_BASE + 0x0048U)
#define FMP_FMPDTK3          (UFS_FMP_BASE + 0x004cU)
#define FMP_FMPDTK4          (UFS_FMP_BASE + 0x0050U)
#define FMP_FMPDTK5          (UFS_FMP_BASE + 0x0054U)
#define FMP_FMPDTK6          (UFS_FMP_BASE + 0x0058U)
#define FMP_FMPDTK7          (UFS_FMP_BASE + 0x005cU)
#define FMP_FMPWCTRL         (UFS_FMP_BASE + 0x0100U)
#define FMP_FMPWSTAT         (UFS_FMP_BASE + 0x0108U)
#define FMP_FMPFEKM0         (UFS_FMP_BASE + 0x0120U)
#define FMP_FMPFEKM1         (UFS_FMP_BASE + 0x0124U)
#define FMP_FMPFEKM2         (UFS_FMP_BASE + 0x0128U)
#define FMP_FMPFEKM3         (UFS_FMP_BASE + 0x012cU)
#define FMP_FMPFEKM4         (UFS_FMP_BASE + 0x0130U)
#define FMP_FMPFEKM5         (UFS_FMP_BASE + 0x0134U)
#define FMP_FMPFEKM6         (UFS_FMP_BASE + 0x0138U)
#define FMP_FMPFEKM7         (UFS_FMP_BASE + 0x013cU)
#define FMP_FMPFTKM0         (UFS_FMP_BASE + 0x0140U)
#define FMP_FMPFTKM1         (UFS_FMP_BASE + 0x0144U)
#define FMP_FMPFTKM2         (UFS_FMP_BASE + 0x0148U)
#define FMP_FMPFTKM3         (UFS_FMP_BASE + 0x014cU)
#define FMP_FMPFTKM4         (UFS_FMP_BASE + 0x0150U)
#define FMP_FMPFTKM5         (UFS_FMP_BASE + 0x0154U)
#define FMP_FMPFTKM6         (UFS_FMP_BASE + 0x0158U)
#define FMP_FMPFTKM7         (UFS_FMP_BASE + 0x015cU)
#define FMP_FMPSBEGIN0       (UFS_FMP_BASE + 0x0200U)
#define FMP_FMPSEND0         (UFS_FMP_BASE + 0x0204U)
#define FMP_FMPSLUN0         (UFS_FMP_BASE + 0x0208U)
#define FMP_FMPSCTRL0        (UFS_FMP_BASE + 0x020cU)
#define FMP_FMPSBEGIN1       (UFS_FMP_BASE + 0x0210U)
#define FMP_FMPSEND1         (UFS_FMP_BASE + 0x0214U)
#define FMP_FMPSLUN1         (UFS_FMP_BASE + 0x0218U)
#define FMP_FMPSCTRL1        (UFS_FMP_BASE + 0x021cU)
#define FMP_FMPSBEGIN2       (UFS_FMP_BASE + 0x0220U)
#define FMP_FMPSEND2         (UFS_FMP_BASE + 0x0224U)
#define FMP_FMPSLUN2         (UFS_FMP_BASE + 0x0228U)
#define FMP_FMPSCTRL2        (UFS_FMP_BASE + 0x022cU)
#define FMP_FMPSBEGIN3       (UFS_FMP_BASE + 0x0230U)
#define FMP_FMPSEND3         (UFS_FMP_BASE + 0x0234U)
#define FMP_FMPSLUN3         (UFS_FMP_BASE + 0x0238U)
#define FMP_FMPSCTRL3        (UFS_FMP_BASE + 0x023cU)
#define FMP_FMPSBEGIN4       (UFS_FMP_BASE + 0x0240U)
#define FMP_FMPSEND4         (UFS_FMP_BASE + 0x0244U)
#define FMP_FMPSLUN4         (UFS_FMP_BASE + 0x0248U)
#define FMP_FMPSCTRL4        (UFS_FMP_BASE + 0x024cU)
#define FMP_FMPSBEGIN5       (UFS_FMP_BASE + 0x0250U)
#define FMP_FMPSEND5         (UFS_FMP_BASE + 0x0254U)
#define FMP_FMPSLUN5         (UFS_FMP_BASE + 0x0258U)
#define FMP_FMPSCTRL5        (UFS_FMP_BASE + 0x025cU)
#define FMP_FMPSBEGIN6       (UFS_FMP_BASE + 0x0260U)
#define FMP_FMPSEND6         (UFS_FMP_BASE + 0x0264U)
#define FMP_FMPSLUN6         (UFS_FMP_BASE + 0x0268U)
#define FMP_FMPSCTRL6        (UFS_FMP_BASE + 0x026cU)
#define FMP_FMPSBEGIN7       (UFS_FMP_BASE + 0x0270U)
#define FMP_FMPSEND7         (UFS_FMP_BASE + 0x0274U)
#define FMP_FMPSLUN7         (UFS_FMP_BASE + 0x0278U)
#define FMP_FMPSCTRL7        (UFS_FMP_BASE + 0x027cU)

#endif //TCC_UFS_H
