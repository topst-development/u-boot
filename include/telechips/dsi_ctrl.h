// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef __TCC_DSI_CTRL_H__
#define __TCC_DSI_CTRL_H__

#include <telechips/fb_dm.h>
#include <asm/io.h>
#include <i2c.h>

#define TRUE 1
#define FALSE 0

#ifdef CONFIG_TCC807X

#define DSI0_CORE_ADDR 0x1C010000
#define DSI1_CORE_ADDR 0x1C020000
#define DSI0_PHY_ADDR 0x1C014000
#define DSI1_PHY_ADDR 0x1C024000

/*
 * Video stream type
 */
enum video_mode {
	VIDEO_NON_BURST_WITH_SYNC_PULSES = 0,
	VIDEO_NON_BURST_WITH_SYNC_EVENTS,
	VIDEO_BURST_WITH_SYNC_PULSES
};

enum operation_mode {
	IDLE_MODE,
	AUTO_CALC_MODE,
	COMMAND_MODE,
	VIDEO_MODE,
	DATA_STREAM_MODE,
};


struct dsih_core_main_t {
	unsigned int manual_mode_en;
	unsigned int to_hs_tx_timeout;
	unsigned int to_hs_tx_rdy_timeout;
	unsigned int to_lp_rx_timeout;
	unsigned int to_lp_rx_rdy_timeout;
	unsigned int to_lp_tx_trig_timeout;
	unsigned int to_lp_tx_ulps_timeout;
};

struct dsih_core_phy_t
{
	/* PHY interface */
	unsigned int phy_type;
	/* number of lanes used - from device tree*/
	unsigned int phy_lanes;
	/* number of bytes in the PPI interface */
	unsigned int ppi_width;
	/* Continuous clock or Non-continuous clock */
	unsigned int clk_type;
	/* Division factor for TX Escape clock to be generated from sys_clk */
	unsigned int phy_lptx_clk_div;
	/* Configures PHY transition time from low-power to high-speed transmission
	 * used in mannual mode
	 */
	unsigned int phy_lp2hs_time;
	/* Configures PHY transition time from high-speed to low-power transmission
	 * used in mannual mode
	 */
	unsigned int phy_hs2lp_time;
	/* Configures byte time for low-power data transmission */
	unsigned int phy_esc_byte_time;
	/*  Configures ratio between frequencies of HSTX clock and IPI clock */
	unsigned int phy_ipi_ratio;
	/* Ratio of frequencies phy_hstx_clk / sys_clk (manual) */
	unsigned int phy_sys_ratio;
	/* Time needed to complete deskew calibration, given in cycles of sys_clk */
	unsigned int phy_cal_time;
	/* Configures the PHY wakeup time that controller will consider when performing an ULPS exit request */
	unsigned int phy_wakeup_time;
};

struct dsih_core_dsi_t
{
	/* Enables Bus Turn Around (BTA) procedures.   */
	unsigned int bta_en;
	/* Enables the EoTp transmission in high-speed. */
	unsigned int eotp_tx_en;
	/* Scrambling enable.   */
	unsigned int scrambling_en;
	/* Configures the video mode transmission type. */
	unsigned int vid_mode_type;
};

struct dsih_core_ipi_t
{
	/* Configures the IPI color depth. */
	unsigned int ipi_depth;
	/* Configures the IPI pixel format. */
	unsigned int ipi_format;
	/* Configures the Horizontal Sync Active period measured in cycles of phy_hstx_clk */
	unsigned int vid_hsa_time;
	/* Configures the Horizontal Back Porch period measured in cycles of phy_hstx_clk. */
	unsigned int vid_hbp_time;
	/* Configures the Horizontal Front Porch period measured in cycles of phy_hstx_clk. */
	unsigned int vid_hfp_time;
	/* Configures the Horizontal Active period measured in cycles of phy_hstx_clk. */
	unsigned int vid_hact_time;
	/* Configures the total line time (HSA+HBP+HACT+HFP) measured in cycles of phy_hstx_clk. */
	unsigned int vid_hline_time;

	/* Configures the Vertical Sync Active period measured in lines.  */
	unsigned int vid_vsa_lines;
	/* Configures the Vertical Back Porch period measured in lines.  */
	unsigned int vid_vbp_lines;
	/* Configures the Vertical Front Active region period measured in lines.  */
	unsigned int vid_vact_lines;
	/* Configures the Vertical Front Porch period measured in lines.   */
	unsigned int vid_vfp_lines;
	/* Configures the number of pixels in a single video packet, 
	 * used in Video mode (for non-burst modes) and Data Stream mode.
	 */
	unsigned int max_pix_pkt;
};


/**
 * Main structures to instantiate the driver
 */
struct mipi_dsi_dev {

    /** HW version */
    uint32_t hw_version;
	unsigned int port;
	unsigned int automode;

