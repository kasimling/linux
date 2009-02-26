#include <linux/version.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/signal.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/page.h>
//#include <asm/arch/irqs.h>
//#include <asm/semaphore.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/clk.h>
#include <asm/pgtable.h>

#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <asm/memory.h>

#include <linux/mutex.h>
#include <linux/wait.h>
/*
#include <linux/version.h>
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,16)
#include <asm/arch/registers-s3c6400.h>
#else
#include <asm-arm/arch-s3c2410/regs-s3c6400-clock.h>
#include <linux/pm.h>
#include <asm/plat-s3c24xx/pm.h>
#endif
*/
#include <plat/regs-clock.h>
#include <plat/map.h>
#include <plat/media.h>

#ifdef CONFIG_S3C6400_PDFW
#include <asm/arch/pd.h>
#if defined(CONFIG_S3C6400_KDPMD) || defined(CONFIG_S3C6400_KDPMD_MODULE)
#include <asm/arch/kdpmd.h>
#endif
#endif

#include "Mfc.h"
#include "MfcConfig.h"
#include "MFC_HW_Init.h"
#include "MFC_Instance.h"
#include "MFC_Inst_Pool.h"
//#include "LogMsg.h"
//#include "MfcMutex.h"
#include "s3c-mfc.h"
#include "FramBufMgr.h"
//#include "MfcMemory.h"
#include "DataBuf.h"
#include "MfcSfr.h"
#include "MfcIntrNotification.h"
#include "MfcDrvParams.h"

static struct clk	*mfc_hclk;
static struct clk	*mfc_sclk;
static struct clk	*mfc_pclk;

static int			_openhandle_count = 0;

static struct mutex	*hMutex = NULL;
unsigned int  	gIntrType = 0;


#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,21)
#define SAVE_START_ADDR 0x100
#define SAVE_END_ADDR	0x200
//static struct sleep_save mfc_save[SAVE_END_ADDR - SAVE_START_ADDR];
static unsigned int mfc_save[SAVE_END_ADDR - SAVE_START_ADDR];
#define MFC_READ_REG( ADDR ) (unsigned int)*(volatile unsigned int *)(ADDR)
#define MFC_WRITE_REG( ADDR, DATA ) *(volatile unsigned int *)(ADDR) = DATA
#endif

extern void s3c2410_pm_do_save(struct sleep_save *ptr, int count);
extern void s3c2410_pm_do_restore(struct sleep_save *ptr, int count);

extern int MFC_GetConfigParams(MFCInstCtx  *pMfcInst, MFC_ARGS   *args);
extern int MFC_SetConfigParams(MFCInstCtx  *pMfcInst, MFC_ARGS   *args);


typedef struct _MFC_HANDLE
{
    MFCInstCtx  *mfc_inst;

#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))
    unsigned char  *pMV;
    unsigned char  *pMBType;
#endif
} MFC_HANDLE;


#ifdef CONFIG_S3C6400_PDFW
int mfc_before_pdoff(void)
{
	printk(KERN_DEBUG, "\n%s: mfc context saving before pdoff\n", __FUNCTION__);
	return 0;
}

int mfc_after_pdon(void)
{
	printk(KERN_DEBUG, "\n%s: mfc initialization after pdon\n", __FUNCTIOIN__);	
	return 0;
}

struct pm_devtype mfc_pmdev = {
	name: "mfc",
	state: DEV_IDLE,
	before_pdoff: mfc_before_pdoff,
	after_pdon: mfc_after_pdon,
};
#endif


DECLARE_WAIT_QUEUE_HEAD(WaitQueue_MFC);

static struct resource	*mfc_mem;
static void __iomem	*mfc_base;


static irqreturn_t s3c_mfc_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned int	intReason;
	MFCInstCtx		*pMfcInst;

	
	pMfcInst = (MFCInstCtx *)dev_id;
	
	intReason	= MfcIntrReason();

	if (intReason & MFC_INTR_ENABLE_RESET) {	// if PIC_RUN, buffer full and buffer empty interrupt
		gIntrType = intReason;
		wake_up_interruptible(&WaitQueue_MFC);
	}
	
	MfcClearIntr();

	return IRQ_HANDLED;
}

