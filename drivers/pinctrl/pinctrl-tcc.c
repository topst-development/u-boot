// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <linux/io.h>
#include <linux/err.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dm/read.h>
#include <mailbox.h>
#include <linux/soc/telechips/tcc_mbox.h>
#include <linux/soc/telechips/tcc_sc_protocol.h>
#include <cpu.h>

//Offset based on GPIO bank base address
#define TCC_GPIO_DATA	(0x00U)
#define TCC_GPIO_OEN	(0x04U)
#define TCC_GPIO_OR		(0x08U)
#define TCC_GPIO_BIC	(0x0CU)
#define TCC_GPIO_XOR	(0x10U)
#define TCC_GPIO_CD0	(0x14U)
#define TCC_GPIO_CD1	(0x18U)
#define TCC_GPIO_PE		(0x1CU)
#define TCC_GPIO_PS		(0x20U)
#define TCC_GPIO_IEN	(0x24U)
#define TCC_GPIO_IS		(0x28U)
#define TCC_GPIO_SR		(0x2CU)
#define TCC_GPIO_FN0	(0x30U)

//Offset based on GPIO base address
#define TCC_EINTSEL0	(0x280U)
#define TCC_EINTSEL1	(0x284U)
#define TCC_EINTSEL2	(0x288U)
#define TCC_EINTSEL3	(0x28CU)
#define TCC_ECLKSEL		(0x2B0U)

struct tcc_bank {
	const char *name;
	u32 bank_base;
	u32 npins;
};

// core_num
//	- common : 0
//	- A72 : 1
//	- A53 : 2
struct telechips_pinctrl_priv {
	u32 nbanks;
	u32 npins;
	u32 gpio_base;
	u32 pmgpio_base;
	u32 core_num;
	struct tcc_bank *bank;
	const struct tcc_sc_fw_handle *handle;
	struct tcc_sc_reg_req_data sc_req_data;
};

struct gpio_state_t {
	s32 pin_func;
	s32 drive_strength;
	const void *pull_disable;
	const void *pull_up;
	const void *pull_down;
	const void *output_low;
	const void *output_high;
	const void *input_enable;
	const void *schmitt_input;
	const void *cmos_input;
	const void *slow_slew;
	const void *fast_slew;
	const void *no_pull;
};

#define TCC_PINCONF_DRIVE_STRENGTH		(1U)
#define TCC_PINCONF_NO_PULL				(2U)
#define TCC_PINCONF_PULL_UP				(3U)
#define TCC_PINCONF_PULL_DOWN			(4U)
#define TCC_PINCONF_INPUT_ENABLE		(5U)
#define TCC_PINCONF_OUTPUT_LOW			(6U)
#define TCC_PINCONF_OUTPUT_HIGH			(7U)
#define TCC_PINCONF_INPUT_BUFFER_ENABLE		(8U)
#define TCC_PINCONF_INPUT_BUFFER_DISABLE	(9U)
#define TCC_PINCONF_SCHMITT_INPUT		(10U)
#define TCC_PINCONF_CMOS_INPUT			(11U)
#define TCC_PINCONF_SLOW_SLEW			(12U)
#define TCC_PINCONF_FAST_SLEW			(13U)
#define TCC_PINCONF_FUNC				(14U)

#ifndef TCC_GPIO_COMPATIBLE
#if defined(CONFIG_TCC803X)
#define TCC_GPIO_COMPATIBLE ("telechips,tcc803x-gpio")
#endif
#if defined(CONFIG_TCC805X)
#define TCC_GPIO_COMPATIBLE ("telechips,tcc805x-gpio")
#endif
#endif

