// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef __CAN_REG_HEADER
#define __CAN_REG_HEADER
#include <stdlib.h>

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

/**************************************************************************************************
*
*                                      CAN Controller Register MAP
*
**************************************************************************************************/
typedef struct CANRegFieldCREL
{
    volatile uint32_t                            rfDAY       :8; //[7:0]
    volatile uint32_t                            rfMON       :8; //[15:8]
    volatile uint32_t                            rfYEAR      :4; //[19:16]
    volatile uint32_t                            rfSUBSTEP   :4; //[23:20]
    volatile uint32_t                            rfSTEP      :4; //[27:24]
    volatile uint32_t                            rfREL       :4; //[31:28]
} CANRegFieldCREL_t;

typedef union CANRegCREL
{
    volatile uint32_t                            rNReg;
    CANRegFieldCREL_t                   rFReg;
} CANRegCREL_t;

typedef struct CANRegFieldENDN
{
    volatile uint32_t                            rfETV       :32; //[31:0]
} CANRegFieldENDN_t;

typedef union CANRegENDN
{
    volatile uint32_t                            rNReg;
    CANRegFieldENDN_t                   rFReg;
} CANRegENDN_t;

typedef struct CANRegFieldDBTP
{
    volatile uint32_t                            rfDSJW      :4; //[3:0]
    volatile uint32_t                            rfDTSEG2    :4; //[7:4]
    volatile uint32_t                            rfDTSEG1    :5; //[12:8]
    volatile uint32_t                            rfRsvd      :3; //[15:13]
    volatile uint32_t                            rfDBRP      :5; //[20:16]
    volatile uint32_t                            rfRsvd1     :2; //[22:21]
    volatile uint32_t                            rfTDC       :1; //[23]
    volatile uint32_t                            rfRsvd2     :8; //[24:31]
} CANRegFieldDBTP_t;

typedef union CANRegDBTP
{
    volatile uint32_t                            rNReg;
    CANRegFieldDBTP_t                   rFReg;
} CANRegDBTP_t;

typedef struct CANRegFieldTEST
{
    volatile uint32_t                            rfRsvd      :4; //[3:0]
    volatile uint32_t                            rfLBCK      :1; //[4]
    volatile uint32_t                            rfTX        :2; //[6:5]
    volatile uint32_t                            rfRX        :1; //[7]
    volatile uint32_t                            rfRsvd1     :24; //[31:8]
} CANRegFieldTEST_t;

typedef union CANRegTEST
{
    volatile uint32_t                            rNReg;
    CANRegFieldTEST_t                   rFReg;
} CANRegTEST_t;

typedef struct CANRegFieldRWD
{
    volatile uint32_t                            rfWDC       :8; //[7:0]
    volatile uint32_t                            rfWDV       :8; //[15:8]
    volatile uint32_t                            rfRsvd      :16; //[31:16]
} CANRegFieldRWD_t;

typedef union CANRegRWD
{
    volatile uint32_t                            rNReg;
    CANRegFieldRWD_t                    rFReg;
} CANRegRWD_t;

typedef struct CANRegFieldCCCR
{
    volatile uint32_t                            rfINIT      :1; //[0]
    volatile uint32_t                            rfCCE       :1; //[1]
    volatile uint32_t                            rfASM       :1; //[2]
    volatile uint32_t                            rfCSA       :1; //[3]
    volatile uint32_t                            rfCSR       :1; //[4]
    volatile uint32_t                            rfMON       :1; //[5]
    volatile uint32_t                            rfDAR       :1; //[6]
    volatile uint32_t                            rfTEST      :1; //[7]
    volatile uint32_t                            rfFDOE      :1; //[8]
    volatile uint32_t                            rfBRSE      :1; //[9]
    volatile uint32_t                            rfUTSU      :1; //[10]
    volatile uint32_t                            rfWMM       :1; //[11]
    volatile uint32_t                            rfPXHD      :1; //[12]
    volatile uint32_t                            rfEFBI      :1; //[13]
    volatile uint32_t                            rfTXP       :1; //[14]
    volatile uint32_t                            rfNISO      :1; //[15]
    volatile uint32_t                            rfRsvd1     :16; //[31:16]
} CANRegFieldCCCR_t;

typedef union CANRegCCCR
{
    volatile uint32_t                            rNReg;
    CANRegFieldCCCR_t                   rFReg;
} CANRegCCCR_t;

typedef struct CANRegFieldNBTP
{
    volatile uint32_t                            rfNTSEG2    :7; //[6:0]
    volatile uint32_t                            rfRsvd      :1; //[7]
    volatile uint32_t                            rfNTSEG1    :8; //[15:8]
    volatile uint32_t                            rfNBRP      :9; //[24:16]
    volatile uint32_t                            rfNSJW      :7; //[31:25]
} CANRegFieldNBTP_t;

typedef union CANRegNBTP
{
    volatile uint32_t                            rNReg;
    CANRegFieldNBTP_t                   rFReg;
} CANRegNBTP_t;

typedef struct CANRegFieldTSCC
{
    volatile uint32_t                            rfTSS       :2; //[1:0]
    volatile uint32_t                            rfRsvd      :14; //[15:2]
    volatile uint32_t                            rfTCP       :4; //[19:16]
    volatile uint32_t                            rfRsvd1     :12; //[31:20]
} CANRegFieldTSCC_t;

typedef union CANRegTSCC
{
    volatile uint32_t                            rNReg;
    CANRegFieldTSCC_t                   rFReg;
} CANRegTSCC_t;

typedef struct CANRegFieldTSCV
{
    volatile uint32_t                            rfTSC       :16; //[15:0]
    volatile uint32_t                            rfRsvd      :16; //[31:16]
} CANRegFieldTSCV_t;

typedef union CANRegTSCV
{
    volatile uint32_t                            rNReg;
    CANRegFieldTSCV_t                   rFReg;
} CANRegTSCV_t;

