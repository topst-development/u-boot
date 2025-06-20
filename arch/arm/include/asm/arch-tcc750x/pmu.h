/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef TCC750X_PMU_H
#define TCC750X_PMU_H

#include <linux/arm-smccc.h>
#include <mach/smc.h>

#define PMU_BASE		(0x1E800000U)

#define PMU_CONFIG		(PMU_BASE + 0x014U)
#define PMU_ECID_UID_PD0	(PMU_BASE + 0x100U)
#define PMU_ECID_UID_PD1	(PMU_BASE + 0x104U)
#define PMU_SECURE_INF01	(PMU_BASE + 0x1C0U)

/* PMU_CONFIG: Boot Mode Port Status */
#define PMU_BM_MASK		(0x0000000FU)

/* PMU_SECURE_INF01: SecureBoot Enable Status */
#define PMU_SB_ENABLE_MASK	(0x00FF0000U)

inline static u32 pmu_readl(u32 addr)
{
	struct arm_smccc_res res;
	arm_smccc_smc(SIP_SEC_MEMORY_READ, (u64)addr, 0, 0, 0, 0, 0, 0, &res);
	return (u32)res.a0;
}

#endif /* TCC750X_PMU_H */