static const struct pinconf_param telechips_conf_params[11] = {
	{"telechips,drive-strength", TCC_PINCONF_DRIVE_STRENGTH, 0U},
	{"telechips,no-pull", TCC_PINCONF_NO_PULL, 0U},
	{"telechips,pull-up", TCC_PINCONF_PULL_UP, 0U},
	{"telechips,pull-down", TCC_PINCONF_PULL_DOWN, 0U},
	{"telechips,input-enable", TCC_PINCONF_INPUT_ENABLE, 0U},
	{"telechips,output-low", TCC_PINCONF_OUTPUT_LOW, 0U},
	{"telechips,output-high", TCC_PINCONF_OUTPUT_HIGH, 0U},
	{"telechips,schmitt-input", TCC_PINCONF_SCHMITT_INPUT, 0U},
	{"telechips,cmos-input", TCC_PINCONF_CMOS_INPUT, 0U},
	{"telechips,slow-slew", TCC_PINCONF_SLOW_SLEW, 0U},
	{"telechips,fast-slew", TCC_PINCONF_FAST_SLEW, 0U}
};

#if defined(CONFIG_PINCTRL_TCC_SCFW)
static s32 gpio_writel_by_scfw
	(u32 address, u32 bit_shift, u32 width, u32 value,
	 struct telechips_pinctrl_priv *priv)
{
	s32 ret;

	if ((priv != NULL) && (priv->handle != NULL)) {
		ret = priv->handle->ops.reg_ops.request_reg_set(
				priv->handle, address, bit_shift, width, value);
	} else {
		(void)pr_err(
			"[ERROR][PINCTRL][%s:%d] priv == NULL || priv->handle == NULL\n",
			__func__, __LINE__);
		return -EINVAL;
	}

	return ret;
}
#endif

#if !defined(CONFIG_PINCTRL_TCC_SCFW) \
	|| defined(CONFIG_TCC_GPIO_BIT_CTRL)
static s32 dm_pinctrl_set_reg
	(u32 address, u32 bit_shift, u32 width, u32 value)
{
	u32 mask;
	u32 reg_data;

	if (address == 0U) {
		(void)pr_err("[ERROR][PINCTRL] address == 0\n");
		return -EINVAL;
	}

	if (width == 0U) {
		(void)pr_err("[ERROR][PINCTRL] width == 0\n");
		return -EINVAL;
	}

	mask = ((u32)1U << width) - 1U;
	reg_data = readl((ulong)address);
	reg_data &= ~(mask << bit_shift);
	reg_data |= (mask & value) << bit_shift;

	writel(reg_data, (ulong)address);

	return 0;
}
#endif

#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
static s32 get_bank_num_for_bit_ctrl(const char *bank_name, u32 *bank_num)
{
	u32 i;
	const char *bank_list[18] = {
		"gpa", "gpb", "gpc", "gpd", "gpe", "gpf", "gpg",
		"gphdmi", "gpsd0", "gpsd1", "gpsd2", "gph",
		"reserved", "reserved",
		"gpma", "gpmb", "gpmc", "gpmd"
	};

	if (bank_num == NULL) {
		(void)pr_err("[ERROR][PINCTRL] bank_num == NULL\n");
		return -EINVAL;
	}

	for (i = 0U; i < 18U; i++) {
		if (strncmp(bank_list[i], bank_name, 6)
			== 0) {
			*bank_num = i;
			return 0;
		}
	}

	(void)pr_err("[ERROR][PINCTRL] Invalid bank_name(%s)\n", bank_name);

	return -EINVAL;
}

static s32 dm_pinctrl_set_bit_ctrl
	(struct telechips_pinctrl_priv *priv,
	 const char *bank_name, u32 pin_num)
{
	const u32 width = 2U;
	s32 ret = 0;
	u32 core_num = priv->core_num;
	u32 bit_shift = pin_num / width;
	u32 bank_num;
	u32 address;

	ret = get_bank_num_for_bit_ctrl(bank_name, &bank_num);
	if (ret != 0) {
		(void)pr_err("[ERROR][PINCTRL] Failed to get bank num\n");
		return ret;
	}

	address = 0x14200000U + 0x800U	//bit ctrl offset
		+ (0x8U * bank_num)         //bank offset
		+ (0x4U * (pin_num / 16U)); //pin offset

#if defined(CONFIG_PINCTRL_TCC_SCFW)
	if (priv->handle != NULL) {
		ret = gpio_writel_by_scfw
			(address, bit_shift, width, core_num, priv);
	} else {
		return -EINVAL;
	}
#else
	ret = dm_pinctrl_set_reg(address, bit_shift, width, core_num);
#endif

	return ret;
}

