// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef __BSP_CAN_HEADER
#define __BSP_CAN_HEADER
#include <stdlib.h>
#include <test/can_porting_chip.h>
/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

typedef enum CANFlagValue
{
    CAN_FLAG_FALSE                      = 0,
    CAN_FLAG_TRUE                       = 1,
    CAN_FLAG_MAX                        = 2
} CANFlagValue_t;

typedef enum CANMode
{
    CAN_MODE_NO_INITIALIZATION          = 0,
    CAN_MODE_OPERATION                  = 1,
    CAN_MODE_SLEEP                      = 2,
    CAN_MODE_WAKE_UP                    = 3,
    CAN_MODE_MONITORING                 = 4,
    CAN_MODE_INTERNAL_TEST              = 5,
    CAN_MODE_EXTERNAL_TEST              = 6,
    CAN_MODE_RESET_CONTROLLER           = 7,
    CAN_MODE_MAX                        = 8
} CANMode_t;

typedef enum CANIdType
{
    CAN_ID_TYPE_STANDARD                = 0,
    CAN_ID_TYPE_EXTENDED                = 1,
    CAN_ID_TYPE_MAX                     = 2
} CANIdType_t;

typedef enum CANTxBufferMode
{
    CAN_TX_BUFFER_MODE_FIFO             = 0,
    CAN_TX_BUFFER_MODE_QUEUE            = 1
} CANTxBufferMode_t;

typedef enum CANTxInterruptType
{
    CAN_TX_INT_TYPE_TRANSMIT_COMPLETED          = 0,
    CAN_TX_INT_TYPE_TRANSMIT_CANCEL_FINISHED    = 1,
    CAN_TX_INT_TYPE_NEW_EVENT                   = 2
} CANTxInterruptType_t;

typedef enum CANMessageBufferType
{
    CAN_RX_BUFFER_TYPE_DBUFFER          = 0,
    CAN_RX_BUFFER_TYPE_FIFO0            = 1,
    CAN_RX_BUFFER_TYPE_FIFO1            = 2,
    CAN_TX_BUFFER_TYPE_FIFO             = 3,
    CAN_TX_BUFFER_TYPE_QUEUE            = 4,
    CAN_TX_BUFFER_TYPE_DBUFFER          = 5
} CANMessageBufferType_t;

typedef enum CANBufferDataField
{
    CAN_BUFFER_DATA_FIELD_8             = 0,
    CAN_BUFFER_DATA_FIELD_12            = 1,
    CAN_BUFFER_DATA_FIELD_16            = 2,
    CAN_BUFFER_DATA_FIELD_20            = 3,
    CAN_BUFFER_DATA_FIELD_24            = 4,
    CAN_BUFFER_DATA_FIELD_32            = 5,
    CAN_BUFFER_DATA_FIELD_48            = 6,
    CAN_BUFFER_DATA_FIELD_64            = 7,
    CAN_BUFFER_DATA_FIELD_MAX           = 8
} CANBufferDataField_t;

typedef enum CANRxOperationMode
{
    CAN_RX_OP_BLOCKING                  = 0,
    CAN_RX_OP_OVERWRITE                 = 1,
    CAN_RX_OP_MAX                       = 2
} CANRxOperationMode_t;

typedef enum CANFilterType
{
    CAN_FILTER_TYPE_RANGE               = 0,
    CAN_FILTER_TYPE_DUAL                = 1,
    CAN_FILTER_TYPE_CLASSIC             = 2,
    CAN_FILTER_TYPE_ELEMENTDISABLED     = 3,
    CAN_FILTER_TYPE_MAX                 = 4
} CANFilterType_t;

typedef enum CANFilterConfig
{
    CAN_FILTER_CFG_DISABLE_ELEMENT      = 0,
    CAN_FILTER_CFG_RXFIFO0              = 1,
    CAN_FILTER_CFG_RXFIFO1              = 2,
    CAN_FILTER_CFG_REJECT_ID            = 3,
    CAN_FILTER_CFG_PRY                  = 4,
    CAN_FILTER_CFG_PRY_N_RXFIFO0        = 5,
    CAN_FILTER_CFG_PRY_N_RXFIFO1        = 6,
    CAN_FILTER_CFG_RXBUFFER             = 7,
    CAN_FILTER_CONFIG_MAX               = 8
} CANFilterConfig_t;

