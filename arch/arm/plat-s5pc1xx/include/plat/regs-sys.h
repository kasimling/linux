/* arch/arm/plat-s5pc1xx/include/plat/regs-sys.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * S5PC1XX system register definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __PLAT_REGS_SYS_H
#define __PLAT_REGS_SYS_H __FILE__

#if defined (CONFIG_PLAT_S5PC1XX)
#define S3C_SYSREG(x)		(S3C_VA_SYS + (x))
#define S3C_OTHERS              S3C_SYSREG(0x8200)

#define S3C_OTHERS_USB_SIG_MASK (1 << 16)
#endif

#define S3C64XX_OTHERS		S3C_SYSREG(0x900)

#define S3C64XX_OTHERS_USBMASK	(1 << 16)

#endif /* _PLAT_REGS_SYS_H */
