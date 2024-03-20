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

#define WDMA_FORMAT 0
#define WDMA_Y2R 1

static unsigned int wdma_format[] = {
	0, 1, 2, 3, 8, 9, 10, 11, 12, 13, 14, 15, 21, 22, 23, 24, 25,
	26, 27, 28, 29, 30, 31};
static unsigned int wdma_format_r2y[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static unsigned int RDMA_SWAP_SIZE[] = {
	450, 450, 338, 450, 450, 450, 338, 338, 450, 450};

struct tav_test_case_t db06[] = {
	/* Test Major 0 */
	{
		WDMA_FORMAT,  0,  "WDMA_FORMAT ",
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
			.width = 640,
			.height = 360,
			.ovp = 24,
			.pos[0] = {0, 0},
		},
		.wdma[0] = {
			.id = VIOC_WDMA01,
			.cont = 1,
			.fmt = VIOC_PXDW_FMT_24_RGB888,
			.width = 640, .height = 360,
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
			.width = 640,
			.height = 360,
			.skip_swreset = 1,
		},
		#endif
	},
	/* Test Major 1 */
	{
		WDMA_Y2R,  0,  "WDMA_Y2R ",
		.rdma[0] = {
			.id = VIOC_RDMA04,
			.en = 1,
			.base[0] = SOURCEADDR,
			.base[1] = GET_ADDR_YUV42X_spU(SOURCEADDR, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(SOURCEADDR, 640, 360),
			.fmt = VIOC_IMG_FMT_UYVY,
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
			.width = 640,
			.height = 360,
			.skip_swreset = 1,
		},
		#endif
	},

};

int tcc_tav_db06(int test_case)	//test_case == 0
{
	int j;

	switch (test_case) {
	case WDMA_FORMAT:	//WDMA Format Success
		for (
			j = VIOC_IMG_FMT_BPP1;
			j < sizeof(wdma_format) / sizeof(unsigned int); j++) {
			db06[test_case].wdma[0].fmt = wdma_format[j];
			db06[test_case].wdma[0].r2y = wdma_format_r2y[j];

			db06[test_case].rdma[1].y2r = wdma_format_r2y[j];
			tcc_tav_vioc_setup(&db06[test_case]);
			mdelay(500);
		}
		break;
	case WDMA_Y2R:	//WDMA Y2R Success
		for (j = 0; j < 10; j++) {
			db06[test_case].rdma[0].fmt = VIOC_IMG_FMT_UYVY + j;

			//FIXME
			db06[test_case].wdma[0].y2r = 1;
			db06[test_case].wdma[0].fmt = 12;
			db06[test_case].rdma[1].fmt = 12;

			tcc_tav_vioc_setup(&db06[test_case]);

			db06[test_case].rdma[0].base[0] +=
				(RDMA_SWAP_SIZE[j] * 1024);
			mdelay(800);
		}
		break;
	}
	return 0;
}
