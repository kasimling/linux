/* ------------------------------------------------------------------------- */
/* 									     */
/* hspi-s3c.h - definitions of s3c specific spi interface	     */
/* 									     */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 2006 Samsung Electronics Co. ltd.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.		     */
/* ------------------------------------------------------------------------- */

#ifndef _HSPI_S3C_H
#define _HSPI_S3C_H

#include <asm/dma.h>
#include <mach/s3c-dma.h>

#if(SPI_CHANNEL==0)
/* SPI CHANNEL 0 */
#define S3C_SPI_TX_DATA_REG	0xEC300018  //SPI TX data
#define S3C_SPI_RX_DATA_REG	0xEC30001C  //SPI RX data
#elif(SPI_CHANNEL==1)
/* SPI CHANNEL 1 */
#define S3C_SPI_TX_DATA_REG	0xEC400018  //SPI TX data
#define S3C_SPI_RX_DATA_REG	0xEC40001C  //SPI RX data
#else
/* SPI CHANNEL 2 */
#define S3C_SPI_TX_DATA_REG	0xEC500018  //SPI TX data
#define S3C_SPI_RX_DATA_REG	0xEC50001C  //SPI RX data
#endif

/* DMA transfer unit (byte). */
#define S3C_DMA_XFER_BYTE   	1
#define S3C_DMA_XFER_WORD	4	

/* spi controller state */
int req_dma_flag = 1;
enum s3c_spi_state {
	STATE_IDLE,
	STATE_XFER_TX,
	STATE_XFER_RX,
	STATE_STOP
};

static struct s3c2410_dma_client s3cspi_dma_client = {
	.name		= "s3c-spi-dma",
};

struct s3c_spi {
	spinlock_t		lock;
	struct semaphore 	sem;
	int 			nr;
	int			dma;
	dma_addr_t		dmabuf_addr;

	struct spi_msg		*msg;
	unsigned int		msg_num;
	unsigned int		msg_idx;
	unsigned int		msg_ptr;
	unsigned int		msg_rd_ptr;

	enum s3c_spi_state	state;

	void __iomem		*regs;
	struct clk		*clk;
	struct device		*dev;
	struct resource		*irq;
	struct resource		*ioarea;
	struct spi_dev		spidev;
};


#endif /* _S3C_SPI_H */
