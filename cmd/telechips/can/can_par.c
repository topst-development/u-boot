// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include <test/can_config.h>
#include <test/can_reg.h>
#include <test/can.h>
#include <test/can_par.h>


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

static CANIdFilterList_t StandardIDFilterPar_CH0[ CAN_STANDARD_ID_FILTER_NUMBER ];
static CANIdFilterList_t ExtendedIDFilterPar_CH0[ CAN_EXTENDED_ID_FILTER_NUMBER ];
static CANIdFilterList_t StandardIDFilterPar_CH1[ CAN_STANDARD_ID_FILTER_NUMBER ];
static CANIdFilterList_t ExtendedIDFilterPar_CH1[ CAN_EXTENDED_ID_FILTER_NUMBER ];
#if defined(CONFIG_TCC807X)
static CANIdFilterList_t StandardIDFilterPar_CH2[ CAN_STANDARD_ID_FILTER_NUMBER ];
static CANIdFilterList_t ExtendedIDFilterPar_CH2[ CAN_EXTENDED_ID_FILTER_NUMBER ];
#endif

#if 1/* can_par.c */

#define TIMING_CASE (0)

/**************************************************************************************************
*
*                                      CAN Timing Information
*
**************************************************************************************************/

/* Arbitration Phase Timing Information */
CANTimingParam_t ArbitrationPhaseTimingPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */

#if (TIMING_CASE == 0)//Peri:80MHz, 1MHz - sample point:75%
#if defined(CONFIG_TCC807X)
    { 5,     8,      3,      4,      4,      0,      0,      0 },
    { 5,     8,      3,      4,      4,      0,      0,      0 },
    { 5,     8,      3,      4,      4,      0,      0,      0 }
#elif defined(CONFIG_TCC750X)
    { 5,     8,      3,      4,      4,      0,      0,      0 },
    { 5,     8,      3,      4,      4,      0,      0,      0 }
#endif
#elif (TIMING_CASE == 1) //Peri:24MHz - for format error
#if defined(CONFIG_TCC807X)
    { 2,     6,      2,      3,      3,      0,      0,      0 },
    { 2,     6,      2,      3,      3,      0,      0,      0 },
    { 2,     6,      2,      3,      3,      0,      0,      0 }
#elif defined(CONFIG_TCC750X)
    { 2,     6,      2,      3,      3,      0,      0,      0 },
    { 2,     6,      2,      3,      3,      0,      0,      0 }
#endif
#elif (TIMING_CASE == 2) //Peri:24MHz - for BIT1 error
#if defined(CONFIG_TCC807X)
    { 2,     6,      2,      3,      3,      0,      0,      0 },
    { 2,     6,      2,      3,      3,      0,      0,      0 },
    { 2,     6,      2,      3,      3,      0,      0,      0 }
#elif defined(CONFIG_TCC750X)
    { 2,     6,      2,      3,      3,      0,      0,      0 },
    { 2,     6,      2,      3,      3,      0,      0,      0 }
#endif
#else
    #### error ####
#endif
};

/* Data Phase Timing Information */
CANTimingParam_t DataPhaseTimingPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */

#if (TIMING_CASE == 0)//Peri:80MHz, 5MHz - sample point:75%
#if defined(CONFIG_TCC807X)
       { 1,     8,      3,      4,      4,      0,      0,      0 },
       { 1,     8,      3,      4,      4,      0,      0,      0 },
       { 1,     8,      3,      4,      4,      0,      0,      0 }
#elif defined(CONFIG_TCC750X)
       { 1,     8,      3,      4,      4,      0,      0,      0 },
       { 1,     8,      3,      4,      4,      0,      0,      0 }
#endif
#elif (TIMING_CASE == 1) //Peri:24MHz - for format error
#if defined(CONFIG_TCC807X)
       { 1,     0,      0,      0,      6,      1,      14,     0 },
       { 1,     0,      0,      0,      6,      1,      14,     0 },
       { 1,     0,      0,      0,      6,      1,      14,     0 }
#elif defined(CONFIG_TCC750X)
       { 1,     0,      0,      0,      6,      1,      14,     0 },
       { 1,     0,      0,      0,      6,      1,      14,     0 }
#endif
#elif (TIMING_CASE == 2) //Peri:24MHz - for BIT1 error
#if defined(CONFIG_TCC807X)
       { 2,     0,      0,      3,      3,      0,      0,      0 },
       { 2,     0,      0,      3,      3,      0,      0,      0 },
       { 2,     0,      0,      3,      3,      0,      0,      0 }