static int s3c_mfc_open(struct inode *inode, struct file *file)
{
	MFC_HANDLE		*handle;
	
	
	//////////////////
	//	Mutex Lock	//
	//////////////////
	mutex_lock(hMutex);

	clk_enable(mfc_hclk);
	clk_enable(mfc_sclk);
	clk_enable(mfc_pclk);

	_openhandle_count++;
	if(_openhandle_count == 1) {
	#if defined(CONFIG_S3C6400_KDPMD) || defined(CONFIG_S3C6400_KDPMD_MODULE)
		kdpmd_set_event(mfc_pmdev.devid, KDPMD_DRVOPEN);
		kdpmd_wakeup();
		kdpmd_wait(mfc_pmdev.devid);
		mfc_pmdev.state = DEV_RUNNING;
		printk(KERN_DEBUG, "\n%s: mfc_open woke up\n", __FUNCTION__);
	#endif

		//////////////////////////////////////
		//	3. MFC Hardware Initialization	//
		//////////////////////////////////////
		if (MFC_HW_Init() == FALSE) 
			return -ENODEV;	
	}


	handle = (MFC_HANDLE *)kmalloc(sizeof(MFC_HANDLE), GFP_KERNEL);
	if(!handle) {
		printk(KERN_ERR, "\n%s: mfc open error\n", __FUNCTION__);
		mutex_unlock(hMutex);
		return -1;
	}
	memset(handle, 0, sizeof(MFC_HANDLE));
	
	
	//////////////////////////////
	//	MFC Instance creation	//
	//////////////////////////////
	handle->mfc_inst = MFCInst_Create();
	if (handle->mfc_inst == NULL) {
		printk(KERN_ERR, "\n%s: fail to mfc instance allocation\n", __FUNCTION__);
		mutex_unlock(hMutex);
		return -1;
	}

	/*
	 * MFC supports multi-instance. so each instance have own data structure
	 * It saves file->private_data
	 */
	file->private_data = (MFC_HANDLE *)handle;
	

	mutex_unlock(hMutex);

	printk(KERN_DEBUG, "\n%s: mfc open success\n", __FUNCTION__);
	
	return 0;
}


static int s3c_mfc_release(struct inode *inode, struct file *file)
{
	MFC_HANDLE				*handle = NULL;
	

	mutex_lock(hMutex);

	handle = (MFC_HANDLE *)file->private_data;
	if (handle->mfc_inst == NULL) {
		mutex_unlock(hMutex);
		return -1;
	};
	
	printk(KERN_DEBUG, "\n%s: deleting instance number = %d\n", __FUNCTION__, handle->mfc_inst->inst_no);

	MFCInst_Delete(handle->mfc_inst);

	//kfree(handle);

	_openhandle_count--;
	if(_openhandle_count == 0) {

	#if defined(CONFIG_S3C6400_KDPMD) || defined(CONFIG_S3C6400_KDPMD_MODULE)
		mfc_pmdev.state = DEV_IDLE;
		kdpmd_set_event(mfc_pmdev.devid, KDPMD_DRVCLOSE);
		kdpmd_wakeup();
		kdpmd_wait(mfc_pmdev.devid);
	#endif

		clk_disable(mfc_hclk);
		clk_disable(mfc_sclk);
		clk_disable(mfc_pclk);		
	}
	
	mutex_unlock(hMutex);
	
	return 0;
}


static ssize_t s3c_mfc_write (struct file *file, const char *buf, size_t
		count, loff_t *pos)
{
	return 0;
}

static ssize_t s3c_mfc_read(struct file *file, char *buf, size_t count, loff_t *pos)
{	
	return 0;
}

