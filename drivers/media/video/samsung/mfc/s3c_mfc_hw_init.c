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
 * @name MFC DRIVER MODULE Module (s3c_mfc_hw_init.c)
 * @author name(email address)
 * @date 03-28-07
 */
#include <linux/kernel.h>

#include "s3c_mfc_base.h"
#include "s3c_mfc_sfr.h"
#include "s3c_mfc_bitproc_buf.h"
#include "s3c_mfc_databuf.h"
#include "s3c_mfc_types.h"
#include "s3c_mfc_config.h"
#include "s3c_mfc_yuv_buf_manager.h"

BOOL s3c_mfc_memory_setup(void)
{
	BOOL ret_bit, ret_dat;
	unsigned char *pDataBuf;

	/* 
	 * MFC SFR(Special Function Registers), Bitprocessor buffer, Data buffer의 
	 * physical address 를 virtual address로 mapping 한다 
	 */

	ret_bit = s3c_mfc_bitproc_buff_mem_mapping();
	if (ret_bit == FALSE) {
		printk(KERN_ERR "\n%s: fail to mapping bitprocessor buffer memory\n", __FUNCTION__);
		return FALSE;
	}

	ret_dat	= s3c_mfc_databuf_memmapping();
	if (ret_dat == FALSE) {
		printk(KERN_ERR "\n%s: fail to mapping data buffer memory \n", __FUNCTION__);
		return FALSE;
	}

	/* FramBufMgr Module Initialization */
	pDataBuf = (unsigned char *)s3c_mfc_get_databuf_virt_addr();
	s3c_mfc_yuv_buf_mgr_init(pDataBuf + S3C_MFC_STREAM_BUF_SIZE, S3C_MFC_YUV_BUF_SIZE);


	return TRUE;
}


BOOL s3c_mfc_hw_init(void)
{
	/* 
	 * 1. Reset the MFC IP
	 */
	s3c_mfc_reset();

	/*
	 * 2. Download Firmware code into MFC
	 */
	s3c_mfc_firmware_into_codebuff();
	s3c_mfc_firmware_into_code_down_reg();
	printk(KERN_DEBUG "\n%s: downloading firmware into bitprocessor\n", __FUNCTION__);

	/* 
	 * 3. Start Bit Processor
	 */
	s3c_mfc_start_bit_processor();

	/* 
	 * 4. Set the Base Address Registers for the following 3 buffers
	 * (CODE_BUF, WORKING_BUF, PARAMETER_BUF)
	 */
	s3c_mfc_config_sfr_bitproc_buffer();

	/* 
	 * 5. Set the Control Registers
	 * 	- STRM_BUF_CTRL
	 * 	- FRME_BUF_CTRL
	 * 	- DEC_FUNC_CTRL
	 * 	- WORK_BUF_CTRL
	 */
	s3c_mfc_config_sfr_ctrl_opts();

	s3c_mfc_get_firmware_version();

	return TRUE;
}