#elif defined(CONFIG_TCC750X)
       { 2,     0,      0,      3,      3,      0,      0,      0 },
       { 2,     0,      0,      3,      3,      0,      0,      0 }
#endif
#else
    #### error ####
#endif
};

/**************************************************************************************************
*
*                                      CAN Buffer Information
*
**************************************************************************************************/

/* TX Buffer */
CANTxBuffer_t TxBufferInfoPar[ CAN_CONTROLLER_NUMBER ] =
{
#if defined(CONFIG_TCC807X)
   /* Buffer count,  FIFO,QUEUE count, FIFO,QUEUE mode,          Buffer size,                Interrupt Config */
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL },
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL },
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL }
#elif defined(CONFIG_TCC750X)
   /* Buffer count,  FIFO,QUEUE count, FIFO,QUEUE mode,          Buffer size,                Interrupt Config */
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL },
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL }
#endif
};

/* RX Dedicated Buffer */
CANRxBuffer_t RxBufferInfoPar[ CAN_CONTROLLER_NUMBER ] =
{
#if defined(CONFIG_TCC807X)
   /* Don't care,    Data Field Size,                Don't care */
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING },
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING },
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING }
#elif defined(CONFIG_TCC750X)
   /* Don't care,    Data Field Size,                Don't care */
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING },
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING }
#endif
};

/* RX FIFO 0 */
CANRxBuffer_t RxBufferFIFO0InfoPar[ CAN_CONTROLLER_NUMBER ] =
{
#if defined(CONFIG_TCC807X)
   /* Buffer count,  Data Field Size,            Operation mode */
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE }
#elif defined(CONFIG_TCC750X)
   /* Buffer count,  Data Field Size,            Operation mode */
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE }
#endif
};

/* RX FIFO 1 */
CANRxBuffer_t RxBufferFIFO1InfoPar[ CAN_CONTROLLER_NUMBER ] =
{
#if defined(CONFIG_TCC807X)
   /* Buffer count,  Data Field Size,            Operation mode */
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE }
#elif defined(CONFIG_TCC750X)
   /* Buffer count,  Data Field Size,            Operation mode */
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE }
#endif
};


#define CAN_DEBUG_MESSAGE (0)

/**************************************************************************************************
*
*                                      CAN Standard ID Filter
*
**************************************************************************************************/

static CANIdFilterList_t StandardIDFilterPar_CH0[ CAN_STANDARD_ID_FILTER_NUMBER ] =
{
   /* FilterType,               FilterElementConfiguration,         FilterID1,  FilterID2 */
    //Range
#if (CAN_DEBUG_MESSAGE == 0)
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_DISABLE_ELEMENT,     0x01,       0x10 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXFIFO0,             0x11,       0x20 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXFIFO1,             0x21,       0x30 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_REJECT_ID,           0x31,       0x40 },
#else
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x3D,       0x200 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x3E,       0x400 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x3F,       0x600 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_REJECT_ID,           0x31,       0x32 },
#endif
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY,                 0x41,       0x50 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x51,       0x60 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x61,       0x70 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x71,       0x0 },
	//Dual
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_DISABLE_ELEMENT,     0x81,       0x90 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXFIFO0,             0x91,       0x100 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXFIFO1,             0x101,      0x110 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_REJECT_ID,           0x111,      0x120 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY,                 0x121,      0x130 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x131,      0x140 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x141,      0x150 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXBUFFER,            0x151,      0x0 },
	//Classic
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_DISABLE_ELEMENT,     0x211,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXFIFO0,             0x221,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXFIFO1,             0x231,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_REJECT_ID,           0x241,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY,                 0x251,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x261,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x271,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXBUFFER,            0x281,      0x0 },
	//Disable
	{CAN_FILTER_TYPE_ELEMENTDISABLED, CAN_FILTER_CFG_DISABLE_ELEMENT, 0x291,    0x291 },
};

