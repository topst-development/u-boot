# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) Telechips Inc.

ccflags-y += -I$(srctree)/include \
	     -I$(srctree)/arch/$(ARCH)/dts/include

obj-y	+= \
	vioc_config.o \
	vioc_ddicfg.o \
	vioc_disp.o \
	vioc_fifo.o \
	vioc_lut.o \
	vioc_outcfg.o \
	vioc_pxdemux.o \
	vioc_rdma.o \
	vioc_scaler.o \
	vioc_vin.o  \
	vioc_viqe.o \
	vioc_wmix.o \
	vioc_wdma.o \
	vioc_intr.o \
	vioc_fdly.o \
	vioc_timer.o \
	vioc_fdly.o \
	vioc_dump.o
#LVDS
obj-$(CONFIG_TELECHIPS_LVDS_DM)	+= lvds_phy.o lvds_wrap.o
