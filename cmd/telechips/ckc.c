// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <command.h>
#include <common.h>
#include <dm/uclass.h>
#include <dm.h>
#include <linux/io.h>
#include <irq.h>
#include <mach/clock.h>
#include <linux/arm-smccc.h>
#include <mach/smc.h>

static int do_tcc_ckc_pll(struct cmd_tbl *cmdtp, int flag,
			  int argc, char * const argv[])
{
	int rec = 0;
	/* set minimum freq. 60Mhz */
	pr_force("%s : Set minimum freq. 60Mhz\n", __func__);
	mdelay(2);
	for (rec = PLL_1; rec <= PLL_4; rec++) {
		tcc_set_pll(rec, ENABLE, 60000000, 0);
		pr_force("PLL %d (%x): %llu\n", rec, readl(0x14000080 + rec*4),
		       tcc_get_pll(rec));
		mdelay(10);
	}

	pr_force("%s : Set maximum freq. 3.2Ghz\n", __func__);
	/* set maximum freq. 3.2Ghz */
	mdelay(2);
	for (rec = PLL_1; rec <= PLL_4; rec++) {
		tcc_set_pll(rec, ENABLE, 3200000000, 0);
		pr_force("PLL %d (%x): %llu\n", rec, readl(0x14000080 + rec*4),
		       tcc_get_pll(rec));
		mdelay(10);
	}

	tcc_set_pll(PLL_1,  ENABLE,  768000000, 3);
	/* for supporting component/composite */
	tcc_set_pll(PLL_2,  ENABLE, 1188000000, 3);
	tcc_set_pll(PLL_3,  ENABLE,  750000000, 0);
	/* for cm bus */
	tcc_set_pll(PLL_4,  ENABLE, 1000000000, 0);
	pr_force("done.\n");

	return 0;
}

static int do_tcc_dpll_set_spectrum_test(struct cmd_tbl *cmdtp, int flag, int argc,
					 char * const argv[])
{
}

