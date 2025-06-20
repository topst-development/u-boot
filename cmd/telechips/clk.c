// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <command.h>
#include <common.h>
#include <mach/clk_list.h>
#include <mach/smc.h>
#include <linux/io.h>
#include <linux/arm-smccc.h>
#include <asm/arch/tcc_ckc.h>
#define PRINT_FORMAT "%*s%-*s %15ld %15d\n"

static struct clk_info src_info;
static struct clk_info pll_info;
static struct clk_info pll_div_info;
static struct clk_info sys_bus_info;
static struct clk_info cpu_bus_info;
static struct clk_info cam_bus_info;
static struct clk_info npu_bus_info;
static struct clk_info peri_info;

static void tcc_clk_show_clk_info(unsigned int parent_id, unsigned int depth);

static void tcc_clk_show_one(unsigned int depth, struct clk_list *info)
{
	pr_force(PRINT_FORMAT, depth*3+1, "", 30-depth*3, info->name, info->rate, info->div);
}

static void tcc_clk_print(struct clk_list *clk_info,
			unsigned int size,
			unsigned int parent_id,
			unsigned int depth,
			bool next_depth)
{
	int i = 0;

	for (i = 0; i < size; i++) {
		if (clk_info[i].src_id == parent_id) {
			tcc_clk_show_one(depth, &clk_info[i]);

			if (next_depth) {
				tcc_clk_show_clk_info(clk_info[i].id, depth + 1);
			}
		}
	}
}


static void tcc_clk_show_clk_info(unsigned int parent_id, unsigned int depth)
{
	switch (depth) {
		/* OSC */
		case 0:
			tcc_clk_print(src_info.clk_list, src_info.count, parent_id, depth, true);
			break;
		/* PLL + BUS + PERI */
		case 1:
			tcc_clk_print(pll_info.clk_list, pll_info.count, parent_id, depth, true);

			tcc_clk_print(sys_bus_info.clk_list, sys_bus_info.count, parent_id, depth, false);
			tcc_clk_print(cpu_bus_info.clk_list, cpu_bus_info.count, parent_id, depth, false);
			tcc_clk_print(cam_bus_info.clk_list, cam_bus_info.count, parent_id, depth, false);
			tcc_clk_print(npu_bus_info.clk_list, npu_bus_info.count, parent_id, depth, false);
			tcc_clk_print(peri_info.clk_list, peri_info.count, parent_id, depth, false);
			break;
		/* PLL DIV + BUS + PERI */
		case 2:
			tcc_clk_print(pll_div_info.clk_list, pll_div_info.count, parent_id, depth, true);

			tcc_clk_print(sys_bus_info.clk_list, sys_bus_info.count, parent_id, depth, false);
			tcc_clk_print(cpu_bus_info.clk_list, cpu_bus_info.count, parent_id, depth, false);
			tcc_clk_print(cam_bus_info.clk_list, cam_bus_info.count, parent_id, depth, false);
			tcc_clk_print(npu_bus_info.clk_list, npu_bus_info.count, parent_id, depth, false);
			tcc_clk_print(peri_info.clk_list, peri_info.count, parent_id, depth, false);
			break;
		/* BUS + PERI */
		case 3:
			tcc_clk_print(sys_bus_info.clk_list, sys_bus_info.count, parent_id, depth, false);
			tcc_clk_print(cpu_bus_info.clk_list, cpu_bus_info.count, parent_id, depth, false);
			tcc_clk_print(cam_bus_info.clk_list, cam_bus_info.count, parent_id, depth, false);
			tcc_clk_print(npu_bus_info.clk_list, npu_bus_info.count, parent_id, depth, false);
			tcc_clk_print(peri_info.clk_list, peri_info.count, parent_id, depth, false);
			break;
		default:
			break;
	}
}