static int s3c_mfc_ioctl(struct inode *inode, struct file *file, unsigned
		int cmd, unsigned long arg)
{
	int					ret	= 0;
	MFCInstCtx			*pMfcInst;
	MFC_HANDLE			*handle;
	unsigned char		*OutBuf	= NULL;
	MFC_CODECMODE		codec_mode	= 0;
	int					buf_size;
	unsigned int		tmp;
	MFC_ARGS			args;
	enc_info_t			enc_info;
	int 	            nStrmLen, nHdrLen;

	void				*temp;
	unsigned int		vir_mv_addr;
	unsigned int		vir_mb_type_addr;

	
	//////////////////////
	//	Parameter Check	//
	//////////////////////
	handle = (MFC_HANDLE *)file->private_data;
	if (handle->mfc_inst == NULL) {
		return -EFAULT;
	}

	pMfcInst = handle->mfc_inst;


	switch(cmd)
	{
		case IOCTL_MFC_MPEG4_ENC_INIT:
		case IOCTL_MFC_H264_ENC_INIT:
		case IOCTL_MFC_H263_ENC_INIT:
			mutex_lock(hMutex);
			
			printk(KERN_DEBUG, "\n%s: cmd = %d\n", __FUNCTION__, cmd);

			copy_from_user(&args.enc_init, (MFC_ENC_INIT_ARG *)arg, sizeof(MFC_ENC_INIT_ARG));
			
			if ( cmd == IOCTL_MFC_MPEG4_ENC_INIT )
				codec_mode = MP4_ENC;
			else if ( cmd == IOCTL_MFC_H264_ENC_INIT )
				codec_mode = AVC_ENC;
			else if ( cmd == IOCTL_MFC_H263_ENC_INIT )
				codec_mode = H263_ENC;
			
			//////////////////////////////
			//	Initialize MFC Instance	//
			//////////////////////////////
			enc_info.width			= args.enc_init.in_width;
			enc_info.height			= args.enc_init.in_height;
			enc_info.bitrate		= args.enc_init.in_bitrate;
			enc_info.gopNum			= args.enc_init.in_gopNum;
			enc_info.frameRateRes	= args.enc_init.in_frameRateRes;
			enc_info.frameRateDiv	= args.enc_init.in_frameRateDiv;

//			enc_info.intraqp	= args.enc_init.in_intraqp;
//			enc_info.qpmax		= args.enc_init.in_qpmax;
//			enc_info.gamma		= args.enc_init.in_gamma;
			
			ret = MFCInst_Enc_Init(pMfcInst, codec_mode, &enc_info);
			
			args.enc_init.ret_code = ret;
			copy_to_user((MFC_ENC_INIT_ARG *)arg, &args.enc_init, sizeof(MFC_ENC_INIT_ARG));
			
			mutex_unlock(hMutex);
			break;
			
		case IOCTL_MFC_MPEG4_ENC_EXE:
		case IOCTL_MFC_H264_ENC_EXE:
		case IOCTL_MFC_H263_ENC_EXE:
			mutex_lock(hMutex);

			copy_from_user(&args.enc_exe, (MFC_ENC_EXE_ARG *)arg, sizeof(MFC_ENC_EXE_ARG));

			tmp = (pMfcInst->width * pMfcInst->height * 3) >> 1;
			dmac_clean_range(pMfcInst->pFramBuf, pMfcInst->pFramBuf + tmp);
			outer_clean_range(__pa(pMfcInst->pFramBuf), __pa(pMfcInst->pFramBuf + tmp));
			
			//////////////////////////
			//	Decode MFC Instance	//
			//////////////////////////
			ret = MFCInst_Encode(pMfcInst, &nStrmLen, &nHdrLen);

			dmac_clean_range(pMfcInst->pStrmBuf, pMfcInst->pStrmBuf + MFC_LINE_BUF_SIZE_PER_INSTANCE);
			outer_clean_range(__pa(pMfcInst->pStrmBuf), __pa(pMfcInst->pStrmBuf + MFC_LINE_BUF_SIZE_PER_INSTANCE));
			
			args.enc_exe.ret_code	= ret;
			if (ret == MFCINST_RET_OK) {
				args.enc_exe.out_encoded_size = nStrmLen;
				args.enc_exe.out_header_size  = nHdrLen;
			}
			copy_to_user((MFC_ENC_EXE_ARG *)arg, &args.enc_exe, sizeof(MFC_ENC_EXE_ARG));
			
			mutex_unlock(hMutex);
			break;
			
		case IOCTL_MFC_MPEG4_DEC_INIT:
		case IOCTL_MFC_H264_DEC_INIT:
		case IOCTL_MFC_H263_DEC_INIT:
		case IOCTL_MFC_VC1_DEC_INIT:
			mutex_lock(hMutex);

			copy_from_user(&args.dec_init, (MFC_DEC_INIT_ARG *)arg, sizeof(MFC_DEC_INIT_ARG));
			
			if ( cmd == IOCTL_MFC_MPEG4_DEC_INIT )
				codec_mode = MP4_DEC;
			else if ( cmd == IOCTL_MFC_H264_DEC_INIT )
				codec_mode = AVC_DEC;
			else if ( cmd == IOCTL_MFC_H263_DEC_INIT)
				codec_mode = H263_DEC;
			else {
				codec_mode = VC1_DEC;
			}
			
			//////////////////////////////
			//	Initialize MFC Instance	//
			//////////////////////////////
			ret = MFCInst_Dec_Init(pMfcInst, codec_mode, args.dec_init.in_strmSize);

			args.dec_init.ret_code	= ret;
			if (ret == MFCINST_RET_OK) {
				args.dec_init.out_width	     = pMfcInst->width;
				args.dec_init.out_height     = pMfcInst->height;
				args.dec_init.out_buf_width  = pMfcInst->buf_width;
				args.dec_init.out_buf_height = pMfcInst->buf_height;
			}
			copy_to_user((MFC_DEC_INIT_ARG *)arg, &args.dec_init, sizeof(MFC_DEC_INIT_ARG));

			mutex_unlock(hMutex);
			break;
			
		case IOCTL_MFC_MPEG4_DEC_EXE:
		case IOCTL_MFC_H264_DEC_EXE:
		case IOCTL_MFC_H263_DEC_EXE:
		case IOCTL_MFC_VC1_DEC_EXE:
			mutex_lock(hMutex);

			copy_from_user(&args.dec_exe, (MFC_DEC_EXE_ARG *)arg, sizeof(MFC_DEC_EXE_ARG));

			//dma_map_single(NULL, pMfcInst->pStrmBuf, MFC_LINE_BUF_SIZE_PER_INSTANCE, DMA_TO_DEVICE);
			dmac_clean_range(pMfcInst->pStrmBuf, pMfcInst->pStrmBuf + MFC_LINE_BUF_SIZE_PER_INSTANCE);
			outer_clean_range(__pa(pMfcInst->pStrmBuf), __pa(pMfcInst->pStrmBuf + MFC_LINE_BUF_SIZE_PER_INSTANCE));
			
			ret = MFCInst_Decode(pMfcInst, args.dec_exe.in_strmSize);

			args.dec_exe.ret_code = ret;
			copy_to_user((MFC_DEC_EXE_ARG *)arg, &args.dec_exe, sizeof(MFC_DEC_EXE_ARG));

			mutex_unlock(hMutex);
			break;
						
		case IOCTL_MFC_GET_LINE_BUF_ADDR:
			mutex_lock(hMutex);

			copy_from_user(&args.get_buf_addr, (MFC_GET_BUF_ADDR_ARG *)arg, sizeof(MFC_GET_BUF_ADDR_ARG));

			ret = MFCInst_GetLineBuf(pMfcInst, &OutBuf, &buf_size);
			
			args.get_buf_addr.out_buf_size	= buf_size;
			args.get_buf_addr.out_buf_addr	= args.get_buf_addr.in_usr_data + (OutBuf - GetDataBufVirAddr());
			args.get_buf_addr.ret_code		= ret;

			copy_to_user((MFC_GET_BUF_ADDR_ARG *)arg, &args.get_buf_addr, sizeof(MFC_GET_BUF_ADDR_ARG));

			mutex_unlock(hMutex);
			break;
			
		case IOCTL_MFC_GET_FRAM_BUF_ADDR:
			mutex_lock(hMutex);

			copy_from_user(&args.get_buf_addr, (MFC_GET_BUF_ADDR_ARG *)arg, sizeof(MFC_GET_BUF_ADDR_ARG));

			if (pMfcInst->pFramBuf == NULL) {
				printk(KERN_ERR, "\n%s: mfc frame buffer is not internally allocated yet\n", __FUNCTION__);
				mutex_unlock(hMutex);
				return -EFAULT;
			}

			// FRAM_BUF address is calculated differently for Encoder and Decoder.
			switch (pMfcInst->codec_mode) {
			case MP4_DEC:
			case AVC_DEC:
			case VC1_DEC:
			case H263_DEC:
				// Decoder case
				args.get_buf_addr.out_buf_size	= (pMfcInst->buf_width * pMfcInst->buf_height * 3) >> 1;
				tmp	= (unsigned int)args.get_buf_addr.in_usr_data + ( ((unsigned int) pMfcInst->pFramBuf)	\
					+ (pMfcInst->run_index) * (args.get_buf_addr.out_buf_size) - (unsigned int)GetDataBufVirAddr() );
#if (MFC_ROTATE_ENABLE == 1)
				if ( (pMfcInst->codec_mode != VC1_DEC) && (pMfcInst->PostRotMode & 0x0010) ) {
					tmp	= (unsigned int)args.get_buf_addr.in_usr_data + ( ((unsigned int) pMfcInst->pFramBuf)	\
					+ (pMfcInst->frambufCnt) * (args.get_buf_addr.out_buf_size) - (unsigned int)GetDataBufVirAddr() );	
				}
#endif
				args.get_buf_addr.out_buf_addr = tmp;
				break;

			case MP4_ENC:
			case AVC_ENC:
			case H263_ENC:
				// Encoder case
				tmp = (pMfcInst->width * pMfcInst->height * 3) >> 1;
				args.get_buf_addr.out_buf_addr = args.get_buf_addr.in_usr_data + (pMfcInst->run_index * tmp) + (int)(pMfcInst->pFramBuf - GetDataBufVirAddr());
				break;
			}

			args.get_buf_addr.ret_code	= MFCINST_RET_OK;
			copy_to_user((MFC_GET_BUF_ADDR_ARG *)arg, &args.get_buf_addr, sizeof(MFC_GET_BUF_ADDR_ARG));

			mutex_unlock(hMutex);
			break;
			
		case IOCTL_MFC_GET_PHY_FRAM_BUF_ADDR:
			mutex_lock(hMutex);

			copy_from_user(&args.get_buf_addr, (MFC_GET_BUF_ADDR_ARG *)arg, sizeof(MFC_GET_BUF_ADDR_ARG));

			args.get_buf_addr.out_buf_size	= (pMfcInst->buf_width * pMfcInst->buf_height * 3) >> 1;
//			args.get_buf_addr.out_buf_size	= ((pMfcInst->width+2*DIVX_PADDING) * (pMfcInst->height+2*DIVX_PADDING) * 3) >> 1;
			tmp	= (unsigned int)S3C6400_BASEADDR_MFC_DATA_BUF + ( ((unsigned int) pMfcInst->pFramBuf)	\
				+ (pMfcInst->run_index) * (args.get_buf_addr.out_buf_size) - (unsigned int)GetDataBufVirAddr() );

//.[ i: sichoi 081103 (ROTATE)
#if (MFC_ROTATE_ENABLE == 1)
            if ( (pMfcInst->codec_mode != VC1_DEC) && (pMfcInst->PostRotMode & 0x0010) ) {
                tmp = (unsigned int)S3C6400_BASEADDR_MFC_DATA_BUF + ( ((unsigned int) pMfcInst->pFramBuf)   \
                + (pMfcInst->frambufCnt) * (args.get_buf_addr.out_buf_size) - (unsigned int)GetDataBufVirAddr() );  
            }
#endif
//.] sichoi 081103

			args.get_buf_addr.out_buf_addr = tmp;
			args.get_buf_addr.ret_code = MFCINST_RET_OK;

			copy_to_user((MFC_GET_BUF_ADDR_ARG *)arg, &args.get_buf_addr, sizeof(MFC_GET_BUF_ADDR_ARG));

			mutex_unlock(hMutex);
			break;

		case IOCTL_MFC_GET_MPEG4_ASP_PARAM:
		#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))

			copy_from_user(&args.mpeg4_asp_param, (MFC_GET_MPEG4ASP_ARG *)arg, sizeof(MFC_GET_MPEG4ASP_ARG));
		
			ret = MFCINST_RET_OK;
			args.mpeg4_asp_param.ret_code              = MFCINST_RET_OK;
			args.mpeg4_asp_param.mp4asp_vop_time_res   = pMfcInst->RET_DEC_SEQ_INIT_BAK_MP4ASP_VOP_TIME_RES;
			args.mpeg4_asp_param.byte_consumed         = pMfcInst->RET_DEC_PIC_RUN_BAK_BYTE_CONSUMED;
			args.mpeg4_asp_param.mp4asp_fcode          = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_FCODE;
			args.mpeg4_asp_param.mp4asp_time_base_last = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_TIME_BASE_LAST;
			args.mpeg4_asp_param.mp4asp_nonb_time_last = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_NONB_TIME_LAST;
			args.mpeg4_asp_param.mp4asp_trd            = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_MP4ASP_TRD;
			
   			args.mpeg4_asp_param.mv_addr      = (args.mpeg4_asp_param.in_usr_mapped_addr + MFC_STRM_BUF_SIZE) + (pMfcInst->mv_mbyte_addr - pMfcInst->phyadrFramBuf);
			args.mpeg4_asp_param.mb_type_addr = args.mpeg4_asp_param.mv_addr + 25920;	
			args.mpeg4_asp_param.mv_size      = 25920; // '25920' is the maximum MV size (=45*36*16)
			args.mpeg4_asp_param.mb_type_size = 1620;  // '1620' is the maximum MBTYE size (=45*36*1)
		
			vir_mv_addr = (pMfcInst->pStrmBuf + MFC_STRM_BUF_SIZE) + (pMfcInst->mv_mbyte_addr - pMfcInst->phyadrFramBuf);
			vir_mb_type_addr = vir_mv_addr + 25920;

			copy_to_user((MFC_GET_MPEG4ASP_ARG *)arg, &args.mpeg4_asp_param, sizeof(MFC_GET_MPEG4ASP_ARG));

			dmac_clean_range(vir_mv_addr, vir_mv_addr + args.mpeg4_asp_param.mv_size);
			outer_clean_range(__pa(vir_mv_addr), __pa(vir_mv_addr + args.mpeg4_asp_param.mv_size));

			dmac_clean_range(vir_mb_type_addr, vir_mb_type_addr + args.mpeg4_asp_param.mb_type_size);
			outer_clean_range(__pa(vir_mb_type_addr), __pa(vir_mb_type_addr + args.mpeg4_asp_param.mb_type_size));
		#endif	
	
		case IOCTL_MFC_GET_CONFIG:
			mutex_lock(hMutex);

			copy_from_user(&args, (MFC_ARGS *)arg, sizeof(MFC_ARGS));

			ret = MFC_GetConfigParams(pMfcInst, &args);

			copy_to_user((MFC_ARGS *)arg, &args, sizeof(MFC_ARGS));

			mutex_unlock(hMutex);
			break;

		case IOCTL_MFC_SET_CONFIG:
			mutex_lock(hMutex);

			copy_from_user(&args, (MFC_ARGS *)arg, sizeof(MFC_ARGS));

			ret = MFC_SetConfigParams(pMfcInst, &args);

			copy_to_user((MFC_ARGS *)arg, &args, sizeof(MFC_ARGS));
			
			mutex_unlock(hMutex);
			break;

		case IOCTL_VIRT_TO_PHYS:
			temp = __virt_to_phys((void *)arg);
			return (int)temp;
			break;
			
		default:
			mutex_lock(hMutex);
			printk(KERN_DEBUG, "\n%s: requested ioctl command is not defined (ioctl cmd = 0x%x)\n", __FUNCTION__, cmd);
			mutex_unlock(hMutex);
			return -EINVAL;
	}

	switch(ret) {
	case MFCINST_RET_OK:
		return TRUE;
	default:
		return -1;
	}
	return -1;
}

