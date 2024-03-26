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
#include <test/can_porting.h>

#include <cpu_func.h>
#include <stdio.h>

/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

static CANErrorType_t CAN_DrvSetTimeStamp
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvSetTimeoutValue
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvSetWatchDog
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvSetControlConfig
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvCopyFilterList
(
    CANController_t *                   psControllerInfo,
    CANIdType_t                         uiFilterType
);

static CANErrorType_t CAN_DrvSetFilterConfig
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvSetBitrate
(
    CANController_t *                   psControllerInfo,
    CANBitRateTiming_t                  ucTimingType
);

static CANErrorType_t CAN_DrvInitTiming
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvStartConfigSetting
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvFinishConfigSetting
(
    CANController_t *                   psControllerInfo
);

static uint32_t CAN_DrvGetSizeOfRamMemory
(
    const CANController_t *             psControllerInfo
);

static CANErrorType_t CAN_DrvRegisterParameterAll
(
    CANController_t *                   psControllerInfo
);

static CANErrorType_t CAN_DrvInitBuffer
(
    CANController_t *                   psControllerInfo,
    const uint32_t                        uiMemory,
    uint32_t                              uiMemorySize
);

static CANErrorType_t CAN_DrvDeinitBuffer
(
    CANController_t *                   psControllerInfo
);

static void CAN_DrvIRQHandler
(
    void *pArg
);

static CANErrorType_t CAN_DrvSetInterruptConfig
(
    CANController_t *                   psControllerInfo
);

#if 1
static void CANTEST_ProcessIRQ
(
    uint8_t                               ucCh
);
#else
static void CAN_DrvProcessIRQ
(
    uint8_t                               ucCh
);
#endif

static void CAN_DrvCallbackNotifyTxEvent
(
    uint8_t                               ucCh,
    CANTxInterruptType_t                uiIntType
);

static void CAN_DrvCallbackNotifyErrorEvent
(
    uint8_t                               ucCh,
    CANErrorType_t                      uiError
);

/**************************************************************************************************
*                                             FUNCTIONS
**************************************************************************************************/

CANErrorType_t CAN_DrvInitChannel
(
    CANController_t *                   psControllerInfo
)
{
    uint8_t               ucCh;
    uint32_t              uiMemSize;
    uint32_t              uiMemAddr;
    CANRegBaseAddr_t *  psConfigBaseRegAddr;
    CANErrorType_t      ret;

    uiMemAddr           = 0UL;
    ret                 = CAN_ERROR_NONE;

    if( ( psControllerInfo != NULL ) && ( psControllerInfo->cMode == CAN_MODE_NO_INITIALIZATION ) )
    {
        /* Set Config */
        ucCh = psControllerInfo->cChannelHandle;
        psControllerInfo->cRegister = ( CANControllerRegister_t * ) CAN_PortingGetControllerRegister( ucCh );

        /* Set HW_Init */
        ( void ) CAN_PortingInitHW( psControllerInfo );
        ( void ) CAN_PortingSetControllerClock( psControllerInfo, CAN_CONTROLLER_CLOCK );
        ( void ) CAN_PortingResetDriver( psControllerInfo );

        /* Set Memory */
        uiMemSize = CAN_DrvGetSizeOfRamMemory( psControllerInfo );

        if( 0UL < uiMemSize )
        {
	    dcache_disable();
            uiMemAddr = CAN_PortingAllocateNonCacheMemory( ucCh, uiMemSize );
        }

        if( uiMemAddr != 0UL )
        {
            psConfigBaseRegAddr = ( CANRegBaseAddr_t * ) CAN_PortingGetMessageRamBaseAddr( ucCh );

            if( psConfigBaseRegAddr != NULL )
            {
                psConfigBaseRegAddr->rFReg.rfBASE_ADDR = ( volatile uint32_t ) ( ( uint32_t ) ( uiMemAddr >> ( uint32_t ) 16UL ) & ( uint32_t ) 0xFFFFUL );
            }

	    //prepare CAN configuration data : timing parameter, Tx/Rx storage setting info
            ( void ) CAN_DrvRegisterParameterAll( psControllerInfo );

	    //init enable
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );
	    //FD enable, bit rate switching enable
            ( void ) CAN_DrvSetControlConfig( psControllerInfo );

            /* Set Buffer */
            ( void ) memset( ( void * ) uiMemAddr, 0, uiMemSize );
            ( void ) CAN_DrvInitBuffer( psControllerInfo, uiMemAddr, uiMemSize );
            ( void ) CAN_MsgInit( psControllerInfo );

            /* Set Filter */
            ( void ) CAN_DrvSetFilterConfig( psControllerInfo );

            /* Set Timing */
            ( void ) CAN_DrvInitTiming( psControllerInfo );

            /* Set Interrupt */
            ( void ) CAN_DrvSetInterruptConfig( psControllerInfo );

            /* Set TimeStamp */
            ( void ) CAN_DrvSetTimeStamp( psControllerInfo );

            /* Set TimeOut */
            ( void ) CAN_DrvSetTimeoutValue( psControllerInfo );

            /* Set WatchDog */
            ( void ) CAN_DrvSetWatchDog( psControllerInfo );

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_OPERATION;
        }
        else
        {
            ret = CAN_ERROR_ALLOC;
        }
    }
    else
    {
        ret = CAN_ERROR_BAD_PARAM;
    }

    return ret;
}

CANErrorType_t CAN_DrvDeinitChannel
(
    CANController_t *                   psControllerInfo
)
{
    uint8_t               ucCh;
    CANRegBaseAddr_t *  psConfigBaseRegAddr;
    CANErrorType_t      ret;

    psConfigBaseRegAddr = NULL;
    ret                 = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        /* Set Config */
        ucCh = psControllerInfo->cChannelHandle;

        /* Set HW_Init */
        ret = CAN_DrvStartConfigSetting( psControllerInfo );

        /* Set Buffer */
        if( ret == CAN_ERROR_NONE )
        {
            ( void ) CAN_MsgInit( psControllerInfo );
        }

        /* Free Memory */
        if( psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr != 0UL )
        {
            CAN_PortingDeallocateNonCacheMemory( ucCh, ( uint32_t * ) &( psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr ) );
        }

        ( void ) CAN_DrvDeinitBuffer( psControllerInfo );

        /* Set Message RAM base address */
        psConfigBaseRegAddr = ( CANRegBaseAddr_t * ) CAN_PortingGetMessageRamBaseAddr( ucCh );

        if( psConfigBaseRegAddr != NULL )
        {
            psConfigBaseRegAddr->rFReg.rfBASE_ADDR = 0;
        }

        psControllerInfo->cMode = CAN_MODE_NO_INITIALIZATION;
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

/* Config */
CANErrorType_t CAN_DrvSetNormalOperationMode
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            if( psControllerInfo->cMode == CAN_MODE_MONITORING )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfMON = false;
            }
            else if( psControllerInfo->cMode == CAN_MODE_INTERNAL_TEST )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfTEST = false;
                psControllerInfo->cRegister->crTest.rFReg.rfLBCK = false;
                psControllerInfo->cRegister->crCCControl.rFReg.rfMON = false;
            }
            else if( psControllerInfo->cMode == CAN_MODE_EXTERNAL_TEST )
            {
                psControllerInfo->cRegister->crTest.rFReg.rfLBCK = false;
                psControllerInfo->cRegister->crCCControl.rFReg.rfTEST = false;
            }
            else
            {
                ; /* Nothing to do */
            }

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_OPERATION;

            CAN_D( "[CAN][Ch:%d] Set OPERATION Mode \r\n", psControllerInfo->cChannelHandle );
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

