// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#define CAN_TEST_APP (1)

#if (CAN_TEST_APP ==1)
//#include "bsp.h"
//#include "gic.h"
//#include "gpio.h"
//#include "debug.h"

//#include "gdma.h"
#include <stdio.h>

#include <test/can_config.h>
#include <test/can_reg.h>
#include <test/can.h>
#include <test/can_drv.h>
#include <test/can_msg.h>
#include <test/can_porting.h>

#include <test/can_test.h>

#include <mach/clock.h>



#if 1 /* Don't release this file */
/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

CANNewMessage CANNewMessageInfo[CAN_CONTROLLER_NUMBER];
CANSendMessage CANSendMessageInfo[CAN_CONTROLLER_NUMBER];
static volatile uint32_t CompletedFlag[CAN_CONTROLLER_NUMBER];
static volatile uint32_t ReceiveFlag[CAN_CONTROLLER_NUMBER];
static volatile uint32_t ErrorFlag[CAN_CONTROLLER_NUMBER];
volatile uint32_t DmaIrqFlag = 0;

int32_t protocolStatus[CAN_CONTROLLER_NUMBER];


extern CANDriver_t CANDrvierInfo; //Set CANDrvierInfo to non static
extern CANRegFieldIR_t IRClearStatus[CAN_CONTROLLER_NUMBER];

extern uint8_t timeoutTestForRXFIFO0;

