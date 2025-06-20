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
#include <test/can_porting.h>

#include <mach/clock.h>

#include <asm/io.h>

#include <linux/delay.h>

#include <irq_func.h>

/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

void CAN_PortingDelay
(
    uint32_t                              uiMsDelay
)
{
    mdelay(uiMsDelay);
#if 0
#if (CAN_SAL_DELAY_SUPPORT == 1)
    SAL_TaskSleep( uiMsDelay );
#else
    uint32_t i;
    uint32_t uiCnt;

    uiCnt = 0;

    uiCnt = uiMsDelay * 10000UL;

    for( i = 0 ; i < uiCnt ; i++ )
    {
        //BSP_NOP_DELAY();
    }
#endif
#endif
}

#define GPIO_PIN_MASK	0x1F
#define SEL_GPIO	0x1
#define SEL_PMGPIO	0x0


CANErrorType_t CAN_PortingInitHW
(
    const CANController_t *             psControllerInfo
)
{
	int32_t ret = 0;

	ret = can_gpio_setting();

	return ret;
}

CANErrorType_t CAN_PortingSetControllerClock
(
    CANController_t *                   psControllerInfo,
    uint32_t                              uiFreq
)
{
	CANErrorType_t ret;
	unsigned long pclk;

	ret = CAN_ERROR_NONE;

	if( psControllerInfo != NULL )
	{
		pclk = get_can_pclk(psControllerInfo->cChannelHandle);

		tcc_set_peri(pclk, CKC_ENABLE, CAN_CONTROLLER_CLOCK, 0);

		psControllerInfo->cFrequency = uiFreq; //80MHz
	}

	return ret;
}

CANErrorType_t CAN_PortingResetDriver
(
    const CANController_t *             psControllerInfo
)
{
    CANErrorType_t ret;

    ret = CAN_ERROR_NONE;

    if( psControllerInfo != NULL )
    {
	    can_reset_channel(psControllerInfo->cChannelHandle);
    }

    return ret;
}

CANErrorType_t CAN_PortingSetInterruptHandler
(
    CANController_t *                   psControllerInfo,
    void                          	(*fnIsr)(void*)
)
{
    CANErrorType_t ret;
    int32_t irq;

    ret = CAN_ERROR_BAD_PARAM;

    if( ( psControllerInfo != NULL ) && ( fnIsr != NULL ) )
    {
	printf("CAN interrupt handler setup\n");
	irq = get_can_irq(psControllerInfo->cChannelHandle);
	irq_install_handler(irq, fnIsr, ( void * ) &( psControllerInfo->cChannelHandle ));
#if (CAN_INTERRUPT_LINE == 0)
        //( void ) GIC_IntVectSet( ( uint32_t ) GIC_CAN0_0 + ( ( uint32_t ) psControllerInfo->cChannelHandle * 2UL ), GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_HIGH, fnIsr, ( void * ) &( psControllerInfo->cChannelHandle ) );
#elif (CAN_INTERRUPT_LINE == 1)
        //( void ) GIC_IntVectSet( ( uint32_t ) GIC_CAN0_1 + ( ( uint32_t ) psControllerInfo->cChannelHandle * 2UL ), GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_HIGH, fnIsr, ( void * ) &( psControllerInfo->cChannelHandle ) );
#else
        ##### ERROR - Select CAN Interrupt Line #####
#endif
        ret = CAN_ERROR_NONE;
    }

    return ret;
}

CANErrorType_t CAN_PortingDisableControllerInterrupts
(
    uint8_t                               ucCh
)
{
    CANErrorType_t ret;
    int32_t irq;

    ret = CAN_ERROR_BAD_PARAM;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
	printf("CAN interrupt mask\n");
	irq = get_can_irq(ucCh);
	irq_mask(irq);
#if (CAN_INTERRUPT_LINE == 0)
        //( void ) GIC_IntSrcDis( ( uint32_t ) GIC_CAN0_0 + ( ( uint32_t ) ucCh * 2UL ) );
#elif (CAN_INTERRUPT_LINE == 1)
        //( void ) GIC_IntSrcDis( ( uint32_t ) GIC_CAN0_1 + ( ( uint32_t ) ucCh * 2UL ) );
#else
        ##### ERROR - Select CAN Interrupt Line #####
#endif
        ret = CAN_ERROR_NONE;
    }

    return ret;
}