CANErrorType_t CAN_DrvSetMonitoringMode
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            psControllerInfo->cRegister->crCCControl.rFReg.rfMON = true;

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_MONITORING;

            CAN_D( "[CAN][Ch:%d] Set MONITORING Mode \r\n", psControllerInfo->cChannelHandle );
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

CANErrorType_t CAN_DrvSetInternalTestMode
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            psControllerInfo->cRegister->crCCControl.rFReg.rfTEST = true;
            psControllerInfo->cRegister->crTest.rFReg.rfLBCK = true;
            psControllerInfo->cRegister->crCCControl.rFReg.rfMON = true;

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_INTERNAL_TEST;

            printf( "[CAN][Ch:%d] Set INTERNAL_TEST Mode \r\n", psControllerInfo->cChannelHandle );
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

CANErrorType_t CAN_DrvSetExternalTestMode
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            psControllerInfo->cRegister->crCCControl.rFReg.rfTEST = true;
            psControllerInfo->cRegister->crTest.rFReg.rfLBCK = true;

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

            psControllerInfo->cMode = CAN_MODE_EXTERNAL_TEST;

            printf( "[CAN][Ch:%d] Set External_TEST Mode \r\n", psControllerInfo->cChannelHandle );
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

CANErrorType_t CAN_DrvSetWakeUpMode
(
    CANController_t *                   psControllerInfo
)
{
    int32_t          iTimeOutValue;
    CANErrorType_t  ret;

    iTimeOutValue   = 400;
    ret             = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            if( psControllerInfo->cMode == CAN_MODE_NO_INITIALIZATION )
            {
                ret = CAN_Init();
            }
            else if( psControllerInfo->cMode == CAN_MODE_SLEEP )
            {
                ( void ) CAN_PortingSetControllerClock( psControllerInfo, CAN_CONTROLLER_CLOCK );
                ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

                psControllerInfo->cRegister->crCCControl.rFReg.rfCSR = false;

                ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );

                while( psControllerInfo->cRegister->crCCControl.rFReg.rfCSA == true )
                {
                    CAN_PortingDelay( 1 );

                    if( iTimeOutValue == 0 )
                    {
                        ret = CAN_ERROR_TIMEOUT;

                        break;
                    }

                    iTimeOutValue--;
                }
            }
            else
            {
                ; /* Nothing to do */
            }

            psControllerInfo->cMode = CAN_MODE_WAKE_UP;
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

CANErrorType_t CAN_DrvSetSleepMode
(
    CANController_t *                   psControllerInfo
)
{
    int32_t          iTimeOutValue;
    CANErrorType_t  ret;

    iTimeOutValue   = 200;
    ret             = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            if( psControllerInfo->cMode == CAN_MODE_OPERATION )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfCSR = true;

                while( psControllerInfo->cRegister->crCCControl.rFReg.rfCSA == false )
                {
                    CAN_PortingDelay( 1 );

                    if( iTimeOutValue == 0 )
                    {
                        break;
                    }

                    iTimeOutValue--;
                }
            }
            else
            {
                ; /* Nothing to do */
            }

            psControllerInfo->cMode = CAN_MODE_SLEEP;
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

static CANErrorType_t CAN_DrvTSUEnableSetting
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            psControllerInfo->cRegister->crCCControl.rFReg.rfUTSU = 1;
            psControllerInfo->cRegister->crCCControl.rFReg.rfWMM = 1;
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

