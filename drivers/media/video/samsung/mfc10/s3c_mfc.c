/* linux/driver/media/video/mfc/s3c_mfc.c
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
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/mutex.h>
#include <linux/wait.h>

#include <asm/cacheflush.h>
#include <asm/memory.h>
#include <asm/io.h>
#include <asm/page.h>
#include <asm/pgtable.h>

//#include <plat/reserved_mem.h>
#include <plat/regs-clock.h>
#include <plat/regs-mfc.h>
#include <plat/map.h>
#include <plat/media.h>


#ifdef CONFIG_S3C6400_PDFW
#include <asm/arch/pd.h>
#if defined(CONFIG_S3C6400_KDPMD) || defined(CONFIG_S3C6400_KDPMD_MODULE)
#include <asm/arch/kdpmd.h>
#endif
#endif

#define S3C_MFC_PHYS_BUFFER_SET

#include "s3c_mfc_base.h"
#include "s3c_mfc_config.h"
#include "s3c_mfc_hw_init.h"
#include "s3c_mfc_instance.h"
#include "s3c_mfc_inst_pool.h"
#include "s3c_mfc.h"
#include "s3c_mfc_yuv_buf_manager.h"
#include "s3c_mfc_databuf.h"
#include "s3c_mfc_sfr.h"
#include "s3c_mfc_intr_noti.h"
#include "s3c_mfc_params.h"

static struct clk	*s3c_mfc_hclk;
static struct clk	*s3c_mfc_sclk;
static struct clk	*s3c_mfc_pclk;

static int			s3c_mfc_openhandle_count = 0;

static struct mutex	*s3c_mfc_mutex = NULL;
unsigned int		s3c_mfc_intr_type = 0;


#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,21)
#define S3C_MFC_SAVE_START_ADDR 0x100
#define S3C_MFC_SAVE_END_ADDR	0x200
static unsigned int s3c_mfc_save[S3C_MFC_SAVE_END_ADDR - S3C_MFC_SAVE_START_ADDR];
#endif

extern int s3c_mfc_get_config_params(s3c_mfc_instance_context_t  *pMfcInst, s3c_mfc_args_t   *args);
extern int s3c_mfc_set_config_params(s3c_mfc_instance_context_t  *pMfcInst, s3c_mfc_args_t   *args);

typedef struct _MFC_HANDLE {
	s3c_mfc_instance_context_t  *mfc_inst;

#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))
	unsigned char  *pMV;
	unsigned char  *pMBType;
#endif
} s3c_mfc_handle_t;


#ifdef CONFIG_S3C6400_PDFW
int s3c_mfc_before_pdoff(void)
{
	printk(KERN_DEBUG "\n%s: mfc context saving before pdoff\n", __FUNCTION__);
	return 0;
}

int s3c_mfc_after_pdon(void)
{
	printk(KERN_DEBUG "\n%s: mfc initialization after pdon\n", __FUNCTIOIN__);	
	return 0;
}

struct pm_devtype s3c_mfc_pmdev = {
	name: "mfc",
	state: DEV_IDLE,
	before_pdoff: s3c_mfc_before_pdoff,
	after_pdon: s3c_mfc_after_pdon,
};
#endif


DECLARE_WAIT_QUEUE_HEAD(s3c_mfc_wait_queue);

static struct resource	*s3c_mfc_mem;
void __iomem	*s3c_mfc_sfr_base_virt_addr;

dma_addr_t s3c_mfc_phys_buffer;


static irqreturn_t s3c_mfc_irq(int irq, void *dev_id)
{
	unsigned int	intReason;
	s3c_mfc_instance_context_t		*pMfcInst;


	pMfcInst = (s3c_mfc_instance_context_t *)dev_id;

	intReason	= s3c_mfc_intr_reason();

	if (intReason & S3C_MFC_INTR_ENABLE_RESET) {	/* if PIC_RUN, buffer full and buffer empty interrupt */
		s3c_mfc_intr_type = intReason;
		wake_up_interruptible(&s3c_mfc_wait_queue);
	}

	s3c_mfc_clear_intr();

	return IRQ_HANDLED;
}

