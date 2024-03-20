// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/telechips/gpio.h>
#include <mailbox.h>
#include <linux/soc/telechips/tcc_mbox.h>
#include <linux/soc/telechips/tcc_sc_protocol.h>
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
#endif
#if defined(CONFIG_TCC805X)
#define TCC_GPIO_COMPATIBLE ("telechips,tcc805x-gpio")
#endif
#endif

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

/* GPIO Driver Model Standard API */

static u32 get_gpio_addr(struct udevice *dev)
{
	struct tcc_gpio_bank *priv = dev_get_priv(dev);
	u32 ret;

	if (strncasecmp(priv->name, "gpk", 3) != 0) {
		/* except gpio k */
		ret = priv->gpio_base + priv->bank_base;
	} else {
		/* if gpio k */
		ret = priv->pmgpio_base + priv->bank_base;
	}

	return ret;
}

static s32 tcc_gpio_get_value(struct udevice *dev, u32 offset)
{
	u32 addr = get_gpio_addr(dev);
	u32 value = readl((ulong)addr) & TCC_GPIO_BIT(offset);

	if (value != 0U) {
		return 1;
		/* comment for QAC, codesonar, kernel coding style */
	} else {
		return 0;
		/* comment for QAC, codesonar, kernel coding style */
	}
}

static s32 tcc_gpio_set_value
	(struct udevice *dev, u32 offset, s32 value)
{
	u32 addr = get_gpio_addr(dev);

	if (value != 0) {
		writel(TCC_GPIO_BIT(offset), (ulong)(addr + TCC_GPIO_OR));
		/* OR register is bit control register, so access directly */
	} else {
		writel(TCC_GPIO_BIT(offset), (ulong)(addr + TCC_GPIO_BIC));
		/* BIC register is bit control register, so access directly */
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
		pr_err("[ERROR][GPIO] bank_num == NULL\n");
		return -EINVAL;
	}

	for (i = 0U; i < 18U; i++) {
		if (strncmp(bank_list[i], bank_name, 6)
				== 0) {
			*bank_num = i;
			return 0;
		}
	}

	pr_err("[ERROR][GPIO] Invalid bank_name(%s)\n", bank_name);

	return -EINVAL;
}

