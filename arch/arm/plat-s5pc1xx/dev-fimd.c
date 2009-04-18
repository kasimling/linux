/* linux/arch/arm/plat-s5pc1xx/dev-fimd.c
 *
 * Copyright 2009 Samsung Electronics
 *	Jinsung Yang <jsgood.yang@samsung.com>
 *	http://samsungsemi.com/
 *
 * S5PC1XX series device definition for FIMD
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>

#include <mach/map.h>

#include <plat/fimd.h>
#include <plat/devs.h>

static struct resource s3c_fimd_resource[] = {
	[0] = {
		.start = S5PC1XX_PA_LCD,
		.end   = S5PC1XX_PA_LCD + S5PC1XX_SZ_LCD - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_LCD0,
		.end   = IRQ_LCD3,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device s3c_device_fimd = {
	.name		  = "s3c-fimd",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_fimd_resource),
	.resource	  = s3c_fimd_resource,
};

static struct s3c_platform_fimd default_fimd_data __initdata = {
	.hw_ver	= 0x50,
	.clk_name = "lcd",
	.clockrate = 66000000,
	.max_wins = 5,
	.max_buffers = 2,
};

void __init s3c_fimd_set_platdata(struct s3c_platform_fimd *pd)
{
	struct s3c_platform_fimd *npd;

	if (!pd)
		pd = &default_fimd_data;

	npd = kmemdup(pd, sizeof(struct s3c_platform_fimd), GFP_KERNEL);
	if (!npd)
		printk(KERN_ERR "%s: no memory for platform data\n", __func__);

	npd->cfg_gpio = s3c_fimd_cfg_gpio;
	npd->backlight_on = s3c_fimd_backlight_on;
	npd->reset_lcd = s3c_fimd_reset_lcd;

	s3c_device_fimd.dev.platform_data = npd;
}