static int s3c_mfc_open(struct inode *inode, struct file *file)
{
	s3c_mfc_handle_t		*handle;


	/* 
	 * Mutex Lock
	 */
	mutex_lock(s3c_mfc_mutex);

	clk_enable(s3c_mfc_hclk);
	clk_enable(s3c_mfc_sclk);
	clk_enable(s3c_mfc_pclk);

	s3c_mfc_openhandle_count++;
	if (s3c_mfc_openhandle_count == 1) {
#if defined(CONFIG_S3C6400_KDPMD) || defined(CONFIG_S3C6400_KDPMD_MODULE)
		kdpmd_set_event(s3c_mfc_pmdev.devid, KDPMD_DRVOPEN);
		kdpmd_wakeup();
		kdpmd_wait(s3c_mfc_pmdev.devid);
		s3c_mfc_pmdev.state = DEV_RUNNING;
		printk(KERN_DEBUG "\n%s: mfc_open woke up\n", __FUNCTION__);
#endif

		/*
		 * 3. MFC Hardware Initialization
		 */
		if (s3c_mfc_hw_init() == FALSE) 
			return -ENODEV;	
	}


	handle = (s3c_mfc_handle_t *)kmalloc(sizeof(s3c_mfc_handle_t), GFP_KERNEL);
	if (!handle) {
		printk(KERN_ERR "\n%s: mfc open error\n", __FUNCTION__);
		mutex_unlock(s3c_mfc_mutex);
		return -ENOMEM;
	}
	memset(handle, 0, sizeof(s3c_mfc_handle_t));


	/* 
	 * MFC Instance creation
	 */
	handle->mfc_inst = s3c_mfc_instance_create();
	if (handle->mfc_inst == NULL) {
		printk(KERN_ERR "\n%s: fail to mfc instance allocation\n", __FUNCTION__);
		mutex_unlock(s3c_mfc_mutex);
		return -EPERM;
	}

	/*
	 * MFC supports multi-instance. so each instance have own data structure
	 * It saves file->private_data
	 */
	file->private_data = (s3c_mfc_handle_t *)handle;

	mutex_unlock(s3c_mfc_mutex);

	printk(KERN_DEBUG "\n%s: mfc open success\n", __FUNCTION__);

	return 0;
}


static int s3c_mfc_release(struct inode *inode, struct file *file)
{
	s3c_mfc_handle_t *handle = NULL;

	mutex_lock(s3c_mfc_mutex);

	handle = (s3c_mfc_handle_t *)file->private_data;
	if (handle->mfc_inst == NULL) {
		mutex_unlock(s3c_mfc_mutex);
		return -EPERM;
	};

	printk(KERN_DEBUG "\n%s: deleting instance number = %d\n", __FUNCTION__, handle->mfc_inst->inst_no);

	s3c_mfc_instance_delete(handle->mfc_inst);

	s3c_mfc_openhandle_count--;
	if (s3c_mfc_openhandle_count == 0) {

#if defined(CONFIG_S3C6400_KDPMD) || defined(CONFIG_S3C6400_KDPMD_MODULE)
		s3c_mfc_pmdev.state = DEV_IDLE;
		kdpmd_set_event(s3c_mfc_pmdev.devid, KDPMD_DRVCLOSE);
		kdpmd_wakeup();
		kdpmd_wait(s3c_mfc_pmdev.devid);
#endif

		clk_disable(s3c_mfc_hclk);
		clk_disable(s3c_mfc_sclk);
		clk_disable(s3c_mfc_pclk);		
	}

	mutex_unlock(s3c_mfc_mutex);

	return 0;
}


static ssize_t s3c_mfc_write (struct file *file, const char *buf, size_t count, loff_t *pos)
{
	return 0;
}

static ssize_t s3c_mfc_read(struct file *file, char *buf, size_t count, loff_t *pos)
{	
	return 0;
}

