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

#include "Mfc.h"
//#include "MfcMemory.h"
//#include "LogMsg.h"
#include "BitProcBuf.h"
#include "MfcConfig.h"
#include "Prism_S.h"



static volatile unsigned char     *vir_pBITPROC_BUF   = NULL;
static unsigned int                phyBITPROC_BUF  = 0;

extern dma_addr_t	s3c_mfc_phys_data_buffer;
extern unsigned char	*s3c_mfc_virt_data_buffer;

BOOL MfcBitProcBufMemMapping()
{
	BOOL	ret = FALSE;


	/*
	// FIRWARE/WORKING/PARAMETER BUFFER  <-- virtual bitprocessor buffer address mapping
	vir_pBITPROC_BUF = (volatile unsigned char *)ioremap_nocache(S3C6400_BASEADDR_MFC_BITPROC_BUF, MFC_BITPROC_BUF_SIZE);
	if (vir_pBITPROC_BUF == NULL)
	{
		printk(KERN_ERR "\n%s: fail to mapping bitprocessor buffer\n", __FUNCTION__);
		return ret;
	}

	// Physical register address mapping
	phyBITPROC_BUF	= S3C6400_BASEADDR_MFC_BITPROC_BUF;
	*/
	vir_pBITPROC_BUF = s3c_mfc_virt_data_buffer;
	phyBITPROC_BUF = s3c_mfc_phys_data_buffer;
	
	ret = TRUE;


	return ret;
}

volatile unsigned char *GetBitProcBufVirAddr()
{
	volatile unsigned char	*pBitProcBuf;

	pBitProcBuf	= vir_pBITPROC_BUF;

	return pBitProcBuf;
}

unsigned char *GetParamBufVirAddr()
{
	unsigned char	*pParamBuf;

	pParamBuf = (unsigned char *)(vir_pBITPROC_BUF + MFC_CODE_BUF_SIZE + MFC_WORK_BUF_SIZE);

	return pParamBuf;
}

void MfcFirmwareIntoCodeBuf()
{
	unsigned int  i, j;
	unsigned int  data;

	unsigned int *uAddrFirmwareCode;

	uAddrFirmwareCode = (unsigned int *)vir_pBITPROC_BUF;

	/////////////////////////////////////////////////
	// Putting the Boot & Firmware code into SDRAM //
	/////////////////////////////////////////////////
	// Boot code(1KB) + Codec Firmware (79KB)
	for (i=j=0 ; i<sizeof(bit_code)/sizeof(bit_code[0]); i+=2, j++)
	{
		data = (bit_code[i] << 16) | bit_code[i+1];
	
		*(uAddrFirmwareCode + j) = data;
	}
}
