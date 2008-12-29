/* linux/arch/arm/plat-s5pc1xx/devs.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * Base S5PC1XX resource and device definitions
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
#include <plat/adc.h>


/* SMC9115 LAN via ROM interface */

static struct resource s3c_smc911x_resources[] = {
      [0] = {
              .start  = S5PC1XX_PA_SMC9115,
              .end    = S5PC1XX_PA_SMC9115 + 0x1fffffff,
              .flags  = IORESOURCE_MEM,
      },
      [1] = {
              .start = IRQ_EINT10,
              .end   = IRQ_EINT10,
              .flags = IORESOURCE_IRQ,
        },
};

struct platform_device s3c_device_smc911x = {
      .name           = "smc911x",
      .id             =  -1,
      .num_resources  = ARRAY_SIZE(s3c_smc911x_resources),
      .resource       = s3c_smc911x_resources,
};

/* LCD Controller */

static struct resource s3c_lcd_resource[] = {
	[0] = {
		.start = S5PC1XX_PA_LCD,
		.end   = S5PC1XX_PA_LCD + SZ_1M - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_LCD0,
		.end   = IRQ_LCD3,
		.flags = IORESOURCE_IRQ,
	}
};

static u64 s3c_device_lcd_dmamask = 0xffffffffUL;

struct platform_device s3c_device_lcd = {
	.name		  = "s3c-lcd",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_lcd_resource),
	.resource	  = s3c_lcd_resource,
	.dev              = {
		.dma_mask		= &s3c_device_lcd_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};

/* ADC */
static struct resource s3c_adc_resource[] = {
	[0] = {
		.start = S3C_PA_ADC,
		.end   = S3C_PA_ADC + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_PENDN,
		.end   = IRQ_PENDN,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start = IRQ_ADC,
		.end   = IRQ_ADC,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device s3c_device_adc = {
	.name		  = "s3c-adc",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_adc_resource),
	.resource	  = s3c_adc_resource,
};

void __init s3c_adc_set_platdata(struct s3c_adc_mach_info *pd)
{
	struct s3c_adc_mach_info *npd;

	npd = kmalloc(sizeof(*npd), GFP_KERNEL);
	if (npd) {
		memcpy(npd, pd, sizeof(*npd));
		s3c_device_adc.dev.platform_data = npd;
	} else {
		printk(KERN_ERR "no memory for ADC platform data\n");
	}
}

/* NAND Controller */

static struct resource s3c_nand_resource[] = {
        [0] = {
                .start = S5PC1XX_PA_NAND,
                .end   = S5PC1XX_PA_NAND + S5PC1XX_SZ_NAND - 1,
                .flags = IORESOURCE_MEM,
        }
};

struct platform_device s3c_device_nand = {
        .name             = "s3c-nand",
        .id               = -1,
        .num_resources    = ARRAY_SIZE(s3c_nand_resource),
        .resource         = s3c_nand_resource,
};

EXPORT_SYMBOL(s3c_device_nand);

/* USB Host Controller */

static struct resource s3c_usb_resource[] = {
        [0] = {
                .start = S5PC1XX_PA_USBHOST,
                .end   = S5PC1XX_PA_USBHOST + S5PC1XX_SZ_USBHOST - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_UHOST,
                .end   = IRQ_UHOST,
                .flags = IORESOURCE_IRQ,
        }
};

static u64 s3c_device_usb_dmamask = 0xffffffffUL;

struct platform_device s3c_device_usb = {
        .name             = "s3c2410-ohci",
        .id               = -1,
        .num_resources    = ARRAY_SIZE(s3c_usb_resource),
        .resource         = s3c_usb_resource,
        .dev              = {
                .dma_mask = &s3c_device_usb_dmamask,
                .coherent_dma_mask = 0xffffffffUL
        }
};

EXPORT_SYMBOL(s3c_device_usb);

/* USB Device (Gadget)*/

static struct resource s3c_usbgadget_resource[] = {
        [0] = {
                .start = S5PC1XX_PA_OTG,
                .end   = S5PC1XX_PA_OTG+S5PC1XX_SZ_OTG-1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_OTG,
                .end   = IRQ_OTG,
                .flags = IORESOURCE_IRQ,
        }
};

struct platform_device s3c_device_usbgadget = {
        .name             = "s3c2410-usbgadget",
        .id               = -1,
        .num_resources    = ARRAY_SIZE(s3c_usbgadget_resource),
        .resource         = s3c_usbgadget_resource,
};

EXPORT_SYMBOL(s3c_device_usbgadget);