CANErrorType_t CAN_PortingEnableControllerInterrupts
(
    uint8_t                               ucCh
)
{
    CANErrorType_t ret;
    int32_t irq;

    ret = CAN_ERROR_BAD_PARAM;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
	printf("CAN interupt unmask\n");
	irq = get_can_irq(ucCh);
	irq_unmask(irq);

        ret = CAN_ERROR_NONE;
    }

    return ret;
}

uint32_t CAN_PortingGetSizeofStandardIDFilterList
(
    uint8_t                               ucCh
)
{
    uint32_t uiSize;

    uiSize = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiSize = CAN_STANDARD_ID_FILTER_NUMBER_MAX * CAN_STANDARD_ID_FILTER_SIZE;
    }

    return uiSize;
}

uint32_t CAN_PortingGetSizeofExtendedIDFilterList
(
    uint8_t                               ucCh
)
{
    uint32_t uiSize;

    uiSize = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiSize = CAN_EXTENDED_ID_FILTER_NUMBER_MAX * CAN_EXTENDED_ID_FILTER_SIZE;
    }

    return uiSize;
}

uint32_t CAN_PortingGetSizeofRxFIFO0
(
    uint8_t                               ucCh
)
{
    uint32_t uiSize;

    uiSize = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiSize = CAN_RX_FIFO_0_MAX * CAN_BUFFER_SIZE;
    }

    return uiSize;
}

uint32_t CAN_PortingGetSizeofRxFIFO1
(
    uint8_t                               ucCh
)
{
    uint32_t uiSize;

    uiSize = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiSize = CAN_RX_FIFO_1_MAX * CAN_BUFFER_SIZE;
    }

    return uiSize;
}

uint32_t CAN_PortingGetSizeofRxBuffer
(
    uint8_t                               ucCh
)
{
    uint32_t uiSize;

    uiSize = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiSize = CAN_RX_BUFFER_MAX * CAN_BUFFER_SIZE;
    }

    return uiSize;
}

uint32_t CAN_PortingGetSizeofTxEventBuffer
(
    uint8_t                               ucCh
)
{
    uint32_t uiSize;

    uiSize = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiSize = CAN_TX_EVENT_FIFO_MAX * CAN_TX_EVENT_SIZE;
    }

    return uiSize;
}

uint32_t CAN_PortingGetSizeofTxBuffer
(
    uint8_t                               ucCh
)
{
    uint32_t uiSize;

    uiSize = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiSize = CAN_TX_BUFFER_MAX * CAN_BUFFER_SIZE;
    }

    return uiSize;
}

uint8_t CAN_PortingGetBitRateSwitchEnable
(
    uint8_t                               ucCh
)
{
    uint8_t ret;

    ret = false;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = true;
    }

    return ret;
}

uint8_t CAN_PortingGetFDEnable
(
    uint8_t                               ucCh
)
{
    uint8_t ret;

    ret = false;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = true;
    }

    return ret;
}

uint8_t CAN_PortingGetStandardIDFilterEnable
(
    uint8_t                               ucCh
)
{
    uint8_t ret;

    ret = false;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = true;
    }

    return ret;
}

uint8_t CAN_PortingGetExtendedIDFilterEnable
(
    uint8_t                               ucCh
)
{
    uint8_t ret;

    ret = false;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = true;
    }

    return ret;
}

uint8_t CAN_PortingGetStandardIDRemoteRejectEnable
(
    uint8_t                               ucCh
)
{
    uint8_t ret;

    ret = false;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = true;
    }

    return ret;
}

uint8_t CAN_PortingGetExtendedIDRemoteRejectEnable
(
    uint8_t                               ucCh
)
{
    uint8_t ret;

    ret = false;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = true;
    }

    return ret;
}

uint8_t CAN_PortingGetTxEventFIFOEnable
(
    uint8_t                               ucCh
)
{
    uint8_t ret;

    ret = false;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = true;
    }

    return ret;
}

