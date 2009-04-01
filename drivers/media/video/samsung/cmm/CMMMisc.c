/* linux/drivers/media/video/samsung/cmm/CMMMisc.c
 *
 * Driver file for Samsung CMM(Codec Memory Management)
 *
 * Jiun Yu, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <stdarg.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <plat/regs-lcd.h>

#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/wait.h>

#include "CMMMisc.h"

static HANDLE hMutex	= NULL;

/*----------------------------------------------------------------------------
*Function: CreateJPGmutex
*Implementation Notes: Create Mutex handle 
-----------------------------------------------------------------------------*/
HANDLE CreateCMMmutex(void)
{
	hMutex = (HANDLE)kmalloc(sizeof(struct mutex), GFP_KERNEL);
	if (hMutex == NULL)
		return NULL;
	
	mutex_init(hMutex);
	
	return hMutex;
}

/*----------------------------------------------------------------------------
*Function: LockJPGMutex
*Implementation Notes: lock mutex 
-----------------------------------------------------------------------------*/
DWORD LockCMMMutex(void)
{
    mutex_lock(hMutex);  
    return 1;
}

/*----------------------------------------------------------------------------
*Function: UnlockJPGMutex
*Implementation Notes: unlock mutex
-----------------------------------------------------------------------------*/
DWORD UnlockCMMMutex(void)
{
	mutex_unlock(hMutex);
	
    return 1;
}

/*----------------------------------------------------------------------------
*Function: DeleteJPGMutex
*Implementation Notes: delete mutex handle 
-----------------------------------------------------------------------------*/
void DeleteCMMMutex(void)
{
	if (hMutex == NULL)
		return;

	mutex_destroy(hMutex);
}

