// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef __BSP_CAN_DRV_HEADER
#define __BSP_CAN_DRV_HEADER
#include <stdlib.h>
/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

typedef struct CANRamAddress
{
    uint32_t                              raStandardIDFilterListAddr;
    uint32_t                              raExtendedIDFilterListAddr;
    uint32_t                              raRxFIFO0Addr;
    uint32_t                              raRxFIFO1Addr;
    uint32_t                              raRxBufferAddr;
    uint32_t                              raTxEventFIFOAddr;
    uint32_t                              raTxBufferAddr;
} CANRamAddress_t;

typedef struct CANController
{
    /* Config */
    CANMode_t                           cMode;
    uint8_t                               cChannelHandle;
    uint32_t                              cFrequency;

    /* Timing */
    CANTimingParam_t *                  cArbiPhaseTimeInfo;
    CANTimingParam_t *                  cDataPhaseTimeInfo;

    /* Buffer */
    CANTxBuffer_t *                     cTxBufferInfo;
    CANRxBuffer_t *                     cDedicatedBufferInfo;
    CANRxBuffer_t *                     cFIFO0BufferInfo;
    CANRxBuffer_t *                     cFIFO1BufferInfo;

    /*Filter */
    uint8_t                               cNumOfStdIDFilterList;
    CANIdFilterList_t *                 cStdIDFilterList;
    uint8_t                               cNumOfExtIDFilterList;
    CANIdFilterList_t *                 cExtIDFilterList;

    /* Register & Address*/
    CANRamAddress_t                     cRamAddressInfo;
    CANControllerRegister_t *           cRegister;
    CANCallBackFunc_t *                 cCallbackFunctions;
} CANController_t;

typedef struct CANDriver
{
    uint8_t                               dIsInitDriver;
    uint8_t                               dNumOfController;
    CANController_t                     dControllerInfo[ CAN_CONTROLLER_NUMBER ];
} CANDriver_t;


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

CANErrorType_t CAN_DrvInitChannel
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvDeinitChannel
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetNormalOperationMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetMonitoringMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetInternalTestMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetExternalTestMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetWakeUpMode
(
    CANController_t *                   psControllerInfo
);

CANErrorType_t CAN_DrvSetSleepMode
(
    CANController_t *                   psControllerInfo
);

uint32_t CAN_DrvGetProtocolStatus
(
    const CANController_t *             psControllerInfo
);

void CAN_DrvCallbackNotifyRxEvent
(
    uint8_t                               ucCh,
    uint32_t                              uiRxIndex,
    CANMessageBufferType_t              uiRxBufferType,
    CANErrorType_t                      uiError
);

/* Not used function */
CANErrorType_t CAN_DrvSetDisableAutomaticRetransmission
(
    CANController_t *                   psControllerInfo,
    bool                             bDisable
);

#endif /* __BSP_CAN_DRV_HEADER__ */

