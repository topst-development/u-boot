# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) Telechips Inc.

# common header
ccflags-y += \
	-I$(srctree)/$(src) \
	-I$(srctree)/include \
	-I$(srctree)/arch/arm/include \

ccflags-$(CONFIG_SPLASH_LOGO_SUPPORT) += \
	-I$(srctree)/$(src)/splash

ccflags-$(CONFIG_TELECHIPS_HDMI_V2_0) += \
	-I$(srctree)/drivers/hdmi/hdmi_v2_0/ \
	-I$(srctree)/drivers/hdmi/hdmi_v2_0/hdmi_api_lib/include

ccflags-$(CONFIG_DISPLAY_PORT_V14) += \
	-I$(srctree)/$(src)/../../displayport/include


obj-$(CONFIG_TELECHIPS_FB_DM) += fb_dm.o fb_dm_lcd_interface.o
obj-$(CONFIG_TELECHIPS_LVDS_DM) += lvds_dm.o
ifneq ($(filter y,$(CONFIG_TCC803X) $(CONFIG_TCC805X)),)
obj-$(CONFIG_TELECHIPS_LVDS_DM) += lvds_ctrl.o
endif

ifneq ($(filter y,$(CONFIG_TELECHIPS_DSI_DM)),)
ccflags-$(CONFIG_TCC750X) += \
  -I$(srctree)/drivers/mipi_dsi/telechips/tcc750x
ccflags-$(CONFIG_TCC807X) += \
  -I$(srctree)/drivers/mipi_dsi/telechips/tcc807x

obj-y += dsi_dm.o dsi_ctrl.o
endif

obj-$(CONFIG_TELECHIPS_HDMI_DM) += hdmi_dm.o hdmiv20_ctrl.o hdmi_v2_0.o
obj-$(CONFIG_TELECHIPS_DPV14_DM) += dpv14_dm.o dpv14_ctrl.o \
			      	    dpv14_max968xx.o
obj-$(CONFIG_FB_SUPPORT_VIOC_DUMP) += vioc_dump.o

# Splah Image
obj-y += splash/

