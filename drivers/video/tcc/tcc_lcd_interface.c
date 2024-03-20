// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

/*****************************************************************************
 *
 * Header Files Include
 *
 ****************************************************************************/
#include <config.h>
#include <common.h>
#include <i2c.h>
#include <lcd.h>
#include <linux/io.h>
#include <asm/gpio.h>
#include <asm/telechips/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/telechips/vioc/reg_physical.h>
#include <asm/telechips/vioc/vioc_disp.h>
#include <asm/telechips/vioc/vioc_rdma.h>
#include <asm/telechips/vioc/vioc_wmix.h>
#include <asm/telechips/vioc/vioc_scaler.h>
#include <asm/telechips/vioc/vioc_config.h>

//#include "tcc_lcd.h"

#include <dm.h>
#include <display.h>

#define TCC8059_EVB 0
#define TCC8050_EVB 1

//extern void gpio_set(unsigned n, unsigned on);
static void lcd_delay_us(int us);

#define ASM_NOP				\
	{				\
		lcd_nop_count++;	\
	}

static unsigned int iBoardType = TCC8059_EVB;

static void lcd_delay_us(int us)
{
	int i = 0;
	unsigned int lcd_nop_count = 0;

	while (us >= 0) {
		for (i = 0; i < 20; i++) {
			if (lcd_nop_count < UINT_MAX)
				ASM_NOP
			else {
				lcd_nop_count = 0;
			}
		}
		us--;
	}
}

/*
 *Description : RGB LCD display port setting
 *DD_num : Display device block number
 *DP_num : Display port(GPIO) number {ex  (0: L0_Lxx) or  (1 :L1_Lxx)}
 *bit_per_pixle : bit per pixel
 */
