// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <dm.h>
#include <getopt.h>
#include <asm/io.h>
#include <pcie_dw_telechips.h>
#include <linux/soc/telechips/tcc-pcie-phy.h>

static struct phy_configure_opts_pcie *cfg = NULL;

static int32_t run_sigtest(void)
{
	int32_t err = 0;

	if (cfg != NULL) {
		err = pcie_dw_tcc_set_phy_conf((void *)cfg); 
	}

	return err;
}

static void print_parameters(void)
{
	if (cfg != NULL) {
		(void)printf("\n------------------- PCIe SigTest Paramters --------------------\n");
		(void)printf("PCIe Generation = 0x%x\n", cfg->gen);
		(void)printf("txX_eq_main = 0x%x\n", cfg->txX_eq_main);
		(void)printf("txX_eq_pre = 0x%x\n", cfg->txX_eq_pre);
		(void)printf("txX_eq_post = 0x%x\n", cfg->txX_eq_post);
		(void)printf("tx_vboost_lvl = 0x%x\n", cfg->tx_vboost_lvl);
		(void)printf("tx_iboost_lvl = 0x%x\n", cfg->tx_iboost_lvl);
		(void)printf("rx_eq_ctle_boost = 0x%x\n", cfg->rx_eq_ctle_boost);
		(void)printf("rx_eq_att_lvl = 0x%x\n", cfg->rx_eq_att_lvl);
		(void)printf("---------------------------------------------------------------\n\n");
	}
}

static char usage_text[] =
	"usage: sigtest [options]\n"
	"Options:\n"
	"\t-g <value>\n"
	"\tSet PCI Gen {default: PCIe Gen3}\n"
	"\t-m <value>\n"
	"\tTransmitter amplitude adjustment control\n"
	"\tControl for setting the transmitter driver output amplitude(main coefficient)\n"
#if defined(CONFIG_PHY_DWC_PCIE)
	"\t-p <integer-value.fraction-value>\n"
#endif
#if defined(CONFIG_PHY_SF_PCIE)
	"\t -p <value>\n"
	"\tMinimum value: 5'b0000 (Min swing)\n"
	"\tMaximum value: 5'b1011 (Max swing)\n"
#endif
	"\tTransmitter Pre-emphasis level adjustment control\n"
	"\tControl for setting the transmitter driver output pre-emphasis(preshoot coefficient)\n"
#if defined(CONFIG_PHY_DWC_PCIE)
	"\t\t-Interger value: 0 to 10\n"
	"\t\t-Fraction value: 0, 0.25, 0.50, 0.75\n"
	"\t-o <integer-value.fraction-value>\n"
#endif
#if defined(CONFIG_PHY_SF_PCIE)
	"\t -o <value>\n"
	"\tMinimum value: 4'b0000 (Min pre-shoot boosting)\n"
	"\tMaximum value: 4'b1010 (Max pre-shoot bootsing)\n"
#endif
	"\tTransmitter Post-emphasis level adjustment control\n"
	"\tControl for setting the transmitter driver output post-emphasis(post coefficient) \n"
#if defined(CONFIG_PHY_DWC_PCIE)
	"\t\t-Interger value: 0 to 15\n"
	"\t\t-Fraction value: 0, 0.25, 0.50, 0.75\n"
	"\t-v <value>\n"
	"\tTX voltage boost maximum level\n"
	"\tSets the maximum achievable TX swing. The recommended default setting for this input is 3'b101(tx_vboost_vref = 0.275V). \n"
	"\t-i <value>\n"
	"\tTransmitter current boost level\n"
	"\tControl the per lane TX swing level when TX amplitude boost mode is enabled using txX_vboost_en. The TX swing on all lanes also affected by the tx_vboost_lvl[2:0] control. \n"
	"\t-c <value>\n"
	"\tRX equalization CTLE boost\n" 
	"\tControl the CTLE boost level; binary encoded\n"
	"\t-a <value>\n"
	"\tRX equalization attenuation level\n"
	"\tControl the AFE attenuation level from -2 dB when set to 3'b000, to - 6 dB when set to 3'b111\n "
#endif
#if defined(CONFIG_PHY_SF_PCIE)
	"\t-v <value>\n"
	"\tMinimum value: 4'b0000 (Min de-emphasis level)\n"
	"\tMaximum value: 4'b1110 (Max de-emphasis level)\n"
