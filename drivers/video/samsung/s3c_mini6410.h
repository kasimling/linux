/*
 * drivers/video/s3c/s3cfb_mini6410.h
 *
 * LCD timing for FriendlyARM mini6410, original in s3c_mini6410.c
 *
 * based on s3cfb_lte480wv.c
 *
 * Copyright (C) 2008 Jinsung Yang <jsgood.yang@samsung.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 */

#ifndef __S3C_MINI6410_H__
#define __S3C_MINI6410_H__

#include <linux/fb.h>

#include <mach/regs-gpio.h>
#include <mach/regs-lcd.h>


/**
 * WARNING: CLKVAL is defined upon 133 MHz HCLK, please update it
 * when HCLK freq changed.
 *   VCLK = 133 MHz / (CLKVAL + 1)
 */

//------------------------------------------------------------------------------
#if defined(CONFIG_FB_S3C_EXT_TFT480272)

#define S3CFB_LCD_TYPE	"N43"
#define S3CFB_VBP		(0x03)	/* back porch */
#define S3CFB_VFP		(0x02)	/* front porch */
#define S3CFB_VSW		(0x02)	/* vsync width */
#define S3CFB_HBP		(0x2d)	/* back porch */
#define S3CFB_HFP		(0x04)	/* front porch */
#define S3CFB_HSW		(0x06)	/* hsync width */

#define S3CFB_HRES		480		/* horizon pixel  x resolition */
#define S3CFB_VRES		272		/* line cnt       y resolution */

#define S3CFB_CLKVAL		11
#define S3CFB_VIDCON1	(S3C_VIDCON1_IVCLK_RISE_EDGE)

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_HSD480272)

#define S3CFB_LCD_TYPE	"H43"
#define S3CFB_VBP		(0x08)	/* back porch */
#define S3CFB_VFP		(0x08)	/* front porch */
#define S3CFB_VSW		(0x01)	/* vsync width */
#define S3CFB_HBP		(0x28)	/* back porch */
#define S3CFB_HFP		(0x05)	/* front porch */
#define S3CFB_HSW		(0x01)	/* hsync width */

#define S3CFB_HRES		480		/* horizon pixel  x resolition */
#define S3CFB_VRES		272		/* line cnt       y resolution */

#define S3CFB_CLKVAL		13
#define S3CFB_VIDCON1	(S3C_VIDCON1_IHSYNC_INVERT | S3C_VIDCON1_IVSYNC_INVERT)

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_TFT800480)

#define S3CFB_LCD_TYPE	"A70"
#define S3CFB_VBP		(0x1d)	/* back porch */
#define S3CFB_VFP		(0x11)	/* front porch */
#define S3CFB_VSW		(0x18)	/* vsync width */
#define S3CFB_HBP		(0x28)	/* back porch */
#define S3CFB_HFP		(0x28)	/* front porch */
#define S3CFB_HSW		(0x30)	/* hsync width */

#define S3CFB_HRES		800		/* horizon pixel  x resolition */
#define S3CFB_VRES		480		/* line cnt       y resolution */

#define S3CFB_CLKVAL		3	/* ~33.25 MHz */
#define S3CFB_VIDCON1	(S3C_VIDCON1_IHSYNC_INVERT | S3C_VIDCON1_IVSYNC_INVERT)

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_S70T800480)

#define S3CFB_LCD_TYPE	"S70"
#define S3CFB_VBP		(0x15)	/* back porch */
#define S3CFB_VFP		(0x16)	/* front porch */
#define S3CFB_VSW		(0x02)	/* vsync width */
#define S3CFB_HBP		(0x2C)	/* back porch */
#define S3CFB_HFP		(0xD2)	/* front porch */
#define S3CFB_HSW		(0x02)	/* hsync width */

#define S3CFB_HRES		800		/* horizon pixel  x resolition */
#define S3CFB_VRES		480		/* line cnt       y resolution */

#define S3CFB_CLKVAL		3	/* ~33.25 MHz */
#define S3CFB_VIDCON1	(S3C_VIDCON1_IHSYNC_INVERT | S3C_VIDCON1_IVSYNC_INVERT)

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_W50I800480)

#define S3CFB_LCD_TYPE	"W50i"
#define S3CFB_VBP		(29)	/* back porch */
#define S3CFB_VFP		(13)	/* front porch */
#define S3CFB_VSW		( 3)	/* vsync width */
#define S3CFB_HBP		(40)	/* back porch */
#define S3CFB_HFP		(40)	/* front porch */
#define S3CFB_HSW		(48)	/* hsync width */

#define S3CFB_HRES		800		/* horizon pixel  x resolition */
#define S3CFB_VRES		480		/* line cnt       y resolution */

#define S3CFB_CLKVAL		3	/* ~33.25 MHz */
#define S3CFB_VIDCON1	(S3C_VIDCON1_IHSYNC_INVERT | S3C_VIDCON1_IVSYNC_INVERT)

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_T240320)

#define S3CFB_LCD_TYPE	"T35"
#define S3CFB_VBP		(0x01)	/* back porch */
#define S3CFB_VFP		(0x01)	/* front porch */
#define S3CFB_VSW		(0x04)	/* vsync width */
#define S3CFB_HBP		(0x01)	/* back porch */
#define S3CFB_HFP		(0x04)	/* front porch */
#define S3CFB_HSW		(0x1E)	/* hsync width */

#define S3CFB_HRES		240		/* horizon pixel  x resolition */
#define S3CFB_VRES		320		/* line cnt       y resolution */

#define S3CFB_CLKVAL		11

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_X240320)

