/* linux/arch/arm/plat-s5pc1xx/bootmem.c
 *
 * Copyright 2009 Samsung Electronics
 *	Jinsung Yang <jsgood.yang@samsung.com>
 *	http://samsungsemi.com/
 *
 * Bootmem helper functions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/swap.h>
#include <mach/memory.h>

void s5pc1xx_reserve_bootmem(void)
{
	/* FIXME: how to get the system memory size? */
	int mem_size = 128 * SZ_1M;
	int reserve_size = 0;
	int bootmem_size;

	/* add here for devices' bootmem size */
#ifdef CONFIG_VIDEO_SAMSUNG_STATIC_MEMORY
	reserve_size += CONFIG_VIDEO_SAMSUNG_STATIC_MEMORY_SIZE * SZ_1K;
#endif

	/* bootmem_size means none-reserved memory size */
	if (reserve_size > 0) {
		bootmem_size = mem_size - reserve_size;
		reserve_bootmem(PHYS_OFFSET, \
				PAGE_ALIGN(bootmem_size), BOOTMEM_DEFAULT);

		printk(KERN_INFO \
			"S5PC1XX: %dMB system memory reserved from 0x%08x\n", \
			reserve_size / SZ_1M, \
			(unsigned int) PHYS_OFFSET + bootmem_size);
	}
}
