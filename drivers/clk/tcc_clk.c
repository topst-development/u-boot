// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <clk-uclass.h>
#include <clk.h>
#include <dm.h>
#include <linux/arm-smccc.h>
#include <mach/clock.h>
#include <mach/smc.h>
#include <linux/sizes.h>
#include <linux/io.h>

// Mode bit
#define ID_CLKCTRL      0x1
#define ID_PERI         0x2
#define ID_PLL          0x3

// peri_id : 12-bit
// div : 4-bit
// setting mode(PLL, peri) : 4-bit
// en bit : 1-bit

#define PERI_EN_SHIFT		24UL
#define	PERI_EN_MASK		0x1UL
#define PERI_MODE_SHIFT		20UL
#define PERI_MODE_MASK		0xFUL
#define PERI_ID_SHIFT		8UL
#define PERI_ID_MASK		0x3FFUL

#define	PERI_DIV_SHIFT		0UL
#define	PERI_DIV_MASK		0xFFUL
#define PERI_FLAGS_SHIFT	0UL
#define	PERI_FLAGS_MASK		0xFFUL

static unsigned long tcc_dm_set_rate(struct clk *pclk, unsigned long rate)
{
	unsigned long mode_bit = ((pclk->data) >>
				PERI_MODE_SHIFT) & PERI_MODE_MASK;
	unsigned long peri_id = ((pclk->data) >>
				PERI_ID_SHIFT) & PERI_ID_MASK;
	unsigned long peri_en = ((pclk->data) >>
				PERI_EN_SHIFT) & PERI_EN_MASK;
	struct arm_smccc_res res;
	unsigned long ret = CKC_NO_OPS_DATA;

	switch (mode_bit) {
	case ID_PERI:
		{
			unsigned long peri_flag = ((pclk->data) >>
						    PERI_FLAGS_SHIFT) &
						    PERI_FLAGS_MASK;
			arm_smccc_smc(SIP_CLK_SET_PCLKCTRL, peri_id, peri_en,
					rate, peri_flag, 0, 0, 0, &res);
			ret = res.a0;
		}
		break;
	case ID_PLL:
		{
			unsigned long pll_div = ((pclk->data) >>
						  PERI_DIV_SHIFT) &
						  PERI_DIV_MASK;
			arm_smccc_smc(SIP_CLK_SET_PLL, peri_id, rate, pll_div,
					0, 0, 0, 0, &res);
			ret = res.a0;
		}
		break;
	case ID_CLKCTRL:
		arm_smccc_smc(SIP_CLK_SET_CLKCTRL, peri_id, peri_en, rate,
				0, 0, 0, 0, &res);
		ret = res.a0;
		break;
	default:
		ret = CKC_NO_OPS_DATA;
		break;
	}

	return ret;
}

static unsigned long tcc_dm_get_rate(struct clk *pclk)
{
	unsigned long mode_bit = ((pclk->data) >>
				  PERI_MODE_SHIFT) &
				  PERI_MODE_MASK;
	unsigned long peri_id = ((pclk->data) >>
				  PERI_ID_SHIFT) &
				  PERI_ID_MASK;
	struct arm_smccc_res res;
	unsigned long ret = CKC_NO_OPS_DATA;

	switch (mode_bit) {
	case ID_PERI:
		arm_smccc_smc(SIP_CLK_GET_PCLKCTRL,
			      peri_id, 0, 0, 0, 0, 0, 0, &res);
		ret = res.a0;
		break;
	case ID_PLL:
		arm_smccc_smc(SIP_CLK_GET_PLL,
			      peri_id, 0, 0, 0, 0, 0, 0, &res);
		ret = res.a0;
		break;
	case ID_CLKCTRL:
		arm_smccc_smc(SIP_CLK_GET_CLKCTRL,
			      peri_id, 0, 0, 0, 0, 0, 0, &res);
		ret = res.a0;
		break;
	default:
		ret = CKC_NO_OPS_DATA;
		break;
	}

	return ret;
}

static const struct clk_ops tcc_clk_ops = {
	.get_rate = tcc_dm_get_rate,
	.set_rate = tcc_dm_set_rate,
};

static struct clk tcc_clk_obj = {
	.dev = NULL,
	.rate = 0,
	.flags = 0,
	.id = 0,
	.data = 1, /* dummy init value */
};

static int tcc_clk_probe(struct udevice *dev)
{
	tcc_clk_obj.dev = dev;

	return 0;
}

/*
 * User interface function for clock setting
 * tcc_clk_set_dpll_config();
 * tcc_clk_set_pll();
 * tcc_clk_set_peri();
 * tcc_clk_set_clkctrl();
 * tcc_clk_get_pll();
 * tcc_clk_get_peri();
 * tcc_clk_get_clkctrl();
 */

