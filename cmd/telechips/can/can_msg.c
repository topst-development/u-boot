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
#include <test/can_drv.h>
#include <test/can_msg.h>

#include <stdio.h>

//#include <sal_internal.h>


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

static CANRxRingBuffer_t RxRingBufferManager[ CAN_CONTROLLER_NUMBER ]; /* Rx Message */


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

static uint8_t CAN_MsgGetDataSizeBySizeType
(
    CANDataLength_t                     uiDataLengthCode
);

static CANDataLength_t CAN_MsgGetSizeTypebyDataSize
(
    uint8_t                               ucDataSize
);

static CANErrorType_t CAN_MsgCopyRxMessage
(
    const CANRamRxBuffer_t *            psSrcReceivedMsg,
    CANMessage_t *                      psRxBuffer
);

static CANMessage_t *CAN_MsgGetRxBufferFromRingBuffer
(
    uint8_t                               ucCh,
    CANRingBufferType_t                 ucType
);

static CANErrorType_t CAN_MsgCopyTxMessage
(
    CANRamTxBuffer_t *                  psTxMsg,
    const CANMessage_t *                psSrcMsg
);

static CANErrorType_t CAN_MsgRequestTxMessage
(
    CANController_t *                   psControllerInfo,
    uint8_t                               ucReqIndex
);

/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

