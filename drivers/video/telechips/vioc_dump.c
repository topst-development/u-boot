// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#include <common.h>
#include <asm/io.h>
#include <asm/telechips/vioc/reg_physical.h>
#include <asm/telechips/vioc/vioc_global.h>
#include <asm/telechips/vioc/vioc_disp.h>
#include <asm/telechips/vioc/vioc_rdma.h>
#include <asm/telechips/vioc/vioc_wmix.h>
#include <asm/telechips/vioc/vioc_scaler.h>
#include <asm/telechips/vioc/vioc_config.h>

#include "vioc_dump.h"

void tcc_capture_viocs(unsigned int disp_num, struct vioc_dumps *dumps)
{
	void __iomem *pvioc_base;
	unsigned long vioc_base;
	int i, j;

	memset(dumps, 0, sizeof(*dumps));

	/* dump rdmas */
	vioc_base = TCC_VIOC_RDMA_BASE(disp_num * 4);
	for (i = 0; i < 4; i++) {
		pvioc_base = (void __iomem *)vioc_base;
		if ((disp_num == 3) && (i > 2)) {
			break;
		}
		for (j = 0; j < 8; j++) {
			dumps->rdma[i].d[j] = readl(pvioc_base + (j << 2));
		}
		vioc_base += RDMA_OFFSET;
	}

	/* dump wmix */
	vioc_base = TCC_VIOC_WMIX_BASE(disp_num);
	pvioc_base = (void __iomem *)vioc_base;
	for (j = 0; j < 8; j++) {
		dumps->wmix.d[j] = readl(pvioc_base + (j << 2));
	}

	/* dump disp */
	vioc_base = TCC_VIOC_DISP_BASE(disp_num);
	pvioc_base = (void __iomem *)vioc_base;
	for (j = 0; j < 32; j++) {
		dumps->disp.d[j] = readl(pvioc_base + (j << 2));
	}
	vioc_disp_clean_fu_status(pvioc_base);

	/* dump wdma */
	vioc_base = TCC_VIOC_WDMA_BASE(disp_num);
	pvioc_base = (void __iomem *)vioc_base;
	for (j = 0; j < 8; j++) {
		dumps->wdma.d[j] = readl(pvioc_base + (j << 2));
	}

	/* dump scaler */
	vioc_base = TCC_VIOC_SC_BASE(0);
	for (i = 0; i < 8; i++) {
		pvioc_base = (void __iomem *)vioc_base;
		for (j = 0; j < 8; j++) {
			dumps->scaler[i].d[j] = readl(pvioc_base + (j << 2));
		}
		vioc_base += SC_OFFSET;
	}

	/* dump configs */
	vioc_base = HwVIOC_CONFIG;
	pvioc_base = (void __iomem *)vioc_base;
	for (j = 0; j < 304; j++) {
		dumps->config.d[j] = readl(pvioc_base + (j << 2));
	}
}

void tcc_dump_viocs(unsigned int disp_num, struct vioc_dumps *dumps)
{
	unsigned long vioc_base;
	int i, j;

	/* dump rdmas */
	pr_force("\r\n\r\nRDMA");
	vioc_base = TCC_VIOC_RDMA_BASE(disp_num * 4);
	for (i = 0; i < 4; i++) {
		if ((disp_num == 3) && (i > 2)) {
			break;
		}
		for (j = 0; j < 8; j++) {
			if ((j % 4) == 0) {
				pr_force("\r\n0x%08lx ", vioc_base + (j << 2));
			}
			pr_force(" 0x%08lx ", dumps->rdma[i].d[j]);
		}
		vioc_base += RDMA_OFFSET;
		pr_force("\r\n");
	}

	/* dump wmix */
	pr_force("\r\n\r\nWMIX");
	vioc_base = TCC_VIOC_WMIX_BASE(disp_num);
	for (j = 0; j < 8; j++) {
		if ((j % 4) == 0) {
			pr_force("\r\n0x%08lx ", vioc_base + (j << 2));
		}
		pr_force(" 0x%08lx ", dumps->wmix.d[j]);
	}

	/* dump disp */
	pr_force("\r\n\r\nDISP");
	vioc_base = TCC_VIOC_DISP_BASE(disp_num);
	for (j = 0; j < 32; j++) {
		if ((j % 4) == 0) {
			pr_force("\r\n0x%08lx ", vioc_base + (j << 2));
		}
		pr_force(" 0x%08lx ", dumps->disp.d[j]);
	}

	/* dump wdma */
	pr_force("\r\n\r\nWDMA");
	vioc_base = TCC_VIOC_WDMA_BASE(disp_num);
	for (j = 0; j < 8; j++) {
		if ((j % 4) == 0) {
			pr_force("\r\n0x%08lx ", vioc_base + (j << 2));
		}
		pr_force(" 0x%08lx ", dumps->wdma.d[j]);
	}

	/* dump scaler */
	pr_force("\r\n\r\nSCALER");
	vioc_base = TCC_VIOC_SC_BASE(0);
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if ((j % 4) == 0) {
				pr_force("\r\n0x%08lx ", vioc_base + (j << 2));
			}
			pr_force(" 0x%08lx ", dumps->scaler[i].d[j]);
		}
		vioc_base += SC_OFFSET;
	}

	/* dump configs */
	pr_force("\r\n\r\nCONFIGS");
	vioc_base = HwVIOC_CONFIG;
	for (j = 0; j < 304; j++) {
		if ((j % 4) == 0) {
			pr_force("\r\n0x%08lx ", vioc_base + (j << 2));
		}
		pr_force(" 0x%08lx ", dumps->config.d[j]);
	}
	pr_force("\r\n");
}