#define CFG_MISC1 (HwVIOC_CONFIG + 0x84UL)
void LCDC_IO_Set(char DD_num, char DP_num, unsigned int bit_per_pixel)
{
	(void)DD_num;
	(void)DP_num;
	(void)bit_per_pixel;
#if 0
	BITCSET(*(unsigned int *)CFG_MISC1, 0x3 << (24 + (DP_num * 2)),
		DD_num << (24 + (DP_num * 2)));

	if (DP_num) {
		tcclcd_gpio_config(
			GPIO_L1_LPXCLK,
			GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW | GPIO_CD1);
		tcclcd_gpio_config(
			GPIO_L1_LHSYNC, GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
		tcclcd_gpio_config(
			GPIO_L1_LVSYNC, GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
		tcclcd_gpio_config(
			GPIO_L1_LACBIAS, GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
	} else {
		tcclcd_gpio_config(
			GPIO_L0_LPXCLK,
			GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW | GPIO_CD1);
		tcclcd_gpio_config(
			GPIO_L0_LHSYNC, GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
		tcclcd_gpio_config(
			GPIO_L0_LVSYNC, GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
		tcclcd_gpio_config(
			GPIO_L0_LACBIAS, GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
	}

	switch (bit_per_pixel) {
	case 24:
		for (i = 18; i < 24; i++) {
			if (DP_num)
				tcclcd_gpio_config(
					GPIO_L1_LPD(
						GPIO_L1_LPD_ADJUST_VALUE2 + i),
					GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
			else
				tcclcd_gpio_config(
					GPIO_L0_LPD(
						GPIO_L0_LPD_ADJUST_VALUE2 + i),
					GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
		}

	case 18:
		for (i = 16; i < 18; i++) {
			if (DP_num)
				tcclcd_gpio_config(
					GPIO_L1_LPD(
						GPIO_L1_LPD_ADJUST_VALUE2 + i),
					GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
			else
				tcclcd_gpio_config(
					GPIO_L0_LPD(
						GPIO_L0_LPD_ADJUST_VALUE2 + i),
					GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
		}

	case 16:
		for (i = 8; i < 16; i++) {
			if (DP_num)
				tcclcd_gpio_config(
					GPIO_L1_LPD(
						GPIO_L1_LPD_ADJUST_VALUE1 + i),
					GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
			else
				tcclcd_gpio_config(
					GPIO_L0_LPD(
						GPIO_L0_LPD_ADJUST_VALUE1 + i),
					GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
		}

	case 8:
		for (i = 0; i < 8; i++) {
			if (DP_num)
				tcclcd_gpio_config(
					GPIO_L1_LPD(
						GPIO_L1_LPD_ADJUST_VALUE1 + i),
					GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
			else
				tcclcd_gpio_config(
					GPIO_L0_LPD(
						GPIO_L0_LPD_ADJUST_VALUE1 + i),
					GPIO_FN1 | GPIO_OUTPUT | GPIO_LOW);
		}
		break;

	default:
		/* do nothing */
		break;
	}
#endif
}

/*
 *Description : RGB LCD display port disasble (set to normal GPIO)
 *DP_num : Display port(GPIO) number {ex  (0: L0_Lxx) or  (1 :L1_Lxx)}
 *bit_per_pixle : bit per pixel
 */
void LCDC_IO_Disable(char DP_num, unsigned int bit_per_pixel)
{
	(void)DP_num;
	(void)bit_per_pixel;
#if 0
	if (DP_num) {
		tcclcd_gpio_config(
			GPIO_L1_LPXCLK, GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
		tcclcd_gpio_config(
			GPIO_L1_LHSYNC, GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
		tcclcd_gpio_config(
			GPIO_L1_LVSYNC, GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
		tcclcd_gpio_config(
			GPIO_L1_LACBIAS, GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
	} else {
		tcclcd_gpio_config(
			GPIO_L0_LPXCLK, GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
		tcclcd_gpio_config(
			GPIO_L0_LHSYNC, GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
		tcclcd_gpio_config(
			GPIO_L0_LVSYNC, GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
		tcclcd_gpio_config(
			GPIO_L0_LACBIAS, GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
	}

	switch (bit_per_pixel) {
	case 24:
		for (i = 18; i < 24; i++) {
			if (DP_num)
				tcclcd_gpio_config(
					GPIO_L1_LPD(
						GPIO_L1_LPD_ADJUST_VALUE2 + i),
					GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
			else
				tcclcd_gpio_config(
					GPIO_L0_LPD(
						GPIO_L0_LPD_ADJUST_VALUE2 + i),
					GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
		}

	case 18:
		for (i = 16; i < 18; i++) {
			if (DP_num)
				tcclcd_gpio_config(
					GPIO_L1_LPD(
						GPIO_L1_LPD_ADJUST_VALUE2 + i),
					GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
			else
				tcclcd_gpio_config(
					GPIO_L0_LPD(
						GPIO_L0_LPD_ADJUST_VALUE2 + i),
					GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
		}

	case 16:
		for (i = 8; i < 16; i++) {
			if (DP_num)
				tcclcd_gpio_config(
					GPIO_L1_LPD(
						GPIO_L1_LPD_ADJUST_VALUE1 + i),
					GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
			else
				tcclcd_gpio_config(
					GPIO_L0_LPD(
						GPIO_L0_LPD_ADJUST_VALUE1 + i),
					GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
		}

	case 8:
		for (i = 0; i < 8; i++) {
			if (DP_num)
				tcclcd_gpio_config(
					GPIO_L1_LPD(
						GPIO_L1_LPD_ADJUST_VALUE1 + i),
					GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
			else
				tcclcd_gpio_config(
					GPIO_L0_LPD(
						GPIO_L0_LPD_ADJUST_VALUE1 + i),
					GPIO_FN0 | GPIO_OUTPUT | GPIO_LOW);
		}
		break;

	default:
		/* do nothing. */
		break;
	}
#endif
}

void lcdc_initialize(unsigned int lcdctrl_num, const struct vidinfo *lcd_spec)
{
	void __iomem *pDISPBase;
	void __iomem *pDISPReg;
	void __iomem *pWIXBase;
	unsigned int value;
	unsigned int ovp = 24;
	int ret = 0;

	/* Set the address for LCDC0 or LCDC1. */
	switch (lcdctrl_num) {
	case (unsigned int)PANEL_PATH_DISP0:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP0;
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pWIXBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_WMIX0;
		break;
	case (unsigned int)PANEL_PATH_DISP1:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP1;
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pWIXBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_WMIX1;
		break;
	case (unsigned int)PANEL_PATH_DISP2:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP2;
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pWIXBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_WMIX2;
		break;
	case (unsigned int)PANEL_PATH_DISP3:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP3;
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pWIXBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_WMIX3;
		break;
	default:
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("error in %s: Invaild lcdc number(%u)\n", __func__,
		       lcdctrl_num);
		ret = -1;
		break;
	}

	if (ret == 0) {

		/* save ovp for core reset */
		VIOC_WMIX_GetOverlayPriority(pWIXBase, &ovp);
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("%s : original ovp value = %u\n", __func__, ovp);
		if (ovp == 5U) { //if reset value
			ovp = 24U; //default ovp
		}

		VIOC_WMIX_SetSize(pWIXBase, lcd_spec->vl_col, lcd_spec->vl_row);
		VIOC_WMIX_SetOverlayPriority(pWIXBase, ovp);
#if defined(LCD_DISPLAY_Y2R_ENABLE)
		VIOC_WMIX_SetBGColor(pWIXBase, 0, 0x80, 0x80, 0);
#else
		VIOC_WMIX_SetBGColor(pWIXBase, 0, 0, 0, 0);
#endif

		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPReg = (void __iomem*)(uintptr_t)((unsigned long)pDISPBase + DCTRL);

		if (lcd_spec->bus_width == 24U) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			value = (__raw_readl(pDISPReg) & ~(DCTRL_PXDW_MASK));
			/* coverity[misra_c_2012_rule_12_2_violation : FALSE] */
			value |= (0xCU << DCTRL_PXDW_SHIFT);
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			__raw_writel(value, pDISPReg);
		} else if (lcd_spec->bus_width == 18U) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			value = (__raw_readl(pDISPReg) & ~(DCTRL_PXDW_MASK));
			/* coverity[misra_c_2012_rule_12_2_violation : FALSE] */
			value |= (0x5U << DCTRL_PXDW_SHIFT);
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			__raw_writel(value, pDISPReg);
		} else {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			value = (__raw_readl(pDISPReg) & ~(DCTRL_PXDW_MASK));
			/* coverity[misra_c_2012_rule_12_2_violation : FALSE] */
			value |= (0x3U << DCTRL_PXDW_SHIFT);
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			__raw_writel(value, pDISPReg);
		}

		if ((lcd_spec->sync_invert & ID_INVERT) != 0U) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			value = (__raw_readl(pDISPReg) & ~(DCTRL_ID_MASK));
			/* coverity[misra_c_2012_rule_12_2_violation : FALSE] */
			value |= (0x1U << DCTRL_ID_SHIFT);
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			__raw_writel(value, pDISPReg);
		}

		if ((lcd_spec->sync_invert & IV_INVERT) != 0U) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			value = (__raw_readl(pDISPReg) & ~(DCTRL_ID_MASK));
			/* coverity[misra_c_2012_rule_12_2_violation : FALSE] */
			value |= (0x1U << DCTRL_IV_SHIFT);
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			__raw_writel(value, pDISPReg);
		}

		if ((lcd_spec->sync_invert & IH_INVERT) != 0U) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			value = (__raw_readl(pDISPReg) & ~(DCTRL_ID_MASK));
			/* coverity[misra_c_2012_rule_12_2_violation : FALSE] */
			value |= (0x1U << DCTRL_IH_SHIFT);
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			__raw_writel(value, pDISPReg);
		}

		if ((lcd_spec->sync_invert & IP_INVERT) != 0U) {
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			value = (__raw_readl(pDISPReg) & ~(DCTRL_IP_MASK));
			/* coverity[misra_c_2012_rule_12_2_violation : FALSE] */
			value |= (0x1U << DCTRL_IP_SHIFT);
			/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
			__raw_writel(value, pDISPReg);
		}

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		value = (__raw_readl(pDISPReg) & ~(DCTRL_Y2R_MASK));
#if defined(LCD_DISPLAY_Y2R_ENABLE)
		value |= (0x1U << DCTRL_Y2R_SHIFT);
#endif
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		__raw_writel(value, pDISPReg);

		/* pDISPBase->uCTRL.bREG.CKG = 0; */ /* clock gating enable */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		value = (__raw_readl(pDISPReg) & ~(DCTRL_CKG_MASK));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		__raw_writel(value, pDISPReg);

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		value = (__raw_readl(pDISPReg) & ~(DCTRL_SREQ_MASK));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		__raw_writel(value, pDISPReg);

		/* pDISPBase->uCTRL.bREG.NI = 1; */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		value = (__raw_readl(pDISPReg) & ~(DCTRL_NI_MASK));
		value |= (0x1U << DCTRL_NI_SHIFT);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		__raw_writel(value, pDISPReg);

		/* Set Auto recovery */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		value = (__raw_readl(pDISPReg) & ~(DCTRL_SRST_MASK));
		value |= (0x1U << DCTRL_SRST_SHIFT);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		__raw_writel(value, pDISPReg);

		/* Set LCD clock */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		value = (__raw_readl(pDISPBase + DCLKDIV) & ~(DCLKDIV_PXCLKDIV_MASK));
		value |= (((uint32_t)lcd_spec->clk_div) << DCLKDIV_PXCLKDIV_SHIFT);

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DCLKDIV);

		/* Background color */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(0x00000000, pDISPBase + DBC);

		/* Horizontal timing */
		value = ((((uint32_t)lcd_spec->vl_col - 1U) << DHTIME1_LPC_SHIFT)
			 | (((uint32_t)lcd_spec->lpw - 1U) << DHTIME1_LPW_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DHTIME1);
		value = ((((uint32_t)lcd_spec->lswc - 1U) << DHTIME2_LSWC_SHIFT)
			 | (((uint32_t)lcd_spec->lewc - 1U) << DHTIME2_LEWC_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DHTIME2);

		/* Vertical timing */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = (__raw_readl(pDISPBase + DVTIME1)
			 & ~(DVTIME1_FPW_MASK | DVTIME1_FLC_MASK));
		value |= ((((uint32_t)lcd_spec->fpw1 - 1U) << DVTIME1_FPW_SHIFT)
			  | (((uint32_t)lcd_spec->vl_row - 1U) << DVTIME1_FLC_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DVTIME1);

		value = ((((uint32_t)lcd_spec->fswc1 - 1U) << DVTIME2_FSWC_SHIFT)
			 | (((uint32_t)lcd_spec->fewc1 - 1U) << DVTIME2_FEWC_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DVTIME2);

		value = ((((uint32_t)lcd_spec->fpw2 - 1U) << DVTIME3_FPW_SHIFT)
			 | (((uint32_t)lcd_spec->vl_row - 1U) << DVTIME3_FLC_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DVTIME3);

		value = ((((uint32_t)lcd_spec->fswc2 - 1U) << DVTIME4_FSWC_SHIFT)
			 | (((uint32_t)lcd_spec->fewc2 - 1U) << DVTIME4_FEWC_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DVTIME4);

		value = (((uint32_t)lcd_spec->vl_row << DDS_VSIZE_SHIFT)
			 | ((uint32_t)lcd_spec->vl_col << DDS_HSIZE_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DDS);

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = (__raw_readl(pDISPBase + DCTRL) & ~(DCTRL_LEN_MASK));
		value |= (0x1U << DCTRL_LEN_SHIFT);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DCTRL);
	}
}

void lcdc_deinitialize(unsigned int display_device_id)
{
	void __iomem *rdma;
	void __iomem *disp;
	unsigned long base_address;
	unsigned int disp_shift;

	base_address = TCC_VIOC_DISP_BASE(display_device_id);
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	disp = (void __iomem *)(uintptr_t)base_address;

	/* Disable RDMA */
	base_address = TCC_VIOC_RDMA_BASE(0U);
	disp_shift = display_device_id << 10U;
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	rdma = (void __iomem*)(uintptr_t)(base_address + disp_shift);
	VIOC_RDMA_SetImageDisable(rdma);

	/* Disable Display Device */
	VIOC_DISP_TurnOff(disp);
}

int lcdc_mux_select(unsigned int mux_id, unsigned int lcdc_id)
{
	unsigned int val;
	int ret = 0;
	unsigned int shift_lcd_sel = 0;
	unsigned int mask_lcd_sel = 0x3u;
	unsigned int set_lcd_sel = 0;
	/* coverity[cert_int36_c_violation : FALSE] */
	/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
	void __iomem* pcfg_misc1 = (void __iomem*)CFG_MISC1;

	if ((mux_id > (unsigned int)PANEL_LCD_MUX3) ||
				(lcdc_id > (unsigned int)PANEL_PATH_DISP3)) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Invalid params - lcd mux id(%u), display input(%u)\n",
		       __func__, __LINE__, mux_id, lcdc_id);
		ret = -1;
	}

	if (ret == 0) {
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		val = __raw_readl(pcfg_misc1);
		shift_lcd_sel = (mux_id * 2U) + 24U;
		mask_lcd_sel = mask_lcd_sel << shift_lcd_sel; //0b11

		//clear lcd_sel
		val &= ~mask_lcd_sel;

		set_lcd_sel = lcdc_id << shift_lcd_sel; //0b11
		val |= set_lcd_sel;

		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(val, pcfg_misc1);
	}

	return ret;
}

int lcdc_get_compatible_display(int rdma_id)
{
	int ret = 0;

	if ((rdma_id < 0) || (rdma_id > 13)) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("%s rdma id is invalid [%d]\n", __func__, rdma_id);
		ret = -EINVAL; //EINVAL
	}

	if (ret != -EINVAL) {
		if (rdma_id <= 3) {
			ret = 0;
		} else if (rdma_id <= 7) {
			ret = 1;
		} else if (rdma_id <= 11) {
			ret = 2;
		} else {//if ((rdma_id >= 12) && (rdma_id <= 13)) {
			ret = 3;
		}
	}

	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("%s : rdma_id = %d, compatible disp_id = %d\n",
		__func__, rdma_id, ret);


	return ret;
}

int lcdc_is_display_device_ready(int rdma_id)
{
	int disp_id = 0;
	int ret = 0;
	void __iomem *pDISPBase = NULL;

	disp_id = lcdc_get_compatible_display(rdma_id);

	if (disp_id == -EINVAL) {
		ret = -EINVAL;
	}

	if (ret == 0) {
		switch (disp_id) {
		case PANEL_PATH_DISP0:
			/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
			pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP0;
			break;
		case PANEL_PATH_DISP1:
			/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
			pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP1;
			break;
		case PANEL_PATH_DISP2:
			/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
			pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP2;
			break;
		case PANEL_PATH_DISP3:
			/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
			pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP3;
			break;
		default:
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err("[%s:%d]Error: Invalid params - display input(%d)\n",
			       __func__, __LINE__, disp_id);
			ret = -EINVAL;
			break;
		}
	}
	if (ret == 0) {
		ret = vioc_disp_get_turn_onoff(pDISPBase);
	}
	if (ret == 1) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("[INFO] %s : display device is running\n",
			 __func__);
	} else {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("[INFO] %s : display device is NOT running\n",
		       __func__);
	}

	return ret;
}

void tcc_lcdc_color_setting(
	uint8_t lcdc, signed char contrast, signed char brightness,
	signed char hue)
{
	void __iomem *pDISPBase;
	int ret = 0;

	switch (lcdc) {
	case (uint8_t)PANEL_PATH_DISP0:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP0;
		break;
	case (uint8_t)PANEL_PATH_DISP1:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP1;
		break;
	case (uint8_t)PANEL_PATH_DISP2:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP2;
		break;
	case (uint8_t)PANEL_PATH_DISP3:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP3;
		break;
	default:
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Invalid params - display input(%u)\n",
		       __func__, __LINE__, (unsigned int)lcdc);
		ret = -1;
		break;
	}

	if (ret == 0) {
		VIOC_DISP_SetColorEnhancement(pDISPBase, contrast, brightness, hue);

		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		pr_info("[%s:%d]lcdc:%u c :%d , b:%d h:%d\n", __func__,
			__LINE__, (unsigned int)lcdc,
			contrast, brightness, hue);
	}
}

void tcc_lcdc_dithering_setting(uint8_t lcdc)
{
	void __iomem *pDISPBase;
	unsigned int value;
	int ret = 0;

	switch (lcdc) {
	case (uint8_t)PANEL_PATH_DISP0:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP0;
		break;
	case (uint8_t)PANEL_PATH_DISP1:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP1;
		break;
	case (uint8_t)PANEL_PATH_DISP2:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP2;
		break;
	case (uint8_t)PANEL_PATH_DISP3:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pDISPBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_DISP3;
		break;
	default:
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Invalid params - display input(%u)\n",
		       __func__, __LINE__, (unsigned int)lcdc);
		ret = -1;
		break;
	}

	if (ret == 0) {
		/* dithering option */
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		value = (__raw_readl(pDISPBase + DCTRL) & ~(DCTRL_PXDW_MASK));
		/* coverity[misra_c_2012_rule_12_2_violation : FALSE] */
		value |= (0x5U << DCTRL_PXDW_SHIFT);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DCTRL);

		/* coverity[misra_c_2012_rule_12_2_violation : FALSE] */
		value = ((0x1U << DDITH_DEN_SHIFT) | (0x1U << DDITH_DSEL_SHIFT));
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(value, pDISPBase + DDITH);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(0x6e4ca280, pDISPBase + DDMAT0);
		/* coverity[misra_c_2012_rule_11_5_violation : FALSE] */
		/* coverity[misra_c_2012_rule_18_4_violation : FALSE] */
		__raw_writel(0x5d7f91b3, pDISPBase + DDMAT1);
	}
}

void tcclcd_image_ch_set(
	unsigned int lcdctrl_num, const struct tcc_lcdc_image_update *ImageInfo)
{
	void __iomem *pRDMA;
	void __iomem *pWIXBase;
	uint32_t base_addr0, base_addr1, base_addr2;
	int ret = 0;

	switch (lcdctrl_num) {
	case PANEL_PATH_DISP0:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pRDMA = (void __iomem *)(uintptr_t)((unsigned long)HwVIOC_RDMA00 +
					 ((unsigned long)ImageInfo->Lcdc_layer * RDMA_OFFSET));
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pWIXBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_WMIX0;
		break;
	case PANEL_PATH_DISP1:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pRDMA = (void __iomem *)(uintptr_t)((unsigned long)HwVIOC_RDMA04 +
					 ((unsigned long)ImageInfo->Lcdc_layer * RDMA_OFFSET));
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pWIXBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_WMIX1;
		break;
	case PANEL_PATH_DISP2:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pRDMA = (void __iomem *)(uintptr_t)(unsigned long)(HwVIOC_RDMA08 +
					 ((unsigned long)ImageInfo->Lcdc_layer * RDMA_OFFSET));
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pWIXBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_WMIX2;
		break;
	case PANEL_PATH_DISP3:
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pRDMA = (void __iomem *)(uintptr_t)(unsigned long)(HwVIOC_RDMA12 +
					 ((unsigned long)ImageInfo->Lcdc_layer * RDMA_OFFSET));
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pWIXBase = (void __iomem *)(uintptr_t)(unsigned long)HwVIOC_WMIX3;
		break;
	default:
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s:%d]Error: Invaild lcdc number(%u)\n", __func__,
		       __LINE__, lcdctrl_num);
		ret = -1;
		break;
	}

	if (ret == 0) {

		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		pr_info("[%s:%d] lcdc:%u ch:%u RDMA:0x%08lx\n", __func__, __LINE__,
			lcdctrl_num, ImageInfo->Lcdc_layer, (unsigned long)pRDMA);

		VIOC_RDMA_SetImageY2RMode(
					  pRDMA, 0); /* Y2RMode Default 0 (Studio Color) */

		if ((ImageInfo->fmt >= TCC_LCDC_IMG_FMT_YUV420SP)
		    && (ImageInfo->fmt <= TCC_LCDC_IMG_FMT_YUV422ITL1)) {
#if defined(LCD_DISPLAY_Y2R_ENABLE)
			VIOC_RDMA_SetImageR2YEnable(pRDMA, 0U);
			VIOC_RDMA_SetImageY2REnable(pRDMA, 0U);
#else
			VIOC_RDMA_SetImageR2YEnable(pRDMA, 0U);
			VIOC_RDMA_SetImageY2REnable(pRDMA, 1U);
#endif
		} else {
#if defined(LCD_DISPLAY_Y2R_ENABLE)
			VIOC_RDMA_SetImageR2YEnable(pRDMA, 1U);
			VIOC_RDMA_SetImageY2REnable(pRDMA, 0U);
#else
			VIOC_RDMA_SetImageR2YEnable(pRDMA, 0U);
			VIOC_RDMA_SetImageY2REnable(pRDMA, 0U);
#endif
		}

		VIOC_RDMA_SetImageOffset(pRDMA, (unsigned int)ImageInfo->fmt,
					 ImageInfo->Frame_width);
		VIOC_RDMA_SetImageFormat(pRDMA, (unsigned int)ImageInfo->fmt);
		VIOC_RDMA_SetImageSize(
				       pRDMA, ImageInfo->Image_width, ImageInfo->Image_height);

		/* image address */
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		base_addr0 = (uint32_t)((unsigned long)ImageInfo->addr0 & UINT_MAX);
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		base_addr1 = (uint32_t)((unsigned long)ImageInfo->addr1 & UINT_MAX);
		/* coverity[misra_c_2012_rule_11_6_violation : FALSE] */
		base_addr2 = (uint32_t)((unsigned long)ImageInfo->addr2 & UINT_MAX);
		VIOC_RDMA_SetImageBase(
				       pRDMA, base_addr0, base_addr1, base_addr2);

		/* Disable Alpha blend */
		VIOC_RDMA_SetImageAlphaEnable(pRDMA, 0);

		/* image position */
		VIOC_WMIX_SetPosition(
				      pWIXBase, ImageInfo->Lcdc_layer, ImageInfo->offset_x,
				      ImageInfo->offset_y);

		/* image enable */
		if (ImageInfo->enable != 0U) {
			VIOC_RDMA_SetImageEnable(pRDMA);
		} else {
			VIOC_RDMA_SetImageDisable(pRDMA);
		}

		VIOC_WMIX_SetUpdate(pWIXBase);
	}
}

void tcclcd_gpioexp_set_value(unsigned int on)
{
	struct udevice *pDEV_I2C_Dev;
	struct udevice *pstBus = NULL;
	unsigned char addr, data, val;
	int ret;
	int done = 0;

	addr = 0x5a;

	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &pstBus);
	if (ret != 0) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[%s][%d]failed to get i2c device class\n", __func__,
		       __LINE__);
		done = 1;
	}
	if (done == 0) {
		ret = dm_i2c_set_bus_speed(pstBus, 400000);
		if (ret != 0) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err("\n[%s:%d]Error: from dm_i2c_set_bus_speed().. Ret(0x%x)\n",
			       __func__, __LINE__, (unsigned int)ret);
			done = 1;
		}
	}
	if (done == 0) {
		ret = i2c_get_chip_for_busnum(0, (int)addr, 1, &pDEV_I2C_Dev);
		if (ret != 0) {
			/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
			/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
			/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
			pr_err("[%s][%d]failed to get i2c handle\n", __func__,
			       __LINE__);
			done = 1;
		}
	}

	if (done == 0) {
		(void)dm_i2c_read(pDEV_I2C_Dev, 0x1A, &val, 1);
		data = 0x6; // GPIO10_TYPE[2] GPIO10_PIN[1:0]
		val &= ~data;

		if (on != 0U) {
			val |= data;
		} else {
			val |= (unsigned char)0x1U;
		}
		(void)dm_i2c_write(pDEV_I2C_Dev, 0x1A, &val, 1);
		mdelay(1);

		(void)dm_i2c_read(pDEV_I2C_Dev, 0x1E, &val, 1);
		data = 0x04; // GPIO10_MODE[2]
		val &= ~data;

		if (on != 0U) {
			val |= data;
		}
		(void)dm_i2c_write(pDEV_I2C_Dev, 0x1E, &val, 1);
	}
}

void tcclcd_gpio_config(unsigned int n, unsigned int flags)
{
/*
 * gpio_config() is not defined yet. it needs to be done later
 */

#if 1
	if (n != GPIO_NC) {
		if (n < GPIO_EXT1) {
			(void)gpio_config(n, flags);
		}
	}
#endif
}

void tcclcd_gpio_set_value(unsigned int n, unsigned int on)
{
/*
 * gpio_config() is not defined yet. it needs to be done later
 */

#if 1
	if (n == GPIO_NC) {
		/* Not Connected */
	} else if (n == GPIO_EXP) {
		tcclcd_gpioexp_set_value(on);
		// temporarily disable code - making error (i2c_read, i2c_write)
	} else if (n >= GPIO_EXT1) {
		/* Extend gpio */
	} else {
		gpio_set(n, on);
	}
#endif
}

unsigned char tcclcd_get_board_type(void)
{
	return (uint8_t)(iBoardType & 0xFFU);
}

static int tcc_lcd_interface_probe(struct udevice *dev)
{
	u32 board_type;
	int ret = 0;

	ret = dev_read_u32(dev, "board-type", &board_type);
	/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
	/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
	/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
	/* coverity[misra_c_2012_rule_14_3_violation : FALSE] */
	pr_info("[%s] : lcd power port init. board_type = %u\n", __func__,
		board_type);
	if (ret < 0) {
		/* coverity[misra_c_2012_rule_20_7_violation : FALSE] */
		/* coverity[misra_c_2012_rule_21_6_violation : FALSE] */
		/* coverity[misra_c_2012_rule_12_1_violation : FALSE] */
		pr_err("[ERROR][LCD] %s: failed to get board type\n", __func__);
	} else {
		init_lcd_power(board_type);

		iBoardType = board_type;
	}

	return ret;
}

static const struct udevice_id tcc_lcd_interface_ids[] = {
	{.compatible = "telechips,tcc_lcd_interace"},
	{}
};

/* coverity[misra_c_2012_rule_21_2_violation : FALSE] */
/* coverity[misra_c_2012_rule_8_4_violation : FALSE] */
/* coverity[cert_dcl37_c_violation : FALSE] */
U_BOOT_DRIVER(tcc_lcd_interface) = {
	/* coverity[cert_str30_c_violation : FALSE] */
	.name = "tcc_lcd_interface",
	.id = UCLASS_DISPLAY,
	.of_match = tcc_lcd_interface_ids,
	.probe = tcc_lcd_interface_probe,
};
