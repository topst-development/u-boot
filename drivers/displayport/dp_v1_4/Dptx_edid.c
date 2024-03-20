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

#include <string.h>

#include "Dptx_v14.h"
#include "Dptx_drm_dp_addition.h"
#include "Dptx_reg.h"
#include "Dptx_dbg.h"

//#define ENABLE_EDID_LAW_DATA_LOG

#define EDID_I2C_ADDR				0x50
#define EDID_I2C_SEGMENT_ADDR		0x30

#define EDID_MAX_EXTRA_BLK			4

#define PRINT_BUF_SIZE			1024

/* Established timing blocks */
#define ET1_800x600_60hz		BIT(0)
#define ET1_800x600_56hz		BIT(1)
#define ET1_640x480_75hz		BIT(2)
#define ET1_640x480_72hz		BIT(3)
#define ET1_640x480_67hz		BIT(4)
#define ET1_640x480_60hz		BIT(5)
#define ET1_720x400_88hz		BIT(6)
#define ET1_720x400_70hz		BIT(7)

#define ET2_1280x1024_75hz		BIT(0)
#define ET2_1024x768_75hz		BIT(1)
#define ET2_1024x768_70hz		BIT(2)
#define ET2_1024x768_60hz		BIT(3)
#define ET2_1024x768_87hz		BIT(4)
#define ET2_832x624_75hz		BIT(5)
#define ET2_800x600_75hz		BIT(6)
#define ET2_800x600_72hz		BIT(7)
#define ET3_1152x870_75hz		BIT(7)


#if defined(ENABLE_EDID_LAW_DATA_LOG)
static void dptx_edid_Print_U8_Buf(uint8_t *pucBuf, uint32_t uiStart_RegOffset, uint32_t uiLength)
{
	char	acStr[PRINT_BUF_SIZE];
	int	iNumOfWritten = 0;
	uint32_t uiOffset;

	iNumOfWritten += snprintf(&acStr[iNumOfWritten], PRINT_BUF_SIZE - iNumOfWritten, "\n");

	for (uiOffset = 0; uiOffset < uiLength; uiOffset++) {
		if (!(uiOffset % 16)) {
			iNumOfWritten += snprintf(&acStr[iNumOfWritten], PRINT_BUF_SIZE - iNumOfWritten, "\n%02x:", (uiStart_RegOffset + uiOffset));
			if (iNumOfWritten >= PRINT_BUF_SIZE)
				break;
		}

		iNumOfWritten += snprintf(&acStr[iNumOfWritten],  PRINT_BUF_SIZE - iNumOfWritten, " %02x", pucBuf[uiOffset]);
		if (iNumOfWritten >= PRINT_BUF_SIZE)
			break;
	}

	dptx_dbg("%s", acStr);
}
#endif