static int s3c_mfc_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int buf_size;
	int nStrmLen, nHdrLen;
	int out;

	void		*temp;
	unsigned int	vir_mv_addr;
	unsigned int	vir_mb_type_addr;
	unsigned int	tmp;
	unsigned char	*OutBuf	= NULL;

	s3c_mfc_instance_context_t	*pMfcInst;
	s3c_mfc_handle_t		*handle;
	s3c_mfc_codec_mode_t		codec_mode = 0;
	s3c_mfc_args_t			args;
	s3c_mfc_enc_info_t		enc_info;

	/* 
	 * Parameter Check
	 */
	handle = (s3c_mfc_handle_t *)file->private_data;
	if (handle->mfc_inst == NULL) {
		return -EFAULT;
	}

	pMfcInst = handle->mfc_inst;

	switch (cmd) {
	case S3C_MFC_IOCTL_MFC_MPEG4_ENC_INIT:
	case S3C_MFC_IOCTL_MFC_H264_ENC_INIT:
	case S3C_MFC_IOCTL_MFC_H263_ENC_INIT:
		mutex_lock(s3c_mfc_mutex);

		printk(KERN_DEBUG "\n%s: cmd = %d\n", __FUNCTION__, cmd);

		out = copy_from_user(&args.enc_init, (s3c_mfc_enc_init_arg_t *)arg, sizeof(s3c_mfc_enc_init_arg_t));

		if ( cmd == S3C_MFC_IOCTL_MFC_MPEG4_ENC_INIT )
			codec_mode = MP4_ENC;
		else if ( cmd == S3C_MFC_IOCTL_MFC_H264_ENC_INIT )
			codec_mode = AVC_ENC;
		else if ( cmd == S3C_MFC_IOCTL_MFC_H263_ENC_INIT )
			codec_mode = H263_ENC;

		/* 
		 * Initialize MFC Instance
		 */
		enc_info.width			= args.enc_init.in_width;
		enc_info.height			= args.enc_init.in_height;
		enc_info.bitrate		= args.enc_init.in_bitrate;
		enc_info.gop_number		= args.enc_init.in_gopNum;
		enc_info.frame_rate_residual	= args.enc_init.in_frameRateRes;
		enc_info.frame_rate_division	= args.enc_init.in_frameRateDiv;

		/*
		enc_info.intraqp	= args.enc_init.in_intraqp;
		enc_info.qpmax		= args.enc_init.in_qpmax;
		enc_info.gamma		= args.enc_init.in_gamma;
		*/

		ret = s3c_mfc_instance_enc_init(pMfcInst, codec_mode, &enc_info);

		args.enc_init.ret_code = ret;
		out = copy_to_user((s3c_mfc_enc_init_arg_t *)arg, &args.enc_init, sizeof(s3c_mfc_enc_init_arg_t));

		mutex_unlock(s3c_mfc_mutex);
		break;

	case S3C_MFC_IOCTL_MFC_MPEG4_ENC_EXE:
	case S3C_MFC_IOCTL_MFC_H264_ENC_EXE:
	case S3C_MFC_IOCTL_MFC_H263_ENC_EXE:
		mutex_lock(s3c_mfc_mutex);

		out = copy_from_user(&args.enc_exe, (s3c_mfc_enc_exe_arg_t *)arg, sizeof(s3c_mfc_enc_exe_arg_t));

		tmp = (pMfcInst->width * pMfcInst->height * 3) >> 1;
		dmac_clean_range(pMfcInst->yuv_buffer, pMfcInst->yuv_buffer + tmp);
		outer_clean_range(__pa(pMfcInst->yuv_buffer), __pa(pMfcInst->yuv_buffer + tmp));

		/* 
		 * Decode MFC Instance
		 */
		ret = s3c_mfc_instance_encode(pMfcInst, &nStrmLen, &nHdrLen);

		dmac_clean_range(pMfcInst->stream_buffer, pMfcInst->stream_buffer + S3C_MFC_LINE_BUF_SIZE_PER_INSTANCE);
		outer_clean_range(__pa(pMfcInst->stream_buffer), 		\
						__pa(pMfcInst->stream_buffer + S3C_MFC_LINE_BUF_SIZE_PER_INSTANCE));

		args.enc_exe.ret_code	= ret;
		if (ret == S3C_MFC_INST_RET_OK) {
			args.enc_exe.out_encoded_size = nStrmLen;
			args.enc_exe.out_header_size  = nHdrLen;
		}
		out = copy_to_user((s3c_mfc_enc_exe_arg_t *)arg, &args.enc_exe, sizeof(s3c_mfc_enc_exe_arg_t));

		mutex_unlock(s3c_mfc_mutex);
		break;

	case S3C_MFC_IOCTL_MFC_MPEG4_DEC_INIT:
	case S3C_MFC_IOCTL_MFC_H264_DEC_INIT:
	case S3C_MFC_IOCTL_MFC_H263_DEC_INIT:
	case S3C_MFC_IOCTL_MFC_VC1_DEC_INIT:
		mutex_lock(s3c_mfc_mutex);

		out = copy_from_user(&args.dec_init, (s3c_mfc_dec_init_arg_t *)arg, sizeof(s3c_mfc_dec_init_arg_t));

		if ( cmd == S3C_MFC_IOCTL_MFC_MPEG4_DEC_INIT )
			codec_mode = MP4_DEC;
		else if ( cmd == S3C_MFC_IOCTL_MFC_H264_DEC_INIT )
			codec_mode = AVC_DEC;
		else if ( cmd == S3C_MFC_IOCTL_MFC_H263_DEC_INIT)
			codec_mode = H263_DEC;
		else {
			codec_mode = VC1_DEC;
		}

		/* 
		 * Initialize MFC Instance
		 */
		ret = s3c_mfc_instance_dec_init(pMfcInst, codec_mode, args.dec_init.in_strmSize);

		args.dec_init.ret_code	= ret;
		if (ret == S3C_MFC_INST_RET_OK) {
			args.dec_init.out_width	     = pMfcInst->width;
			args.dec_init.out_height     = pMfcInst->height;
			args.dec_init.out_buf_width  = pMfcInst->buf_width;
			args.dec_init.out_buf_height = pMfcInst->buf_height;
		}
		out = copy_to_user((s3c_mfc_dec_init_arg_t *)arg, &args.dec_init, sizeof(s3c_mfc_dec_init_arg_t));

		mutex_unlock(s3c_mfc_mutex);
		break;

	case S3C_MFC_IOCTL_MFC_MPEG4_DEC_EXE:
	case S3C_MFC_IOCTL_MFC_H264_DEC_EXE:
	case S3C_MFC_IOCTL_MFC_H263_DEC_EXE:
	case S3C_MFC_IOCTL_MFC_VC1_DEC_EXE:
		mutex_lock(s3c_mfc_mutex);

		out = copy_from_user(&args.dec_exe, (s3c_mfc_dec_exe_arg_t *)arg, sizeof(s3c_mfc_dec_exe_arg_t));

		dmac_clean_range(pMfcInst->stream_buffer, pMfcInst->stream_buffer + S3C_MFC_LINE_BUF_SIZE_PER_INSTANCE);
		outer_clean_range(__pa(pMfcInst->stream_buffer), 			\
						__pa(pMfcInst->stream_buffer + S3C_MFC_LINE_BUF_SIZE_PER_INSTANCE));

		ret = s3c_mfc_instance_decode(pMfcInst, args.dec_exe.in_strmSize);

		args.dec_exe.ret_code = ret;
		out = copy_to_user((s3c_mfc_dec_exe_arg_t *)arg, &args.dec_exe, sizeof(s3c_mfc_dec_exe_arg_t));

		mutex_unlock(s3c_mfc_mutex);
		break;

	case S3C_MFC_IOCTL_MFC_GET_LINE_BUF_ADDR:
		mutex_lock(s3c_mfc_mutex);

		out = copy_from_user(&args.get_buf_addr, (s3c_mfc_get_buf_addr_arg_t *)arg, sizeof(s3c_mfc_get_buf_addr_arg_t));

		ret = s3c_mfc_instance_get_line_buffer(pMfcInst, &OutBuf, &buf_size);

		args.get_buf_addr.out_buf_size	= buf_size;
		args.get_buf_addr.out_buf_addr	= args.get_buf_addr.in_usr_data + (OutBuf - s3c_mfc_get_databuf_virt_addr());
		args.get_buf_addr.ret_code	= ret;

		out = copy_to_user((s3c_mfc_get_buf_addr_arg_t *)arg, &args.get_buf_addr, sizeof(s3c_mfc_get_buf_addr_arg_t));

		mutex_unlock(s3c_mfc_mutex);
		break;

	case S3C_MFC_IOCTL_MFC_GET_YUV_BUF_ADDR:
		mutex_lock(s3c_mfc_mutex);

		out = copy_from_user(&args.get_buf_addr, (s3c_mfc_get_buf_addr_arg_t *)arg, sizeof(s3c_mfc_get_buf_addr_arg_t));

		if (pMfcInst->yuv_buffer == NULL) {
			printk(KERN_ERR "\n%s: mfc frame buffer is not internally allocated yet\n", __FUNCTION__);
			mutex_unlock(s3c_mfc_mutex);
			return -EFAULT;
		}

		/* FRAM_BUF address is calculated differently for Encoder and Decoder. */
		switch (pMfcInst->codec_mode) {
		case MP4_DEC:
		case AVC_DEC:
		case VC1_DEC:
		case H263_DEC:
			/* Decoder case */
			args.get_buf_addr.out_buf_size = (pMfcInst->buf_width * pMfcInst->buf_height * 3) >> 1;
			tmp = (unsigned int)args.get_buf_addr.in_usr_data + (((unsigned int) pMfcInst->yuv_buffer)	\
						+ (pMfcInst->run_index) * (args.get_buf_addr.out_buf_size) -		\
						(unsigned int)s3c_mfc_get_databuf_virt_addr());
#if (S3C_MFC_ROTATE_ENABLE == 1)
			if ((pMfcInst->codec_mode != VC1_DEC) && (pMfcInst->post_rotation_mode & 0x0010)) {
				tmp = (unsigned int)args.get_buf_addr.in_usr_data + (((unsigned int)pMfcInst->yuv_buffer)  \
				+ (pMfcInst->yuv_buffer_count) * (args.get_buf_addr.out_buf_size) - 			   \
				(unsigned int)s3c_mfc_get_databuf_virt_addr());	
			}
#endif
			args.get_buf_addr.out_buf_addr = tmp;
			break;

		case MP4_ENC:
		case AVC_ENC:
		case H263_ENC:
			/* Encoder case */
			tmp = (pMfcInst->width * pMfcInst->height * 3) >> 1;
			args.get_buf_addr.out_buf_addr = args.get_buf_addr.in_usr_data + (pMfcInst->run_index * tmp) + 	\
								(int)(pMfcInst->yuv_buffer - s3c_mfc_get_databuf_virt_addr());
			break;
		}

		args.get_buf_addr.ret_code = S3C_MFC_INST_RET_OK;
		out = copy_to_user((s3c_mfc_get_buf_addr_arg_t *)arg, &args.get_buf_addr, sizeof(s3c_mfc_get_buf_addr_arg_t));

		mutex_unlock(s3c_mfc_mutex);
		break;

	case S3C_MFC_IOCTL_MFC_GET_PHY_FRAM_BUF_ADDR:
		mutex_lock(s3c_mfc_mutex);

		out = copy_from_user(&args.get_buf_addr, (s3c_mfc_get_buf_addr_arg_t *)arg, sizeof(s3c_mfc_get_buf_addr_arg_t));

		args.get_buf_addr.out_buf_size = (pMfcInst->buf_width * pMfcInst->buf_height * 3) >> 1;
		tmp = (unsigned int)S3C_MFC_BASEADDR_DATA_BUF + (((unsigned int)pMfcInst->yuv_buffer)		\
				+ (pMfcInst->run_index) * (args.get_buf_addr.out_buf_size) - 			\
				(unsigned int)s3c_mfc_get_databuf_virt_addr());

#if (S3C_MFC_ROTATE_ENABLE == 1)
		if ((pMfcInst->codec_mode != VC1_DEC) && (pMfcInst->post_rotation_mode & 0x0010)) {
				tmp = (unsigned int)S3C_MFC_BASEADDR_DATA_BUF + (((unsigned int) pMfcInst->yuv_buffer)   \
				+ (pMfcInst->yuv_buffer_count) * (args.get_buf_addr.out_buf_size) - 			 \
				(unsigned int)s3c_mfc_get_databuf_virt_addr());
		}
#endif

		args.get_buf_addr.out_buf_addr = tmp;
		args.get_buf_addr.ret_code = S3C_MFC_INST_RET_OK;

		out = copy_to_user((s3c_mfc_get_buf_addr_arg_t *)arg, &args.get_buf_addr, sizeof(s3c_mfc_get_buf_addr_arg_t));

		mutex_unlock(s3c_mfc_mutex);
		break;

	case S3C_MFC_IOCTL_MFC_GET_MPEG4_ASP_PARAM:
#if (defined(DIVX_ENABLE) && (DIVX_ENABLE == 1))

		out = copy_from_user(&args.mpeg4_asp_param, (s3c_mfc_get_mpeg4asp_arg_t *)arg, 			\
										sizeof(s3c_mfc_get_mpeg4asp_arg_t));

		ret = S3C_MFC_INST_RET_OK;
		args.mpeg4_asp_param.ret_code              = S3C_MFC_INST_RET_OK;
		args.mpeg4_asp_param.mp4asp_vop_time_res   = pMfcInst->RET_DEC_SEQ_INIT_BAK_MP4ASP_VOP_TIME_RES;
		args.mpeg4_asp_param.byte_consumed         = pMfcInst->RET_DEC_PIC_RUN_BAK_BYTE_CONSUMED;
		args.mpeg4_asp_param.mp4asp_fcode          = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_FCODE;
		args.mpeg4_asp_param.mp4asp_time_base_last = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_TIME_BASE_LAST;
		args.mpeg4_asp_param.mp4asp_nonb_time_last = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_NONB_TIME_LAST;
		args.mpeg4_asp_param.mp4asp_trd            = pMfcInst->RET_DEC_PIC_RUN_BAK_MP4ASP_MP4ASP_TRD;

		args.mpeg4_asp_param.mv_addr = (args.mpeg4_asp_param.in_usr_mapped_addr + S3C_MFC_STREAM_BUF_SIZE) 	\
								+ (pMfcInst->mv_mbyte_addr - pMfcInst->phys_addr_yuv_buffer);
		args.mpeg4_asp_param.mb_type_addr = args.mpeg4_asp_param.mv_addr + 25920;	
		args.mpeg4_asp_param.mv_size      = 25920; /* '25920' is the maximum MV size (=45*36*16) */
		args.mpeg4_asp_param.mb_type_size = 1620;  /* '1620' is the maximum MBTYE size (=45*36*1) */

		vir_mv_addr = (unsigned int)((pMfcInst->stream_buffer + S3C_MFC_STREAM_BUF_SIZE) + 			\
								(pMfcInst->mv_mbyte_addr - pMfcInst->phys_addr_yuv_buffer));
		vir_mb_type_addr = vir_mv_addr + 25920;

		out = copy_to_user((s3c_mfc_get_mpeg4asp_arg_t *)arg, &args.mpeg4_asp_param, 				\
										sizeof(s3c_mfc_get_mpeg4asp_arg_t));

		dmac_clean_range((unsigned char *)vir_mv_addr, (unsigned char *)(vir_mv_addr + args.mpeg4_asp_param.mv_size));
		outer_clean_range(__pa(vir_mv_addr), __pa(vir_mv_addr + args.mpeg4_asp_param.mv_size));

		dmac_clean_range((unsigned char *)vir_mb_type_addr, 	\
						(unsigned char *)(vir_mb_type_addr + args.mpeg4_asp_param.mb_type_size));
		outer_clean_range(__pa(vir_mb_type_addr), __pa(vir_mb_type_addr + args.mpeg4_asp_param.mb_type_size));
#endif	
		break;

	case S3C_MFC_IOCTL_MFC_GET_CONFIG:
		mutex_lock(s3c_mfc_mutex);

		out = copy_from_user(&args, (s3c_mfc_args_t *)arg, sizeof(s3c_mfc_args_t));

		ret = s3c_mfc_get_config_params(pMfcInst, &args);

		out = copy_to_user((s3c_mfc_args_t *)arg, &args, sizeof(s3c_mfc_args_t));

		mutex_unlock(s3c_mfc_mutex);
		break;

	case S3C_MFC_IOCTL_MFC_SET_CONFIG:
		mutex_lock(s3c_mfc_mutex);

		out = copy_from_user(&args, (s3c_mfc_args_t *)arg, sizeof(s3c_mfc_args_t));

		ret = s3c_mfc_set_config_params(pMfcInst, &args);

		out = copy_to_user((s3c_mfc_args_t *)arg, &args, sizeof(s3c_mfc_args_t));

		mutex_unlock(s3c_mfc_mutex);
		break;

	case S3C_MFC_IOCTL_VIRT_TO_PHYS:
		temp = __virt_to_phys((void *)arg);
		return (int)temp;
		break;

	default:
		mutex_lock(s3c_mfc_mutex);
		printk(KERN_DEBUG "\n%s: requested ioctl command is not defined (ioctl cmd = 0x%x)\n", __FUNCTION__, cmd);
		mutex_unlock(s3c_mfc_mutex);
		return -ENOIOCTLCMD;
	}

	switch (ret) {
	case S3C_MFC_INST_RET_OK:
		return 0;
	default:
		return -EPERM;
	}
	return -EPERM;
}

