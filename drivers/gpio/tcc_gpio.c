// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
//#include <asm/telechips/gpio.h>
#include <mailbox.h>
#include <linux/bug.h>
#include <cpu.h>

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
#define TCC_GPIO_FN(x)	(0x30U + ((x) * 0x4U))

#define TCC_GPIO_BIT(x) ((u32)1U << (x))

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

#define GPIO_ERR_BANK_GET_BIT1				(0)
#define GPIO_ERR_BANK_GET_BIT2				(1)
#define GPIO_ERR_PIN_GET_BIT1				(2)
#define GPIO_ERR_PIN_GET_BIT2				(3)
#define GPIO_ERR_PIN_GET_BIT3				(4)
#define GPIO_ERR_PIN_SET_BIT1				(5)
#define GPIO_ERR_PIN_SET_BIT2				(6)
#define GPIO_ERR_DIR_INPUT1					(7)
#define GPIO_ERR_DIR_INPUT2					(8)
#define GPIO_ERR_DIR_INPUT3					(9)
#define GPIO_ERR_DIR_INPUT4					(10)
#define GPIO_ERR_DIR_INPUT5					(11)
#define GPIO_ERR_DIR_OUTPUT1				(12)
#define GPIO_ERR_DIR_OUTPUT2				(13)
#define GPIO_ERR_DIR_OUTPUT3				(14)
#define GPIO_ERR_DIR_OUTPUT4				(15)
#define GPIO_ERR_DIR_OUTPUT5				(16)
#define GPIO_ERR_GET_CORE_NUM				(17)
#define GPIO_ERR_PROBE1						(18)
#define GPIO_ERR_PROBE2						(19)
#define GPIO_ERR_PROBE3						(20)

static const char *err_log[21] = {
	" bank_num == NULL",			//0
	"Invalid bank_name",			//1
	"priv == NULL",					//2
	"Failed to get bank num",		//3
	"core_num == NULL",				//4
	"priv == NULL",					//5
	"Failed to get bank num",		//6
	"Failed to get bit ctrl",		//7
	"Invalid bit control value",	//8
	"Failed to disable output",		//9
	"Failed to disable output",		//10
	"Failed to enable input",		//11
	"Failed to get bit ctrl",		//12
	"Invalid bit control value",	//13
	"Failed to disable input",		//14
	"Failed to disable input",		//15
	"Failed to enable output",		//16
	"core_num == NULL",				//17
	"dev == NULL",					//18
	"Failed to get core num",		//29
	"priv->handle == NULL"			//20
};

//core_num
//	- common : 0
//	- A72 : 1
//	- A53 : 2
struct tcc_gpio_bank {
	const char *name;
	u32 gpio_base;
	u32 pmgpio_base;
	u32 bank_base;
	u32 npins;
	u32 core_num;
	const struct tcc_sc_fw_handle *handle;
};

static void gpio_err_log(u32 num, u32 val)
{
	(void)pr_err("[ERROR][GPIO] err_num:%s, val : %u\n", err_log[num], val);
}

static void gpio_add_wrap(u32 ui_a, u32 ui_b, u32 *ui_ret)
{
	if (ui_a > (UINT_MAX - ui_b)) {
		BUG();
	} else {
		*ui_ret = ui_a + ui_b;
	}
}

/* GPIO Driver Model Standard API */

static u32 get_gpio_addr(const struct udevice *dev)
{
	const struct tcc_gpio_bank *priv = dev_get_priv(dev);
	u32 ret = 0;

#if !defined(CONFIG_TCC807X)
	if (strncasecmp(priv->name, "gpk", 3) != 0) {
		/* except gpio k */
		ret = priv->gpio_base + priv->bank_base;
	} else {
		/* if gpio k */
		ret = priv->pmgpio_base + priv->bank_base;
	}
#else
	/*use Normal GPIO for GPIO K in TCC807x*/
	ret = priv->gpio_base + priv->bank_base;
#endif
	return ret;
}