uint8_t CAN_PortingGetWatchDogEnable
(
    uint8_t                               ucCh
)
{
    uint8_t ret;

    ret = false;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = true;
    }

    return ret;
}

uint8_t CAN_PortingGetTimeOutEnable
(
    uint8_t                               ucCh
)
{
    uint8_t ret;

    ret = false;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = true;
    }

    return ret;
}

uint8_t CAN_PortingGetTimeStampEnable
(
    uint8_t                               ucCh
)
{
    uint8_t ret;

    ret = false;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        ret = true;
    }

    return ret;
}

uint32_t CAN_PortingAllocateNonCacheMemory
(
    uint8_t                               ucCh,
    uint32_t                              uiMemSize
)
{
    uint32_t uiMemAddr;
    uint32_t uiAlignedMemSize;

    uiMemAddr           = 0;
    uiAlignedMemSize    = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiAlignedMemSize = uiMemSize & 0xFFFFF000UL;

        if( ( uiMemSize & 0xFFFUL ) != 0UL )
        {
            uiAlignedMemSize += 0x1000UL;
        }

        uiMemAddr = CAN_NON_CACHE_MEMORY_START + ( uiAlignedMemSize * ucCh );
	memset((uint32_t *)uiMemAddr, 0, uiMemSize);
    }
	printf("[MW] CAN_PortingAllocateNonCacheMemory = %d\n", uiMemAddr);

    return uiMemAddr;
}

void CAN_PortingDeallocateNonCacheMemory
(
    uint8_t                               ucCh,
    uint32_t *                            puiMemAddr
)
{
    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        *puiMemAddr = ( uint32_t ) 0;
    }
}

uint32_t CAN_PortingGetControllerRegister
(
    uint8_t                               ucCh
)
{
    uint32_t uiControllerAddr;

    uiControllerAddr = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiControllerAddr = CAN_BASE_ADDR + ( CAN_CHANNEL_ADDR_OFFSET * ( uint32_t ) ucCh );
    }

    return uiControllerAddr;
}

uint32_t CAN_PortingGetConfigBaseAddr
(
    void
)
{
    uint32_t uiAddr;

    uiAddr = CAN_CONFIG_ADDR;

    return uiAddr;
}

uint32_t CAN_PortingGetMessageRamBaseAddr
(
    uint8_t                               ucCh
)
{
    uint32_t uiAddr;

    uiAddr = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiAddr = CAN_CONFIG_BASE_ADDR + ( ( uint32_t ) 0x4UL * (( uint32_t ) ucCh + CAN_BASE_PORT) );
    }

    return uiAddr;
}

uint32_t CAN_PortingGetTSCFGAddr
(
	uint8_t                               ucCh
)
{
	uint32_t uiAddr;

	uiAddr = 0;

	if( ucCh < CAN_CONTROLLER_NUMBER )
	{
		uiAddr = CAN_TSCFG_ADDR;
	}

	return uiAddr;
}


uint32_t CAN_PortingGetConfigEXTS0Addr
(
    uint8_t                               ucCh
)
{
    uint32_t uiAddr;

    uiAddr = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiAddr = CAN_CONFIG_EXTS_CTRL0_ADDR;
    }

    return uiAddr;
}

uint32_t CAN_PortingGetConfigEXTS1Addr
(
    uint8_t                               ucCh
)
{
    uint32_t uiAddr;

    uiAddr = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiAddr = CAN_CONFIG_EXTS_CTRL1_ADDR;
    }

    return uiAddr;
}

uint32_t CAN_PortingGetConfigWritePasswordAddr
(
    uint8_t                               ucCh
)
{
    uint32_t uiAddr;

    uiAddr = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiAddr = CAN_CONFIG_WR_PW_ADDR;
    }

    return uiAddr;
}

uint32_t CAN_PortingGetConfigWriteLockAddr
(
    uint8_t                               ucCh
)
{
    uint32_t uiAddr;

    uiAddr = 0;

    if( ucCh < CAN_CONTROLLER_NUMBER )
    {
        uiAddr = CAN_CONFIG_WR_LOCK_ADDR;
    }

    return uiAddr;
}

