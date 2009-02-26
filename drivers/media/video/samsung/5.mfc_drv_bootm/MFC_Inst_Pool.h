#ifndef __SAMSUNG_SYSLSI_APDEV_MFC_INST_POOL_H__
#define __SAMSUNG_SYSLSI_APDEV_MFC_INST_POOL_H__


int MfcInstPool_NumAvail(void);

int MfcInstPool_Occupy(void);
int MfcInstPool_Release(int instance_no);

void MfcInstPool_OccupyAll(void);
void MfcInstPool_ReleaseAll(void);

#endif /* __SAMSUNG_SYSLSI_APDEV_MFC_INST_POOL_H__ */
