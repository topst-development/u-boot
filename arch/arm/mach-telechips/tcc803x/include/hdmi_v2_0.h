// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 - present Telechips and/or its affiliates.
 */
#ifndef __HDMI_V2_0__
#define __HDMI_V2_0__

#include <common.h>

// Type
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

// Type
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// CLOCK FREQ
#define HDMI_PHY_REF_CLK_RATE (24000000)
#define HDMI_HDCP14_CLK_RATE (50000000)
/**
 * DDC_SFRCLK is HDMI_HDCP14_CLK_RATE divided by 20000.
 */
#define HDMI_DDC_SFRCLK (HDMI_HDCP14_CLK_RATE / 20000)
#define HDMI_APB_CLK (50000000)
#define HDMI_HDCP22_CLK_RATE (50000000)
#define HDMI_SPDIF_REF_CLK_RATE (44100*64*2)

/* HDMI GPIO ALIAS */
#define GPIO_PORT_ALIAS_CEC TCC_GPC(13)
#define GPIO_PORT_ALIAS_HPD TCC_GPC(14)
#define GPIO_PORT_ALIAS_SDA TCC_GPC(9)
#define GPIO_PORT_ALIAS_SCL TCC_GPC(8)

#define GPIO_FN_ALIAS_CEC GPIO_FN1
#define GPIO_FN_ALIAS_HPD GPIO_FN1
#define GPIO_FN_ALIAS_SDA GPIO_FN1
#define GPIO_FN_ALIAS_SCL GPIO_FN1

/* Source Product Description Information */
#define HDMI_SPD_VENDOR_NAME "TCC"
#define HDMI_SPD_PRODUCT_DESCRPTION "TCC803x"
#define HDMI_SPD_SOURCE_INFORMATION 1 /* Digital-STB */

enum em_hdmi_ref {
	EM_HDMI_REF_PAD_XIN = 0,
	EM_HDMI_REF_PAD_HDMI_XTAL,
	EM_HDMI_REF_PAD_PERI_CLK,
	EM_HDMI_REF_PAD_PMU_XIN
};

// Detailed Timing Descriptor.
struct detailed_timing {
	/** VIC code */
	u32 mCode;

	/** Identifies modes that ONLY can be displayed in YCC 4:2:0 */
	u8 mLimitedToYcc420;

	/** Identifies modes that can also be displayed in YCC 4:2:0 */
	u8 mYcc420;

	u16 mPixelRepetitionInput;

	/** Hz * 1000 */
	u32 mPixelClock;

	/** 1 for interlaced, 0 progressive */
	u8 mInterlaced;

	u16 mHActive;

	u16 mHBlanking;

	u16 mHBorder;

	u16 mHImageSize;

	u16 mHSyncOffset;

	u16 mHSyncPulseWidth;

	/** 0 for Active low, 1 active high */
	u8 mHSyncPolarity;

	u16 mVActive;

	u16 mVBlanking;

	u16 mVBorder;

	u16 mVImageSize;

	u16 mVSyncOffset;

	u16 mVSyncPulseWidth;

	/** 0 for Active low, 1 active high */
	u8 mVSyncPolarity;
};

// Video Paramters
enum em_video_mode {
	MODE_UNDEFINED = -1,
	DVI = 0,
	HDMI
};

enum em_color_depth {
	COLOR_DEPTH_INVALID = 0,
	COLOR_DEPTH_8 = 8,
	COLOR_DEPTH_10 = 10,
	COLOR_DEPTH_12 = 12,
	COLOR_DEPTH_16 = 16
};

enum em_pixel_repetition {
	PIXEL_REPETITION_OFF = 0,
	PIXEL_REPETITION_1 = 1,
	PIXEL_REPETITION_2 = 2,
	PIXEL_REPETITION_3 = 3,
	PIXEL_REPETITION_4 = 4,
	PIXEL_REPETITION_5 = 5,
	PIXEL_REPETITION_6 = 6,
	PIXEL_REPETITION_7 = 7,
	PIXEL_REPETITION_8 = 8,
	PIXEL_REPETITION_9 = 9,
	PIXEL_REPETITION_10 = 10
};

