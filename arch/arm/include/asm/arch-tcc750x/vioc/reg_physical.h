/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef PLATFORM_REG_PHYSICAL_H
#define PLATFORM_REG_PHYSICAL_H

/*
 * VIOC register 4K align remap enable
 * -----------------------------------
 * The base address of the VIOC Component register is remapped according to
 * the VIOC_REMAP.REMAP_EN setting.
 *
 * VIOC_REMAP.REMAP_EN is set, the register address of the VIOC components
 * is remapped to 4K byte align.
 * This means that the address of the VIOC components is at least 4K byte apart.
 * Refer to VIOC Overall Register Map for details.
 */
#include <linux/bitops.h>
#include <asm/arch/vioc/vioc_ddicfg.h>

#if 0
#define is_VIOC_REMAP VIOC_DDICONFIG_GetViocRemap()
#else
#define is_VIOC_REMAP (0)
#endif

#if 0
static inline unsigned int is_VIOC_REMAP(void) {
	return VIOC_DDICONFIG_GetViocRemap();
}
#endif

struct TCC_DEF16BIT_IDX_TYPE {
	unsigned VALUE : 16;
};

union TCC_DEF16BIT_TYPE {
	unsigned short nREG;
	struct TCC_DEF16BIT_IDX_TYPE bREG;
};

struct TCC_DEF32BIT_IDX_TYPE {
	unsigned VALUE : 32;
};

union TCC_DEF32BIT_TYPE {
	unsigned long nREG;
	struct TCC_DEF32BIT_IDX_TYPE bREG;
};

/******************************************************************
 *
 *	TCC805X DataSheet PART 7 DISPLAY BUS
 *
 ******************************************************************/
#define HwVIOC_BASE (unsigned long)(0x17000000U)

/* DISP */
#define DISP_OFFSET (0x1000UL)
#define HwVIOC_DISP0 (HwVIOC_BASE + 0x00000UL)
#define TCC_VIOC_DISP_BASE(x) ((unsigned long)HwVIOC_DISP0 + ((unsigned long)DISP_OFFSET * (x)))

/* RDMA */
#define RDMA_OFFSET (0x1000UL)
#define HwVIOC_RDMA00 (HwVIOC_BASE + 0x010000UL)
#define HwVIOC_RDMA01 (HwVIOC_BASE + 0x011000UL)
#define HwVIOC_RDMA02 (HwVIOC_BASE + 0x012000UL)
#define HwVIOC_RDMA03 (HwVIOC_BASE + 0x013000UL)
#define HwVIOC_RDMA04 (HwVIOC_BASE + 0x014000UL)
#define HwVIOC_RDMA05 (HwVIOC_BASE + 0x015000UL)
#define HwVIOC_RDMA06 (HwVIOC_BASE + 0x016000UL)
#define TCC_VIOC_RDMA_BASE(x) (HwVIOC_RDMA00 + (RDMA_OFFSET * (x)))

/* WMIX */
#define WMIX_OFFSET (0x1000UL)
#define HwVIOC_WMIX0 (HwVIOC_BASE + 0x80000UL)
#define HwVIOC_WMIX1 (HwVIOC_BASE + 0x81000UL)
#define TCC_VIOC_WMIX_BASE(x) (HwVIOC_WMIX0 + (WMIX_OFFSET * (x)))

/* SCALER */
#define SC_OFFSET (0x1000UL)
#define HwVIOC_SC0 (HwVIOC_BASE + 0x60000UL)
#define HwVIOC_SC1 (HwVIOC_BASE + 0x61000UL)
#define HwVIOC_SC2 (HwVIOC_BASE + 0x62000UL)
#define TCC_VIOC_SC_BASE(x) (HwVIOC_SC0 + (SC_OFFSET * (x)))

/* WDMA */
#define WDMA_OFFSET (0x1000UL)
#define HwVIOC_WDMA00 (HwVIOC_BASE + 0x90000UL)
#define HwVIOC_WDMA01 (HwVIOC_BASE + 0x91000UL)
#define HwVIOC_WDMA02 (HwVIOC_BASE + 0x92000UL)
#define HwVIOC_WDMA03 (HwVIOC_BASE + 0x93000UL)
#define TCC_VIOC_WDMA_BASE(x) (HwVIOC_WDMA00 + (WDMA_OFFSET * (x)))