static CANErrorType_t CAN_DrvSetTimeStamp
(
    CANController_t *                   psControllerInfo
)
{
    CANRegExtTSCtrl0_t *    psExtTSCtrl0;
    CANRegExtTSCtrl1_t *    psEXTTSCtrl1;
	CANRegTSCFG_t *			psTSCFG;
    CANErrorType_t          ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( CAN_PortingGetTimeStampEnable( psControllerInfo->cChannelHandle ) == true )
        {
            if( psControllerInfo->cRegister != NULL )
            {
                psControllerInfo->cRegister->crTimestampCounterConfiguration.rFReg.rfTSS = 0x2;
                psExtTSCtrl0 = ( CANRegExtTSCtrl0_t * ) CAN_PortingGetConfigEXTS0Addr( psControllerInfo->cChannelHandle );
                psExtTSCtrl0->rFReg.rfENABLE = true;

                psEXTTSCtrl1 = ( CANRegExtTSCtrl1_t * ) CAN_PortingGetConfigEXTS1Addr( psControllerInfo->cChannelHandle );
                psEXTTSCtrl1->rFReg.rfEXT_TIMESTAMP_DIV_RATIO = CAN_TIMESTAMP_RATIO;
                psEXTTSCtrl1->rFReg.rfEXT_TIMESTAMP_COMP = CAN_TIMESTAMP_COMP;

				psTSCFG = ( CANRegTSCFG_t * ) CAN_PortingGetTSCFGAddr( psControllerInfo->cChannelHandle );
				psTSCFG->rFReg.rfTSUE = 1;
				//psTSCFG->rFReg.rfTBCS = 1;
				//psTSCFG->rFReg.rfSCP = 1;
				( void ) CAN_DrvTSUEnableSetting( psControllerInfo );

                CAN_D( "[CAN]Set External TimeStamp \r\n", psControllerInfo->cChannelHandle );

            }
            else
            {
                ret = CAN_ERROR_NOT_INIT;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvSetTimeoutValue
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( CAN_PortingGetTimeOutEnable( psControllerInfo->cChannelHandle ) == true )
        {
            if( psControllerInfo->cRegister != NULL )
            {
                if( psControllerInfo->cRegister->crTimestampCounterConfiguration.rFReg.rfTCP == 0UL )
                {
                    psControllerInfo->cRegister->crTimestampCounterConfiguration.rFReg.rfTCP = CAN_TIMESTAMP_PRESCALER;
                }

                psControllerInfo->cRegister->crTimeoutCounterConfiguration.rFReg.rfTOP = CAN_TIMEOUT_VALUE;
                psControllerInfo->cRegister->crTimeoutCounterConfiguration.rFReg.rfTOS = CAN_TIMEOUT_TYPE;
                psControllerInfo->cRegister->crTimeoutCounterConfiguration.rFReg.rfETOC = true;
            }
            else
            {
                ret = CAN_ERROR_NOT_INIT;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvSetWatchDog
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( CAN_PortingGetWatchDogEnable( psControllerInfo->cChannelHandle ) == true )
        {
            if( psControllerInfo->cRegister != NULL )
            {
                psControllerInfo->cRegister->crRAMWatchdog.rFReg.rfWDC = CAN_WATCHDOG_VALUE;
            }
            else
            {
                ret = CAN_ERROR_NOT_INIT;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvSetControlConfig
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            if( CAN_PortingGetFDEnable( psControllerInfo->cChannelHandle ) == true )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfFDOE = true;
            }
            if( CAN_PortingGetBitRateSwitchEnable( psControllerInfo->cChannelHandle ) == true )
            {
                psControllerInfo->cRegister->crCCControl.rFReg.rfBRSE = true;
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

static CANErrorType_t CAN_DrvCopyFilterList
(
    CANController_t *                   psControllerInfo,
    CANIdType_t                         uiFilterType
)
{
    uint8_t                           ucCh;
    uint8_t                           ucNumOfFilter;
    uint32_t                          uiTempAddr;
    CANIdFilterList_t *             psFilterList;
    CANRamStdIDFilterElement_t *    psStdFilterElement;
    CANRamExtIDFilterElement_t *    psExtFilterElement;
    CANErrorType_t                  ret;

    ucCh            = 0;
    uiTempAddr      = 0;
    psFilterList    = NULL;
    ret             = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        ucCh = psControllerInfo->cChannelHandle;

        if( psControllerInfo->cRegister != NULL )
        {
            if( uiFilterType == CAN_ID_TYPE_STANDARD )
            {
                if( psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr != 0UL )
                {
                    psStdFilterElement = ( CANRamStdIDFilterElement_t * ) psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr;
                    psFilterList = ( CANIdFilterList_t * ) StandardIDFilterPar.idfList[ ucCh ];

                    if( psFilterList != NULL )
                    {
                        for( ucNumOfFilter = 0U ; ucNumOfFilter < CAN_STANDARD_ID_FILTER_NUMBER ; ucNumOfFilter++ )
                        {
                            psStdFilterElement->rFReg.rfSFT      = ( volatile uint32_t ) ( ( uint32_t ) ( psFilterList->idflFilterType ) & 0x3UL );
                            //psStdFilterElement->rFReg.rfSSYNC    = 1;
                            psStdFilterElement->rFReg.rfSFEC     = ( volatile uint32_t ) ( ( uint32_t ) ( psFilterList->idflFilterElementConfiguration ) & 0x7UL );
                            psStdFilterElement->rFReg.rfSFID1    = ( volatile uint32_t ) ( psFilterList->idflFilterID1 & 0x7FFUL );
                            psStdFilterElement->rFReg.rfSFID2    = ( volatile uint32_t ) ( psFilterList->idflFilterID2 & 0x7FFUL );

                            uiTempAddr          = ( uint32_t ) psStdFilterElement;
                            uiTempAddr          += sizeof( CANRamStdIDFilterElement_t );
                            psStdFilterElement  = ( CANRamStdIDFilterElement_t * ) uiTempAddr;

                            uiTempAddr      = ( uint32_t ) psFilterList;
                            uiTempAddr      += sizeof( CANIdFilterList_t );
                            psFilterList    = ( CANIdFilterList_t * ) uiTempAddr;
                        }

                        psControllerInfo->cNumOfStdIDFilterList = ucNumOfFilter;
                        psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfLSS = ucNumOfFilter;
                    }
                }
                else
                {
                    ret = CAN_ERROR_NOT_INIT;
                }
            }
            else if( uiFilterType == CAN_ID_TYPE_EXTENDED )
            {
                if( psControllerInfo->cRamAddressInfo.raExtendedIDFilterListAddr != 0UL )
                {
                    psExtFilterElement = ( CANRamExtIDFilterElement_t * ) psControllerInfo->cRamAddressInfo.raExtendedIDFilterListAddr;
                    psFilterList = ( CANIdFilterList_t * ) ExtendedIDFilterPar.idfList[ ucCh ];

                    if( psFilterList != NULL )
                    {
                        for( ucNumOfFilter = 0U ; ucNumOfFilter < CAN_EXTENDED_ID_FILTER_NUMBER ; ucNumOfFilter++ )
                        {
                            psExtFilterElement->rF0.rFReg.rfEFEC    = ( volatile uint32_t ) ( ( uint32_t ) ( psFilterList->idflFilterElementConfiguration ) & 0x7UL );
                            psExtFilterElement->rF1.rFReg.rfEFT     = ( volatile uint32_t ) ( ( uint32_t ) ( psFilterList->idflFilterType ) & 0x3UL );
                            psExtFilterElement->rF0.rFReg.rfEFID1   = ( volatile uint32_t ) ( psFilterList->idflFilterID1 & 0x1FFFFFFFUL );
                            psExtFilterElement->rF1.rFReg.rfEFID2   = ( volatile uint32_t ) ( psFilterList->idflFilterID2 & 0x1FFFFFFFUL );

                            uiTempAddr          = ( uint32_t ) psExtFilterElement;
                            uiTempAddr          += sizeof( CANRamExtIDFilterElement_t );
                            psExtFilterElement  = ( CANRamExtIDFilterElement_t * ) uiTempAddr;

                            uiTempAddr          = ( uint32_t ) psFilterList;
                            uiTempAddr          += sizeof( CANIdFilterList_t );
                            psFilterList        = ( CANIdFilterList_t * ) uiTempAddr;
                        }

                        psControllerInfo->cNumOfExtIDFilterList = ucNumOfFilter;
                        psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfLSE = ucNumOfFilter;
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

static CANErrorType_t CAN_DrvSetFilterConfig
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            if( CAN_PortingGetStandardIDFilterEnable( psControllerInfo->cChannelHandle ) == true )
            {
                ret = CAN_DrvCopyFilterList( psControllerInfo, CAN_ID_TYPE_STANDARD );
            }

            if( ret == CAN_ERROR_NONE )
            {
                if( CAN_PortingGetExtendedIDFilterEnable( psControllerInfo->cChannelHandle ) == true )
                {
                    ret = CAN_DrvCopyFilterList( psControllerInfo, CAN_ID_TYPE_EXTENDED );
                }
            }

            psControllerInfo->cRegister->crGlobalFilterConfiguration.rFReg.rfRRFS = CAN_PortingGetStandardIDRemoteRejectEnable( psControllerInfo->cChannelHandle );
            psControllerInfo->cRegister->crGlobalFilterConfiguration.rFReg.rfRRFE = CAN_PortingGetExtendedIDRemoteRejectEnable( psControllerInfo->cChannelHandle );
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

/* Timing */
static CANErrorType_t CAN_DrvSetBitrate
(
    CANController_t *                   psControllerInfo,
    CANBitRateTiming_t                  ucTimingType
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( ucTimingType == CAN_BIT_RATE_TIMING_ARBITRATION )
        {
            psControllerInfo->cRegister->crNominalBitTiming.rFReg.rfNSJW    = ( volatile uint32_t ) ( psControllerInfo->cArbiPhaseTimeInfo->tpSJW ) - 1UL;
            psControllerInfo->cRegister->crNominalBitTiming.rFReg.rfNBRP    = ( volatile uint32_t ) ( psControllerInfo->cArbiPhaseTimeInfo->tpBRP ) - 1UL;
            psControllerInfo->cRegister->crNominalBitTiming.rFReg.rfNTSEG1  = ( volatile uint32_t ) ( psControllerInfo->cArbiPhaseTimeInfo->tpPROP ) + ( volatile uint32_t ) ( psControllerInfo->cArbiPhaseTimeInfo->tpPhaseSeg1 ) - 1UL;
            psControllerInfo->cRegister->crNominalBitTiming.rFReg.rfNTSEG2  = ( volatile uint32_t ) ( psControllerInfo->cArbiPhaseTimeInfo->tpPhaseSeg2 ) - 1UL;
        }
        else if( ucTimingType == CAN_BIT_RATE_TIMING_DATA )
        {
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfDSJW   = ( volatile uint32_t ) ( psControllerInfo->cDataPhaseTimeInfo->tpSJW ) - 1UL;
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfDBRP   = ( volatile uint32_t ) ( psControllerInfo->cDataPhaseTimeInfo->tpBRP ) - 1UL;
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfDTSEG1 = ( volatile uint32_t ) ( psControllerInfo->cDataPhaseTimeInfo->tpPROP ) + ( volatile uint32_t ) ( psControllerInfo->cDataPhaseTimeInfo->tpPhaseSeg1 ) - 1UL;
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfDTSEG2 = ( volatile uint32_t ) ( psControllerInfo->cDataPhaseTimeInfo->tpPhaseSeg2 ) - 1UL;
            psControllerInfo->cRegister->crDataBitTiming.rFReg.rfTDC    = ( volatile uint32_t ) ( psControllerInfo->cDataPhaseTimeInfo->tpTDC );
            psControllerInfo->cRegister->crTransmitterDelayCompensation.rFReg.rfTDCO = ( volatile uint32_t ) ( psControllerInfo->cDataPhaseTimeInfo->tpTDCOffset );
            psControllerInfo->cRegister->crTransmitterDelayCompensation.rFReg.rfTDCF = ( volatile uint32_t ) ( psControllerInfo->cDataPhaseTimeInfo->tpTDCFilterWindow );

            psControllerInfo->cRegister->crCCControl.rFReg.rfBRSE = true;
        }
        else
        {
            ret = CAN_ERROR_BAD_PARAM;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvInitTiming
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cArbiPhaseTimeInfo != NULL )
        {
            /* Arbitration Phase Bit Timing */
            ( void ) CAN_DrvSetBitrate( psControllerInfo, CAN_BIT_RATE_TIMING_ARBITRATION );

            /* Data Phase Bit Timing */
            if( CAN_PortingGetBitRateSwitchEnable( psControllerInfo->cChannelHandle ) == true )
            {
                if( psControllerInfo->cDataPhaseTimeInfo != NULL )
                {
                    ( void ) CAN_DrvSetBitrate( psControllerInfo, CAN_BIT_RATE_TIMING_DATA );
                }
            }
        }
        else
        {
            ret = CAN_ERROR_BAD_PARAM;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvStartConfigSetting
(
    CANController_t *                   psControllerInfo
)
{
    uint32_t          uiCnt;
    CANErrorType_t  ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            psControllerInfo->cRegister->crCCControl.rFReg.rfINIT = 1;

            for( uiCnt = 0UL ; uiCnt < 0x100UL ; uiCnt++ ) {
                ;
            }

            psControllerInfo->cRegister->crCCControl.rFReg.rfCCE = 1;
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

static CANErrorType_t CAN_DrvFinishConfigSetting
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            psControllerInfo->cRegister->crCCControl.rFReg.rfCCE = 0;
            psControllerInfo->cRegister->crCCControl.rFReg.rfINIT = 0;
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

static uint32_t CAN_DrvGetSizeOfRamMemory
(
    const CANController_t *             psControllerInfo
)
{
    uint32_t uiStdIDFilterListSize;
    uint32_t uiExtIDFilterListSize;
    uint32_t uiRxFIFO0Size;
    uint32_t uiRxFIFO1Size;
    uint32_t uiRxBufferSize;
    uint32_t uiTxEvtFIFOSize;
    uint32_t uiTxBufferSize;
    uint32_t ret;

    ret = 0;

    if( psControllerInfo != NULL )
    {
        /* StandardIDFilterListSize */
        uiStdIDFilterListSize = CAN_PortingGetSizeofStandardIDFilterList( psControllerInfo->cChannelHandle );

        /* ExtendedIDFilterListSize */
        uiExtIDFilterListSize = CAN_PortingGetSizeofExtendedIDFilterList( psControllerInfo->cChannelHandle );

        /* RxFIFO0Size */
        uiRxFIFO0Size = CAN_PortingGetSizeofRxFIFO0( psControllerInfo->cChannelHandle );

        /* RxFIFO1Size */
        uiRxFIFO1Size = CAN_PortingGetSizeofRxFIFO1( psControllerInfo->cChannelHandle );

        /* RxBufferSize */
        uiRxBufferSize = CAN_PortingGetSizeofRxBuffer( psControllerInfo->cChannelHandle );

        /* TxEventFIFOSize */
        uiTxEvtFIFOSize = CAN_PortingGetSizeofTxEventBuffer( psControllerInfo->cChannelHandle );

        /* TxBufferSize */
        uiTxBufferSize = CAN_PortingGetSizeofTxBuffer( psControllerInfo->cChannelHandle );

        ret = uiStdIDFilterListSize + uiExtIDFilterListSize + uiRxFIFO0Size + uiRxFIFO1Size + uiRxBufferSize + uiTxEvtFIFOSize + uiTxBufferSize;
    }
	printf("[MW] CAN_DrvGetSizeOfRamMemory = %d\n", ret);

    return ret;
}

static CANErrorType_t CAN_DrvRegisterParameterAll
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        /* ArbitrationPhaseTiming */
        psControllerInfo->cArbiPhaseTimeInfo = ( CANTimingParam_t * ) &ArbitrationPhaseTimingPar[ psControllerInfo->cChannelHandle ];

        /* DATAPhaseTiming */
        psControllerInfo->cDataPhaseTimeInfo = ( CANTimingParam_t * ) &DataPhaseTimingPar[ psControllerInfo->cChannelHandle ];

        /* TxBufferInfo */
        psControllerInfo->cTxBufferInfo = ( CANTxBuffer_t * ) &TxBufferInfoPar[ psControllerInfo->cChannelHandle ];

        /* RxDedicatedBufferInfo */
        psControllerInfo->cDedicatedBufferInfo = ( CANRxBuffer_t * ) &RxBufferInfoPar[ psControllerInfo->cChannelHandle ];

        /* RxBufferFIFO0Info */
        psControllerInfo->cFIFO0BufferInfo = ( CANRxBuffer_t * ) &RxBufferFIFO0InfoPar[ psControllerInfo->cChannelHandle ];

        /* RxBufferFIFO1Info */
        psControllerInfo->cFIFO1BufferInfo = ( CANRxBuffer_t * ) &RxBufferFIFO1InfoPar[ psControllerInfo->cChannelHandle ];

        psControllerInfo->cCallbackFunctions = ( CANCallBackFunc_t * ) &CANCallbackFunctions;
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

/* Set Buffer */
static CANErrorType_t CAN_DrvInitBuffer
(
    CANController_t *                   psControllerInfo,
    const uint32_t                        uiMemory,
    uint32_t                              uiMemorySize
)
{
    uint32_t          uiStartAddr;
    CANErrorType_t  ret;

    uiStartAddr     = 0UL;
    ret             = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( ( uiMemory != 0UL ) && ( 0UL < uiMemorySize ) )
        {
            uiStartAddr = uiMemory;
			printk("[mw] start address : 0x%x\n", uiStartAddr);

            if( psControllerInfo->cRegister != NULL )
            {
                /* StandardIDFilterListSize */
                if( CAN_PortingGetStandardIDFilterEnable( psControllerInfo->cChannelHandle ) == true )
                {
                    psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr = uiStartAddr;

                    psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfLSS = 0;
                    psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfFLSSA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                    uiStartAddr += CAN_PortingGetSizeofStandardIDFilterList( psControllerInfo->cChannelHandle );
					printk("[mw] ExtendedIDFilterList addr: 0x%x\n", uiStartAddr);
                }

                /* ExtendedIDFilterListSize */
                if( CAN_PortingGetExtendedIDFilterEnable( psControllerInfo->cChannelHandle ) == true )
                {
                    psControllerInfo->cRamAddressInfo.raExtendedIDFilterListAddr = uiStartAddr;

                    psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfLSE = 0;
                    psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfFLESA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                    uiStartAddr += CAN_PortingGetSizeofExtendedIDFilterList( psControllerInfo->cChannelHandle );
					printk("[mw] RxBufferFIFO0Info addr: 0x%x\n", uiStartAddr);
                }

                /* RxBufferFIFO0Info */
                if( psControllerInfo->cFIFO0BufferInfo != NULL )
                {
	printf("[MW] CAN_DrvGetSizeOfRamMemory = %d\n", ret);
                    psControllerInfo->cRamAddressInfo.raRxFIFO0Addr = uiStartAddr;

                    psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfF0DS = ( volatile uint32_t ) psControllerInfo->cFIFO0BufferInfo->rbDataFieldSize;
                    psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxOM = ( volatile uint32_t ) psControllerInfo->cFIFO0BufferInfo->rbOperationMode;
                    psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxWM = 0xA; //Watermark
                    psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxS = psControllerInfo->cFIFO0BufferInfo->rbNumberOfElement;
                    psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxSA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                    uiStartAddr += CAN_PortingGetSizeofRxFIFO0( psControllerInfo->cChannelHandle );
					printk("[mw] RxBufferFIFO1Info addr: 0x%x\n", uiStartAddr);
                }

                /* RxBufferFIFO1Info */
                if( psControllerInfo->cFIFO1BufferInfo != NULL )
                {
                    psControllerInfo->cRamAddressInfo.raRxFIFO1Addr = uiStartAddr;

                    psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfF1DS = ( volatile uint32_t ) psControllerInfo->cFIFO1BufferInfo->rbDataFieldSize;
                    psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxOM = ( volatile uint32_t ) psControllerInfo->cFIFO1BufferInfo->rbOperationMode;
                    psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxWM = 0xA; //Watermark
                    psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxS = psControllerInfo->cFIFO1BufferInfo->rbNumberOfElement;
                    psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxSA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                    uiStartAddr += CAN_PortingGetSizeofRxFIFO1( psControllerInfo->cChannelHandle );
					printk("[mw] RxDedicatedBufferInfo addr: 0x%x\n", uiStartAddr);
                }

                /* RxDedicatedBufferInfo */
                if( psControllerInfo->cDedicatedBufferInfo != NULL )
                {
                    psControllerInfo->cRamAddressInfo.raRxBufferAddr = uiStartAddr;

                    psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfRBDS = ( volatile uint32_t ) psControllerInfo->cDedicatedBufferInfo->rbDataFieldSize;
                    psControllerInfo->cRegister->crRxBufferConfiguration.rFReg.rfRBSA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                    uiStartAddr += CAN_PortingGetSizeofRxBuffer( psControllerInfo->cChannelHandle );
					printk("[mw] TxBufferInfo addr: 0x%x\n", uiStartAddr);
                }

                /* TxBufferInfo */
                if( psControllerInfo->cTxBufferInfo != NULL )
                {
                    psControllerInfo->cRamAddressInfo.raTxBufferAddr = uiStartAddr;

                    psControllerInfo->cRegister->crTxBufferElementSizeConfiguration.rFReg.rfTBDS = psControllerInfo->cTxBufferInfo->tbInterruptConfig;
                    psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTFQM = ( volatile uint32_t ) psControllerInfo->cTxBufferInfo->tbTxFIFOorQueueMode;
                    psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTFQS = psControllerInfo->cTxBufferInfo->tbNumberOfFIFOorQueue;
                    psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfNDTB = psControllerInfo->cTxBufferInfo->tbNumberOfDedicatedBuffer;
                    psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTBSA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                    uiStartAddr += CAN_PortingGetSizeofTxBuffer( psControllerInfo->cChannelHandle );
					printk("[mw] TxEventFIFO addr: 0x%x\n", uiStartAddr);

                    if( CAN_PortingGetTxEventFIFOEnable( psControllerInfo->cChannelHandle ) == true )
                    {
                        psControllerInfo->cRamAddressInfo.raTxEventFIFOAddr = uiStartAddr;

                        psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFWM = ( volatile uint32_t ) psControllerInfo->cTxBufferInfo->tbNumberOfDedicatedBuffer + ( volatile uint32_t ) psControllerInfo->cTxBufferInfo->tbNumberOfFIFOorQueue;
                        psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFS = ( volatile uint32_t ) psControllerInfo->cTxBufferInfo->tbNumberOfDedicatedBuffer + ( volatile uint32_t ) psControllerInfo->cTxBufferInfo->tbNumberOfFIFOorQueue;
                        psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFSA = ( uiStartAddr & 0xFFFFUL ) >> 2UL;

                        ( void ) CAN_PortingGetSizeofTxEventBuffer( psControllerInfo->cChannelHandle );
                    }
                }
            }
        }
        else
        {
            ret = CAN_ERROR_BAD_PARAM;
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

static CANErrorType_t CAN_DrvDeinitBuffer
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( ( psControllerInfo != NULL ) && ( psControllerInfo->cRegister != NULL ) )
    {
        /* StandardIDFilterListSize */
        if( CAN_PortingGetStandardIDFilterEnable( psControllerInfo->cChannelHandle ) == true )
        {
            psControllerInfo->cRamAddressInfo.raStandardIDFilterListAddr = 0;

            psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfLSS = 0;
            psControllerInfo->cRegister->crStandardIDFilterConfiguration.rFReg.rfFLSSA = 0;
        }

        /* ExtendedIDFilterListSize */
        if( CAN_PortingGetExtendedIDFilterEnable( psControllerInfo->cChannelHandle ) == true )
        {
            psControllerInfo->cRamAddressInfo.raExtendedIDFilterListAddr = 0;

            psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfLSE = 0;
            psControllerInfo->cRegister->crExtendedIDFilterConfiguration.rFReg.rfFLESA = 0;
        }

        /* RxBufferFIFO0Info */
        if( psControllerInfo->cFIFO0BufferInfo != NULL )
        {
            psControllerInfo->cRamAddressInfo.raRxFIFO0Addr = 0;

            psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfF0DS = 0;
            psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxOM = ( volatile uint32_t ) psControllerInfo->cFIFO0BufferInfo->rbOperationMode;
            psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxWM = 0xA; //Watermark
            psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxS = 0;
            psControllerInfo->cRegister->crRxFIFO0Configuration.rFReg.rfFxSA = 0;
        }
        /* RxBufferFIFO1Info */
        if( psControllerInfo->cFIFO1BufferInfo != NULL )
        {
            psControllerInfo->cRamAddressInfo.raRxFIFO1Addr = 0;

            psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfF1DS = 0;
            psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxOM = ( volatile uint32_t ) psControllerInfo->cFIFO1BufferInfo->rbOperationMode;
            psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxWM = 0xA; //Watermark
            psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxS = 0;
            psControllerInfo->cRegister->crRxFIFO1Configuration.rFReg.rfFxSA = 0;
        }

        /* RxDedicatedBufferInfo */
        if( psControllerInfo->cDedicatedBufferInfo != NULL )
        {
            psControllerInfo->cRamAddressInfo.raRxBufferAddr = 0;

            psControllerInfo->cRegister->crRxBufferOrFIFOElementSizeConfiguration.rFReg.rfRBDS = 0;
            psControllerInfo->cRegister->crRxBufferConfiguration.rFReg.rfRBSA = 0;
        }

        /* TxBufferInfo */
        if( psControllerInfo->cTxBufferInfo != NULL )
        {
            psControllerInfo->cRamAddressInfo.raTxBufferAddr = 0;

            psControllerInfo->cRegister->crTxBufferElementSizeConfiguration.rFReg.rfTBDS = 0;
            psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTFQM = 0;
            psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTFQS = 0;
            psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfNDTB = 0;
            psControllerInfo->cRegister->crTxBufferConfiguration.rFReg.rfTBSA = 0;

            if( CAN_PortingGetTxEventFIFOEnable( psControllerInfo->cChannelHandle ) == true )
            {
                psControllerInfo->cRamAddressInfo.raTxEventFIFOAddr = 0;

                psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFWM = 0;
                psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFS = 0;
                psControllerInfo->cRegister->crTxEventFIFOConfiguration.rFReg.rfEFSA = 0;
            }
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}


/* ERROR */
uint32_t CAN_DrvGetProtocolStatus
(
    const CANController_t *             psControllerInfo
)
{
    uint32_t ret;

    ret = 0;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            ret = ( uint32_t ) psControllerInfo->cRegister->crProtocolStatusRegister.rNReg;
        }
    }

    return ret;
}

/*  IRQ  */
static void CAN_DrvIRQHandler
(
    void *                              pArg
)
{
    uint8_t ucCh;
    CANController_t * psControllerInfo;

    psControllerInfo = NULL;

    if( pArg != NULL )
    {
        ucCh = * ( uint8_t * ) pArg;

        if( ucCh < CAN_CONTROLLER_NUMBER )
        {
            psControllerInfo = CAN_GetControllerInfo( ucCh );

            if( ( psControllerInfo != NULL ) && ( psControllerInfo->cMode != CAN_MODE_NO_INITIALIZATION ) && ( psControllerInfo->cMode != CAN_MODE_SLEEP ) )
            {
                //CAN_DrvProcessIRQ( ucCh );
                CANTEST_ProcessIRQ( ucCh );
            }
        }
    }
}

static CANErrorType_t CAN_DrvSetInterruptConfig
(
    CANController_t *                   psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        psControllerInfo->cRegister->crInterruptEnable.rNReg = CAN_INTERRUPT_ENABLE;
        psControllerInfo->cRegister->crInterruptLineEnable.rNReg = CAN_INTERRUPT_LINE_ENABLE;
        psControllerInfo->cRegister->crInterruptLineSelect.rNReg = CAN_INTERRUPT_LINE_SEL;

        psControllerInfo->cRegister->crTxBufferTransmissionInterruptEnable = 0xFFFFFFFFUL;
        psControllerInfo->cRegister->crTxBufferCancellationFinishedInterruptEnable = 0xFFFFFFFFUL;

        ( void ) CAN_PortingSetInterruptHandler( psControllerInfo, &CAN_DrvIRQHandler );
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

#if 1 /* can_drv.c */
CANRegFieldIR_t IRClearStatus[CAN_CONTROLLER_NUMBER];
uint8_t timeoutTestForRXFIFO0 = false; //for chip verification test, need to remove this

static void CANTEST_ProcessIRQ(uint8_t ucCh)
{
    CANController_t *   psControllerInfo;
    CANRegFieldIR_t     sIRStatus;
    CANRegFieldIR_t     sIRClearStatus;

    psControllerInfo = NULL;

    psControllerInfo = CAN_GetControllerInfo( ucCh );
    if( psControllerInfo != NULL )
    {
        ( void ) memset( &sIRClearStatus, 0, sizeof( CANRegFieldIR_t ) );

        sIRStatus = psControllerInfo->cRegister->crInterruptRegister.rFReg;

        // 29 : Access to Reserved Address
        if( sIRStatus.rfARA != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_ACCESS_RESERVED_ADDRESS );

            printf( "[CAN Channel : %d] Access to Reserved Address \r\n", ucCh );

            sIRClearStatus.rfARA = 1;
            IRClearStatus[ ucCh ].rfARA = 1;
        }

        // 28 : Protocol Error in Data Phase
        if( sIRStatus.rfPED != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_PROTOCOL );

            printf( "[CAN Channel : %d] Protocol Error in Data Phase \r\n", ucCh );

            sIRClearStatus.rfPED = 1;
            IRClearStatus[ ucCh ].rfPED = 1;
        }

        // 27 : Protocol Error in Arbitration Phase
        if( sIRStatus.rfPEA != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_PROTOCOL );

            printf( "[CAN Channel : %d] Protocol Error in Arbitration Phase \r\n", ucCh );

            sIRClearStatus.rfPEA = 1;
            IRClearStatus[ ucCh ].rfPEA = 1;
        }

        // 26 : Watchdog Interrupt
        if( sIRStatus.rfWDI != 0UL )
        {
            printf( "[CAN Channel : %d] Watchdog Interrupt \r\n", ucCh );

            sIRClearStatus.rfWDI = 1;
            IRClearStatus[ ucCh ].rfWDI = 1;
        }

        // 25 : Bus_Off Status
        if( sIRStatus.rfBO != 0UL )
        {
            printf( "[CAN Channel : %d] Bus_Off Status \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_BUS_OFF );

            sIRClearStatus.rfBO = 1;
            IRClearStatus[ ucCh ].rfBO = 1;
        }

        // 24 : Warning Status
        if( sIRStatus.rfEW != 0UL )
        {
            printf( "[CAN Channel : %d] Warning Status \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_WARNING );

            sIRClearStatus.rfEW = 1;
            IRClearStatus[ ucCh ].rfEW = 1;
        }

        // 23 : Error Passive
        if( sIRStatus.rfEP != 0UL )
        {
            printf( "[CAN Channel : %d] Error Passive \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_PASSIVE );

            sIRClearStatus.rfEP = 1;
            IRClearStatus[ ucCh ].rfEP = 1;
        }

        // 22 : Error Logging Overflow
        if( sIRStatus.rfELO != 0UL )
        {
            printf( "[CAN Channel : %d] Error Logging Overflow \r\n", ucCh );

            sIRClearStatus.rfELO = 1;
            IRClearStatus[ ucCh ].rfELO = 1;
        }

        // 21 : Bit Error Uncorrected
        if( sIRStatus.rfBEU != 0UL )
        {
            printf( "[CAN Channel : %d] Bit Error Uncorrected \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_BIT );

            sIRClearStatus.rfBEU = 1;
            IRClearStatus[ ucCh ].rfBEU = 1;
        }

        // 20 : Bit Error Corrected
        if( sIRStatus.rfBEC != 0UL )
        {
            printf( "[CAN Channel : %d] Bit Error Corrected \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_BIT );

            sIRClearStatus.rfBEC = 1;
            IRClearStatus[ ucCh ].rfBEC = 1;
        }

        // 19 : Message stored to Dedicated Rx Buffer
        if( sIRStatus.rfDRX != 0UL )
        {
            ( void ) CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_DBUFFER );

            sIRClearStatus.rfDRX = 1;
            IRClearStatus[ ucCh ].rfDRX = 1;
        }

        // 18 : Timeout Occurred
        if( sIRStatus.rfTOO != 0UL )
        {
            printf( "[CAN Channel : %d] Timeout Occurred \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_TIMEOUT );

            sIRClearStatus.rfTOO = 1;
            IRClearStatus[ ucCh ].rfTOO = 1;
        }

        // 17 : Message RAM Access Failure
        if( sIRStatus.rfMRAF != 0UL )
        {
            printf( "[CAN Channel : %d] Message RAM Access Failure \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_RAM_ACCESS_FAIL );

            sIRClearStatus.rfMRAF = 1;
            IRClearStatus[ ucCh ].rfMRAF = 1;
        }

        // 16 : Timestamp Wraparound
        if( sIRStatus.rfTSW != 0UL )
        {
            sIRClearStatus.rfTSW = 1;
            IRClearStatus[ ucCh ].rfTSW = 1;
        }

        // 15 : Tx Event FIFO Element Lost
        if( sIRStatus.rfTEFL != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_TX_EVENT_LOST );

            sIRClearStatus.rfTEFL = 1;
            IRClearStatus[ ucCh ].rfTEFL = 1;
        }

        // 14 : Tx Event FIFO Full
        if( sIRStatus.rfTEFF != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_TX_EVENT_FULL );

            sIRClearStatus.rfTEFF = 1;
            IRClearStatus[ ucCh ].rfTEFF = 1;
        }

        // 13 : Tx Event FIFO Watermark Reached
        if( sIRStatus.rfTEFW != 0UL )
        {
            sIRClearStatus.rfTEFW = 1;
            IRClearStatus[ ucCh ].rfTEFW = 1;
        }

        // 12 : Tx Event FIFO New Entry
        if( sIRStatus.rfTEFN != 0UL )
        {
            CAN_DrvCallbackNotifyTxEvent( ucCh, CAN_TX_INT_TYPE_NEW_EVENT );

            sIRClearStatus.rfTEFN = 1;
            IRClearStatus[ ucCh ].rfTEFN = 1;
        }

        // 11 : Tx FIFO Empty
        if( sIRStatus.rfTFE != 0UL )
        {
            sIRClearStatus.rfTFE = 1;
            IRClearStatus[ ucCh ].rfTFE = 1;
        }

        // 10 : Transmission Cancellation Finished
        if( sIRStatus.rfTCF != 0UL )
        {
            CAN_DrvCallbackNotifyTxEvent( ucCh, CAN_TX_INT_TYPE_TRANSMIT_CANCEL_FINISHED );

            sIRClearStatus.rfTCF = 1;
            IRClearStatus[ ucCh ].rfTCF = 1;
        }

        // 9 : Transmission Completed
        if( sIRStatus.rfTC != 0UL )
        {
            CAN_DrvCallbackNotifyTxEvent( ucCh, CAN_TX_INT_TYPE_TRANSMIT_COMPLETED );

            sIRClearStatus.rfTC = 1;
            IRClearStatus[ ucCh ].rfTC = 1;
        }

        // 8 : High Priority Message
        if( sIRStatus.rfHPM != 0UL )
        {
            printf( "[CAN Channel : %d] High Priority Message \r\n", ucCh );

            sIRClearStatus.rfHPM = 1;
            IRClearStatus[ ucCh ].rfHPM = 1;
        }

        // 7 : Rx FIFO 1 Message Lost
        if( sIRStatus.rfRF1L != 0UL )
        {
            CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_LOST );

            sIRClearStatus.rfRF1L = 1;
            IRClearStatus[ ucCh ].rfRF1L = 1;
        }

        // 6 : Rx FIFO 1 Full
        if( sIRStatus.rfRF1F != 0UL )
        {
            CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_FULL );

            sIRClearStatus.rfRF1F = 1;
            IRClearStatus[ ucCh ].rfRF1F = 1;
        }

        // 5 : Rx FIFO 1 Watermark Reached
        if( sIRStatus.rfRF1W != 0UL )
        {
            sIRClearStatus.rfRF1W = 1;
            IRClearStatus[ ucCh ].rfRF1W = 1;
        }

        // 4 : Rx FIFO 1 New Message
        if( sIRStatus.rfRF1N != 0UL )
        {
            ( void ) CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_FIFO1 );

            sIRClearStatus.rfRF1N = 1;
            IRClearStatus[ ucCh ].rfRF1N = 1;
        }

        // 3 : Rx FIFO 0 Message Lost
        if( sIRStatus.rfRF0L != 0UL )
        {
            CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_LOST );

            sIRClearStatus.rfRF0L = 1;
            IRClearStatus[ ucCh ].rfRF0L = 1;
        }

        // 2 : Rx FIFO 0 Full
        if( sIRStatus.rfRF0F != 0UL )
        {
            CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_FULL );

            sIRClearStatus.rfRF0F = 1;
            IRClearStatus[ ucCh ].rfRF0F = 1;
        }

        // 1 : Rx FIFO 0 Watermark Reached
        if( sIRStatus.rfRF0W != 0UL )
        {
            sIRClearStatus.rfRF0W = 1;
            IRClearStatus[ ucCh ].rfRF0W = 1;
        }

        // 0 : Rx FIFO 0 New Message
        if( sIRStatus.rfRF0N != 0UL )
        {
            if(timeoutTestForRXFIFO0 == false) {
                ( void ) CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_FIFO0 );
            }

            sIRClearStatus.rfRF0N = 1;
            IRClearStatus[ ucCh ].rfRF0N = 1;
        }

        memcpy( &(IRClearStatus[ ucCh ]), &sIRClearStatus, sizeof( CANRegFieldIR_t ) );

        psControllerInfo->cRegister->crInterruptRegister.rFReg = sIRClearStatus;
    }
}
#else
static void CAN_DrvProcessIRQ
(
    uint8_t                               ucCh
)
{
    CANController_t *   psControllerInfo;
    CANRegFieldIR_t     sIRStatus;
    CANRegFieldIR_t     sIRClearStatus;

    psControllerInfo = NULL;

    psControllerInfo = CAN_GetControllerInfo( ucCh );
    if( psControllerInfo != NULL )
    {
        ( void ) memset( &sIRClearStatus, 0, sizeof( CANRegFieldIR_t ) );

        sIRStatus = psControllerInfo->cRegister->crInterruptRegister.rFReg;

        // 29 : Access to Reserved Address
        if( sIRStatus.rfARA != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_ACCESS_RESERVED_ADDRESS );

            CAN_D( "[CAN Channel : %d] Access to Reserved Address \r\n", ucCh );

            sIRClearStatus.rfARA = 1;
        }

        // 28 : Protocol Error in Data Phase
        if( sIRStatus.rfPED != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_PROTOCOL );

            CAN_D( "[CAN Channel : %d] Protocol Error in Data Phase \r\n", ucCh );

            sIRClearStatus.rfPED = 1;
        }

        // 27 : Protocol Error in Arbitration Phase
        if( sIRStatus.rfPEA != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_PROTOCOL );

            CAN_D( "[CAN Channel : %d] Protocol Error in Arbitration Phase \r\n", ucCh );

            sIRClearStatus.rfPEA = 1;
        }

        // 26 : Watchdog Interrupt
        if( sIRStatus.rfWDI != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Watchdog Interrupt \r\n", ucCh );

            sIRClearStatus.rfWDI = 1;
        }

        // 25 : Bus_Off Status
        if( sIRStatus.rfBO != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Bus_Off Status \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_BUS_OFF );

            sIRClearStatus.rfBO = 1;
        }

        // 24 : Warning Status
        if( sIRStatus.rfEW != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Warning Status \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_WARNING );

            sIRClearStatus.rfEW = 1;
        }

        // 23 : Error Passive
        if( sIRStatus.rfEP != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Error Passive \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_PASSIVE );

            sIRClearStatus.rfEP = 1;
        }

        // 22 : Error Logging Overflow
        if( sIRStatus.rfELO != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Error Logging Overflow \r\n", ucCh );

            sIRClearStatus.rfELO = 1;
        }

        // 21 : Bit Error Uncorrected
        if( sIRStatus.rfBEU != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Bit Error Uncorrected \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_BIT );

            sIRClearStatus.rfBEU = 1;
        }

        // 20 : Bit Error Corrected
        if( sIRStatus.rfBEC != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Bit Error Corrected \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_BIT );

            sIRClearStatus.rfBEC = 1;
        }

        // 19 : Message stored to Dedicated Rx Buffer
        if( sIRStatus.rfDRX != 0UL )
        {
            ( void ) CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_DBUFFER );

            sIRClearStatus.rfDRX = 1;
        }

        // 18 : Timeout Occurred
        if( sIRStatus.rfTOO != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Timeout Occurred \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_TIMEOUT );

            sIRClearStatus.rfTOO = 1;
        }

        // 17 : Message RAM Access Failure
        if( sIRStatus.rfMRAF != 0UL )
        {
            CAN_D( "[CAN Channel : %d] Message RAM Access Failure \r\n", ucCh );

            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_RAM_ACCESS_FAIL );

            sIRClearStatus.rfMRAF = 1;
        }

        // 16 : Timestamp Wraparound
        if( sIRStatus.rfTSW != 0UL )
        {
            sIRClearStatus.rfTSW = 1;
        }

        // 15 : Tx Event FIFO Element Lost
        if( sIRStatus.rfTEFL != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_TX_EVENT_LOST );

            sIRClearStatus.rfTEFL = 1;
        }

        // 14 : Tx Event FIFO Full
        if( sIRStatus.rfTEFF != 0UL )
        {
            CAN_DrvCallbackNotifyErrorEvent( ucCh, CAN_ERROR_INT_TX_EVENT_FULL );

            sIRClearStatus.rfTEFF = 1;
        }

        // 13 : Tx Event FIFO Watermark Reached
        if( sIRStatus.rfTEFW != 0UL )
        {
            sIRClearStatus.rfTEFW = 1;
        }

        // 12 : Tx Event FIFO New Entry
        if( sIRStatus.rfTEFN != 0UL )
        {
            CAN_DrvCallbackNotifyTxEvent( ucCh, CAN_TX_INT_TYPE_NEW_EVENT );

            sIRClearStatus.rfTEFN = 1;
        }

        // 11 : Tx FIFO Empty
        if( sIRStatus.rfTFE != 0UL )
        {
            sIRClearStatus.rfTFE = 1;
        }

        // 10 : Transmission Cancellation Finished
        if( sIRStatus.rfTCF != 0UL )
        {
            CAN_DrvCallbackNotifyTxEvent( ucCh, CAN_TX_INT_TYPE_TRANSMIT_CANCEL_FINISHED );

            sIRClearStatus.rfTCF = 1;
        }

        // 9 : Transmission Completed
        if( sIRStatus.rfTC != 0UL )
        {
            CAN_DrvCallbackNotifyTxEvent( ucCh, CAN_TX_INT_TYPE_TRANSMIT_COMPLETED );

            sIRClearStatus.rfTC = 1;
        }

        // 8 : High Priority Message
        if( sIRStatus.rfHPM != 0UL )
        {
            CAN_D( "[CAN Channel : %d] High Priority Message \r\n", ucCh );

            sIRClearStatus.rfHPM = 1;
        }

        // 7 : Rx FIFO 1 Message Lost
        if( sIRStatus.rfRF1L != 0UL )
        {
            CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_LOST );

            sIRClearStatus.rfRF1L = 1;
        }

        // 6 : Rx FIFO 1 Full
        if( sIRStatus.rfRF1F != 0UL )
        {
            CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_FULL );

            sIRClearStatus.rfRF1F = 1;
        }

        // 5 : Rx FIFO 1 Watermark Reached
        if( sIRStatus.rfRF1W != 0UL )
        {
            sIRClearStatus.rfRF1W = 1;
        }

        // 4 : Rx FIFO 1 New Message
        if( sIRStatus.rfRF1N != 0UL )
        {
            ( void ) CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_FIFO1 );

            sIRClearStatus.rfRF1N = 1;
        }

        // 3 : Rx FIFO 0 Message Lost
        if( sIRStatus.rfRF0L != 0UL )
        {
            CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_LOST );

            sIRClearStatus.rfRF0L = 1;
        }

        // 2 : Rx FIFO 0 Full
        if( sIRStatus.rfRF0F != 0UL )
        {
            CAN_DrvCallbackNotifyRxEvent( ucCh, 0xFF, CAN_RX_BUFFER_TYPE_FIFO0, CAN_ERROR_FIFO_QUEUE_FULL );

            sIRClearStatus.rfRF0F = 1;
        }

        // 1 : Rx FIFO 0 Watermark Reached
        if( sIRStatus.rfRF0W != 0UL )
        {
            sIRClearStatus.rfRF0W = 1;
        }

        // 0 : Rx FIFO 0 New Message
        if( sIRStatus.rfRF0N != 0UL )
        {
            ( void ) CAN_MsgPutRxMessage( ucCh, CAN_RX_BUFFER_TYPE_FIFO0 );

            sIRClearStatus.rfRF0N = 1;
        }

        psControllerInfo->cRegister->crInterruptRegister.rFReg = sIRClearStatus;
    }
}
#endif

