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

#endif