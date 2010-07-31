/* linux/arch/arm/mach-s3c6410/mach-mini6410.c
 *
 * Copyright 2010 Kasim Ling <ling_kasim@yahoo.cn>
 *    http://gitorious.org/opencsbc
 * Heavily based on mach-smdk6410.c
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
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/dm9000.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/delay.h>
#include <linux/pwm_backlight.h>

#include <video/platform_lcd.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/regs-fb.h>
#include <mach/map.h>

#include <asm/irq.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <mach/regs-modem.h>
#include <mach/regs-gpio.h>
#include <mach/regs-sys.h>
#include <plat/iic.h>
#include <plat/fb.h>
#include <plat/gpio-cfg.h>

#include <mach/s3c6410.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/ts.h>
#include <plat/pm.h>
#include <plat/udc-hs.h>
#include <linux/usb/android_composite.h>

#define UCON S3C2410_UCON_DEFAULT | S3C2410_UCON_UCLK
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

#define MACH_MINI6410_DM9K_BASE (0x18000000 + 0x300)

static struct s3c2410_uartcfg mini6410_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[3] = {
		.hwport	     = 3,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
};

/* framebuffer and LCD setup. */
static struct s3c_fb_pd_win mini6410_fb_win0 = {
        /* this is to ensure we use win0 */
        .win_mode       = {
                .pixclock       = 115440,
                .left_margin    = 3,
                .right_margin   = 2,
                .upper_margin   = 1,
                .lower_margin   = 1,
                .hsync_len      = 40,
                .vsync_len      = 1,
                .xres           = 480,
                .yres           = 272,
        },
        .max_bpp        = 32,
        .default_bpp    = 16,
        .virtual_x      = 480,
        .virtual_y      = 544, /* 2*272 */
};

static struct s3c_fb_platdata mini6410_lcd_pdata __initdata = {
        .setup_gpio     = s3c64xx_fb_gpio_setup_24bpp,
        .win[0]         = &mini6410_fb_win0,
        .vidcon0        = VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
        .vidcon1        = VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC,
};

/* DM9000AEP 10/100 ethernet controller */

static struct resource mini6410_dm9k_resource[] = {
        [0] = {
                .start = MACH_MINI6410_DM9K_BASE,
                .end   = MACH_MINI6410_DM9K_BASE + 3,
                .flags = IORESOURCE_MEM
        },
        [1] = {
                .start = MACH_MINI6410_DM9K_BASE + 4,
                .end   = MACH_MINI6410_DM9K_BASE + 7,
                .flags = IORESOURCE_MEM
        },
        [2] = {
                .start = S3C_EINT(7),
                .end   = S3C_EINT(7),
                .flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
        }
};

/*
 * The DM9000 has no eeprom, and it's MAC address is set by
 * the bootloader before starting the kernel.
 */
static struct dm9000_plat_data mini6410_dm9k_pdata = {
        .flags          = (DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM),
};

static struct platform_device mini6410_device_eth = {
        .name           = "dm9000",
        .id             = -1,
        .num_resources  = ARRAY_SIZE(mini6410_dm9k_resource),
        .resource       = mini6410_dm9k_resource,
        .dev            = {
                .platform_data  = &mini6410_dm9k_pdata,
        },
};

static struct s3c2410_ts_mach_info mini6410_touchscreen_pdata __initdata = {
       .delay                  = 65535,
       .presc                  = 99,
       .oversampling_shift     = 4,
};

static struct gpio_keys_button mini6410_buttons[] = {
        {
                .gpio           = S3C64XX_GPN(0),               /* K1 */
                .code           = KEY_F1,
                .desc           = "Button K1(MENU)",
                .active_low     = 1,
        },
        {
                .gpio           = S3C64XX_GPN(1),               /* K2 */
                .code           = KEY_HOME,
                .desc           = "Button K2(HOME)",
                .active_low     = 1,
        },
        {
                .gpio           = S3C64XX_GPN(2),               /* K3 */
                .code           = KEY_BACK,
                .desc           = "Button K3(BACK)",
                .active_low     = 1,
        },
        {
                .gpio           = S3C64XX_GPN(3),               /* K4 */
                .code           = KEY_ENTER,
                .desc           = "Button K4(ENTER)",
                .active_low     = 1,
                .wakeup         = 1,
        },
        {
                .gpio           = S3C64XX_GPN(4),               /* K5 */
                .code           = KEY_DOWN,
                .desc           = "Button K5(DOWN)",
                .active_low     = 1,
        },

        {
                .gpio           = S3C64XX_GPN(5),               /* K6 */
                .code           = KEY_UP,
                .desc           = "Button K6(UP)",
                .active_low     = 1,
        },
};

static struct gpio_keys_platform_data mini6410_button_data = {
        .buttons        = mini6410_buttons,
        .nbuttons       = ARRAY_SIZE(mini6410_buttons),
};

static struct platform_device mini6410_button_device = {
        .name           = "gpio-keys",
        .id             = -1,
        .dev            = {
                .platform_data  = &mini6410_button_data,
        }
};

static int mini6410_bl_init(struct device *dev)
{
        int ret;

        ret = gpio_request(S3C64XX_GPE(0), "lcd backlight enable");
        if (!ret)
                ret = gpio_direction_output(S3C64XX_GPE(0), 0);

        return ret;
}

static int mini6410_bl_notify(struct device *dev, int brightness)
{
        /*
         * translate from CIELUV/CIELAB L*->brightness, E.G. from
         * perceived luminance to light output. Assumes range 0..25600
         */
        if (brightness < 0x800) {
                /* Y = Yn * L / 903.3 */
                brightness = (100*256 * brightness + 231245/2) / 231245;
        } else {
                /* Y = Yn * ((L + 16) / 116 )^3 */
                int t = (brightness*4 + 16*1024 + 58)/116;
                brightness = 25 * ((t * t * t + 0x100000/2) / 0x100000);
        }
        gpio_set_value(S3C64XX_GPE(0), brightness);


        return brightness;
}

