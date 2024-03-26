// SPDX-License-Identifier: GPL-2.0
/**
 * Copyright (C) 2018 Synopsys, Inc.
 *
 * @file dsih_api.c
 * @brief Synopsys MIPI DSI driver API
 * included as a part of Synopsys MIPI DSI Host controller driver
 *
 * @author Luis Oliveira <luis.oliveira@synopsys.com>
 * Modified by Telechips
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include "dsih_api.h"
#include "dsih_hal.h"
#include "dsih_dphy.h"

#define PRECISION_FACTOR 1000
#define VIDEO_PACKET_OVERHEAD 6 /* HEADER (4 bytes) + CRC (2 bytes) */
#define NULL_PACKET_OVERHEAD 6  /* HEADER (4 bytes) + CRC (2 bytes) */
#define SHORT_PACKET 4			/* HEADER (2 bytes) + CRC (2 bytes) */
#define BLANKING_PACKET 6		/* HEADER (4 bytes) + CRC (2 bytes) */

/*
 * Enable return to low power mode inside video periods when timing allows
 *
 * @param dev pointer to structure holding the DSI Host core information
 * @param hfp allow to return to lp inside horizontal front porch pixels
 * @param hbp allow to return to lp inside horizontal back porch pixels
 * @param vactive allow to return to lp inside vertical active lines
 * @param vfp allow to return to lp inside vertical front porch lines
 * @param vbp allow to return to lp inside vertical back porch lines
 * @param vsync allow to return to lp inside vertical sync lines
 */
void mipi_dsih_allow_return_to_lp(struct mipi_dsi_dev *dev,
	int hfp, int hbp, int vactive, int vfp, int vbp, int vsync)
{
	if (dev != NULL)
	{
		mipi_dsih_hal_dpi_lp_during_hfp(dev, hfp);
		mipi_dsih_hal_dpi_lp_during_hbp(dev, hbp);
		mipi_dsih_hal_dpi_lp_during_vactive(dev, vactive);
		mipi_dsih_hal_dpi_lp_during_vfp(dev, vfp);
		mipi_dsih_hal_dpi_lp_during_vbp(dev, vbp);
		mipi_dsih_hal_dpi_lp_during_vsync(dev, vsync);

		return;
	}
}
/*
 * Set DCS command packet transmission to low power
 *
 * @param dev pointer to structure holding the DSI Host core information
 * @param long_write command packets
 * @param short_write command packets with none and one parameters
 * @param short_read command packets with none parameters
 */
void mipi_dsih_dcs_cmd_lp_transmission(struct mipi_dsi_dev *dev,
									   int long_write, int short_write,
									   int short_read)
{
	if (dev != NULL)
	{
		mipi_dsih_hal_dcs_wr_tx_type(dev, 0, short_write);
		mipi_dsih_hal_dcs_wr_tx_type(dev, 1, short_write);
		/* long packet*/
		mipi_dsih_hal_dcs_wr_tx_type(dev, 3, long_write);
		mipi_dsih_hal_dcs_rd_tx_type(dev, 0, short_read);

		return;
	}
}
/*
 * Set Generic interface packet transmission to low power
 *
 * @param dev pointer to structure holding the DSI Host core information
 * @param long_write command packets
 * @param short_write command packets with none, one and two parameters
 * @param short_read command packets with none, one and two parameters
 */
void mipi_dsih_gen_cmd_lp_transmission(struct mipi_dsi_dev *dev,
									   int long_write, int short_write,
									   int short_read)
{
	if (dev != NULL)
	{
		mipi_dsih_hal_gen_wr_tx_type(dev, 0, short_write);
		mipi_dsih_hal_gen_wr_tx_type(dev, 1, short_write);
		mipi_dsih_hal_gen_wr_tx_type(dev, 2, short_write);
		/* long packet*/
		mipi_dsih_hal_gen_wr_tx_type(dev, 3, long_write);
		mipi_dsih_hal_gen_rd_tx_type(dev, 0, short_read);
		mipi_dsih_hal_gen_rd_tx_type(dev, 1, short_read);
		mipi_dsih_hal_gen_rd_tx_type(dev, 2, short_read);

		return;
	}
}
/* Packet handling
 *
 * Enable all receiving activities (applying a Bus Turn Around).
 *  - Disabling using this function will stop all acknowledges by the
 * peripherals and no interrupts from low-level protocol error reporting
 * will be able to rise.
 * - Enabling any receiving function (command or frame ACKs, ECC,
 * tear effect ACK or EoTp receiving) will enable this automatically,
 * but it must be EXPLICITLY be disabled to disabled all kinds of
 * receiving functionality.
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable or disable
 * @return error code
 */
