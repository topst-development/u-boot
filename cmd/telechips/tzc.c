// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022, Telechips Inc.
 */

#include <stdlib.h>
#include <command.h>
#include <cpu_func.h>
#include <linux/libfdt.h>
#include <linux/io.h>
#include <mach/smc.h>

/**
 * @brief TrustZone Address Space Control Test
 *
 */
#define ID_ACCESS_WRITE(id)	(1UL<<(16+(id)))
#define ID_ACCESS_WRITE_ALL	(0xFFFF0000UL)
#define ID_ACCESS_READ(id)	(1UL<<(id))
#define ID_ACCESS_READ_ALL	(0x0000FFFFUL)

#define TZC_NAME_MAX	16

/*
 * Controls secure access to a region. If not enabled secure access is not
 * allowed to region.
 */
enum tzc_region_attributes {
	TZC_REGION_S_NONE = 0,
	TZC_REGION_S_RD = 1,
	TZC_REGION_S_WR = 2,
	TZC_REGION_S_RDWR = (TZC_REGION_S_RD | TZC_REGION_S_WR)
};

struct tzc_region_cfg {
	uint64_t base;
	uint64_t top;
	uint32_t filter;
	enum tzc_region_attributes sec_attr;
	uint32_t ns_device_access;
};

enum tzc_type {
	TZC_TYPE_TZC = 0,
	TZC_TYPE_OMC,
};

struct master_id_t {
	uintptr_t reg;
	uint32_t offs;
};
struct tzc_test_t {
	uint32_t base;
	enum tzc_type type;
	struct master_id_t m_id;
	uintptr_t addr;
	uint32_t action;
	uint32_t region_num;	/* region numbers */
	uint32_t offset;	/* avaliable region start offset*/
	uint32_t filters;
};

static int tee_read32(uintptr_t addr, uint32_t *val)
{
	struct arm_smccc_res res;
	int ret = -1;

	arm_smccc_smc(SIP_SEC_MEMORY_READ, addr, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 == 0U) {
		if (val == NULL) {
			pr_force("read (tee): 0x%08lx\n", res.a1);
		} else {
			*val = res.a1;
		}
		ret = 0;
	}

	return ret;
}

static int tee_write32(uintptr_t addr, uint32_t val)
{
	struct arm_smccc_res res;
	int ret = -1;

	arm_smccc_smc(SIP_SEC_MEMORY_WRITE, addr, val, 0, 0, 0, 0, 0, &res);
	if (res.a0 == 0U) {
		ret = 0;
	}

	return ret;
}

static uint32_t ree_read32(uintptr_t addr)
{
	return readl(addr);
}

static void ree_write32(uintptr_t addr, uint32_t val)
{
	writel(val, addr);
	flush_dcache_all();
}

static int tzc_init(uint32_t paddr, struct tzc_test_t *item)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_TZC_GET_INFO, paddr, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 == 0U) {
		item->type = res.a1;
		item->filters = res.a2;
		item->region_num = res.a3;
	}
	return res.a0;
}

static int tzc_set_region(uint32_t region, uint32_t filters,
			  struct tzc_region_cfg *tzc)
{
	struct arm_smccc_res res;

	/* allow sec/nonsec access */
	arm_smccc_smc(SIP_TZC_SET_CONFIG, region, filters, TZC_REGION_S_RDWR,
		ID_ACCESS_WRITE_ALL|ID_ACCESS_READ_ALL, 0, 0, 0, &res);

	/* set memory region */
	arm_smccc_smc(SIP_TZC_SET_ADDRESS, region, tzc->base, tzc->top,
					0, 0, 0, 0, &res);
	if (res.a0 != 0U) {
		goto exit;
	}

	/* set secure attribute */
	arm_smccc_smc(SIP_TZC_SET_CONFIG, region, filters, tzc->sec_attr,
					tzc->ns_device_access, 0, 0, 0, &res);
	if (res.a0 != 0U) {
		goto exit;
	}

exit:
	return res.a0;
}

static int tzc_get_region(u32 region, struct tzc_region_cfg *tzc)
{
	struct arm_smccc_res res;
	struct tzc_region_cfg tzc_tmp;
	int ret = -1;

	arm_smccc_smc(SIP_TZC_GET_ADDRESS, region, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 == 0U) {
		tzc_tmp.base = res.a1;
		tzc_tmp.top = res.a2;

		arm_smccc_smc(SIP_TZC_GET_CONFIG, region, 0, 0, 0, 0, 0, 0, &res);
		if (res.a0 == 0U) {
			tzc_tmp.filter = res.a1;
			tzc_tmp.sec_attr = res.a2;
			tzc_tmp.ns_device_access = res.a3;
			memcpy(tzc, &tzc_tmp, sizeof(struct tzc_region_cfg));
			ret = 0;
		}
	}

	return ret;
}

