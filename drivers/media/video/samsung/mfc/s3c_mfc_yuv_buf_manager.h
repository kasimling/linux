#ifndef _S3C_MFC_YUV_BUF_MANAGER_H
#define _S3C_MFC_YUV_BUF_MANAGER_H


#include "s3c_mfc_types.h"


BOOL            s3c_mfc_yuv_buf_mgr_init(unsigned char *pBufBase, int nBufSize);
void            s3c_mfc_yuv_buffer_mgr_final(void);

unsigned char  *s3c_mfc_yuv_buffer_mgr_commit(int idx_commit, int commit_size);
void            s3c_yuv_buffer_mgr_free(int idx_commit);

unsigned char  *s3c_mfc_yuv_buffer_mgr_get_buffer(int idx_commit);
int             s3c_mfc_yuv_buffer_mgr_get_buffer_size(int idx_commit);

void            s3c_mfc_yuv_buffer_mgr_print_commit_info(void);


#endif /* _S3C_MFC_YUV_BUF_MANAGER_H */
