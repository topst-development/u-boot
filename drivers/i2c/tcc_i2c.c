// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <asm/arch/i2c.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <clk.h>
#include <time.h>
#include <linux/io.h>
#include <mach/clock.h>

static void tcc_i2c_reg_dump(const struct tcc_i2c *i2c)
{
	(void)pr_debug("[DEBUG][I2C] PRES\t: %#x\n",
			readl(i2c->regs + I2C_PRES));
	(void)pr_debug("[DEBUG][I2C] CTRL\t: %#x\n",
			readl(i2c->regs + I2C_CTRL));
	(void)pr_debug("[DEBUG][I2C] CMD\t: %#x\n",
			readl(i2c->regs + I2C_CMD));
	(void)pr_debug("[DEBUG][I2C] SR\t: %#x\n",
			readl(i2c->regs + I2C_SR));
	(void)pr_debug("[DEBUG][I2C] TR\t: %#x\n",
			readl(i2c->regs + I2C_TR));

	if ((i2c->soc_info->id == TCC803X) ||
		(i2c->soc_info->id == TCC805X) ||
		(i2c->soc_info->id == TCC750X) ||
		(i2c->soc_info->id == TCC807X)) {
		(void)pr_debug("[DEBUG][I2C] TR1\t: %#x\n",
				readl(i2c->regs + I2C_TR1));
	}

	if ((i2c->soc_info->id == TCC897X) ||
		(i2c->soc_info->id == TCC803X) ||
		(i2c->soc_info->id == TCC805X) ||
		(i2c->soc_info->id == TCC807X)) {
		(void)pr_debug("[DEBUG][I2C] PORT_CFG0: %#x\n",
				readl(i2c->pcfg + I2C_PCFG0));
		(void)pr_debug("[DEBUG][I2C] PORT_CFG1: %#x\n",
				readl(i2c->pcfg + I2C_PCFG1));
		(void)pr_debug("[DEBUG][I2C] IRQ_STS\t: %#x\n",
				readl(i2c->pcfg + I2C_ISTS));
	}

	if ((i2c->soc_info->id == TCC803X) ||
		(i2c->soc_info->id == TCC805X) ||
		(i2c->soc_info->id == TCC807X)) {
		(void)pr_debug("[DEBUG][I2C] PORT_CFG2: %#x\n",
				readl(i2c->pcfg + I2C_PCFG2));
	}

	if (i2c->soc_info->shared_ch == i2c->channel) {
		(void)pr_debug("[DEBUG][I2C] TISR\t: %#x\n",
				readl(i2c->regs + I2C_TISR));
		(void)pr_debug("[DEBUG][I2C] TIMR\t: %#x\n",
				readl(i2c->regs + I2C_TIMR));
		(void)pr_debug("[DEBUG][I2C] TVR\t: %#x\n",
				readl(i2c->regs + I2C_TVR));
		(void)pr_debug("[DEBUG][I2C] ACR\t: %#x\n",
				readl(i2c->regs + I2C_ACR));
	}
}

static int32_t tcc_i2c_set_noise_filter(struct tcc_i2c *i2c)
{
	uint32_t tr_reg, filter_cnt, fbus_ioclk_Mhz, temp;
	ulong temp_u64;
	int32_t ret = 0;

	if ((i2c->soc_info->id == TCC803X) ||
		(i2c->soc_info->id == TCC805X) ||
		(i2c->soc_info->id == TCC750X) ||
		(i2c->soc_info->id == TCC807X)) {
		temp_u64 = clk_get_rate(&(i2c->fclk));
	} else if (i2c->soc_info->id == TCC897X) {
		temp_u64 = tcc_get_clkctrl(i2c->fclk.id);
	}

	temp_u64 /= 1000000U;
	if (temp_u64 < UINT_MAX) {
		fbus_ioclk_Mhz = (uint32_t)temp_u64;
	} else {
		ret = -EINVAL;
	}

	if (ret == 0) {
		if (fbus_ioclk_Mhz < 1U) {
			(void)pr_err("[ERROR][I2C] FBUS_IO %uMHz is not enough!\n",
				fbus_ioclk_Mhz);
			ret = -EIO;
		}
	}

	if (ret == 0) {
		if ((UINT_MAX / fbus_ioclk_Mhz) < i2c->noise_filter) {
			(void)pr_err("[ERROR][I2C] noise filter %u ns is too much\n",
					i2c->noise_filter);
			ret = -EIO;
		}
	}

	if (ret == 0) {
		temp = i2c->noise_filter * fbus_ioclk_Mhz;
		filter_cnt = (temp / 1000U) + 2U;
		if (filter_cnt > 0x1FU) {
			(void)pr_warn("[WARN][I2C] Noiser filter cnt is over than 0x1F\n");
		}

		/* set Noise filter counter load value */
		tr_reg = readl(i2c->regs + I2C_TR);
		if ((i2c->soc_info->id == TCC897X) ||
			((i2c->soc_info->id == TCC803X) && (i2c->channel < 4))) {
			tr_reg |= (uint32_t)(filter_cnt & 0x000001FU);
		} else {
			tr_reg |= (uint32_t)((filter_cnt & 0x000001FFU) << 20);
		}
		writel(tr_reg, i2c->regs + I2C_TR);

		(void)pr_debug("[DEBUG][I2C] FBUS IO %uMHz, noise filter load vaule %u\n",
				fbus_ioclk_Mhz, filter_cnt);
	}

	return ret;
}

