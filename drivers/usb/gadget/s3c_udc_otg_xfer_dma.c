/*
 * drivers/usb/gadget/s3c_udc_otg_xfer_dma.c
 * Samsung S3C on-chip full/high speed USB OTG 2.0 device controllers
 *
 * Copyright (C) 2009 for Samsung Electronics
 *
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
 *
 */

#define GINTMSK_INIT	(INT_OUT_EP|INT_IN_EP|INT_RESUME|INT_ENUMDONE|INT_RESET|INT_SUSPEND)
#define DOEPMSK_INIT	(CTRL_OUT_EP_SETUP_PHASE_DONE|AHB_ERROR|TRANSFER_DONE)
#define DIEPMSK_INIT	(NON_ISO_IN_EP_TIMEOUT|AHB_ERROR|TRANSFER_DONE)
#define GAHBCFG_INIT	(PTXFE_HALF|NPTXFE_HALF|MODE_DMA|BURST_INCR4|GBL_INT_UNMASK)

/* Send Zero Length Packet for status stage of control transfer */
static inline void s3c_send_zlp(void)
{
	u32 diepsiz0, diepctl0;

	diepsiz0 = readl(S3C_UDC_OTG_DIEPTSIZ0);
	diepctl0 = readl(S3C_UDC_OTG_DIEPCTL0);

	writel(diepsiz0|1<<19| 0<<0, S3C_UDC_OTG_DIEPTSIZ0);
	writel(diepctl0|DEPCTL_EPENA|DEPCTL_CNAK|EP3_IN<<11, S3C_UDC_OTG_DIEPCTL0);
}

static int setdma_rx(struct s3c_ep *ep, struct s3c_request *req)
{
	u32 *buf, ctrl;
	u32 length, pktcnt;
	u32 ep_num = ep_index(ep);

	buf = req->req.buf + req->req.actual;
	prefetchw(buf);

	length = req->req.length - req->req.actual;
	dma_cache_maint(buf, length, DMA_FROM_DEVICE);

	if(length == 0)
		pktcnt = 1;
	else
		pktcnt = (length - 1)/(ep->ep.maxpacket) + 1;

	if(ep_num == EP0_CON) {
		ctrl =  readl(S3C_UDC_OTG_DOEPCTL0);

		writel(virt_to_phys(buf), S3C_UDC_OTG_DOEPDMA0);
		writel((pktcnt<<19)|(length<<0), S3C_UDC_OTG_DOEPTSIZ0);
		writel(DEPCTL_EPENA|DEPCTL_CNAK|ctrl, S3C_UDC_OTG_DOEPCTL0);

		DEBUG_OUT_EP("%s: RX DMA start : DOEPDMA0 = 0x%x, DOEPTSIZ0 = 0x%x, DOEPCTL0 = 0x%x\n"
				"\tbuf = 0x%p, pktcnt = %d, xfersize = %d\n",
				__FUNCTION__,
				readl(S3C_UDC_OTG_DOEPDMA0),
				readl(S3C_UDC_OTG_DOEPTSIZ0),
				readl(S3C_UDC_OTG_DOEPCTL0),
				buf, pktcnt, length);

	} else if (ep_num == EP1_OUT) {
		ctrl =  readl(S3C_UDC_OTG_DOEPCTL1);

		writel(virt_to_phys(buf), S3C_UDC_OTG_DOEPDMA1);
		writel((pktcnt<<19)|(length<<0), S3C_UDC_OTG_DOEPTSIZ1);
		writel(DEPCTL_EPENA|DEPCTL_CNAK|ctrl, S3C_UDC_OTG_DOEPCTL1);

		DEBUG_OUT_EP("%s: RX DMA start : DOEPDMA1 = 0x%x, DOEPTSIZ1 = 0x%x, DOEPCTL1 = 0x%x\n"
				"\tbuf = 0x%p, pktcnt = %d, xfersize = %d\n",
				__FUNCTION__,
				readl(S3C_UDC_OTG_DOEPDMA1),
				readl(S3C_UDC_OTG_DOEPTSIZ1),
				readl(S3C_UDC_OTG_DOEPCTL1),
				buf, pktcnt, length);

	} else {
		DEBUG_OUT_EP("Not Support EP\n");
	}

	return 0;

}

