/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
*/

/*
* Modified by Telechips Inc.
*/



#include <asm/io.h>
#include <linux/delay.h>
#include "dptx_v14.h"
#include "dptx_reg.h"
#include "dptx_dbg.h"

static uint32_t __dptx_readl(struct Dptx_Params *pstDptx, uint32_t uiOffset)
{
	u32 uiData;

	uiData = readl(pstDptx->pvDPLink_BaseAddr + uiOffset);

	return uiData;
}

static void __dptx_writel(struct Dptx_Params *pstDptx, uint32_t uiOffset, uint32_t uiData)
{
	writel(uiData, (pstDptx->pvDPLink_BaseAddr + uiOffset));
}

uint32_t Dptx_Reg_Readl(struct Dptx_Params *pstDptx, uint32_t uiOffset)
{
	u32 uiReadData;

	uiReadData = __dptx_readl(pstDptx, uiOffset);

	return uiReadData;
}

void Dptx_Reg_Writel(struct Dptx_Params *pstDptx, uint32_t uiOffset, uint32_t uiData)
{
	__dptx_writel(pstDptx, uiOffset, uiData);
}

uint32_t Dptx_Reg_Direct_Read(uint32_t uiRegAddr)
{
	uint32_t uiData;
	uint64_t ulRegAddr;
	void *pioRegAddr;

	ulRegAddr = (uint64_t)uiRegAddr;
	pioRegAddr = (void *)ulRegAddr;

	uiData = readl(pioRegAddr);

	return uiData;
}

void Dptx_Reg_Direct_Write(uint32_t uiRegAddr, uint32_t uiData)
{
	uint64_t ulRegAddr;
	void *pioRegAddr;

	ulRegAddr = (uint64_t)uiRegAddr;
	pioRegAddr = (void *)ulRegAddr;

	writel(uiData, pioRegAddr);
}

uint32_t dptx_phy_read(struct Dptx_Params *pstDptx, uint32_t address)
{
	const uint32_t phy_read_bit = BIT(PHYREG_CMDADDR_PHY_READ_SHIFT);
	uint32_t phy_cmd = phy_read_bit |
			      (address & PHYREG_CMDADDR_PHY_ADDRESS_MASK);
	uint32_t phy_data;
	uint32_t loop;

	Dptx_Reg_Writel(pstDptx, DPTX_PHYREG_CMDADDR, phy_cmd);


	for (loop = 0; loop < 100; loop++) {
		phy_cmd = Dptx_Reg_Readl(pstDptx, DPTX_PHYREG_CMDADDR);
		if ((phy_cmd & phy_read_bit) == 0) {
			break;
		}
		udelay(100);
	}
	phy_data = Dptx_Reg_Readl(pstDptx, DPTX_PHYREG_DATA);
	phy_data &= PHYREG_DATA_PHY_DATA_DATA_MAS;

	return phy_data;
}



int32_t dptx_phy_write(struct Dptx_Params *pstDptx, uint32_t address,
		       uint32_t phy_data)
{
	const uint32_t phy_write_bit = BIT(PHYREG_CMDADDR_PHY_WRITE_SHIFT);
	uint32_t phy_cmd = phy_write_bit |
			      (address & PHYREG_CMDADDR_PHY_ADDRESS_MASK);
	uint32_t loop;
	int32_t ret = 0;

	Dptx_Reg_Writel(pstDptx, DPTX_PHYREG_DATA, (phy_data & 0xFFFF));
	Dptx_Reg_Writel(pstDptx, DPTX_PHYREG_CMDADDR, phy_cmd);

	for (loop = 0; loop < 100; loop++) {
		phy_cmd = Dptx_Reg_Readl(pstDptx, DPTX_PHYREG_CMDADDR);
		if ((phy_cmd & phy_write_bit) == 0) {
			break;
		}
		udelay(100);
	}
	return ret;
}

