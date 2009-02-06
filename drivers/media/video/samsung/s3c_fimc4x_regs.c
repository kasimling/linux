/* linux/drivers/media/video/samsung/s3c_fimc4x_regs.c
 *
 * Register interface file for Samsung Camera Interface (FIMC) driver
 *
 * Jinsung Yang, Copyright (c) 2009 Samsung Electronics
 * 	http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include <mach/map.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-bank-h3.h>
#include <plat/regs-fimc.h>
#include <plat/fimc.h>

#include "s3c_fimc.h"

void s3c_fimc_clear_irq(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIGCTRL);

	cfg |= S3C_CIGCTRL_IRQ_CLR;

	writel(cfg, ctrl->regs + S3C_CIGCTRL);
}

int s3c_fimc_check_fifo(struct s3c_fimc_control *ctrl)
{
	u32 cfg, status, flag;

	status = readl(ctrl->regs + S3C_CISTATUS);
	flag = S3C_CISTATUS_OVFIY | S3C_CISTATUS_OVFICB | S3C_CISTATUS_OVFICR;

	if (status & flag) {
		cfg = readl(ctrl->regs + S3C_CIWDOFST);
		cfg |= (S3C_CIWDOFST_CLROVFIY | S3C_CIWDOFST_CLROVFICB | S3C_CIWDOFST_CLROVFICR);
		writel(cfg, ctrl->regs + S3C_CIWDOFST);

		cfg = readl(ctrl->regs + S3C_CIWDOFST);
		cfg &= ~(S3C_CIWDOFST_CLROVFIY | S3C_CIWDOFST_CLROVFICB | S3C_CIWDOFST_CLROVFICR);
		writel(cfg, ctrl->regs + S3C_CIWDOFST);
	}

	return 0;
}

void s3c_fimc_select_camera(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIGCTRL);

	cfg &= ~S3C_CIGCTRL_SELCAM_ITU_MASK;

	if (ctrl->in_cam->id == 0)
		cfg |= S3C_CIGCTRL_SELCAM_ITU_A;
	else
		cfg |= S3C_CIGCTRL_SELCAM_ITU_B;

	writel(cfg, ctrl->regs + S3C_CIGCTRL);
}

void s3c_fimc_set_source_format(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_camera *cam = ctrl->in_cam;
	u32 cfg = 0;

	cfg |= (cam->mode | cam->order422);
	cfg |= S3C_CISRCFMT_SOURCEHSIZE(cam->width);
	cfg |= S3C_CISRCFMT_SOURCEVSIZE(cam->height);

	writel(cfg, ctrl->regs + S3C_CISRCFMT);
}

void s3c_fimc_set_window_offset(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_window_offset *offset = &ctrl->in_cam->offset;
	u32 cfg;
	
	cfg = readl(ctrl->regs + S3C_CIWDOFST);
	cfg &= ~(S3C_CIWDOFST_WINHOROFST_MASK | S3C_CIWDOFST_WINVEROFST_MASK);
	cfg |= S3C_CIWDOFST_WINHOROFST(offset->h1);
	cfg |= S3C_CIWDOFST_WINVEROFST(offset->v1);
	cfg |= S3C_CIWDOFST_WINOFSEN;
	writel(cfg, ctrl->regs + S3C_CIWDOFST);

	cfg = 0;
	cfg |= S3C_CIWDOFST2_WINHOROFST2(offset->h2);
	cfg |= S3C_CIWDOFST2_WINVEROFST2(offset->v2);
	writel(cfg, ctrl->regs + S3C_CIWDOFST2);
}

void s3c_fimc_reset(struct s3c_fimc_control *ctrl)
{
	u32 cfg;

	/*
	 * we have to write 1 to the CISRCFMT[31] before
	 * getting started the sw reset
	 *
	 */
	cfg = readl(ctrl->regs + S3C_CISRCFMT);
	cfg |= S3C_CISRCFMT_ITU601_8BIT;
	writel(cfg, ctrl->regs + S3C_CISRCFMT);

	/* s/w reset */
	cfg = readl(ctrl->regs + S3C_CIGCTRL);
	cfg |= (S3C_CIGCTRL_SWRST | S3C_CIGCTRL_IRQ_LEVEL);
	writel(cfg, ctrl->regs + S3C_CIGCTRL);
	mdelay(1);

	cfg = readl(ctrl->regs + S3C_CIGCTRL);
	cfg &= ~S3C_CIGCTRL_SWRST;
	writel(cfg, ctrl->regs + S3C_CIGCTRL);

	/* in case of ITU656, CISRCFMT[31] should be 0 */
	if (ctrl->in_cam && ctrl->in_cam->mode == ITU_656_YCBCR422_8BIT) {
		cfg = readl(ctrl->regs + S3C_CISRCFMT);
		cfg &= ~S3C_CISRCFMT_ITU601_8BIT;
		writel(cfg, ctrl->regs + S3C_CISRCFMT);
	}
}