typedef struct CANRegFieldTOCC
{
    volatile uint32_t                            rfETOC      :1; //[0]
    volatile uint32_t                            rfTOS       :2; //[2:1]
    volatile uint32_t                            rfRsvd      :13; //[15:3]
    volatile uint32_t                            rfTOP       :16; //[31:16]
} CANRegFieldTOCC_t;

typedef union CANRegTOCC
{
    volatile uint32_t                            rNReg;
    CANRegFieldTOCC_t                   rFReg;
} CANRegTOCC_t;

typedef struct CANRegFieldTOCV
{
    volatile uint32_t                            rfTOC       :16; //[15:0]
    volatile uint32_t                            rfRsvd      :16; //[31:16]
} CANRegFieldTOCV_t;

typedef union CANRegTOCV
{
    volatile uint32_t                            rNReg;
    CANRegFieldTOCV_t                   rFReg;
} CANRegTOCV_t;

typedef struct CANRegFieldECR
{
    volatile uint32_t                            rfTEC       :8; //[7:0]
    volatile uint32_t                            rfREC       :7; //[14:8]
    volatile uint32_t                            rfRP        :1; //[15]
    volatile uint32_t                            rfCEL       :8; //[23:16]
    volatile uint32_t                            rfRsvd      :8; //[31:24]
} CANRegFieldECR_t;

typedef union CANRegECR
{
    volatile uint32_t                            rNReg;
    CANRegFieldECR_t                    rFReg;
} CANRegECR_t;

typedef struct CANRegFieldPSR
{
    volatile uint32_t                            rfLEC       :3; //[2:0]
    volatile uint32_t                            rfACT       :2; //[4:3]
    volatile uint32_t                            rfEP        :1; //[5]
    volatile uint32_t                            rfEW        :1; //[6]
    volatile uint32_t                            rfBO        :1; //[7]
    volatile uint32_t                            rfDLEC      :3; //[10:8]
    volatile uint32_t                            rfRESI      :1; //[11]
    volatile uint32_t                            rfRBRS      :1; //[12]
    volatile uint32_t                            rfRFDF      :1; //[13]
    volatile uint32_t                            rfPXE       :1; //[14]
    volatile uint32_t                            rfRsvd      :1; //[15]
    volatile uint32_t                            rfTDCV      :7; //[22:16]
    volatile uint32_t                            rfRsvd1     :9; //[31:23]
} CANRegFieldPSR_t;

typedef union CANRegPSR
{
    volatile uint32_t                            rNReg;
    CANRegFieldPSR_t                    rFReg;
} CANRegPSR_t;

typedef struct CANRegFieldTDCR
{
    volatile uint32_t                            rfTDCF      :7; //[6:0]
    volatile uint32_t                            rfRsvd      :1; //[7]
    volatile uint32_t                            rfTDCO      :7; //[14:8]
    volatile uint32_t                            rfRsvd1     :17; //[31:15]
} CANRegFieldTDCR_t;

typedef union CANRegTDCR
{
    volatile uint32_t                            rNReg;
    CANRegFieldTDCR_t                   rFReg;
} CANRegTDCR_t;

typedef struct CANRegFieldIR
{
    volatile uint32_t                            rfRF0N      :1; //[0]
    volatile uint32_t                            rfRF0W      :1; //[1]
    volatile uint32_t                            rfRF0F      :1; //[2]
    volatile uint32_t                            rfRF0L      :1; //[3]
    volatile uint32_t                            rfRF1N      :1; //[4]
    volatile uint32_t                            rfRF1W      :1; //[5]
    volatile uint32_t                            rfRF1F      :1; //[6]
    volatile uint32_t                            rfRF1L      :1; //[7]
    volatile uint32_t                            rfHPM       :1; //[8]
    volatile uint32_t                            rfTC        :1; //[9]
    volatile uint32_t                            rfTCF       :1; //[10]
    volatile uint32_t                            rfTFE       :1; //[11]
    volatile uint32_t                            rfTEFN      :1; //[12]
    volatile uint32_t                            rfTEFW      :1; //[13]
    volatile uint32_t                            rfTEFF      :1; //[14]
    volatile uint32_t                            rfTEFL      :1; //[15]
    volatile uint32_t                            rfTSW       :1; //[16]
    volatile uint32_t                            rfMRAF      :1; //[17]
    volatile uint32_t                            rfTOO       :1; //[18]
    volatile uint32_t                            rfDRX       :1; //[19]
    volatile uint32_t                            rfBEC       :1; //[20]
    volatile uint32_t                            rfBEU       :1; //[21]
    volatile uint32_t                            rfELO       :1; //[22]
    volatile uint32_t                            rfEP        :1; //[23]
    volatile uint32_t                            rfEW        :1; //[24]
    volatile uint32_t                            rfBO        :1; //[25]
    volatile uint32_t                            rfWDI       :1; //[26]
    volatile uint32_t                            rfPEA       :1; //[27]
    volatile uint32_t                            rfPED       :1; //[28]
    volatile uint32_t                            rfARA       :1; //[29]
    volatile uint32_t                            rfRsvd      :2; //[31:30]
} CANRegFieldIR_t;

typedef union CANRegIR
{
    volatile uint32_t                            rNReg;
    CANRegFieldIR_t                     rFReg;
} CANRegIR_t;