static int32_t tcc_i2c_access_arbitration(const struct tcc_i2c *i2c, bool on)
{
	ulong tmo = 0;
	int32_t ret = 0;

	if ((i2c->soc_info->id == TCC805X) ||
		(i2c->soc_info->id == TCC750X) ||
		(i2c->soc_info->id == TCC807X)) {
		if (on == (bool)false) {
			/* clear permission */
			writel(0U, i2c->regs + I2C_ACR);
		} else {
			tmo = get_timer(0UL);
			/* Read status whether shared i2c is idle */
			while ((readl(i2c->regs + I2C_ACR) & 0xF0UL) != 0UL) {
				if (get_timer(tmo) > I2C_ACCESS_TIMEOUT) {
					(void)pr_err("[ERROR][I2C] Shared I2C is busy - timeout 0\n");
					tcc_i2c_reg_dump(i2c);
					ret = -ETIMEDOUT;
					break;
				}
			}

			if (ret == 0) {
				/* request access perimission */
				writel((u32)BIT(0), i2c->regs + I2C_ACR);

				tmo = get_timer(0UL);
				/* Check status for permission */
				while ((readl(i2c->regs + I2C_ACR) & 0x10UL) == 0U) {
					if (get_timer(tmo) > I2C_ACCESS_TIMEOUT) {
						/* clear permission */
						writel(0U, i2c->regs + I2C_ACR);
						(void)pr_err("[ERROR][I2C] Shared I2C is busy - timeout 1\n");
						tcc_i2c_reg_dump(i2c);
						ret = -ETIMEDOUT;
						break;
					}
				}
			}
		}
	}

	return ret;
}

static int32_t wait_intr(const struct tcc_i2c *i2c)
{
	ulong tmo = 0;
	uint32_t cmd_reg;
	uint32_t cnt = 0;
	int32_t ret = 0;

	if ((i2c->soc_info->id == TCC803X) ||
		(i2c->soc_info->id == TCC805X) ||
		(i2c->soc_info->id == TCC750X) ||
		(i2c->soc_info->id == TCC807X)) {
		tmo = get_timer(0UL);
		while ((readl(i2c->regs + I2C_CMD) & 0xF0UL) != 0U) {
			if (get_timer(tmo) > I2C_CMD_TIMEOUT) {
				(void)pr_err("[ERROR][I2C] i2c cmd timeout - 0 (check sclk status)\n");
				tcc_i2c_reg_dump(i2c);
				ret = -ETIMEDOUT;
				break;
			}
		}

		if (ret == 0) {
			tmo = get_timer(0UL);
			/* Check whether transfer is in progress */
			while ((readl(i2c->regs + I2C_SR) & BIT(1)) != 0U) {
				if (get_timer(tmo) > I2C_CMD_TIMEOUT) {
					(void)pr_err("[ERROR][I2C] i2c cmd timeout - 1 (check sclk status)\n");
					tcc_i2c_reg_dump(i2c);
					ret = -ETIMEDOUT;
					break;
				}
			}
		}

		if (ret == 0) {
			/* Check Arbitration Lost */
			if ((readl(i2c->regs + I2C_SR) & BIT(5)) != 0U) {
				(void)pr_err("[ERROR][I2C] arbitration lost.(check sclk, sda status)\n");
				ret = -EIO;
			} else {
				/* Clear a pending interrupt */
				cmd_reg = readl(i2c->regs + I2C_CMD);
				writel((cmd_reg | 1U), i2c->regs + I2C_CMD);
			}
		}
	} else if (i2c->soc_info->id == TCC897X) {
		while (readl(i2c->regs + I2C_CMD) & 0xF0U) {
			cnt++;
			if (cnt > 100000) {
				pr_err("[ERROR][I2C] ch %d time out!\n", i2c->channel);
				ret = -EIO;
				break;
			}
		}
	}

	return ret;
}

