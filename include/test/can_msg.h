// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef __BSP_CAN_MSG_HEADER
#define __BSP_CAN_MSG_HEADER

/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

typedef struct CANRxRingBuffer
{
    uint32_t                              rrbInitFlag;
    uint32_t                              rrbMsgBufferMax;
    uint32_t                              rrbHeadIdx;
    uint32_t                              rrbTailIdx;
    CANMessage_t                        rrbMsg[ CAN_RX_MSG_RING_BUFFER_MAX ];
} CANRxRingBuffer_t;


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

CANErrorType_t CAN_MsgInit
(
    const CANController_t *             psControllerInfo
);

CANErrorType_t CAN_MsgPutRxMessage
(
    uint8_t                               ucCh,
    CANMessageBufferType_t              uiBufferType
);

uint32_t CAN_MsgGetCountOfRxMessage
(
    uint8_t                               ucCh
);

CANErrorType_t CAN_MsgGetRxMessage
(
    uint8_t                               ucCh,
    CANMessage_t *                      psRxMsg
);

CANErrorType_t CAN_MsgSetTxMessage
(
    CANController_t *                   psControllerInfo,
    const CANMessage_t *                psMsg,
    uint8_t *                             pucTxBufferIndex
);

/* Not used function */
CANErrorType_t CAN_MsgRequestTxAllMessage
(
    CANController_t *                   psControllerInfo,
    uint32_t                              uiTxBufferAllIndex
);

CANErrorType_t CAN_MsgRequestTxMessageCancellation
(
    CANController_t *                   psControllerInfo,
    uint8_t                               ucTxBufferIndex
);

uint32_t CAN_MsgGetCountOfTxEvent
(
    uint8_t                               ucCh
);

CANErrorType_t CAN_MsgGetTxEventMessage
(
    uint8_t                               ucCh,
    CANTxEvent_t *                      psTxEvtBuffer
);

#endif /* __BSP_CAN_MSG_HEADER__ */