static u32 dm_pinctrl_get_bit_ctrl(const char *bank_name, u32 pin_num)
{
	u32 address;
	u32 bank_num;
	u32 value;

	if (get_bank_num_for_bit_ctrl(bank_name, &bank_num) != 0) {
		(void)pr_err(
				"[ERROR][PINCTRL] Failed to get bank num for bit ctrl\n"
				);
		return 0U;
	}

	address = 0x14200000U + 0x800U	//bit ctrl offset
		+ (0x8U * bank_num)				//bank offset
		+ (0x4U * (pin_num / 16U));		//pin offset
	value = readl((ulong)address);

	return (value >> (pin_num / 2U)) & 0x2U;
}

static s32 check_and_set_bit_ctrl
	(struct telechips_pinctrl_priv *priv,
	 const char *bank_name, u32 pin_num)
{
	u32 bit_ctrl_num = dm_pinctrl_get_bit_ctrl(bank_name, pin_num);
	s32 ret = 0;
	u32 core_num;

	if (priv == NULL) {
		(void)pr_err("[ERROR][PINCTRL] priv == NULL\n");
		return -EINVAL;
	}

	core_num = priv->core_num;

	if (core_num != bit_ctrl_num) {
		if (bit_ctrl_num == 0U) {
			// if the bit is not used anywhere(0),
			// set bit to core number
			ret = dm_pinctrl_set_bit_ctrl(
					priv, bank_name, pin_num);
		} else  {
			(void)pr_err(
					"[ERROR][PINCTRL] Invalid bit control value\n"
					);
			(void)pr_err(
					"\tbit ctrl num(%u) != core num(%u)\n"
					"\tthis gpio already used at another core\n",
					bit_ctrl_num, core_num);
			return -EINVAL;
		}
	}

	return ret;
}
#endif

static s32 integrated_gpio_writel
	(u32 address, u32 bit_shift, u32 width, u32 value, struct udevice *dev)
{
	s32 ret;
#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
	struct telechips_pinctrl_priv *priv = dev_get_priv(dev);
	u32 bit_ctrl_addr;

	/* If bit control is possible, SCFW is not used */
	bit_ctrl_addr
		= address + 0x1000U + (0x800U * (priv->core_num));
	ret = dm_pinctrl_set_reg(bit_ctrl_addr, bit_shift, width, value);
#else
/*
 * If bit control is impossible, use SCFW.
 * If there is no SCFW option directly access the register.
 */
#if defined(CONFIG_PINCTRL_TCC_SCFW)
	struct telechips_pinctrl_priv *priv = dev_get_priv(dev);

	ret = gpio_writel_by_scfw(address, bit_shift, width, value, priv);
#else
	ret = dm_pinctrl_set_reg(address, bit_shift, width, value);
#endif
#endif

	return ret;
}

static s32 tcc_get_pins_count(struct udevice *dev)
{
	struct telechips_pinctrl_priv *priv = dev_get_priv(dev);

	return (s32)(priv->npins);
}

static s32 tcc_get_groups_count(struct udevice *dev)
{
	struct telechips_pinctrl_priv *priv = dev_get_priv(dev);

	return (s32)(priv->nbanks);
}

static const char *tcc_get_group_name(struct udevice *dev,
					  u32 selector)
{
	struct telechips_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->bank[selector].name;
}

#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
static s32 get_core_num(u32 *core_num)
{
	struct udevice *dev;
	char desc[512];
	s32 ret;
	char *buf = NULL;

	if (core_num == NULL) {
		(void)pr_err("[ERROR][PINCTRL] core_num == NULL\n");
		return -EINVAL;
	}

	ret = uclass_first_device_err(UCLASS_CPU, &dev);
	if (ret != 0) {
		(void)pr_err(
			"[ERROR][PINCTRL]%s: Could not get CPU device (err = %d)\n"
			, __func__, ret);
		return ret;
	}

	if (dev == NULL) {
		(void)pr_err("[ERROR][PINCTRL] dev == NULL\n");
		return -EINVAL;
	}

	ret = cpu_get_desc(dev, desc, (s32)(sizeof(desc)));
	if (ret != 0) {
		(void)pr_err(
			"[ERROR][PINCTRL]%s: Could not get CPU description (err = %d)\n"
			, dev->name, ret);
		return ret;
	}

	strtok(desc, "-");
	buf = strtok(NULL, "A");

	if (strtoul(buf, NULL, 10) == 72U) {
		/* A72 */
		*core_num = 1U;
	} else {
		/* A53 */
		*core_num = 2U;
	}

	return 0;
}
#endif