int mipi_dsih_enable_rx(struct mipi_dsi_dev *dev, int enable)
{
	mipi_dsih_hal_bta_en(dev, enable);
	return TRUE;
}
/*
 * Enable command packet acknowledges by the peripherals
 *
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable or disable
 * @return error code
 */
int mipi_dsih_peripheral_ack(struct mipi_dsi_dev *dev, int enable)
{
	if (dev != NULL)
	{
		mipi_dsih_hal_cmd_ack_en(dev, enable);
		if (enable)
			mipi_dsih_hal_bta_en(dev, 1);
		return TRUE;
	}
	return -ENODEV;
}
/**
 * Enable tearing effect acknowledges by the peripherals (wait for TE)
 *
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable or disable
 * @return error code
 */
int mipi_dsih_tear_effect_ack(struct mipi_dsi_dev *dev, int enable)
{
	if (dev != NULL)
	{

		mipi_dsih_hal_tear_effect_ack_en(dev, enable);
		if (enable)
			mipi_dsih_hal_bta_en(dev, 1);
		return TRUE;
	}
	return -ENODEV;
}
/**
 * Enable the receiving of EoT packets at the end of LS transmission.
 *
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable or disable
 * @return error code
 */
int mipi_dsih_eotp_rx(struct mipi_dsi_dev *dev, int enable)
{
	if (dev != NULL)
	{

		mipi_dsih_hal_gen_eotp_rx_en(dev, enable);
		if (enable)
			mipi_dsih_hal_bta_en(dev, 1);
		return TRUE;
	}
	return -ENODEV;
}
/**
 * @short Enable the listening to ECC bytes. This allows for recovering from
 * 1 bit errors. To report ECC events, the ECC events should be registered
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable or disable
 * @return error code
 */
int mipi_dsih_ecc_rx(struct mipi_dsi_dev *dev, int enable)
{
	if (dev != NULL)
	{

		mipi_dsih_hal_gen_ecc_rx_en(dev, enable);
		if (enable)
			mipi_dsih_hal_bta_en(dev, 1);
		return TRUE;
	}
	return -ENODEV;
}
/**
 * @short Enable the sending of EoT (End of Transmission) packets at the end of HS
 * transmission. It was made optional in the DSI spec. for retro-compatibility.
 * @param dev pointer to structure holding the DSI Host core information
 * @param enable or disable
 * @return error code
 */
int mipi_dsih_eotp_tx(struct mipi_dsi_dev *dev, int enable)
{
	if (dev != NULL)
	{
		mipi_dsih_hal_gen_eotp_tx_en(dev, enable);
		return TRUE;
	}
	return -ENODEV;
}

void mipi_dsih_dump_videoparam(dsih_dpi_video_t *video_params)
{
	pr_err("======================\nDump video param\n======================\n");
	pr_err("| no_of_lanes %d\n", video_params->no_of_lanes);
	pr_err("| virtual_channel %d\n", video_params->virtual_channel);
	pr_err("| video_mode %d\n", (unsigned int)video_params->video_mode);
	pr_err("| max_hs_to_lp_cycles %d\n", video_params->max_hs_to_lp_cycles);
	pr_err("| max_lp_to_hs_cycles %d\n", video_params->max_lp_to_hs_cycles);
	pr_err("| max_clk_hs_to_lp_cycles %d\n", video_params->max_clk_hs_to_lp_cycles);
	pr_err("| max_clk_lp_to_hs_cycles %d\n", video_params->max_clk_lp_to_hs_cycles);
	pr_err("| non_continuous_clock %d\n", video_params->non_continuous_clock);
	pr_err("| receive_ack_packets %d\n", video_params->receive_ack_packets);
	pr_err("| byte_clock %luKhz\n", video_params->byte_clock);
	pr_err("| pixel_clock %luKhz\n", video_params->pixel_clock);
	pr_err("| color_coding %d\n", (unsigned int)video_params->color_coding);
	pr_err("| is_18_loosely %d\n", video_params->is_18_loosely);
	pr_err("| data_en_polarity %d\n", video_params->data_en_polarity);
	pr_err("| h_polarity %d\n", video_params->h_polarity);
	pr_err("| h_active_pixels %d\n", video_params->h_active_pixels);
	pr_err("| h_sync_pixels %d\n", video_params->h_sync_pixels);
	pr_err("| h_back_porch_pixels %d\n", video_params->h_back_porch_pixels);
	pr_err("| h_total_pixels %d\n", video_params->h_total_pixels);
	pr_err("| v_active_lines %d\n", video_params->v_active_lines);
	pr_err("| v_sync_lines %d\n", video_params->v_sync_lines);
	pr_err("| v_back_porch_lines %d\n", video_params->v_back_porch_lines);
	pr_err("| v_total_lines %d\n", video_params->v_total_lines);
	pr_err("| eotp_rx_en %d\n", video_params->eotp_rx_en);
	pr_err("| eotp_tx_en %d\n", video_params->eotp_tx_en);
	pr_err("| no_of_chunks %d\n", video_params->no_of_chunks);
	pr_err("| null_packet_size %d\n", video_params->null_packet_size);
	pr_err("| dpi_lp_cmd_en %d\n", video_params->dpi_lp_cmd_en);
	pr_err("======================\nDump done\n======================\n");
	return;
}
/**
 * Enable command mode
 * - This function shall be explicitly called before commands are send if they
 * are to be sent in command mode and not interlaced with video
 * - If video mode is ON, it will be turned off automatically
 * @param dev pointer to structure holding the DSI Host core information
 * @param en enable/disable
 */
