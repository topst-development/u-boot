// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/telechips/gpio.h>
#include <mailbox.h>
#include <linux/soc/telechips/tcc_mbox.h>
#include <linux/soc/telechips/tcc_sc_protocol.h>

struct ext_gpio {
	u32 i2c_ch;
	u32 addr;
	u32 gpio_id;
	s32 (*config)(struct ext_gpio *ext, u32 nr, u32 flags);
	void (*set)(struct ext_gpio *ext, u32 nr, u32 on);
	s32 (*get)(struct ext_gpio *ext, u32 nr);
};

static struct ext_gpio *ext_gpios[TCC_EXT_GPIO_MAX];
static struct gpioregs GPIO_REGS[14] = {
	{ /* GPIO A */
		.data		= GPIO_REG(0x000U),
		.out_en		= GPIO_REG(0x004U),
		.out_or		= GPIO_REG(0x008U),
		.out_bic	= GPIO_REG(0x00CU),
		.out_xor	= GPIO_REG(0x010U),
		.strength0	= GPIO_REG(0x014U),
		.strength1	= GPIO_REG(0x018U),
		.pull_enable	= GPIO_REG(0x01CU),
		.pull_select	= GPIO_REG(0x020U),
		.in_en		= GPIO_REG(0x024U),
		.in_type	= GPIO_REG(0x028U),
		.slew_rate	= GPIO_REG(0x02CU),
		.func_select0	= GPIO_REG(0x030U),
		.func_select1	= GPIO_REG(0x034U),
		.func_select2	= GPIO_REG(0x038U),
		.func_select3	= GPIO_REG(0x03CU),
	},
	{ /* GPIO B */
		.data		= GPIO_REG(0x040U),
		.out_en		= GPIO_REG(0x044U),
		.out_or		= GPIO_REG(0x048U),
		.out_bic	= GPIO_REG(0x04CU),
		.out_xor	= GPIO_REG(0x050U),
		.strength0	= GPIO_REG(0x054U),
		.strength1	= GPIO_REG(0x058U),
		.pull_enable	= GPIO_REG(0x05CU),
		.pull_select	= GPIO_REG(0x060U),
		.in_en		= GPIO_REG(0x064U),
		.in_type	= GPIO_REG(0x068U),
		.slew_rate	= GPIO_REG(0x06CU),
		.func_select0	= GPIO_REG(0x070U),
		.func_select1	= GPIO_REG(0x074U),
		.func_select2	= GPIO_REG(0x078U),
		.func_select3	= GPIO_REG(0x07CU),
	},
	{ /* GPIO C */
		.data		= GPIO_REG(0x080U),
		.out_en		= GPIO_REG(0x084U),
		.out_or		= GPIO_REG(0x088U),
		.out_bic	= GPIO_REG(0x08CU),
		.out_xor	= GPIO_REG(0x090U),
		.strength0	= GPIO_REG(0x094U),
		.strength1	= GPIO_REG(0x098U),
		.pull_enable	= GPIO_REG(0x09CU),
		.pull_select	= GPIO_REG(0x0A0U),
		.in_en		= GPIO_REG(0x0A4U),
		.in_type	= GPIO_REG(0x0A8U),
		.slew_rate	= GPIO_REG(0x0ACU),
		.func_select0	= GPIO_REG(0x0B0U),
		.func_select1	= GPIO_REG(0x0B4U),
		.func_select2	= GPIO_REG(0x0B8U),
		.func_select3	= GPIO_REG(0x0BCU),
	},
	{ /* GPIO E */
		.data		= GPIO_REG(0x100U),
		.out_en		= GPIO_REG(0x104U),
		.out_or		= GPIO_REG(0x108U),
		.out_bic	= GPIO_REG(0x10CU),
		.out_xor	= GPIO_REG(0x110U),
		.strength0	= GPIO_REG(0x114U),
		.strength1	= GPIO_REG(0x118U),
		.pull_enable	= GPIO_REG(0x11CU),
		.pull_select	= GPIO_REG(0x120U),
		.in_en		= GPIO_REG(0x124U),
		.in_type	= GPIO_REG(0x128U),
		.slew_rate	= GPIO_REG(0x12CU),
		.func_select0	= GPIO_REG(0x130U),
		.func_select1	= GPIO_REG(0x134U),
		.func_select2	= GPIO_REG(0x138U),
		.func_select3	= GPIO_REG(0x13CU),
	},
	{ /* GPIO G */
		.data		= GPIO_REG(0x180U),
		.out_en		= GPIO_REG(0x184U),
		.out_or		= GPIO_REG(0x188U),
		.out_bic	= GPIO_REG(0x18CU),
		.out_xor	= GPIO_REG(0x190U),
		.strength0	= GPIO_REG(0x194U),
		.strength1	= GPIO_REG(0x198U),
		.pull_enable	= GPIO_REG(0x19CU),
		.pull_select	= GPIO_REG(0x1A0U),
		.in_en		= GPIO_REG(0x1A4U),
		.in_type	= GPIO_REG(0x1A8U),
		.slew_rate	= GPIO_REG(0x1ACU),
		.func_select0	= GPIO_REG(0x1B0U),
		.func_select1	= GPIO_REG(0x1B4U),
		.func_select2	= GPIO_REG(0x1B8U),
		.func_select3	= GPIO_REG(0x1BCU),
	},
	{ /* GPIO H */
		.data		= GPIO_REG(0x640U),
		.out_en		= GPIO_REG(0x644U),
		.out_or		= GPIO_REG(0x648U),
		.out_bic	= GPIO_REG(0x64CU),
		.out_xor	= GPIO_REG(0x650U),
		.strength0	= GPIO_REG(0x654U),
		.strength1	= GPIO_REG(0x658U),
		.pull_enable	= GPIO_REG(0x65CU),
		.pull_select	= GPIO_REG(0x660U),
		.in_en		= GPIO_REG(0x664U),
		.in_type	= GPIO_REG(0x668U),
		.slew_rate	= GPIO_REG(0x66CU),
		.func_select0	= GPIO_REG(0x670U),
		.func_select1	= GPIO_REG(0x674U),
		.func_select2	= GPIO_REG(0x678U),
		.func_select3	= GPIO_REG(0x67CU),
	},
	{ /* GPIO K */
		.data		= PMGPIO_REG(0x680U),
		.out_en		= PMGPIO_REG(0x684U),
		.out_or		= PMGPIO_REG(0x688U),
		.out_bic	= PMGPIO_REG(0x68CU),
		.out_xor	= PMGPIO_REG(0x690U),
		.strength0	= PMGPIO_REG(0x694U),
		.strength1	= PMGPIO_REG(0x698U),
		.pull_enable	= PMGPIO_REG(0x69CU),
		.pull_select	= PMGPIO_REG(0x6A0U),
		.in_en		= PMGPIO_REG(0x6A4U),
		.in_type	= PMGPIO_REG(0x6A8U),
		.slew_rate	= PMGPIO_REG(0x6ACU),
		.func_select0	= PMGPIO_REG(0x6B0U),
		.func_select1	= PMGPIO_REG(0x6B4U),
		.func_select2	= PMGPIO_REG(0x6B8U),
		.func_select3	= PMGPIO_REG(0x6BCU),
	},
	{ /* GPIO SD0 */
		.data		= GPIO_REG(0x200U),
		.out_en		= GPIO_REG(0x204U),
		.out_or		= GPIO_REG(0x208U),
		.out_bic	= GPIO_REG(0x20CU),
		.out_xor	= GPIO_REG(0x210U),
		.strength0	= GPIO_REG(0x214U),
		.strength1	= GPIO_REG(0x218U),
		.pull_enable	= GPIO_REG(0x21CU),
		.pull_select	= GPIO_REG(0x220U),
		.in_en		= GPIO_REG(0x224U),
		.in_type	= GPIO_REG(0x228U),
		.slew_rate	= GPIO_REG(0x22CU),
		.func_select0	= GPIO_REG(0x230U),
		.func_select1	= GPIO_REG(0x234U),
		.func_select2	= GPIO_REG(0x238U), //reserved
		.func_select3	= GPIO_REG(0x23CU), //reserved
	},
	{ /* GPIO SD1 */
		.data		= GPIO_REG(0x240U),
		.out_en		= GPIO_REG(0x244U),
		.out_or		= GPIO_REG(0x248U),
		.out_bic	= GPIO_REG(0x24CU),
		.out_xor	= GPIO_REG(0x250U),
		.strength0	= GPIO_REG(0x254U),
		.strength1	= GPIO_REG(0x258U),
		.pull_enable	= GPIO_REG(0x25CU),
		.pull_select	= GPIO_REG(0x260U),
		.in_en		= GPIO_REG(0x264U),
		.in_type	= GPIO_REG(0x268U),
		.slew_rate	= GPIO_REG(0x26CU),
		.func_select0	= GPIO_REG(0x270U),
		.func_select1	= GPIO_REG(0x274U),
		.func_select2	= GPIO_REG(0x278U), //reserved
		.func_select3	= GPIO_REG(0x27CU), //reserved
	},
	{ /* GPIO SD2 */
		.data		= GPIO_REG(0x600U),
		.out_en		= GPIO_REG(0x604U),
		.out_or		= GPIO_REG(0x608U),
		.out_bic	= GPIO_REG(0x60CU),
		.out_xor	= GPIO_REG(0x610U),
		.strength0	= GPIO_REG(0x614U),
		.strength1	= GPIO_REG(0x618U),
		.pull_enable	= GPIO_REG(0x61CU),
		.pull_select	= GPIO_REG(0x620U),
		.in_en		= GPIO_REG(0x624U),
		.in_type	= GPIO_REG(0x628U),
		.slew_rate	= GPIO_REG(0x62CU),
		.func_select0	= GPIO_REG(0x630U),
		.func_select1	= GPIO_REG(0x634U),
		.func_select2	= GPIO_REG(0x638U),	//reserved
		.func_select3	= GPIO_REG(0x63CU),	//reserved
	},
	{ /* GPIO MA */
		.data		= GPIO_REG(0x6C0U),
		.out_en		= GPIO_REG(0x6C4U),
		.out_or		= GPIO_REG(0x6C8U),
		.out_bic	= GPIO_REG(0x6CCU),
		.out_xor	= GPIO_REG(0x6D0U),
		.strength0	= GPIO_REG(0x6D4U),
		.strength1	= GPIO_REG(0x6D8U),
		.pull_enable	= GPIO_REG(0x6DCU),
		.pull_select	= GPIO_REG(0x6E0U),
		.in_en		= GPIO_REG(0x6E4U),
		.in_type	= GPIO_REG(0x6E8U),
		.slew_rate	= GPIO_REG(0x6ECU),
		.func_select0	= GPIO_REG(0x6F0U),
		.func_select1	= GPIO_REG(0x6F4U),
		.func_select2	= GPIO_REG(0x6F8U),
		.func_select3	= GPIO_REG(0x6FCU),
	},
	{ /* GPIO MB */
		.data		= GPIO_REG(0x700U),
		.out_en		= GPIO_REG(0x704U),
		.out_or		= GPIO_REG(0x708U),
		.out_bic	= GPIO_REG(0x70CU),
		.out_xor	= GPIO_REG(0x710U),
		.strength0	= GPIO_REG(0x714U),
		.strength1	= GPIO_REG(0x718U),
		.pull_enable	= GPIO_REG(0x71CU),
		.pull_select	= GPIO_REG(0x720U),
		.in_en		= GPIO_REG(0x724U),
		.in_type	= GPIO_REG(0x728U),
		.slew_rate	= GPIO_REG(0x72CU),
		.func_select0	= GPIO_REG(0x730U),
		.func_select1	= GPIO_REG(0x734U),
		.func_select2	= GPIO_REG(0x738U),
		.func_select3	= GPIO_REG(0x73CU),
	},
	{ /* GPIO MC */
		.data		= GPIO_REG(0x740U),
		.out_en		= GPIO_REG(0x744U),
		.out_or		= GPIO_REG(0x748U),
		.out_bic	= GPIO_REG(0x74CU),
		.out_xor	= GPIO_REG(0x750U),
		.strength0	= GPIO_REG(0x754U),
		.strength1	= GPIO_REG(0x758U),
		.pull_enable = GPIO_REG(0x75CU),
		.pull_select	= GPIO_REG(0x760U),
		.in_en		= GPIO_REG(0x764U),
		.in_type	= GPIO_REG(0x768U),
		.slew_rate	= GPIO_REG(0x76CU),
		.func_select0 = GPIO_REG(0x770U),
		.func_select1	= GPIO_REG(0x774U),
		.func_select2	= GPIO_REG(0x778U),
		.func_select3	= GPIO_REG(0x77CU),
	},
	{ /* GPIO MD */
		.data		= GPIO_REG(0x780U),
		.out_en		= GPIO_REG(0x784U),
		.out_or		= GPIO_REG(0x788U),
		.out_bic	= GPIO_REG(0x78CU),
		.out_xor	= GPIO_REG(0x790U),
		.strength0	= GPIO_REG(0x794U),
		.strength1	= GPIO_REG(0x798U),
		.pull_enable	= GPIO_REG(0x79CU),
		.pull_select	= GPIO_REG(0x7A0U),
		.in_en		= GPIO_REG(0x7A4U),
		.in_type	= GPIO_REG(0x7A8U),
		.slew_rate	= GPIO_REG(0x7ACU),
		.func_select0	= GPIO_REG(0x7B0U),
		.func_select1	= GPIO_REG(0x7B4U),
		.func_select2	= GPIO_REG(0x7B8U),
		.func_select3	= GPIO_REG(0x7BCU),
	},
};


