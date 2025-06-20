// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef __BSP_CAN_PAR_HEADER
#define __BSP_CAN_PAR_HEADER

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

extern CANTimingParam_t ArbitrationPhaseTimingPar[ CAN_CONTROLLER_NUMBER ];

extern CANTimingParam_t DataPhaseTimingPar[ CAN_CONTROLLER_NUMBER ];

extern CANTxBuffer_t TxBufferInfoPar[ CAN_CONTROLLER_NUMBER ];

extern CANRxBuffer_t RxBufferInfoPar[ CAN_CONTROLLER_NUMBER ];

extern CANRxBuffer_t RxBufferFIFO0InfoPar[ CAN_CONTROLLER_NUMBER ];

extern CANRxBuffer_t RxBufferFIFO1InfoPar[ CAN_CONTROLLER_NUMBER ];

extern CANCallBackFunc_t CANCallbackFunctions;

extern CANIdFilter_t StandardIDFilterPar;

extern CANIdFilter_t ExtendedIDFilterPar;


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/


#endif // __BSP_CAN_PAR_HEADER__