typedef enum CANDataLength
{
    CAN_DATA_LENGTH_0                   = 0,
    CAN_DATA_LENGTH_1                   = 1,
    CAN_DATA_LENGTH_2                   = 2,
    CAN_DATA_LENGTH_3                   = 3,
    CAN_DATA_LENGTH_4                   = 4,
    CAN_DATA_LENGTH_5                   = 5,
    CAN_DATA_LENGTH_6                   = 6,
    CAN_DATA_LENGTH_7                   = 7,
    CAN_DATA_LENGTH_8                   = 8,
    CAN_DATA_LENGTH_12                  = 9,
    CAN_DATA_LENGTH_16                  = 10,
    CAN_DATA_LENGTH_20                  = 11,
    CAN_DATA_LENGTH_24                  = 12,
    CAN_DATA_LENGTH_32                  = 13,
    CAN_DATA_LENGTH_48                  = 14,
    CAN_DATA_LENGTH_64                  = 15,
    CAN_DATA_LENGTH_MAX                 = 16
} CANDataLength_t;

typedef enum CANBitRateTiming
{
    CAN_BIT_RATE_TIMING_ARBITRATION     = 0, //Norminal bit Timing
    CAN_BIT_RATE_TIMING_DATA            = 1, //Data bit Timing
    CAN_BIT_RATE_TIMING_MAX             = 2
} CANBitRateTiming_t;

typedef enum CANRingBufferType
{
    CAN_RING_BUFFER_FIRST               = 0,
    CAN_RING_BUFFER_LAST                = 1,
    CAN_RING_BUFFER_MAX                 = 2
} CANRingBufferType_t;

typedef enum CANErrorType
{
    CAN_ERROR_NONE                          = 0,
    CAN_ERROR_NOT_INIT                      = 1,
    CAN_ERROR_BAD_PARAM                     = 2,
    CAN_ERROR_ALLOC                         = 3,
    CAN_ERROR_NO_MESSAGE                    = 4,
    CAN_ERROR_FIFO_QUEUE_FULL               = 5,
    CAN_ERROR_FIFO_QUEUE_LOST               = 6,
    CAN_ERROR_CONTROLLER_MODE               = 7,
    CAN_ERROR_TIMEOUT                       = 8,
    CAN_ERROR_NO_BUFFER                     = 9,

    CAN_ERROR_INT_ACCESS_RESERVED_ADDRESS   = 20,
    CAN_ERROR_INT_PROTOCOL                  = 21,
    CAN_ERROR_INT_BUS_OFF                   = 22,
    CAN_ERROR_INT_WARNING                   = 23,
    CAN_ERROR_INT_PASSIVE                   = 24,
    CAN_ERROR_INT_BIT                       = 25,
    CAN_ERROR_INT_TIMEOUT                   = 26,
    CAN_ERROR_INT_RAM_ACCESS_FAIL           = 27,
    CAN_ERROR_INT_TX_EVENT_FULL             = 28,
    CAN_ERROR_INT_TX_EVENT_LOST             = 29,

    CAN_ERROR_CODE_STUFF                    = 41,
    CAN_ERROR_CODE_FORM                     = 42,
    CAN_ERROR_CODE_ACK                      = 43,
    CAN_ERROR_CODE_BIT1                     = 44,
    CAN_ERROR_CODE_BIT0                     = 45,
    CAN_ERROR_CODE_CRC                      = 46,
    CAN_ERROR_CODE_NO_CHANGE                = 47
} CANErrorType_t;

typedef void (*CANNotifyTxEventCB)
(
    uint8_t                               ucCh,
    CANTxInterruptType_t                uiIntType
);

typedef void (*CANNotifyRxEventCB)
(
    uint8_t                               ucCh,
    uint32_t                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
);

typedef void (*CANNotifyErrorEventCB)
(
    uint8_t                               ucCh,
    CANErrorType_t                      uiError
);

typedef struct CANTimingParam
{
    uint8_t                               tpBRP;
    uint8_t                               tpPROP;
    uint8_t                               tpPhaseSeg1;
    uint8_t                               tpPhaseSeg2;
    uint8_t                               tpSJW;
    uint8_t                               tpTDC;
    uint8_t                               tpTDCOffset;
    uint8_t                               tpTDCFilterWindow;
} CANTimingParam_t;