void s3c_fimc_reset_camera(void)
{
	void __iomem *regs = ioremap(S5PC1XX_PA_FIMC0, SZ_4K);
	u32 cfg;

#if (CONFIG_VIDEO_FIMC_CAM_RESET == 1)
	cfg = readl(regs + S3C_CIGCTRL);
	cfg |= S3C_CIGCTRL_CAMRST_A;
	writel(cfg, regs + S3C_CIGCTRL);
	udelay(200);

	cfg = readl(regs + S3C_CIGCTRL);
	cfg &= ~S3C_CIGCTRL_CAMRST_A;
	writel(cfg, regs + S3C_CIGCTRL);
	udelay(2000);
#else
	cfg = readl(regs + S3C_CIGCTRL);
	cfg &= ~S3C_CIGCTRL_CAMRST_A;
	writel(cfg, regs + S3C_CIGCTRL);
	udelay(200);

	cfg = readl(regs + S3C_CIGCTRL);
	cfg |= S3C_CIGCTRL_CAMRST_A;
	writel(cfg, regs + S3C_CIGCTRL);
	udelay(2000);
#endif

#if (CONFIG_VIDEO_FIMC_CAM_CH == 1)
	cfg = readl(S5PC1XX_GPH3CON);
	cfg &= ~S5PC1XX_GPH3_CONMASK(6);
	cfg |= S5PC1XX_GPH3_OUTPUT(6);
	writel(cfg, S5PC1XX_GPH3CON);

	cfg = readl(S5PC1XX_GPH3DAT);
	cfg &= ~(0x1 << 6);
	writel(cfg, S5PC1XX_GPH3DAT);
	udelay(200);

	cfg |= (0x1 << 6);
	writel(cfg, S5PC1XX_GPH3DAT);
	udelay(2000);
#endif

	iounmap(regs);
}

void s3c_fimc_set_polarity(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_camera *cam = ctrl->in_cam;
	u32 cfg;

	cfg = readl(ctrl->regs + S3C_CIGCTRL);

	cfg &= ~(S3C_CIGCTRL_INVPOLPCLK | S3C_CIGCTRL_INVPOLVSYNC | \
		 S3C_CIGCTRL_INVPOLHREF | S3C_CIGCTRL_INVPOLHSYNC);

	if (cam->polarity.pclk)
		cfg |= S3C_CIGCTRL_INVPOLPCLK;

	if (cam->polarity.vsync)
		cfg |= S3C_CIGCTRL_INVPOLVSYNC;

	if (cam->polarity.href)
		cfg |= S3C_CIGCTRL_INVPOLHREF;

	if (cam->polarity.hsync)
		cfg |= S3C_CIGCTRL_INVPOLHSYNC;

	writel(cfg, ctrl->regs + S3C_CIGCTRL);
}

void s3c_fimc_set_target_format(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	u32 cfg = 0;

	switch (frame->format) {
	case FORMAT_RGB565: /* fall through */
	case FORMAT_RGB666: /* fall through */
	case FORMAT_RGB888:
		cfg |= S3C_CITRGFMT_OUTFORMAT_RGB;
		break;

	case FORMAT_YCBCR420:
		cfg |= S3C_CITRGFMT_OUTFORMAT_YCBCR420;
		break;

	case FORMAT_YCBCR422:
		if (frame->planes == 1)
			cfg |= S3C_CITRGFMT_OUTFORMAT_YCBCR422_1PLANE;
		else
			cfg |= S3C_CITRGFMT_OUTFORMAT_YCBCR422;

		break;
	}

	cfg |= S3C_CITRGFMT_TARGETHSIZE(frame->width);
	cfg |= S3C_CITRGFMT_TARGETVSIZE(frame->height);
	cfg |= (frame->flip << S3C_CITRGFMT_FLIP_SHIFT);

	if (ctrl->rot90) {
		cfg &= ~(S3C_CITRGFMT_INROT90_CLOCKWISE | \
			S3C_CITRGFMT_OUTROT90_CLOCKWISE);

		if (ctrl->out_type == PATH_OUT_DMA)
			cfg |= S3C_CITRGFMT_OUTROT90_CLOCKWISE;
		else if (ctrl->out_type == PATH_OUT_LCD_FIFO)
			cfg |= S3C_CITRGFMT_INROT90_CLOCKWISE;
	}

	writel(cfg, ctrl->regs + S3C_CITRGFMT);

	cfg = S3C_CITAREA_TARGET_AREA(frame->width * frame->height);
	writel(cfg, ctrl->regs + S3C_CITAREA);
}

