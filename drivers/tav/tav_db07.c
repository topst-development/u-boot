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

#define SCALE_DOWN	0U
#define SCALE_UP	1U
#define MAX_RESOLUTION	2U

struct tav_test_case_t db07[] = {
	/* Test Major 0 */
	{
		SCALE_DOWN,  0,  "ScaleDown",
		.rdma[0] = {
			.id = VIOC_RDMA04,
			.en = 1,
			.base[0] = SOURCEADDR,
			.base[1] = GET_ADDR_YUV42X_spU(SOURCEADDR, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(SOURCEADDR, 640, 360),
			.fmt = VIOC_IMG_FMT_ARGB8888,
			.width = 640,
			.height = 360,
			.bypass = 0,
		},
		.wmix[0] = {
			.id = VIOC_WMIX1,
			.bg = {0xFF, 0x00, 0x00, 0xFF}, /* RED */
			.width = 640,
			.height = 360,
			.ovp = 24,
			.pos[0] = {0, 0},
		},
		.wdma[0] = {
			.id = VIOC_WDMA01,
			.cont = 1,
			.fmt = VIOC_IMG_FMT_ARGB8888,
			.width = 320,
			.height = 180,
			.base[0] = DESTADDR,
			.base[1] = 0,
			.base[2] = 0,
		},

		.scaler = {
			.id = VIOC_SCALER1,
			.plugin = VIOC_RDMA04,
			.bypass = 0,

			.src_height = 360,
			.src_width = 640,

			.dst_height = 180,
			.dst_width = 320,

			.ypos = 0,
			.xpos = 0,
		},

		.rdma[1] = {
			.id = VIOC_RDMA01,
			.en = 1,
			.base[0] = DESTADDR,
			.base[1] = GET_ADDR_YUV42X_spU(DESTADDR, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(DESTADDR, 640, 360),
			.fmt = VIOC_IMG_FMT_ARGB8888,
			.width = 320,
			.height = 180,
		},
		#if 1
		.wmix[1] = {
			.id = VIOC_WMIX0,
			.bg = {0xFF, 0x00, 0x00, 0xFF}, /* RED */
			.ovp = 20,
			.width = 320,
			.height = 180,
			.skip_swreset = 1,
		},
		#endif
	},
	/* Test Major 1 */
	{
		SCALE_UP,  0,  "ScaleUp ",
		.rdma[0] = {
			.id = VIOC_RDMA04,
			.en = 1,
			.base[0] = SOURCEADDR,
			.base[1] = GET_ADDR_YUV42X_spU(SOURCEADDR, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(SOURCEADDR, 640, 360),
			.fmt = VIOC_IMG_FMT_ARGB8888,
			.width = 640,
			.height = 360,
			.bypass = 0,
		},
		.wmix[0] = {
			.id = VIOC_WMIX1,
			.bg = {0xFF, 0x00, 0x00, 0xFF}, /* RED */
			.width = 1280,
			.height = 720,
			.ovp = 24,
			.pos[0] = {0, 0},
		},
		.wdma[0] = {
			.id = VIOC_WDMA01,
			.cont = 1,
			.fmt = VIOC_IMG_FMT_ARGB8888,
			.width = 1280,
			.height = 720,
			.base[0] = DESTADDR,
			.base[1] = 0,
			.base[2] = 0,
		},

		.scaler = {
			.id = VIOC_SCALER1,
			.plugin = VIOC_RDMA04,
			.bypass = 0,

			.src_height = 360,
			.src_width = 640,

			.dst_height = 720,
			.dst_width = 1280,

			.ypos = 0,
			.xpos = 0,
		},

		.rdma[1] = {
			.id = VIOC_RDMA01,
			.en = 1,
			.base[0] = DESTADDR,
			.base[1] = GET_ADDR_YUV42X_spU(DESTADDR, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(DESTADDR, 640, 360),
			.fmt = VIOC_IMG_FMT_ARGB8888,
			.width = 1280,
			.height = 720,
		},
		#if 1
		.wmix[1] = {
			.id = VIOC_WMIX0,
			.bg = {0xFF, 0x00, 0x00, 0xFF}, /* RED */
			.ovp = 20,
			.width = 1280,
			.height = 720,
			.skip_swreset = 1,
		},
		#endif
	},
	/* Test Major 2 */
	{
		MAX_RESOLUTION,  0,  "Max Resolution ",
		.rdma[0] = {
			.id = VIOC_RDMA04,
			.en = 1,
			.base[0] = SOURCEADDR,
			.base[1] = GET_ADDR_YUV42X_spU(SOURCEADDR, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(SOURCEADDR, 640, 360),
			.fmt = VIOC_IMG_FMT_ARGB8888,
			.width = 640,
			.height = 360,
			.bypass = 0,
		},
		.wmix[0] = {
			.id = VIOC_WMIX1,
			.bg = {0xFF, 0x00, 0x00, 0xFF}, /* RED */
			.width = 1920,
			.height = 1080,
			.ovp = 24,
			.pos[0] = {0, 0},
		},
		.wdma[0] = {
			.id = VIOC_WDMA01,
			.cont = 1,
			.fmt = VIOC_IMG_FMT_ARGB8888,
			.width = 1920,
			.height = 1080,
			.base[0] = DESTADDR,
			.base[1] = 0,
			.base[2] = 0,
		},

		.scaler = {
			.id = VIOC_SCALER1,
			.plugin = VIOC_RDMA04,
			.bypass = 0,

			.src_height = 360,
			.src_width = 640,

			.dst_height = 1080,
			.dst_width = 1920,

			.ypos = 0,
			.xpos = 0,
		},

		.rdma[1] = {
			.id = VIOC_RDMA01,
			.en = 1,
			.base[0] = DESTADDR,
			.base[1] = GET_ADDR_YUV42X_spU(DESTADDR, 1920, 1080),
			.base[2] = GET_ADDR_YUV422_spV(DESTADDR, 1920, 1080),
			.fmt = VIOC_IMG_FMT_ARGB8888,
			.width = 1920,
			.height = 1080,
		},
		#if 1
		.wmix[1] = {
			.id = VIOC_WMIX0,
			.bg = {0xFF, 0x00, 0x00, 0xFF}, /* RED */
			.ovp = 20,
			.width = 1920,
			.height = 1080,
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

	pr_info("#### RefAddr = %4x, DestAddr = %4x\n", RefAddr, DestAddr);

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

int tcc_tav_db07(int test_case)
{
	int test_case_count = sizeof(db07) / sizeof(struct tav_test_case_t);

	pr_info(
		"[INF][db07]%s - all test case count is (%d) - test_case is (%d)\r\n",
		__func__, test_case_count, test_case);

	switch (test_case) {
	case SCALE_DOWN: //Success
		tcc_tav_vioc_setup(&db07[test_case]);
		break;
	case SCALE_UP: //Success
		tcc_tav_vioc_setup(&db07[test_case]);
		break;
	case MAX_RESOLUTION: //Success
		tcc_tav_vioc_setup(&db07[test_case]);
		break;
	}
	return 0;
}
