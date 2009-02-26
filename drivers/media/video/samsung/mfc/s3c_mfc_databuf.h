#ifndef _S3C_MFC_DATABUF_H
#define _S3C_MFC_DATABUF_H

#include "s3c_mfc_types.h"

BOOL s3c_mfc_databuf_memmapping(void);

volatile unsigned char *s3c_mfc_get_databuf_virt_addr(void);
volatile unsigned char *s3c_mfc_get_yuv_buff_virt_addr(void);
unsigned int s3c_mfc_get_databuf_phys_addr(void);
unsigned int s3c_mfc_get_yuv_buff_phys_addr(void);

#endif /* _S3C_MFC_DATABUF_H */