/* LUT */
#define HwVIOCLUT_BASE (HwVIOC_BASE + 0x070000UL)

/* FIFO */
#define HwVIOC_FIFO (HwVIOC_BASE + 0x0B2000UL)

/* CONFIG */
#define HwVIOC_CONFIG (HwVIOC_BASE + 0x0B0000UL)

#define HwVIOC_TIMER (HwVIOC_BASE + 0x0B1000UL)

// DISP Control Reg
#define HwDISP_EVP BIT(31)		 // External Vsync Polarity
#define HwDISP_EVS BIT(30)		 // External Vsync Enable
#define HwDISP_R2YMD (BIT(29) + BIT(28)) // RGB to YCbCr Conversion Option
#define HwDISP_FLDINV (BIT(27))		 // Field Output Polarity Register
#define HwDISP_ADVI BIT(26)		 // Advanced interlaced mode
#define HwDISP_656 BIT(24)		 // CCIR 656 Mode
#define HwDISP_CKG BIT(23)  // Clock Gating Enable for Timing Generator
#define HwDISP_SREQ BIT(22) // Stop Request
#define HwDISP_Y2RMD (BIT(21) + BIT(20)) // YUV to RGB converter mode register
#define HwDISP_PXDW (BIT(19) + BIT(18) + BIT(17) + BIT(16)) // PXDW
#define HwDISP_ID BIT(15)   // Inverted Data Enable
#define HwDISP_IV BIT(14)   // Inverted Vertical Sync
#define HwDISP_IH BIT(13)   // Inverted Horizontal Sync
#define HwDISP_IP BIT(12)   // Inverted Pixel Clock
#define HwDISP_CLEN BIT(11) // Clipping Enable
#define HwDISP_R2Y BIT(10)  // RGB to YCbCr Converter Enable for Output
#define HwDISP_DP BIT(9)    // Double Pixel Data
#define HwDISP_NI BIT(8)    // Non-Interlace
#define HwDISP_TV BIT(7)    // TV mode
#define HwDISP_SRST BIT(6)  // Device display reset
#define HwDISP_Y2R BIT(4)   // YUV to RGB converter enable register
#define HwDISP_SWAP (BIT(3) + BIT(2) + BIT(1)) // Output RGB overlay swap
#define HwDISP_LEN BIT(0)		       // LCD Controller Enable

// RDMA Control Reg
#define HwDMA_INTL BIT(31)	   // Interlaced Image
#define HwDMA_BFMD BIT(30)	   // Bfield mode
#define HwDMA_BF BIT(29)	     // Bottom field
#define HwDMA_IEN BIT(28)	    // Image Display Function for Each Image
#define HwDMA_STRM BIT(27)	   // streaming mode
#define HwDMA_3DMD BIT(26) + BIT(25) // 3D mode type
#define HwDMA_ASEL BIT(24)	   // Image Displaying Function for Each Image
#define HwDMA_UVI BIT(23)	    // UV ineterpolation
#define HwDMA_NUVI BIT(22)
#define HwDMA_R2YMD (BIT(19) + BIT(18)) // RGB to YUV converter mode register
#define HWDMA_R2Y BIT(17)		// RGB to YUV converter enable register
#define HwDMA_UPD BIT(16)		// data update enable
#define HwDMA_PD BIT(15)		// Bit padding
#define HwDMA_SWAP BIT(14) + BIT(13) + BIT(12) // RGB swap register
#define HwDMA_AEN BIT(11)		       // Alpha enable register
#define HwDMA_Y2RMD (BIT(10) + BIT(9))	 // YCbCr to RGB Conversion Option
#define HwDMA_Y2R BIT(8) // YCbCr to RGB Conversion Enable Bit
#define HwDMA_BR BIT(7)  // Bit Reverse
#define HwDMA_FMT (BIT(4) + BIT(3) + BIT(2) + BIT(1) + BIT(0)) // Image Format

// DISP status
#define HwLSTATUS_VS BIT(31)   // Monitoring vertical sync
#define HwLSTATUS_BUSY BIT(30) // Busy signal
#define HwLSTATUS_EF \
	BIT(29) // Even-Field(Read Only). 0:Odd field or frame, 1:Even field or
		// frame
