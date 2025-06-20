# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 Telechips Inc.

ifeq ($(CONFIG_USE_MAINCORE), y)
BODY_OFFSET	:= 4096
SOC_NAME	:= 807x
ROM_NAME	:= ap0_bl3.rom
IMAGE_NAME	:= AP0-BL3
else
BODY_OFFSET	:= 4096
SOC_NAME	:= 807x
ROM_NAME	:= ap1_bl3.rom
IMAGE_NAME	:= AP1-BL3
endif
