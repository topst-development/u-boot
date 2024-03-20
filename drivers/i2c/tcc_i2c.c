// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <clk.h>
#include <time.h>
#include <linux/io.h>
#include <asm/arch/clock.h>
#include <asm/telechips/i2c.h>

#define TCC_I2C_VER1 1UL
#define TCC_I2C_VER2 2UL

static void tcc_i2c_reg_dump(struct tcc_i2c *i2c)
{
	pr_debug("[DEBUG][I2C] PRES\t: %#x\n", readl(i2c->regs + I2C_PRES));
	pr_debug("[DEBUG][I2C] CTRL\t: %#x\n", readl(i2c->regs + I2C_CTRL));
	pr_debug("[DEBUG][I2C] CMD\t: %#x\n", readl(i2c->regs + I2C_CMD));
	pr_debug("[DEBUG][I2C] SR\t: %#x\n", readl(i2c->regs + I2C_SR));
	pr_debug("[DEBUG][I2C] TR\t: %#x\n", readl(i2c->regs + I2C_TR));
	pr_debug("[DEBUG][I2C] TR1\t: %#x\n", readl(i2c->regs + I2C_TR1));
	pr_debug("[DEBUG][I2C] PORT_CFG0: %#x\n", readl(i2c->pcfg + I2C_PCFG0));
	pr_debug("[DEBUG][I2C] PORT_CFG2: %#x\n", readl(i2c->pcfg + I2C_PCFG2));
	pr_debug("[DEBUG][I2C] IRQ_STS\t: %#x\n", readl(i2c->pcfg + I2C_ISTS));

	if ((i2c->version != TCC_I2C_VER1) && (i2c->channel == TCC_I2C_CH7)) {
		pr_debug("[DEBUG][I2C] TISR\t: %#x\n",
				readl(i2c->regs + I2C_TISR));
		pr_debug("[DEBUG][I2C] TIMR\t: %#x\n",
				readl(i2c->regs + I2C_TIMR));
		pr_debug("[DEBUG][I2C] TVR\t: %#x\n",
				readl(i2c->regs + I2C_TVR));
		pr_debug("[DEBUG][I2C] ACR\t: %#x\n",
				readl(i2c->regs + I2C_ACR));
	}
}

static int32_t tcc_i2c_set_noise_filter(struct tcc_i2c *i2c)
{
	uint32_t tr_reg;
	uint32_t filter_cnt;
	uint32_t fbus_ioclk_Mhz;
	uint32_t temp;

	fbus_ioclk_Mhz = (uint32_t)(clk_get_rate(&(i2c->fclk)) / 1000000U);
	if (fbus_ioclk_Mhz < 1U) {
		pr_err("[ERROR][I2C] FBUS_IO %dMHz is not enough!\n",
				fbus_ioclk_Mhz);
		return -EIO;
	}
	if ((UINT_MAX / fbus_ioclk_Mhz) < i2c->noise_filter) {
		pr_err("[ERROR][I2C] noise filter %dns is too much\n",
				i2c->noise_filter);
		return -EIO;
	}
	temp = i2c->noise_filter * fbus_ioclk_Mhz;
	filter_cnt = (temp / 1000U) + 2U;
	if (filter_cnt > 0x1FU) {
		pr_warn("[WARN][I2C] Noiser filter cnt is over than 0x1F\n");
	}

	/* set Noise filter counter load value */
	tr_reg = readl(i2c->regs + I2C_TR);
	if ((i2c->version == TCC_I2C_VER1) && (i2c->channel < 4)) {
		tr_reg |= (uint32_t)(filter_cnt & 0x000001FU);
	} else {
		tr_reg |= (uint32_t)((filter_cnt & 0x000001FFU) << 20);
	}
	writel(tr_reg, i2c->regs + I2C_TR);

	pr_debug("[DEBUG][I2C] FBUS IO %dMHz, noise filter load vaule %d\n",
			fbus_ioclk_Mhz, filter_cnt);
	return 0;
}

