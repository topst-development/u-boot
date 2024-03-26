// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef __CAN_CONFIG_HEADER
#define __CAN_CONFIG_HEADER

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/

/* Debug message control */
#if (DEBUG_ENABLE)
#define CAN_D(fmt, args...)             {LOGD(DBG_TAG_CAN, fmt, ## args)}
#define CAN_E(fmt, args...)             {LOGE(DBG_TAG_CAN, fmt, ## args)}
#else
#define CAN_D(fmt, args...)
#define CAN_E(fmt, args...)
#endif

//#define CAN_FPGA_TEST /* Only for FPGA environment */


#define CAN_REG_PSR_PXE                 (14)
#define CAN_REG_PSR_RFDF                (13)
#define CAN_REG_PSR_RBRS                (12)
#define CAN_REG_PSR_RESI                (11)
#define CAN_REG_PSR_DLEC                (8)
#define CAN_REG_PSR_BO                  (7)
#define CAN_REG_PSR_EW                  (6)
#define CAN_REG_PSR_EP                  (5)
#define CAN_REG_PSR_ACT                 (3)
#define CAN_REG_PSR_LEC                 (0)


#define CAN_DATA_LENGTH_SIZE            (64U)

#define CAN_STANDARD_ID_FILTER_NUMBER_MAX   (128UL)
#define CAN_EXTENDED_ID_FILTER_NUMBER_MAX   (64UL)

#define CAN_STANDARD_ID_FILTER_NUMBER   (25U)
#define CAN_EXTENDED_ID_FILTER_NUMBER   (25U)

#define CAN_RX_FIFO_0_MAX               (64UL)
#define CAN_RX_FIFO_1_MAX               (64UL)
#define CAN_RX_BUFFER_MAX               (64UL)
#define CAN_TX_EVENT_FIFO_MAX           (32UL)
#define CAN_TX_BUFFER_MAX               (32UL)

#define CAN_STANDARD_ID_FILTER_SIZE     (4U)
#define CAN_EXTENDED_ID_FILTER_SIZE     (8U)
#define CAN_BUFFER_SIZE                 (8U+CAN_DATA_LENGTH_SIZE)
#define CAN_TX_EVENT_SIZE               (8U)

#define CAN_CONTROLLER_CLOCK            (80000000UL)    /* 80MHz */


#define CAN_RX_MSG_RING_BUFFER_MAX      (192UL)

/* TimeStamp */
#define CAN_TIMESTAMP_PRESCALER         (15)  /* Prescaler = MAX(15) +1 */
#define CAN_TIMESTAMP_TYPE              (1)   /* 0 : Internal Timestamp, 1: External Timestamp */
#define CAN_TIMESTAMP_RATIO             (CAN_TIMESTAMP_PRESCALER)
#define CAN_TIMESTAMP_COMP              (0xFFFF)

/* TimerCount */
#define CAN_TIMEOUT_VALUE               (0xFFFF)  /* reset = 0xFFFF(MAX) */
#define CAN_TIMEOUT_TYPE                (0x02) /* 0x00:Continuous, 0x01:TxEventFIFO, 0x02:RxFIFO0, 0x03:RxFIFO1 */

/* Interrupt */
#define CAN_INTERRUPT_LINE              (0U) /* 0:Line0, 1:Line1 */

#define CAN_INTERRUPT_ENABLE            (0x3FFEFFFFUL)

#if (CAN_INTERRUPT_LINE == 0U)
    #define CAN_INTERRUPT_LINE_SEL      (0x0)
    #define CAN_INTERRUPT_LINE_ENABLE   (0x1)
#elif (CAN_INTERRUPT_LINE == 1U)
    #define CAN_INTERRUPT_LINE_SEL      (0x3FFFFFFFUL)
    #define CAN_INTERRUPT_LINE_ENABLE   (0x2)
#else
##### ERROR - Select CAN Interrupt Line #####
#endif


/* WatchDog */
#define CAN_WATCHDOG_VALUE              (0xFF)


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/


#endif // __CAN_CONFIG_HEADER