static int setdma_tx(struct s3c_ep *ep, struct s3c_request *req)
{
	u32 *buf, ctrl;
	u32 length, pktcnt;
	u32 ep_num = ep_index(ep);

	buf = req->req.buf + req->req.actual;
	prefetch(buf);
	length = req->req.length - req->req.actual;

	if(ep_num == EP0_CON) {
		length = min(length, (u32)ep_maxpacket(ep));
	}

	req->req.actual += length;
	dma_cache_maint(buf, length, DMA_TO_DEVICE);

	if(length == 0) {
		pktcnt = 1;
	} else {
		pktcnt = (length - 1)/(ep->ep.maxpacket) + 1;
	}

	if(ep_num == EP0_CON) {
		ctrl = readl(S3C_UDC_OTG_DIEPCTL0);

		writel(virt_to_phys(buf), S3C_UDC_OTG_DIEPDMA0);
		writel((pktcnt<<19)|(length<<0), (u32) S3C_UDC_OTG_DIEPTSIZ0);
		writel(DEPCTL_EPENA|DEPCTL_CNAK|(EP2_IN<<11), (u32) S3C_UDC_OTG_DIEPCTL0);

		DEBUG_IN_EP("%s:TX DMA start : DIEPDMA0 = 0x%x, DIEPTSIZ0 = 0x%x, DIEPCTL0 = 0x%x\n"
				"\tbuf = 0x%p, pktcnt = %d, xfersize = %d\n",
				__FUNCTION__,
				readl(S3C_UDC_OTG_DIEPDMA0),
				readl(S3C_UDC_OTG_DIEPTSIZ0),
				readl(S3C_UDC_OTG_DIEPCTL0),
				buf, pktcnt, length);

	} else if (ep_num == EP2_IN) {
		ctrl =  readl(S3C_UDC_OTG_DIEPCTL2);

		writel(virt_to_phys(buf), S3C_UDC_OTG_DIEPDMA2);
		writel((pktcnt<<19)|(length<<0), S3C_UDC_OTG_DIEPTSIZ2);
		writel(DEPCTL_EPENA|DEPCTL_CNAK|(EP0_CON<<11)|ctrl, (u32) S3C_UDC_OTG_DIEPCTL2);

		DEBUG_IN_EP("%s:TX DMA start : DIEPDMA2 = 0x%x, DIEPTSIZ2 = 0x%x, DIEPCTL2 = 0x%x\n"
				"\tbuf = 0x%p, pktcnt = %d, xfersize = %d\n",
				__FUNCTION__,
				readl(S3C_UDC_OTG_DIEPDMA2),
				readl(S3C_UDC_OTG_DIEPTSIZ2),
				readl(S3C_UDC_OTG_DIEPCTL2),
				buf, pktcnt, length);

	} else if (ep_num == EP3_IN) {
		ctrl =  readl(S3C_UDC_OTG_DIEPCTL3);

		writel(virt_to_phys(buf), S3C_UDC_OTG_DIEPDMA3);
		writel((pktcnt<<19)|(length<<0), S3C_UDC_OTG_DIEPTSIZ3);
		writel(DEPCTL_EPENA|DEPCTL_CNAK|(EP0_CON<<11)| ctrl, (u32) S3C_UDC_OTG_DIEPCTL3);

		DEBUG_IN_EP("%s:TX DMA start : DIEPDMA3 = 0x%x, DIEPTSIZ3 = 0x%x, DIEPCTL3 = 0x%x\n"
				"\tbuf = 0x%p, pktcnt = %d, xfersize = %d\n",
				__FUNCTION__,
				readl(S3C_UDC_OTG_DIEPDMA3),
				readl(S3C_UDC_OTG_DIEPTSIZ3),
				readl(S3C_UDC_OTG_DIEPCTL3),
				buf, pktcnt, length);

	} else {
		DEBUG_IN_EP("%s: --> Error Unused Endpoint!!\n",
				__FUNCTION__);
		BUG();
	}

	return length;
}

static void complete_rx(struct s3c_udc *dev, u32 ep_idx)
{
	struct s3c_ep *ep = &dev->ep[ep_idx];
	struct s3c_request *req = NULL;
	u32 csr=0, count_bytes=0, xfer_length, is_short = 0;
	DEBUG_OUT_EP("%s\n",__FUNCTION__);

	if (list_empty(&ep->queue)) {
		DEBUG_OUT_EP("%s: NULL REQ on OUT EP-%d\n", __FUNCTION__, ep_idx);
		return;

	}

	req = list_entry(ep->queue.next,	struct s3c_request, queue);

	if(ep_idx == EP0_CON) {
		csr = readl(S3C_UDC_OTG_DOEPTSIZ0);
		count_bytes = (csr & 0x7f);

	} else if (ep_idx == EP1_OUT) {
		csr = readl(S3C_UDC_OTG_DOEPTSIZ1);
		count_bytes = (csr & 0x7fff);

	} else {
		DEBUG_OUT_EP("%s : Not support EP\n", __FUNCTION__);
	}

	dma_cache_maint(req->req.buf, req->req.length, DMA_FROM_DEVICE);
	xfer_length = req->req.length-count_bytes;
	req->req.actual += min(xfer_length, req->req.length-req->req.actual);
	is_short = (xfer_length < ep->ep.maxpacket);

	DEBUG_OUT_EP("%s: EP-%d RX DMA done : %d/%d bytes received%s, DOEPTSIZ = 0x%x, %d bytes remained\n",
			__FUNCTION__, ep_idx, req->req.actual, req->req.length,
			is_short ? "/S" : "", csr, count_bytes);

	if (is_short || req->req.actual == xfer_length) {
		if(ep_idx == EP0_CON && dev->ep0state == DATA_STATE_RECV) {
			DEBUG_OUT_EP("	=> Send ZLP\n");
			dev->ep0state = WAIT_FOR_SETUP;
			s3c_send_zlp();

		} else {
			done(ep, req, 0);

			if(!list_empty(&ep->queue)) {
				req = list_entry(ep->queue.next, struct s3c_request, queue);
				DEBUG_OUT_EP("%s: Next Rx request start...\n", __FUNCTION__);
				setdma_rx(ep, req);
			}
		}
	}
}