int s3c_mfc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long size	= vma->vm_end - vma->vm_start;
	unsigned long maxSize;
	unsigned long pageFrameNo;

	pageFrameNo = __phys_to_pfn(S3C_MFC_BASEADDR_DATA_BUF);

	maxSize = S3C_MFC_DATA_BUF_SIZE + PAGE_SIZE - (S3C_MFC_DATA_BUF_SIZE % PAGE_SIZE);

	if (size > maxSize) {
		return -EINVAL;
	}

	vma->vm_flags |= VM_RESERVED | VM_IO;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (remap_pfn_range(vma, vma->vm_start, pageFrameNo, size, vma->vm_page_prot)) {
		printk(KERN_ERR "\n%s: fail to remap\n", __FUNCTION__);
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
	int	size;
	int	ret;
	struct resource *res;
	unsigned int	mfc_clk;

	/* mfc clock enable  */
	s3c_mfc_hclk = clk_get(NULL, "hclk_mfc");
	if (!s3c_mfc_hclk) {
		printk(KERN_ERR "\n%s: failed to get mfc hclk source\n", __FUNCTION__);
		return -ENOENT;
	}
	clk_enable(s3c_mfc_hclk);

	s3c_mfc_sclk = clk_get(NULL, "sclk_mfc");
	if (!s3c_mfc_sclk) {
		printk(KERN_ERR "\n%s: failed to get mfc sclk source\n", __FUNCTION__);
		return -ENOENT;
	}
	clk_enable(s3c_mfc_sclk);

	s3c_mfc_pclk = clk_get(NULL, "pclk_mfc");
	if (!s3c_mfc_pclk) {
		printk(KERN_ERR "\n%s: failed to get mfc pclk source\n", __FUNCTION__);
		return -ENOENT;
	}
	clk_enable(s3c_mfc_pclk);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		printk(KERN_ERR "\n%s: failed to get memory region resouce\n", __FUNCTION__);
		return -ENOENT;
	}

	size = (res->end-res->start)+1;
	s3c_mfc_mem = request_mem_region(res->start, size, pdev->name);
	if (s3c_mfc_mem == NULL) {
		printk(KERN_ERR "\n%s: failed to get memory region\n", __FUNCTION__);
		return -ENOENT;
	}

	s3c_mfc_sfr_base_virt_addr = ioremap_nocache(res->start, size);
	if (s3c_mfc_sfr_base_virt_addr == 0) {
		printk(KERN_ERR "\n%s: failed to ioremap() region\n", __FUNCTION__);
		return -EINVAL;
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		printk(KERN_ERR "\n%s: failed to get irq resource\n", __FUNCTION__);
		return -ENOENT;
	}

	ret = request_irq(res->start, s3c_mfc_irq, IRQF_DISABLED, pdev->name, pdev);
	if (ret != 0) {
		printk(KERN_ERR "\n%s: failed to install irq (%d)\n", __FUNCTION__, ret);
		return ret;
	}

	s3c_mfc_phys_buffer = s3c_get_media_memory(S3C_MDEV_MFC);
	//s3c_mfc_virt_buffer = ioremap_nocache(s3c_mfc_phys_buffer, s3c_get_media_memsize(S3C_MDEV_MFC))

	/* mutex creation and initialization */
	s3c_mfc_mutex = (struct mutex *)kmalloc(sizeof(struct mutex), GFP_KERNEL);
	if (s3c_mfc_mutex == NULL)
		return -ENOMEM;

	mutex_init(s3c_mfc_mutex);

	/* mfc clock set 133 Mhz */
	mfc_clk = readl(S3C_CLK_DIV0);
	mfc_clk |= (1 << 28);
	__raw_writel(mfc_clk, S3C_CLK_DIV0);

	/*
	 * 2. MFC Memory Setup
	 */
	if (s3c_mfc_memory_setup() == FALSE)
		return -ENOMEM;

	/*
	 * 3. MFC Hardware Initialization
	 */
	if (s3c_mfc_hw_init() == FALSE)
		return -ENODEV;

	ret = misc_register(&s3c_mfc_miscdev);

	clk_disable(s3c_mfc_hclk);
	clk_disable(s3c_mfc_sclk);
	clk_disable(s3c_mfc_pclk);

	return 0;
}

