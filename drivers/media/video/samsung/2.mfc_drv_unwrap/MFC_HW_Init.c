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
 * This source file is for initializing the MFC's H/W setting.
 *
 * @name MFC DRIVER MODULE Module (MFC_HW_Init.c)
 * @author name(email address)
 * @date 03-28-07
 */
#include <linux/kernel.h>

#include "Mfc.h"
#include "MfcSfr.h"
#include "BitProcBuf.h"
#include "DataBuf.h"
#include "MfcTypes.h"
//#include "LogMsg.h"
#include "MfcConfig.h"
#include "FramBufMgr.h"


BOOL MFC_MemorySetup(void)
{
	//MfcMemMapping();
	BOOL ret_sfr, ret_bit, ret_dat;
	unsigned char *pDataBuf;


	// MFC SFR(Special Function Registers), Bitprocessor buffer, Data buffer의 
	// physical address 를 virtual address로 mapping 한다 
	ret_sfr	= MfcSfrMemMapping();
	if (ret_sfr == FALSE) {
		printk(KERN_ERR, "\n%s: fail to mapping mfc sfr memory\n", __FUNCTION__);
		return FALSE;
	}

	ret_bit = MfcBitProcBufMemMapping();
	if (ret_bit == FALSE) {
		printk(KERN_ERR, "\n%s: fail to mapping bitprocessor buffer memory\n", __FUNCTION__);
		return FALSE;
	}

	ret_dat	= MfcDataBufMemMapping();
	if (ret_dat == FALSE) {
		printk(KERN_ERR, "\n%s: fail to mapping data buffer memory \n", __FUNCTION__);
		return FALSE;
	}


	// FramBufMgr Module Initialization
	pDataBuf = (unsigned char *)GetDataBufVirAddr();
	FramBufMgrInit(pDataBuf + MFC_STRM_BUF_SIZE, MFC_FRAM_BUF_SIZE);


	return TRUE;
}


BOOL MFC_HW_Init(void)
{
	/////////////////////////
	//                     //
	// 1. Reset the MFC IP //
	//                     //
	/////////////////////////
	MfcReset();


	////////////////////////////////////////
	//                                    //
	// 2. Download Firmware code into MFC //
	//                                    //
	////////////////////////////////////////
	MfcFirmwareIntoCodeBuf();
	MfcFirmwareIntoCodeDownReg();
	printk(KERN_DEBUG, "\n%s: downloading firmware into bitprocessor\n", __FUNCTION__);

	////////////////////////////
	//                        //
	// 3. Start Bit Processor //
	//                        //
	////////////////////////////
	MfcStartBitProcessor();


	////////////////////////////////////////////////////////////////////
	//                                                                //
	// 4. Set the Base Address Registers for the following 3 buffers  //
	//        (CODE_BUF, WORKING_BUF, PARAMETER_BUF)                  //
	//                                                                //
	////////////////////////////////////////////////////////////////////
	MfcConfigSFR_BITPROC_BUF();


	//////////////////////////////////
	//                              //
	// 5. Set the Control Registers //
	//       - STRM_BUF_CTRL        //
	//       - FRME_BUF_CTRL        //
	//       - DEC_FUNC_CTRL        //
	//       - WORK_BUF_CTRL        //
	//                              //
	//////////////////////////////////
	MfcConfigSFR_CTRL_OPTS();



	GetFirmwareVersion();


	return TRUE;
}

