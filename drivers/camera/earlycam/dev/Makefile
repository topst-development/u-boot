# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) Telechips Inc.

ccflags-y +=					\
	-I$(srctree)/$(src)/../include		\
	-I$(srctree)/$(src)/../include/dev

obj-y									+= videosource_if.o

obj-$(CONFIG_EARLYCAMERA_SOLUTION_VIDEODECODER_ANALOGDEVICES_ADV7182)	+= adv7182.o
obj-$(CONFIG_EARLYCAMERA_SOLUTION_VIDEODECODER_INTERSIL_ISL79988)	+= isl79988.o
obj-$(CONFIG_EARLYCAMERA_SOLUTION_DESERIALIZER_MAXIM_MAX9276)		+= max9276.o
