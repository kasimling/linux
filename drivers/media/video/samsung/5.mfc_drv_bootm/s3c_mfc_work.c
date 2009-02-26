/* linux/drivers/media/video/samsung/mfc/s3c_mfc.c
 *
 * Core file for Samsung Multi Format Codecs (MFC) driver
 *
 * Jiun Yu, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/platform_device.h>

#include <asm/io.h>

#include <plat/media.h>

#include "s3c_mfc.h"

static void __iomem	*mfc_base;
static dma_addr_t phy_buffer;

static char banner[] __initdata = KERN_INFO "S3C64XX MFC Driver, (c) 2009 Samsung Electronics.\n";

static int s3c_mfc_probe(struct platform_device *pdev)
{
	phy_buffer = s3c_get_media_memory(S3C_MDEV_MFC);
	mfc_base = ioremap_nocache(phy_buffer, s3c_get_media_memsize(S3C_MDEV_MFC));

	printk(KERN_DEBUG "%s: pyh mem:0x%X\t vir mem:0x%X\n", __FUNCTION__, phy_buffer, (unsigned int)mfc_base);

	return 0;
}

static int s3c_mfc_remove(struct platform_device *pdev)
{
	iounmap(mfc_base);

	return 0;
}

static struct platform_driver s3c_mfc_driver = {
	.probe		= s3c_mfc_probe,
	.remove		= s3c_mfc_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = MFC_NAME,
	},
};

static int __init s3c_mfc_init(void)
{
	printk(banner);

	if (platform_driver_register(&s3c_mfc_driver) != 0) {
		return -1;
	}

	return 0;
}

static void __exit s3c_mfc_exit(void)
{
	platform_driver_unregister(&s3c_mfc_driver);
}


module_init(s3c_mfc_init);
module_exit(s3c_mfc_exit);

MODULE_AUTHOR("Jiun, Yu");
MODULE_LICENSE("GPL");