static int s3c_mfc_remove(struct platform_device *dev)
{
	if (s3c_mfc_mem != NULL) {
		release_resource(s3c_mfc_mem);
		kfree(s3c_mfc_mem);
		s3c_mfc_mem = NULL;
	}

	free_irq(IRQ_MFC, dev);

	misc_deregister(&s3c_mfc_miscdev);
	return 0;
}

static int s3c_mfc_suspend(struct platform_device *dev, pm_message_t state)
{

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,21)
	int	inst_no;
	int	is_mfc_on = 0;
	int	i, index = 0;

	s3c_mfc_instance_context_t *mfcinst_ctx;
	unsigned int	dwMfcBase;

	mutex_lock(s3c_mfc_mutex);

	is_mfc_on = 0;

	/* 
	 * 1. Power Off state 
	 * Invalidate all the MFC Instances
	 */
	for (inst_no = 0; inst_no < S3C_MFC_NUM_INSTANCES_MAX; inst_no++) {
		mfcinst_ctx = s3c_mfc_instance_get_context(inst_no);
		if (mfcinst_ctx) {
			is_mfc_on = 1;

			/* 
			 * On Power Down, the MFC instance is invalidated.
			 * Then the MFC operations (DEC_EXE, ENC_EXE, etc.) will not be performed 
			 * until it is validated by entering Power up state transition
			 */
			s3c_mfc_instance_power_off_state(mfcinst_ctx);
			printk(KERN_ERR "\n%s: mfc suspend %d-th instance is invalidated\n", __FUNCTION__, inst_no);
		}
	}

	/* 2. Command MFC sleep and save MFC SFR */
	if (is_mfc_on) {
		dwMfcBase = s3c_mfc_sfr_base_virt_addr;

		for (i = S3C_MFC_SAVE_START_ADDR; i <= S3C_MFC_SAVE_END_ADDR; i += 4) {
			s3c_mfc_save[index] = readl(dwMfcBase + i);
			index++;	
		}

		s3c_mfc_sleep();
	}


	/* 3. Disable MFC clock */
	clk_disable(s3c_mfc_hclk);
	clk_disable(s3c_mfc_sclk);
	clk_disable(s3c_mfc_pclk);

	mutex_unlock(s3c_mfc_mutex);