static void complete_tx(struct s3c_udc *dev, u32 ep_idx)
{
	struct s3c_ep *ep = &dev->ep[ep_idx];
	struct s3c_request *req;
	u32 count_bytes = 0;

	if (list_empty(&ep->queue)) {
		DEBUG_IN_EP("%s: NULL REQ on IN EP-%d\n", __FUNCTION__, ep_idx);
		return;

	}

	req = list_entry(ep->queue.next, struct s3c_request, queue);

	if(dev->ep0state == DATA_STATE_XMIT) {
		u32 last = write_fifo_ep0(ep, req);

		if(last) {
			dev->ep0state = WAIT_FOR_SETUP;
		}

		return;
	}

	if(ep_idx == EP0_CON) {
		count_bytes = (readl(S3C_UDC_OTG_DIEPTSIZ0)) & 0x7f;
		req->req.actual = req->req.length-count_bytes;

		DEBUG_IN_EP("%s: TX DMA done : %d/%d bytes sent, DIEPTSIZ0 = 0x%x\n",
				__FUNCTION__, req->req.actual,
				req->req.length,
				readl(S3C_UDC_OTG_DIEPTSIZ0));

	} else if (ep_idx == EP2_IN) {
		count_bytes = (readl(S3C_UDC_OTG_DIEPTSIZ2)) & 0x7fff;
		req->req.actual = req->req.length-count_bytes;

		DEBUG_IN_EP("%s: TX DMA done : %d/%d bytes sent, DIEPTSIZ2 = 0x%x\n",
				__FUNCTION__, req->req.actual,
				req->req.length,
				readl(S3C_UDC_OTG_DIEPTSIZ2));

	} else if (ep_idx == EP3_IN) {
		count_bytes = (readl(S3C_UDC_OTG_DIEPTSIZ3)) & 0x7fff;
		req->req.actual = req->req.length-count_bytes;

		DEBUG_IN_EP("%s: TX DMA done : %d/%d bytes sent, DIEPTSIZ3 = 0x%x\n",
				__FUNCTION__, req->req.actual,
				req->req.length,
				readl(S3C_UDC_OTG_DIEPTSIZ3));
	} else {
		DEBUG_IN_EP("%s: --> Error Unused Endpoint-%d!!\n", __FUNCTION__, ep_idx);
	}

	if (req->req.actual == req->req.length) {
		done(ep, req, 0);

		if(!list_empty(&ep->queue)) {
			req = list_entry(ep->queue.next, struct s3c_request, queue);
			DEBUG_IN_EP("%s: Next Tx request start...\n", __FUNCTION__);
			setdma_tx(ep, req);
		}
	}
}

static void process_ep_in_intr(struct s3c_udc *dev)
{
	u32 ep_int, ep_int_status, ep_ctrl;

	ep_int = readl(S3C_UDC_OTG_DAINT);
	DEBUG_IN_EP("\tDAINT : 0x%x \n", ep_int);


	/* CONTROL IN endpont */
	if (ep_int & (1<<EP0_CON)) {
		ep_int_status = readl(S3C_UDC_OTG_DIEPINT0);
		ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL0);
		DEBUG_EP0("\tEP0-IN : DIEPINT0 = 0x%x, DIEPCTL0 = 0x%x \n", ep_int_status, ep_ctrl);
		writel(ep_int_status, S3C_UDC_OTG_DIEPINT0); 		// Interrupt Clear

		if (ep_int_status & TRANSFER_DONE) {
			DEBUG_EP0("\tEP0-IN transaction completed - (TX DMA done)\n");

			complete_tx(dev, EP0_CON);

			if(dev->ep0state == WAIT_FOR_SETUP) {
				writel((1 << 19)|sizeof(struct usb_ctrlrequest), S3C_UDC_OTG_DOEPTSIZ0);
				writel(virt_to_phys(&usb_ctrl), S3C_UDC_OTG_DOEPDMA0);
				writel(ep_ctrl|DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
			}

		}

	}

	/* EP2 - BULK IN endpont */
	else if(ep_int & (1<<EP2_IN)) {	/* BULK IN endpont */
		ep_int_status = readl(S3C_UDC_OTG_DIEPINT2);
		ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL2);

		DEBUG_IN_EP("\tEP2-IN : DIEPINT2 = 0x%x, DIEPCTL2 = 0x%x\n", ep_int_status, ep_ctrl);

		if (ep_int_status & TRANSFER_DONE) {
			DEBUG_IN_EP("\tBULK IN transaction completed - (TX DMA done)\n");
			complete_tx(dev, EP2_IN);
		}
		writel(ep_int_status, S3C_UDC_OTG_DIEPINT2); 		// ep2 Interrupt Clear

	}

	/* EP3 - INTR IN endpont */
	else if(ep_int & (1<<EP3_IN)) {	/* BULK IN endpont */
		ep_int_status = readl(S3C_UDC_OTG_DIEPINT3);
		ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL3);

		DEBUG_IN_EP("\tEP3-IN : DIEPINT3 = 0x%x, DIEPCTL3 = 0x%x\n", ep_int_status, ep_ctrl);

		if (ep_int_status & TRANSFER_DONE) {
			DEBUG_IN_EP("\tBULK IN transaction completed - (TX DMA done)\n");
			complete_tx(dev, EP3_IN);
		}
		writel(ep_int_status, S3C_UDC_OTG_DIEPINT3); 		// ep2 Interrupt Clear
	}

}

