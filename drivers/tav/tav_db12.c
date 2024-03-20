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

#define FIFOADDR	0x12003B00U
#define FIFOBASE	0x48000000U

#define SOURCEADDR	0x40000000U
#define DESTADDR	0x45000000U
#define SOURCEADDR2	0x30000000U
#define DESTADDR2	0x35000000U

#define BUFFER_CLEAR	0U
#define FIFO_TEST	1U

struct tav_test_case_t db12[] = {
	/* Test Major 0 */
	{
		BUFFER_CLEAR, 0, "Enable Path",

		.rdma[0] = {
			.id = VIOC_RDMA04,
			.en = 1,
			.base[0] = SOURCEADDR,
			.base[1] = GET_ADDR_YUV42X_spU(DESTADDR2, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(DESTADDR2, 640, 360),
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
			.cont = 1,
			.fmt = VIOC_PXDW_FMT_24_RGB888,
			.width = 640,
			.height = 360,
			.base[0] = DESTADDR,
			.base[1] = 0,
			.base[2] = 0,
		},
		.rdma[1] = {
			.id = VIOC_RDMA08,
			.en = 1,
			.base[0] = SOURCEADDR2,
			.base[1] = GET_ADDR_YUV42X_spU(SOURCEADDR2, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(SOURCEADDR2, 640, 360),
			.fmt = VIOC_PXDW_FMT_24_RGB888,
			.width = 640,
			.height = 360,
			.bypass = 0,
		},
		.wmix[1] = {
			.id = VIOC_WMIX2,
			.bg = {0x00, 0x00, 0x00, 0x00}, /* RED */
			.width = 640,
			.height = 360,
			.ovp = 24,
			.pos[0] = {0, 0},
		},
		.wdma[1] = {
			.id = VIOC_WDMA02,
			.cont = 1,
			.fmt = VIOC_PXDW_FMT_24_RGB888,
			.width = 640,
			.height = 360,
			.base[0] = DESTADDR2,
			.base[1] = 0,
			.base[2] = 0,
		},
		.fifo = {
			.id = VIOC_FIFO,
			.ch = 0,
			.wdma_id = VIOC_WDMA02,
			.rdma_id = VIOC_RDMA04,
			.eempty = 2,
			.efull = 1,
			.nentry = 4,
			.base[0] = FIFOBASE,
			.base[1] = FIFOBASE + 1000000,
			.base[2] = FIFOBASE + 2000000,
			.base[3] = FIFOBASE + 3000000,
		}
	},

	{
		FIFO_TEST, 0, "Frame FIFO",

		// PATH 1 - RDMA 10 ~ WMIX 2 ~ WDMA 2
		.rdma[0] = {
			.id = VIOC_RDMA04,
			.en = 1,
			.base[0] = SOURCEADDR,
			.base[1] = GET_ADDR_YUV42X_spU(DESTADDR2, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(DESTADDR2, 640, 360),
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
			.cont = 1,
			.fmt = VIOC_PXDW_FMT_24_RGB888,
			.width = 640,
			.height = 360,
			.base[0] = DESTADDR,
			.base[1] = 0,
			.base[2] = 0,
		},
		.rdma[1] = {
			.id = VIOC_RDMA08,
			.en = 1,
			.base[0] = SOURCEADDR2,
			.base[1] = GET_ADDR_YUV42X_spU(SOURCEADDR2, 640, 360),
			.base[2] = GET_ADDR_YUV422_spV(SOURCEADDR2, 640, 360),
			.fmt = VIOC_PXDW_FMT_24_RGB888,
			.width = 640,
			.height = 360,
			.bypass = 0,
		},
		.wmix[1] = {
			.id = VIOC_WMIX2,
			.bg = {0x00, 0x00, 0x00, 0x00}, /* RED */
			.width = 640,
			.height = 360,
			.ovp = 24,
			.pos[0] = {0, 0},
		},
		.wdma[1] = {
			.id = VIOC_WDMA02,
			.cont = 1,
			.fmt = VIOC_PXDW_FMT_24_RGB888,
			.width = 640,
			.height = 360,
			.base[0] = DESTADDR2,
			.base[1] = 0,
			.base[2] = 0,
		},
		.fifo = {
			.id = VIOC_FIFO,
			.ch = 0,
			.wdma_id = VIOC_WDMA02,
			.rdma_id = VIOC_RDMA04,
			.eempty = 2,
			.efull = 1,
			.nentry = 4,
			.base[0] = FIFOBASE,
			.base[1] = FIFOBASE + 1000000,
			.base[2] = FIFOBASE + 2000000,
			.base[3] = FIFOBASE + 3000000,
		}
	},
};

static int ImageCompare(unsigned long RefAddr, unsigned long DestAddr)
{
	int i;
	unsigned int count = 0;

	pr_force(
		"#### RefAddr = %p, DestAddr = %p\n",
		(void *)RefAddr, (void *)DestAddr);

	for (i = 0; i < (640 * 360 * 4); i += 4) {
		if ((readl(RefAddr + i)) != (readl(DestAddr + i)))
			count++;
	}

	pr_force("count = %d of %d\n", count, i / 4);
	return count;
}

static void TestResult(unsigned int errorCount)
{
	if (errorCount == 0)
		pr_force("Test Success\n");
	else
		pr_force("Test Failed (count is %d)\n", errorCount);
}


int tcc_tav_db12(int test_case)	//test_case == 0
{
	int test_case_count;
	int result;
	long i;

	test_case_count = sizeof(db12)/sizeof(struct tav_test_case_t);
	pr_force(
		"[INF][DB06]%s - all test case count is (%d) - test_case is (%d)\r\n",
		__func__, test_case_count, test_case);

	// buffer clear
	for (i = 0; i < ((640 * 360 * 4)); i++)
		writel(0x0, SOURCEADDR + 4 * i);
	flush_dcache_all();

	//Draw Image
	for (i = 0; i < (640 * 360 * 4); i++)
		writel((i) + 0xFF111111, SOURCEADDR2 + (i) * 4);
	flush_dcache_all();

	for (i = 0; i < test_case_count; i++) {
		if (test_case !=  db12[i].major)
			continue;
		pr_force(
			"[INF][DB12] Major_%d Minor_%d name=%s , Test_case = %x \r\n",
			db12[i].major, db12[i].minor,
			db12[i].test_name, test_case);
		switch (test_case) {
		case BUFFER_CLEAR:
			tcc_tav_vioc_setup(&db12[i]);
			flush_dcache_all();
			mdelay(600);
			break;
		case FIFO_TEST:
			tcc_tav_vioc_setup(&db12[i]);
			flush_dcache_all();
			mdelay(600);
			tcc_tav_vioc_fifo_setup(&db12[i].fifo);
			flush_dcache_all();
			mdelay(600);
			pr_force("0x%x\n", readl(DESTADDR));
			result = ImageCompare(SOURCEADDR2, DESTADDR);
			TestResult(result);
		break;

		default:
			tcc_tav_vioc_setup(&db12[i]);
		break;
		}
	}
	return 0;
}
