// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <init.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <linux/sizes.h>
#include <mach/smc.h>

#define MT_MEMORY \
	(u64)PTE_BLOCK_MEMTYPE(MT_NORMAL) | \
	(u64)PTE_BLOCK_NON_SHARE

#define MT_DEVICE \
	(u64)PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | \
	(u64)PTE_BLOCK_NON_SHARE | (u64)PTE_BLOCK_PXN | (u64)PTE_BLOCK_UXN

DECLARE_GLOBAL_DATA_PTR;

#if !(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
#if defined(CONFIG_PRAM)
#error "Cannot enable cache early on board_init_f if CONFIG_PRAM is set!"
#endif
static s32 reserve_mmu_early(void)
{
	s32 ret;

	/* From setup_dest_addr() */
	gd->relocaddr = board_get_usable_ram_top(gd->mon_len);

	/* From reserve_round_4k() */
	gd->relocaddr &= ~(4096UL - 1UL);

	/* Call reserve_mmu() earlier to pre-calculate tlb info */
	ret = arch_reserve_mmu();

	/* Now we don't need relocaddr anymore */
	gd->relocaddr = 0UL;

	return ret;
}
#else
static s32 reserve_mmu_early(void) {}
#endif

/* Always keep main memory region on top */
static struct mm_region arch_mem_map[] = {
	{
		/* main memory */
		.virt = 0,
		.phys = 0,
		.size = 0,
		.attrs = MT_MEMORY,
	}, {
		/* devices */
		.virt = DEVICE_BASE,
		.phys = DEVICE_BASE,
		.size = DEVICE_SIZE,
		.attrs = MT_DEVICE,
	},
#ifdef CONFIG_PCIE_TELECHIPS
	{
		/* PCIe */
		.virt = 0x500000000ULL,
		.phys = 0x500000000ULL,
		.size = 0x40000000ULL,
		.attrs = MT_DEVICE,
	},
#endif
	{
		/* sentinel */
		.size = 0,
	}
};

/* For arch/arm/cpu/armv8/cache_v8.c */
struct mm_region *mem_map = arch_mem_map;

static void memory_region_init(void)
{
	mem_map->virt = gd->ram_base;
	mem_map->phys = gd->ram_base;
	mem_map->size = gd->ram_size;
}

static void memory_map_init(void)
{
	s32 ret = fdtdec_setup_mem_size_base();

	if (ret < 0) {
		gd->ram_base = DRAM_BASE;
		gd->ram_size = DRAM_SIZE;
	}

	memory_region_init();
}

int mach_cpu_init(void)
{
	s32 ret = reserve_mmu_early();

	if (ret == 0) {
		memory_map_init();
		enable_caches();
	}

	return ret;
}

int dram_init(void)
{
	return 0;
}

#if defined(CONFIG_USE_MAINCORE)
int dram_init_banksize(void)
{
	struct arm_smccc_res res;
	u64 actual_size;

	arm_smccc_smc(SIP_GET_DRAM_SIZE, 0, 0, 0, 0, 0, 0, 0, &res);
	actual_size = (u64)res.a0 << 20U;

	if (actual_size <= DRAM_SIZE) {
		gd->bd->bi_dram[0].start = DRAM_BASE;
		gd->bd->bi_dram[0].size = actual_size;
		gd->bd->bi_dram[1].start = 0;
		gd->bd->bi_dram[1].size = 0;
	} else {
		gd->bd->bi_dram[0].start = DRAM_BASE;
		gd->bd->bi_dram[0].size = DRAM_SIZE;
		gd->bd->bi_dram[1].start = DRAM_BASE_EXT;
		gd->bd->bi_dram[1].size = actual_size - DRAM_SIZE;
	}

	return 0;
}
#else
int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = gd->ram_size;
	gd->bd->bi_dram[1].start = 0;
	gd->bd->bi_dram[1].size = 0;

	return 0;
}
#endif

ulong board_get_usable_ram_top(ulong total_size)
{
	ofnode node = ofnode_path("/config");
	u32 size = ofnode_read_u32_default(node, "u-boot,reloc-off", SZ_128M);

	if ((UINT_MAX - (u32)CONFIG_SYS_TEXT_BASE) < size) {
		/* Use default offset in case the size is too big */
		size = SZ_128M;
	}

	return (u32)CONFIG_SYS_TEXT_BASE + size;
}
