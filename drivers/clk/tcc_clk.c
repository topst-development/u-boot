// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <clk-uclass.h>
#include <clk.h>
#include <dm.h>
#include <mach/clock.h>
#include <mach/smc.h>
#include <linux/arm-smccc.h>
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


/* [DR]
 * tcc_dm_set_rate is call back of struct clk_ops.set_rate
 * function is declare as
 * ‘ulong (*set_rate)(struct clk *clk, ulong rate)'
 */
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

#define CLK_TFA_VERSION(major, minor, patch)	\
	((major)*1000000 + (minor)*1000 + (patch))

#define CHECK_TFA_VERSION_MIN(major, minor, patch) \
	(CLK_TFA_VERSION((major), (minor), (patch)) <= vtfa_clk)

#define CHECK_TFA_VERSION_IS(major, minor, patch) \
	(CLK_TFA_VERSION((major), (minor), (patch)) == vtfa_clk)

static uint32_t vtfa_clk;

unsigned long tcc_set_swreset(unsigned long id, unsigned long op)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_SWRESET, id, op, 0, 0, 0, 0, 0, &res);

	return res.a0;
}

/*
 * User interface function for clock setting
 * tcc_clk_init();
 * tcc_clk_set_dpll_config();
 * tcc_clk_set_pll();
 * tcc_clk_set_peri();
 * tcc_clk_set_clkctrl();
 * tcc_clk_get_pll();
 * tcc_clk_get_peri();
 * tcc_clk_get_clkctrl();
 */

/*
 * Telechips clock initialize function.
 */
void tcc_clk_init(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_INIT, 0, 0, 0, 0, 0, 0, 0, &res);

	vtfa_clk = CLK_TFA_VERSION(res.a0, res.a1, res.a2);
}

/*
 * Dithered PLL configurateion function.
 *
 * @ id        : Target DPLL ID
 * @ modfreq   : Modulation Frequency
 * @ modrate   : Modulation Rate
 *              The average clock frequency will changed in range of
 *              -(modrate/2)% ~ +(modrate/2)%
 *
 *               Predefined macros.
 *               SSCG_MR0p5 - for 0.5% modulation rate
 *               SSCG_MR1p0 - for 1.0% modulation rate
 *
 * 		 If the user want to use a modulation rate other than the macro,
 * 		 they must multiply by 1000.
 *
 * @ sel_pf    : Modulation method of SSCG.
 *              SSCG_DOWN_SPREAD   - The average clock frequency will set about
 *                                   PLL frequency - (modrate/2)%
 *              SSCG_UP_SPREAD     - The average clock frequency will set about
 *                                   PLL frequency
 *              SSCG_CENTER_SPREAD - The average clock frequency will set about
 *                                   PLL frequency + (modrate/2)%
 *
 *              This parameter only work since tcc807x.
 *              Before tcc807x, SSCG_CENTER_SPREAD is default.
 *
 * usage example)
 *   1. 30K modulation frequency & 0.5% modulation rate
 *     tcc_set_dpll_config(PLL_2, 30000UL, SSCG_MR0p5, SSCG_CENTER_SPREAD);
 *
 *   2. 30K modulation frequency & 1.5% modulation rate
 *     tcc_set_dpll_config(PLL_2, 30000UL, 1500UL, SSCG_DOWN_SPREAD);
 */
unsigned long tcc_set_dpll_config(unsigned long id, unsigned long modfreq,
				unsigned long modrate, unsigned long sel_pf)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_SET_DPLL_CONFIG,
		      id, modfreq, modrate, sel_pf, 0, 0, 0, &res);

	return CHECK_TFA_VERSION_IS(1, 1, 0) ? res.a3 : res.a0;
}

/*
 * PLL configurateion function.
 *
 * @ id        : Target PLL ID
 *               See arch/arm/mach-telechips/[board]/incluce/tcc_ckc.h
 * @ en	       : Enable or disable PLL
 *               Currently, TF-A will ignore disable request of pll.
 * @ rate      : Target pll rate.
 *               See chip specification Document section
 *               Electrical Characteristics for Normal PLL &
 *               Electrical Characteristics for Dither PLL
 * @ tcc_div   : Divider value of PLL
 *               Odd divider value is not guarantee 50:50 duty cycle.
 *
 * usage example)
 *   1. Set PLL_0 Fout to 1200 MHz & set divider to 2
 *     tcc_set_ll(PLL_0, CKC_ENABLE, 1200000000UL, 2);
 *
 *   2. Set PLL_0 Fout to 1200 MHz & disable divider
 *     tcc_set_ll(PLL_0, CKC_ENABLE, 1200000000UL, 0);
 */
