#ifndef __SAMSUNG_SYSLSI_APDEV_log_msg_H__
#define __SAMSUNG_SYSLSI_APDEV_log_msg_H__


typedef enum
{
	LOG_TRACE   = 0,
	LOG_WARNING = 1,
	LOG_ERROR   = 2
} LOG_LEVEL;


#ifdef __cplusplus
extern "C" {
#endif


void log_msg(LOG_LEVEL level, const char *func_name, const char *msg, ...);

#ifdef __cplusplus
}
#endif

#endif /* __SAMSUNG_SYSLSI_APDEV_log_msg_H__ */
