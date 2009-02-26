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
 * This source file is for controlling the Data buffer.
 *
 * @name MFC DRIVER MODULE Module (DataBuf.c)
 * @author name(email address)
 * @date 03-28-07
 */
#include <asm/io.h>
#include <linux/kernel.h>
#include <plat/reserved_mem.h>

#include "Mfc.h"
#include "MfcTypes.h"
//#include "MfcMemory.h"
//#include "LogMsg.h"
#include "DataBuf.h"
#include "MfcConfig.h"

static volatile unsigned char     *vir_pDATA_BUF      = NULL;

static unsigned int                phyDATA_BUF     = 0;


BOOL MfcDataBufMemMapping()
{
	BOOL	ret = FALSE;

	// STREAM BUFFER, FRAME BUFFER  <-- virtual data buffer address mapping
	vir_pDATA_BUF = (volatile unsigned char *)ioremap_nocache(S3C6400_BASEADDR_MFC_DATA_BUF, MFC_DATA_BUF_SIZE);
	if (vir_pDATA_BUF == NULL) 
	{
		printk(KERN_ERR "\n%s: fail to mapping data buffer\n", __FUNCTION__);
		return ret;
	}
	
	printk(KERN_DEBUG "\n%s: virtual address of data buffer = 0x%x\n", __FUNCTION__, (unsigned int)vir_pDATA_BUF);

	// Physical register address mapping
	phyDATA_BUF	= S3C6400_BASEADDR_MFC_DATA_BUF;


	ret = TRUE;

	return ret;
}

volatile unsigned char *GetDataBufVirAddr()
{
	volatile unsigned char	*pDataBuf;

	pDataBuf	= vir_pDATA_BUF;

	return pDataBuf;	
}

volatile unsigned char *GetFramBufVirAddr()
{
	volatile unsigned char	*pFramBuf;

	pFramBuf	= vir_pDATA_BUF + MFC_STRM_BUF_SIZE;

	return pFramBuf;	
}

unsigned int GetDataBufPhyAddr()
{
	unsigned int	phyDataBuf;

	phyDataBuf	= phyDATA_BUF;

	return phyDataBuf;
}

unsigned int GetFramBufPhyAddr()
{
	unsigned int	phyFramBuf;

	phyFramBuf	= phyDATA_BUF + MFC_STRM_BUF_SIZE;

	return phyFramBuf;
}