static int32_t dptx_edid_parse_established_timing(struct Dptx_Params *dptx)
{
	uint8_t byte1, byte2, byte3;

	byte1 = dptx->pucEdidBuf[35];
	byte2 = dptx->pucEdidBuf[36];
	byte3 = dptx->pucEdidBuf[37];

	if (byte1 & ET1_800x600_60hz) {
		dptx_dbg("Sink supports ET1_800x600_60hz\n");

		dptx->eEstablished_Timing = DMT_800x600_60hz;
		return DPTX_RETURN_NO_ERROR;
	}

	if (byte1 & ET1_640x480_60hz) {
		dptx_dbg("Sink supports ET1_640x480_60hz ");

		dptx->eEstablished_Timing = DMT_640x480_60hz;
		return DPTX_RETURN_NO_ERROR;
	}

	if (byte2 & ET2_1024x768_60hz) {
		dptx_dbg("Sink supports ET2_1024x768_60hz ");

		dptx->eEstablished_Timing = DMT_1024x768_60hz;
		return DPTX_RETURN_NO_ERROR;
	}

	if (byte1 & ET1_800x600_56hz)
		dptx_dbg("Sink supports ET1_800x600_56hz, but we dont ");
	if (byte1 & ET1_640x480_75hz)
		dptx_dbg("Sink supports ET1_640x480_75hz, but we dont ");
	if (byte1 & ET1_640x480_72hz)
		dptx_dbg("Sink supports ET1_640x480_72hz, but we dont ");
	if (byte1 & ET1_640x480_67hz)
		dptx_dbg("Sink supports ET1_640x480_67hz, but we dont ");
	if (byte1 & ET1_720x400_88hz)
		dptx_dbg("Sink supports ET1_720x400_88hz, but we dont ");
	if (byte1 & ET1_720x400_70hz)
		dptx_dbg("Sink supports ET1_720x400_70hz, but we dont ");

	if (byte2 & ET2_1280x1024_75hz)
		dptx_dbg("Sink supports ET2_1280x1024_75hz, but we dont ");
	if (byte2 & ET2_1024x768_75hz)
		dptx_dbg("Sink supports ET2_1024x768_75hz, but we dont ");
	if (byte2 & ET2_1024x768_70hz)
		dptx_dbg("Sink supports ET2_1024x768_70hz, but we dont ");
	if (byte2 & ET2_1024x768_87hz)
		dptx_dbg("Sink supports ET2_1024x768_87hz, but we dont ");
	if (byte2 & ET2_832x624_75hz)
		dptx_dbg("Sink supports ET2_832x624_75hz, but we dont ");
	if (byte2 & ET2_800x600_75hz)
		dptx_dbg("Sink supports ET2_800x600_75hz, but we dont ");
	if (byte2 & ET2_800x600_72hz)
		dptx_dbg("Sink supports ET2_800x600_72hz, but we dont ");

	if (byte3 & ET3_1152x870_75hz)
		dptx_dbg("Sink supports ET3_1152x870_75hz, but we dont ");

	dptx->eEstablished_Timing = DMT_NONE;

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_edid_read_block(struct Dptx_Params *pstDptx, uint8_t ucBlockNum)
{
	uint8_t	ucRetry;
	uint8_t	ucOffset, ucSegment;
	uint8_t	*pucBuffer_Copied;
	int32_t	iRetVal;

again:
	ucOffset  = (ucBlockNum * DPTX_EDID_BUFLEN);
	ucSegment = (ucBlockNum >> 1);

	iRetVal = Dptx_Aux_Write_Bytes_To_I2C(pstDptx, EDID_I2C_SEGMENT_ADDR, &ucSegment, 1);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("Error from Dptx_Aux_Write_Bytes_To_I2C() by block %d", ucBlockNum);
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Write_Bytes_To_I2C(pstDptx, EDID_I2C_ADDR, &ucOffset, 1);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("Error from Dptx_Aux_Write_Bytes_To_I2C() by block %d", ucBlockNum);
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Read_Bytes_From_I2C(pstDptx, EDID_I2C_ADDR, &pstDptx->pucEdidBuf[ucBlockNum * DPTX_EDID_BUFLEN], DPTX_EDID_BUFLEN);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		if (ucRetry == 0) {
			ucRetry = 1;
			dptx_warn("Retry from Dptx_Aux_Read_Bytes_From_I2C() by block %d", ucBlockNum);
			goto again;
		} else {
			dptx_err("Error from Dptx_Aux_Read_Bytes_From_I2C() by block %d", ucBlockNum);
			return iRetVal;
		}
	}

	iRetVal = Dptx_Aux_Write_AddressOnly_To_I2C(pstDptx, 0x50);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		memset(pstDptx->pucSecondary_EDID, 0, DPTX_EDID_BUFLEN);
		return iRetVal;
	}

	if (ucBlockNum > 0) {
		pucBuffer_Copied = (pstDptx->pucEdidBuf + DPTX_EDID_BUFLEN);
		memcpy(pstDptx->pucSecondary_EDID, pucBuffer_Copied, DPTX_EDID_BUFLEN);
	}

	pucBuffer_Copied = (pstDptx->pucEdidBuf + (DPTX_EDID_BUFLEN * ucBlockNum));

#if defined(ENABLE_EDID_LAW_DATA_LOG)
	dptx_edid_Print_U8_Buf(pucBuffer_Copied, 0, DPTX_EDID_BUFLEN);
