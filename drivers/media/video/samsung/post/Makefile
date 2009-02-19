#################################################
# Makefile for Post Processor 
# 2007 (C) Samsung Electronics 
# Author : SungJun Bae <june.bae@samsung.com>
#################################################

obj-$(CONFIG_VIDEO_POST)	+= s3c_pp_common.o s3c_pp_6400.o

EXTRA_CFLAGS += -Idrivers/media/video

ifeq ($(CONFIG_VIDEO_POST_DEBUG),y)
EXTRA_CFLAGS += -DDEBUG
endif
