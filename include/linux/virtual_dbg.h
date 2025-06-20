/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2023 Jayden.K
 * This function is provided for debugging purposes.
 * It does not guarantee any bugs or errors in operation.
 */

#ifndef VIRTUAL_DBG_PRINT_H
#define VIRTUAL_DBG_PRINT_H

int virtual_dbg_write(const char* fmt, ...);
void virtual_dbg_flush(void);

void trace_set_name(char *new_trace_name);
void trace_reg_rw(char* dev_name, bool write, uint32_t con, uintptr_t reg, uint32_t byte, uint32_t val);
void trace_reg_name_print(void);
void trace_reg_flush(char* filter);
void trace_reg_flush_all(void);

#endif