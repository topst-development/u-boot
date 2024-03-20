// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <fdtdec.h>
#include <mach/smc.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_ARM64)
#include <asm/armv8/mmu.h>

#define MT_DEVICE \
	(u64)PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | \
	(u64)PTE_BLOCK_NON_SHARE | (u64)PTE_BLOCK_PXN | (u64)PTE_BLOCK_UXN

#define MT_DRAM_AREA \
	(u64)PTE_BLOCK_MEMTYPE(MT_NORMAL) | \
	(u64)PTE_BLOCK_NON_SHARE

#define MM_REGION_IO	(0)
#define MM_REGION_DRAM	(1)

static struct mm_region tcc_mm_region[] = {
	/* I/Os */ { 0x10000000ULL, 0x10000000ULL, 0x10000000ULL, MT_DEVICE },
	/* DRAM */ { 0, 0, 0, MT_DRAM_AREA },
	/* ---- */ { 0, 0, 0, 0 }
};

struct mm_region *mem_map = tcc_mm_region;

static void memory_region_init(void)
{
	struct mm_region *mr = &tcc_mm_region[MM_REGION_DRAM];

	mr->virt = gd->ram_base;
	mr->phys = gd->ram_base;
	mr->size = gd->ram_size;
}
#else
static void memory_region_init(void) {}
#endif

static void memory_map_init(void)
{
	phys_size_t size;
	s32 ret;

	gd->ram_base = DRAM_BASE;
	gd->ram_size = DRAM_SIZE;

	ret = fdtdec_setup_mem_size_base();
	size = gd->ram_size;

	if ((ret == 0) && (size > DRAM_SIZE)) {
		gd->ram_size = DRAM_SIZE;
	}

	memory_region_init();
}

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
	ret = reserve_mmu();

	/* Now we don't need relocaddr anymore */
	gd->relocaddr = 0UL;

	return ret;
}
#else
static s32 reserve_mmu_early(void) {}
#endif

int mach_cpu_init(void)
{
	s32 ret;

#if !defined(CONFIG_ARM64)
	/*
	 * For AArch32 build, mmu_setup() requires bi_dram setup to get
	 * address range to enable d-cache.
	 *
	 * bd_t is not allocated yet on this stage, so set temporary bd_t
	 * and map bi_dram to cover DRAM area in 32-bit address space.
	 */
	bd_t tmp_bd;

	gd->bd = &tmp_bd;
	gd->bd->bi_dram[0].start = DRAM_BASE;
	gd->bd->bi_dram[0].size = DRAM_SIZE;
	gd->bd->bi_dram[1].start = 0;
	gd->bd->bi_dram[1].size = 0;
#endif

	ret = reserve_mmu_early();

	if (ret == 0) {
		memory_map_init();
		enable_caches();
	}

	gd->bd = NULL;

	return ret;
}

int dram_init(void) { return 0; }

#if defined(CONFIG_TCC_MAINCORE)
static void dram_init_actual_banksize(void)
{
	struct arm_smccc_res res;
	u64 actual_size;

	arm_smccc_smc(SIP_GET_DRAM_SIZE, 0, 0, 0, 0, 0, 0, 0, &res);
	actual_size = (u64)res.a0 << 20U;

	if (actual_size <= DRAM_SIZE) {
		gd->bd->bi_dram[0].size = actual_size;
		gd->bd->bi_dram[1].start = 0;
		gd->bd->bi_dram[1].size = 0;
	} else {
		gd->bd->bi_dram[0].size = DRAM_SIZE;
		gd->bd->bi_dram[1].start = DRAM_BASE_EXT;
		gd->bd->bi_dram[1].size = actual_size - DRAM_SIZE;
	}
}
#else
static void dram_init_actual_banksize(void) {}
#endif

#if defined(CONFIG_NR_DRAM_BANKS)
int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = gd->ram_size;

	dram_init_actual_banksize();

	return 0;
}
#endif
