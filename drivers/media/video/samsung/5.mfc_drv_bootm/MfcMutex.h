#ifndef __SAMSUNG_SYSLSI_APDEV_MFC_MUTEX_H__
#define __SAMSUNG_SYSLSI_APDEV_MFC_MUTEX_H__


#include "MfcTypes.h"


BOOL MFC_Mutex_Create(void);
void MFC_Mutex_Delete(void);
BOOL MFC_Mutex_Lock(void);
BOOL MFC_Mutex_Release(void);

#endif /* __SAMSUNG_SYSLSI_APDEV_MFC_MUTEX_H__ */