/*
 * Dithered PLL configurateion function.
 *
 * @ id        : Target DPLL ID
 * @ modfreq   : Modulation Frequency
 * @ modrate   : Modulation Rate
 *               Use predefined macros is recommended for modrate.
 *               SSCG_MR0p5 - for 0.5% modulation rate
 *               SSCG_MR1p0 - for 1.0% modulation rate
 *
 * 		 If the user want to use a modulation rate other than the macro,
 * 		 they must multiply by 1000.
 *
 * usage example)
 *   1. 30K modulation frequency & 0.5% modulation rate
 *     tcc_set_dpll_config(PLL_2, 30000UL, SSCG_MR0p5);
 *
 *   2. 30K modulation frequency & 1.5% modulation rate
 *     tcc_set_dpll_config(PLL_2, 30000UL, 1500UL);
 */
unsigned long tcc_set_dpll_config(unsigned long id, unsigned long modfreq,
				unsigned long modrate)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_SET_DPLL_CONFIG,
		      id, modfreq, modrate, 0, 0, 0, 0, &res);

	return res.a0;
}

unsigned long tcc_set_pll(unsigned long id, unsigned long en,
				unsigned long rate, unsigned long tcc_div)
{
	struct arm_smccc_res res;
	(void)en;

	arm_smccc_smc(SIP_CLK_SET_PLL,
		      id, rate, tcc_div, 0, 0, 0, 0, &res);

	return 0;
}

unsigned long tcc_set_peri(unsigned long id, unsigned long en,
				unsigned long rate, unsigned long flags)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_SET_PCLKCTRL,
		      id, en, rate, flags, 0, 0, 0, &res);

	return 0;
}

unsigned long tcc_set_clkctrl(unsigned long id, unsigned long en,
				unsigned long rate)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_SET_CLKCTRL, id, en, rate, 0, 0, 0, 0, &res);

	return 0;
}

unsigned long tcc_get_pll(unsigned long id)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_GET_PLL, id, 0, 0, 0, 0, 0, 0, &res);

	return res.a0;
}

unsigned long tcc_get_peri(unsigned long id)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_GET_PCLKCTRL, id, 0, 0, 0, 0, 0, 0, &res);

	return res.a0;
}

unsigned long tcc_get_clkctrl(unsigned long id)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_GET_CLKCTRL, id, 0, 0, 0, 0, 0, 0, &res);

	return res.a0;
}

unsigned long tcc_set_iobus_pwdn(unsigned long id, bool pwdn)
{
	struct arm_smccc_res res;
	unsigned long p = (pwdn ? 1UL : 0UL);

	arm_smccc_smc(SIP_CLK_PWDN_IOBUS, id, p, 0, 0, 0, 0, 0, &res);

	return 0;
}

unsigned long tcc_set_iobus_swreset(unsigned long id, bool reset)
{
	struct arm_smccc_res res;
	unsigned long rst = (reset ? 1UL : 0UL);

	arm_smccc_smc(SIP_CLK_RESET_IOBUS, id, rst, 0, 0, 0, 0, 0, &res);

	return 0;
}

unsigned long tcc_set_hsiobus_pwdn(unsigned long id, bool pwdn)
{
	struct arm_smccc_res res;
	unsigned long p = (pwdn ? 1UL : 0UL);

	arm_smccc_smc(SIP_CLK_PWDN_HSIOBUS, id, p, 0, 0, 0, 0, 0, &res);

	return 0;
}

unsigned long tcc_set_hsiobus_swreset(unsigned long id, bool reset)
{
	struct arm_smccc_res res;
	unsigned long rst = (reset ? 1UL : 0UL);

	arm_smccc_smc(SIP_CLK_RESET_HSIOBUS, id, rst, 0, 0, 0, 0, 0, &res);

	return 0;
}

unsigned long tcc_set_cpubus_pwdn(unsigned long id, bool pwdn)
{
	struct arm_smccc_res res;
	unsigned long p = (pwdn ? 1UL : 0UL);

	arm_smccc_smc(SIP_CLK_PWDN_CPUBUS, id, p, 0, 0, 0, 0, 0, &res);

	return 0;
}

unsigned long tcc_set_cpubus_swreset(unsigned long id, bool reset)
{
	struct arm_smccc_res res;
	unsigned long rst = (reset ? 1UL : 0UL);

	arm_smccc_smc(SIP_CLK_RESET_CPUBUS, id, rst, 0, 0, 0, 0, 0, &res);

	return 0;
}

static const struct udevice_id tcc_clk_ids[] = {
	{ .compatible = "telechips,ckc"},
	{}
};

U_BOOT_DRIVER(tcc_clk) = {
	.name = "tcc_clk",
	.id = UCLASS_CLK,
	.ops = &tcc_clk_ops,
	.of_match = tcc_clk_ids,
	.probe = tcc_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