#if defined(CONFIG_TCC_GPIO_SCFW)
extern const struct tcc_sc_fw_handle *tcc_sc_fw_get_handle_by_name(void);

static void gpio_request_scfw
	(u32 address, u32 bit_shift, u32 width,
	 u32 value, const struct tcc_sc_fw_handle *handle)
{
	if (handle != NULL) {
		handle->ops.reg_ops.request_reg_set
			(handle, address, bit_shift, width, value);
	}
}

static s32 gpio_config_scfw(u32 n, u32 flags)
{
	struct gpioregs *r = GPIO_REGS + ((n&GPIO_REGMASK) >> GPIO_REG_SHIFT);
	u32 num = n & GPIO_BITMASK;
	const struct tcc_sc_fw_handle *handle = tcc_sc_fw_get_handle_by_name();

	if (handle == NULL) {
		(void)pr_err(
			"[ERROR][GPIO][%s:%d]scfw handle == NULL\n"
			, __func__, __LINE__);
		return -EINVAL;
	}

	if ((n&GPIO_REGMASK) >= GPIO_PORTEXT1) {
		u32 id = ((n&GPIO_REGMASK) - GPIO_PORTEXT1)>>GPIO_REG_SHIFT;
		return ext_gpios[id]->config(ext_gpios[id], n, flags);
	}

	if ((flags & GPIO_FN_BITMASK) != 0U) {
		u32 fn = ((flags & GPIO_FN_BITMASK) >> GPIO_FN_SHIFT) - 1U;
		u32 fn_offset = num / 8U;
		u32 fn_shift = (num % 8U) * 4U;

		gpio_request_scfw
			(r->func_select0 + (fn_offset * 0x4U),
			fn_shift, 4U, fn, handle);
	}

	if ((flags & GPIO_CD_BITMASK) != 0U) {
		u32 cd = ((flags & GPIO_CD_BITMASK) >> GPIO_CD_SHIFT) - 1U;
		u32 cd_offset = num / 8U;
		u32 cd_shift = (num % 8U) * 4U;

		gpio_request_scfw(
				r->strength0 + (cd_offset * 0x4U),
				cd_shift, 2U, cd, handle);
	}

	if ((flags & GPIO_OUTPUT) != 0U) {
		if ((flags & GPIO_HIGH)
				!= 0U) {
			gpio_request_scfw(r->data, num, 1, 1, handle);
		}
		if ((flags & GPIO_LOW)
				!= 0U) {
			gpio_request_scfw(r->data, num, 1, 0, handle);
		}
		gpio_request_scfw(r->out_en, num, 1, 1, handle);
		gpio_request_scfw(r->in_en, num, 1, 0, handle);
	} else if ((flags & GPIO_INPUT) != 0U) {
		gpio_request_scfw(r->out_en, num, 1, 0, handle);
		gpio_request_scfw(r->in_en, num, 1, 1, handle);
	} else {
		/* empty */
	}

	if ((flags & GPIO_PULLUP) != 0U) {
		gpio_request_scfw(r->pull_select, num, 1, 1, handle);
		gpio_request_scfw(r->pull_enable, num, 1, 1, handle);
	} else if ((flags & GPIO_PULLDOWN) != 0U) {
		gpio_request_scfw(r->pull_select, num, 1, 0, handle);
		gpio_request_scfw(r->pull_enable, num, 1, 1, handle);
	} else if ((flags & GPIO_PULLDISABLE) != 0U) {
		gpio_request_scfw(r->pull_enable, num, 1, 0, handle);
	} else {
		/* empty */
	}

	return 0;
}

