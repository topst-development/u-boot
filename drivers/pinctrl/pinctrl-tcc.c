// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/bug.h>
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
#if defined(CONFIG_TCC807X)
#define TCC_GPIO_CD2	(0x2CU)
#else
#define TCC_GPIO_SR		(0x2CU)
#endif
#define TCC_GPIO_FN0	(0x30U)
#define TCC_GPFLEX_SEL0	(0x600U)

//Offset based on GPIO base address
#define TCC_EINTSEL0	(0x280U)
#define TCC_EINTSEL1	(0x284U)
#define TCC_EINTSEL2	(0x288U)
#define TCC_EINTSEL3	(0x28CU)
#define TCC_ECLKSEL		(0x2B0U)

#define TCC_GPIO_DS01_MASK (0x3U)
#define TCC_GPIO_DS2_MASK (0x1U)

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
#if defined(CONFIG_PINCTRL_TCC_SCFW)
	const struct tcc_sc_fw_handle *handle;
	struct tcc_sc_reg_req_data sc_req_data;
#endif
};

struct gpio_state_t {
	s32 pin_func;
	s32 mfio[2]; // [0] : mfio num, [1] mfio sel
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
#elif defined(CONFIG_TCC805X)
#define TCC_GPIO_COMPATIBLE ("telechips,tcc805x-gpio")
#elif defined(CONFIG_TCC750X)
#define TCC_GPIO_COMPATIBLE ("telechips,tcc750x-gpio")
#elif defined(CONFIG_TCC807X)
#define TCC_GPIO_COMPATIBLE ("telechips,tcc807x-gpio")
#else
#error Unsupported Platform
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

#define PINCTRL_ERR_SCFW				(0)
#define PINCTRL_ERR_SET_REG1				(1)
#define PINCTRL_ERR_SET_REG2				(2)
#define PINCTRL_ERR_BIT_CTRL1				(3)
#define PINCTRL_ERR_BIT_CTRL2				(4)
#define PINCTRL_ERR_BIT_CTRL3				(5)
#define PINCTRL_ERR_BIT_CTRL4				(6)
#define PINCTRL_ERR_BIT_CTRL5				(7)
#define PINCTRL_ERR_BIT_CTRL6				(8)
#define PINCTRL_ERR_GET_CORE_NUM1			(9)
#define PINCTRL_ERR_GET_CORE_NUM2			(10)
#define PINCTRL_ERR_GET_CORE_NUM3			(11)
#define PINCTRL_ERR_GET_CORE_NUM4			(12)
#define PINCTRL_ERR_PROBE1				(13)
#define PINCTRL_ERR_PROBE2				(14)
#define PINCTRL_ERR_PROBE3				(15)
#define PINCTRL_ERR_GET_GPIO_STATE1			(16)
#define PINCTRL_ERR_SET_STATE1				(17)
#define PINCTRL_ERR_SET_STATE2				(18)
#define PINCTRL_ERR_SET_STATE3				(19)
#define PINCTRL_ERR_SET_STATE4				(20)

static const char *err_log[21] = {
	"priv == NULL || priv->handle == NULL",		//0
	"address == 0",					//1
	"width == 0",					//2
	"bank_num == NULL",				//3
	"Invalid bank_name",				//4
	"Failed to get bank num",			//5
	"Failed to get bank num for bit ctrl",		//6
	"priv == NULL",					//7
	"Invalid bit control value",			//8
	"core_num == NULL",				//9
	"Could not get CPU device",			//10
	"dev == NULL",					//11
	"Could not get CPU description",		//12
	"Invalid dev",					//13
	"No phandle",					//14
	"No reg node in DT",				//15
	"state == NULL",				//16
	"config == NULL",				//17
	"priv == NULL",					//18
	"address == 0",					//29
	"Invalid bit control"				//20
};

static void pinctrl_err_log(uint32_t num, uint32_t val) {

	(void)pr_err("[ERROR][PINCTRL]:%s, val : %u\n", err_log[num], val);
}

#if defined(CONFIG_PINCTRL_TCC_SCFW)
static s32 gpio_writel_by_scfw
	(u32 address, u32 bit_shift, u32 width, u32 value,
	 const struct telechips_pinctrl_priv *priv)
{
	s32 ret;

	if ((priv != NULL) && (priv->handle != NULL)) {
		ret = priv->handle->ops.reg_ops.request_reg_set(
				priv->handle, address, bit_shift, width, value);
	} else {
		pinctrl_err_log(PINCTRL_ERR_SCFW, 0);
		ret = -EINVAL;
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
		pinctrl_err_log(PINCTRL_ERR_SET_REG1, 0);
		return -EINVAL;
	}

	if (width == 0U) {
		pinctrl_err_log(PINCTRL_ERR_SET_REG2, 0);
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
		pinctrl_err_log(PINCTRL_ERR_BIT_CTRL1, 0);
		return -EINVAL;
	}

	for (i = 0U; i < 18U; i++) {
		if (strncmp(bank_list[i], bank_name, 6)
			== 0) {
			*bank_num = i;
			return 0;
		}
	}

	pinctrl_err_log(PINCTRL_ERR_BIT_CTRL2, 0);

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
		pinctrl_err_log(PINCTRL_ERR_BIT_CTRL3, 0);
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
		pinctrl_err_log(PINCTRL_ERR_BIT_CTRL4, 0);
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
		pinctrl_err_log(PINCTRL_ERR_BIT_CTRL5, 0);
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
			pinctrl_err_log(PINCTRL_ERR_BIT_CTRL6, 0);
			return -EINVAL;
		}
	}

	return ret;
}
#endif


