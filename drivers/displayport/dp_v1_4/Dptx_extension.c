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

#include "Dptx_v14.h"
#include "Dptx_drm_dp_addition.h"
#include "Dptx_reg.h"
#include "Dptx_dbg.h"

#define NUM_OF_CLEAR_VC_PAYLOAD_IDS			3
#define NUM_OF_MST_VCP_TABLEs				8
#define INVALID_MST_PORT_NUM				0xFF
#define INVALID_MST_RAD_PORT_NUM			-1

#define DPCD_DOWN_REP_SIZE					256
#define MAX_CHECK_DPCD_VCP_UPDATED			1000
#define MAX_NUMBER_TO_WAIT_MSG_REPLY		1000
#define MAX_CHECK_MST_ACT					10

#define PRINT_BUF_SIZE						1024

#define MAX_NUM_OF_SUB_BRANCH				2

struct Dptx_Topology_Params {

	struct drm_dp_sideband_msg_rx			stMainBranch_Msg_Rx;
	struct drm_dp_sideband_msg_reply_body	stMainBranch_Msg_Reply;
	struct drm_dp_sideband_msg_rx			stSubBranch_Msg_Rx[MAX_NUM_OF_SUB_BRANCH];
	struct drm_dp_sideband_msg_reply_body	stSubBranch_Msg_Reply[MAX_NUM_OF_SUB_BRANCH];
};

struct Dptx_Topology_Params		stDptx_Topology_Params;


