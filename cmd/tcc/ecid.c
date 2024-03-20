// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <console.h>
#include <asm/io.h>
#include <mach/smc.h>
#include <asm/system.h>
#include <irq.h>
//#include <asm/telechips/irqs.h>

#define ECID_CON 0x14200290
#define ECID_PD0 0x14200298 // [31:0]
#define ECID_PD1 0x1420029c // [63:32]

#define PMU_USR0_BASE 0x144000E8
#define PMU_SEC_BASE 0x144000F0
#define PMU_USR1_BASE 0x144000F8
#define PMU_UNIQUE_ID_BASE 0x14400100

#define MODE 31		// mode enable
#define CS 30		// chip select enable
#define FSET 29		// signal to enable sense amplifier
#define PRCHG 27	// signal to pre-charge reference nodes of
			//sense amplifier
#define PROG 26		// enable signal to program fuse selected by SDI shift
#define SCK 25		// scan mode clock
#define SDI 24		// the data scan-in port of the fuse latch
#define SIGDEV 23	// signal to open up both fuse and reference nodes to
			// the sense amplifier
#define A 17		// signal to select sensing bit divided by 8 in
			//sensing operation
#define SEL 14		// 000 : User0 (Temp-Sensor)
			// 001 : SEC
			// 010 : User1 (Mac Address)
			// 011 : Unique-ID
			// 111 : CA53
#define FB_GRP_EN 13
#define FBS 12

static int do_read_ecid(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	uintptr_t ecid_con_reg = ECID_CON;
	uintptr_t ecid_pd0 = ECID_PD0;
	uintptr_t ecid_pd1 = ECID_PD1;
	int ecid_num = 0;
	int ecid_addr = 0;
	unsigned int ecid_data_parallel[4][2];
	unsigned int pmu_data_parallel[4][2];

	writel((1 << MODE), ecid_con_reg);
	writel((1 << MODE | 1 << CS), ecid_con_reg);

	for (ecid_num = 0; ecid_num < 4; ecid_num++) {
		// 0: USER0, 1: SEC, 2:USR1, 3:SEC
		writel((1 << MODE | 1 << CS | ecid_num << SEL), ecid_con_reg);

		for (ecid_addr = 0; ecid_addr < 8; ecid_addr++) {
			writel((1 << MODE | 1 << CS | ecid_addr << A |
					ecid_num << SEL), ecid_con_reg);
			writel((1 << MODE | 1 << CS | ecid_addr << A |
					ecid_num << SEL | 1 << SIGDEV),
					ecid_con_reg);
			writel((1 << MODE | 1 << CS | ecid_addr << A |
					ecid_num << SEL | 1 << SIGDEV |
					1 << PRCHG), ecid_con_reg);
			writel((1 << MODE | 1 << CS | ecid_addr << A |
					ecid_num << SEL | 1 << SIGDEV |
					1 << PRCHG | 1 << FSET), ecid_con_reg);
			writel((1 << MODE | 1 << CS | ecid_addr << A |
					ecid_num << SEL | 1 << PRCHG  |
					1 << FSET), ecid_con_reg);
			writel((1 << MODE | 1 << CS | ecid_addr << A |
					ecid_num << SEL | 1 << FSET),
					ecid_con_reg);
			writel((1 << MODE | 1 << CS | ecid_addr << A |
					ecid_num << SEL), ecid_con_reg);
		}
		// High 16 Bit
		ecid_data_parallel[ecid_num][1] = readl(ecid_pd1);
		// Low 16 Bit
		ecid_data_parallel[ecid_num][0] = readl(ecid_pd0);
		writel(readl(ecid_con_reg) & ~((0x7) << SEL), ecid_con_reg);
		writel(readl(ecid_con_reg) & ~((0) << PRCHG), ecid_con_reg);

		pr_force("ECID[%d] Parallel Read = 0x%04X%08X\n",
			 ecid_num, ecid_data_parallel[ecid_num][1],
			 ecid_data_parallel[ecid_num][0]);
	}

	pmu_data_parallel[0][1] = readl(PMU_USR0_BASE + 4) & 0xFFFF;
	pmu_data_parallel[0][0] = readl(PMU_USR0_BASE);
	pmu_data_parallel[1][1] = readl(PMU_SEC_BASE + 4) & 0xFFFF;
	pmu_data_parallel[1][0] = readl(PMU_SEC_BASE);
	pmu_data_parallel[2][1] = readl(PMU_USR1_BASE + 4) & 0xFFFF;
	pmu_data_parallel[2][0] = readl(PMU_USR1_BASE);
	pmu_data_parallel[3][1] = readl(PMU_UNIQUE_ID_BASE + 4) & 0xFFFF;
	pmu_data_parallel[3][0] = readl(PMU_UNIQUE_ID_BASE);

	for (ecid_num = 0; ecid_num < 4; ecid_num++) {
		pr_force("PMU[%d] Parallel Read = 0x%04X%08X\n",
			 ecid_num, pmu_data_parallel[ecid_num][1],
			 pmu_data_parallel[ecid_num][0]);
	}
	return 0;
}

U_BOOT_CMD(ecid_read, 1, 1, do_read_ecid,
	   "Read ECID fusing data",
	   "");