static CANIdFilterList_t StandardIDFilterPar_CH1[ CAN_STANDARD_ID_FILTER_NUMBER ] =
{
   /* FilterType,               FilterElementConfiguration,         FilterID1,  FilterID2 */
    //Range
#if (CAN_DEBUG_MESSAGE == 0)
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_DISABLE_ELEMENT,     0x01,       0x10 },
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXFIFO0,             0x11,       0x20 },
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXFIFO1,             0x21,       0x30 },
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_REJECT_ID,           0x31,       0x40 },
#else
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x3D,       0x200 },
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x3E,       0x400 },
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x3F,       0x600 },
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_REJECT_ID,           0x31,       0x32 },
#endif
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY,                 0x41,       0x50 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x51,       0x60 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x61,       0x70 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x71,       0x0 },
	//Dual
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_DISABLE_ELEMENT,     0x81,       0x90 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXFIFO0,             0x91,       0x100 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXFIFO1,             0x101,      0x110 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_REJECT_ID,           0x111,      0x120 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY,                 0x121,      0x130 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x131,      0x140 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x141,      0x150 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXBUFFER,            0x151,      0x0 },
	//Classic
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_DISABLE_ELEMENT,     0x211,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXFIFO0,             0x221,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXFIFO1,             0x231,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_REJECT_ID,           0x241,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY,                 0x251,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x261,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x271,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXBUFFER,            0x281,      0x0 },
	//Disable
	{CAN_FILTER_TYPE_ELEMENTDISABLED, CAN_FILTER_CFG_DISABLE_ELEMENT, 0x291,    0x291 },
};

#if defined(CONFIG_TCC807X)
#if 1
static CANIdFilterList_t StandardIDFilterPar_CH2[ CAN_STANDARD_ID_FILTER_NUMBER ] =
{
   /* FilterType,               FilterElementConfiguration,         FilterID1,  FilterID2 */
    //Range
#if (CAN_DEBUG_MESSAGE == 0)
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_DISABLE_ELEMENT,     0x01,       0x10 },
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXFIFO0,             0x11,       0x20 },
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXFIFO1,             0x21,       0x30 },
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_REJECT_ID,           0x31,       0x40 },
#else
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x3D,       0x200 },
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x3E,       0x400 },
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x3F,       0x600 },
    {CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_REJECT_ID,           0x31,       0x32 },
#endif
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY,                 0x41,       0x50 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x51,       0x60 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x61,       0x70 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x71,       0x0 },
	//Dual
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_DISABLE_ELEMENT,     0x81,       0x90 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXFIFO0,             0x91,       0x100 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXFIFO1,             0x101,      0x110 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_REJECT_ID,           0x111,      0x120 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY,                 0x121,      0x130 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x131,      0x140 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x141,      0x150 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXBUFFER,            0x151,      0x0 },
	//Classic
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_DISABLE_ELEMENT,     0x211,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXFIFO0,             0x221,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXFIFO1,             0x231,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_REJECT_ID,           0x241,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY,                 0x251,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x261,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x271,      0x7F0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXBUFFER,            0x281,      0x0 },
	//Disable
	{CAN_FILTER_TYPE_ELEMENTDISABLED, CAN_FILTER_CFG_DISABLE_ELEMENT, 0x291,    0x291 },
};
#endif
#endif
CANIdFilter_t StandardIDFilterPar =
{
#if defined(CONFIG_TCC807X)
    { &StandardIDFilterPar_CH0[ 0 ], &StandardIDFilterPar_CH1[ 0 ],
	&StandardIDFilterPar_CH2[ 0 ]
    },
#elif defined(CONFIG_TCC750X)
    { &StandardIDFilterPar_CH0[ 0 ], &StandardIDFilterPar_CH1[ 0 ]
    },
#endif
};


/**************************************************************************************************
*
*                                      CAN Extended ID Filter
*
**************************************************************************************************/

static CANIdFilterList_t ExtendedIDFilterPar_CH0[ CAN_EXTENDED_ID_FILTER_NUMBER ] =
{
   /* FilterType,               FilterElementConfiguration,         FilterID1,  FilterID2 */
    //Range
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_DISABLE_ELEMENT,     0x1001,     0x1010 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXFIFO0,             0x1011,     0x1020 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXFIFO1,             0x1021,     0x1030 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_REJECT_ID,           0x1031,     0x1040 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY,                 0x1041,     0x1050 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x1051,     0x1060 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x1061,     0x1070 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x1071,     0x0 },
	//Dual
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_DISABLE_ELEMENT,     0x1081,     0x1090 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXFIFO0,             0x1091,     0x1100 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXFIFO1,             0x1101,     0x1110 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_REJECT_ID,           0x1111,     0x1120 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY,                 0x1121,     0x1130 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x1131,     0x1140 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x1141,     0x1150 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXBUFFER,            0x1151,     0x0 },
	//Classic
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_DISABLE_ELEMENT,     0x1211,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXFIFO0,             0x1221,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXFIFO1,             0x1231,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_REJECT_ID,           0x1241,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY,                 0x1251,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x1261,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x1271,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXBUFFER,            0x1281,     0x0 },
	//Disable
	{CAN_FILTER_TYPE_ELEMENTDISABLED, CAN_FILTER_CFG_DISABLE_ELEMENT, 0x1291,   0x1291 },
};