typedef struct CANRegFieldIE
{
    volatile uint32_t                            rfRF0NE     :1; //[0]
    volatile uint32_t                            rfRF0WE     :1; //[1]
    volatile uint32_t                            rfRF0FE     :1; //[2]
    volatile uint32_t                            rfRF0LE     :1; //[3]
    volatile uint32_t                            rfRF1NE     :1; //[4]
    volatile uint32_t                            rfRF1WE     :1; //[5]
    volatile uint32_t                            rfRF1FE     :1; //[6]
    volatile uint32_t                            rfRF1LE     :1; //[7]
    volatile uint32_t                            rfHPME      :1; //[8]
    volatile uint32_t                            rfTCE       :1; //[9]
    volatile uint32_t                            rfTCFE      :1; //[10]
    volatile uint32_t                            rfTFEE      :1; //[11]
    volatile uint32_t                            rfTEFNE     :1; //[12]
    volatile uint32_t                            rfTEFWE     :1; //[13]
    volatile uint32_t                            rfTEFFE     :1; //[14]
    volatile uint32_t                            rfTEFLE     :1; //[15]
    volatile uint32_t                            rfTSWE      :1; //[16]
    volatile uint32_t                            rfMRAFE     :1; //[17]
    volatile uint32_t                            rfTOOE      :1; //[18]
    volatile uint32_t                            rfDRXE      :1; //[19]
    volatile uint32_t                            rfBECE      :1; //[20]
    volatile uint32_t                            rfBEUE      :1; //[21]
    volatile uint32_t                            rfELOE      :1; //[22]
    volatile uint32_t                            rfEPE       :1; //[23]
    volatile uint32_t                            rfEWE       :1; //[24]
    volatile uint32_t                            rfBOE       :1; //[25]
    volatile uint32_t                            rfWDIE      :1; //[26]
    volatile uint32_t                            rfPEAE      :1; //[27]
    volatile uint32_t                            rfPEDE      :1; //[28]
    volatile uint32_t                            rfARAE      :1; //[29]
    volatile uint32_t                            rfRsvd      :2; //[31:30]
} CANRegFieldIE_t;

typedef union CANRegIE
{
    volatile uint32_t                            rNReg;
    CANRegFieldIE_t                     rFReg;
} CANRegIE_t;

typedef struct CANRegFieldILS
{
    volatile uint32_t                            rfRF0NL     :1; //[0]
    volatile uint32_t                            rfRF0WL     :1; //[1]
    volatile uint32_t                            rfRF0FL     :1; //[2]
    volatile uint32_t                            rfRF0LL     :1; //[3]
    volatile uint32_t                            rfRF1NL     :1; //[4]
    volatile uint32_t                            rfRF1WL     :1; //[5]
    volatile uint32_t                            rfRF1FL     :1; //[6]
    volatile uint32_t                            rfRF1LL     :1; //[7]
    volatile uint32_t                            rfHPML      :1; //[8]
    volatile uint32_t                            rfTCL       :1; //[9]
    volatile uint32_t                            rfTCFL      :1; //[10]
    volatile uint32_t                            rfTFEL      :1; //[11]
    volatile uint32_t                            rfTEFNL     :1; //[12]
    volatile uint32_t                            rfTEFWL     :1; //[13]
    volatile uint32_t                            rfTEFFL     :1; //[14]
    volatile uint32_t                            rfTEFLL     :1; //[15]
    volatile uint32_t                            rfTSWL      :1; //[16]
    volatile uint32_t                            rfMRAFL     :1; //[17]
    volatile uint32_t                            rfTOOL      :1; //[18]
    volatile uint32_t                            rfDRXL      :1; //[19]
    volatile uint32_t                            rfBECL      :1; //[20]
    volatile uint32_t                            rfBEUL      :1; //[21]
    volatile uint32_t                            rfELOL      :1; //[22]
    volatile uint32_t                            rfEPL       :1; //[23]
    volatile uint32_t                            rfEWL       :1; //[24]
    volatile uint32_t                            rfBOL       :1; //[25]
    volatile uint32_t                            rfWDIL      :1; //[26]
    volatile uint32_t                            rfPEAL      :1; //[27]
    volatile uint32_t                            rfPEDL      :1; //[28]
    volatile uint32_t                            rfARAL      :1; //[29]
    volatile uint32_t                            rfRsvd      :2; //[31:30]
} CANRegFieldILS_t;

typedef union CANRegILS
{
    volatile uint32_t                            rNReg;
    CANRegFieldILS_t                    rFReg;
} CANRegILS_t;

typedef struct CANRegFieldILE
{
    volatile uint32_t                            rfEINT0     :1; //[0]
    volatile uint32_t                            rfEINT1     :1; //[1]
    volatile uint32_t                            rfRsvd      :30; //[31:2]
} CANRegFieldILE_t;

typedef union CANRegILE
{
    volatile uint32_t                            rNReg;
    CANRegFieldILE_t                    rFReg;
} CANRegILE_t;

typedef struct CANRegFieldGFC
{
    volatile uint32_t                            rfRRFE      :1; //[0]
    volatile uint32_t                            rfRRFS      :1; //[1]
    volatile uint32_t                            rfANFE      :2; //[3:2]
    volatile uint32_t                            rfANFS      :2; //[5:4]
    volatile uint32_t                            rfRsvd      :26; //[31:6]
} CANRegFieldGFC_t;

typedef union CANRegGFC
{
    volatile uint32_t                            rNReg;
    CANRegFieldGFC_t                    rFReg;
} CANRegGFC_t;

typedef struct CANRegFieldSIDFC
{
    volatile uint32_t                            rfRsvd      :2; //[1:0]
    volatile uint32_t                            rfFLSSA     :14; //[15:2]
    volatile uint32_t                            rfLSS       :8; //[23:16]
    volatile uint32_t                            rfRsvd1     :8; //[31:24]
} CANRegFieldSIDFC_t;

typedef union CANRegSIDFC
{
    volatile uint32_t                            rNReg;
    CANRegFieldSIDFC_t                  rFReg;
} CANRegSIDFC_t;

typedef struct CANRegFieldXIDFC
{
    volatile uint32_t                            rfRsvd      :2; //[1:0]
    volatile uint32_t                            rfFLESA     :14; //[15:2]
    volatile uint32_t                            rfLSE       :7; //[22:16]
    volatile uint32_t                            rfRsvd1     :9; //[31:23]
} CANRegFieldXIDFC_t;