static s32 integrated_gpio_writel
	(u32 address, u32 bit_shift, u32 width, u32 value, const struct udevice *dev)
{
	s32 ret;
#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
	struct telechips_pinctrl_priv *priv = dev_get_priv(dev);
	/* If bit control is possible, SCFW is not used */
	u32 bit_ctrl_addr = address + 0x1000U + (0x800U * (priv->core_num));

	ret = dm_pinctrl_set_reg(bit_ctrl_addr, bit_shift, width, value);
#else
/*
 * If bit control is impossible, use SCFW.
 * If there is no SCFW option directly access the register.
 */
#if defined(CONFIG_PINCTRL_TCC_SCFW)
	const struct telechips_pinctrl_priv *priv = dev_get_priv(dev);

	ret = gpio_writel_by_scfw(address, bit_shift, width, value, priv);
#else
	ret = dm_pinctrl_set_reg(address, bit_shift, width, value);
#endif
#endif

	return ret;
}

static s32 tcc_get_pins_count(struct udevice *dev)
{
	const struct telechips_pinctrl_priv *priv = dev_get_priv(dev);
	s32 ret = 0;

	if (priv == NULL) {
		ret = -1;
	}

	if (ret == 0) {
		if (priv->npins < 0xFFFFFFFFU) {
			ret = (s32)priv->npins;
		} else {
			ret = -1;
		}
	}

	return ret;
}

static s32 tcc_get_groups_count(struct udevice *dev)
{
	const struct telechips_pinctrl_priv *priv = dev_get_priv(dev);
	s32 ret = 0;

	if (priv == NULL) {
		ret = -1;
	}

	if (ret == 0) {
		if (priv->nbanks < 0xFFFFFFFFU) {
			ret = (s32)priv->nbanks;
		} else {
			ret = -1;
		}
	}

	return ret;
}