static s32 tcc_pinctrl_probe(struct udevice *dev)
{
	struct telechips_pinctrl_priv *priv = dev_get_priv(dev);
	ofnode node;
	u32 regs[4];
	u32 i;

	if (!dev_of_valid(dev)) {
		(void)pr_err(
			"[ERROR][PINCTRL][%s:%d] Invalid dev\n"
			, __func__, __LINE__);
		return -EINVAL;
	}

#if defined(CONFIG_PINCTRL_TCC_SCFW)
	priv->handle = tcc_sc_fw_get_handle_by_name();
	if (priv->handle == NULL) {
		(void)pr_err(
			"[ERROR][PINCTRL][%s:%d] priv->handle == NULL\n"
			, __func__, __LINE__);
		return -EINVAL;
	}
#endif

#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
	if (get_core_num(&(priv->core_num)) != 0) {
		(void)pr_err("[ERROR][PINCTRL] failed to get core num\n");
		return -EINVAL;
	}
#endif

	node = dev_ofnode(dev);
	if (ofnode_read_u32_array(node, "reg", regs, 4) == 0) {
		//[0] : gpio addr, [1] : gpio range,
		//[2] : pmgpio addr, [3] : pmgpio rnage
		priv->gpio_base = regs[0];
		priv->pmgpio_base = regs[2];
	} else {
		return -EINVAL;
	}

	node = ofnode_by_compatible(ofnode_null(), TCC_GPIO_COMPATIBLE);

	i = 0;
	while (ofnode_valid(node)) {
		priv->nbanks++;
		node = ofnode_by_compatible(node, TCC_GPIO_COMPATIBLE);
		i++;
	}

	priv->sc_req_data.config = NULL;
	priv->bank = kcalloc(priv->nbanks, sizeof(struct tcc_bank), GFP_KERNEL);

	node = ofnode_by_compatible(ofnode_null(), TCC_GPIO_COMPATIBLE);

	i = 0;
	while (ofnode_valid(node)) {
		if (ofnode_read_u32_array(node, "reg", regs, 2) == 0) {
			priv->bank[i].bank_base = regs[0];
			priv->bank[i].npins = regs[1];
			priv->bank[i].name = ofnode_get_name(node);
		}

		node = ofnode_by_compatible(node, TCC_GPIO_COMPATIBLE);
		i++;
	}

	return 0;
}

//Comment processing because there is no place in use.
#if 0
static s32 tcc_gpio_set_eint
	(u32 bank_base, u32 eint_num, u32 eint_source, struct udevice *dev)
{
	const u32 width = 8U;
	u32 eint_addr = bank_base + TCC_EINTSEL0 + ((eint_num / 4U) * 0x4U);
	u32 bit_shift = (eint_num % 4U) * width;

	if (eint_num > 15U) {
		pr_err("[ERROR][PINCTRL] Invalid eint_num(%u)\n", eint_num);
		return -EINVAL;
	}

#if defined(CONFIG_PINCTRL_TCC_SCFW)
	struct telechips_pinctrl_priv *priv = dev_get_priv(dev);

	return gpio_writel_by_scfw
			(eint_addr, bit_shift, width, eint_source, priv);
#else
	return dm_pinctrl_set_reg
			(eint_addr, bit_shift, width, eint_source);
#endif
}