static void process_ep_out_intr(struct s3c_udc * dev)
{
	u32 ep_int, ep_int_status, ep_ctrl;

	ep_int = readl(S3C_UDC_OTG_DAINT);
	DEBUG_OUT_EP("\tDAINT : 0x%x \n", ep_int);

	/* CONTROL OUT endpont */
	if (ep_int & ((1<<EP0_CON)<<16)) {
		ep_int_status = readl(S3C_UDC_OTG_DOEPINT0);
		ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL0);
		DEBUG_EP0("\tEP0-OUT : DOEPINT0 = 0x%x, DOEPCTL0 = 0x%x\n", ep_int_status, ep_ctrl);

		 if (ep_int_status & CTRL_OUT_EP_SETUP_PHASE_DONE) {

			DEBUG_EP0("\tSETUP packet(transaction) arrived\n");
			s3c_handle_ep0(dev);
			writel(ep_int_status & CTRL_OUT_EP_SETUP_PHASE_DONE, S3C_UDC_OTG_DOEPINT0);	// Interrupt Clear

		} else if (ep_int_status & TRANSFER_DONE) {

			DEBUG_EP0("\tEP0-OUT transaction completed - (RX DMA done)\n");
			complete_rx(dev, EP0_CON);
			writel((1 << 19)|sizeof(struct usb_ctrlrequest), S3C_UDC_OTG_DOEPTSIZ0);
			writel(ep_ctrl|DEPCTL_EPENA|DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);	// ep0 OUT enable, clear nak
			writel(ep_int_status & TRANSFER_DONE, S3C_UDC_OTG_DOEPINT0);	// Interrupt Clear

		} else {
			writel(ep_int_status, S3C_UDC_OTG_DOEPINT0);	// Interrupt Clear
		}

	} else if (ep_int & ((1<<EP1_OUT)<<16)) {/* EP1  - BULK OUT endpont */
		ep_int_status = readl(S3C_UDC_OTG_DOEPINT1);
		ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL1);

		DEBUG_OUT_EP("\tEP1-OUT : DOEPINT1 = 0x%x, DOEPCTL1 = 0x%x\n", ep_int_status, ep_ctrl);

		if (ep_int_status & TRANSFER_DONE) {
			DEBUG_OUT_EP("\tBULK OUT packet(transaction) arrived - (RX DMA done)\n");
			complete_rx(dev, EP1_OUT);
		}
		writel(ep_int_status, S3C_UDC_OTG_DOEPINT1); 		// ep1 Interrupt Clear

	}


}

/*
 *	elfin usb client interrupt handler.
 */
