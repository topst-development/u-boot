/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCC_CLK_LIST
#define TCC_CLK_LIST

#define SRC		0
#define PLL		1
#define PLL_DIV		2
#define SYS_BUS		3
#define CPU_BUS		4
#define CAM_BUS		5
#define NPU_BUS		6
#define PERI		7

struct clk_list {
	unsigned long id;
	char *name;
	unsigned int src_id;
	unsigned int div;
	unsigned long rate;
};

struct clk_info {
	struct clk_list *clk_list;
	unsigned int count;
};

int get_clk_list(unsigned int id, struct clk_info *info);
int sys_bclk_sel_to_parent_id(unsigned int sel);
int cpu_bclk_sel_to_parent_id(unsigned int sel);
int cam_bclk_sel_to_parent_id(unsigned int sel, unsigned int id);
int npu_bclk_sel_to_parent_id(unsigned int sel);
int pclk_sel_to_parent_id(unsigned int sel);

#endif