#define HwLSTATUS_DEOF BIT(28) // DMA End of Frame flag

#define HwLSTATUS_SREQ BIT(5) // Device stop request
#define HwLSTATUS_DD BIT(4)   // Disable done
#define HwLSTATUS_RU BIT(3)   // Register update flag
#define HwLSTATUS_VSF BIT(2)  // VS falling flag
#define HwLSTATUS_VSR BIT(1)  // VS rising flag
#define HwLSTATUS_FU BIT(0)   // LCD output fifo under-run flag.

// Encoder Mode Control A
#define HwTVECMDA_PWDENC_PD \
	BIT(7) // Power down mode for entire digital logic of TV encoder
#define HwTVECMDA_FDRST_1 BIT(6) // Chroma is free running as compared to H-sync
#define HwTVECMDA_FDRST_0 \
	0 // Relationship between color burst & H-sync is maintained for video
	  // standards
#define HwTVECMDA_FSCSEL(X) ((X)*BIT(4))
#define HwTVECMDA_FSCSEL_NTSC \
	HwTVECMDA_FSCSEL(     \
		0) // Color subcarrier frequency is 3.57954545 MHz for NTSC
#define HwTVECMDA_FSCSEL_PALX \
	HwTVECMDA_FSCSEL(1) // Color subcarrier frequency is 4.43361875 MHz for
			    // PAL-B,D,G,H,I,N
#define HwTVECMDA_FSCSEL_PALM \
	HwTVECMDA_FSCSEL(     \
		2) // Color subcarrier frequency is 3.57561149 MHz for PAL-M
#define HwTVECMDA_FSCSEL_PALCN \
	HwTVECMDA_FSCSEL(3) // Color subcarrier frequency is 3.58205625 MHz for
			    // PAL-combination N
#define HwTVECMDA_FSCSEL_MASK HwTVECMDA_FSCSEL(3)
#define HwTVECMDA_PEDESTAL BIT(3)     // Video Output has a pedestal
#define HwTVECMDA_NO_PEDESTAL 0       // Video Output has no pedestal
#define HwTVECMDA_PIXEL_SQUARE BIT(2) // Input data is at square pixel rates.
#define HwTVECMDA_PIXEL_601 0	 // Input data is at 601 rates.
#define HwTVECMDA_IFMT_625 BIT(1)     // Output data has 625 lines
#define HwTVECMDA_IFMT_525 0	  // Output data has 525 lines
#define HwTVECMDA_PHALT_PAL BIT(0)    // PAL encoded chroma signal output
#define HwTVECMDA_PHALT_NTSC 0	// NTSC encoded chroma signal output

// Encoder Mode Control B
#define HwTVECMDB_YBIBLK_BLACK \
	BIT(4) // Video data is forced to Black level for Vertical non VBI
	       // processed lines.
#define HwTVECMDB_YBIBLK_BYPASS \
	0 // Input data is passed through forn non VBI processed lines.
#define HwTVECMDB_CBW(X) ((X)*BIT(2))
#define HwTVECMDB_CBW_LOW HwTVECMDB_CBW(0)    // Low Chroma band-width
#define HwTVECMDB_CBW_MEDIUM HwTVECMDB_CBW(1) // Medium Chroma band-width
#define HwTVECMDB_CBW_HIGH HwTVECMDB_CBW(2)   // High Chroma band-width
#define HwTVECMDB_CBW_MASK HwTVECMDB_CBW(3)   //
#define HwTVECMDB_YBW(X) ((X)*BIT(0))
#define HwTVECMDB_YBW_LOW HwTVECMDB_YBW(0)    // Low Luma band-width
#define HwTVECMDB_YBW_MEDIUM HwTVECMDB_YBW(1) // Medium Luma band-width
#define HwTVECMDB_YBW_HIGH HwTVECMDB_YBW(2)   // High Luma band-width
#define HwTVECMDB_YBW_MASK HwTVECMDB_YBW(3)   //

