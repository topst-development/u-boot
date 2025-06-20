// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */
#include <test/can_porting_chip.h>
const static struct can_gpio can_input_tcc750x[] = {
	{ 0x3, { 0x1E0200C4 } },		// GPIO C00, C01
	{ 0xC, { 0x1E0200C4} },			// GPIO C02, C03
};

static const struct can_gpio can_func_tcc750x[] = {
	{ 0x11, { 0x1E0200D0 } },		// GPIO C00, C01
	{ 0x1100, { 0x1E0200D0 } },		// GPIO C00, C01
};

#if (TCC807X_CAN_TEST_CASE == 0)
static const struct can_gpio can_input_tcc807x[] = {
	{ 0x6000, { 0x142006A4 } },		// GPIO K13, K14
	{ 0x18000, { 0x142006A4 } },		// GPIO K15, K16
	{ 0x60000, { 0x142006A4} },		// GPIO K17, K18
};

static const struct can_gpio can_func_tcc807x[] = {
	{ 0x2200000, { 0x142006B4 } },		// GPIO K13, K14
	{ 0x20000000, { 0x142006B4 }, 1, 0x2 },	// GPIO K15, K16
	{ 0x220, { 0x142006B8} },		// GPIO K17, K18
};
#elif (TCC807X_CAN_TEST_CASE == 1)
static const struct can_gpio can_input_tcc807x[] = {
	{ 0x180000, { 0x142006A4 } },		// GPIO K19, K20
	{ 0x3, { 0x142006A4 } },		// GPIO K1, K2
	{ 0xC, { 0x142006A4} },		// GPIO K3, K4
};

static const struct can_gpio can_func_tcc807x[] = {
	{ 0x22000, { 0x142006B8 } },		// GPIO K19, K20
	{ 0x22, { 0x142006B0 }},	// GPIO K1, K2
	{ 0x2200, { 0x142006B0} },		// GPIO K3, K4
};
#elif (TCC807X_CAN_TEST_CASE == 2)
static const struct can_gpio can_input_tcc807x[] = {
	{ 0x60, { 0x142006A4 } },		// GPIO K5, K6
	{ 0x180, { 0x142006A4 } },		// GPIO K7, K8
	{ 0x600, { 0x142006A4} },		// GPIO K9, K10
};

static const struct can_gpio can_func_tcc807x[] = {
	{ 0x2200000, { 0x142006B0 } },		// GPIO K5, K6
	{ 0x20000000, { 0x142006B0 }, 1, 0x2},	// GPIO K7, K8
	{ 0x330, { 0x142006B4} },		// GPIO K9, K10
};
#elif (TCC807X_CAN_TEST_CASE == 3)
static const struct can_gpio can_input_tcc807x[] = {
	{ 0x30, { 0x142007A4 } },		// GPIO MD4, MD5
	{ 0xC0, { 0x142007A4 } },		// GPIO MD6, MD7
	{ 0x300, { 0x142007A4} },		// GPIO MD8, MD9
};

static const struct can_gpio can_func_tcc807x[] = {
	{ 0x220000, { 0x142007B0 } },		// GPIO MD4, MD5
	{ 0x22000000, { 0x142007B0 }},	// GPIO MD6, MD7
	{ 0x22, { 0x142007B8} },		// GPIO MD8, MD9
};

#endif

static int32_t can_set_func(const struct can_gpio *can_func)
{
	int32_t ret = 0;
	uint32_t val, ui_addr;
	void *addr;
	union gpio_addr g_addr;

	addr = can_func->g_addr.v_addr;
	val = readl(addr);
	val |= can_func->g_val;
	writel(val, addr);

	if (can_func->excess) {
		/*
		 * add 4 to obtain address of next function register.
		 */
		ui_addr = can_func->g_addr.ui_addr;

		if (UINT_MAX - ui_addr < 4) {
			printk("Failed to configure function\n");
			ret = -EINVAL;
		}

		if (ret == 0) {
			ui_addr += 4;
			g_addr.ui_addr = ui_addr;
			addr = g_addr.v_addr;
			val = readl(addr);
			val |= can_func->ex_val;
			writel(val, addr);
		}
	}

	return ret;
}

const struct can_gpio *get_can_input(void)
{
	const struct can_gpio *ret;

	if (IS_ENABLED(CONFIG_TCC750X))
		ret = can_input_tcc750x;
	else if (IS_ENABLED(CONFIG_TCC807X))
		ret = can_input_tcc807x;

	return ret;
}

const struct can_gpio *get_can_func(void)
{
	const struct can_gpio *ret;

	if (IS_ENABLED(CONFIG_TCC750X))
		ret = can_func_tcc750x;
	else if (IS_ENABLED(CONFIG_TCC807X))
		ret = can_func_tcc807x;

	return ret;
}

int32_t can_gpio_setting(void)
{
	int32_t ret = 0;
	uint32_t i, val;
	void *addr;
	const struct can_gpio *can_input = get_can_input();
	const struct can_gpio *can_func = get_can_func();

	for (i = 0; i < CAN_CONTROLLER_NUMBER; i++) {
		addr = can_input[i].g_addr.v_addr;
		val = readl(addr);
		val |= can_input[i].g_val;
		writel(val, addr);

		ret = can_set_func(&can_func[i]);

		if (ret != 0) {
			printk("Failed to configure GPIO\n");
			break;
		}

	}

	return ret;
}
