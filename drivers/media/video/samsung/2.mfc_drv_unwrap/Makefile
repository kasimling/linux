
obj-$(CONFIG_VIDEO_MFC)	+= s3c-mfc.o Prism_S_V137.o BitProcBuf.o DataBuf.o FramBufMgr.o MFC_HW_Init.o MFC_Inst_Pool.o MFC_Instance.o MfcSfr.o MfcSetConfig.o 

EXTRA_CFLAGS += -DLINUX
EXTRA_CFLAGS += -DDIVX_ENABLE

ifeq ($(CONFIG_VIDEO_MFC_DEBUG),y)
EXTRA_CFLAGS += -DDEBUG
endif
