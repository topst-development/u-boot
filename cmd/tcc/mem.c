// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */


#include <common.h>
#include <console.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>
#endif
#include <hash.h>
#include <inttypes.h>
#include <mapmem.h>
#include <watchdog.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <mach/smc.h>
#include <linux/arm-smccc.h>
#include <mach/ddr_set.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_MEMTEST_SCRATCH
#define CONFIG_SYS_MEMTEST_SCRATCH 0
#endif

#define cal_svref(a)	soc_vref[a] * 10 + soc_vref[a]

#define WINDOW_MAX 100
#define SOC_MAX_VREF 0x3F
#define DRAM_MAX_VREF 0x32

#ifndef CONFIG_ARM64

#ifdef CONFIG_TCC897X
#define BASE_ADDRESS 0x90000000
#else
#define BASE_ADDRESS 0x40000000
#endif
#endif

enum {
	MR0,
	MR1,
	MR2,
	MR3,
	MR4,
	MR5,
	MR6,
	MR7,
	MR8,
	MR9,
	MR10,
	MR11,
	MR12,
	MR13,
	MR14,
	MR15,
	MR16,
	MR17,
	MR18,
	MR19,
	MR20,
	MR21,
	MR22,
	MR23,
	MR24,
	MR25,
	MR26,
	MR27,
	MR28,
	MR29,
	MR30,
	MR31,
	MR32,
	MR33,
	MR34,
	MR35,
	MR36,
	MR37,
	MR38,
	MR39,
	MR40,
};