typedef union CANRegXIDFC
{
    volatile uint32_t                            rNReg;
    CANRegFieldXIDFC_t                  rFReg;
} CANRegXIDFC_t;

typedef struct CANRegFieldXIDAM
{
    volatile uint32_t                            rfEIDM      :29; //[28:0]
    volatile uint32_t                            rfRsvd      :3; //[31:29]
} CANRegFieldXIDAM_t;

typedef union CANRegXIDAM
{
    volatile uint32_t                            rNReg;
    CANRegFieldXIDAM_t                  rFReg;
} CANRegXIDAM_t;

typedef struct CANRegFieldHPMS
{
    volatile uint32_t                            rfBIDX      :6; //[5:0]
    volatile uint32_t                            rfMSI       :2; //[7:6]
    volatile uint32_t                            rfFIDX      :7; //[14:8]
    volatile uint32_t                            rfFLST      :1; //[15]
    volatile uint32_t                            rfRsvd      :16; //[31:16]
} CANRegFieldHPMS_t;

typedef union CANRegHPMS
{
    volatile uint32_t                            rNReg;
    CANRegFieldHPMS_t                   rFReg;
} CANRegHPMS_t;

typedef struct CANRegFieldNDAT1
{
    volatile uint32_t                            rfND0       :1; //[0]
    volatile uint32_t                            rfND1       :1; //[1]
    volatile uint32_t                            rfND2       :1; //[2]
    volatile uint32_t                            rfND3       :1; //[3]
    volatile uint32_t                            rfND4       :1; //[4]
    volatile uint32_t                            rfND5       :1; //[5]
    volatile uint32_t                            rfND6       :1; //[6]
    volatile uint32_t                            rfND7       :1; //[7]
    volatile uint32_t                            rfND8       :1; //[8]
    volatile uint32_t                            rfND9       :1; //[9]
    volatile uint32_t                            rfND10      :1; //[10]
    volatile uint32_t                            rfND11      :1; //[11]
    volatile uint32_t                            rfND12      :1; //[12]
    volatile uint32_t                            rfND13      :1; //[13]
    volatile uint32_t                            rfND14      :1; //[14]
    volatile uint32_t                            rfND15      :1; //[15]
    volatile uint32_t                            rfND16      :1; //[16]
    volatile uint32_t                            rfND17      :1; //[17]
    volatile uint32_t                            rfND18      :1; //[18]
    volatile uint32_t                            rfND19      :1; //[19]
    volatile uint32_t                            rfND20      :1; //[20]
    volatile uint32_t                            rfND21      :1; //[21]
    volatile uint32_t                            rfND22      :1; //[22]
    volatile uint32_t                            rfND23      :1; //[23]
    volatile uint32_t                            rfND24      :1; //[24]
    volatile uint32_t                            rfND25      :1; //[25]
    volatile uint32_t                            rfND26      :1; //[26]
    volatile uint32_t                            rfND27      :1; //[27]
    volatile uint32_t                            rfND28      :1; //[28]
    volatile uint32_t                            rfND29      :1; //[29]
    volatile uint32_t                            rfND30      :1; //[30]
    volatile uint32_t                            rfND31      :1; //[31]
} CANRegFieldNDAT1_t;

typedef union CANRegNDAT1
{
    volatile uint32_t                            rNReg;
    CANRegFieldNDAT1_t                  rFReg;
} CANRegNDAT1_t;

typedef struct CANRegFieldNDAT2
{
    volatile uint32_t                            rfND32      :1; //[0]
    volatile uint32_t                            rfND33      :1; //[1]
    volatile uint32_t                            rfND34      :1; //[2]
    volatile uint32_t                            rfND35      :1; //[3]
    volatile uint32_t                            rfND36      :1; //[4]
    volatile uint32_t                            rfND37      :1; //[5]
    volatile uint32_t                            rfND38      :1; //[6]
    volatile uint32_t                            rfND39      :1; //[7]
    volatile uint32_t                            rfND40      :1; //[8]
    volatile uint32_t                            rfND41      :1; //[9]
    volatile uint32_t                            rfND42      :1; //[10]
    volatile uint32_t                            rfND43      :1; //[11]
    volatile uint32_t                            rfND44      :1; //[12]
    volatile uint32_t                            rfND45      :1; //[13]
    volatile uint32_t                            rfND46      :1; //[14]
    volatile uint32_t                            rfND47      :1; //[15]
    volatile uint32_t                            rfND48      :1; //[16]
    volatile uint32_t                            rfND49      :1; //[17]
    volatile uint32_t                            rfND50      :1; //[18]
    volatile uint32_t                            rfND51      :1; //[19]
    volatile uint32_t                            rfND52      :1; //[20]
    volatile uint32_t                            rfND53      :1; //[21]
    volatile uint32_t                            rfND54      :1; //[22]
    volatile uint32_t                            rfND55      :1; //[23]
    volatile uint32_t                            rfND56      :1; //[24]
    volatile uint32_t                            rfND57      :1; //[25]
    volatile uint32_t                            rfND58      :1; //[26]
    volatile uint32_t                            rfND59      :1; //[27]
    volatile uint32_t                            rfND60      :1; //[28]
    volatile uint32_t                            rfND61      :1; //[29]
    volatile uint32_t                            rfND62      :1; //[30]
    volatile uint32_t                            rfND63      :1; //[31]
} CANRegFieldNDAT2_t;

typedef union CANRegNDAT2
{
    volatile uint32_t                            rNReg;
    CANRegFieldNDAT2_t                  rFReg;
} CANRegNDAT2_t;