#endif

	return 0;
}

static int s3c_mfc_resume(struct platform_device *pdev)
{

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,21)

	int 		i, index = 0;
	int         	inst_no;
	int		is_mfc_on = 0;
	unsigned int	mfc_pwr, dwMfcBase;
	unsigned int	domain_v_ready;
	s3c_mfc_instance_context_t 		*mfcinst_ctx;

	mutex_lock(s3c_mfc_mutex);

	clk_enable(s3c_mfc_hclk);
	clk_enable(s3c_mfc_sclk);
	clk_enable(s3c_mfc_pclk);

	/* 1. MFC Power On(Domain V) */
	mfc_pwr = readl(S3C_NORMAL_CFG);
	mfc_pwr |= (1 << 9);
	__raw_writel(mfc_pwr, S3C_NORMAL_CFG);

	/* 2. Check MFC power on */
	do {
		domain_v_ready = readl(S3C_BLK_PWR_STAT);
		printk(KERN_DEBUG "\n%s: domain v ready = 0x%X\n", __FUNCTION__, domain_v_ready);
		msleep(1);
	} while (!(domain_v_ready & (1 << 1)));

	/* 3. Firmware download */
	s3c_mfc_firmware_into_code_down_reg();

	/* 
	 * 4. Power On state
	 * Validate all the MFC Instances
	 */
	for (inst_no = 0; inst_no < S3C_MFC_NUM_INSTANCES_MAX; inst_no++) {
		mfcinst_ctx = s3c_mfc_instance_get_context(inst_no);
		if (mfcinst_ctx) {
			is_mfc_on = 1;

			/* 
			 * When MFC Power On, the MFC instance is validated.
			 * Then the MFC operations (DEC_EXE, ENC_EXE, etc.) will be performed again
			 */
			s3c_mfc_instance_power_on_state(mfcinst_ctx);
			printk(KERN_DEBUG "\n%s: mfc resume %d-th instance is validated\n", __FUNCTION__, inst_no);
		}
	}


	if (is_mfc_on) {
		/* 5. Restore MFC SFR */
		dwMfcBase = s3c_mfc_sfr_base_virt_addr;
		for (i = S3C_MFC_SAVE_START_ADDR; i <= S3C_MFC_SAVE_END_ADDR; i += 4 ) {
			writel(s3c_mfc_save[index], dwMfcBase + i);
			index++;
		}

		/* 6. Command MFC wakeup */
		s3c_mfc_wakeup();
	}

	mutex_unlock(s3c_mfc_mutex);

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
	pd_register_dev(&s3c_mfc_pmdev, "domain_v");
	printk(KERN_INFO "\n%s: mfc devid = %d\n", __FUNCTION__, s3c_mfc_pmdev.devid);
#endif

	if (platform_driver_register(&s3c_mfc_driver) != 0) {
		printk(KERN_ERR "\n%s: fail to register platform device\n", __FUNCTION__);
		return -EPERM;
	}

	return 0;
}

static void __exit s3c_mfc_exit(void)
{
	mutex_destroy(s3c_mfc_mutex);

#ifdef CONFIG_S3C6400_PDFW
	pd_unregister_dev(&s3c_mfc_pmdev);
#endif

	platform_driver_unregister(&s3c_mfc_driver);
	printk(KERN_DEBUG "\n%s: S3C6400 MFC driver exit\n", __FUNCTION__);	
}


module_init(s3c_mfc_init);
module_exit(s3c_mfc_exit);

MODULE_AUTHOR("PyoungJae, Jung");
MODULE_DESCRIPTION("S3C MFC (Multi Function Codec - FIMV) Device Driver");
MODULE_LICENSE("GPL");

