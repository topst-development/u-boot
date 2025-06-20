/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_I2C_H
#define TCC_I2C_H

#include <clk.h>
#include <linux/compat.h>
#include <linux/types.h>

/*
 * I2C Registers
 */
#define I2C_PRES 	((u32)0x00)
#define I2C_CTRL 	((u32)0x04)
#define I2C_TXR  	((u32)0x08)
#define I2C_CMD 	((u32)0x0C)
#define I2C_RXR 	((u32)0x10)
#define I2C_SR  	((u32)0x14)
#define I2C_TR   	((u32)0x18)
#define I2C_TR1		((u32)0x24)

/*
 * I2C Controller 3 Arbitration Registers
 */
#define I2C_TISR	((u32)0xF0)
#define I2C_TIMR	((u32)0xF4)
#define I2C_TVR 	((u32)0xF8)
#define I2C_ACR		((u32)0xFC)

/*
 * I2C Port Configuration Registers
 */
#define	I2C_PCFG0	((u32)0x00)
#define	I2C_PCFG1	((u32)0x04)
#define	I2C_PCFG2	((u32)0x08)
#define	I2C_ISTS	((u32)0x10)

/* I2C CTRL Registers */
#define I2C_CTRL_EN	((u32)BIT(7))

/* I2C CMD Registers */
#define I2C_CMD_STA	((u32)BIT(7))
#define I2C_CMD_STO	((u32)BIT(6))
#define I2C_CMD_RD	((u32)BIT(5))
#define I2C_CMD_WR	((u32)BIT(4))
#define I2C_CMD_ACK	((u32)BIT(3))

/* I2C SR Registers */
#define I2C_SR_NACK	((u32)BIT(7))

/* read/write bit */
#define I2C_WR	0
#define I2C_RD	1

/* TIMEOUT */
#define I2C_CMD_TIMEOUT		50UL  /* ms */
#define I2C_ACCESS_TIMEOUT 	500UL /* ms */

#define TCC897X ((u8)0U)
#define TCC803X ((u8)1U)
#define TCC805X ((u8)2U)
#define TCC750X ((u8)3U)
#define TCC807X ((u8)4U)

/*
 * Information about I2C controller
 */
struct tcc_i2c {
	spinlock_t	lock;

	void __iomem    *regs;			/* base address */
	void __iomem    *pcfg;			/* port configuration address */
	struct i2c_msg  *msg;
	int32_t	        msg_num;

	int             channel;		/* channel */
	struct clk      fclk;			/* FBUS_IO */
	struct clk      hclk;			/* IOBUS */
	struct clk      pclk;			/* Peri. Clock */
	uint32_t        speed;			/* speed */
	uint32_t        port;			/* port number */

	bool            use_pw;
	uint32_t        pwh;
	uint32_t        pwl;
	uint32_t        noise_filter;
	uint32_t        sda_mfio;
	uint32_t        scl_mfio;
	const struct tcc_i2c_soc_info *soc_info;
};

struct tcc_i2c_soc_info {
	uint8_t id;
	uint8_t last_ch;
	int8_t  shared_ch;
};

#endif
