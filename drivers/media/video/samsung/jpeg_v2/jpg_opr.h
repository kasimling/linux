/*
 * Project Name JPEG DRIVER IN Linux
 * Copyright  2007 Samsung Electronics Co, Ltd. All Rights Reserved.
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics  ("Confidential Information").
 * you shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics
 *
 * This file implements JPEG driver.
 *
 * @name JPEG DRIVER MODULE Module (JPGOpr.h)
 * @author Jiun Yu (jiun.yu@samsung.com)
 * @date 05-07-07
 * @author modify kwak Hyun Min (hyunmin.kwak@samsung.com) 
 * date 12-03-20 for linux 2.6.28 
 */

#ifndef __JPG_OPR_H__
#define __JPG_OPR_H__

#include <linux/interrupt.h>

extern wait_queue_head_t wait_queue_jpeg;

typedef enum {
	JPG_FAIL,
	JPG_SUCCESS,
	OK_HD_PARSING,
	ERR_HD_PARSING,
	OK_ENC_OR_DEC,
	ERR_ENC_OR_DEC,
	ERR_UNKNOWN
} jpg_return_status;

typedef enum {
	JPG_RGB16,
	JPG_YCBYCR,
	JPG_TYPE_UNKNOWN
} image_type_t;

typedef enum {
	JPG_444,
	JPG_422,
	JPG_420,
	JPG_400,
	RESERVED1,
	RESERVED2,
	JPG_411,
	JPG_SAMPLE_UNKNOWN
} sample_mode_t;

typedef enum {
	YCBCR_422,
	YCBCR_420,
	YCBCR_SAMPLE_UNKNOWN
} out_mode_t;

typedef enum {
	JPG_MODESEL_YCBCR = 1,
	JPG_MODESEL_RGB,
	JPG_MODESEL_UNKNOWN
} in_mode_t;

typedef enum {
	JPG_MAIN,
	JPG_THUMBNAIL
} encode_type_t;

typedef enum {
	JPG_QUALITY_LEVEL_1 = 0, /*high quality*/
	JPG_QUALITY_LEVEL_2,
	JPG_QUALITY_LEVEL_3,
	JPG_QUALITY_LEVEL_4     /*low quality*/
} image_quality_type_t;

typedef struct {
	sample_mode_t		sample_mode;
	encode_type_t		dec_type;
	out_mode_t       	out_format;
	UINT32			width;
	UINT32			height;
	UINT32			data_size;
	UINT32			file_size;
} jpg_dec_proc_param;

typedef struct {
	sample_mode_t		sample_mode;
	encode_type_t		enc_type;
	in_mode_t      		in_format;
	image_quality_type_t 	quality;
	UINT32			width;
	UINT32			height;
	UINT32			data_size;
	UINT32			file_size;
} jpg_enc_proc_param;

typedef struct {
	char			*in_buf;
	char			*phy_in_buf;
	int			in_buf_size;
	char			*out_buf;
	char    		*phy_out_buf;
	int			out_buf_size;
	char			*in_thumb_buf;
	char			*phy_in_thumb_buf;
	int			in_thumb_buf_size;
	char			*out_thumb_buf;
	char			*phy_out_thumb_buf;
	int			out_thumb_buf_size;
	char			*mapped_addr;
	jpg_dec_proc_param	*dec_param;
	jpg_enc_proc_param	*enc_param;
	jpg_enc_proc_param	*thumb_enc_param;
} jpg_args;


jpg_return_status decode_jpg(sspc100_jpg_ctx *jpg_ctx, jpg_dec_proc_param *dec_param);
void reset_jpg(sspc100_jpg_ctx *jpg_ctx);
void decode_header(sspc100_jpg_ctx *jpg_ctx, jpg_dec_proc_param *dec_param);
void decode_body(sspc100_jpg_ctx *jpg_ctx);
sample_mode_t get_sample_type(sspc100_jpg_ctx *jpg_ctx);
void get_xy(sspc100_jpg_ctx *jpg_ctx, UINT32 *x, UINT32 *y);
UINT32 get_yuv_size(out_mode_t out_format, UINT32 width, UINT32 height);
jpg_return_status encode_jpg(sspc100_jpg_ctx *jpg_ctx, jpg_enc_proc_param    *enc_param);
jpg_return_status wait_for_interrupt(void);

#endif