typedef struct CANRegFieldRXFxC
{
    volatile uint32_t                            rfRsvd      :2; //[1:0]
    volatile uint32_t                            rfFxSA      :14; //[15:2]
    volatile uint32_t                            rfFxS       :7; //[22:16]
    volatile uint32_t                            rfRsvd1     :1; //[23]
    volatile uint32_t                            rfFxWM      :7; //[30:24]
    volatile uint32_t                            rfFxOM      :1; //[31]
} CANRegFieldRXFxC_t;

typedef union CANRegRXFxC
{
    volatile uint32_t                            rNReg;
    CANRegFieldRXFxC_t                  rFReg;
} CANRegRXFxC_t;

typedef struct CANRegFieldRXFxS
{
    volatile uint32_t                            rfFxFL      :7; //[6:0]
    volatile uint32_t                            rfRsvd      :1; //[7]
    volatile uint32_t                            rfFxGI      :6; //[13:8]
    volatile uint32_t                            rfRsvd1     :2; //[15:14]
    volatile uint32_t                            rfFxPI      :6; //[21:16]
    volatile uint32_t                            rfRsvd2     :2; //[23:22]
    volatile uint32_t                            rfFxF       :1; //[24]
    volatile uint32_t                            rfRFxL      :1; //[25]
    volatile uint32_t                            rfRsvd3     :4; //[29:26]
    volatile uint32_t                            rfDMS       :2; //[31:30] only use in FIFO 1
} CANRegFieldRXFxS_t;

typedef union CANRegRXFxS
{
    volatile uint32_t                            rNReg;
    CANRegFieldRXFxS_t                  rFReg;
} CANRegRXFxS_t;

typedef struct CANRegFieldRXFxA
{
    volatile uint32_t                            rfFxAI      :6; //[5:0]
    volatile uint32_t                            rfRsvd      :26; //[31:6]
} CANRegFieldRXFxA_t;

typedef union CANRegRXFxA
{
    volatile uint32_t                            rNReg;
    CANRegFieldRXFxA_t                  rFReg;
} CANRegRXFxA_t;

typedef struct CANRegFieldRXBC
{
    volatile uint32_t                            rfRsvd      :2; //[1:0]
    volatile uint32_t                            rfRBSA      :14; //[15:2]
    volatile uint32_t                            rfRsvd1     :16; //[31:16]
} CANRegFieldRXBC_t;

typedef union CANRegRXBC
{
    volatile uint32_t                            rNReg;
    CANRegFieldRXBC_t                   rFReg;
} CANRegRXBC_t;

typedef struct CANRegFieldRXESC
{
    volatile uint32_t                            rfF0DS      :3; //[2:0]
    volatile uint32_t                            rfRsvd      :1; //[3]
    volatile uint32_t                            rfF1DS      :3; //[6:4]
    volatile uint32_t                            rfRsvd1     :1; //[7]
    volatile uint32_t                            rfRBDS      :3; //[10:8]
    volatile uint32_t                            rfRsvd2     :21; //[31:11]
} CANRegFieldRXESC_t;

typedef union CANRegRXESC
{
    volatile uint32_t                            rNReg;
    CANRegFieldRXESC_t                  rFReg;
} CANRegRXESC_t;

typedef struct CANRegFieldTXBC
{
    volatile uint32_t                            rfRsvd      :2; //[1:0]
    volatile uint32_t                            rfTBSA      :14; //[15:2]
    volatile uint32_t                            rfNDTB      :6; //[21:16]
    volatile uint32_t                            rfRsvd1     :2; //[23:22]
    volatile uint32_t                            rfTFQS      :6; //[29:24]
    volatile uint32_t                            rfTFQM      :1; //[30]
    volatile uint32_t                            rfRsvd2     :1; //[31]
} CANRegFieldTXBC_t;

typedef union CANRegTXBC
{
    volatile uint32_t                            rNReg;
    CANRegFieldTXBC_t                   rFReg;
} CANRegTXBC_t;

typedef struct CANRegFieldTXFQS
{
    volatile uint32_t                            rfTFFL      :6; //[5:0]
    volatile uint32_t                            rfRsvd      :2; //[7:6]
    volatile uint32_t                            rfTFGI      :6; //[13:8]
    volatile uint32_t                            rfRsvd1     :2; //[15:14]
    volatile uint32_t                            rfTFQPI     :5; //[20:16]
    volatile uint32_t                            rfTFQF      :1; //[21]
    volatile uint32_t                            rfRsvd2     :10; //[31:22]
} CANRegFieldTXFQS_t;

typedef union CANRegTXFQS
{
    volatile uint32_t                            rNReg;
    CANRegFieldTXFQS_t                  rFReg;
} CANRegTXFQS_t;

typedef struct CANRegFieldTXESC
{
    volatile uint32_t                            rfTBDS      :3; //[2:0]
    volatile uint32_t                            rfRsvd      :29; //[31:3]
} CANRegFieldTXESC_t;

typedef union CANRegTXESC
{
    volatile uint32_t                            rNReg;
    CANRegFieldTXESC_t                  rFReg;
} CANRegTXESC_t;

