// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef TCC_VIOC_DUMP_HEADER
#define TCC_VIOC_DUMP_HEADER

struct vioc_dump_dma {
	unsigned long d[8];
};
struct vioc_dump_wmix {
	unsigned long d[8];
};
struct vioc_dump_disp {
	unsigned long d[32];
};
struct vioc_dump_scaler {
	unsigned long d[8];
};
struct vioc_dump_config {
	unsigned long d[304];
};
struct vioc_dumps {
	struct vioc_dump_dma rdma[4];
	struct vioc_dump_wmix wmix;
	struct vioc_dump_disp disp;
	struct vioc_dump_dma wdma;
	struct vioc_dump_scaler scaler[8];
	struct vioc_dump_config config;
};

void tcc_capture_viocs(unsigned int disp_num, struct vioc_dumps *dumps);
void tcc_dump_viocs(unsigned int disp_num, struct vioc_dumps *dumps);
#endif