static int32_t dptx_ext_wait_sideband_msg_reply_ready(struct Dptx_Params *pstDptx)
{
	uint8_t	ucSink_Svc_IRQ_Vector_ESI0, ucSink_Service_IRQ_Vector;
	int32_t	iRetVal, iCount = 0;

	while (true) {
		iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_DEVICE_SERVICE_IRQ_VECTOR_ESI0, &ucSink_Svc_IRQ_Vector_ESI0);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return  iRetVal;

		iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_DEVICE_SERVICE_IRQ_VECTOR, &ucSink_Service_IRQ_Vector);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return  iRetVal;

		if ((ucSink_Service_IRQ_Vector & DP_DOWN_REP_MSG_RDY) || (ucSink_Svc_IRQ_Vector_ESI0 & DP_DOWN_REP_MSG_RDY))
			break;

		if (iCount++ > MAX_NUMBER_TO_WAIT_MSG_REPLY) {
			dptx_notice("Timed out(%d ms) from sideband Msg reply ready", (u32)iCount);
			return  DPTX_RETURN_EBUSY;
		}

		mdelay(1);
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_clear_sideband_msg_reply(struct Dptx_Params *pstDptx)
{
	int iCount = 0;
	uint8_t ucSink_Service_IRQ_Vector_ESI0;
	uint8_t ucSink_Service_IRQ_Vector;
	int32_t iRetVal;

	while (true) {
		iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_DEVICE_SERVICE_IRQ_VECTOR, &ucSink_Service_IRQ_Vector);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return  iRetVal;

		iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_DEVICE_SERVICE_IRQ_VECTOR_ESI0, &ucSink_Service_IRQ_Vector_ESI0);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return  iRetVal;

		if (!(ucSink_Service_IRQ_Vector & DP_DOWN_REP_MSG_RDY || ucSink_Service_IRQ_Vector_ESI0 & DP_DOWN_REP_MSG_RDY))
			break;

		iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_DEVICE_SERVICE_IRQ_VECTOR, DP_DOWN_REP_MSG_RDY);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return  iRetVal;

		iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_DEVICE_SERVICE_IRQ_VECTOR_ESI0, ucSink_Service_IRQ_Vector_ESI0);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return  iRetVal;

		if (iCount++ > MAX_NUMBER_TO_WAIT_MSG_REPLY) {
			dptx_notice("Timed out(%d ms) from clear sideband Msg reply", iCount);
			return  DPTX_RETURN_EBUSY;
		}

		mdelay(1);
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_get_sideband_msg_down_reply(struct Dptx_Params *pstDptx, uint8_t ucRequest_id, uint8_t *pucMsg_Out, uint8_t *pucMsg_len)
{
	uint8_t aucSink_SidebandMsg_Buf[DPCD_DOWN_REP_SIZE], aucMessage[1024];
	uint8_t ucHeader_Len, ucMsg_Len, ucRetries = 0;
	uint8_t ucFirst = 1;
	int32_t iRetVal;
	struct drm_dp_sideband_msg_hdr stDp_SidebandMsg_Header;

	*pucMsg_len = 0;

again:
	memset(aucMessage, 0, 1024);
	ucMsg_Len = 0;

	while (true) {
		iRetVal = dptx_ext_wait_sideband_msg_reply_ready(pstDptx);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return  iRetVal;

		iRetVal = Dptx_Aux_Read_Bytes_From_DPCD(pstDptx, DP_SIDEBAND_MSG_DOWN_REP_BASE, aucSink_SidebandMsg_Buf, DPCD_DOWN_REP_SIZE);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return  iRetVal;

		iRetVal = Drm_Addition_Decode_Sideband_Msg_Hdr(&stDp_SidebandMsg_Header, aucSink_SidebandMsg_Buf, DPCD_DOWN_REP_SIZE, &ucHeader_Len);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return  iRetVal;

		dptx_dbg("\nRequest id(%d): lct=%d, lcr=%d", ucRequest_id, stDp_SidebandMsg_Header.lct, stDp_SidebandMsg_Header.lcr);
		dptx_dbg("\nrad=0x%x, bcast=%d", stDp_SidebandMsg_Header.lct > 1 ? stDp_SidebandMsg_Header.rad[0] : 0, stDp_SidebandMsg_Header.broadcast);
		dptx_dbg("\npath=%d, msglen=%d, somt=%d, eomt=%d, seqno=%d", stDp_SidebandMsg_Header.path_msg,
																	stDp_SidebandMsg_Header.msg_len,
																	stDp_SidebandMsg_Header.somt,
																	stDp_SidebandMsg_Header.eomt,
																	stDp_SidebandMsg_Header.seqno);

		stDp_SidebandMsg_Header.msg_len -= 1;

		memcpy(&aucMessage[ucMsg_Len], &aucSink_SidebandMsg_Buf[ucHeader_Len], stDp_SidebandMsg_Header.msg_len);

		ucMsg_Len += stDp_SidebandMsg_Header.msg_len;

		if (ucFirst && !stDp_SidebandMsg_Header.somt) {
			dptx_err("SOMT not set ");
			return  DPTX_RETURN_EBUSY;
		}

		ucFirst = 0;

		iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_DEVICE_SERVICE_IRQ_VECTOR, DP_DOWN_REP_MSG_RDY);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return  iRetVal;

		if (stDp_SidebandMsg_Header.eomt)
			break;
	}
	if ((aucMessage[0] & 0x7F) != ucRequest_id) {
		if (ucRetries < 3) {
			dptx_warn("id %d does not match expected %d, retrying ", aucMessage[0] & 0x7F, ucRequest_id);
			ucRetries++;
			goto again;
		} else {
			dptx_err("id %d does not match expected %d ", aucMessage[0] & 0x7F, ucRequest_id);
			return  DPTX_RETURN_EBUSY;
		}
	}

	iRetVal = dptx_ext_clear_sideband_msg_reply(pstDptx);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	if (pucMsg_Out)
		memcpy(pucMsg_Out, aucMessage, ucMsg_Len);

	*pucMsg_len = ucMsg_Len;

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_clear_sideband_msg_payload_id_table(struct Dptx_Params *pstDptx)
{
	uint8_t	ucReply_Len;
	uint8_t	aucMsg_Buf[256], *pucMssage;
	int32_t	iRetVal, iMsg_Len = 256;

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

	Drm_Addition_Encode_Sideband_Msg_Hdr(&stSideBand_MsgHeader, aucMsg_Buf, &iMsg_Len);

	pucMssage = &aucMsg_Buf[iMsg_Len];
	pucMssage[0] = DP_CLEAR_PAYLOAD_ID_TABLE;

	Drm_Addition_Encode_SideBand_Msg_CRC(pucMssage, 1);

	iMsg_Len += 2;

	iRetVal = Dptx_Aux_Write_Bytes_To_DPCD(pstDptx, DP_SIDEBAND_MSG_DOWN_REQ_BASE, aucMsg_Buf, iMsg_Len);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	iRetVal = dptx_ext_get_sideband_msg_down_reply(pstDptx, DP_CLEAR_PAYLOAD_ID_TABLE, NULL, &ucReply_Len);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_set_sideband_msg_enum_path_resources(struct Dptx_Params *pstDptx, uint8_t ucStreamSink_PortNum, uint8_t ucVCP_Id, uint8_t ucRAD_PortNum)
{
	uint8_t	ucReply_Len;
	uint8_t	aucMsg_Buf[256];
	uint8_t	*msg;
	int	iMsg_Len = 256;
	int32_t	iRetVal;
	struct drm_dp_sideband_msg_hdr stMsg_Header;

	memset(&stMsg_Header, 0, sizeof(struct drm_dp_sideband_msg_hdr));

	dptx_dbg("Port %d for allocate: RAD Port = %d ", ucStreamSink_PortNum, ucRAD_PortNum);

	stMsg_Header.lct		= 1;
	stMsg_Header.lcr		= 0;
	stMsg_Header.rad[0]		= 0;
	stMsg_Header.broadcast  = false;
	stMsg_Header.path_msg	= 0;
	stMsg_Header.msg_len	= 3;
	stMsg_Header.somt		= 1;
	stMsg_Header.eomt		= 1;
	stMsg_Header.seqno		= 0;

	if (ucRAD_PortNum != INVALID_MST_PORT_NUM) {
		stMsg_Header.lct = 2;
		stMsg_Header.lcr = 1;
		stMsg_Header.rad[0] |= ((ucRAD_PortNum << 4) & 0xF0);
	}

	Drm_Addition_Encode_Sideband_Msg_Hdr(&stMsg_Header, aucMsg_Buf, &iMsg_Len);

//	dptx_ext_print_buf( aucMsg_Buf, iMsg_Len );

	msg		= &aucMsg_Buf[iMsg_Len];
	msg[0]	= DP_ENUM_PATH_RESOURCES;
	msg[1]	= ((ucStreamSink_PortNum & 0xF) << 4);

	Drm_Addition_Encode_SideBand_Msg_CRC(msg, 2);

	iMsg_Len += 3;

	iRetVal = Dptx_Aux_Write_Bytes_To_DPCD(pstDptx, DP_SIDEBAND_MSG_DOWN_REQ_BASE, aucMsg_Buf, iMsg_Len);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	iRetVal = dptx_ext_get_sideband_msg_down_reply(pstDptx, DP_ENUM_PATH_RESOURCES, NULL, &ucReply_Len);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_set_sideband_msg_allocate_payload(struct Dptx_Params *pstDptx,
																						uint8_t ucStreamSink_PortNum,
																						uint8_t ucVCP_Id,
																						u16 usPBN,
																						uint8_t ucRAD_PortNum)
{
	uint8_t	ucReply_Len;
	uint8_t	aucMsg_Buf[256];
	uint8_t	*msg;
	int	iMsg_Len = 256;
	int32_t	iRetVal;
	struct drm_dp_sideband_msg_hdr stMsg_Header;

	memset(&stMsg_Header, 0, sizeof(struct drm_dp_sideband_msg_hdr));

	dptx_dbg("Port %d allocates VCP Id= %d, RAD Port = %d", ucStreamSink_PortNum, ucVCP_Id, ucRAD_PortNum);

	stMsg_Header.lct		= 1;
	stMsg_Header.lcr		= 0;
	stMsg_Header.rad[0]		= 0;
	stMsg_Header.broadcast	= false;
	stMsg_Header.path_msg	= 1;
	stMsg_Header.msg_len	= 6;
	stMsg_Header.somt		= 1;
	stMsg_Header.eomt		= 1;
	stMsg_Header.seqno		= 0;

	if (ucRAD_PortNum != INVALID_MST_PORT_NUM) {
		stMsg_Header.lct = 2;
		stMsg_Header.lcr = 1;
		stMsg_Header.rad[0] |= ((ucRAD_PortNum << 4) & 0xF0);
	}

	Drm_Addition_Encode_Sideband_Msg_Hdr(&stMsg_Header, aucMsg_Buf, &iMsg_Len);

//	dptx_ext_print_buf( aucMsg_Buf, iMsg_Len );

	msg		= &aucMsg_Buf[iMsg_Len];

	msg[0]	= DP_ALLOCATE_PAYLOAD;
	msg[1]	= ((ucStreamSink_PortNum & 0xF) << 4);
	msg[2]	= (ucVCP_Id & 0x7F);
	msg[3]	= (usPBN >> 8);
	msg[4]	= (usPBN & 0xFF);

	Drm_Addition_Encode_SideBand_Msg_CRC(msg, 5);

	iMsg_Len += 6;

	iRetVal = Dptx_Aux_Write_Bytes_To_DPCD(pstDptx, DP_SIDEBAND_MSG_DOWN_REQ_BASE, aucMsg_Buf, iMsg_Len);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	iRetVal = dptx_ext_get_sideband_msg_down_reply(pstDptx, DP_ALLOCATE_PAYLOAD, NULL, &ucReply_Len);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_set_sideband_msg_link_address(struct Dptx_Params *dptx,
																				struct drm_dp_sideband_msg_rx *pstSideband_Msg_Rx,
																				struct drm_dp_sideband_msg_reply_body *pstSideband_Msg_Reply,
																				uint8_t ucPort_ConnectToBranch)
{
	uint8_t aucMsgHdr_Buf[256];
	uint8_t *pucMsg;
	int iMsg_Len = 256;
	int32_t iRetVal;
	struct drm_dp_sideband_msg_hdr stMsg_Header;

	memset(&stMsg_Header, 0, sizeof(struct drm_dp_sideband_msg_hdr));

	stMsg_Header.lct		= 1;
	stMsg_Header.lcr		= 0;
	stMsg_Header.rad[0]		= 0;
	stMsg_Header.broadcast	= false;
	stMsg_Header.path_msg	= 0;
	stMsg_Header.msg_len	= 2;
	stMsg_Header.somt		= 1;
	stMsg_Header.eomt		= 1;
	stMsg_Header.seqno		= 0;

	if (ucPort_ConnectToBranch !=
		INVALID_MST_PORT_NUM) {
		stMsg_Header.lct = 2;
		stMsg_Header.lcr = 1;
		stMsg_Header.rad[0] |= ((ucPort_ConnectToBranch << 4) & 0xF0);
	}

	Drm_Addition_Encode_Sideband_Msg_Hdr(&stMsg_Header, aucMsgHdr_Buf, &iMsg_Len);

	pucMsg = &aucMsgHdr_Buf[iMsg_Len];
	pucMsg[0] = DP_LINK_ADDRESS;

	Drm_Addition_Encode_SideBand_Msg_CRC(pucMsg, 1);

	iMsg_Len += 2;

//	dptx_ext_print_buf( aucMsgHdr_Buf, iMsg_Len );

	iRetVal = Dptx_Aux_Write_Bytes_To_DPCD(dptx, DP_SIDEBAND_MSG_DOWN_REQ_BASE, aucMsgHdr_Buf, iMsg_Len);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	iRetVal = dptx_ext_get_sideband_msg_down_reply(dptx, DP_LINK_ADDRESS, pstSideband_Msg_Rx->msg, (uint8_t *)&iMsg_Len);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	pstSideband_Msg_Rx->curlen = iMsg_Len;

//	dptx_ext_print_buf( pstSideband_Msg_Rx->msg, iMsg_Len );

	iRetVal = Drm_Addition_Parse_Sideband_Link_Address(pstSideband_Msg_Rx, pstSideband_Msg_Reply);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_get_port_composition(struct Dptx_Params *pstDptx)
{
	uint8_t	ucMainPort_Count;
	uint8_t ucBranchPort_Count;
	uint8_t ucBranchPort_Number = 0;
	uint8_t ucSinkDevPort_Index = 0;
	int32_t	iRetVal;
	struct drm_dp_sideband_msg_rx *pstMain_Msg_Rx, *pstMsg_Rx;
	struct drm_dp_sideband_msg_reply_body *pstMain_Msg_Reply;
	struct drm_dp_sideband_msg_reply_body *pstMsg_Reply;
	struct Dptx_Topology_Params *pstDptx_Topology_Params;

	pstDptx_Topology_Params = &stDptx_Topology_Params;

	memset(&pstDptx_Topology_Params->stMainBranch_Msg_Rx, 0, sizeof(pstDptx_Topology_Params->stMainBranch_Msg_Rx));
	memset(&pstDptx_Topology_Params->stMainBranch_Msg_Reply, 0, sizeof(pstDptx_Topology_Params->stMainBranch_Msg_Reply));

	memset(&pstDptx->aucStreamSink_PortNumber[0], INVALID_MST_PORT_NUM, (sizeof(uint8_t) * PHY_INPUT_STREAM_MAX));
	memset(&pstDptx->aucRAD_PortNumber[0], INVALID_MST_PORT_NUM, (sizeof(uint8_t) * PHY_INPUT_STREAM_MAX));

	iRetVal = dptx_ext_clear_sideband_msg_payload_id_table(pstDptx);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	pstMain_Msg_Rx = &pstDptx_Topology_Params->stMainBranch_Msg_Rx;
	pstMain_Msg_Reply = &pstDptx_Topology_Params->stMainBranch_Msg_Reply;

	iRetVal = dptx_ext_set_sideband_msg_link_address(pstDptx, pstMain_Msg_Rx, pstMain_Msg_Reply, INVALID_MST_PORT_NUM);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	dptx_dbg("1st Brach DP_LINK_ADDRESS-NPORTS = %d", pstMain_Msg_Reply->u.link_addr.nports);

	for (ucMainPort_Count = 0; ucMainPort_Count < pstMain_Msg_Reply->u.link_addr.nports; ucMainPort_Count++) {
		if (pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].peer_device_type == PEER_STREAM_SINK_DEV &&
			!pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].mcs &&
			pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].ddps) {
			pstDptx->aucStreamSink_PortNumber[ucSinkDevPort_Index] =
				pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].port_number;
			dptx_notice(" ==> Stream Sink port[%d] = %d, RAD Port = %d", ucSinkDevPort_Index,
																			pstDptx->aucStreamSink_PortNumber[ucSinkDevPort_Index],
																			pstDptx->aucRAD_PortNumber[ucSinkDevPort_Index]);

			ucSinkDevPort_Index++;
		}

		if (pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].input_port == INPUT_PORT_TYPE_TX &&
			pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].peer_device_type == PEER_BRANCHING_DEV &&
			pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].mcs &&
			pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].ddps) {
			if (ucBranchPort_Number >= MAX_NUM_OF_SUB_BRANCH) {
				dptx_warn("Num of branchs is reached to Max(%d)", (MAX_NUM_OF_SUB_BRANCH + 1));
				return DPTX_RETURN_NO_ERROR;
			}

			pstMsg_Rx = &pstDptx_Topology_Params->stSubBranch_Msg_Rx[ucBranchPort_Number];
			pstMsg_Reply = &pstDptx_Topology_Params->stSubBranch_Msg_Reply[ucBranchPort_Number];

			ucBranchPort_Number++;

			iRetVal = dptx_ext_set_sideband_msg_link_address(pstDptx, pstMsg_Rx, pstMsg_Reply, pstMain_Msg_Reply->u.link_addr.ports[ucMainPort_Count].port_number);
			if (iRetVal != DPTX_RETURN_NO_ERROR)
				return  iRetVal;

			dptx_dbg("%d Brach DP_LINK_ADDRESS-NPORTS = %d", (ucBranchPort_Number + 1), pstMsg_Reply->u.link_addr.nports);

			for (ucBranchPort_Count = 0; ucBranchPort_Count < pstMsg_Reply->u.link_addr.nports; ucBranchPort_Count++) {
				if (pstMsg_Reply->u.link_addr.ports[ucBranchPort_Count].input_port == INPUT_PORT_TYPE_TX &&
					pstMsg_Reply->u.link_addr.ports[ucBranchPort_Count].peer_device_type == PEER_STREAM_SINK_DEV &&
					!pstMsg_Reply->u.link_addr.ports[ucBranchPort_Count].mcs &&
					pstMsg_Reply->u.link_addr.ports[ucBranchPort_Count].ddps) {
					pstDptx->aucStreamSink_PortNumber[ucSinkDevPort_Index] = pstMsg_Reply->u.link_addr.ports[ucBranchPort_Count].port_number;
					pstDptx->aucRAD_PortNumber[ucSinkDevPort_Index] = pstMsg_Reply->u.link_addr.ports[ucMainPort_Count].port_number;

					dptx_notice("==>Sink port[%d] = %d, RAD Port = %d", ucSinkDevPort_Index,
																		pstMsg_Reply->u.link_addr.ports[ucBranchPort_Count].port_number,
																		pstDptx->aucRAD_PortNumber[ucSinkDevPort_Index]);

					ucSinkDevPort_Index++;

					if (ucSinkDevPort_Index >= PHY_INPUT_STREAM_MAX) {
						dptx_notice("Port index is reached to %d", PHY_INPUT_STREAM_MAX);
						return DPTX_RETURN_NO_ERROR;
					}
				}
			}
		}
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_get_link_numof_slots(struct Dptx_Params *pstDptx, u16 iPlayloadBandWidth_Number, uint8_t *pucNumOfSlots)
{
	uint8_t ucLink_BandWidth;
	int32_t iRetVal;
	uint32_t uiDivider;

	iRetVal = Dptx_Core_PHY_Rate_To_Bandwidth(pstDptx, pstDptx->stDptxLink.ucLinkRate, &ucLink_BandWidth);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  DPTX_RETURN_EINVAL;

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
	default:
		uiDivider = (15 * pstDptx->stDptxLink.ucNumOfLanes);
		break;
	}

	*pucNumOfSlots = DIV_ROUND_UP(iPlayloadBandWidth_Number, uiDivider);

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_clear_sink_vcpid_table(struct Dptx_Params *pstDptx)
{
	uint8_t	aucPayload_Allocate_Set[NUM_OF_CLEAR_VC_PAYLOAD_IDS];
	uint8_t	ucPayload_Updated_Status;
	int	uiRetry_LinkUpdated = 0;
	int32_t	iRetVal;

	aucPayload_Allocate_Set[0] = 0x00;
	aucPayload_Allocate_Set[1] = 0x00;
	aucPayload_Allocate_Set[2] = 0x3F;

	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_PAYLOAD_TABLE_UPDATE_STATUS, DP_PAYLOAD_TABLE_UPDATED);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	iRetVal = Dptx_Aux_Write_Bytes_To_DPCD(pstDptx, DP_PAYLOAD_ALLOCATE_SET, aucPayload_Allocate_Set, NUM_OF_CLEAR_VC_PAYLOAD_IDS);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	while (true) {
		iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_PAYLOAD_TABLE_UPDATE_STATUS, &ucPayload_Updated_Status);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return  iRetVal;

		if (ucPayload_Updated_Status & DP_PAYLOAD_TABLE_UPDATED)
			break;

		if (uiRetry_LinkUpdated++ > MAX_CHECK_DPCD_VCP_UPDATED)
			dptx_warn("Payload table in Sink is not updated for %dms", (uiRetry_LinkUpdated * 1));

		mdelay(1);
	}

	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_PAYLOAD_TABLE_UPDATE_STATUS, DP_PAYLOAD_TABLE_UPDATED);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_clear_link_vcp_tables(struct Dptx_Params *pstDptx)
{
	for (uint8_t ucCount = 0; ucCount < NUM_OF_MST_VCP_TABLEs; ucCount++)
		Dptx_Reg_Writel(pstDptx,	DPTX_MST_VCP_TABLE_REG_N(ucCount), 0);

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_set_link_vcpid_table_slot(struct Dptx_Params *pstDptx, uint8_t ucStart_SlotNum, uint8_t ucNumOfSlots, uint8_t ucStream_Index)
{
	uint8_t ucSlotNum, ucCount;
	uint32_t uiVCP_RegOffset, uiRegMap_VCPTable;
	uint32_t uiBit_Shift = 0, uiBit_Mask = 0;

	if ((ucStart_SlotNum + ucNumOfSlots) > DPTX_MAX_LINK_SLOTS) {
		dptx_err("S Slot(%d) + Num of Slots(%d) is larger than max slots(%d)", ucStart_SlotNum, 	ucNumOfSlots, (uint32_t)DPTX_MAX_LINK_SLOTS);
		return DPTX_RETURN_EINVAL;
	}

	dptx_dbg("----- Setting %d slots for stream %d", ucStart_SlotNum, ucStream_Index);

	for (ucCount = 0; ucCount < ucNumOfSlots; ucCount++) {
		ucSlotNum = (ucStart_SlotNum + ucCount);

		uiBit_Shift = ((ucSlotNum & 0x7) * 4);
		uiBit_Mask = GENMASK(uiBit_Shift + 3, uiBit_Shift);

		uiVCP_RegOffset = DPTX_MST_VCP_TABLE_REG_N(ucSlotNum >> 3);
		uiRegMap_VCPTable = Dptx_Reg_Readl(pstDptx, uiVCP_RegOffset);

		uiRegMap_VCPTable &= ~uiBit_Mask;
		uiRegMap_VCPTable |= ((ucStream_Index << uiBit_Shift) & uiBit_Mask);

		Dptx_Reg_Writel(pstDptx, uiVCP_RegOffset, uiRegMap_VCPTable);
	}

	return DPTX_RETURN_NO_ERROR;
}

static int32_t dptx_ext_set_sink_vcpid_table_slot(struct Dptx_Params *pstDptx, uint8_t ucStart_SlotNum, uint8_t ucNumOfSlots, uint8_t ucStream_Index)
{
	uint8_t ucStatus, ucWriteBuf[3];
	int32_t iRetVal;
	uint32_t uiRetry_LinkUpdated = 0;

	ucWriteBuf[0] = ucStream_Index;
	ucWriteBuf[1] = ucStart_SlotNum;
	ucWriteBuf[2] = ucNumOfSlots;

	dptx_dbg("----- Setting %d slots for stream %d", ucStart_SlotNum, ucStream_Index);

	iRetVal = Dptx_Aux_Write_Bytes_To_DPCD(pstDptx, DP_PAYLOAD_ALLOCATE_SET, ucWriteBuf, 3);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	do {
		iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_PAYLOAD_TABLE_UPDATE_STATUS, &ucStatus);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return  iRetVal;

		if (ucStatus & DP_PAYLOAD_TABLE_UPDATED)
			break;

		if (uiRetry_LinkUpdated == MAX_CHECK_DPCD_VCP_UPDATED)
			dptx_warn("Payload table in Sink is not updated for %dms", 	(uiRetry_LinkUpdated * 1));

		udelay(1);
	} while (uiRetry_LinkUpdated++ < MAX_CHECK_DPCD_VCP_UPDATED);

	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_PAYLOAD_TABLE_UPDATE_STATUS, DP_PAYLOAD_TABLE_UPDATED);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Ext_Set_SideBand_Msg_Supported(struct Dptx_Params *pstDptx, bool bSideBand_MSG_Supported)
{
	pstDptx->bSideBand_MSG_Supported = bSideBand_MSG_Supported;

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Ext_Set_Stream_Capability(struct Dptx_Params *pstDptx)
{
	uint8_t	ucMST_Mode_Caps;
	int32_t	iRetVal;
	uint32_t	uiRegMap_Cctl;

	iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_MSTM_CAP, &ucMST_Mode_Caps);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return  iRetVal;

	uiRegMap_Cctl = Dptx_Reg_Readl(pstDptx, DPTX_CCTL);

	if (pstDptx->bMultStreamTransport) {
		if (ucMST_Mode_Caps & DP_MST_CAP) {
			dptx_notice("MST is profiled and Sink supports MST");
			uiRegMap_Cctl |= DPTX_CCTL_ENABLE_MST_MODE;
			Dptx_Reg_Writel(pstDptx, DPTX_CCTL, uiRegMap_Cctl);

			iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_MSTM_CTRL, (DP_MST_EN | DP_UP_REQ_EN | DP_UPSTREAM_IS_SRC));
			if (iRetVal != DPTX_RETURN_NO_ERROR)
				return  iRetVal;
		} else {
			dptx_warn("MST is profiled in Source but Sink doesn't support MST");

			pstDptx->bMultStreamTransport = false;
			pstDptx->ucNumOfStreams = 1;

			uiRegMap_Cctl &= ~DPTX_CCTL_ENABLE_MST_MODE;
			Dptx_Reg_Writel(pstDptx, DPTX_CCTL, uiRegMap_Cctl);

			iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_MSTM_CTRL, ~(DP_MST_EN | DP_UP_REQ_EN | DP_UPSTREAM_IS_SRC));
			if (iRetVal != DPTX_RETURN_NO_ERROR)
				return  iRetVal;
		}
	} else {
		uiRegMap_Cctl &= ~DPTX_CCTL_ENABLE_MST_MODE;
		Dptx_Reg_Writel(pstDptx, DPTX_CCTL, uiRegMap_Cctl);

		if (ucMST_Mode_Caps & DP_MST_CAP) {
			dptx_notice("MST is NOT profiled in Source but Sink supports MST");
			iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_MSTM_CTRL, ~(DP_MST_EN | DP_UP_REQ_EN | DP_UPSTREAM_IS_SRC));
			if (iRetVal != DPTX_RETURN_NO_ERROR)
				return  iRetVal;
		} else {
			dptx_notice("MST is not profiled in Source and Sink doesn't support MST");
		}
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Ext_Set_VCPID_MST(struct Dptx_Params *pstDptx, uint8_t ucNumOfStreams, uint8_t aucVCP_Id[PHY_INPUT_STREAM_MAX])
{
	if (aucVCP_Id == NULL) {
		dptx_err("Invalid parameter as aucVCP_Id == NULL");
		return DPTX_RETURN_EINVAL;
	}

	for (uint8_t ucElements = 0; ucElements < ucNumOfStreams; ucElements++)
		pstDptx->aucVCP_Id[ucElements] = aucVCP_Id[ucElements];

	return DPTX_RETURN_NO_ERROR;
}


