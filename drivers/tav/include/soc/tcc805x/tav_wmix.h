// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TAV_WMIX_H__
#define __TAV_WMIX_H__

struct TAV_WMIX_POS {
	unsigned int xpos;
	unsigned int ypos;
};

struct TAV_WMIX_KEY {
	/* 0: disable - default value, 1: enable - custom value */
	unsigned int key_en;
	unsigned int key_r;
	unsigned int key_g;
	unsigned int key_b;
	unsigned int mask_r;
	unsigned int mask_g;
	unsigned int mask_b;
};

struct TAV_WMIX_ALPAH_REGION {
	int valid; /* 0: default value, 1: custom value */
	/* MACON */
	/*
	 * alpha_sel0_out ACON0[0]
	 * 0 SEL0_Out = AlphaB
	 * 1 SEL0_Out = 256 – AlphaB
	 */
	unsigned int alpha_sel0_out;

	/*
	 * alpha_result_a ACON0[2:1]
	 * 0 Result_A = AlphaA * SEL0_Out
	 * 1 Result_A = AlphaA * 256
	 * 2 Result_A = AlphaA * 128
	 * 3 Result_A = 0
	 */
	unsigned int alpha_result_a;

	/*
	 * alpha_sel1_out ACON1[0]
	 * 0 SEL1_Out = AlphaA
	 * 1 SEL1_Out = 256 – AlphaA
	 */
	unsigned int alpha_sel1_out;

	/*
	 * alpha_result_a ACON1[2:1]
	 * 0 Result_B = AlphaB * SEL1_Out
	 * 1 Result_B = AlphaB * 256
	 * 2 Result_B = AlphaB * 128
	 * 3 Result_B = 0
	 */
	unsigned int alpha_result_b;

	/* MCCON */
	/*
	 * pixel_sel0_out CCON0[1:0]
	 * 0 SEL0_Out = AlphaA
	 * 1 SEL0_Out = AlphaB
	 * 2 SEL0_Out = 256 - AlphaA
	 * 3 SEL0_Out = 256 – AlphaB
	 */
	unsigned int pixel_sel0_out;

	/*
	 * pixel_result_a CCON0[3:2]
	 * 0 Result_A = PixelDataA * SEL0_Out
	 * 1 Result_A = PixelDataA * 256
	 * 2 Result_A = PixelDataA * 128
	 * 3 Result_A = 0
	 */
	unsigned int pixel_result_a;

	/*
	 * pixel_sel1_out CCON1[1:0]
	 * 0 SEL1_Out = AlphaA
	 * 1 SEL1_Out = AlphaB
	 * 2 SEL1_Out = 256 – AlphaA
	 * 3 SEL1_Out = 256 – AlphaB
	 */
	unsigned int pixel_sel1_out;

	/*
	 * pixel_result_b CCON1[3:2]
	 * 0 Result_B = PixelDataB * SEL1_Out
	 * 1 Result_B = PixelDataB * 256
	 * 2 Result_B = PixelDataB * 128
	 * 3 Result_B = 0
	 */
	unsigned int pixel_result_b;
};

struct TAV_WMIX_MIX_ROP {
	int valid;

	/*
	 * ROP
	 * Default is 0x18
	 * ROPMODE[4:3] = 2 → Global Alpha
	 * ROPMODE[4:3] = 3 → Pixel Alpha
	 * ROPMODE[4:3] = 0 → Not defined
	 * Should be ROPMOD[2:0] = 0
	 */
	unsigned int ropmode;

	/*
	 * ASEL
	 * Default is 0x3
	 * 0: Image alpha → 0 ~ 255 (0% ~ 99.6%)
	 * 1: Not defined
	 * 2: Image alpha → 1 ~ 256 (0.39% ~ 100%)
	 * 3: Image alpha → 0 ~ 127, 129 ~ 256 (0% ~ 49.6%, 50.3% ~ 100%)
	 */
	unsigned int asel;
	unsigned int alpha0; /* Default is 0 */
	unsigned int alpha1; /* Default is 0 */
};

struct TAV_WMIX_MIX {
	struct TAV_WMIX_KEY key;
	struct TAV_WMIX_ALPAH_REGION region[4];
	struct TAV_WMIX_MIX_ROP rop;

};

struct TAV_TEST_CASE_WMIX {
	int id;
	/* CTRL */
	unsigned int ovp;
	unsigned int skip_swreset;

	/* [0] R, [1] G, [2]B, [3] A */
	unsigned int bg[4];

	unsigned int height;
	unsigned int width;
	struct TAV_WMIX_POS pos[4];
	struct TAV_WMIX_MIX mix[3]; /* L2 - L1 - L0 */
};

#endif	/*__TAV_WMIX_H__*/
