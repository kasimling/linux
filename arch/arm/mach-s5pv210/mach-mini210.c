/* linux/arch/arm/mach-s5pv210/mach-mini210.c
 *
 * Copyright (c) 2012 Alex Ling <kasimling@gmail.com>
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/sysdev.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <mach/map.h>
#include <mach/regs-clock.h>

#include <plat/regs-serial.h>
#include <plat/regs-srom.h>
#include <plat/gpio-cfg.h>
#include <plat/s5pv210.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/iic.h>
#include <plat/pm.h>
#include <plat/s5p-time.h>

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define MINI210_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define MINI210_ULCON_DEFAULT	S3C2410_LCON_CS8

#define MINI210_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

unsigned int pm_debug_scratchpad = 0;

static struct s3c2410_uartcfg mini210_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= MINI210_UCON_DEFAULT,
		.ulcon		= MINI210_ULCON_DEFAULT,
		.ufcon		= MINI210_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= MINI210_UCON_DEFAULT,
		.ulcon		= MINI210_ULCON_DEFAULT,
		.ufcon		= MINI210_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= MINI210_UCON_DEFAULT,
		.ulcon		= MINI210_ULCON_DEFAULT,
		.ufcon		= MINI210_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= MINI210_UCON_DEFAULT,
		.ulcon		= MINI210_ULCON_DEFAULT,
		.ufcon		= MINI210_UFCON_DEFAULT,
	},
};

void s3c_setup_uart_cfg_gpio(unsigned char port)
{
	/* To be updated */
}
EXPORT_SYMBOL(s3c_setup_uart_cfg_gpio);

static struct platform_device *mini210_devices[] __initdata = {
	&s3c_device_hsmmc0,
	&s3c_device_hsmmc1,
	&s3c_device_hsmmc2,
	&s3c_device_hsmmc3,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
	&s3c_device_rtc,
	&s3c_device_wdt,
	&s5pv210_device_ac97,
	&s5pv210_device_iis0,
	&s5pv210_device_spdif,
	&samsung_asoc_dma,
	&s3c_device_timer[3],
};

static struct i2c_board_info mini210_i2c_devs0[] __initdata = {
	/* To Be Updated */
};

static struct i2c_board_info mini210_i2c_devs1[] __initdata = {
	/* To Be Updated */
};

static struct i2c_board_info mini210_i2c_devs2[] __initdata = {
	/* To Be Updated */
};

static void __init mini210_map_io(void)
{
	s5p_init_io(NULL, 0, S5P_VA_CHIPID);
	s3c24xx_init_clocks(24000000);
	s3c24xx_init_uarts(mini210_uartcfgs, ARRAY_SIZE(mini210_uartcfgs));
	s5p_set_timer_source(S5P_PWM2, S5P_PWM4);
}

static void __init mini210_machine_init(void)
{
	s3c_pm_init();

	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);
	s3c_i2c2_set_platdata(NULL);
	i2c_register_board_info(0, mini210_i2c_devs0,
			ARRAY_SIZE(mini210_i2c_devs0));
	i2c_register_board_info(1, mini210_i2c_devs1,
			ARRAY_SIZE(mini210_i2c_devs1));
	i2c_register_board_info(2, mini210_i2c_devs2,
			ARRAY_SIZE(mini210_i2c_devs2));

	platform_add_devices(mini210_devices, ARRAY_SIZE(mini210_devices));
}

MACHINE_START(MINI210, "MINI210")
	/* Maintainer: Alex Ling <kasimling@gmail.com> */
	.boot_params	= S5P_PA_SDRAM + 0x100,
	.init_irq	= s5pv210_init_irq,
	.map_io	= mini210_map_io,
	.init_machine	= mini210_machine_init,
	.timer		= &s5p_timer,
MACHINE_END
