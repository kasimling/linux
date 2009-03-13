/*
 * Project Name MFC DRIVER 
 * Copyright  2007 Samsung Electronics Co, Ltd. All Rights Reserved. 
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics  ("Confidential Information").   
 * you shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics 
 *
 * This source file is for printing the driver's log messages.
 *
 * @name MFC DRIVER MODULE Module (log_msg.c)
 * @author Jiun, Yu(jiun.yu@samsung.com)
 * @date 03-28-07
 */

#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#include <asm/param.h>

#include "LogMsg.h"

static const LOG_LEVEL log_level = LOG_WARNING;

static const char *modulename = "MFC_DRV";

static const char *level_str[] = {"DEBUG", "TRACE", "WARNING", "ERROR"};

void LOG_MSG(LOG_LEVEL level, const char *func_name, const char *msg, ...)
{
	char buf[256];
	va_list argptr;

	if (level < log_level)
		return;

	sprintf(buf, "[%s: %s] %s: ", modulename, level_str[level], func_name);

	va_start(argptr, msg);
	vsprintf(buf + strlen(buf), msg, argptr);
	printk(buf);
	va_end(argptr);
}

