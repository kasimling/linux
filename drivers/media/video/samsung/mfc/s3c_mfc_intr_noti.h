#ifndef _S3C_MFC_INTR_NOTI_H
#define _S3C_MFC_INTR_NOTI_H


#define S3C_MFC_INTR_NOTI_TIMEOUT    1000

/*
 * MFC Interrupt Enable Macro Definition
 */
#define S3C_MFC_INTR_ENABLE_ALL    0xCCFF
#define S3C_MFC_INTR_ENABLE_RESET  0xC00E

/*
 * MFC Interrupt Reason Macro Definition
 */
#define S3C_MFC_INTR_REASON_NULL		0x0000
#define S3C_MFC_INTR_REASON_BUFFER_EMPTY        0xC000
#define S3C_MFC_INTR_REASON_INTRNOTI_TIMEOUT    (-99)


#endif /* _S3C_MFC_INTR_NOTI_H */
