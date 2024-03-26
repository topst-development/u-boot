# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 Telechips Inc.

ifeq ($(CONFIG_USE_MAINCORE), y)
BODY_OFFSET	:= 4096
SOC_NAME	:= 805x
ROM_NAME	:= ca72_bl3.rom
IMAGE_NAME	:= A72-BL3
else
BODY_OFFSET	:= 4096
SOC_NAME	:= 805x
ROM_NAME	:= ca53_bl3.rom
IMAGE_NAME	:= A53-BL3
endif