int s3c_mfc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long size	= vma->vm_end - vma->vm_start;
	unsigned long maxSize;
	unsigned long pageFrameNo;

	pageFrameNo = __phys_to_pfn(S3C6400_BASEADDR_MFC_DATA_BUF);

	maxSize = MFC_DATA_BUF_SIZE + PAGE_SIZE - (MFC_DATA_BUF_SIZE % PAGE_SIZE);

	if(size > maxSize) {
		return -EINVAL;
	}

	vma->vm_flags |= VM_RESERVED | VM_IO;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if( remap_pfn_range(vma, vma->vm_start, pageFrameNo, size,	\
				vma->vm_page_prot) ) {
		printk(KERN_ERR, "\n%s: fail to remap\n", __FUNCTION__);
		return -EAGAIN;
	}

	return 0;
}


static struct file_operations s3c_mfc_fops = {
			owner:		THIS_MODULE,
			open:		s3c_mfc_open,
			release:	s3c_mfc_release,
			ioctl:		s3c_mfc_ioctl,
			read:		s3c_mfc_read,
			write:		s3c_mfc_write,
			mmap:		s3c_mfc_mmap,
};


static struct miscdevice s3c_mfc_miscdev = {
			minor:		252, 		
			name:		"s3c-mfc",
			fops:		&s3c_mfc_fops
};


