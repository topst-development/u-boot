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

#define NUM_OF_CLEAR_VC_PAYLOAD_IDS	3
#define NUM_OF_MST_VCP_TABLEs 		8

#define DPTX_EDID_MAX_EXTRA_BLK 	4
#define DPTX_EDID_EXT_BLK_FIELD 	126

#define DPCD_DOWN_REP_SIZE 		256u
#define MAX_MSG_BUFFER_SIZE 		256u

#define MAX_CHECK_DPCD_VCP_UPDATED 	500
#define MAX_NUMBER_TO_WAIT_MSG_REPLY 	500
#define MAX_CHECK_MST_ACT 		10

#define MAX_NUM_OF_SUB_BRANCH 		2

struct sideband_msg_reply_data {
	uint8_t sb_replay_msg[DPCD_DOWN_REP_SIZE];
	uint8_t sb_replay_msg_length;
	struct  drm_dp_sideband_msg_hdr sb_msg_header;
};

struct Dptx_Topology_Params {
	struct drm_dp_sideband_msg_rx stMainBranch_Msg_Rx;
	struct drm_dp_sideband_msg_reply_body stMainBranch_Msg_Reply;
	struct drm_dp_sideband_msg_rx stSubBranch_Msg_Rx[MAX_NUM_OF_SUB_BRANCH];
	struct drm_dp_sideband_msg_reply_body
		stSubBranch_Msg_Reply[MAX_NUM_OF_SUB_BRANCH];
};

struct Dptx_Topology_Params stDptx_Topology_Params;

static int32_t dptx_ext_wait_sideband_msg_reply_ready(struct Dptx_Params *pstDptx)
{
	uint8_t svc_irq_vector_esi0, svc_irq_vector;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;
	uint32_t count = 0;
	bool is_loop_done = false;

	while (!is_loop_done) {
		ret_val = Dptx_Aux_Read_DPCD(pstDptx,
					     DP_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
					     &svc_irq_vector_esi0);
		if (ret_val != DPTX_RETURN_NO_ERROR) {
			is_loop_done = true;
		}

		if (!is_loop_done) {
			ret_val = Dptx_Aux_Read_DPCD(
				pstDptx, DP_DEVICE_SERVICE_IRQ_VECTOR,
				&svc_irq_vector);
			if (ret_val != DPTX_RETURN_NO_ERROR) {
				is_loop_done = true;
			}
		}

		if (!is_loop_done) {
			if ((svc_irq_vector & DP_DOWN_REP_MSG_RDY) ||
			    (svc_irq_vector_esi0 & DP_DOWN_REP_MSG_RDY)) {
				is_loop_done = true;
				ret_val = DPTX_RETURN_NO_ERROR;
			}
		}

		if (!is_loop_done) {
			if (count++ > MAX_NUMBER_TO_WAIT_MSG_REPLY) {
				dptx_err(
					"Timed out for %d ms from sideband message reply ready",
					count);
				is_loop_done = true;
				ret_val = DPTX_RETURN_ENODEV;
			}
		}

		if (!is_loop_done) {
			mdelay(1);
		}
	}

	return ret_val;
}

static int32_t dptx_ext_clear_sideband_msg_reply(struct Dptx_Params *pstDptx)
{
	uint8_t svc_irq_vector_esi0, svc_irq_vector;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;
	uint32_t count = 0;
	bool is_loop_done = false;

	while (!is_loop_done) {
		ret_val = Dptx_Aux_Read_DPCD(pstDptx,
					     DP_DEVICE_SERVICE_IRQ_VECTOR,
					     &svc_irq_vector);
		if (ret_val != DPTX_RETURN_NO_ERROR) {
			is_loop_done = true;
		}

		if (!is_loop_done) {
			ret_val = Dptx_Aux_Read_DPCD(
				pstDptx, DP_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
				&svc_irq_vector_esi0);
			if (ret_val != DPTX_RETURN_NO_ERROR) {
				is_loop_done = true;
			}
		}

		if (!is_loop_done) {
			if (!(svc_irq_vector & DP_DOWN_REP_MSG_RDY ||
			      svc_irq_vector_esi0 & DP_DOWN_REP_MSG_RDY)) {
				is_loop_done = true;
				ret_val = DPTX_RETURN_NO_ERROR;
			}
		}

		if (!is_loop_done) {
			ret_val = Dptx_Aux_Write_DPCD(
				pstDptx, DP_DEVICE_SERVICE_IRQ_VECTOR,
				DP_DOWN_REP_MSG_RDY);
			if (ret_val != DPTX_RETURN_NO_ERROR) {
				is_loop_done = true;
			}
		}

		if (!is_loop_done) {
			ret_val = Dptx_Aux_Write_DPCD(
				pstDptx, DP_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
				svc_irq_vector_esi0);
			if (ret_val != DPTX_RETURN_NO_ERROR) {
				is_loop_done = true;
			}
		}

		if (!is_loop_done) {
			if (count++ > MAX_NUMBER_TO_WAIT_MSG_REPLY) {
				dptx_err(
					"Timed out for %d ms from clear sideband message reply",
					count);
				is_loop_done = true;
				ret_val = DPTX_RETURN_I2C_OVER_AUX_NO_ACK;
			}
		}

		if (!is_loop_done) {
			mdelay(1);
		}
	}

	return ret_val;
}

