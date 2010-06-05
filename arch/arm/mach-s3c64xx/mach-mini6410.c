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
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/delay.h>

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

#define UCON S3C2410_UCON_DEFAULT | S3C2410_UCON_UCLK
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

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
        .default_bpp    = 18,
};

static struct s3c_fb_platdata mini6410_lcd_pdata __initdata = {
        .setup_gpio     = s3c64xx_fb_gpio_setup_24bpp,
        .win[0]         = &mini6410_fb_win0,
        .vidcon0        = VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
        .vidcon1        = VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC,
};

static struct map_desc mini6410_iodesc[] = {};

static struct platform_device *mini6410_devices[] __initdata = {
	&s3c_device_hsmmc0,
/*	&s3c_device_hsmmc1,	*/
	&s3c_device_fb,
	&s3c_device_i2c0,
	&s3c_device_ohci,
	&s3c_device_usb_hsotg,
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
	s3c_i2c0_set_platdata(NULL);

	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));

	s3c_fb_set_platdata(&mini6410_lcd_pdata);

	platform_add_devices(mini6410_devices, ARRAY_SIZE(mini6410_devices));
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
