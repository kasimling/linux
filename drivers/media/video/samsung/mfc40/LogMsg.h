#ifndef _S3c_MFC_LOG_MSG_H_
#define _S3c_MFC_LOG_MSG_H_

typedef enum
{
	LOG_DEBUG = 0,
	LOG_TRACE,
	LOG_WARNING,
	LOG_ERROR
} LOG_LEVEL;


void LOG_MSG(LOG_LEVEL level, const char *func_name, const char *msg, ...);


#endif /* _S3c_MFC_LOG_MSG_H_ */
