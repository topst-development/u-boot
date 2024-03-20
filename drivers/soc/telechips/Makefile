# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) Telechips Inc.
#

# Makefile for the U-boot SOC specific device drivers.

obj-$(CONFIG_ARCH_TELECHIPS)		+= otp/
obj-$(CONFIG_ARCH_TELECHIPS)		+= auth/

ifndef CONFIG_TCC803X
obj-$(CONFIG_ARCH_TELECHIPS)		+= hsm_mbox/
endif