static int s3c_mfc_probe(struct platform_device *pdev)
{
	struct resource *res;
	int 			size;
	int				ret;
	unsigned int	mfc_clk;
	
	// mfc clock enable 
	mfc_hclk	= clk_get(NULL, "hclk_mfc");
	if (!mfc_hclk) {
		printk(KERN_ERR "\n%s: failed to get mfc hclk source\n", __FUNCTION__);
		return -ENOENT;
	}
	clk_enable(mfc_hclk);

	mfc_sclk	= clk_get(NULL, "sclk_mfc");
	if (!mfc_sclk) {
		printk(KERN_ERR "\n%s: failed to get mfc sclk source\n", __FUNCTION__);
		return -ENOENT;
	}
	clk_enable(mfc_sclk);

	mfc_pclk	= clk_get(NULL, "pclk_mfc");
	if (!mfc_pclk) {
		printk(KERN_ERR "\n%s: failed to get mfc pclk source\n", __FUNCTION__);
		return -ENOENT;
	}
	clk_enable(mfc_pclk);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		printk(KERN_ERR "\n%s: failed to get memory region resouce\n", __FUNCTION__);
		return -ENOENT;
	}

	size = (res->end-res->start)+1;
	mfc_mem = request_mem_region(res->start, size, pdev->name);
	if (mfc_mem == NULL) {
		printk(KERN_ERR "\n%s: failed to get memory region\n", __FUNCTION__);
		return -ENOENT;
	}

	mfc_base = ioremap(res->start, size);
	if (mfc_base == 0) {
		printk(KERN_ERR "\n%s: failed to ioremap() region\n", __FUNCTION__);
		return -EINVAL;
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		printk(KERN_ERR "\n%s: failed to get irq resource\n", __FUNCTION__);
		return -ENOENT;
	}

	
	ret = request_irq(res->start, s3c_mfc_irq, 0, pdev->name, pdev);
	if (ret != 0) {
		printk(KERN_ERR "failed to install irq (%d)\n", __FUNCTION__, ret);
		return ret;
	}

	/* mutex creation and initialization */
	hMutex = (struct mutex *)kmalloc(sizeof(struct mutex), GFP_KERNEL);
	if (hMutex == NULL)
		return -ENOMEM;

	mutex_init(hMutex);
	
	// mfc clock set 133 Mhz
	mfc_clk = readl(S3C_CLK_DIV0);
	mfc_clk |= (1<<28);
	__raw_writel(mfc_clk, S3C_CLK_DIV0);

	//////////////////////////////////
	//	2. MFC Memory Setup			//
	//////////////////////////////////
	if (MFC_MemorySetup() == FALSE)
		return -ENOMEM;
	
	//////////////////////////////////////
	//	3. MFC Hardware Initialization	//
	//////////////////////////////////////
	if (MFC_HW_Init() == FALSE)
		return -ENODEV;

	ret = misc_register(&s3c_mfc_miscdev);

	clk_disable(mfc_hclk);
	clk_disable(mfc_sclk);
	clk_disable(mfc_pclk);

	return 0;
}

