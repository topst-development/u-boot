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


#define MAX_TRY_CLOCK_RECOVERY					5		/* Vesa Spec. Figure 3-20 */
#define MAX_TRY_CHANNEL_EQ						5		/* Vesa Spec. Figure 3-21 */
#define MAX_TRY_SINK_LINK_STATUS_UPDATE			10		/* Vesa Spec. Figure 3-20 */
#define MAX_TRY_SINK_UPDATE_STATUS				100

#define DP_TRAIN_VOLTAGE_LEVEL_MASK				0x03
#define DP_TRAIN_PRE_EMPHASIS_LEVEL_MASK		0x0C

#define DP_TRAIN_CLOCK_RECOVERY					true
#define DP_TRAIN_CHANNEL_EQ						false


static int32_t dptx_link_get_clock_recovery_status(struct Dptx_Params *dptx, bool bCR_Training, bool *pbCR_Done)
{
	uint8_t ucDPCD_AuxReadInterval, ucRetry_LinkStatus = 0;
	uint8_t ucDPCD_LaneAlign_Status, ucLaneX_Status;
	uint8_t ucLane_Index;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;
	uint32_t uiAuxReadInterval_Delay;

	if (pbCR_Done == NULL) {
		dptx_err("Invalid paramter...pbCR_Done == NULL ");
		return DPTX_RETURN_EINVAL;
	}

	if (bCR_Training) {
		uiAuxReadInterval_Delay = 100;
		dptx_dbg("CR Training => delay time to 100us to check CR Status");
	} else {
		iRetVal = Dptx_Aux_Read_DPCD(dptx, DP_TRAINING_AUX_RD_INTERVAL, &ucDPCD_AuxReadInterval);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		uiAuxReadInterval_Delay = min_t(uint32_t, (ucDPCD_AuxReadInterval & SINK_TRAINING_AUX_RD_INTERVAL_MASK), 4);

		uiAuxReadInterval_Delay = (uiAuxReadInterval_Delay == 0) ? 400U : (uiAuxReadInterval_Delay * 4000);

		dptx_dbg("EQ Training => Aux interval(0x%x) => delay time to %dus", ucDPCD_AuxReadInterval, uiAuxReadInterval_Delay);
	}

	udelay(uiAuxReadInterval_Delay);

	do {
		iRetVal = Dptx_Aux_Read_DPCD(dptx, DP_LANE_ALIGN_STATUS_UPDATED, &ucDPCD_LaneAlign_Status);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		if (ucDPCD_LaneAlign_Status & DP_LINK_STATUS_UPDATED) {
			dptx_dbg("Link status and Adjust Request updated(0x%x) after %dms", ucDPCD_LaneAlign_Status, ucRetry_LinkStatus);
			break;
		}

		if (ucRetry_LinkStatus == MAX_TRY_SINK_LINK_STATUS_UPDATE) {
			dptx_dbg("Link status and Adjust Request NOT updated(0x%x) for 100ms", ucDPCD_LaneAlign_Status);
		}

		mdelay(1);
	} while (ucRetry_LinkStatus++ < MAX_TRY_SINK_UPDATE_STATUS);

	iRetVal = Dptx_Aux_Read_Bytes_From_DPCD(dptx, DP_LANE0_1_STATUS, dptx->stDptxLink.aucTraining_Status, DP_LINK_STATUS_SIZE);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	*pbCR_Done = true;

	for (ucLane_Index = 0; ucLane_Index < dptx->stDptxLink.ucNumOfLanes; ucLane_Index++) {
		ucLaneX_Status = Drm_Addition_Get_Lane_Status(dptx->stDptxLink.aucTraining_Status, ucLane_Index);
		if ((ucLaneX_Status & DP_LANE_CR_DONE) == 0) {
			*pbCR_Done = false;
		}
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_link_adjust_clock_recovery(struct Dptx_Params *pstDptx)
{
	uint8_t ucDPCD_Adjust_Request_LaneX, ucLane_Index;
	uint8_t aucDPCD_Adjusted_Value[DPTX_MAX_LINK_LANES] = { 0, };
	uint8_t aucSink_ConfigValues[DPTX_MAX_LINK_LANES] = { 0, };
	int32_t iRetVal;

	switch (pstDptx->stDptxLink.ucNumOfLanes) {
	case (uint8_t)PHY_LANE_4:
		iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_ADJUST_REQUEST_LANE2_3, &ucDPCD_Adjust_Request_LaneX);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		aucDPCD_Adjusted_Value[2] = (ucDPCD_Adjust_Request_LaneX & 0x0F);
		aucDPCD_Adjusted_Value[3] = ((ucDPCD_Adjust_Request_LaneX & 0xF0) >> 4);

		iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_ADJUST_REQUEST_LANE0_1, &ucDPCD_Adjust_Request_LaneX);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		aucDPCD_Adjusted_Value[0] = (ucDPCD_Adjust_Request_LaneX & 0x0F);
		aucDPCD_Adjusted_Value[1] = ((ucDPCD_Adjust_Request_LaneX & 0xF0) >> 4);

		dptx_dbg("LANE0(Vsw %d, PreEm %d), LANE1(Vsw %d, PreEm %d), LANE2(Vsw %d, PreEm %d), LANE3(Vsw %d, PreEm %d)",
				(aucDPCD_Adjusted_Value[0] & 0x03), (aucDPCD_Adjusted_Value[0] & 0x0C) >> 2,
				(aucDPCD_Adjusted_Value[1] & 0x03), (aucDPCD_Adjusted_Value[1] & 0x0C) >> 2,
				(aucDPCD_Adjusted_Value[2] & 0x03), (aucDPCD_Adjusted_Value[2] & 0x0C) >> 2,
				(aucDPCD_Adjusted_Value[3] & 0x03), (aucDPCD_Adjusted_Value[3] & 0x0C) >> 2);
		break;
	case (uint8_t)PHY_LANE_2:
	case (uint8_t)PHY_LANE_1:
		iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_ADJUST_REQUEST_LANE0_1, &ucDPCD_Adjust_Request_LaneX);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		aucDPCD_Adjusted_Value[0] = (ucDPCD_Adjust_Request_LaneX & 0x0F);
		aucDPCD_Adjusted_Value[1] = ((ucDPCD_Adjust_Request_LaneX & 0xF0) >> 4);

		dptx_dbg("LANE0(Vsw %d, PreEm %d), LANE1(Vsw %d, PreEm %d)",
					(aucDPCD_Adjusted_Value[0] & 0x03), (aucDPCD_Adjusted_Value[0] & 0x0C) >> 2,
					(aucDPCD_Adjusted_Value[1] & 0x03), (aucDPCD_Adjusted_Value[1] & 0x0C) >> 2);
		break;
	default:
		dptx_err("Invalid number of lanes %d ", pstDptx->stDptxLink.ucNumOfLanes);
		return DPTX_RETURN_EINVAL;
	}

	for (ucLane_Index = 0; ucLane_Index < pstDptx->stDptxLink.ucNumOfLanes; ucLane_Index++) {
		pstDptx->stDptxLink.aucVoltageSwing_level[ucLane_Index] = (aucDPCD_Adjusted_Value[ucLane_Index] & DP_TRAIN_VOLTAGE_LEVEL_MASK);
		pstDptx->stDptxLink.aucPreEmphasis_level[ucLane_Index] = ((aucDPCD_Adjusted_Value[ucLane_Index] & DP_TRAIN_PRE_EMPHASIS_LEVEL_MASK) >> 2);

		iRetVal = Dptx_Sec_Set_EQ(pstDptx,
									ucLane_Index,
									pstDptx->stDptxLink.ucLinkRate,
									pstDptx->stDptxLink.aucVoltageSwing_level[ucLane_Index],
									pstDptx->stDptxLink.aucPreEmphasis_level[ucLane_Index]);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		iRetVal = Dptx_Core_Set_PHY_PreEmphasis(pstDptx, ucLane_Index, (enum PHY_PRE_EMPHASIS_LEVEL)pstDptx->stDptxLink.aucPreEmphasis_level[ucLane_Index]);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		iRetVal = Dptx_Core_Set_PHY_VSW(pstDptx, ucLane_Index, (enum PHY_VOLTAGE_SWING_LEVEL)pstDptx->stDptxLink.aucVoltageSwing_level[ucLane_Index]);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		aucSink_ConfigValues[ucLane_Index] = 0;

		aucSink_ConfigValues[ucLane_Index] |= ((pstDptx->stDptxLink.aucVoltageSwing_level[ucLane_Index] << DP_TRAIN_VOLTAGE_SWING_SHIFT) & DP_TRAIN_VOLTAGE_SWING_MASK);
		if (pstDptx->stDptxLink.aucVoltageSwing_level[ucLane_Index] == DP_TRAIN_VOLTAGE_SWING_LEVEL_3) {
			dptx_dbg("Lane %d VSW reached to level 3", ucLane_Index);
			aucSink_ConfigValues[ucLane_Index] |= DP_TRAIN_MAX_SWING_REACHED;
		}

		aucSink_ConfigValues[ucLane_Index] |= ((pstDptx->stDptxLink.aucPreEmphasis_level[ucLane_Index] <<  DP_TRAIN_PRE_EMPHASIS_SHIFT) & DP_TRAIN_PRE_EMPHASIS_MASK);
		if (pstDptx->stDptxLink.aucPreEmphasis_level[ucLane_Index] == (DP_TRAIN_PRE_EMPH_LEVEL_3 >> 3)) {
			dptx_dbg("Lane %d Pre-emphasis reached to level 3", ucLane_Index);
			aucSink_ConfigValues[ucLane_Index] |= DP_TRAIN_MAX_PRE_EMPHASIS_REACHED;
		}
	}

	iRetVal = Dptx_Aux_Write_Bytes_To_DPCD(pstDptx, DP_TRAINING_LANE0_SET, aucSink_ConfigValues, pstDptx->stDptxLink.ucNumOfLanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_link_get_ch_equalization_status(struct Dptx_Params *pstDptx, bool *pbCR_Done, bool *pbEQ_Done)
{
	int32_t	iRetVal;

	iRetVal = dptx_link_get_clock_recovery_status(pstDptx, DP_TRAIN_CHANNEL_EQ, pbEQ_Done);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	*pbEQ_Done = Drm_Addition_Get_Channel_EQ_Status(pstDptx->stDptxLink.aucTraining_Status, pstDptx->stDptxLink.ucNumOfLanes);

	if ((*pbCR_Done)   && (*pbEQ_Done)) {
		dptx_dbg("***CH_CR_DONE => Done, CH_EQ_DONE => Done");
	} else {
		dptx_dbg("***CH_CR_DONE => %d, CH_EQ_DONE => %d", *pbCR_Done, *pbEQ_Done);
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_link_reduce_rate(struct Dptx_Params *pstDptx, bool *pbRate_Reduced)
{
	uint8_t ucLinkRate = (uint8_t)DPTX_PHYIF_CTRL_RATE_RBR;;

	*pbRate_Reduced = true;

	switch (pstDptx->stDptxLink.ucLinkRate) {
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		ucLinkRate = DPTX_PHYIF_CTRL_RATE_HBR2;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		ucLinkRate = DPTX_PHYIF_CTRL_RATE_HBR;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		ucLinkRate = DPTX_PHYIF_CTRL_RATE_RBR;
		break;
	case DPTX_PHYIF_CTRL_RATE_RBR:
		*pbRate_Reduced = false;
		dptx_warn("Rate is reached to RBR ");
		return DPTX_RETURN_NO_ERROR;
	default:
		*pbRate_Reduced = false;
		dptx_err("Invalid PHY rate %d\n", pstDptx->stDptxLink.ucLinkRate);
		return DPTX_RETURN_EINVAL;
	}

	dptx_info(" Reducing rate from %s to %s",
				pstDptx->stDptxLink.ucLinkRate == DPTX_PHYIF_CTRL_RATE_RBR ? "RBR" :
				(pstDptx->stDptxLink.ucLinkRate == DPTX_PHYIF_CTRL_RATE_HBR) ?  "HBR" :
				(pstDptx->stDptxLink.ucLinkRate == DPTX_PHYIF_CTRL_RATE_HBR2) ? "HB2":"HBR3",
				ucLinkRate == DPTX_PHYIF_CTRL_RATE_RBR ? "RBR" :
				(ucLinkRate == DPTX_PHYIF_CTRL_RATE_HBR) ?  "HBR" :
				(ucLinkRate == DPTX_PHYIF_CTRL_RATE_HBR2) ? "HB2":"HBR3");

	pstDptx->stDptxLink.ucLinkRate = ucLinkRate;

	return DPTX_RETURN_NO_ERROR;
}

static bool dptx_link_reduce_lanes(struct Dptx_Params *pstDptx, bool *pbLane_Reduced)
{
	uint8_t ucNumOfLanes;

	*pbLane_Reduced = true;

	switch(pstDptx->stDptxLink.ucNumOfLanes) 
	{
		case (uint8_t)PHY_LANE_4:
			ucNumOfLanes = (uint8_t)PHY_LANE_2;
			break;
		case (uint8_t)PHY_LANE_2:
			ucNumOfLanes = (uint8_t)PHY_LANE_1;
			break;
		case (uint8_t)PHY_LANE_1:
			*pbLane_Reduced = false;
			dptx_warn("The number of lanes is reached to 1");
			return DPTX_RETURN_NO_ERROR;
		default:
			*pbLane_Reduced = false;
			dptx_err("Invalid number of lanes %d\n", pstDptx->stDptxLink.ucNumOfLanes);
			return DPTX_RETURN_EINVAL;
	}

	dptx_info("Reducing lanes from %d to %d", pstDptx->stDptxLink.ucNumOfLanes, ucNumOfLanes);
	
	pstDptx->stDptxLink.ucNumOfLanes = ucNumOfLanes;
    pstDptx->stDptxLink.ucLinkRate = pstDptx->ucMax_Rate;
	
	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_link_perform_clock_recovery(struct Dptx_Params *pstDptx, bool *pbCR_Done)
{
	uint8_t ucLaneCount, ucCR_Try;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;

	dptx_dbg("Training clock_recovery with Setting to TPS 1 in PHY and Pattern Sequence 1 in Sink...");

	iRetVal = Dptx_Core_Set_PHY_Pattern(pstDptx, DPTX_PHYIF_CTRL_TPS_1);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_TRAINING_PATTERN_SET, DP_TRAINING_PATTERN_1);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = dptx_link_get_clock_recovery_status(pstDptx, DP_TRAIN_CLOCK_RECOVERY, pbCR_Done);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	if (*pbCR_Done) {
		dptx_dbg("Clock Recovery has been done at once !!!");
		return DPTX_RETURN_NO_ERROR;
	}

	for (ucCR_Try = 0; ucCR_Try < MAX_TRY_CLOCK_RECOVERY; ucCR_Try++) {
		iRetVal = dptx_link_adjust_clock_recovery(pstDptx);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		for (ucLaneCount = 0; ucLaneCount < pstDptx->stDptxLink.ucNumOfLanes; ucLaneCount++) {
			if (pstDptx->stDptxLink.aucVoltageSwing_level[ucLaneCount] != DP_TRAIN_VOLTAGE_SWING_LEVEL_3) {
				break;
			}
		}

		if (ucLaneCount == pstDptx->stDptxLink.ucNumOfLanes) {
			dptx_err("All %d Lanes are reached to level 3", ucLaneCount);
			return DPTX_RETURN_ENODEV;
		}

		iRetVal = dptx_link_get_clock_recovery_status(pstDptx, DP_TRAIN_CLOCK_RECOVERY, pbCR_Done);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		if (*pbCR_Done) {
			dptx_dbg("Clock Recovery has been done after %d retrials !!!", (ucCR_Try + 1));
			return DPTX_RETURN_NO_ERROR;
		}
	}

	dptx_warn("Clock Recovery fails.");

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_link_perform_ch_equalization(struct Dptx_Params *pstDptx, bool *pbCR_Done, bool *pbEQ_Done)
{
	uint8_t ucSink_Pattern, ucDPCD_LaneAlign_Status, ucRetry_SinkStatus = 0;
	uint8_t ucEQ_Try;
	int32_t iRetVal;
	uint32_t uiPHY_Ppattern;

	switch (pstDptx->ucMax_Rate) {
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		if (Drm_dp_tps4_supported(pstDptx->aucDPCD_Caps)) {
			uiPHY_Ppattern = DPTX_PHYIF_CTRL_TPS_4;
			ucSink_Pattern = DP_TRAINING_PATTERN_4;
			break;
		}
		if (Drm_dp_tps3_supported(pstDptx->aucDPCD_Caps)) {
			uiPHY_Ppattern = DPTX_PHYIF_CTRL_TPS_3;
			ucSink_Pattern = DP_TRAINING_PATTERN_3;
			break;
		}
		uiPHY_Ppattern = DPTX_PHYIF_CTRL_TPS_2;
		ucSink_Pattern = DP_TRAINING_PATTERN_2;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		if (Drm_dp_tps3_supported(pstDptx->aucDPCD_Caps)) {
			uiPHY_Ppattern = DPTX_PHYIF_CTRL_TPS_3;
			ucSink_Pattern = DP_TRAINING_PATTERN_3;
			break;
		}
		uiPHY_Ppattern = DPTX_PHYIF_CTRL_TPS_2;
		ucSink_Pattern = DP_TRAINING_PATTERN_2;
		break;
	case DPTX_PHYIF_CTRL_RATE_RBR:
	case DPTX_PHYIF_CTRL_RATE_HBR:
		uiPHY_Ppattern = DPTX_PHYIF_CTRL_TPS_2;
		ucSink_Pattern = DP_TRAINING_PATTERN_2;
		break;
	default:
		dptx_err("Invalid rate %d ", pstDptx->stDptxLink.ucLinkRate);
		return DPTX_RETURN_EINVAL;
	}

	dptx_dbg("PHY Pattern(0x%x), Sink Pattern(0x%x)... ", uiPHY_Ppattern, ucSink_Pattern);

	iRetVal = Dptx_Core_Set_PHY_Pattern(pstDptx, uiPHY_Ppattern);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_TRAINING_PATTERN_SET, ucSink_Pattern);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = dptx_link_get_ch_equalization_status(pstDptx, pbCR_Done, pbEQ_Done);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	if (*pbCR_Done == false) {
		dptx_warn("CR fails on EQ Training. ");
		return DPTX_RETURN_NO_ERROR;
	}

	if (*pbEQ_Done) {
		dptx_dbg("Ch EQ has been done at once !!!");
		return DPTX_RETURN_NO_ERROR;
	}

	for (ucEQ_Try = 0; ucEQ_Try < MAX_TRY_CHANNEL_EQ; ucEQ_Try++) {
		iRetVal = dptx_link_adjust_clock_recovery(pstDptx);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		iRetVal = dptx_link_get_ch_equalization_status(pstDptx, pbCR_Done, pbEQ_Done);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		if (*pbCR_Done == false) {
			dptx_warn("CR fails on EQ Training ");
			return DPTX_RETURN_NO_ERROR;
		}

		if (*pbEQ_Done) {
			do {
				iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_LANE_ALIGN_STATUS_UPDATED, &ucDPCD_LaneAlign_Status);
				if (iRetVal != DPTX_RETURN_NO_ERROR) {
					return iRetVal;
				}

				if (ucDPCD_LaneAlign_Status & DP_LINK_STATUS_UPDATED) {
					dptx_dbg("Link status and Adjust Request updated(0x%x) after %dms", ucDPCD_LaneAlign_Status, ucRetry_SinkStatus);
					break;
				}

				if (ucRetry_SinkStatus == MAX_TRY_SINK_LINK_STATUS_UPDATE)
					dptx_info("Link status and Adjust Request NOT updated(0x%x) for 100ms", ucDPCD_LaneAlign_Status);

				mdelay(1);
			} while (ucRetry_SinkStatus++ < MAX_TRY_SINK_UPDATE_STATUS);
		}
	}

	if (*pbEQ_Done == false) {
		dptx_warn("Channel equalization fails");
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_link_initiate_training(struct Dptx_Params *pstDptx)
{
	bool	bSink_EnhancedFraming;
	uint8_t ucSink_BW = 0, ucSink_LaneCountSet = 0;
	uint8_t ucSink_DownSpread = 0, ucSink_ANSI_Coding = 0;
	uint8_t ucSink_TrainingSet = 0;
	uint8_t ucLane_Index;
	uint32_t uiRegMap_Cctl;
	int32_t iRetVal = DPTX_RETURN_NO_ERROR;

	dptx_info("Starting... Num of lanes(%u), Link rate(%u)", pstDptx->stDptxLink.ucNumOfLanes, pstDptx->stDptxLink.ucLinkRate);

	iRetVal = Dptx_Core_Set_PHY_PowerState(pstDptx, PHY_POWER_DOWN_PHY_CLOCK);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Core_Get_PHY_BUSY_Status(pstDptx, pstDptx->stDptxLink.ucNumOfLanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("From Dptx_Core_Get_PHY_BUSY_Status()");
	}

	iRetVal = Dptx_Core_Set_PHY_NumOfLanes(pstDptx, (uint8_t)pstDptx->stDptxLink.ucNumOfLanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Core_Set_PHY_PowerState(pstDptx, PHY_POWER_ON);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Core_Get_PHY_BUSY_Status(pstDptx, pstDptx->stDptxLink.ucNumOfLanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("From Dptx_Core_Get_PHY_BUSY_Status()");
	}

	iRetVal = Dptx_Core_Set_PHY_PowerState(pstDptx, PHY_POWER_DOWN_PHY_CLOCK);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Core_Get_PHY_BUSY_Status(pstDptx, pstDptx->stDptxLink.ucNumOfLanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("From Dptx_Core_Get_PHY_BUSY_Status()");
	}

	iRetVal = Dptx_Core_Set_PHY_Rate(pstDptx, (enum PHY_LINK_RATE)pstDptx->stDptxLink.ucLinkRate);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Core_Set_PHY_PowerState(pstDptx, PHY_POWER_ON);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Core_Get_PHY_BUSY_Status(pstDptx, pstDptx->stDptxLink.ucNumOfLanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("From Dptx_Core_Get_PHY_BUSY_Status()");
	}

	for (ucLane_Index = 0; ucLane_Index < pstDptx->stDptxLink.ucNumOfLanes; ucLane_Index++) {
		ucSink_TrainingSet = 0;

		iRetVal = Dptx_Sec_Set_EQ(pstDptx,
									ucLane_Index,
									pstDptx->stDptxLink.ucLinkRate,
									pstDptx->stDptxLink.aucVoltageSwing_level[ucLane_Index],
									pstDptx->stDptxLink.aucPreEmphasis_level[ucLane_Index]);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		iRetVal = Dptx_Core_Set_PHY_PreEmphasis(pstDptx, ucLane_Index, (enum PHY_PRE_EMPHASIS_LEVEL)pstDptx->stDptxLink.aucPreEmphasis_level[ucLane_Index]);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		iRetVal = Dptx_Core_Set_PHY_VSW(pstDptx, ucLane_Index, (enum PHY_VOLTAGE_SWING_LEVEL)pstDptx->stDptxLink.aucVoltageSwing_level[ucLane_Index]);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}

		ucSink_TrainingSet |= pstDptx->stDptxLink.aucVoltageSwing_level[ucLane_Index];
		ucSink_TrainingSet |= (pstDptx->stDptxLink.aucPreEmphasis_level[ucLane_Index] << DP_TRAIN_PRE_EMPHASIS_SHIFT);
		iRetVal = Dptx_Aux_Write_DPCD(pstDptx, (DP_TRAINING_LANE0_SET + ucLane_Index), ucSink_TrainingSet);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			return iRetVal;
		}
	}

	iRetVal = Dptx_Core_Set_PHY_Pattern(pstDptx, DPTX_PHYIF_CTRL_TPS_NONE);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_TRAINING_PATTERN_SET, DP_TRAINING_PATTERN_DISABLE);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Core_Enable_PHY_XMIT(pstDptx, pstDptx->stDptxLink.ucNumOfLanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Link_PHY_Rate_To_Bandwidth(pstDptx, pstDptx->stDptxLink.ucLinkRate, &ucSink_BW);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_LINK_BW_SET, ucSink_BW);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	ucSink_LaneCountSet = pstDptx->stDptxLink.ucNumOfLanes;
	bSink_EnhancedFraming = Drm_dp_enhanced_frame_cap(pstDptx->aucDPCD_Caps);
	ucSink_LaneCountSet = (bSink_EnhancedFraming) ? (ucSink_LaneCountSet | DP_ENHANCED_FRAME_CAP) : ucSink_LaneCountSet;

	uiRegMap_Cctl = Dptx_Reg_Readl(pstDptx, DPTX_CCTL);
	uiRegMap_Cctl = (bSink_EnhancedFraming) ? (uiRegMap_Cctl | DPTX_CCTL_ENH_FRAME_EN) : (uiRegMap_Cctl & ~DPTX_CCTL_ENH_FRAME_EN);
	Dptx_Reg_Writel(pstDptx, DPTX_CCTL, uiRegMap_Cctl);

	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_LANE_COUNT_SET, ucSink_LaneCountSet);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	if (pstDptx->bSpreadSpectrum_Clock && (pstDptx->aucDPCD_Caps[DP_MAX_DOWNSPREAD] & SINK_TDOWNSPREAD_MASK)) {
		ucSink_DownSpread = DP_SPREAD_AMP_0_5;
	}

	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_DOWNSPREAD_CTRL, ucSink_DownSpread);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	ucSink_ANSI_Coding = DP_SET_ANSI_8B10B;
	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_MAIN_LINK_CHANNEL_CODING_SET, ucSink_ANSI_Coding);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_link_init_training(struct Dptx_Params *pstDptx, uint8_t ucLink_Rate, uint8_t ucNumOfLanes)
{
	uint8_t ucSink_Max_Rate, ucSink_Max_Lanes;
	int32_t ret;

	memset(pstDptx->stDptxLink.aucPreEmphasis_level, 0, sizeof(uint8_t) * PRE_EMPHASIS_LEVEL_MAX);
	memset(pstDptx->stDptxLink.aucVoltageSwing_level, 0, sizeof(uint8_t) * VOLTAGE_SWING_LEVEL_MAX);
	memset(pstDptx->stDptxLink.aucTraining_Status, 0, DP_LINK_STATUS_SIZE);

	ucSink_Max_Lanes = Drm_dp_max_lane_count(pstDptx->aucDPCD_Caps);

	ret = Dptx_Link_Bandwidth_To_PHY_Rate(pstDptx, pstDptx->aucDPCD_Caps[DP_MAX_LINK_RATE], &ucSink_Max_Rate);
	if (ret == DPTX_RETURN_NO_ERROR) {
		pstDptx->stDptxLink.ucNumOfLanes = (ucNumOfLanes > ucSink_Max_Lanes) ? ucSink_Max_Lanes : ucNumOfLanes;
		pstDptx->stDptxLink.ucLinkRate = (ucLink_Rate > ucSink_Max_Rate) ? ucSink_Max_Rate : ucLink_Rate;

		dptx_info("Init link training :");
		dptx_info("  Src dev %d lanes <-> Sink dev %d lanes", (uint32_t)ucNumOfLanes, (uint32_t)ucSink_Max_Lanes);
		dptx_info("  Src rate = %s  <-> Sink rate = %s\n",
					ucLink_Rate == DPTX_PHYIF_CTRL_RATE_RBR ? "RBR" :
					(ucLink_Rate == DPTX_PHYIF_CTRL_RATE_HBR) ? "HBR" :
					(ucLink_Rate == DPTX_PHYIF_CTRL_RATE_HBR2) ? "HBR2":"HBR3",
					ucSink_Max_Rate == DPTX_PHYIF_CTRL_RATE_RBR ? "RBR" :
					(ucSink_Max_Rate == DPTX_PHYIF_CTRL_RATE_HBR) ?  "HBR" :
					(ucSink_Max_Rate == DPTX_PHYIF_CTRL_RATE_HBR2) ? "HBR2":"HBR3");

		if (pstDptx->ucMax_Rate != pstDptx->stDptxLink.ucLinkRate) {
			dptx_info("Reducing Link rate = %s -> Sink one = %s\n",
					pstDptx->ucMax_Rate == DPTX_PHYIF_CTRL_RATE_RBR ? "RBR" :
					(pstDptx->ucMax_Rate == DPTX_PHYIF_CTRL_RATE_HBR) ?  "HBR" :
					(pstDptx->ucMax_Rate == DPTX_PHYIF_CTRL_RATE_HBR2) ? "HBR2":"HBR3",
					pstDptx->stDptxLink.ucLinkRate == DPTX_PHYIF_CTRL_RATE_RBR ? "RBR" :
					(pstDptx->stDptxLink.ucLinkRate == DPTX_PHYIF_CTRL_RATE_HBR) ?  "HBR" :
					(pstDptx->stDptxLink.ucLinkRate == DPTX_PHYIF_CTRL_RATE_HBR2) ? "HBR2":"HBR3");

			ret = Dptx_Cfg_Reset(pstDptx, pstDptx->stDptxLink.ucLinkRate);
			if (ret == DPTX_RETURN_NO_ERROR) {
				if (pstDptx->ePhy_Dev == PHY_DEVICE_SEC) {
					ret = Dptx_Sec_PHY_Init(pstDptx, pstDptx->stDptxLink.ucLinkRate, pstDptx->stDptxLink.ucNumOfLanes);
				}
			}
			if (ret == DPTX_RETURN_NO_ERROR) {
				ret = Dptx_Core_Init(pstDptx);
			}
			if (ret == DPTX_RETURN_NO_ERROR) {
				ret = Dptx_Ext_Set_Stream_Capability(pstDptx);
			}
		}
	}
	return ret;
}

