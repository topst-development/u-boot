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
#include <test/can_drv.h>
#include <test/can_msg.h>
#include <test/can_porting.h>
#include <test/can_par.h>


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

CANDriver_t CANDrvierInfo;



/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/



/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

CANErrorType_t CAN_Init
(
    void
)
{
    uint8_t               ucCh;
    CANController_t *   psControllerInfo;
    CANErrorType_t      ret;

    psControllerInfo    = NULL;
    ret                 = CAN_ERROR_NONE;

    for( ucCh = 0 ; ucCh < CAN_CONTROLLER_NUMBER ; ucCh++ )
    {
        psControllerInfo = &CANDrvierInfo.dControllerInfo[ ucCh ];
        psControllerInfo->cChannelHandle = ucCh;

        ( void ) CAN_PortingDisableControllerInterrupts( psControllerInfo->cChannelHandle );

        ret = CAN_DrvInitChannel( psControllerInfo );

        if( ret != CAN_ERROR_NONE )
        {
            CAN_D( "[CAN]CAN_Init Failed \r\n", psControllerInfo->cChannelHandle );
            break;
        }

        ( void ) CAN_PortingEnableControllerInterrupts( psControllerInfo->cChannelHandle );
    }

    if( ret == CAN_ERROR_NONE )
    {
        CAN_D( "[CAN]CAN_Init Completed \r\n", psControllerInfo->cChannelHandle );
    }

    return ret;
}

CANErrorType_t CAN_Deinit
(
    void
)
{
    uint8_t               ucCh;
    CANController_t *   psControllerInfo;
    CANErrorType_t      ret;

    psControllerInfo    = NULL;
    ret                 = CAN_ERROR_NONE;

    for( ucCh = 0 ; ucCh < CAN_CONTROLLER_NUMBER ; ucCh++ )
    {
        psControllerInfo = &CANDrvierInfo.dControllerInfo[ ucCh ];
        psControllerInfo->cChannelHandle = ucCh;

        ( void ) CAN_PortingDisableControllerInterrupts( psControllerInfo->cChannelHandle );

        ( void ) CAN_PortingResetDriver( psControllerInfo );

        ret = CAN_DrvDeinitChannel( psControllerInfo );

        if( ret != CAN_ERROR_NONE )
        {
            CAN_D( "[CAN]CAN_Deinit Failed \r\n", psControllerInfo->cChannelHandle );
            break;
        }
    }

    return ret;
}

CANErrorType_t CAN_InitMessage
(
    uint8_t                               ucCh
)
{
    CANController_t *   psControllerInfo;
    CANErrorType_t      ret;

    psControllerInfo    = NULL;
    ret                 = CAN_ERROR_NONE;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = &CANDrvierInfo.dControllerInfo[ ucCh ];
        ( void ) CAN_MsgInit( psControllerInfo );
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

uint32_t CAN_CheckNewRxMessage
(
    uint8_t                               ucCh
)
{
    uint32_t  ret;

    ret = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = CAN_MsgGetCountOfRxMessage( ucCh );
    }

    return ret;
}