static void CAN_DrvCallbackNotifyTxEvent
(
    uint8_t                               ucCh,
    CANTxInterruptType_t                uiIntType
)
{
    CANController_t * psControllerInfo;

    psControllerInfo = NULL;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = CAN_GetControllerInfo( ucCh );

        if( ( psControllerInfo != NULL ) && ( psControllerInfo->cCallbackFunctions != NULL ) )
        {
            if( psControllerInfo->cCallbackFunctions->cbNotifyTxEvent != NULL )
            {
                psControllerInfo->cCallbackFunctions->cbNotifyTxEvent( ucCh, uiIntType );
            }
        }
    }
}

void CAN_DrvCallbackNotifyRxEvent
(
    uint8_t                               ucCh,
    uint32_t                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
)
{
    CANController_t * psControllerInfo;

    psControllerInfo = NULL;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = CAN_GetControllerInfo( ucCh );

        if( ( psControllerInfo != NULL ) &&  (psControllerInfo->cCallbackFunctions != NULL ) )
        {
            if( psControllerInfo->cCallbackFunctions->cbNotifyRxEvent != NULL )
            {
                psControllerInfo->cCallbackFunctions->cbNotifyRxEvent( ucCh, uiRxIndex, uiRxBufferType, uiError );
            }
        }
    }
}

static void CAN_DrvCallbackNotifyErrorEvent
(
    uint8_t                               ucCh,
    CANErrorType_t                      uiError
)
{
    CANController_t * psControllerInfo;

    psControllerInfo = NULL;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        psControllerInfo = CAN_GetControllerInfo( ucCh );

        if( ( psControllerInfo != NULL ) && ( psControllerInfo->cCallbackFunctions != NULL ) )
        {
            if( psControllerInfo->cCallbackFunctions->cbNotifyErrorEvent != NULL )
            {
                psControllerInfo->cCallbackFunctions->cbNotifyErrorEvent( ucCh, uiError );
            }
        }
    }
}

/* Not used function */
CANErrorType_t CAN_DrvSetDisableAutomaticRetransmission
(
    CANController_t *                   psControllerInfo,
    bool bDisable
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
        if( psControllerInfo->cRegister != NULL )
        {
            ( void ) CAN_DrvStartConfigSetting( psControllerInfo );

            psControllerInfo->cRegister->crCCControl.rFReg.rfDAR = bDisable;

            ( void ) CAN_DrvFinishConfigSetting( psControllerInfo );
        }
    }
    else
    {
        ret = CAN_ERROR_NOT_INIT;
    }

    return ret;
}

