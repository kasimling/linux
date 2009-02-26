#ifndef __SAMSUNG_SYSLSI_APDEV_DATA_BUF_H__
#define __SAMSUNG_SYSLSI_APDEV_DATA_BUF_H__

#include "MfcTypes.h"


BOOL MfcDataBufMemMapping(void);

volatile unsigned char *GetDataBufVirAddr(void);
volatile unsigned char *GetFramBufVirAddr(void);
unsigned int GetDataBufPhyAddr(void);
unsigned int GetFramBufPhyAddr(void);


#endif /* __SAMSUNG_SYSLSI_APDEV_DATA_BUF_H__ */
