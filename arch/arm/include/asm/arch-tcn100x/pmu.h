/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCC807X_PMU_H
#define TCC807X_PMU_H

#include <linux/io.h>

#define PMU_BASE		(0x4B200000U)

#define PMU_CONFIG		(PMU_BASE + 0x004U)
#define PMU_ECID_UID_PD0	(PMU_BASE + 0x118U)
#define PMU_ECID_UID_PD1	(PMU_BASE + 0x11CU)
#define PMU_SECURE_INF02	(PMU_BASE + 0x15CU)

/* PMU_CONFIG: Boot Mode Port Status */
#define PMU_BM_MASK		(0x0000000FU)

/* PMU_SECURE_INF02: SecureBoot Enable Status */
#define PMU_SB_ENABLE_MASK	(0x0FFF0000U)

inline static u32 pmu_readl(u32 addr)
{
	void *virt = phys_to_virt(addr);
	return readl(virt);
}

#endif /* TCC807X_PMU_H */