static int s3c_mfc_remove(struct platform_device *dev)
{
	if (mfc_mem != NULL) {
		release_resource(mfc_mem);
		kfree(mfc_mem);
		mfc_mem = NULL;
	}

	free_irq(IRQ_MFC, dev);
	
	misc_deregister(&s3c_mfc_miscdev);
	return 0;
}

static int s3c_mfc_suspend(struct platform_device *dev, pm_message_t state)
{

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,21)

	MFCInstCtx *mfcinst_ctx;
	int         inst_no;
	int			is_mfc_on = 0;
	int			i, index = 0;
	unsigned int	dwMfcBase;


	mutex_lock(hMutex);

	
	is_mfc_on = 0;

	// 1. Power Off state
	// Invalidate all the MFC Instances
	for (inst_no = 0; inst_no < MFC_NUM_INSTANCES_MAX; inst_no++) {
		mfcinst_ctx = MFCInst_GetCtx(inst_no);
		if (mfcinst_ctx) {
			is_mfc_on = 1;

			// On Power Down, the MFC instance is invalidated.
			// Then the MFC operations (DEC_EXE, ENC_EXE, etc.) will not be performed
			// until it is validated by entering Power up state transition
			MFCInst_PowerOffState(mfcinst_ctx);
			printk(KERN_ERR "\n%s: mfc suspend %d-th instance is invalidated\n", __FUNCTION__, inst_no);
		}
	}

	// 2. Command MFC sleep and save MFC SFR
	if (is_mfc_on) {
		dwMfcBase = (unsigned int)GetMfcSfrVirAddr();

		for(i=SAVE_START_ADDR; i <= SAVE_END_ADDR; i+=4)
		{
			mfc_save[index] = (unsigned int)MFC_READ_REG(dwMfcBase + i);
			index++;	
		}

		MFC_Sleep();
	}


	// 3. Disable MFC clock
	clk_disable(mfc_hclk);
	clk_disable(mfc_sclk);
	clk_disable(mfc_pclk);
	
