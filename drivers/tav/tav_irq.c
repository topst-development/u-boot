// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/telechips/vioc/vioc_rdma.h>
#include <asm/telechips/vioc/vioc_intr.h>
#include <asm/telechips/vioc/vioc_config.h>
#include <asm/telechips/vioc/vioc_rdma.h>
#include <asm/telechips/vioc/vioc_wdma.h>
#include <asm/telechips/vioc/vioc_disp.h>
#include <asm/telechips/vioc/vioc_global.h>

int vioc_irq_init;
static int sample_irq = INT_VIOC0;
static int sample_intr = VIOC_INTR_DEV0;
static int sample_raw_irq = VIOC_DISP_INTR_RU;

static void tcc_vioc_disp_handler(void *data)
{
	int *intr = (int *)data;
	unsigned int vioc_irq_status;

	vioc_irq_status = vioc_intr_get_status(*intr);

	if (vioc_irq_status & (1 << sample_raw_irq))
		pr_info("RU");
	vioc_intr_clear(*intr, vioc_irq_status);
}

static void tcc_tav_interrupt_disable(void)
{
	tcc_vioc_irq_free_handler(sample_irq, sample_intr);
	vioc_intr_clear(sample_intr, 0xFF);
	vioc_intr_disable(sample_irq, sample_intr, sample_raw_irq);
}

static void tcc_tav_interrupt_enable(void)
{
	if (vioc_irq_init == 0) {
		vioc_irq_init = 1;
		vioc_intr_initialize();
	}
	vioc_intr_clear(sample_intr, 0xFF);
	tcc_vioc_irq_install_handler(
		sample_irq, sample_intr, tcc_vioc_disp_handler, &sample_intr);
	vioc_intr_enable(sample_irq, sample_intr, sample_raw_irq);
}

int tcc_tav_irq(int enable)
{
	if (enable)
		tcc_tav_interrupt_enable();
	else
		tcc_tav_interrupt_disable();
	return 0;
}
