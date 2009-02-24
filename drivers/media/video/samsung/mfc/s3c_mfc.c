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

#include <plat/media.h>

#include "s3c_mfc.h"

static struct platform_driver s3c_mfc_driver = {
//	.probe		= s3c_mfc_probe,
//	.remove		= s3c_mfc_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = MFC_NAME,
	},
};

static int s3c_mfc_init(void)
{
	if (platform_driver_register(&s3c_mfc_driver) != 0) {
		return -1;
	}

	printk("---------------T E S T M F C----------------\n");
	dma_addr_t buffer;

	unsigned char *virt;

	buffer = s3c_get_media_memory(S3C_MDEV_MFC);

	virt = ioremap_nocache(buffer, SZ_16M);

	return 0;
}

static void s3c_mfc_exit(void)
{
	platform_driver_unregister(&s3c_mfc_driver);
}


module_init(s3c_mfc_init);
module_exit(s3c_mfc_exit);

MODULE_AUTHOR("Jiun, Yu");
MODULE_LICENSE("GPL");