static int do_pr_force_mc_info(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	unsigned int phy0_reg = 0x13310000;
	unsigned int phy1_reg = 0x13320000;
	unsigned int temp0, temp1, temp2, temp3;
	int i;

	/* Master Delay*/
	pr_force("\n 1. Master Delay\n");
	pr_force("================================================\n");
	pr_force("PHY0 Master Delay - 0x%x\n",
	       (readl(phy0_reg + 0xb4) & 0x3FF00) >> 8);
	pr_force("PHY1 Master Delay - 0x%x\n",
	       (readl(phy1_reg + 0xb4) & 0x3FF00) >> 8);
	pr_force("================================================\n");

	/* ZQ calibration result*/
	temp0 = readl(phy0_reg + 0x3cc);
	temp1 = readl(phy1_reg + 0x3cc);
	pr_force("\n 2. ZQ Calibration Result\n");
	pr_force("================================================\n");
	pr_force("PHY0 ZQ_pmon - 0x%x, ZQ_nmon - 0x%x, ZQ_Cal_fail - %d\n",
	       (temp0 & 0x1C0) >> 6, (temp0 & 0x38) >> 3, (temp0 & 0x2) >> 1);
	pr_force("PHY1 ZQ_pmon - 0x%x, ZQ_nmon - 0x%x, ZQ_Cal_fail - %d\n",
	       (temp1 & 0x1C0) >> 6, (temp1 & 0x38) >> 3, (temp1 & 0x2) >> 1);
	pr_force("================================================\n");
	/*Training fail status*/
	pr_force("\n 3. DQ Training Fail Status\n");
	pr_force("================================================\n");
	pr_force(" - Read Training\n");
	temp0 = readl(phy0_reg + 0xadc);
	temp1 = readl(phy0_reg + 0xae0);
	temp2 = readl(phy1_reg + 0xadc);
	temp3 = readl(phy1_reg + 0xae0);
	pr_force(
		" : DQ(PHY0) - 0x%04x, DM(PHY0) - 0x%x, DQ(PHY1) - 0x%04x, DM(PHY1) - 0x%x\n",
	       temp0, temp1, temp2, temp3);
	pr_force(" - Write Training CS0\n");
	temp0 = readl(phy0_reg + 0xae4);
	temp1 = readl(phy0_reg + 0xae8);
	temp2 = readl(phy1_reg + 0xae4);
	temp3 = readl(phy1_reg + 0xae8);
	pr_force(
		" : DQ(PHY0) - 0x%04x, DM(PHY0) - 0x%x, DQ(PHY1) - 0x%04x, DM(PHY1) - 0x%x\n",
	       temp0, temp1, temp2, temp3);
	pr_force(" - Write Training CS1\n");
	temp0 = readl(phy0_reg + 0xaec);
	temp1 = readl(phy0_reg + 0xaf0);
	temp2 = readl(phy1_reg + 0xaf4);
	temp3 = readl(phy1_reg + 0xaf8);
	pr_force(
		" : DQ(PHY0) - 0x%04x, DM(PHY0) - 0x%x, DQ(PHY1) - 0x%04x, DM(PHY1) - 0x%x\n",
	       temp0, temp1, temp2, temp3);

	pr_force("================================================\n");

	/*Read Calibration result*/
	pr_force("\n 4. Read DQ Training Result\n");
	pr_force("================================================\n");
	for (i = 0 ; i < 8 ; i++) {
		temp0 = readl(phy0_reg + 0x614 + (0xc*i)) & 0x1FF;
		temp1 = readl(phy0_reg + 0x190 + (0xc*i)) & 0x1FF;
		temp2 = (temp1 - temp0) + temp1;
		pr_force("DQ%d : Min = %d, Max = %d, Center = %d\n",
		       i, temp0, temp2, temp1);
	}
	temp0 = readl(phy0_reg + 0x610) & 0x1FF;
	temp1 = readl(phy0_reg + 0x18c) & 0x1FF;
	temp2 = (temp1 - temp0) + temp1;
	pr_force("DM  : Min = %d, Max = %d, Center = %d\n",
		 temp0, temp2, temp1);

	for (i = 0 ; i < 8 ; i++) {
		temp0 = readl(phy0_reg + 0x614 + (0xc*i)) >> 16;
		temp1 = readl(phy0_reg + 0x190 + (0xc*i)) >> 16;
		temp2 = (temp1 - temp0) + temp1;
		if (i == 0 || i == 1)
			pr_force("DQ%d : Min = %d, Max = %d, Center = %d\n",
			       i + 8, temp0, temp2, temp1);
		else
			pr_force("DQ%d: Min = %d, Max = %d, Center = %d\n",
			       i + 8, temp0, temp2, temp1);
	}
	temp0 = readl(phy0_reg + 0x610) >> 16;
	temp1 = readl(phy0_reg + 0x18c) >> 16;
	temp2 = (temp1 - temp0) + temp1;
	pr_force("DM  : Min = %d, Max = %d, Center = %d\n",
		 temp0, temp2, temp1);

	for (i = 0 ; i < 8 ; i++) {
		temp0 = readl(phy1_reg + 0x614 + (0xc*i)) & 0x1FF;
		temp1 = readl(phy1_reg + 0x190 + (0xc*i)) & 0x1FF;
		temp2 = (temp1 - temp0) + temp1;
		pr_force("DQ%d: Min = %d, Max = %d, Center = %d\n",
		       i + 16, temp0, temp2, temp1);
	}
	temp0 = readl(phy1_reg + 0x610) & 0x1FF;
	temp1 = readl(phy1_reg + 0x18c) & 0x1FF;
	temp2 = (temp1 - temp0) + temp1;
	pr_force("DM  : Min = %d, Max = %d, Center = %d\n",
		 temp0, temp2, temp1);

	for (i = 0 ; i < 8 ; i++) {
		temp0 = readl(phy1_reg + 0x614 + (0xc*i)) >> 16;
		temp1 = readl(phy1_reg + 0x190 + (0xc*i)) >> 16;
		temp2 = (temp1 - temp0) + temp1;
		pr_force("DQ%d: Min = %d, Max = %d, Center = %d\n",
		       i + 24, temp0, temp2, temp1);
	}
	temp0 = readl(phy1_reg + 0x610) >> 16;
	temp1 = readl(phy1_reg + 0x18c) >> 16;
	temp2 = (temp1 - temp0) + temp1;
	pr_force("DM  : Min = %d, Max = %d, Center = %d\n",
		 temp0, temp2, temp1);
	pr_force("================================================\n");
	/* Write Calibration result */
	pr_force("\n 5. Write DQ Training Result\n");
	pr_force("================================================\n");
	pr_force("CS0 Result -\n");
	for (i = 0 ; i < 8 ; i++) {
		temp0 = readl(phy0_reg + 0x490 + (0xc*i)) & 0x1FF;
		temp1 = readl(phy0_reg + 0x1f0 + (0xc*i)) & 0x1FF;
		temp2 = (temp1 - temp0) + temp1;
		pr_force("DQ%d : Min = %d, Max = %d, Center = %d\n",
		       i, temp0, temp2, temp1);
	}
	temp0 = readl(phy0_reg + 0x4f0) & 0x1FF;
	temp1 = readl(phy0_reg + 0x250) & 0x1FF;
	temp2 = (temp1 - temp0) + temp1;
	pr_force("DM  : Min = %d, Max = %d, Center = %d\n",
		 temp0, temp2, temp1);

	for (i = 0 ; i < 8 ; i++) {
		temp0 = readl(phy0_reg + 0x490 + (0xc*i)) >> 16;
		temp1 = readl(phy0_reg + 0x1f0 + (0xc*i)) >> 16;
		temp2 = (temp1 - temp0) + temp1;
		if (i == 0 || i == 1)
			pr_force("DQ%d : Min = %d, Max = %d, Center = %d\n",
			       i+8, temp0, temp2, temp1);
		else
			pr_force("DQ%d: Min = %d, Max = %d, Center = %d\n",
			       i+8, temp0, temp2, temp1);
	}

	temp0 = readl(phy0_reg + 0x4f0) >> 16;
	temp1 = readl(phy0_reg + 0x250) >> 16;
	temp2 = (temp1 - temp0) + temp1;
	pr_force("DM  : Min = %d, Max = %d, Center = %d\n",
		 temp0, temp2, temp1);

	for (i = 0 ; i < 8 ; i++) {
		temp0 = readl(phy1_reg + 0x490 + (0xc*i)) & 0x1FF;
		temp1 = readl(phy1_reg + 0x1f0 + (0xc*i)) & 0x1FF;
		temp2 = (temp1 - temp0) + temp1;
		pr_force("DQ%d: Min = %d, Max = %d, Center = %d\n",
		       i + 16, temp0, temp2, temp1);
	}
	temp0 = readl(phy1_reg + 0x4f0) & 0x1FF;
	temp1 = readl(phy1_reg + 0x250) & 0x1FF;
	temp2 = (temp1 - temp0) + temp1;
	pr_force("DM  : Min = %d, Max = %d, Center = %d\n",
		 temp0, temp2, temp1);

	for (i = 0 ; i < 8 ; i++) {
		temp0 = readl(phy1_reg + 0x490 + (0xc*i)) >> 16;
		temp1 = readl(phy1_reg + 0x1f0 + (0xc*i)) >> 16;
		temp2 = (temp1 - temp0) + temp1;
		pr_force("DQ%d: Min = %d, Max = %d, Center = %d\n",
		       i + 24, temp0, temp2, temp1);
	}
	temp0 = readl(phy1_reg + 0x4f0) >> 16;
	temp1 = readl(phy1_reg + 0x250) >> 16;
	temp2 = (temp1 - temp0) + temp1;
	pr_force("DM  : Min = %d, Max = %d, Center = %d\n",
		 temp0, temp2, temp1);

	pr_force("\nCS1 Result -\n");
	for (i = 0 ; i < 8 ; i++) {
		temp0 = readl(phy0_reg + 0x500 + (0xc*i)) & 0x1FF;
		temp1 = readl(phy0_reg + 0x410 + (0xc*i)) & 0x1FF;
		temp2 = (temp1 - temp0) + temp1;
		pr_force("DQ%d : Min = %d, Max = %d, Center = %d\n",
		       i, temp0, temp2, temp1);
	}
	temp0 = readl(phy0_reg + 0x560) & 0x1FF;
	temp1 = readl(phy0_reg + 0x470) & 0x1FF;
	temp2 = (temp1 - temp0) + temp1;
	pr_force("DM  : Min = %d, Max = %d, Center = %d\n",
		 temp0, temp2, temp1);

	for (i = 0 ; i < 8 ; i++) {
		temp0 = readl(phy0_reg + 0x500 + (0xc*i)) >> 16;
		temp1 = readl(phy0_reg + 0x410 + (0xc*i)) >> 16;
		temp2 = (temp1 - temp0) + temp1;
		if (i == 0 || i == 1)
			pr_force("DQ%d : Min = %d, Max = %d, Center = %d\n",
			       i+8, temp0, temp2, temp1);
		else
			pr_force("DQ%d: Min = %d, Max = %d, Center = %d\n",
			       i+8, temp0, temp2, temp1);
	}
	temp0 = readl(phy0_reg + 0x560) >> 16;
	temp1 = readl(phy0_reg + 0x470) >> 16;
	temp2 = (temp1 - temp0) + temp1;
	pr_force("DM  : Min = %d, Max = %d, Center = %d\n",
		 temp0, temp2, temp1);

	for (i = 0 ; i < 8 ; i++) {
		temp0 = readl(phy1_reg + 0x500 + (0xc*i)) & 0x1FF;
		temp1 = readl(phy1_reg + 0x410 + (0xc*i)) & 0x1FF;
		temp2 = (temp1 - temp0) + temp1;
		pr_force("DQ%d: Min = %d, Max = %d, Center = %d\n",
		       i + 16, temp0, temp2, temp1);
	}
	temp0 = readl(phy1_reg + 0x560) & 0x1FF;
	temp1 = readl(phy1_reg + 0x470) & 0x1FF;
	temp2 = (temp1 - temp0) + temp1;
	pr_force("DM  : Min = %d, Max = %d, Center = %d\n",
		 temp0, temp2, temp1);

	for (i = 0 ; i < 8 ; i++) {
		temp0 = readl(phy1_reg + 0x500 + (0xc*i)) >> 16;
		temp1 = readl(phy1_reg + 0x410 + (0xc*i)) >> 16;
		temp2 = (temp1 - temp0) + temp1;
		pr_force("DQ%d: Min = %d, Max = %d, Center = %d\n",
		       i + 24, temp0, temp2, temp1);
	}
	temp0 = readl(phy1_reg + 0x560) >> 16;
	temp1 = readl(phy1_reg + 0x470) >> 16;
	temp2 = (temp1 - temp0) + temp1;
	pr_force("DM  : Min = %d, Max = %d, Center = %d\n",
		 temp0, temp2, temp1);
	pr_force("================================================\n");
	/*PRBS Calibration result*/
	pr_force("\n 6. PRBS Training Result\n");
	pr_force("================================================\n");
	pr_force(" - Read PRBS Training Result\n");
	temp0 = readl(phy0_reg + 0xac0) & 0x1FF;
	temp1 = readl(phy0_reg + 0xac4) & 0x1FF;
	if (readl(phy0_reg + 0x68c) & 0x100)
		temp2 = -readl(phy0_reg + 0x68c) & 0xFF;
	else
		temp2 = readl(phy0_reg + 0x68c) & 0xFF;
	if (temp2 < 0)
		pr_force(" Lane0 : Left=%d, Right=%d, Offset=%d\n",
		       temp0, temp1, temp2);
	else
		pr_force(" Lane0 : Left=%d, Right=%d, Offset=+%d\n",
		       temp0, temp1, temp2);
	temp0 = readl(phy0_reg + 0xac0) >> 16;
	temp1 = readl(phy0_reg + 0xac4) >> 16;
	if ((readl(phy0_reg + 0x68c) >> 16) & 0x100)
		temp2 = -(readl(phy0_reg + 0x68c) >> 16) & 0xFF;
	else
		temp2 = (readl(phy0_reg + 0x68c) >> 16) & 0xFF;
	if (temp2 < 0)
		pr_force(" Lane1 : Left=%d, Right=%d, Offset=%d\n",
		       temp0, temp1, temp2);
	else
		pr_force(" Lane1 : Left=%d, Right=%d, Offset=+%d\n",
		       temp0, temp1, temp2);
	temp0 = readl(phy1_reg + 0xac0) & 0x1FF;
	temp1 = readl(phy1_reg + 0xac4) & 0x1FF;
	if (readl(phy1_reg + 0x68c) & 0x100)
		temp2 = -readl(phy1_reg + 0x68c) & 0xFF;
	else
		temp2 = readl(phy1_reg + 0x68c) & 0xFF;
	if (temp2 < 0)
		pr_force(" Lane0 : Left=%d, Right=%d, Offset=%d\n",
		       temp0, temp1, temp2);
	else
		pr_force(" Lane2 : Left=%d, Right=%d, Offset=+%d\n",
		       temp0, temp1, temp2);
	temp0 = readl(phy1_reg + 0xac0) >> 16;
	temp1 = readl(phy1_reg + 0xac4) >> 16;
	if ((readl(phy1_reg + 0x68c) >> 16) & 0x100)
		temp2 = -(readl(phy1_reg + 0x68c) >> 16) & 0xFF;
	else
		temp2 = (readl(phy1_reg + 0x68c) >> 16) & 0xFF;
	if (temp2 < 0)
		pr_force(" Lane1 : Left=%d, Right=%d, Offset=%d\n",
		       temp0, temp1, temp2);
	else
		pr_force(" Lane3 : Left=%d, Right=%d, Offset=+%d\n",
		       temp0, temp1, temp2);

	pr_force("\n - Write PRBS Training Result(CS0)\n");
	temp0 = readl(phy0_reg + 0xac8) & 0x1FF;
	temp1 = readl(phy0_reg + 0xacc) & 0x1FF;
	if (readl(phy0_reg + 0xad0) & 0x100)
		temp2 = -readl(phy0_reg + 0xad0) & 0xFF;
	else
		temp2 = readl(phy0_reg + 0xad0) & 0xFF;
	if (temp2 < 0)
		pr_force(" Lane0 : Left=%d, Right=%d, Offset=%d\n",
		       temp0, temp1, temp2);
	else
		pr_force(" Lane0 : Left=%d, Right=%d, Offset=+%d\n",
		       temp0, temp1, temp2);
	temp0 = readl(phy0_reg + 0xac8) >> 16;
	temp1 = readl(phy0_reg + 0xacc) >> 16;
	if ((readl(phy0_reg + 0xad0)>>16) & 0x100)
		temp2 = -(readl(phy0_reg + 0xad0) >> 16) & 0xFF;
	else
		temp2 = (readl(phy0_reg + 0xad0) >> 16) & 0xFF;
	if (temp2 < 0)
		pr_force(" Lane1 : Left=%d, Right=%d, Offset=%d\n",
		       temp0, temp1, temp2);
	else
		pr_force(" Lane1 : Left=%d, Right=%d, Offset=+%d\n",
		       temp0, temp1, temp2);
	temp0 = readl(phy1_reg + 0xac8) & 0x1FF;
	temp1 = readl(phy1_reg + 0xacc) & 0x1FF;
	if (readl(phy1_reg + 0xad0) & 0x100)
		temp2 = -readl(phy1_reg + 0xad0) & 0xFF;
	else
		temp2 = readl(phy1_reg + 0xad0) & 0xFF;
	if (temp2 < 0)
		pr_force(" Lane2 : Left=%d, Right=%d, Offset=%d\n",
		       temp0, temp1, temp2);
	else
		pr_force(" Lane2 : Left=%d, Right=%d, Offset=+%d\n",
		       temp0, temp1, temp2);
	temp0 = readl(phy1_reg + 0xac8) >> 16;
	temp1 = readl(phy1_reg + 0xacc) >> 16;
	if ((readl(phy1_reg + 0xad0) >> 16) & 0x100)
		temp2 = -(readl(phy1_reg + 0xad0) >> 16) & 0xFF;
	else
		temp2 = (readl(phy1_reg + 0xad0) >> 16) & 0xFF;
	if (temp2 < 0)
		pr_force(" Lane3 : Left=%d, Right=%d, Offset=%d\n",
		       temp0, temp1, temp2);
	else
		pr_force(" Lane3 : Left=%d, Right=%d, Offset=+%d\n",
		       temp0, temp1, temp2);

	pr_force("\n - Write PRBS Training Result(CS1)\n");
	temp0 = readl(phy0_reg + 0x69c) & 0x1FF;
	temp1 = readl(phy0_reg + 0x6a0) & 0x1FF;
	if (readl(phy0_reg + 0x690) & 0x100)
		temp2 = -readl(phy0_reg + 0x690) & 0xFF;
	else
		temp2 = readl(phy0_reg + 0x690) & 0xFF;
	if (temp2 < 0)
		pr_force(" Lane0 : Left=%d, Right=%d, Offset=%d\n",
		       temp0, temp1, temp2);
	else
		pr_force(" Lane0 : Left=%d, Right=%d, Offset=+%d\n",
		       temp0, temp1, temp2);
	temp0 = readl(phy0_reg + 0x69c) >> 16;
	temp1 = readl(phy0_reg + 0x6a0) >> 16;

	if ((readl(phy0_reg + 0x690)>>16) & 0x100)
		temp2 = -(readl(phy0_reg + 0x690) >> 16) & 0xFF;
	else
		temp2 = (readl(phy0_reg + 0x690)>>16)&0xFF;
	if (temp2 < 0)
		pr_force(" Lane1 : Left=%d, Right=%d, Offset=%d\n",
		       temp0, temp1, temp2);
	else
		pr_force(" Lane1 : Left=%d, Right=%d, Offset=+%d\n",
		       temp0, temp1, temp2);
	temp0 = readl(phy1_reg + 0x69c) & 0x1FF;
	temp1 = readl(phy1_reg + 0x6a0) & 0x1FF;
	if (readl(phy1_reg + 0x690)&0x100)
		temp2 = -readl(phy1_reg + 0x690) & 0xFF;
	else
		temp2 = readl(phy1_reg + 0x690) & 0xFF;
	if (temp2 < 0)
		pr_force(" Lane0 : Left=%d, Right=%d, Offset=%d\n",
		       temp0, temp1, temp2);
	else
		pr_force(" Lane2 : Left=%d, Right=%d, Offset=+%d\n",
		       temp0, temp1, temp2);
	temp0 = readl(phy1_reg + 0x69c) >> 16;
	temp1 = readl(phy1_reg + 0x6a0) >> 16;
	if ((readl(phy1_reg + 0x690) >> 16) & 0x100)
		temp2 = -(readl(phy1_reg + 0x690) >> 16) & 0xFF;
	else
		temp2 = (readl(phy1_reg + 0x690) >> 16) & 0xFF;
	if (temp2 < 0)
		pr_force(" Lane1 : Left=%d, Right=%d, Offset=%d\n",
		       temp0, temp1, temp2);
	else
		pr_force(" Lane3 : Left=%d, Right=%d, Offset=+%d\n",
		       temp0, temp1, temp2);
	pr_force("================================================\n");
	pr_force("\n 7. VREF Setting\n");
	pr_force("================================================\n");
	pr_force("SoC VREF - PHY0:0x%02x , PHY1:0x%02x\n",
	       readl(phy0_reg + 0x3ec) & 0x3F, readl(phy1_reg + 0x3ec) & 0x3F);
	pr_force("DRAM VREF - Range:%d, VREF:0x%02x\n",
	       readl(phy0_reg + 0xad4) >> 6, readl(phy0_reg + 0xad4) & 0x3F);
	pr_force("================================================\n");
	return 0;
}