static int32_t tcc_i2c_message_start(const struct tcc_i2c *i2c, const struct i2c_msg *msg)
{
	uint32_t addr;
	int32_t ret = 0;

	if (msg == NULL) {
		(void)pr_err("[ERROR][I2C] %s: i2c_msg is null!\n", __func__);
		ret = -ENOMEM;
	} else {
		addr = (msg->addr & 0x7fU) << 1;

		if ((msg->flags & (u16)I2C_M_RD) != 0U) {
			addr |= 1U;
		}
		if ((msg->flags & (u16)I2C_M_REV_DIR_ADDR) != 0U) {
			addr ^= 1U;
		}
		writel(addr, i2c->regs+I2C_TXR);
		writel((I2C_CMD_STA | I2C_CMD_WR), i2c->regs+I2C_CMD);

		ret = wait_intr(i2c);
	}

	return ret;
}

static int32_t tcc_i2c_stop(const struct tcc_i2c *i2c)
{
	int32_t ret = 0;

	writel(I2C_CMD_STO, i2c->regs+I2C_CMD);
	ret = wait_intr(i2c);
	return ret;
}

static int32_t tcc_i2c_acked(const struct tcc_i2c *i2c)
{
	uint32_t temp;
	int32_t ret = 0;

	if ((i2c->msg->flags & (u16)I2C_M_IGNORE_NAK) == 0U) {
		temp = readl(i2c->regs+I2C_SR);
		if ((temp & I2C_SR_NACK) != 0U) {
			(void)pr_debug("[DEBUG][I2C] %s: NACK received\n", __func__);
			ret = -ENXIO;
		}
	}

	return ret;
}

static int32_t recv_i2c(const struct tcc_i2c *i2c)
{
	int32_t ret = 0;
	uint32_t i, temp_u32;

	ret = tcc_i2c_message_start(i2c, i2c->msg);
	if (ret != 0) {
		(void)pr_err("[ERROR][I2C] %s: failed to send slave address\n",
				__func__);
	}

	if (ret == 0) {
		ret = tcc_i2c_acked(i2c);
	}

	if (ret == 0) {
		for (i = 0; i < i2c->msg->len; i++) {
			if (i == (i2c->msg->len - 1UL)) {
				writel((I2C_CMD_RD | I2C_CMD_ACK), i2c->regs+I2C_CMD);
			} else {
				writel(I2C_CMD_RD, i2c->regs+I2C_CMD);
			}
			ret = wait_intr(i2c);
			if (ret != 0) {
				break;
			}
			temp_u32 = readl(i2c->regs+I2C_RXR) & (u32)0xFFU;
			i2c->msg->buf[i] = (u8)temp_u32;
		}
	}

	return ret;
}

static int32_t send_i2c(const struct tcc_i2c *i2c)
{
	uint32_t i;
	int32_t ret = 0;

	ret = tcc_i2c_message_start(i2c, i2c->msg);
	if (ret != 0) {
		(void)pr_err("[ERROR][I2C] %s: failed to send slave address\n",
				__func__);
	}

	if (ret == 0) {
		ret = tcc_i2c_acked(i2c);
	}

	if (ret == 0) {
		for (i = 0; i < i2c->msg->len; i++) {
			writel(i2c->msg->buf[i], i2c->regs+I2C_TXR);

			writel(I2C_CMD_WR, i2c->regs+I2C_CMD);

			ret = wait_intr(i2c);

			if (ret == 0) {
				ret = tcc_i2c_acked(i2c);
			}

			if(ret != 0) {
				break;
			}
		}
	}

	return ret;
}

static int32_t tcc_i2c_xfer(struct udevice *bus,
		struct i2c_msg *msg, int32_t nmsg)
{
	struct tcc_i2c *i2c = dev_get_priv(bus);
	int32_t i;
	int32_t ret = 0;

