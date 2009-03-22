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

#include <stdarg.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <asm/param.h>
#include <linux/delay.h>

#include "log_msg.h"

//#define DEBUG 1

static const log_level g_log_level = LOG_TRACE;

static const char *module_name = "JPEG_DRV";

static const char *level_str[] = {"TRACE", "WARNING", "ERROR"};

void log_msg(log_level level, const char *func_name, const char *msg, ...)
{

	char buf[256];
	va_list argptr;

	if (level < g_log_level)
		return;

	sprintf(buf, "[%s: %s] %s: ", module_name, level_str[level], func_name);

	va_start(argptr, msg);
	vsprintf(buf + strlen(buf), msg, argptr);

	if (level == LOG_TRACE) {
#ifdef DEBUG
		printk(buf);
#endif
	} else {
		printk(buf);
	}

	va_end(argptr);
}