static void s3c_fimc_set_output_dma_size(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	u32 cfg = 0;

	if (ctrl->rot90) {
		cfg |= S3C_ORGISIZE_HORIZONTAL(frame->height);
		cfg |= S3C_ORGISIZE_VERTICAL(frame->width);
	} else {
		cfg |= S3C_ORGISIZE_HORIZONTAL(frame->width);
		cfg |= S3C_ORGISIZE_VERTICAL(frame->height);
	}

	writel(cfg, ctrl->regs + S3C_ORGOSIZE);
}

void s3c_fimc_set_output_dma(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	u32 cfg;

	/* for offsets */
	cfg = 0;
	cfg |= S3C_CIOYOFF_HORIZONTAL(frame->offset.y_h);
	cfg |= S3C_CIOYOFF_VERTICAL(frame->offset.y_v);
	writel(cfg, ctrl->regs + S3C_CIOYOFF);

	cfg = 0;
	cfg |= S3C_CIOCBOFF_HORIZONTAL(frame->offset.cb_h);
	cfg |= S3C_CIOCBOFF_VERTICAL(frame->offset.cb_v);
	writel(cfg, ctrl->regs + S3C_CIOCBOFF);

	cfg = 0;
	cfg |= S3C_CIOCROFF_HORIZONTAL(frame->offset.cr_h);
	cfg |= S3C_CIOCROFF_VERTICAL(frame->offset.cr_v);
	writel(cfg, ctrl->regs + S3C_CIOCROFF);

	/* for original size */
	s3c_fimc_set_output_dma_size(ctrl);
	
	/* for output dma control */
	cfg = readl(ctrl->regs + S3C_CIOCTRL);

	cfg &= ~(S3C_CIOCTRL_ORDER2P_MASK | S3C_CIOCTRL_ORDER422_MASK | \
		 S3C_CIOCTRL_YCBCR_PLANE_MASK);

	if (frame->planes == 1)
		cfg |= frame->order_1p;
	else if (frame->planes == 2)
		cfg |= (S3C_CIOCTRL_YCBCR_2PLANE | \
			(frame->order_2p << S3C_CIOCTRL_ORDER2P_SHIFT));
	else if (frame->planes == 3)
		cfg |= S3C_CIOCTRL_YCBCR_3PLANE;

	writel(cfg, ctrl->regs + S3C_CIOCTRL);
}

void s3c_fimc_enable_lastirq(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIOCTRL);

	cfg |= S3C_CIOCTRL_LASTIRQ_ENABLE;
	writel(cfg, ctrl->regs + S3C_CIOCTRL);
}

void s3c_fimc_disable_lastirq(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIOCTRL);

	cfg &= ~S3C_CIOCTRL_LASTIRQ_ENABLE;
	writel(cfg, ctrl->regs + S3C_CIOCTRL);
}

void s3c_fimc_set_prescaler(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_scaler *sc = &ctrl->scaler;
	u32 cfg = 0, shfactor;

	shfactor = 10 - (sc->hfactor + sc->vfactor);

	cfg |= S3C_CISCPRERATIO_SHFACTOR(shfactor);
	cfg |= S3C_CISCPRERATIO_PREHORRATIO(sc->pre_hratio);
	cfg |= S3C_CISCPRERATIO_PREVERRATIO(sc->pre_vratio);

	writel(cfg, ctrl->regs + S3C_CISCPRERATIO);

	cfg = 0;
	cfg |= S3C_CISCPREDST_PREDSTWIDTH(sc->pre_dst_width);
	cfg |= S3C_CISCPREDST_PREDSTHEIGHT(sc->pre_dst_height);

	writel(cfg, ctrl->regs + S3C_CISCPREDST);
}