void mipi_dsih_cmd_mode(struct mipi_dsi_dev *dev, int en)
{
	if (dev != NULL)
		mipi_dsih_hal_gen_cmd_mode_en(dev, en);
	else
		pr_err("%s:Device is null\n", __func__);
}

unsigned int mipi_dsih_dpi_get_hlbcc(unsigned int hcomponent, unsigned long pclk, unsigned int lane_mbps)
{
	unsigned long lbcc;
#if 1
	unsigned int frac;
	lbcc = hcomponent * lane_mbps * (1000 / 8);
	frac = lbcc % pclk;
	lbcc = lbcc / pclk;
	if(frac) {
		pr_err("round up\n");
			lbcc++;
	}
#else
	unsigned long t_pix;
	unsigned long t_byte;
	unsigned long lbclk;

	lbclk = lane_mbps / 8 * 1000;

	t_pix = 1000000000000 / pclk;
	t_byte = 1000000000000 / lbclk;

	t_pix *= 100000;
	t_byte *= 10;
	pr_err("\n\nT_PIXE : %ldns\nT_BYTE : %ldns\n", t_pix, t_byte);
	lbcc = (hcomponent * (t_pix / t_byte));
	pr_err("lbcc : %ld, remainder : %ld \n", lbcc, lbcc % 10000);
	if(lbcc % 10000) {
		lbcc+= 10000;
	}
	lbcc /= 10000;
#endif	
	return lbcc;
}

/**
 * @short Configure DPI video interface
 * @param dev pointer to structure holding the DSI Host core information
 * @param video_params pointer to video stream-to-send information
 * @return error code
 */