typedef struct CANTxBuffer
{
    uint8_t                               tbNumberOfDedicatedBuffer;
    uint8_t                               tbNumberOfFIFOorQueue;
    CANTxBufferMode_t                   tbTxFIFOorQueueMode;
    CANBufferDataField_t                tbBufferElementSize;
    uint32_t                              tbInterruptConfig;
} CANTxBuffer_t;

typedef struct CANRxBuffer
{
    uint8_t                               rbNumberOfElement;
    CANBufferDataField_t                rbDataFieldSize;
    CANRxOperationMode_t                rbOperationMode;
} CANRxBuffer_t;

typedef struct CANIdFilterList
{
    CANFilterType_t                     idflFilterType;
    CANFilterConfig_t                   idflFilterElementConfiguration;
    uint32_t                              idflFilterID1;
    uint32_t                              idflFilterID2;
} CANIdFilterList_t;

typedef struct CANIdFilter
{
    CANIdFilterList_t *                 idfList[CAN_CONTROLLER_NUMBER];
} CANIdFilter_t;

typedef struct CANMessage
{
    CANMessageBufferType_t              mBufferType;
    uint8_t                               mBufferIndex;
    uint8_t                               mErrorStateIndicator;
    uint8_t                               mExtendedId;
    uint8_t                               mRemoteTransmitRequest;
    uint32_t                              mId;
    uint8_t                               mFDFormat;
    uint8_t                               mBitRateSwitching;
    uint8_t                               mMessageMarker;
    uint8_t                               mEventFIFOControl;
    uint8_t                               mDataLength; //byte unit
    uint8_t                               mData[CAN_DATA_LENGTH_SIZE];
} CANMessage_t;

typedef struct CANTxEvent
{
    uint8_t                               teMessageMarker;
    uint8_t                               teFDFormat;
    uint8_t                               teBitRateSwitching;
    uint8_t                               teDataLength; //byte unit
    uint8_t                               teExtendedId;
    uint8_t                               teRemoteTransmitRequest;
    uint32_t                              teId;
} CANTxEvent_t;

typedef struct CANCallBackFunc
{
    CANNotifyTxEventCB                  cbNotifyTxEvent;
    CANNotifyRxEventCB                  cbNotifyRxEvent;
    CANNotifyErrorEventCB               cbNotifyErrorEvent;
} CANCallBackFunc_t;

/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

CANErrorType_t CAN_Init
(
    void
);

CANErrorType_t CAN_Deinit
(
    void
);

CANErrorType_t CAN_InitMessage
(
    uint8_t                               ucCh
);

uint32_t CAN_CheckNewRxMessage
(
    uint8_t                               ucCh
);

CANErrorType_t CAN_GetNewRxMessage
(
    uint8_t                               ucCh,
    CANMessage_t *                      psMsg
);

CANErrorType_t CAN_SendMessage
(
    uint8_t                               ucCh,
    const CANMessage_t *                psMsg,
    uint8_t *                             pucTxBufferIndex
);

CANErrorType_t CAN_RequestTxMessageCancellation
(
    uint8_t                               ucCh,
    uint8_t                               ucTxIndex
);

uint32_t CAN_CheckNewTxEvent
(
    uint8_t                               ucCh
);

CANErrorType_t CAN_GetNewTxEvent
(
    uint8_t                               ucCh,
    CANTxEvent_t *                      psTxEvent
);

CANErrorType_t CAN_SetControllerMode
(
    uint8_t                               ucCh,
    CANMode_t                           ucControllerMode
);

CANMode_t CAN_GetControllerMode
(
    uint8_t                               ucCh
);

uint32_t CAN_GetProtocolStatus
(
    uint8_t                               ucCh
);

void *CAN_GetControllerInfo
(
    uint8_t                               ucCh
);

CANErrorType_t CAN_RegisterCallbackFunctionTx
(
    CANNotifyTxEventCB                  pCbFnTx
);

CANErrorType_t CAN_RegisterCallbackFunctionRx
(
    CANNotifyRxEventCB                  pCbFnRx
);

CANErrorType_t CAN_RegisterCallbackFunctionError
(
    CANNotifyErrorEventCB               pCbFnError
);

#endif /*__BSP_CAN_HEADER */