CANErrorType_t CAN_MsgInit
(
    const CANController_t *             psControllerInfo
)
{
    uint8_t           ucCh;
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        ucCh = psControllerInfo->cChannelHandle;

        RxRingBufferManager[ ucCh ].rrbTailIdx = 0;
        RxRingBufferManager[ ucCh ].rrbHeadIdx = 0;
        RxRingBufferManager[ ucCh ].rrbMsgBufferMax = CAN_RX_MSG_RING_BUFFER_MAX;

        ( void )memset( &RxRingBufferManager[ ucCh ].rrbMsg[ 0 ], 0, ( sizeof( CANMessage_t ) * RxRingBufferManager[ ucCh ].rrbMsgBufferMax ) );

        RxRingBufferManager[ ucCh ].rrbInitFlag = true;
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static uint8_t CAN_MsgGetDataSizeBySizeType
(
    CANDataLength_t                     uiDataLengthCode
)
{
    uint8_t ucDataSize;

    ucDataSize = 0;

    switch( uiDataLengthCode )
    {
        case CAN_DATA_LENGTH_12:    // 9,
        {
            ucDataSize = 12;

            break;
        }

        case CAN_DATA_LENGTH_16:    // 10,
        {
            ucDataSize = 16;

            break;
        }

        case CAN_DATA_LENGTH_20:    // 11,
        {
            ucDataSize = 20;

            break;
        }

        case CAN_DATA_LENGTH_24:    // 12,
        {
            ucDataSize = 24;

            break;
        }

        case CAN_DATA_LENGTH_32:    // 13,
        {
            ucDataSize = 32;

            break;
        }

        case CAN_DATA_LENGTH_48:    // 14,
        {
            ucDataSize = 48;

            break;
        }

        case CAN_DATA_LENGTH_64:    // 15,
        {
            ucDataSize = 64;

            break;
        }

        default:
        {
            if( uiDataLengthCode <= CAN_DATA_LENGTH_8 )
            {
                ucDataSize = ( uint8_t ) uiDataLengthCode;
            }
            else
            {
                ucDataSize = 64;
            }

            break;
        }
    }

    return ucDataSize;
}

static CANDataLength_t CAN_MsgGetSizeTypebyDataSize
(
    uint8_t                               ucDataSize
)
{
    CANDataLength_t uiDataLengthCode;

    uiDataLengthCode = CAN_DATA_LENGTH_0;

    if( ucDataSize <= 8U )
    {
        uiDataLengthCode = ( CANDataLength_t ) ucDataSize;
    }
    else if( ucDataSize <= 12U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_12;
    }
    else if( ucDataSize <= 16U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_16;
    }
    else if( ucDataSize <= 20U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_20;
    }
    else if( ucDataSize <= 24U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_24;
    }
    else if( ucDataSize <= 32U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_32;
    }
    else if( ucDataSize <= 48U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_48;
    }
    else if( ucDataSize <= 64U )
    {
        uiDataLengthCode = CAN_DATA_LENGTH_64;
    }
    else
    {
        uiDataLengthCode = CAN_DATA_LENGTH_64;
    }

    return uiDataLengthCode;
}

static CANErrorType_t CAN_MsgCopyRxMessage
(
    const CANRamRxBuffer_t *            psSrcReceivedMsg,
    CANMessage_t *                      psRxBuffer
)
{
    uint8_t           i;
    uint8_t           j;
    uint8_t           ucRemainLength;
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( ( psSrcReceivedMsg != NULL ) && ( psRxBuffer != NULL ) )
    {
        psRxBuffer->mErrorStateIndicator = ( uint8_t ) psSrcReceivedMsg->rR0.rFReg.rfESI;
        psRxBuffer->mRemoteTransmitRequest = ( uint8_t ) psSrcReceivedMsg->rR0.rFReg.rfRTR;
        psRxBuffer->mFDFormat = ( uint8_t ) psSrcReceivedMsg->rR1.rFReg.rfFDF;
        psRxBuffer->mExtendedId = ( uint8_t ) psSrcReceivedMsg->rR0.rFReg.rfXTD;

        if( psRxBuffer->mExtendedId == true )
        {
            psRxBuffer->mId = ( uint32_t ) psSrcReceivedMsg->rR0.rFReg.rfID;
        }
        else
        {
            psRxBuffer->mId = ( uint32_t ) ( ( ( uint32_t ) ( psSrcReceivedMsg->rR0.rFReg.rfID ) >> 18UL ) & 0x7FFUL );
        }

        psRxBuffer->mDataLength = CAN_MsgGetDataSizeBySizeType( ( CANDataLength_t ) psSrcReceivedMsg->rR1.rFReg.rfDLC );

        if( 0U < psRxBuffer->mDataLength )
        {
            ucRemainLength = psRxBuffer->mDataLength % 4U;

            for( i = 0U ; i < ( psRxBuffer->mDataLength / 4U ) ; i++ )
            {
                for( j = 0U ; j < 4U ; j++ )
                {
                    psRxBuffer->mData[ j + ( 4U * i ) ] = ( uint8_t ) ( ( ( uint32_t ) ( ( uint32_t ) psSrcReceivedMsg->rData[ i ] >> ( j * 8U ) ) ) & 0xFFU );
                }
            }

            if( 0U < ucRemainLength )
            {
                for( j = 0U ; j < ucRemainLength ; j++ )
                {
                    psRxBuffer->mData[ j + ( 4U * i ) ] = ( uint8_t ) ( ( ( uint32_t ) ( ( uint32_t )psSrcReceivedMsg->rData[ i ] >> ( j * 8U ) ) ) & 0xFFU );
                }
            }
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

static CANMessage_t * CAN_MsgGetRxBufferFromRingBuffer
(
    uint8_t                               ucCh,
    CANRingBufferType_t                 ucType
)
{
    uint32_t          uiTotalNum;
    CANMessage_t *  psBuffer;

    uiTotalNum      = 0;
    psBuffer        = NULL;

    if( RxRingBufferManager[ ucCh ].rrbInitFlag == true )
    {
        uiTotalNum = CAN_MsgGetCountOfRxMessage( ucCh );

        if( uiTotalNum < CAN_RX_MSG_RING_BUFFER_MAX )
        {
            if( ( uiTotalNum == 0UL ) || ( ucType == CAN_RING_BUFFER_LAST ) )
            {
                psBuffer = &RxRingBufferManager[ ucCh ].rrbMsg[ RxRingBufferManager[ ucCh ].rrbTailIdx ];

                RxRingBufferManager[ ucCh ].rrbTailIdx++;

                if( RxRingBufferManager[ ucCh ].rrbMsgBufferMax <= RxRingBufferManager[ ucCh ].rrbTailIdx )
                {
                    RxRingBufferManager[ ucCh ].rrbTailIdx = 0;
                }
            }
            else
            {
                if( 0UL < RxRingBufferManager[ ucCh ].rrbHeadIdx )
                {
                    RxRingBufferManager[ ucCh ].rrbHeadIdx--;
                }
                else
                {
                    RxRingBufferManager[ ucCh ].rrbHeadIdx = RxRingBufferManager[ ucCh ].rrbMsgBufferMax - 1UL;
                }

                psBuffer = &RxRingBufferManager[ ucCh ].rrbMsg[ RxRingBufferManager[ ucCh ].rrbHeadIdx ];
            }
        }
    }

    return psBuffer;
}

CANErrorType_t CAN_MsgPutRxMessage
(
    uint8_t                               ucCh,
    CANMessageBufferType_t              uiBufferType
)
{
    uint8_t               uiSavedFlag;
    uint32_t              i;
    uint32_t              j;
    uint32_t              uiIndex;
    uint32_t              uiRxMsgAddr;
    uint32_t              uiRxFIFOAddr;
    uint32_t              uiCheckFlag;
    uint32_t              uiGetIdx;
    uint32_t              uiPutIdx;
    uint32_t              uiNewMsgFlag[ 2 ];
    CANRegFieldRXFxS_t  sRxFIFOStatus_bReg;
    CANRegFieldRXFxA_t *psRxFIFOAcknowledge_bReg;
    CANMessage_t *      psRxBuffer;
    CANRamRxBuffer_t *  psReceivedMsg;
    CANController_t *   psControllerInfo;
    CANRingBufferType_t ucRingBufferType;
    CANErrorType_t      ret;

    uiSavedFlag         = false;
    uiCheckFlag         = 0;
    uiNewMsgFlag[ 0 ]   = 0;
    uiNewMsgFlag[ 1 ]   = 0;
    psRxBuffer          = NULL;
    ret                 = CAN_ERROR_NONE;

    psControllerInfo = CAN_GetControllerInfo(ucCh);

    if( psControllerInfo != NULL )
    {
        if( RxRingBufferManager[ ucCh ].rrbInitFlag == true )
        {
            if( uiBufferType == CAN_RX_BUFFER_TYPE_DBUFFER )
            {
                uiNewMsgFlag[ 0 ] = psControllerInfo->cRegister->crNewData1.rNReg;
                uiNewMsgFlag[ 1 ] = psControllerInfo->cRegister->crNewData2.rNReg;

                for( i = 0UL ; i < 2UL ; i++ )
                {
                    for( j = 0UL ; j < 32UL ; j++ )
                    {
                        uiCheckFlag = ( uint32_t ) 1UL << j;

                        if( ( uiNewMsgFlag[ i ] & uiCheckFlag ) != 0UL )
                        {
                            uiIndex = j + ( 32UL * i );

                            psRxBuffer = CAN_MsgGetRxBufferFromRingBuffer( ucCh, CAN_RING_BUFFER_LAST );

                            if( psRxBuffer != NULL )
                            {
                                psRxBuffer->mBufferIndex = ( uint8_t ) uiIndex;
                                uiRxMsgAddr = psControllerInfo->cRamAddressInfo.raRxBufferAddr + ( sizeof( CANRamRxBuffer_t ) * ( uiIndex ) );
                                psReceivedMsg = ( CANRamRxBuffer_t * ) uiRxMsgAddr;
                                psRxBuffer->mBufferType = CAN_RX_BUFFER_TYPE_DBUFFER;

                                ret = CAN_MsgCopyRxMessage( psReceivedMsg, psRxBuffer );

                                if( ret == CAN_ERROR_NONE )
                                {
                                    uiSavedFlag = true;
                                }
                            }
                            else
                            {
                                ret = CAN_ERROR_NO_BUFFER;
                            }

                            break;
                        }
                    }

                    if( uiSavedFlag == true )
                    {
                        break;
                    }
                }

                if( uiSavedFlag == true )
                {
                    if( i == 0UL )
                    {
                        psControllerInfo->cRegister->crNewData1.rNReg = uiNewMsgFlag[ 0 ];
                    }
                    else
                    {
                        psControllerInfo->cRegister->crNewData2.rNReg = uiNewMsgFlag[ 1 ];
                    }
                }
            }
            else
            {
                if( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO0 )
                {
                    sRxFIFOStatus_bReg = psControllerInfo->cRegister->crRxFIFO0Status.rFReg;
                    psRxFIFOAcknowledge_bReg = &psControllerInfo->cRegister->crRxFIFO0Acknowledge.rFReg;
                    uiRxFIFOAddr = psControllerInfo->cRamAddressInfo.raRxFIFO0Addr;
                }
                else if( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO1 )
                {
                    sRxFIFOStatus_bReg = psControllerInfo->cRegister->crRxFIFO1Status.rFReg;
                    psRxFIFOAcknowledge_bReg = &psControllerInfo->cRegister->crRxFIFO1Acknowledge.rFReg;
                    uiRxFIFOAddr = psControllerInfo->cRamAddressInfo.raRxFIFO1Addr;
                }
                else
                {
                    /* unknown buffer type */
                    ret = CAN_ERROR_NO_MESSAGE;
                }

                if( ( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO0 ) || ( uiBufferType == CAN_RX_BUFFER_TYPE_FIFO1 ) )
                {
                    uiGetIdx = sRxFIFOStatus_bReg.rfFxGI;
                    uiPutIdx = sRxFIFOStatus_bReg.rfFxPI;

                    if( uiGetIdx != uiPutIdx )
                    {
                        uiIndex = uiGetIdx;

                        if( ( uint32_t ) psControllerInfo->cRegister->crHighPriorityMessageStatus.rFReg.rfBIDX == uiIndex )
                        {
                            ucRingBufferType = CAN_RING_BUFFER_FIRST;
                        }
                        else
                        {
                            ucRingBufferType = CAN_RING_BUFFER_LAST;
                        }

                        psRxBuffer = CAN_MsgGetRxBufferFromRingBuffer( ucCh, ucRingBufferType );

                        if( psRxBuffer != NULL )
                        {
                            uiRxMsgAddr = uiRxFIFOAddr + ( sizeof( CANRamRxBuffer_t ) * uiIndex );
                            psReceivedMsg = ( CANRamRxBuffer_t * ) uiRxMsgAddr;
                            psRxBuffer->mBufferType = uiBufferType;
                            psRxBuffer->mBufferIndex = ( uint8_t ) uiIndex;

                            ret = CAN_MsgCopyRxMessage( psReceivedMsg, psRxBuffer );

                            if( ret == CAN_ERROR_NONE )
                            {
                                uiSavedFlag = true;
                                psRxFIFOAcknowledge_bReg->rfFxAI = uiIndex;
                            }
                        }
                        else
                        {
                            ret = CAN_ERROR_NO_BUFFER;
                        }
                    }
                }
            }

            if( uiSavedFlag == true )
            {
                CAN_DrvCallbackNotifyRxEvent( ucCh, ( uint32_t )psRxBuffer->mBufferIndex, uiBufferType, CAN_ERROR_NONE );
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

uint32_t CAN_MsgGetCountOfRxMessage
(
    uint8_t                               ucCh
)
{
    uint32_t ret;

    ret = 0;

    if( RxRingBufferManager[ ucCh ].rrbInitFlag == true )
    {
        if( RxRingBufferManager[ ucCh ].rrbHeadIdx <= RxRingBufferManager[ ucCh ].rrbTailIdx )
        {
            ret = RxRingBufferManager[ ucCh ].rrbTailIdx - RxRingBufferManager[ ucCh ].rrbHeadIdx;
        }
        else
        {
            ret = ( CAN_RX_MSG_RING_BUFFER_MAX - RxRingBufferManager[ ucCh ].rrbHeadIdx ) + RxRingBufferManager[ ucCh ].rrbTailIdx;
        }
    }

    return ret;
}

CANErrorType_t CAN_MsgGetRxMessage
(
    uint8_t                               ucCh,
    CANMessage_t *                      psRxMsg
)
{
    uint32_t          uiMsgCount;
    CANErrorType_t  ret;

    uiMsgCount      = 0;
    ret             = CAN_ERROR_NONE;

    if( ( RxRingBufferManager[ ucCh ].rrbInitFlag == true ) && ( psRxMsg != NULL ) )
    {
        uiMsgCount = CAN_MsgGetCountOfRxMessage( ucCh );

        if( 0UL < uiMsgCount )
        {
            ( void ) memcpy( psRxMsg, &RxRingBufferManager[ ucCh ].rrbMsg[ RxRingBufferManager[ ucCh ].rrbHeadIdx ], sizeof( CANMessage_t ) );

            RxRingBufferManager[ ucCh ].rrbHeadIdx++;

            if( CAN_RX_MSG_RING_BUFFER_MAX <= RxRingBufferManager[ ucCh ].rrbHeadIdx )
            {
                RxRingBufferManager[ ucCh ].rrbHeadIdx = 0;

                ret = CAN_ERROR_BAD_PARAM;
            }
        }
        else
        {
            ret = CAN_ERROR_NO_MESSAGE;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

/* Tx Message */
static CANErrorType_t CAN_MsgCopyTxMessage
(
    CANRamTxBuffer_t *                  psTxMsg,
    const CANMessage_t *                psSrcMsg
)
{
    uint8_t           i;
    uint8_t           j;
    uint8_t           ucRemainLength;
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( ( psTxMsg != NULL ) && ( psSrcMsg != NULL ) )
    {
        psTxMsg->rT0.rFReg.rfXTD = psSrcMsg->mExtendedId;

        if( psSrcMsg->mExtendedId == true )
        {
            psTxMsg->rT0.rFReg.rfID = psSrcMsg->mId;
        }
        else
        {
            psTxMsg->rT0.rFReg.rfID = ( volatile uint32_t ) ( ( psSrcMsg->mId & 0x7FFUL ) << 18UL );
        }

        psTxMsg->rT0.rFReg.rfRTR    = psSrcMsg->mRemoteTransmitRequest;
        psTxMsg->rT0.rFReg.rfESI    = psSrcMsg->mErrorStateIndicator;
        psTxMsg->rT1.rFReg.rfBRS    = psSrcMsg->mBitRateSwitching;
        psTxMsg->rT1.rFReg.rfEFC    = psSrcMsg->mEventFIFOControl;
        psTxMsg->rT1.rFReg.rfFDF    = psSrcMsg->mFDFormat;
        psTxMsg->rT1.rFReg.rfMM     = psSrcMsg->mMessageMarker;
        psTxMsg->rT1.rFReg.rfTSCE   = 1;
        psTxMsg->rT1.rFReg.rfDLC    = (volatile uint32_t)CAN_MsgGetSizeTypebyDataSize(psSrcMsg->mDataLength);

        if( 0U < psSrcMsg->mDataLength )
        {
            for( i = 0U ; i < ( psSrcMsg->mDataLength / 4U ) ; i++ )
            {
                psTxMsg->rData[ i ] = 0;

                for( j = 0U ; j < 4U ; j++)
                {
                    psTxMsg->rData[ i ] |= ( ( uint32_t ) ( psSrcMsg->mData[ j + ( i * 4U ) ] ) << ( 8U * j ) );
                }
            }

            if( 0U < ( psSrcMsg->mDataLength % 4U ) )
            {
                psTxMsg->rData[ i ] = 0UL;

                ucRemainLength = psSrcMsg->mDataLength % 4U;

                for( j = 0U ; j < ucRemainLength ; j++ )
                {
                    psTxMsg->rData[ i ] |= ( ( uint32_t ) ( psSrcMsg->mData[ j + ( i * 4U ) ] ) << ( 8U * j ) );
                }
            }
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_MsgSetTxMessage
(
    CANController_t *                   psControllerInfo,
    const CANMessage_t *                psMsg,
    uint8_t *                             pucTxBufferIndex
)
{
    uint8_t                       ucReqIndex;
    uint32_t                      uiTxMsgAddr;
    CANRamTxBuffer_t *          psTxMsg;
    CANControllerRegister_t *   psControllerReg;
    CANErrorType_t              ret;

    ret = CAN_ERROR_NONE;

    if( ( psControllerInfo != NULL ) && ( psMsg != NULL ) )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            psControllerReg = psControllerInfo->cRegister;

            if( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_FIFO )
            {
                ucReqIndex = ( uint8_t ) psControllerReg->crTxFIFOOrQueueStatus.rFReg.rfTFQPI;
                uiTxMsgAddr = psControllerInfo->cRamAddressInfo.raTxBufferAddr + ( sizeof( CANRamTxBuffer_t ) * ( uint32_t ) ucReqIndex );
                psTxMsg = ( CANRamTxBuffer_t * ) uiTxMsgAddr;

                *pucTxBufferIndex = ucReqIndex;
            }
            else if( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_QUEUE )
            {
                ucReqIndex = psMsg->mBufferIndex + ( uint8_t ) psControllerReg->crTxFIFOOrQueueStatus.rFReg.rfTFQPI; // it seems it doesn't need to add psMsg->mBufferIndex to ucReqIndex.
                uiTxMsgAddr = psControllerInfo->cRamAddressInfo.raTxBufferAddr + ( sizeof( CANRamTxBuffer_t ) * ( uint32_t ) ucReqIndex );
                psTxMsg = ( CANRamTxBuffer_t * ) uiTxMsgAddr;

                *pucTxBufferIndex = ucReqIndex;
            }
            else if( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_DBUFFER )
            {
                ucReqIndex = psMsg->mBufferIndex;
                uiTxMsgAddr = psControllerInfo->cRamAddressInfo.raTxBufferAddr + ( sizeof( CANRamTxBuffer_t ) * ( uint32_t ) ucReqIndex );
                psTxMsg = ( CANRamTxBuffer_t * ) uiTxMsgAddr;

                *pucTxBufferIndex = ucReqIndex;
            }
            else
            {
                /* unknown buffer type */
                ret = CAN_ERROR_NO_MESSAGE;
            }

            if( ( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_FIFO ) || ( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_QUEUE ) || ( psMsg->mBufferType == CAN_TX_BUFFER_TYPE_DBUFFER ) )
            {
                ( void ) CAN_MsgCopyTxMessage( psTxMsg, psMsg );

                if( psControllerInfo->cTxBufferInfo->tbTxFIFOorQueueMode != CAN_TX_BUFFER_MODE_QUEUE ) {
                    ret = CAN_MsgRequestTxMessage( psControllerInfo, ucReqIndex );
                } else {
                    ret = CAN_ERROR_NONE;
                }
            }
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_MsgRequestTxMessage
(
    CANController_t *                   psControllerInfo,
    uint8_t                               ucReqIndex
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            psControllerInfo->cRegister->crTxBufferAddRequest |= ( uint32_t ) ( ( uint32_t ) 1UL << ucReqIndex );
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

/* Not used function */
CANErrorType_t CAN_MsgRequestTxAllMessage
(
    CANController_t *                   psControllerInfo,
    uint32_t                              uiTxBufferAllIndex
)
{
    CANErrorType_t ret;
    //uint32_t test_val, test_pending;
    //uint32_t test_val1, test_pending1;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            psControllerInfo->cRegister->crTxBufferAddRequest |= uiTxBufferAllIndex;
	    //psControllerInfo->cRegister->crTxBufferAddRequest |= 0xB000F;
	    //test_val = psControllerInfo->cRegister->crTxFIFOOrQueueStatus.rNReg;
	    //test_pending = psControllerInfo->cRegister->crTxBufferRequestPending.rNReg;
	    //psControllerInfo->cRegister->crTxBufferAddRequest |= 0x40000;
	    //test_val1 = psControllerInfo->cRegister->crTxFIFOOrQueueStatus.rNReg;
	    //test_pending1 = psControllerInfo->cRegister->crTxBufferRequestPending.rNReg;
	    //printf("Addreq : 0x%x TXFQS : 0x%x TXBRP:0x%x\n", 0xB000F, test_val, test_pending);
	    //printf("Addreq1 : 0x%x TXFQS1 : 0x%x TXBRP1:0x%x\n", 0x40000, test_val1, test_pending1);

        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

CANErrorType_t CAN_MsgRequestTxMessageCancellation
(
    CANController_t *                   psControllerInfo,
    uint8_t                               ucTxBufferIndex
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            psControllerInfo->cRegister->crTxBufferCancellationRequest |= ( uint32_t ) ( ( uint32_t ) 1UL << ucTxBufferIndex );
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

uint32_t CAN_MsgGetCountOfTxEvent
(
    uint8_t                               ucCh
)
{
    CANController_t *   psControllerInfo;
    uint32_t              ret;

    ret = 0;

    psControllerInfo = CAN_GetControllerInfo( ucCh );

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            ret = psControllerInfo->cRegister->crTxEventFIFOStatus.rFReg.rfEFFL;
        }
    }

    return ret;
}

CANErrorType_t CAN_MsgGetTxEventMessage
(
    uint8_t                               ucCh,
    CANTxEvent_t *                      psTxEvtBuffer
)
{
    uint32_t                  uiIndex;
    uint32_t                  uiTxEvtMsgAddr;
    CANController_t *       psControllerInfo;
    CANRamTxEventFIFO_t *   psSavedTxEvt;
    CANErrorType_t          ret;

    uiIndex                 = 0;
    psControllerInfo        = NULL;
    ret                     = CAN_ERROR_NONE;

    if( psTxEvtBuffer != NULL )
    {
        psControllerInfo = CAN_GetControllerInfo( ucCh );

        if( ( psControllerInfo != NULL ) && ( psControllerInfo->cRegister != NULL ) )
        {
            if( psControllerInfo->cRegister->crTxEventFIFOStatus.rFReg.rfEFFL != 0UL )
            {
                uiIndex = psControllerInfo->cRegister->crTxEventFIFOStatus.rFReg.rfEFGI;
                uiTxEvtMsgAddr = psControllerInfo->cRamAddressInfo.raTxEventFIFOAddr + ( sizeof( CANRamTxEventFIFO_t ) * uiIndex );
                psSavedTxEvt = ( CANRamTxEventFIFO_t * ) uiTxEvtMsgAddr;

                psTxEvtBuffer->teExtendedId = ( uint8_t ) psSavedTxEvt->rE0.rFReg.rfXTD;

                if( psTxEvtBuffer->teExtendedId == 0U )
                {
                    psTxEvtBuffer->teId = ( uint32_t ) ( ( ( uint32_t ) ( psSavedTxEvt->rE0.rFReg.rfID ) >> 18UL ) & 0x7FFUL );
                }
                else
                {
                    psTxEvtBuffer->teId = ( uint32_t ) ( psSavedTxEvt->rE0.rFReg.rfID );
                }

                psTxEvtBuffer->teRemoteTransmitRequest = ( uint8_t ) psSavedTxEvt->rE0.rFReg.rfRTR;
                psTxEvtBuffer->teFDFormat = ( uint8_t ) psSavedTxEvt->rE1.rFReg.rfFDF;
                psTxEvtBuffer->teBitRateSwitching = ( uint8_t ) psSavedTxEvt->rE1.rFReg.rfBRS;
                psTxEvtBuffer->teDataLength = CAN_MsgGetDataSizeBySizeType( ( CANDataLength_t ) psSavedTxEvt->rE1.rFReg.rfDLC );

                psControllerInfo->cRegister->crTxEventFIFOAcknowledge.rFReg.rfEFAI = uiIndex;
            }
            else
            {
                ret = CAN_ERROR_NO_MESSAGE;
            }
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