static int do_tcc_clk_show(struct cmd_tbl *cmdtp, int flag,
			   int argc, char * const argv[])
{
	struct arm_smccc_res res;
	unsigned long rate;
	unsigned int i, div, src_id;
	struct clk_list *info;
	int ret = 0;

	get_clk_list(SRC, &src_info);
	get_clk_list(PLL, &pll_info);
	get_clk_list(PLL_DIV, &pll_div_info);
	get_clk_list(SYS_BUS, &sys_bus_info);
	get_clk_list(CPU_BUS, &cpu_bus_info);
	get_clk_list(CAM_BUS, &cam_bus_info);
	get_clk_list(NPU_BUS, &npu_bus_info);
	get_clk_list(PERI, &peri_info);

	/* Get PLL Div Clock */
	info = pll_div_info.clk_list;
	for (i = 0; i < pll_div_info.count; i++) {
		arm_smccc_smc(SIP_CLK_GET_PLL_DIV, info[i].id, 0, 0, 0, 0, 0, 0, &res);
		if (res.a0 < 0) {
			pr_force("Unsupported feature, TF-A needs to be updated\n");
			ret = -ENOSYS;
			break;
		} else if (res.a2 < 0) {
			pr_force("Failed to get [%s] info, ret: %lu\n",
				info[i].name,
				res.a2);
			rate = 0;
			src_id = -ENODEV;
			div = 0;
		} else {
			rate = res.a0;
			src_id = info[i].src_id;
			div = res.a1;
		}


		info[i].rate = rate;
		info[i].src_id = src_id;
		info[i].div = div;
	}

	if (ret == 0) {
		/* Get PLL Clock */
		info = pll_info.clk_list;
		for (i = 0; i < pll_info.count; i++) {
			rate = 0;
			src_id = PLL_XIN;
			div = 0;

			arm_smccc_smc(SIP_CLK_GET_PLL, info[i].id, 0, 0, 0, 0, 0, 0, &res);
			if (res.a1 < 0) {
				pr_force("Failed to get [%s] info, ret: %lu\n",
					info[i].name,
					res.a1);
				src_id = -ENODEV;
			} else {
				rate = res.a0;
			}

			info[i].rate = rate;
			info[i].src_id = src_id;
			info[i].div = div;
		}

		/* Get System Bus Clock */
		info = sys_bus_info.clk_list;
		for (i = 0; i < sys_bus_info.count; i++) {
			arm_smccc_smc(SIP_CLK_GET_CLKCTRL, info[i].id, 0, 0, 0, 0, 0, 0, &res);
			if (res.a3 < 0) {
				pr_force("Failed to get [%s] info, ret: %lu\n",
					info[i].name,
					res.a3);
				rate = 0;
				src_id = -ENODEV;
				div = 0;
			} else {
				rate = res.a0;
				src_id = res.a1;
				div = res.a2;
			}

			info[i].rate = rate;
			info[i].src_id = sys_bclk_sel_to_parent_id(src_id);
			info[i].div = div;
		}

		/* Get CPU Bus Clock */
		info = cpu_bus_info.clk_list;
		for (i = 0; i < cpu_bus_info.count; i++) {
			arm_smccc_smc(SIP_CLK_GET_CLKCTRL, info[i].id, 0, 0, 0, 0, 0, 0, &res);
			if (res.a3 < 0) {
				pr_force("Failed to get [%s] info, ret: %lu\n",
					info[i].name,
					res.a3);
				rate = 0;
				src_id = -ENODEV;
				div = 0;
			} else {
				rate = res.a0;
				src_id = res.a1;
				div = res.a2;
			}

			info[i].rate = rate;
			info[i].src_id = cpu_bclk_sel_to_parent_id(src_id);
			info[i].div = div;
		}

		/* Get CAM Bus Clock */
		info = cam_bus_info.clk_list;
		for (i = 0; i < cam_bus_info.count; i++) {
			arm_smccc_smc(SIP_CLK_GET_CLKCTRL, info[i].id, 0, 0, 0, 0, 0, 0, &res);
			if (res.a3 < 0) {
				pr_force("Failed to get [%s] info, ret: %lu\n",
					info[i].name,
					res.a3);
				rate = 0;
				src_id = -ENODEV;
				div = 0;
			} else {
				rate = res.a0;
				src_id = res.a1;
				div = res.a2;
			}

			info[i].rate = rate;
			info[i].src_id = cam_bclk_sel_to_parent_id(src_id, info[i].id);
			info[i].div = div;
		}

		/* Get NPU Bus Clock */
		info = npu_bus_info.clk_list;
		for (i = 0; i < npu_bus_info.count; i++) {
			arm_smccc_smc(SIP_CLK_GET_CLKCTRL, info[i].id, 0, 0, 0, 0, 0, 0, &res);
			if (res.a3 < 0) {
				pr_force("Failed to get [%s] info, ret: %lu\n",
					info[i].name,
					res.a3);
				rate = 0;
				src_id = -ENODEV;
				div = 0;
			} else {
				rate = res.a0;
				src_id = res.a1;
				div = res.a2;
			}

			info[i].rate = rate;
			info[i].src_id = npu_bclk_sel_to_parent_id(src_id);
			info[i].div = div;
		}

		/* Get Peri Clock */
		info = peri_info.clk_list;
		for (i = 0; i < peri_info.count; i++) {
			arm_smccc_smc(SIP_CLK_GET_PCLKCTRL, info[i].id, 0, 0, 0, 0, 0, 0, &res);
			rate = res.a0;
			src_id = res.a1;
			div = res.a2;

			info[i].rate = rate;
			info[i].src_id = pclk_sel_to_parent_id(src_id);
			info[i].div = div;
		}

		pr_force("--------------------------------------------------------------------------------\n");
		pr_force("              clock                       Rate              Divider             \n");
		pr_force("--------------------------------------------------------------------------------\n");
		tcc_clk_show_clk_info(0, 0);
	}

	return 0;
}

U_BOOT_CMD(
	   clock, 2, 1, do_tcc_clk_show, "Telechips Clock Information tool", ""
	  );