static int do_clkdds_vwm(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0, valid_dds = 0;
	unsigned char clk_dds;
	struct arm_smccc_res res;

	pr_force("\n Clock Strength Margin Window\n");
	pr_force("CLK Strength :  1  2  3  4  5  6  7\n");
	pr_force("              ");

	for (clk_dds = 1; clk_dds <= 7; clk_dds++) {
		arm_smccc_smc(SIP_DRAM_TOOL_CLK, clk_dds, 0,
						0, 0, 0, 0, 0, &res);
		if (res.a0 == 0)
			valid_dds = clk_dds;
		else
			valid_dds = 0;

		if (valid_dds != 0)
			pr_force("  *");
		else
			pr_force("   ");
	}
	pr_force("\n");
	pr_force("Done\n");
	return ret;
}


static int do_cadds_vwm(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	int ret = 0;
	unsigned char ca_dds, ca_odt;
	struct arm_smccc_res res;

	pr_force("\n Address/CS Strength Margin Window\n");
	pr_force("Address/CS Strength :  1  2  3  4  5  6  7\n");

	for (ca_odt = 1; ca_odt <= 6; ca_odt++) {
		pr_force("CA_ODT %d             ", ca_odt);
		for (ca_dds = 1; ca_dds <= 7; ca_dds++) {
			arm_smccc_smc(SIP_DRAM_TOOL_CA, ca_dds, ca_odt,
						0, 0, 0, 0, 0, &res);
			if (res.a0 == 0)
				pr_force("  *");
			else
				pr_force("   ");
		}
		pr_force("\n");
	}
	pr_force("Done\n");

	return ret;
}

