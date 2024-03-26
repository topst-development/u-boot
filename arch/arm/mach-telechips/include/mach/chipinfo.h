/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#ifndef MACH_CHIPINFO_H
#define MACH_CHIPINFO_H

#include <asm/arch/pmu.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <mach/smc.h>

#define INFO_UNK (~((u32)0))

/* Data structure for chip info and getters for it */
struct chip_info {
	u32 rev;
	u32 name;
	u64 id;
	u32 code;
};

static inline u32 get_chip_rev(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CHIP_REV, 0, 0, 0, 0, 0, 0, 0, &res);

	return ensure_u32(res.a0);
}

static inline u32 get_chip_name(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CHIP_NAME, 0, 0, 0, 0, 0, 0, 0, &res);

	return ensure_u32(res.a0);
}

static inline u32 get_chip_family(void)
{
	u32 name = get_chip_name();

	return name & 0xFFF0U;
}

static inline u64 get_chip_id(void)
{
	u64 uid[2];

	uid[0] = (u64)pmu_readl(PMU_ECID_UID_PD0);
	uid[1] = (u64)pmu_readl(PMU_ECID_UID_PD1);

	return ((uid[1] << 32U) | uid[0]) & ~((u64)1U << 63U);
}

static inline u32 get_chip_code(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CHIP_CODE, 0, 0, 0, 0, 0, 0, 0, &res);

	return (res.a0 == SMC_UNK) ? 0U : ensure_u32(res.a0);
}

static inline void get_chip_info(struct chip_info *info)
{
	info->rev = get_chip_rev();
	info->name = get_chip_name();
	info->id = get_chip_id();
	info->code = get_chip_code();
}

#define BOOT_INFO_DUAL_BOOT	(0U)
#define BOOT_INFO_SINGLE_BOOT	(1U)

#define is_dual_boot(bootsel)	((bootsel) == BOOT_INFO_DUAL_BOOT)

#define BOOT_INFO_MAIN_CORE	(0U)
#define BOOT_INFO_SUB_CORE	(1U)

#define is_main_core(coreid)	((coreid) == BOOT_INFO_MAIN_CORE)

#define PIN2PIN			(0x1U << 29)

/* Data structure for boot info and getters for it */
struct boot_info {
	u32 bootsel;
	u32 coreid;
};

static inline u32 get_boot_sel(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CHIP_GET_BOOT_INFO, 0, 0, 0, 0, 0, 0, 0, &res);

	return (res.a0 == SMC_OK) ? ensure_u32(res.a1) : INFO_UNK;
}

static inline u32 get_core_identity(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CHIP_GET_BOOT_INFO, 0, 0, 0, 0, 0, 0, 0, &res);

	return (res.a0 == SMC_OK) ? ensure_u32(res.a2) : INFO_UNK;
}

static inline void get_boot_info(struct boot_info *info)
{
	info->bootsel = get_boot_sel();
	info->coreid = get_core_identity();
}

/* Data structure for SiP version info and getters for it */
struct sip_version {
	u32 major;
	u32 minor;
	u32 patch;
};

static inline void get_sip_version(struct sip_version *ver)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_SVC_VERSION, 0, 0, 0, 0, 0, 0, 0, &res);

	ver->major = ensure_u32(res.a0);
	ver->minor = ensure_u32(res.a1);
	ver->patch = ensure_u32(res.a2);
}

/* Functions for getting/setting boot time info */
static inline void add_boot_time_stamp(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CHIP_ADD_BOOTTIME, 0, 0, 0, 0, 0, 0, 0, &res);
}

static inline void add_boot_time_stamp_dummy(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CHIP_ADD_BOOTTIME, 1, 0, 0, 0, 0, 0, 0, &res);
}

static inline u32 get_boot_time_stamp(ulong index)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CHIP_GET_BOOTTIME, index, 0, 0, 0, 0, 0, 0, &res);

	return ensure_u32(res.a0);
}

static inline u32 get_boot_time_stamp_num(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CHIP_GET_BOOTTIME_NUM, 0, 0, 0, 0, 0, 0, 0, &res);

	return ensure_u32(res.a0);
}

#endif /* MACH_CHIPINFO_H */
