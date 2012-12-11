/*
 * linux/arch/arm/mach-s5pv210/mini210-lcds.c
 *
 * Copyright (c) 2011 FriendlyARM (www.arm9.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/gpio.h>

#include <plat/fb.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>

#include <../../../drivers/video/samsung/s3cfb.h>


/* s3cfb configs for supported LCD */

static struct s3cfb_lcd wvga_w50 = {
	.width= 800,
	.height = 480,
	.p_width = 108,
	.p_height = 64,
	.bpp = 32,
	.freq = 85,

	.timing = {
		.h_fp = 40,
		.h_bp = 40,
		.h_sw = 48,
		.v_fp = 13,
		.v_fpe = 1,
		.v_bp = 29,
		.v_bpe = 1,
		.v_sw = 3,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_a70 = {
	.width = 800,
	.height = 480,
	.p_width = 152,
	.p_height = 90,
	.bpp = 32,
	.freq = 85,

	.timing = {
		.h_fp = 40,
		.h_bp = 40,
		.h_sw = 48,
		.v_fp = 17,
		.v_fpe = 1,
		.v_bp = 29,
		.v_bpe = 1,
		.v_sw = 24,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_s70 = {
	.width = 800,
	.height = 480,
	.p_width = 154,
	.p_height = 86,
	.bpp = 32,
	.freq = 85,

	.timing = {
		.h_fp = 210,
		.h_bp = 44,
		.h_sw = 2,
		.v_fp = 22,
		.v_fpe = 1,
		.v_bp = 21,
		.v_bpe = 1,
		.v_sw = 2,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};


/* Try to guess LCD panel by kernel command line, or
 * using *W50* as default */

static struct {
	char *name;
	struct s3cfb_lcd *lcd;
} mini210_lcd_config[] = {
	{ "W50", &wvga_w50 },
	{ "A70", &wvga_a70 },
	{ "S70", &wvga_s70 },
};

static int lcd_idx = 0;

static int __init mini210_setup_lcd(char *str)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(mini210_lcd_config); i++) {
		if (!strcasecmp(mini210_lcd_config[i].name, str)) {
			lcd_idx = i;
			break;
		}
	}

	return 0;
}
early_param("lcd", mini210_setup_lcd);


struct s3cfb_lcd *mini210_get_lcd(void)
{
	printk("MINI210: %s selected\n", mini210_lcd_config[lcd_idx].name);

	return mini210_lcd_config[lcd_idx].lcd;
}

void mini210_get_lcd_res(int *w, int *h)
{
	struct s3cfb_lcd *lcd = mini210_lcd_config[lcd_idx].lcd;

	if (w)
		*w = lcd->width;
	if (h)
		*h = lcd->height;

	return;
}
EXPORT_SYMBOL(mini210_get_lcd_res);