static int do_rdqs_vwm(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	int i, ret;
	int valid_left = 0, valid_right = 0;
	struct arm_smccc_res res;

	pr_force("\n Read DQ/DQS Valid Margin Window\n");

	for (i = 0; i <= 0xFF; i++) {
		arm_smccc_smc(SIP_DRAM_TOOL_RDQS, (-i), 0,
					0, 0, 0, 0, 0, &res);
		if (res.a0 == 0)
			valid_left = i;
		else
			break;
	}

	for (i = 0; i <= 0xFF; i++) {
		arm_smccc_smc(SIP_DRAM_TOOL_RDQS, (i), 0,
					0, 0, 0, 0, 0, &res);
		if (res.a0 == 0)
			valid_right = i;
		else
			break;
	}
	pr_force(" ");
	for (i = 0; i < WINDOW_MAX; i++) {
		if (i < (WINDOW_MAX/2)) {
			if (((WINDOW_MAX/2)-i) <= valid_left)
				if (valid_left == 0)
					pr_force("X");
				else
					pr_force("*");

			else
				pr_force("X");
		} else {
			if ((i-(WINDOW_MAX/2)) <= valid_right)
				if (valid_right == 0)
					pr_force("X");
				else
					pr_force("*");
			else
				pr_force("X");
		}
		if (i == (WINDOW_MAX/2))
			pr_force("|");
	}

	pr_force("\n Left valid offset : %d\n ", valid_left);
	pr_force("Right valid offset : %d\n", valid_right);
	pr_force("\nDone.\n");
	ret = 1;
	return ret;
}

