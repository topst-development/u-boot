#
# Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ALL-y += tcmkimage
CLEAN_FILES += tcmktool

SOC_NAME := $(subst tcc,,$(CONFIG_SYS_SOC))

ifeq ($(CONFIG_TCC805X), y)
ifeq ($(CONFIG_TCC_MAINCORE), y)
IMAGE_NAME := A72-BL3
ROM_NAME := ca72_bl3.rom
else
IMAGE_NAME := A53-BL3
ROM_NAME := ca53_bl3.rom
endif
endif

ifeq ($(CONFIG_TCC803X), y)
ifeq ($(CONFIG_TCC_MAINCORE), y)
IMAGE_NAME := A53-BL3
ROM_NAME = u-boot.rom
else
endif
endif

IMAGE_VERSION := 0.0.0
TARGET_ADDRESS := $(CONFIG_SYS_TEXT_BASE)

ifeq ($(CONFIG_ARM64), y)
EXEC_STATE := 0x64
else
EXEC_STATE := 0x32
endif

# tcmktool build
ifeq (${KBUILD_SRC},)
	tcmktool := ./tcmktool
else
	tcmktool := ${KBUILD_SRC}/tcmktool
endif

$(tcmktool): tools/tcmkimage/tcmktool.c
	@gcc -DSOC_NAME=\"$(SOC_NAME)\" -DUSE_HOSTCC -idirafterinclude -idirafter../include -I./tools/tcmkimage $^ -o $@
	@echo "Built $@ successfully"

tcmkimage: u-boot.bin $(tcmktool)
	$(tcmktool) $(UBOOT_BIN) $(ROM_NAME) $(IMAGE_NAME) $(IMAGE_VERSION) \
		$(TARGET_ADDRESS) $(EXEC_STATE) >> /dev/null
	@echo "Built $@ successfully"

