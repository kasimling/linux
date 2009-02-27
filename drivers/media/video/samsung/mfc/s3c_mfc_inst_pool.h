/* linux/driver/media/video/mfc/s3c_mfc_inst_pool.h
 *
 * Header file for Samsung MFC (Multi Function Codec - FIMV) driver 
 *
 * PyoungJae Jung, Jiun Yu, Copyright (c) 2009 Samsung Electronics 
 * http://www.samsungsemi.com/ 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _S3C_MFC_INST_POOL_H
#define _S3C_MFC_INST_POOL_H

int s3c_mfc_inst_pool_num_avail(void);

int s3c_mfc_inst_pool_occupy(void);
int s3c_mfc_inst_pool_release(int instance_no);

void s3c_mfc_inst_pool_occypy_all(void);
void s3c_mfc_inst_pool_release_all(void);

#endif /* _S3C_MFC_INST_POOL_H */
