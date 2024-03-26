/* SPDX-License-Identifier: GPL-2.0 */
/**
 * Copyright (C) 2018 Synopsys, Inc.
 *
 * @file dsih_dphy.h
 * @brief Synopsys MIPI D-PHY functions
 * included as a part of Synopsys MIPI DSI Host controller driver
 *
 * @author Luis Oliveira <luis.oliveira@synopsys.com>
 * Modified by Telechips
 */

#ifndef MIPI_DSIH_DPHY_H_
#define MIPI_DSIH_DPHY_H_

#include "dsih_includes.h"

#define R_DPHY_LPCLK_CTRL	(0x94)
#define R_DPHY_RSTZ		(0xA0)
#define R_DPHY_IF_CFG		(0xA4)
#define R_DPHY_ULPS_CTRL	(0xA8)
#define R_DPHY_TX_TRIGGERS  	(0xAC)
#define R_DPHY_STATUS  	   	(0xB0)
#define R_DPHY_TST_CRTL0 	(0xB4)
#define R_DPHY_TST_CRTL1  	(0xB8)

#define MIPI_PHY_PLL0_OFFSET    (0x08)
#define MIPI_PHY_PLL1_OFFSET    (0x0C)
#define MIPI_PHY_PLL2_OFFSET    (0x10)
#define MIPI_PHY_MCTRL0_OFFSET   (0x14)
#define MIPI_PHY_MCTRL1_OFFSET   (0x18)
#define MIPI_PHY_MCTRL2_OFFSET   (0x1C)
#define MIPI_PHY_MCTRL3_OFFSET   (0x20)
#define MIPI_PHY_MCTRL4_OFFSET   (0x24)

/* obligatory functions - code can be changed for different phys*/
int 	mipi_dsih_dphy_open(struct mipi_dsi_dev *dev);

int 	mipi_dsih_dphy_configure(struct mipi_dsi_dev *dev,
                                 unsigned int no_of_lanes,
                                 unsigned int output_freq);

int 	mipi_dsih_dphy_close(struct mipi_dsi_dev *dev);

void 	mipi_dsih_dphy_clock_en(struct mipi_dsi_dev *dev, int en);

void 	mipi_dsih_dphy_reset(struct mipi_dsi_dev *dev, int reset);

void 	mipi_dsih_dphy_shutdown(struct mipi_dsi_dev *dev, int powerup);

int 	mipi_dsih_dphy_get_force_pll(struct mipi_dsi_dev *dev);

void 	mipi_dsih_dphy_force_pll(struct mipi_dsi_dev *dev, int force);

int 	mipi_dsih_dphy_wakeup_pll(struct mipi_dsi_dev *dev);

void 	mipi_dsih_dphy_stop_wait_time(struct mipi_dsi_dev *dev,
                                      unsigned int no_of_byte_cycles);

void 	mipi_dsih_dphy_no_of_lanes(struct mipi_dsi_dev *dev,
                                   unsigned int no_of_lanes);

unsigned int 	mipi_dsih_dphy_get_no_of_lanes(struct mipi_dsi_dev *dev);

void 	mipi_dsih_dphy_enable_hs_clk(struct mipi_dsi_dev *dev,
                                     int enable);

int 	mipi_dsih_dphy_escape_mode_trigger(struct mipi_dsi_dev *dev,
        unsigned int trigger_request);

int 	mipi_dsih_dphy_ulps_data_lanes(struct mipi_dsi_dev *dev,
                                       int enable);

int 	mipi_dsih_dphy_ulps_clk_lane(struct mipi_dsi_dev *dev,
                                     int enable);

unsigned int mipi_dsih_dphy_status(struct mipi_dsi_dev *dev, unsigned int mask);

/* end of obligatory functions*/
void 	mipi_dsih_dphy_test_clock(struct mipi_dsi_dev *dev, int value);

void 	mipi_dsih_dphy_test_clear(struct mipi_dsi_dev *dev, int value);

void 	mipi_dsih_dphy_test_en(struct mipi_dsi_dev *dev,
                               unsigned int on_falling_edge);

unsigned int mipi_dsih_dphy_test_data_out(struct mipi_dsi_dev *dev);

void 	mipi_dsih_dphy_test_data_in(struct mipi_dsi_dev *dev,
                                    unsigned int test_data);

void 	mipi_dsih_dphy_write(struct mipi_dsi_dev *dev, unsigned int address,
                             unsigned int * data, unsigned int data_length);

void 	mipi_dsih_dphy_write_word(struct mipi_dsi_dev *dev,
                                  unsigned int reg_address, unsigned int data);

void 	mipi_dsih_dphy_write_part(struct mipi_dsi_dev *dev,
                                  unsigned int reg_address, unsigned int data,
                                  unsigned int shift, unsigned int width);

unsigned int	 mipi_dsih_dphy_read_word(struct mipi_dsi_dev *dev,
                                      unsigned int reg_address);

unsigned int 	mipi_dsih_dphy_read_part(struct mipi_dsi_dev *dev,
                                     unsigned int reg_address,
                                     unsigned int shift,
                                     unsigned int width);

unsigned int 	dsi_get_transition_times(struct mipi_dsi_dev *dev);

void 	mipi_dsih_dphy_delay(struct mipi_dsi_dev *dev, int value);

void	mipi_dsih_dphy_mmcm_pclk(struct mipi_dsi_dev *dev, unsigned int pclk);
/*Testchip*/
void 	mipi_dsih_dphy_set_base_dir_tx(struct mipi_dsi_dev *dev);

void tcc_dsi_phy_init(struct mipi_dsi_dev *dev);
#endif	/* MIPI_DSIH_DPHY_H_ */