// Encoder Clock Generator
#define HwTVEGLK_XT24_24MHZ BIT(4)     // 24MHz Clock input
#define HwTVEGLK_XT24_27MHZ 0	  // 27MHz Clock input
#define HwTVEGLK_GLKEN_RST_EN BIT(3)   // Reset Genlock
#define HwTVEGLK_GLKEN_RST_DIS ~BIT(3) // Release Genlock
#define HwTVEGLK_GLKE(X) ((X)*BIT(1))
#define HwTVEGLK_GLKE_INT \
	HwTVEGLK_GLKE(0) // Chroma Fsc is generated from internal constants
			 // based on current user setting
#define HwTVEGLK_GLKE_RTCO \
	HwTVEGLK_GLKE(2) // Chroma Fsc is adjusted based on external RTCO input
#define HwTVEGLK_GLKE_CLKI \
	HwTVEGLK_GLKE(3) // Chroma Fsc tracks non standard encoder clock (CLKI)
			 // frequency
#define HwTVEGLK_GLKE_MASK HwTVEGLK_GLKE(3) //
#define HwTVEGLK_GLKEN_GLKPL_HIGH BIT(0)    // PAL ID polarity is active high
#define HwTVEGLK_GLKEN_GLKPL_LOW 0	  // PAL ID polarity is active low

// Encoder Mode Control C
#define HwTVECMDC_CSMDE_EN BIT(7) // Composite Sync mode enabled
#define HwTVECMDC_CSMDE_DIS \
	~BIT(7) // Composite Sync mode disabled (pin is tri-stated)
#define HwTVECMDC_CSMD(X) ((X)*BIT(5))
#define HwTVECMDC_CSMD_CSYNC \
	HwTVECMDC_CSMD(0) // CSYN pin is Composite sync signal
#define HwTVECMDC_CSMD_KEYCLAMP \
	HwTVECMDC_CSMD(1) // CSYN pin is Keyed clamp signal
#define HwTVECMDC_CSMD_KEYPULSE \
	HwTVECMDC_CSMD(2) // CSYN pin is Keyed pulse signal
#define HwTVECMDC_CSMD_MASK HwTVECMDC_CSMD(3)
#define HwTVECMDC_RGBSYNC(X) ((X)*BIT(3))
#define HwTVECMDC_RGBSYNC_NOSYNC \
	HwTVECMDC_RGBSYNC(0) // Disable RGBSYNC (when output is configured for
			     // analog EGB mode)
#define HwTVECMDC_RGBSYNC_RGB \
	HwTVECMDC_RGBSYNC(1) // Sync on RGB output signal (when output is
			     // configured for analog EGB mode)
#define HwTVECMDC_RGBSYNC_G \
	HwTVECMDC_RGBSYNC(2) // Sync on G output signal (when output is
			     // configured for analog EGB mode)
#define HwTVECMDC_RGBSYNC_MASK HwTVECMDC_RGBSYNC(3)

// DAC Output Selection
#define HwTVEDACSEL_DACSEL_CODE0 \
	0 // Data output is diabled (output is code '0')
#define HwTVEDACSEL_DACSEL_CVBS BIT(0) // Data output in CVBS format

// DAC Power Down
#define HwTVEDACPD_PD_EN BIT(0)   // DAC Power Down Enabled
#define HwTVEDACPD_PD_DIS ~BIT(0) // DAC Power Down Disabled

// Sync Control
#define HwTVEICNTL_FSIP_ODDHIGH BIT(7) // Odd field active high
#define HwTVEICNTL_FSIP_ODDLOW 0       // Odd field active low
#define HwTVEICNTL_VSIP_HIGH BIT(6)    // V-sync active high
#define HwTVEICNTL_VSIP_LOW 0	  // V-sync active low
#define HwTVEICNTL_HSIP_HIGH BIT(5)    // H-sync active high
#define HwTVEICNTL_HSIP_LOW 0	  // H-sync active low
#define HwTVEICNTL_HSVSP_RISING BIT(4) // H/V-sync latch enabled at rising edge
#define HwTVEICNTL_HVVSP_FALLING 0     // H/V-sync latch enabled at falling edge
#define HwTVEICNTL_VSMD_START \
	BIT(3) // Even/Odd field H/V sync output are aligned to video line start
#define HwTVEICNTL_VSMD_MID \
	0 // Even field H/V sync output are aligned to video line midpoint