void s3c_fimc_set_scaler(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_scaler *sc = &ctrl->scaler;
	u32 cfg = (S3C_CISCCTRL_CSCR2Y_WIDE | S3C_CISCCTRL_CSCY2R_WIDE);

	if (sc->bypass)
		cfg |= S3C_CISCCTRL_SCALERBYPASS;

	if (sc->scaleup_h)
		cfg |= S3C_CISCCTRL_SCALEUP_H;

	if (sc->scaleup_v)
		cfg |= S3C_CISCCTRL_SCALEUP_V;

	if (ctrl->in_type == PATH_IN_DMA) {
		if (ctrl->in_frame->format == FORMAT_RGB565)
			cfg |= S3C_CISCCTRL_INRGB_FMT_RGB565;
		else if (ctrl->in_frame->format == FORMAT_RGB666)
			cfg |= S3C_CISCCTRL_INRGB_FMT_RGB666;
		else if (ctrl->in_frame->format == FORMAT_RGB888)
			cfg |= S3C_CISCCTRL_INRGB_FMT_RGB888;
	}

	if (ctrl->out_type == PATH_OUT_DMA) {
		if (ctrl->out_frame.format == FORMAT_RGB565)
			cfg |= S3C_CISCCTRL_OUTRGB_FMT_RGB565;
		else if (ctrl->out_frame.format == FORMAT_RGB666)
			cfg |= S3C_CISCCTRL_OUTRGB_FMT_RGB666;
		else if (ctrl->out_frame.format == FORMAT_RGB888)
			cfg |= S3C_CISCCTRL_OUTRGB_FMT_RGB888;
	} else {
		cfg |= S3C_CISCCTRL_LCDPATHEN_FIFO;

		if (ctrl->out_frame.scan == SCAN_TYPE_INTERLACE)
			cfg |= S3C_CISCCTRL_INTERLACE;
		else
			cfg |= S3C_CISCCTRL_PROGRESSIVE;
	}

	cfg |= S3C_CISCCTRL_MAINHORRATIO(sc->main_hratio);
	cfg |= S3C_CISCCTRL_MAINVERRATIO(sc->main_vratio);

	writel(cfg, ctrl->regs + S3C_CISCCTRL);	
}

void s3c_fimc_start_scaler(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CISCCTRL);

	cfg |= S3C_CISCCTRL_SCALERSTART;
	writel(cfg, ctrl->regs + S3C_CISCCTRL);
}

void s3c_fimc_stop_scaler(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CISCCTRL);

	cfg &= ~S3C_CISCCTRL_SCALERSTART;
	writel(cfg, ctrl->regs + S3C_CISCCTRL);
}

void s3c_fimc_enable_capture(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIIMGCPT);

	cfg &= ~S3C_CIIMGCPT_CPT_FREN_ENABLE;
	cfg |= (S3C_CIIMGCPT_IMGCPTEN | S3C_CIIMGCPT_IMGCPTEN_SC);
	writel(cfg, ctrl->regs + S3C_CIIMGCPT);
}

void s3c_fimc_disable_capture(struct s3c_fimc_control *ctrl)
{
	u32 cfg = readl(ctrl->regs + S3C_CIIMGCPT);

	cfg &= ~(S3C_CIIMGCPT_IMGCPTEN | S3C_CIIMGCPT_IMGCPTEN_SC);
	writel(cfg, ctrl->regs + S3C_CIIMGCPT);
}

void s3c_fimc_set_effect(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_effect *effect = &ctrl->out_frame.effect;
	u32 cfg = (S3C_CIIMGEFF_IE_ENABLE | S3C_CIIMGEFF_IE_SC_AFTER);

	cfg |= effect->type;

	if (effect->type == EFFECT_ARBITRARY) {
		cfg |= S3C_CIIMGEFF_PAT_CB(effect->pat_cb);
		cfg |= S3C_CIIMGEFF_PAT_CR(effect->pat_cr);
	}

	writel(cfg, ctrl->regs + S3C_CIIMGEFF);
}

