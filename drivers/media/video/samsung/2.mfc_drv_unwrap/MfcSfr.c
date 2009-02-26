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
 * This source file is for setting the MFC's registers.
 *
 * @name MFC DRIVER MODULE Module (MfcSfr.c)
 * @author name(email address)
 * @date 03-28-07
 */

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <asm/io.h>

#include "MfcSfr.h"
//#include "MfcMemory.h"
//#include "LogMsg.h"
#include "MfcConfig.h"
#include "Prism_S.h"
//#include "MfcMutex.h"
#include "MfcIntrNotification.h"


static volatile S3C6400_MFC_SFR   *vir_pMFC_SFR		= NULL;
static volatile unsigned int      *vir_pSW_RESET	= NULL;

static unsigned int                phyMFC_SFR		= 0;
static unsigned int                phySW_RESET		= 0;

extern wait_queue_head_t	WaitQueue_MFC;
extern unsigned int		gIntrType;

int MFC_Sleep()
{
	// Wait until finish executing command.
	while( vir_pMFC_SFR->BUSY_FLAG != 0 )
		udelay(1);
	
	// Issue Sleep Command.
	vir_pMFC_SFR->BUSY_FLAG = 0x01;
	vir_pMFC_SFR->RUN_CMD = 0x0A;
	while( vir_pMFC_SFR->BUSY_FLAG != 0 )
		udelay(1);

	return 1;
}

int MFC_Wakeup()
{
	
	// Bit processor gets started.
	vir_pMFC_SFR->BUSY_FLAG = 0x01; 
	vir_pMFC_SFR->CODE_RUN = 0x01;
	while( vir_pMFC_SFR->BUSY_FLAG != 0 )
		udelay(1);
	
	// Bit processor wakes up.
	vir_pMFC_SFR->BUSY_FLAG = 0x01;
	vir_pMFC_SFR->RUN_CMD = 0x0B;
	while( vir_pMFC_SFR->BUSY_FLAG != 0 )
		udelay(1);

	return 1;
}


static char *GetCmdString(MFC_COMMAND mfc_cmd)
{
	switch ((int)mfc_cmd) {
	case SEQ_INIT:
		return "SEQ_INIT";

	case SEQ_END:
		return "SEQ_END";

	case PIC_RUN:
		return "PIC_RUN";

	case SET_FRAME_BUF:
		return "SET_FRAME_BUF";

	case ENC_HEADER:
		return "ENC_HEADER";

	case ENC_PARA_SET:
		return "ENC_PARA_SET";

	case DEC_PARA_SET:
		return "DEC_PARA_SET";

	case GET_FW_VER:
		return "GET_FW_VER";

	}

	return "UNDEF CMD";
}

static int WaitForReady(void)
{
	int   i;

#ifdef _WIN32_WCE
	for (i=0; i<15; i++) {
		if (vir_pMFC_SFR->BUSY_FLAG == 0) {
			return TRUE;
		}
		udelay(2);
	}
#else
	for (i=0; i<1000; i++) {
		if (vir_pMFC_SFR->BUSY_FLAG == 0) {
			return TRUE;
		}
		udelay(100);	// 1/1000 second
	}
#endif

	printk(KERN_DEBUG, "\n%s: timeout in waiting for the bitprocessor available\n", __FUNCTION__);


	return FALSE;
}


int GetFirmwareVersion(void)
{
	unsigned int prd_no, ver_no;

	WaitForReady();

	vir_pMFC_SFR->RUN_CMD     = GET_FW_VER;

	printk(KERN_DEBUG, "\n%s: GET_FW_VER command was issued\n", __FUNCTION__);

	WaitForReady();

	prd_no = vir_pMFC_SFR->param.dec_seq_init.RET_SEQ_SUCCESS >> 16;
	ver_no = (vir_pMFC_SFR->param.dec_seq_init.RET_SEQ_SUCCESS & 0x00FFFF);

	printk(KERN_DEBUG, "\n%s: GET_FW_VER => 0x%x, 0x%x\n", __FUNCTION__, prd_no, ver_no);
	printk(KERN_DEBUG, "\n%s: BUSY_FLAG => %d\n", __FUNCTION__, vir_pMFC_SFR->BUSY_FLAG);

	return vir_pMFC_SFR->param.dec_seq_init.RET_SEQ_SUCCESS;
}


