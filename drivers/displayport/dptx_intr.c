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


#define DPTX_EDID_START_BIT_OF_1ST_DETAILED_DES	54u
#define DPTX_EDID_SIZE_OF_DETAILED_DES 		18u

/**
 * @brief Handles DisplayPort HPD Plug event
 *
 * This function is called when a DisplayPort HPD PLUG event occurs. It ensures
 * that the DisplayPort Source Device performs the necessary actions.
 * Using DPCD, it retrieves information from the DisplayPort SINK and initiates
 * link training between the DisplayPort Source and Sink. Once link training
 * is completed, the function sets up the video timing for the DisplayPort
 * Source.
 *
 * @param[in] dev_param DisplayPort driver context pointer containing general
 *                    information for DisplayPort control.
 * @return 0 if the function executes successfully; otherwise, a negative value is returned.
 *
 */
int32_t Dptx_Intr_Handle_Hotplug(struct Dptx_Params *dev_param)
{
	struct dptx_video_params *video_params = NULL;
	int32_t ret = DPTX_RETURN_NO_ERROR;
	uint8_t dp_stream_id;

	ret = dptx_power_up_sink_device_and_check_aux_hysteresis(dev_param);
	if (DPTX_RETURN_ERROR(ret)) {
		dptx_err("DPCDW - DP_SET_POWER_D0");
		/* For KCS */
	} else {
		(void)memset(dev_param->aucDPCD_Caps, 0, DPTX_SINK_CAP_SIZE);
		ret = drm_addition_read_dpcd_caps(dev_param);
		if (DPTX_RETURN_ERROR(ret)) {
			dptx_err("DPCDR - DP_DPCD_REV");
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		dptx_dbg("Sink DP Revision %x.%x ", ((uint32_t)dev_param->aucDPCD_Caps[0] & 0xF0u) >> 4, dev_param->aucDPCD_Caps[0] & 0xFu);

		ret = dptx_core_check_ssc_enable(dev_param);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Ext_Set_Stream_Capability(dev_param);
		if (DPTX_RETURN_ERROR(ret)) {
			dptx_err("Stream - ext_capability");
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Link_Perform_Training(dev_param, dev_param->ucMax_Rate, dev_param->ucMax_Lanes);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		if (dev_param->bMultStreamTransport) {
			ret = Dptx_Ext_Set_Topology_Configuration(dev_param);
			if (DPTX_RETURN_ERROR(ret)) {
				dptx_err("MST - Topology Configure");
			}
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		for (dp_stream_id = 0u; dp_stream_id < dev_param->ucNumOfStreams; dp_stream_id++) {
			video_params = &dev_param->video_params[dp_stream_id];

			if (video_params->video_code == (uint32_t)DPTX_VIC_READ_PANEL_EDID) {
				dptx_dbg("Using video code by reading EDID... ");

				video_params->dtd_param.uiPixel_Clock = video_params->pixel_clock;

				ret = dptx_vidin_calculate_average_tu_symbols(dev_param, dp_stream_id);
				if (DPTX_RETURN_SUCCESS(ret)) {
					ret = Dptx_Vidin_Set_Video_TimingChange(dev_param, dp_stream_id);
					if (DPTX_RETURN_ERROR(ret)) {
						dptx_err("VIdeo - Average TU");
					}
				}
			} else {
				ret = Dptx_Vidin_Set_Video_TimingChange_FromVIC(dev_param,
					video_params->video_code, (u8)dp_stream_id);
				if (DPTX_RETURN_ERROR(ret)) {
					dptx_err("VIdeo - Timing");
				}
			}
			if (DPTX_RETURN_ERROR(ret)) {
				break;
			}
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = dptx_cfg_set_mux_bypass(dev_param);
	}

	return ret;
}

int32_t Dptx_Intr_Handle_HotUnplug(struct Dptx_Params *dev_param)
{
	int32_t	iRetVal;
	u32	uiRegMap_DptxCctl, uiRegMap_PhyIFCtrl;

	dev_param->bEstablish_Timing_Present = false;

	uiRegMap_DptxCctl = Dptx_Reg_Readl(dev_param, DPTX_CCTL);
	uiRegMap_DptxCctl &= ~DPTX_CCTL_ENABLE_FEC;
	Dptx_Reg_Writel(dev_param, DPTX_CCTL, uiRegMap_DptxCctl);

	mdelay(100);

	uiRegMap_PhyIFCtrl = Dptx_Reg_Readl(dev_param, DPTX_PHYIF_CTRL);
	uiRegMap_PhyIFCtrl &= ~DPTX_PHYIF_CTRL_XMIT_EN_MASK;

	(void)Dptx_Core_Set_PHY_PowerState(dev_param, PHY_POWER_DOWN_PHY_CLOCK);

	iRetVal = Dptx_Core_Get_PHY_BUSY_Status(dev_param, dev_param->stDptxLink.ucNumOfLanes);
	if (iRetVal != DPTX_RETURN_NO_ERROR) {
		dptx_err("From Dptx_Core_Get_PHY_BUSY_Status()");
	}

	return DPTX_RETURN_NO_ERROR;
}

int32_t Dptx_Intr_Handle_Edid(struct Dptx_Params *dev_param, u8 dp_stream_id)
{
	uint8_t detaied_video_code[DPTX_EDID_SIZE_OF_DETAILED_DES] = {0, };
	uint8_t pixel_clock_msb, pixel_clock_lsb, detailed_timing_index;

	struct dptx_video_params *video_params = NULL;
	struct Dptx_Dtd_Params dtd_param;
	int32_t	ret = DPTX_RETURN_NO_ERROR;

	if (dev_param->bMultStreamTransport) {
		if (dp_stream_id >= (u8)PHY_INPUT_STREAM_MAX) {
			dptx_err("Invalid stream index( %d ) on MST", dp_stream_id);
			ret = -DPTX_RETURN_EINVAL;
		}
	} else {
		if (dp_stream_id >= (u8)PHY_INPUT_STREAM_1) {
			dptx_err("Invalid stream index( %d ) on SST", dp_stream_id);
			ret = -DPTX_RETURN_EINVAL;
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		video_params = &dev_param->video_params[dp_stream_id];

		if (dev_param->bMultStreamTransport) {
			ret = Dptx_Edid_Read_EDID_Over_Sideband_Msg(dev_param, dp_stream_id);
		} else {
			ret = Dptx_Edid_Read_EDID_I2C_Over_Aux(dev_param);
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		ret = Dptx_Edid_Check_Detailed_Timing_Descriptors(dev_param);
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		video_params->video_format_standard = VFS_CEA_861;
		if (dev_param->bEstablish_Timing_Present) {
			ret = Dptx_Vidin_Fill_DTD_BasedOn_EST_Timings(dev_param,
								      &dtd_param,
								      dp_stream_id);
			if (DPTX_RETURN_ERROR(ret)) {
				ret = Dptx_Vidin_Fill_Dtd(&dtd_param, (uint8_t)DPTX_DEFAULT_VIDEO_CODE,
							   video_params->video_refresh_rate,
							   video_params->video_format_standard);
			}
		} else {
			ret = Dptx_Edid_Verify_EDID(dev_param->pucEdidBuf);
			if (DPTX_RETURN_ERROR(ret)) {
				ret = Dptx_Vidin_Fill_Dtd(&dtd_param, (uint8_t)DPTX_DEFAULT_VIDEO_CODE,
							video_params->video_refresh_rate,
							video_params->video_format_standard);
			} else {

				for (detailed_timing_index = 0u; detailed_timing_index < 4u; detailed_timing_index++) {
					pixel_clock_msb = dev_param->pucEdidBuf[(DPTX_EDID_START_BIT_OF_1ST_DETAILED_DES +
						(detailed_timing_index *
						DPTX_EDID_SIZE_OF_DETAILED_DES) + 1u)];
					pixel_clock_lsb = dev_param->pucEdidBuf[(DPTX_EDID_START_BIT_OF_1ST_DETAILED_DES +
						(detailed_timing_index *
						DPTX_EDID_SIZE_OF_DETAILED_DES))];

					if ((pixel_clock_msb != 0u) && (pixel_clock_lsb != 0u)) {
						(void)memcpy(detaied_video_code, &dev_param->pucEdidBuf[DPTX_EDID_START_BIT_OF_1ST_DETAILED_DES +
							     (detailed_timing_index * DPTX_EDID_SIZE_OF_DETAILED_DES)],
							     DPTX_EDID_SIZE_OF_DETAILED_DES);
						break;
					}
				}
				if (detailed_timing_index >= 4u) {
					dptx_err("DisplayPort Sink[%u] doesnt support detailed timing", dp_stream_id);
					ret = -DPTX_RETURN_ENODEV;
				} else {
					ret = Dptx_Vidin_Parse_Dtd(&dtd_param, detaied_video_code);
				}
				if (DPTX_RETURN_ERROR(ret)) {
					ret = Dptx_Vidin_Fill_Dtd(&dtd_param, (u8)DPTX_DEFAULT_VIDEO_CODE, video_params->video_refresh_rate, video_params->video_format_standard);
				}
			}
		}
	}
	if (DPTX_RETURN_SUCCESS(ret)) {
		(void)memcpy(&video_params->dtd_param, &dtd_param, sizeof(struct Dptx_Dtd_Params));
	}

	return ret;
}

 /**
 * @brief Check if HPD is high
  *
 * This function checks whether HPD is high by reading the HPD_STATUS register.
 * Note: The DPTX_HPDSTS_STATUS bit indicates the current status of the HPD input.
 *       However, even if this value is high, it does not guarantee that HPD is
 *       in the PLUGGED state.
  *
 * @param[in] dev_param DisplayPort driver context pointer containing general
 *                    information for DisplayPort control.
 * @return true if the HPD is high; otherwise, false indicating HPD is low.
 *
 */
static bool dptx_intr_get_hotplug_pin(struct Dptx_Params *dev_param)
{
	bool hpd_high = (bool)false;
	uint32_t reg_val;

	reg_val = Dptx_Reg_Readl(dev_param, DPTX_HPDSTS);
	if ((reg_val & (uint32_t)DPTX_HPDSTS_STATUS) != 0u) {
		hpd_high = (bool)true;
	}

	return hpd_high;
}

/**
 * @brief Get the current HPD status.
 *
 * This function checks the HPD (Hot Plug Detect) status by reading the HPD_STATUS register
 * from the DisplayPort transmitter (DPTX). It interprets the read value to determine
 * if the HPD signal is in an unplugged, transient (PLUS), or stable (PLUGGED) state.
 *
 * The DPTX_HPDSTS_STATUS bit indicates the current status of the HPD input. Note that even if
 * this bit is high, it does not necessarily guarantee that the HPD is in the PLUGGED state.
 *
 * The function checks specific values from the register:
 * - If the value is 1 or 4, it returns HPD_STATUS_PLUS, indicating a transient or IRQ state.
 * - If the value is 7, it returns HPD_STATUS_PLUGGED, indicating a stable connection.
 * - In all other cases, it returns HPD_STATUS_UNPLUGGED.
 *
 * @param[in] dev_param DisplayPort driver context pointer containing general
 *                      information for DisplayPort control.
 *
 * @return enum hpd_detection_status
 *         - HPD_STATUS_UNPLUGGED if the connection is not detected.
 *         - HPD_STATUS_PLUS if a transient IRQ-like state is detected (0.5ms ~ 1ms or 2ms ~ < 100ms low).
 *         - HPD_STATUS_PLUGGED if the connection is stable and active.
 */
enum hpd_detection_status dptx_intr_get_hotplug_status(struct Dptx_Params *dev_param)
{
	enum hpd_detection_status hpd_status = HPD_STATUS_UNPLUGGED;
	uint32_t hpd_val;

	if (dev_param != NULL) {
		hpd_val = Dptx_Reg_Readl(dev_param, DPTX_HPDSTS);
		hpd_val &= DPTX_HPDSTS_STATE_MASK;
		hpd_val >>= DPTX_HPDSTS_STATE_SHIFT;

		if ((hpd_val == 1u) || (hpd_val == 4u)) {
			hpd_status = HPD_STATUS_PLUS;
		}
		if (hpd_val == 7u) {
			hpd_status = HPD_STATUS_PLUGGED;
		}

	}
	return hpd_status;
}


/**
 * @brief Check if HPD and wait HPD to PLUGGED STATE
 *
 * This function first checks if HPD is high and if HPD is hig, check whether
 * HPD is in PLUGGED state for up to 100ms.
 * Note: When DislayPort is initialized, it takes at least 100ms to confirm
 * HPD PLUGGED.
 *
 * @param[in] dev_param DisplayPort driver context pointer containing general
 *                    information for DisplayPort control.
 * @return true if the HPD is PLUGGED; otherwise, false indicating HPD is UNPLUGGED.
 *
 */
bool dptx_intr_check_hpd_and_wait_hpd_to_plugged(struct Dptx_Params *dev_param)
{
	bool hpd_plugged = (bool)false;
	bool loop_end = (bool)false;
	uint32_t loop;

	if (dptx_intr_get_hotplug_pin(dev_param)) {
		for (loop = 0u; loop < 20u; loop++) {
			if (dptx_intr_get_hotplug_status(dev_param) == HPD_STATUS_PLUGGED) {
				hpd_plugged = (bool)true;
				loop_end = (bool)true;
			}
			if (!dptx_intr_get_hotplug_pin(dev_param)) {
				loop_end = (bool)true;
			}
			if (loop_end) {
				break;
			}
			mdelay(10);
		}
	}
	return hpd_plugged;
}