#else
static s32 gpio_config_direct(u32 n, u32 flags)
{
	struct gpioregs *r = GPIO_REGS + ((n&GPIO_REGMASK) >> GPIO_REG_SHIFT);
	u32 num = n&GPIO_BITMASK;
	u32 bit = ((u32)1U << num);
	u32 data;

	if ((n&GPIO_REGMASK) >= GPIO_PORTEXT1) {
		u32 id = ((n&GPIO_REGMASK) - GPIO_PORTEXT1)>>GPIO_REG_SHIFT;
		return ext_gpios[id]->config(ext_gpios[id], n, flags);
	}

	if ((flags & GPIO_FN_BITMASK) != 0U) {
		u32 fn = ((flags & GPIO_FN_BITMASK) >> GPIO_FN_SHIFT) - 1U;
		u32 fn_offset = num / 8U;
		u32 fn_shift = (num % 8U) * 4U;
		u32 fn_reg = readl(
				(ulong)(r->func_select0 + (fn_offset * 0x4U)));

		fn_reg &= ~((u32)0xfU << fn_shift);
		fn_reg |= (0xfU & fn) << fn_shift;

		writel(fn_reg, (ulong)(r->func_select0 + (fn_offset * 0x4U)));
	}

	if ((flags & GPIO_CD_BITMASK) != 0U) {
		u32 cd = ((flags & GPIO_CD_BITMASK) >> GPIO_CD_SHIFT) - 1U;
		u32 cd_offset = num / 8U;
		u32 cd_shift = (num % 8U) * 4U;
		u32 cd_reg = readl((ulong)(r->strength0 + (cd_offset * 0x4U)));

		cd_reg &= ~((u32)0x3U << cd_shift);
		cd_reg |= (0x3U & cd) << cd_shift;

		writel(cd_reg, (ulong)(r->strength0 + (cd_offset * 0x4U)));
	}

	if ((flags & GPIO_OUTPUT) != 0U) {
		if ((flags & GPIO_HIGH) != 0U) {
			data = readl((ulong)(r->data)) | bit;
			writel(data, (ulong)(r->data));
		}
		if ((flags & GPIO_LOW) != 0U) {
			data = readl((ulong)(r->data)) & (~bit);
			writel(data, (ulong)(r->data));
		}
		data = readl((ulong)(r->out_en)) | bit;
		writel(data, (ulong)(r->out_en));

		data = readl((ulong)(r->in_en)) & (~bit);
		writel(data, (ulong)(r->in_en));
	} else if ((flags & GPIO_INPUT) != 0U) {
		data = readl((ulong)(r->in_en)) | bit;
		writel(data, (ulong)(r->in_en));

		data = readl((ulong)(r->out_en)) & (~bit);
		writel(data, (ulong)(r->out_en));
	} else {
		/* empty */
	}

	if ((flags & GPIO_PULLUP) != 0U) {
		data = readl((ulong)(r->pull_select)) | bit;
		writel(data, (ulong)(r->pull_select));

		data = readl((ulong)(r->pull_enable)) | bit;
		writel(data, (ulong)(r->pull_enable));
	} else if ((flags & GPIO_PULLDOWN) != 0U) {
		data = readl((ulong)(r->pull_select)) & (~bit);
		writel(data, (ulong)(r->pull_select));

		data = readl((ulong)(r->pull_enable)) | bit;
		writel(data, (ulong)(r->pull_enable));
	} else if ((flags & GPIO_PULLDISABLE) != 0U) {
		data = readl((ulong)(r->pull_enable)) & (~bit);
		writel(data, (ulong)(r->pull_enable));
	} else {
		/* empty */
	}

	return 0;
}
#endif

