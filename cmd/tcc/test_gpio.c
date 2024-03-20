// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <dm/pinctrl.h>
#include <dm/uclass-internal.h>
#include <linux/io.h>
#include <linux/err.h>
#include <dm/read.h>
#include <irq.h>
#include <dt-bindings/interrupt-controller/arm-gic.h>
#include <linux/soc/telechips/tcc_mbox.h>
#include <linux/soc/telechips/tcc_sc_protocol.h>
#include <asm/gpio.h>

#define TCC_GPIO_DATA   (0x00U)
#define TCC_GPIO_OEN    (0x04U)
#define TCC_GPIO_OR     (0x08U)
#define TCC_GPIO_BIC    (0x0CU)
#define TCC_GPIO_XOR    (0x10U)
#define TCC_GPIO_CD0    (0x14U)
#define TCC_GPIO_CD1    (0x18U)
#define TCC_GPIO_PE     (0x1CU)
#define TCC_GPIO_PS     (0x20U)
#define TCC_GPIO_IEN    (0x24U)
#define TCC_GPIO_IS     (0x28U)
#define TCC_GPIO_SR     (0x2CU)
#define TCC_GPIO_FN(x)  (0x30U + ((x) * 0x4U))

#define TCC_GPIO_BIT(x) ((u32)1U << (x))

//gpio driver
struct tcc_gpio_bank {
	const char *name;
	u32 gpio_base;
	u32 pmgpio_base;
	u32 bank_base;
	u32 npins;
	u32 core_num;
	const struct tcc_sc_fw_handle *handle;
};

//pinctrl driver
struct tcc_bank {
	const char *name;
	u32 bank_base;
	u32 npins;
};

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