#define S3CFB_LCD_TYPE	"X35"
#define S3CFB_VBP		( 4)	/* back porch */
#define S3CFB_VFP		( 2)	/* front porch */
#define S3CFB_VSW		(10)	/* vsync width */
#define S3CFB_HBP		( 8)	/* back porch */
#define S3CFB_HFP		( 6)	/* front porch */
#define S3CFB_HSW		(18)	/* hsync width */

#define S3CFB_HRES		240		/* horizon pixel  x resolition */
#define S3CFB_VRES		320		/* line cnt       y resolution */

#define S3CFB_CLKVAL		23	/* ~5.542 MHz */
#define S3CFB_VIDCON1	(S3C_VIDCON1_IVCLK_RISE_EDGE | S3C_VIDCON1_IVDEN_INVERT)

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_ZQ320240)

#define S3CFB_LCD_TYPE  "ZQ35"
#define S3CFB_VBP		(0x0c)	/* back porch */
#define S3CFB_VFP		(0x04)	/* front porch */
#define S3CFB_VSW		(0x01)	/* vsync width */
#define S3CFB_HBP		(0x46)	/* back porch */
#define S3CFB_HFP		(0x04)	/* front porch */
#define S3CFB_HSW		(0x01)	/* hsync width */

#define S3CFB_HRES		320		/* horizon pixel  x resolition */
#define S3CFB_VRES		240		/* line cnt       y resolution */

#define S3CFB_CLKVAL		22

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_G10V640480)

#define S3CFB_LCD_TYPE	"G10"
#define S3CFB_VBP		(0x22)	/* back porch */
#define S3CFB_VFP		(0x0a)	/* front porch */
#define S3CFB_VSW		(0x01)	/* vsync width */
#define S3CFB_HBP		(0x63)	/* back porch */
#define S3CFB_HFP		(0x3c)	/* front porch */
#define S3CFB_HSW		(0x01)	/* hsync width */

#define S3CFB_HRES		640		/* horizon pixel  x resolition */
#define S3CFB_VRES		480		/* line cnt       y resolution */

#define S3CFB_CLKVAL		4

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_TFT640480)

#define S3CFB_LCD_TYPE	"L80"
#define S3CFB_VBP		(0x01)	/* back porch */
#define S3CFB_VFP		(0x01)	/* front porch */
#define S3CFB_VSW		(0x01)	/* vsync width */
#define S3CFB_HBP		(0x03)	/* back porch */
#define S3CFB_HFP		(0x03)	/* front porch */
#define S3CFB_HSW		(0x28)	/* hsync width */

#define S3CFB_HRES		640		/* horizon pixel  x resolition */
#define S3CFB_VRES		480		/* line cnt       y resolution */

#define S3CFB_CLKVAL		3

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_VGA1024768)

#define S3CFB_LCD_TYPE	"XGA"
#define S3CFB_VBP		(0x02)	/* back porch */
#define S3CFB_VFP		(0x02)	/* front porch */
#define S3CFB_VSW		(0x10)	/* vsync width */
#define S3CFB_HBP		(0x02)	/* back porch */
#define S3CFB_HFP		(0x02)	/* front porch */
#define S3CFB_HSW		(0x2A)	/* hsync width */

#define S3CFB_HRES		1024	/* horizon pixel  x resolition */
#define S3CFB_VRES		768		/* line cnt       y resolution */

#define S3CFB_CLKVAL		5

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_VGA800600)

#define S3CFB_LCD_TYPE	"SVGA"
#define S3CFB_VBP		(0x02)	/* back porch */
#define S3CFB_VFP		(0x02)	/* front porch */
#define S3CFB_VSW		(0x10)	/* vsync width */
#define S3CFB_HBP		(0x02)	/* back porch */
#define S3CFB_HFP		(0x02)	/* front porch */
#define S3CFB_HSW		(0x2A)	/* hsync width */

#define S3CFB_HRES		800		/* horizon pixel  x resolition */
#define S3CFB_VRES		600		/* line cnt       y resolution */

#define S3CFB_CLKVAL		5

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_VGA640480)

#define S3CFB_LCD_TYPE	"VGA"
#define S3CFB_VBP		(0x02)	/* back porch */
#define S3CFB_VFP		(0x02)	/* front porch */
#define S3CFB_VSW		(0x10)	/* vsync width */
#define S3CFB_HBP		(0x02)	/* back porch */
#define S3CFB_HFP		(0x02)	/* front porch */
#define S3CFB_HSW		(0x2A)	/* hsync width */

#define S3CFB_HRES		640		/* horizon pixel  x resolition */
#define S3CFB_VRES		480		/* line cnt       y resolution */

#define S3CFB_CLKVAL		5

//------------------------------------------------------------------------------
#elif defined(CONFIG_FB_S3C_EXT_EZVGA800600)

#define S3CFB_LCD_TYPE	"EZVGA"
#define S3CFB_VBP		(0x02)	/* back porch */
#define S3CFB_VFP		(0x02)	/* front porch */
#define S3CFB_VSW		(0x10)	/* vsync width */
#define S3CFB_HBP		(0xA8)	/* back porch */
#define S3CFB_HFP		(0x11)	/* front porch */
#define S3CFB_HSW		(0x2A)	/* hsync width */

#define S3CFB_HRES		800		/* horizon pixel  x resolition */
#define S3CFB_VRES		600		/* line cnt       y resolution */

#define S3CFB_CLKVAL		2

//------------------------------------------------------------------------------
#else
#error "mini6410 frame buffer driver not configured"
#endif


#endif // __S3C_MINI6410_H__

