// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <tav_vioc.h>

#include <asm/telechips/vioc/vioc_global.h>

#include <common.h>
#include <asm/io.h>
#include <asm/telechips/vioc/reg_physical.h>
#include <asm/telechips/vioc/vioc_blk.h>
#include <asm/telechips/vioc/vioc_global.h>
#include <asm/telechips/vioc/vioc_config.h>
#include <asm/telechips/vioc/vioc_scaler.h>
#include <asm/telechips/vioc/vioc_ddicfg.h>
#include <asm/telechips/vioc/vioc_viqe.h>

#define SOURCEADDR	0x40000000
#define DESTADDR	0x48000000
#define REFADDR		0x30000000

#define RDMA_SWAP 0U

struct tav_test_case_t db05[] = {
	/* Test Major 0 */
	{
		RDMA_SWAP,  0,  "RDMA_SWAP ",
		.rdma[0] = {
			.id = VIOC_RDMA04,
			.en = 1,
			.base[0] = SOURCEADDR,
			.base[1] = GET_ADDR_YUV42X_spU(SOURCEADDR, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(SOURCEADDR, 640, 360),
			.fmt = VIOC_PXDW_FMT_24_RGB888,
			.width = 640,
			.height = 360,
			.bypass = 0,
		},
		.wmix[0] = {
			.id = VIOC_WMIX1,
			.bg = {0xFF, 0x00, 0x00, 0xFF}, /* RED */
			.width = 640, .height = 360,
			.ovp = 24,
			.pos[0] = {0, 0},
		},
		.wdma[0] = {
			.id = VIOC_WDMA01,
			.cont = 1,
			.fmt = VIOC_PXDW_FMT_24_RGB888,
			.width = 640,
			.height = 360,
			.base[0] = DESTADDR,
			.base[1] = GET_ADDR_YUV42X_spU(DESTADDR, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(DESTADDR, 640, 360),
		},

		.scaler = {
			.id = VIOC_SCALER1,
			.plugin = VIOC_RDMA04,
			.bypass = 1,

			.src_height = 360,
			.src_width = 640,

			.dst_height = 360,
			.dst_width = 640,

			.ypos = 0,
			.xpos = 0,
		},

		.rdma[1] = {
			.id = VIOC_RDMA01,
			.en = 1,
			.base[0] = DESTADDR,
			.base[1] = GET_ADDR_YUV42X_spU(DESTADDR, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(DESTADDR, 640, 360),
			.fmt = VIOC_PXDW_FMT_24_RGB888,
			.width = 640,
			.height = 360,
		},
		#if 1
		.wmix[1] = {
			.id = VIOC_WMIX0,
			.bg = {0xFF, 0x00, 0x00, 0xFF}, /* RED */
			.ovp = 20,
			.width = 640, .height = 360,
			.skip_swreset = 1,
		},
		#endif
	},
};

#if defined(CONFIG_TAV_USES_CMP)
static int ImageCompare(
	unsigned long RefAddr, unsigned long DestAddr, unsigned long ImgSize)
{
	int i;
	unsigned int count = 0;

	pr_info(
		"#### RefAddr = %p, DestAddr = %p\n",
		(void *)RefAddr, (void *)DestAddr);

	for (i = 0; i < (ImgSize * 1024 / 4); i++) {
		if (
			(0xFF000000 | readl(RefAddr + i * 4)) !=
			readl(DestAddr + i * 4))
			count++;
	}

	pr_info("count = %d\n", count);
	return count;
}

static int TestResult(unsigned int errorCount)
{
	if (errorCount == 0)
		pr_info("Test Success\n");
	else
		pr_info("Test Failed (count is %d)\n", errorCount);
}
#endif

int tcc_tav_db05(int test_case)
{
	int j;

	switch (test_case) {
	case RDMA_SWAP:	//RDMA SWAP Success
		for (j = VIOC_SWAP_RGB; j < VIOC_SWAP_MAX; j++) {
			db05[test_case].rdma[0].swap = VIOC_SWAP_RGB + j;
			tcc_tav_vioc_setup(&db05[test_case]);
			mdelay(500);
		}
	break;
	}

	return 0;
}