static s32 tcc_gpio_set_eclk
	(u32 bank_base, u32 eclk_num, u32 eclk_source, struct udevice *dev)
{
	const u32 width = 8U;
	u32 eclk_addr = bank_base + TCC_ECLKSEL;
	u32 bit_shift = eclk_num * width;

	if (eclk_num > 3U) {
		pr_err("[ERROR][PINCTRL] Invalid eclk_num(%u)\n", eclk_num);
		return -EINVAL;
	}

#if defined(CONFIG_PINCTRL_TCC_SCFW)
	struct telechips_pinctrl_priv *priv = dev_get_priv(dev);

	return gpio_writel_by_scfw
			(eclk_addr, bit_shift, width, eclk_source, priv);
#else
	return dm_pinctrl_set_reg
			(eclk_addr, bit_shift, width, eclk_source);
#endif
}
#endif

static s32 tcc_gpio_set_function
	(u32 bank_base, u32 pin_num, u32 func, struct udevice *dev)
{
	const u32 width = 4U;
	u32 func_addr = bank_base + TCC_GPIO_FN0 + ((pin_num / 8U) * 0x4U);
	u32 bit_shift = (pin_num % 8U) * 4U;

	return integrated_gpio_writel
			(func_addr, bit_shift, width, func, dev);
}

static s32 tcc_gpio_set_drive_strength
	(u32 bank_base, u32 pin_num, u32 ds, struct udevice *dev)
{
	const u32 width = 2U;
	u32 ds_addr = bank_base + TCC_GPIO_CD0 + ((pin_num / 16U) * 0x4U);
	u32 bit_shift = (pin_num % 16U) * 2U;

	return integrated_gpio_writel
			(ds_addr, bit_shift, width, ds, dev);
}

static s32 tcc_gpio_pull_enable
	(u32 bank_base, u32 pin_num, u32 pe, struct udevice *dev)
{
	u32 pe_addr = bank_base + TCC_GPIO_PE;

	return integrated_gpio_writel
			(pe_addr, pin_num, 1U, pe, dev);
}

static s32 tcc_gpio_pull_select
	(u32 bank_base, u32 pin_num, u32 ps, struct udevice *dev)
{
	u32 ps_addr = bank_base + TCC_GPIO_PS;

	return integrated_gpio_writel
			(ps_addr, pin_num, 1U, ps, dev);
}

static s32 tcc_gpio_output_enable
	(u32 bank_base, u32 pin_num, u32 oen, struct udevice *dev)
{
	u32 oen_addr = bank_base + TCC_GPIO_OEN;

	return integrated_gpio_writel
			(oen_addr, pin_num, 1U, oen, dev);
}

static s32 tcc_gpio_set_output(u32 bank_base, u32 pin_num, u32 value)
{
	u32 bit = ((u32)1U) << pin_num;

	if (value == 1U) {	//output high
		writel(bit, (ulong)(bank_base + TCC_GPIO_OR));
		/* OR register is bit control register, so access directly */
	} else {			//output low
		writel(bit, (ulong)(bank_base + TCC_GPIO_BIC));
		/* BIC register is bit control register, so access directly */
	}

	return 0;
}

static s32 tcc_gpio_input_enable(
		u32 bank_base, u32 pin_num, u32 ien, struct udevice *dev)
{
	u32 ien_addr = bank_base + TCC_GPIO_IEN;

	return integrated_gpio_writel
			(ien_addr, pin_num, 1U, ien, dev);
}

static u32 get_pin_base(struct udevice *dev, const char *pin_name, u32 *pin_num)
{
	struct telechips_pinctrl_priv *priv = dev_get_priv(dev);
	u32 i;
	size_t len;
	ulong ul_pin_num;

	if (pin_num == NULL) {
		(void)pr_err(
			"[ERROR][PINCTRL][%s:%d] pin_num == NULL\n"
			, __func__, __LINE__);
		return 0U;
	}

	for (i = 0; i < priv->nbanks; i++) {
		len = (u32)((priv->bank[i].name != NULL)
				? strnlen(priv->bank[i].name, 10U) : 0U);

		if (strncasecmp(pin_name, priv->bank[i].name, len) == 0) {
			if (strict_strtoul(pin_name + len, 10, &ul_pin_num)
					!= 0) {
				break;
			}

			*pin_num = (u32)ul_pin_num;

			if (strncasecmp(pin_name, "gpk", 3) != 0) {
				return priv->gpio_base
					+ priv->bank[i].bank_base;
			} else {
				return priv->pmgpio_base
					+ priv->bank[i].bank_base;
			}
		}
	}

	return 0U;	//not exist
}