#endif
	"\t-s\n"
	"\tShow configureation parameters\n"
	"\t-r\n"
	"\tSet configureation parameters\n";

static void print_usage(void)
{
	(void)fprintf(stderr, usage_text);
}

#ifdef CONFIG_PHY_DWC_PCIE
/**
 * get_tx_eq_value() - Convert the "integer-value.fraction-value" identifier into a number
 *
 * @input: [IN] Value string in the form "integer-value.fraction" where each is in hex
 * @output: [OUT]Device string in the form "integer-value.fraction" where each is in hex
 */
static void dwc_get_tx_eq_coefficient(char *in, uint8_t *out)
{
	if ((in != NULL) && (out != NULL)) {
		int32_t len;

		len = strlen(in);
		if (len <= 5) {
			int32_t idx = 0;
			char cnum[12];

			*out = 0x0U;
			for (idx = 0; idx < len; idx++) {
				if (in[idx] == '.') {
					/* Integer value */
					memcpy(cnum, &in[0], idx);
					cnum[idx] = '\0';
					*out |= (dectoul(cnum, NULL) << 2);
					idx += 1;

					/* Fraction value */
					memcpy(cnum, &in[idx], len - idx);
					cnum[len - idx] = '\0';
					*out |= (dectoul(cnum, NULL)/25);
					break;
				}
			}
		}
	}
}
#endif

static bool parse_args(struct getopt_state *state, int32_t argc, char *const argv[])
{
	bool ret = false;

	if ((cfg != NULL) && (state != NULL)) {
		int32_t opt;

		while ((opt = getopt(state, argc, argv, "g:m:p:o:v:i:c:a:rh")) > 0) {
			switch (opt) {
			case 'g':
				cfg->gen = simple_strtol(state->arg, NULL, 0);
				break;
			case 'm':
				cfg->txX_eq_main = simple_strtol(state->arg, NULL, 0);
				break;
			case 'p':
#ifdef CONFIG_PHY_DWC_PCIE
				dwc_get_tx_eq_coefficient(state->arg, &cfg->txX_eq_pre);
#else
				cfg->txX_eq_pre = simple_strtol(state->arg, NULL, 0);
#endif
				break;
			case 'o':
#ifdef CONFIG_PHY_DWC_PCIE
				dwc_get_tx_eq_coefficient(state->arg, &cfg->txX_eq_post);
#else
				cfg->txX_eq_post = simple_strtol(state->arg, NULL, 0);
#endif
				break;
			case 'v':
				cfg->tx_vboost_lvl = simple_strtol(state->arg, NULL, 0);
				break;
			case 'i':
				cfg->tx_iboost_lvl = simple_strtol(state->arg, NULL, 0);
				break;
			case 'c':
				cfg->rx_eq_ctle_boost = simple_strtol(state->arg, NULL, 0);
				break;
			case 'a':
				cfg->rx_eq_att_lvl = simple_strtol(state->arg, NULL, 0);
				break;
			case 'r':
				ret = true;
				break;
			case 'h':
			default:
				print_usage();
				break;
			}
		}
	}

	return ret;
}

static int32_t sigtest_cmd_handler(
	struct cmd_tbl *cmdtp, int32_t flag, int32_t argc, char *const argv[])
{
	int32_t err = 0;

	(void)cmdtp;
	(void)flag;

	if (cfg == NULL) {
		cfg = (struct phy_configure_opts_pcie *)
			malloc(sizeof(struct phy_configure_opts_pcie));
		if (cfg == NULL) {
			err = -ENOMEM;
		} else {
			(void)memset(cfg, 0x0, sizeof(struct phy_configure_opts_pcie));
		}
	}

	if (err == 0) {
		struct getopt_state state;

		getopt_init_state(&state);
		if (parse_args(&state, argc, argv)) {
			err = run_sigtest();
		} else {
			print_parameters();
		}
	}

	if (err != 0) {
		(void)printf("PCIe SQ Control failed! please check parameters\n");
	}

	return err;
}

U_BOOT_CMD(
	sigtest, CONFIG_SYS_MAXARGS, 8,
	sigtest_cmd_handler,
	usage_text, usage_text
);