#endif

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Edid_Check_Detailed_Timing_Descriptors(struct Dptx_Params *pstDptx)
{
	if ((pstDptx->pucEdidBuf[54] == 0 && pstDptx->pucEdidBuf[55] == 0) &&
		(pstDptx->pucEdidBuf[72] == 0 && pstDptx->pucEdidBuf[73] == 0) &&
		(pstDptx->pucEdidBuf[90] == 0 && pstDptx->pucEdidBuf[91] == 0) &&
		(pstDptx->pucEdidBuf[108] == 0 && pstDptx->pucEdidBuf[109] == 0)) {
		dptx_dbg("Establish timing bitmap is presented...");

		pstDptx->bEstablish_Timing_Present = true;

		dptx_edid_parse_established_timing(pstDptx);
	} else {
		dptx_dbg("Detailed timing descriptor is presented... continuing with usual way");
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Edid_Read_EDID_I2C_Over_Aux(struct Dptx_Params *pstDptx)
{
	uint8_t	*pucFirst_Edid_Block, ucExt_Blocks;
	int32_t	iRetVal;
	unsigned int	i;

	memset(pstDptx->pucEdidBuf, 0, DPTX_EDID_BUFLEN);

	iRetVal = dptx_edid_read_block(pstDptx, 0);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	ucExt_Blocks = pstDptx->pucEdidBuf[126];
	if (ucExt_Blocks == 0) {

#if defined(ENABLE_EDID_LAW_DATA_LOG)
		dptx_edid_Print_U8_Buf(pstDptx->pucEdidBuf, 0, DPTX_EDID_BUFLEN);
#endif

		return DPTX_RETURN_NO_ERROR;
	}

	if (ucExt_Blocks > EDID_MAX_EXTRA_BLK) {
		dptx_warn("num_ext_blocks(%d) are larger than 4: down to 4", pstDptx->pucEdidBuf[126]);
		ucExt_Blocks = EDID_MAX_EXTRA_BLK;
	}

	dptx_dbg("The number of num_ext_blocks = %d", pstDptx->pucEdidBuf[126]);

	pucFirst_Edid_Block = malloc(DPTX_EDID_BUFLEN);
	if (pucFirst_Edid_Block == NULL) {
		dptx_err("pucFirst_Edid_Block is NULL");
		return DPTX_RETURN_ENOMEM;
	}

	memcpy(pucFirst_Edid_Block, pstDptx->pucEdidBuf, DPTX_EDID_BUFLEN);

	free(pstDptx->pucEdidBuf);

	pstDptx->pucEdidBuf = malloc(DPTX_EDID_BUFLEN * ucExt_Blocks + DPTX_EDID_BUFLEN);
	if (pstDptx->pucEdidBuf == NULL) {
		dptx_err("dptx->edid is NULL");
		return DPTX_RETURN_ENOMEM;
	}

	memcpy(pstDptx->pucEdidBuf, pucFirst_Edid_Block, DPTX_EDID_BUFLEN);

	for (i = 1; i <= ucExt_Blocks; i++) {
		iRetVal = dptx_edid_read_block(pstDptx, i);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			goto fail;
	}

	free(pucFirst_Edid_Block);

	return DPTX_RETURN_NO_ERROR;

fail:
	free(pucFirst_Edid_Block);

	return DPTX_RETURN_ENODEV;
}

int32_t Dptx_Edid_Read_EDID_Over_Sideband_Msg(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	uint8_t	ucExt_Blocks;
	int32_t	iRetVal;

	memset(pstDptx->pucEdidBuf, 0, DPTX_EDID_BUFLEN);
	memset(pstDptx->pucSecondary_EDID, 0, DPTX_EDID_BUFLEN);

	iRetVal = Dptx_Ext_Remote_I2C_Read(pstDptx, ucStream_Index);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

#if defined(ENABLE_EDID_LAW_DATA_LOG)
	dptx_edid_Print_U8_Buf(pstDptx->pucEdidBuf, 0, DPTX_EDID_BUFLEN);
#endif

	ucExt_Blocks = pstDptx->pucEdidBuf[126];

	if (ucExt_Blocks > 0) {
#if defined(ENABLE_EDID_LAW_DATA_LOG)
		dptx_edid_Print_U8_Buf(pstDptx->pucSecondary_EDID, 0, DPTX_EDID_BUFLEN);
#endif
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Edid_Verify_EDID(struct Dptx_Params *pstDptx)
{
	int32_t	iRetCmp;
	uint32_t uiEdid_CheckSum = 0;
	uint8_t aucEdid_Header[8];

	aucEdid_Header[0] = 0x00;
	aucEdid_Header[1] = 0xFF;
	aucEdid_Header[2] = 0xFF;
	aucEdid_Header[3] = 0xFF;
	aucEdid_Header[4] = 0xFF;
	aucEdid_Header[5] = 0xFF;
	aucEdid_Header[6] = 0xFF;
	aucEdid_Header[7] = 0x00;

	iRetCmp = memcmp(pstDptx->pucEdidBuf, aucEdid_Header, sizeof(aucEdid_Header));
	if (iRetCmp) {
		dptx_err("Invalid EDID header( 0x%x, 0x%x, 0x%x, 0x%x, 0x%x ) ",
					pstDptx->pucEdidBuf[0],
					pstDptx->pucEdidBuf[1],
					pstDptx->pucEdidBuf[2],
					pstDptx->pucEdidBuf[3],
					pstDptx->pucEdidBuf[4]);
		return DPTX_RETURN_ENODEV;
	}

	for (int i = 0; i < DPTX_EDID_BUFLEN; i++)
		uiEdid_CheckSum += pstDptx->pucEdidBuf[i];

	if (uiEdid_CheckSum & 0xFF) {
		dptx_err("Invalid EDID checksum( 0x%x )", uiEdid_CheckSum);
		return DPTX_RETURN_ENODEV;
	}

	return DPTX_RETURN_NO_ERROR;
}