	if (i2c == NULL) {
		(void)pr_err("[ERROR][I2C] %s: tcc_i2c is null!\n", __func__);
		ret = -ENOMEM;
	} else {
		if (i2c->channel == i2c->soc_info->shared_ch) {
			ret = tcc_i2c_access_arbitration(i2c, true);
		}

		if (ret == 0) {
			for (i = 0; i < nmsg; i++) {
				i2c->msg        = &msg[i];
				i2c->msg->flags = msg[i].flags;
				i2c->msg_num    = nmsg;

				if ((i2c->msg->flags & (u16)I2C_M_RD) != 0U) {
					ret = recv_i2c(i2c);
					if (ret < 0) {
						(void)pr_warn("[WARN][I2C] receiving error addr 0x%x err %d\n",
								i2c->msg->addr, ret);
					}
				} else {
					ret = send_i2c(i2c);
					if (ret < 0) {
						(void)pr_warn("[WARN][I2C] sending error addr 0x%x err %d\n",
								i2c->msg->addr, ret);
					}
				}

				if (ret < 0) {
					break;
				}

				if ((i2c->msg->flags & (u16)I2C_M_STOP) != 0U) {
					(void)tcc_i2c_stop(i2c);
				}
			}
		}

		(void)tcc_i2c_stop(i2c);
		if (i2c->channel == i2c->soc_info->shared_ch) {
			(void)tcc_i2c_access_arbitration(i2c, false);
		}
	}

	return ret;
}

static int32_t tcc_i2c_probe_chip(struct udevice *bus,
		uint chip_addr, uint chip_flags)
{
	struct i2c_msg msg;
	int32_t ret = 0;

	/* configure i2c message */
	msg.addr = chip_addr;
	msg.flags = chip_flags;
	msg.len = 0;

	/* send slave address */
	ret = tcc_i2c_xfer(bus, &msg, 1);

	return ret;
}
static uint8_t tcc_i2c_get_pcfg(const struct tcc_i2c *i2c, int32_t ch)
{
	void __iomem *base = i2c->pcfg;
	uint32_t port, shift;
	uint8_t ret = 0xFFU;

	if (ch < 4) {
		base += I2C_PCFG0;
	} else if (ch < 8) {
		base += I2C_PCFG2;
	} else {
		base += I2C_PCFG1;
	}

	if ((ch >= 0) && (ch <= i2c->soc_info->last_ch + 4)) {
		ch %= 4;
		shift = (u32)ch << 3U;
		port = readl(base);

		port >>= shift;
		ret = (u8)(port & (u32)0xFFU);
	}

	return ret;
}

static void tcc_i2c_set_pcfg(const struct tcc_i2c *i2c, int32_t ch, uint8_t port)
{
	uint32_t temp_u32, shift;
	void __iomem *base = i2c->pcfg;

	if (ch < 4) {
		base += I2C_PCFG0;
	} else if (ch < 8) {
		base += I2C_PCFG2;
	} else {
		base += I2C_PCFG1;
	}

	if ((ch >= 0) && (ch <= i2c->soc_info->last_ch + 4)) {
		ch %= 4;
		shift = (u32)ch << 3U;
		temp_u32 = readl(base);

		if (shift <= (3U * (u8)BITS_PER_BYTE)) {
			temp_u32 &= ~((u32)0xFFU << shift);
			temp_u32 |= ((u32)port << shift);
			writel(temp_u32, base);
		}
	}
}

static int32_t tcc_i2c_set_port(const struct tcc_i2c *i2c)
{
	int32_t ch = i2c->channel;
	uint8_t port = (u8)(i2c->port & 0xFFU);
	uint8_t conflict;
	int32_t i, ret = 0;

	/* check channel and port */
	if (ch < 0) {
		(void)pr_err("[ERROR][I2C] %s: channel(%d), port(%u) is wrong number !\n",
				__func__, ch, port);
		ret = -EINVAL;
	} else {
		tcc_i2c_set_pcfg(i2c, ch, port);

		/* clear conflict for each i2c master/slave core */
		for (i = 0; i <= i2c->soc_info->last_ch + 4; i++) {
			if (i == ch) {
				continue;
			}
			conflict = tcc_i2c_get_pcfg(i2c, i);
			if (port == conflict) {
				tcc_i2c_set_pcfg(i2c, i, (u8)0xFFU);
			}
		}
	}

	return ret;
}