static CANIdFilterList_t ExtendedIDFilterPar_CH1[ CAN_EXTENDED_ID_FILTER_NUMBER ] =
{
   /* FilterType,               FilterElementConfiguration,         FilterID1,  FilterID2 */
    //Range
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_DISABLE_ELEMENT,     0x1001,     0x1010 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXFIFO0,             0x1011,     0x1020 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXFIFO1,             0x1021,     0x1030 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_REJECT_ID,           0x1031,     0x1040 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY,                 0x1041,     0x1050 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x1051,     0x1060 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x1061,     0x1070 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x1071,     0x0 },
	//Dual
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_DISABLE_ELEMENT,     0x1081,     0x1090 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXFIFO0,             0x1091,     0x1100 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXFIFO1,             0x1101,     0x1110 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_REJECT_ID,           0x1111,     0x1120 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY,                 0x1121,     0x1130 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x1131,     0x1140 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x1141,     0x1150 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXBUFFER,            0x1151,     0x0 },
	//Classic
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_DISABLE_ELEMENT,     0x1211,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXFIFO0,             0x1221,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXFIFO1,             0x1231,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_REJECT_ID,           0x1241,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY,                 0x1251,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x1261,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x1271,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXBUFFER,            0x1281,     0x0 },
	//Disable
	{CAN_FILTER_TYPE_ELEMENTDISABLED, CAN_FILTER_CFG_DISABLE_ELEMENT, 0x1291,   0x1291 },
};

#if defined(CONFIG_TCC807X)
#if 1
static CANIdFilterList_t ExtendedIDFilterPar_CH2[ CAN_EXTENDED_ID_FILTER_NUMBER ] =
{
   /* FilterType,               FilterElementConfiguration,         FilterID1,  FilterID2 */
    //Range
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_DISABLE_ELEMENT,     0x1001,     0x1010 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXFIFO0,             0x1011,     0x1020 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXFIFO1,             0x1021,     0x1030 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_REJECT_ID,           0x1031,     0x1040 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY,                 0x1041,     0x1050 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x1051,     0x1060 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x1061,     0x1070 },
	{CAN_FILTER_TYPE_RANGE,     CAN_FILTER_CFG_RXBUFFER,            0x1071,     0x0 },
	//Dual
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_DISABLE_ELEMENT,     0x1081,     0x1090 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXFIFO0,             0x1091,     0x1100 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXFIFO1,             0x1101,     0x1110 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_REJECT_ID,           0x1111,     0x1120 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY,                 0x1121,     0x1130 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x1131,     0x1140 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x1141,     0x1150 },
	{CAN_FILTER_TYPE_DUAL,      CAN_FILTER_CFG_RXBUFFER,            0x1151,     0x0 },
	//Classic
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_DISABLE_ELEMENT,     0x1211,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXFIFO0,             0x1221,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXFIFO1,             0x1231,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_REJECT_ID,           0x1241,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY,                 0x1251,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY_N_RXFIFO0,       0x1261,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_PRY_N_RXFIFO1,       0x1271,     0xFFF0 },
	{CAN_FILTER_TYPE_CLASSIC,   CAN_FILTER_CFG_RXBUFFER,            0x1281,     0x0 },
	//Disable
	{CAN_FILTER_TYPE_ELEMENTDISABLED, CAN_FILTER_CFG_DISABLE_ELEMENT, 0x1291,   0x1291 },
};
#endif
#endif
#else
/**************************************************************************************************
*
*                                      CAN Timing Information
*
**************************************************************************************************/

