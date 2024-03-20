// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include <linux/io.h>
#include <linux/kernel.h>

#define MAX_POLL_COUNT	1000
static void __iomem *base;
static u32 code;
static u32 data;

static enum command_ret_t poll_wait(u32 offs, u32 req_val)
{
	u32 value;
	int cnt;

	if (!base || !code || !data) {
		pr_err("HPI|Code|Data address is not set\n");
		return CMD_RET_FAILURE;
	}

	for (cnt = 0 ; cnt < MAX_POLL_COUNT ; cnt++) {
		value = ioread32(base + offs);
		if (value == req_val)
			break;
		mdelay(1);
	}

	if (value != req_val) {
		pr_err("0x%02x: 0x%08x. riquired(0x%08x)\n",
		       offs, value, req_val);
		pr_err("0x%02x: 0x%08x\n", 0x60, ioread32(base + 0x60));
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

static enum command_ret_t send_cmd(u32 cmd, u32 req_val)
{
	enum command_ret_t ret;
	int value;

	ret = poll_wait(0x3c, 0x00000000);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	iowrite32(0x0, base + 0x30);
	iowrite32(0x0, base + 0x34);
	iowrite32(cmd, base + 0x38);
	do {
		value = ioread32(base + 0x14);
	} while (!value);

	ret = poll_wait(0x48, req_val);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	ret = poll_wait(0x40, 0x0);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	iowrite32(0x1, base + 0x4c);
	iowrite32(value, base + 0x14);

	return CMD_RET_SUCCESS;
}

u32 atox(char *str)
{
	u32 val, ret;
	int len, i;

	len = strlen(str);
	if (len != 10 && len != 8)
		return 0;
	else if ((len == 10) && strncmp(str, "0x", 2))
		return 0;

	ret = 0;
	for (i = (len - 8) ; i < len ; i++) {
		if ((str[i] >= '0') && (str[i] <= '9'))
			val = str[i] - '0';
		else if ((str[i] >= 'A') && (str[i] <= 'F'))
			val = (str[i] - 'A') + 0xa;
		else if ((str[i] >= 'a') && (str[i] <= 'a'))
			val = (str[i] - 'a') + 0xa;
		else
			return 0;
		ret = (ret << 4) + val;
	}

	return ret;
}

static enum command_ret_t set_base(char *base_str)
{
	base = (void *)(uintptr_t)atox(base_str);
	if (!base) {
		pr_err("Invalid base address: %s\n", base_str);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static enum command_ret_t boot(char *code_str, char *data_str)
{
	enum command_ret_t ret;
	u32 val, cnt;

	if (!base) {
		pr_err("base address is not set\n");
		return CMD_RET_FAILURE;
	}

	code = atox(code_str);
	if (!code) {
		pr_err("Invalid code address: %s\n", code_str);
		return CMD_RET_FAILURE;
	}

	data = atox(data_str);
	if (!data) {
		pr_err("Invalid data address: %s\n", data_str);
		return CMD_RET_FAILURE;
	}

	flush_dcache_range(code, data);

	/*
	 * Set Code Base Address
	 */
	pr_force("Set code base\n");

	iowrite32(code, base + 0x20);
	ret = poll_wait(0x20, code);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	iowrite32(0x1, base + 0x2c);
	cnt = 1000;
	do {
		val = ioread32(base + 0x14);
		if ((val & (0x3<<16)) == (0x3<<16))
			break;
	} while (cnt--);
	if (cnt == 0)
		return CMD_RET_FAILURE;

	ret = poll_wait(0x48, 0x0000F201);
	if (ret != CMD_RET_SUCCESS)
		return ret;
	pr_force("Firmware version: 0x%08x\n", ioread32(base + 0x40));

	ret = poll_wait(0x44, 0x00000000);
	if (ret != CMD_RET_SUCCESS)
		return ret;
	iowrite32(0x1, base + 0x4c);
	iowrite32(val, base + 0x14);

	/*
	 * Set R/W Data Base Address
	 */
	pr_force("Set r/w data base\n");

	ret = poll_wait(0x3c, 0x00000000);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	iowrite32(data, base + 0x30);
	iowrite32(0x0, base + 0x34);

	iowrite32(0x0000F20E, base + 0x38);
	cnt = 1000;
	do {
		val = ioread32(base + 0x14);
		if ((val & (0x3<<16)) == (0x3<<16))
			break;
	} while (cnt--);
	if (cnt == 0)
		return CMD_RET_FAILURE;
	ret = poll_wait(0x48, 0x0000F20E);
	if (ret != CMD_RET_SUCCESS)
		return ret;

#if (0) // skip
	ret = poll_wait(0x40, data);
	if (ret != CMD_RET_SUCCESS)
		return ret;
#endif

	ret = poll_wait(0x44, 0x00000000);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	iowrite32(0x1, base + 0x4c);
	iowrite32(val, base + 0x14);

	pr_force("Firmware Boot Success\n");

	return CMD_RET_SUCCESS;
}

static enum command_ret_t authentication(void)
{
	enum command_ret_t ret;

	if (!base || !code || !data) {
		pr_err("HPI|Code|Data address is not set\n");
		return CMD_RET_FAILURE;
	}

	pr_force("Check the HDCP 2.x capability of the receiver\n");
	ret = send_cmd(0x42, 0x242);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	pr_force("Initiate %s\n", __func__);
	ret = send_cmd(0x47, 0x247);
	if (ret != CMD_RET_SUCCESS)
		return ret;

	pr_force("Authentication Success\n");

	return CMD_RET_SUCCESS;
}

static enum command_ret_t reset(void)
{
	void __iomem *dp_link;
	u32 val;

	if (!base) {
		pr_err("base address is not set\n");
		return CMD_RET_FAILURE;
	}

	dp_link = base - 0x40000;

	/* s/w reset */
	val = ioread32(dp_link+0x204);
	val |= 1<<2;
	iowrite32(val, dp_link+0x204);
	udelay(10);
	val &= ~(1<<2);
	iowrite32(val, dp_link+0x204);

	/* enable hdcp */
	iowrite32(0x2, dp_link+0xe00);

	return CMD_RET_SUCCESS;
}

static enum command_ret_t
do_dwc_hl(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *cmd;

	if (argc < 2)
		return CMD_RET_USAGE;

	cmd = argv[1];
	if (!strcmp(cmd, "base"))
		return set_base(argv[2]);
	else if (!strcmp(cmd, "boot") && (argc == 4))
		return boot(argv[2], argv[3]);
	else if (!strcmp(cmd, "auth") && (argc == 2))
		return authentication();
	else if (!strcmp(cmd, "reset") && (argc == 2))
		return reset();

	return CMD_RET_USAGE;
}

U_BOOT_CMD(dwc_hl,	4,	0,	do_dwc_hl,
	   "HDCP DP test without Host Library",
	   "<cmd>\n"
	   "       base <addr>        - Set HPI base address\n"
	   "       boot <code> <data> - ESM Firmware Boot\n"
	   "       auth               - Tx Authentication\n"
	   "       reset              - Reset HDCP"
);