int32_t Dptx_Link_Perform_Training(struct Dptx_Params *pstDptx, uint8_t ucRate, uint8_t ucNumOfLanes)
{
	bool bCR_Done = false, bEQ_Done = false;
	bool bRate_Reduced = false, bLane_Reduced = false;
	uint8_t ucDPCD_SinkCount, ucMST_Mode_Caps;
	int32_t iRetVal;

	iRetVal = dptx_link_init_training(pstDptx, ucRate, ucNumOfLanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* For KCS */
		goto fail;
	}

again:
	iRetVal = dptx_link_initiate_training(pstDptx);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* For KCS */
		goto fail;
	}

	iRetVal = dptx_link_perform_clock_recovery(pstDptx, &bCR_Done);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		/* For KCS */
		goto fail;
	}

	if (!bCR_Done) {
		iRetVal = dptx_link_reduce_rate(pstDptx, &bRate_Reduced);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			goto fail;
		}

		if (!bRate_Reduced) {
			if(!(pstDptx->stDptxLink.aucTraining_Status[PHY_LANE_0] & DP_LANE_CR_DONE)) {
				dptx_err("Clock Recovery of Lane 0 has been failed !!!" );
				goto fail;
			}

			iRetVal = dptx_link_reduce_lanes(pstDptx, &bLane_Reduced);
			if (iRetVal != DPTX_RETURN_NO_ERROR) {
				goto fail;
			}

			if (!bLane_Reduced) {
				goto fail;
			}
		}

		dptx_link_init_training(pstDptx, pstDptx->stDptxLink.ucLinkRate, pstDptx->stDptxLink.ucNumOfLanes);
		goto again;
	}

	iRetVal = dptx_link_perform_ch_equalization(pstDptx, &bCR_Done, &bEQ_Done);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		goto fail;
	}

	if (!bCR_Done || !bEQ_Done) {
		iRetVal = dptx_link_reduce_rate(pstDptx, &bRate_Reduced);
		if (iRetVal != DPTX_RETURN_NO_ERROR) {
			goto fail;
		}

		if (!bRate_Reduced) {
			if(!(pstDptx->stDptxLink.aucTraining_Status[PHY_LANE_0] & DP_LANE_CR_DONE)) {
				dptx_err("Clock Recovery of Lane 0 has been failed !!!" );
				goto fail;
			}

			iRetVal = dptx_link_reduce_lanes(pstDptx, &bLane_Reduced);
			if (iRetVal != DPTX_RETURN_NO_ERROR) {
				goto fail;
			}

			if (!bLane_Reduced) {
				goto fail;
			}
		}

		dptx_link_init_training(pstDptx, pstDptx->stDptxLink.ucLinkRate, pstDptx->stDptxLink.ucNumOfLanes);
		goto again;
	}

	iRetVal = Dptx_Core_Set_PHY_Pattern(pstDptx, DPTX_PHYIF_CTRL_TPS_NONE);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_TRAINING_PATTERN_SET, DP_TRAINING_PATTERN_DISABLE);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_MSTM_CAP, &ucMST_Mode_Caps);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Core_Enable_PHY_XMIT(pstDptx, pstDptx->stDptxLink.ucNumOfLanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_SINK_COUNT, &ucDPCD_SinkCount);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	iRetVal = Dptx_Aux_Read_DPCD(pstDptx, 0x2002, &ucDPCD_SinkCount);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		return iRetVal;
	}

	dptx_info("Link training succeeded with %s - %s",
				pstDptx->stDptxLink.ucLinkRate == DPTX_PHYIF_CTRL_RATE_RBR ? "RBR" :
				(pstDptx->stDptxLink.ucLinkRate == DPTX_PHYIF_CTRL_RATE_HBR) ? "HBR" :
				(pstDptx->stDptxLink.ucLinkRate == DPTX_PHYIF_CTRL_RATE_HBR2) ? "HBR2":"HBR3",
				pstDptx->stDptxLink.ucNumOfLanes == 1U ? "1 lane" :
				(pstDptx->stDptxLink.ucNumOfLanes == 2U) ?  "2 lanes" :"4 lanes");

	return DPTX_RETURN_NO_ERROR;