static const char *tcc_get_group_name(struct udevice *dev, u32 selector)
{
	const struct telechips_pinctrl_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		BUG();
	}

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
		pinctrl_err_log(PINCTRL_ERR_GET_CORE_NUM1, 0);
		return -EINVAL;
	}

	ret = uclass_first_device_err(UCLASS_CPU, &dev);
	if (ret != 0) {
		pinctrl_err_log(PINCTRL_ERR_GET_CORE_NUM2, 0);
		return ret;
	}

	if (dev == NULL) {
		pinctrl_err_log(PINCTRL_ERR_GET_CORE_NUM3, 0);
		return -EINVAL;
	}

	ret = cpu_get_desc(dev, desc, (s32)(sizeof(desc)));
	if (ret != 0) {
		pinctrl_err_log(PINCTRL_ERR_GET_CORE_NUM4, 0);
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

static s32 tcc_pinctrl_init(struct telechips_pinctrl_priv *priv) {


	s32 ret = 0;

#if defined(CONFIG_PINCTRL_TCC_SCFW)
	priv->handle = tcc_sc_fw_get_handle_by_name();
	if (priv->handle == NULL) {
		pinctrl_err_log(PINCTRL_ERR_PROBE2, 0);
		ret = -EINVAL;
	}
#endif

#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
	if (ret == 0) {
		if (get_core_num(&(priv->core_num)) != 0) {
			ret = -EINVAL;
		}
	}
#endif

	return ret;


}

static void tcc_pinctrl_dt_bank(struct telechips_pinctrl_priv *priv){

	ofnode node;
	u32 i;
	u32 regs[2];

	node = ofnode_by_compatible(ofnode_null(), TCC_GPIO_COMPATIBLE);

	i = 0;
	while (ofnode_valid(node)) {
		if (priv->nbanks < UINT_MAX) {
			priv->nbanks++;
		} else {
			BUG();
		}
		node = ofnode_by_compatible(node, TCC_GPIO_COMPATIBLE);
		if (i < UINT_MAX) {
			i++;
		} else {
			BUG();
		}
	}

#if defined(CONFIG_PINCTRL_TCC_SCFW)
	priv->sc_req_data.config = NULL;
#endif
	priv->bank = kcalloc(priv->nbanks, sizeof(struct tcc_bank), GFP_KERNEL);

	if (priv->bank == NULL) {
		BUG();
	}

	node = ofnode_by_compatible(ofnode_null(), TCC_GPIO_COMPATIBLE);

	i = 0;
	while (ofnode_valid(node)) {
		if (ofnode_read_u32_array(node, "reg", regs, 2) == 0) {
			priv->bank[i].bank_base = regs[0];
			priv->bank[i].npins = regs[1];
			priv->bank[i].name = ofnode_get_name(node);
		}

		node = ofnode_by_compatible(node, TCC_GPIO_COMPATIBLE);
		if (i < UINT_MAX) {
			i++;
		} else {
			BUG();
		}
	}
}

static s32 tcc_pinctrl_init_dt(struct udevice *dev,
					struct telechips_pinctrl_priv *priv) {
	s32 ret = 0;
	ofnode node;
	u32 regs[4];

	node = dev_ofnode(dev);
	if (ofnode_read_u32_array(node, "reg", regs, 4) == 0) {
		//[0] : gpio addr, [1] : gpio range,
		//[2] : pmgpio addr, [3] : pmgpio rnage
		priv->gpio_base = regs[0];
		priv->pmgpio_base = regs[2];
	} else {
		pinctrl_err_log(PINCTRL_ERR_PROBE3, 0);
		ret = -EINVAL;
	}

	if (ret == 0) {
		tcc_pinctrl_dt_bank(priv);

	}

	return ret;

}

static s32 tcc_pinctrl_probe(struct udevice *dev)
{
	struct telechips_pinctrl_priv *priv = dev_get_priv(dev);
	s32 ret = 0;

	if (priv == NULL) {
		BUG();
	}

	if (!ofnode_valid(dev_ofnode(dev))) {
		pinctrl_err_log(PINCTRL_ERR_PROBE1, 0);
		ret = -EINVAL;
	}

	if (ret == 0) {
		ret = tcc_pinctrl_init(priv);
	}

	if (ret == 0) {
		ret = tcc_pinctrl_init_dt(dev, priv);
	}

	return ret;
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

static void pctl_add_wrap(u32 ui_a, u32 ui_b, u32 *ui_ret) {

	if (ui_a > (UINT_MAX - ui_b)) {
		BUG();
	} else {
		*ui_ret = ui_a + ui_b;
	}

}

static u32 get_gpio_base(const struct udevice *dev)
{
	const struct telechips_pinctrl_priv *priv = dev_get_priv(dev);
	return priv->gpio_base;
}

static s32 tcc_gpio_set_function
	(u32 bank_base, u32 pin_num, u32 func, const struct udevice *dev)
{
	const u32 width = 4U;
	u32 func_addr;
	pctl_add_wrap(bank_base, TCC_GPIO_FN0, &func_addr);
	pctl_add_wrap(func_addr, ((pin_num / 8U) * 0x4U), &func_addr);
	u32 bit_shift = (pin_num % 8U) * 4U;

	return integrated_gpio_writel
			(func_addr, bit_shift, width, func, dev);
}

static s32 tcc_gpio_set_mfio
	(u32 bank_base, u32 mfio, u32 mfio_sel, const struct udevice *dev)
{
	const u32 width = 8U;
	u32 mfio_addr;
	u32 gpio_base = get_gpio_base(dev);
	pctl_add_wrap(gpio_base, TCC_GPFLEX_SEL0, &mfio_addr);
	pctl_add_wrap(mfio_addr, ((mfio_sel / 4U) * 0x4U), &mfio_addr);
	u32 bit_shift = (mfio_sel % 4U) * 8U;

	return integrated_gpio_writel
			(mfio_addr, bit_shift, width, mfio, dev);
}

static s32 tcc_gpio_set_drive_strength
	(u32 bank_base, u32 pin_num, u32 ds, const struct udevice *dev)
{
	const u32 width = 2U;
	u32 value = 0U;
	s32 ret = 0U;
	u32 ds_addr;
#if defined(CONFIG_TCC807X)
	u32 ds2_addr;
#endif
	pctl_add_wrap(bank_base, TCC_GPIO_CD0, &ds_addr);
	pctl_add_wrap(ds_addr, ((pin_num /16U) * 0x4U), &ds_addr);
	u32 bit_shift = (pin_num % 16U) * 2U;

	value = ds & TCC_GPIO_DS01_MASK;
	ret =  integrated_gpio_writel
		(ds_addr, bit_shift, width, value, dev);

#if defined(CONFIG_TCC807X)
	pctl_add_wrap(bank_base, TCC_GPIO_CD2, &ds2_addr);
	if (ret == 0U) {
		value = (ds >> 2U) & TCC_GPIO_DS2_MASK;
		ret =  integrated_gpio_writel
			(ds2_addr, pin_num, 1U, value, dev);
	}
#endif
	return ret;
}

static s32 tcc_gpio_pull_enable
	(u32 bank_base, u32 pin_num, u32 pe, const struct udevice *dev)
{
	u32 pe_addr;

	pctl_add_wrap(bank_base, TCC_GPIO_PE, &pe_addr);

	return integrated_gpio_writel
			(pe_addr, pin_num, 1U, pe, dev);
}

static s32 tcc_gpio_pull_select
	(u32 bank_base, u32 pin_num, u32 ps, const struct udevice *dev)
{
	u32 ps_addr;

	pctl_add_wrap(bank_base, TCC_GPIO_PS, &ps_addr);

	return integrated_gpio_writel
			(ps_addr, pin_num, 1U, ps, dev);
}

static s32 tcc_gpio_output_enable
	(u32 bank_base, u32 pin_num, u32 oen, const struct udevice *dev)
{
	u32 oen_addr;

	pctl_add_wrap(bank_base, TCC_GPIO_OEN, &oen_addr);

	return integrated_gpio_writel
			(oen_addr, pin_num, 1U, oen, dev);
}

static s32 tcc_gpio_set_output(u32 bank_base, u32 pin_num, u32 value)
{
	u32 bit;
	u32 tmp_addr;
	uintptr_t pctl_addr;

	if (pin_num > 0x1FU) {
		BUG();
	} else {
		bit = (unsigned int)1U << pin_num;
	}

	if (value == 1U) {	//output high
		pctl_add_wrap(bank_base, TCC_GPIO_OR, &tmp_addr);
		pctl_addr = tmp_addr;
		writel(bit, pctl_addr);
		/* OR register is bit control register, so access directly */
	} else {			//output low
		pctl_add_wrap(bank_base, TCC_GPIO_BIC, &tmp_addr);
		pctl_addr = tmp_addr;
		writel(bit, pctl_addr);
		/* BIC register is bit control register, so access directly */
	}

	return 0;
}

static s32 tcc_gpio_input_enable(
		u32 bank_base, u32 pin_num, u32 ien, const struct udevice *dev)
{
	u32 ien_addr;

	pctl_add_wrap(bank_base, TCC_GPIO_IEN, &ien_addr);

	return integrated_gpio_writel
			(ien_addr, pin_num, 1U, ien, dev);
}

static u32 get_pin_base(const struct udevice *dev, const char *pin_name, u32 *pin_num)
{
	const struct telechips_pinctrl_priv *priv = dev_get_priv(dev);
	u32 i;
	size_t len;
	ulong ul_pin_num;
	u32 ret = 0;

	if ((priv != NULL) && (pin_num != NULL)) {

		for (i = 0; i < priv->nbanks; i++) {
			len = (priv->bank[i].name != NULL)
					? strnlen(priv->bank[i].name, 10U) : 0U;

			if (strncasecmp(pin_name, priv->bank[i].name, len) == 0) {
				if (strict_strtoul(pin_name + len, 10, &ul_pin_num) == 0) {

					if (ul_pin_num <= 0xFFFFFFFFU) {
						*pin_num = (u32)ul_pin_num;
					} else {
						BUG();
					}

#if !defined(CONFIG_TCC807X)
					if (strncasecmp(pin_name, "gpk", 3) != 0) {
						ret = priv->gpio_base
							+ priv->bank[i].bank_base;
					} else {
						ret = priv->pmgpio_base
							+ priv->bank[i].bank_base;
					}
#else
					/*use Normal GPIO for GPIO K in TCC807x*/
					ret = priv->gpio_base
						+ priv->bank[i].bank_base;
#endif
				}
				break;
			}
		}
	}

	return ret;
}

static s32 tcc_gpio_set_input_type
	(u32 bank_base, u32 pin_num, u32 is, const struct udevice *dev)
{
	u32 is_addr;

	pctl_add_wrap(bank_base, TCC_GPIO_IS, &is_addr);

	return integrated_gpio_writel
			(is_addr, pin_num, 1U, is, dev);
}

static s32 tcc_gpio_set_slew_rate
	(u32 bank_base, u32 pin_num, u32 sr, const struct udevice *dev)
{
#if defined(CONFIG_TCC807X)
	return 0;
#else
	u32 sr_addr = bank_base + TCC_GPIO_SR;

	pctl_add_wrap(bank_base, TCC_GPIO_SR, &sr_addr);

	return integrated_gpio_writel
			(sr_addr, pin_num, 1U, sr, dev);
#endif
}

static s32 get_state_count(const struct gpio_state_t *state)
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

	if (state->mfio[0]
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
	s32 ret = 0;

	if (state == NULL) {
		pinctrl_err_log(PINCTRL_ERR_GET_GPIO_STATE1, 0);
		ret = -EINVAL;
	}

	if (ret == 0) {

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
		if (ofnode_read_u32_array(node, "telechips,mfio", (u32 *)&state->mfio, 2) != 0) {
			state->mfio[0] = -1; // mfio num
			state->mfio[1] = -1; // mfid sel
		}
		state->drive_strength = ofnode_read_s32_default
			(node, "telechips,drive-strength", -1);

		ret = get_state_count(state);

	}

	return ret;
}

static void tcc_pinctrl_set_config2(u32 addr, u32 pin_num,
			struct gpio_state_t state, const struct udevice *dev) {

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
		(void)tcc_gpio_set_input_type(addr, pin_num, 1, dev);
	} else if (state.cmos_input != NULL) {
		(void)tcc_gpio_set_input_type(addr, pin_num, 0, dev);
	} else {
		/* empty */
	}

	if (state.slow_slew != NULL) {
		(void)tcc_gpio_set_slew_rate(addr, pin_num, 1, dev);
	} else if (state.fast_slew != NULL) {
		(void)tcc_gpio_set_slew_rate(addr, pin_num, 0, dev);
	} else {
		/* empty */
	}

}

static void tcc_pinctrl_set_config(u32 addr, u32 pin_num,
			struct gpio_state_t state, const struct udevice *dev) {

	if (state.pin_func != -1) {
		(void)tcc_gpio_set_function(addr, pin_num,
			    (u32)state.pin_func, dev);
	}

	if (state.mfio[0] != -1 && state.mfio[1] != -1) {
		// MFIO GPIO Function must be 7
		(void)tcc_gpio_set_function(addr, pin_num,
			    7U, dev);

		// [0] : mfio num, [1] mfio sel
		(void)tcc_gpio_set_mfio(addr, (u32)state.mfio[0], (u32)state.mfio[1], dev);
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

	tcc_pinctrl_set_config2(addr, pin_num, state, dev);

}

static s32 tcc_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	ofnode node;
	s32 pin_count;
	s32 i;
	u32 pin_num;
	const char *name;
	u32 addr;
	const struct telechips_pinctrl_priv *priv;
	struct gpio_state_t state;
	s32 ret = 0;

	if (config == NULL) {
		pinctrl_err_log(PINCTRL_ERR_SET_STATE1, 0);
		ret = -EINVAL;
	}

	if (ret == 0) {
		node = config->node_;

		priv = dev_get_priv(dev);
		if (priv == NULL) {
			pinctrl_err_log(PINCTRL_ERR_SET_STATE2, 0);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		pin_count = ofnode_read_string_count(node, "telechips,pins");
		if (pin_count <= 0) {
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		(void)get_gpio_state(node, &state);

		for (i = 0; i < pin_count; i++) {
			if (ofnode_read_string_index(node, "telechips,pins", i, &name)
					!= 0) {
				continue;
			}

			addr = get_pin_base(dev, name, &pin_num);

			if (addr == 0U) {
				pinctrl_err_log(PINCTRL_ERR_SET_STATE3, 0);
				ret = -EINVAL;
			}

#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
			if (ret == 0) {
				if (check_and_set_bit_ctrl(priv, name, pin_num) != 0) {
					pinctrl_err_log(PINCTRL_ERR_SET_STATE4, 0);
					return -EINVAL;
				}
			}
#endif

			if (ret == 0) {
				tcc_pinctrl_set_config(addr, pin_num, state, dev);
			}
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
	.priv_auto = (s32)sizeof(struct telechips_pinctrl_priv),
	.of_match = telechips_pinctrl_match,
	.ops = &telechips_pinctrl_ops,
	.probe = tcc_pinctrl_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
