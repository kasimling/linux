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
#include <asm/setup.h>
#include <mach/memory.h>

/* FIXME: temporary implementation to avoid compile error */
int dma_needs_bounce(struct device *dev, dma_addr_t addr, size_t size)
{
	return 0;
}

void s5pc1xx_reserve_bootmem(void)
{
	struct bootmem_data *bdata;
	unsigned long sdram_start, sdram_size;
	unsigned long reserved;

	bdata = NODE_DATA(0)->bdata;
	sdram_start = bdata->node_min_pfn << PAGE_SHIFT;
	sdram_size = (bdata->node_low_pfn << PAGE_SHIFT) - sdram_start;
	reserved = 0;

	sdram_start &= PAGE_MASK;
	
	/* add here for devices' bootmem size */
#ifdef CONFIG_VIDEO_FIMC_STATIC_MEMORY
	reserved += CONFIG_VIDEO_FIMC_STATIC_MEMSIZE * SZ_1K;
#endif

	if (reserved > 0) {
		reserve_bootmem(sdram_start, \
				PAGE_ALIGN(sdram_size - reserved), BOOTMEM_DEFAULT);

		printk(KERN_INFO \
			"s3c64xx: %lu bytes SDRAM reserved at 0x%08x\n", reserved, \
			(unsigned int) (sdram_start + sdram_size - reserved));
	}
}
