/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) Telechips Inc.
 */

#ifndef TCC_I2C_H
#define TCC_I2C_H
#include <asm/telechips/tcc_ckc.h>

/*
 * I2C Registers
 */
#define I2C_PRES 0x00
#define I2C_CTRL 0x04
#define I2C_TXR  0x08
#define I2C_CMD  0x0C
#define I2C_RXR  0x10
#define I2C_SR   0x14
#define I2C_TR   0x18
#define I2C_TR1  0x24

/*
 * I2C Controller 7 Arbitration Registers
 */
#define I2C_TISR 0xF0
#define I2C_TIMR 0xF4
#define I2C_TVR  0xF8
#define I2C_ACR  0xFC

/*
 * I2C Port Configuration Registers
 */
#define	I2C_PCFG0	0x00
#define	I2C_PCFG1	0x04
#define	I2C_PCFG2	0x08
#define	I2C_ISTS	0x10

/* I2C CTRL Registers */
#define I2C_CTRL_EN		BIT(7)

/* I2C CMD Registers */
#define I2C_CMD_STA		BIT(7)
#define I2C_CMD_STO		BIT(6)
#define I2C_CMD_RD		BIT(5)
#define I2C_CMD_WR		BIT(4)
#define I2C_CMD_ACK		BIT(3)

/* I2C SR Registers */
#define I2C_SR_NACK		BIT(7)


/* read/write bit */
#define I2C_WR	0
#define I2C_RD	1

/* TIMEOUT */
#define I2C_CMD_TIMEOUT 50UL /* ms */
#define I2C_ACCESS_TIMEOUT 500UL /* ms */

#define TCC_I2C_CH0 0
#define TCC_I2C_CH1 1
#define TCC_I2C_CH2 2
#define TCC_I2C_CH3 3
#define TCC_I2C_CH4 4
#define TCC_I2C_CH5 5
#define TCC_I2C_CH6 6
#define TCC_I2C_CH7 7
#define TCC_I2C_CH_NUM 8

/*
 * Information about I2C controller
 */
struct tcc_i2c {
	spinlock_t	lock;

	void __iomem	*regs;			/* base address */
	void __iomem	*pcfg;			/* port configuration address */
	struct i2c_msg	*msg;
	int32_t		msg_num;

	ulong          version;			/* I2C Controller Version */
	int            channel;			/* channel */
	struct clk     fclk;			/* FBUS_IO */
	struct clk     hclk;			/* IOBUS */
	struct clk     pclk;			/* Peri. Clock */
	uint32_t   speed;			/* speed */
	uint32_t   port;			/* port number */

	bool       use_pw;
	uint32_t   pwh;
	uint32_t   pwl;
	uint32_t   noise_filter;
};

#endif
