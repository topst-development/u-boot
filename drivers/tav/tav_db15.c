// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <tav.h>
#include <tav_vioc.h>

#include <asm/telechips/vioc/vioc_global.h>
#include <asm/telechips/vioc/vioc_timer.h>
#include <asm/telechips/vioc/vioc_intr.h>

static int timer_irq_first = 2;
static int timer_irq = INT_VIOC0;
static int timer_intr = VIOC_INTR_TIMER;

static unsigned int previous_time[4];

static void tcc_tav_timer_handler(void *data)
{
	unsigned int boot_time_ms_gab;
	unsigned long status;
	void __iomem *vioc_timer = VIOC_TIMER_GetAddress();

	status = VIOC_TIMER_GetIrqStatus(vioc_timer);

	if (status & (1 << VIOC_TIMER_IRQ_TIMER0))
		pr_info("T0 %d\r\n",  VIOC_TIMER_GetCurTime(vioc_timer));
	if (status & (1 << VIOC_TIMER_IRQ_TIMER1))
		pr_info("T1 %d\r\n",  VIOC_TIMER_GetCurTime(vioc_timer));
	if (status & (1 << VIOC_TIMER_IRQ_TIREQ0)) {
		boot_time_ms_gab = get_timer(previous_time[2]);
		pr_info(
			"TIREQ0 - %d with %dms\r\n",
			VIOC_TIMER_GetCurTime(vioc_timer), boot_time_ms_gab);
	}
	if (status & (1 << VIOC_TIMER_IRQ_TIREQ1)) {
		boot_time_ms_gab = get_timer(previous_time[2]);
		pr_info(
			"TIREQ1 - %d with %dms\r\n",
			VIOC_TIMER_GetCurTime(vioc_timer), boot_time_ms_gab);
	}
	VIOC_TIMER_ClearIrqStatus(vioc_timer, status);
}

static void tcc_tav_timer_interrupt_disable(void)
{
	void __iomem *vioc_timer = VIOC_TIMER_GetAddress();

	tcc_vioc_irq_free_handler(timer_irq, timer_intr);
	VIOC_TIMER_ClearIrqStatus(vioc_timer, 0xF);
}

static void tcc_tav_timer_interrupt_enable(void)
{
	void __iomem *vioc_timer = VIOC_TIMER_GetAddress();

	if (vioc_irq_init == 0) {
		vioc_irq_init = 1;
		vioc_intr_initialize();
	}
	VIOC_TIMER_ClearIrqStatus(vioc_timer, 0xF);
	tcc_vioc_irq_install_handler(
		timer_irq, timer_intr, tcc_tav_timer_handler, &timer_intr);
}