static int do_rdqsl_vwm(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	int i, j, ret;
	int valid_left = 0, valid_right = 0;
	struct arm_smccc_res res;

	pr_force("\n Lane Read DQ/DQS Valid Margin Window\n");

	for (j = 0; j < 4; j++) {
		for (i = 0; i <= 0xFF; i++) {
			arm_smccc_smc(SIP_DRAM_TOOL_RDQSL, (-i), j,
						0, 0, 0, 0, 0, &res);
			if (res.a0 == 0)
				valid_left = i;
			else
				break;
		}

		for (i = 0; i <= 0xFF; i++) {
			arm_smccc_smc(SIP_DRAM_TOOL_RDQSL, (i), j,
						0, 0, 0, 0, 0, &res);
			if (res.a0 == 0)
				valid_right = i;
			else
				break;
		}
		pr_force(" ");
		for (i = 0; i < WINDOW_MAX; i++) {
			if (i < (WINDOW_MAX/2)) {
				if (((WINDOW_MAX/2)-i) <= valid_left)
					if (valid_left == 0)
						pr_force("X");
					else
						pr_force("*");
				else
					pr_force("X");
			} else {
				if ((i-(WINDOW_MAX/2)) <= valid_right)
					if (valid_right == 0)
						pr_force("X");
					else
						pr_force("*");
				else
					pr_force("X");
			}
			if (i == (WINDOW_MAX/2))
				pr_force("|");
		}
		pr_force("\n Lane %d  Left valid offset : %d\n ",
			 j, valid_left);
		pr_force("Lane %d Right valid offset : %d\n", j, valid_right);
	}
	pr_force("\nDone.\n");
	ret = 1;
	return ret;
}

static int do_wdqsl_vwm(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	int i, j, ret;
	int valid_left = 0, valid_right = 0;
	struct arm_smccc_res res;

	pr_force("\n Lane Write DQ/DQS Valid Margin Window\n");

	for (j = 0; j < 4; j++) {
		for (i = 0; i <= 0xFF; i++) {
			arm_smccc_smc(SIP_DRAM_TOOL_WDQSL, (-i), j,
						0, 0, 0, 0, 0, &res);
			if (res.a0 == 0)
				valid_left = i;
			else
				break;
		}

		for (i = 0; i <= 0xFF; i++) {
			arm_smccc_smc(SIP_DRAM_TOOL_WDQSL, (i), j,
						0, 0, 0, 0, 0, &res);
			if (res.a0 == 0)
				valid_right = i;
			else
				break;
		}

		pr_force(" ");
		for (i = 0; i < WINDOW_MAX; i++) {
			if (i < (WINDOW_MAX/2)) {
				if (((WINDOW_MAX/2)-i) <= valid_left)
					if (valid_left == 0)
						pr_force("X");
					else
						pr_force("*");
				else
					pr_force("X");
			} else {
				if ((i-(WINDOW_MAX/2)) <= valid_right)
					if (valid_right == 0)
						pr_force("X");
					else
						pr_force("*");
				else
					pr_force("X");
			}
			if (i == (WINDOW_MAX/2))
				pr_force("|");
		}
		pr_force("\n Lane %d  Left valid offset : %d\n ",
			 j, valid_left);
		pr_force("Lane %d Right valid offset : %d\n", j, valid_right);
	}
	pr_force("\nDone.\n");
	ret = 1;
	return ret;
}

static int do_wdqs_vwm(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	int i, ret;
	int valid_left = 0, valid_right = 0;
	struct arm_smccc_res res;

	pr_force("\n Write DQ/DQS Valid Margin Window\n");

	for (i = 0; i <= 0xFF; i++) {
		arm_smccc_smc(SIP_DRAM_TOOL_WDQS, (-i), 0,
						0, 0, 0, 0, 0, &res);
		if (res.a0 == 0)
			valid_left = i;
		else
			break;
	}

	for (i = 0; i <= 0xFF; i++) {
		arm_smccc_smc(SIP_DRAM_TOOL_WDQS, (i), 0,
						0, 0, 0, 0, 0, &res);
		if (res.a0 == 0)
			valid_right = i;
		else
			break;
	}

	pr_force(" ");
	for (i = 0; i < WINDOW_MAX; i++) {
		if (i < (WINDOW_MAX/2)) {
			if (((WINDOW_MAX/2)-i) <= valid_left)
				if (valid_left == 0)
					pr_force("X");
				else
					pr_force("*");
			else
				pr_force("X");
		} else {
			if ((i-(WINDOW_MAX/2)) <= valid_right)
				if (valid_right == 0)
					pr_force("X");
				else
					pr_force("*");
			else
				pr_force("X");
		}
		if (i == (WINDOW_MAX/2))
			pr_force("|");
	}

	pr_force("\n Left max valid offset : %d\n ", valid_left);
	pr_force(" Right max valid offset : %d\n", valid_right);
	pr_force("\nDone.\n");
	ret = 1;
	return ret;
}

static int do_svref_set(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	int i, ret = 0;
	int vref_value;
	uintptr_t phy_reg;

	if (argv[1] == NULL) {
		pr_force("Not valid\n set_soc_vref [Vref]\n");
		return -1;
	}

	vref_value = simple_strtoul(argv[1], NULL, 10);

	pr_force("\n SoC VREF Change\n");

	vref_value = (unsigned int)vref_value / 11;
	pr_force("find vref value - %d\n", vref_value);
	for (i = 0; i < 64; i++) {
		if (vref_value == soc_vref[i])
			vref_value = i;
	}

	if (vref_value > 64) {
		pr_force("Please, check your input\n");
		return -1;
	}
	pr_force("\n Change SoC Vref value : %08x\n", (unsigned int)vref_value);
	pr_force("phy0: %08x\n", (unsigned int)ddr_readl(0x13310000 + 0x3ec));
	pr_force("phy1: %08x\n", (unsigned int)ddr_readl(0x13320000 + 0x3ec));

	for (i = 0; i < 2; i++) {
		phy_reg = (i) ? 0x13320000 : 0x13310000;
		/*Enable FSBST*/
		ddr_writel(phy_reg + 0x3ec,
			   (ddr_readl(phy_reg + 0x3ec) | 0x4040));
		ddr_writel(phy_reg+0x3c8,
			   ((ddr_readl(phy_reg + 0x3c8) & ~(0x7 << 21)) |
			   (0x1 << 21)));
		/*set soc vref value*/
		ddr_writel(phy_reg + 0x3ec,
			   ((ddr_readl(phy_reg + 0x3ec) & ~(0x3F3F)) |
			   (vref_value << 8 | vref_value)));
		udelay(5);
		/*Disable FSBST*/
		ddr_writel(phy_reg + 0x3ec,
			   (ddr_readl(phy_reg + 0x3ec) & ~(0x4040)));
	}

	pr_force("phy0: %08x\n", (unsigned int)ddr_readl(0x13310000 + 0x3ec));
	pr_force("phy1: %08x\n", (unsigned int)ddr_readl(0x13320000 + 0x3ec));
	pr_force("\nDone.\n");
	return ret;
}



