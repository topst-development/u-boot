// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <asm/io.h>
#include <asm/arch/serial.h>
#include <asm/arch/clock.h>
#include <dm/ofnode.h>
#include <mailbox.h>
#include <linux/soc/telechips/tcc_mbox.h>
#include <linux/soc/telechips/tcc_sc_protocol.h>

static void tcc_serial_set_portcfg(unsigned int con, unsigned int cfg_id,
	unsigned int *port_cfg)
{
#if defined(CONFIG_TCC_SC_FIRMWARE)
	const struct tcc_sc_fw_handle *handle = tcc_sc_fw_get_handle_by_name();
#endif
	uint32_t *portcfg, ucon, *tmp_portcfg, shift, portcfg_val;
	int32_t err = 0, num = 0;

	if (con < 4u) {
		ucon = con;
		portcfg = port_cfg;
	} else if (con < 8u) {
		ucon = con%4u;
		portcfg = port_cfg+1u;
	} else {
		err = -EINVAL;
		pr_err(
			"[ERROR][SERIAL] %s controller number is wrong %d(err %d)\n"
			, __func__, con, err);
	}

	for (num = 0; num < TCC_SERIAL_MAX_CH; num++) {

		shift = (num%4)*8;
		tmp_portcfg = port_cfg+(num/4u);
		portcfg_val = readl(tmp_portcfg);
		portcfg_val = ((portcfg_val>>shift)&0xff);

		if ((con != num) && (portcfg_val == cfg_id)) {
#if defined(CONFIG_TCC_SC_FIRMWARE)
			if (handle != NULL) {
				handle->ops.reg_ops.request_reg_set(handle,
					    (uintptr_t)tmp_portcfg, shift,
						8u, 0x3F);
			} else {
				err = -ENXIO;
				pr_err("[ERROR][SERIAL] %s failed to get SC FW handle(err %d)\n"
							, __func__, err);
			}
#else
			portcfg_val = readl(tmp_portcfg);
			portcfg_val = (portcfg_val & ~(0xFF<<(shift))) |
					(0x3F << shift);
			writel(portcfg_val, tmp_portcfg);
#endif
		}

	}

	if (err != -EINVAL) {
#if defined(CONFIG_TCC_SC_FIRMWARE)
		if (handle != NULL) {
			handle->ops.reg_ops.request_reg_set(handle,
				    (uintptr_t)portcfg, ucon*8u, 8u, cfg_id);
		} else {
			err = -ENXIO;
			pr_err(
					"[ERROR][SERIAL] %s failed to get SC FW handle(err %d)\n"
					, __func__, err);
		}
#else
		if (portcfg != NULL) {
			portcfg_val = readl(portcfg);
			writel((portcfg_val & ~(0xFF<<(ucon*8u)))
				    | ((cfg_id&0xFF) << (ucon*8u)), portcfg);
		} else {
			err = -ENXIO;
			pr_err(
					"[ERROR][SERIAL] %s no address(port_cfgm err %d)"
					, __func__, err);
		}
#endif
	}

}

void tcc_serial_pre_init(ofnode cur_serial_node)
{
	ofnode pinctrl_node;
	unsigned int *portcfg;//TCC_UART_CFG;
	unsigned int *serial_addr;
	unsigned long cfg_id;
	unsigned int con;
	unsigned int serial_clk;
	int size;
	u32 serial_phandle = 0;
	int tcc_clk_id[4] = {0,};
	const u32 *serial_clk_raw;
	const char *pinctrl_name, *string_temp;
	char cfg_id_string[3] = {0,};
	unsigned int cfg_num_len, i;

	serial_addr = (unsigned int *)ofnode_get_addr_index(cur_serial_node, 0);
	con = ((uintptr_t)serial_addr&0x70000u)>>0x10u;

	ofnode_read_u32_array(cur_serial_node, "clocks", (u32 *)tcc_clk_id, 4);

	portcfg = (unsigned int *)ofnode_get_addr_index(cur_serial_node, 1);

	ofnode_read_u32(cur_serial_node, "pinctrl-0", &serial_phandle);
	pinctrl_node = ofnode_get_by_phandle(serial_phandle);
	pinctrl_name = ofnode_get_name(pinctrl_node);
	if (pinctrl_name != NULL) {
		pinctrl_name += 4;
		string_temp = strstr(pinctrl_name, "_");
		cfg_num_len = (uintptr_t)string_temp - (uintptr_t)pinctrl_name;
		for (i = 0; i < cfg_num_len; i++) {
			cfg_id_string[i] = *(pinctrl_name+i);
			//for coding style
		}
		cfg_id_string[i+1u] = '\0';
		cfg_id = simple_strtoul((const char *)cfg_id_string, NULL, 10);

		serial_clk_raw = ofnode_get_property(cur_serial_node, "clock",
			&size);
		if (serial_clk_raw != NULL) {
			serial_clk = fdt32_to_cpu(*serial_clk_raw);

			if (serial_clk == 0u)
				serial_clk = 48000000;

			tcc_serial_set_portcfg(con, (unsigned int)cfg_id,
				portcfg);

			tcc_set_iobus_swreset(tcc_clk_id[3], true);
			tcc_set_iobus_pwdn(tcc_clk_id[3], true);
			tcc_set_iobus_pwdn(tcc_clk_id[3], false);
			tcc_set_iobus_swreset(tcc_clk_id[3], false);

			tcc_set_peri(tcc_clk_id[1], true, serial_clk, 0);
		} else {
			pr_err(
				"[ERROR][SERIAL] %s failed to get serial clock\n"
				, __func__);
		}
	} else {
		pr_err(
			"[ERROR][SERIAL] %s failed to get pinctrl name\n"
			, __func__);
	}
}