static irqreturn_t s3c_udc_irq(int irq, void *_dev)
{
	struct s3c_udc *dev = _dev;
	u32 intr_status;
	u32 usb_status, gintmsk;

	spin_lock(&dev->lock);

	intr_status = readl(S3C_UDC_OTG_GINTSTS);
	gintmsk = readl(S3C_UDC_OTG_GINTMSK);

	DEBUG_ISR("\n**** %s : GINTSTS=0x%x(on state %s), GINTMSK : 0x%x, DAINT : 0x%x, DAINTMSK : 0x%x\n",
			__FUNCTION__, intr_status, state_names[dev->ep0state], gintmsk,
			readl(S3C_UDC_OTG_DAINT), readl(S3C_UDC_OTG_DAINTMSK));

	if (!intr_status) {
		spin_unlock(&dev->lock);
		return IRQ_HANDLED;
	}

	if (intr_status & INT_ENUMDONE) {
		DEBUG_SETUP("####################################\n");
		DEBUG_SETUP("    %s: Speed Detection interrupt\n",
				__FUNCTION__);

		writel(INT_ENUMDONE, S3C_UDC_OTG_GINTSTS);
		usb_status = (readl(S3C_UDC_OTG_DSTS) & 0x6);

		if (usb_status & (USB_FULL_30_60MHZ | USB_FULL_48MHZ)) {
			DEBUG_SETUP("    %s: Full Speed Detection\n",__FUNCTION__);
			set_max_pktsize(dev, USB_SPEED_FULL);

		} else {
			DEBUG_SETUP("    %s: High Speed Detection : 0x%x\n", __FUNCTION__, usb_status);
			set_max_pktsize(dev, USB_SPEED_HIGH);
		}
	}

	if (intr_status & INT_EARLY_SUSPEND) {
		DEBUG_SETUP("####################################\n");
		DEBUG_SETUP("    %s:Early suspend interrupt\n", __FUNCTION__);
		writel(INT_EARLY_SUSPEND, S3C_UDC_OTG_GINTSTS);
	}

	if (intr_status & INT_SUSPEND) {
		usb_status = readl(S3C_UDC_OTG_DSTS);
		DEBUG_SETUP("####################################\n");
		DEBUG_SETUP("    %s:Suspend interrupt :(DSTS):0x%x\n", __FUNCTION__, usb_status);
		writel(INT_SUSPEND, S3C_UDC_OTG_GINTSTS);

		if (dev->gadget.speed != USB_SPEED_UNKNOWN
		    && dev->driver
		    && dev->driver->suspend) {

			dev->driver->suspend(&dev->gadget);
		}
	}

	if (intr_status & INT_RESUME) {
		DEBUG_SETUP("####################################\n");
		DEBUG_SETUP("    %s: Resume interrupt\n", __FUNCTION__);
		writel(INT_RESUME, S3C_UDC_OTG_GINTSTS);

		if (dev->gadget.speed != USB_SPEED_UNKNOWN
		    && dev->driver
		    && dev->driver->resume) {

			dev->driver->resume(&dev->gadget);
		}
	}

	if (intr_status & INT_RESET) {
		usb_status = readl(S3C_UDC_OTG_GOTGCTL);
		DEBUG_SETUP("####################################\n");
		DEBUG_SETUP("    %s: Reset interrupt - (GOTGCTL):0x%x\n", __FUNCTION__, usb_status);
		writel(INT_RESET, S3C_UDC_OTG_GINTSTS);

		if((usb_status & 0xc0000) == (0x3 << 18)) {
			if(reset_available) {
				DEBUG_SETUP("     ===> OTG core got reset (%d)!! \n", reset_available);
				reconfig_usbd();
				dev->ep0state = WAIT_FOR_SETUP;
				reset_available = 0;

				writel((1 << 19)|sizeof(struct usb_ctrlrequest), S3C_UDC_OTG_DOEPTSIZ0);
				writel(virt_to_phys(&usb_ctrl), S3C_UDC_OTG_DOEPDMA0);

				DEBUG_SETUP("%s : OTG_DOEPTSIZ0=0x%x, OTG_DOEPDMA0=0x%x, usb_ctrl=0x%p, size=%d\n",
					__FUNCTION__, readl(S3C_UDC_OTG_DOEPTSIZ0),
					readl(S3C_UDC_OTG_DOEPDMA0),&usb_ctrl, sizeof(struct usb_ctrlrequest));

				writel(DEPCTL_EPENA |DEPCTL_CNAK, S3C_UDC_OTG_DOEPCTL0);
			}
		} else {
			reset_available = 1;
			DEBUG_SETUP("      RESET handling skipped : reset_available : %d\n", reset_available);
		}
	}

	if (intr_status & INT_IN_EP) {
		DEBUG_IN_EP("####################################\n");
		DEBUG_IN_EP("    %s: EP In interrupt \n", __FUNCTION__);

		process_ep_in_intr(dev);
	}

	if(intr_status & INT_OUT_EP) {
		DEBUG_OUT_EP("####################################\n");
		DEBUG_OUT_EP("    %s: EP OUT interrupt \n", __FUNCTION__);

		process_ep_out_intr(dev);
	}

	spin_unlock(&dev->lock);

	return IRQ_HANDLED;
}

/** Queue one request
 *  Kickstart transfer if needed
 */
