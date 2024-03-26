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
#include <vsprintf.h>

#include "dptx_v14.h"
#include "dptx_drm_dp_addition.h"
#include "dptx_reg.h"
#include "dptx_dbg.h"

#define ENABLE_EDID_LAW_DATA_DUMP

#define EDID_MAX_EXTRA_BLK		4
#define EDID_EXT_BLK_FIELD		126

#define PRINT_BUF_SIZE			DPTX_EDID_BUFLEN

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


#if defined(ENABLE_EDID_LAW_DATA_DUMP)
static void dptx_edid_Print_U8_Buf(uint8_t *pucBuf, uint32_t uiStart_RegOffset, uint32_t uiLength)
{
	char	acStr[PRINT_BUF_SIZE];
	int	iNumOfWritten = 0;
	uint32_t uiOffset;

	iNumOfWritten += snprintf(&acStr[iNumOfWritten], PRINT_BUF_SIZE - iNumOfWritten, "\n");

	for (uiOffset = 0; uiOffset < uiLength; uiOffset++) {
		if (!(uiOffset % 16)) {
			iNumOfWritten += snprintf(&acStr[iNumOfWritten], PRINT_BUF_SIZE - iNumOfWritten, "\n%02x:", (uiStart_RegOffset + uiOffset));

			if (iNumOfWritten >= PRINT_BUF_SIZE) {
				break;
			}
		}

		iNumOfWritten += snprintf(&acStr[iNumOfWritten],  PRINT_BUF_SIZE - iNumOfWritten, " %02x", pucBuf[uiOffset]);
		
		if (iNumOfWritten >= PRINT_BUF_SIZE) {
			break;
		}
	}

	dptx_info("%s", acStr);
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

static int32_t dptx_edid_read_block_from_I2cOverAux(struct Dptx_Params *pstDptx, uint8_t ucBlockNum)
{
	uint8_t ucRetry = 0;
	uint8_t ucOffset, ucSegment;
	int iRetVal;

again:
	ucOffset  = (ucBlockNum * DPTX_ONE_EDID_BLK_LEN);
	ucSegment = (ucBlockNum >> 1);

	iRetVal = Dptx_Aux_Write_Bytes_To_I2C(pstDptx, EDID_I2C_OVER_AUX_SEGMENT_ADDR, &ucSegment, 1);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("from Dptx_Aux_Write_Bytes_To_I2C() by block %u", ucBlockNum);
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Write_Bytes_To_I2C(pstDptx, EDID_I2C_OVER_AUX_ADDR, &ucOffset, 1);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("from Dptx_Aux_Write_Bytes_To_I2C() by block %u", ucBlockNum);
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Read_Bytes_From_I2C(pstDptx, EDID_I2C_OVER_AUX_ADDR, &pstDptx->pucEdidBuf[ucBlockNum * DPTX_ONE_EDID_BLK_LEN], DPTX_ONE_EDID_BLK_LEN);
	if (iRetVal !=  DPTX_RETURN_NO_ERROR) {
		if (ucRetry == 0) {
			ucRetry = 1;
			goto again;
		} else {
			dptx_err("from Dptx_Aux_Read_Bytes_From_I2C() by block %u", ucBlockNum);
			return iRetVal;
		}
	}

	iRetVal = Dptx_Aux_Write_AddressOnly_To_I2C(pstDptx, EDID_I2C_OVER_AUX_ADDR);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("from Dptx_Aux_Write_Bytes_To_I2C() by block %u", ucBlockNum);
		return iRetVal;
	}

	iRetVal = (ucBlockNum == 0) ? Dptx_Edid_Verify_EDID(pstDptx->pucEdidBuf) : DPTX_RETURN_NO_ERROR;


#if defined(ENABLE_EDID_LAW_DATA_DUMP)
{
	uint8_t *pucBuffer_Copied;

	pucBuffer_Copied = (ucBlockNum > 0) ? &pstDptx->pucEdidBuf[ucBlockNum * DPTX_ONE_EDID_BLK_LEN] : pstDptx->pucEdidBuf;

	dptx_edid_Print_U8_Buf(pucBuffer_Copied, 0, DPTX_ONE_EDID_BLK_LEN);
}
#endif

	return iRetVal;
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
	uint8_t ucExt_Blocks, ucBlk_Index;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;

	memset(pstDptx->pucEdidBuf, 0, DPTX_EDID_BUFLEN);

	iRetVal = dptx_edid_read_block_from_I2cOverAux(pstDptx, 0);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_info("Sink doesn't support EDID data on I2C Over Aux");
		return iRetVal;
	}

	ucExt_Blocks = pstDptx->pucEdidBuf[EDID_EXT_BLK_FIELD];
	if (ucExt_Blocks == 0) {

#if defined(ENABLE_EDID_LAW_DATA_DUMP)
		dptx_edid_Print_U8_Buf(pstDptx->pucEdidBuf, 0, DPTX_EDID_BUFLEN);
#endif

		return DPTX_RETURN_NO_ERROR;
	}

	if (ucExt_Blocks > EDID_MAX_EXTRA_BLK) {
		dptx_warn("Ext blocks(%u) are larger than Max %d->down to %d",
					ucExt_Blocks,
					(uint32_t)EDID_MAX_EXTRA_BLK,
					(uint32_t)EDID_MAX_EXTRA_BLK);

		ucExt_Blocks = EDID_MAX_EXTRA_BLK;
	}

	for (ucBlk_Index = 1; ucBlk_Index <= ucExt_Blocks; ucBlk_Index++) {
		iRetVal = dptx_edid_read_block_from_I2cOverAux(pstDptx, ucBlk_Index);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			break;
		}
	}

	return iRetVal;
}