static s32 dm_gpio_get_bit_ctrl
	(struct tcc_gpio_bank *priv, u32 pin_num, u32 *core_num)
{
	u32 bank_num = 0;
	u32 address;
	u32 value;

	if (priv == NULL) {
		pr_err("[ERROR][GPIO] priv == NULL\n");
		return -EINVAL;
	}

	if (get_bank_num_for_bit_ctrl(priv->name, &bank_num) != 0) {
		pr_err("[ERROR][GPIO] Failed to get bank num\n");
		return -EINVAL;
	}

	if (core_num == NULL) {
		pr_err("[ERROR][GPIO] core_num == NULL\n");
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
		pr_err("[ERROR][GPIO] priv == NULL\n");
		return -EINVAL;
	}

	if (get_bank_num_for_bit_ctrl(priv->name, &bank_num) != 0) {
		pr_err("[ERROR][GPIO] Failed to get bank num\n");
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
		priv->handle->ops.reg_ops.request_reg_set(
				priv->handle, address, bit_num, 2U, core_num);
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
#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
	s32 ret;
	struct tcc_gpio_bank *priv = dev_get_priv(dev);
	u32 bit_control;

	if (dm_gpio_get_bit_ctrl(priv, offset, &bit_control) != 0) {
		pr_err("[ERROR][GPIO] Failed to get bit ctrl\n");
		return -EINVAL;
	}

	if (bit_control != priv->core_num) {
		if (bit_control == 0U) {
			// if the bit is not used anywhere(0),
			//	set bit to core number
			dm_gpio_set_bit_contrl(priv, offset, priv->core_num);
		} else {
			pr_err("[ERROR][GPIO] Invalid bit control value\n");
			pr_err(
				"\tbit control(%u) != core num(%u)\n"
				"\tthis gpio already used at another core\n",
				bit_control, priv->core_num);
			return -EINVAL;
		}
	}

	/* bit control address */
	addr += 0x1000U + (0x800U * (priv->core_num));

	ret = dm_gpio_set_reg
		(addr + TCC_GPIO_OEN, offset, 1U, 0U);
	if (ret != 0) {
		pr_err("[ERROR][GPIO] Failed to disable output\n");
		return ret;
	}

	dm_gpio_set_reg
		(addr + TCC_GPIO_IEN, offset, 1U, 1U);
#else
#if defined(CONFIG_TCC_GPIO_SCFW)
	s32 ret;
	struct tcc_gpio_bank *priv = dev_get_priv(dev);

	if (priv->handle != NULL) {
		ret = priv->handle->ops.reg_ops.request_reg_set(
				priv->handle, addr + TCC_GPIO_OEN,
				offset, 1U, 0U);
		if (ret != 0) {
			(void)pr_err(
				"[ERROR][GPIO][%s:%d] Failed to disable output\n"
				, __func__, __LINE__);
			return ret;
		}

		ret = priv->handle->ops.reg_ops.request_reg_set(
				priv->handle, addr + TCC_GPIO_IEN,
				offset, 1U, 1U);
		if (ret != 0) {
			(void)pr_err(
				"[ERROR][GPIO][%s:%d] Failed to enable input\n"
				, __func__, __LINE__);
			return ret;
		}
	} else {
		return -EINVAL;
	}
#else
	u32 oen;
	u32 ien;

	/* Disable Output */
	oen = readl((ulong)(addr + TCC_GPIO_OEN))
		& (~TCC_GPIO_BIT(offset));
	writel(oen, (ulong)(addr + TCC_GPIO_OEN));

	/* Enable Input */
	ien = readl((ulong)(addr + TCC_GPIO_IEN))
		| TCC_GPIO_BIT(offset);
	writel(ien, (ulong)(addr + TCC_GPIO_IEN));
#endif
#endif

	return 0;
}

static s32 tcc_gpio_direction_output(struct udevice *dev, u32 offset, s32 value)
{
	u32 addr = get_gpio_addr(dev);
#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
	s32 ret;
	struct tcc_gpio_bank *priv = dev_get_priv(dev);
	u32 bit_control;

	if (dm_gpio_get_bit_ctrl(priv, offset, &bit_control) != 0) {
		pr_err("[ERROR][GPIO] Failed to get bit ctrl\n");
		return -EINVAL;
	}

	if (bit_control != priv->core_num) {
		if (bit_control == 0U) {
			// if the bit is not used anywhere(0),
			//	set bit to core number
			dm_gpio_set_bit_contrl(priv, offset, priv->core_num);
		} else {
			pr_err("[ERROR][GPIO] Invalid bit control value\n");
			pr_err(
				"\tbit ctrl num(%u) != core num(%u)\n"
				"\tthis gpio already used at another core\n",
				bit_control, priv->core_num);

			return -EINVAL;
		}
	}

	ret = dm_gpio_set_reg
		(addr + TCC_GPIO_IEN, offset, 1U, 0U);
	if (ret != 0) {
		pr_err("[ERROR][GPIO] Failed to disable input\n");
		return ret;
	}

	dm_gpio_set_reg
		(addr + TCC_GPIO_OEN, offset, 1U, 1U);
#else
#if defined(CONFIG_TCC_GPIO_SCFW)
	s32 ret;
	struct tcc_gpio_bank *priv = dev_get_priv(dev);

	if (priv->handle != NULL) {
		ret = priv->handle->ops.reg_ops.request_reg_set(
				priv->handle, addr + TCC_GPIO_IEN,
				offset, 1U, 0U);
		if (ret != 0) {
			(void)pr_err(
				"[ERROR][GPIO][%s:%d] Failed to disable input\n"
				, __func__, __LINE__);
			return ret;
		}

		ret = priv->handle->ops.reg_ops.request_reg_set(
				priv->handle, addr + TCC_GPIO_OEN,
				offset, 1U, 1U);
		if (ret != 0) {
			(void)pr_err(
				"[ERROR][GPIO][%s:%d] Failed to enable output\n"
				, __func__, __LINE__);
			return ret;
		}
	} else {
		return -EINVAL;
	}
#else
	u32 oen;
	u32 ien;

	/* Disable Input */
	ien = readl((ulong)(addr + TCC_GPIO_IEN))
		& (~TCC_GPIO_BIT(offset));
	writel(ien, (ulong)(addr + TCC_GPIO_IEN));

	/* Enable output */
	oen = readl((ulong)(addr + TCC_GPIO_OEN))
		| TCC_GPIO_BIT(offset);
	writel(oen, (ulong)(addr + TCC_GPIO_OEN));
#endif
#endif

	/* Set data regigster */
	return tcc_gpio_set_value(dev, offset, value);
}

static s32 tcc_gpio_get_direction(struct udevice *dev, u32 offset)
{
	u32 oen;
	u32 ien;
	u32 addr = get_gpio_addr(dev);

	oen = readl((ulong)(addr + TCC_GPIO_OEN))
		& TCC_GPIO_BIT(offset);

	ien = readl((ulong)(addr + TCC_GPIO_IEN))
		& TCC_GPIO_BIT(offset);

	if (oen != 0U) {
		return (s32)GPIOF_OUTPUT;
		/* comment for QAC, codesonar, kernel coding style */
	} else if (ien != 0U) {
		return (s32)GPIOF_INPUT;
		/* comment for QAC, codesonar, kernel coding style */
	} else {
		return (s32)GPIOF_UNUSED;
		/* comment for QAC, codesonar, kernel coding style */
	}
}

static s32 tcc_gpio_get_function(struct udevice *dev, u32 offset)
{
	u32 addr;
	u32 fn_offset;
	u32 reg_fn;
	u32 func;

	fn_offset = (offset / 8U);
	addr = get_gpio_addr(dev) + TCC_GPIO_FN(fn_offset);
	reg_fn = readl((ulong)addr);
	func = ((reg_fn >> ((offset % 8U) * 4U)) & 0xfU);

	if (func == 0U) {
		return tcc_gpio_get_direction(dev, offset);
		/* comment for QAC, codesonar, kernel coding style */
	} else {
		return (s32)GPIOF_FUNC;
		/* comment for QAC, codesonar, kernel coding style */
	}
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
	s32 ret;
	char *buf = NULL;

	if (core_num == NULL) {
		pr_err("[ERROR][GPIO] core_num == NULL\n");
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

static s32 tcc_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct tcc_gpio_bank *priv = dev_get_priv(dev);
	ofnode node;
	u32 regs[4];

	if (!dev_of_valid(dev)) {
		(void)pr_err(
			"[ERROR][GPIO][%s:%d] dev == NULL\n"
			, __func__, __LINE__);
		return -EINVAL;
	}

#if defined(CONFIG_TCC_GPIO_BIT_CTRL)
	if (get_core_num(&(priv->core_num)) != 0) {
		(void)pr_err(
			"[ERROR][GPIO][%s:%d] Failed to get core num\n"
			, __func__, __LINE__);
		return -EINVAL;
	}
#endif

#if defined(CONFIG_TCC_GPIO_SCFW)
	priv->handle = tcc_sc_fw_get_handle_by_name();
	if (!priv->handle) {
		(void)pr_err(
			"[ERROR][GPIO][%s:%d] priv->handle == NULL\n"
			, __func__, __LINE__);
		return -EINVAL;
	}
#endif

	node = dev_ofnode(dev);
	priv->name = ofnode_get_name(node);
	if (ofnode_read_u32_array(node, "reg", regs, 2) == 0) {
		priv->bank_base = regs[0];	//base address
		priv->npins = regs[1];		//count
	}

	node = ofnode_by_compatible(ofnode_null(), "telechips,pinctrl");

	if (ofnode_valid(node)) {
		if (ofnode_read_u32_array(node, "reg", regs, 4) == 0) {
			priv->gpio_base = regs[0];
			priv->pmgpio_base = regs[2];
		}
	}

	uc_priv->bank_name = priv->name;
	uc_priv->gpio_count = priv->npins;
	uc_priv->gpio_base = priv->gpio_base + priv->bank_base;

	return 0;
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
	.priv_auto_alloc_size = (s32)sizeof(struct tcc_gpio_bank),
	.probe = tcc_gpio_probe,
};