s32 gpio_config(u32 n, u32 flags)
{
#if defined(CONFIG_TCC_GPIO_SCFW)
	return gpio_config_scfw(n, flags);
#else
	return gpio_config_direct(n, flags);
#endif
}

void gpio_set(u32 n, u32 on)
{
	struct gpioregs *r = GPIO_REGS + ((n&GPIO_REGMASK) >> GPIO_REG_SHIFT);
	u32 num = n & GPIO_BITMASK;
	u32 bit = ((u32)1U << num);

	if ((n&GPIO_REGMASK) >= GPIO_PORTEXT1) {
		u32 id = ((n&GPIO_REGMASK) - GPIO_PORTEXT1)>>GPIO_REG_SHIFT;

		ext_gpios[id]->set(ext_gpios[id], n, on);
		return;
	}

	if (on != 0U) {
		writel(bit, (ulong)(r->out_or));
		/* OR register is bit control register, so access directly */
	} else {
		writel(bit, (ulong)(r->out_bic));
		/* BIC register is bit control register, so access directly */
	}
}

s32 gpio_get(u32 n)
{
	struct gpioregs *r = GPIO_REGS + ((n&GPIO_REGMASK) >> GPIO_REG_SHIFT);
	u32 num = n&GPIO_BITMASK;
	u32 bit = ((u32)1U << num);
	u32 reg_data;

	if ((n&GPIO_REGMASK) >= GPIO_PORTEXT1) {
		u32 id = ((n&GPIO_REGMASK) - GPIO_PORTEXT1)>>GPIO_REG_SHIFT;
		return ext_gpios[id]->get(ext_gpios[id], n);
	}

	reg_data = readl((ulong)(r->data));

	return ((reg_data & bit) != 0U) ? 1 : 0;
}

s32 register_ext_gpios(u32 gpio_id, struct ext_gpio *gpios)
{
	u32 id = ((gpio_id&GPIO_REGMASK)-GPIO_PORTEXT1)>>GPIO_REG_SHIFT;

	if (id >= TCC_EXT_GPIO_MAX) {
		return -1;
		/* for kernel coding style */
	}

	if (ext_gpios[id] != NULL) {
		return -2;
		/* for kernel coding style */
	}

	ext_gpios[id] = gpios;
	return 0;
}