unsigned long tcc_set_pll(unsigned long id, unsigned long en,
				unsigned long rate, unsigned long tcc_div)
{
	struct arm_smccc_res res;
	(void)en;

	arm_smccc_smc(SIP_CLK_SET_PLL,
		      id, rate, tcc_div, 0, 0, 0, 0, &res);

	return 0;
}

/*
 * PLL configurateion function.
 * Some PLL has 2 or more extra dividers.
 * This function help to set divider value of extra divider.
 * XXX: This function only work in tcc750x
 *
 * @ id        : Target PLL ID
 *               See arch/arm/mach-telechips/[board]/incluce/tcc_ckc.h
 * @ tcc_div   : Divider value of PLL
 *               Odd divider value is not guarantee 50:50 duty cycle.
 *
 * usage example)
 *   1. Set PLL0 divider to 2
 *     tcc_set_pll_div(PLL_DIV_0, 2);
 *
 *   2. Disable PLL0 divider.
 *     tcc_set_pll_div(PLL_DIV_0, 0);
 */
unsigned long tcc_set_pll_div(unsigned long id, unsigned long tcc_div)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_SET_PLL_DIV,
		      id, tcc_div, CKC_ENABLE, 0, 0, 0, 0, &res);

	return 0;
}

/*
 * Peripheral clock configurateion function.
 *
 * @ id        : Target peripheral ID
 *               See arch/arm/mach-telechips/[board]/include/tcc_ckc.h
 * @ en	       : Enable or disable Peripheral clock
 *               Currently, TF-A will ignore disable request of pll.
 * @ rate      : Target peripheral rate.
 *               See chip specification Document (Electrical Characteristics)
 * @ flags     : Clock flags for periphaeral
 *               See arch/arm/mach-telechips/[board]/include/clock.h
 *
 * usage example)
 *   1. Set timer t clock 12 MHz with no flags
 *     tcc_set_peri(PERI_TCT, CKC_ENABLE, 12000000UL, 0);
 *
 *   2. Set timer t clock 12 MHz with XIN
 *     tcc_set_peri(PERI_TCT, CKC_ENABLE, 12000000UL, CLK_F_SRC_CLK(SMU_PCLKCTRL_SEL_PLL0));
 */
unsigned long tcc_set_peri(unsigned long id, unsigned long en,
				unsigned long rate, unsigned long flags)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_SET_PCLKCTRL,
		      id, en, rate, flags, 0, 0, 0, &res);

	return 0;
}

/*
 * Bus clock configurateion function.
 *
 * @ id        : Target Bus ID
 *               See arch/arm/mach-telechips/[board]/include/tcc_ckc.h
 * @ en	       : Enable or disable Peripheral clock
 *               Currently, TF-A will ignore disable request of pll.
 * @ rate      : Target peripheral rate.
 *               See chip specification Document (Electrical Characteristics)
 * @ flags     : Clock flags for bus
 *               See arch/arm/mach-telechips/[board]/include/clock.h
 *               This parameter only work since tc750x.
 *
 * usage example)
 *   1. Set CPU BUS clock 400 MHz with no flags
 *     tcc_set_clkctrl(FBUS_CPUB, CKC_ENABLE, 400000000UL, 0UL);
 *
 *   2. Set timer t clock 400 MHz with PLL0
 *     tcc_set_clkctrl(FBUS_CPUB, CKC_ENABLE, 400000000UL, CLK_F_SRC_CLK(SMU_CLKCTRL_SEL_PLL0));
 */
unsigned long tcc_set_clkctrl(unsigned long id, unsigned long en,
				unsigned long rate, unsigned long flags)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_SET_CLKCTRL, id, en, rate, flags, 0, 0, 0, &res);

	return 0;
}

/*
 * Calculate PLL clock function.
 *
 * @ id        : Target PLL ID
 *               See arch/arm/mach-telechips/[board]/include/tcc_ckc.h
 */
unsigned long tcc_get_pll(unsigned long id)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_GET_PLL, id, 0, 0, 0, 0, 0, 0, &res);

	return res.a0;
}

/*
 * Calcualte peripheral clock function.
 *
 * @ id        : Target Bus ID
 *               See arch/arm/mach-telechips/[board]/include/tcc_ckc.h
 */
unsigned long tcc_get_peri(unsigned long id)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SIP_CLK_GET_PCLKCTRL, id, 0, 0, 0, 0, 0, 0, &res);

	return res.a0;
}

/*
 * Calculate BUS clock function.
 *
 * @ id        : Target BUS ID
 *               See arch/arm/mach-telechips/[board]/include/tcc_ckc.h
 */
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

/*
 * [DR]
 * u-boot API U_BOOT_DRIVER has defects it's inside.
 */
U_BOOT_DRIVER(tcc_clk) = {
	.name = "tcc_clk",
	.id = UCLASS_CLK,
	.ops = &tcc_clk_ops,
	.of_match = tcc_clk_ids,
	.probe = tcc_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