void s3c_fimc_set_input_dma(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_in_frame *frame = ctrl->in_frame;
	u32 cfg;

	/* for offsets */
	cfg = 0;
	cfg |= S3C_CIIYOFF_HORIZONTAL(frame->offset.y_h);
	cfg |= S3C_CIIYOFF_VERTICAL(frame->offset.y_v);
	writel(cfg, ctrl->regs + S3C_CIIYOFF);

	cfg = 0;
	cfg |= S3C_CIICBOFF_HORIZONTAL(frame->offset.cb_h);
	cfg |= S3C_CIICBOFF_VERTICAL(frame->offset.cb_v);
	writel(cfg, ctrl->regs + S3C_CIICBOFF);

	cfg = 0;
	cfg |= S3C_CIICROFF_HORIZONTAL(frame->offset.cr_h);
	cfg |= S3C_CIICROFF_VERTICAL(frame->offset.cr_v);
	writel(cfg, ctrl->regs + S3C_CIICROFF);

	/* for original size */
	cfg = 0;
	cfg |= S3C_ORGISIZE_HORIZONTAL(frame->width);
	cfg |= S3C_ORGISIZE_VERTICAL(frame->height);
	writel(cfg, ctrl->regs + S3C_ORGISIZE);
	
	/* FIXME: for real size */
	cfg = 0;
	cfg |= S3C_CIREAL_ISIZE_WIDTH(frame->width - (frame->offset.y_h * 2));
	cfg |= S3C_CIREAL_ISIZE_HEIGHT(frame->height - (frame->offset.y_v * 2));
	writel(cfg, ctrl->regs + S3C_CIREAL_ISIZE);

	/* for input dma control */
	cfg = (S3C_MSCTRL_SUCCESSIVE_COUNT(4) | \
		S3C_MSCTRL_INPUT_MEMORY | S3C_MSCTRL_ENVID);

	switch (frame->format) {
	case FORMAT_RGB565: /* fall through */
	case FORMAT_RGB666: /* fall through */
	case FORMAT_RGB888:
		cfg |= S3C_MSCTRL_INFORMAT_RGB;
		break;

	case FORMAT_YCBCR420:
		cfg |= S3C_MSCTRL_INFORMAT_YCBCR420;

		if (frame->planes == 2)
			cfg |= (S3C_MSCTRL_C_INT_IN_2PLANE | \
				(frame->order_2p << S3C_MSCTRL_2PLANE_SHIFT));
		else
			cfg |= S3C_MSCTRL_C_INT_IN_3PLANE;

		break;

	case FORMAT_YCBCR422:
		if (frame->planes == 1)
			cfg |= (frame->order_1p | \
				S3C_MSCTRL_INFORMAT_YCBCR422_1PLANE);
		else {
			cfg |= S3C_MSCTRL_INFORMAT_YCBCR422;

			if (frame->planes == 2)
				cfg |= (S3C_MSCTRL_C_INT_IN_2PLANE | \
					(frame->order_2p << S3C_MSCTRL_2PLANE_SHIFT));
			else
				cfg |= S3C_MSCTRL_C_INT_IN_3PLANE;
		}

		break;
	}

	writel(cfg, ctrl->regs + S3C_MSCTRL);
}

void s3c_fimc_set_output_address(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_out_frame *frame = &ctrl->out_frame;
	struct s3c_fimc_frame_addr *addr;
	int i;

	for (i = 0; i < frame->nr_frames; i++) {
		addr = &frame->addr[i];
		writel(addr->phys_y, ctrl->regs + S3C_CIOYSA1 + (i * 4));
		writel(addr->phys_cb, ctrl->regs + S3C_CIOCBSA1 + (i * 4));
		writel(addr->phys_cr, ctrl->regs + S3C_CIOCRSA1 + (i * 4));
	}
}

int s3c_fimc_get_frame_count(struct s3c_fimc_control *ctrl)
{
	return S3C_CISTATUS_GET_FRAME_COUNT(readl(ctrl->regs + S3C_CISTATUS));
}

void s3c_fimc_change_effect(struct s3c_fimc_control *ctrl)
{
	struct s3c_fimc_effect *effect = &ctrl->out_frame.effect;
	u32 cfg = readl(ctrl->regs + S3C_CIIMGEFF);

	cfg &= ~S3C_CIIMGEFF_FIN_MASK;
	cfg |= (effect->type | S3C_CIIMGEFF_IE_ENABLE);

	if (effect->type == EFFECT_ARBITRARY) {
		cfg &= ~S3C_CIIMGEFF_PAT_CBCR_MASK;
		cfg |= S3C_CIIMGEFF_PAT_CB(effect->pat_cb);
		cfg |= S3C_CIIMGEFF_PAT_CR(effect->pat_cr);
	}

	writel(cfg, ctrl->regs + S3C_CIIMGEFF);
}

void s3c_fimc_change_rotate(struct s3c_fimc_control *ctrl)
{
	u32 cfg;

	cfg = readl(ctrl->regs + S3C_CITRGFMT);
	cfg &= ~(S3C_CITRGFMT_FLIP_MASK | S3C_CITRGFMT_OUTROT90_CLOCKWISE);
	cfg |= (ctrl->out_frame.flip << S3C_CITRGFMT_FLIP_SHIFT);

	if (ctrl->rot90) {
		cfg &= ~(S3C_CITRGFMT_INROT90_CLOCKWISE | \
				S3C_CITRGFMT_OUTROT90_CLOCKWISE);
		if (ctrl->out_type == PATH_OUT_DMA)
			cfg |= S3C_CITRGFMT_OUTROT90_CLOCKWISE;
		else if (ctrl->out_type == PATH_OUT_LCD_FIFO)
			cfg |= S3C_CITRGFMT_INROT90_CLOCKWISE;
	}

	writel(cfg, ctrl->regs + S3C_CITRGFMT);

	s3c_fimc_set_output_dma_size(ctrl);
}