/*
	// 4. MFC Power Off(Domain V)
	mfc_pwr = readl(S3C_NORMAL_CFG);
	mfc_pwr &= ~(1<<9);
	printk("mfc_pwr : 0x%X\n", mfc_pwr);
	__raw_writel(mfc_pwr, S3C_NORMAL_CFG);
*/

	mutex_unlock(hMutex);

#endif

	return 0;
}

static int s3c_mfc_resume(struct platform_device *pdev)
{

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,21)

	unsigned int	mfc_pwr, dwMfcBase;
	int 			i, index = 0;
	MFCInstCtx 		*mfcinst_ctx;
	int         	inst_no;
	unsigned int	domain_v_ready;
	int				is_mfc_on = 0;
	
	
	mutex_lock(hMutex);
	
	clk_enable(mfc_hclk);
	clk_enable(mfc_sclk);
	clk_enable(mfc_pclk);


	// 1. MFC Power On(Domain V)
	mfc_pwr = readl(S3C_NORMAL_CFG);
	mfc_pwr |= (1<<9);
	__raw_writel(mfc_pwr, S3C_NORMAL_CFG);


	// 2. Check MFC power on
	do {
		domain_v_ready = readl(S3C_BLK_PWR_STAT);
		printk(KERN_DEBUG, "\n%s: domain v ready = 0x%X\n", __FUNCTION__, domain_v_ready);
		msleep(1);
	}while(!(domain_v_ready & (1<<1)));


	// 3. Firmware download
	MfcFirmwareIntoCodeDownReg();


	// 4. Power On state
	// Validate all the MFC Instances
	for (inst_no = 0; inst_no < MFC_NUM_INSTANCES_MAX; inst_no++) {
		mfcinst_ctx = MFCInst_GetCtx(inst_no);
		if (mfcinst_ctx) {
			is_mfc_on = 1;

			// When MFC Power On, the MFC instance is validated.
			// Then the MFC operations (DEC_EXE, ENC_EXE, etc.) will be performed again
			MFCInst_PowerOnState(mfcinst_ctx);
			printk(KERN_DEBUG, "\n%s: mfc resume %d-th instance is validated\n", __FUNCTION__, inst_no);
		}
	}


	if (is_mfc_on) {
		// 5. Restore MFC SFR
		dwMfcBase = (unsigned int)GetMfcSfrVirAddr();
		for( i=SAVE_START_ADDR; i<= SAVE_END_ADDR; i+=4 )
		{
			MFC_WRITE_REG( ( dwMfcBase + i ), mfc_save[index] );
			index++;
		}

		// 6. Command MFC wakeup
		MFC_Wakeup();
	}

	mutex_unlock(hMutex);

