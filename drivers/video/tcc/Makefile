# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) Telechips Inc.

ccflags-y += \
	-I$(srctree)/$(src)
ccflags-$(CONFIG_TCC_HDMI_V2_0) += \
	-I$(srctree)/drivers/hdmi/hdmi_v2_0/ \
	-I$(srctree)/drivers/hdmi/hdmi_v2_0/hdmi_api_lib/include
ccflags-y += \
	-I$(srctree)/$(src)/../../displayport/dp_v1_4/include

obj-$(CONFIG_TCC_LCD) += tcc_fb.o tcc_lcd_interface.o
obj-$(CONFIG_DISPLAY_PORT_V14) += lcd_DPV14.o
obj-$(CONFIG_TCC_LVDS_CTRL) += tcc_lvds_ctrl.o
obj-$(CONFIG_LCD_FLD0800) += lcd_FLD0800.o
obj-$(CONFIG_LCD_SLVDS_SAMPLE) += lcd_SLVDS_SAMPLE.o
obj-$(CONFIG_LCD_SLVDS_EXT) += lcd_SLVDS_EXT.o
obj-$(CONFIG_LCD_TM123XDHP90) += lcd_TM123XDHP90.o
obj-$(CONFIG_LCD_HDMIV20) += lcd_HDMIV20.o
obj-$(CONFIG_TCC_HDMI_V2_0) += tcc_hdmi_v2_0.o