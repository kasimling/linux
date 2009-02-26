/* linux/drivers/media/video/samsung/mfc/s3c_mfc.h
 *
 * Header file for Samsung Multi Format Codecs (MFC) driver
 *
 * Jiun Yu, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

typedef enum __MFC_CODEC_MODE
{
	MP4_DEC    = 0,
	MP4_ENC    = 1,
	AVC_DEC    = 2,
	AVC_ENC    = 3,
	VC1_DEC    = 4,
	H263_DEC   = 5,
	H263_ENC   = 6
} MFC_CODECMODE;

typedef enum __MFC_COMMAND
{
	SEQ_INIT         = 0x01,
	SEQ_END          = 0x02,
	PIC_RUN          = 0x03,
	SET_FRAME_BUF    = 0x04,
	ENC_HEADER       = 0x05,
	ENC_PARA_SET     = 0x06,
	DEC_PARA_SET     = 0x07,
	ENC_PARAM_CHANGE = 0x09,
	SLEEP            = 0x0A,
	WAKEUP           = 0x0B,
	GET_FW_VER       = 0x0F
} MFC_COMMAND;


typedef struct
{
	int retate;
	int deblockenable;
} MFC_DECODE_OPTIONS;

#define MFC_NAME		"s3c-mfc"

#define MFC_IOCTL_MAGIC	'M'

#define S3C_MFC_MPEG4_DEC_INIT			_IO(MFC_IOCTL_MAGIC, 0) // (0x00800001)
#define S3C_MFC_MPEG4_ENC_INIT			_IO(MFC_IOCTL_MAGIC, 1) // (0x00800002)
#define S3C_MFC_MPEG4_DEC_EXE			_IO(MFC_IOCTL_MAGIC, 2) // (0x00800003)
#define S3C_MFC_MPEG4_ENC_EXE			_IO(MFC_IOCTL_MAGIC, 3) // (0x00800004)

#define S3C_MFC_H264_DEC_INIT			_IO(MFC_IOCTL_MAGIC, 4) // (0x00800005)
#define S3C_MFC_H264_ENC_INIT			_IO(MFC_IOCTL_MAGIC, 5) // (0x00800006)
#define S3C_MFC_H264_DEC_EXE			_IO(MFC_IOCTL_MAGIC, 6) // (0x00800007)
#define S3C_MFC_H264_ENC_EXE			_IO(MFC_IOCTL_MAGIC, 7) // (0x00800008)

#define S3C_MFC_H263_DEC_INIT			_IO(MFC_IOCTL_MAGIC, 8) // (0x00800009)
#define S3C_MFC_H263_ENC_INIT			_IO(MFC_IOCTL_MAGIC, 9) // (0x0080000A)
#define S3C_MFC_H263_DEC_EXE			_IO(MFC_IOCTL_MAGIC, 10) // (0x0080000B)
#define S3C_MFC_H263_ENC_EXE			_IO(MFC_IOCTL_MAGIC, 11) // (0x0080000C)

#define S3C_MFC_VC1_DEC_INIT			_IO(MFC_IOCTL_MAGIC, 12) // (0x0080000D)
#define S3C_MFC_VC1_DEC_EXE			_IO(MFC_IOCTL_MAGIC, 13) // (0x0080000E)

#define S3C_MFC_GET_LINE_BUF_ADDR		_IO(MFC_IOCTL_MAGIC, 14) //  	(0x0080000F)
#define S3C_MFC_GET_RING_BUF_ADDR		_IO(MFC_IOCTL_MAGIC, 15) //  	(0x00800010)
#define S3C_MFC_GET_FRAM_BUF_ADDR		_IO(MFC_IOCTL_MAGIC, 16) // 	(0x00800011)
#define S3C_MFC_GET_POST_BUF_ADDR		_IO(MFC_IOCTL_MAGIC, 17) //  	(0x00800012)
#define S3C_MFC_GET_PHY_FRAM_BUF_ADDR		_IO(MFC_IOCTL_MAGIC, 18) //  	(0x00800013)
#define S3C_MFC_GET_CONFIG			_IO(MFC_IOCTL_MAGIC, 19) // (0x00800016)
#define S3C_MFC_GET_MPEG4_ASP_PARAM		_IO(MFC_IOCTL_MAGIC, 20) // 	(0x00800017)

#define S3C_MFC_SET_H263_MULTIPLE_SLICE		_IO(MFC_IOCTL_MAGIC, 21) // 		(0x00800014)
#define S3C_MFC_SET_CONFIG			_IO(MFC_IOCTL_MAGIC, 22) // (0x00800015)

#define S3C_MFC_SET_DISP_CONFIG			_IO(MFC_IOCTL_MAGIC, 23) // (0x00800111)
#define S3C_MFC_GET_FRAME_SIZE			_IO(MFC_IOCTL_MAGIC, 24) // (0x00800112)
#define S3C_MFC_SET_PP_DISP_SIZE		_IO(MFC_IOCTL_MAGIC, 25) // (0x00800113)
#define S3C_MFC_SET_DEC_INBUF_TYPE		_IO(MFC_IOCTL_MAGIC, 26) // (0x00800114)

#define IOCTL_VIRT_TO_PHYS			0x12345678

#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))
#define IOCTL_CACHE_FLUSH_B_FRAME		_IO(MFC_IOCTL_MAGIC, 27) // (0x00800115)
#define S3C_MFC_GET_PHY_B_FRAME_BUF_ADDR	_IO(MFC_IOCTL_MAGIC, 28) // (0x00800116)
#define S3C_MFC_GET_B_FRAME_BUF_ADDR		_IO(MFC_IOCTL_MAGIC, 29) // (0x00800117)
#endif