static int do_dvref_set(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	int i = 0;
	ulong vref_value;

	if (argv[1] == NULL) {
		pr_force("Not valid\n set_ddr_vref [Vref]\n");
		return -1;
	}

	vref_value = simple_strtoul(argv[1], NULL, 10);
	pr_force("\n DRAM VREF Change\n");

	vref_value = ((vref_value/11) - 100)/4;
	if (vref_value < 0 || vref_value > 50) {
		pr_force("Please, check your input\n");
		return -1;
	}
	pr_force("find vref value - %ld\n", vref_value);
	pr_force("Before the change - Range:%d, VREF:0x%02x\n",
	       readl(0x13310000 + 0xad4) >> 6,
	       readl(0x13310000 + 0xad4) & 0x3F);
	i = mr_read(MR14, 0, 2);
	pr_force("Before the change - MR14 DQ : 0x%x\n", i & 0x3F);
	mr_write(MR14, 0x40 | vref_value);
	mr_write(MR14, 0x00 | vref_value);
	ddr_writel(0x13310000 + 0xad4, vref_value);
	udelay(30);
	i = mr_read(MR14, 0, 2);
	pr_force("After the change - MR14 DQ : 0x%x\n", i & 0x3F);
	pr_force("After the change - Range:%d, VREF:0x%02x\n",
	       readl(0x13310000 + 0xad4) >> 6,
	       readl(0x13310000 + 0xad4) & 0x3F);
	return 1;
}


static int do_set_soc_odt(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	uintptr_t phy0_reg = 0x13310000;
	uintptr_t phy1_reg = 0x13320000;
	int soc_odt = 0;
	int read_odt = 0;

	if (argv[1] == NULL) {
		pr_force("Not valid\n set_soc_odt [DDS(1-7)]");
		return -1;
	}

	soc_odt = simple_strtoul(argv[1], NULL, 10);
	if ((soc_odt > 7) || (soc_odt < 1)) {
		pr_force("Not valid\n set_soc_odt [DDS(1-7)]");
		return -1;
	}

	pr_force("\n SoC odt Change\n");
	read_odt = ddr_readl(phy0_reg + ZQ_CON6);
	pr_force("Before the change - ZQ_DS1_TERM :%d ZQ_DS0_TERM : %d\n",
			(read_odt >> 11),
			((read_odt & 0x38) >> 3));
	/*set soc odt*/
	ddr_writel(phy0_reg + ZQ_CON6, 0x0101 |
			(soc_odt << ZQ_DS1_TERM) | (soc_odt << ZQ_DS0_TERM));
	ddr_writel(phy1_reg + ZQ_CON6, 0x0101 |
			(soc_odt << ZQ_DS1_TERM) | (soc_odt << ZQ_DS0_TERM));
	read_odt = ddr_readl(phy0_reg + ZQ_CON6);
	pr_force("After the change - ZQ_DS1_TERM :%d ZQ_DS0_TERM : %d\n",
			(read_odt >> 11), ((read_odt & 0x38) >> 3));
	udelay(30);
	return 0;
}

static int do_set_soc_dds(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	uintptr_t phy0_reg = 0x13310000;
	uintptr_t phy1_reg = 0x13320000;
	int soc_dds = 0;
	int read_dds = 0;

	if (argv[1] == NULL) {
		pr_force("Not valid\n set_soc_dds [DDS(1-7)]");
		return -1;
	}

	soc_dds = simple_strtoul(argv[1], NULL, 10);
	if ((soc_dds > 7) || (soc_dds < 1)) {
		pr_force("Not valid\n set_soc_dds [DDS(1-7)]");
		return -1;
	}
	read_dds = ddr_readl(phy0_reg + ZQ_CON3);
	pr_force("\n SoC dds Change\n");
	pr_force("Before the change - ZQ_DS1_DDS  :%d ZQ_DS0_DDS  : %d\n",
			((read_dds >> 11) & 0x7), ((read_dds >> 3) & 0x7));
	pr_force("                    ZQ_DS1_PDDS :%d ZQ_DS0_PDDS : %d\n",
			((read_dds >> 8) & 0x7), ((read_dds) & 0x7));
	/*set soc dds*/
	ddr_writel(phy0_reg + ZQ_CON3, (soc_dds << ZQ_DS1_DDS) |
			(soc_dds << ZQ_DS1_PDDS) |
			(soc_dds << ZQ_DS0_DDS) |
			(soc_dds << ZQ_DS0_PDDS));
	ddr_writel(phy1_reg + ZQ_CON3, (soc_dds<<ZQ_DS1_DDS) |
			(soc_dds << ZQ_DS1_PDDS) |
			(soc_dds << ZQ_DS0_DDS) |
			(soc_dds << ZQ_DS0_PDDS));
	udelay(30);
	read_dds = ddr_readl(phy0_reg + ZQ_CON3);
	pr_force("After the change  - ZQ_DS1_DDS  :%d ZQ_DS0_DDS  : %d\n",
			((read_dds >> 11) & 0x7), ((read_dds >> 3) & 0x7));
	pr_force("                    ZQ_DS1_PDDS :%d ZQ_DS0_PDDS : %d\n",
			((read_dds >> 8) & 0x7), ((read_dds) & 0x7));
	return 0;
}

static int do_set_ddr_odt(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	unsigned char mr_data;
	unsigned char dram_odt = 0;

	if (argv[1] == NULL) {
		pr_force("Not valid\n set_ddr_odt [ODT(1-7)]\n");
		return -1;
	}

	dram_odt = simple_strtoul(argv[1], NULL, 16);

	if (((dram_odt > 7) || (dram_odt < 1))) {
		pr_force("Not valid\n set_ddr_odt [ODT(1-7)]\n");
		return -1;
	}
	mr_data = 0x60 | dram_odt;
	mr_write(MR11, mr_data);
	udelay(30);
	pr_force("MR 11 : %x  DQ ODT : %d\n", mr_data, (mr_data & 0x7));
	return 0;
}

static int do_set_ddr_dds(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	unsigned char mr_data;
	unsigned char dram_dds = 0;

	if (argv[1] == NULL) {
		pr_force("Not valid\n set_ddr_dds [DDS(1-7)]\n");
		return -1;
	}
	dram_dds = simple_strtoul(argv[1], NULL, 16);

	if (((dram_dds > 7) || (dram_dds < 1))) {
		pr_force("Not valid\n set_ddr_dds [DDS(1-7)]\n");
		return -1;
	}
	mr_data = 0xC7 | (dram_dds << 3);
	mr_write(MR3, mr_data);
	udelay(30);
	pr_force("Change MR 3 : %x  DQ DDS : %d\n",
			mr_data, ((mr_data >> 3) & 0x7));
	return 0;
}



