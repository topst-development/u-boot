// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef __CAN_TEST_H
#define __CAN_TEST_H

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

#define APP_TASK_CAN_STK_SIZE                   2048
#define CAN_TEST_TASK_PRIORITY  (8u)

#define CAN_OK      (0)
#define CAN_FAIL    (-1)

#define PROTOCOL_ERR_NO     0x0
#define PROTOCOL_ERR_STUFF  0x1
#define PROTOCOL_ERR_FORMAT 0x2
#define PROTOCOL_ERR_ACK    0x3
#define PROTOCOL_ERR_BIT1   0x4
#define PROTOCOL_ERR_BIT0   0x5
#define PROTOCOL_ERR_CRC    0x6

#define LEC_MASK 0x7
#define DLEC_MASK 0x700

#define TCC_PMU_BASE_MICOM          0x1B936000
#define PMU_GPIOMA_SEL (TCC_PMU_BASE_MICOM + 0x160)

#define CANTestMessageNumber 10
#define CANMessageNumberForTest 1 // CANMessageNumberForTest <= CANTestMessageNumber

typedef struct {
    uint32_t RxIndex;
    uint32_t RxBufferType;
    int32_t ErrorType;
    uint32_t NewMsgFlag;
} CANNewMessage;

typedef struct {
    int32_t TxIndex;
    uint32_t TxID;
    uint32_t TxInterruptType;
    uint32_t ErrorType;
    uint32_t RequestSendMessageFlag;
    uint32_t RequestSendMessageCancelFlag;
    uint32_t TimeoutCount;
} CANSendMessage;

/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

int CANTestMain(void);
void CAN_MM_TxEvent(uint32_t Channel, uint32_t InterruptType);
void CAN_MM_RxEvent(uint32_t Channel, uint32_t RxIndex, uint32_t BufferType, int32_t ErrorType);
void CAN_MM_ErrorEvent(uint32_t Channel, int32_t ErrorType);
void CAN_MM_CheckSendMessage(void);
void CAN_MM_CheckTxEvent(void);
void CAN_MM_CheckReceivedMessage(void);
void CAN_MM_CheckStatus(void);
void CanTestTask(void *p_arg);
void create_can_test_task(void);
void init_can_test( void );

#endif  // __CAN_TEST_H