static s32 tcc_gpio_get_value(struct udevice *dev, u32 offset)
{
	u32 addr = get_gpio_addr(dev);
	s32 ret = 0;

	if (offset > 31U) {
		ret = -1;
	}

	if (ret != -1) {
		u32 value = readl((ulong)addr) & TCC_GPIO_BIT(offset);

		if (value != 0U) {
			ret = 1;
			/* comment for QAC, codesonar, kernel coding style */
		} else {
			ret = 0;
			/* comment for QAC, codesonar, kernel coding style */
		}
	}
	return ret;
}

static s32 tcc_gpio_set_value
	(struct udevice *dev, u32 offset, s32 value)
{
	u32 addr = get_gpio_addr(dev);
	u32 gpio_addr;
	s32 ret = 0;

	if (offset > 31U) {
		ret = -1;
	}

	if (ret != -1) {
		if (value != 0) {
			gpio_add_wrap(addr, TCC_GPIO_OR, &gpio_addr);
			writel(TCC_GPIO_BIT(offset), (ulong)gpio_addr);
			/* OR register is bit control register, so access directly */
		} else {
			gpio_add_wrap(addr, TCC_GPIO_BIC, &gpio_addr);
			writel(TCC_GPIO_BIT(offset), (ulong)gpio_addr);
			/* BIC register is bit control register, so access directly */
		}
	}

	return 0;
}

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
		gpio_err_log(GPIO_ERR_BANK_GET_BIT1, 0);
		return -EINVAL;
	}

	for (i = 0U; i < 18U; i++) {
		if (strncmp(bank_list[i], bank_name, 6)
				== 0) {
			*bank_num = i;
			return 0;
		}
	}

	gpio_err_log(GPIO_ERR_BANK_GET_BIT2, 0);

	return -EINVAL;
}

static s32 dm_gpio_get_bit_ctrl
	(struct tcc_gpio_bank *priv, u32 pin_num, u32 *core_num)
{
	u32 bank_num = 0;
	u32 address;
	u32 value;

	if (priv == NULL) {
		gpio_err_log(GPIO_ERR_PIN_GET_BIT1, 0);
		return -EINVAL;
	}

	if (get_bank_num_for_bit_ctrl(priv->name, &bank_num) != 0) {
		gpio_err_log(GPIO_ERR_PIN_GET_BIT2, 0);
		return -EINVAL;
	}

	if (core_num == NULL) {
		gpio_err_log(GPIO_ERR_PIN_GET_BIT3, 0);
		return -EINVAL;
	}

	address = 0x14200000U + 0x800U		//bit ctrl offset
			+ (0x8U * bank_num)			//bank offset
			+ (0x4U * (pin_num / 16U));	//pin offset
	value = readl((ulong)address);

	*core_num = (value >> (pin_num / 2U)) & 0x2U;

	return 0;
}

static s32 dm_gpio_set_bit_contrl
	(struct tcc_gpio_bank *priv, u32 pin_num, u32 core_num)
{
	u32 bank_num;
	u32 address;
	u32 bit_num;

	if (priv == NULL) {
		gpio_err_log(GPIO_ERR_PIN_SET_BIT1, 0);
		return -EINVAL;
	}

	if (get_bank_num_for_bit_ctrl(priv->name, &bank_num) != 0) {
		gpio_err_log(GPIO_ERR_PIN_SET_BIT2, 0);
		return -EINVAL;
	}

	bit_num = pin_num / 2U;
	address = 0x14200000U + 0x800U	//bit ctrl offset
			+ (0x8U * bank_num)			//bank offset
			+ (0x4U * (pin_num / 16U));	//pin offset
#if !defined(CONFIG_TCC_GPIO_SCFW)
	const u32 mask = 0x3U;
	u32 reg_data = readl((ulong)address);

	reg_data &= ~(mask << bit_num);
	reg_data |= (mask & core_num) << bit_num;
	writel(reg_data, (ulong)address);
#else

	if (priv->handle != NULL) {
		priv->handle->ops.reg_ops.request_reg_set(priv->handle, address, bit_num, 2U, core_num);
	} else {
		return -EINVAL;
	}
#endif

	return 0;
}
#endif