int mipi_dsih_dpi_video(struct mipi_dsi_dev *dev)
{
	int error = 0;
	unsigned int video_size = 0;
	unsigned int no_of_chunks = 0;
	unsigned int null_packet_size = 0;

	dsih_dpi_video_t *video_params;

	unsigned int lane_mbps;
	/* check DSI controller dev */
	if (dev == NULL)
		return -ENODEV;
	lane_mbps = dev->data_rate;
	video_params = &dev->dpi_video;

	if(lane_mbps >= 1500) { // SOC Guide
		mipi_dsih_hal_timeout_clock_division(dev, 1);	
		mipi_dsih_hal_tx_escape_division(dev, 0xF);
	} else {
		mipi_dsih_hal_timeout_clock_division(dev, 1);	
		mipi_dsih_hal_tx_escape_division(dev, 0x0);
	}
	video_size = video_params->h_active_pixels;

	mipi_dsih_hal_power(dev, 0); // Off power
	mipi_dsih_dphy_no_of_lanes(dev, video_params->no_of_lanes); // set no of lane

	mipi_dsih_hal_int_mask_0(dev, 0x0); // mask interrupt
	mipi_dsih_hal_int_mask_1(dev, 0x0); // mask interrupt

	if (video_params->receive_ack_packets)
	{
		/*
		 * if ACK is requested, enable BTA
		 * otherwise leave as is
		 */
		mipi_dsih_hal_bta_en(dev, 1);
	}

	mipi_dsih_hal_dpi_video_mode_en(dev, 1); // Video mode
	mipi_dsih_hal_dpi_color_coding(dev, video_params->color_coding); // COLOR_CODE_24BIT
	mipi_dsih_hal_dpi_lp_cmd_en(dev, 0); // disable lp command transmission
	mipi_dsih_hal_dpi_video_mode_type(dev, video_params->video_mode); // VIDEO_NON_BURST_WITH_SYNC_PULSES

#if 1
// TODO : horizotal Cal
	video_params->h_total_pixels = mipi_dsih_dpi_get_hlbcc(video_params->h_total_pixels, video_params->pixel_clock, lane_mbps);
	pr_err("lbcc for htotal : %d\n", video_params->h_total_pixels);
	//tmp = (video_params->h_total_pixels * video_params->byte_clock);
	//video_params->h_total_pixels = DIV_ROUND_UP(tmp, video_params->pixel_clock);
	
	video_params->h_sync_pixels = mipi_dsih_dpi_get_hlbcc(video_params->h_sync_pixels, video_params->pixel_clock, lane_mbps);
	//video_params->h_sync_pixels = (video_params->h_sync_pixels * video_params->byte_clock) / video_params->pixel_clock;
	pr_err("lbcc for hsync : %d\n", video_params->h_sync_pixels);
	
	video_params->h_back_porch_pixels = mipi_dsih_dpi_get_hlbcc(video_params->h_back_porch_pixels, video_params->pixel_clock, lane_mbps);
	//video_params->h_back_porch_pixels = (video_params->h_back_porch_pixels * video_params->byte_clock) / video_params->pixel_clock;
	pr_err("lbcc for hbp : %d\n", video_params->h_back_porch_pixels);
#else
	video_params->h_total_pixels *= 2;
	video_params->h_sync_pixels *= 2;
	video_params->h_back_porch_pixels *= 2;
#endif
	mipi_dsih_hal_dpi_hline(dev, video_params->h_total_pixels);
	mipi_dsih_hal_dpi_hsa(dev, video_params->h_sync_pixels);
	mipi_dsih_hal_dpi_hbp(dev, video_params->h_back_porch_pixels);

	mipi_dsih_hal_dpi_vactive(dev, video_params->v_active_lines);
	mipi_dsih_hal_dpi_vfp(dev,
			   video_params->v_total_lines - (video_params->v_back_porch_lines + video_params->v_sync_lines + video_params->v_active_lines));
	mipi_dsih_hal_dpi_vbp(dev, video_params->v_back_porch_lines);
	mipi_dsih_hal_dpi_vsync(dev, 3);

	mipi_dsih_hal_dpi_hsync_pol(dev, video_params->h_polarity); // 1 to active low
	mipi_dsih_hal_dpi_vsync_pol(dev, video_params->v_polarity); // 1 to active low
	mipi_dsih_hal_dpi_dataen_pol(dev, video_params->data_en_polarity);
	mipi_dsih_hal_dpi_color_mode_pol(dev, 0);
	mipi_dsih_hal_dpi_shut_down_pol(dev, 0);
	mipi_dsih_hal_gen_eotp_tx_en(dev, 1);
	mipi_dsih_hal_gen_eotp_rx_en(dev, 1);
	mipi_dsih_allow_return_to_lp(dev, 1, 1, 1, 1, 1, 1);

	mipi_dsih_hal_dpi_video_packet_size(dev, video_size);
	mipi_dsih_hal_dpi_chunks_no(dev, no_of_chunks);
	mipi_dsih_hal_dpi_null_packet_size(dev, null_packet_size);
	mipi_dsih_hal_dpi_video_vc(dev, video_params->virtual_channel);


	if(dev->data_rate >= 1500)
		__raw_writel(0x1, 0x160100cc); // skew calibration
	/* Pattern gen */
	mipi_dsih_hal_vpg_orientation_act(dev, 0);
	mipi_dsih_hal_enable_vpg_act(dev, 0);
	mipi_dsih_hal_activate_shadow_registers(dev, 1);
	mipi_dsih_hal_request_registers_change(dev);
	mipi_dsih_dphy_enable_hs_clk(dev, 1);
	mipi_dsih_hal_power(dev, 1);

	mipi_dsih_hal_int_mask_0(dev, 0xFFFFFFFF); // unmask interrupt
	mipi_dsih_hal_int_mask_1(dev, 0xFFFFFFFF); // unmask interrupt

	//mipi_dsih_dump_videoparam(video_params);
	return error;
}

/**
 * Enable video mode
 * - If command mode is ON, it will be turned off automatically
 * @param dev pointer to structure holding the DSI Host core information
 * @param en enable/disable
 */