int32_t Dptx_Edid_Read_EDID_Over_Sideband_Msg(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;

	memset(pstDptx->pucEdidBuf, 0, DPTX_EDID_BUFLEN);

	iRetVal = Dptx_Ext_Remote_I2C_Read(pstDptx, ucStream_Index);

#if defined(ENABLE_EDID_LAW_DATA_DUMP)
	dptx_edid_Print_U8_Buf(pstDptx->pucEdidBuf, 0, DPTX_ONE_EDID_BLK_LEN);
#endif

#if defined(ENABLE_EDID_LAW_DATA_DUMP)
{
	uint8_t ucExt_Blocks;

	ucExt_Blocks = pstDptx->pucEdidBuf[EDID_EXT_BLK_FIELD];
	if (ucExt_Blocks > 0) {

		dptx_edid_Print_U8_Buf(&pstDptx->pucEdidBuf[DPTX_ONE_EDID_BLK_LEN], 0, DPTX_ONE_EDID_BLK_LEN);
	}
}
#endif

	return iRetVal;
}

int32_t Dptx_Edid_Verify_EDID(uint8_t *pucEDID_Buf)
{
	const uint8_t aucHeader[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
	int32_t iCompCmp;
	uint32_t uiElements, uiEdid_CheckSum = 0;

	if (pucEDID_Buf == NULL) {
		dptx_err("EDID Buffer ptr is NULL");
		return DPTX_RETURN_EINVAL;
	}

	iCompCmp = memcmp(pucEDID_Buf, aucHeader, sizeof(aucHeader));
	if (iCompCmp) {
		dptx_err("Invalid EDID header(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)",
					pucEDID_Buf[0],
					pucEDID_Buf[1],
					pucEDID_Buf[2],
					pucEDID_Buf[3],
					pucEDID_Buf[4]);

		return DPTX_RETURN_ENODEV;
	}

	for (uiElements = 0; uiElements < DPTX_ONE_EDID_BLK_LEN; uiElements++) {
		uiEdid_CheckSum += pucEDID_Buf[uiElements];
	}

	if (uiEdid_CheckSum & 0xFF) {
		dptx_err("Invalid EDID checksum( 0x%x )", uiEdid_CheckSum);
		return DPTX_RETURN_ENODEV;
	}

	return DPTX_RETURN_NO_ERROR;
}