static int s3c_queue(struct usb_ep *_ep, struct usb_request *_req,
			 gfp_t gfp_flags)
{
	struct s3c_request *req;
	struct s3c_ep *ep;
	struct s3c_udc *dev;
	unsigned long flags;
	u32 ep_num;

	req = container_of(_req, struct s3c_request, req);
	if (unlikely(!_req || !_req->complete || !_req->buf || !list_empty(&req->queue))) {

		DEBUG("%s: bad params\n", __FUNCTION__);
		return -EINVAL;
	}

	ep = container_of(_ep, struct s3c_ep, ep);

	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name))) {

		DEBUG("%s: bad ep\n", __FUNCTION__);
		return -EINVAL;
	}

	ep_num = (u32)ep_index(ep);
	dev = ep->dev;
	if (unlikely(!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)) {

		DEBUG("%s: bogus device state %p\n", __FUNCTION__, dev->driver);
		return -ESHUTDOWN;
	}

	DEBUG("\n%s: %s queue req %p, len %d buf %p\n",
		__FUNCTION__,_ep->name, _req, _req->length, _req->buf);

	spin_lock_irqsave(&dev->lock, flags);

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	/* kickstart this i/o queue? */
	DEBUG("%s: Add to ep=%d, Q empty=%d, stopped=%d\n",
		__FUNCTION__, ep_num, list_empty(&ep->queue), ep->stopped);

	if (list_empty(&ep->queue) && !ep->stopped) {
		u32 csr;

		if (ep_num == 0) {
			/* EP0 */
			list_add_tail(&req->queue, &ep->queue);
			s3c_ep0_kick(dev, ep);
			req = 0;

		} else if (ep_num == EP2_IN || ep_num == EP3_IN) {
			csr = readl((u32) S3C_UDC_OTG_GINTSTS);
			DEBUG_IN_EP("%s: ep_is_in, S3C_UDC_OTG_GINTSTS=0x%x\n",
				__FUNCTION__, csr);

			setdma_tx(ep, req);

		} else {
			csr = readl((u32) S3C_UDC_OTG_GINTSTS);
			DEBUG_OUT_EP("%s: ep_is_out, S3C_UDC_OTG_GINTSTS=0x%x\n",
				__FUNCTION__, csr);

			setdma_rx(ep, req);
		}
	}

	/* pio or dma irq handler advances the queue. */
	if (likely(req != 0)) {
		list_add_tail(&req->queue, &ep->queue);
	}

	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

/****************************************************************/
/* End Point 0 related functions                                */
/****************************************************************/

/* return:  0 = still running, 1 = completed, negative = errno */
static int write_fifo_ep0(struct s3c_ep *ep, struct s3c_request *req)
{
	u32 max;
	unsigned count;
	int is_last;

	max = ep_maxpacket(ep);

	DEBUG_EP0("%s: max = %d\n", __FUNCTION__, max);

	count = setdma_tx(ep, req);

	/* last packet is usually short (or a zlp) */
	if (likely(count != max))
		is_last = 1;
	else {
		if (likely(req->req.length != req->req.actual) || req->req.zero)
			is_last = 0;
		else
			is_last = 1;
	}

	DEBUG_EP0("%s: wrote %s %d bytes%s %d left %p\n", __FUNCTION__,
		  ep->ep.name, count,
		  is_last ? "/L" : "", req->req.length - req->req.actual, req);

	/* requests complete when all IN data is in the FIFO */
	if (is_last) {
		ep->dev->ep0state = WAIT_FOR_SETUP;
		return 1;
	}

	return 0;
}

static __inline__ int s3c_fifo_read(struct s3c_ep *ep, u32 *cp, int max)
{
	u32 bytes;

	bytes = sizeof(struct usb_ctrlrequest);
	dma_cache_maint(&usb_ctrl, bytes, DMA_FROM_DEVICE);
	DEBUG_EP0("%s: bytes=%d, ep_index=%d \n", __FUNCTION__, bytes, ep_index(ep));

	return bytes;
}

/**
 * udc_set_address - set the USB address for this device
 * @address:
 *
 * Called from control endpoint function
 * after it decodes a set address setup packet.
 */
static void udc_set_address(struct s3c_udc *dev, unsigned char address)
{
	u32 ctrl = readl(S3C_UDC_OTG_DCFG);
	writel(address << 4 | ctrl, S3C_UDC_OTG_DCFG);
	writel((1 << 19)|(0<<0), S3C_UDC_OTG_DIEPTSIZ0);

	ctrl = readl(S3C_UDC_OTG_DIEPCTL0);
	writel(DEPCTL_EPENA|DEPCTL_CNAK|ctrl, S3C_UDC_OTG_DIEPCTL0); /* EP0: Control IN */

	DEBUG_EP0("%s: USB OTG 2.0 Device address=%d, DCFG=0x%x\n",
		__FUNCTION__, address, readl(S3C_UDC_OTG_DCFG));

	dev->usb_address = address;
}

static void s3c_ep0_read(struct s3c_udc *dev)
{
	struct s3c_request *req;
	struct s3c_ep *ep = &dev->ep[0];
	int ret;

	if (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct s3c_request, queue);

	} else {
		DEBUG("%s: ---> BUG\n", __FUNCTION__);
		BUG();	//logic ensures		-jassi
		return;
	}

	DEBUG_EP0("%s: req.length = 0x%x, req.actual = 0x%x\n",
		__FUNCTION__, req->req.length, req->req.actual);

	if(req->req.length == 0) {
		dev->ep0state = WAIT_FOR_SETUP;
		done(ep, req, 0);
		return;
	}

	ret = setdma_rx(ep, req);

	if (ret) {
		dev->ep0state = WAIT_FOR_SETUP;
		done(ep, req, 0);
		return;
	}

}