	unsigned int num_of_dsi;
	unsigned int lane0;
	unsigned int lane1;
    /* MIPI DSI Controller */
    void __iomem *core_addr; // dsi register
    void __iomem *cfg_addr; // cfg register
    void __iomem *phy_addr; // cfg register

	// data_rate [Mhz]
    uint32_t data_rate;
	// pixel_clock (Khz)
    unsigned long pclk;
	// phy_hxtx_clk [Khz]
	unsigned int phy_hstx_clk;
	// sys_clk [Khz]
	unsigned int sys_clk;

	struct dsih_core_main_t main_cfg;
	struct dsih_core_phy_t phy_cfg;
	struct dsih_core_dsi_t dsi_cfg;
	struct dsih_core_ipi_t ipi_cfg;
};

#define CAM_CFG_REG		0x1C004000
#define	MIPI_DSI_CH_SEL_OFFSET	0x10

#define	S0_MUX_SEL_SHIFT	0
#define	S1_MUX_SEL_SHIFT	3

#define	TRVC_DIS_SHIFT	8
#define	SDM_DIS_SHIFT	9

int32_t dsi_enable_v2(struct mipi_dsi_dev *dev, const struct display_timing *timings);
#endif



#ifdef CONFIG_TCC750X
#define CAM_SWRST_REG	0x16004000
#define MIPI_SWRST_SHIFT	0

#define MIPI_PHY_M_RESETN 	2
#define MIPI_PHY_S_RESETN 	3


#define DSI_CORE_ADDR	0x16010000
#define CAM_CFG_ADDR	0x16004000


/** Version supported by this driver */
#define DSI_VERSION_130  0x3133302A
#define DSI_VERSION_131  0x3133312A
#define DSI_VERSION_140  0x3134302A

#define DSIH_PIXEL_TOLERANCE	(2)
/* no of tries to access the fifo */
#define DSIH_FIFO_ACTIVE_WAIT	(200)
#define DSIH_PHY_ACTIVE_WAIT	(50000)
/* a value to start PHY PLL - random */
#define DEFAULT_BYTE_CLOCK	(432000)
#define MAX_NULL_SIZE	(1023)
#define FIFO_DEPTH	(1096)
/* bytes (32bit registers) */
#define WORD_LENGTH	(4)

/*
 * Video stream type
 */
typedef enum {
	VIDEO_NON_BURST_WITH_SYNC_PULSES = 0,
	VIDEO_NON_BURST_WITH_SYNC_EVENTS,
	VIDEO_BURST_WITH_SYNC_PULSES
} dsih_video_mode_t;
/*
 * Color coding type (depth and pixel configuration)
 */
typedef enum {
	COLOR_CODE_16BIT_CONFIG1 = 0,
	COLOR_CODE_16BIT_CONFIG2 = 1,
	COLOR_CODE_16BIT_CONFIG3 = 2,
	COLOR_CODE_18BIT_CONFIG1 = 3,
	COLOR_CODE_18BIT_CONFIG2 = 4,
	COLOR_CODE_24BIT = 5,
	COLOR_CODE_20BIT_YCC422_LOOSELY = 6,
	COLOR_CODE_24BIT_YCC422 = 7,
	COLOR_CODE_16BIT_YCC422 = 8,
	COLOR_CODE_30BIT = 9,
	COLOR_CODE_36BIT = 10,
	COLOR_CODE_12BIT_YCC420 = 11,
	COLOR_CODE_DSC24 = 12,
	COLOR_CODE_MAX
} dsih_color_coding_t;

/**
 * Register configurations
 */
typedef struct {
	/** Register offset */
	uint32_t addr;
	/** Register data [in or out] */
	uint32_t data;
} register_config_t;

/**
 * MIPI D-PHY
 * Holds the mapping of API to the OS, logging I/O, and hardware access layer
 * and HW module information.
 */
typedef struct dphy_t {
	/**Number of lanes*/
	unsigned int lanes;
	/**output freq*/
	unsigned int output_freq;
	/** Reference frequency provided to PHY module [KHz] - REQUIRED */
	unsigned int reference_freq;
#ifdef GEN_3
	/* Is this DPHY G118?*/
	unsigned int is_g118;
#endif
} dphy_t;

/**
 * Video configurations
 * Holds information about the video stream to be sent through the DPI interface.
 */