static int32_t tcc_i2c_access_arbitration(struct tcc_i2c *i2c, bool on)
{
	ulong tmo = 0;

	if (i2c->version == TCC_I2C_VER1) {
		return 0;
	}

	if (!on) {
		/* clear permission */
		writel(0U, i2c->regs+I2C_ACR);
		return 0;
	}

	tmo = get_timer(0UL);
	/* Read status whether i2c 7 is idle */
	while ((readl(i2c->regs + I2C_ACR) & 0xF0UL) != 0UL) {
		if (get_timer(tmo) > I2C_ACCESS_TIMEOUT) {
			pr_err("[ERROR][I2C] I2C bus 7 is busy - timeout 0\n");
			tcc_i2c_reg_dump(i2c);
			return -ETIMEDOUT;
		}
	}

	/* request access perimission */
	writel(BIT(0), i2c->regs + I2C_ACR);

	tmo = get_timer(0UL);
	/* Check status for permission */
	while ((readl(i2c->regs + I2C_ACR) & 0x10UL) == 0U) {
		if (get_timer(tmo) > I2C_ACCESS_TIMEOUT) {
			/* clear permission */
			writel(0U, i2c->regs+I2C_ACR);
			pr_err("[ERROR][I2C] I2C bus 7 is busy - timeout 1\n");
			tcc_i2c_reg_dump(i2c);
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int32_t wait_intr(struct tcc_i2c *i2c)
{
	ulong tmo = 0;
	uint32_t cmd_reg;

	tmo = get_timer(0UL);
	while ((readl(i2c->regs + I2C_CMD) & 0xF0UL) != 0U) {
		if (get_timer(tmo) > I2C_CMD_TIMEOUT) {
			pr_err("[ERROR][I2C] i2c cmd timeout - 0 (check sclk status)\n");
			tcc_i2c_reg_dump(i2c);
			return -ETIMEDOUT;
		}
	}

	tmo = get_timer(0UL);
	/* Check whether transfer is in progress */
	while ((readl(i2c->regs + I2C_SR) & BIT(1)) != 0U) {
		if (get_timer(tmo) > I2C_CMD_TIMEOUT) {
			pr_err("[ERROR][I2C] i2c cmd timeout - 1 (check sclk status)\n");
			tcc_i2c_reg_dump(i2c);
			return -ETIMEDOUT;
		}
	}
	/* Check Arbitration Lost */
	if ((readl(i2c->regs + I2C_SR) & BIT(5)) != 0UL) {
		pr_err("[ERROR][I2C] arbitration lost.(check sclk, sda status)\n");
		return -EIO;
	}

	/* Clear a pending interrupt */
	cmd_reg = readl(i2c->regs+I2C_CMD);
	writel((cmd_reg | 1U), i2c->regs+I2C_CMD);

	return 0;
}

static int32_t tcc_i2c_message_start(struct tcc_i2c *i2c, struct i2c_msg *msg)
{
	uint32_t addr;

	if (msg == NULL) {
		pr_err("[ERROR][I2C] %s: i2c_msg is null!\n", __func__);
		return -ENOMEM;
	}
	addr = (msg->addr & 0x7fU) << 1;

	if ((msg->flags & (u16)I2C_M_RD) != 0U) {
		addr |= 1U;
	}
	if ((msg->flags & (u16)I2C_M_REV_DIR_ADDR) != 0U) {
		addr ^= 1U;
	}
	writel(addr, i2c->regs+I2C_TXR);
	writel((I2C_CMD_STA | I2C_CMD_WR), i2c->regs+I2C_CMD);

	return wait_intr(i2c);
}

static int32_t tcc_i2c_stop(struct tcc_i2c *i2c)
{
	int32_t ret = 0;

	writel(I2C_CMD_STO, i2c->regs+I2C_CMD);
	ret = wait_intr(i2c);
	return ret;
}

static int32_t tcc_i2c_acked(struct tcc_i2c *i2c)
{
	uint32_t temp;

	if ((i2c->msg->flags & (u16)I2C_M_IGNORE_NAK) != 0U) {
		return 0;
	}

	temp = readl(i2c->regs+I2C_SR);
	if ((temp & I2C_SR_NACK) != 0U) {
		pr_debug("[DEBUG][I2C] %s: NACK received\n", __func__);
		return -ENXIO;
	}

	return 0;
}

static int32_t recv_i2c(struct tcc_i2c *i2c)
{
	int32_t ret;
	uint32_t i;

	ret = tcc_i2c_message_start(i2c, i2c->msg);
	if (ret != 0) {
		pr_err("[ERROR][I2C] %s: failed to send slave address\n",
				__func__);
		return ret;
	}

	ret = tcc_i2c_acked(i2c);
	if (ret != 0) {
		return ret;
	}

	for (i = 0; i < i2c->msg->len; i++) {
		if (i == (i2c->msg->len - 1UL)) {
			writel((I2C_CMD_RD | I2C_CMD_ACK), i2c->regs+I2C_CMD);
		} else {
			writel(I2C_CMD_RD, i2c->regs+I2C_CMD);
		}
		ret = wait_intr(i2c);
		if (ret != 0) {
			return ret;
		}
		i2c->msg->buf[i] = (uchar)readl(i2c->regs+I2C_RXR);
	}
	return 0;
}

static int32_t send_i2c(struct tcc_i2c *i2c)
{
	uint32_t i;
	int32_t ret;

	ret = tcc_i2c_message_start(i2c, i2c->msg);
	if (ret != 0) {
		pr_err("[ERROR][I2C] %s: failed to send slave address\n",
				__func__);
		return ret;
	}

	ret = tcc_i2c_acked(i2c);
	if (ret != 0) {
		return ret;
	}

	for (i = 0; i < i2c->msg->len; i++) {
		writel(i2c->msg->buf[i], i2c->regs+I2C_TXR);

		writel(I2C_CMD_WR, i2c->regs+I2C_CMD);

		ret = wait_intr(i2c);
		if (ret != 0) {
			return ret;
		}

		ret = tcc_i2c_acked(i2c);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

static int32_t tcc_i2c_xfer(struct udevice *bus,
		struct i2c_msg *msg, int32_t nmsg)
{
	struct tcc_i2c *i2c = dev_get_priv(bus);
	int32_t i;
	int32_t ret = 0;

	if (i2c == NULL) {
		pr_err("[ERROR][I2C] %s: tcc_i2c is null!\n", __func__);
		return -ENOMEM;
	}

	if (i2c->channel == 7) {
		ret = tcc_i2c_access_arbitration(i2c, true);
		if (ret < 0) {
			return ret;
		}
	}

	for (i = 0; i < nmsg; i++) {
		i2c->msg        = &msg[i];
		i2c->msg->flags = msg[i].flags;
		i2c->msg_num    = nmsg;

		if ((i2c->msg->flags & (u16)I2C_M_RD) != 0U) {
			ret = recv_i2c(i2c);
			if (ret < 0) {
				pr_warn("[WARN][I2C] receiving error addr 0x%x err %d\n",
						i2c->msg->addr, ret);
				goto fail;
			}
		} else {
			ret = send_i2c(i2c);
			if (ret < 0) {
				pr_warn("[WARN][I2C] sending error addr 0x%x err %d\n",
						i2c->msg->addr, ret);
				goto fail;
			}
		}

		if ((i2c->msg->flags & (u16)I2C_M_STOP) != 0U) {
			(void)tcc_i2c_stop(i2c);
		}
	}

fail:
	(void)tcc_i2c_stop(i2c);
	if (i2c->channel == 7) {
		(void)tcc_i2c_access_arbitration(i2c, false);
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
	if (ret < 0) {
		return ret;
	}

	return 0;
}
static uint8_t tcc_i2c_get_pcfg(void __iomem *base, int32_t ch)
{
	uint32_t res, shift;

	if (ch < 4) {
		base += I2C_PCFG0;
	} else {
		base += I2C_PCFG2;
		ch -= 4;
	}

	res = readl(base);
	shift = ch << 3;
	res >>= shift;

	return (res & 0xFFU);
}

static void tcc_i2c_set_pcfg(void __iomem *base, int32_t ch, uint32_t port)
{
	uint32_t val, shift;

	if (ch < 4) {
		base += I2C_PCFG0;
	} else {
		base += I2C_PCFG2;
		ch -= 4;
	}

	val = readl(base);
	shift = ch << 3;
	val &= ~(0xFFU << shift);
	val |= (port << shift);
	writel(val, base);
}

static int32_t tcc_i2c_set_port(struct tcc_i2c *i2c)
{
	int32_t ch = i2c->channel;
	uint32_t port = i2c->port;
	int32_t conflict;
	int32_t i;

	/* check channel and port */
	if (ch < 0) {
		pr_err("[ERROR][I2C] %s: channel(%d), port(%d) is wrong number !\n",
				__func__, ch, port);
		return -EINVAL;
	}

	tcc_i2c_set_pcfg(i2c->pcfg, ch, port);

	/* clear conflict for each i2c master core */
	for (i = 0; i < TCC_I2C_CH_NUM; i++) {
		if (i == ch)
			continue;
		conflict = tcc_i2c_get_pcfg(i2c->pcfg, i);
		if (port == conflict)
			tcc_i2c_set_pcfg(i2c->pcfg, i, 0xff);
	}

	/* clear conflict for each i2c slave core */
	for (i = 0; i < 4; i++) {
		conflict = tcc_i2c_get_pcfg(i2c->pcfg + I2C_PCFG1, i);
		if (port == conflict)
			tcc_i2c_set_pcfg(i2c->pcfg + I2C_PCFG1, i, 0xff);
	}

	return 0;
}

static int32_t tcc_i2c_set_clk(struct tcc_i2c *i2c)
{
	ulong peri_clk = 6000000UL; /* 6MHz */
	ulong io_clk;
	ulong ret;
	uint32_t prescale;

	if (i2c == NULL) {
		pr_err("[ERROR][I2C] %s: tcc_i2c is null!\n", __func__);
		return -ENOMEM;
	}

	/* I2C peripheral clock sets 6MHz */
	ret = clk_set_rate(&i2c->pclk, peri_clk);
	if (ret != 0UL) {
		pr_err("[ERROR][I2C] %s: failed to set peripheral clock\n",
				__func__);
		return -EIO;
	}

	/* calculate prescale */
	peri_clk = clk_get_rate(&(i2c->pclk));
	prescale = ((u32)peri_clk / (i2c->speed * (i2c->pwh + i2c->pwl)));
	if (prescale < 1UL) {
		pr_err("[ERROR][I2C] %s: prescale %d is not acceptable.\n",
				__func__, prescale);
		return -EINVAL;
	}
	prescale -= 1U;
	writel(prescale, i2c->regs + I2C_PRES);

	/* get io bus clock */
	io_clk = clk_get_rate(&(i2c->fclk));
	if (io_clk < (4UL * peri_clk)) {
		pr_err("[ERROR][I2C] %s: not enough IO BUS clock\n", __func__);
		return -EINVAL;
	}

	pr_debug("[DEBUG][I2C] %s: speed %d, prescale %d, peri_clk %ld, io clk: %ld\n",
			__func__, i2c->speed, prescale, peri_clk, io_clk);

	return 0;
}

static int32_t tcc_i2c_set_bus_speed(struct udevice *dev, uint32_t speed)
{
	struct tcc_i2c *i2c = dev_get_priv(dev);
	int32_t ret;

	if (i2c == NULL) {
		pr_err("[ERROR][I2C] %s: tcc_i2c is null!\n", __func__);
		return -ENOMEM;
	}

	if (speed > 400000UL) {
		pr_err("[ERROR][I2C] %s: speed %dHz is not supported\n",
				__func__, speed);
		return -EINVAL;
	}

	i2c->speed = speed;
	ret = tcc_i2c_set_clk(i2c);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int32_t tcc_i2c_init(struct tcc_i2c *i2c)
{
	int32_t ret;
	int32_t error_status = 0;

	/* swreset i2c iobus */
	(void)tcc_set_iobus_swreset((int32_t)(i2c->hclk.id), true);
	(void)tcc_set_iobus_swreset((int32_t)(i2c->hclk.id), false);

	/* enable i2c iobus */
	ret = tcc_set_iobus_pwdn((int32_t)(i2c->hclk.id), false);
	if (ret < 0) {
		pr_err("[ERROR][I2C] %s: iobus not enabled\n", __func__);
	}

	/* To Read/Write Register, i2c7 checks arbitration */
	if (i2c->channel == 7) {
		ret = tcc_i2c_access_arbitration(i2c, true);
		if (ret < 0) {
			return ret;
		}
	}

	/* configure pwh, pwl */
	if (i2c->use_pw) {
		writel(((i2c->pwh << 16) | i2c->pwl), i2c->regs+I2C_TR1);
	}

	/* set clock */
	ret = tcc_i2c_set_clk(i2c);
	if (ret < 0) {
		error_status = ret;
		goto err;
	}

	ret = tcc_i2c_set_noise_filter(i2c);
	if (ret < 0) {
		error_status = ret;
		goto err;
	}

	/* port configruation */
	ret = tcc_i2c_set_port(i2c);
	if (ret < 0) {
		pr_err("[ERROR][I2C] %s: failed to set port\n", __func__);
		error_status = ret;
		goto err;
	}

	/* enable i2c core */
	writel(I2C_CTRL_EN, i2c->regs + I2C_CTRL);

err:
	if (i2c->channel == 7) {
		(void)tcc_i2c_access_arbitration(i2c, false);
	}
	return (error_status < 0) ? error_status : 0;
}
static int32_t tcc_i2c_parse_dt(struct udevice *dev)
{
	struct tcc_i2c *i2c = dev_get_priv(dev);
	int32_t ret;

	if (i2c == NULL) {
		pr_err("[ERROR][I2C] %s: tcc_i2c is null!\n", __func__);
		return -ENOMEM;
	}
	/* base, port configuration address */
	i2c->regs = (void *)dev_read_addr_index(dev, 0);
	i2c->pcfg = (void *)dev_read_addr_index(dev, 1);

	/* channel */
	ret = dev_read_alias_seq(dev, &i2c->channel);
	if (ret < 0) {
		pr_err("[ERROR][I2C] %s: failed to get channel.\n", __func__);
		return ret;
	}
	if (i2c->channel < 0) {
		pr_err("[ERROR][I2C] %s: channel %d is wrong.\n",
				__func__, i2c->channel);
		return -EINVAL;
	}

	/* get clock info */
	ret = tcc_clk_get_by_index(dev, 0, &i2c->pclk);
	if (ret < 0) {
		pr_err("[ERROR][I2C] failed to get peripheral clock\n");
		return ret;
	}
	ret = tcc_clk_get_by_index(dev, 1, &i2c->hclk);
	if (ret < 0) {
		pr_err("[ERROR][I2C] failed to get iobus clock\n");
		return ret;
	}
	ret = tcc_clk_get_by_index(dev, 2, &i2c->fclk);
	if (ret < 0) {
		pr_err("[ERROR][I2C] failed to get fbus io clock\n");
		return ret;
	}

	/* port number */
	ret = dev_read_u32(dev, "port-mux", &i2c->port);
	if (ret < 0) {
		pr_err("[ERROR][I2C] failed to get port number\n");
		return ret;
	}

	i2c->speed = (uint32_t)dev_read_u32_default(dev,
			"clock-frequency", 400000);

	i2c->noise_filter = (uint32_t)dev_read_u32_default(dev,
			"noise_filter", 50);
	if (i2c->noise_filter < 50U) {
		pr_warn("[WARN][I2C] I2C must suppress noise of less than 50ns.\n");
		i2c->noise_filter = 50U;
	}

	/* pwh, pwl */
	i2c->pwh = (uint32_t)dev_read_u32_default(dev, "pulse-width-high", 2);
	i2c->pwl = (uint32_t)dev_read_u32_default(dev, "pulse-width-low", 3);
	i2c->use_pw = (bool)true;

	if ((i2c->version == TCC_I2C_VER1) && (i2c->channel < 4)) {
		pr_debug("[DEBUG][I2C] Not support pwh, pwl function\n");
		i2c->pwh = 2;
		i2c->pwl = 3;
		i2c->use_pw = (bool)false;
	}
	return 0;
}

static int32_t tcc_i2c_probe(struct udevice *dev)
{
	struct tcc_i2c *i2c  = dev_get_priv(dev);
	int32_t ret;

	i2c->version = dev_get_driver_data(dev);

	ret = tcc_i2c_parse_dt(dev);
	if (ret < 0) {
		pr_err("[ERROR][I2C] failed to parsing device tree.\n");
		return ret;
	}

	/* initialize i2c */
	ret = tcc_i2c_init(i2c);
	if (ret < 0) {
		pr_err("[ERROR][I2C] %s: failed to initialize i2c\n", __func__);
		return ret;
	}

	return 0;
}

static const struct dm_i2c_ops tcc_i2c_ops = {
	.xfer       = tcc_i2c_xfer,
	.probe_chip = tcc_i2c_probe_chip,
	.set_bus_speed  = tcc_i2c_set_bus_speed,
};

static const struct udevice_id tcc_i2c_ids[] = {
	{	.compatible = "telechips,tcc803x-i2c",
		.data = TCC_I2C_VER1 },
	{	.compatible = "telechips,tcc805x-i2c",
		.data = TCC_I2C_VER2 },
	{ }
};

U_BOOT_DRIVER(tcc_i2c) = {
	.name     = "tcc-i2c",
	.id       = UCLASS_I2C,
	.of_match = tcc_i2c_ids,
	.probe    = tcc_i2c_probe,
	.priv_auto_alloc_size = sizeof(struct tcc_i2c),
	.ops      = &tcc_i2c_ops,
};

