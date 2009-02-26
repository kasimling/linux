#ifndef _S3C_MFC_INST_POOL_H
#define _S3C_MFC_INST_POOL_H

int s3c_mfc_inst_pool_num_avail(void);

int s3c_mfc_inst_pool_occupy(void);
int s3c_mfc_inst_pool_release(int instance_no);

void s3c_mfc_inst_pool_occypy_all(void);
void s3c_mfc_inst_pool_release_all(void);

#endif /* _S3C_MFC_INST_POOL_H */