static s32 tcc_gpio_set_input_type
	(u32 bank_base, u32 pin_num, u32 is, struct udevice *dev)
{
	u32 is_addr = bank_base + TCC_GPIO_IS;

	return integrated_gpio_writel
			(is_addr, pin_num, 1U, is, dev);
}

static s32 tcc_gpio_set_slew_rate
	(u32 bank_base, u32 pin_num, u32 sr, struct udevice *dev)
{
	u32 sr_addr = bank_base + TCC_GPIO_SR;

	return integrated_gpio_writel
			(sr_addr, pin_num, 1U, sr, dev);
}

static s32 get_state_count(struct gpio_state_t *state)
{
	s32 count = 0;

	if (state->drive_strength
			!= -1) {
		count++;
	}

	if (state->pin_func
			!= -1) {
		count++;
	}

	if ((state->pull_up != NULL)
			|| (state->pull_down != NULL)) {
		count += 2;
	} else if ((state->pull_disable != NULL)
			|| (state->no_pull != NULL)) {
		count++;
	} else {
		/* empty */
	}

	if (state->output_low != NULL) {
		/* 2 = 3 - 1 */
		count += 2;
	} else if (state->output_high != NULL) {
		/* 2 = 3 - 1 */
		count += 2;
	} else {
		/* empty */
	}

	if (state->input_enable
			!= NULL) {
		count += 2;
	}

	if ((state->schmitt_input != NULL)
			|| (state->cmos_input != NULL)) {
		count++;
	}

	if ((state->slow_slew != NULL)
			|| (state->fast_slew != NULL)) {
		count++;
	}

	return count;
}

static s32 get_gpio_state(ofnode node, struct gpio_state_t *state)
{
	if (state == NULL) {
		(void)pr_err(
			"[ERROR][PINCTRL][%s:%d] state == NULL\n"
			, __func__, __LINE__);
		return -EINVAL;
	}

	state->output_low = ofnode_get_property
		(node, "telechips,output-low", NULL);
	state->output_high = ofnode_get_property
		(node, "telechips,output-high", NULL);
	state->input_enable = ofnode_get_property
		(node, "telechips,input-enable", NULL);
	state->schmitt_input = ofnode_get_property
		(node, "telechips,schmitt-input", NULL);
	state->cmos_input = ofnode_get_property
		(node, "telechips,cmos-input", NULL);
	state->slow_slew = ofnode_get_property
		(node, "telechips,slow-slew", NULL);
	state->fast_slew = ofnode_get_property
		(node, "telechips,fast-slew", NULL);
	state->pull_up = ofnode_get_property
		(node, "telechips,pull-up", NULL);
	state->pull_down = ofnode_get_property
		(node, "telechips,pull-down", NULL);
	state->pull_disable = ofnode_get_property
		(node, "telechips,pull-disable", NULL);
	state->no_pull = ofnode_get_property
		(node, "telechips,no-pull", NULL);
	state->pin_func = ofnode_read_s32_default
		(node, "telechips,pin-function", -1);
	state->drive_strength = ofnode_read_s32_default
		(node, "telechips,drive-strength", -1);

	return get_state_count(state);
}

