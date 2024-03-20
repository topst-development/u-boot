// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <errno.h>
#include <test/ut.h>
#include <test/test.h>

DECLARE_GLOBAL_DATA_PTR;

#define TEST_NAME_PREFIX	"driver_test_"

struct unit_test_state global_driver_test_state;

int do_driver_test(const char *test_name)
{
	struct unit_test *tests = ll_entry_start(struct unit_test, driver_test);
	const int n_ents = ll_entry_count(struct unit_test, driver_test);
	struct unit_test_state *uts = &global_driver_test_state;
	struct unit_test *test;
	int run_count;

	if (!test_name)
		printf("Running %d driver tests\n", n_ents);

	run_count = 0;
	for (test = tests; test < tests + n_ents; test++) {
		const char *name = test->name;

		if (!strncmp(name, TEST_NAME_PREFIX,
				sizeof(TEST_NAME_PREFIX) - 1))
			name += sizeof(TEST_NAME_PREFIX) - 1;

		if (test_name && strcmp(test_name, name))
			continue;

		printf("Test: %s\n", name);
		run_count++;

		test->func(uts);
	}

	if (test_name && !run_count)
		printf("Test '%s' not found\n", test_name);
	else
		printf("Failures: %d\n", uts->fail_count);

#ifdef CONFIG_SPL_BUILD
	while (1) ;
#endif

	return 0;
}

int do_ut_driver(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *test_name = NULL;

	if (argc > 1)
		test_name = argv[1];

	return do_driver_test(test_name);
}