/* Arbitration Phase Timing Information */
CANTimingParam_t ArbitrationPhaseTimingPar[ CAN_CONTROLLER_NUMBER ] =
{
#if 0//def CAN_FPGA_TEST
#if defined(CONFIG_TCC807X)
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */
    { 4,     0,      10,     1,      1,      0,      0,      0 },  //channel 0, speed : 250Kbps / sample point : 87.5%
    { 4,     0,      10,     1,      1,      0,      0,      0 },  //channel 1, speed : 250Kbps / sample point : 87.5%
    { 4,     0,      10,     1,      1,      0,      0,      0 },  //channel 2, speed : 250Kbps / sample point : 87.5%
#elif defined(CONFIG_TCC750X)
    { 4,     0,      10,     1,      1,      0,      0,      0 },  //channel 0, speed : 250Kbps / sample point : 87.5%
    { 4,     0,      10,     1,      1,      0,      0,      0 }  //channel 1, speed : 250Kbps / sample point : 87.5%
#endif
#else
#if defined(CONFIG_TCC807X)
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */
    { 1,     47,     16,     16,     16,     0,      0,      0 },  //channel 0, speed : 500Kbps / sample point : 80%
    { 1,     47,     16,     16,     16,     0,      0,      0 },  //channel 1, speed : 500Kbps / sample point : 80%
    { 1,     47,     16,     16,     16,     0,      0,      0 }   //channel 2, speed : 500Kbps / sample point : 80%
#elif defined(CONFIG_TCC750X)
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */
    { 1,     47,     16,     16,     16,     0,      0,      0 },  //channel 0, speed : 500Kbps / sample point : 80%
    { 1,     47,     16,     16,     16,     0,      0,      0 }  //channel 1, speed : 500Kbps / sample point : 80%
#endif
#endif
};

/* Data Phase Timing Information */
CANTimingParam_t DataPhaseTimingPar[ CAN_CONTROLLER_NUMBER ] =
{
#if 0//def CAN_FPGA_TEST
#if defined(CONFIG_TCC807X)
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */
    { 4,     0,      10,     1,      1,      0,      0,      0 },  //channel 0, speed : 250Kbps / sample point : 87.5%
    { 4,     0,      10,     1,      1,      0,      0,      0 },  //channel 1, speed : 250Kbps / sample point : 87.5%
    { 4,     0,      10,     1,      1,      0,      0,      0 },  //channel 2, speed : 250Kbps / sample point : 87.5%
#elif defined(CONFIG_TCC750X)
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */
    { 4,     0,      10,     1,      1,      0,      0,      0 },  //channel 0, speed : 250Kbps / sample point : 87.5%
    { 4,     0,      10,     1,      1,      0,      0,      0 }  //channel 1, speed : 250Kbps / sample point : 87.5%
#endif
#else
#if defined(CONFIG_TCC807X)
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */
    { 1,     0,      13,     6,      6,      1,      14,     0 },  //channel 0, speed : 2Mbps / sample point : 70%
    { 1,     0,      13,     6,      6,      1,      14,     0 },  //channel 1, speed : 2Mbps / sample point : 70%
    { 1,     0,      13,     6,      6,      1,      14,     0 }   //channel 2, speed : 2Mbps / sample point : 70%
#elif defined(CONFIG_TCC750X)
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */
    { 1,     0,      13,     6,      6,      1,      14,     0 },  //channel 0, speed : 2Mbps / sample point : 70%
    { 1,     0,      13,     6,      6,      1,      14,     0 }  //channel 1, speed : 2Mbps / sample point : 70%
#endif
#endif
};


/**************************************************************************************************
*
*                                      CAN Buffer Information
*
**************************************************************************************************/

/* TX Buffer */
CANTxBuffer_t TxBufferInfoPar[ CAN_CONTROLLER_NUMBER ] =
{
#if defined(CONFIG_TCC807X)
   /* Buffer count,  FIFO,QUEUE count, FIFO,QUEUE mode,          Buffer size,                Interrupt Config */
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL },
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL },
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL }
#elif defined(CONFIG_TCC750X)
   /* Buffer count,  FIFO,QUEUE count, FIFO,QUEUE mode,          Buffer size,                Interrupt Config */
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL },
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL }
#endif
};

/* RX Dedicated Buffer */
CANRxBuffer_t RxBufferInfoPar[ CAN_CONTROLLER_NUMBER ] =
{
#if defined(CONFIG_TCC807X)
   /* Don't care,    Data Field Size,                Don't care */
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING },
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING },
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING }
#elif defined(CONFIG_TCC750X)
   /* Don't care,    Data Field Size,                Don't care */
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING },
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING },
#endif
};