static int32_t tcc_i2c_set_clk(struct tcc_i2c *i2c)
{
	ulong peri_clk = 4000000UL; /* 4MHz */
	ulong io_clk, clk_ret;
	uint32_t prescale;
	int32_t ret = 0;

	if (i2c == NULL) {
		(void)pr_err("[ERROR][I2C] %s: tcc_i2c is null!\n", __func__);
		ret = -ENOMEM;
	}

	if (ret == 0) {
		/* I2C peripheral clock sets 6MHz */
		if ((i2c->soc_info->id == TCC803X) ||
			(i2c->soc_info->id == TCC805X) ||
			(i2c->soc_info->id == TCC750X) ||
			(i2c->soc_info->id == TCC807X)) {
			clk_ret = clk_set_rate(&i2c->pclk, peri_clk);
		} else if (i2c->soc_info->id == TCC897X) {
			clk_ret = tcc_set_peri(i2c->pclk.id, CKC_ENABLE, peri_clk, 0);
		}

		if (clk_ret != 0U) {
			(void)pr_err("[ERROR][I2C] %s: failed to set peripheral clock\n",
			__func__);
			ret = -EIO;
		}
	}

	if (ret == 0) {
		/* calculate prescale */
		if ((i2c->soc_info->id == TCC803X) ||
			(i2c->soc_info->id == TCC805X) ||
			(i2c->soc_info->id == TCC750X) ||
			(i2c->soc_info->id == TCC807X)) {
			peri_clk = clk_get_rate(&(i2c->pclk));
		} else if (i2c->soc_info->id == TCC897X) {
			peri_clk = tcc_get_peri(i2c->pclk.id);
		}

		if (peri_clk <= 0xFFFFFFFFU) {
			prescale = ((u32)peri_clk / (i2c->speed * (i2c->pwh + i2c->pwl)));
			if (prescale < 1UL) {
				(void)pr_err("[ERROR][I2C] %s: prescale %u is not acceptable.\n",
						__func__, prescale);
				ret = -EINVAL;
			}
		} else {
			(void)pr_err("[ERROR][I2C] %s: Invalid peri_clk: %lu.\n",
					__func__, peri_clk);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		prescale -= 1U;
		writel(prescale, i2c->regs + I2C_PRES);

		/* get io bus clock */
		if ((i2c->soc_info->id == TCC803X) ||
			(i2c->soc_info->id == TCC805X) ||
			(i2c->soc_info->id == TCC750X) ||
			(i2c->soc_info->id == TCC807X)) {
			io_clk = clk_get_rate(&(i2c->fclk));
		} else if (i2c->soc_info->id == TCC897X) {
			io_clk = tcc_get_clkctrl(i2c->fclk.id);
		}

		if (io_clk < (4UL * peri_clk)) {
			(void)pr_err("[ERROR][I2C] %s: not enough IO BUS clock\n", __func__);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		(void)pr_debug("[DEBUG][I2C] %s: speed %u, prescale %u, peri_clk %lu, io clk: %lu\n",
				__func__, i2c->speed, prescale, peri_clk, io_clk);

	}

	return ret;
}

static int32_t tcc_i2c_set_bus_speed(struct udevice *dev, uint32_t speed)
{
	struct tcc_i2c *i2c = dev_get_priv(dev);
	int32_t ret = 0;

	if (i2c == NULL) {
		(void)pr_err("[ERROR][I2C] %s: tcc_i2c is null!\n", __func__);
		ret = -ENOMEM;
	}

	if (ret == 0) {
		if (speed > 400000UL) {
			(void)pr_err("[ERROR][I2C] %s: speed %uHz is not supported\n",
					__func__, speed);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		i2c->speed = speed;
		ret = tcc_i2c_set_clk(i2c);
	}

	return ret;
}

static int tcc_i2c_get_bus_speed(struct udevice *bus)
{
	struct tcc_i2c *i2c = dev_get_priv(bus);

	return i2c->speed;
}

static int32_t tcc_i2c_init(struct tcc_i2c *i2c)
{
	int32_t ret = 0;

	if ((i2c->soc_info->id == TCC803X) ||
		(i2c->soc_info->id == TCC805X) ||
		(i2c->soc_info->id == TCC807X)) {
		/* swreset i2c iobus */
		(void)tcc_set_iobus_swreset(i2c->hclk.id, (bool)true);
		(void)tcc_set_iobus_swreset(i2c->hclk.id, (bool)false);

		/* enable i2c iobus */
		(void)tcc_set_iobus_pwdn(i2c->hclk.id, (bool)false);
	}

	/* To Read/Write Register, last i2c channel checks arbitration */
	if (i2c->channel == i2c->soc_info->shared_ch) {
		ret = tcc_i2c_access_arbitration(i2c, true);
	}

	if (ret == 0) {
		/* configure pwh, pwl */
		if (i2c->use_pw) {
			writel(((i2c->pwh << 16) | i2c->pwl), i2c->regs + I2C_TR1);
		}

		/* set clock */
		ret = tcc_i2c_set_clk(i2c);
	}

	if (ret == 0) {
		ret = tcc_i2c_set_noise_filter(i2c);
	}

	if (ret == 0) {
		if ((i2c->soc_info->id == TCC897X) ||
			(i2c->soc_info->id == TCC803X) ||
			(i2c->soc_info->id == TCC805X) ||
			(i2c->soc_info->id == TCC807X)) {
				/* port configruation */
				ret = tcc_i2c_set_port(i2c);
				if (ret < 0) {
					(void)pr_err("[ERROR][I2C] %s: failed to set port\n", __func__);
				}
		}
	}

	if (ret == 0) {
		/* enable i2c core */
		writel(I2C_CTRL_EN, i2c->regs + I2C_CTRL);
	}

	if (i2c->channel == i2c->soc_info->shared_ch) {
		(void)tcc_i2c_access_arbitration(i2c, false);
	}

	return ret;
}

static int32_t tcc_i2c_parse_dt(struct udevice *dev)
{
	struct tcc_i2c *i2c = dev_get_priv(dev);
	int32_t ret = 0;
	int32_t temp_s32;

	if (i2c == NULL) {
		(void)pr_err("[ERROR][I2C] %s: tcc_i2c is null!\n", __func__);
		ret = -ENOMEM;
	}

	if (ret == 0) {
		/* base, port configuration address */
		i2c->regs = (void *)dev_read_addr_index(dev, 0);

		if ((i2c->soc_info->id == TCC897X) ||
			(i2c->soc_info->id == TCC803X) ||
			(i2c->soc_info->id == TCC805X) ||
			(i2c->soc_info->id == TCC807X)) {
			i2c->pcfg = (void *)dev_read_addr_index(dev, 1);
		}

		/* channel */
		ret = dev_read_alias_seq(dev, &i2c->channel);
		if (ret < 0) {
			(void)pr_err("[ERROR][I2C] %s: failed to get channel.\n", __func__);
		} else {
			ret = 0;
		}
	}

	if (ret == 0) {
		if (i2c->channel < 0) {
			(void)pr_err("[ERROR][I2C] %s: channel %d is wrong.\n",
					__func__, i2c->channel);
			ret = -EINVAL;
		}
	}

	if (ret == 0) {
		/* get clock info */
		ret = tcc_clk_get_by_index(dev, 0, &i2c->pclk);
		if (ret < 0) {
			(void)pr_err("[ERROR][I2C] failed to get peripheral clock\n");
		}
	}

	if (ret == 0) {
		if ((i2c->soc_info->id == TCC803X) ||
			(i2c->soc_info->id == TCC805X) ||
			(i2c->soc_info->id == TCC807X)) {
			ret = tcc_clk_get_by_index(dev, 1, &i2c->hclk);
			if (ret < 0) {
				(void)pr_err("[ERROR][I2C] failed to get iobus clock\n");
			} else {
				ret = tcc_clk_get_by_index(dev, 2, &i2c->fclk);
				if (ret < 0) {
					(void)pr_err("[ERROR][I2C] failed to get fbus io clock\n");
				}
			}
		} else if ((i2c->soc_info->id == TCC897X) ||
				(i2c->soc_info->id == TCC750X)) {
			ret = tcc_clk_get_by_index(dev, 1, &i2c->fclk);
			if (ret < 0) {
				(void)pr_err("[ERROR][I2C] failed to get fbus io clock\n");
			}
	    }
	}

	if (ret == 0) {
		if ((i2c->soc_info->id == TCC897X) ||
			(i2c->soc_info->id == TCC803X) ||
			(i2c->soc_info->id == TCC805X) ||
			(i2c->soc_info->id == TCC807X)) {
			/* port number */
			ret = dev_read_u32(dev, "port-mux", &i2c->port);
			if (ret < 0) {
				(void)pr_err("[ERROR][I2C] failed to get port number\n");
			}
		}
	}

	if (ret == 0) {
		temp_s32 = dev_read_u32_default(dev,
				"clock-frequency", 400000);
		if ((temp_s32 >= 0) && (temp_s32 <= 400000)) {
			i2c->speed = (uint32_t)temp_s32;
		} else {
			i2c->speed = 400000U;
		}

		temp_s32 = dev_read_u32_default(dev,
				"noise_filter", 50);
		if (temp_s32 < 50) {
			(void)pr_warn("[WARN][I2C] I2C must suppress noise of less than 50ns.\n");
			i2c->noise_filter = 50U;
		} else {
			i2c->noise_filter = (uint32_t)temp_s32;
		}

		/* pwh, pwl */
		temp_s32 = dev_read_u32_default(dev, "pulse-width-high", 2);
		if (temp_s32 >= 0) {
			i2c->pwh = (uint32_t)temp_s32;
		} else {
			i2c->pwh = 2;
		}

		temp_s32 = dev_read_u32_default(dev, "pulse-width-low", 3);
		if (temp_s32 >= 0) {
			i2c->pwl = (uint32_t)temp_s32;
		} else {
			i2c->pwl = 3;
		}

		i2c->use_pw = (bool)true;

		if ((i2c->soc_info->id == TCC897X) ||
			((i2c->soc_info->id == TCC803X) && (i2c->channel < 4))) {
			(void)pr_debug("[DEBUG][I2C] Not support pwh, pwl function\n");
			i2c->pwh = 2;
			i2c->pwl = 3;
			i2c->use_pw = (bool)false;
		}
	}

	return ret;
}

static int32_t tcc_i2c_probe(struct udevice *dev)
{
	struct tcc_i2c *i2c = dev_get_priv(dev);
	int32_t ret = 0;

	i2c->soc_info = (const struct tcc_i2c_soc_info*)dev_get_driver_data(dev);

	ret = tcc_i2c_parse_dt(dev);
	if (ret < 0) {
		(void)pr_err("[ERROR][I2C] failed to parsing device tree.\n");
	} else {
		/* initialize i2c */
		ret = tcc_i2c_init(i2c);
		if (ret < 0) {
			(void)pr_err("[ERROR][I2C] %s: failed to initialize i2c\n", __func__);
		}
	}

	return ret;
}

static const struct tcc_i2c_soc_info tcc897x_soc_info = {
	.id = TCC897X,
	.last_ch = 3,
	.shared_ch = -1,
};

static const struct tcc_i2c_soc_info tcc803x_soc_info = {
	.id = TCC803X,
	.last_ch = 7,
	.shared_ch = -1,
};

static const struct tcc_i2c_soc_info tcc805x_soc_info = {
	.id = TCC805X,
	.last_ch = 7,
	.shared_ch = 7,
};

static const struct tcc_i2c_soc_info tcc750x_soc_info = {
	.id = TCC750X,
	.last_ch = 2,
	.shared_ch = 2,
};

static const struct tcc_i2c_soc_info tcc807x_soc_info = {
	.id = TCC807X,
	.last_ch = 7,
	.shared_ch = 7,
};

static const struct dm_i2c_ops tcc_i2c_ops = {
	.xfer          = tcc_i2c_xfer,
	.probe_chip    = tcc_i2c_probe_chip,
	.set_bus_speed = tcc_i2c_set_bus_speed,
	.get_bus_speed = tcc_i2c_get_bus_speed,
};

static const struct udevice_id tcc_i2c_ids[] = {
	{	.compatible = "telechips,tcc897x-i2c",
		.data = (ulong)&tcc897x_soc_info},
	{	.compatible = "telechips,tcc803x-i2c",
		.data = (ulong)&tcc803x_soc_info},
	{	.compatible = "telechips,tcc805x-i2c",
		.data = (ulong)&tcc805x_soc_info},
	{	.compatible = "telechips,tcc750x-i2c",
		.data = (ulong)&tcc750x_soc_info},
	{	.compatible = "telechips,tcc807x-i2c",
		.data = (ulong)&tcc807x_soc_info},
	{ }
};

U_BOOT_DRIVER(tcc_i2c) = {
	.name     = "tcc-i2c",
	.id       = UCLASS_I2C,
	.of_match = tcc_i2c_ids,
	.probe    = tcc_i2c_probe,
	.priv_auto = (int)sizeof(struct tcc_i2c),
	.ops      = &tcc_i2c_ops,
};