static int do_svref_vwm(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	int i, j, ret = 1;
	int valid_left = 0, valid_right = 0;
	int vref_value;
	struct arm_smccc_res res;
	int add_tab_left = 0;
	int add_tab_right = 0;

	pr_force("\n SoC VREF Valid Margin Window");
	for (j = 0; j < 103; j++)
		pr_force(" ");

	pr_force("left | right [picoseconds]");
	for (j = 0; j < 136; j++)
		pr_force(" ");
	pr_force("left | right [picoseconds]\n");

	vref_value = SOC_MAX_VREF;
	while (1) {
		for (j = 0; j < 2; j++) {
			pr_force(" ");
			if (vref_value < 10)
				pr_force("phys[%d] soc vref 0.%d V - ",
				       j, cal_svref(vref_value));
			else
				pr_force("phys[%d] soc vref 0.%d V - ",
				       j, cal_svref(vref_value));
			valid_left = 0;
			valid_right = 0;
			for (i = 0; i <= 0xFF; i++) {
				arm_smccc_smc(SIP_DRAM_TOOL_SVREF,
					  vref_value, (-i), j,
					0, 0, 0, 0, &res);
				if (res.a0 == 0)
					valid_left = i;
				else
					break;
			}
			for (i = 0; i <= 0xFF; i++) {
				arm_smccc_smc(SIP_DRAM_TOOL_SVREF,
					  vref_value, i, j,
					0, 0, 0, 0, &res);
				if (res.a0 == 0)
					valid_right = i;
				else
					break;
			}

			for (i = 0; i < WINDOW_MAX; i++) {
				if (i < (WINDOW_MAX/2)) {
					if ((((WINDOW_MAX/2)-i) <=
					    valid_left) &&
					    (valid_left != 0)) {
						pr_force("*");
						add_tab_left++;
					} else {
						pr_force("X");
					}
				} else {
					if (((i-(WINDOW_MAX/2)) <=
					valid_right) &&
					(valid_right != 0)) {
						pr_force("*");
						add_tab_right++;
					} else {
						pr_force("X");
					}
				}
				if (i == (WINDOW_MAX/2))
					pr_force("|");
			}

			if (add_tab_left != 0)
				add_tab_left = ((add_tab_left + 1)*46);

			if (add_tab_right != 0)
				add_tab_right = ((add_tab_right + 1)*46);

			if (j == 0)
				pr_force(" %3d.%d | %3d.%d [picoseconds] |||",
				       add_tab_left/10, add_tab_left%10,
				       add_tab_right/10, add_tab_right%10);
			if (j == 1)
				pr_force(" %3d.%d | %3d.%d [picoseconds]\n",
				       add_tab_left/10, add_tab_left%10,
				       add_tab_right/10, add_tab_right%10);
			add_tab_left = 0;
			add_tab_right = 0;
		}
		if (vref_value > 48)
			vref_value -= 1;
		else if (vref_value == 48)
			vref_value = 0;
		else if (vref_value == 47)
			break;
		else if (vref_value < 48)
			vref_value += 1;
	}
	arm_smccc_smc(SIP_DRAM_TOOL_SVREF, 0x1d, (0),
					0, 0, 0, 0, 0, &res);
	arm_smccc_smc(SIP_DRAM_TOOL_SVREF, 0x1d, (0),
					0, 0, 0, 0, 1, &res);
	pr_force("\nDone.\n");
	return ret;
}

static int do_dvref_vwm(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	int i, j, ret = 1;
	int valid_left = 0, valid_right = 0;
	int vref_value;
	int add_tab_left = 0;
	int add_tab_right = 0;
	struct arm_smccc_res res;

	pr_force("\nDRAM VREF Valid Margin Window");

	for (j = 0; j < 105; j++)
		pr_force(" ");
	pr_force("left | right [picoseconds]");

	for (j = 0; j < 138; j++)
		pr_force(" ");
	pr_force("left | right [picoseconds]\n");

	vref_value = DRAM_MAX_VREF;
	while (vref_value >= 0) {
		for (j = 0; j < 2; j++) {
			pr_force(" ");
			if (vref_value < 10)
				pr_force("range[%d] dram vref 0.%d V - ", j,
				       ((vref_value*4 + 100)*11));
			else
				pr_force("range[%d] dram vref 0.%d V - ", j,
				       ((vref_value*4 + 100)*11));
			valid_left = 0;
			valid_right = 0;
			for (i = 0; i <= 0xFF; i++) {
				arm_smccc_smc(SIP_DRAM_TOOL_DVREF, vref_value,
					  j, (-i),
					0, 0, 0, 0, &res);
				if (res.a0 == 0)
					valid_left = i;
				else
					break;
			}

			for (i = 0; i <= 0xFF; i++) {
				arm_smccc_smc(SIP_DRAM_TOOL_DVREF, vref_value,
					  j, i,
					0, 0, 0, 0, &res);
				if (res.a0 == 0)
					valid_right = i;
				else
					break;
			}

			for (i = 0; i < WINDOW_MAX; i++) {
				if (i < (WINDOW_MAX/2)) {
					if ((((WINDOW_MAX/2)-i) <=
					    valid_left) &&
					    (valid_left != 0)) {
						pr_force("*");
						add_tab_left++;
					} else {
						pr_force("X");
					}
				} else {
					if (((i-(WINDOW_MAX/2)) <=
					   valid_right) &&
					   (valid_right != 0)) {
						pr_force("*");
						add_tab_right++;
					} else {
						pr_force("X");
					}
				}
				if (i == (WINDOW_MAX/2))
					pr_force("|");
			}
			if (add_tab_left != 0)
				add_tab_left = ((add_tab_left + 1)*46);

			if (add_tab_right != 0)
				add_tab_right = ((add_tab_right + 1)*46);

			if (j == 0)
				pr_force(" %3d.%d | %3d.%d [picoseconds] |||",
				       add_tab_left/10, add_tab_left%10,
				       add_tab_right/10, add_tab_right%10);
			if (j == 1)
				pr_force(" %3d.%d | %3d.%d [picoseconds]\n",
				       add_tab_left/10, add_tab_left%10,
				       add_tab_right/10, add_tab_right%10);
			add_tab_left = 0;
			add_tab_right = 0;
		}
		vref_value -= 1;
	}
	pr_force("\nDone.\n");
	return ret;
}


