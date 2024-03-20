# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) Telechips Inc.
#

ccflags-y += \
	 -I$(srctree)/$(src) \
	 -I$(srctree)/$(src)/include \

ccflags-$(CONFIG_TCC803X) += -I$(srctree)/$(src)/include/soc/tcc803x
ccflags-$(CONFIG_TCC805X) += -I$(srctree)/$(src)/include/soc/tcc805x

obj-$(CONFIG_TCC_APPB_VERIFICATION) += 	tav_vioc/tav_vioc.o \
					tav_vioc/tav_vioc_rdma.o \
					tav_vioc/tav_vioc_wdma.o \
					tav_vioc/tav_vioc_scaler.o \
					tav_vioc/tav_vioc_wmix.o \
					tav_vioc/tav_vioc_fifo.o \
					tav_vioc/tav_vioc_fdly.o

obj-$(CONFIG_TCC_APPB_VERIFICATION) += tav_db05.o
obj-$(CONFIG_TCC_APPB_VERIFICATION) += tav_db06.o
obj-$(CONFIG_TCC_APPB_VERIFICATION) += tav_db07.o
obj-$(CONFIG_TCC_APPB_VERIFICATION) += tav_db08.o
obj-$(CONFIG_TCC_APPB_VERIFICATION) += tav_db12.o
obj-$(CONFIG_TCC_APPB_VERIFICATION) += tav_db15.o
obj-$(CONFIG_TCC_APPB_VERIFICATION) += tav_irq.o