fail:
	Dptx_Core_Set_PHY_Pattern(pstDptx, DPTX_PHYIF_CTRL_TPS_NONE);
	Dptx_Aux_Write_DPCD(pstDptx, DP_TRAINING_PATTERN_SET, DP_TRAINING_PATTERN_DISABLE);

	dptx_err("Failed link training !!!");
	return DPTX_RETURN_ENODEV;
}

int32_t Dptx_Link_PHY_Rate_To_Bandwidth(struct Dptx_Params *pstDptx, uint8_t ucRate, uint8_t *pucBandWidth)
{
	switch (ucRate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		*pucBandWidth = DP_LINK_BW_1_62;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		*pucBandWidth = DP_LINK_BW_2_7;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		*pucBandWidth = DP_LINK_BW_5_4;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		*pucBandWidth = DP_LINK_BW_8_1;
		break;
	default:
		dptx_err("Invalid rate %d ", (uint32_t)ucRate);
		return DPTX_RETURN_EINVAL;
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Link_Bandwidth_To_PHY_Rate(struct Dptx_Params *pstDptx, uint8_t ucBandWidth, uint8_t *pucRate)
{
	switch (ucBandWidth) {
	case DP_LINK_BW_1_62:
		*pucRate = DPTX_PHYIF_CTRL_RATE_RBR;
		break;
	case DP_LINK_BW_2_7:
		*pucRate = DPTX_PHYIF_CTRL_RATE_HBR;
		break;
	case DP_LINK_BW_5_4:
		*pucRate = DPTX_PHYIF_CTRL_RATE_HBR2;
		break;
	case DP_LINK_BW_8_1:
		*pucRate = DPTX_PHYIF_CTRL_RATE_HBR3;
		break;
	default:
		dptx_err("Invalid link rate -> %d ", (uint32_t)ucBandWidth);
		return DPTX_RETURN_EINVAL;
	}

	return DPTX_RETURN_NO_ERROR;
}