static int tzc_region_dump(struct tzc_test_t *item)
{
	struct tzc_region_cfg tzc;
	int region;

	for(region=0 ; region < item->region_num ; region++) {
		if (tzc_get_region(region, &tzc) != 0) {
			break;
		}

		pr_force("region_%02d: %016llx--%016llx: filter(0x%x), access(0x%08x), attr(0x%x)\n",
			region, tzc.base, tzc.top, tzc.filter,
			tzc.ns_device_access, tzc.sec_attr);
	}
	pr_force("\n");

	return CMD_RET_SUCCESS;
}

static int tzc_info(int argc, char * const argv[])
{
	uintptr_t addr = simple_strtoul(argv[0], NULL, 16);
	uintptr_t offs = simple_strtoul(argv[1], NULL, 16);
	struct tzc_test_t item;
	int idx, rc, tzc_cnt = 0;

	for (idx = 0 ; idx < 16 ; idx++) {
		rc = tzc_init(addr + (offs*idx), &item);
		if (rc != 0) {
			continue;
		} else {
			tzc_cnt += 1;
		}

		pr_force("\n   ### %s: base:0x%lx filtes:%d region_num:%d ###\n",
				(item.type == TZC_TYPE_TZC) ? "TZC-400" : "TZC OMC",
				addr+(offs*idx), item.filters, item.region_num);
		tzc_region_dump(&item);
	}

	if (tzc_cnt == 0) {
		pr_err("\n   ### No TZC Controller. ###\n");
		rc = CMD_RET_FAILURE;
	} else {
		rc = CMD_RET_SUCCESS;
	}

	return rc;
}

static int tzc_region_attribute_config(struct tzc_region_cfg *tzc,
				      const struct tzc_test_t *item,
				      uint32_t region, int case_id)
{
	uint32_t ree_rd, ree_wr, tee_rd, tee_wr;

	const char * case_str[] = {
		"secure read. ",
		"secure write.",
		"secure rd/wr.",
		"non-secure read. ",
		"non-secure write.",
		"non-secure rd/wr.",
	};
	int rc = CMD_RET_FAILURE;

	ree_wr = (region << 16) | (region << 0);
	tee_wr = (region << 24) | (region << 8);

	if ((item->action == 0) && (item->m_id.reg == 0)) {
		pr_force("o region_%d: %s", region, case_str[case_id]);
	}

	tzc_set_region(region, item->filters, tzc);

	if (case_id < 3) {
		ree_write32(item->addr, ree_wr);
		tee_write32(item->addr, tee_wr);
		tee_read32(item->addr, &tee_rd);
		ree_rd = ree_read32(item->addr);
		if ((item->action == 0) && (item->m_id.reg == 0)) {
			pr_force(" Rw:0x%08x, Tw:0x%08x, Tr:0x%08x, Rr:0x%08x\n",
					ree_wr, tee_wr, tee_rd, ree_rd);
		}
	} else {
		tee_write32(item->addr, tee_wr);
		ree_write32(item->addr, ree_wr);
		ree_rd = ree_read32(item->addr);
		tee_read32(item->addr, &tee_rd);
		if ((item->action == 0) && (item->m_id.reg == 0)) {
			pr_force(" Tw:0x%08x, Rw:0x%08x, Rr:0x%08x, Tr:0x%08x\n",
					tee_wr, ree_wr, ree_rd, tee_rd);
		}

	}

	if ((case_id == 0) && (tee_wr == ree_rd) && (tee_rd == 0)) {
		rc = CMD_RET_SUCCESS;
	} else if ((case_id == 1) && (ree_wr == tee_rd) && (tee_rd == ree_rd)) {
		rc = CMD_RET_SUCCESS;
	} else if ((case_id == 2) && (ree_wr == ree_rd) && (tee_rd == 0)) {
		rc = CMD_RET_SUCCESS;
	} else if ((case_id == 3) && (ree_wr == tee_rd) && (ree_rd == 0)) {
		rc = CMD_RET_SUCCESS;
	} else if ((case_id == 4) && (tee_wr == ree_rd) && (ree_rd == tee_rd)) {
		rc = CMD_RET_SUCCESS;
	} else if ((case_id == 5) && (tee_wr == tee_rd) && (ree_rd == 0)) {
		rc = CMD_RET_SUCCESS;
	}

	return rc;
}

