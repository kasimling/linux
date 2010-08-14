/* linux/arch/arm/plat-s3c64xx/dev-g2d.c
 *
 * Copyright 2010 OpenCSBC. 
 * http://gitorious.org/opencsbc
 *
 * Base S3C64XX G2D resource and device definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/platform_device.h>

#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <mach/hardware.h>
#include <mach/map.h>

#include <plat/devs.h>

/* FIMG-2D controller */
static struct resource s3c_g2d_resource[] = {
        [0] = {
                .start  = S3C64XX_PA_G2D,
                .end    = S3C64XX_PA_G2D + S3C64XX_SZ_G2D - 1,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_2D,
                .end    = IRQ_2D,
                .flags  = IORESOURCE_IRQ,
        }
};

struct platform_device s3c_device_g2d = {
        .name           = "s3c-g2d",
        .id             = -1,
        .num_resources  = ARRAY_SIZE(s3c_g2d_resource),
        .resource       = s3c_g2d_resource
};
EXPORT_SYMBOL(s3c_device_g2d);