int32_t Dptx_Ext_Get_Link_PayloadBandwidthNumber(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	uint8_t	ucRest;
	int32_t	iRetVal;
	uint32_t uiPBN_BPP = 0, uiPBN_M_BPP = 0, uiTenfold_M_BPP = 0;
	struct Dptx_Video_Params *pstVideoParams =	&pstDptx->stVideoParams;

	if ((pstVideoParams->ucPixel_Encoding == PIXEL_ENCODING_TYPE_RGB) ||
		(pstVideoParams->ucPixel_Encoding == PIXEL_ENCODING_TYPE_YCBCR444)) {
		uiPBN_BPP = ((pstDptx->stVideoParams.uiPeri_Pixel_Clock[ucStream_Index] * VIDEO_LINK_BPP_RGB_YCbCr444 * 64) / 54);
	} else if (pstVideoParams->ucPixel_Encoding == PIXEL_ENCODING_TYPE_YCBCR422) {
		uiPBN_BPP = ((pstDptx->stVideoParams.uiPeri_Pixel_Clock[ucStream_Index] * VIDEO_LINK_BPP_YCbCr422 * 64) / 54);
	} else {
		dptx_err("Unknown Pixel encoding type(%d)", pstVideoParams->ucPixel_Encoding);
		return DPTX_RETURN_EINVAL;
	}

	uiPBN_BPP *= (1006);

	uiPBN_M_BPP = (uiPBN_BPP / 1000000);
	uiTenfold_M_BPP = (uiPBN_BPP / 100000);

	ucRest = (uiTenfold_M_BPP - (uiPBN_M_BPP * 10));
	if (ucRest >= 5)
		pstDptx->ausPayloadBandwidthNumber[ucStream_Index] = (u16)(uiPBN_M_BPP + 1);
	else
		pstDptx->ausPayloadBandwidthNumber[ucStream_Index] = 	(u16)uiPBN_M_BPP;

	iRetVal = dptx_ext_get_link_numof_slots(pstDptx, pstDptx->ausPayloadBandwidthNumber[ucStream_Index], &pstDptx->aucNumOfSlots[ucStream_Index]);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	dptx_dbg("Stream %d : PBN(%d) <- Rest(%d),number of slots: %d", ucStream_Index,
																	pstDptx->ausPayloadBandwidthNumber[ucStream_Index],
																	ucRest,
																	pstDptx->aucNumOfSlots[ucStream_Index]);

	return DPTX_RETURN_NO_ERROR;
}


