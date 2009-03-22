/*
 * Project Name JPEG DRIVER IN Linux
 * Copyright  2007 Samsung Electronics Co, Ltd. All Rights Reserved.
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics  ("Confidential Information").
 * you shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics
 *
 * This file implements JPEG driver.
 *
 * @name JPEG DRIVER MODULE Module (JPGMem.c)
 * @author Jiun Yu (jiun.yu@samsung.com)
 * @date 04-07-07
 * @author modify kwak Hyun Min (hyunmin.kwak@samsung.com) 
 * date 12-03-20 for linux 2.6.28 
 */
#include <asm/io.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/types.h>

#include "jpg_mem.h"
#include "jpg_misc.h"
#include "log_msg.h"


/*----------------------------------------------------------------------------
*Function: phy_to_vir_addr

*Parameters: 		dwContext		:
*Return Value:		True/False
*Implementation Notes: memory mapping from physical addr to virtual addr
-----------------------------------------------------------------------------*/
void *phy_to_vir_addr(UINT32 phy_addr, int mem_size)
{
	void	*reserved_mem;

	reserved_mem = (void *)ioremap((unsigned long)phy_addr, (int)mem_size);

	if (reserved_mem == NULL) {
		log_msg(LOG_ERROR, "phy_to_vir_addr", "DD::Phyical to virtual memory mapping was failed!\r\n");
		return NULL;
	}

	return reserved_mem;
}

void *mem_move(void *dst, const void *src, unsigned int size)
{
	return memmove(dst, src, size);
}

void *mem_alloc(unsigned int size)
{
	void	*alloc_mem;

	alloc_mem = (void *)kmalloc((int)size, GFP_KERNEL);

	if (alloc_mem == NULL) {
		log_msg(LOG_ERROR, "Mem_Alloc", "memory allocation failed!\r\n");
		return NULL;
	}

	return alloc_mem;
}
