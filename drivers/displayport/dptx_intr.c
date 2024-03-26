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

#include <linux/drm_dp_helper.h>
#include <linux/delay.h>

#include "dptx_v14.h"
#include "dptx_drm_dp_addition.h"
#include "dptx_reg.h"
#include "dptx_dbg.h"


#define EDID_START_BIT_OF_1ST_DETAILED_DES			54
#define EDID_SIZE_OF_DETAILED_DES				18

int32_t Dptx_Intr_Handle_Hotplug(struct Dptx_Params *pstDptx)
{
	bool	bSink_SSC_Profiled;
	uint8_t ucDCDPValue;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	struct Dptx_Video_Params	*pstVideoParams = &pstDptx->stVideoParams;

	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_SET_POWER, DP_SET_POWER_D0);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* For KCS */
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_MAX_DOWNSPREAD, &ucDCDPValue);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* For KCS */
		return iRetVal;
	}

	if (ucDCDPValue & SINK_TDOWNSPREAD_MASK) {
		dptx_dbg("SSC enable on the sink side");
		bSink_SSC_Profiled = true;
	} else {
		dptx_dbg("SSC disabled on the sink side");
		bSink_SSC_Profiled = false;
	}

	iRetVal = Dptx_Core_Set_PHY_NumOfLanes(pstDptx, pstDptx->ucMax_Lanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* For KCS */
		return iRetVal;
	}

	iRetVal = Dptx_Core_Set_PHY_SSC(pstDptx, bSink_SSC_Profiled);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* For KCS */
		return iRetVal;
	}

	iRetVal = Dptx_Ext_Set_Stream_Capability(pstDptx);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* For KCS */
		return iRetVal;
	}

	memset(pstDptx->aucDPCD_Caps, 0, DPTX_SINK_CAP_SIZE);

	iRetVal = Dptx_Aux_Read_Bytes_From_DPCD(pstDptx, DP_DPCD_REV, pstDptx->aucDPCD_Caps, DPTX_SINK_CAP_SIZE);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* For KCS */
		return iRetVal;
	}

	if (pstDptx->aucDPCD_Caps[DP_TRAINING_AUX_RD_INTERVAL] & DP_EXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT) {
		dptx_dbg("Sink has extended receiver capability... read from 0x2200");

		iRetVal = Dptx_Aux_Read_Bytes_From_DPCD(pstDptx, 0x2200, pstDptx->aucDPCD_Caps, DPTX_SINK_CAP_SIZE);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			/* For KCS */
			return iRetVal;
		}
	}

	dptx_dbg("Sink DP Revision %x.%x ", (pstDptx->aucDPCD_Caps[0] & 0xF0) >> 4, pstDptx->aucDPCD_Caps[0] & 0xF);

	iRetVal = Dptx_Link_Perform_Training(pstDptx, pstDptx->ucMax_Rate, pstDptx->ucMax_Lanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* For KCS */
		return iRetVal;
	}

	if (pstDptx->bMultStreamTransport) {
		iRetVal = Dptx_Ext_Set_Topology_Configuration(pstDptx);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			/* For KCS */
			return iRetVal;
		}
	}

	for (u8 ucStreamIndex = 0; ucStreamIndex < pstDptx->ucNumOfStreams; ucStreamIndex++) {
		if (pstDptx->stVideoParams.auiVideo_Code[ucStreamIndex] == DPTX_VIC_READ_PANEL_EDID) {
			dptx_dbg("Using video code by reading EDID... ");

			pstVideoParams->stDtdParams[ucStreamIndex].uiPixel_Clock = pstVideoParams->uiPeri_Pixel_Clock[ucStreamIndex];

			iRetVal = Dptx_Vidin_Calculate_Video_Average_TU_Symbols(pstDptx,
																			pstDptx->stDptxLink.ucNumOfLanes,
																			pstDptx->stDptxLink.ucLinkRate,
																			pstVideoParams->ucBitPerComponent,
																			pstVideoParams->ucPixel_Encoding,
																			pstVideoParams->stDtdParams[ucStreamIndex].uiPixel_Clock,
																			ucStreamIndex);
			if (iRetVal != DPTX_RETURN_NO_ERROR) {
				/* For KCS */
				return iRetVal;
			}

			iRetVal = Dptx_Vidin_Set_Video_TimingChange(pstDptx, ucStreamIndex);
			if (iRetVal != DPTX_RETURN_NO_ERROR) {
				/* For KCS */
				return iRetVal;
			}
		}else {
			iRetVal = Dptx_Vidin_Set_Video_TimingChange_FromVIC(pstDptx, pstVideoParams->auiVideo_Code[ucStreamIndex], (u8)ucStreamIndex);
			if (iRetVal != DPTX_RETURN_NO_ERROR) {
				/* For KCS */
				return iRetVal;
			}
		}
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Intr_Handle_HotUnplug(struct Dptx_Params *pstDptx)
{
	int32_t	iRetVal;
	u32	uiRegMap_DptxCctl, uiRegMap_PhyIFCtrl;

	pstDptx->bEstablish_Timing_Present = false;

	uiRegMap_DptxCctl = Dptx_Reg_Readl(pstDptx, DPTX_CCTL);
	uiRegMap_DptxCctl &= ~DPTX_CCTL_ENABLE_FEC;
	Dptx_Reg_Writel(pstDptx, DPTX_CCTL, uiRegMap_DptxCctl);

	mdelay(100);

	uiRegMap_PhyIFCtrl = Dptx_Reg_Readl(pstDptx, DPTX_PHYIF_CTRL);
	uiRegMap_PhyIFCtrl &= ~DPTX_PHYIF_CTRL_XMIT_EN_MASK;

	(void)Dptx_Core_Set_PHY_PowerState(pstDptx, PHY_POWER_DOWN_PHY_CLOCK);

	iRetVal = Dptx_Core_Get_PHY_BUSY_Status(pstDptx, pstDptx->stDptxLink.ucNumOfLanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("From Dptx_Core_Get_PHY_BUSY_Status()");
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Intr_Handle_Edid(struct Dptx_Params *pstDptx, u8 ucStream_Index)
{
	u8	aucPreferred_VIC[EDID_SIZE_OF_DETAILED_DES];
	u8	ucPClock_MSB, ucPClock_LSB;
	int32_t	iRetVal;
	struct Dptx_Dtd_Params stDTD;
	struct Dptx_Video_Params *pstVideoParams = &pstDptx->stVideoParams;

	if (pstDptx->bMultStreamTransport) {
		if (ucStream_Index >= PHY_INPUT_STREAM_MAX) {
			dptx_err("Invalid stream index( %d ) on MST", ucStream_Index);
			return DPTX_RETURN_EINVAL;
		}
	} else {
		if (ucStream_Index >= PHY_INPUT_STREAM_1) {
			dptx_err("Invalid stream index( %d ) on SST", ucStream_Index);
			return DPTX_RETURN_EINVAL;
		}
	}

	if (pstDptx->bMultStreamTransport) {
		iRetVal = Dptx_Edid_Read_EDID_Over_Sideband_Msg(pstDptx, ucStream_Index);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}
	} else {
		iRetVal = Dptx_Edid_Read_EDID_I2C_Over_Aux(pstDptx);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}
	}

	iRetVal = Dptx_Edid_Check_Detailed_Timing_Descriptors(pstDptx);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	pstVideoParams->ucVideo_Format = CEA_861;

	if (pstDptx->bEstablish_Timing_Present) {
		iRetVal = Dptx_Vidin_Fill_DTD_BasedOn_EST_Timings(pstDptx, &stDTD);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			iRetVal = Dptx_Vidin_Fill_Dtd(&stDTD, (u8)DPTX_DEFAULT_VIDEO_CODE, pstVideoParams->uiRefresh_Rate, pstVideoParams->ucVideo_Format);
			if (iRetVal != DPTX_RETURN_NO_ERROR) {
				return iRetVal;
			}
		}
	} else {
		iRetVal = Dptx_Edid_Verify_EDID(pstDptx->pucEdidBuf);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			iRetVal = Dptx_Vidin_Fill_Dtd(&stDTD, (u8)DPTX_DEFAULT_VIDEO_CODE, pstVideoParams->uiRefresh_Rate, pstVideoParams->ucVideo_Format);
			if (iRetVal != DPTX_RETURN_NO_ERROR) {
				return iRetVal;
			}
		} else {
			for (u8 ucDesc_Inex = 0; ucDesc_Inex < 4; ucDesc_Inex++) {
				ucPClock_MSB = pstDptx->pucEdidBuf[(EDID_START_BIT_OF_1ST_DETAILED_DES + (ucDesc_Inex * EDID_SIZE_OF_DETAILED_DES) + 1)];
				ucPClock_LSB = pstDptx->pucEdidBuf[(EDID_START_BIT_OF_1ST_DETAILED_DES + (ucDesc_Inex * EDID_SIZE_OF_DETAILED_DES))];

				if ((ucPClock_MSB != 0) && (ucPClock_LSB != 0)) {
					memcpy(aucPreferred_VIC, pstDptx->pucEdidBuf + EDID_START_BIT_OF_1ST_DETAILED_DES + (ucDesc_Inex * EDID_SIZE_OF_DETAILED_DES), EDID_SIZE_OF_DETAILED_DES);
				}
			}

			iRetVal = Dptx_Vidin_Parse_Dtd(&stDTD, aucPreferred_VIC);
			if (iRetVal != DPTX_RETURN_NO_ERROR) {
				pstVideoParams->ucVideo_Format = CEA_861;
				iRetVal = Dptx_Vidin_Fill_Dtd(&stDTD, (u8)DPTX_DEFAULT_VIDEO_CODE, pstVideoParams->uiRefresh_Rate, pstVideoParams->ucVideo_Format);
				if (iRetVal != DPTX_RETURN_NO_ERROR) {
					return iRetVal;
				}
			}
		}
	}

	memcpy(&pstVideoParams->stDtdParams[ucStream_Index], &stDTD, sizeof(struct Dptx_Dtd_Params));

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Intr_Get_HotPlug_Status(struct Dptx_Params *pstDptx, uint8_t *pucHotPlug_Status)
{
	uint32_t uiHpdStatus;

	if (pucHotPlug_Status == NULL) {
		dptx_err("pucHotPlug_Status == NULL");
		return DPTX_RETURN_EINVAL;
	}

	uiHpdStatus = Dptx_Reg_Readl(pstDptx, DPTX_HPDSTS);
	if (uiHpdStatus & DPTX_HPDSTS_STATUS) {
		dptx_dbg("Hot plugged -> HPD_STATUS[0x%08x]: 0x%08x", DPTX_HPDSTS, uiHpdStatus);
		*pucHotPlug_Status = (uint8_t)HPD_STATUS_PLUGGED;
	} else {
		dptx_dbg("Hot unplugged -> HPD_STATUS[0x%08x]: 0x%08x", DPTX_HPDSTS, uiHpdStatus);
		*pucHotPlug_Status = (uint8_t)HPD_STATUS_UNPLUGGED;
	}

	return DPTX_RETURN_NO_ERROR;
}