/*
 * DATA_STATE_XMIT
 */
static int s3c_ep0_write(struct s3c_udc *dev)
{
	struct s3c_request *req;
	struct s3c_ep *ep = &dev->ep[0];
	int ret, need_zlp = 0;

	DEBUG_EP0("%s: ep0 write\n", __FUNCTION__);

	if (list_empty(&ep->queue)) {
		req = 0;

	} else {
		req = list_entry(ep->queue.next, struct s3c_request, queue);
	}

	if (!req) {
		DEBUG_EP0("%s: NULL REQ\n", __FUNCTION__);
		return 0;
	}

	DEBUG_EP0("%s: req.length = 0x%x, req.actual = 0x%x\n",
		__FUNCTION__, req->req.length, req->req.actual);

	if (req->req.length == 0) {
		dev->ep0state = WAIT_FOR_SETUP;
		s3c_send_zlp();
	   	done(ep, req, 0);
		return 1;
	}

	if (req->req.length - req->req.actual == ep0_fifo_size) {
		/* Next write will end with the packet size, */
		/* so we need Zero-length-packet */
		need_zlp = 1;
	}

	ret = write_fifo_ep0(ep, req);

	if ((ret == 1) && !need_zlp) {
		/* Last packet */
		DEBUG_EP0("%s: finished, waiting for status\n", __FUNCTION__);
		dev->ep0state = WAIT_FOR_SETUP;

	} else {
		DEBUG_EP0("%s: not finished\n", __FUNCTION__);
		dev->ep0state = DATA_STATE_XMIT;
	}

	if (need_zlp) {
		DEBUG_EP0("%s: Need ZLP!\n", __FUNCTION__);
		dev->ep0state = DATA_STATE_NEED_ZLP;
	}

	if(ret) {
	 	done(ep, req, 0);
	}

	return 1;
}

u16	g_status;

/*
 * WAIT_FOR_SETUP (OUT_PKT_RDY)
 */
