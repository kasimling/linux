#ifndef _S3C_MFC_BITPROC_BUF_H
#define _S3C_MFC_BITPROC_BUF_H

#include "s3c_mfc_types.h"

BOOL s3c_mfc_bitproc_buff_mem_mapping(void);
volatile unsigned char *s3c_mfc_get_bitproc_buff_virt_addr(void);
unsigned char *s3c_mfc_get_param_buff_virt_addr(void);

void s3c_mfc_firmware_into_codebuff(void);

#endif /* _S3C_MFC_BITPROC_BUF_H */