int tcc_tav_db15(int test_case)
{
	unsigned int boot_time_ms, previous_timer_ms_0;
	unsigned int boot_time_ms_gab, current_timer_ms_0;
	void __iomem *vioc_timer = VIOC_TIMER_GetAddress();

	pr_info("[INF][DB15] %s - 0x%p\r\n", __func__, vioc_timer);
	switch (test_case) {
	case 1:
		pr_info("[INF][DB15] [%d] VIOC Timer - 100us \r\n", test_case);

		/* 100 us */
		VIOC_TIMER_SetUsecEnable(vioc_timer, 1, 100);

		boot_time_ms = get_timer(0);
		previous_timer_ms_0 = VIOC_TIMER_GetCurTime(vioc_timer);
		do {
			boot_time_ms_gab = get_timer(boot_time_ms);
			current_timer_ms_0 = VIOC_TIMER_GetCurTime(vioc_timer);
		} while (boot_time_ms_gab < 1000);

		pr_info(
			"[INF][DB15] TestCase 0 - Result is %d \r\n",
			current_timer_ms_0 - previous_timer_ms_0);

		break;
	case 2:
		pr_info("[INF][DB15] [%d] VIOC Timer - 200us \r\n", test_case);

		/* 1ms */
		VIOC_TIMER_SetUsecEnable(vioc_timer, 1, 200);

		boot_time_ms = get_timer(0);
		previous_timer_ms_0 = VIOC_TIMER_GetCurTime(vioc_timer);
		do {
			boot_time_ms_gab = get_timer(boot_time_ms);
			current_timer_ms_0 = VIOC_TIMER_GetCurTime(vioc_timer);
		} while (boot_time_ms_gab < 1000);

		pr_info(
			"[INF][DB15] TestCase 0 - Result is %d \r\n",
			current_timer_ms_0 - previous_timer_ms_0);

		break;
	case 3:
		pr_info(
			"[INF][DB15] [%d] VIOC Timer - TIMER 1 1000Hz and 500Hz \r\n",
			test_case);
		VIOC_TIMER_SetUsecEnable(vioc_timer, 0, 0);
		mdelay(1);
		VIOC_TIMER_SetTimer(
			vioc_timer, VIOC_TIMER_IRQ_TIMER0, 1, 24000);
		VIOC_TIMER_SetTimer(
			vioc_timer, VIOC_TIMER_IRQ_TIMER1, 1, 48000);

		tcc_tav_timer_interrupt_enable();
		VIOC_TIMER_SetUsecEnable(vioc_timer, 1, 100);
		previous_time[0] = previous_time[1] = get_timer(0);
		mdelay(100); //23
		VIOC_TIMER_SetTimer(vioc_timer, VIOC_TIMER_IRQ_TIMER0, 0, 0);
		VIOC_TIMER_SetTimer(vioc_timer, VIOC_TIMER_IRQ_TIMER1, 0, 0);
		VIOC_TIMER_SetUsecEnable(vioc_timer, 0, 0);
		tcc_tav_timer_interrupt_disable();
		break;

	case 4:
		pr_info(
			"[INF][DB15] [%d] VIOC Timer - TIMER 1 2000Hz and 500Hz \r\n",
			test_case);
		VIOC_TIMER_SetUsecEnable(vioc_timer, 0, 0);
		mdelay(1);
		VIOC_TIMER_SetTimer(
			vioc_timer, VIOC_TIMER_IRQ_TIMER0, 1, 12000);
		VIOC_TIMER_SetTimer(
			vioc_timer, VIOC_TIMER_IRQ_TIMER1, 1, 48000);

		tcc_tav_timer_interrupt_enable();
		VIOC_TIMER_SetUsecEnable(vioc_timer, 1, 100);
		previous_time[0] = previous_time[1] = get_timer(0);
		mdelay(100); //23
		VIOC_TIMER_SetTimer(vioc_timer, VIOC_TIMER_IRQ_TIMER0, 0, 0);
		VIOC_TIMER_SetTimer(vioc_timer, VIOC_TIMER_IRQ_TIMER1, 0, 0);
		VIOC_TIMER_SetUsecEnable(vioc_timer, 0, 0);
		tcc_tav_timer_interrupt_disable();
		break;

	case 5:
		// TIMERQ 0
		pr_info(
			"[INF][DB15] [%d] VIOC Timer - TIREQ 1000 and 2000 \r\n",
			test_case);

		/* Disable */
		for (; timer_irq_first; timer_irq_first--) {
			VIOC_TIMER_SetUsecEnable(vioc_timer, 0, 0);
			mdelay(1);
			VIOC_TIMER_SetTiReq(
				vioc_timer, VIOC_TIMER_IRQ_TIREQ0, 1, 1000);
			VIOC_TIMER_SetTiReq(
				vioc_timer, VIOC_TIMER_IRQ_TIREQ1, 1, 2000);

			tcc_tav_timer_interrupt_enable();
			VIOC_TIMER_SetUsecEnable(vioc_timer, 1, 100);
			previous_time[2] = get_timer(0);
			mdelay(1000);
			VIOC_TIMER_SetTiReq(
				vioc_timer, VIOC_TIMER_IRQ_TIREQ0, 0, 0);
			VIOC_TIMER_SetTiReq(
				vioc_timer, VIOC_TIMER_IRQ_TIREQ1, 0, 0);
			VIOC_TIMER_SetUsecEnable(vioc_timer, 0, 0);
			tcc_tav_timer_interrupt_disable();
		}
		timer_irq_first = 1;
		break;
	}

	return 0;
}