int32_t Dptx_Ext_Set_Link_VCP_Tables(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	uint8_t ucPrev_NumOfSlots, ucCurrent_NumOfSlots;
	int32_t iRetVal;

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

	iRetVal = dptx_ext_set_link_vcpid_table_slot(pstDptx, (ucPrev_NumOfSlots + 1), ucCurrent_NumOfSlots, (ucStream_Index + 1));
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Ext_Set_Sink_VCP_Table_Slots(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	uint8_t	ucPrev_NumOfSlots, ucCurrent_NumOfSlots;
	int32_t	iRetVal;

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
		return  DPTX_RETURN_EINVAL;
	}

	iRetVal = dptx_ext_set_sink_vcpid_table_slot(pstDptx, (ucPrev_NumOfSlots + 1), ucCurrent_NumOfSlots, (ucStream_Index + 1));
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Ext_Clear_VCP_Tables(struct Dptx_Params *pstDptx)
{
	int32_t	iRetVal;

	iRetVal = dptx_ext_clear_sink_vcpid_table(pstDptx);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	iRetVal = dptx_ext_clear_link_vcp_tables(pstDptx);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Ext_Initiate_MST_Act(struct Dptx_Params *pstDptx)
{
	bool bACT_TimedOut = false;
	uint32_t uiRegMap_Cctl, uiRetry_MstAct = 0;

	uiRegMap_Cctl = Dptx_Reg_Readl(pstDptx, DPTX_CCTL);
	uiRegMap_Cctl |= DPTX_CCTL_INITIATE_MST_ACT;

	Dptx_Reg_Writel(pstDptx, DPTX_CCTL, uiRegMap_Cctl);

	while (true) {
		uiRegMap_Cctl = Dptx_Reg_Readl(pstDptx, DPTX_CCTL);
		if (!(uiRegMap_Cctl & DPTX_CCTL_INITIATE_MST_ACT)) {
			break;
		}

		if (uiRetry_MstAct++ >= MAX_CHECK_MST_ACT) {
			if (!bACT_TimedOut) {
				dptx_warn("[1st]MST_ACT_SEQ timeout..waiting for %dms ", (uiRetry_MstAct - 1));

				uiRetry_MstAct = 0;

				uiRegMap_Cctl = Dptx_Reg_Readl(pstDptx, DPTX_CCTL);
				uiRegMap_Cctl &= ~((uint32_t)DPTX_CCTL_INITIATE_MST_ACT);
				Dptx_Reg_Writel(pstDptx, DPTX_CCTL, uiRegMap_Cctl);

				uiRegMap_Cctl |= (uint32_t)DPTX_CCTL_INITIATE_MST_ACT;
				Dptx_Reg_Writel(pstDptx, DPTX_CCTL, uiRegMap_Cctl);

				bACT_TimedOut = true;
			} else {
				dptx_warn("[2nd]MST_ACT_SEQ timeout..waiting for %dms ", (uiRetry_MstAct - 1));
				return DPTX_RETURN_MST_ACT_TIMEOUT;
			}
		}

		mdelay(1);
	}

	return  DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Ext_Get_Topology(struct Dptx_Params *pstDptx)
{
	uint8_t	ucSink_Playload_Status;
	uint8_t	ucStream_Count;
	int32_t	iRetVal;
	uint32_t uiRetry_LinkUpdated = 0;

	if (pstDptx->bSideBand_MSG_Supported) {
		iRetVal = dptx_ext_get_port_composition(pstDptx);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return iRetVal;
	}

	iRetVal = Dptx_Ext_Clear_VCP_Tables(pstDptx);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	for (ucStream_Count = 0; ucStream_Count < pstDptx->ucNumOfStreams; ucStream_Count++) {
		iRetVal = Dptx_Ext_Get_Link_PayloadBandwidthNumber(pstDptx, ucStream_Count);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return iRetVal;
	}

	for (ucStream_Count = 0; ucStream_Count < pstDptx->ucNumOfStreams; ucStream_Count++) {
		iRetVal = Dptx_Ext_Set_Link_VCP_Tables(pstDptx, ucStream_Count);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return iRetVal;
	}

	for (ucStream_Count = 0; ucStream_Count < pstDptx->ucNumOfStreams; ucStream_Count++) {
		iRetVal = Dptx_Ext_Set_Sink_VCP_Table_Slots(pstDptx, ucStream_Count);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return iRetVal;
	}

	iRetVal = Dptx_Ext_Initiate_MST_Act(pstDptx);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	do {
		iRetVal = Dptx_Aux_Read_DPCD(pstDptx, DP_PAYLOAD_TABLE_UPDATE_STATUS, &ucSink_Playload_Status);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return iRetVal;

		if (ucSink_Playload_Status & DP_PAYLOAD_ACT_HANDLED)
			break;

		if (uiRetry_LinkUpdated == MAX_CHECK_DPCD_VCP_UPDATED)
			dptx_warn("Act in Sink is not handled for %dms ", (uiRetry_LinkUpdated * 2));

		mdelay(2);
	} while (uiRetry_LinkUpdated++ < MAX_CHECK_DPCD_VCP_UPDATED);

	iRetVal = Dptx_Aux_Write_DPCD(pstDptx, DP_PAYLOAD_TABLE_UPDATE_STATUS, 0x3);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	if (pstDptx->bSideBand_MSG_Supported) {
		for (u8 ucIndex = 0; ucIndex < pstDptx->ucNumOfStreams; ucIndex++) {
			if (pstDptx->aucStreamSink_PortNumber[ucIndex] != INVALID_MST_PORT_NUM) {
				iRetVal = dptx_ext_set_sideband_msg_enum_path_resources(pstDptx,
																				pstDptx->aucStreamSink_PortNumber[ucIndex],
																				(ucIndex + 1),
																				pstDptx->aucRAD_PortNumber[ucIndex]);
				if (iRetVal != DPTX_RETURN_NO_ERROR)
					return iRetVal;

			iRetVal = dptx_ext_set_sideband_msg_allocate_payload(pstDptx,
																		pstDptx->aucStreamSink_PortNumber[ucIndex],
																		pstDptx->aucVCP_Id[ucIndex],
																		pstDptx->ausPayloadBandwidthNumber[ucIndex],
																		pstDptx->aucRAD_PortNumber[ucIndex]);
				if (iRetVal != DPTX_RETURN_NO_ERROR)
					return iRetVal;
			}
		}
	}

	return  DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Ext_Remote_I2C_Read(struct Dptx_Params *pstDptx, uint8_t ucStream_Index)
{
	uint8_t	ucReply_Len, ucPort_Index, ucRad_Port;
	uint8_t	aucBuf[300];
	uint8_t *pucMsg;
	int	len = 256;
	int32_t	iRetVal;
	struct drm_dp_sideband_msg_hdr			stMsg_Header;

	iRetVal = dptx_ext_get_port_composition(pstDptx);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	ucPort_Index = pstDptx->aucStreamSink_PortNumber[ucStream_Index];
	if (ucPort_Index == INVALID_MST_PORT_NUM) {
		dptx_err("Stream %d isn't allocated ", ucStream_Index);
		return  DPTX_RETURN_ENODEV;
	}

	ucRad_Port = pstDptx->aucRAD_PortNumber[ucStream_Index];

	memset(&stMsg_Header, 0, sizeof(struct drm_dp_sideband_msg_hdr));

	stMsg_Header.lct		= 1;
	stMsg_Header.lcr		= 0;
	stMsg_Header.rad[0]		= 0;
	stMsg_Header.broadcast	= false;
	stMsg_Header.path_msg	= 0;
	stMsg_Header.msg_len	= 9;
	stMsg_Header.somt		= 1;
	stMsg_Header.eomt		= 1;
	stMsg_Header.seqno		= 0;

	if (ucRad_Port != INVALID_MST_PORT_NUM) {
		stMsg_Header.lct = 2;
		stMsg_Header.lcr = 1;
		stMsg_Header.rad[0] |= ((ucRad_Port << 4) & 0xF0);
	}

	Drm_Addition_Encode_Sideband_Msg_Hdr(&stMsg_Header, aucBuf, &len);

	pucMsg = &aucBuf[len];
	pucMsg[0]	= DP_REMOTE_I2C_READ;
	pucMsg[1]	= ((ucPort_Index & 0xF) << 4);
	pucMsg[1]	|= (1 & 0x3);
	pucMsg[2]	= (0x50 & 0x7F);
	pucMsg[3]	= 1;
	pucMsg[4]	= (0 << 5);
	pucMsg[5]	= (0 & 0xF);
	pucMsg[6]	= (0x50 & 0x7F);
	pucMsg[7]	= 128;

	Drm_Addition_Encode_SideBand_Msg_CRC(pucMsg, 8);

	len += 9;

	iRetVal = Dptx_Aux_Write_Bytes_To_DPCD(pstDptx, DP_SIDEBAND_MSG_DOWN_REQ_BASE, aucBuf, len);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	memset(aucBuf, 0, 300);

	iRetVal = dptx_ext_get_sideband_msg_down_reply(pstDptx, DP_REMOTE_I2C_READ, aucBuf, &ucReply_Len);
	if (iRetVal != DPTX_RETURN_NO_ERROR)
		return iRetVal;

	if (aucBuf[2] == 0) {
		dptx_warn("No EDID data in Sink");
		return  DPTX_RETURN_ENODEV;
	}

	dptx_dbg("I2C Remote messages replied => ");
	dptx_dbg(" -.Reply type: %s", (aucBuf[0] & 0x80) ? "NAK" : "ACK");
	dptx_dbg(" -.Request id: %s", ((aucBuf[0] & 0x7F) == DP_REMOTE_I2C_READ) ? "REMOTE_I2C_READ" : "Wrong ID");
	dptx_dbg(" -.Port Number: %d <- (%d, %d)", (aucBuf[1] & 0x0F), ucPort_Index, ucRad_Port);
	dptx_dbg(" -.Num of bytes read: %d", aucBuf[2]);
	dptx_dbg(" -.Num of extensions: %d", aucBuf[126 + 3]);

	memcpy(pstDptx->pucEdidBuf, &aucBuf[3], DPTX_EDID_BUFLEN);

	if (aucBuf[126 + 3] > 0) {
		Drm_Addition_Encode_Sideband_Msg_Hdr(&stMsg_Header, aucBuf, &len);

		pucMsg = &aucBuf[len];
		pucMsg[0]	= DP_REMOTE_I2C_READ;
		pucMsg[1]	= ((ucPort_Index & 0xF) << 4);
		pucMsg[1]	|= (1 & 0x3);
		pucMsg[2]	= (0x50 & 0x7F);
		pucMsg[3]	= 1;
		pucMsg[4]	= 128;
		pucMsg[5]	= (0 & 0xF);
		pucMsg[6]	= (0x50 & 0x7F);
		pucMsg[7]	= 128;

		Drm_Addition_Encode_SideBand_Msg_CRC(pucMsg, 8);

		len += 9;

		iRetVal = Dptx_Aux_Write_Bytes_To_DPCD(pstDptx, DP_SIDEBAND_MSG_DOWN_REQ_BASE, aucBuf, len);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return iRetVal;

		memset(aucBuf, 0, 300);

		iRetVal = dptx_ext_get_sideband_msg_down_reply(pstDptx, DP_REMOTE_I2C_READ, aucBuf, &ucReply_Len);
		if (iRetVal != DPTX_RETURN_NO_ERROR)
			return iRetVal;

		dptx_dbg("I2C Remote messages replied => ");
		dptx_dbg(" -.Reply type: %s", (aucBuf[0] & 0x80) ? "ACK":"NAK");
		dptx_dbg(" -.Request id: %s", ((aucBuf[0] & 0x7F) == DP_REMOTE_I2C_READ) ? "REMOTE_I2C_READ":"Wrong ID");
		dptx_dbg(" -.Port Number: %d <- (%d, %d)", (aucBuf[1] & 0x0F), ucPort_Index, ucRad_Port);
		dptx_dbg(" -.Num of bytes read: %d", aucBuf[2]);
		dptx_dbg(" -.Num of extensions: %d", aucBuf[126 + 3]);

		memcpy(pstDptx->pucSecondary_EDID, &aucBuf[3], DPTX_EDID_BUFLEN);
	}

	return  DPTX_RETURN_NO_ERROR;
}