typedef struct CANRegFieldTXBRP
{
    volatile uint32_t                            rfTRP0      :1; //[0]
    volatile uint32_t                            rfTRP1      :1; //[1]
    volatile uint32_t                            rfTRP2      :1; //[2]
    volatile uint32_t                            rfTRP3      :1; //[3]
    volatile uint32_t                            rfTRP4      :1; //[4]
    volatile uint32_t                            rfTRP5      :1; //[5]
    volatile uint32_t                            rfTRP6      :1; //[6]
    volatile uint32_t                            rfTRP7      :1; //[7]
    volatile uint32_t                            rfTRP8      :1; //[8]
    volatile uint32_t                            rfTRP9      :1; //[9]
    volatile uint32_t                            rfTRP10     :1; //[10]
    volatile uint32_t                            rfTRP11     :1; //[11]
    volatile uint32_t                            rfTRP12     :1; //[12]
    volatile uint32_t                            rfTRP13     :1; //[13]
    volatile uint32_t                            rfTRP14     :1; //[14]
    volatile uint32_t                            rfTRP15     :1; //[15]
    volatile uint32_t                            rfTRP16     :1; //[16]
    volatile uint32_t                            rfTRP17     :1; //[17]
    volatile uint32_t                            rfTRP18     :1; //[18]
    volatile uint32_t                            rfTRP19     :1; //[19]
    volatile uint32_t                            rfTRP20     :1; //[20]
    volatile uint32_t                            rfTRP21     :1; //[21]
    volatile uint32_t                            rfTRP22     :1; //[22]
    volatile uint32_t                            rfTRP23     :1; //[23]
    volatile uint32_t                            rfTRP24     :1; //[24]
    volatile uint32_t                            rfTRP25     :1; //[25]
    volatile uint32_t                            rfTRP26     :1; //[26]
    volatile uint32_t                            rfTRP27     :1; //[27]
    volatile uint32_t                            rfTRP28     :1; //[28]
    volatile uint32_t                            rfTRP29     :1; //[29]
    volatile uint32_t                            rfTRP30     :1; //[30]
    volatile uint32_t                            rfTRP31     :1; //[31]
} CANRegFieldTXBRP_t;

typedef union CANRegTXBRP
{
    volatile uint32_t                            rNReg;
    CANRegFieldTXBRP_t                  rFReg;
} CANRegTXBRP_t;

typedef struct CANRegFieldTXEFC
{
    volatile uint32_t                            rfRsvd      :2; //[1:0]
    volatile uint32_t                            rfEFSA      :14; //[15:2]
    volatile uint32_t                            rfEFS       :6; //[21:16]
    volatile uint32_t                            rfRsvd1     :2; //[23:22]
    volatile uint32_t                            rfEFWM      :6; //[29:24]
    volatile uint32_t                            rfRsvd2     :2; //[31:30]
} CANRegFieldTXEFC_t;

typedef union CANRegTXEFC
{
    volatile uint32_t                            rNReg;
    CANRegFieldTXEFC_t                  rFReg;
} CANRegTXEFC_t;

typedef struct CANRegFieldTXEFS
{
    volatile uint32_t                            rfEFFL      :6; //[5:0]
    volatile uint32_t                            rfRsvd      :2; //[7:6]
    volatile uint32_t                            rfEFGI      :5; //[12:8]
    volatile uint32_t                            rfRsvd1     :3; //[15:13]
    volatile uint32_t                            rfEFPI      :5; //[20:16]
    volatile uint32_t                            rfRsvd2     :3; //[23:21]
    volatile uint32_t                            rfEFF       :1; //[24]
    volatile uint32_t                            rfTEFL      :1; //[25]
    volatile uint32_t                            rfRsvd3     :6; //[31:26]
} CANRegFieldTXEFS_t;

typedef union CANRegTXEFS
{
    volatile uint32_t                            rNReg;
    CANRegFieldTXEFS_t                  rFReg;
} CANRegTXEFS_t;

typedef struct CANRegFieldTXEFA
{
    volatile uint32_t                            rfEFAI      :5; //[4:0]
    volatile uint32_t                            rfRsvd      :27; //[31:5]
} CANRegFieldTXEFA_t;

typedef union CANRegTXEFA
{
    volatile uint32_t                            rNReg;
    CANRegFieldTXEFA_t                  rFReg;
} CANRegTXEFA_t;

typedef struct CANRegFieldTSCFG
{
    volatile uint32_t                            rfTSUE      :1; //[0]
    volatile uint32_t                            rfTBCS      :1; //[1]
    volatile uint32_t                            rfSCP       :1; //[2]
    volatile uint32_t                            rfRsvd      :5; //[7:3]
    volatile uint32_t                            rfTBPRE     :8; //[15:8]
    volatile uint32_t                            rfRsvd2     :16; //[31:16]
} CANRegFieldTSCFG_t;

typedef union CANRegTSCFG
{
    volatile uint32_t                            rNReg;
    CANRegFieldTSCFG_t                  rFReg;
} CANRegTSCFG_t;

/**************************************************************************************************
*
*                                      CAN Configuration Register MAP
*
**************************************************************************************************/
typedef struct CANRegFieldBaseAddr
{
    volatile uint32_t                            rfBASE_ADDR :16; //[15:0]
    volatile uint32_t                            rfRsvd      :16; //[31:16]
} CANRegFieldBaseAddr_t;

typedef union CANRegBaseAddr
{
    volatile uint32_t                            rNReg;
    CANRegFieldBaseAddr_t               rFReg;
} CANRegBaseAddr_t;

typedef struct CANRegFieldExtTSCtrl0
{
    volatile uint32_t                            rfENABLE    :1; //[0]
    volatile uint32_t                            rfCLEAR     :1; //[1]
    volatile uint32_t                            rfRsvd      :30; //[31:2]
} CANRegFieldExtTSCtrl0_t;

typedef union CANRegExtTSCtrl0
{
    volatile uint32_t                            rNReg;
    CANRegFieldExtTSCtrl0_t             rFReg;
} CANRegExtTSCtrl0_t;

typedef struct CANRegFieldExtTSCtrl1
{
    volatile uint32_t                            rfEXT_TIMESTAMP_DIV_RATIO :16; //[15:0]
    volatile uint32_t                            rfEXT_TIMESTAMP_COMP      :16; //[31:16]
} CANRegFieldExtTSCtrl1_t;

typedef union CANRegExtTSCtrl1
{
    volatile uint32_t                            rNReg;
    CANRegFieldExtTSCtrl1_t             rFReg;
} CANRegExtTSCtrl1_t;

#if 1 //unused structure
typedef struct CANRegExtTSValue
{
    volatile uint32_t                            rNReg;
} CANRegExtTSValue_t;

