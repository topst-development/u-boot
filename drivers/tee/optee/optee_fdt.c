// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2023, Telechips Inc.
 */

#include <linux/arm-smccc.h>
#include <linux/libfdt.h>
#include <linux/printk.h>
#include <fdt_support.h>

#include "optee_smc.h"
#include "optee_msg.h"

#define OPTEE_MSG_FUNCID_GET_OS_BUILDDATE	0xFFF1

/*
 * Get build date of Trusted OS.
 * Returns: a0: date(yyyymmdd)
 *          a1: time(hhmmss)
 */
#define OPTEE_SMC_FUNCID_GET_OS_BUILDDATE OPTEE_MSG_FUNCID_GET_OS_BUILDDATE
#define OPTEE_SMC_CALL_GET_OS_BUILDDATE \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_GET_OS_BUILDDATE)

#define DATE_STRING_LENTH	32

struct optee_date_t {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};

static const char *month_str[] = {
	"---",
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};

static uint8_t convert_to_dec_8(unsigned long arg)
{
	char str[3];
	unsigned long val;

	(void)sprintf(str, "%02lx", arg & 0xFFu);
	val = strtoul(str, NULL, 10);

	return (uint8_t)(val & 0xFFu);
}

static uint16_t convert_to_dec_16(unsigned long arg)
{
	char str[5];
	unsigned long val;

	(void)sprintf(str, "%04lx", arg & 0xFFFFu);
	val = strtoul(str, NULL, 10);

	return (uint16_t)(val & 0xFFFFu);
}

static void get_optee_os_build_date(struct optee_date_t *date)
{
	struct arm_smccc_res res;

	arm_smccc_smc(OPTEE_SMC_CALL_GET_OS_BUILDDATE, 0, 0, 0, 0, 0, 0, 0,
		&res);

	if ((res.a0 & 0x80000000u) == 0u) {
		date->year = convert_to_dec_16(res.a0>>16);
		date->month = convert_to_dec_8(res.a0>>8);
		date->day = convert_to_dec_8(res.a0);
		date->hour = convert_to_dec_8(res.a1>>16);
		date->minute = convert_to_dec_8(res.a1>>8);
		date->second = convert_to_dec_8(res.a1);

		if ((date->month == 0u) || (date->month > 12u) ||
		    (date->day == 0u) || (date->day > 31u) ||
		    (date->hour >= 24u) || (date->minute >= 60u) ||
		    (date->second >= 60u)) {
			(void)memset(date, 0x0, sizeof(struct optee_date_t));
		}
	}
}

int fdt_parse_optee(void *fdt)
{
	int offset;
	struct optee_date_t date;
	char str[32];
	union {
		struct arm_smccc_res smccc;
		struct optee_smc_call_get_os_revision_result result;
	} res = {
		.result = {
		.build_id = 0
		}
	};

	(void)memset(&date, 0x0, sizeof(struct optee_date_t));
	get_optee_os_build_date(&date);
	(void)sprintf(str, "(%s %u %02u:%02u:%02u KST %04u)",
		month_str[date.month], date.day,
		date.hour, date.minute, date.second, date.year);

	arm_smccc_smc(OPTEE_SMC_CALL_GET_OS_REVISION, 0, 0, 0, 0, 0, 0, 0,
		&res.smccc);

	if ((res.result.major & 0x80000000U) == 0U) {
		(void)pr_force("OPTEE OS: %lu.%lu.%lu-%08lx %s\n",
			res.result.major, res.result.minor,
			res.result.reserved1, res.result.build_id,
			str);

		if (fdt != NULL) {
			/* enable reserved memory */
			offset = fdt_node_offset_by_compatible(
				(const void *)fdt, -1, (const char*)"optee.os");
			if (offset > 0) {
				(void)fdt_setprop_string(fdt, offset,
					"status", "okay");
			} else {
				panic("optee.os is not defined in device tree\n");
			}
		}
	} else {
		(void)pr_force("OPTEE OS may not be stored in storage. res: 0x%lx\n",
			res.result.major);
	}

	return 0;
}
