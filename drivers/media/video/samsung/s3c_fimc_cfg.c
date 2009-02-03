/* linux/drivers/media/video/samsung/s3c_fimc_cfg.c
 *
 * Configuration support file for Samsung Camera Interface (FIMC) driver
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/slab.h>
#include <linux/bootmem.h>
#include <linux/string.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "s3c_fimc.h"

#ifdef CONFIG_VIDEO_SAMSUNG_STATIC_MEMORY
#define s3c_fimc_malloc(__size)		alloc_bootmem(__size)
#define s3c_fimc_free(__buff, __size)	free_bootmem((u32) __buff, (u32) __size)
#else
#define	s3c_fimc_malloc(__size)		kmalloc((size_t) __size, GFP_DMA)
#define s3c_fimc_free(__buff, __size)	kfree(__buff)
#endif

int s3c_fimc_alloc_output_memory(struct s3c_fimc_out_frame *info)
{
	struct s3c_fimc_frame_addr *frame;
	int i, ret, nr_frames = info->nr_frames;
	u32 size = info->width * info->height;
	u32 cbcr_size = 0, *buf_size = NULL, one_p_size;

	switch (info->format) {
	case FORMAT_RGB565:
		size *= 2;
		buf_size = &size;
		break;

	case FORMAT_RGB666:	/* fall through */
	case FORMAT_RGB888:
		size *= 4;
		buf_size = &size;
		break;

	case FORMAT_YCBCR420:
		cbcr_size = size / 4;
		one_p_size = size + (2 * cbcr_size);
		buf_size = &one_p_size;
		break;

	case FORMAT_YCBCR422:
		cbcr_size = size / 2;
		one_p_size = size + (2 * cbcr_size);
		buf_size = &one_p_size;
		break;
	}

	if (*buf_size % PAGE_SIZE != 0)
		*buf_size = (*buf_size / PAGE_SIZE + 1) * PAGE_SIZE;

	info->buf_size = *buf_size;

	switch (info->format) {
	case FORMAT_RGB565:	/* fall through */
	case FORMAT_RGB666:	/* fall through */
	case FORMAT_RGB888:
		for (i = 0; i < nr_frames; i++) {
			frame = &info->addr[i];

			frame->virt_rgb = s3c_fimc_malloc(info->buf_size);
			if (frame->virt_rgb == NULL) {
				err("cannot allocate memory\n");
				ret = -ENOMEM;
				goto alloc_fail;
			}

			frame->phys_rgb = virt_to_phys(frame->virt_rgb);
		}

		for (i = nr_frames; i < S3C_FIMC_MAX_FRAMES; i++) {
			frame = &info->addr[i];
			frame->virt_rgb = info->addr[i - nr_frames].virt_rgb;
			frame->phys_rgb = info->addr[i - nr_frames].phys_rgb;
		}

		break;

	case FORMAT_YCBCR420:	/* fall through */
	case FORMAT_YCBCR422:
		for (i = 0; i < nr_frames; i++) {
			frame = &info->addr[i];

			frame->virt_y = s3c_fimc_malloc(info->buf_size);
			if (frame->virt_y == NULL) {
				err("cannot allocate memory\n");
				ret = -ENOMEM;
				goto alloc_fail;
			}

			frame->virt_cb = frame->virt_y + size;
			frame->virt_cr = frame->virt_cb + cbcr_size;

			frame->phys_y = virt_to_phys(frame->virt_y);
			frame->phys_cb = frame->phys_y + size;
			frame->phys_cr = frame->phys_cb + cbcr_size;
		}

		for (i = nr_frames; i < S3C_FIMC_MAX_FRAMES; i++) {
			frame = &info->addr[i];
			frame->virt_y = info->addr[i - nr_frames].virt_y;
			frame->virt_cb = info->addr[i - nr_frames].virt_cb;
			frame->virt_cr = info->addr[i - nr_frames].virt_cr;
			frame->phys_y = info->addr[i - nr_frames].phys_y;
			frame->phys_cb = info->addr[i - nr_frames].phys_cb;
			frame->phys_cr = info->addr[i - nr_frames].phys_cr;
		}

		break;
	}

	return 0;

