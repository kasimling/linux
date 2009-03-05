/* linux/driver/media/video/mfc/s3c_mfc_databuf.c
 *
 * C file for Samsung MFC (Multi Function Codec - FIMV) driver 
 *
 * PyoungJae Jung, Jiun Yu, Copyright (c) 2009 Samsung Electronics 
 * http://www.samsungsemi.com/ 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/io.h>
#include <linux/kernel.h>

#include "s3c_mfc_base.h"
#include "s3c_mfc_types.h"
#include "s3c_mfc_databuf.h"
#include "s3c_mfc_config.h"

static volatile unsigned char     *s3c_mfc_virt_data_buf = NULL;
static unsigned int                s3c_mfc_phys_data_buf = 0;


BOOL s3c_mfc_databuf_memmapping()
{
	BOOL	ret = FALSE;

	/* STREAM BUFFER, FRAME BUFFER  <-- virtual data buffer address mapping */
	s3c_mfc_virt_data_buf = (volatile unsigned char *)ioremap_nocache(S3C_MFC_BASEADDR_DATA_BUF, S3C_MFC_DATA_BUF_SIZE);
	if (s3c_mfc_virt_data_buf == NULL) {
		printk(KERN_ERR "\n%s: fail to mapping data buffer\n", __FUNCTION__);
		return ret;
	}

	printk(KERN_DEBUG "\n%s: virtual address of data buffer = 0x%x\n", __FUNCTION__, (unsigned int)s3c_mfc_virt_data_buf);

	/* Physical register address mapping */
	s3c_mfc_phys_data_buf = S3C_MFC_BASEADDR_DATA_BUF;

	ret = TRUE;

	return ret;
}

volatile unsigned char *s3c_mfc_get_databuf_virt_addr()
{
	volatile unsigned char	*data_buf;

	data_buf	= s3c_mfc_virt_data_buf;

	return data_buf;	
}

volatile unsigned char *s3c_mfc_get_yuv_buff_virt_addr()
{
	volatile unsigned char	*yuv_buff;

	yuv_buff	= s3c_mfc_virt_data_buf + S3C_MFC_STREAM_BUF_SIZE;

	return yuv_buff;	
}

unsigned int s3c_mfc_get_databuf_phys_addr()
{
	unsigned int	phys_data_buf;

	phys_data_buf	= s3c_mfc_phys_data_buf;

	return phys_data_buf;
}

unsigned int s3c_mfc_get_yuv_buff_phys_addr()
{
	unsigned int	phys_yuv_buff;

	phys_yuv_buff	= s3c_mfc_phys_data_buf + S3C_MFC_STREAM_BUF_SIZE;

	return phys_yuv_buff;
}
