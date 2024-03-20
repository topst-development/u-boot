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

#include <linux/delay.h>

#include "Dptx_v14.h"
#include "Dptx_reg.h"
#include "Dptx_dbg.h"


#define DPTX_AUX_MAX_RW_RETRY				200
#define DPTX_AUX_MAX_REPLY_RETRY			50

static int32_t dptx_aux_read_data(struct Dptx_Params *dptx, uint8_t *pucBuffer, uint32_t uiLength)
{
	uint32_t	i;
	uint32_t	*puiData = dptx->stAuxParams.auiReadData;

	for (i = 0; i < uiLength; i++)
		pucBuffer[i] = (puiData[i / 4] >> ((i % 4) * 8)) & 0xff;

	return DPTX_RETURN_NO_ERROR;
}

static void dptx_aux_clear_data(struct Dptx_Params *pstDptx)
{
	Dptx_Reg_Writel(pstDptx, DPTX_AUX_DATA0, 0);
	Dptx_Reg_Writel(pstDptx, DPTX_AUX_DATA1, 0);
	Dptx_Reg_Writel(pstDptx, DPTX_AUX_DATA2, 0);
	Dptx_Reg_Writel(pstDptx, DPTX_AUX_DATA3, 0);
}

static int32_t dptx_aux_write_data(struct Dptx_Params *pstDptx, uint8_t const *pucBuffer, uint32_t uiLength)
{
	uint32_t	auiWriteData[4];

	memset(auiWriteData, 0, sizeof(uint32_t) * 4);

	for (unsigned int i = 0; i < uiLength; i++) {
		auiWriteData[i / 4] |= (pucBuffer[i] << ((i % 4) * 8));
	}

	Dptx_Reg_Writel(pstDptx, DPTX_AUX_DATA0, auiWriteData[0]);
	Dptx_Reg_Writel(pstDptx, DPTX_AUX_DATA1, auiWriteData[1]);
	Dptx_Reg_Writel(pstDptx, DPTX_AUX_DATA2, auiWriteData[2]);
	Dptx_Reg_Writel(pstDptx, DPTX_AUX_DATA3, auiWriteData[3]);

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_aux_read_write(struct Dptx_Params *pstDptx,
												bool bRead_Mode,
												bool bI2C_Mode,
												bool bMot_Mode,
												bool bAddr_Only,
												uint32_t uiAddr,
												uint8_t *pucBuffer,
												uint32_t uiLength)
{
	bool		bAckSuccess;
	uint8_t		ucTtries = 0;
	uint8_t		ucAux_Status, ucAux_Bytes_Read;
	uint32_t	uiCmd_Type, uiAux_Cmd;
	uint32_t	uiCount = 0;

	if ((uiLength > 16) || (uiLength == 0)) {
		dptx_err("AUX read/write len must be 1-15, len=%d ", uiLength);
		return DPTX_RETURN_EINVAL;
	}

	uiCmd_Type = bRead_Mode ? DPTX_AUX_CMD_TYPE_READ : DPTX_AUX_CMD_TYPE_WRITE;

	if (!bI2C_Mode)
		uiCmd_Type |= DPTX_AUX_CMD_TYPE_NATIVE;

	if (bI2C_Mode && bMot_Mode)
		uiCmd_Type |= DPTX_AUX_CMD_TYPE_MOT;

	uiAux_Cmd = ((uiCmd_Type << DPTX_AUX_CMD_TYPE_SHIFT) | (uiAddr << DPTX_AUX_CMD_ADDR_SHIFT) | ((uiLength - 1) << DPTX_AUX_CMD_REQ_LEN_SHIFT));

	if (bAddr_Only)
		uiAux_Cmd |= DPTX_AUX_CMD_I2C_ADDR_ONLY;

	while (ucTtries++ < DPTX_AUX_MAX_RW_RETRY) {
		Dptx_Core_Clear_General_Interrupt(pstDptx, (uint32_t)DPTX_ISTS_AUX_REPLY);

		dptx_aux_clear_data(pstDptx);

		if (bRead_Mode == DPTX_AUX_CMD_TYPE_WRITE)
			dptx_aux_write_data(pstDptx, pucBuffer, uiLength);

		Dptx_Reg_Writel(pstDptx, DPTX_AUX_CMD, uiAux_Cmd);

		uiCount = 0;

		while (true) {
			pstDptx->stAuxParams.uiAuxStatus = Dptx_Reg_Readl(pstDptx, DPTX_AUX_STS);

			if (!(pstDptx->stAuxParams.uiAuxStatus & DPTX_AUX_STS_REPLY_RECEIVED))
				break;

			if (uiCount++ > DPTX_AUX_MAX_REPLY_RETRY) {
				dptx_err("No response from Sink->try(%d)", ucTtries);
				return DPTX_RETURN_ENODEV;
			}

			udelay(100);
		};

		bAckSuccess			= false;
		ucAux_Status		= ((pstDptx->stAuxParams.uiAuxStatus & DPTX_AUX_STS_STATUS_MASK) >> DPTX_AUX_STS_STATUS_SHIFT);
		ucAux_Bytes_Read	= ((pstDptx->stAuxParams.uiAuxStatus & DPTX_AUX_STS_BYTES_READ_MASK) >> DPTX_AUX_STS_BYTES_READ_SHIFT);

		switch (ucAux_Status) {
		case DPTX_AUX_STS_STATUS_ACK:
			if (ucAux_Bytes_Read == 0) {
				dptx_dbg("STATUS_ACK, but Bytes Read = 0, Retry(%d)", ucTtries);
				Dptx_Core_Soft_Reset(pstDptx, DPTX_SRST_CTRL_AUX);
				mdelay(1);
			} else {
				//dptx_dbg("STATUS_ACK.. try %d", ucTtries);
				bAckSuccess = true;
			}
			break;
		case DPTX_AUX_STS_STATUS_NACK:
		case DPTX_AUX_STS_STATUS_I2C_NACK:
			dptx_err("AUX Nack Status(%d).. try %d ", ucAux_Status, ucTtries);
			return DPTX_RETURN_I2C_OVER_AUX_NO_ACK;
		case DPTX_AUX_STS_STATUS_I2C_DEFER:
		case DPTX_AUX_STS_STATUS_DEFER:
			dptx_dbg("AUX Defer status(%d).. try %d ", ucAux_Status, ucTtries);
			mdelay(1);
			break;
		default:
			dptx_err("AUX Status Invalid ");
			Dptx_Core_Soft_Reset(pstDptx, DPTX_SRST_CTRL_AUX);
			mdelay(1);
			break;
		}

		if (bAckSuccess)
			break;
	}

	if (bAckSuccess) {
		if (bRead_Mode) {
			pstDptx->stAuxParams.auiReadData[0] = Dptx_Reg_Readl(pstDptx, DPTX_AUX_DATA0);
			pstDptx->stAuxParams.auiReadData[1] = Dptx_Reg_Readl(pstDptx, DPTX_AUX_DATA1);
			pstDptx->stAuxParams.auiReadData[2] = Dptx_Reg_Readl(pstDptx, DPTX_AUX_DATA2);
			pstDptx->stAuxParams.auiReadData[3] = Dptx_Reg_Readl(pstDptx, DPTX_AUX_DATA3);

			dptx_aux_read_data(pstDptx, pucBuffer, uiLength);

			return DPTX_RETURN_NO_ERROR;
		}
	} else {
		dptx_err("AUX communication exceeded retries to %d", ucTtries);
		return DPTX_RETURN_ENODEV;
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Aux_Read_DPCD(struct Dptx_Params *pstDptx, uint32_t uiAddr, uint8_t *pucBuffer)
{
	bool bRead_Mode = true, bI2C_Mode = false;
	bool bEnableMot_Mode = true, bAdressOnly = false;
	int32_t iRetVal;

	iRetVal = dptx_aux_read_write(pstDptx,
									bRead_Mode,
									bI2C_Mode,
									bEnableMot_Mode,
									bAdressOnly,
									uiAddr,
									pucBuffer,
									1);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Aux_Read_Bytes_From_DPCD(struct Dptx_Params *pstDptx, uint32_t uiAddr, uint8_t *pucBuffer, uint32_t uiLength)
{
	bool bRead_Mode = true, bI2C_Mode = false;
	bool bEnableMot_Mode = true, bAdressOnly = false;
	int32_t iRetVal;
	uint32_t uiActive_Length = 0;

	for (uint32_t i = 0; i < uiLength;) {
		uiActive_Length = min_t(unsigned int, uiLength - i, 16);

		iRetVal = dptx_aux_read_write(pstDptx,
										bRead_Mode,
										bI2C_Mode,
										bEnableMot_Mode,
										bAdressOnly,
										uiAddr + i,
										&pucBuffer[i],
										uiActive_Length);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return iRetVal;

		i += uiActive_Length;
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Aux_Write_DPCD(struct Dptx_Params *pstDptx, uint32_t uiAddr, uint8_t ucBuffer)
{
	bool bRead_Mode = false, bI2C_Mode = false;
	bool bEnableMot_Mode = true, bAdressOnly = false;
	int32_t iRetVal;

	iRetVal = dptx_aux_read_write(pstDptx,
									bRead_Mode,
									bI2C_Mode,
									bEnableMot_Mode,
									bAdressOnly,
									uiAddr,
									&ucBuffer,
									1);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Aux_Write_Bytes_To_DPCD(struct Dptx_Params *pstDptx, uint32_t uiAddr, uint8_t *pucBuffer, uint32_t uiLength)
{
	bool bRead_Mode = false, bI2C_Mode = false;
	bool bEnableMot_Mode = true, bAdressOnly = false;
	int32_t iRetVal;
	uint32_t uiActive_Length = 0;

	for (uint32_t i = 0; i < uiLength;) {
		uiActive_Length = min_t(unsigned int, uiLength - i, 16);

		iRetVal = dptx_aux_read_write(pstDptx,
										bRead_Mode,
										bI2C_Mode,
										bEnableMot_Mode,
										bAdressOnly,
										uiAddr + i,
										&pucBuffer[i],
										uiActive_Length);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return iRetVal;

		i += uiActive_Length;
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Aux_Read_Bytes_From_I2C(struct Dptx_Params *pstDptx, uint32_t uiDevice_Addr, uint8_t *pucBuffer, uint32_t uiLength)
{
	bool bRead_Mode = true, bI2C_Mode = true;
	bool bEnableMot_Mode = true, bAdressOnly = false;
	int32_t iRetVal;
	uint32_t uiActive_Length = 0;

	for (uint32_t i = 0; i < uiLength;) {
		uiActive_Length = min_t(unsigned int, uiLength - i, 16);

		iRetVal = dptx_aux_read_write(pstDptx,
										bRead_Mode,
										bI2C_Mode,
										bEnableMot_Mode,
										bAdressOnly,
										uiDevice_Addr,
										&pucBuffer[i],
										uiActive_Length);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return iRetVal;

		i += uiActive_Length;
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Aux_Write_Bytes_To_I2C(struct Dptx_Params *pstDptx, uint32_t uiDevice_Addr, uint8_t *pucBuffer, uint32_t uiLength)
{
	bool bRead_Mode = false, bI2C_Mode = true;
	bool bEnableMot_Mode = true, bAdressOnly = false;
	int32_t iRetVal;
	uint32_t uiActive_Length = 0;

	for (uint32_t i = 0; i < uiLength;) {

		uiActive_Length = min_t(unsigned int, uiLength - i, 16);

		iRetVal = dptx_aux_read_write(pstDptx,
										bRead_Mode,
										bI2C_Mode,
										bEnableMot_Mode,
										bAdressOnly,
										uiDevice_Addr,
										&pucBuffer[i],
										uiActive_Length);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return iRetVal;

		i += uiActive_Length;
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Aux_Write_AddressOnly_To_I2C(struct Dptx_Params *pstDptx, uint32_t uiDevice_Addr)
{
	bool bRead_Mode = false, bI2C_Mode = true;
	bool bEnableMot_Mode = false, bAdressOnly = true;
	uint8_t ucByte;
	int32_t iRetVal;

	iRetVal = dptx_aux_read_write(pstDptx,
									bRead_Mode,
									bI2C_Mode,
									bEnableMot_Mode,
									bAdressOnly,
									uiDevice_Addr,
									&ucByte,
									1);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	return DPTX_RETURN_NO_ERROR;
}