alloc_fail:
	for (i = 0; i < nr_frames; i++) {
		frame = &info->addr[i];

		if (!frame->virt_y) {
			s3c_fimc_free(frame->virt_y, info->buf_size);
			frame->virt_y = NULL;
		}
	}

	return ret;
}

void s3c_fimc_free_output_memory(struct s3c_fimc_out_frame *info)
{
	struct s3c_fimc_frame_addr *frame;
	int i;

	for (i = 0; i < info->nr_frames; i++) {
		frame = &info->addr[i];

		if (!frame->virt_y)
			s3c_fimc_free(frame->virt_y, info->buf_size);

		memset(frame, 0, sizeof(*frame));
	}
}

void s3c_fimc_set_nr_frames(struct s3c_fimc_control *ctrl, int nr)
{
	ctrl->out_frame.nr_frames = nr;
}

static int s3c_fimc_set_output_format(struct v4l2_pix_format *fmt,
					struct s3c_fimc_out_frame *frame)
{
	int depth = 0;

	switch (fmt->pixelformat) {
	case V4L2_PIX_FMT_RGB565:
		frame->format = FORMAT_RGB565;
		frame->planes = 1;
		depth = 16;
		break;

	case V4L2_PIX_FMT_RGB24:
		frame->format = FORMAT_RGB888;
		frame->planes = 1;
		depth = 24;
		break;

	case V4L2_PIX_FMT_NV12:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 2;
		frame->order_2p = LSB_CBCR;
		depth = 12;
		break;

	case V4L2_PIX_FMT_NV21:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 2;
		frame->order_2p = LSB_CRCB;
		depth = 12;
		break;

	case V4L2_PIX_FMT_NV12X:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 2;
		frame->order_2p = MSB_CBCR;
		depth = 12;
		break;

	case V4L2_PIX_FMT_NV21X:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 2;
		frame->order_2p = MSB_CRCB;
		depth = 12;
		break;

	case V4L2_PIX_FMT_YUV420:
		frame->format = FORMAT_YCBCR420;
		frame->planes = 3;
		depth = 12;
		break;

	case V4L2_PIX_FMT_YUYV:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 1;
		frame->order_1p = OUT_ORDER422_YCBYCR;
		depth = 16;
		break;

	case V4L2_PIX_FMT_YVYU:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 1;
		frame->order_1p = OUT_ORDER422_YCRYCB;
		depth = 16;
		break;

	case V4L2_PIX_FMT_UYVY:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 1;
		frame->order_1p = OUT_ORDER422_CBYCRY;
		depth = 16;
		break;

	case V4L2_PIX_FMT_VYUY:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 1;
		frame->order_1p = OUT_ORDER422_CRYCBY;
		depth = 16;
		break;

	case V4L2_PIX_FMT_UV12:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 2;
		frame->order_1p = LSB_CBCR;
		depth = 16;
		break;

	case V4L2_PIX_FMT_UV21:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 2;
		frame->order_1p = LSB_CRCB;
		depth = 16;
		break;

	case V4L2_PIX_FMT_UV12X:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 2;
		frame->order_1p = MSB_CBCR;
		depth = 16;
		break;

	case V4L2_PIX_FMT_UV21X:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 2;
		frame->order_1p = MSB_CRCB;
		depth = 16;
		break;

	case V4L2_PIX_FMT_YUV422P:
		frame->format = FORMAT_YCBCR422;
		frame->planes = 3;
		depth = 16;
		break;
	}

	return depth;
}

