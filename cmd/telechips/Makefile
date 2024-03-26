# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 Telechips Inc.

obj-$(CONFIG_CMD_BLKREAD) += blkread.o
obj-$(CONFIG_CMD_TCC_OTP) += otp.o
obj-$(CONFIG_CMD_TCC_OTP3) += otp3.o
obj-$(CONFIG_CMD_BOOT_SUBCORE) += boot_subcore.o
obj-$(CONFIG_CMD_CLK_CHANGER) += tcc_clk_changer.o
obj-$(CONFIG_CMD_TIMER_TEST) += timer_test.o
obj-$(CONFIG_CMD_TCC_OPTEE) += optee.o
obj-$(CONFIG_CMD_TCC_SECURE_BOOT) += secureboot.o
obj-$(CONFIG_CMD_TCC_TZC) += tzc.o
obj-$(CONFIG_CMD_TCC_SMEM) += smem.o
obj-$(CONFIG_CMD_TCC_PWM) += pwm.o
obj-$(CONFIG_CMD_TCC_MMC_TUNING) += mmc_tuning.o
obj-$(CONFIG_CMD_TCC_PCIE_SIGTEST) += sigtest.o
obj-$(CONFIG_CMD_TCC_CAN_TEST) += can/ can.o
obj-$(CONFIG_CMD_TEST_GPIO) += test_gpio.o

ccflags-$(CONFIG_TELECHIPS_DPV14_DM) += \
	 -I$(srctree)/$(src)/../../drivers/displayport/include
obj-$(CONFIG_TELECHIPS_DPV14_DM) += dpv14.o

obj-$(CONFIG_TELECHIPS_FB_DM) += fb_dm.o