BOOL MfcIssueCmd(int inst_no, MFC_CODECMODE codec_mode, MFC_COMMAND mfc_cmd)
{
	unsigned int intr_reason;

	vir_pMFC_SFR->RUN_INDEX     = inst_no;

	if (codec_mode == H263_DEC) {
		vir_pMFC_SFR->RUN_COD_STD	= MP4_DEC;
	} else if (codec_mode == H263_ENC) {
		vir_pMFC_SFR->RUN_COD_STD	= MP4_ENC;
	} else {
		vir_pMFC_SFR->RUN_COD_STD   = codec_mode;
	}
		
	switch (mfc_cmd) 
	{
	case PIC_RUN:
	case SEQ_INIT:
	case SEQ_END:
		vir_pMFC_SFR->RUN_CMD       = mfc_cmd;

		if(interruptible_sleep_on_timeout(&WaitQueue_MFC, 500) == 0)
		{
			MfcStreamEnd();
			return FALSE; 
		}
		
		intr_reason = gIntrType;
		
		if (intr_reason == MFC_INTR_REASON_INTRNOTI_TIMEOUT) {
			printk(KERN_ERR, "\n%s: command = %s, WaitInterruptNotification returns TIMEOUT\n", __FUNCTION__, GetCmdString(mfc_cmd));
			return FALSE;
		}
		if (intr_reason & MFC_INTR_REASON_BUFFER_EMPTY) {
			printk(KERN_ERR, "\n%s: command = %s, BUFFER EMPTY interrupt was raised\n", GetCmdString(mfc_cmd));
			return FALSE;
		}
		break;
		
	default:
		if (WaitForReady() == FALSE) {
			printk(KERN_ERR, "\n%s: command = %s, bitprocessor is busy before issuing the command\n", GetCmdString(mfc_cmd));
			return FALSE;
		}

		vir_pMFC_SFR->RUN_CMD       = mfc_cmd;
	
		WaitForReady();
			
	} 

	return TRUE;
}


BOOL MfcSfrMemMapping(void)
{
	BOOL	ret = FALSE;

	// virtual address mapping
	vir_pMFC_SFR = (volatile S3C6400_MFC_SFR *)ioremap_nocache(S3C6400_BASEADDR_MFC_SFR, S3C6400_MFC_SFR_SW_RESET_ADDR);
	if (vir_pMFC_SFR == NULL)
	{
		printk(KERN_ERR, "\n%s: fail to mapping mfc sfr\n", __FUNCTION__);
		return ret;
	}

	printk(KERN_DEBUG, "\n%s: virtual address of mfc sfr = 0x%x\n", __FUNCTION__, vir_pMFC_SFR);
	
	vir_pSW_RESET = (unsigned int *) ((int)vir_pMFC_SFR  +  S3C6400_MFC_SFR_SW_RESET_ADDR);

	// Physical address mapping
	phyMFC_SFR	= S3C6400_BASEADDR_MFC_SFR;
	phySW_RESET	= S3C6400_BASEADDR_MFC_SFR + S3C6400_MFC_SFR_SW_RESET_ADDR;


	ret = TRUE;

	return ret;
}

volatile S3C6400_MFC_SFR *GetMfcSfrVirAddr(void)
{
	volatile S3C6400_MFC_SFR	*mfc_sfr;

	mfc_sfr = vir_pMFC_SFR;

	return mfc_sfr;
}

void *MfcGetCmdParamRegion(void)
{
	return (void *) &(vir_pMFC_SFR->param);
}

// Perform the SW_RESET
void MfcReset(void)
{
	*vir_pSW_RESET = 0x00;
	*vir_pSW_RESET = 0x01;
	vir_pMFC_SFR->INT_ENABLE = MFC_INTR_ENABLE_RESET;	// Interrupt is enabled for PIC_RUN command and empty/full STRM_BUF status.
	vir_pMFC_SFR->INT_REASON = MFC_INTR_REASON_NULL;
	vir_pMFC_SFR->BITS_INT_CLEAR = 0x1;
}