#define TCC_DEBUG_MSG(format, ...) \
do {\
	pr_err("[%s:%d]", __func__, __LINE__);\
	pr_err(format, ##__VA_ARGS__);\
} while (0)

struct datasheet_register {
	u32 offset;
	u32 value;
};

struct extintr_t {
	char *bank_name;
	s32 pin_num;
};

static struct extintr_t extintr[] = {
	{"x", 0},	//0: no source

	//1 ~ 32
	{"gpa", 0}, {"gpa", 1}, {"gpa", 2}, {"gpa", 3}, {"gpa", 4},
	{"gpa", 5}, {"gpa", 6}, {"gpa", 7}, {"gpa", 8}, {"gpa", 9},
	{"gpa", 10}, {"gpa", 11}, {"gpa", 12}, {"gpa", 13}, {"gpa", 14},
	{"gpa", 15}, {"gpa", 16}, {"gpa", 17}, {"gpa", 18}, {"gpa", 19},
	{"gpa", 20}, {"gpa", 21}, {"gpa", 22}, {"gpa", 23}, {"gpa", 24},
	{"gpa", 25}, {"gpa", 26}, {"gpa", 27}, {"gpa", 28}, {"gpa", 29},
	{"gpa", 30}, {"gpa", 31},

	//33 ~ 61
	{"gpb", 0}, {"gpb", 1}, {"gpb", 2}, {"gpb", 3}, {"gpb", 4},
	{"gpb", 5}, {"gpb", 6}, {"gpb", 7}, {"gpb", 8}, {"gpb", 9},
	{"gpb", 10}, {"gpb", 11}, {"gpb", 12}, {"gpb", 13}, {"gpb", 14},
	{"gpb", 15}, {"gpb", 16}, {"gpb", 17}, {"gpb", 18}, {"gpb", 19},
	{"gpb", 20}, {"gpb", 21}, {"gpb", 22}, {"gpb", 23}, {"gpb", 24},
	{"gpb", 25}, {"gpb", 26}, {"gpb", 27}, {"gpb", 28},

	//62 ~ 91
	{"gpc", 0}, {"gpc", 1}, {"gpc", 2}, {"gpc", 3}, {"gpc", 4},
	{"gpc", 5}, {"gpc", 6}, {"gpc", 7}, {"gpc", 8}, {"gpc", 9},
	{"gpc", 10}, {"gpc", 11}, {"gpc", 12}, {"gpc", 13}, {"gpc", 14},
	{"gpc", 15}, {"gpc", 16}, {"gpc", 17}, {"gpc", 18}, {"gpc", 19},
	{"gpc", 20}, {"gpc", 21}, {"gpc", 22}, {"gpc", 23}, {"gpc", 24},
	{"gpc", 25}, {"gpc", 26}, {"gpc", 27}, {"gpc", 28}, {"gpc", 29},

	//92 ~ 111
	{"gpe", 0}, {"gpe", 1}, {"gpe", 2}, {"gpe", 3}, {"gpe", 4},
	{"gpe", 5}, {"gpe", 6}, {"gpe", 7}, {"gpe", 8}, {"gpe", 9},
	{"gpe", 10}, {"gpe", 11}, {"gpe", 12}, {"gpe", 13}, {"gpe", 14},
	{"gpe", 15}, {"gpe", 16}, {"gpe", 17}, {"gpe", 18}, {"gpe", 19},

	//112 ~ 122
	{"gpg", 0}, {"gpg", 1}, {"gpg", 2}, {"gpg", 3}, {"gpg", 4},
	{"gpg", 5}, {"gpg", 6}, {"gpg", 7}, {"gpg", 8}, {"gpg", 9},
	{"gpg", 10},

	//123 ~ 134
	{"gph", 0}, {"gph", 1}, {"gph", 2}, {"gph", 3}, {"gph", 4},
	{"gph", 5}, {"gph", 6}, {"gph", 7}, {"gph", 8}, {"gph", 9},
	{"gph", 10}, {"gph", 11},

	//135 ~ 164
	{"gpma", 0}, {"gpma", 1}, {"gpma", 2}, {"gpma", 3}, {"gpma", 4},
	{"gpma", 5}, {"gpma", 6}, {"gpma", 7}, {"gpma", 8}, {"gpma", 9},
	{"gpma", 10}, {"gpma", 11}, {"gpma", 12}, {"gpma", 13}, {"gpma", 14},
	{"gpma", 15}, {"gpma", 16}, {"gpma", 17}, {"gpma", 18}, {"gpma", 19},
	{"gpma", 20}, {"gpma", 21}, {"gpma", 22}, {"gpma", 23},	{"gpma", 24},
	{"gpma", 25}, {"gpma", 26}, {"gpma", 27}, {"gpma", 28}, {"gpma", 29},

	{"x", 0}, // RTC source

	//166 ~ 180
	{"gpsd0", 0}, {"gpsd0", 1}, {"gpsd0", 2}, {"gpsd0", 3}, {"gpsd0", 4},
	{"gpsd0", 5}, {"gpsd0", 6}, {"gpsd0", 7}, {"gpsd0", 8}, {"gpsd0", 9},
	{"gpsd0", 10}, {"gpsd0", 11}, {"gpsd0", 12}, {"gpsd0", 13},
	{"gpsd0", 14},

	//181 ~ 191
	{"gpsd1", 0}, {"gpsd1", 1}, {"gpsd1", 2}, {"gpsd1", 3}, {"gpsd1", 4},
	{"gpsd1", 5}, {"gpsd1", 6}, {"gpsd1", 7}, {"gpsd1", 8}, {"gpsd1", 9},
	{"gpsd1", 10},

	//192 ~ 201
	{"gpsd2", 0}, {"gpsd2", 1}, {"gpsd2", 2}, {"gpsd2", 3}, {"gpsd2", 4},
	{"gpsd2", 5}, {"gpsd2", 6}, {"gpsd2", 7}, {"gpsd2", 8}, {"gpsd2", 9},

	//202 ~ 227
	{"gpmb", 0}, {"gpmb", 1}, {"gpmb", 2}, {"gpmb", 3}, {"gpmb", 4},
	{"gpmb", 5}, {"gpmb", 6}, {"gpmb", 7}, {"gpmb", 8}, {"gpmb", 9},
	{"gpmb", 10}, {"gpmb", 11}, {"gpmb", 12}, {"gpmb", 13}, {"gpmb", 14},
	{"gpmb", 15}, {"gpmb", 16}, {"gpmb", 17}, {"gpmb", 18}, {"gpmb", 19},
	{"gpmb", 20}, {"gpmb", 21}, {"gpmb", 22}, {"gpmb", 23}, {"gpmb", 24},
	{"gpmb", 25},

	//228 ~ 251
	{"gpmc", 0}, {"gpmc", 1}, {"gpmc", 2}, {"gpmc", 3}, {"gpmc", 4},
	{"gpmc", 5}, {"gpmc", 6}, {"gpmc", 7}, {"gpmc", 8}, {"gpmc", 9},
	{"gpmc", 10}, {"gpmc", 11}, {"gpmc", 12}, {"gpmc", 13}, {"gpmc", 14},
	{"gpmc", 15}, {"gpmc", 16}, {"gpmc", 17}, {"gpmc", 18}, {"gpmc", 19},
	{"gpmc", 20}, {"gpmc", 21}, {"gpmc", 22}, {"gpmc", 23},

	//252 ~ 253
	{"gpmd", 8}, {"gpmd", 9},

	//254 ~ 255
	{"gpmd", 16}, {"gpmd", 17},
};

#if TCC805_ES_CHIP
struct datasheet_register gpio_datasheet_register[] = {
{0x0, 0x0},
{0x4, 0x0},
{0x8, 0x0},
{0xC, 0x0},
{0x010, 0x0},
{0x014, 0x0},
{0x018, 0x0},
{0x01C, 0xFFFFFFFF},
{0x020, 0x0},
{0x024, 0xFFFFFFFF},
{0x028, 0x0},
{0x02C, 0x0},
{0x030, 0x33333333},
{0x034, 0x33333333},
{0x038, 0x33333333},
{0x03C, 0x33333},
{0x040, 0x0},
{0x044, 0x0},
{0x048, 0x0},
{0x04C, 0x0},
{0x050, 0x0},
{0x054, 0x0},
{0x058, 0x0},
{0x05C, 0x1FFFFFFF},
{0x060, 0x0},
{0x064, 0x1FFFFFFF},
{0x068, 0x0},
{0x06C, 0x0},
{0x070, 0x0},
{0x074, 0x0},
{0x078, 0x0},
{0x07C, 0x0},
{0x080, 0x0},
{0x084, 0x0},
{0x088, 0x0},
{0x08C, 0x0},
{0x090, 0x0},
{0x094, 0x0},
{0x098, 0x0},
{0x09C, 0x3FFFFFFF},
{0x0A0, 0x0},
{0x0A4, 0x3FFFFFFF},
{0x0A8, 0x0},
{0x0AC, 0x0},
{0x0B0, 0x0},
{0x0B4, 0x0},
{0x0B8, 0x0},
{0x0BC, 0x0},
{0x100, 0x0},
{0x104, 0x0},
{0x108, 0x0},
{0x10C, 0x0},
{0x110, 0x0},
{0x114, 0x0},
{0x118, 0x0},
{0x11C, 0x000FFFFF},
{0x120, 0x0},
{0x124, 0x000FFFFF},
{0x128, 0x0},
{0x12C, 0x0},
{0x130, 0x0},
{0x134, 0x0},
{0x138, 0x0},
{0x13C, 0x0},
{0x180, 0x0},
{0x184, 0x0},
{0x188, 0x0},
{0x18C, 0x0},
{0x190, 0x0},
{0x194, 0x0},
{0x198, 0x0},
{0x19C, 0x7FF},
{0x1A0, 0x0},
{0x1A4, 0x7FF},
{0x1A8, 0x0},
{0x1AC, 0x0},
{0x1B0, 0x0},
{0x1B4, 0x0},
{0x1B8, 0x0},
{0x1BC, 0x0},
{0x200, 0x0},
{0x204, 0x0},
{0x208, 0x0},
{0x20C, 0x0},
{0x210, 0x0},
{0x214, 0x0},
{0x21C, 0x7FFF},
{0x220, 0x0},
{0x224, 0x7FFF},
{0x228, 0x0},
{0x22C, 0x0},
{0x230, 0x0},
{0x234, 0x0},
{0x240, 0x0},
{0x244, 0x0},
{0x248, 0x0},
{0x24C, 0x0},
{0x250, 0x0},
{0x254, 0x0},
{0x25C, 0x7FF},
{0x260, 0x0},
{0x264, 0x7FF},
{0x268, 0x0},
{0x26C, 0x0},
{0x270, 0x0},
{0x274, 0x0},
{0x280, 0x0},
{0x284, 0x0},
{0x288, 0x0},
{0x28C, 0x0},
{0x290, 0x0},
{0x294, 0x0},
{0x298, 0x0},
{0x29C, 0x0},
{0x2A0, 0x0},
{0x2A4, 0x0},
{0x2B0, 0x0},
{0x300, 0x0},
{0x304, 0x0},
{0x308, 0x0},
{0x310, 0x0},
{0x318, 0x0},
{0x320, 0x0},
{0x324, 0x0},
{0x328, 0x0},
{0x32C, 0x0},
{0x334, 0x0},
{0x380, 0xFFFFFFFF},
{0x384, 0xFFFFFFFF},
{0x388, 0xFFFFFFFF},
{0x390, 0xFFFFFFFF},
{0x398, 0xFFFFFFFF},
{0x3A0, 0xFFFFFFFF},
{0x3A4, 0xFFFFFFFF},
{0x3A8, 0xFFFFFFFF},
{0x3AC, 0xFFFFFFFF},
{0x3B4, 0xFFFFFFFF},
{0x400, 0x0},
{0x404, 0x0},
{0x408, 0x0},
{0x40C, 0x0},
{0x410, 0x0},
{0x414, 0x0},
{0x420, 0x0},
{0x424, 0x0},
{0x428, 0x0},
{0x42C, 0x0},
{0x430, 0x0},
{0x434, 0x0},
{0x440, 0x0},
{0x444, 0x0},
{0x448, 0x0},
{0x44C, 0x0},
{0x450, 0x0},
{0x454, 0x0},
{0x480, 0x0},
{0x484, 0x0},
{0x488, 0x0},
{0x48C, 0x0},
{0x490, 0x0},
{0x494, 0x0},
{0x4C0, 0x0},
{0x4C4, 0x0},
{0x4C8, 0x0},
{0x4CC, 0x0},
{0x4D0, 0x0},
{0x4D4, 0x0},
{0x500, 0x0},
{0x504, 0x0},
{0x508, 0x0},
{0x50C, 0x0},
{0x510, 0x0},
{0x514, 0x0},
{0x520, 0x0},
{0x524, 0x0},
{0x528, 0x0},
{0x52C, 0x0},
{0x530, 0x0},
{0x534, 0x0},
{0x540, 0x0},
{0x544, 0x0},
{0x548, 0x0},
{0x54C, 0x0},
{0x550, 0x0},
{0x554, 0x0},
{0x560, 0x0},
{0x564, 0x0},
{0x568, 0x0},
{0x56C, 0x0},
{0x570, 0x0},
{0x574, 0x0},
{0x5A0, 0x0},
{0x5A4, 0x0},
{0x5A8, 0x0},
{0x5AC, 0x0},
{0x5B0, 0x0},
{0x5B4, 0x0},
{0x600, 0x0},
{0x604, 0x0},
{0x608, 0x0},
{0x60C, 0x0},
{0x610, 0x0},
{0x614, 0x0},
{0x61C, 0x3FF},
{0x620, 0x0},
{0x624, 0x3FF},
{0x628, 0x0},
{0x62C, 0x0},
{0x630, 0x0},
{0x634, 0x0},
{0x640, 0x0},
{0x644, 0x0},
{0x648, 0x0},
{0x64C, 0x0},
{0x650, 0x0},
{0x654, 0x0},
{0x658, 0x0},
{0x65C, 0xFFF},
{0x660, 0x0},
{0x664, 0xFFF},
{0x668, 0x0},
{0x66C, 0x0},
{0x670, 0x0},
{0x674, 0x0},
{0x678, 0x0},
{0x67C, 0x0},
{0x6C0, 0x0},
{0x6C4, 0x0},
{0x6C8, 0x0},
{0x6CC, 0x0},
{0x6D0, 0x0},
{0x6D4, 0x0},
{0x6D8, 0x0},
{0x6DC, 0x3FFFFFFF},
{0x6E0, 0x0},
{0x6E4, 0x3FFFFFFF},
{0x6E8, 0x0},
{0x6EC, 0x0},
{0x6F0, 0x33333333},
{0x6F4, 0x33333333},
{0x6F8, 0x33333333},
{0x6FC, 0x33333},
{0x700, 0x0},
{0x704, 0x0},
{0x708, 0x0},
{0x70C, 0x0},
{0x710, 0x0},
{0x714, 0x0},
{0x718, 0x0},
{0x71C, 0xFFFFFFFF},
{0x720, 0x0},
{0x724, 0xFFFFFFFF},
{0x728, 0x0},
{0x72C, 0x0},
{0x730, 0x0},
{0x734, 0x0},
{0x738, 0x0},
{0x73C, 0x0},
{0x740, 0x0},
{0x744, 0x0},
{0x748, 0x0},
{0x74C, 0x0},
{0x750, 0x0},
{0x754, 0x0},
{0x758, 0x0},
{0x75C, 0x3FFFFFFF},
{0x760, 0x0},
{0x764, 0x3FFFFFFF},
{0x768, 0x0},
{0x76C, 0x0},
{0x770, 0x0},
{0x774, 0x0},
{0x778, 0x0},
{0x77C, 0x0},
{0x780, 0x0},
{0x784, 0x0},
{0x788, 0x0},
{0x78C, 0x0},
{0x790, 0x0},
{0x794, 0x0},
{0x798, 0x0},
{0x79C, 0x3FFFF},
{0x7A0, 0x0},
{0x7A4, 0x3FFFF},
{0x7A8, 0x0},
{0x7AC, 0x0},
{0x7B0, 0x0},
{0x7B4, 0x0},
{0x7B8, 0x0},
{0x7BC, 0x0},
};
#else
struct datasheet_register gpio_datasheet_register[] = {
{0x0, 0x0},	//A
{0x4, 0x0},
{0x8, 0x0},
{0xC, 0x0},
{0x010, 0x0},
{0x014, 0x0},
{0x018, 0x0},
{0x01C, 0xFFFFFFFF},
{0x020, 0x0},
{0x024, 0x1FFFFFFF},	//
{0x028, 0x0},
{0x02C, 0x0},
{0x030, 0x33333333},
{0x034, 0x33333333},
{0x038, 0x33333333},
{0x03C, 0x33333},
{0x040, 0x0},	//B
{0x044, 0x0},
{0x048, 0x0},
{0x04C, 0x0},
{0x050, 0x0},
{0x054, 0x0},
{0x058, 0x0},
{0x05C, 0x1FFFFFFF},
{0x060, 0x0},
{0x064, 0x0},	//
{0x068, 0x0},
{0x06C, 0x0},
{0x070, 0x0},
{0x074, 0x0},
{0x078, 0x0},
{0x07C, 0x0},
{0x080, 0x0},
{0x084, 0x0},
{0x088, 0x0},
{0x08C, 0x0},
{0x090, 0x0},
{0x094, 0x0},
{0x098, 0x0},
{0x09C, 0x3FFFFFFF},
{0x0A0, 0x0},
{0x0A4, 0x0},	//
{0x0A8, 0x0},
{0x0AC, 0x0},
{0x0B0, 0x0},
{0x0B4, 0x0},
{0x0B8, 0x0},
{0x0BC, 0x0},
{0x100, 0x0},
{0x104, 0x0},
{0x108, 0x0},
{0x10C, 0x0},
{0x110, 0x0},
{0x114, 0x0},
{0x118, 0x0},
{0x11C, 0x000FFFFF},
{0x120, 0x0},
{0x124, 0x0},
{0x128, 0x0},
{0x12C, 0x0},
{0x130, 0x0},
{0x134, 0x0},
{0x138, 0x0},
{0x13C, 0x0},
{0x180, 0x0},
{0x184, 0x0},
{0x188, 0x0},
{0x18C, 0x0},
{0x190, 0x0},
{0x194, 0x0},
{0x198, 0x0},
{0x19C, 0x7FF},
{0x1A0, 0x0},
{0x1A4, 0x0},	//
{0x1A8, 0x0},
{0x1AC, 0x0},
{0x1B0, 0x0},
{0x1B4, 0x0},
{0x1B8, 0x0},
{0x1BC, 0x0},
{0x200, 0x0},
{0x204, 0x0},
{0x208, 0x0},
{0x20C, 0x0},
{0x210, 0x0},
{0x214, 0x0},
{0x21C, 0x7C00},	////
{0x220, 0x0},
{0x224, 0x0},	//
{0x228, 0x0},
{0x22C, 0x0},
{0x230, 0x0},
{0x234, 0x0},
{0x240, 0x0},
{0x244, 0x0},
{0x248, 0x0},
{0x24C, 0x0},
{0x250, 0x0},
{0x254, 0x0},
{0x25C, 0x400},		////
{0x260, 0x0},
{0x264, 0x0},	//
{0x268, 0x0},
{0x26C, 0x0},
{0x270, 0x0},
{0x274, 0x0},
{0x280, 0x0},
{0x284, 0x0},
{0x288, 0x0},
{0x28C, 0x0},
{0x290, 0x0},
{0x294, 0x0},
{0x298, 0x0},
{0x29C, 0x0},
{0x2A0, 0x0},
{0x2A4, 0x0},
{0x2B0, 0x0},
{0x300, 0x0},
{0x304, 0x0},
{0x308, 0x0},
{0x310, 0x0},
{0x318, 0x0},
{0x320, 0x0},
{0x324, 0x0},
{0x328, 0x0},
{0x32C, 0x0},
{0x334, 0x0},
{0x380, 0xFFFFFFFF},
{0x384, 0xFFFFFFFF},
{0x388, 0xFFFFFFFF},
{0x390, 0xFFFFFFFF},
{0x398, 0xFFFFFFFF},
{0x3A0, 0xFFFFFFFF},
{0x3A4, 0xFFFFFFFF},
{0x3A8, 0xFFFFFFFF},
{0x3AC, 0xFFFFFFFF},
{0x3B4, 0xFFFFFFFF},
{0x400, 0x0},
{0x404, 0x0},
{0x408, 0x0},
{0x40C, 0x0},
{0x410, 0x0},
{0x414, 0x0},
{0x420, 0x0},
{0x424, 0x0},
{0x428, 0x0},
{0x42C, 0x0},
{0x430, 0x0},
{0x434, 0x0},
{0x440, 0x0},
{0x444, 0x0},
{0x448, 0x0},
{0x44C, 0x0},
{0x450, 0x0},
{0x454, 0x0},
{0x480, 0x0},
{0x484, 0x0},
{0x488, 0x0},
{0x48C, 0x0},
{0x490, 0x0},
{0x494, 0x0},
{0x4C0, 0x0},
{0x4C4, 0x0},
{0x4C8, 0x0},
{0x4CC, 0x0},
{0x4D0, 0x0},
{0x4D4, 0x0},
{0x500, 0x0},
{0x504, 0x0},
{0x508, 0x0},
{0x50C, 0x0},
{0x510, 0x0},
{0x514, 0x0},
{0x520, 0x0},
{0x524, 0x0},
{0x528, 0x0},
{0x52C, 0x0},
{0x530, 0x0},
{0x534, 0x0},
{0x540, 0x0},
{0x544, 0x0},
{0x548, 0x0},
{0x54C, 0x0},
{0x550, 0x0},
{0x554, 0x0},
{0x560, 0x0},
{0x564, 0x0},
{0x568, 0x0},
{0x56C, 0x0},
{0x570, 0x0},
{0x574, 0x0},
{0x5A0, 0x0},
{0x5A4, 0x0},
{0x5A8, 0x0},
{0x5AC, 0x0},
{0x5B0, 0x0},
{0x5B4, 0x0},
{0x600, 0x0},
{0x604, 0x0},
{0x608, 0x0},
{0x60C, 0x0},
{0x610, 0x0},
{0x614, 0x0},
{0x61C, 0x0},		////
{0x620, 0x0},
{0x624, 0x0},	//
{0x628, 0x0},
{0x62C, 0x0},
{0x630, 0x0},
{0x634, 0x0},
{0x640, 0x0},
{0x644, 0x0},
{0x648, 0x0},
{0x64C, 0x0},
{0x650, 0x0},
{0x654, 0x0},
{0x658, 0x0},
{0x65C, 0xFFF},
{0x660, 0x0},
{0x664, 0x0},	//
{0x668, 0x0},
{0x66C, 0x0},
{0x670, 0x0},
{0x674, 0x0},
{0x678, 0x0},
{0x67C, 0x0},
{0x6C0, 0x0},
{0x6C4, 0x0},
{0x6C8, 0x0},
{0x6CC, 0x0},
{0x6D0, 0x0},
{0x6D4, 0x0},
{0x6D8, 0x0},
{0x6DC, 0x3FFFFFFF},
{0x6E0, 0x0},
{0x6E4, 0x0},	//
{0x6E8, 0x0},
{0x6EC, 0x0},
{0x6F0, 0x33333333},
{0x6F4, 0x33333333},
{0x6F8, 0x33333333},
{0x6FC, 0x33333},
{0x700, 0x0},
{0x704, 0x0},
{0x708, 0x0},
{0x70C, 0x0},
{0x710, 0x0},
{0x714, 0x0},
{0x718, 0x0},
{0x71C, 0xFFFFFFFF},
{0x720, 0x0},
{0x724, 0x0},	//
{0x728, 0x0},
{0x72C, 0x0},
{0x730, 0x0},
{0x734, 0x0},
{0x738, 0x0},
{0x73C, 0x0},
{0x740, 0x0},
{0x744, 0x0},
{0x748, 0x0},
{0x74C, 0x0},
{0x750, 0x0},
{0x754, 0x0},
{0x758, 0x0},
{0x75C, 0x3FFFFFFF},
{0x760, 0x0},
{0x764, 0x0},	//
{0x768, 0x0},
{0x76C, 0x0},
{0x770, 0x0},
{0x774, 0x0},
{0x778, 0x0},
{0x77C, 0x0},
{0x780, 0x0},
{0x784, 0x0},
{0x788, 0x0},
{0x78C, 0x0},
{0x790, 0x0},
{0x794, 0x0},
{0x798, 0x0},
{0x79C, 0x3FFFF},
{0x7A0, 0x0},
{0x7A4, 0x0},	//
{0x7A8, 0x0},
{0x7AC, 0x0},
{0x7B0, 0x0},
{0x7B4, 0x0},
{0x7B8, 0x0},
{0x7BC, 0x0},
};
#endif



//Offset based on GPIO bank base address
#define TCC_TEST_GPIO_DATA	0x00
#define TCC_TEST_GPIO_OEN	0x04
#define TCC_TEST_GPIO_OR	0x08
#define TCC_TEST_GPIO_BIC	0x0C
#define TCC_TEST_GPIO_XOR	0x10
#define TCC_TEST_GPIO_CD0	0x14
#define TCC_TEST_GPIO_CD1	0x18
#define TCC_TEST_GPIO_PE	0x1C
#define TCC_TEST_GPIO_PS	0x20
#define TCC_TEST_GPIO_IEN	0x24
#define TCC_TEST_GPIO_IS	0x28
#define TCC_TEST_GPIO_SR	0x2C
#define TCC_TEST_GPIO_FN0	0x30

//Offset based on GPIO base address
#define TCC_TEST_EINTSEL0	0x280
#define TCC_TEST_EINTSEL1	0x284
#define TCC_TEST_EINTSEL2	0x288
#define TCC_TEST_EINTSEL3	0x28C
#define TCC_TEST_ECLKSEL	0x2B0

#define TCC_TEST_FNLOCKEN	0x300
#define TCC_TEST_ENLOCKEN	0x380
#define TCC_TEST_VI2O_OEN	0x400
#define TCC_TEST_VI2I_DAT	0x404
#define TCC_TEST_VI2O_EN	0x408
#define TCC_TEST_VI2I_EN	0x40C
#define TCC_TEST_MON_DO		0x410
#define TCC_TEST_MON_OEN	0x414

static s32 test_gpio_set_slew_rate(u32 bank_base, u32 pin_num, u32 sr);
static s32 test_gpio_get_slew_rate(u32 bank_base, u32 pin_num);
static s32 test_gpio_set_input_type(u32 bank_base, u32 pin_num, u32 is);
static s32 test_gpio_get_input_type(u32 bank_base, u32 pin_num);
static s32 test_gpio_set_input_enable(u32 bank_base, u32 pin_num, u32 ien);
static s32 test_gpio_get_input_enable(u32 bank_base, u32 pin_num);
static s32 test_gpio_set_data(u32 bank_base, u32 pin_num, u32 value);
static s32 test_gpio_get_data(u32 bank_base, u32 pin_num);
static s32 test_gpio_set_output_enable(u32 bank_base, u32 pin_num, u32 oen);
static s32 test_gpio_get_output_enable(u32 bank_base, u32 pin_num);
static s32 test_gpio_set_pull_select(u32 bank_base, u32 pin_num, u32 ps);
static s32 test_gpio_get_pull_select(u32 bank_base, u32 pin_num);
static s32 test_gpio_set_pull_enable(u32 bank_base, u32 pin_num, u32 pe);
static s32 test_gpio_get_pull_enable(u32 bank_base, u32 pin_num);
static s32 test_gpio_set_drive_strength(
		u32 bank_base, u32 pin_num, u32 drive_strength);
static s32 test_gpio_get_drive_strength(u32 bank_base, u32 pin_num);
static s32 test_gpio_set_function(u32 bank_base, u32 pin_num, u32 func);
static s32 test_gpio_get_function(u32 bank_base, u32 pin_num);
static s32 test_gpio_set_eclk(u32 bank_base, u32 eclk_num, u32 eint_source);
static s32 test_gpio_get_eclk(u32 bank_base, u32 eclk_num);
static s32 test_gpio_set_eint(u32 bank_base, u32 eint_num, u32 eint_source);
static s32 test_gpio_get_eint(u32 bank_base, u32 eint_num);
static s32 test_slew_rate(u32 addr, s32 pin_num);
static s32 test_input_type(u32 addr, s32 pin_num);
static s32 test_pull_select(u32 addr, s32 pin_num);
static s32 test_pull_enable(u32 addr, s32 pin_num);
static s32 test_drive_strength(u32 addr, s32 pin_num);
static s32 test_function(u32 addr, s32 pin_num);
static s32 test_output(u32 addr, s32 pin_num);
static s32 test_gpio_set_xor_reg(u32 bank_base, u32 pin_num);
static s32 test_gpio_set_bic_reg(u32 bank_base, u32 pin_num);
static s32 test_gpio_set_or_reg(u32 bank_base, u32 pin_num);
static s32 test_xor_register(u32 addr, s32 pin_num);
static s32 test_bic_register(u32 addr, s32 pin_num);
static s32 test_or_register(u32 addr, s32 pin_num);

static s32 test_output_enable(u32 addr, s32 pin_num);
static s32 test_input_enable(u32 addr, s32 pin_num);
static s32 test_eint(u32 addr);
static s32 test_eclk(u32 addr);
static s32 is_used_gpio(const char *bank_name, u32 pin_num);
static void print_used_pins(void);
static s32 get_normal_bank_index(const char *bank_name);
static s32 get_lock_bank_index(const char *bank_name);
static s32 get_virtual_bank_index(const char *bank_name);

static void test_rw_all(struct udevice *dev);
static void test_rw(struct udevice *dev, const char *bank_name, s32 pin_num);
static void test_eint_eclk(struct udevice *dev);
static void test_output_enable_lock(struct udevice *dev, const char *bank_name);
static void test_function_lock(struct udevice *dev, const char *bank_name);
static void test_input_by_tpg(struct udevice *dev, const char *bank_name);
static void test_output_by_tpg(struct udevice *dev, const char *bank_name);


static s32 test_get_virtual_gpio_mon(
		u32 gpio_base, const char *bank_name,
		u32 pin_num, u32 offset);
static s32 test_set_virtual_gpio(
		u32 gpio_base, const char *bank_name,
		u32 pin_num, u32 value, u32 offset);

static s32 is_gpio_ma_4(const char *bank_name, u32 pin_num);

static s32 test_manual_output(u32 addr, s32 pin_num, s32 value);
static s32 test_manual_input(u32 addr, s32 pin_num);
static u32 reg_readl
	(u32 addr, u32 pin_num, u32 width);

//only use - func lock, out enable lock, tpg
struct index_of_bank {
	const char *bank_name;
	s32 index;
	s32 lock_index;
	s32 virtual_index;
};

struct index_of_bank bank_list[] = {
	{"gpa", 0, 0, 0},
	{"gpb", 1, 1, 1},
	{"gpc", 2, 2, 2},
	{"gpe", 4, 4, 4},
	{"gpg", 6, 6, 6},
	{"gpsd0", 8, 8, 8},
	{"gpsd1", 9, 9, 9},
	{"gpsd2", 24, 10, 10},
	{"gph", 25, 11, 11},
	{"gpma", 27, 13, 13},
	{"gpmb", 28, 14, 14},
	{"gpmc", 29, 15, 15},
	{"gpmd", 30, 16, 30},
};

struct used_pin {
	const char *bank_name;
	u32 pin_num;
	const char *reason;
};

static struct used_pin used_pins[] = {
	//{"gpma", 4, "panic"},
	//{"gpc", 10, "a72 uart"},
	//{"gpc", 11, "a72 uart"},
};

static u32 reg_readl
	(u32 addr, u32 pin_num, u32 width)
{
	u32 mask;
	u32 reg_data;
	u32 bit_shift;

	if (width == 0U) {
		(void)pr_err(
				"[ERROR][PINCTRL] %s : width == 0\n"
				, __func__);
		return 0;
	}
	mask = (u32)1U << width;
	if (mask > 0U) {
		mask -= 1U;
		/* comment for kernel coding style */
	}

	bit_shift = (pin_num % (32U / width));
	if (((UINT_MAX) / width) >= bit_shift) {
		bit_shift *= width;
		/* comment for kernel coding style */
	}

	reg_data = readl(addr);

	return (reg_data >> bit_shift) & mask;
}
static void print_used_pins(void)
{
	s32 i;
	s32 sizeOfUsedPins = ARRAY_SIZE(used_pins);

	if (sizeOfUsedPins > 0) {
		TCC_DEBUG_MSG(
			"\033[33m The pins below have not been tested for the following reasons \033[0m\n"
			);
		for (i = 0; i < sizeOfUsedPins; i++) {
			TCC_DEBUG_MSG(
				"\033[33m [%s-%ld] : %s \033[0m\n"
				, used_pins[i].bank_name,
				used_pins[i].pin_num, used_pins[i].reason);
		}
	}
}

//if gpma 4 set 1, reboot
static s32 is_gpio_ma_4(const char *bank_name, u32 pin_num)
{
	if ((strncmp(bank_name, "gpma", 4) == 0)
		&& (pin_num == 4)) {
		TCC_DEBUG_MSG(
			"\033[33m [gpma-4] : if control gpio, system will be panic\033[0m\n"
			);
		return 0;
	}

	return -1;
}

static s32 is_used_gpio(const char *bank_name, u32 pin_num)
{
	s32 i;
	s32 sizeOfUsedPins = ARRAY_SIZE(used_pins);
	char temp_bank_name[6];

	for (i = 0; i < strlen(bank_name); i++) {
		temp_bank_name[i] = bank_name[i];
		if (*(bank_name + i) == '-') {
			temp_bank_name[i] = '\0';
			break;
		}
	}

	for (i = 0; i < sizeOfUsedPins; i++) {
		if ((strncmp(temp_bank_name, used_pins[i].bank_name,
				strlen(temp_bank_name)) == 0)
				&& (pin_num == used_pins[i].pin_num)) {
			TCC_DEBUG_MSG(
				"[%s-%ld] : %s\n"
				, temp_bank_name, pin_num, used_pins[i].reason);
			return 0;
		}
	}

	return -1;
}

static s32 test_set_virtual_gpio(
		u32 gpio_base, const char *bank_name,
		u32 pin_num, u32 value, u32 offset)
{
	u32 addr;
	s32 bank_index;

	bank_index = get_virtual_bank_index(bank_name);

	if (bank_index
			== -1) {
		return -1;
	}

	addr = gpio_base + offset + (0x20 * bank_index);

	if (value == 1) {
		writel(readl(addr) | BIT(pin_num)
				, addr);
	} else {
		writel(readl(addr) & (~BIT(pin_num))
				, addr);
	}

	return 0;
}

static s32 test_get_virtual_gpio_mon(
	u32 gpio_base, const char *bank_name,
	u32 pin_num, u32 offset)
{
	u32 addr;
	s32 bank_index;

	bank_index = get_virtual_bank_index(bank_name);

	if (bank_index
			== -1) {
		return -1;
	}

	addr = gpio_base + offset + (0x20 * bank_index);

	return !!(readl(addr) & BIT(pin_num));
}

static s32 get_normal_bank_index(const char *bank_name)
{
	s32 bank_index = -1;
	s32 i;

	for (i = 0; i < ARRAY_SIZE(bank_list); i++) {
		if (strncmp(bank_list[i].bank_name, bank_name,
				strlen(bank_list[i].bank_name)) == 0) {
			bank_index = bank_list[i].index;
			break;
		}
	}

	if (bank_index == -1) {
		TCC_DEBUG_MSG(
			"\033[31m %s is invalid name \033[0m\n"
			, bank_name);
	}

	return bank_index;
}

static s32 get_lock_bank_index(const char *bank_name)
{
	s32 bank_index = -1;
	s32 i;

	for (i = 0; i < ARRAY_SIZE(bank_list); i++) {
		if (strncmp(bank_list[i].bank_name, bank_name,
				strlen(bank_list[i].bank_name)) == 0) {
			bank_index = bank_list[i].lock_index;
			break;
		}
	}

	if (bank_index == -1) {
		TCC_DEBUG_MSG(
			"\033[31m %s is invalid name \033[0m\n"
			, bank_name);
	}

	return bank_index;
}

static s32 get_virtual_bank_index(
		const char *bank_name)
{
	s32 bank_index = -1;
	s32 i;

	for (i = 0; i < ARRAY_SIZE(bank_list); i++) {
		if (strncmp(bank_list[i].bank_name, bank_name,
				strlen(bank_list[i].bank_name)) == 0) {
			bank_index = bank_list[i].virtual_index;
			break;
		}
	}

	if (bank_index == -1) {
		TCC_DEBUG_MSG(
			"\033[31m %s is invalid name \033[0m\n"
			, bank_name);
	}

	return bank_index;
}

static s32 test_gpio_set_output_lock_enable(
	u32 gpio_base, const char *bank_name,
	u32 pin_num, u32 value)
{
	u32 addr;
	s32 bank_index = get_lock_bank_index(bank_name);

	if (bank_index
			== -1) {
		return -1;
	}

	addr = gpio_base + TCC_TEST_ENLOCKEN
		+ (0x4 * bank_index);

	if (value == 1) {	//output enable lock - enable
		writel(readl(addr) | BIT(pin_num)
				, addr);
	} else {		//output enable lock - disable
		writel(readl(addr) & (~BIT(pin_num))
				, addr);
	}

	return 0;
}

static s32 test_gpio_set_func_lock_enable(
	u32 gpio_base, const char *bank_name,
	u32 pin_num, u32 value)
{
	u32 addr;
	s32 bank_index;

	bank_index = get_lock_bank_index(bank_name);

	if (bank_index
			== -1) {
		return -1;
	}

	addr = gpio_base + TCC_TEST_FNLOCKEN
		+ (0x4 * bank_index);

	if (value == 1) {
		writel(readl(addr) | BIT(pin_num)
				, addr);
	} else {
		writel(readl(addr) & (~BIT(pin_num))
				, addr);
	}

	return 0;
}

static void test_output_by_tpg(
		struct udevice *dev, const char *bank_name)
{
	s32 i, j;
	u32 bank_base;
	u32 addr;
	u32 bank_index;
	u32 backup_value[4];
	struct telechips_pinctrl_priv *priv
		= dev_get_priv(dev);

	for (i = 0; i < priv->nbanks; i++) {
		if (bank_name != NULL &&
			strncmp(priv->bank[i].name,
				bank_name, strlen(bank_name)) != 0) {
			continue;
		}

		bank_base = priv->gpio_base
			+ priv->bank[i].bank_base;
		for (j = 0; j < priv->bank[i].npins; j++) {
			if (is_used_gpio(
					priv->bank[i].name, j) == 0) {
				continue;
			}

			if (strncmp(priv->bank[i].name,
					"gpk", 3) == 0) {
				continue;
			}

			TCC_DEBUG_MSG(
				"bank : %s, pin num : %d\n"
				, priv->bank[i].name, j);

			//save
			backup_value[0]
				= test_gpio_get_output_enable(bank_base, j);
			backup_value[1]
				= test_gpio_get_data(bank_base, j);
			backup_value[2]
				= test_gpio_get_function(bank_base, j);
			backup_value[3]
				= test_gpio_get_input_enable(bank_base, j);

			test_set_virtual_gpio(
				priv->gpio_base, priv->bank[i].name,
				j, 0, TCC_TEST_VI2O_EN);
			test_set_virtual_gpio(
				priv->gpio_base, priv->bank[i].name,
				j, 0, TCC_TEST_VI2O_OEN);

			test_gpio_set_output_enable(bank_base, j, 1);
			test_gpio_set_data(bank_base, j, 1);
			test_gpio_set_function(bank_base, j, 0);

			if (test_get_virtual_gpio_mon(
					priv->gpio_base,
					priv->bank[i].name, j,
					TCC_TEST_MON_DO) != 1) {
				TCC_DEBUG_MSG(
					"\033[31m Failed test mon do - [%s %d] - expect : 1, read : %d\033[0m\n"
					, priv->bank[i].name, j
					, test_get_virtual_gpio_mon(
						priv->gpio_base,
						priv->bank[i].name,
						j, TCC_TEST_MON_DO));
			}
			if (test_get_virtual_gpio_mon(
					priv->gpio_base,
					priv->bank[i].name, j,
					TCC_TEST_MON_OEN) != 1) {
				TCC_DEBUG_MSG(
					"\033[31m Failed test mon oen - [%s %d] - expect : 1, read : %d\033[0m\n"
					, priv->bank[i].name, j
					, test_get_virtual_gpio_mon(
						priv->gpio_base,
						priv->bank[i].name,
						j, TCC_TEST_MON_OEN));
			}

			//restore
			test_gpio_set_output_enable(
					bank_base, j, backup_value[0]);
			test_gpio_set_data(
					bank_base, j, backup_value[1]);
			test_gpio_set_function(
					bank_base, j, backup_value[2]);
			test_gpio_set_input_enable(
					bank_base, j, backup_value[3]);
		}
		TCC_DEBUG_MSG(
			"[%s] test done\n"
			, priv->bank[i].name);
	}
	//virtual gpio
	//vi2o_en -> 0
	//vi2o_oen -> 0

	//real gpio
	//output enable
	//output data -> 1
	//function 0
	//read monitor do, oen
}

static void test_input_by_tpg(
		struct udevice *dev, const char *bank_name)
{
	s32 i, j;
	u32 bank_base;
	u32 addr;
	u32 bank_index;
	u32 backup_value[4];
	struct telechips_pinctrl_priv *priv
		= dev_get_priv(dev);

	for (i = 0; i < priv->nbanks; i++) {
		if (bank_name != NULL &&
			strncmp(priv->bank[i].name,
				bank_name, strlen(bank_name)) != 0) {
			continue;
		}

		bank_base = priv->gpio_base + priv->bank[i].bank_base;
		for (j = 0; j < priv->bank[i].npins; j++) {
			if (is_used_gpio(
					priv->bank[i].name, j) == 0) {
				continue;
			}

			if (strncmp(
					priv->bank[i].name, "gpk", 3) == 0) {
				continue;
			}

			TCC_DEBUG_MSG(
				"bank : %s, pin num : %d\n"
				, priv->bank[i].name, j);

			//save
			backup_value[0]
				= test_gpio_get_output_enable(bank_base, j);
			backup_value[1]
				= test_gpio_get_data(bank_base, j);
			backup_value[2]
				= test_gpio_get_function(bank_base, j);
			backup_value[3]
				= test_gpio_get_input_enable(bank_base, j);

			test_set_virtual_gpio(
					priv->gpio_base,
					priv->bank[i].name,
					j, 0, TCC_TEST_VI2I_EN);
			test_set_virtual_gpio(
					priv->gpio_base,
					priv->bank[i].name,
					j, 1, TCC_TEST_VI2O_EN);
			test_set_virtual_gpio(
					priv->gpio_base,
					priv->bank[i].name,
					j, 1, TCC_TEST_VI2O_OEN);
			test_set_virtual_gpio(
					priv->gpio_base,
					priv->bank[i].name,
					j, 1, TCC_TEST_VI2I_DAT);

			//test
			test_gpio_set_output_enable(bank_base, j, 0);
			test_gpio_set_data(bank_base, j, 0);
			test_gpio_set_function(bank_base, j, 0);
			test_gpio_set_input_enable(bank_base, j, 1);

			if (test_gpio_get_data(bank_base, j) != 1) {
				TCC_DEBUG_MSG(
						"\033[31m Failed test - [%s %d] - expect : 1, read : %d\033[0m\n"
						, priv->bank[i].name, j,
						test_gpio_get_data(
							bank_base, j));
			}

			//restore
			test_set_virtual_gpio(priv->gpio_base,
					priv->bank[i].name, j, 0,
					TCC_TEST_VI2O_EN);
			test_set_virtual_gpio(priv->gpio_base,
					priv->bank[i].name, j, 0,
					TCC_TEST_VI2O_OEN);
			test_set_virtual_gpio(priv->gpio_base,
					priv->bank[i].name, j, 0,
					TCC_TEST_VI2I_DAT);

			test_gpio_set_output_enable(
					bank_base, j, backup_value[0]);
			test_gpio_set_data(
					bank_base, j, backup_value[1]);
			test_gpio_set_function(
					bank_base, j, backup_value[2]);
			test_gpio_set_input_enable(
					bank_base, j, backup_value[3]);
		}
		TCC_DEBUG_MSG("[%s] test done\n", priv->bank[i].name);
	}
	//virtual gpio
	//vi2i_en -> 0		Virtual Input Enable Register
	//vi2o_en -> 1		Virtual Output Enable Register
	//vi2o_oen -> 1		Virtual Output Data Regster
	//vi2i_dat -> 1		Virtual Input Data Register

	//real gpio
	//function 0
	//output disable
	//output data -> 0
	//input buffer enable
	//read gpio data register
}

static void test_function_lock(
		struct udevice *dev, const char *bank_name)
{
	s32 i;
	s32 j;
	u32 bank_base;
	u32 addr;
	u32 bank_index;
	u32 backup_value;
	u32 expect_value;
	struct telechips_pinctrl_priv *priv
		= dev_get_priv(dev);

	for (i = 0; i < priv->nbanks; i++) {
		if (bank_name != NULL &&
			strncmp(priv->bank[i].name,
				bank_name, strlen(bank_name)) != 0) {
			continue;
		}

		if (strncmp(priv->bank[i].name,
					"gpk", 3) == 0) {
			continue;
		}

		bank_base = priv->gpio_base + priv->bank[i].bank_base;
		for (j = 0; j < priv->bank[i].npins; j++) {
			//save
			backup_value = test_gpio_get_function(bank_base, j);

			//test
			test_gpio_set_func_lock_enable
				(priv->gpio_base, priv->bank[i].name, j, 1);
			expect_value = backup_value > 5 ?
				backup_value - 1 : backup_value + 1;
			test_gpio_set_function(bank_base, j, expect_value);

			if (test_gpio_get_function(bank_base, j)
					== expect_value) {
				TCC_DEBUG_MSG(
					"\033[31m Failed test - [%s %d] \033[0m\n"
					, priv->bank[i].name, j);
				TCC_DEBUG_MSG(
					"\033[31m backup_value : %d, expect_value : %d \033[0m\n"
					, backup_value, expect_value);
			}

			//restore
			test_gpio_set_func_lock_enable
				(priv->gpio_base, priv->bank[i].name, j, 0);
			test_gpio_set_function
				(bank_base, j, backup_value);
		}
		TCC_DEBUG_MSG(
			"[%s] test done\n"
			, priv->bank[i].name);
	}
}

static void test_output_enable_lock(
		struct udevice *dev, const char *bank_name)
{
	s32 i, j;
	u32 bank_base;
	u32 addr;
	u32 bank_index;
	u32 backup_value[3];
	struct telechips_pinctrl_priv *priv
		= dev_get_priv(dev);

	for (i = 0; i < priv->nbanks; i++) {
		if (bank_name != NULL &&
			strncmp(priv->bank[i].name,
				bank_name, strlen(bank_name)) != 0) {
			continue;
		}

		if (strncmp(priv->bank[i].name,
					"gpk", 3) == 0) {
			continue;
		}

		bank_base = priv->gpio_base + priv->bank[i].bank_base;
		for (j = 0; j < priv->bank[i].npins; j++) {
			//save
			backup_value[0]
				= test_gpio_get_output_enable(bank_base, j);

			//test
			test_gpio_set_output_lock_enable
				(priv->gpio_base, priv->bank[i].name, j, 0);
			test_gpio_set_output_enable(bank_base, j, 0);

			test_gpio_set_output_enable(bank_base, j, 1);
			if (test_gpio_get_output_enable(bank_base, j) == 1) {
				TCC_DEBUG_MSG(
					"\033[31m output enable Failed test - [%s %d] \033[0m\n"
					, priv->bank[i].name, j);
			}

			//restore
			test_gpio_set_output_lock_enable
				(priv->gpio_base, priv->bank[i].name, j, 1);
			test_gpio_set_output_enable
				(bank_base, j, backup_value[0]);
		}
		TCC_DEBUG_MSG(
			"[%s] test done\n", priv->bank[i].name);
	}
}

void test_rw(struct udevice *dev,
		const char *bank_name, s32 pin_num)
{
	s32 i;
	s32 j;
	u32 addr;
	struct telechips_pinctrl_priv *priv
		= dev_get_priv(dev);

	if (pin_num == -1) {
		TCC_DEBUG_MSG(
			"start test [%s]\n", bank_name);
	} else {
		TCC_DEBUG_MSG(
			"start test [%s %d]\n", bank_name, pin_num);
	}

	for (i = 0; i < priv->nbanks; i++) {
		if (strncmp(priv->bank[i].name, bank_name, strlen(bank_name))
				== 0) {
			if (strncmp(bank_name, "gpk", 3) != 0) {
				addr = priv->gpio_base
					+ priv->bank[i].bank_base;
			} else {
				continue;
			}
			for (j = 0; j < priv->bank[i].npins; j++) {
				if (pin_num == j || pin_num == -1) {
					if (is_used_gpio(bank_name, j)
							== 0) {
						continue;
					}
					TCC_DEBUG_MSG(
						"bank : %s, pin num : %d\n"
						, priv->bank[i].name, j);

					if ((is_gpio_ma_4(bank_name, j) == 0)
						&& (
						test_function(addr, j)
						|| test_slew_rate(addr, j)
						|| test_input_type(addr, j)
						|| test_input_enable(addr, j)
						|| test_drive_strength(
							addr, j))) {
						TCC_DEBUG_MSG(
							"\033[31m Failed test - [%s %d] \033[0m\n"
							, priv->bank[i].name
							, j);
					} else if (test_output(addr, j)
						|| test_output_enable(addr, j)
						|| test_function(addr, j)
						|| test_pull_enable(addr, j)
						|| test_pull_select(addr, j)
						|| test_slew_rate(addr, j)
						|| test_input_type(addr, j)
						|| test_input_enable(addr, j)
						|| test_drive_strength(addr, j)
						|| test_xor_register(addr, j)
						|| test_bic_register(addr, j)
						|| test_or_register(addr, j)
						) {
						TCC_DEBUG_MSG(
							"\033[31m Failed test - [%s %d] \033[0m\n"
							, priv->bank[i].name
							, j);
					}
				}
			}
		}
	}
}

void test_rw_all(struct udevice *dev)
{
	s32 i;
	s32 pin_num;
	struct telechips_pinctrl_priv *priv;
	u32 addr;

	TCC_DEBUG_MSG("start test all\n");

	priv = dev_get_priv(dev);

	TCC_DEBUG_MSG("priv->nbanks : %ld\n", priv->nbanks);
	for (i = 0; i < priv->nbanks; i++) {
		addr = priv->gpio_base + priv->bank[i].bank_base;
		TCC_DEBUG_MSG("addr : 0x%lX\n", addr);
		for (pin_num = 0; pin_num < priv->bank[i].npins; pin_num++) {
			TCC_DEBUG_MSG("%s %d\n", priv->bank[i].name, pin_num);
			if (is_used_gpio(priv->bank[i].name, pin_num)
					== 0) {
				continue;
			}

			TCC_DEBUG_MSG(
				"bank : %s, pin num : %d\n"
				, priv->bank[i].name, pin_num);

			if (test_output(addr, pin_num)
				|| test_output_enable(addr, pin_num)
				|| test_function(addr, pin_num)
				|| test_pull_enable(addr, pin_num)
				|| test_pull_select(addr, pin_num)
				|| test_slew_rate(addr, pin_num)
				|| test_input_type(addr, pin_num)
				|| test_input_enable(addr, pin_num)
				|| test_drive_strength(addr, pin_num)
				|| test_xor_register(addr, pin_num)
				|| test_bic_register(addr, pin_num)
				|| test_or_register(addr, pin_num)
				) {
				TCC_DEBUG_MSG(
					"\033[31m Failed test - [%s %d] \033[0m\n"
					, priv->bank[i].name, pin_num);
			}
		}
	}

	if (test_eint(priv->gpio_base)
		|| test_eclk(priv->gpio_base)
	) {
		TCC_DEBUG_MSG(
			"\033[31m Failed test \033[0m\n");
	}
}

void test_eint_eclk(struct udevice *dev)
{
	struct telechips_pinctrl_priv *priv;

	TCC_DEBUG_MSG("start test eint, eclk\n");

	priv = dev_get_priv(dev);
	if (test_eint(priv->gpio_base)
		|| test_eclk(priv->gpio_base)
	) {
		TCC_DEBUG_MSG(
			"\033[31m Failed test \033[0m\n");
	}
}

DEFINE_MUTEX(eint_rising_handler_mutex);
DEFINE_MUTEX(eint_falling_handler_mutex);

s32 rising_flag;
s32 falling_flag;

void test_rising_handler(void *args)
{
	TCC_DEBUG_MSG("rising handler\n");
	mutex_lock(&eint_rising_handler_mutex);
	rising_flag = 1;
	mutex_unlock(&eint_rising_handler_mutex);
}

void test_falling_handler(void *args)
{
	TCC_DEBUG_MSG("falling handler\n");
	mutex_lock(&eint_falling_handler_mutex);
	falling_flag = 1;
	mutex_unlock(&eint_falling_handler_mutex);
}

void test_eint_port_mux(
		struct udevice *dev, u32 bank_base,
		s32 pin_num, s32 eint_source, s32 sel)
{
	struct telechips_pinctrl_priv *priv;
	char test_c;
	s32 eint;
	s32 i;

	TCC_DEBUG_MSG("start external interrupt port mux test\n");

	priv = dev_get_priv(dev);

//handler
	for (eint = 0; eint < 16; eint++) {
		if (sel != -1) {
			eint = sel;
		/* comment for kernel coding style */
		}
		pr_err("external selection number : %d\n", eint);

//set test
		//prepare GPIO
		test_gpio_set_function(bank_base, pin_num, 0);
		test_gpio_set_output_enable(bank_base, pin_num, 1);
		test_gpio_set_input_enable(bank_base, pin_num, 1);
		test_gpio_set_data(bank_base, pin_num, 0);

		//set GPIO_A0 at eint 0~15
		test_gpio_set_eint(priv->gpio_base, eint, eint_source);

		//irq id
		//external id : 0~15 + 16~31
		irq_install_handler(eint + 32, test_rising_handler, NULL);
		irq_install_handler(eint + 16 + 32, test_falling_handler, NULL);

		irq_set_type(eint + 32, IRQ_TYPE_EDGE_RISING);
		irq_set_type(eint + 16 + 32,
				IRQ_TYPE_EDGE_RISING);	//falling

		//enable irq
		irq_unmask(eint + 32);			//rising
		irq_unmask(eint + 16 + 32);		//falling

//rising signal and wait handler
		mutex_lock(&eint_rising_handler_mutex);
		rising_flag = 0;
		mutex_unlock(&eint_rising_handler_mutex);

		test_gpio_set_data(bank_base, pin_num, 1);
		pr_err("0 -> 1\n");
		while (rising_flag
				== 0) {
		}

//falling signal and wait handler
		mutex_lock(&eint_falling_handler_mutex);
		falling_flag = 0;
		mutex_unlock(&eint_falling_handler_mutex);

		test_gpio_set_data(bank_base, pin_num, 0);
		pr_err("1 -> 0\n");
		while (falling_flag
				== 0) {
		}

//clear test
		//clear eint
		test_gpio_set_eint(priv->gpio_base, eint, 0);

		//clear handler
		irq_free_handler(eint + 32);
		irq_free_handler(eint + 16 + 32);

		//clear irq
		irq_mask(eint + 32);
		irq_mask(eint + 16 + 32);

		if (sel != -1) {
			break;
		/* comment for kernel coding style */
		}
	}
}

static s32 test_or_register(u32 addr, s32 pin_num)
{
	u32 read_value;
	u32 expect_value;
	u32 pre_value;
	u32 backup_value[3];

	backup_value[0] = test_gpio_get_function(addr, pin_num);
	backup_value[1] = test_gpio_get_output_enable(addr, pin_num);
	backup_value[2] = test_gpio_get_data(addr, pin_num);

	test_gpio_set_function(addr, pin_num, 0);
	test_gpio_set_output_enable(addr, pin_num, 1);
	test_gpio_set_data(addr, pin_num, 0);

	pre_value = test_gpio_get_data(addr, pin_num);
	test_gpio_set_or_reg(addr, pin_num);
	read_value = test_gpio_get_data(addr, pin_num);
	expect_value = 1;
	if (expect_value != read_value) {
		TCC_DEBUG_MSG(
			"Failed test - expect value : %ld, real value : %ld\n"
			, expect_value, read_value);
		return -1;
	}
	TCC_DEBUG_MSG(
		"output [%ld -> %ld]\n"
		, pre_value, read_value);

	test_gpio_set_function(addr, pin_num, backup_value[0]);
	test_gpio_set_output_enable(addr, pin_num, backup_value[1]);
	test_gpio_set_data(addr, pin_num, backup_value[2]);

	return 0;
}

static s32 test_bic_register(u32 addr, s32 pin_num)
{
	u32 read_value;
	u32 expect_value;
	u32 pre_value;
	u32 backup_value[3];

	backup_value[0] = test_gpio_get_function(addr, pin_num);
	backup_value[1] = test_gpio_get_output_enable(addr, pin_num);
	backup_value[2] = test_gpio_get_data(addr, pin_num);

	test_gpio_set_function(addr, pin_num, 0);
	test_gpio_set_output_enable(addr, pin_num, 1);
	test_gpio_set_data(addr, pin_num, 1);

	pre_value = test_gpio_get_data(addr, pin_num);
	test_gpio_set_bic_reg(addr, pin_num);
	read_value = test_gpio_get_data(addr, pin_num);
	expect_value = 0;
	if (expect_value != read_value) {
		TCC_DEBUG_MSG(
			"Failed test - expect value : %ld, real value : %ld\n"
			, expect_value, read_value);
		return -1;
	}
	TCC_DEBUG_MSG(
		"output [%ld -> %ld]\n"
		, pre_value, read_value);

	test_gpio_set_function(addr, pin_num, backup_value[0]);
	test_gpio_set_output_enable(addr, pin_num, backup_value[1]);
	test_gpio_set_data(addr, pin_num, backup_value[2]);

	return 0;
}

static s32 test_xor_register(u32 addr, s32 pin_num)
{
	u32 read_value;
	u32 expect_value;
	u32 pre_value;
	u32 backup_value[3];

	backup_value[0] = test_gpio_get_function(addr, pin_num);
	backup_value[1] = test_gpio_get_output_enable(addr, pin_num);
	backup_value[2] = test_gpio_get_data(addr, pin_num);

	test_gpio_set_function(addr, pin_num, 0);
	test_gpio_set_output_enable(addr, pin_num, 1);
	test_gpio_set_data(addr, pin_num, 0);

	pre_value = test_gpio_get_data(addr, pin_num);
	test_gpio_set_xor_reg(addr, pin_num);
	read_value = test_gpio_get_data(addr, pin_num);
	expect_value = 1;
	if (expect_value != read_value) {
		TCC_DEBUG_MSG(
			"Failed test - expect value : %ld, real value : %ld\n"
			, expect_value, read_value);
		return -1;
	}
	TCC_DEBUG_MSG(
		"output [%ld -> %ld]\n"
		, pre_value, read_value);

	pre_value = test_gpio_get_data(addr, pin_num);
	test_gpio_set_xor_reg(addr, pin_num);
	read_value = test_gpio_get_data(addr, pin_num);
	expect_value = 0;
	if (expect_value != read_value) {
		TCC_DEBUG_MSG(
			"Failed test - expect value : %ld, real value : %ld\n"
			, expect_value, read_value);
		return -1;
	}
	TCC_DEBUG_MSG(
		"output [%ld -> %ld]\n"
		, pre_value, read_value);

	test_gpio_set_function(addr, pin_num, backup_value[0]);
	test_gpio_set_output_enable(addr, pin_num, backup_value[1]);
	test_gpio_set_data(addr, pin_num, backup_value[2]);

	return 0;
}

static s32 test_output(u32 addr, s32 pin_num)
{
	u32 read_value;
	u32 expect_value;
	u32 pre_value;
	u32 backup_value[3];

	backup_value[0] = test_gpio_get_function(addr, pin_num);
	backup_value[1] = test_gpio_get_output_enable(addr, pin_num);
	backup_value[2] = test_gpio_get_data(addr, pin_num);

	test_gpio_set_function(addr, pin_num, 0);
	test_gpio_set_output_enable(addr, pin_num, 1);

	for (expect_value = 0; expect_value <= 1; expect_value++) {
		pre_value = test_gpio_get_data(addr, pin_num);
		test_gpio_set_data(addr, pin_num, expect_value);
		read_value = test_gpio_get_data(addr, pin_num);
		if (expect_value != read_value) {
			TCC_DEBUG_MSG(
				"Failed test - expect value : %ld, real value : %ld\n"
				, expect_value, read_value);
			return -1;
		}
		TCC_DEBUG_MSG(
			"output [%ld -> %ld]\n"
			, pre_value, read_value);
	}

	test_gpio_set_function(addr, pin_num, backup_value[0]);
	test_gpio_set_output_enable(addr, pin_num, backup_value[1]);
	test_gpio_set_data(addr, pin_num, backup_value[2]);

	return 0;
}

static s32 test_output_enable(u32 addr, s32 pin_num)
{
	u32 expect_value;
	u32 read_value;
	u32 pre_value;
	u32 backup_value;

	backup_value = test_gpio_get_output_enable(addr, pin_num);

	for (expect_value = 0; expect_value <= 1; expect_value++) {
		pre_value = test_gpio_get_output_enable(addr, pin_num);
		test_gpio_set_output_enable(addr, pin_num, expect_value);
		read_value = test_gpio_get_output_enable(addr, pin_num);
		if (expect_value != read_value) {
			TCC_DEBUG_MSG(
				"Failed test - expect value : %ld, real value : %ld\n"
				, expect_value, read_value);
			return -1;
		}
		TCC_DEBUG_MSG(
			"output enable [%ld -> %ld]\n"
			, pre_value, read_value);
	}

	test_gpio_set_output_enable(addr, pin_num, backup_value);

	return 0;
}

static s32 test_function(u32 addr, s32 pin_num)
{
	u32 expect_value;
	u32 read_value;
	u32 backup_value;
	u32 pre_value;

	backup_value = test_gpio_get_function(addr, pin_num);

	for (expect_value = 0; expect_value < 16; expect_value++) {
		pre_value = test_gpio_get_function(addr, pin_num);
		test_gpio_set_function(addr, pin_num, expect_value);
		read_value = test_gpio_get_function(addr, pin_num);
		if (expect_value != read_value) {
			TCC_DEBUG_MSG(
				"Failed test - expect value : %ld, real value : %ld\n"
				, expect_value, read_value);
			return -1;
		}
		TCC_DEBUG_MSG(
			"function [%ld -> %ld]\n"
			, pre_value, read_value);
	}

	test_gpio_set_function(addr, pin_num, backup_value);

	return 0;
}

static s32 test_pull_enable(u32 addr, s32 pin_num)
{
	u32 expect_value;
	u32 read_value;
	u32 pre_value;
	u32 backup_value;

	backup_value = test_gpio_get_pull_enable(addr, pin_num);

	for (expect_value = 0; expect_value <= 1; expect_value++) {
		pre_value = test_gpio_get_pull_enable(addr, pin_num);
		test_gpio_set_pull_enable(addr, pin_num, expect_value);
		read_value = test_gpio_get_pull_enable(addr, pin_num);
		if (expect_value != read_value) {
			TCC_DEBUG_MSG(
				"Failed test - expect value : %ld, real value : %ld\n"
				, expect_value, read_value);
			return -1;
		}
		TCC_DEBUG_MSG(
			"pull enable [%ld -> %ld]\n"
			, pre_value, read_value);
	}

	test_gpio_set_pull_enable(addr, pin_num, backup_value);

	return 0;
}

static s32 test_pull_select(u32 addr, s32 pin_num)
{
	u32 expect_value;
	u32 read_value;
	u32 pre_value;
	u32 backup_value[2];

	backup_value[0] = test_gpio_get_pull_enable(addr, pin_num);
	backup_value[1] = test_gpio_get_pull_select(addr, pin_num);
	test_gpio_set_pull_enable(addr, pin_num, 1);

	for (expect_value = 0; expect_value <= 1; expect_value++) {
		pre_value = test_gpio_get_pull_select(addr, pin_num);
		test_gpio_set_pull_select(addr, pin_num, expect_value);
		read_value = test_gpio_get_pull_select(addr, pin_num);
		if (expect_value != read_value) {
			TCC_DEBUG_MSG(
				"Failed test - expect value : %ld, real value : %ld\n"
				, expect_value, read_value);
			return -1;
		}
		TCC_DEBUG_MSG(
			"pull select [%ld -> %ld]\n"
			, pre_value, read_value);
	}

	test_gpio_set_pull_enable(addr, pin_num, backup_value[0]);
	test_gpio_set_pull_select(addr, pin_num, backup_value[1]);

	return 0;
}

static s32 test_slew_rate(u32 addr, s32 pin_num)
{
	u32 expect_value;
	u32 read_value;
	u32 pre_value;
	u32 backup_value;

	backup_value = test_gpio_get_slew_rate(addr, pin_num);

	for (expect_value = 0; expect_value <= 1; expect_value++) {
		pre_value = test_gpio_get_slew_rate(addr, pin_num);
		test_gpio_set_slew_rate(addr, pin_num, expect_value);
		read_value = test_gpio_get_slew_rate(addr, pin_num);
		if (expect_value != read_value) {
			TCC_DEBUG_MSG(
				"Failed test - expect value : %ld, real value : %ld\n"
				, expect_value, read_value);
			return -1;
		}
		TCC_DEBUG_MSG(
			"slew rate [%ld -> %ld]\n"
			, pre_value, read_value);
	}

	test_gpio_set_slew_rate(addr, pin_num, backup_value);

	return 0;
}

static s32 test_input_type(u32 addr, s32 pin_num)
{
	u32 expect_value;
	u32 read_value;
	u32 pre_value;
	u32 backup_value;

	backup_value = test_gpio_get_input_type(addr, pin_num);

	for (expect_value = 0; expect_value <= 1; expect_value++) {
		pre_value = test_gpio_get_input_type(addr, pin_num);
		test_gpio_set_input_type(addr, pin_num, expect_value);
		read_value = test_gpio_get_input_type(addr, pin_num);
		if (expect_value != read_value) {
			TCC_DEBUG_MSG(
				"Failed test - expect value : %ld, real value : %ld\n"
				, expect_value, read_value);
			return -1;
		}
		TCC_DEBUG_MSG(
			"input type [%ld -> %ld]\n"
			, pre_value, read_value);
	}

	test_gpio_set_input_type(addr, pin_num, backup_value);

	return 0;
}

static s32 test_input_enable(u32 addr, s32 pin_num)
{
	u32 read_value;
	u32 expect_value;
	u32 pre_value;
	u32 backup_value[2];

	backup_value[0] = test_gpio_get_input_enable(addr, pin_num);
	backup_value[1] = test_gpio_get_function(addr, pin_num);
	test_gpio_set_function(addr, pin_num, 0);

	for (expect_value = 0; expect_value <= 1; expect_value++) {
		pre_value = test_gpio_get_input_enable(addr, pin_num);
		test_gpio_set_input_enable(addr, pin_num, expect_value);
		read_value = test_gpio_get_input_enable(addr, pin_num);
		if (expect_value != read_value) {
			TCC_DEBUG_MSG(
				"Failed test - expect value : %ld, real value : %ld\n"
				, expect_value, read_value);
			return -1;
		}
		TCC_DEBUG_MSG(
			"input enable [%ld -> %ld]\n"
			, pre_value, read_value);
	}

	test_gpio_set_input_enable(addr, pin_num, backup_value[0]);
	test_gpio_set_function(addr, pin_num, backup_value[1]);

	return 0;
}

static s32 test_drive_strength(u32 addr, s32 pin_num)
{
	u32 expect_value;
	u32 read_value;
	u32 pre_value;
	u32 backup_value;

	backup_value = test_gpio_get_drive_strength(addr, pin_num);

	for (expect_value = 0; expect_value < 4; expect_value++) {
		pre_value = test_gpio_get_drive_strength(addr, pin_num);
		test_gpio_set_drive_strength(addr, pin_num, expect_value);
		read_value = test_gpio_get_drive_strength(addr, pin_num);
		if (expect_value != read_value) {
			TCC_DEBUG_MSG(
				"Failed test - expect value : %ld, real value : %ld\n"
				, expect_value, read_value);
			return -1;
		}
		TCC_DEBUG_MSG(
			"drive strength [%ld -> %ld]\n"
			, pre_value, read_value);
	}

	test_gpio_set_drive_strength(addr, pin_num, backup_value);

	return 0;
}

static u32 get_pin_base(
		struct udevice *dev, const char *pin_name, u32 *pin_num)
{
	struct telechips_pinctrl_priv *priv = dev_get_priv(dev);
	u32 i;
	size_t len;
	ulong ul_pin_num;

	for (i = 0; i < priv->nbanks; i++) {
		len = priv->bank[i].name ? strlen(priv->bank[i].name) : 0;

		if (strncasecmp(
				pin_name, priv->bank[i].name, len) == 0) {
			if (strict_strtoul(
					pin_name + len, 10, &ul_pin_num) != 0) {
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

	return -EINVAL;	//not exist
}

static s32 test_eclk(u32 addr)
{
	u32 expect_value;
	u32 read_value;
	u32 eclk_num;
	u32 pre_value;
	u32 backup_value;

	for (eclk_num = 0; eclk_num < 4; eclk_num++) {
		backup_value = test_gpio_get_eclk(addr, eclk_num);

		TCC_DEBUG_MSG(
				"external External Input Clock number : %ld\n"
				, eclk_num);

		for (expect_value = 0; expect_value < 255; expect_value++) {
			pre_value = test_gpio_get_eclk(addr, eclk_num);
			test_gpio_set_eclk(addr, eclk_num, expect_value);
			read_value = test_gpio_get_eclk(addr, eclk_num);
			if (expect_value != read_value) {
				TCC_DEBUG_MSG(
					"Failed test - expect value : %ld, real value : %ld\n"
					, expect_value, read_value);
				return -1;
			}
			TCC_DEBUG_MSG(
				"eclk [%ld -> %ld]\n"
				, pre_value, read_value);
		}

		test_gpio_set_eclk(addr, eclk_num, backup_value);
	}

	return 0;
}

static s32 test_eint(u32 addr)
{
	u32 expect_value;
	u32 read_value;
	u32 eint_num;
	u32 pre_value;
	u32 backup_value;

	for (eint_num = 0; eint_num < 16; eint_num++) {
		backup_value = test_gpio_get_eint(addr, eint_num);

		TCC_DEBUG_MSG("external interrupt number : %ld\n", eint_num);
		for (expect_value = 0; expect_value < 255; expect_value++) {
			pre_value = test_gpio_get_eint(addr, eint_num);
			test_gpio_set_eint(addr, eint_num, expect_value);
			read_value = test_gpio_get_eint(addr, eint_num);
			if (expect_value != read_value) {
				TCC_DEBUG_MSG(
						"Failed test - expect value : %ld, real value : %ld\n",
					expect_value, read_value);
				return -1;
			}
			TCC_DEBUG_MSG(
				"eint [%ld -> %ld]\n"
				, pre_value, read_value);
		}

		test_gpio_set_eint(addr, eint_num, backup_value);
	}

	return 0;
}

static s32 test_gpio_get_eint(u32 bank_base, u32 eint_num)
{
	u32 eint_addr
		= bank_base + TCC_TEST_EINTSEL0 + ((eint_num / 4) * 0x4);
	u32 bit_shift = (eint_num % 4) * 8;
	u32 value = readl(eint_addr);

	return ((value & (0xff << bit_shift)) >> bit_shift);
}

static s32 test_gpio_set_eint(
		u32 bank_base, u32 eint_num, u32 eint_source)
{
	u32 eint_addr
		= bank_base + TCC_TEST_EINTSEL0 + ((eint_num / 4) * 0x4);
	u32 bit_shift = (eint_num % 4) * 8;
	u32 value = readl(eint_addr);

	value &= ~(0xff << bit_shift);
	value |= (0xff & eint_source) << bit_shift;

	writel(value, eint_addr);

	return 0;
}

static s32 test_gpio_get_eclk(
		u32 bank_base, u32 eclk_num)
{
	u32 eclk_addr = bank_base + TCC_TEST_ECLKSEL;
	u32 bit_shift = eclk_num * 8;
	u32 value = readl(eclk_addr);

	if (eclk_num > 3) {
		return -EINVAL;
		/* comment for kernel coding style */
	}

	return ((value & (0xff << bit_shift)) >> bit_shift);
}

static s32 test_gpio_set_eclk(
		u32 bank_base, u32 eclk_num, u32 eint_source)
{
	u32 eclk_addr = bank_base + TCC_TEST_ECLKSEL;
	u32 bit_shift = eclk_num * 8;
	u32 value = readl(eclk_addr);

	if (eclk_num > 3) {
		return -EINVAL;
		/* comment for kernel coding style */
	}

	value &= ~(0xff << bit_shift);
	value |= (0xff & eint_source) << bit_shift;

	writel(value, eclk_addr);

	return 0;
}

static s32 test_gpio_get_function(u32 bank_base, u32 pin_num)
{
	u32 func_addr
		= bank_base + TCC_TEST_GPIO_FN0 + ((pin_num / 8) * 0x4);
	u32 bit_shift = (pin_num % 8) * 4;
	u32 value = readl(func_addr);

	return ((value & (0xf << bit_shift)) >> bit_shift);
}

static s32 test_gpio_set_function(u32 bank_base, u32 pin_num, u32 func)
{
	u32 func_addr = bank_base + TCC_TEST_GPIO_FN0 + ((pin_num / 8) * 0x4);
	u32 bit_shift = (pin_num % 8) * 4;
	u32 value = readl(func_addr);


	value &= ~(0xf << bit_shift);
	value |= (0xf & func) << bit_shift;

	writel(value, func_addr);

	return 0;
}

static s32 test_gpio_get_drive_strength(u32 bank_base, u32 pin_num)
{
	u32 drive_strength_addr
		= bank_base + TCC_TEST_GPIO_CD0 + ((pin_num / 16) * 0x4);
	u32 bit_shift = (pin_num % 16) * 2;
	u32 value = readl(drive_strength_addr);

	return ((value & (0x3 << bit_shift)) >> bit_shift);
}

static s32 test_gpio_set_drive_strength(
		u32 bank_base, u32 pin_num, u32 drive_strength)
{
	u32 drive_strength_addr
		= bank_base + TCC_TEST_GPIO_CD0
			+ ((pin_num / 16) * 0x4);
	u32 bit_shift = (pin_num % 16) * 2;
	u32 value = readl(drive_strength_addr);

	value &= ~(0x3 << bit_shift);
	value |= (0x3 & drive_strength) << bit_shift;

	writel(value, drive_strength_addr);

	return 0;
}

static s32 test_gpio_get_pull_enable(u32 bank_base, u32 pin_num)
{
	u32 pull_enable_addr = bank_base + TCC_TEST_GPIO_PE;

	return !!(readl(pull_enable_addr) & BIT(pin_num));
}

static s32 test_gpio_set_pull_enable(u32 bank_base, u32 pin_num, u32 pe)
{
	u32 pull_enable_addr = bank_base + TCC_TEST_GPIO_PE;

	if (pe == 1) {	//pull enable
		writel(
			readl(pull_enable_addr) | BIT(pin_num)
			, pull_enable_addr);
	} else {		//pull disable
		writel(
			readl(pull_enable_addr) & (~BIT(pin_num))
			, pull_enable_addr);
	}

	return 0;
}

static s32 test_gpio_get_pull_select(u32 bank_base, u32 pin_num)
{
	u32 pull_select_addr = bank_base + TCC_TEST_GPIO_PS;

	return !!(readl(pull_select_addr) & BIT(pin_num));
}

static s32 test_gpio_set_pull_select(u32 bank_base, u32 pin_num, u32 ps)
{
	u32 pull_select_addr = bank_base + TCC_TEST_GPIO_PS;

	if (ps == 1) {	//pull up
		writel(
			readl(pull_select_addr) | BIT(pin_num)
			, pull_select_addr);
	} else {		//pull down
		writel(
			readl(pull_select_addr) & (~BIT(pin_num))
			, pull_select_addr);
	}

	return 0;
}

static s32 test_gpio_get_output_enable(u32 bank_base, u32 pin_num)
{
	u32 oen_addr = bank_base + TCC_TEST_GPIO_OEN;

	return !!(readl(oen_addr) & BIT(pin_num));
}

static s32 test_gpio_set_output_enable(u32 bank_base, u32 pin_num, u32 oen)
{
	u32 oen_addr = bank_base + TCC_TEST_GPIO_OEN;

	if (oen == 1) {	//output enable
		writel(readl(oen_addr) | BIT(pin_num), oen_addr);
		/* comment for kernel coding style */
	} else {		//output disable
		writel(readl(oen_addr) & (~BIT(pin_num)), oen_addr);
		/* comment for kernel coding style */
	}

	return 0;
}

static s32 test_gpio_get_data(u32 bank_base, u32 pin_num)
{
	u32 data_addr = bank_base + TCC_TEST_GPIO_DATA;

	return !!(readl(data_addr) & BIT(pin_num));
}

static s32 test_gpio_set_or_reg(u32 bank_base, u32 pin_num)
{
	u32 data_addr = bank_base + TCC_TEST_GPIO_OR;

	writel(readl(data_addr) | BIT(pin_num), data_addr);

	return 0;
}

static s32 test_gpio_set_bic_reg(u32 bank_base, u32 pin_num)
{
	u32 data_addr = bank_base + TCC_TEST_GPIO_BIC;

	writel(readl(data_addr) | BIT(pin_num), data_addr);

	return 0;
}

static s32 test_gpio_set_xor_reg(u32 bank_base, u32 pin_num)
{
	u32 data_addr = bank_base + TCC_TEST_GPIO_XOR;

	writel(readl(data_addr) | BIT(pin_num), data_addr);

	return 0;
}

static s32 test_gpio_set_data(u32 bank_base, u32 pin_num, u32 value)
{
	u32 data_addr = bank_base + TCC_TEST_GPIO_DATA;

	if (value == 1) {	//output high
		writel(readl(data_addr) | BIT(pin_num), data_addr);
		/* comment for kernel coding style */
	} else {			//output low
		writel(readl(data_addr) & (~BIT(pin_num)), data_addr);
		/* comment for kernel coding style */
	}

	return 0;
}

static s32 test_gpio_get_input_enable(u32 bank_base, u32 pin_num)
{
	u32 ien_addr = bank_base + TCC_TEST_GPIO_IEN;

	return !!(readl(ien_addr) & BIT(pin_num));
}

static s32 test_gpio_set_input_enable(
		u32 bank_base, u32 pin_num, u32 ien)
{
	u32 ien_addr = bank_base + TCC_TEST_GPIO_IEN;

	if (ien == 1) {	//input enable
		writel(readl(ien_addr) | BIT(pin_num), ien_addr);
		/* comment for kernel coding style */
	} else {		//input disable
		writel(readl(ien_addr) & (~BIT(pin_num)), ien_addr);
		/* comment for kernel coding style */
	}

	return 0;
}

static s32 test_gpio_get_input_type(u32 bank_base, u32 pin_num)
{
	u32 is_addr = bank_base + TCC_TEST_GPIO_IS;

	return !!(readl(is_addr) & BIT(pin_num));
}

static s32 test_gpio_set_input_type(u32 bank_base, u32 pin_num, u32 is)
{
	u32 is_addr = bank_base + TCC_TEST_GPIO_IS;

	if (is == 1) {	//Schmitt input
		writel(readl(is_addr) | BIT(pin_num), is_addr);
		/* comment for kernel coding style */
	} else {		//CMOS input
		writel(readl(is_addr) & (~BIT(pin_num)), is_addr);
		/* comment for kernel coding style */
	}

	return 0;
}

static s32 test_gpio_get_slew_rate(u32 bank_base, u32 pin_num)
{
	u32 sr_addr = bank_base + TCC_TEST_GPIO_SR;

	return !!(readl(sr_addr) & BIT(pin_num));
}

static s32 test_gpio_set_slew_rate(u32 bank_base, u32 pin_num, u32 sr)
{
	u32 sr_addr = bank_base + TCC_TEST_GPIO_SR;

	if (sr == 1) {	//Slow slew rate
		writel(readl(sr_addr) | BIT(pin_num), sr_addr);
		/* comment for kernel coding style */
	} else {		//Fast slew rate
		writel(readl(sr_addr) & (~BIT(pin_num)), sr_addr);
		/* comment for kernel coding style */
	}

	return 0;
}

static s32 do_gpio_unit_test(
		cmd_tbl_t *cmdtp, s32 flag,
		s32 argc, char * const argv[])
{
	struct udevice *dev;
	s32 ret;
	s32 offset;
	u32 gpio;
	char str_gpio[100];
	u32 bank_addr;

	for (ret = uclass_first_device(UCLASS_GPIO, &dev);
	     dev;
	     ret = uclass_next_device(&dev)) {

		struct tcc_gpio_bank *priv = dev_get_priv(dev);
		const char *bank_name;
		s32 num_bits;

		bank_name = gpio_get_bank_info(dev, &num_bits);
		if (!num_bits) {
			TCC_DEBUG_MSG(
				"GPIO device %s has no bits\n"
				, dev->name);
			continue;
		}

		if (memcmp(bank_name, "gpk", 3) == 0) {
			debug("gpk is not target\n");
			continue;
		}

		TCC_DEBUG_MSG(
			"start test oen, ien - %s\n"
			, bank_name);
		bank_addr = priv->gpio_base + priv->bank_base;

		for (offset = 0; offset < num_bits; offset++) {
			sprintf(str_gpio, "%s%d", bank_name, offset);
			ret = gpio_lookup_name(str_gpio, NULL, NULL, &gpio);
			if (ret) {
				pr_err("GPIO: '%s' not found\n", str_gpio);
				continue;
			}

			ret = gpio_request(gpio, "test_gpio");
			ret = gpio_get_raw_function(dev, offset, NULL);
			if (ret != GPIOF_FUNC) {
				gpio_direction_input(gpio);
				ret = reg_readl(bank_addr + TCC_GPIO_IEN,
						offset, 1);
				if (ret != 1) {
					pr_err(
						"\033[31mFailed gpio_direction_input\033[0m\n"
						);
					ret = -1;
					break;
				}

				gpio_direction_output(gpio, 1);
				ret = reg_readl(bank_addr + TCC_GPIO_OEN,
						offset, 1);
				if (ret != 1) {
					pr_err(
						"\033[31mFailed gpio_direction_input\033[0m\n"
						);
					ret = -1;
					break;
				}

				gpio_direction_input(gpio);
				ret = reg_readl(bank_addr + TCC_GPIO_IEN,
						offset, 1);
				if (ret != 1) {
					pr_err(
						"\033[31mFailed gpio_direction_input\033[0m\n"
						);
					ret = -1;
					break;
				}
			} else {
				debug(
					"\033[33mis function\033[0m\n"
					);
			}
		}
		if (ret < 0) {
			pr_err(
				"\033[31mFailed test - %s\033[0m\n"
				, str_gpio);
			ret = -1;
		} else {
			pr_err(
				"\033[32mcomplete test - %s\033[0m\n"
				, bank_name);
		}
	}

	return ret;
}

static s32 do_test_rw_all(
		cmd_tbl_t *cmdtp, s32 flag,
		s32 argc, char * const argv[])
{
	struct udevice *dev;

	uclass_foreach_dev_probe(UCLASS_PINCTRL, dev) {
			test_rw_all(dev);
	}

	return CMD_RET_SUCCESS;
}

static s32 do_test_rw(
		cmd_tbl_t *cmdtp, s32 flag,
		s32 argc, char * const argv[])
{
	struct udevice *dev;
	const char *bank_name;
	s32 pin_num = -1;
	s32 i;

	uclass_foreach_dev_probe(UCLASS_PINCTRL, dev) {
		pr_err("argc : %d\n", argc);
		for (i = 0; i < argc; i++) {
			pr_err("argv[%d] : %s\n", i, argv[i]);
		/* comment for kernel coding style */
		}

		if (argc < 2) {
			for (i = 0; i < ARRAY_SIZE(bank_list); i++) {
				test_rw(dev, bank_list[i].bank_name, -1);
			/* comment for kernel coding style */
			}
		} else {
			bank_name = argv[1];
			if (argc >= 3) {
				pin_num = simple_strtoul(argv[2], NULL, 10);
			/* comment for kernel coding style */
			}

			if (bank_name != NULL) {
				test_rw(dev, bank_name, pin_num);
			/* comment for kernel coding style */
			}
		}
	}

	print_used_pins();

	return CMD_RET_SUCCESS;
}

static s32 test_manual_input(u32 addr, s32 pin_num)
{
	u32 backup_value[3];

	backup_value[0] = test_gpio_get_input_enable(addr, pin_num);
	backup_value[1] = test_gpio_get_function(addr, pin_num);
	backup_value[2] = test_gpio_get_output_enable(addr, pin_num);

	test_gpio_set_function(addr, pin_num, 0);
	test_gpio_set_input_enable(addr, pin_num, 1);

	TCC_DEBUG_MSG("input value [%ld]\n", test_gpio_get_data(addr, pin_num));

	test_gpio_set_input_enable(addr, pin_num, backup_value[0]);
	test_gpio_set_function(addr, pin_num, backup_value[1]);
	test_gpio_set_output_enable(addr, pin_num, backup_value[2]);

	return 0;
}

static s32 do_test_manual_input(
		cmd_tbl_t *cmdtp, s32 flag,
		s32 argc, char * const argv[])
{
	u32 addr = 0;
	struct udevice *dev;
	const char *bank_name;
	s32 pin_num = -1;
	s32 i;
	struct telechips_pinctrl_priv *priv;

	if (argc < 3) {
		pr_err("require argument(bank name, pin number)\n");
		return CMD_RET_USAGE;
	}

	bank_name = argv[1];
	pin_num = simple_strtoul(argv[2], NULL, 10);

	uclass_foreach_dev_probe(UCLASS_PINCTRL, dev) {
		priv = dev_get_priv(dev);

		for (i = 0; i < priv->nbanks; i++) {
			if (strncmp(priv->bank[i].name,
					bank_name, strlen(bank_name))
					== 0) {
				addr = priv->gpio_base
					+ priv->bank[i].bank_base;
			}
		}

		if (addr == 0) {
			return CMD_RET_FAILURE;
		/* comment for kernel coding style */
		}

		test_manual_input(addr, pin_num);
	}

	print_used_pins();

	return CMD_RET_SUCCESS;
}

static s32 test_manual_output(u32 addr, s32 pin_num, s32 value)
{
	char input_temp;
	u32 read_value, expect_value, pre_value, backup_value[3];

	backup_value[0] = test_gpio_get_function(addr, pin_num);
	backup_value[1] = test_gpio_get_output_enable(addr, pin_num);
	backup_value[2] = test_gpio_get_data(addr, pin_num);

	test_gpio_set_function(addr, pin_num, 0);
	test_gpio_set_output_enable(addr, pin_num, 1);

	test_gpio_set_data(addr, pin_num, value);

	TCC_DEBUG_MSG(
			"Press any key to stop the output test and restore to the previous register.\n"
			);
	input_temp = getc();

	test_gpio_set_function(addr, pin_num, backup_value[0]);
	test_gpio_set_output_enable(addr, pin_num, backup_value[1]);
	test_gpio_set_data(addr, pin_num, backup_value[2]);

	return 0;
}

static s32 do_test_manual_output(
		cmd_tbl_t *cmdtp, s32 flag,
		s32 argc, char * const argv[])
{
	u32 addr = 0;
	struct udevice *dev;
	const char *bank_name;
	s32 pin_num = -1;
	s32 value;
	s32 i;
	struct telechips_pinctrl_priv *priv;

	if (argc < 4) {
		pr_err("require argument(bank name, pin number, value)\n");
		return CMD_RET_USAGE;
	}

	bank_name = argv[1];
	pin_num = simple_strtoul(argv[2], NULL, 10);
	value = simple_strtoul(argv[3], NULL, 10);

	uclass_foreach_dev_probe(UCLASS_PINCTRL, dev) {
		priv = dev_get_priv(dev);

		for (i = 0; i < priv->nbanks; i++) {
			if (strncmp(priv->bank[i].name,
					bank_name, strlen(bank_name))
					== 0) {
				addr = priv->gpio_base
					+ priv->bank[i].bank_base;
			}
		}

		if (addr == 0) {
			return CMD_RET_FAILURE;
		/* comment for kernel coding style */
		}

		test_manual_output(addr, pin_num, value);
	}

	return CMD_RET_SUCCESS;
}

static s32 do_test_externel(
		cmd_tbl_t *cmdtp, s32 flag,
		s32 argc, char * const argv[])
{
	struct udevice *dev;

	uclass_foreach_dev_probe(UCLASS_PINCTRL, dev) {
		test_eint_eclk(dev);
	}

	print_used_pins();

	return CMD_RET_SUCCESS;
}

static s32 do_test_eint_port_mux(
		cmd_tbl_t *cmdtp, s32 flag,
		s32 argc, char * const argv[])
{
	struct udevice *dev;
	const char *bank_name;
	s32 pin_num = -1;
	s32 pin_num_itr = 0;
	s32 itr_mode = 0;
	s32 eint_source = 0;
	s32 i;
	s32 npins = 0;
	u32 addr;
	struct telechips_pinctrl_priv *priv = NULL;
	s32 eint_sel = -1;

	if (argc == 2) {
		pr_err("iterator mode\n");
		itr_mode = 1;
	}

	if (argc < 2) {
		pr_err(
				"require argument(bank name, pin number(optional)\n"
				);
		return CMD_RET_USAGE;
	}

	bank_name = argv[1];
	if (argc >= 3) {
		pin_num = simple_strtoul(argv[2], NULL, 10);
		/* comment for kernel coding style */
	}
	if (argc >= 4) {
		eint_sel = simple_strtoul(argv[3], NULL, 10);
		/* comment for kernel coding style */
	}

	uclass_foreach_dev_probe(UCLASS_PINCTRL, dev) {
		priv = dev_get_priv(dev);
		break;
	}

	if (priv != NULL) {
		for (i = 0; i < priv->nbanks; i++) {
			pr_err("bank name : %s\n", priv->bank[i].name);
			if (strncmp(priv->bank[i].name,
					bank_name, strlen(bank_name))
					== 0) {
				addr = priv->gpio_base
					+ priv->bank[i].bank_base;
				npins = priv->bank[i].npins;
			}
		}

		for (pin_num_itr = 0; pin_num_itr < npins; pin_num_itr++) {
			if (itr_mode == 1) {
				pin_num = pin_num_itr;
				/* comment for kernel coding style */
			}

			for (i = 0; i < ARRAY_SIZE(extintr); i++) {
				if (strncmp(extintr[i].bank_name,
						bank_name, strlen(bank_name))
						== 0) {
					eint_source = i + pin_num;
					break;
				}
			}

			if (addr == 0) {
				pr_err("invalid address\n");
				return CMD_RET_FAILURE;
			}

			pr_err("============================\n");
			pr_err("Test eint GPIO %s - %d\n", bank_name, pin_num);
			pr_err("eint source number : %d\n", eint_source);
			pr_err("============================\n");

			test_eint_port_mux
				(dev, addr, pin_num, eint_source, eint_sel);

			if (itr_mode == 0) {
				break;
			/* comment for kernel coding style */
			}
		}

	}

	return CMD_RET_SUCCESS;
}

static s32 do_test_lock(
		cmd_tbl_t *cmdtp, s32 flag,
		s32 argc, char * const argv[])
{
	struct udevice *dev;
	const char *bank_name;

	if (argc >= 2) {
		bank_name = argv[1];
		/* comment for kernel coding style */
	} else {
		bank_name = NULL;
		/* comment for kernel coding style */
	}
	uclass_foreach_dev_probe(UCLASS_PINCTRL, dev) {
		test_function_lock(dev, bank_name);
		test_output_enable_lock(dev, bank_name);
	}

	return CMD_RET_SUCCESS;
}

static s32 do_test_virtual(
		cmd_tbl_t *cmdtp, s32 flag,
		s32 argc, char * const argv[])
{
	struct udevice *dev;
	const char *bank_name;

	if (argc >= 2) {
		bank_name = argv[1];
		/* comment for kernel coding style */
	} else {
		bank_name = NULL;
		/* comment for kernel coding style */
	}
	uclass_foreach_dev_probe(UCLASS_PINCTRL, dev) {
		test_output_by_tpg(dev, bank_name);
		test_input_by_tpg(dev, bank_name);
	}

	return CMD_RET_SUCCESS;
}

static s32 do_test_check_datasheet(
		cmd_tbl_t *cmdtp, s32 flag,
		s32 argc, char * const argv[])
{
	struct udevice *dev;
	struct telechips_pinctrl_priv *priv;
	s32 i;
	u32 read_value;

	TCC_DEBUG_MSG("Start check datasheet\n");

	uclass_foreach_dev_probe(UCLASS_PINCTRL, dev) {
		priv = dev_get_priv(dev);
		for (i = 0; i < ARRAY_SIZE(gpio_datasheet_register); i++) {
			read_value
				= readl(
					gpio_datasheet_register[i].offset
					+ priv->gpio_base);
			if (read_value != gpio_datasheet_register[i].value) {
				TCC_DEBUG_MSG(
					"\033[31m [0x%X] datasheet : 0x%.8X, read : 0x%.8X\033[0m\n"
					, gpio_datasheet_register[i].offset
						+ priv->gpio_base
					, gpio_datasheet_register[i].value
					, read_value);
			}
		}
	}

	TCC_DEBUG_MSG("Complete check datasheet\n");

	return CMD_RET_SUCCESS;
}

static s32 do_read_reg(
		cmd_tbl_t *cmdtp, s32 flag,
		s32 argc, char * const argv[])
{
	u32 addr = simple_strtoul(argv[1], NULL, 16);

	pr_err("0x%.8X : 0x%.8X\n", addr, readl(addr));

	return CMD_RET_SUCCESS;
}


static cmd_tbl_t gpio_subcmd[] = {
	U_BOOT_CMD_MKENT(unit_test, 3, 1, do_gpio_unit_test, "", ""),
	U_BOOT_CMD_MKENT(rw, 3, 1, do_test_rw, "", ""),
	U_BOOT_CMD_MKENT(external, 1, 1, do_test_externel, "", ""),
	U_BOOT_CMD_MKENT(lock, 2, 1, do_test_lock, "", ""),
	U_BOOT_CMD_MKENT(virtual, 2, 1, do_test_virtual, "", ""),
	U_BOOT_CMD_MKENT(output, 4, 1, do_test_manual_output, "", ""),
	U_BOOT_CMD_MKENT(input, 3, 1, do_test_manual_input, "", ""),
	U_BOOT_CMD_MKENT(datasheet, 3, 1, do_test_check_datasheet, "", ""),
	U_BOOT_CMD_MKENT(reg, 3, 1, do_read_reg, "", ""),
	U_BOOT_CMD_MKENT(ex_mux, 4, 1, do_test_eint_port_mux, "", ""),
};

static s32 do_test_gpio(cmd_tbl_t *cmdtp, s32 flag, s32 argc,
		     char * const argv[])
{
	cmd_tbl_t *cmd;

	argc--;
	argv++;

	cmd = find_cmd_tbl(argv[0], gpio_subcmd, ARRAY_SIZE(gpio_subcmd));
	if (!cmd || argc > cmd->maxargs)
		return CMD_RET_USAGE;

	return cmd->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(test_gpio, CONFIG_SYS_MAXARGS, 1, do_test_gpio,
	   "test gpio",

	   "unit_test                            - simple test to set oen and ien using gpio driver\n"
	   "rw <bank name>                       - test read/write gpio register\n"
	   "                                     - example : test_gpio rw gpa\n"
	   "external                             - test read/write extenal interrupt, clock register\n"
	   "                                     - example : test_gpio external\n"
	   "lock <bank name>                     - test output enable lock, function lock register\n"
	   "                                     - example : test_gpio lock gpa\n"
	   "output <bank name> <pin num> <value> - test gpio output manualy\n"
	   "                                     - example : test_gpio output gpa 1 1\n"
	   "input <bank name> <pin num>          - test gpio input manualy\n"
	   "                                     - example : test_gpio input gpa 1\n"
	   "datasheet                            - test gpio reset value\n"
	   "                                     - example : test_gpio datasheet\n"
	   "ex_mux <bank name> <pin num>         - external interrupt port mux test\n"
	   "                                     - example : test_gpio ex_mux gpa\n"
	   "                                     - example : test_gpio ex_mux gpsd0 1\n"
	   "virtual <bank name>                  - virtual gpio input/output test\n"
	   "                                     - example : test_gpio virtual gpa\n"
)
