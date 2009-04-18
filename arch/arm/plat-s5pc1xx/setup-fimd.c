/* linux/arch/arm/plat-s5pc1xx/setup-fimd.c
 *
 * Copyright 2009 Samsung Electronics
 *	Jinsung Yang <jsgood.yang@samsung.com>
 *	http://samsungsemi.com/
 *
 * Base S5PC1XX FIMD gpio configuration
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <plat/gpio-cfg.h>

struct platform_device; /* don't need the contents */

void s3c_fimd_cfg_gpio(struct platform_device *dev)
{
	int i;

	for (i = 0; i < 8; i++)
		s3c_gpio_cfgpin(S5PC1XX_GPF0(i), S3C_GPIO_SFN(2));

	for (i = 0; i < 8; i++)
		s3c_gpio_cfgpin(S5PC1XX_GPF1(i), S3C_GPIO_SFN(2));

	for (i = 0; i < 8; i++)
		s3c_gpio_cfgpin(S5PC1XX_GPF2(i), S3C_GPIO_SFN(2));

	for (i = 0; i < 4; i++)
		s3c_gpio_cfgpin(S5PC1XX_GPF3(i), S3C_GPIO_SFN(2));
}

int s3c_fimd_backlight_on(struct platform_device *dev)
{
	int err;

	if (gpio_is_valid(S5PC1XX_GPD(0))) {
		err = gpio_request(S5PC1XX_GPD(0), "GPD");

		if (err) {
			printk(KERN_ERR "failed to request GPD for "
				"lcd backlight control\n");
			return err;
		}

		gpio_direction_output(S5PC1XX_GPD(0), 1);
	}

	return 0;
}

int s3c_fimd_reset_lcd(struct platform_device *dev)
{
	int err;

	if (gpio_is_valid(S5PC1XX_GPH0(6))) {
		err = gpio_request(S5PC1XX_GPH0(6), "GPH0");

		if (err) {
			printk(KERN_ERR "failed to request GPH0 for "
				"lcd reset control\n");
			return err;
		}

		gpio_direction_output(S5PC1XX_GPH0(6), 1);
	}

	mdelay(100);

	gpio_set_value(S5PC1XX_GPH0(6), 0);
	mdelay(10);

	gpio_set_value(S5PC1XX_GPH0(6), 1);
	mdelay(10);

	gpio_free(S5PC1XX_GPH0(6));
	gpio_free(S5PC1XX_GPD(0));

	return 0;
}

