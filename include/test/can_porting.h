// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef __BSP_CAN_PORTING_HEADER
#define __BSP_CAN_PORTING_HEADER

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

//#include "gic.h"
//#include "pmio.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


#define CAN_SAL_DELAY_SUPPORT           (0)

/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

void CAN_PortingDelay
(
    uint32_t                              uiMsDelay
);

CANErrorType_t CAN_PortingInitHW
(
    const CANController_t *             psControllerInfo
);

CANErrorType_t CAN_PortingSetControllerClock
(
    CANController_t *                   psControllerInfo,
    uint32_t                              uiFreq
);

CANErrorType_t CAN_PortingResetDriver
(
    const CANController_t *             psControllerInfo
);

CANErrorType_t CAN_PortingSetInterruptHandler
(
    CANController_t *                   psControllerInfo,
    void                                (*fnIsr)(void*)
);

CANErrorType_t CAN_PortingDisableControllerInterrupts
(
    uint8_t                               ucCh
);

CANErrorType_t CAN_PortingEnableControllerInterrupts
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetSizeofStandardIDFilterList
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetSizeofExtendedIDFilterList
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetSizeofRxFIFO0
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetSizeofRxFIFO1
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetSizeofRxBuffer
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetSizeofTxEventBuffer
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetSizeofTxBuffer
(
    uint8_t                               ucCh
);

uint8_t CAN_PortingGetBitRateSwitchEnable
(
    uint8_t                               ucCh
);

uint8_t CAN_PortingGetFDEnable
(
    uint8_t                               ucCh
);

uint8_t CAN_PortingGetStandardIDFilterEnable
(
    uint8_t                               ucCh
);

uint8_t CAN_PortingGetExtendedIDFilterEnable
(
    uint8_t                               ucCh
);

uint8_t CAN_PortingGetStandardIDRemoteRejectEnable
(
    uint8_t                               ucCh
);

uint8_t CAN_PortingGetExtendedIDRemoteRejectEnable
(
    uint8_t                               ucCh
);

uint8_t CAN_PortingGetTxEventFIFOEnable
(
    uint8_t                               ucCh
);

uint8_t CAN_PortingGetWatchDogEnable
(
    uint8_t                               ucCh
);

uint8_t CAN_PortingGetTimeOutEnable
(
    uint8_t                               ucCh
);

uint8_t CAN_PortingGetTimeStampEnable
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingAllocateNonCacheMemory
(
    uint8_t                               ucCh,
    uint32_t                              uiMemSize
);

void CAN_PortingDeallocateNonCacheMemory
(
    uint8_t                               ucCh,
    uint32_t *                            puiMemAddr
);

uint32_t CAN_PortingGetControllerRegister
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetConfigBaseAddr
(
    void
);

uint32_t CAN_PortingGetMessageRamBaseAddr
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetConfigEXTS0Addr
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetConfigEXTS1Addr
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetConfigWritePasswordAddr
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetConfigWriteLockAddr
(
    uint8_t                               ucCh
);

uint32_t CAN_PortingGetTSCFGAddr
(
	uint8_t                               ucCh
);

#endif // __BSP_CAN_PORTING_HEADER__

