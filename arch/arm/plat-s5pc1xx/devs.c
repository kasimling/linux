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

/* HS-MMC controller */
extern struct s3c_hsmmc_cfg s3c_hsmmc0_platform;
extern struct s3c_hsmmc_cfg s3c_hsmmc1_platform;
extern struct s3c_hsmmc_cfg s3c_hsmmc2_platform;

static struct resource s3c_hsmmc0_resource[] = {
        [0] = {
                .start = S5PC1XX_PA_HSMMC0,
                .end   = S5PC1XX_PA_HSMMC0+S5PC1XX_SZ_HSMMC-1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_HSMMC0,
                .end   = IRQ_HSMMC0,
                .flags = IORESOURCE_IRQ,
        },
};

static struct resource s3c_hsmmc1_resource[] = {
        [0] = {
                .start = S5PC1XX_PA_HSMMC1,
                .end   = S5PC1XX_PA_HSMMC1+S5PC1XX_SZ_HSMMC-1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_HSMMC1,
                .end   = IRQ_HSMMC1,
                .flags = IORESOURCE_IRQ,
        },
};

static struct resource s3c_hsmmc2_resource[] = {
        [0] = {
                .start = S5PC1XX_PA_HSMMC2,
                .end   = S5PC1XX_PA_HSMMC2+S5PC1XX_SZ_HSMMC-1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_HSMMC2,
                .end   = IRQ_HSMMC2,
                .flags = IORESOURCE_IRQ,
        },
};

struct platform_device s3c_device_hsmmc0 = {
        .name             = "s3c-hsmmc",
        .id               = 0,
        .num_resources    = ARRAY_SIZE(s3c_hsmmc0_resource),
        .resource         = s3c_hsmmc0_resource,
        .dev            = {
                .platform_data = &s3c_hsmmc0_platform,
        }
};

struct platform_device s3c_device_hsmmc1 = {
        .name             = "s3c-hsmmc",
        .id               = 1,
        .num_resources    = ARRAY_SIZE(s3c_hsmmc1_resource),
        .resource         = s3c_hsmmc1_resource,
        .dev            = {
                .platform_data = &s3c_hsmmc1_platform,
        }
};

#if 1
struct platform_device s3c_device_hsmmc2 = {
        .name             = "s3c-hsmmc",
        .id               = 2,
        .num_resources    = ARRAY_SIZE(s3c_hsmmc2_resource),
        .resource         = s3c_hsmmc2_resource,
        .dev            = {
                .platform_data = &s3c_hsmmc2_platform,
        }
};
#endif

EXPORT_SYMBOL(s3c_device_hsmmc0);
EXPORT_SYMBOL(s3c_device_hsmmc1);
EXPORT_SYMBOL(s3c_device_hsmmc2);

static struct resource s3c_rtc_resource[] = {
        [0] = {
                .start = S3C_PA_RTC,
                .end   = S3C_PA_RTC + 0xff,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_RTC_ALARM,
                .end   = IRQ_RTC_ALARM,
                .flags = IORESOURCE_IRQ,
        },
        [2] = {
                .start = IRQ_RTC_TIC,
                .end   = IRQ_RTC_TIC,
                .flags = IORESOURCE_IRQ
        }
};

struct platform_device s3c_device_rtc = {
        .name             = "s3c2410-rtc",
        .id               = -1,
        .num_resources    = ARRAY_SIZE(s3c_rtc_resource),
        .resource         = s3c_rtc_resource,
};

/* OneNAND Controller */
static struct resource s3c_onenand_resource[] = {
        [0] = {
                .start = S5PC1XX_PA_ONENAND,
                .end   = S5PC1XX_PA_ONENAND + S5PC1XX_SZ_ONENAND - 1,
                .flags = IORESOURCE_MEM,
        }
};

struct platform_device s3c_device_onenand = {
        .name             = "onenand",
        .id               = -1,
        .num_resources    = ARRAY_SIZE(s3c_onenand_resource),
        .resource         = s3c_onenand_resource,
};

EXPORT_SYMBOL(s3c_device_onenand);

/* Keypad interface */
static struct resource s3c_keypad_resource[] = {
	[0] = {
		.start = S3C_PA_KEYPAD,
		.end   = S3C_PA_KEYPAD+ S3C_SZ_KEYPAD - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_KEYPAD,
		.end   = IRQ_KEYPAD,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device s3c_device_keypad = {
	.name		  = "s3c-keypad",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_keypad_resource),
	.resource	  = s3c_keypad_resource,
};

EXPORT_SYMBOL(s3c_device_keypad);

/* SPI (0) */

static struct resource s3c_spi0_resource[] = {
        [0] = {
                .start = S5PC1XX_PA_SPI0,
                .end   = S5PC1XX_PA_SPI0 + S5PC1XX_SZ_SPI - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_SPI0,
                .end   = IRQ_SPI0,
                .flags = IORESOURCE_IRQ,
        }

};

static u64 s3c_device_spi0_dmamask = 0xffffffffUL;

struct platform_device s3c_device_spi0 = {
        .name             = "s3c2410-spi",
        .id               = 0,
        .num_resources    = ARRAY_SIZE(s3c_spi0_resource),
        .resource         = s3c_spi0_resource,
        .dev              = {
                .dma_mask = &s3c_device_spi0_dmamask,
                .coherent_dma_mask = 0xffffffffUL
        }
};

EXPORT_SYMBOL(s3c_device_spi0);

/* SPI (1) */

static struct resource s3c_spi1_resource[] = {
        [0] = {
                .start = S5PC1XX_PA_SPI1,
                .end   = S5PC1XX_PA_SPI1 + S5PC1XX_SZ_SPI - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_SPI1,
                .end   = IRQ_SPI1,
                .flags = IORESOURCE_IRQ,
        }
};

static u64 s3c_device_spi1_dmamask = 0xffffffffUL;

struct platform_device s3c_device_spi1 = {
        .name             = "s3c2410-spi",
        .id               = 1,
        .num_resources    = ARRAY_SIZE(s3c_spi1_resource),
        .resource         = s3c_spi1_resource,
        .dev              = {
                .dma_mask = &s3c_device_spi1_dmamask,
                .coherent_dma_mask = 0xffffffffUL
        }
};

EXPORT_SYMBOL(s3c_device_spi1);

/* SPI (2) */

static struct resource s3c_spi2_resource[] = {
        [0] = {
                .start = S5PC1XX_PA_SPI2,
                .end   = S5PC1XX_PA_SPI2 + S5PC1XX_SZ_SPI - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_SPI2,
                .end   = IRQ_SPI2,
                .flags = IORESOURCE_IRQ,
        }

};

static u64 s3c_device_spi2_dmamask = 0xffffffffUL;

struct platform_device s3c_device_spi2 = {
        .name             = "s3c2410-spi",
        .id               = 2,
        .num_resources    = ARRAY_SIZE(s3c_spi2_resource),
        .resource         = s3c_spi2_resource,
        .dev              = {
                .dma_mask = &s3c_device_spi2_dmamask,
                .coherent_dma_mask = 0xffffffffUL
        }
};

EXPORT_SYMBOL(s3c_device_spi2);