#define HwTVEICNTL_ISYNC(X) ((X)*BIT(0))
#define HwTVEICNTL_ISYNC_FSI \
	HwTVEICNTL_ISYNC(0) // Alignment input format from FSI pin
#define HwTVEICNTL_ISYNC_HVFSI \
	HwTVEICNTL_ISYNC(1) // Alignment input format from HSI,VSI,FSI pin
#define HwTVEICNTL_ISYNC_HVSI \
	HwTVEICNTL_ISYNC(2) // Alignment input format from HSI,VSI pin
#define HwTVEICNTL_ISYNC_VFSI \
	HwTVEICNTL_ISYNC(3) // Alignment input format from VSI,FSI pin
#define HwTVEICNTL_ISYNC_VSI \
	HwTVEICNTL_ISYNC(4) // Alignment input format from VSI pin
#define HwTVEICNTL_ISYNC_ESAV_L \
	HwTVEICNTL_ISYNC(       \
		5) // Alignment input format from EAV,SAV codes (line by line)
#define HwTVEICNTL_ISYNC_ESAV_F \
	HwTVEICNTL_ISYNC(6) // Alignment input format from EAV,SAV codes (frame
			    // by frame)
#define HwTVEICNTL_ISYNC_FREE \
	HwTVEICNTL_ISYNC(7) // Alignment is free running (Master mode)
#define HwTVEICNTL_ISYNC_MASK HwTVEICNTL_ISYNC(7)

// Offset Control
#define HwTVEHVOFFST_INSEL(X) ((X)*BIT(6))
#define HwTVEHVOFFST_INSEL_BW16_27MHZ \
	HwTVEHVOFFST_INSEL(0) // 16bit YUV 4:2:2 sampled at 27MHz
#define HwTVEHVOFFST_INSEL_BW16_13P5MH \
	HwTVEHVOFFST_INSEL(1) // 16bit YUV 4:2:2 sampled at 13.5MHz
#define HwTVEHVOFFST_INSEL_BW8_13P5MHZ \
	HwTVEHVOFFST_INSEL(2) // 8bit YUV 4:2:2 sampled at 13.5MHz
#define HwTVEHVOFFST_INSEL_MASK HwTVEHVOFFST_INSEL(3)
#define HwTVEHVOFFST_VOFFST_256 \
	BIT(3) // Vertical offset bit 8 (Refer to HwTVEVOFFST)
#define HwTVEHVOFFST_HOFFST_1024 \
	BIT(2) // Horizontal offset bit 10 (Refer to HwTVEHOFFST)
#define HwTVEHVOFFST_HOFFST_512 \
	BIT(1) // Horizontal offset bit 9 (Refer to HwTVEHOFFST)
#define HwTVEHVOFFST_HOFFST_256 \
	BIT(0) // Horizontal offset bit 8 (Refer to HwTVEHOFFST)

// Sync Output Control
#define HwTVEHSVSO_VSOB_256 BIT(6)  // VSOB bit 8 (Refer to HwVSOB)
#define HwTVEHSVSO_HSOB_1024 BIT(5) // HSOB bit 10 (Refer to HwHSOB)
#define HwTVEHSVSO_HSOB_512 BIT(4)  // HSOB bit 9 (Refer to HwHSOB)
#define HwTVEHSVSO_HSOB_256 BIT(3)  // HSOB bit 8 (Refer to HwHSOB)
#define HwTVEHSVSO_HSOE_1024 BIT(2) // HSOE bit 10 (Refer to HwHSOE)
#define HwTVEHSVSO_HSOE_512 BIT(1)  // HSOE bit 9 (Refer to HwHSOE)
#define HwTVEHSVSO_HSOE_256 BIT(0)  // HSOE bit 8 (Refer to HwHSOE)

// Trailing Edge of Vertical Sync Control
#define HwTVEVSOE_VSOST(X) \
	((X)*BIT(6)) // Programs V-sync relative location for Odd/Even Fields.
#define HwTVEVSOE_NOVRST_EN BIT(5) // No vertical reset on every field
#define HwTVEVSOE_NOVRST_NORMAL \
	0 // Normal vertical reset operation (interlaced output timing)
#define HwTVEVSOE_VSOE(X) ((X)*BIT(0)) // Trailing Edge of Vertical Sync Control