void mipi_dsih_video_mode(struct mipi_dsi_dev *dev, int en)
{
	if (dev != NULL)
		mipi_dsih_hal_dpi_video_mode_en(dev, en);
	else
		pr_err("%s:Device is null\n", __func__);
}
/**
 * Get the current active mode
 * - 1 command mode
 * - 2 DPI video mode
 */
int mipi_dsih_active_mode(struct mipi_dsi_dev *dev)
{
	if (dev == NULL)
		return -ENODEV;

	if (mipi_dsih_hal_gen_is_cmd_mode(dev))
		return 1;
	else if (mipi_dsih_hal_dpi_is_video_mode(dev))
		return 2;

	return TRUE;
}
/**
 * Send a generic write command
 * @param dev pointer to structure holding the DSI Host core information
 * @param vc destination virtual channel
 * @param params byte-addressed array of command parameters
 * @param param_length length of the above array
 * @return error code
 * @note this function has an active delay to wait for the buffer to clear.
 * The delay is limited to DSIH_FIFO_ACTIVE_WAIT x register access time
 */
int mipi_dsih_gen_wr_cmd(struct mipi_dsi_dev *dev, unsigned int vc, unsigned int *params, unsigned int param_length)
{
	unsigned int data_type = 0;

	if (dev == NULL)
		return -ENODEV;

	switch (param_length)
	{
	case 0:
		data_type = 0x03;
		break;
	case 1:
		data_type = 0x13;
		break;
	case 2:
		data_type = 0x23;
		break;
	default:
		data_type = 0x29;
		break;
	}
	return mipi_dsih_gen_wr_packet(dev, vc, data_type, params, param_length);
}
/**
 * Send a packet on the generic interface
 * @param dev pointer to structure holding the DSI Host core information
 * @param vc destination virtual channel
 * @param data_type type of command, inserted in first byte of header
 * @param params byte array of command parameters
 * @param param_length length of the above array
 * @return error code
 * @note this function has an active delay to wait for the buffer to clear.
 * The delay is limited to:
 * (param_length / 4) x DSIH_FIFO_ACTIVE_WAIT x register access time
 * @note the controller restricts the sending of .
 * This function will not be able to send Null and Blanking packets due to
 *  controller restriction
 */