static int32_t dptx_ext_process_sideband_msg_down_req_reply(struct Dptx_Params *dev_param,
							    uint8_t request_id,
							    struct sideband_msg_reply_data *reply_data,
							    bool need_check_somt)
{
	uint8_t sb_msg_buf[DPCD_DOWN_REP_SIZE] = {0, };
	uint8_t sb_msg_header_length = 0u;

	int32_t ret = DPTX_RETURN_NO_ERROR;

	ret = dptx_ext_wait_sideband_msg_reply_ready(dev_param);
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Aux_Read_Bytes_From_DPCD(dev_param,
						    DP_SIDEBAND_MSG_DOWN_REP_BASE,
						    sb_msg_buf, DPCD_DOWN_REP_SIZE);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Drm_Addition_Decode_Sideband_Msg_Hdr(&reply_data->sb_msg_header,
							   sb_msg_buf, DPCD_DOWN_REP_SIZE,
							   &sb_msg_header_length);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		dptx_dbg("\nRequest id(%d): lct=%d, lcr=%d",
			 request_id, reply_data->sb_msg_header.lct, reply_data->sb_msg_header.lcr);
		dptx_dbg("\nrad=0x%x, bcast=%d", (reply_data->sb_msg_header.lct > 1u) ? reply_data->sb_msg_header.rad[0] : 0u,
						reply_data->sb_msg_header.broadcast);
		dptx_dbg("\npath=%d, msglen=%d, somt=%d, eomt=%d, seqno=%d",
			reply_data->sb_msg_header.path_msg, reply_data->sb_msg_header.msg_len,
			reply_data->sb_msg_header.somt, reply_data->sb_msg_header.eomt, reply_data->sb_msg_header.seqno);

		if (reply_data->sb_msg_header.msg_len < 2u) {
			dptx_err("sideband message header lenght %u is not valid",
				  reply_data->sb_msg_header.msg_len);
			ret = -DPTX_RETURN_EINVAL;
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		if ((DPCD_DOWN_REP_SIZE - reply_data->sb_replay_msg_length) < (reply_data->sb_msg_header.msg_len - 1u)) {
			dptx_err("Potential sideband reply message buffer overflow: sb_replay_msg_length(%u) + sb_msg_header.msg_len(%u)",
				  reply_data->sb_replay_msg_length,
				  reply_data->sb_msg_header.msg_len);
			ret = -DPTX_RETURN_EINVAL;
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		reply_data->sb_msg_header.msg_len -= 1u;

		(void)memcpy(&reply_data->sb_replay_msg[reply_data->sb_replay_msg_length], &sb_msg_buf[sb_msg_header_length], reply_data->sb_msg_header.msg_len);

		reply_data->sb_replay_msg_length += reply_data->sb_msg_header.msg_len;

		if (need_check_somt && !reply_data->sb_msg_header.somt) {
			/*
			 * In the header of the first sideband message,
			 * the Start of Message Transaction (SMT) bit must be set.
			 */
			dptx_err("The First Sideband MSG dose not set SMT bit");
			ret = -DPTX_RETURN_EBUSY;
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Aux_Write_DPCD(dev_param, DP_DEVICE_SERVICE_IRQ_VECTOR, DP_DOWN_REP_MSG_RDY);
	}
	return ret;
}

static int32_t dptx_ext_get_sideband_msg_down_req_reply(struct Dptx_Params *dev_param,
							uint8_t request_id,
							uint8_t sb_out_msg[256],
							uint8_t *sb_out_msg_length)
{

	struct sideband_msg_reply_data replay_data;
	bool need_check_somt = true;

	uint32_t down_req_replay_retry;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	*sb_out_msg_length = 0u;

	for (down_req_replay_retry = 0u; down_req_replay_retry < 3u; down_req_replay_retry++) {
		(void)memset(&replay_data, 0, sizeof(replay_data));

		while (true) {
			ret = dptx_ext_process_sideband_msg_down_req_reply(dev_param,
									   request_id,
									   &replay_data,
									   need_check_somt);
			if (DPTX_RETURN_SUCCESS(ret)) {
				need_check_somt = false;
				if (!replay_data.sb_msg_header.eomt) {
					continue;
				}
			}
			break;
		}
		if (DPTX_RETURN_SUCCESS(ret)) {
			if ((replay_data.sb_replay_msg[0] & 0x7fu) != request_id) {
				if (down_req_replay_retry < 3) {
					dptx_warn("request_id %u does not match expected %u, retrying", replay_data.sb_replay_msg[0] & 0x7fu, request_id);
					dptx_err("request_id %u does not match expected %u, retrying", replay_data.sb_replay_msg[0] & 0x7fu, request_id);
					continue;
				}
			}
		}
		break;
	}
	if (down_req_replay_retry >= 3u) {
		dptx_err("request_id %u does not match expected %u, giving up", replay_data.sb_replay_msg[0] & 0x7fu, request_id);
		ret = -DPTX_RETURN_EBUSY;
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = dptx_ext_clear_sideband_msg_reply(dev_param);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		if (sb_out_msg != NULL) {
			(void)memcpy(sb_out_msg, replay_data.sb_replay_msg, replay_data.sb_replay_msg_length);
		}
		*sb_out_msg_length = replay_data.sb_replay_msg_length;
	}
	return ret;
}

static int32_t dptx_ext_set_sideband_msg_enum_path_resources(
	struct Dptx_Params *pstDptx, uint8_t ucStreamSink_PortNum,
	uint8_t ucVCP_Id, uint8_t ucRAD_PortNum)
{
	uint8_t ucReply_Len;
	uint8_t aucMsg_Buf[256];
	uint8_t *msg;
	int32_t msg_len = 256;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;
	struct drm_dp_sideband_msg_hdr stMsg_Header;

	memset(&stMsg_Header, 0, sizeof(struct drm_dp_sideband_msg_hdr));

	dptx_dbg("Port %d for allocate: RAD Port = %d", ucStreamSink_PortNum,
		 ucRAD_PortNum);

	stMsg_Header.lct = 1;
	stMsg_Header.lcr = 0;
	stMsg_Header.rad[0] = 0;
	stMsg_Header.broadcast = false;
	stMsg_Header.path_msg = 0;
	stMsg_Header.msg_len = 3;
	stMsg_Header.somt = 1;
	stMsg_Header.eomt = 1;
	stMsg_Header.seqno = 0;

	if (ucRAD_PortNum != INVALID_MST_PORT_NUM) {
		stMsg_Header.lct = 2;
		stMsg_Header.lcr = 1;
		stMsg_Header.rad[0] |= ((ucRAD_PortNum << 4) & 0xF0);
	}

	Drm_Addition_Encode_Sideband_Msg_Hdr(&stMsg_Header, aucMsg_Buf,
					     &msg_len);

	msg = &aucMsg_Buf[msg_len];
	msg[0] = DP_ENUM_PATH_RESOURCES;
	msg[1] = ((ucStreamSink_PortNum & 0xF) << 4);

	Drm_Addition_Encode_SideBand_Msg_CRC(msg, 2);

	msg_len += 3;

	ret_val = Dptx_Aux_Write_Bytes_To_DPCD(
		pstDptx, DP_SIDEBAND_MSG_DOWN_REQ_BASE, aucMsg_Buf, msg_len);
	if (ret_val == DPTX_RETURN_NO_ERROR) {
		ret_val = dptx_ext_get_sideband_msg_down_req_reply(
			pstDptx, DP_ENUM_PATH_RESOURCES, (uint8_t *)NULL, &ucReply_Len);
	}

	return ret_val;
}

static int32_t dptx_ext_set_sideband_msg_allocate_payload(
	struct Dptx_Params *pstDptx, uint8_t ucStreamSink_PortNum,
	uint8_t ucVCP_Id, uint16_t usPBN, uint8_t ucRAD_PortNum)
{
	uint8_t ucReply_Len;
	uint8_t aucMsg_Buf[256];
	uint8_t *msg;
	int32_t msg_len = 256;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;
	struct drm_dp_sideband_msg_hdr stMsg_Header;

	memset(&stMsg_Header, 0, sizeof(struct drm_dp_sideband_msg_hdr));

	dptx_dbg("Port %d allocates, VCP Id= %d, RAD Port = %d",
		 ucStreamSink_PortNum, ucVCP_Id, ucRAD_PortNum);

	stMsg_Header.lct = 1;
	stMsg_Header.lcr = 0;
	stMsg_Header.rad[0] = 0;
	stMsg_Header.broadcast = false;
	stMsg_Header.path_msg = 1;
	stMsg_Header.msg_len = 6;
	stMsg_Header.somt = 1;
	stMsg_Header.eomt = 1;
	stMsg_Header.seqno = 0;

	if (ucRAD_PortNum != INVALID_MST_PORT_NUM) {
		stMsg_Header.lct = 2;
		stMsg_Header.lcr = 1;
		stMsg_Header.rad[0] |= ((ucRAD_PortNum << 4) & 0xF0);
	}

	Drm_Addition_Encode_Sideband_Msg_Hdr(&stMsg_Header, aucMsg_Buf,
					     &msg_len);

	msg = &aucMsg_Buf[msg_len];

	msg[0] = DP_ALLOCATE_PAYLOAD;
	msg[1] = ((ucStreamSink_PortNum & 0xF) << 4);
	msg[2] = (ucVCP_Id & 0x7F);
	msg[3] = (usPBN >> 8);
	msg[4] = (usPBN & 0xFF);

	Drm_Addition_Encode_SideBand_Msg_CRC(msg, 5);

	msg_len += 6;

	ret_val = Dptx_Aux_Write_Bytes_To_DPCD(
		pstDptx, DP_SIDEBAND_MSG_DOWN_REQ_BASE, aucMsg_Buf, msg_len);
	if (ret_val == DPTX_RETURN_NO_ERROR) {
		ret_val = dptx_ext_get_sideband_msg_down_req_reply(
			pstDptx, DP_ALLOCATE_PAYLOAD, (uint8_t *)NULL, &ucReply_Len);
	}

	return ret_val;
}

static int32_t dptx_ext_set_sideband_msg_link_address(
	struct Dptx_Params *dptx,
	struct drm_dp_sideband_msg_rx *pstSideband_Msg_Rx,
	struct drm_dp_sideband_msg_reply_body *pstSideband_Msg_Reply,
	uint8_t ucPort_ConnectToBranch)
{
	uint8_t msg_header_buf[256];
	uint8_t *msg;
	int32_t msg_len = 256;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;
	struct drm_dp_sideband_msg_hdr stMsg_Header;
	bool internal_ok = true;

	memset(&stMsg_Header, 0, sizeof(struct drm_dp_sideband_msg_hdr));

	stMsg_Header.lct = 1;
	stMsg_Header.lcr = 0;
	stMsg_Header.rad[0] = 0;
	stMsg_Header.broadcast = false;
	stMsg_Header.path_msg = 0;
	stMsg_Header.msg_len = 2;
	stMsg_Header.somt = 1;
	stMsg_Header.eomt = 1;
	stMsg_Header.seqno = 0;

	if (ucPort_ConnectToBranch != INVALID_MST_PORT_NUM) {
		stMsg_Header.lct = 2;
		stMsg_Header.lcr = 1;
		stMsg_Header.rad[0] |= ((ucPort_ConnectToBranch << 4) & 0xF0);
	}

	Drm_Addition_Encode_Sideband_Msg_Hdr(&stMsg_Header, msg_header_buf,
					     &msg_len);

	msg = &msg_header_buf[msg_len];
	msg[0] = DP_LINK_ADDRESS;

	Drm_Addition_Encode_SideBand_Msg_CRC(msg, 1);

	msg_len += 2;

	if (msg_len < 0) {
		ret_val = DPTX_RETURN_EINVAL;
		internal_ok = false;
	}

	if (internal_ok) {
		ret_val = Dptx_Aux_Write_Bytes_To_DPCD(
			dptx, DP_SIDEBAND_MSG_DOWN_REQ_BASE, msg_header_buf, msg_len);
		if (ret_val != DPTX_RETURN_NO_ERROR) {
			internal_ok = false;
		}
	}

	if (internal_ok) {
		ret_val = dptx_ext_get_sideband_msg_down_req_reply(
			dptx, DP_LINK_ADDRESS, pstSideband_Msg_Rx->msg,
			(uint8_t *)&msg_len);
		if (ret_val != DPTX_RETURN_NO_ERROR) {
			internal_ok = false;
		}
	}

	if (internal_ok) {
		pstSideband_Msg_Rx->curlen = msg_len;

		ret_val = Drm_Addition_Parse_Sideband_Link_Address(
			pstSideband_Msg_Rx, pstSideband_Msg_Reply);
		if (ret_val != DPTX_RETURN_NO_ERROR) {
			dptx_err(
				"Error from Drm_Addition_Parse_Sideband_Link_Address()");
			internal_ok = false;
		}
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_get_link_numof_slots(struct Dptx_Params *pstDptx,
					     uint16_t usPBN,
					     uint8_t *pucNumOfSlots)
{
	uint8_t ucLink_BandWidth;
	uint32_t uiDivider;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	ret_val = Dptx_Link_PHY_Rate_To_Bandwidth(
		pstDptx, pstDptx->stDptxLink.ucLinkRate, &ucLink_BandWidth);
	if (ret_val != DPTX_RETURN_NO_ERROR) {
		return ret_val;
	}

	switch (ucLink_BandWidth) {
	case DP_LINK_BW_1_62:
		uiDivider = (3 * pstDptx->stDptxLink.ucNumOfLanes);
		break;
	case DP_LINK_BW_2_7:
		uiDivider = (5 * pstDptx->stDptxLink.ucNumOfLanes);
		break;
	case DP_LINK_BW_5_4:
		uiDivider = (10 * pstDptx->stDptxLink.ucNumOfLanes);
		break;
	case DP_LINK_BW_8_1:
		uiDivider = (15 * pstDptx->stDptxLink.ucNumOfLanes);
		break;
	default:
		dptx_err("Invalid index of BW : %d",
			 (uint32_t)ucLink_BandWidth);
		return DPTX_RETURN_EINVAL;
	}

	*pucNumOfSlots = DIV_ROUND_UP(usPBN, uiDivider);

	return DPTX_RETURN_NO_ERROR;
}

#if defined(ANALYZER_TEST_CODE)
static bool dptx_ext_is_sink_vcpid_table_zero(struct Dptx_Params *pstDptx)
{
	const uint32_t payload_slot_count = 0x3fu;
	uint8_t payload_slots[payload_slot_count];
	uint32_t loop;
	bool table_is_zero = false;

	int ret = DPTX_RETURN_NO_ERROR;

	ret = Dptx_Aux_Read_Bytes_From_DPCD(
		pstDptx, (uint32_t)DP_PAYLOAD_TABLE_UPDATE_STATUS + 1u,
		payload_slots, payload_slot_count);

	if (DPTX_RETURN_SUCCESS(ret)) {
		for (loop = 0u; loop < payload_slot_count; loop++) {
			if (payload_slots[loop] != 0u) {
				break;
			}
		}
		if (loop == payload_slot_count) {
			table_is_zero = true;
		}
	}
	return table_is_zero;
}
#endif

static int32_t dptx_ext_clear_sink_vcpid_table(struct Dptx_Params *pstDptx)
{
	uint8_t aucPayload_Allocate_Set[NUM_OF_CLEAR_VC_PAYLOAD_IDS] = {0x00,
									0x00,
									0x3F};
	uint8_t ucPayload_Updated_Status;
	int32_t uiRetry_LinkUpdated = 0;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	ret_val = Dptx_Aux_Write_DPCD(pstDptx, DP_PAYLOAD_TABLE_UPDATE_STATUS,
				      DP_PAYLOAD_TABLE_UPDATED);
	if (ret_val != DPTX_RETURN_NO_ERROR)
		return ret_val;

	ret_val = Dptx_Aux_Write_Bytes_To_DPCD(pstDptx, DP_PAYLOAD_ALLOCATE_SET,
					       aucPayload_Allocate_Set,
					       NUM_OF_CLEAR_VC_PAYLOAD_IDS);
	if (ret_val != DPTX_RETURN_NO_ERROR)
		return ret_val;

	while (true) {
		ret_val = Dptx_Aux_Read_DPCD(pstDptx,
					     DP_PAYLOAD_TABLE_UPDATE_STATUS,
					     &ucPayload_Updated_Status);
		if (ret_val != DPTX_RETURN_NO_ERROR) {
			return ret_val;
		}

		if (ucPayload_Updated_Status & DP_PAYLOAD_TABLE_UPDATED) {
			break;
		}

#if defined(ANALYZER_TEST_CODE)
		if (dptx_ext_is_sink_vcpid_table_zero(pstDptx)) {
			break;
		}
#endif

		if (uiRetry_LinkUpdated++ > MAX_CHECK_DPCD_VCP_UPDATED) {
			dptx_err(
				"Payload table in Sink is not updated for %dms",
				(uiRetry_LinkUpdated * 1));
			break;
		}

		mdelay(1);
	}

	ret_val = Dptx_Aux_Write_DPCD(pstDptx, DP_PAYLOAD_TABLE_UPDATE_STATUS,
				      DP_PAYLOAD_TABLE_UPDATED);
	if (ret_val != DPTX_RETURN_NO_ERROR) {
		return ret_val;
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_clear_link_vcp_tables(struct Dptx_Params *pstDptx)
{
	uint8_t ucElements;

	for (ucElements = 0; ucElements < NUM_OF_MST_VCP_TABLEs; ucElements++) {
		Dptx_Reg_Writel(pstDptx, DPTX_MST_VCP_TABLE_REG_N(ucElements),
				0);
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_set_link_vcpid_table_slot(struct Dptx_Params *pstDptx,
						  uint8_t ucStart_SlotNum,
						  uint8_t ucNumOfSlots,
						  uint8_t ucStream_Index)
{
	uint8_t ucSlotNum, ucCount;
	uint32_t uiVCP_RegOffset, uiRegMap_VCPTable;
	uint32_t uiBit_Shift = 0, uiBit_Mask = 0;

	if ((ucStart_SlotNum + ucNumOfSlots) > DPTX_MAX_LINK_SLOTS) {
		dptx_err(
			"Start Slot(%d) + Num of Slots(%d) is larger than max slots(%d)",
			ucStart_SlotNum, ucNumOfSlots,
			(uint32_t)DPTX_MAX_LINK_SLOTS);
		return DPTX_RETURN_EINVAL;
	}

	dptx_dbg("----- Setting %d slots for stream %d", ucStart_SlotNum,
		 ucStream_Index);

	for (ucCount = 0; ucCount < ucNumOfSlots; ucCount++) {
		ucSlotNum = (ucStart_SlotNum + ucCount);

		uiBit_Shift = ((ucSlotNum & 0x7u) * 4u);
		uiBit_Mask = GENMASK((uiBit_Shift + 3u), uiBit_Shift);

		uiVCP_RegOffset = DPTX_MST_VCP_TABLE_REG_N(ucSlotNum >> 3);
		uiRegMap_VCPTable = Dptx_Reg_Readl(pstDptx, uiVCP_RegOffset);

		uiRegMap_VCPTable &= ~uiBit_Mask;
		uiRegMap_VCPTable |=
			(((uint32_t)ucStream_Index << uiBit_Shift) & uiBit_Mask);

		Dptx_Reg_Writel(pstDptx, uiVCP_RegOffset, uiRegMap_VCPTable);
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_set_sink_vcpid_table_slot(struct Dptx_Params *pstDptx,
						  uint8_t ucStart_SlotNum,
						  uint8_t ucNumOfSlots,
						  uint8_t ucStream_Index)
{
	uint8_t ucStatus, ucWriteBuf[3];
	int32_t ret_val;
	uint32_t uiRetry_LinkUpdated = 0;

	ucWriteBuf[0] = ucStream_Index;
	ucWriteBuf[1] = ucStart_SlotNum;
	ucWriteBuf[2] = ucNumOfSlots;

	dptx_dbg("----- Setting %d slots for stream %d", ucStart_SlotNum,
		 ucStream_Index);

	ret_val = Dptx_Aux_Write_Bytes_To_DPCD(pstDptx, DP_PAYLOAD_ALLOCATE_SET,
					       ucWriteBuf, 3);
	if (ret_val != DPTX_RETURN_NO_ERROR) {
		return ret_val;
	}

	do {
		ret_val = Dptx_Aux_Read_DPCD(
			pstDptx, DP_PAYLOAD_TABLE_UPDATE_STATUS, &ucStatus);
		if (ret_val != DPTX_RETURN_NO_ERROR) {
			return ret_val;
		}

		if (ucStatus & DP_PAYLOAD_TABLE_UPDATED) {
			break;
		}

		if (uiRetry_LinkUpdated == MAX_CHECK_DPCD_VCP_UPDATED) {
			dptx_err(
				"Payload table in Sink is not updated for %dms",
				(uiRetry_LinkUpdated * 1));
		}

		udelay(1);
	} while (uiRetry_LinkUpdated++ < MAX_CHECK_DPCD_VCP_UPDATED);

	ret_val = Dptx_Aux_Write_DPCD(pstDptx, DP_PAYLOAD_TABLE_UPDATE_STATUS,
				      DP_PAYLOAD_TABLE_UPDATED);
	if (ret_val != DPTX_RETURN_NO_ERROR) {
		return ret_val;
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t
Dptx_Ext_Get_Link_PayloadBandwidthNumber(struct Dptx_Params *dev_param,
					 uint8_t dp_stream_id)
{

	uint32_t peak_pixel_bandwidth, pbn_margin, pbn_margin_mhz,
		 pbn_margin_tenfold, pbn_margin_fraction;
	const struct dptx_video_params *video_params = NULL;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	if (dp_stream_id >= (uint32_t)PHY_INPUT_STREAM_MAX) {
		dptx_err("DisplayPort stream id %u is out of range ", dp_stream_id);
		ret = -DPTX_RETURN_EINVAL;
	} else {
		video_params = &dev_param->video_params[dp_stream_id];

		if (video_params->pixel_clock > DPTX_PIXEL_CLOCK_KHZ_MAX) {
			/* 600MHz is maximum PCLK for DDI BUS */
			dptx_err("pixel_clock_khz %uKHz is out of range", video_params->pixel_clock);
			ret = -DPTX_RETURN_EINVAL;
		} else {
			switch (video_params->pixel_encoding) {
			case PIXEL_ENCODING_TYPE_RGB:
			case PIXEL_ENCODING_TYPE_YCBCR444:
				peak_pixel_bandwidth = ((video_params->pixel_clock *
							(uint32_t)VIDEO_LINK_BPP_RGB_YCbCr444 * 64u) / 54u);
				dptx_info("RGB video_params[%u] pclk = %u, peak_pixel_bandwidth = %u\n",
					dp_stream_id, video_params->pixel_clock, peak_pixel_bandwidth);
				break;
			case PIXEL_ENCODING_TYPE_YCBCR422:
				peak_pixel_bandwidth = ((video_params->pixel_clock *
							(uint32_t)VIDEO_LINK_BPP_YCbCr422 * 64u) / 54u);
				dptx_info("422 video_params[%u] pclk = %u, peak_pixel_bandwidth = %u\n",
					dp_stream_id, video_params->pixel_clock, peak_pixel_bandwidth);
				break;
			default:
				dptx_err("Unknown Pixel encoding type(%d)",
					video_params->pixel_encoding);
				ret = -DPTX_RETURN_EINVAL;
				break;
			}
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		pbn_margin = peak_pixel_bandwidth * (1006u);
		pbn_margin_mhz = (pbn_margin / 1000000u);
		pbn_margin_tenfold = (pbn_margin / 100000u);

		pbn_margin_fraction = (pbn_margin_tenfold - (pbn_margin_mhz * 10u));

		dev_param->ausPayloadBandwidthNumber[dp_stream_id] =
			(pbn_margin_fraction >= 5u) ? (uint16_t)(pbn_margin_mhz + 1u) :
					(uint16_t)pbn_margin_mhz;

		ret = dptx_ext_get_link_numof_slots(
			dev_param, dev_param->ausPayloadBandwidthNumber[dp_stream_id],
			&dev_param->aucNumOfSlots[dp_stream_id]);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		dptx_dbg("Stream %d : PBN(%d) <- Rest(%d), The number of slots: %d",
			dp_stream_id,
			dev_param->ausPayloadBandwidthNumber[dp_stream_id], pbn_margin_fraction,
			dev_param->aucNumOfSlots[dp_stream_id]);
	}
	return ret;
}

static int32_t dptx_ext_get_prev_num_of_slots(struct Dptx_Params *dev_param, uint8_t stream_idx, uint8_t *prev_num_of_slots)
{
	int ret_val = DPTX_RETURN_NO_ERROR;

	switch (stream_idx) {
	case PHY_INPUT_STREAM_0:
		*prev_num_of_slots = 0;
		break;
	case PHY_INPUT_STREAM_1:
		*prev_num_of_slots = dev_param->aucNumOfSlots[PHY_INPUT_STREAM_0];
		break;
	case PHY_INPUT_STREAM_2:
		*prev_num_of_slots = dev_param->aucNumOfSlots[PHY_INPUT_STREAM_0];
		*prev_num_of_slots += dev_param->aucNumOfSlots[PHY_INPUT_STREAM_1];
		break;
	case PHY_INPUT_STREAM_3:
		*prev_num_of_slots = dev_param->aucNumOfSlots[PHY_INPUT_STREAM_0];
		*prev_num_of_slots += dev_param->aucNumOfSlots[PHY_INPUT_STREAM_1];
		*prev_num_of_slots += dev_param->aucNumOfSlots[PHY_INPUT_STREAM_2];
		break;
	default:
		dptx_err("Invalid stream index(%d)", stream_idx);
		ret_val = DPTX_RETURN_EINVAL;
	}

	return ret_val;
}

static int32_t Dptx_Ext_Set_Link_VCP_Tables(struct Dptx_Params *pstDptx,
					    uint8_t ucStream_Index)
{
	uint8_t ucPrev_NumOfSlots, ucCurrent_NumOfSlots;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	ucCurrent_NumOfSlots = pstDptx->aucNumOfSlots[ucStream_Index];

	if (dptx_ext_get_prev_num_of_slots(pstDptx, ucStream_Index, &ucPrev_NumOfSlots) != DPTX_RETURN_NO_ERROR) {
		ret_val = DPTX_RETURN_EINVAL;
	} else if (ucPrev_NumOfSlots >= 64) {
		dptx_err("Invalid num of slots(%u)", ucPrev_NumOfSlots);
		ret_val = DPTX_RETURN_EINVAL;
	} else {
		ret_val = dptx_ext_set_link_vcpid_table_slot(pstDptx,
							     ucPrev_NumOfSlots + 1u,
							     ucCurrent_NumOfSlots,
							     (ucStream_Index + 1u));
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t Dptx_Ext_Set_Sink_VCP_Table_Slots(struct Dptx_Params *pstDptx,
						 uint8_t ucStream_Index)
{
	uint8_t ucPrev_NumOfSlots, ucCurrent_NumOfSlots;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	ucCurrent_NumOfSlots = pstDptx->aucNumOfSlots[ucStream_Index];

	switch (ucStream_Index) {
	case PHY_INPUT_STREAM_0:
		ucPrev_NumOfSlots = 0;
		break;
	case PHY_INPUT_STREAM_1:
		ucPrev_NumOfSlots = pstDptx->aucNumOfSlots[PHY_INPUT_STREAM_0];
		break;
	case PHY_INPUT_STREAM_2:
		ucPrev_NumOfSlots = pstDptx->aucNumOfSlots[PHY_INPUT_STREAM_0];
		ucPrev_NumOfSlots += pstDptx->aucNumOfSlots[PHY_INPUT_STREAM_1];
		break;
	case PHY_INPUT_STREAM_3:
		ucPrev_NumOfSlots = pstDptx->aucNumOfSlots[PHY_INPUT_STREAM_0];
		ucPrev_NumOfSlots += pstDptx->aucNumOfSlots[PHY_INPUT_STREAM_1];
		ucPrev_NumOfSlots += pstDptx->aucNumOfSlots[PHY_INPUT_STREAM_2];
		break;
	default:
		dptx_err("Invalid stream index(%d)", ucStream_Index);
		return DPTX_RETURN_EINVAL;
	}

	ret_val = dptx_ext_set_sink_vcpid_table_slot(pstDptx,
						     (ucPrev_NumOfSlots + 1),
						     ucCurrent_NumOfSlots,
						     (ucStream_Index + 1));
	if (ret_val != DPTX_RETURN_NO_ERROR) {
		return ret_val;
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t Dptx_Ext_Clear_VCP_Tables(struct Dptx_Params *pstDptx)
{
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	ret_val = dptx_ext_clear_sink_vcpid_table(pstDptx);
	if (ret_val != DPTX_RETURN_NO_ERROR) {
		return ret_val;
	}

	ret_val = dptx_ext_clear_link_vcp_tables(pstDptx);
	if (ret_val != DPTX_RETURN_NO_ERROR) {
		return ret_val;
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_mst_act(struct Dptx_Params *pstDptx)
{
	uint32_t loop, act_retry, reg_val;
	int32_t ret = DPTX_RETURN_NO_ERROR;

	uint8_t sink_payload_status = 0u;

	/* trigger the link to start an ACT sequence */
	reg_val = Dptx_Reg_Readl(pstDptx, DPTX_CCTL);
	reg_val |= DPTX_CCTL_INITIATE_MST_ACT;
	Dptx_Reg_Writel(pstDptx, DPTX_CCTL, reg_val);

	for (act_retry = 0u; act_retry < 2u; act_retry++) {
		for (loop = 0u; loop < (uint32_t)MAX_CHECK_MST_ACT; loop++) {
			reg_val = Dptx_Reg_Readl(pstDptx, DPTX_CCTL);
			if ((reg_val & DPTX_CCTL_INITIATE_MST_ACT) == 0u) {
				break;
			}
			mdelay(1);
		}
		if (loop >= (uint32_t)MAX_CHECK_MST_ACT) {
			if (act_retry == 0u) {
				dptx_err("[1st]MST_ACT_SEQ timeout");
				reg_val = Dptx_Reg_Readl(pstDptx, DPTX_CCTL);
				reg_val &=
					~((uint32_t)DPTX_CCTL_INITIATE_MST_ACT);
				Dptx_Reg_Writel(pstDptx, DPTX_CCTL, reg_val);
				udelay(1);

				reg_val |= DPTX_CCTL_INITIATE_MST_ACT;
				Dptx_Reg_Writel(pstDptx, DPTX_CCTL, reg_val);
			} else {
				dptx_err("[2nd]MST_ACT_SEQ timeout");
				ret = -DPTX_RETURN_MST_ACT_TIMEOUT;
				break;
			}
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		for (loop = 0u; loop < (uint32_t)MAX_CHECK_DPCD_VCP_UPDATED;
		     loop++) {
			ret = Dptx_Aux_Read_DPCD(pstDptx,
						 DP_PAYLOAD_TABLE_UPDATE_STATUS,
						 &sink_payload_status);
			if (DPTX_RETURN_SUCCESS(ret)) {
				if ((sink_payload_status & DP_PAYLOAD_ACT_HANDLED) != 0u) {
					break;
				}
				mdelay(2);
			}
		}
		if (loop >= (uint32_t)MAX_CHECK_DPCD_VCP_UPDATED) {
			dptx_err("Act in Sink is not handled");
			ret = -DPTX_RETURN_EBUSY;
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Aux_Write_DPCD(pstDptx,
					  DP_PAYLOAD_TABLE_UPDATE_STATUS,
					  DP_PAYLOAD_TABLE_UPDATED);
	}

	return ret;
}

static int32_t
Dptx_Ext_Clear_SidebandMsg_PayloadID_Table(struct Dptx_Params *pstDptx)
{
	uint8_t ucReply_Len;
	uint8_t aucMsg_Buf[MAX_MSG_BUFFER_SIZE], *pucMssage;
	int32_t msg_len = 0;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	struct drm_dp_sideband_msg_hdr stSideBand_MsgHeader = {
		.lct = 1,
		.lcr = 6,
		.rad = { 0, },
		.broadcast = true,
		.path_msg = 1,
		.msg_len = 2,
		.somt = 1,
		.eomt = 1,
		.seqno = 0,
	};

	Drm_Addition_Encode_Sideband_Msg_Hdr(&stSideBand_MsgHeader, aucMsg_Buf,
					     &msg_len);

	pucMssage = &aucMsg_Buf[msg_len];
	pucMssage[0] = DP_CLEAR_PAYLOAD_ID_TABLE;

	Drm_Addition_Encode_SideBand_Msg_CRC(pucMssage, 1);

	msg_len += 2;

	if (msg_len < 0) {
		ret_val = DPTX_RETURN_EINVAL;

		dptx_err("Invaild msg_len: %d", msg_len);
	} else if (DPTX_RETURN_ERROR(Dptx_Aux_Write_Bytes_To_DPCD(
			   pstDptx, DP_SIDEBAND_MSG_DOWN_REQ_BASE, aucMsg_Buf,
			   msg_len))) {
		ret_val = ETIMEDOUT;
	} else if (DPTX_RETURN_ERROR(dptx_ext_get_sideband_msg_down_req_reply(
			   pstDptx, DP_CLEAR_PAYLOAD_ID_TABLE, (uint8_t *)NULL,
			   &ucReply_Len))) {
		ret_val = ETIMEDOUT;
	} else {
		ret_val = DPTX_RETURN_NO_ERROR;
	}

	return ret_val;
}

static int32_t Dptx_Ext_Get_TopologyState(struct Dptx_Params *pstDptx)
{
	uint8_t ucMainPort_Count, ucBranchPort_Count;
	uint8_t ucBranchPort_Number = 0, ucSinkDevPort_Index = 0;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	struct drm_dp_sideband_msg_rx *pstMain_Msg_Rx, *pstMsg_Rx;
	struct drm_dp_sideband_msg_reply_body *pstMain_Msg_Reply, *pstMsg_Reply;
	struct Dptx_Topology_Params *pstDptx_Topology_Params =
		&stDptx_Topology_Params;

	memset(&pstDptx_Topology_Params->stMainBranch_Msg_Rx, 0,
	       sizeof(pstDptx_Topology_Params->stMainBranch_Msg_Rx));
	memset(&pstDptx_Topology_Params->stMainBranch_Msg_Reply, 0,
	       sizeof(pstDptx_Topology_Params->stMainBranch_Msg_Reply));

	memset(&pstDptx->aucStreamSink_PortNumber[0], INVALID_MST_PORT_NUM,
	       (sizeof(uint8_t) * PHY_INPUT_STREAM_MAX));
	memset(&pstDptx->aucRAD_PortNumber[0], INVALID_MST_PORT_NUM,
	       (sizeof(uint8_t) * PHY_INPUT_STREAM_MAX));

	pstMain_Msg_Rx = &pstDptx_Topology_Params->stMainBranch_Msg_Rx;
	pstMain_Msg_Reply = &pstDptx_Topology_Params->stMainBranch_Msg_Reply;

	ret_val = Dptx_Ext_Clear_SidebandMsg_PayloadID_Table(pstDptx);
	if (ret_val != DPTX_RETURN_NO_ERROR) {
		return ret_val;
	}

	ret_val = dptx_ext_set_sideband_msg_link_address(pstDptx,
							 pstMain_Msg_Rx,
							 pstMain_Msg_Reply,
							 INVALID_MST_PORT_NUM);
	if (ret_val != DPTX_RETURN_NO_ERROR) {
		return ret_val;
	}

	for (ucMainPort_Count = 0;
	     ucMainPort_Count < pstMain_Msg_Reply->u.link_addr.nports;
	     ucMainPort_Count++) {
		if (pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count]
				    .peer_device_type == PEER_STREAM_SINK_DEV &&
		    !pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].mcs &&
		    pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].ddps) {
			dptx_info(
				"Sink[%u]:  port = %d, RAD Port = %d",
				ucSinkDevPort_Index,
				pstDptx->aucStreamSink_PortNumber
					[ucSinkDevPort_Index],
				pstDptx->aucRAD_PortNumber[ucSinkDevPort_Index]);

			pstDptx->aucStreamSink_PortNumber[ucSinkDevPort_Index] =
				pstMain_Msg_Reply->u.link_addr
					.ports[ucMainPort_Count]
					.port_number;

			ucSinkDevPort_Index++;
		}

		if (pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count]
				    .input_port == INPUT_PORT_TYPE_TX &&
		    pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count]
				    .peer_device_type == PEER_BRANCHING_DEV &&
		    pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].mcs &&
		    pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].ddps) {
			if (ucBranchPort_Number >= MAX_NUM_OF_SUB_BRANCH) {
				dptx_warn(
					"Num of branchs is reached to Max(%d)",
					(MAX_NUM_OF_SUB_BRANCH + 1));
				return DPTX_RETURN_NO_ERROR;
			}

			pstMsg_Rx = &pstDptx_Topology_Params->stSubBranch_Msg_Rx
					     [ucBranchPort_Number];
			pstMsg_Reply =
				&pstDptx_Topology_Params->stSubBranch_Msg_Reply
					 [ucBranchPort_Number];

			ucBranchPort_Number++;

			ret_val = dptx_ext_set_sideband_msg_link_address(
				pstDptx, pstMsg_Rx, pstMsg_Reply,
				pstMain_Msg_Reply->u.link_addr
					.ports[ucMainPort_Count]
					.port_number);
			if (ret_val != DPTX_RETURN_NO_ERROR) {
				return ret_val;
			}

			for (ucBranchPort_Count = 0;
			     ucBranchPort_Count <
			     pstMsg_Reply->u.link_addr.nports;
			     ucBranchPort_Count++) {
				if (pstMsg_Reply->u.link_addr
						    .ports[ucBranchPort_Count]
						    .input_port ==
					    INPUT_PORT_TYPE_TX &&
				    pstMsg_Reply->u.link_addr
						    .ports[ucBranchPort_Count]
						    .peer_device_type ==
					    PEER_STREAM_SINK_DEV &&
				    !pstMsg_Reply->u.link_addr
					     .ports[ucBranchPort_Count]
					     .mcs &&
				    pstMsg_Reply->u.link_addr
					    .ports[ucBranchPort_Count]
					    .ddps) {
					dptx_info(
						"Sink[%u]: port = %u, RAD Port = %u",
						ucSinkDevPort_Index,
						pstMsg_Reply->u.link_addr
							.ports[ucBranchPort_Count]
							.port_number,
						pstDptx->aucRAD_PortNumber
							[ucSinkDevPort_Index]);
					pstDptx->aucStreamSink_PortNumber
						[ucSinkDevPort_Index] =
						pstMsg_Reply->u.link_addr
							.ports[ucBranchPort_Count]
							.port_number;
					pstDptx->aucRAD_PortNumber
						[ucSinkDevPort_Index] =
						pstMsg_Reply->u.link_addr
							.ports[ucMainPort_Count]
							.port_number;
					ucSinkDevPort_Index++;

					if (ucSinkDevPort_Index >=
					    PHY_INPUT_STREAM_MAX) {
						dptx_err(
							"Port index is reached to %d",
							PHY_INPUT_STREAM_MAX);
						return DPTX_RETURN_NO_ERROR;
					}
				}
			}
		}
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Ext_Set_Stream_Capability(struct Dptx_Params *pstDptx)
{
	bool mst_caps = (bool)false;
	uint32_t uiRegMap_Cctl;
	int32_t ret_val = DPTX_RETURN_NO_ERROR;

	mst_caps = drm_addition_read_mst_cap(pstDptx);

	uiRegMap_Cctl = Dptx_Reg_Readl(pstDptx, DPTX_CCTL);

	if (pstDptx->bMultStreamTransport) {
		if (mst_caps) {
			dptx_info("[MST]Src: profiled <-> Sink : support\n");

			uiRegMap_Cctl |= DPTX_CCTL_ENABLE_MST_MODE;
			Dptx_Reg_Writel(pstDptx, DPTX_CCTL, uiRegMap_Cctl);

			ret_val =
				Dptx_Aux_Write_DPCD(pstDptx, DP_MSTM_CTRL,
						    (DP_MST_EN | DP_UP_REQ_EN |
						     DP_UPSTREAM_IS_SRC));
			if (ret_val != DPTX_RETURN_NO_ERROR) {
				return ret_val;
			}
		} else {
			dptx_info(
				"[MST]Src: profiled <-> Sink : not support\n");

			pstDptx->bMultStreamTransport = false;
			pstDptx->ucNumOfStreams = 1;

			uiRegMap_Cctl &= ~DPTX_CCTL_ENABLE_MST_MODE;
			Dptx_Reg_Writel(pstDptx, DPTX_CCTL, uiRegMap_Cctl);

			if (pstDptx->aucDPCD_Caps[DP_DPCD_REV] >= 0x12) {
				ret_val = Dptx_Aux_Write_DPCD(
					pstDptx, DP_MSTM_CTRL,
					~(DP_MST_EN | DP_UP_REQ_EN |
					  DP_UPSTREAM_IS_SRC));
				if (ret_val != DPTX_RETURN_NO_ERROR) {
					return ret_val;
				}
			}
		}
	} else {
		uiRegMap_Cctl &= ~DPTX_CCTL_ENABLE_MST_MODE;
		Dptx_Reg_Writel(pstDptx, DPTX_CCTL, uiRegMap_Cctl);

		if (mst_caps) {
			dptx_info(
				"[MST]Src: not profiled <-> Sink : support\n");

			ret_val =
				Dptx_Aux_Write_DPCD(pstDptx, DP_MSTM_CTRL,
						    ~(DP_MST_EN | DP_UP_REQ_EN |
						      DP_UPSTREAM_IS_SRC));
			if (ret_val != DPTX_RETURN_NO_ERROR) {
				return ret_val;
			}
		} else {
			dptx_info(
				"[MST]Src: not profiled <-> Sink : not support\n");
		}
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Ext_Set_Topology_Configuration(struct Dptx_Params *pstDptx)
{
	uint8_t dptx_stream_id;
	int32_t ret = DPTX_RETURN_NO_ERROR;
	bool support_sideband_msg = false;

	if (pstDptx->hw_config.support_sideband_msg != 0u) {
		support_sideband_msg = drm_addition_read_mst_cap(pstDptx);
	}
	if (support_sideband_msg) {
		ret = Dptx_Ext_Get_TopologyState(pstDptx);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Ext_Clear_VCP_Tables(pstDptx);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		for (dptx_stream_id = 0;
		     dptx_stream_id < pstDptx->ucNumOfStreams;
		     dptx_stream_id++) {
			ret = Dptx_Ext_Get_Link_PayloadBandwidthNumber(
				pstDptx, dptx_stream_id);
			if (DPTX_RETURN_SUCCESS(ret)) {
				if (support_sideband_msg) {
					if (pstDptx->aucStreamSink_PortNumber
						    [dptx_stream_id] !=
					    INVALID_MST_PORT_NUM) {
						ret = dptx_ext_set_sideband_msg_enum_path_resources(
							pstDptx,
							pstDptx->aucStreamSink_PortNumber
								[dptx_stream_id],
							(dptx_stream_id + 1),
							pstDptx->aucRAD_PortNumber
								[dptx_stream_id]);
					}
				}
			}
			if (DPTX_RETURN_ERROR(ret)) {
				break;
			}
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		for (dptx_stream_id = 0;
		     dptx_stream_id < pstDptx->ucNumOfStreams;
		     dptx_stream_id++) {
			ret = Dptx_Ext_Set_Link_VCP_Tables(pstDptx,
							   dptx_stream_id);
			if (DPTX_RETURN_SUCCESS(ret)) {
				ret = Dptx_Ext_Set_Sink_VCP_Table_Slots(
					pstDptx, dptx_stream_id);
			}
			if (DPTX_RETURN_SUCCESS(ret)) {
				ret = dptx_ext_mst_act(pstDptx);
			}
			if (DPTX_RETURN_SUCCESS(ret)) {
				if (support_sideband_msg) {
					ret = dptx_ext_set_sideband_msg_allocate_payload(
						pstDptx,
						pstDptx->aucStreamSink_PortNumber
							[dptx_stream_id],
						pstDptx->aucVCP_Id
							[dptx_stream_id],
						pstDptx->ausPayloadBandwidthNumber
							[dptx_stream_id],
						pstDptx->aucRAD_PortNumber
							[dptx_stream_id]);
				}
			}
			if (DPTX_RETURN_ERROR(ret)) {
				break;
			}
		}
	}

	return ret;
}

int32_t Dptx_Ext_Remote_I2C_Read(struct Dptx_Params *pstDptx,
				 uint8_t ucStream_Index)
{
	uint8_t ucReply_Len, ucPort_Index, ucRad_Port, ucExt_Blocks,
		ucBlk_Index;
	uint8_t aucReq_Buf[MAX_MSG_BUFFER_SIZE],
		aucRep_Buf[MAX_MSG_BUFFER_SIZE];
	uint8_t *pucMsg;
	int32_t len = 256;
	int32_t ret_val;
	struct drm_dp_sideband_msg_hdr stMsg_Header;

	ucPort_Index = pstDptx->aucStreamSink_PortNumber[ucStream_Index];
	if (ucPort_Index == INVALID_MST_PORT_NUM) {
		dptx_err("Stream %d isn't allocated ", ucStream_Index);
		return DPTX_RETURN_EINVAL;
	}

	ucRad_Port = pstDptx->aucRAD_PortNumber[ucStream_Index];

	memset(&stMsg_Header, 0, sizeof(struct drm_dp_sideband_msg_hdr));

	stMsg_Header.lct = 1;
	stMsg_Header.lcr = 0;
	stMsg_Header.rad[0] = 0;
	stMsg_Header.broadcast = false;
	stMsg_Header.path_msg = 0;
	stMsg_Header.msg_len = 9;
	stMsg_Header.somt = 1;
	stMsg_Header.eomt = 1;
	stMsg_Header.seqno = 0;

	if (ucRad_Port != INVALID_MST_PORT_NUM) {
		stMsg_Header.lct = 2;
		stMsg_Header.lcr = 1;
		stMsg_Header.rad[0] |= ((ucRad_Port << 4) & 0xF0);
	}

	Drm_Addition_Encode_Sideband_Msg_Hdr(&stMsg_Header, aucReq_Buf, &len);

	pucMsg = &aucReq_Buf[len];
	pucMsg[0] = DP_REMOTE_I2C_READ;
	pucMsg[1] = ((ucPort_Index & 0xF) << 4);
	pucMsg[1] |= (1 & 0x3);
	pucMsg[2] = (0x50 & 0x7F);
	pucMsg[3] = 1; // Num of bytes to write
	pucMsg[4] = (0 << 5); // I2C data to write
	pucMsg[5] = (0 & 0xF);
	pucMsg[6] = (0x50 & 0x7F);
	pucMsg[7] = (DPTX_ONE_EDID_BLK_LEN);

	Drm_Addition_Encode_SideBand_Msg_CRC(pucMsg, 8);

	len += 9;

	ret_val = Dptx_Aux_Write_Bytes_To_DPCD(
		pstDptx, DP_SIDEBAND_MSG_DOWN_REQ_BASE, aucReq_Buf, len);
	if (ret_val != DPTX_RETURN_NO_ERROR)
		return ret_val;

	ret_val = dptx_ext_get_sideband_msg_down_req_reply(
		pstDptx, DP_REMOTE_I2C_READ, aucRep_Buf, &ucReply_Len);
	if (ret_val != DPTX_RETURN_NO_ERROR)
		return ret_val;

	if (aucRep_Buf[2] == 0) {
		dptx_warn("No EDID data in Sink");
		return DPTX_RETURN_ENODEV;
	}

	memcpy(pstDptx->pucEdidBuf, &aucRep_Buf[3], DPTX_ONE_EDID_BLK_LEN);

	dptx_dbg("I2C Remote messages replied => ");
	dptx_dbg(" -.Reply type: %s", (aucRep_Buf[0] & 0x80) ? "NAK" : "ACK");
	dptx_dbg(" -.Request id: %s",
		 ((aucRep_Buf[0] & 0x7F) == DP_REMOTE_I2C_READ) ?
			 "REMOTE_I2C_READ" :
			 "Wrong ID");
	dptx_dbg(" -.Port Number: %d <- (%d, %d)", (aucRep_Buf[1] & 0x0F),
		 ucPort_Index, ucRad_Port);
	dptx_dbg(" -.Num of bytes read: %d", aucRep_Buf[2]);
	dptx_dbg(" -.Num of extensions: %d", aucRep_Buf[126 + 3]);

	ucExt_Blocks = aucRep_Buf[(DPTX_EDID_EXT_BLK_FIELD + 3)];
	if (ucExt_Blocks == 0)
		return DPTX_RETURN_NO_ERROR;

	if (ucExt_Blocks > DPTX_EDID_MAX_EXTRA_BLK) {
		dptx_warn(
			"The number of extended blocks is larger than Max %d -> down to %d",
			(uint32_t)DPTX_EDID_MAX_EXTRA_BLK,
			(uint32_t)DPTX_EDID_MAX_EXTRA_BLK);
		ucExt_Blocks = DPTX_EDID_MAX_EXTRA_BLK;
	}

	for (ucBlk_Index = 1; ucBlk_Index <= ucExt_Blocks; ucBlk_Index++) {
		Drm_Addition_Encode_Sideband_Msg_Hdr(&stMsg_Header, aucReq_Buf,
						     &len);

		pucMsg = &aucReq_Buf[len];
		pucMsg[0] = DP_REMOTE_I2C_READ;
		pucMsg[1] = ((ucPort_Index & 0xF) << 4);
		pucMsg[1] |= (1 & 0x3);
		pucMsg[2] = (0x50 & 0x7F);
		pucMsg[3] = 1;
		pucMsg[4] = (ucBlk_Index * DPTX_ONE_EDID_BLK_LEN);
		pucMsg[5] = (0 & 0xF);
		pucMsg[6] = (0x50 & 0x7F);
		pucMsg[7] = (DPTX_ONE_EDID_BLK_LEN);

		Drm_Addition_Encode_SideBand_Msg_CRC(pucMsg, 8);

		len += 9;

		ret_val = Dptx_Aux_Write_Bytes_To_DPCD(
			pstDptx, DP_SIDEBAND_MSG_DOWN_REQ_BASE, aucReq_Buf,
			len);
		if (ret_val != DPTX_RETURN_NO_ERROR)
			return ret_val;

		ret_val = dptx_ext_get_sideband_msg_down_req_reply(
			pstDptx, DP_REMOTE_I2C_READ, aucRep_Buf, &ucReply_Len);
		if (ret_val != DPTX_RETURN_NO_ERROR)
			return ret_val;

		memcpy(&pstDptx->pucEdidBuf[(DPTX_ONE_EDID_BLK_LEN *
					     ucBlk_Index)],
		       &aucRep_Buf[3], DPTX_ONE_EDID_BLK_LEN);

		dptx_dbg("Extended messages replied => ");
		dptx_dbg(" -.Reply type: %s",
			 (aucRep_Buf[0] & 0x80) ? "ACK" : "NAK");
		dptx_dbg(" -.Request id: %s",
			 ((aucRep_Buf[0] & 0x7F) == DP_REMOTE_I2C_READ) ?
				 "REMOTE_I2C_READ" :
				 "Wrong ID");
		dptx_dbg(" -.Port Number: %d <- (%d, %d)",
			 (aucRep_Buf[1] & 0x0F), ucPort_Index, ucRad_Port);
		dptx_dbg(" -.Num of bytes read: %d", aucRep_Buf[2]);
		dptx_dbg(" -.Num of extensions: %d", aucRep_Buf[126 + 3]);
	}

	return DPTX_RETURN_NO_ERROR;
}
