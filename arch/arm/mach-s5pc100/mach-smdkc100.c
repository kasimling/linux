/* linux/arch/arm/mach-s5pc100/mach-smdkc100.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/clk.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/map.h>
#include <mach/regs-mem.h>
#include <mach/gpio.h>

#include <asm/irq.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <plat/regs-rtc.h>
#include <plat/iic.h>
#include <plat/fimc.h>

#include <plat/nand.h>
#include <plat/partition.h>
#include <plat/s5pc100.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/ts.h>
#include <plat/adc.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-bank-k0.h>
#if defined(CONFIG_USB_GADGET_S3C_OTGD) || \
	defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)
#include <plat/regs-otg.h>
#include <plat/regs-clock.h>
#include <plat/pll.h>
#include <linux/usb/ch9.h>

/* S3C_USB_CLKSRC 0: EPLL 1: CLK_48M */
#define S3C_USB_CLKSRC	1
#define OTGH_PHY_CLK_VALUE      (0x22)  /* UTMI Interface, otg_phy input clk 12Mhz Oscillator */
#endif

#if defined(CONFIG_PM)
#include <plat/pm.h>
#endif

#define UCON S3C2410_UCON_DEFAULT | S3C2410_UCON_UCLK
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

extern struct sys_timer s5pc1xx_timer;

static struct s3c2410_uartcfg smdkc100_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
};

struct map_desc smdkc100_iodesc[] = {};

static struct platform_device *smdkc100_devices[] __initdata = {
	&s3c_device_lcd,
        &s3c_device_nand,
        &s3c_device_onenand,
	&s3c_device_keypad,
	&s3c_device_ts,
	&s3c_device_adc,
        &s3c_device_rtc,
	&s3c_device_smc911x,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
        &s3c_device_usb,
	&s3c_device_usbgadget,
        &s3c_device_hsmmc0,
        &s3c_device_hsmmc1,
        &s3c_device_spi0,
        &s3c_device_spi1,
	&s3c_device_mfc,
	&s3c_device_fimc0,
	&s3c_device_fimc1,
	&s3c_device_fimc2,
};


static struct s3c_ts_mach_info s3c_ts_platform __initdata = {
	.delay 			= 10000,
	.presc 			= 49,
	.oversampling_shift	= 2,
	.resol_bit 		= 12,
	.s3c_adc_con		= ADC_TYPE_2,
};

static struct s3c_adc_mach_info s3c_adc_platform __initdata = {
        /* s5pc100 supports 12-bit resolution */
        .delay  = 10000,
        .presc  = 49,
        .resolution = 12,
};

static struct i2c_board_info i2c_devs0[] __initdata = {
	{ I2C_BOARD_INFO("24c08", 0x50), },
};

static struct i2c_board_info i2c_devs1[] __initdata = {
	{ I2C_BOARD_INFO("24c128", 0x57), },
};

static void __init smdkc100_map_io(void)
{
	s3c_device_nand.name = "s5pc100-nand";
	s5pc1xx_init_io(smdkc100_iodesc, ARRAY_SIZE(smdkc100_iodesc));
	s3c24xx_init_clocks(0);
	s3c24xx_init_uarts(smdkc100_uartcfgs, ARRAY_SIZE(smdkc100_uartcfgs));
}

static void __init smdkc100_smc911x_set(void)
{
	unsigned int tmp;

	tmp = __raw_readl(S5PC1XX_GPK0CON);
	tmp &=~(0x7<<12);
	tmp |=(S5PC1XX_GPK0_3_SROM_CSn3);
	__raw_writel(tmp, S5PC1XX_GPK0CON);

	tmp = __raw_readl(S5PC1XX_SROM_BW);
	tmp &=~(0xF<<12);
	tmp |= (1<<12);
	__raw_writel(tmp, S5PC1XX_SROM_BW);

	__raw_writel((0x0<<28)|(0x4<<24)|(0xd<<16)|(0x1<<12)|(0x4<<8)|(0x6<<4)|(0x0<<0), S5PC1XX_SROM_BC3);
}