int mipi_dsih_gen_wr_packet(struct mipi_dsi_dev *dev, unsigned int vc, unsigned int data_type, unsigned int *params, unsigned int param_length)
{
	/* active delay iterator */
	int timeout = 0;
	/* iterators */
	int i = 0;
	int j = 0;
	/* holds padding bytes needed */
	int compliment_counter = 0;
	unsigned int *payload = 0;
	/* temporary variable to arrange bytes into words */
	unsigned int temp = 0;
	unsigned int word_count = 0;
	dsih_cmd_mode_video_t *video_params;
	video_params = &dev->cmd_mode_video;

	if (dev == NULL)
	{
		pr_err("Null device\n");
		return -ENODEV;
	}

	if (video_params->te == 1)
	{
		mipi_dsih_tear_effect_ack(dev, 1);
		pr_info("tear effect ack enabled!\n");
	}

	pr_info( "param_length: %d\n", param_length);

	if ((params == 0) && (param_length != 0))
	{ /* pointer NULL */
		pr_err( "null params\n");
		return FALSE;
	}

	if (param_length > 200)
	{
		pr_err( "param length too large");
		return FALSE;
	}

	if (param_length > 2)
	{

		/*
		 * LONG PACKET - write word count to header and the rest to payload
		 */
		payload = params + (2 * sizeof(params[0]));
		word_count = (params[1] << 8) | params[0];

		pr_info( "word count: 0x%X", word_count);

		if (word_count > 200)
		{
			pr_err( "word count too large");
			return FALSE;
		}

		if ((param_length - 2) < word_count)
		{
			pr_err(
					"sent > input payload. complemented with zeroes");
			compliment_counter = (param_length - 2) - word_count;
		}
		else if ((param_length - 2) > word_count)
		{
			pr_err(
					"overflow - input > sent. payload truncated");
		}

		for (i = 0; i < (param_length - 2); i += j)
		{
			temp = 0;
			for (j = 0; (j < 4) && ((j + i) < (param_length - 2)); j++)
			{
				/* temp = (payload[i + 3] << 24) | (payload[i + 2] << 16) | (payload[i + 1] << 8) | payload[i]; */
				temp |= payload[i + j] << (j * 8);
			}
			/* check if payload Tx fifo is not full */
			for (timeout = 0; timeout < DSIH_FIFO_ACTIVE_WAIT; timeout++)
			{
				if (mipi_dsih_hal_gen_packet_payload(dev, temp))
					break;
				mdelay(500);
			}
			if (!(timeout < DSIH_FIFO_ACTIVE_WAIT))
			{
				dev->timeout++;
				pr_err("timeout! %u\n", dev->timeout);
				return FALSE;
			}
		}
		/*
		 * If word count entered by the user more than actual
		 * parameters received fill with zeroes
		 *
		 * A fail safe mechanism, otherwise controller will
		 * want to send data from an empty buffer
		 */
		for (i = 0; i < compliment_counter; i++)
		{

			/* check if payload Tx fifo is not full */
			for (timeout = 0; timeout < DSIH_FIFO_ACTIVE_WAIT; timeout++)
			{
				if (!mipi_dsih_hal_gen_packet_payload(dev, 0x00))
					break;
			}
			if (!(timeout < DSIH_FIFO_ACTIVE_WAIT))
			{
				pr_err("timeout!!\n");
				return FALSE;
			}
		}
	}
	for (timeout = 0; timeout < DSIH_FIFO_ACTIVE_WAIT; timeout++)
	{
		/* check if payload Tx fifo is not full */
		if (!mipi_dsih_hal_gen_cmd_fifo_full(dev))
		{
			if (param_length == 0)
				return mipi_dsih_hal_gen_packet_header(dev, vc, data_type,
													   0x0, 0x0);
			else if (param_length == 1)
				return mipi_dsih_hal_gen_packet_header(dev, vc, data_type,
													   0x0, params[0]);
			else
				return mipi_dsih_hal_gen_packet_header(dev, vc, data_type,
													   params[1], params[0]);
			break;
		}
	}
	return TRUE;
}
/**
 * Send a DCS READ command to peripheral
 * function sets the packet data type automatically
 * @param dev pointer to structure holding the DSI Host core information
 * @param vc destination virtual channel
 * @param command DCS code
 * @param bytes_to_read no of bytes to read (expected to arrive at buffer)
 * @param read_buffer pointer to 8-bit array to hold the read buffer words
 * return read_buffer_length
 * @note this function has an active delay to wait for the buffer to clear.
 * The delay is limited to 2 x DSIH_FIFO_ACTIVE_WAIT
 * (waiting for command buffer, and waiting for receiving)
 * @note this function will enable BTA
 */
unsigned int mipi_dsih_dcs_rd_cmd(struct mipi_dsi_dev *dev, unsigned int vc, unsigned int command, unsigned int bytes_to_read, unsigned int *read_buffer)
{
	if (dev == NULL)
		return -ENODEV;

	switch (command)
	{
	case 0xA8:
	case 0xA1:
	case 0x45:
	case 0x3E:
	case 0x2E:
	case 0x0F:
	case 0x0E:
	case 0x0D:
	case 0x0C:
	case 0x0B:
	case 0x0A:
	case 0x08:
	case 0x07:
	case 0x06:
		/* COMMAND_TYPE 0x06 - DCS Read no params refer to DSI spec p.47 */
		return mipi_dsih_gen_rd_packet(dev, vc, 0x06, 0x0, command, bytes_to_read, read_buffer);
	default:
		pr_err( "invalid DCS command 0x%X", command);
		return TRUE;
	}
	return TRUE;
}
/**
 * Send Generic READ command to peripheral
 * - function sets the packet data type automatically
 * @param dev pointer to structure holding the DSI Host core information
 * @param vc destination virtual channel
 * @param params byte array of command parameters
 * @param param_length length of the above array
 * @param bytes_to_read no of bytes to read (expected to arrive at buffer)
 * @param read_buffer pointer to 8-bit array to hold the read buffer words
 * return read_buffer_length
 * @note this function has an active delay to wait for the buffer to clear.
 * The delay is limited to 2 x DSIH_FIFO_ACTIVE_WAIT
 * (waiting for command buffer, and waiting for receiving)
 * @note this function will enable BTA
 */