static int do_rimp_vwm(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	int i, j, ret = 1;
	int valid_left = 0, valid_right = 0;
	int soc_odt;
	struct arm_smccc_res res;

	pr_force("\n Read Impedence Valid Margin Window\n");

	soc_odt = 7;
	while (soc_odt >= 1) {
		for (j = 1; j <= 6; j++) {
			pr_force(" ");
			pr_force("CPU_ODT[%d] DRAM_DDS[%d] - ", soc_odt, j);
			valid_left = 0;
			valid_right = 0;
			for (i = 0; i <= 0xFF; i++) {
				arm_smccc_smc(SIP_DRAM_TOOL_RIMP, soc_odt,
					  j, (-i),
					0, 0, 0, 0, &res);
				if (res.a0 == 0)
					valid_left = i;
				else
					break;
			}

			for (i = 0; i <= 0xFF; i++) {
				arm_smccc_smc(SIP_DRAM_TOOL_RIMP, soc_odt,
					  j, i,
					0, 0, 0, 0, &res);
				if (res.a0 == 0)
					valid_right = i;
				else
					break;
			}

			for (i = 0; i < WINDOW_MAX; i++) {
				if (i < (WINDOW_MAX/2)) {
					if ((((WINDOW_MAX/2)-i) <=
					    valid_left) &&
					    (valid_left != 0)) {
						pr_force("*");
					} else {
						pr_force("X");
					}
				} else {
					if (((i-(WINDOW_MAX/2)) <=
					    valid_right) &&
					    (valid_right != 0)) {
						pr_force("*");
					} else {
						pr_force("X");
					}
				}
				if (i == (WINDOW_MAX/2))
					pr_force("|");
			}
			pr_force("\n");
		}
		soc_odt -= 1;
	}
	arm_smccc_smc(SIP_DRAM_TOOL_RIMP, 4, 6, 0,
				0, 0, 0, 0, &res);
	pr_force("\nDone.\n");
	return ret;
}


static int do_wimp_vwm(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	int i, j, ret = 1;
	int valid_left = 0, valid_right = 0;
	int soc_dds;
	struct arm_smccc_res res;

	pr_force("\n Write Impedence Valid Margin Window\n");

	soc_dds = 7;
	while (soc_dds >= 1) {
		for (j = 1; j <= 6; j++) {
			pr_force(" ");
			pr_force("CPU_DDS[%d] DRAM_ODT[%d] - ", soc_dds, j);
			valid_left = 0;
			valid_right = 0;
			for (i = 0; i <= 0xFF; i++) {
				arm_smccc_smc(SIP_DRAM_TOOL_WIMP, soc_dds,
					  j, (-i),
					0, 0, 0, 0, &res);
				if (res.a0 == 0)
					valid_left = i;
				else
					break;
			}

			for (i = 0; i <= 0xFF; i++) {
				arm_smccc_smc(SIP_DRAM_TOOL_WIMP, soc_dds,
					  j, i,
					0, 0, 0, 0, &res);
				if (res.a0 == 0)
					valid_right = i;
				else
					break;
			}

			for (i = 0; i < WINDOW_MAX; i++) {
				if (i < (WINDOW_MAX/2)) {
					if ((((WINDOW_MAX/2)-i) <=
					    valid_left) &&
					    (valid_left != 0)) {
						pr_force("*");
					} else {
						pr_force("X");
					}
				} else {
					if (((i-(WINDOW_MAX/2)) <=
					    valid_right) &&
					    (valid_right != 0)) {
						pr_force("*");
					} else {
						pr_force("X");
					}
				}
				if (i == (WINDOW_MAX/2))
					pr_force("|");
			}
			pr_force("\n");
		}
		soc_dds -= 1;
	}
	pr_force("\nDone.\n");
	return ret;
}

static int measure_margins(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	do_rdqs_vwm(cmdtp, flag, argc, argv);
	do_rdqsl_vwm(cmdtp, flag, argc, argv);
	do_wdqs_vwm(cmdtp, flag, argc, argv);
	do_wdqsl_vwm(cmdtp, flag, argc, argv);
	do_dvref_vwm(cmdtp, flag, argc, argv);
	do_svref_vwm(cmdtp, flag, argc, argv);
	do_wimp_vwm(cmdtp, flag, argc, argv);
	do_rimp_vwm(cmdtp, flag, argc, argv);

	return 0;
}

/**************************************************/

U_BOOT_CMD(
		print_dmc_info,	1,	1,	do_pr_force_mc_info,
		"print dram init info",
		""
	);

U_BOOT_CMD(
		measure_margin,	1,	1,	measure_margins,
		"measure margins",
		""
	);

U_BOOT_CMD(
		clock_margin,	1,	1,	do_clkdds_vwm,
		"measure clock strength margin",
		""
	);

U_BOOT_CMD(
		ca_margin,	1,	1,	do_cadds_vwm,
		"measure Address/CS strength margin",
		""
	);


U_BOOT_CMD(
		read_dqs_margin,	1,	1,	do_rdqs_vwm,
		"measure read dq/dqs margin",
		""
	);

U_BOOT_CMD(
		write_dqs_margin,	1,	1,	do_wdqs_vwm,
		"measure write dq/dqs margin",
		""
	);

U_BOOT_CMD(
		read_dqsl_margin,	1,	1,	do_rdqsl_vwm,
		"measure read dq/dqs margin",
		""
	);

U_BOOT_CMD(
		write_dqsl_margin,	1,	1,	do_wdqsl_vwm,
		"measure write dq/dqs margin",
		""
	);


U_BOOT_CMD(
		soc_vref_margin,	1,	1,	do_svref_vwm,
		"measure soc vref margin",
		""
	);

U_BOOT_CMD(
		dram_vref_margin,	1,	1,	do_dvref_vwm,
		"measure dram vref margin",
		""
	);

U_BOOT_CMD(
		read_impedance_margin,	1,	1,	do_rimp_vwm,
		"measure read impedance margin",
		""
	);

U_BOOT_CMD(
		write_impedance_margin,	1,	1,	do_wimp_vwm,
		"measure write impedance margin",
		""
	);

U_BOOT_CMD(
		set_ddr_vref, 2,      1,	do_dvref_set,
		"change ddr vref",
		""
	  );

U_BOOT_CMD(
		set_soc_vref, 2,      1,      do_svref_set,
		"change soc vref",
		""
	  );

U_BOOT_CMD(
		set_soc_odt, 2,      1,      do_set_soc_odt,
		"change soc dds",
		""
	  );

U_BOOT_CMD(
		set_soc_dds, 2,      1,      do_set_soc_dds,
		"change soc dds",
		""
	  );

U_BOOT_CMD(
		set_ddr_odt, 2,      1,      do_set_ddr_odt,
		"change ddr odt",
		""
	  );

U_BOOT_CMD(
		set_ddr_dds, 2,      1,      do_set_ddr_dds,
		"change ddr dds",
		""
	  );
