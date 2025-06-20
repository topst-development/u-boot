// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <command.h>
#include <common.h>
#include <console.h>
#include <mach/smc.h>

static uint32_t smem_read(uint32_t addr)
{
	struct arm_smccc_res res;
	arm_smccc_smc(SIP_SEC_MEMORY_READ, addr, 0, 0, 0, 0, 0, 0, &res);
	return res.a0;
}

static uint32_t smem_write(uint32_t addr, uint32_t value)
{
	struct arm_smccc_res res;
	arm_smccc_smc(SIP_SEC_MEMORY_WRITE, addr, value, 0, 0, 0, 0, 0, &res);
	return res.a0;
}

static int do_read_smem(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	uint32_t addr, size, i;

	addr = simple_strtoul(argv[1], NULL, 16);
	if (addr % 4) {
		return CMD_RET_FAILURE;
	}

	size = simple_strtoul(argv[2], NULL, 16);
	if (size % 4) {
		return CMD_RET_FAILURE;
	}

	while (0 < size) {
		printf("[%08X]", addr);
		for (i = 0; (i < 4) && (size > 0); i++) {
			printf(" %08X", smem_read(addr));
			size -= 4;
			addr += 4;
		}
		printf("\n");
	}

	return CMD_RET_SUCCESS;
}

static int do_write_smem(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	uint32_t addr, value;

	addr = simple_strtoul(argv[1], NULL, 16);
	value = simple_strtoul(argv[2], NULL, 16);

	value = smem_write(addr, value);
	printf("[%08X] %08X\n", addr, value);

	return CMD_RET_SUCCESS;
}

static struct cmd_tbl cmd_smem[] = {
	U_BOOT_CMD_MKENT(r, 3, 0, do_read_smem, "",""),
	U_BOOT_CMD_MKENT(w, 3, 0, do_write_smem,"",""),
};

static int do_smem(struct cmd_tbl *cmdtp, int flag, int argc,
				char * const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_smem, ARRAY_SIZE(cmd_smem));

	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs) {
		return CMD_RET_USAGE;
	}

	if (flag == CMD_FLAG_REPEAT) {
		return CMD_RET_FAILURE;
	}

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	smem, 8, 1, do_smem,
	"TCC Secure Memory Access Command For only dev",
	" - smem r <address> <size>\n"
	" - smem w <address> <value>\n"
);
