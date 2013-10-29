/*
 * Driver for SMSC USB4640 HSIC USB 2.0 hub controller driver
 *
 * Copyright (c) 2013 Alex Ling (kasimling@gmail.com)
 * based on SMSC USB3503 USB 2.0 hub controller driver by Dongjin Kim
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/platform_data/usb4640.h>

struct usb4640 {
	struct device		*dev;
	int	gpio_reset;
};

static int usb4640_reset(struct usb4640 *hub)
{
	if (gpio_is_valid(hub->gpio_reset)) {
		gpio_set_value_cansleep(hub->gpio_reset, 0);
		msleep(1);
		gpio_set_value_cansleep(hub->gpio_reset, 1);
	}

	return 0;
}

static int usb4640_probe(struct usb4640 *hub)
{
	struct device *dev = hub->dev;
	struct usb4640_platform_data *pdata = dev_get_platdata(dev);
	struct device_node *np = dev->of_node;
	int err;

	if (pdata) {
		hub->gpio_reset		= pdata->gpio_reset;
	} else if (np) {
		hub->gpio_reset = of_get_named_gpio(np, "reset-gpios", 0);
		if (hub->gpio_reset == -EPROBE_DEFER)
			return -EPROBE_DEFER;
	}

	if (gpio_is_valid(hub->gpio_reset)) {
		err = devm_gpio_request_one(dev, hub->gpio_reset,
				GPIOF_OUT_INIT_HIGH, "usb4640 reset");
		if (err) {
			dev_err(dev,
				"unable to request GPIO %d as reset pin (%d)\n",
				hub->gpio_reset, err);
			return err;
		}
	}

	usb4640_reset(hub);

	return 0;
}

static int usb4640_platform_probe(struct platform_device *pdev)
{
	struct usb4640 *hub;

	hub = devm_kzalloc(&pdev->dev, sizeof(struct usb4640), GFP_KERNEL);
	if (!hub) {
		dev_err(&pdev->dev, "private data alloc fail\n");
		return -ENOMEM;
	}
	hub->dev = &pdev->dev;

	return usb4640_probe(hub);
}

#ifdef CONFIG_OF
static const struct of_device_id usb4640_of_match[] = {
	{ .compatible = "smsc,usb4640", },
	{ .compatible = "smsc,usb4640i", },
	{},
};
MODULE_DEVICE_TABLE(of, usb4640_of_match);
#endif

static struct platform_driver usb4640_platform_driver = {
	.driver = {
		.name = USB4640_NAME,
		.of_match_table = of_match_ptr(usb4640_of_match),
		.owner = THIS_MODULE,
	},
	.probe		= usb4640_platform_probe,
};

static int __init usb4640_init(void)
{
	int err;

	err = platform_driver_register(&usb4640_platform_driver);
	if (err != 0)
		pr_err("usb4640: Failed to register platform driver: %d\n",
		       err);

	return 0;
}
module_init(usb4640_init);

static void __exit usb4640_exit(void)
{
	platform_driver_unregister(&usb4640_platform_driver);
}
module_exit(usb4640_exit);

MODULE_AUTHOR("Alex Ling <kasimling@gmail.com>");
MODULE_DESCRIPTION("USB4640 USB HUB driver");
MODULE_LICENSE("GPL");
