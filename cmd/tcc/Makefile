# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) Telechips Inc.

obj-$(CONFIG_CMD_TCC_AVM) += avm.o
obj-$(CONFIG_CMD_TCC_BLKREAD) += blkread.o
obj-$(CONFIG_CMD_TCC_ECID) += ecid.o
obj-$(CONFIG_CMD_TCC_THERMAL) += thermal.o
obj-$(CONFIG_CMD_TCC_MEM) += mem.o
obj-$(CONFIG_CMD_TCC_SYNC) += sync.o
obj-$(CONFIG_CMD_TCC_TIMER_TEST) += timer_test.o
obj-$(CONFIG_CMD_TCC_WDT) += wdt.o
obj-$(CONFIG_CMD_TEST_GPIO) += test_gpio.o
obj-$(CONFIG_CMD_TCC_OTP) += otp.o
obj-$(CONFIG_CMD_TCC_PWM) += pwm.o
obj-$(CONFIG_CMD_TCC_PROC_RST) += proc_reset.o
obj-$(CONFIG_CMD_TCC_CLK_SHOW) += clk.o
obj-$(CONFIG_CMD_TCC_CLK_CHANGER) += tcc_clk_changer.o

ccflags-$(CONFIG_TCC_APPB_VERIFICATION) += \
	-I$(srctree)/$(src)/../../drivers/tav
obj-$(CONFIG_TCC_APPB_VERIFICATION) += tav.o

ccflags-$(CONFIG_DISPLAY_PORT_V14) += \
	 -I$(srctree)/$(src)/../../drivers/displayport/dp_v1_4/include
ccflags-$(CONFIG_TCC_LCD) += \
	 -I$(srctree)/$(src)/../../drivers/video/tcc
obj-$(CONFIG_DISPLAY_PORT_V14) += dpv14.o

obj-$(CONFIG_CMD_TCC_OPTEE) += optee.o
obj-$(CONFIG_CMD_TCC_MMC_TUNING) += mmc_tuning.o

# For development
obj-y += secureboot.o

obj-$(CONFIG_EARLYCAMERA_SOLUTION) += earlycamera.o

obj-$(CONFIG_CMD_DWC_HOSTLIB) += dwc_hl.o

ifdef CONFIG_TCC_HDMI_V2_0
ccflags-y += \
	-I$(srctree)/drivers/video/tcc \
	-I$(srctree)/drivers/hdmi/hdmi_v2_0 \
	-I$(srctree)/drivers/hdmi/hdmi_v2_0/hdmi_api_lib/include
obj-y += hdmi.o
endif
