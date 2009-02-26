#ifndef __SAMSUNG_SYSLSI_APDEV_MFC_INTR_NOTIFICATION_H__
#define __SAMSUNG_SYSLSI_APDEV_MFC_INTR_NOTIFICATION_H__


#ifdef __cplusplus
extern "C" {
#endif


int  SendInterruptNotification(int intr_type);
int  WaitInterruptNotification(void);


#ifdef __cplusplus
}
#endif

#define MFC_INTR_NOTI_TIMEOUT    1000

/*
 * MFC Interrupt Enable Macro Definition
 */
#define MFC_INTR_ENABLE_ALL    0xCCFF
#define MFC_INTR_ENABLE_RESET    0xC00E

/*
 * MFC Interrupt Reason Macro Definition
 */
#define MFC_INTR_REASON_NULL            0x0000
#define MFC_INTR_REASON_BUFFER_EMPTY        0xC000
#define MFC_INTR_REASON_INTRNOTI_TIMEOUT    (-99)


#endif /* __SAMSUNG_SYSLSI_APDEV_MFC_INTR_NOTIFICATION_H__ */