#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
static s32 dm_gpio_set_reg
	(u32 address, u32 bit_number, u32 width, u32 value)
{
	/* set Register */
	u32 mask;
	u32 reg_data;

	if (width == 0U) {
		return -EINVAL;
		/* comment for QAC, codesonar, kernel coding style */
	}

	if (address == 0U) {
		return -EINVAL;
		/* comment for QAC, codesonar, kernel coding style */
	}

	reg_data = readl((ulong)address);
	mask = ((u32)1U << width) - 1U;
	reg_data &= ~(mask << bit_number);
	reg_data |= (mask & value) << bit_number;

	writel(reg_data, (ulong)address);

	return 0;
}
#endif

static s32 tcc_gpio_direction_input(struct udevice *dev, u32 offset)
{
	u32 addr = get_gpio_addr(dev);
	s32 ret = 0;
#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
	u32 gpio_addr;
	struct tcc_gpio_bank *priv = dev_get_priv(dev);
	u32 bit_control;

	if (dm_gpio_get_bit_ctrl(priv, offset, &bit_control) != 0) {
		gpio_err_log(GPIO_ERR_DIR_INPUT1, 0);
		ret = -EINVAL;
	}

	if (ret == 0) {
		if (bit_control != priv->core_num) {
			if (bit_control == 0U) {
				// if the bit is not used anywhere(0),
				//	set bit to core number
				dm_gpio_set_bit_contrl(priv, offset, priv->core_num);
			} else {
				gpio_err_log(GPIO_ERR_DIR_INPUT2, bit_control);
				ret = -EINVAL;
			}
		}
		if (ret == 0) {
			/* bit control address */
			addr += 0x1000U + (0x800U * (priv->core_num));

			gpio_add_wrap(addr, TCC_GPIO_OEN, &gpio_addr);
			ret = dm_gpio_set_reg
				(gpio_addr, offset, 1U, 0U);
			if (ret != 0) {
				gpio_err_log(GPIO_ERR_DIR_INPUT3, 0);
			}
			if (ret == 0) {
				gpio_add_wrap(addr, TCC_GPIO_IEN, &gpio_addr);
				dm_gpio_set_reg
					(gpio_addr, offset, 1U, 1U);
			}
		}
	}
#else
#if defined(CONFIG_TCC_GPIO_SCFW)
	u32 gpio_addr;
	const struct tcc_gpio_bank *priv = dev_get_priv(dev);

	if (priv == NULL) {
		ret = -1;
	}
	if (ret != -1) {
		if (priv->handle != NULL) {
			gpio_add_wrap(addr, TCC_GPIO_OEN, &gpio_addr);
			ret = priv->handle->ops.reg_ops.request_reg_set(
					priv->handle, gpio_addr,
					offset, 1U, 0U);
			if (ret != 0) {
				gpio_err_log(GPIO_ERR_DIR_INPUT4, 0);
			} else {

				gpio_add_wrap(addr, TCC_GPIO_IEN, &gpio_addr);
				ret = priv->handle->ops.reg_ops.request_reg_set(
						priv->handle, gpio_addr,
						offset, 1U, 1U);
				if (ret != 0) {
					gpio_err_log(GPIO_ERR_DIR_INPUT5, 0);
				}
			}
		} else {
			ret = -EINVAL;
		}
	}
#else
	u32 oen;
	u32 ien;

	if (offset > 31U) {
		ret = -1;
	}

	if (ret != -1) {
		/* Disable Output */
		oen = readl((ulong)(addr + TCC_GPIO_OEN))
			& (~TCC_GPIO_BIT(offset));
		writel(oen, (ulong)(addr + TCC_GPIO_OEN));

		/* Enable Input */
		ien = readl((ulong)(addr + TCC_GPIO_IEN))
			| TCC_GPIO_BIT(offset);
		writel(ien, (ulong)(addr + TCC_GPIO_IEN));
	}
#endif
#endif

	return ret;
}