// Clear the MFC Interrupt
// After catching the MFC Interrupt,
// it is required to call this functions for clearing the interrupt-related register.
void MfcClearIntr(void)
{
	vir_pMFC_SFR->BITS_INT_CLEAR = 0x1;
	vir_pMFC_SFR->INT_REASON     = MFC_INTR_REASON_NULL;
}

// Check INT_REASON register of MFC (the interrupt reason register)
unsigned int MfcIntrReason(void)
{
	return vir_pMFC_SFR->INT_REASON;
}

// Set the MFC's SFR of DEC_FUNC_CTRL to 1.
// It means that the data will not be added more to the STRM_BUF.
// It is required in RING_BUF mode (VC-1 DEC).
void MfcSetEos(int buffer_mode)
{
	if (buffer_mode == 1){
		vir_pMFC_SFR->DEC_FUNC_CTRL = 1<<1;
	} else{
		vir_pMFC_SFR->DEC_FUNC_CTRL = 1;	// 1: Whole stream is in buffer.
	}
}

void MfcStreamEnd()
{
	vir_pMFC_SFR->DEC_FUNC_CTRL = 0;
}

void MfcFirmwareIntoCodeDownReg(void)
{
	unsigned int  i;
	unsigned int  data;


	///////////////////////////////////////////////////////
	// Download the Boot code into MFC's internal memory //
	///////////////////////////////////////////////////////
	for (i=0; i<512; i++)
	{
		data = bit_code[i];

		vir_pMFC_SFR->CODE_DN_LOAD = ((i<<16) | data); // i: 13bit addr
	}

}

void MfcStartBitProcessor(void)
{
	vir_pMFC_SFR->CODE_RUN = 0x01;
}


void MfcStopBitProcessor(void)
{
	vir_pMFC_SFR->CODE_RUN = 0x00;
}

void MfcConfigSFR_BITPROC_BUF(void)
{
	// CODE BUFFER ADDRESS (BASE + 0x100)
	//   : Located from the Base address of the BIT PROCESSOR'S Firmware code segment
	vir_pMFC_SFR->CODE_BUF_ADDR = S3C6400_BASEADDR_MFC_BITPROC_BUF;


	// WORKING BUFFER ADDRESS (BASE + 0x104)
	//   : Located from the next to the BIT PROCESSOR'S Firmware code segment
	vir_pMFC_SFR->WORK_BUF_ADDR = vir_pMFC_SFR->CODE_BUF_ADDR + MFC_CODE_BUF_SIZE;


	// PARAMETER BUFFER ADDRESS (BASE + 0x108)
	//   : Located from the next to the WORKING BUFFER
	vir_pMFC_SFR->PARA_BUF_ADDR = vir_pMFC_SFR->WORK_BUF_ADDR + MFC_WORK_BUF_SIZE;
}

void MfcConfigSFR_CTRL_OPTS(void)
{
	unsigned int  uRegData;

	// BIT STREAM BUFFER CONTROL (BASE + 0x10C)
	uRegData = vir_pMFC_SFR->STRM_BUF_CTRL;
	vir_pMFC_SFR->STRM_BUF_CTRL = (uRegData & ~(0x03)) | BUF_STATUS_FULL_EMPTY_CHECK_BIT | STREAM_ENDIAN_LITTLE;


	// FRAME MEMORY CONTROL  (BASE + 0x110)
	vir_pMFC_SFR->FRME_BUF_CTRL = FRAME_MEM_ENDIAN_LITTLE;


	// DECODER FUNCTION CONTROL (BASE + 0x114)
	vir_pMFC_SFR->DEC_FUNC_CTRL = 0;	// 0: Whole stream is not in buffer.

	// WORK BUFFER CONTROL (BASE + 0x11C)
	vir_pMFC_SFR->WORK_BUF_CTRL = 0;	// 0: Work buffer control is disabled.
}