typedef struct CANRegCfgWrPw
{
    volatile uint32_t                            rNReg;
} CANRegCfgWrPw_t;

typedef struct CANRegFieldCfgWrLock
{
    volatile uint32_t                            rfCFG_WR_LOCK   :1; //[0]
    volatile uint32_t                            rfRsvd          :31; //[31:1]
} CANRegFieldCfgWrLock_t;

typedef union CANRegCfgWrLock
{
    volatile uint32_t                            rNReg;
    CANRegFieldCfgWrLock_t              rFReg;
} CANRegCfgWrLock_t;
#endif

/**************************************************************************************************
*
*                                      CAN Message RAM MAP
*
**************************************************************************************************/
typedef struct CANRamFieldRxBufferR0
{
    volatile uint32_t                            rfID        :29; //[28:0]
    volatile uint32_t                            rfRTR       :1; //[29]
    volatile uint32_t                            rfXTD       :1; //[30]
    volatile uint32_t                            rfESI       :1; //[31]
} CANRamFieldRxBufferR0_t;

typedef union CANRamRxBufferR0
{
    volatile uint32_t                            rNReg;
    CANRamFieldRxBufferR0_t             rFReg;
} CANRamRxBufferR0_t;

typedef struct CANRamFieldRxBufferR1
{
    volatile uint32_t                            rfRXTS      :16; //[15:0]
    volatile uint32_t                            rfDLC       :4; //[19:16]
    volatile uint32_t                            rfBRS       :1; //[20]
    volatile uint32_t                            rfFDF       :1; //[21]
    volatile uint32_t                            rfRsvd      :2; //[23:22]
    volatile uint32_t                            rfFIDX      :7; //[30:24]
    volatile uint32_t                            rfANMF      :1; //[31]
} CANRamFieldRxBufferR1_t;

typedef union CANRamRxBufferR1
{
    volatile uint32_t                            rNReg;
    CANRamFieldRxBufferR1_t             rFReg;
} CANRamRxBufferR1_t;

/*
typedef union CANRamRxBufferR2
{
    volatile uint32_t                            rNReg;
    CANRamFieldRxBufferR2_t             rFReg;
} CANRamRxBufferR2_t;
*/
typedef struct CANRamRxBuffer
{
    CANRamRxBufferR0_t                  rR0;
    CANRamRxBufferR1_t                  rR1;
   // CANRamRxBufferR2_t                  rR2;
    uint32_t                              rData[CAN_DATA_LENGTH_SIZE / sizeof(uint32_t)];
} CANRamRxBuffer_t;

typedef struct CANRamFieldTxBufferT0
{
    volatile uint32_t                            rfID        :29; //[28:0]
    volatile uint32_t                            rfRTR       :1; //[29]
    volatile uint32_t                            rfXTD       :1; //[30]
    volatile uint32_t                            rfESI       :1; //[31]
} CANRamFieldTxBufferT0_t;

typedef union CANRamTxBufferT0
{
    volatile uint32_t                            rNReg;
    CANRamFieldTxBufferT0_t             rFReg;
} CANRamTxBufferT0_t;

typedef struct CANRamFieldTxBufferT1
{
    volatile uint32_t                            rfRsvd      :16; //[15:0]
    volatile uint32_t                            rfDLC       :4; //[19:16]
    volatile uint32_t                            rfBRS       :1; //[20]
    volatile uint32_t                            rfFDF       :1; //[21]
    volatile uint32_t                            rfTSCE      :1; //[22]
    volatile uint32_t                            rfEFC       :1; //[23]
    volatile uint32_t                            rfMM        :8; //[31:24]
} CANRamFieldTxBufferT1_t;

typedef union CANRamTxBufferT1
{
    volatile uint32_t                            rNReg;
    CANRamFieldTxBufferT1_t             rFReg;
} CANRamTxBufferT1_t;

typedef struct CANRamTxBuffer
{
    CANRamTxBufferT0_t                  rT0;
    CANRamTxBufferT1_t                  rT1;
    uint32_t                              rData[16];
} CANRamTxBuffer_t;

typedef struct CANRamFieldStdIDFilterElement
{
    volatile uint32_t                            rfSFID2     :11; //[10:0]
    volatile uint32_t                            rfRsvd      :4; //[14:11]
    volatile uint32_t                            rfSSYNC     :1; //[15]
    volatile uint32_t                            rfSFID1     :11; //[26:16]
    volatile uint32_t                            rfSFEC      :3; //[29:27]
    volatile uint32_t                            rfSFT       :2; //[31:30]
} CANRamFieldStdIDFilterElement_t;

typedef union CANRamStdIDFilterElement
{
    volatile uint32_t                            rNReg;
    CANRamFieldStdIDFilterElement_t     rFReg;
} CANRamStdIDFilterElement_t;

typedef struct CANRamFieldExtIDFilterF0Element
{
    volatile uint32_t                            rfEFID1     :29; //[28:0]
    volatile uint32_t                            rfEFEC      :3; //[31:29]
} CANRamFieldExtIDFilterF0Element_t;

typedef union CANRamExtIDFilterF0Element
{
    volatile uint32_t                            rNReg;
    CANRamFieldExtIDFilterF0Element_t   rFReg;
} CANRamExtIDFilterF0Element_t;

typedef struct CANRamFieldExtIDFilterF1Element
{
    volatile uint32_t                            rfEFID2     :29; //[28:0]
    volatile uint32_t                            rfRsvd      :1; //[29]
    volatile uint32_t                            rfEFT       :2; //[31:30]
} CANRamFieldExtIDFilterF1Element_t;

typedef union CANRamExtIDFilterF1Element
{
    volatile uint32_t                            rNReg;
    CANRamFieldExtIDFilterF1Element_t   rFReg;
} CANRamExtIDFilterF1Element_t;

