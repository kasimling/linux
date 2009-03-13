#ifndef _S3C_MFC_LOGMSG_H_
#define _S3C_MFC_LOGMSG_H_


typedef enum
{
	LOG_DEBUG   = 0,
	LOG_TRACE,
	LOG_WARNING,
	LOG_ERROR
} LOG_LEVEL;

void LOG_MSG(LOG_LEVEL level, const char *func_name, const char *msg, ...);

#endif /* _S3C_MFC_LOGMSG_H_ */