unsigned int mipi_dsih_gen_rd_cmd(struct mipi_dsi_dev *dev, unsigned int vc, unsigned int *params, unsigned int param_length, unsigned int bytes_to_read, unsigned int *read_buffer)
{
	unsigned int data_type = 0;

	if (dev == NULL)
		return -ENODEV;

	switch (param_length)
	{
	case 0:
		data_type = 0x04;
		return mipi_dsih_gen_rd_packet(dev, vc, data_type, 0x00, 0x00, bytes_to_read, read_buffer);
	case 1:
		data_type = 0x14;
		return mipi_dsih_gen_rd_packet(dev, vc, data_type, 0x00, params[0], bytes_to_read, read_buffer);
	case 2:
		data_type = 0x24;
		return mipi_dsih_gen_rd_packet(dev, vc, data_type, params[1], params[0], bytes_to_read, read_buffer);
	default:
		return TRUE;
	}
}

/*
 * Dump values stored in the DSI host core registers
 *
 * @param dev pointer to structure holding the DSI Host core information
 * @param all whether to dump all the registers, no register_config array need
 * be provided if dump is to standard IO
 * @param config array of register_config_t type where addresses and values are
 * stored
 * @param config_length the length of the config array
 * @return the number of the registers that were read
 */
unsigned int mipi_dsih_dump_register_configuration(struct mipi_dsi_dev *dev, int all,
											   register_config_t *config, unsigned int config_length)
{
	unsigned int current = 0;
	unsigned int count = 0;

	if (dev == NULL)
		return -ENODEV;

	if (all)
	{
		/* dump all registers */
		for (current = R_DSI_HOST_VERSION;
			 current <= R_DSI_HOST_VID_VACTIVE_LINES_ACT;
			 count++, current += (R_DSI_HOST_PWR_UP - R_DSI_HOST_VERSION))
		{

			if ((config_length == 0) || (config == 0) || count >= config_length)
			{
				/* no place to write - write to STD IO */
				pr_err("DSI 0x%X:0x%X\n", current, mipi_dsih_read_word(dev, current));
			}
			else
			{
				config[count].addr = current;
				config[count].data = mipi_dsih_read_word(dev, current);
			}
		}
	}
	else
	{
		if (config == 0)
		{
			pr_err( "invalid buffer");
			return FALSE;
		}
		else
		{
			for (count = 0; count < config_length; count++)
			{
				config[count].data =
					mipi_dsih_read_word(dev, config[count].addr);
			}
		}
	}
	return count;
}
/*
 * Write values to DSI host core registers
 *
 * @param dev pointer to structure holding the DSI Host core information
 * @param config array of register_config_t type where register addresses and
 * their new values are stored
 * @param config_length the length of the config array
 * @return the number of the registers that were written to
 */
unsigned int mipi_dsih_write_register_configuration(struct mipi_dsi_dev *dev,
												register_config_t *config, unsigned int config_length)
{
	unsigned int count = 0;

	if (dev == NULL)
		return -ENODEV;

	for (count = 0; count < config_length; count++)
		mipi_dsih_write_word(dev, config[count].addr, config[count].data);

	return count;
}

/*
 * Reset the DSI Host controller
 *
 * Sends a pulse to the shut down register
 * @param dev pointer to structure holding the DSI Host core information
 */
void mipi_dsih_reset_controller(struct mipi_dsi_dev *dev)
{
	mipi_dsih_hal_power(dev, 0);
	udelay(1);
	mipi_dsih_hal_power(dev, 1);
}
/*
 * Shutdown the DSI Host controller
 *
 * @param dev pointer to structure holding the DSI Host core information
 * @param shutdown (1) power up (0)
 */
void mipi_dsih_shutdown_controller(struct mipi_dsi_dev *dev, int shutdown)
{
	mipi_dsih_hal_power(dev, !shutdown);
}
/*
 * Reset the PHY module being controlled by the DSI Host controller
 *
 * Sends a pulse to the PPI reset signal
 * @param dev pointer to structure holding the DSI Host core information
 */
void mipi_dsih_reset_phy(struct mipi_dsi_dev *dev)
{
	mipi_dsih_dphy_reset(dev, 0);
	mipi_dsih_dphy_reset(dev, 1);
}
/**
 * Shutdown the PHY module being controlled by the DSI Host controller
 *
 * @param dev pointer to structure holding the DSI Host core information
 * @param shutdown (1) power up (0)
 */
void mipi_dsih_shutdown_phy(struct mipi_dsi_dev *dev, int shutdown)
{
	mipi_dsih_dphy_shutdown(dev, !shutdown);
}

/* PRESP Time outs */
/**
 * Timeout for peripheral (for controller to stay still) after LP data
 * transmission write requests
 * @param dev pointer to structure holding the DSI Host core information
 * @param no_of_byte_cycles period for which the DWC_mipi_dsi_host keeps the
 * link still, after sending a low power write operation. This period is
 * measured in cycles of lanebyteclk
 */