// VBI Control Register
#define HwTVEVCTRL_VBICTL(X) \
	((X)*BIT(5)) // VBI Control indicating the current line is VBI.
#define HwTVEVCTRL_VBICTL_NONE \
	HwTVEVCTRL_VBICTL(0) // Do nothing, pass as active video.
#define HwTVEVCTRL_VBICTL_10LINE \
	HwTVEVCTRL_VBICTL(1) // Insert blank(Y:16, Cb,Cr: 128), for example, 10
			     // through 21st line.
#define HwTVEVCTRL_VBICTL_1LINE \
	HwTVEVCTRL_VBICTL(2) // Insert blank data 1 line less for CC processing.
#define HwTVEVCTRL_VBICTL_2LINE \
	HwTVEVCTRL_VBICTL(3) // Insert blank data 2 line less for CC and CGMS
			     // processing.
#define HwTVEVCTRL_MASK HwTVEVCTRL_VBICTL(3)
#define HwTVEVCTRL_CCOE_EN BIT(4) // Closed caption odd field enable.
#define HwTVEVCTRL_CCEE_EN BIT(3) // Closed caption even field enable.
#define HwTVEVCTRL_CGOE_EN \
	BIT(2) // Copy generation management system enable odd field.
#define HwTVEVCTRL_CGEE_EN \
	BIT(1) // Copy generation management system enable even field.
#define HwTVEVCTRL_WSSE_EN BIT(0) // Wide screen enable.

// Connection between LCDC & TVEncoder Control
#define HwTVEVENCON_EN_EN BIT(0) // Connection between LCDC & TVEncoder Enabled
#define HwTVEVENCON_EN_DIS \
	~BIT(0) // Connection between LCDC & TVEncoder Disabled

// I/F between LCDC & TVEncoder Selection
#define HwTVEVENCIF_MV_1 BIT(1) // reserved
#define HwTVEVENCIF_FMT_1 \
	BIT(0)		    // PXDATA[7:0] => CIN[7:0], PXDATA[15:8] => YIN[7:0]
#define HwTVEVENCIF_FMT_0 0 // PXDATA[7:0] => YIN[7:0], PXDATA[15:8] => CIN[7:0]

/************************************************************************
 *   DDI_CONFIG (Base Addr = 0x17400000) R/W
 *************************************************************************/

#define HwDDI_CONFIG_BASE (0x17400000U)

// Power Down
#define HwDDIC_PWDN_L2_LCLK BIT(18) // L2_LCLK(peri) Clock select
#define HwDDIC_PWDN_L1_LCLK BIT(17) // L1_LCLK(peri) Clock select
#define HwDDIC_PWDN_L0_LCLK BIT(16) // L0_LCLK(peri) Clock select

#define HwDDIC_PWDN_HDMI BIT(2) // HDMI
#define HwDDIC_PWDN_NTSC BIT(1) // NTSL/PAL
#define HwDDIC_PWDN_LCDC BIT(0) // VIOC

// Soft Reset
//#define HwDDIC_SWRESET_HDMI                  BIT(2)           // HDMI
//#define HwDDIC_SWRESET_NTSC                  BIT(1)              // NTSL/PAL
#define HwDDIC_SWRESET_ISP_AXI BIT(3) // ISP AXI
#define HwDDIC_SWRESET_DP_AXI BIT(2)  // DP AXI
#define HwDDIC_SWRESET_LCDC BIT(0)    // VIOC

#if 0
/**************************************************************************
 *
 *	TCC805X DataSheet PART 9 Cortex-M4 BUS
 *
 **************************************************************************/

#define HwCORTEXM4_CODE_MEM_BASE (0x19080000)
#define HwCORTEXM4_CODE_MEM_SIZE (0x00010000)
#define HwCORTEXM4_DATA_MEM_BASE (0x19090000)
#define HwCORTEXM4_DATA_MEM_SIZE (0x00010000)
#define HwCORTEXM4_MAILBOX0_BASE (0x19000000)
#define HwCORTEXM4_MAILBOX1_BASE (0x19010000)

#define HwCORTEXM4_TSD_CFG_BASE (0x19100000)
#endif

#endif /* _PLATFORM_REG_PHYSICAL_H_ */