typedef struct CANRamExtIDFilterElement
{
    CANRamExtIDFilterF0Element_t        rF0;
    CANRamExtIDFilterF1Element_t        rF1;
} CANRamExtIDFilterElement_t;

typedef struct CANRamFieldTxEventFIFOE0
{
    volatile uint32_t                            rfID        :29; //[28:0]
    volatile uint32_t                            rfRTR       :1; //[29]
    volatile uint32_t                            rfXTD       :1; //[30]
    volatile uint32_t                            rfESI       :1; //[31]
} CANRamFieldTxEventFIFOE0_t;

typedef union CANRamTxEventFIFOE0
{
    volatile uint32_t                            rNReg;
    CANRamFieldTxEventFIFOE0_t          rFReg;
} CANRamTxEventFIFOE0_t;

typedef struct CANRamFieldTxEventFIFOE1
{
    volatile uint32_t                            rfTXTS  :16; //[15:0]
    volatile uint32_t                            rfDLC   :4; //[19:16]
    volatile uint32_t                            rfBRS   :1; //[20]
    volatile uint32_t                            rfFDF   :1; //[21]
    volatile uint32_t                            rfET    :2; //[23:22]
    volatile uint32_t                            rfMM    :8; //[31:24]
} CANRamFieldTxEventFIFOE1_t;

typedef union CANRamTxEventFIFOE1
{
    volatile uint32_t                            rNReg;
    CANRamFieldTxEventFIFOE1_t          rFReg;
} CANRamTxEventFIFOE1_t;

typedef struct CANRamTxEventFIFO
{
    CANRamTxEventFIFOE0_t               rE0;
    CANRamTxEventFIFOE1_t               rE1;
} CANRamTxEventFIFO_t;

typedef struct CANControllerRegister
{
    CANRegCREL_t                        crCoreRelease;
    CANRegENDN_t                        crEndian;
    uint32_t                              crRsvd;
    CANRegDBTP_t                        crDataBitTiming;
    CANRegTEST_t                        crTest;
    CANRegRWD_t                         crRAMWatchdog;
    CANRegCCCR_t                        crCCControl;
    CANRegNBTP_t                        crNominalBitTiming;
    CANRegTSCC_t                        crTimestampCounterConfiguration;
    CANRegTSCV_t                        crTimestampCounterValue;
    CANRegTOCC_t                        crTimeoutCounterConfiguration;
    CANRegTOCV_t                        crTimeoutCounterValue;
    uint32_t                              crRsvd1[4];
    CANRegECR_t                         crErrorCounterRegister;
    CANRegPSR_t                         crProtocolStatusRegister;
    CANRegTDCR_t                        crTransmitterDelayCompensation;
    uint32_t                              crRsvd2;
    CANRegIR_t                          crInterruptRegister;
    CANRegIE_t                          crInterruptEnable;
    CANRegILS_t                         crInterruptLineSelect;
    CANRegILE_t                         crInterruptLineEnable;
    uint32_t                              crRsvd3[8];
    CANRegGFC_t                         crGlobalFilterConfiguration;
    CANRegSIDFC_t                       crStandardIDFilterConfiguration;
    CANRegXIDFC_t                       crExtendedIDFilterConfiguration;
    uint32_t                              crRsvd4;
    CANRegXIDAM_t                       crExtendedIDANDMask;
    CANRegHPMS_t                        crHighPriorityMessageStatus;
    CANRegNDAT1_t                       crNewData1;
    CANRegNDAT2_t                       crNewData2;
    CANRegRXFxC_t                       crRxFIFO0Configuration;
    CANRegRXFxS_t                       crRxFIFO0Status;
    CANRegRXFxA_t                       crRxFIFO0Acknowledge;
    CANRegRXBC_t                        crRxBufferConfiguration;
    CANRegRXFxC_t                       crRxFIFO1Configuration;
    CANRegRXFxS_t                       crRxFIFO1Status;
    CANRegRXFxA_t                       crRxFIFO1Acknowledge;
    CANRegRXESC_t                       crRxBufferOrFIFOElementSizeConfiguration;
    CANRegTXBC_t                        crTxBufferConfiguration;
    CANRegTXFQS_t                       crTxFIFOOrQueueStatus;
    CANRegTXESC_t                       crTxBufferElementSizeConfiguration;
    CANRegTXBRP_t                       crTxBufferRequestPending;
    uint32_t                              crTxBufferAddRequest;
    uint32_t                              crTxBufferCancellationRequest;
    uint32_t                              crTxBufferTransmissionOccurred;
    uint32_t                              crTxBufferCancellationFinished;
    uint32_t                              crTxBufferTransmissionInterruptEnable;
    uint32_t                              crTxBufferCancellationFinishedInterruptEnable;
    uint32_t                              crRsvd5[2];
    CANRegTXEFC_t                       crTxEventFIFOConfiguration;
    CANRegTXEFS_t                       crTxEventFIFOStatus;
    CANRegTXEFA_t                       crTxEventFIFOAcknowledge;
    uint32_t                              crRsvd6[25];
    CANRegCREL_t                        crCoreRelease2;
	CANRegTSCFG_t						crTimeStampConfiguration;
} CANControllerRegister_t;

#if 1 //unused structure
typedef struct CANConfigRegister
{
    uint32_t                              crRsvd;
    CANRegBaseAddr_t                    crBaseAddr[3];
    CANRegExtTSCtrl0_t                  crExtTimeStampCtrl0;
    CANRegExtTSCtrl1_t                  crExtTimeStampCtrl1;
    CANRegExtTSValue_t                  crExtTimeStampValue;
    uint32_t                              crRsvd1;
    CANRegCfgWrPw_t                     crCfgWrPw;
    CANRegCfgWrLock_t                   crCfgWrLock;
} CANConfigRegister_t;
#endif


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/


#endif // __CAN_REG_HEADER

