# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 Telechips Inc.

# Define below in arch/arm/mach-telechips/$(SOC)/config.mk
# - BODY_OFFSET
# - SOC_NAME
# - ROM_NAME
# - IMAGE_NAME

INPUTS-y	+= tcmkimage
CLEAN_FILES	+= tcmktool

BUILD_VERSION	:= ""
BUILD_COMMIT	:= ""
TARGET_ADDRESS	:= $(CONFIG_SYS_TEXT_BASE)

ifeq ($(CONFIG_ARM64), y)
EXEC_STATE	:= 0x64
else
EXEC_STATE	:= 0x32
endif

ifeq ($(KBUILD_OUTPUT),)
OUTPUT_DIR	:= .
else
OUTPUT_DIR	:= $(KBUILD_OUTPUT)
endif

# tcmktool build
tcmktool := $(OUTPUT_DIR)/tcmktool
$(tcmktool): tools/tcmkimage/tcmktool.c
	$(Q)$(HOSTCC) -DBODY_OFFSET=$(BODY_OFFSET) -DSOC_NAME=\"$(SOC_NAME)\" \
		      -DUSE_HOSTCC -idirafter $(srctree)/include $^ -o $@
	@echo "  Built $@ successfully"

# tcmktool command
define MAKE_ROM
	$(Q)$(tcmktool) $(1) $(2) $(3) \
			$(BUILD_VERSION) $(BUILD_COMMIT) $(4) $(EXEC_STATE) \
			>> /dev/null
	@echo "  Built $(2) successfully"
endef

# tcmktool command
tcmkimage: u-boot.bin $(tcmktool)
	$(call MAKE_ROM, $(UBOOT_BIN), \
			 $(OUTPUT_DIR)/$(ROM_NAME), \
			 $(IMAGE_NAME), \
			 $(TARGET_ADDRESS))