int s3c_fimc_set_output_frame(struct s3c_fimc_control *ctrl,
				struct v4l2_pix_format *fmt, int priv)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	int depth = 0;

	frame->width = fmt->width;
	frame->height = fmt->height;

	depth = s3c_fimc_set_output_format(fmt, frame);

	switch (fmt->field) {
	case V4L2_FIELD_INTERLACED:
	case V4L2_FIELD_INTERLACED_TB:
	case V4L2_FIELD_INTERLACED_BT:
		frame->scan = SCAN_TYPE_INTERLACE;
		break;

	default:
		frame->scan = SCAN_TYPE_PROGRESSIVE;
		break;
	}

	if (frame->addr[0].virt_y == NULL) {
		if (s3c_fimc_alloc_output_memory(frame))
			err("cannot allocate memory\n");
		else
			s3c_fimc_set_output_address(ctrl);
	}

	return depth;
}

int s3c_fimc_frame_handler(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	int ret;

	frame->skip_frames++;

	if ((ctrl->flag & S3C_FIMC_FLAG_IRQ_NORMAL) && \
		frame->skip_frames > frame->nr_frames * 2) {
		s3c_fimc_set_iflag(ctrl->flag, S3C_FIMC_FLAG_IRQ_X);
	}

	switch (ctrl->flag & S3C_FIMC_IRQ_MASK) {
	case S3C_FIMC_FLAG_IRQ_NORMAL:
		s3c_fimc_set_sflag(ctrl->flag, S3C_FIMC_FLAG_RUNNING);
		ret = S3C_FIMC_FRAME_SKIP;
		break;

	case S3C_FIMC_FLAG_IRQ_X:
		s3c_fimc_enable_lastirq(ctrl);
		s3c_fimc_disable_lastirq(ctrl);
		s3c_fimc_set_sflag(ctrl->flag, S3C_FIMC_FLAG_HANDLE_IRQ);
		s3c_fimc_set_iflag(ctrl->flag, S3C_FIMC_FLAG_IRQ_Y);
		ret = S3C_FIMC_FRAME_SKIP;
		break;

	case S3C_FIMC_FLAG_IRQ_Y:
		s3c_fimc_set_iflag(ctrl->flag, S3C_FIMC_FLAG_IRQ_LAST);
		s3c_fimc_disable_capture(ctrl);
		ret = S3C_FIMC_FRAME_SKIP;
		break;

	case S3C_FIMC_FLAG_IRQ_LAST:
		ret = S3C_FIMC_FRAME_TAKE;
		break;

	default:
		ret = S3C_FIMC_FRAME_SKIP;
		break;
	}

	return ret;
}

u8 *s3c_fimc_get_current_frame(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;

	return frame->addr[frame->cfn].virt_y;
}

static int s3c_fimc_get_scaler_factor(u32 src, u32 tar, u32 *ratio, u32 *shift)
{
	if (src >= tar * 64) {
		err("out of pre-scaler range\n");
		return -EINVAL;
	} else if (src >= tar * 32) {
		*ratio = 32;
		*shift = 5;
	} else if (src >= tar * 16) {
		*ratio = 16;
		*shift = 4;
	} else if (src >= tar * 8) {
		*ratio = 8;
		*shift = 3;
	} else if (src >= tar * 4) {
		*ratio = 4;
		*shift = 2;
	} else if (src >= tar * 2) {
		*ratio = 2;
		*shift = 1;
	} else {
		*ratio = 1;
		*shift = 0;
	}

	return 0;
}

int s3c_fimc_set_scaler_info(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_window_offset *ofs = &ctrl->in_cam->offset;
	struct s3c_fimc_scaler *sc = &ctrl->scaler;
	int ret, tx, ty, sx, sy;

	tx = ctrl->out_frame.width;
	ty = ctrl->out_frame.height;

	if (tx <= 0 || ty <= 0) {
		err("invalid target size\n");
		ret = -EINVAL;
		goto err_size;
	}

	sx = ctrl->in_cam->width - (ofs->h1 + ofs->h2);
	sy = ctrl->in_cam->height - (ofs->v1 + ofs->v2);

	sc->real_width = sx;
	sc->real_height = sy;

	if (sx <= 0 || sy <= 0) {
		err("invalid source size\n");
		ret = -EINVAL;
		goto err_size;
	}

	s3c_fimc_get_scaler_factor(sx, tx, &sc->pre_hratio, &sc->hfactor);
	s3c_fimc_get_scaler_factor(sy, ty, &sc->pre_vratio, &sc->vfactor);

	if (sx / sc->pre_hratio > ctrl->pdata->line_length)
		info("line buffer size overflow\n");

	sc->pre_dst_width = sx / sc->pre_hratio;
	sc->pre_dst_height = sy / sc->pre_vratio;

	sc->main_hratio = (sx << 8) / (tx << sc->hfactor);
	sc->main_vratio = (sy << 8) / (ty << sc->vfactor);

	sc->scaleup_h = (sx <= tx) ? 1 : 0;
	sc->scaleup_v = (sy <= ty) ? 1 : 0;

	s3c_fimc_set_prescaler(ctrl);
	s3c_fimc_set_scaler(ctrl);

	return 0;

err_size:
	return ret;
}

