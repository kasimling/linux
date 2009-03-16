/* 
 * drivers/media/video/samsung/mfc40/s3c_mfc_logmsg.h
 *
 * Header file for Samsung MFC (Multi Function Codec - FIMV) driver
 *
 * PyoungJae Jung, Jiun Yu, Copyright (c) 2009 Samsung Electronics
 * http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

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