static s32 tcc_gpio_direction_output(struct udevice *dev, u32 offset, s32 value)
{
	u32 addr = get_gpio_addr(dev);
	u32 gpio_addr;
#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
	s32 ret = 0;
	const struct tcc_gpio_bank *priv = dev_get_priv(dev);
	u32 bit_control;

	if (dm_gpio_get_bit_ctrl(priv, offset, &bit_control) != 0) {
		gpio_err_log(GPIO_ERR_DIR_OUTPUT1, 0);
		ret = -EINVAL;
	}
	if (ret == 0) {
		if (bit_control != priv->core_num) {
			if (bit_control == 0U) {
				// if the bit is not used anywhere(0),
				//	set bit to core number
				dm_gpio_set_bit_contrl(priv, offset, priv->core_num);
			} else {
				gpio_err_log(GPIO_ERR_DIR_OUTPUT2, bit_control);

				ret = -EINVAL;
			}
		}
		if (ret == 0) {
			ret = dm_gpio_set_reg
				(addr + TCC_GPIO_IEN, offset, 1U, 0U);
			if (ret != 0) {
				gpio_err_log(GPIO_ERR_DIR_OUTPUT3, 0);
			} else {
				dm_gpio_set_reg
					(addr + TCC_GPIO_OEN, offset, 1U, 1U);
			}
		}
	}
#else
#if defined(CONFIG_TCC_GPIO_SCFW)
	s32 ret = 0;
	const struct tcc_gpio_bank *priv = dev_get_priv(dev);
	if (priv == NULL) {
		ret = -1;
	}
	if (ret != -1) {
		if (priv->handle != NULL) {
			gpio_add_wrap(addr, TCC_GPIO_IEN, &gpio_addr);
			ret = priv->handle->ops.reg_ops.request_reg_set(
					priv->handle, gpio_addr,
					offset, 1U, 0U);
			if (ret != 0) {
				gpio_err_log(GPIO_ERR_DIR_OUTPUT4, 0);
			} else {
				gpio_add_wrap(addr, TCC_GPIO_OEN, &gpio_addr);
				ret = priv->handle->ops.reg_ops.request_reg_set(priv->handle, gpio_addr, offset, 1U, 1U);
				if (ret != 0) {
					gpio_err_log(GPIO_ERR_DIR_OUTPUT5, 0);
				}
			}
		} else {
			ret = -EINVAL;
		}
	}
#else
	s32 ret = 0;
	u32 oen;
	u32 ien;

	if (offset > 31U) {
		ret = -1;
	}

	if (ret != -1) {
		/* Disable Input */
		gpio_add_wrap(addr, TCC_GPIO_IEN, &gpio_addr);
		ien = readl((ulong)gpio_addr)
			& (~TCC_GPIO_BIT(offset));
		writel(ien, (ulong)(gpio_addr));

		/* Enable output */
		gpio_add_wrap(addr, TCC_GPIO_OEN, &gpio_addr);
		oen = readl((ulong)(gpio_addr))
			| TCC_GPIO_BIT(offset);
		writel(oen, (ulong)(gpio_addr));
	}
#endif
#endif
	if (ret == 0) {
		ret = tcc_gpio_set_value(dev, offset, value);
	}
	/* Set data regigster */
	return ret;
}

static s32 tcc_gpio_get_direction(struct udevice *dev, u32 offset)
{
	u32 oen;
	u32 ien;
	u32 addr = get_gpio_addr(dev);
	u32 gpio_addr;
	s32 ret = 0;

	if (offset > 31U) {
		ret = -1;
	}

	if (ret != -1) {
		gpio_add_wrap(addr, TCC_GPIO_OEN, &gpio_addr);
		oen = readl((ulong)gpio_addr)
			& TCC_GPIO_BIT(offset);

		gpio_add_wrap(addr, TCC_GPIO_IEN, &gpio_addr);
		ien = readl((ulong)gpio_addr)
			& TCC_GPIO_BIT(offset);

		if (oen != 0U) {
			ret = (s32)GPIOF_OUTPUT;
			/* comment for QAC, codesonar, kernel coding style */
		} else if (ien != 0U) {
			ret = (s32)GPIOF_INPUT;
			/* comment for QAC, codesonar, kernel coding style */
		} else {
			ret = (s32)GPIOF_UNUSED;
			/* comment for QAC, codesonar, kernel coding style */
		}
	}
	return ret;
}