static int do_tcc_pll_div_set_test(struct cmd_tbl *cmdtp, int flag, int argc,
				   char * const argv[])
{
	unsigned int clkdiv0 = 0x14000200;
	unsigned int clkdiv1 = 0x14000204;
	unsigned int i = 0;
	unsigned int passed = 1;

	for (i = 0; i < 5; i++) {
		if (i == 4) {
			unsigned int div = 0;
			unsigned int shift_div = 24;
			unsigned int div_mask = 0x3F;
			unsigned int shift_en = 31;

			pr_force("PLLDIV %d\n", i);
			for (div = 0; div < 64; div++) {
				writel(readl(clkdiv1)
				       & ~(0x1 << shift_en), clkdiv1);
				writel(readl(clkdiv1)
				       & ~(div_mask << shift_div), clkdiv1);
				writel(readl(clkdiv1)
				       | (div << shift_div), clkdiv1);
				writel(readl(clkdiv1)
				       | (0x1 << shift_en), clkdiv1);


				if (((readl(clkdiv1) >> shift_div)
				     & div_mask) == div) {
					pr_force("PLL DIV Configuration Success
					       :: DIV : %d, REG : %x\n"
					       , div, readl(clkdiv1));
				} else {
					pr_force("DIV Configuration Failed::");
					passed = 0;
					break;
				}
				mdelay(10);
			}
		} else {
			unsigned int div = 0;
			unsigned int shift_div = 8*(3-i);
			unsigned int div_mask = 0x3F;
			unsigned int shift_en = 8*(3-i) + 7;

			pr_force("PLLDIV %d\n", i);
			for (div = 0; div < 64; div++) {
				writel(readl(clkdiv0)
				       & ~(0x1 << shift_en), clkdiv0);
				writel(readl(clkdiv0)
				       & ~(div_mask << shift_div), clkdiv0);
				writel(readl(clkdiv0
				       | (div << shift_div), clkdiv0);
				writel(readl(clkdiv0)
				       | (0x1 << shift_en), clkdiv0);


				if (((readl(clkdiv0) >> shift_div)
				     & div_mask) == div) {
					pr_force("PLL DIV Configuration Success
					       :: DIV : %d, REG : %x\n"
					       , div, readl(clkdiv0));
				} else {
					pr_force("DIV Configuration Failed
					       :: DIV : %d , REG : %x\n"
					       , div, readl(clkdiv0));
					passed = 0;
					break;
				}
				mdelay(10);
			}
		}

		if (passed == 0)
			break;
	}
	if (passed == 1)
		pr_force("Test Passed.\n");
	else
		pr_force("Test Failed.\n");
}

U_BOOT_CMD(
	   tcc_ckc_pll, 2, 1, do_tcc_ckc_pll, "ckc pll configuration", ""
	  );

U_BOOT_CMD(
	   tcc_dpll_set_spectrum, 2, 1, do_tcc_dpll_set_spectrum_test,
	   "ckc pll spectrum configuration", ""
	  );

U_BOOT_CMD(
	   tcc_pll_div_set_test, 2, 1, do_tcc_pll_div_set_test,
	   "ckc pll div configuration", ""
	  );

static int do_tcc_busclk_src_change(struct cmd_tbl *cmdtp, int flag, int argc,
				    char * const argv[])
{
	int i, j;
	unsigned int bus_reg[] = {0x14000008, 0x1400000C, 0x14000018,
		0x1400001C, 0x14000024, 0x1400002C,
		0x14000040, 0x14000044, 0x14000048};
	int lim = ARRAY_SIZE(bus_reg)/sizeof(bus_reg[0]);

	tcc_set_pll(PLL_1,  ENABLE, 1400000000, 3);
	/* for supporting component/composite */
	tcc_set_pll(PLL_2,  ENABLE, 1188000000, 3);
	tcc_set_pll(PLL_3,  ENABLE,  750000000, 0);
	/* for cm bus */
	tcc_set_pll(PLL_4,  ENABLE,  100000000, 0);

	for (i = 0; i < lim; i++) {
		pr_force("REG ADDRESS : %x\n", bus_reg[i]);

		if ((bus_reg[i] == 0x1400001C) || (bus_reg[i] == 0x1400002C)) {
			continue;
		}

		for (j = 0; j < 16; j++) {
			if ((j == 6) || (j == 13) || (j == 14) || (j == 15))
				continue;

			clkctrl_write(bus_reg[i], 1, 0, j);

			mdelay(1000);

			pr_force("SRC : %d | REG DUMP : %x\n",
			       j, readl(bus_reg[i]));
		}
		pr_force("\n");
	}
}

static int do_tcc_busclk_cfg_change(struct cmd_tbl *cmdtp, int flag, int argc,
				    char * const argv[])
{
	int i, j;
	unsigned int bus_reg[] = {0x14000008, 0x1400000C, 0x14000018, 0x1400001C
		, 0x14000024, 0x1400002C, 0x14000040, 0x14000044, 0x14000048};
	int lim = ARRAY_SIZE(bus_reg)/sizeof(bus_reg[0]);

	for (i = 0; i < lim; i++) {
		pr_force("REG ADDRESS : %x\n", bus_reg[i]);

		if ((bus_reg[i] == 0x1400001C) || (bus_reg[i] == 0x1400002C)) {
			continue;
		}

		for (j = 0; j < 15; j++) {
			writel((readl(bus_reg[i]) & ~(0x1E0))
			       | ((j) << 5), bus_reg[i]);
			mdelay(1000);
			pr_force("CONFIG : %d | REG DUMP : %x\n",
			       j, readl(bus_reg[i]));
		}
		pr_force("\n");
	}
}

#define CLKCTRL_SEL_MIN         0
#define CLKCTRL_SEL_MAX         15
#define CLKCTRL_SEL_SHIFT       0
#define CLKCTRL_SEL_MASK        0xF
#define CLKCTRL_CONFIG_MIN      1
#define CLKCTRL_CONFIG_MAX      15
#define CLKCTRL_CONFIG_SHIFT    5
#define CLKCTRL_CONFIG_MASK     0xF
#define CLKCTRL_EN_SHIFT        22
#define CLKCTRL_CFGRQ_SHIFT     29
#define CLKCTRL_SYNRQ_SHIFT     30
#define CLKCTRL_CHGRQ_SHIFT     31


void clkctrl_write(unsigned int reg, unsigned int en, unsigned int config,
		   unsigned int sel)
{
	unsigned int cur_config;

	cur_config = (readl(reg) >> CLKCTRL_CONFIG_SHIFT) & CLKCTRL_CONFIG_MASK;

	if (config >= cur_config) {
		writel((readl(reg)
			&(~(CLKCTRL_CONFIG_MASK<<CLKCTRL_CONFIG_SHIFT)))
		       |((config&CLKCTRL_CONFIG_MASK)<<CLKCTRL_CONFIG_SHIFT),
		       reg);
		while (readl(reg) & (1U<<CLKCTRL_CFGRQ_SHIFT))
			;
		writel((readl(reg)
			&(~(CLKCTRL_SEL_MASK<<CLKCTRL_SEL_SHIFT)))
		       |((sel&CLKCTRL_SEL_MASK)<<CLKCTRL_SEL_SHIFT), reg);
		while (readl(reg) & (1U<<CLKCTRL_CHGRQ_SHIFT))
			;
	} else {
		writel((readl(reg)
			&(~(CLKCTRL_SEL_MASK<<CLKCTRL_SEL_SHIFT)))
		       |((sel&CLKCTRL_SEL_MASK)<<CLKCTRL_SEL_SHIFT), reg);
		while (readl(reg) & (1U<<CLKCTRL_CHGRQ_SHIFT))
			;
		writel((readl(reg)
			&(~(CLKCTRL_CONFIG_MASK<<CLKCTRL_CONFIG_SHIFT)))
		       |((config&CLKCTRL_CONFIG_MASK)<<CLKCTRL_CONFIG_SHIFT),
		       reg);
		while (readl(reg) & (1U<<CLKCTRL_CFGRQ_SHIFT))
			;
	}
	writel((readl(reg)&(~(1<<CLKCTRL_EN_SHIFT)))
	       |((en&1)<<CLKCTRL_EN_SHIFT), reg);
	while (readl(reg) & (1 << CLKCTRL_CFGRQ_SHIFT))
		;
}

U_BOOT_CMD(
	   tcc_busclk_cfg_change, 2, 1, do_tcc_busclk_cfg_change,
	   "bus cfg change", ""
	  );

U_BOOT_CMD(
	   tcc_busclk_src_change, 2, 1, do_tcc_busclk_src_change,
	   "bus clk change", ""
	  );

static int do_tcc_periclk_src_change(struct cmd_tbl *cmdtp, int flag, int argc,
				     char * const argv[])
{
	struct arm_smccc_res res;
	unsigned int target_peri;
	unsigned int src = 0;

	arm_smccc_smc(SIP_DEV_CHIP, 0, 0, 0, 0, 0, 0, 0, &res);

	//	pr_force("Rev : %x\n", res.a0);
	res.a0 = 0x8059;

	if (res.a0 == 0x8059) {
		target_peri = 0x14000708;
	} else if (res.a0 == 0x8050) {
		target_peri = 0x1400070C;
	} else {
	}

	pr_force("Register Address : %x\n", target_peri);
	for (src = 0; src < 32; src++) {
		unsigned int field = 0x1F000000;
		unsigned int wr_val = src << 24;
		unsigned int en = 0x20000000;

		writel(readl(target_peri) & ~en, target_peri);
		writel(readl(target_peri) & ~field, target_peri);
		mdelay(100);
		writel(readl(target_peri) | wr_val, target_peri);
		writel(readl(target_peri) | en, target_peri);

		pr_force("src : %d | REG : %x\n", src, readl(target_peri));
	}
}

static int do_tcc_periclk_div_change(struct cmd_tbl *cmdtp, int flag, int argc,
				     char * const argv[])
{
	struct arm_smccc_res res;
	unsigned int target_peri;
	unsigned int div = 0;
	unsigned int passed = 1;

	arm_smccc_smc(SIP_DEV_CHIP, 0, 0, 0, 0, 0, 0, 0, &res);

	//      pr_force("Rev : %x\n", res.a0);
	res.a0 = 0x8059;
	if (res.a0 == 0x8059) {
		target_peri = 0x14000708;
	} else if (res.a0 == 0x8050) {
		target_peri = 0x1400070C;
	} else {
	}

	pr_force("Register Address : %x\n", target_peri);
	for (div = 0; div < 1024; div++) {
		unsigned int field = 0x00000FFF;
		unsigned int en = 0x20000000;
		unsigned int wr_val = div;

		writel(readl(target_peri) & ~en, target_peri);
		writel(readl(target_peri) & ~field, target_peri);
		mdelay(100);
		writel(readl(target_peri) | wr_val, target_peri);
		writel(readl(target_peri) | en, target_peri);

		if ((readl(target_peri) & field) == div) {
			pr_force("DIV Configuration Success -- div :
			       %d | REG : %x\n"
			       , div, readl(target_peri));
		} else {
			passed = 0;
			pr_force("DIV Configuration Error.\n");
			break;
		}
	}
	if (passed != 1) {
		pr_force("Test Failed.\n");
	} else {
		pr_force("Test Passed.\n");
	}
}

static int do_tcc_periclk_mode_change(struct cmd_tbl *cmdtp, int flag, int argc,
				      char * const argv[])
{
	unsigned int target_peri;

	target_peri = 0x14000764;

	pr_force("Register Address : %x\n", target_peri);
	{
		unsigned int en = 0x20000000;
		unsigned int md = 0x80000000;

		writel(readl(target_peri) & ~en, target_peri);
		writel(readl(target_peri) & ~md, target_peri);
		mdelay(100);
		writel(readl(target_peri) | en, target_peri);
	}
	pr_force("Mode Change\n");
	pr_force("REG : %x\n", readl(target_peri));
	{
		unsigned int en = 0x20000000;
		unsigned int md = 0x80000000;

		writel(readl(target_peri) & ~en, target_peri);
		writel(readl(target_peri) | md, target_peri);
		mdelay(100);
		writel(readl(target_peri) | en, target_peri);
	}
	pr_force("REG : %x\n", readl(target_peri));

}

U_BOOT_CMD(
	   tcc_periclk_src_change, 2, 1, do_tcc_periclk_src_change,
	   "peri clk src change", ""
	  );

U_BOOT_CMD(
	   tcc_periclk_div_change, 2, 1, do_tcc_periclk_div_change,
	   "peri clk div change", ""
	  );

U_BOOT_CMD(
	   tcc_periclk_mode_change, 2, 1, do_tcc_periclk_mode_change,
	   "peri clk mode change", ""
	  );

static int do_tcc_cpu0_ckc_range_check(struct cmd_tbl *cmdtp, int flag, int argc,
				       char * const argv[])
{
	unsigned long long start = 60*1000*1000;
	int correct_cnt = 0;

	pr_force("CPU 0 Core Clock Setting Test Start(60Mhz ~ 3.2Ghz)\n");

	for (start; start < (unsigned long long)(3200*1000*1000);
	     start += 10*1000*1000) {
		tcc_set_clkctrl(FBUS_CPU0, 1, start, 0UL);

		unsigned int p = (readl(0x17110008)) & 0x3F;
		unsigned int m = (readl(0x17110008) >> 6) & 0x3FF;
		unsigned int s = (readl(0x17110008) >> 16) & 0x7;

		pr_force("P : %d | M : %d | S : %d\n", p, m, s);
		pr_force("Freq. = %llu || REG : %x || out freq : %llu\n",
			  start, readl(0x17110008),
			  ((unsigned long long)(24000000*m))/(p*(1<<s)));
		mdelay(100);
		if (((unsigned long long)(24000000*m))/(p*(1<<s)) == start)
			correct_cnt++;
	}
	pr_force("CPU 0 Core Clock Setting Test Succcessed :: Correct : %d\n"
	       , correct_cnt);
}

static int do_tcc_cpu1_ckc_range_check(struct cmd_tbl *cmdtp, int flag, int argc,
				       char * const argv[])
{
	unsigned long long start = 60*1000*1000;
	int correct_cnt = 0;

	pr_force("CPU 1 Core Clock Setting Test Start(60Mhz ~ 3.2Ghz)\n");

	for (start; start*2 <
	     (unsigned long long)(3200*1000*1000); start += 10*1000*1000) {
		tcc_set_clkctrl(FBUS_CPU1, 1, start, 0UL);

		unsigned int p = (readl(0x17210008)) & 0x3F;
		unsigned int m = (readl(0x17210008) >> 6) & 0x3FF;
		unsigned int s = (readl(0x17210008) >> 16) & 0x7;

		pr_force("P : %d | M : %d | S : %d\n", p, m, s);
		pr_force("Freq. = %llu || REG : %x || out freq : %llu\n",
			  start, readl(0x17210008),
		       ((unsigned long long)(24000000*m))/(p*(1<<s)));
		mdelay(100);
		if (((unsigned long long)(24000000*m))/(p*(1<<s))
		    == start*2)
			correct_cnt++;
	}
	pr_force("CPU 1 Core Clock Setting Test Succcessed :: Correct : %d\n",
	       correct_cnt);
}

U_BOOT_CMD(
	   tcc_cpu0_ckc_range_check, 2, 1, do_tcc_cpu0_ckc_range_check,
	   "cpu 0 core clock range check", ""
	  );

U_BOOT_CMD(
	   tcc_cpu1_ckc_range_check, 2, 1, do_tcc_cpu1_ckc_range_check,
	   "cpu 1 core clock range check", ""
	  );

static int do_tcc_cpu0_ckc_change_cmd(struct cmd_tbl *cmdtp, int flag, int argc,
				      char * const argv[])
{
	unsigned long long target_freq;
	unsigned int ruler;
	unsigned long long summation = 0;
	//	pr_force("in : %s\n", argv[1]);

	for (ruler = 0; argv[1][ruler] != 0; ruler++) {
		summation *= 10;
		summation += (argv[1][ruler]-'0');
		//		pr_force("summation : %llu\n", summation);
	}
	pr_force("Target Frequency(Hz) : %llu\n", summation);

	tcc_set_clkctrl(FBUS_CPU0, 1, summation, 0UL);

	pr_force("Freq. = %llu\n", tcc_get_clkctrl(FBUS_CPU0));

	return 0;
}

U_BOOT_CMD(
	   tcc_cpu0_ckc_change, 2, 1, do_tcc_cpu0_ckc_change_cmd,
	   "cpu 1 core clock range check", ""
	  );
