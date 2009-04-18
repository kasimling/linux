/* linux/arch/arm/plat-s5pc1xx/include/plat/fimd.h
 *
 * Platform header file for Samsung Display Controller (FIMD) driver
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _FIMD_H
#define _FIMD_H

struct platform_device;

struct s3c_platform_fimd {
	int		hw_ver;
	const char	clk_name[16];
	u32		clockrate;
	int		max_wins;
	int		max_buffers;

	void		(*cfg_gpio)(struct platform_device *dev);
	int		(*backlight_on)(struct platform_device *dev);
	int		(*reset_lcd)(struct platform_device *dev);
};

extern void s3c_fimd_set_platdata(struct s3c_platform_fimd *fimd);

/* defined by architecture to configure gpio */
extern void s3c_fimd_cfg_gpio(struct platform_device *dev);
extern int s3c_fimd_backlight_on(struct platform_device *dev);
extern int s3c_fimd_reset_lcd(struct platform_device *dev);

#endif /* _FIMD_H */