static void __init smdkc100_machine_init(void)
{
        s3c_device_nand.dev.platform_data = &s3c_nand_mtd_part_info;
	s3c_device_onenand.dev.platform_data = &s3c_onenand_data;

	smdkc100_smc911x_set();

	s3c_ts_set_platdata(&s3c_ts_platform);
	s3c_adc_set_platdata(&s3c_adc_platform);

	/* i2c */
	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);
	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));

	/* fimc */
	s3c_fimc0_set_platdata(NULL);
	s3c_fimc1_set_platdata(NULL);
	s3c_fimc2_set_platdata(NULL);

	platform_add_devices(smdkc100_devices, ARRAY_SIZE(smdkc100_devices));
#if defined(CONFIG_PM)
	s5pc1xx_pm_init();
#endif
}

MACHINE_START(SMDKC100, "SMDKC100")
	/* Maintainer: Ben Dooks <ben@fluff.org> */
	.phys_io	= S3C_PA_UART & 0xfff00000,
	.io_pg_offst	= (((u32)S3C_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S5PC1XX_PA_SDRAM + 0x100,

	.init_irq	= s5pc100_init_irq,
	.map_io		= smdkc100_map_io,
	.init_machine	= smdkc100_machine_init,
	.timer		= &s5pc1xx_timer,
MACHINE_END


#if	defined(CONFIG_USB_GADGET_S3C_OTGD) || \
	defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)

/* Initializes OTG Phy. */
void otg_phy_init(void) {
        writel(readl(S5P_OTHERS)|S5P_OTHERS_USB_SIG_MASK, S5P_OTHERS);
        writel(0x0, S3C_USBOTG_PHYPWR);         /* Power up */
        writel(OTGH_PHY_CLK_VALUE, S3C_USBOTG_PHYCLK);
        writel(0x7, S3C_USBOTG_RSTCON);

        udelay(50);
        writel(0x0, S3C_USBOTG_RSTCON);
        udelay(50);
}
EXPORT_SYMBOL(otg_phy_init);

/* USB Control request data struct must be located here for DMA transfer */
struct usb_ctrlrequest usb_ctrl __attribute__((aligned(8)));
EXPORT_SYMBOL(usb_ctrl);

/* OTG PHY Power Off */
void otg_phy_off(void) {
        writel(readl(S3C_USBOTG_PHYCLK) | (0X1 << 4), S3C_USBOTG_PHYCLK);
        writel(readl(S5P_OTHERS)&~S5P_OTHERS_USB_SIG_MASK, S5P_OTHERS);
}
EXPORT_SYMBOL(otg_phy_off);
#endif

#if defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)
void usb_host_clk_en(void) {
	struct clk *otg_clk;

        switch (S3C_USB_CLKSRC) {
        case 0: /* epll clk */
                /* Setting the epll clk to 48 MHz, P=3, M=96, S=3 */
                writel((readl(S5P_EPLL_CON) & ~(S5P_EPLL_MASK)) | (S5P_EPLL_EN \
						| S5P_EPLLVAL(96,3,3)), S5P_EPLL_CON);
                writel((readl(S5P_CLK_SRC0) | S5P_CLKSRC0_EPLL_MASK), S5P_CLK_SRC0);
                writel((readl(S5P_CLK_SRC1)& ~S5P_CLKSRC1_UHOST_MASK), S5P_CLK_SRC1);

                /* USB host clock divider ratio is 1 */
                writel((readl(S5P_CLK_DIV2)& ~S5P_CLKDIV2_UHOST_MASK), S5P_CLK_DIV2);
                break;

	case 1: /* oscillator 12M clk */
		otg_clk = clk_get(NULL, "otg");
		clk_enable(otg_clk);
		otg_phy_init();
		writel((readl(S5P_CLK_SRC1) | S5P_CLKSRC1_CLK48M_MASK) \
						| S5P_CLKSRC1_UHOST_MASK, S5P_CLK_SRC1);

		//USB host colock divider ratio is 1
		writel(readl(S5P_CLK_DIV2)& ~S5P_CLKDIV2_UHOST_MASK, S5P_CLK_DIV2);
		break;
	/* Add other clock sources here */

        default:
                printk(KERN_INFO "Unknown USB Host Clock Source\n");
                BUG();
                break;
        }

        writel(readl(S5P_CLKGATE_D10)|S5P_CLKGATE_D10_USBHOST, S5P_CLKGATE_D10);
        writel(readl(S5P_SCLKGATE0)|S5P_CLKGATE_SCLK0_USBHOST, S5P_SCLKGATE0);

}

EXPORT_SYMBOL(usb_host_clk_en);
#endif

#if defined(CONFIG_RTC_DRV_S3C)
/* RTC common Function for samsung APs*/
unsigned int s3c_rtc_set_bit_byte(void __iomem *base, uint offset, uint val)
{
        writeb(val, base + offset);

        return 0;
}

unsigned int s3c_rtc_read_alarm_status(void __iomem *base)
{
        return 1;
}

void s3c_rtc_set_pie(void __iomem *base, uint to)
{
        unsigned int tmp;

        tmp = readw(base + S3C2410_RTCCON) & ~S3C_RTCCON_TICEN;

        if (to)
                tmp |= S3C_RTCCON_TICEN;

        writew(tmp, base + S3C2410_RTCCON);
}

void s3c_rtc_set_freq_regs(void __iomem *base, uint freq, uint *s3c_freq)
{
        unsigned int tmp;

        tmp = readw(base + S3C2410_RTCCON) & (S3C_RTCCON_TICEN | S3C2410_RTCCON_RTCEN );
        writew(tmp, base + S3C2410_RTCCON);
        *s3c_freq = freq;
        tmp = (32768 / freq)-1;
        writel(tmp, base + S3C2410_TICNT);
}

void s3c_rtc_enable_set(struct platform_device *pdev,void __iomem *base, int en)
{
        unsigned int tmp;

        if (!en) {
                tmp = readw(base + S3C2410_RTCCON);
                writew(tmp & ~ (S3C2410_RTCCON_RTCEN | S3C_RTCCON_TICEN), base + S3C2410_RTCCON);
        } else {
                /* re-enable the device, and check it is ok */
                if ((readw(base+S3C2410_RTCCON) & S3C2410_RTCCON_RTCEN) == 0){
                        dev_info(&pdev->dev, "rtc disabled, re-enabling\n");

                        tmp = readw(base + S3C2410_RTCCON);
                        writew(tmp|S3C2410_RTCCON_RTCEN, base+S3C2410_RTCCON);
                }

                if ((readw(base + S3C2410_RTCCON) & S3C2410_RTCCON_CNTSEL)){
                        dev_info(&pdev->dev, "removing RTCCON_CNTSEL\n");

                        tmp = readw(base + S3C2410_RTCCON);
                        writew(tmp& ~S3C2410_RTCCON_CNTSEL, base+S3C2410_RTCCON);
                }

                if ((readw(base + S3C2410_RTCCON) & S3C2410_RTCCON_CLKRST)){
                        dev_info(&pdev->dev, "removing RTCCON_CLKRST\n");

                        tmp = readw(base + S3C2410_RTCCON);
                        writew(tmp & ~S3C2410_RTCCON_CLKRST, base+S3C2410_RTCCON);
                }
        }
}
#endif

#if defined(CONFIG_KEYPAD_S3C) || defined (CONFIG_KEYPAD_S3C_MODULE)
void s3c_setup_keypad_cfg_gpio(int rows, int columns)
{
	unsigned int gpio;
	unsigned int end;

	end = S5PC1XX_GPH3(rows);

	/* Set all the necessary GPH2 pins to special-function 0 */
	for (gpio = S5PC1XX_GPH3(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}

	end = S5PC1XX_GPH2(columns);

	/* Set all the necessary GPK pins to special-function 0 */
	for (gpio = S5PC1XX_GPH2(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}
}

EXPORT_SYMBOL(s3c_setup_keypad_cfg_gpio);
#endif