static int tzc_region_attribute_test(const struct tzc_region_cfg *p_tzc,
				     const struct tzc_test_t *item, uint32_t region)
{
	struct tzc_region_cfg *tzc = NULL;
	int rc = CMD_RET_FAILURE;

	tzc = malloc(sizeof(struct tzc_region_cfg));
	if (tzc == NULL)
		goto err;

	memcpy(tzc, p_tzc, sizeof(struct tzc_region_cfg));

	if (region == 1) {
		pr_force("* secure block transter test. (ree_wr->tee_wr->tee_rd->ree_rd)\n");
	}

	pr_force("* region_%d: secure block transter test.\n", region);
	tzc->ns_device_access = ID_ACCESS_WRITE_ALL|ID_ACCESS_READ_ALL;
	tzc->sec_attr = TZC_REGION_S_WR;
	rc = tzc_region_attribute_config(tzc, item, region, 0);
	if (rc == CMD_RET_FAILURE)
		goto err;
	tzc->sec_attr = TZC_REGION_S_RD;
	rc = tzc_region_attribute_config(tzc, item, region, 1);
	if (rc == CMD_RET_FAILURE)
		goto err;
	tzc->sec_attr = TZC_REGION_S_NONE;
	rc = tzc_region_attribute_config(tzc, item, region, 2);
	if (rc == CMD_RET_FAILURE)
		goto err;

	/* reset tzc setting before return */
	memset(tzc, 0x0, sizeof(struct tzc_region_cfg));
	tzc->top = 0xfff;
	tzc_set_region(region, item->filters, tzc);

	pr_force("  region_%d: OK\n", region);
	rc = CMD_RET_SUCCESS;

err:
	free(tzc);
	if (rc != CMD_RET_SUCCESS) {
		pr_force("  region_%d: FAILED\n", region);
	}

	return rc;
}

static int tzc_nsec_access_test(const struct tzc_region_cfg *p_tzc,
				const struct tzc_test_t *item, uint32_t region)
{
	struct tzc_region_cfg *tzc = NULL;
	uint32_t m_id = 0, value, mask;
	int rc = CMD_RET_FAILURE;

	tzc = malloc(sizeof(struct tzc_region_cfg));
	if (tzc == NULL)
		goto err;

	memcpy(tzc, p_tzc, sizeof(struct tzc_region_cfg));

	if (region == 1) {
		pr_force("* non-secure block transter test. (tee_wr->ree_wr->ree_rd->tee_rd)\n");
	}

	if (item->m_id.reg != 0) {
		rc = tee_read32(item->m_id.reg, &value);
		if (rc != 0) {
			goto err;
		} else {
			mask = value & (~(0xFFUL << item->m_id.offs));
		}
	}

	for (m_id = 0 ; m_id < 16 ; m_id++) {
		pr_force("* region_%d: id:%d: non-secure block transter test.\n", region, m_id);
		if (item->m_id.reg != 0) {
			value = mask | (((m_id<<4)|m_id)<<item->m_id.offs);
			tee_write32(item->m_id.reg, value);
		}

		tzc->sec_attr = TZC_REGION_S_RDWR;
		tzc->ns_device_access = ID_ACCESS_WRITE(m_id);
		rc = tzc_region_attribute_config(tzc, item, region, 3);
		if (rc == CMD_RET_FAILURE)
			goto err;
		tzc->ns_device_access = ID_ACCESS_READ(m_id);
		rc = tzc_region_attribute_config(tzc, item, region, 4);
		if (rc == CMD_RET_FAILURE)
			goto err;
		tzc->ns_device_access = 0x00000000;
		rc = tzc_region_attribute_config(tzc, item, region, 5);
		if (rc == CMD_RET_FAILURE)
			goto err;
		pr_force("  region_%d: id:%d: OK\n", region, m_id);

		if (item->m_id.reg == 0) {
			break;
		}
	}

	/* reset tzc setting before return */
	memset(tzc, 0x0, sizeof(struct tzc_region_cfg));
	tzc->top = 0xfff;
	tzc_set_region(region, item->filters, tzc);

	pr_force("  region_%d: OK\n", region);
	rc = CMD_RET_SUCCESS;

err:
	free(tzc);
	if (rc != CMD_RET_SUCCESS) {
		pr_force("  region_%d: id:%d: FAILED\n", region, m_id);
	}

	return rc;
}


static int tzc_access_test(struct tzc_region_cfg *tzc,
			 struct tzc_test_t *item)
{
	uint32_t region = 1;
	int rc;