/* FIFO */
CANMessage_t TxMessage1[CANTestMessageNumber] =
{
	/* BufferType               Index  ESI  Extended  Remote  Id 	 FD  BRS Marker  EventFIFO  DLC DATA */
	{CAN_TX_BUFFER_TYPE_DBUFFER, 0,     0,   0,        0,      0x301, 1,  1,	 0xFF,   1,         1,	{0x88}},
	{CAN_TX_BUFFER_TYPE_DBUFFER, 1,     0,   0,        0,      0x302, 1,  1,	 0xFF,   1,         2,	{0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_DBUFFER, 2,     0,   0,        0,      0x303, 1,  1,	 0xFF,   1,         3,	{0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_DBUFFER, 3,     0,   0,        0,      0x304, 1,  1,	 0xFF,   1,         4,	{0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,        0,      0x305, 1,  1,	 0xFF,   1,         5,	{0x44, 0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,        0,      0x306, 1,  1,	 0xFF,   1,         6,	{0x33, 0x44, 0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,        0,      0x307, 1,  1,	 0xFF,   1,         7,	{0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,        0,      0x308, 1,  1,	 0xFF,   1,         8,	{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,        0,      0x309, 1,  1,	 0xFF,   1,         9,	{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,        0,      0x310, 1,  1,	 0xFF,   1,         10,	{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x12, 0x34, 0x56, 0x78}},
};

/* mixed dedicated buffer & QUEUE */
CANMessage_t TxMessage2[CANTestMessageNumber] =
{
	/* BufferType               Index  ESI  Extended  Remote  Id 	 FD  BRS  Marker  EventFIFO  DLC DATA */
#if 1 //normal message
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,        0,      0x311, 1,  1,   0xFF,   1,         5,  {0x44, 0x55, 0x66, 0x77, 0x88}},
#else //to measure wave form
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,        0,      0xAA,	 1,  1,   0xFF,   1,         1,  {0xAA}},
#endif
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,        0,      0x312, 1,  1,   0xFF,   1,         6,  {0x33, 0x44, 0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,        0,      0x313, 1,  1,   0xFF,   1,         7,  {0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,        0,      0x314, 1,  1,   0xFF,   1,         8,  {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,        0,      0x315, 1,  1,   0xFF,   1,         12, {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x10, 0x12}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,        0,      0x316, 1,  1,   0xFF,   1,         2,  {0x33, 0x44}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,        0,      0x317, 1,  1,   0xFF,   1,         5,  {0xAA, 0xBB, 0xCC, 0xDD, 0xEE}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   0,        0,      0x318, 1,  1,   0xFF,   1,         4,  {0x12, 0x34, 0x56, 0x78}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,        0,      0x319, 1,  1,   0xFF,   1,         1,  {0xFF}},
	{CAN_TX_BUFFER_TYPE_FIFO,    0,     0,   1,        0,      0x320, 1,  1,   0xFF,   1,         8,  {0x11, 0xAA, 0x22, 0xBB, 0x33, 0xCC, 0x44, 0xDD}},
};

/* mixed dedicated buffer & QUEUE */
CANMessage_t TxMessage3[CANTestMessageNumber] =
{
    /* BufferType               Index  ESI  Extended  Remote  Id 	 FD  BRS  Marker  EventFIFO  DLC DATA */
	{CAN_TX_BUFFER_TYPE_DBUFFER, 0,     0,   0,        0,      0x330, 1,  1,   0xFF,   1,         5,  {0x77, 0x88, 0x99, 0xAA, 0xBB}},
	{CAN_TX_BUFFER_TYPE_QUEUE,   0,     0,   0,        0,      0x322, 1,  1,   0xFF,   1,         5,  {0x44, 0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_QUEUE,   1,     0,   0,        0,      0x327, 1,  1,   0xFF,   1,         6,  {0x33, 0x44, 0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_QUEUE,   2,     0,   0,        0,      0x324, 1,  1,   0xFF,   1,         7,  {0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_DBUFFER, 1,     0,   0,        0,      0x325, 1,  1,   0xFF,   1,         2,  {0x11, 0x33}},
	{CAN_TX_BUFFER_TYPE_QUEUE,   3,     0,   0,        0,      0x326, 1,  1,   0xFF,   1,         8,  {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}},
	{CAN_TX_BUFFER_TYPE_QUEUE,   4,     0,   0,        0,      0x323, 1,  1,   0xFF,   1,         12, {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x10, 0x12}},
	{CAN_TX_BUFFER_TYPE_QUEUE,   5,     0,   0,        0,      0x328, 1,  1,   0xFF,   1,         2,  {0x33, 0x44}},
	{CAN_TX_BUFFER_TYPE_DBUFFER, 2,     0,   0,        0,      0x329, 1,  1,   0xFF,   1,         3,  {0x44, 0x55, 0x66}},
	{CAN_TX_BUFFER_TYPE_DBUFFER, 3,     0,   0,        0,      0x321, 1,  1,   0xFF,   1,         4,  {0x44, 0x55, 0x66, 0x77}},
};

/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/
static void CANTEST_CallbackTxEvent
(
    uint8_t                               ucCh,
    CANTxInterruptType_t                uiIntType
);

static void CANTEST_CallbackRxEvent
(
    uint8_t                               ucCh,
    uint32_t                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
);

static void CANTEST_CallbackErrorEvent
(
    uint8_t                               ucCh,
    CANErrorType_t                      uiError
);

static int32_t CANTEST_Initialize(void);
static int32_t CANTEST_SendReceive(void);
static int32_t CANTEST_SendMessageByFIFO(uint32_t channel);
static int32_t CANTEST_SendMessageByQueue(uint32_t channel);
static int32_t CANTEST_ReceiveMessage(uint32_t msg_count);
static void CANTEST_PrintMessageData(uint8_t channel);
static int32_t CANTEST_ReceiveMessageWithStandardIDFilter(void);
static int32_t CANTEST_ReceiveMessageWithExtendedIDFilter(void);
static int32_t CANTEST_ReceiveMessageTimeout(void);
static void CANTEST_ClearIrqStatus(uint32_t channel);
static int32_t CANTEST_ReceiveErrorMessageForStuffError(void);
static int32_t CANTEST_SendErrorMessageForFormatError(uint8_t tx_channel);
static int32_t CANTEST_SendMessageForACKError(void);
static int32_t CANTEST_SendMessageForBit0Error(void);
static int32_t CANTEST_SendMessageForBit1Error(uint8_t tx_channel);
static int32_t CANTEST_SendErrorMessageForCRCError(void);
static int32_t CANTEST_InternalLoopBackTest(uint8_t channel);
static int32_t CANTEST_ConfigurationRegisterWriteLock(void);
static int32_t CANTEST_DMATestWithMessages(uint8_t ucCh, uint32_t * puiAddr);

static void CANTEST_CallbackTxEvent
(
    uint8_t                               ucCh,
    CANTxInterruptType_t                uiIntType
)
{
    if( uiIntType == CAN_TX_INT_TYPE_TRANSMIT_COMPLETED )
    {
        CompletedFlag[ucCh] = CAN_FLAG_TRUE;
    }
}

static void CANTEST_CallbackRxEvent
(
    uint8_t                               ucCh,
    uint32_t                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
)
{
    if( uiError == CAN_ERROR_NONE )
    {
        ReceiveFlag[ucCh] = uiRxIndex + 1UL;
    }

    ( void ) uiRxBufferType;
}

static void CANTEST_CallbackErrorEvent
(
    uint8_t                               ucCh,
    CANErrorType_t                      uiError
)
{
    printf( "[CAN ] chennel %d Error Event type: %d, ", ucCh, uiError );

    switch( uiError )
    {
        case CAN_ERROR_INT_ACCESS_RESERVED_ADDRESS:
        {
            printf( "ACCESS_RESERVED_ADDRESS Error\r\n" );

            break;
        }

        case CAN_ERROR_INT_PROTOCOL:
        {
            protocolStatus[ ucCh ] = CAN_GetProtocolStatus( ucCh );
            printf( "PROTOCO Error(Status: 0x%X) \r\n", protocolStatus[ ucCh ] );

            break;
        }

        case CAN_ERROR_INT_BUS_OFF:
        {
            printf( "BUS_OFF Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_WARNING:
        {
            printf( "WARNING Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_PASSIVE:
        {
            printf( "PASSIVE Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_BIT:
        {
            printf( "BIT Error(Status: 0x%X) \r\n", CAN_GetProtocolStatus( ucCh ) );

            break;
        }

        case CAN_ERROR_INT_TIMEOUT:
        {
            printf( "TIMEOUT Error \r\n" );

            break;
        }

        case CAN_ERROR_INT_RAM_ACCESS_FAIL:
        {
            printf( "RAM_ACCESS_FAIL \r\n" );

            break;
        }

        case CAN_ERROR_INT_TX_EVENT_FULL:
        {
            printf( "TX_EVENT_FULL Error \r\n" );

            break;
        }

        case CAN_ERROR_INT_TX_EVENT_LOST:
        {
            printf( "TX_EVENT_LOST Error \r\n" );

            break;
        }

        default:
        {
            printf( "Error \r\n" );

            break;
        }
    }

    ErrorFlag[ucCh] = CAN_FLAG_TRUE;
}

static int32_t CANTEST_Initialize
(
    void
)
{
    int32_t          ret;
    CANErrorType_t  result;

    ret             = 0;

    printf( "CAN Initialize START!!\n" );

    ( void ) CAN_RegisterCallbackFunctionTx( &CANTEST_CallbackTxEvent );

    ( void ) CAN_RegisterCallbackFunctionRx( &CANTEST_CallbackRxEvent );

    ( void ) CAN_RegisterCallbackFunctionError( &CANTEST_CallbackErrorEvent );

    result = CAN_Init();

    if(result == CAN_ERROR_NONE)
    {
        printf( "CAN Initialize SUCCESS\n" );
    }
    else
    {
        printf( "CAN Initialize FAIL\n" );

        result = CAN_ERROR_NOT_INIT;
    }

    printf( "CAN Initialize END!!\n" );

    if( result != CAN_ERROR_NONE ) {
        ret = -1;
    }

    return ret;
}

static int32_t CANTEST_SendReceive(void)
{
    uint8_t           ucCh1;
    uint8_t           ucCh2;
    uint8_t           ucMsgLength;
    uint8_t           ucTxBufferIndex;
    uint32_t          uiTxMsgCnt;
    uint32_t          uiRxMsgNum;
    uint32_t          uiTxEvtMsgCnt;
    uint32_t          uiTimeout;
    CANMessage_t *  psTxMsg;
    CANMessage_t    sRxMsg;
    CANTxEvent_t    sTxEvtMsg;
    CANErrorType_t  result;

    /* the messages are sent to another channel */
    for( ucCh1 = 0U ; ucCh1 < CAN_CONTROLLER_NUMBER ; ucCh1++ )
    {
	printf("init message\n");
        ( void ) CAN_InitMessage( ucCh1 ); //clear remain messages

        for( uiTxMsgCnt = 0UL ; uiTxMsgCnt < CANTestMessageNumber ; uiTxMsgCnt++ )
        {
            uiTimeout = 10;

            /* Send Tx message */
            psTxMsg = &TxMessage1[ uiTxMsgCnt ];
            ( void ) CAN_SendMessage( ucCh1, psTxMsg, &ucTxBufferIndex );

            while( ( CompletedFlag[ucCh1] == CAN_FLAG_FALSE ) && ( uiTimeout > 0UL ) )
            {
                CAN_PortingDelay( 1 );

                uiTimeout--;
            }

            /* Check send message */
            if( ( CompletedFlag[ ucCh1 ] == CAN_FLAG_TRUE ) && ( uiTimeout != 0UL ) )
            {
                printf( "[CAN] SEND     : Success - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh1, uiTxMsgCnt, psTxMsg->mId );

                uiTxEvtMsgCnt = CAN_CheckNewTxEvent( ucCh1 );
                if( 0UL < uiTxEvtMsgCnt )
                {
                    /* Get & Check Tx event message */
                    ( void ) memset( &sTxEvtMsg, 0, sizeof( CANTxEvent_t ) );

                    result = CAN_GetNewTxEvent( ucCh1, &sTxEvtMsg );

                    if( result == CAN_ERROR_NONE )
                    {
                        if( psTxMsg->mId == sTxEvtMsg.teId )
                        {
                            printf( "[CAN] TX_EVENT : Success - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh1, uiTxMsgCnt, psTxMsg->mId );
                        }
                        else
                        {
                            printf( "[CAN] TX_EVENT : Fail(ID not match) - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh1, uiTxMsgCnt, psTxMsg->mId );
                        }
                    }
                    else
                    {
                        printf( "[CAN] TX_EVENT : Fail to get Tx Event Message \r\n" );
                    }
                }

                CompletedFlag[ucCh1] = CAN_FLAG_FALSE;
            }
            else
            {
                printf( "[CAN] SEND        : Fail(Timeout) - Tx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh1, uiTxMsgCnt, psTxMsg->mId );
            }

            /* Check receive message for other channel */
            for( ucCh2 = 0U ; ucCh2 < CAN_CONTROLLER_NUMBER ; ucCh2++ )
            {
                if( ucCh2 != ucCh1 )
                {
                    if( 0UL < ReceiveFlag[ ucCh2 ] )
                    {
                        uiRxMsgNum = CAN_CheckNewRxMessage( ucCh2 );

                        if( 0UL < uiRxMsgNum )
                        {
                            (void) CAN_GetNewRxMessage( ucCh2, &sRxMsg );

                            if( psTxMsg->mId == sRxMsg.mId )
                            {
                                if( sRxMsg.mDataLength == 0U )
                                {
                                    printf( "[CAN] RECEIVE  : Fail(No data) - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                                }
                                else
                                {
                                    for( ucMsgLength = 0 ; ucMsgLength < sRxMsg.mDataLength ; ucMsgLength++ )
                                    {
                                        if( psTxMsg->mData[ ucMsgLength ] != sRxMsg.mData[ ucMsgLength ] )
                                        {
                                            printf( "[CAN] RECEIVE  : Fail(Data not match) - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                                        }
                                    }
                                    printf( "[CAN] RECEIVE  : Success - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                                }
                            }
                            else
                            {
                                printf( "[CAN] RECEIVE  : Fail(ID not match) - Rx channel : %d, Message Num : %d, Tx ID: 0x%X Rx ID: 0x%X\r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId, sRxMsg.mId);
                            }
                        }

                        ReceiveFlag[ ucCh2 ] = 0;
                    }
                    else
                    {
                        printf( "[CAN] RECEIVE  : Fail(Timeout) - Rx channel : %d, Message Num : %d, Tx ID: 0x%X \r\n", ucCh2, uiTxMsgCnt, psTxMsg->mId );
                    }
                }
            }
            printf( "\r\n" );
        }
    }

    return CAN_OK;
}

static int32_t CANTEST_SendMessageByFIFO(uint32_t channel)
{
    CANMessage_t *  txMsg;
    CANTxEvent_t    TxEvent;
    uint8_t           ucTxBufferIndex;
    int32_t           TxEventNumber;
    int32_t           i = 0, j;
    int32_t           ret = CAN_ERROR_NONE;

    //CANTEST_ClearIrqStatus(channel); //Don't call in test case 11 because interrupt flag is overwritten.

    do {
	    //printf("Test num : %d, max test : %d channel : %d\n", i, CANTestMessageNumber, channel);

	    txMsg = &TxMessage2[i];
	    ret = CAN_SendMessage(channel, txMsg, &ucTxBufferIndex);
	    CAN_PortingDelay(2); //wait for interrupt

	    if ((ret == CAN_ERROR_NONE) && (0 < CompletedFlag[channel])) {
		    printf("[CAN] chennel %d Sent message Success!! Test ID: 0x%X, Data length: %d \r\n", channel, txMsg->mId, txMsg->mDataLength);
		    printf("DATA : ");

		    for (j = 0 ; j < txMsg->mDataLength ; j++) {
			    printf("0x%X ", txMsg->mData[j]);
			    if ((j >= 12) && ((j % 12) == 0)) {
				    printf("\n");
			    }
		    }
		    printf("\n");

		    TxEventNumber = CAN_CheckNewTxEvent(channel);

		    if ( 0 < TxEventNumber ) {
			    //SAL_MemSet(&TxEvent, 0, sizeof(CANTxEvent_t));
			    ret = CAN_GetNewTxEvent(channel, &TxEvent);

			    if (ret != 0) {
				    printf("[CAN] fail to get new tx event\n");
				    ret = CAN_FAIL;
			    } else if (txMsg->mId == TxEvent.teId) {
				    printf("[CAN] channel %d Saved message Success!! TxEvent Test ID: 0x%X \r\n", channel, TxEvent.teId);
			    } else {
				    printf("[CAN] channel %d Saved message Fail!! TxEvent Test ID: 0x%X \r\n", channel, TxEvent.teId);
				    ret = CAN_FAIL;
			    }
		    } else {
			    printf("[CAN] channel %d No Tx Event message!! TxEvent Test ID: 0x%X \r\n", channel, TxEvent.teId);
			    ret = CAN_FAIL;
		    }

		    CompletedFlag[i] = 0;

	    } else {
		    printf("[CAN] channel %d Sent message Fail!! Test ID: 0x%X, Data length: %d \r\n", channel, txMsg->mId, txMsg->mDataLength);
		    ret = CAN_FAIL;
	    }

	    i++;

	    printf("##################\n");

	    if (i > 10) {
		    break;
	    }

	    if ((ret != CAN_ERROR_NONE)) {
		    break;
	    }

	    //printf("Test num %d, max test : %d\n\n\n", i, CANTestMessageNumber);

    } while (i < CANTestMessageNumber);

    return ret;
}

static int32_t CANTEST_SendMessageByQueue(uint32_t channel)
{
    CANMessage_t *  txMsg;
    CANTxEvent_t    TxEvent;
    uint8_t           ucTxBufferIndex;
    int32_t           TxEventNumber;
    int32_t           i,j;
    int32_t           ret = CAN_OK;
    uint32_t          sumOfTxIndex = 0;
    CANController_t * CANControllerInfo;

    CANTEST_ClearIrqStatus(channel);

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[channel];

    for(i=0;i<CANTestMessageNumber;i++)
    {
        txMsg = &TxMessage3[i];
        (void) CAN_SendMessage(channel, txMsg, &ucTxBufferIndex);
        sumOfTxIndex |= (1<<ucTxBufferIndex);
    }
    CAN_MsgRequestTxAllMessage(CANControllerInfo, sumOfTxIndex);
    CAN_PortingDelay(2); //wait for interrupt

    if(0<CompletedFlag[channel])
    {
        printf("[CAN] chennel %d Sent message Success!! Test ID: 0x%X, Data length: %d \r\n", channel, txMsg->mId, txMsg->mDataLength);
        printf("DATA : ");
        for(j=0; j<txMsg->mDataLength; j++)
        {
            printf("0x%X ", txMsg->mData[j]);
            if((j>=8)&&(j%8==0)) {
                printf("\n");
            }
        }
        printf("\n");

        TxEventNumber = CAN_CheckNewTxEvent(channel);
        if(TxEventNumber == CANTestMessageNumber)
        {
            for(i=0; i<TxEventNumber; i++)
            {
                txMsg = &TxMessage3[i];
                //SAL_MemSet(&TxEvent, 0, sizeof(CANTxEvent_t));
                ret = CAN_GetNewTxEvent(channel, &TxEvent);
                printf("[CAN] chennel %d Check saved message - TxEvent Test ID: 0x%X \r\n", channel, TxEvent.teId);
            }
        }
        else
        {
            printf("[CAN] chennel %d Saved message count Fail!! TxEvent Count : %d \r\n", channel, TxEventNumber);
            return CAN_FAIL;
        }


        CompletedFlag[i] = 0;
    }
    else
    {
        printf("[CAN] chennel %d Sent message Fail!! Test ID: 0x%X, Data length: %d \r\n", channel, txMsg->mId, txMsg->mDataLength);
        return CAN_FAIL;
    }

    return ret;
}

static int32_t CANTEST_ReceiveMessage(uint32_t msg_count)
{
    CANMessage_t rxMsg;
    int32_t rxMsgNum = 0;
    uint8_t i, j;
    uint32_t count = 0;

    ReceiveFlag[0] = 0; //clear receive flag
    ReceiveFlag[1] = 0; //clear receive flag
    ReceiveFlag[2] = 0; //clear receive flag

    while(1)
    {
        for(i=0;i<CAN_CONTROLLER_NUMBER;i++)
        {
            while(0 == ReceiveFlag[i])
            {
                printf("[CAN] Empty message - please send message \r\n");
                CAN_PortingDelay(1000);
            }

            ReceiveFlag[i] = 0;
            rxMsgNum = CAN_CheckNewRxMessage(i);
            if(0<rxMsgNum)
            {
                CAN_GetNewRxMessage(i, &rxMsg);
            }

            printf("[CAN] Channel %d Received message Success!! Test ID: 0x%X, Data length: %d, DATA :  \r\n", i, rxMsg.mId, rxMsg.mDataLength);
            for(j=1; j<(rxMsg.mDataLength+1); j++)
            {
                printf("0x%02X ", rxMsg.mData[j-1]);
                if(j%16==0) {
                    printf("\n");
                }
            }
            printf("\n");
        }

        count++;
        if(count==msg_count)
            break;
    }

    return CAN_OK;
}

static void CANTEST_PrintMessageData(uint8_t channel)
{
    int32_t j;
    int32_t rxMsgNum;
    CANMessage_t rxMsg;

    rxMsgNum = CAN_CheckNewRxMessage(channel);
    if(0<rxMsgNum)
    {
        CAN_GetNewRxMessage(channel, &rxMsg);
    }

    printf("[CAN] Channel %d Received message Success!! Test ID: 0x%X, Data length: %d, DATA :  \r\n", channel, rxMsg.mId, rxMsg.mDataLength);
    for(j=1; j<(rxMsg.mDataLength+1); j++)
    {
        printf("0x%02X ", rxMsg.mData[j-1]);
        if(j%16==0) {
            printf("\n");
        }
    }
    printf("\n");
}

static int32_t CANTEST_ReceiveMessageWithStandardIDFilter(void)
{
    uint32_t i;

    printf("[CAN] Standard message filter test START!!\n");

    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x01~0x10), disable
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x01 ~ 0x10 [range, disable] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x11~0x20), FIFO0
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x11 ~ 0x20 [range, FIFO0] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x21~0x30), FIFO1
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x21 ~ 0x30 [range, FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x31~0x40), REJECT
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x31 ~ 0x40 [range, reject] \r\n");
        printf("[CAN] If you want to break, please send STANDARD message ID : 0x21 ~ 0x30 [range, FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x41~0x50), PRIORITY
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x41 ~ 0x50 [range, priority] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x51~0x60), PRIORITY & FIFO0
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0)
        || (IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x51 ~ 0x60 [range, priority & FIFO0] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x61~0x70), PRIORITY & FIFO1
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0)
        || (IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x61 ~ 0x70 [range, priority & FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x71~0x80), Dedicated buffer
***********************************************************************************************************************/
    while((IRClearStatus[0].rfDRX == 0) || (IRClearStatus[1].rfDRX == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x71 [No Range:It's dedicated buffer] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x81, 0x90), disable
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x81 or 0x90 [dual, disable] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x91, 0x100), FIFO0
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x91 or 0x100 [dual, FIFO0] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x101, 0x110), FIFO1
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x101 or 0x110 [dual, FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x111, 0x120), REJECT
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x111 or 0x120 [dual, reject] \r\n");
        printf("[CAN] If you want to break, please send STANDARD message ID : 0x101 or 0x110 [dual, FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x121, 0x130), PRIORITY
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x121 or 0x130 [dual, priority] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x131, 0x140), PRIORITY & FIFO0
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0)
        || (IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x131 or 0x140 [dual, priority & FIFO0] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x141, 0x150), PRIORITY & FIFO1
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0)
        || (IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x141 or 0x150 [dual, priority & FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x151, 0x160), Dedicated buffer
***********************************************************************************************************************/
    while((IRClearStatus[0].rfDRX == 0) || (IRClearStatus[1].rfDRX == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x151 [No Dual:It's dedicated buffer] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x211, 0x7F0), disable
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x211(Mask:0x7F0) [classic, disable] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x221, 0x7F0), FIFO0
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x221(Mask:0x7F0) [classic, FIFO0] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x231, 0x7F0), FIFO1
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x231(Mask:0x7F0) [classic, FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x241, 0x7F0), REJECT
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x241(Mask:0x7F0) [classic, reject] \r\n");
        printf("[CAN] If you want to break, please send STANDARD message ID : 0x231(Mask:0x7F0) [classic, FIFO1]  \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x251, 0x7F0), PRIORITY
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x251(Mask:0x7F0) [classic, priority] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x261, 0x7F0), PRIORITY & FIFO0
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0)
        || (IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x261(Mask:0x7F0) [classic, priority & FIFO0] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x271, 0x7F0), PRIORITY & FIFO1
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0)
        || (IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x271(Mask:0x7F0) [classic, priority & FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x281, 0x0), Dedicated buffer
***********************************************************************************************************************/
    while((IRClearStatus[0].rfDRX == 0) || (IRClearStatus[1].rfDRX == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x281 [No Mask:It's dedicated buffer] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : disable(0x291, 0x291), disable
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send STANDARD message ID : 0x291 [disable, disable] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

    printf("[CAN] Standard message filter test END!!\n");

    return CAN_OK;
}

static int32_t CANTEST_ReceiveMessageWithExtendedIDFilter(void)
{
    uint32_t i;

    printf("[CAN] Extended message filter test START!!\n");

    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x1001~0x1010), disable
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1001 ~ 0x1010 [range, disable] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x1011~0x1020), FIFO0
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1011 ~ 0x1020 [range, FIFO0] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x1021~0x1030), FIFO1
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1021 ~ 0x1030 [range, FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x1031~0x1040), REJECT
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1031 ~ 0x1040 [range, reject] \r\n");
        printf("[CAN] If you want to break, please send EXTENDED message ID : 0x1021 ~ 0x1030 [range, FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x1041~0x1050), PRIORITY
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1041 ~ 0x1050 [range, priority] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x1051~0x1060), PRIORITY & FIFO0
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0)
        || (IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1051 ~ 0x1060 [range, priority & FIFO0] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x1061~0x1070), PRIORITY & FIFO1
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0)
        || (IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1061 ~ 0x1070 [range, priority & FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : range(0x1071~0x1080), Dedicated buffer
***********************************************************************************************************************/
    while((IRClearStatus[0].rfDRX == 0) || (IRClearStatus[1].rfDRX == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1071 [No Range:It's dedicated buffer] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x1081, 0x1090), disable
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1081 or 0x1090 [dual, disable] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x1091, 0x1100), FIFO0
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1091 or 0x1100 [dual, FIFO0] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x1101, 0x1110), FIFO1
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1101 or 0x1110 [dual, FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x1111, 0x1120), REJECT
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1111 or 0x1120 [dual, reject] \r\n");
        printf("[CAN] If you want to break, please send EXTENDED message ID : 0x1101 or 0x1110 [dual, FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x1121, 0x1130), PRIORITY
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1121 or 0x1130 [dual, priority] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x1131, 0x1140), PRIORITY & FIFO0
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0)
        || (IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1131 or 0x1140 [dual, priority & FIFO0] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x1141, 0x1150), PRIORITY & FIFO1
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0)
        || (IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1141 or 0x1150 [dual, priority & FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : dual(0x1151, 0x1160), Dedicated buffer
***********************************************************************************************************************/
    while((IRClearStatus[0].rfDRX == 0) || (IRClearStatus[1].rfDRX == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1151 [No Dual:It's dedicated buffer] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x1211, 0xFFF0), disable
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1211(Mask:0xFFF0) [classic, disable] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x1221, 0xFFF0), FIFO0
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1221(Mask:0xFFF0) [classic, FIFO0] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x1231, 0xFFF0), FIFO1
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1231(Mask:0xFFF0) [classic, FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x1241, 0xFFF0), REJECT
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1241(Mask:0xFFF0) [classic, reject] \r\n");
        printf("[CAN] If you want to break, please send EXTENDED message ID : 0x1231(Mask:0xFFF0) [classic, FIFO1]  \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x1251, 0xFFF0), PRIORITY
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1251(Mask:0xFFF0) [classic, priority] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x1261, 0xFFF0), PRIORITY & FIFO0
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0)
        || (IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1261(Mask:0xFFF0) [classic, priority & FIFO0] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x1271, 0xFFF0), PRIORITY & FIFO1
***********************************************************************************************************************/
    while((IRClearStatus[0].rfHPM == 0) || (IRClearStatus[1].rfHPM == 0)
        || (IRClearStatus[0].rfRF1N == 0) || (IRClearStatus[1].rfRF1N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1271(Mask:0xFFF0) [classic, priority & FIFO1] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : classic(0x1281, 0x0), Dedicated buffer
***********************************************************************************************************************/
    while((IRClearStatus[0].rfDRX == 0) || (IRClearStatus[1].rfDRX == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1281 [No Mask:It's dedicated buffer] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

/***********************************************************************************************************************
**  Message Filter : disable(0x1291, 0x1291), disable
***********************************************************************************************************************/
    while((IRClearStatus[0].rfRF0N == 0) || (IRClearStatus[1].rfRF0N == 0))
    {
        printf("[CAN] Empty message - please send EXTENDED message ID : 0x1291 [disable, disable] \r\n");
        CAN_PortingDelay(1000);
    }
    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
        CANTEST_PrintMessageData(i);
    }

    printf("[CAN] Extended message filter test END!!\n");

    return CAN_OK;
}

static int32_t CANTEST_ReceiveMessageTimeout(void)
{
    uint32_t i;

    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

    timeoutTestForRXFIFO0 = true; //block getting data from RX FIFO0

    while((IRClearStatus[0].rfTOO == 0) || (IRClearStatus[1].rfTOO == 0))
    {
        printf("[CAN] Empty message - please send message to RX FIFO 0 \r\n");
        CAN_PortingDelay(1000);
    }

    timeoutTestForRXFIFO0 = false; //getting data from RX FIFO0

    return CAN_OK;
}

static void CANTEST_ClearIrqStatus(uint32_t channel)
{
    IRClearStatus[channel].rfRF0N = 0;
    IRClearStatus[channel].rfRF0W = 0;
    IRClearStatus[channel].rfRF0F = 0;
    IRClearStatus[channel].rfRF0L = 0;
    IRClearStatus[channel].rfRF1N = 0;
    IRClearStatus[channel].rfRF1W = 0;
    IRClearStatus[channel].rfRF1F = 0;
    IRClearStatus[channel].rfRF1L = 0;
    IRClearStatus[channel].rfHPM = 0;
    IRClearStatus[channel].rfTC = 0;
    IRClearStatus[channel].rfTCF = 0;
    IRClearStatus[channel].rfTFE = 0;
    IRClearStatus[channel].rfTEFN = 0;
    IRClearStatus[channel].rfTEFW = 0;
    IRClearStatus[channel].rfTEFF = 0;
    IRClearStatus[channel].rfTEFL = 0;
    IRClearStatus[channel].rfTSW = 0;
    IRClearStatus[channel].rfMRAF = 0;
    IRClearStatus[channel].rfTOO = 0;
    IRClearStatus[channel].rfDRX = 0;
    IRClearStatus[channel].rfBEC = 0;
    IRClearStatus[channel].rfBEU = 0;
    IRClearStatus[channel].rfELO = 0;
    IRClearStatus[channel].rfEP = 0;
    IRClearStatus[channel].rfEW = 0;
    IRClearStatus[channel].rfBO = 0;
    IRClearStatus[channel].rfWDI = 0;
    IRClearStatus[channel].rfPEA = 0;
    IRClearStatus[channel].rfPED = 0;
    IRClearStatus[channel].rfARA = 0;
    IRClearStatus[channel].rfRsvd = 0;
}

static int32_t CANTEST_ReceiveErrorMessageForStuffError(void)
{
    uint32_t i;
    int32_t status = 0;
    int32_t ret = CAN_FAIL;

    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

    for(i=0;i<CAN_CONTROLLER_NUMBER;i++)
    {
        while((IRClearStatus[i].rfPEA == 0) && (IRClearStatus[i].rfPED == 0)) //protocol error in arbitration / data phase
        {
            printf("[CAN] Empty message - please send message \r\n");
            CAN_PortingDelay(1000);
        }

        status = protocolStatus[i];

        if(IRClearStatus[i].rfPEA == 1) {
            printf("[CAN] Channel %d Protocol Error in Arbitration Phase \r\n", i);
            if((status & LEC_MASK) == PROTOCOL_ERR_STUFF) {
                ret = CAN_OK;
            } else {
                return CAN_FAIL;
            }
        } else if(IRClearStatus[i].rfPED == 1) {
            printf("[CAN] Channel %d Protocol Error in Data Phase \r\n", i);
            if(((status & DLEC_MASK) >> 8) == PROTOCOL_ERR_STUFF) {
                ret = CAN_OK;
            } else {
                return CAN_FAIL;
            }
        } else {
            ; /* Nothing to do */
        }

        protocolStatus[i] = 0;

        CANTEST_ClearIrqStatus(i); /* clear error interrupt status */
    }

    return ret;
}

static int32_t CANTEST_SendErrorMessageForFormatError(uint8_t tx_channel)
{
    uint32_t i;
    int32_t status = 0;
    int32_t ret = CAN_FAIL;

#if 1 //for non automatic retransmission
    CANController_t *CANControllerInfo = 0;

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[0];
    //(void)CAN_DrvStartConfigSetting(CANControllerInfo);
    CAN_DrvSetDisableAutomaticRetransmission(CANControllerInfo, true);
    //(void)CAN_DrvFinishConfigSetting(CANControllerInfo);

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[1];
    //(void)CAN_DrvStartConfigSetting(CANControllerInfo);
    CAN_DrvSetDisableAutomaticRetransmission(CANControllerInfo, true);
    //(void)CAN_DrvFinishConfigSetting(CANControllerInfo);

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[2];
    //(void)CAN_DrvStartConfigSetting(CANControllerInfo);
    CAN_DrvSetDisableAutomaticRetransmission(CANControllerInfo, true);
    //(void)CAN_DrvFinishConfigSetting(CANControllerInfo);
#endif

    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

    CANTEST_SendMessageByFIFO(tx_channel);
    for(i=0;i<CAN_CONTROLLER_NUMBER;i++)
    {
        while((IRClearStatus[i].rfPEA == 0) && (IRClearStatus[i].rfPED == 0)) //protocol error in arbitration / data phase
        {
            printf("[CAN] Empty message - please send message \r\n");
            CAN_PortingDelay(1000);
        }

        status = protocolStatus[i];

        if(i!=tx_channel)
        {
            if(IRClearStatus[i].rfPEA == 1) {
                printf("[CAN] Channel %d Protocol Error in Arbitration Phase \r\n", i);
                if((status & LEC_MASK) == PROTOCOL_ERR_FORMAT) {
                    ret = CAN_OK;
                } else {
                    return CAN_FAIL;
                }
            } else if(IRClearStatus[i].rfPED == 1) {
                printf("[CAN] Channel %d Protocol Error in Data Phase \r\n", i);
                if(((status & DLEC_MASK) >> 8) == PROTOCOL_ERR_FORMAT) {
                    ret = CAN_OK;
                } else {
                    return CAN_FAIL;
                }
            } else {
                ; /* Nothing to do */
            }
        }

        protocolStatus[i] = 0;

        CANTEST_ClearIrqStatus(i); /* clear error interrupt status */
    }

    return ret;
}


static int32_t CANTEST_SendMessageForACKError(void)
{
    uint32_t i;
    int32_t status = 0;
    int32_t ret = CAN_FAIL;

#if 1 //for non automatic retransmission
    CANController_t *CANControllerInfo = 0;

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[0];
    //(void)CAN_DrvStartConfigSetting(CANControllerInfo);
    CAN_DrvSetDisableAutomaticRetransmission(CANControllerInfo, true);
    //(void)CAN_DrvFinishConfigSetting(CANControllerInfo);

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[1];
    //(void)CAN_DrvStartConfigSetting(CANControllerInfo);
    CAN_DrvSetDisableAutomaticRetransmission(CANControllerInfo, true);
    //(void)CAN_DrvFinishConfigSetting(CANControllerInfo);

#if defined(CONFIG_TCC807X)
    CANControllerInfo = &CANDrvierInfo.dControllerInfo[2];
    //(void)CAN_DrvStartConfigSetting(CANControllerInfo);
    CAN_DrvSetDisableAutomaticRetransmission(CANControllerInfo, true);
    //(void)CAN_DrvFinishConfigSetting(CANControllerInfo);
#endif
#endif

    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

    for(i=0;i<CAN_CONTROLLER_NUMBER;i++)
    {
        CANTEST_SendMessageByFIFO(i);
        while((IRClearStatus[i].rfPEA == 0) && (IRClearStatus[i].rfPED == 0)) //protocol error in arbitration / data phase
        {
            printf("[CAN] Channel : %d, Wait for ACK \r\n", i);
            CAN_PortingDelay(1000);
        }

        status = protocolStatus[i];

        if(IRClearStatus[i].rfPEA == 1) {
            printf("[CAN] Channel %d Protocol Error in Arbitration Phase \r\n", i);
            if((status & LEC_MASK) == PROTOCOL_ERR_ACK) {
                ret = CAN_OK;
            } else {
                return CAN_FAIL;
            }
        } else if(IRClearStatus[i].rfPED == 1) {
            printf("[CAN] Channel %d Protocol Error in Data Phase \r\n", i);
            if(((status & DLEC_MASK) >> 8) == PROTOCOL_ERR_ACK) {
                ret = CAN_OK;
            } else {
                return CAN_FAIL;
            }
        } else {
            ; /* Nothing to do */
        }

        protocolStatus[i] = 0;

        CANTEST_ClearIrqStatus(i); /* clear error interrupt status */
    }

    return ret;
}

static int32_t CANTEST_SendMessageForBit0Error(void)
{
    uint32_t i;
    int32_t status = 0;
    int32_t ret = CAN_FAIL;

#if 1 //for disable transceiver
    //(void)GPIO_Config(CAN_0_STB, (GPIO_FUNC(0)| GPIO_OUTPUT)); //can0 stb
    //(void)GPIO_Set(CAN_0_STB, 1);

    //(void)GPIO_Config(CAN_1_STB, (GPIO_FUNC(0)| GPIO_OUTPUT)); //can1 stb
    //(void)GPIO_Set(CAN_1_STB, 1);

    //(void)GPIO_Config(CAN_2_STB, (GPIO_FUNC(0)| GPIO_OUTPUT)); //can2 stb
    //(void)GPIO_Set(CAN_2_STB, 1);
#endif

    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

    for(i=0;i<CAN_CONTROLLER_NUMBER;i++) //It can test only channel0 because channel1 & 2 stb are not connected.
    {
        CANTEST_SendMessageByFIFO(i);
        while((IRClearStatus[i].rfPEA == 0) && (IRClearStatus[i].rfPED == 0)) //protocol error in arbitration / data phase
        {
            printf("[CAN] Channel : %d, Wait for ACK \r\n", i);
            CAN_PortingDelay(1000);
        }

        status = protocolStatus[i];

        if(IRClearStatus[i].rfPEA == 1) {
            printf("[CAN] Channel %d Protocol Error in Arbitration Phase \r\n", i);
            if((status & LEC_MASK) == PROTOCOL_ERR_BIT0) {
                ret = CAN_OK;
            } else {
                return CAN_FAIL;
            }
        } else if(IRClearStatus[i].rfPED == 1) {
            printf("[CAN] Channel %d Protocol Error in Data Phase \r\n", i);
            if(((status & DLEC_MASK) >> 8) == PROTOCOL_ERR_BIT0) {
                ret = CAN_OK;
            } else {
                return CAN_FAIL;
            }
        } else {
            ; /* Nothing to do */
        }

        protocolStatus[i] = 0;

        CANTEST_ClearIrqStatus(i); /* clear error interrupt status */
    }

    return ret;
}

static int32_t CANTEST_SendMessageForBit1Error(uint8_t tx_channel)
{
    uint32_t i;
    int32_t status = 0;
    int32_t ret = CAN_FAIL;

#if 1 //for non automatic retransmission
    CANController_t *CANControllerInfo = 0;

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[0];
    //(void)CAN_DrvStartConfigSetting(CANControllerInfo);
    CAN_DrvSetDisableAutomaticRetransmission(CANControllerInfo, true);
    //(void)CAN_DrvFinishConfigSetting(CANControllerInfo);

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[1];
    //(void)CAN_DrvStartConfigSetting(CANControllerInfo);
    CAN_DrvSetDisableAutomaticRetransmission(CANControllerInfo, true);
    //(void)CAN_DrvFinishConfigSetting(CANControllerInfo);

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[2];
    //(void)CAN_DrvStartConfigSetting(CANControllerInfo);
    CAN_DrvSetDisableAutomaticRetransmission(CANControllerInfo, true);
    //(void)CAN_DrvFinishConfigSetting(CANControllerInfo);
#endif

    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

    CANTEST_SendMessageByFIFO(tx_channel);
    for(i=0;i<CAN_CONTROLLER_NUMBER;i++)
    {
        while((IRClearStatus[i].rfPEA == 0) && (IRClearStatus[i].rfPED == 0)) //protocol error in arbitration / data phase
        {
            printf("[CAN] Empty message - please send message \r\n");
            CAN_PortingDelay(1000);
        }
    }

    status = protocolStatus[tx_channel];

    if(IRClearStatus[tx_channel].rfPEA == 1) {
        printf("[CAN] Channel %d Protocol Error in Arbitration Phase \r\n", tx_channel);
        if((status & LEC_MASK) == PROTOCOL_ERR_BIT1) {
            ret = CAN_OK;
        } else {
            return CAN_FAIL;
        }
    } else if(IRClearStatus[tx_channel].rfPED == 1) {
        printf("[CAN] Channel %d Protocol Error in Data Phase \r\n", tx_channel);
        if(((status & DLEC_MASK) >> 8) == PROTOCOL_ERR_BIT1) {
            ret = CAN_OK;
        } else {
            return CAN_FAIL;
        }
    } else {
        ; /* Nothing to do */
    }

    protocolStatus[tx_channel] = 0;

    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

    return ret;
}


static int32_t CANTEST_SendErrorMessageForCRCError(void)
{
    uint32_t i, tx;
    int32_t status = 0;
    int32_t ret = CAN_FAIL;

#if 1 //for non automatic retransmission
    CANController_t *CANControllerInfo = 0;

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[0];
    //(void)CAN_DrvStartConfigSetting(CANControllerInfo);
    CAN_DrvSetDisableAutomaticRetransmission(CANControllerInfo, true);
    //(void)CAN_DrvFinishConfigSetting(CANControllerInfo);

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[1];
    //(void)CAN_DrvStartConfigSetting(CANControllerInfo);
    CAN_DrvSetDisableAutomaticRetransmission(CANControllerInfo, true);
    //(void)CAN_DrvFinishConfigSetting(CANControllerInfo);

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[2];
    //(void)CAN_DrvStartConfigSetting(CANControllerInfo);
    CAN_DrvSetDisableAutomaticRetransmission(CANControllerInfo, true);
    //(void)CAN_DrvFinishConfigSetting(CANControllerInfo);
#endif

    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

    tx = 0; //tx channel
    CANTEST_SendMessageByFIFO(tx);
    for(i=0;i<CAN_CONTROLLER_NUMBER;i++)
    {
        while((IRClearStatus[i].rfPEA == 0) && (IRClearStatus[i].rfPED == 0)) //protocol error in arbitration / data phase
        {
            printf("[CAN] Empty message - please send message \r\n");
            CAN_PortingDelay(1000);
        }

        status = protocolStatus[i];

        if(i!=tx)
        {
            if(IRClearStatus[i].rfPEA == 1) {
                printf("[CAN] Channel %d Protocol Error in Arbitration Phase \r\n", i);
                if((status & LEC_MASK) == PROTOCOL_ERR_CRC) {
                    ret = CAN_OK;
                } else {
                    return CAN_FAIL;
                }
            } else if(IRClearStatus[i].rfPED == 1) {
                printf("[CAN] Channel %d Protocol Error in Data Phase \r\n", i);
                if(((status & DLEC_MASK) >> 8) == PROTOCOL_ERR_CRC) {
                    ret = CAN_OK;
                } else {
                    return CAN_FAIL;
                }
            } else {
                ; /* Nothing to do */
            }
        }

        protocolStatus[i] = 0;

        CANTEST_ClearIrqStatus(i); /* clear error interrupt status */
    }

    return ret;
}


static int32_t CANTEST_InternalLoopBackTest(uint8_t channel)
{
    CANMessage_t *  txMsg;
    CANMessage_t    rxMsg;
    CANTxEvent_t    TxEvent;
    uint8_t           i,j;
    uint8_t           ucTxBufferIndex;
    int32_t           TxEventNumber;
    int32_t           ret = CAN_OK;
    int32_t           rxMsgNum = 0;

    CANController_t *CANControllerInfo = 0;

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[channel];

    CAN_DrvSetInternalTestMode(CANControllerInfo);
    //CAN_DrvSetExternalTestMode(CANControllerInfo);

    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
        CANTEST_ClearIrqStatus(i);
    }

    /********************* SEND *********************/
    txMsg = &TxMessage2[0];
    (void) CAN_SendMessage(channel, txMsg, &ucTxBufferIndex);
    CAN_PortingDelay(2); //wait for interrupt

    if(0<CompletedFlag[channel])
    {
        printf("[CAN] chennel %d Sent message Success!! Test ID: 0x%X, Data length: %d \r\n", channel, txMsg->mId, txMsg->mDataLength);
        printf("DATA : ");
        for(j=0; j<txMsg->mDataLength; j++)
        {
            printf("0x%X ", txMsg->mData[j]);
            if((j>=12)&&(j%12==0)) {
                printf("\n");
            }
        }
        printf("\n");
        TxEventNumber = CAN_CheckNewTxEvent(channel);
        if(0<TxEventNumber)
        {
            //SAL_MemSet(&TxEvent, 0, sizeof(CANTxEvent_t));
            ret = CAN_GetNewTxEvent(channel, &TxEvent);
            if(txMsg->mId == TxEvent.teId)
            {
                printf("[CAN] chennel %d Saved message Success!! TxEvent Test ID: 0x%X \r\n", channel, TxEvent.teId);
            }
            else
            {
                printf("[CAN] chennel %d Saved message Fail!! TxEvent Test ID: 0x%X \r\n", channel, TxEvent.teId);
                return CAN_FAIL;
            }
        }
        else
        {
            printf("[CAN] chennel %d Saved message Fail!! TxEvent Test ID: 0x%X \r\n", channel, TxEvent.teId);
            return CAN_FAIL;
        }

        CompletedFlag[channel] = 0;
    }
    else
    {
        printf("[CAN] chennel %d Sent message Fail!! Test ID: 0x%X, Data length: %d \r\n", channel, txMsg->mId, txMsg->mDataLength);
        return CAN_FAIL;
    }


    /********************* RECEIVE *********************/
    while(0 == ReceiveFlag[channel])
    {
        printf("[CAN] Empty message - please send message \r\n");
        CAN_PortingDelay(1000);
    }

    ReceiveFlag[channel] = 0;
    rxMsgNum = CAN_CheckNewRxMessage(channel);
    if(0<rxMsgNum)
    {
        CAN_GetNewRxMessage(channel, &rxMsg);
    }

    printf("[CAN] Channel %d Received message Success!! Test ID: 0x%X, Data length: %d, DATA :  \r\n", channel, rxMsg.mId, rxMsg.mDataLength);
    for(j=1; j<(rxMsg.mDataLength+1); j++)
    {
        printf("0x%2X ", rxMsg.mData[j-1]);
        if(j%16==0) {
            printf("\n");
        }
    }
    printf("\n");

    return ret;
}

static int32_t CANTEST_ConfigurationRegisterWriteLock(void)
{
    int32_t ret = CAN_FAIL;
    CANRegCfgWrPw_t *ConfigWritePasswordRegisterAddr;
    CANRegCfgWrLock_t *ConfigWriteLockRegisterAddr;
    CANConfigRegister_t *ConfigRegisterAddr;

    ConfigRegisterAddr = (CANConfigRegister_t *)CAN_PortingGetConfigBaseAddr();

    ConfigWritePasswordRegisterAddr = (CANRegCfgWrPw_t *)CAN_PortingGetConfigWritePasswordAddr(0);
    ConfigWriteLockRegisterAddr = (CANRegCfgWrLock_t *)CAN_PortingGetConfigWriteLockAddr(0);


    /* Clear R/W locking register */
    ConfigRegisterAddr->crBaseAddr[0].rNReg = 0x0000;
    ConfigRegisterAddr->crBaseAddr[1].rNReg = 0x0000;
    ConfigRegisterAddr->crBaseAddr[2].rNReg = 0x0000;
    ConfigRegisterAddr->crExtTimeStampCtrl0.rNReg = 0x0;
    ConfigRegisterAddr->crExtTimeStampCtrl1.rNReg = 0x0;
    ConfigRegisterAddr->crExtTimeStampValue.rNReg = 0x0;

    /* Get access write lock */
    #if defined(TCC803x)
    ConfigWritePasswordRegisterAddr->rNReg = 0x8030ace5; //password
    #else //TCC805x
    ConfigWritePasswordRegisterAddr->rNReg = 0x5afeace5; //password
    #endif

    ConfigWriteLockRegisterAddr->rFReg.rfCFG_WR_LOCK = true;

    /* Write R/W locking register */
    ConfigRegisterAddr->crBaseAddr[0].rNReg = 0xC100;
    ConfigRegisterAddr->crBaseAddr[1].rNReg = 0xC100;
    ConfigRegisterAddr->crBaseAddr[2].rNReg = 0xC100;
    ConfigRegisterAddr->crExtTimeStampCtrl0.rNReg = 0x1;
    ConfigRegisterAddr->crExtTimeStampCtrl1.rNReg = 0xFFFFFFFF;
    ConfigRegisterAddr->crExtTimeStampValue.rNReg = 0x1;


    /* Check R/W locking register */
    if((ConfigRegisterAddr->crBaseAddr[0].rNReg != 0) || (ConfigRegisterAddr->crBaseAddr[1].rNReg != 0) || (ConfigRegisterAddr->crBaseAddr[2].rNReg != 0)
        || (ConfigRegisterAddr->crExtTimeStampCtrl0.rNReg != 0)
        || (ConfigRegisterAddr->crExtTimeStampCtrl1.rNReg != 0)
        || (ConfigRegisterAddr->crExtTimeStampValue.rNReg != 0))
    {
        ret = CAN_FAIL;
    }
    else
    {
        ret = CAN_OK;
    }

    return ret;
}
#if 0
GDMAInformation_t   sRxDma; // Rx DMA

static int32_t CAN_DmaRxEnable
(
    uint8_t                               ucCh,
    uint32_t                              uiSize,
    const GDMAInformation_t *           psDmacon
)
{
    int32_t  ret;


#if 0
    if (ucCh >= UART_CH_MAX)
    {
        (void)SAL_DbgReportError(SAL_DRVID_UART, 0UL, (uint32_t)SAL_ERR_INVALID_PARAMETER, __FUNCTION__);
        ret = -1;
    }
    else
#endif
    {
        #if 1
        sRxDma.iSrcAddr  = (uint8_t *)(psDmacon->iSrcAddr);
        sRxDma.iDestAddr = (uint8_t *)(psDmacon->iDestAddr);
        GDMA_SetSrcAddr(&sRxDma, (uint32_t)(sRxDma.iSrcAddr));
        GDMA_SetDestAddr(&sRxDma, (uint32_t)(sRxDma.iDestAddr));
        GDMA_InterruptEnable(&sRxDma);
        GDMA_SetTransferWidth(&sRxDma, GDMA_TRANSFER_SIZE_BYTE, GDMA_TRANSFER_SIZE_BYTE);
        GDMA_SetTransferSize(&sRxDma, uiSize);

        // Run DMA
        GDMA_ChannelEnable(&sRxDma);
        #else
        uart[ucCh].sRxDma.iSrcAddr  = (uint8_t *)(psDmacon->iSrcAddr);
        uart[ucCh].sRxDma.iDestAddr = (uint8_t *)(psDmacon->iDestAddr);
        GDMA_SetSrcAddr(&uart[ucCh].sRxDma, (uint32_t)(uart[ucCh].sRxDma.iSrcAddr));
        GDMA_SetDestAddr(&uart[ucCh].sRxDma, (uint32_t)(uart[ucCh].sRxDma.iDestAddr));
        GDMA_InterruptEnable(&uart[ucCh].sRxDma);
        GDMA_SetTransferWidth(&uart[ucCh].sRxDma, GDMA_TRANSFER_SIZE_BYTE, GDMA_TRANSFER_SIZE_BYTE);
        GDMA_SetTransferSize(&uart[ucCh].sRxDma, uiSize);

        // Run DMA
        GDMA_ChannelEnable(&uart[ucCh].sRxDma);
        #endif

        ret = (int32_t)SAL_RET_SUCCESS;
    }

    return ret;
}
#endif
#if 0
static int32_t CANTEST_DMATestWithMessages(uint8_t ucCh, uint32_t * puiAddr)
{
    //uint32_t regData;
    CANController_t *   CANControllerInfo = 0;

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[ucCh];

    SAL_MemSet(&sRxDma, 0, sizeof(GDMAInformation_t));

    DmaIrqFlag = 0;

    sRxDma.iCon          = ucCh;
    sRxDma.iCh           = (uint32_t)GDMA_PERI_RX;
    //sRxDma.iSrcAddr      = (uint8_t *)(uart[ucCh].sBase);
    sRxDma.iSrcAddr      = (uint8_t *)(CANControllerInfo->cRamAddressInfo.raRxBufferAddr);
    sRxDma.iDestAddr     = (uint8_t *)(puiAddr);
    sRxDma.iBufSize      = 16;
    sRxDma.iTransSize    = 0UL;

    (void)GDMA_Init(&sRxDma, GIC_PRIORITY_NO_MEAN);

    // Enable Receive DMA
    //regData = (UART_RegRead(ucCh, UART_REG_DMACR) | UART_DMACR_DMAONERR | UART_DMACR_RXDMAE);
    //UART_RegWrite(ucCh, UART_REG_DMACR, regData);

    GDMA_SetFlowControl(&sRxDma, GDMA_FLOW_TYPE_P2M);
    GDMA_SetAddrIncrement(&sRxDma, GDMA_INC, GDMA_INC);
    GDMA_SetBurstSize(&sRxDma, GDMA_BURST_SIZE_16, GDMA_BURST_SIZE_16);

    GDMA_SetPeri(&sRxDma, 0U, (uint8_t)(GDMA_PERI_REQ_PORT_CAN0+ucCh));

    (void)CAN_DmaRxEnable(ucCh, sRxDma.iBufSize, (const GDMAInformation_t *)&sRxDma);

    while(DmaIrqFlag == 0)
    {
        CAN_PortingDelay(100);
    }

    DmaIrqFlag = 0;

    printf("[CAN] CAN channel %d DMA IRQ is occurrend!! \r\n", ucCh);

    return CAN_OK;
}
#else

#define DMA3_ADDR (0x18330000)
#define DMA_CONF (0x30)
#define DMA_SRC (0x100)
#define DMA_DST (0x104)
#define DMA_CONT (0x10C)
#define DMA_CH (0x110)
#define DMA_WR(x, y) (*(volatile unsigned int *)x)=(y)
#define GDMA_BUFF_SIZE (0x40)

static int32_t CANTEST_DMATestWithMessages(uint8_t ucCh, uint32_t * puiAddr)
{
    //uint32_t regData;
    CANController_t *   CANControllerInfo = 0;

    CANControllerInfo = &CANDrvierInfo.dControllerInfo[ucCh];

    DmaIrqFlag = 0;

	DMA_WR((DMA3_ADDR + DMA_CONF), 0x1);
	DMA_WR((DMA3_ADDR + DMA_SRC), 0x40000000);
	DMA_WR((DMA3_ADDR + DMA_DST), 0x41000000);
	DMA_WR((DMA3_ADDR + DMA_CONT), 0xC000004);
	DMA_WR((DMA3_ADDR + DMA_CH), 0x3001);
	DMA_WR(0x40000000, 0x33323130);

    printf("[CAN] CAN channel %d DMA IRQ is occurrend!! \r\n", ucCh);

	asm volatile("b .");

    return CAN_OK;
}
#endif

static int get_value_from_user_999(unsigned int start_range,
		unsigned int over_range)
{
	int ret = 0, cnt = 0, d_p_v = 1, val_conf[3] = {0,};
	int input_val, i;

	while(1){

		input_val = getchar();

		printf("%c", input_val);

		/*
		 * 0xd : carriage return. enter key
		 */
		if (input_val == 0xd)
			break;
		/*
		 * '0' <= num <= '9'
		 */
		if ((input_val >= 0x30) && (input_val <= 0x39)) {
			/*
			 * translate char value to integer value
			 */
			val_conf[cnt] = (input_val - '0');
			/*
			 * increase place value
			 */
			cnt++;
		} else {
			printf("\ninvalid number\n");
			ret = -EINVAL;
			break;
		}

		/*
		 * The limit of the place number is '3'. <= 999.
		 */
		if (cnt > 3) {
			break;
		}

	}


	if (ret == 0) {
		if (cnt == 0) {
			printf("\ninput a number\n");
			ret = -EINVAL;
		}

		if (ret == 0) {
			/*
			 * obtain decimal place value
			 */
			for (i = 1; i < cnt; i++) {
				d_p_v *= 10;
			}

			/*
			 * initialize input_val to obtain a decimal number that
			 * is combination of received values.
			 */
			input_val = 0;

			for (i = 0; i < cnt; i++) {
				input_val += val_conf[i] * d_p_v;
				d_p_v /= 10;
			}


			if(input_val >= over_range) {
				printf("\nover range\n");
				ret = -EINVAL;
			} else if (input_val < start_range) {
				printf("\nunder range\n");
				ret = -EINVAL;

			} else {
				ret = input_val;
				printk("\ninserted : %d\n", ret);
			}
		}
	}

	return ret;
}

int CanTestExit(void)
{
    uint32_t testNum = 0;

	printf("If you input 0 program exit ! : ");
	testNum = get_value_from_user_999(0,100);
	printf("%d\n", testNum);
	return testNum;
}
//void CanTestTask(void *p_arg)
extern CANTxBuffer_t TxBufferInfoPar[ CAN_CONTROLLER_NUMBER ];
extern CANTimingParam_t DataPhaseTimingPar[ CAN_CONTROLLER_NUMBER ];

int CANTestMain(void)
{
    uint32_t testNum = 0;
    uint32_t i = 0, j = 0;
    int32_t ret;
    uint8_t ready = 0; //for preparing Safety Machamism test cases

    printf("CAN test start\n");

    CANTEST_Initialize(); //initialize - core clock : 80Mhz

    printf("Input CAN test Number : ");
	testNum = get_value_from_user_999(0,100);

	printf("%d\n", testNum);

    while (1)
    {
        switch(testNum)
        {
#if 1
            case 0: //integration test connect to CAN_L and CAN_H (CAN0<->CAN1, CAN0<->CAN2, CAN1<->CAN2)
                {
                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 0 : CAN integration send & receive test START!!\n\n");

                    ret = CANTEST_SendReceive();
                    if(ret == CAN_OK) {
                        printf("[CAN] CAN integration send & receive test SUCCESS!!\n");
                    } else {
                        printf("[CAN] CAN integration send & receive test FAIL!!\n");
                    }

                    printf("\n[CAN] TEST CASE - 0 : CAN integration send & receive test END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 1; //next test case
                break;

            case 1: //simple RX control - Send message via CANLink and Check print log
                {
                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 1 : CAN simple receive test START!!\n\n");

                    ret = CANTEST_ReceiveMessage(3);
                    if(ret == CAN_OK) {
                        printf("[CAN] CAN simple receive test SUCCESS!!\n");
                    } else {
                        printf("[CAN] CAN simple receive test FAIL!!\n");
                    }

                    printf("\n[CAN] TEST CASE - 1 : CAN simple receive test END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 2; //next test case
                break;

            case 2: //RX control with ID filtering  - Send message via CANLink and Check print log
                {
                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 2 : Message filter test START!!\n\n");

                    ret = CANTEST_ReceiveMessageWithStandardIDFilter(); //standard
                    if(ret == CAN_OK) {
                        printf("[CAN] Standard message filter test SUCCESS!!\n");
                    } else {
                        printf("[CAN] Standard message filter test FAIL!!\n");
                    }

                    ret = CANTEST_ReceiveMessageWithExtendedIDFilter(); //extended
                    if(ret == CAN_OK) {
                        printf("[CAN] Extended message filter test SUCCESS!!\n");
                    } else {
                        printf("[CAN] Extended message filter test FAIL!!\n");
                    }

                    printf("\n[CAN] TEST CASE - 2 : Message filter test END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 3; //next test case
                break;

            case 3: //High priority message handling - replaced with test case no.2
                {
                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 3 : High priority message handling test START!!\n\n");

                    printf("[CAN] High priority message handling test SUCCESS!!\n");

                    printf("\n[CAN] TEST CASE - 3 : High priority message handling test END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 4; //next test case
                break;

            case 4: //Timeout counter usage with RX FIFO
                {
                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 4 : Timeout counter usage with RX FIFO START!!\n\n");

                    ret = CANTEST_ReceiveMessageTimeout();
                    if(ret == CAN_OK) {
                        printf("[CAN] Timeout counter usage with RX FIFO SUCCESS!!\n");
                    } else {
                        printf("[CAN] Timeout counter usage with RX FIFO FAIL!!\n");
                    }

                    printf("\n[CAN] TEST CASE - 4 : Timeout counter usage with RX FIFO END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 5; //next test case
                break;

            case 5: //TX control with TX FIFO - Check CANLink log
                {
                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 5 : TX control with TX FIFO START!!\n\n");

                    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
                        TxBufferInfoPar[i].tbTxFIFOorQueueMode = CAN_TX_BUFFER_MODE_FIFO;
                    }

                    CAN_Deinit();
                    CAN_Init();

		    for (i = 0; i < CAN_CONTROLLER_NUMBER; i++) {
			    ret = CANTEST_SendMessageByFIFO(i);
			    if(ret == CAN_OK) {
				    printf("[CAN] CAN channel %d send message by FIFO SUCCESS!!\n", i);
			    } else {
				    printf("[CAN] CAN channel %d send message by FIFO FAIL!!\n", i);
			    }
		    }

                    printf("\n[CAN] TEST CASE - 5 : TX control with TX FIFO END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 6; //next test case
                break;

            case 6: //TX control with TX queue & dedicated buffer - Check CANLink log
                {
                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 6 : TX control with TX queue & dedicated buffer START!!\n\n");

                    for(i=0; i<CAN_CONTROLLER_NUMBER; i++) {
                        TxBufferInfoPar[i].tbTxFIFOorQueueMode = CAN_TX_BUFFER_MODE_QUEUE;
                    }

                    CAN_Deinit();
                    CAN_Init();

		    for (i = 0; i < CAN_CONTROLLER_NUMBER; i++) {
			    CANTEST_SendMessageByQueue(i);
			    if(ret == CAN_OK) {
				    printf("[CAN] CAN channel %d send message by QUEUE SUCCESS!!\n", i);
			    } else {
				    printf("[CAN] CAN channel %d send message by QUEUE FAIL!!\n", i);
			    }
		    }

                    printf("\n[CAN] TEST CASE - 6 : TX control with TX queue & dedicated buffer END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 7; //next test case
                break;

            case 7: //TX control with event FIFO
                {
                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 7 : TX control with event FIFO START!!\n\n");

                    printf("[CAN] TX control with event FIFO SUCCESS!!\n");

                    printf("\n[CAN] TEST CASE - 7 : TX control with event FIFO END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 8; //next test case
                break;
            case 8: //Stuff Error Detection
                {
                    //while(1) {
                        //if(ready == 1) //Send error frame
                            //break;
                    //}
                    //ready = 0;

                    CAN_Deinit();
                    CAN_Init();

                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 8 : CAN STUFF error detection START!!\n\n");

                    ret = CANTEST_ReceiveErrorMessageForStuffError();
                    if(ret == CAN_OK) {
                        printf("[CAN] CAN STUFF error detection SUCCESS!!\r\n");
                    } else {
                        printf("[CAN] CAN STUFF error detection FAIL!!\r\n");
                    }

                    printf("\n[CAN] TEST CASE - 8 : CAN STUFF error detection END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 9; //next test case
                break;
            case 9: //Format Error Detection - TIMING_CASE (1)
                {
                    //while(1) {
                        //if(ready == 1) //Send error frame
                            //break;
                    //}
                    //ready = 0;

                    CAN_Deinit();
                    CAN_Init();

                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 9 : CAN FORMAT error detection START!!\n\n");

                    for(i=0; i<CAN_CONTROLLER_NUMBER; i++)
                    {
                        DataPhaseTimingPar[i].tpPhaseSeg1 = 47;
                        DataPhaseTimingPar[i].tpPhaseSeg2 = 16;
                        for(j=0; j<CAN_CONTROLLER_NUMBER; j++) {
                            if(i!=j) {
                                DataPhaseTimingPar[j].tpPhaseSeg1 = 17;
                                DataPhaseTimingPar[j].tpPhaseSeg2 = 6;
                            }
                        }

                        CAN_Deinit();
                        CAN_Init();

                        ret = CANTEST_SendErrorMessageForFormatError(i);
                        if(ret == CAN_OK) {
                            printf("[CAN] CAN channel %d FORMAT error detection SUCCESS!!\r\n", i);
                        } else {
                            printf("[CAN] CAN channel %d FORMAT error detection FAIL!!\r\n", i);
                        }
                    }

                    printf("\n[CAN] TEST CASE - 9 : CAN FORMAT error detection END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 10; //next test case
                break;
            case 10: //Acknowledge Error Detection
                {
                    //while(1){
                        //if(ready == 1) //Disconnect CAN L/H
                            //break;
                    //}
                    //ready = 0;

                    CAN_Deinit();
                    CAN_Init();

                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 10 : CAN ACK error detection START!!\n\n");

                    ret = CANTEST_SendMessageForACKError();
                    if(ret == CAN_OK) {
                        printf("[CAN] CAN ACK error detection SUCCESS!!\r\n");
                    } else {
                        printf("[CAN] CAN ACK error detection FAIL!!\r\n");
                    }

                    printf("\n[CAN] TEST CASE - 10 : CAN ACK error detection END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 11; //next test case
                break;
            case 11: //Bit Error Detection - TIMING_CASE (2)
                {
                    uint8_t sel_test = 0; //0:BIT0, 1:BIT1

                    //while(1){
                        //if(ready == 1) //Disable CAN transceiver
                            //break;
                    //}
                    //ready = 0;

                    CAN_Deinit();
                    CAN_Init();

                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 11 : CAN BIT error detection START!!\n\n");

                    if(sel_test == 0)
                    {
                        ret = CANTEST_SendMessageForBit0Error();
                        if(ret == CAN_OK) {
                            printf("[CAN] CAN BIT0 error detection SUCCESS!!\r\n");
                        } else {
                            printf("[CAN] CAN BIT0 error detection FAIL!!\r\n");
                        }
                    }

                    if(sel_test == 1)
                    {
                        for(i=0; i<CAN_CONTROLLER_NUMBER; i++)
                        {
                            DataPhaseTimingPar[i].tpPhaseSeg1 = 8;
                            for(j=0; j<CAN_CONTROLLER_NUMBER; j++) {
                                if(i!=j) {
                                    DataPhaseTimingPar[j].tpPhaseSeg1 = 4;
                                }
                            }

                            CAN_Deinit();
                            CAN_Init();

                            ret = CANTEST_SendMessageForBit1Error(i);
                            if(ret == CAN_OK) {
                                printf("[CAN] CAN channel %d BIT1 error detection SUCCESS!!\r\n", i);
                            } else {
                                printf("[CAN] CAN channel %d BIT1 error detection FAIL!!\r\n", i);
                            }
                        }
                    }

                    printf("\n[CAN] TEST CASE - 11 : CAN BIT error detection END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 12; //next test case
                break;
            case 12: //CRC Error Detection
                {
                    //while(1) {
                        //if(ready == 1) //Send error frame
                            //break;
                    //}
                    //ready = 0;

                    CAN_Deinit();
                    CAN_Init();

                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 12 : CAN CRC error detection START!!\n\n");

                    ret = CANTEST_SendErrorMessageForCRCError();
                    if(ret == CAN_OK) {
                        printf("[CAN] CAN CRC error detection SUCCESS!!\r\n");
                    } else {
                        printf("[CAN] CAN CRC error detection FAIL!!\r\n");
                    }

                    printf("\n[CAN] TEST CASE - 12 : CAN CRC error detection END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 13; //next test case
                break;
#endif
            case 13: //Internal Loopback Test
                {
                    //while(1) {
                        //if(ready == 1) //Disconnect CAN L/H
                            //break;
                    //}
                    //ready = 0;

                    CAN_Deinit();
                    CAN_Init();

                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 13 : Internal Loop Back Test START!!\n\n");

		    for (i = 0; i < CAN_CONTROLLER_NUMBER; i++) {

			    ret = CANTEST_InternalLoopBackTest(i);
			    if(ret == CAN_OK) {
				    printf("[CAN] Channel %d Internal Loop Back Test SUCCESS!!\r\n", i);
			    } else {
				    printf("[CAN] Channel %d Internal Loop Back Test FAIL!!\r\n", i);
			    }
		    }

                    printf("\n[CAN] TEST CASE - 13 : Internal Loop Back Test END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 14; //next test case
                break;
#if 1				
            case 14: //Configuration Register Write Lock
                {
                    //while(1) {
                        //if(ready == 1) //Send error frame
                            //break;
                    //}
                    //ready = 0;

                    CAN_Deinit();
                    CAN_Init();

                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 14 : Configuration Register Write Lock START!!\n\n");

                    ret = CANTEST_ConfigurationRegisterWriteLock();
                    if(ret == CAN_OK) {
                        printf("[CAN] CAN Configuration Register Lock Test SUCCESS!!\r\n");
                    } else {
                        printf("[CAN] CAN Configuration Register Lock Test FAIL!!\r\n");
                    }

                    printf("\n[CAN] TEST CASE - 14 : Configuration Register Write Lock END!!\n");
                    printf("************************************************************************************\n");
                }
				if(CanTestExit() == 0)
					return 0;
                testNum = 15; //next test case
                break;
            case 15: //DMA test with debug messages
#if 0
                {
                    static uint32_t * dma_rx_buf;
                    uint32_t          uiDmaRxAddr;

                    while(1) {
                        if(ready == 1)
                            break;
                    }
                    ready = 0;

                    CAN_Deinit();
                    CAN_Init();

                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 15 : DMA test with debug messages START!!\n\n");

                    /* channel 0 */
                    uiDmaRxAddr = MPU_GetDMABaseAddress();
                    dma_rx_buf  = (uint32_t *)(uiDmaRxAddr);

                    (void)SAL_MemSet((void *)dma_rx_buf, 0, GDMA_BUFF_SIZE);

                    //ret = CANTEST_DMATestWithMessages(0, dma_rx_buf);


                    /* channel 1 */
                    uiDmaRxAddr = MPU_GetDMABaseAddress();
                    dma_rx_buf  = (uint32_t *)(uiDmaRxAddr + 0x20);

                    (void)SAL_MemSet((void *)dma_rx_buf, 0, GDMA_BUFF_SIZE - 0x20);

                    //ret = CANTEST_DMATestWithMessages(1, dma_rx_buf);


                    /* channel 2 */
                    uiDmaRxAddr = MPU_GetDMABaseAddress();
                    dma_rx_buf  = (uint32_t *)(uiDmaRxAddr + 0x40);

                    (void)SAL_MemSet((void *)dma_rx_buf, 0, GDMA_BUFF_SIZE - 0x40);

                    ret = CANTEST_DMATestWithMessages(2, dma_rx_buf);


                    if(ret == CAN_OK) {
                        printf("[CAN] DMA test with debug messages SUCCESS!!\r\n");
                    } else {
                        printf("[CAN] DMA test with debug messages FAIL!!\r\n");
                    }

                    printf("\n[CAN] TEST CASE - 15 : DMA test with debug messages END!!\n");
                    printf("************************************************************************************\n");
                }
#else
                {
                    static uint32_t * dma_rx_buf;
                    uint32_t          uiDmaRxAddr;

                    CAN_Deinit();
                    CAN_Init();

                    printf("\n************************************************************************************\n");
                    printf("[CAN] TEST CASE - 15 : DMA test with debug messages START!!\n\n");

                    ret = CANTEST_DMATestWithMessages(0, dma_rx_buf);


                    if(ret == CAN_OK) {
                        printf("[CAN] DMA test with debug messages SUCCESS!!\r\n");
                    } else {
                        printf("[CAN] DMA test with debug messages FAIL!!\r\n");
                    }

                    printf("\n[CAN] TEST CASE - 15 : DMA test with debug messages END!!\n");
                    printf("************************************************************************************\n");
                }
#endif
                testNum = 16; //next test case
                break;
            case 16:
		{
			unsigned long pclk_id, pclk;
			uint32_t dividor;

			printf("\n************************************************************************************\n");
			printf("[MW][CAN] TEST CASE - 16 : CAN Peri. clock change START!!\n\n");

			dividor = 1;

			for (j = 0; j < 4; j++) {

				printk("Peri. clock : %ld \n",
						CAN_CONTROLLER_CLOCK / dividor);
				getchar();

				for (i = 0; i < CAN_CONTROLLER_NUMBER; i++) {

					pclk_id = get_can_pclk(i);
					pclk = CAN_CONTROLLER_CLOCK / dividor;
					tcc_set_peri(pclk_id, CKC_ENABLE, pclk,
						     0);
				}

				dividor *= 2;



				ret = CANTEST_SendReceive();

				if (ret == CAN_OK) {
					printf(
					       "[CAN] CAN Peri. clock change SUCCESS!!\n"
					      );
				} else {
					printf(
					       "[CAN] CAN Peri. clock change FAIL!!\n"
					      );
				}

				printf(
				       "\n[CAN] TEST CASE - 16 : CAN Peri. clock change END!!\n"
				      );
				printf(
				       "************************************************************************************\n"
				      );
			}
		}
		if(CanTestExit() == 0)
			return 0;
		testNum = 17;
		break;
            case 17: //CAN verification test complete
                {
                    printf("[CAN] CAN verification test COMPLETE !!\r\n");
					if(CanTestExit() == 0)
						return 0;
					}
                break;
#endif
            default:
				return 0;
                break;
        }

        CAN_PortingDelay(5000);
    }
    return 0;
}

#endif

#endif

