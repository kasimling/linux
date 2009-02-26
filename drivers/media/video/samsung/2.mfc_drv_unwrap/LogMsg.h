#ifndef __SAMSUNG_SYSLSI_APDEV_LOG_MSG_H__
#define __SAMSUNG_SYSLSI_APDEV_LOG_MSG_H__


typedef enum
{
	KERN_DEBUG   = 0,
	LOG_WARNING = 1,
	KERN_ERR   = 2
} LOG_LEVEL;


#ifdef __cplusplus
extern "C" {
#endif


void printk(LOG_LEVEL level, const char *func_name, const char *msg, ...);


#ifndef _WIN32_WCE
void Sleep(unsigned int ms);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SAMSUNG_SYSLSI_APDEV_LOG_MSG_H__ */