	/* secure/non-secure transfer block test without action */
	for (region = 1 ; region < item->region_num ; region++) {
		rc = tzc_region_attribute_test(tzc, item, region);
		if (rc == CMD_RET_FAILURE) {
			goto exit;
		}
		if (item->action > 0) {
			break;
		}
	}
	pr_force("  OK\n\n");

	for (region = 1 ; region < item->region_num ; region++) {
		rc = tzc_nsec_access_test(tzc, item, region);
		if (rc == CMD_RET_FAILURE) {
			goto exit;
		}
		if (item->action > 0) {
			break;
		}
	}
	pr_force("  OK\n\n");

exit:
	if (rc != CMD_RET_SUCCESS) {
		tzc_region_dump(item);
	}

	return rc;
}

static int tzc_verify(int argc, char * const argv[])
{
	struct tzc_region_cfg tzc;
	struct tzc_test_t item;
	uint32_t region;

	memset(&item, 0x0, sizeof(struct tzc_test_t));

	switch (argc) {
	case 2:
		item.base = simple_strtoul(argv[0], NULL, 16);
		item.addr =  simple_strtoul(argv[1], NULL, 16);
		break;
	default:
		return CMD_RET_USAGE;
	}
	if (tzc_init(item.base, &item) != 0) {
		return CMD_RET_FAILURE;
	}

	/* reset all regions for test */
	memset(&tzc, 0x0, sizeof(struct tzc_region_cfg));
	for(region=item.region_num-1 ; region > 0 ; region--) {
		if (tzc_set_region(region, 0, &tzc) != 0) {
			pr_err("failed to reset %d region\n", region);
		}
	}

	/* set region 0 */
	tzc.filter = item.filters;
	tzc.ns_device_access = ID_ACCESS_WRITE_ALL|ID_ACCESS_READ_ALL;
	tzc.sec_attr = TZC_REGION_S_RDWR;
	tzc_set_region(0, item.filters, &tzc);

	/* run tzc access test*/

	/* set test region base and top */
	if (item.type == TZC_TYPE_OMC) {
		if (item.addr < DRAM_BASE) {
			return CMD_RET_FAILURE;
		} else if ( true
#ifdef CONFIG_ARM64
			&& (((item.addr>>32) & 0xFFFFFFFFUL) == 0UL)
#endif
			) {
			tzc.base = item.addr - DRAM_BASE;
			tzc.top = (item.addr - DRAM_BASE) + 0xfff;
		} else {
			tzc.base = item.addr;
			tzc.top = item.addr + 0xfff;
		}
	} else {
		tzc.base = (item.addr&0xFFFFF000);
		tzc.top = (item.addr&0xFFFFF000) + 0xfff;
	}

	pr_force("\n======================================================================\n");
	if (item.type == TZC_TYPE_OMC) {
		pr_force("*  OMC-TZC (DRAM) IP-PV Test\n*\n");
	} else {
		pr_force("*  TZC-400 (reg:0x%08x) IP-PV Test\n*\n", item.base);
	}
	pr_force("*  - Test memory address: 0x%lx\n", item.addr);
	if (item.m_id.reg) {
		pr_force("*  - Test Master(Main Core) ID reg: 0x%lx & 0xFF<<%d\n",
			item.m_id.reg, item.m_id.offs);
	}
	pr_force("======================================================================\n");

	return tzc_access_test(&tzc, &item);
}

static int do_tzc(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd = argv[1];
	int rc = CMD_RET_FAILURE;

	if (argc != 4) {
		rc = CMD_RET_USAGE;
	} else if (!strcmp(cmd, "info")) {
		rc = tzc_info(argc - 2, &argv[2]);
	} else if (!strcmp(cmd, "verify")) {
		rc = tzc_verify(argc - 2, &argv[2]);
	} else {
		rc = CMD_RET_USAGE;
	}

	if ((rc != CMD_RET_SUCCESS) && (rc != CMD_RET_USAGE)) {
		pr_err("Failed to run %s\n", argv[0]);
	}

	return rc;
}

U_BOOT_CMD(tzc, 4, 0, do_tzc,
	   "TrustZone Address Space Control Test",
	   "<cmd>      [<args>, ...]\n"
	   "info       <base> <offs> - print all tzc-400 infomation\n"
	   "           <base> : tzc base address\n"
	   "           <offs> : each tzc controller offsets\n"
	   "verify     <base> <addr> - run full verification\n"
	   "           <base> : tzc base address\n"
	   "           <addr> : memory address for test\n"
);
