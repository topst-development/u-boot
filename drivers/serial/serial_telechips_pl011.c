// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <asm/io.h>
#include <asm/arch/serial.h>
#include <mach/clock.h>
#include <dm/ofnode.h>
#if defined(CONFIG_TCC_SC_FIRMWARE)
#include <mailbox.h>
#include <linux/soc/telechips/tcc_mbox.h>
#include <linux/soc/telechips/tcc_sc_protocol.h>
#endif
#include <linux/bug.h>
#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>
#include <dm/platform_data/serial_pl01x.h>
#include <serial.h>
#include "serial_pl01x_internal.h"

#define UART_FR 0x18
#define UART_FR_RXFE 0x10

static void tcc_serial_set_portcfg(unsigned int con, unsigned int cfg_id,
				   uintptr_t port_cfg)
{
#if !defined(CONFIG_TCC750X)
#if defined(CONFIG_TCC_SC_FIRMWARE)
	const struct tcc_sc_fw_handle *handle = tcc_sc_fw_get_handle_by_name();
#endif
	uintptr_t portcfg_base, portcfg, tmp_portcfg;
	unsigned int ucon;
	unsigned int shift, portcfg_val, num;
	s32 err = 0;

	if (port_cfg <= 0xFFFFFFFFU) {
		portcfg_base = port_cfg;
		portcfg = port_cfg;
	} else {
		BUG();
	}

	if (con < 4U) {
		ucon = con;
	} else if (con < 8U) {
		ucon = con % 4U;
		portcfg += 4U;
	} else {
		pr_err("[ERROR][SERIAL] %s controller number is wrong %u(err %d)\n"
			, __func__, con, err);
		BUG();
	}

	for (num = 0; num < TCC_SERIAL_MAX_CH; num++) {
		shift = (num % 4U) * 8U;
		tmp_portcfg = portcfg_base + ((num / 4U) * 4U);
		portcfg_val = readl(tmp_portcfg);
		portcfg_val = ((portcfg_val >> shift) & 0xFFU);

		if ((con != num) && (portcfg_val == cfg_id)) {
#if defined(CONFIG_TCC_SC_FIRMWARE)
			if (handle != NULL) {
				err = handle->ops.reg_ops.request_reg_set
				    (handle, tmp_portcfg, shift, 8u, 0x3F);
			} else {
				pr_err("[ERROR][SERIAL] %s failed to get SC FW handle(err %d)\n"
							, __func__, err);
				BUG();
			}

			if (err < 0) {
				BUG();
			}
#else
			portcfg_val = readl(tmp_portcfg);
			portcfg_val = (portcfg_val & ~(0xFF << (shift))) |
					(0x3F << shift);
			writel(portcfg_val, tmp_portcfg);
#endif
		}
	}

#if defined(CONFIG_TCC_SC_FIRMWARE)
	if (handle != NULL) {
		if ((UINT_MAX / 8U) > ucon) {
			ucon *= 8U;
		} else {
			BUG();
		}

		err = handle->ops.reg_ops.request_reg_set(handle,
						    portcfg, ucon, 8u, cfg_id);

		if (err < 0) {
			BUG();
		}
	} else {
		pr_err("[ERROR][SERIAL] %s failed to get SC FW handle(err %d)\n"
							    , __func__, err);
		BUG();
	}
#else
	if (portcfg != 0U) {
		portcfg_val = readl(portcfg);
		writel((portcfg_val & ~(0xFF << (ucon * 8u)))
			    | ((cfg_id & 0xFF) << (ucon * 8u)), portcfg);
	} else {
		pr_err("[ERROR][SERIAL] %s no address(port_cfgm err %d)"
							    , __func__, err);
		BUG();
	}
#endif
#endif

}

static void tcc_set_clk(const u32 *tcc_clk_id)
{
#if !defined(CONFIG_TCC750X)
	(void)tcc_set_iobus_swreset(tcc_clk_id[3], true);
	(void)tcc_set_iobus_pwdn(tcc_clk_id[3], true);
	(void)tcc_set_iobus_pwdn(tcc_clk_id[3], false);
	(void)tcc_set_iobus_swreset(tcc_clk_id[3], false);
#endif
}


static void tcc_serial_set_port_clk(unsigned int con, unsigned int cfg_id,
				    uintptr_t portcfg, const u32 *tcc_clk_id,
				    ofnode cur_serial_node)
{
	unsigned int serial_clk;
	const u32 *serial_clk_raw;
	int size;

	serial_clk_raw = (const u32 *)ofnode_get_property
				    (cur_serial_node, "clock", &size);

	if (serial_clk_raw != NULL) {
		serial_clk = fdt32_to_cpu(*serial_clk_raw);

		if (serial_clk == 0u) {
			serial_clk = 48000000;
		}

		tcc_serial_set_portcfg(con, cfg_id, portcfg);

		tcc_set_clk(tcc_clk_id);

#if defined(CONFIG_TCC807X)
		(void)tcc_set_peri(tcc_clk_id[1], 1UL, serial_clk, CLK_F_SRC_CLK(SMU_CLKCTRL_SEL_XIN));
#else
		(void)tcc_set_peri(tcc_clk_id[1], 1UL, serial_clk, 0);
#endif
	} else {
		pr_err("[ERROR][SERIAL] %s failed to get serial clock\n"
			    , __func__);
		BUG();
	}
}