static s32 tcc_gpio_get_function(struct udevice *dev, u32 offset)
{
	u32 addr;
	u32 fn_offset;
	u32 reg_fn;
	u32 func;
	s32 ret = 0;
	u32 gpio_addr;

	if (offset > 31U) {
		ret = -1;
	}

	if (ret != -1) {
		fn_offset = (offset / 8U);
		addr = get_gpio_addr(dev);
		gpio_add_wrap(addr, TCC_GPIO_FN(fn_offset), &gpio_addr);
		reg_fn = readl((ulong)gpio_addr);
		func = ((reg_fn >> ((offset % 8U) * 4U)) & 0xfU);

		if (func == 0U) {
			ret = tcc_gpio_get_direction(dev, offset);
			/* comment for QAC, codesonar, kernel coding style */
		} else {
			ret = (s32)GPIOF_FUNC;
			/* comment for QAC, codesonar, kernel coding style */
		}
	}
	return ret;
}

static const struct dm_gpio_ops tcc_gpio_ops = {
	.direction_input = tcc_gpio_direction_input,
	.direction_output = tcc_gpio_direction_output,
	.get_value = tcc_gpio_get_value,
	.set_value = tcc_gpio_set_value,
	.get_function = tcc_gpio_get_function,
};

#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
static s32 get_core_num(u32 *core_num)
{
	struct udevice *dev;
	char desc[512];
	s32 ret = 0;
	char *buf = NULL;

	if (core_num == NULL) {
		gpio_err_log(GPIO_ERR_GET_CORE_NUM, 0);
		return -EINVAL;
	}

	ret = uclass_first_device_err(UCLASS_CPU, &dev);
	if (ret != 0) {
		debug("%s: Could not get CPU device (err = %d)\n",
				__func__, ret);
		return ret;
	}

	ret = cpu_get_desc(dev, desc, sizeof(desc));
	if (ret != 0) {
		debug("%s: Could not get CPU description (err = %d)\n",
				dev->name, ret);
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

static s32 tcc_gpio_init_dt(struct udevice *dev, struct tcc_gpio_bank *priv)
{
	u32 regs[4];
	ofnode node;
	s32 ret = 0;

	node = dev_ofnode(dev);
	priv->name = ofnode_get_name(node);
	if (ofnode_read_u32_array(node, "reg", regs, 2) == 0) {
		priv->bank_base = regs[0];	//base address
		priv->npins = regs[1];		//count
	} else {
		ret = -1;
	}

	if (ret != -1) {
		node = ofnode_by_compatible(ofnode_null(), "telechips,pinctrl");

		if (ofnode_valid(node)) {
			if (ofnode_read_u32_array(node, "reg", regs, 4) == 0) {
				priv->gpio_base = regs[0];
				priv->pmgpio_base = regs[2];
			} else {
				ret = -1;
			}
		}
	}
	return ret;
}

static s32 tcc_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct tcc_gpio_bank *priv = dev_get_priv(dev);
	s32 ret = 0;

	if (priv == NULL) {
		ret = -1;
	}
	if (ret == 0) {
		if (!ofnode_valid(dev_ofnode(dev))) {
			gpio_err_log(GPIO_ERR_PROBE1, 0);
			ret = -EINVAL;
		}

		if (ret == 0) {
#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
			if (get_core_num(&(priv->core_num)) != 0) {
				gpio_err_log(GPIO_ERR_PROBE2, 0);
				ret = -EINVAL;
			}
#endif

#if defined(CONFIG_TCC_GPIO_SCFW)
			priv->handle = tcc_sc_fw_get_handle_by_name();
			if (priv->handle == NULL) {
				gpio_err_log(GPIO_ERR_PROBE3, 0);
				ret = -EINVAL;
			}
#endif
			if (ret == 0) {
				ret = tcc_gpio_init_dt(dev, priv);
				if (ret == 0) {
					uc_priv->bank_name = priv->name;
					uc_priv->gpio_count = priv->npins;
					uc_priv->gpio_base = priv->gpio_base + priv->bank_base;
				}
			}
		}
	}
	return ret;
}

static const struct udevice_id tcc_gpio_ids[2] = {
	{ .compatible = TCC_GPIO_COMPATIBLE},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(tcc_gpio) = {
	.name = "tcc_gpio",
	.id = UCLASS_GPIO,
	.of_match = tcc_gpio_ids,
	.ops = &tcc_gpio_ops,
	.priv_auto = (s32)sizeof(struct tcc_gpio_bank),
	.probe = tcc_gpio_probe,
};
