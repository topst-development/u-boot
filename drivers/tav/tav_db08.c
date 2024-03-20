// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <tav_vioc.h>

#include <asm/telechips/vioc/vioc_global.h>
#include <cpu_func.h>
#include <common.h>
#include <asm/io.h>
#include <asm/telechips/vioc/reg_physical.h>
#include <asm/telechips/vioc/vioc_blk.h>
#include <asm/telechips/vioc/vioc_global.h>
#include <asm/telechips/vioc/vioc_config.h>
#include <asm/telechips/vioc/vioc_scaler.h>
#include <asm/telechips/vioc/vioc_ddicfg.h>
#include <asm/telechips/vioc/vioc_viqe.h>
#include <asm/telechips/vioc/vioc_fifo.h>
#include <asm/telechips/vioc/vioc_wdma.h>

#define FDLYBASE0	0x40000000U
#define FDLYBASE1	0x45000000U

#define SOURCEADDR	0x30000000U
#define DESTADDR	0x35000000U

#define BUFFER_CLEAR	0U
#define FRAME_DELAY	1U
struct tav_test_case_t db08[] = {
	/* Test Major 0 */
	{
		BUFFER_CLEAR, 0, "Enable Path",
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
			.bg = {0x00, 0x00, 0x00, 0x00}, /* RED */
			.width = 640,
			.height = 360,
			.ovp = 24,
			.pos[0] = {0, 0},
		},
		.wdma[0] = {
			.id = VIOC_WDMA01,
			.cont = 0,
			.fmt = VIOC_PXDW_FMT_24_RGB888,
			.width = 640,
			.height = 360,
			.base[0] = DESTADDR,
			.base[1] = 0,
			.base[2] = 0,
		},
		.scaler = {
			.id = VIOC_SCALER1,
			.plugin = VIOC_RDMA04,
			.bypass = 1,
		},
		.fdly = {
			.scaler_id = VIOC_SCALER1,
			.fmt = 1,
			.ren = 1,
			.base0 = FDLYBASE0,
			.base1 = FDLYBASE1,
		}
	},

	{
		FRAME_DELAY, 0, "FRAME_DELAY",
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
			.bg = {0x00, 0x00, 0x00, 0x00},
			.width = 640,
			.height = 360,
			.ovp = 24,
			.skip_swreset = 1,
			.pos[0] = {0, 0},
		},
		.wdma[0] = {
			.id = VIOC_WDMA01,
			.cont = 0,
			.fmt = VIOC_PXDW_FMT_24_RGB888,
			.width = 640,
			.height = 360,
			.base[0] = DESTADDR,
			.base[1] = 0,
			.base[2] = 0,
		},

		.scaler = {
			.id = VIOC_SCALER1,
			.plugin = VIOC_RDMA04,
			.bypass = 1,
		},

		.fdly = {
			.scaler_id = VIOC_SCALER1,
			.fmt = 1,
			.ren = 1,
			.base0 = FDLYBASE0,
			.base1 = FDLYBASE1,
			#if 1
			.bg0 = 0x11,
			.bg1 = 0x22,
			.bg2 = 0x33,
			.bg3 = 0xFF,
			#endif
			//set background color default
		}
	},
};

int tcc_tav_db08(int test_case)
{
	long i;
	int test_case_count;

	test_case_count = sizeof(db08) / sizeof(struct tav_test_case_t);
	pr_force(
		"[INF][DB06]%s - all test case count is (%d) - test_case is (%d)\r\n",
		__func__, test_case_count, test_case);

	for (i = 0; i < ((640 * 360 * 4)); i++)
		writel(0xEEEEEEEE, SOURCEADDR + 4 * i);
	flush_dcache_all();

	for (i = 0; i < test_case_count; i++) {
		if (test_case != db08[i].major)
			continue;
		pr_force(
			"[INF][db08] Major_%d Minor_%d name=%s , Test_case = %x \r\n",
			db08[i].major, db08[i].minor,
			db08[i].test_name, test_case);

		switch (test_case) {
		case BUFFER_CLEAR:
			tcc_tav_vioc_setup(&db08[i]);
			flush_dcache_all();
			mdelay(600);
			break;
		case FRAME_DELAY:
			tcc_tav_vioc_scaler_setup(&db08[i].scaler);
			tcc_tav_vioc_fdly_setup(&db08[i].fdly);
			tcc_tav_vioc_setup(&db08[i]);
			flush_dcache_all();
			mdelay(600);

			if (readl(DESTADDR) == 0xFF112233)
				pr_force("Test done\n");
			else
				pr_force("Test fail\n");
			break;

		default:
				tcc_tav_vioc_setup(&db08[i]);
			break;
		}
	}

	return 0;
}

