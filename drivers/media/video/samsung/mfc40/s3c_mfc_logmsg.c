/*
 * drivers/media/video/samsung/mfc40/s3c_mfc_logmsg.c
 *
 * C file for Samsung MFC (Multi Function Codec - FIMV) driver
 *
 * PyoungJae Jung, Jiun Yu, Copyright (c) 2009 Samsung Electronics
 * http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
#include <linux/string.h>
#include <linux/kernel.h>
#include <asm/param.h>
#include <linux/delay.h>

#include "s3c_mfc_logmsg.h"

static const LOG_LEVEL log_level = LOG_WARNING;

static const char *modulename = "MFC_DRV";

static const char *level_str[] = {"DEBUG", "TRACE", "WARNING", "ERROR"};

void LOG_MSG(LOG_LEVEL level, const char *func_name, const char *msg, ...)
{
	char buf[256];
	va_list argptr;

	//if (level < log_level)
	//	return;

	sprintf(buf, "[%s: %s] %s: ", modulename, level_str[level], func_name);

	va_start(argptr, msg);
	vsprintf(buf + strlen(buf), msg, argptr);
	printk(buf);
	va_end(argptr);
}
*/