enum em_operation_mode {
	HDMI_14 = 1,
	HDMI_20,
	MHL_24,
	MHL_PACKEDPIXEL
};

enum em_encoding {
	ENC_UNDEFINED = -1,
	RGB = 0,
	YCC444,
	YCC422,
	YCC420
};

enum em_colorimetry {
	ITU601 = 1,
	ITU709,
	EXTENDED_COLORIMETRY
};

enum em_ext_colorimetry {
	XV_YCC601 = 0,
	XV_YCC709,
	S_YCC601,
	ADOBE_YCC601,
	ADOBE_RGB,
	BT2020YCCBCR,
	BT2020YCBCR
};

struct video_params {
	enum em_video_mode mHdmi;
	enum em_encoding mEncodingOut;
	enum em_encoding mEncodingIn;
	u8 mColorResolution;
	u8 mPixelRepetitionFactor;
	struct detailed_timing mDtd;
	u8 mRgbQuantizationRange;
	u8 mPixelPackingDefaultPhase;
	u8 mColorimetry;
	u8 mScanInfo;
	u8 mActiveFormatAspectRatio;
	u8 mNonUniformScaling;
	enum em_ext_colorimetry mExtColorimetry;
	u8 mColorimetryDataBlock;
	u8 mItContent;
	u16 mEndTopBar;
	u16 mStartBottomBar;
	u16 mEndLeftBar;
	u16 mStartRightBar;
	u16 mCscFilter;
	u16 mCscA[4];
	u16 mCscC[4];
	u16 mCscB[4];
	u16 mCscScale;
	u8 mHdmiVideoFormat;
	u8 m3dStructure;
	u8 m3dExtData;
	u8 mHdmiVic;
	u8 mHdmi20;
};

struct lcdc_timimg_parms_t {
	unsigned int id;
	unsigned int iv;
	unsigned int ih;
	unsigned int ip;
	unsigned int dp;
	unsigned int ni;
	unsigned int tv;
	unsigned int tft;
	unsigned int stn;

	//LHTIME1
	unsigned int lpw;
	unsigned int lpc;
	//LHTIME2
	unsigned int lswc;
	unsigned int lewc;
	//LVTIME1
	unsigned int vdb;
	unsigned int vdf;
	unsigned int fpw;
	unsigned int flc;
	//LVTIME2
	unsigned int fswc;
	unsigned int fewc;
	//LVTIME3
	unsigned int fpw2;
	unsigned int flc2;
	//LVTIME4
	unsigned int fswc2;
	unsigned int fewc2;
};

struct HDMIVideoParameter {
};

struct HDMIAudioParameter {
};

struct fc_spd_info {
	const uint8_t *vName;
	uint8_t vLength;
	const uint8_t *pName;
	uint8_t pLength;
	uint8_t code;
	uint8_t autoSend;
};

/*
 * For detailed handling of this structure, refer to documentation of the
 * functions
 */
struct product_params {
	/* Vendor Name of eight 7-bit ASCII characters */
	uint8_t mVendorName[8];

	uint8_t mVendorNameLength;

	/*
	 * Product name or description, consists of sixteen 7-bit ASCII
	 * characters
	 */
	uint8_t mProductName[16];

	uint8_t mProductNameLength;

	/* Code that classifies the source device (CEA Table 15) */
	uint8_t mSourceType;

	/* oui 24 bit IEEE Registration Identifier */
	uint32_t mOUI;

	uint8_t mVendorPayload[24];

	uint8_t mVendorPayloadLength;
};

#if defined(CONFIG_HDMI_PANEL_MODE)
#define HDMI_COLOR_DEPTH	COLOR_DEPTH_8
#define HDMI_COLOR_SPACE	RGB
#else
#define HDMI_COLOR_DEPTH	COLOR_DEPTH_8
#define HDMI_COLOR_SPACE	YCC444
#endif

int dwc_hdmi_get_hpd(void);
void dwc_hdmi_set_ref_clock(enum em_hdmi_ref hdmi_ref);
void hdmi_ddi_config_init(unsigned int display_device);
void tcc_hdmi_set_enable(struct video_params *videoParams);
void tcc_hdmi_set_disable(void);
void tcc_hdmi_set_power_off(void);
#endif