static s32 tcc_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	ofnode node;
	s32 pin_count;
	s32 i;
	u32 pin_num;
	const char *name;
	u32 addr;
	struct telechips_pinctrl_priv *priv;
	struct gpio_state_t state;

	if (config == NULL) {
		(void)pr_err(
			"[ERROR][PINCTRL][%s:%d] config == NULL\n"
			, __func__, __LINE__);
		return -EINVAL;
	}
	node = config->node;

	priv = dev_get_priv(dev);
	if (priv == NULL) {
		(void)pr_err(
			"[ERROR][PINCTRL][%s:%d] priv == NULL\n"
			, __func__, __LINE__);
		return -EINVAL;
	}

	pin_count = ofnode_read_string_count(node, "telechips,pins");
	if (pin_count <= 0) {
		return -EINVAL;
		/* comment for QAC, codesonar, kernel coding style */
	}

	(void)get_gpio_state(node, &state);

	for (i = 0; i < pin_count; i++) {
		if (ofnode_read_string_index(node, "telechips,pins", i, &name)
				!= 0) {
			continue;
		}

		addr = get_pin_base(dev, name, &pin_num);

		if (addr == 0U) {
			(void)pr_err(
				"[ERROR][PINCTRL][%s:%d] address == 0\n"
				, __func__, __LINE__);
			return -EINVAL;
		}

#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
		if (check_and_set_bit_ctrl(priv, name, pin_num) != 0) {
			(void)pr_err(
				"[ERROR][PINCTRL][%s:%d] Invalid bit control\n"
				, __func__, __LINE__);
			return -EINVAL;
		}
#endif

		if (state.pin_func != -1) {
			(void)tcc_gpio_set_function(addr, pin_num,
					(u32)state.pin_func, dev);
		}

		if (state.drive_strength != -1) {
			(void)tcc_gpio_set_drive_strength(addr, pin_num,
					(u32)state.drive_strength, dev);
		}

		if (state.pull_up != NULL) {
			(void)tcc_gpio_pull_enable(addr, pin_num, 1, dev);
			(void)tcc_gpio_pull_select(addr, pin_num, 1, dev);
		} else if (state.pull_down != NULL) {
			(void)tcc_gpio_pull_enable(addr, pin_num, 1, dev);
			(void)tcc_gpio_pull_select(addr, pin_num, 0, dev);
		} else if ((state.pull_disable != NULL)
				|| (state.no_pull != NULL)) {
			(void)tcc_gpio_pull_enable(addr, pin_num, 0, dev);
		} else {
			/* empty */
		}

		if (state.output_low != NULL) {
			(void)tcc_gpio_output_enable(addr, pin_num, 1, dev);
			(void)tcc_gpio_set_output(addr, pin_num, 0);
			(void)tcc_gpio_input_enable(addr, pin_num, 0, dev);
		} else if (state.output_high != NULL) {
			(void)tcc_gpio_output_enable(addr, pin_num, 1, dev);
			(void)tcc_gpio_set_output(addr, pin_num, 1);
			(void)tcc_gpio_input_enable(addr, pin_num, 0, dev);
		} else {
			/* empty */
		}

		if (state.input_enable != NULL) {
			(void)tcc_gpio_output_enable(addr, pin_num, 0, dev);
			(void)tcc_gpio_input_enable(addr, pin_num, 1, dev);
		}

		if (state.schmitt_input != NULL) {
			(void)tcc_gpio_set_input_type
				(addr, pin_num, 1, dev);
		} else if (state.cmos_input != NULL) {
			(void)tcc_gpio_set_input_type
				(addr, pin_num, 0, dev);
		} else {
			/* empty */
		}

		if (state.slow_slew != NULL) {
			(void)tcc_gpio_set_slew_rate
				(addr, pin_num, 1, dev);
		} else if (state.fast_slew != NULL) {
			(void)tcc_gpio_set_slew_rate
				(addr, pin_num, 0, dev);
		} else {
			/* empty */
		}
	}

	return 0;
}

static const struct pinctrl_ops telechips_pinctrl_ops = {
	.get_pins_count = tcc_get_pins_count,
	.get_groups_count = tcc_get_groups_count,
	.get_group_name = tcc_get_group_name,
	.pinconf_num_params = ARRAY_SIZE(telechips_conf_params),
	.pinconf_params = telechips_conf_params,
	.set_state = tcc_pinctrl_set_state,
};

static const struct udevice_id telechips_pinctrl_match[2] = {
	{ .compatible = "telechips,pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(telechips_pinctrl) = {
	.name = "telechips_pinctrl",
	.id = UCLASS_PINCTRL,
	.priv_auto_alloc_size = (s32)sizeof(struct telechips_pinctrl_priv),
	.of_match = telechips_pinctrl_match,
	.ops = &telechips_pinctrl_ops,
	.probe = tcc_pinctrl_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
