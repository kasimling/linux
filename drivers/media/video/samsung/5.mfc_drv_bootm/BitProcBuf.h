#ifndef __SAMSUNG_SYSLSI_APDEV_BIT_PROC_BUF_H__
#define __SAMSUNG_SYSLSI_APDEV_BIT_PROC_BUF_H__

#include "MfcTypes.h"

BOOL MfcBitProcBufMemMapping(void);
volatile unsigned char *GetBitProcBufVirAddr(void);
unsigned char *GetParamBufVirAddr(void);

void MfcFirmwareIntoCodeBuf(void);

#endif /* __SAMSUNG_SYSLSI_APDEV_BIT_PROC_BUF_H__ */