void s3c_fimc_start_dma(struct s3c_fimc_control *ctrl)
{
	s3c_fimc_set_source_format(ctrl);
	s3c_fimc_set_window_offset(ctrl);
	s3c_fimc_set_polarity(ctrl);
	s3c_fimc_set_scaler_info(ctrl);
	s3c_fimc_set_target_format(ctrl);
	s3c_fimc_set_output_dma(ctrl);
	s3c_fimc_enable_capture(ctrl);
	s3c_fimc_start_scaler(ctrl);
}

void s3c_fimc_stop_dma(struct s3c_fimc_control *ctrl)
{
	s3c_fimc_disable_capture(ctrl);
	s3c_fimc_stop_scaler(ctrl);
}

void s3c_fimc_restart_dma(struct s3c_fimc_control *ctrl)
{
	s3c_fimc_stop_dma(ctrl);
	s3c_fimc_start_dma(ctrl);
}

void s3c_fimc_change_resolution(struct s3c_fimc_control *ctrl,
					enum s3c_fimc_cam_res_t res)
{
	struct s3c_fimc_camera *cam = ctrl->in_cam;

	s3c_fimc_stop_scaler(ctrl);
	s3c_fimc_i2c_command(ctrl, I2C_CAM_RESOLUTION, res);

	switch (res) {
	case CAM_RES_QSVGA:
		info("resolution changed to QSVGA (400x300) mode\n");
		cam->width = 400;
		cam->height = 300;
		break;

	case CAM_RES_VGA:
		info("resolution changed to VGA (640x480) mode\n");
		cam->width = 640;
		cam->height = 480;
		break;

	case CAM_RES_SVGA:
		info("resolution changed to SVGA (800x600) mode\n");
		cam->width = 800;
		cam->height = 600;
		break;

	case CAM_RES_SXGA:
		info("resolution changed to SXGA (1280x1024) mode\n");
		cam->width = 1280;
		cam->height = 1024;
		break;

	case CAM_RES_UXGA:
		info("resolution changed to UXGA (1600x1200) mode\n");
		cam->width = 1600;
		cam->height = 1200;
		break;

	case CAM_RES_DEFAULT:	/* fall through */
	case CAM_RES_MAX:
		/* nothing to do */
		break;
	}
}

int s3c_fimc_check_zoom(struct s3c_fimc_control *ctrl, int type)
{
	struct s3c_fimc_scaler *sc = &ctrl->scaler;
	struct s3c_fimc_window_offset *offset = &ctrl->in_cam->offset;
	int sx = sc->real_width;
	int zoom_pixels = S3C_FIMC_ZOOM_PIXELS * 2;
	int zoom_size = sx - (offset->h1 + offset->h2 + zoom_pixels);
	
	switch (type) {
	case V4L2_CID_ZOOMIN:
		if (zoom_size / sc->pre_hratio > sc->line_length) {
			err("already reached to zoom-in boundary\n");
			return -EINVAL;
		}

		sc->zoom_depth++;
		break;

	case V4L2_CID_ZOOMOUT:
		if (sc->zoom_depth > 0)
			sc->zoom_depth--;
		else {
			err("already reached to zoom-out boundary\n");
			return -EINVAL;
		}

		break;
	}

	return 0;
}