/* RX FIFO 0 */
CANRxBuffer_t RxBufferFIFO0InfoPar[ CAN_CONTROLLER_NUMBER ] =
{
#if defined(CONFIG_TCC807X)
   /* Buffer count,  Data Field Size,            Operation mode */
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE }
#elif defined(CONFIG_TCC750X)
   /* Buffer count,  Data Field Size,            Operation mode */
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
#endif
};

/* RX FIFO 1 */
CANRxBuffer_t RxBufferFIFO1InfoPar[ CAN_CONTROLLER_NUMBER ] =
{
#if defined(CONFIG_TCC807X)
   /* Buffer count,  Data Field Size,            Operation mode */
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE }
#elif defined(CONFIG_TCC750X)
   /* Buffer count,  Data Field Size,            Operation mode */
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE }
#endif
};


/**************************************************************************************************
*
*                                      CAN Standard ID Filter
*
**************************************************************************************************/

static CANIdFilterList_t StandardIDFilterPar_CH0[ CAN_STANDARD_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
#if 1
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x000,      0x100 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x101,      0x200 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x201,      0x300 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x301,      0x400 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x401,      0x0   },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x402,      0x7FF }
#else
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,         0x3D,      0x200 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,         0x3E,      0x400 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,   	  0x3F,      0x600 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x301,      0x800 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x401,      0x0   },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x402,      0x7FF }
#endif
};

static CANIdFilterList_t StandardIDFilterPar_CH1[ CAN_STANDARD_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
#if 1
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x000,      0x100 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x101,      0x200 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x201,      0x300 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x301,      0x400 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x401,      0x0   },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x402,      0x7FF }
#else
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,         0x3D,      0x200 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,         0x3E,      0x400 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,   	  0x3F,      0x600 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x301,      0x800 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x401,      0x0   },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x402,      0x7FF }
#endif
};

#if defined(CONFIG_TCC807X)
static CANIdFilterList_t StandardIDFilterPar_CH2[ CAN_STANDARD_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x000,      0x100 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x101,      0x200 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x201,      0x300 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x301,      0x400 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x401,      0x0   },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x402,      0x7FF }
};
#endif

CANIdFilter_t StandardIDFilterPar =
{
#if defined(CONFIG_TCC807X)
    { &StandardIDFilterPar_CH0[ 0 ], &StandardIDFilterPar_CH1[ 0 ], &StandardIDFilterPar_CH2[ 0 ] },
#elif defined(CONFIG_TCC750X)
    { &StandardIDFilterPar_CH0[ 0 ], &StandardIDFilterPar_CH1[ 0 ]},
#endif
};


/**************************************************************************************************
*
*                                      CAN Extended ID Filter
*
**************************************************************************************************/

static CANIdFilterList_t ExtendedIDFilterPar_CH0[ CAN_EXTENDED_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x0000,     0x1000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x1001,     0x2000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x2001,     0x3000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x3001,     0x4000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x4001,     0x0        },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x4002,     0x1FFFFFFF }
};

static CANIdFilterList_t ExtendedIDFilterPar_CH1[ CAN_EXTENDED_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x0000,     0x1000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x1001,     0x2000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x2001,     0x3000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x3001,     0x4000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x4001,     0x0        },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x4002,     0x1FFFFFFF }
};

static CANIdFilterList_t ExtendedIDFilterPar_CH2[ CAN_EXTENDED_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x0000,     0x1000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x1001,     0x2000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x2001,     0x3000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x3001,     0x4000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x4001,     0x0        },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x4002,     0x1FFFFFFF }
};
#endif

CANIdFilter_t ExtendedIDFilterPar =
{
#if defined(CONFIG_TCC807X)
    { &ExtendedIDFilterPar_CH0[ 0 ], &ExtendedIDFilterPar_CH1[ 0 ],
	&ExtendedIDFilterPar_CH2[ 0 ]
    },
#elif defined(CONFIG_TCC750X)
    { &ExtendedIDFilterPar_CH0[ 0 ], &ExtendedIDFilterPar_CH1[ 0 ]
    },
#endif
};


/**************************************************************************************************
*
*                                      CAN Call Back Function
*
**************************************************************************************************/

CANCallBackFunc_t CANCallbackFunctions =
{
    NULL,  //cbNotifyRxEvent
    NULL,  //cbNotifyTxEvent
    NULL,  //cbNotifyErrorEvent
};


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

