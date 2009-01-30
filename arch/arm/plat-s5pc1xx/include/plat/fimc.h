/* linux/arch/arm/plat-s5pc1xx/include/plat/fimc.h
 *
 * Platform header file for Samsung Camera Interface (FIMC) driver
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _FIMC_H
#define _FIMC_H

struct platform_device;

struct s3c_platform_fimc {
	const char	clk_name[16];
	int		line_length;
	int		nr_frames;

	void		(*cfg_gpio)(struct platform_device *dev);
};

extern void s3c_fimc0_set_platdata(struct s3c_platform_fimc *fimc);
extern void s3c_fimc1_set_platdata(struct s3c_platform_fimc *fimc);
extern void s3c_fimc2_set_platdata(struct s3c_platform_fimc *fimc);

/* defined by architecture to configure gpio */
extern void s3c_fimc0_cfg_gpio(struct platform_device *dev);
extern void s3c_fimc1_cfg_gpio(struct platform_device *dev);
extern void s3c_fimc2_cfg_gpio(struct platform_device *dev);

extern void s3c_fimc_s5k4ba_init(void);

#endif /* _FIMC_H */