#endif

	return 0;
}

static struct platform_driver s3c_mfc_driver = {
	.probe		= s3c_mfc_probe,
	.remove		= s3c_mfc_remove,
	.shutdown	= NULL,
	.suspend	= s3c_mfc_suspend,
	.resume		= s3c_mfc_resume,
	.driver		= {
	.owner		= THIS_MODULE,
	.name		= "s3c-mfc",
	},
};


static char banner[] __initdata = KERN_INFO "S3C6400 MFC Driver, (c) 2007 Samsung Electronics\n";

static int __init s3c_mfc_init(void)
{

	printk(banner);

#ifdef CONFIG_S3C6400_PDFW
	pd_register_dev(&mfc_pmdev, "domain_v");
	printk(KERN_INFO "\n%s: mfc devid = %d\n", __FUNCTION__, mfc_pmdev.devid);
#endif

	if(platform_driver_register(&s3c_mfc_driver)!=0)
  	{
   		printk(KERN_ERR, "\n%s: fail to register platform device\n", __FUNCTION__);
   		return -1;
  	}

	return 0;
}

static void __exit s3c_mfc_exit(void)
{
	mutex_destroy(hMutex);
	
#ifdef CONFIG_S3C6400_PDFW
	pd_unregister_dev(&mfc_pmdev);
#endif

	platform_driver_unregister(&s3c_mfc_driver);
	printk(KERN_DEBUG, "\n%s: S3C6400 MFC driver exit\n", __FUNCTION__);	
}


module_init(s3c_mfc_init);
module_exit(s3c_mfc_exit);

MODULE_AUTHOR("Jiun, Yu");
MODULE_LICENSE("GPL");

