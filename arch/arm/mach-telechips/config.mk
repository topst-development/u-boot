# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 Telechips Inc.

KBUILD_CFLAGS += -Werror

include $(srctree)/arch/arm/mach-telechips/$(SOC)/config.mk
include $(srctree)/tools/tcmkimage/Makefile