typedef struct {
	/** Number of lanes used to send current video */
	unsigned int no_of_lanes;
	/** Virtual channel number to send this video stream */
	unsigned int virtual_channel;
	/** Video mode, whether burst with sync pulses, or packets with either sync pulses or events */
	dsih_video_mode_t video_mode;
	/** Maximum number of byte clock cycles needed by the PHY to transition
	 * the data lanes from high speed to low power - REQUIRED */
	unsigned int max_hs_to_lp_cycles;
	/** Maximum number of byte clock cycles needed by the PHY to transition
	 * the data lanes from low power to high speed - REQUIRED */
	unsigned int max_lp_to_hs_cycles;
	/** Maximum number of byte clock cycles needed by the PHY to transition
	 * the clock lane from high speed to low power - REQUIRED */
	unsigned int max_clk_hs_to_lp_cycles;
	/** Maximum number of byte clock cycles needed by the PHY to transition
	 * the clock lane from low power to high speed - REQUIRED */
	unsigned int max_clk_lp_to_hs_cycles;
	/** Enable non coninuous clock for energy saving
	 * - Clock lane will go to LS while not transmitting video */
	int non_continuous_clock;
	/** Enable receiving of ack packets */
	int receive_ack_packets;
	/** Byte (lane) clock [KHz] */
	unsigned long byte_clock;
	/** Pixel (DPI) Clock [KHz]*/
	unsigned long pixel_clock;
	/** Colour coding - BPP and Pixel configuration */
	dsih_color_coding_t color_coding;
	/** ser_des_mode */
	unsigned int ser_des_mode;
	/** Is 18-bit loosely packets (valid only when BPP == 18) */
	int is_18_loosely;
	/** Data enable signal (dpidaten) whether it is active high or low */
	int data_en_polarity;
	/** Horizontal synchronisation signal (dpihsync) whether it is active high or low */
	int h_polarity;
	/** Horizontal resolution or Active Pixels */
	unsigned int h_active_pixels;	/* hadr */
	/** Horizontal Sync Pixels - min 4 for best performance */
	unsigned int h_sync_pixels;
	/** Horizontal back porch pixels */
	unsigned int h_back_porch_pixels;	/* hbp */
	/** Total Horizontal pixels */
	unsigned int h_total_pixels;	/* h_total */
	/** Vertical synchronisation signal (dpivsync) whether it is active high or low */
	int v_polarity;
	/** Vertical active lines (resolution) */
	unsigned int v_active_lines;	/* vadr */
	/** Vertical sync lines */
	unsigned int v_sync_lines;
	/** Vertical back porch lines */
	unsigned int v_back_porch_lines;	/* vbp */
	/** Total no of vertical lines */
	unsigned int v_total_lines;	/* v_total */
	/** When set to 1, this bit enables the EoTp reception */
	int eotp_rx_en;
	/** When set to 1, this bit enables the EoTp transmission */
	int eotp_tx_en;
	/** This register configures the number of chunks to use */
	int no_of_chunks;
	/** This register configures the size of null packets */
	unsigned int null_packet_size;
    int dpi_lp_cmd_en;
} dsih_dpi_video_t;

typedef struct {
	/** Number of lanes used to send current video */
	unsigned int no_of_lanes;
	/** virtual channel */
	unsigned int virtual_channel;
	/** Commands to be sent in high speed or low power */
	int lp;
	/** Colour coding - BPP and Pixel configuration */
	dsih_color_coding_t color_coding;
	/** Top horizontal pixel position in the display */
	unsigned int h_start;
	/** Horizontal resolution or Active Pixels */
	unsigned int h_active_pixels;	/* hadr */
	/** Left most line position in the display */
	unsigned int v_start;
	/** Vertical active lines (resolution) */
	unsigned int v_active_lines;	/* vadr */
	/** Whether Tearing effect should be requested */
	int te;
	/** bta enable*/
	int bta;
	/** packet size of write memory command -
	 * 0 is default (optimum usage of RAM) */
	unsigned int packet_size;
	/** Byte (lane) clock [KHz] */
	unsigned int byte_clock;
	/** Pixel (DPI) Clock [KHz]*/
	unsigned long pixel_clock;
	/** Send setup packets */
	unsigned int send_setup_packets;
	/** Diplay type*/
	int display_type;

} dsih_cmd_mode_video_t;

/**
 * Main structures to instantiate the driver
 */
struct mipi_dsi_dev {

    /** HW version */
    uint32_t hw_version;

    /* timeout for FIFO full */
    int timeout;

    /* MIPI DSI Controller */
    void __iomem *core_addr; // dsi register
    void __iomem *cfg_addr; // cfg register
    dphy_t phy;

    /*
     * number of lanes physically connected to controller
     */
    uint8_t max_lanes;

	/*
	 * D-PHY lane Speed (Mhz)
	 */
    uint32_t data_rate;
    /*
     * maximum number of byte clock cycles needed by the PHY to perform
     * the Bus Turn Around operation - REQUIRED
     */
    uint16_t max_bta_cycles;

    /*
     * describe the color mode pin (dpicolorm) whether it is active
     * high or low - REQUIRED
     */
    int color_mode_polarity;

    /*
     * describe the shut down pin (dpishutdn) whether it is
     * active high or low - REQUIRED
     */
    int shut_down_polarity;

    dsih_dpi_video_t dpi_video;
    dsih_dpi_video_t dpi_video_old;
    dsih_cmd_mode_video_t cmd_mode_video;
    dsih_cmd_mode_video_t cmd_mode_video_old;
};

int32_t dsi_enable_v1(struct mipi_dsi_dev *dev, const struct display_timing *timings);
#endif
#endif
