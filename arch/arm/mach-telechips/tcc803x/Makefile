# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) Telechips Inc.

obj-y += reboot.o
obj-y += tcc_secureboot.o

obj-$(CONFIG_OF_SYSTEM_SETUP) += fdt.o
obj-$(CONFIG_BOOT_SUBCORE_BY_MAINCORE) += ca7s.o
obj-$(CONFIG_BOOT_CM4_BY_MAINCORE) += cm4.o

#Telechips Framebuffer
obj-y	+= vioc/vioc_config.o vioc/vioc_ddicfg.o \
			vioc/vioc_disp.o vioc/vioc_fifo.o \
			vioc/vioc_lut.o vioc/vioc_outcfg.o \
			vioc/vioc_pxdemux.o vioc/vioc_rdma.o \
			vioc/vioc_scaler.o vioc/vioc_vin.o  \
			vioc/vioc_viqe.o vioc/vioc_wmix.o \
			vioc/vioc_wdma.o \
			vioc/vioc_intr.o \
			vioc/vioc_fdly.o \
			vioc/vioc_timer.o \
			vioc/vioc_fdly.o \
			vioc/vioc_dump.o
#LVDS
obj-$(CONFIG_TCC_FB)	+= vioc/lvds_phy.o vioc/lvds_wrap.o