static void mini6410_bl_exit(struct device *dev)
{
        gpio_free(S3C64XX_GPE(0));
}

static struct platform_pwm_backlight_data mini6410_backlight_data = {
        .pwm_id         = 1,
        .max_brightness = 100 * 256,
        .dft_brightness = 40 * 256,
        .pwm_period_ns  = 1000000000 / (100 * 256 * 20),
        .init           = mini6410_bl_init,
        .notify         = mini6410_bl_notify,
        .exit           = mini6410_bl_exit,

};


static struct platform_device mini6410_backlight_device = {
        .name           = "pwm-backlight",
        .dev            = {
                .parent = &s3c_device_timer[1].dev,
                .platform_data = &mini6410_backlight_data,
        },
};

static struct s3c_hsotg_plat mini6410_otg_data = {
        .is_osc         = 0,
};

static struct usb_mass_storage_platform_data mass_storage_pdata = {
        .nluns = 1,
        .vendor = "FriendlyARM",
        .product = "MINI6410",
        .release = 0x0100,
};

static struct platform_device usb_mass_storage_device = {
        .name = "usb_mass_storage",
        .id = -1,
        .dev = {
                .platform_data = &mass_storage_pdata,
        },
};

static char *usb_functions[] = { "usb_mass_storage" };
static char *usb_functions_adb[] = { "usb_mass_storage", "adb" };

static struct android_usb_product usb_products[] = {
        {
                .product_id     = 0x6401,
                .num_functions  = ARRAY_SIZE(usb_functions),
                .functions      = usb_functions,
        },
        {
                .product_id     = 0x6402,
                .num_functions  = ARRAY_SIZE(usb_functions_adb),
                .functions      = usb_functions_adb,
        },
};

static struct android_usb_platform_data android_usb_pdata = {
        .vendor_id = 0x18d1,
        .product_id = 0x6402,
        .version = 0x0100,
        .serial_number = "42",
        .product_name = "MINI6410",
        .manufacturer_name = "FriendlyARM",
        .num_products = ARRAY_SIZE(usb_products),
        .products = usb_products,
        .num_functions = ARRAY_SIZE(usb_functions_adb),
        .functions = usb_functions_adb,
};

static struct platform_device android_usb_device = {
        .name = "android_usb",
        .id = -1,
        .dev = {
                .platform_data = &android_usb_pdata,
        },
};

static struct map_desc mini6410_iodesc[] = {};

static struct platform_device *mini6410_devices[] __initdata = {
	&s3c_device_hsmmc0,
/*	&s3c_device_hsmmc1,	*/
	&s3c_device_fb,
	&s3c_device_i2c0,
	&s3c_device_ohci,
	&s3c_device_usb_hsotg,
	&usb_mass_storage_device,
	&android_usb_device,
	&s3c_device_adc,
	&s3c_device_ts,
	&mini6410_device_eth,
	&mini6410_button_device,
	&s3c_device_timer[1],
	&s3c_device_rtc,
	&mini6410_backlight_device,
};

static struct i2c_board_info i2c_devs0[] __initdata = {
};

static void __init mini6410_map_io(void)
{
	u32 tmp;

	s3c64xx_init_io(mini6410_iodesc, ARRAY_SIZE(mini6410_iodesc));
	s3c24xx_init_clocks(12000000);
	s3c24xx_init_uarts(mini6410_uartcfgs, ARRAY_SIZE(mini6410_uartcfgs));

	/* set the LCD type */

	tmp = __raw_readl(S3C64XX_SPCON);
	tmp &= ~S3C64XX_SPCON_LCD_SEL_MASK;
	tmp |= S3C64XX_SPCON_LCD_SEL_RGB;
	__raw_writel(tmp, S3C64XX_SPCON);

	/* remove the lcd bypass */
	tmp = __raw_readl(S3C64XX_MODEM_MIFPCON);
	tmp &= ~MIFPCON_LCD_BYPASS;
	__raw_writel(tmp, S3C64XX_MODEM_MIFPCON);

}

static void __init mini6410_machine_init(void)
{
	int i;

	s3c_i2c0_set_platdata(NULL);

	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));

	s3c_fb_set_platdata(&mini6410_lcd_pdata);
	s3c24xx_ts_set_platdata(&mini6410_touchscreen_pdata);

        /* mark the key as input, without pullups (there is one on the board) */
        for (i = 0; i < ARRAY_SIZE(mini6410_buttons); i++) {
                s3c_gpio_setpull(mini6410_buttons[i].gpio, S3C_GPIO_PULL_NONE);
                s3c_gpio_cfgpin(mini6410_buttons[i].gpio,
                                        S3C_GPIO_SFN(0));
        }

	s3c_device_usb_hsotg.dev.platform_data = &mini6410_otg_data;
	platform_add_devices(mini6410_devices, ARRAY_SIZE(mini6410_devices));

	s3c_pm_init();
}

MACHINE_START(MINI6410, "MINI6410")
	/* Maintainer: Kasim Ling <ling_kasim@yahoo.cn> */
	.phys_io	= S3C_PA_UART & 0xfff00000,
	.io_pg_offst	= (((u32)S3C_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C64XX_PA_SDRAM + 0x100,

	.init_irq	= s3c6410_init_irq,
	.map_io		= mini6410_map_io,
	.init_machine	= mini6410_machine_init,
	.timer		= &s3c24xx_timer,
MACHINE_END