CANErrorType_t CAN_GetNewRxMessage
(
    uint8_t                               ucCh,
    CANMessage_t *                      psMsg
)
{
    CANController_t *   psControllerInfo;
    CANErrorType_t      ret;

    psControllerInfo    = NULL;
    ret                 = CAN_ERROR_NONE;

    if( ( ucCh < CAN_CONTROLLER_NUMBER ) && ( psMsg != NULL ) )
    {
        psControllerInfo = &CANDrvierInfo.dControllerInfo[ ucCh ];

        if( ( psControllerInfo->cMode == CAN_MODE_OPERATION )
         || ( psControllerInfo->cMode == CAN_MODE_MONITORING )
         || ( psControllerInfo->cMode == CAN_MODE_INTERNAL_TEST )
         || ( psControllerInfo->cMode == CAN_MODE_EXTERNAL_TEST ) )
        {
            ret = CAN_MsgGetRxMessage( ucCh, psMsg );
        }
        else
        {
            ret = CAN_ERROR_CONTROLLER_MODE;
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_SendMessage
(
    uint8_t                               ucCh,
    const CANMessage_t *                psMsg,
    uint8_t *                             pucTxBufferIndex
)
{
    CANController_t *   psControllerInfo;
    CANErrorType_t      ret;

    psControllerInfo    = NULL;
    ret                 = CAN_ERROR_NONE;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = &CANDrvierInfo.dControllerInfo[ ucCh ];

        if( psMsg != NULL )
        {
            if( ( psControllerInfo->cMode == CAN_MODE_OPERATION )
             || ( psControllerInfo->cMode == CAN_MODE_MONITORING )
             || ( psControllerInfo->cMode == CAN_MODE_INTERNAL_TEST )
             || ( psControllerInfo->cMode == CAN_MODE_EXTERNAL_TEST ) )
            {
                ret = CAN_MsgSetTxMessage( psControllerInfo, psMsg, pucTxBufferIndex );
            }
            else
            {
                ret = CAN_ERROR_CONTROLLER_MODE;
            }
        }
        else
        {
            ret = CAN_ERROR_NOT_INIT;
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_RequestTxMessageCancellation
(
    uint8_t                               ucCh,
    uint8_t                               ucTxIndex
)
{
    CANController_t *   psControllerInfo;
    CANErrorType_t      ret;

    psControllerInfo    = NULL;
    ret                 = CAN_ERROR_NONE;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = &CANDrvierInfo.dControllerInfo[ ucCh ];

        if( psControllerInfo->cMode == CAN_MODE_OPERATION )
        {
            ret = CAN_MsgRequestTxMessageCancellation( psControllerInfo, ucTxIndex );
        }
        else
        {
            ret = CAN_ERROR_CONTROLLER_MODE;
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

uint32_t CAN_CheckNewTxEvent
(
    uint8_t                               ucCh
)
{
    uint32_t  ret;

    ret = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = CAN_MsgGetCountOfTxEvent( ucCh );
    }

    return ret;
}

CANErrorType_t CAN_GetNewTxEvent
(
    uint8_t                               ucCh,
    CANTxEvent_t *                      psTxEvent
)
{
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = CAN_MsgGetTxEventMessage( ucCh, psTxEvent );
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_SetControllerMode
(
    uint8_t                               ucCh,
    CANMode_t                           ucControllerMode
)
{
    uint8_t               ucChannel;
    CANController_t *   psControllerInfo;
    CANErrorType_t      ret;

    psControllerInfo    = NULL;
    ret                 = CAN_ERROR_NONE;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = &CANDrvierInfo.dControllerInfo[ ucCh ];

        switch( ucControllerMode )
        {
            case CAN_MODE_NO_INITIALIZATION:
            {
                /* nothing */

                break;
            }

            case CAN_MODE_OPERATION:
            {
                ret = CAN_DrvSetNormalOperationMode( psControllerInfo );

                break;
            }

            case CAN_MODE_SLEEP:
            {
                for( ucChannel = 0 ; ucChannel < CAN_CONTROLLER_NUMBER ; ucChannel++ )
                {
                    psControllerInfo = &CANDrvierInfo.dControllerInfo[ ucChannel ];
                    psControllerInfo->cChannelHandle = ucChannel;

                    ( void ) CAN_PortingDisableControllerInterrupts( psControllerInfo->cChannelHandle );

                    ret = CAN_DrvSetSleepMode( psControllerInfo );
                }

                break;
            }

            case CAN_MODE_WAKE_UP:
            {
                for( ucChannel = 0 ; ucChannel < CAN_CONTROLLER_NUMBER ; ucChannel++ )
                {
                    psControllerInfo = &CANDrvierInfo.dControllerInfo[ ucChannel ];
                    psControllerInfo->cChannelHandle = ucChannel;

                    ( void ) CAN_DrvSetWakeUpMode( psControllerInfo );
                    ( void ) CAN_PortingEnableControllerInterrupts( psControllerInfo->cChannelHandle );

                    psControllerInfo->cMode = CAN_MODE_OPERATION;
                }

                break;
            }

            case CAN_MODE_MONITORING:
            {
                ret = CAN_DrvSetMonitoringMode( psControllerInfo );

                break;
            }

            case CAN_MODE_RESET_CONTROLLER:
            {
                ( void ) CAN_Deinit();
                ( void ) CAN_Init();

                psControllerInfo->cMode = CAN_MODE_OPERATION;

                break;
            }

            case CAN_MODE_INTERNAL_TEST:
            {
                ret = CAN_DrvSetInternalTestMode( psControllerInfo );

                break;
            }

            case CAN_MODE_EXTERNAL_TEST:
            {
                ret = CAN_DrvSetExternalTestMode( psControllerInfo );

                break;
            }

            default:
            {
                ret = CAN_ERROR_BAD_PARAM;

                break;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANMode_t CAN_GetControllerMode
(
    uint8_t                               ucCh
)
{
    CANController_t *   psControllerInfo;
    CANMode_t           ret;

    psControllerInfo    = NULL;
    ret                 = CAN_MODE_NO_INITIALIZATION;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = &CANDrvierInfo.dControllerInfo[ ucCh ];

        ret = psControllerInfo->cMode;
    }

    return ret;
}

uint32_t CAN_GetProtocolStatus
(
    uint8_t                               ucCh
)
{
    CANController_t *   psControllerInfo;
    uint32_t              ret;

    psControllerInfo    = NULL;
    ret                 = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = &CANDrvierInfo.dControllerInfo[ ucCh ];

        ret = CAN_DrvGetProtocolStatus( psControllerInfo );
    }

    return ret;
}

void *CAN_GetControllerInfo
(
    uint8_t                               ucCh
)
{
    CANController_t *   psControllerInfo;

    psControllerInfo    = NULL;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = &CANDrvierInfo.dControllerInfo[ ucCh ];
    }

    return ( void * ) psControllerInfo;
}

CANErrorType_t CAN_RegisterCallbackFunctionTx
(
    CANNotifyTxEventCB                  pCbFnTx
)
{
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( pCbFnTx != NULL )
    {
        CANCallbackFunctions.cbNotifyTxEvent = pCbFnTx;
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_RegisterCallbackFunctionRx
(
    CANNotifyRxEventCB                  pCbFnRx
)
{
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( pCbFnRx != NULL )
    {
        CANCallbackFunctions.cbNotifyRxEvent = pCbFnRx;
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_RegisterCallbackFunctionError
(
    CANNotifyErrorEventCB               pCbFnError
)
{
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( pCbFnError != NULL )
    {
        CANCallbackFunctions.cbNotifyErrorEvent = pCbFnError;
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