void mipi_dsih_presp_timeout_low_power_write(struct mipi_dsi_dev *dev,
											 unsigned int no_of_byte_cycles)
{
	mipi_dsih_hal_presp_timeout_low_power_write(dev, no_of_byte_cycles);
}
/**
 * Timeout for peripheral (for controller to stay still) after LP data
 * transmission read requests
 * @param dev pointer to structure holding the DSI Host core information
 * @param no_of_byte_cycles period for which the DWC_mipi_dsi_host keeps the
 * link still, after sending a low power read operation. This period is
 * measured in cycles of lanebyteclk
 */
void mipi_dsih_presp_timeout_low_power_read(struct mipi_dsi_dev *dev,
											unsigned int no_of_byte_cycles)
{
	mipi_dsih_hal_presp_timeout_low_power_read(dev, no_of_byte_cycles);
}
/**
 * Timeout for peripheral (for controller to stay still) after HS data
 * transmission write requests
 * @param dev pointer to structure holding the DSI Host core information
 * @param no_of_byte_cycles period for which the DWC_mipi_dsi_host keeps the
 * link still, after sending a high-speed write operation. This period is
 * measured in cycles of lanebyteclk
 */
void mipi_dsih_presp_timeout_high_speed_write(struct mipi_dsi_dev *dev,
											  unsigned int no_of_byte_cycles)
{
	mipi_dsih_hal_presp_timeout_high_speed_write(dev, no_of_byte_cycles);
}
/**
 * Timeout for peripheral between HS data transmission read requests
 * @param dev pointer to structure holding the DSI Host core information
 * @param no_of_byte_cycles period for which the DWC_mipi_dsi_host keeps the
 * link still, after sending a high-speed read operation. This period is
 * measured in cycles of lanebyteclk
 */
void mipi_dsih_presp_timeout_high_speed_read(struct mipi_dsi_dev *dev,
											 unsigned int no_of_byte_cycles)
{
	mipi_dsih_hal_presp_timeout_high_speed_read(dev, no_of_byte_cycles);
}
/**
 * Timeout for peripheral (for controller to stay still) after bus turn around
 * @param dev pointer to structure holding the DSI Host core information
 * @param no_of_byte_cycles period for which the DWC_mipi_dsi_host keeps the
 * link still, after sending a BTA operation. This period is
 * measured in cycles of lanebyteclk
 */
void mipi_dsih_presp_timeout_bta(struct mipi_dsi_dev *dev, unsigned int no_of_byte_cycles)
{
	mipi_dsih_hal_presp_timeout_bta(dev, no_of_byte_cycles);
}

unsigned int mipi_dsih_check_dbi_fifos_state(struct mipi_dsi_dev *dev)
{
	unsigned int cnt = 0;
	while (cnt < 5000 && mipi_dsih_read_word(dev, R_DSI_HOST_CMD_PKT_STATUS) != 0x15)
	{
		cnt++;
	}
	return (mipi_dsih_read_word(dev, R_DSI_HOST_CMD_PKT_STATUS) != 0x15) ? -1 : 1;
}
unsigned int mipi_dsih_check_ulpm_mode(struct mipi_dsi_dev *dev)
{
	return (mipi_dsih_read_word(dev, R_DSI_HOST_PHY_STATUS) != 0x1528) ? -1 : 1;
}

/**
 * Stop Video Pattern Generator
 * @param dev pointer to structure holding the DSI Host core information
 */
void stop_video_pattern(struct mipi_dsi_dev *dev)
{
	mipi_dsih_hal_enable_vpg_act(dev, 0);

	mipi_dsih_reset_controller(dev);
}

/**
 * Start Video Pattern Generator
 * @param dev pointer to structure holding the DSI Host core information
 * @param orientation pattern orientiation
 * @param pattern type of pattern (BER or STANDARD)
 */
void start_video_pattern(struct mipi_dsi_dev *dev, unsigned char orientation,
						 unsigned char pattern)
{
	mipi_dsih_reset_controller(dev);
	mipi_dsih_hal_enable_vpg_act(dev, 0);
	mipi_dsih_hal_vpg_orientation_act(dev, orientation);
	mipi_dsih_hal_vpg_mode_act(dev, pattern);
	mipi_dsih_hal_enable_vpg_act(dev, 1);

	/*if (mipi_dsih_hal_read_state_shadow_registers(dev))
	 	mipi_dsih_hal_request_registers_change(dev);*/
}