static void s3c_ep0_setup(struct s3c_udc *dev)
{
	struct s3c_ep *ep = &dev->ep[0];
	int i, bytes, is_in;
	u32 ep_ctrl, pktcnt;

	/* Nuke all previous transfers */
	nuke(ep, -EPROTO);

	/* read control req from fifo (8 bytes) */
	bytes = s3c_fifo_read(ep, (u32 *)&usb_ctrl, 8);

	DEBUG_SETUP("Read CTRL REQ %d bytes\n", bytes);
	DEBUG_SETUP("  CTRL.bRequestType = 0x%x (is_in %d)\n", usb_ctrl.bRequestType,
		    usb_ctrl.bRequestType & USB_DIR_IN);
	DEBUG_SETUP("  CTRL.bRequest = 0x%x\n", usb_ctrl.bRequest);
	DEBUG_SETUP("  CTRL.wLength = 0x%x\n", usb_ctrl.wLength);
	DEBUG_SETUP("  CTRL.wValue = 0x%x (%d)\n", usb_ctrl.wValue, usb_ctrl.wValue >> 8);
	DEBUG_SETUP("  CTRL.wIndex = 0x%x\n", usb_ctrl.wIndex);

	/* Set direction of EP0 */
	if (likely(usb_ctrl.bRequestType & USB_DIR_IN)) {
		ep->bEndpointAddress |= USB_DIR_IN;
		is_in = 1;

	} else {
		ep->bEndpointAddress &= ~USB_DIR_IN;
		is_in = 0;
	}

	dev->req_pending = 1;

	/* Handle some SETUP packets ourselves */
	switch (usb_ctrl.bRequest) {
		case USB_REQ_SET_ADDRESS:
			if (usb_ctrl.bRequestType
				!= (USB_TYPE_STANDARD | USB_RECIP_DEVICE))
				break;

			DEBUG_SETUP("%s: *** USB_REQ_SET_ADDRESS (%d)\n",
					__FUNCTION__, usb_ctrl.wValue);
			udc_set_address(dev, usb_ctrl.wValue);
			return;

		case USB_REQ_SET_CONFIGURATION :
			DEBUG_SETUP("============================================\n");
			DEBUG_SETUP("%s: USB_REQ_SET_CONFIGURATION (%d)\n",
					__FUNCTION__, usb_ctrl.wValue);
config_change:
			writel((1 << 19)|(0<<0), S3C_UDC_OTG_DIEPTSIZ0);

			ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL0);
			writel(DEPCTL_EPENA|DEPCTL_CNAK|ep_ctrl, S3C_UDC_OTG_DIEPCTL0); /* EP0: Control IN */

			// For Startng EP1 on this new configuration
			ep_ctrl = readl(S3C_UDC_OTG_DOEPCTL1);
			writel(DEPCTL_CNAK|DEPCTL_BULK_TYPE|DEPCTL_USBACTEP|ep_ctrl, S3C_UDC_OTG_DOEPCTL1); /* EP1: Bulk OUT */

			// For starting EP2 on this new configuration
			ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL2);
			writel(DEPCTL_CNAK|DEPCTL_BULK_TYPE|DEPCTL_USBACTEP|ep_ctrl, S3C_UDC_OTG_DIEPCTL2); /* EP2: Bulk IN */

			// For starting EP3 on this new configuration
			ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL3);
			writel(DEPCTL_CNAK|DEPCTL_BULK_TYPE|DEPCTL_USBACTEP|ep_ctrl, S3C_UDC_OTG_DIEPCTL3); /* EP3: INTR IN */

			DEBUG_SETUP("%s:(DOEPCTL1):0x%x, (DIEPCTL2):0x%x, (DIEPCTL3):0x%x\n",
				__FUNCTION__,
				readl(S3C_UDC_OTG_DOEPCTL1),
				readl(S3C_UDC_OTG_DIEPCTL2),
				readl(S3C_UDC_OTG_DIEPCTL3));

			DEBUG_SETUP("============================================\n");

			reset_available = 1;
			dev->req_config = 1;
			break;

		case USB_REQ_GET_DESCRIPTOR:
			DEBUG_SETUP("%s: *** USB_REQ_GET_DESCRIPTOR  \n",__FUNCTION__);
			break;

		case USB_REQ_SET_INTERFACE:
			DEBUG_SETUP("%s: *** USB_REQ_SET_INTERFACE (%d)\n",
					__FUNCTION__, usb_ctrl.wValue);
			goto config_change;
			break;

		case USB_REQ_GET_CONFIGURATION:
			DEBUG_SETUP("%s: *** USB_REQ_GET_CONFIGURATION  \n",__FUNCTION__);
			break;

		case USB_REQ_GET_STATUS:
			DEBUG_SETUP("%s: *** USB_REQ_GET_STATUS  \n",__FUNCTION__);

			if ((usb_ctrl.bRequestType & (USB_DIR_IN | USB_TYPE_MASK))
					!= (USB_DIR_IN | USB_TYPE_STANDARD)) {

				DEBUG_SETUP("%s: *** USB_REQ_GET_STATUS : delegated !!!  \n",__FUNCTION__);
					break;
			}

			g_status = __constant_cpu_to_le16(0);

			dma_cache_maint(&g_status, 2, DMA_TO_DEVICE);
			pktcnt = 1;

			writel(virt_to_phys(&g_status), S3C_UDC_OTG_DIEPDMA0);
			writel((pktcnt<<19)|(2<<0), (u32) S3C_UDC_OTG_DIEPTSIZ0);

			ep_ctrl = readl(S3C_UDC_OTG_DIEPCTL0);

			writel(DEPCTL_EPENA|DEPCTL_CNAK|(EP0_CON<<11), (u32) S3C_UDC_OTG_DIEPCTL0);

			dev->ep0state = WAIT_FOR_SETUP;
			return;
		default:
			DEBUG_SETUP("%s: *** Default of usb_ctrl.bRequest=0x%x happened.\n",
					__FUNCTION__, usb_ctrl.bRequest);
			break;
	}

	if (likely(dev->driver)) {
		/* device-2-host (IN) or no data setup command,
		 * process immediately */
		spin_unlock(&dev->lock);
		DEBUG_SETUP("%s: ctrlrequest will be passed to fsg_setup()\n", __FUNCTION__);
		i = dev->driver->setup(&dev->gadget, (struct usb_ctrlrequest *)&usb_ctrl);
		spin_lock(&dev->lock);

		if (i < 0) {
			/* setup processing failed, force stall */
			DEBUG_SETUP("%s: gadget setup FAILED (stalling), setup returned %d\n",
				__FUNCTION__, i);
			/* ep->stopped = 1; */
			dev->ep0state = WAIT_FOR_SETUP;
		}
	}
}

/*
 * handle ep0 interrupt
 */
static void s3c_handle_ep0(struct s3c_udc *dev)
{
	if (dev->ep0state == WAIT_FOR_SETUP) {
		DEBUG_EP0("%s: WAIT_FOR_SETUP\n", __FUNCTION__);
		s3c_ep0_setup(dev);

	} else {
		DEBUG_EP0("%s: strange state!!(state = %s)\n",
			__FUNCTION__, state_names[dev->ep0state]);
	}
}

static void s3c_ep0_kick(struct s3c_udc *dev, struct s3c_ep *ep)
{
	DEBUG_EP0("%s: ep_is_in = %d\n", __FUNCTION__, ep_is_in(ep));
	if (ep_is_in(ep)) {
		dev->ep0state = DATA_STATE_XMIT;
		s3c_ep0_write(dev);

	} else {
		dev->ep0state = DATA_STATE_RECV;
		s3c_ep0_read(dev);
	}
}
