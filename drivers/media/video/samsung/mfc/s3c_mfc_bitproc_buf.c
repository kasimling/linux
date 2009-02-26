/*
 * Project Name MFC DRIVER 
 * Copyright  2007 Samsung Electronics Co, Ltd. All Rights Reserved. 
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics  ("Confidential Information").   
 * you shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics 
 *
 * This source file is for controlling Bitprocessor's buffer.
 *
 * @name MFC DRIVER MODULE Module (BitProcBuf.c)
 * @author name(email address)
 * @date 03-28-07
 */
#include <asm/io.h>
#include <linux/kernel.h>
#include <plat/reserved_mem.h>

#include "s3c_mfc_base.h"
#include "s3c_mfc_bitproc_buf.h"
#include "s3c_mfc_config.h"
#include "prism_s.h"

static volatile unsigned char     *s3c_mfc_virt_bitproc_buff  = NULL;
static unsigned int                s3c_mfc_phys_bitproc_buff  = 0;


BOOL s3c_mfc_bitproc_buff_mem_mapping()
{
	BOOL	ret = FALSE;

	/* FIRWARE/WORKING/PARAMETER BUFFER  <-- virtual bitprocessor buffer address mapping */
	s3c_mfc_virt_bitproc_buff = (volatile unsigned char *)ioremap_nocache(S3C_MFC_BASEADDR_BITPROC_BUF, S3C_MFC_BITPROC_BUF_SIZE);
	if (s3c_mfc_virt_bitproc_buff == NULL) {
		printk(KERN_ERR "\n%s: fail to mapping bitprocessor buffer\n", __FUNCTION__);
		return ret;
	}

	/* Physical register address mapping */
	s3c_mfc_phys_bitproc_buff = S3C_MFC_BASEADDR_BITPROC_BUF;

	ret = TRUE;

	return ret;
}

volatile unsigned char *s3c_mfc_get_bitproc_buff_virt_addr()
{
	volatile unsigned char	*pBitProcBuf;

	pBitProcBuf = s3c_mfc_virt_bitproc_buff;

	return pBitProcBuf;
}

unsigned char *s3c_mfc_get_param_buff_virt_addr()
{
	unsigned char	*pParamBuf;

	pParamBuf = (unsigned char *)(s3c_mfc_virt_bitproc_buff + S3C_MFC_CODE_BUF_SIZE + S3C_MFC_WORK_BUF_SIZE);

	return pParamBuf;
}

void s3c_mfc_firmware_into_codebuff()
{
	unsigned int  i, j;
	unsigned int  data;

	unsigned int *uAddrFirmwareCode;

	uAddrFirmwareCode = (unsigned int *)s3c_mfc_virt_bitproc_buff;

	/* 
	 * Putting the Boot & Firmware code into SDRAM 
	 * Boot code(1KB) + Codec Firmware (79KB)
	 *
	 */
	for (i = j = 0 ; i < sizeof(s3c_mfc_bit_code) / sizeof(s3c_mfc_bit_code[0]); i += 2, j++) {
		data = (s3c_mfc_bit_code[i] << 16) | s3c_mfc_bit_code[i + 1];

		*(uAddrFirmwareCode + j) = data;
	}
}