static void tcc_serial_init_port_clk(u32 serial_phandle, uintptr_t portcfg,
				     const u32 *tcc_clk_id, unsigned int con,
				     ofnode cur_serial_node)
{
	ofnode pinctrl_node;
	const char *pinctrl_name, *string_temp;
	long cfg_num_len, i;
	char cfg_id_string[3] = {0,};
	unsigned long temp_long;
	unsigned int cfg_id;

	pinctrl_node = ofnode_get_by_phandle(serial_phandle);
	pinctrl_name = ofnode_get_name(pinctrl_node);
	if (pinctrl_name != NULL) {
		pinctrl_name += 4;
		string_temp = strstr(pinctrl_name, "_");
		if (string_temp == NULL) {
			BUG();
		}
		cfg_num_len = string_temp - pinctrl_name;
		for (i = 0; i < cfg_num_len; i++) {
			cfg_id_string[i] = *(pinctrl_name + i);
		}
		cfg_id_string[i + 1] = '\0';
		temp_long = simple_strtoul(cfg_id_string, NULL, 10);

		if (temp_long <= 0xFFFFFFFFU) {
			cfg_id = (unsigned int)temp_long;
		} else {
			BUG();
		}

		tcc_serial_set_port_clk(con, cfg_id, portcfg, tcc_clk_id,
					cur_serial_node);
	} else {
		pr_err("[ERROR][SERIAL] %s failed to get pinctrl name\n"
			, __func__);
		BUG();
	}
}

void tcc_serial_rx_flush(uintptr_t serial_addr)
{
	uintptr_t serial_fr_addr = serial_addr + UART_FR;
	while (!(readl(serial_fr_addr) & UART_FR_RXFE))
		readl(serial_addr);
}

void tcc_serial_pre_init(ofnode cur_serial_node)
{
	int ret = 0;
	uintptr_t portcfg;//TCC_UART_CFG;
	uintptr_t serial_addr;
	unsigned long temp_long;
	unsigned int con;
	u32 serial_phandle = 0;
	u32 tcc_clk_id[4] = {0,};

	serial_addr = (uintptr_t)ofnode_get_addr_index(cur_serial_node, 0);
	temp_long = (serial_addr & 0x70000u) >> 0x10u;

	if (temp_long <= 0xFFFFFFFFU) {
		con = (unsigned int)temp_long;
	} else {
		BUG();
	}

	ret = ofnode_read_u32_array(cur_serial_node, "clocks", tcc_clk_id, 4);

	if (ret < 0) {
		BUG();
	}

	portcfg = (uintptr_t)ofnode_get_addr_index(cur_serial_node, 1);

	ret = ofnode_read_u32(cur_serial_node, "pinctrl-0", &serial_phandle);

	if (ret < 0) {
		BUG();
	}

	tcc_serial_init_port_clk(serial_phandle, portcfg, tcc_clk_id, con,
				 cur_serial_node);

	tcc_serial_rx_flush(serial_addr);
}

static int telechips_pl011_serial_probe(struct udevice *dev)
{
	int ret;

	ret = pl01x_serial_of_to_plat(dev);
	if (ret)
		return ret;

	if (!(gd->flags & GD_FLG_RELOC))
		tcc_serial_pre_init(dev_ofnode(dev));

	return pl01x_serial_probe(dev);
}

static const struct dm_serial_ops telechips_pl011_serial_ops = {
	.putc = pl01x_serial_putc,
	.pending = pl01x_serial_pending,
	.getc = pl01x_serial_getc,
	.setbrg = pl01x_serial_setbrg,
};

static const struct udevice_id telechips_pl011_serial_id[] = {
	{.compatible = "telechips,telechips-pl011", .data = TYPE_PL011},
	{}
};

U_BOOT_DRIVER(telechips_pl011_uart) = {
	.name	= "telechips_pl011",
	.id	= UCLASS_SERIAL,
	.of_match = of_match_ptr(telechips_pl011_serial_id),
	.probe	= telechips_pl011_serial_probe,
	.plat_auto	= sizeof(struct pl01x_serial_plat),
	.ops	= &telechips_pl011_serial_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL) || CONFIG_IS_ENABLED(OF_BOARD)
	.flags	= DM_FLAG_PRE_RELOC,
#endif
	.priv_auto	= sizeof(struct pl01x_priv),
};
