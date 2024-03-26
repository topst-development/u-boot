// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_PCIE_PHY_H
#define TCC_PCIE_PHY_H

/**
 * struct phy_configure_opts_pcie - PCIe PHY configuration set
 *
 * This structure is used to tune PCIe Signal Quality.
 */
struct phy_configure_opts_pcie {
	/**
	 * @gen:
	 *
	 * PCIe Genereation, that determines the maximum data transfer rate
	 * supported by the interface.
	 *
	 * Maximum value: PCIe Gen4(1 to 4)
	 */
	uint8_t		gen;

	/**
	 * @txX_eq_main:
	 *
	 * Transmitter amplitude adjustment control, Control for setting
	 * the transmitter driver output amplitude (main coefficient).
	 */
	uint8_t		txX_eq_main;
	
	/**
	 * @txX_eq_ovrd:
	 *
	 * Transmitter equalization setting overrides, External overrides for
	 * the per-protocol settings of the PHY's TX equalization inputs. The
	 * PCS internally determines the hard-coded optimal TX equalization
	 * settings for each protocol and rate.
	 */
	uint8_t		txX_eq_ovrd;

	/**
	 * @txX_eq_pre:
	 *
	 * Transmitter Pre-emphasis level adjustment control, Control for setting
	 * the transmitter driver output pre-emphasis (preshoot coefficient).
	 */
	uint8_t		txX_eq_pre;

	/**
	 * @txX_eq_post:
	 *
	 * Transmitter Post-emphasis level adjustment control, Control for setting
	 * the transmitter driver output post-emphasis (post coefficient).
	 */
	uint8_t		txX_eq_post;

	/**
	 * @tx_vboost_lvl:
	 *
	 * Tx Voltage boost maximum level, Set the maximum achievable Tx swing
	 * The recommended default setting for this input is 3'b101
	 * (tx_vboost_vref = 0.275V)
	 *
	 * Minimum value: 3'b000 (vptxX)
	 * Maximum value: 3'b101 (1100 mVppd)
	 */
	uint8_t		tx_vboost_lvl;

	/**
	 * @tx_iboost_lvl:
	 *
	 * Transmitter current boost level, Control the per lane TX swing level
	 * when TX amplitude boost mode is enabled using txX_vboost_en. The TX
	 * swing on all lanes also affected by the tx_vboost_lvl[2:0] control.
	 *
	 * Minimum value: 4'b0000 (819 mVppd)
	 * Maximum value: 4'b1111 (1100 mVppd)
	 */
	uint8_t		tx_iboost_lvl;

	/**
	 * @rx_eq_ctle_boost:
	 *
	 * Rx equalization CTLE boost, Control the CTLE boost level;
	 * binary encoded.
	 *
	 * Maximum value: 5'b11111
	 */
	uint8_t		rx_eq_ctle_boost;

	/**
	 * @rx_eq_att_lvl:
	 *
	 * RX equalization attenuation level; binary encoded.
	 *
	 * Minimum value: 3'b000 (-2dB)
	 * Maximum value: 3'b111 (-6dB)
	 */
	uint8_t		rx_eq_att_lvl;

	/**
	 * @rx_los_threshold:
	 *
	 * Receiver LOS/Idle detection threshold, Set the LOS threshold level
	 * trip point.
	 *
	 * Minimum value: 3'b000 (Reserved)
	 * Maximum value: 3'b111 (270 mVppd)
	 */
	uint8_t		rx_los_threshold;
};
#endif /* TCC_PCIE_PHY_H */
