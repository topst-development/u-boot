// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2023 Jayden.K
 * This function is provided for debugging purposes.
 * It does not guarantee any bugs or errors in operation.
 */
#include <linux/compat.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <vsprintf.h>

struct virtual_dbg_node {
	struct list_head dbg_list;
	char *dbg_log;
	uint32_t dbg_len;
};

static LIST_HEAD(virtual_dbg_list);


/* MAX 4096 */
int virtual_dbg_write(const char* fmt, ...)
{
        struct virtual_dbg_node *dbg_node;
	char *print_buffer;
        va_list args;
	int dbg_len;
	int ret = 0;

	dbg_node = malloc(sizeof(struct virtual_dbg_node));

	if (dbg_node == NULL) {
		ret = -ENOMEM;
	} else {
		dbg_node->dbg_log = NULL;

		print_buffer = malloc(4096);
	}
	if (print_buffer != NULL) {
		va_start(args, fmt);
		dbg_len = vsnprintf(print_buffer, 4096, fmt, args);
		va_end(args);

		if (dbg_len < 0) {
			(void)pr_err("%s io error\n", __func__);
			ret = -EIO;
		} else {
			if (dbg_len > 4095) {
				/* memdup return the number of characters printed (excluding the null byte used to end output to strings). */
				(void)pr_err("%s out of range %d/4096\n", __func__, dbg_len);
				ret = -EIO;
			} else {
				dbg_node->dbg_log = kmemdup(print_buffer, dbg_len +1, GFP_KERNEL);
			}
		}
		free(print_buffer);
	}
	if (dbg_node != NULL) {
		if (dbg_node->dbg_log == NULL) {
			ret = -EIO;
		} else {
			list_add_tail(&dbg_node->dbg_list, &virtual_dbg_list);
		}
	}
	return ret;
}

void virtual_dbg_flush(void)
{
	struct virtual_dbg_node *dbg_node, *dbg_cmp;

	list_for_each_entry_safe(dbg_node, dbg_cmp, &virtual_dbg_list, dbg_list) {
		if (dbg_node != NULL) {
			if (dbg_node->dbg_log != NULL) {
				(void)pr_force("%s\n", dbg_node->dbg_log);
				kfree(dbg_node->dbg_log);
				dbg_node->dbg_log = NULL;

				list_del(&dbg_node->dbg_list);
			}
			free(dbg_node);
			dbg_node = NULL;
		}
	}

	if (!list_empty(&virtual_dbg_list)) {
		pr_err("%s list is not empty\n", __func__);
	}
}
