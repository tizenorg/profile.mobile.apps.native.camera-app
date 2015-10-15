/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include <string.h>
#include <audio_io.h>
#include "cam_mm.h"
#include "cam_debug.h"
#include "cam_property.h"
#include "cam_typeconverter.h"
#include "cam_app.h"
#include "cam_sound_session_manager.h"
#include "cam_utils.h"
#include "cam_ev_edc_callback.h"
#include "cam_standby_view.h"

typedef struct _CamMMHandle {
	camera_h hcam;
	recorder_h hrec;
	audio_in_h haudio;
	camera_device_e hdev;
} CamMMHandle;

static unsigned int g_caps = 0;
static int g_caps_cb_cnt = 0;
static int gError = CAMERA_ERROR_NONE;

static bool __get_iso_cb(camera_attr_iso_e iso, void *user_data);
//static bool __get_effect_cb(camera_effect_gs_e effect, void *user_data);
static bool __get_fps_by_resolution_cb(camera_attr_fps_e fps, void *user_data);
static bool __get_fps_cb(camera_attr_fps_e fps, void *user_data);
static bool __get_wb_cb(camera_attr_whitebalance_e wb, void *user_data);
static bool __get_focus_cb(camera_attr_af_mode_e focus, void *user_data);
static bool __get_metering_cb(camera_attr_exposure_mode_e metering, void *user_data);
static bool __get_scene_cb(camera_attr_scene_mode_e scene, void *user_data);
static bool __get_flash_cb(camera_attr_flash_mode_e flash, void *user_data);
static bool __get_capture_res_cb(int width, int height, void *user_data);
static bool __get_recording_res_cb(int width, int height, void *user_data);



void rec_detail_error_get(int err_no)
{
	switch (err_no) {
	case RECORDER_ERROR_INVALID_PARAMETER:
		cam_debug(LOG_CAM, "RECORDER_ERROR_INVALID_PARAMETER");
		break;
	case RECORDER_ERROR_INVALID_STATE:
		cam_debug(LOG_CAM, "RECORDER_ERROR_INVALID_STATE");
		break;
	case RECORDER_ERROR_OUT_OF_MEMORY:
		cam_debug(LOG_CAM, "RECORDER_ERROR_OUT_OF_MEMORY");
		break;
	case RECORDER_ERROR_DEVICE:
		cam_debug(LOG_CAM, "RECORDER_ERROR_DEVICE");
		break;
	case RECORDER_ERROR_INVALID_OPERATION:
		cam_debug(LOG_CAM, "RECORDER_ERROR_INVALID_OPERATION");
		break;
	case RECORDER_ERROR_SOUND_POLICY:
		cam_debug(LOG_CAM, "RECORDER_ERROR_SOUND_POLICY");
		break;
	case RECORDER_ERROR_NONE:
		cam_debug(LOG_CAM, "NO ERROR");
		break;
	default:
		cam_critical(LOG_CAM, "unknown error,err_no = %d", err_no);

	}

}
void cam_detail_error_get(int err_no)
{
	switch (err_no) {
	case CAMERA_ERROR_INVALID_PARAMETER:
		cam_debug(LOG_CAM, "CAMERA_ERROR_INVALID_PARAMETER");
		break;
	case CAMERA_ERROR_INVALID_STATE:
		cam_debug(LOG_CAM, "CAMERA_ERROR_INVALID_STATE");
		break;
	case CAMERA_ERROR_OUT_OF_MEMORY:
		cam_debug(LOG_CAM, "CAMERA_ERROR_OUT_OF_MEMORY");
		break;
	case CAMERA_ERROR_DEVICE:
		cam_debug(LOG_CAM, "CAMERA_ERROR_DEVICE");
		break;
	case CAMERA_ERROR_INVALID_OPERATION:
		cam_debug(LOG_CAM, "CAMERA_ERROR_INVALID_OPERATION");
		break;
	case CAMERA_ERROR_SOUND_POLICY:
		cam_debug(LOG_CAM, "CAMERA_ERROR_SOUND_POLICY");
		break;
	case CAMERA_ERROR_NONE:
		cam_debug(LOG_CAM, "NO ERROR");
		break;
	default:
		cam_debug(LOG_CAM, "unknown error,err_no = %d", err_no);

	}
}


static CamMMHandle *g_mm_handle = NULL;

int cam_mm_get_cam_state(void)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	int ret = 0;
	camera_state_e state = 0;
	ret = camera_get_state(g_mm_handle->hcam, &state);
	if (ret == CAMERA_ERROR_NONE) {
		return (int)state;
	} else {
		return -1;
	}
}

int cam_mm_get_state(void)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	int ret = 0;
	camera_state_e state = 0;
	struct appdata *ad =  (struct appdata *)cam_appdata_get();
	g_return_val_if_fail(ad, FALSE);
	CamAppData *camapp = ad->camapp_handle;
	g_return_val_if_fail(camapp, FALSE);
	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		g_return_val_if_fail(g_mm_handle->hcam, FALSE);
		ret = camera_get_state(g_mm_handle->hcam, (camera_state_e *)&state);
		if (ret == CAMERA_ERROR_NONE) {
			return (int)state;
		} else {
			return -1;
		}
	}
	if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		g_return_val_if_fail(g_mm_handle->hrec, FALSE);
		ret = recorder_get_state(g_mm_handle->hrec, (recorder_state_e *)&state);
		if (ret == RECORDER_ERROR_NONE) {
			return (int)state;
		} else {
			return -1;
		}
	}
	return -1;
}

int cam_mm_get_rec_state(void)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);
	int ret = 0;
	recorder_state_e state = 0;
	ret = recorder_get_state(g_mm_handle->hrec, (recorder_state_e *)&state);
	if (ret == RECORDER_ERROR_NONE) {
		return (int)state;
	} else {
		return -1;
	}
}

gboolean cam_mm_get_video_device(int *device)
{
	g_return_val_if_fail(g_mm_handle, FALSE);

	/*if (g_mm_handle->hdev > CAMERA_DEVICE_CAMERAPLEX) {
		return FALSE;
	}*/

	*device = g_mm_handle->hdev;
	cam_debug(LOG_CAM, "%d,%d",g_mm_handle->hdev,*device);
	return TRUE;
}

gboolean cam_mm_get_preview_size(int *width, int *height)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	ret = camera_get_preview_resolution(g_mm_handle->hcam,  width, height);

	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_get_preview_resolution error code = %d" , ret);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_set_preview_size(int width, int height)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_set_preview_resolution(g_mm_handle->hcam, width, height);

	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_preview_resolution failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_get_zoom(int *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	ret = camera_attr_get_zoom(g_mm_handle->hcam, value);

	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_zoom failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_set_zoom(int value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_set_zoom(g_mm_handle->hcam, value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_zoom failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_is_support_anti_hand_shake()
{
	bool ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_is_supported_anti_shake(g_mm_handle->hcam);

	return ret;

}

gboolean cam_mm_get_anti_hand_shake(gboolean *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_is_enabled_anti_shake(g_mm_handle->hcam, (bool *)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_is_enabled_anti_shake failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;

}

gboolean cam_mm_set_anti_hand_shake(gboolean value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_enable_anti_shake(g_mm_handle->hcam, value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_enable_anti_shake failed - code[%x]", ret);
		return FALSE;
	}

	return  TRUE;

}
gboolean cam_mm_set_gps_data(double lat, double lon, double alt)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	e = camera_attr_set_geotag(g_mm_handle->hcam, lat, lon, alt);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_geotag failed - code[%x]", e);
		return FALSE;;
	}
	return TRUE;
}

gboolean cam_mm_remove_gps_data()
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	e = camera_attr_remove_geotag(g_mm_handle->hcam);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_remove_geotag failed - code[%x]", e);
		return FALSE;;
	}
	return TRUE;
}
gboolean cam_mm_is_support_video_stabilization()
{
	bool ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_is_supported_video_stabilization(g_mm_handle->hcam);

	return ret;

}

gboolean cam_mm_get_video_stabilization(gboolean *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_is_enabled_video_stabilization(g_mm_handle->hcam, (bool *)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_is_enabled_video_stabilization failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;

}

gboolean cam_mm_set_video_stabilization(gboolean value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_enable_video_stabilization(g_mm_handle->hcam, value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_enable_video_stabilization failed - code[%x]", ret);
		return FALSE;
	}

	return  TRUE;
}

gboolean cam_mm_get_metering(int *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_exposure_mode(g_mm_handle->hcam, (camera_attr_exposure_mode_e *)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_exposure_mode failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_set_metering(int value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_set_exposure_mode(g_mm_handle->hcam, (camera_attr_exposure_mode_e)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_exposure_mode failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_get_fps(int *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_preview_fps(g_mm_handle->hcam, (camera_attr_fps_e *)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_exposure_mode failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_set_fps(camera_attr_fps_e value)
{
	int ret = 0;
#ifdef CAMERA_MACHINE_I686
	value = FPS_30; //Setting the default fps value to 30
#endif
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	cam_critical(LOG_MM,"camera_attr_set_preview_fps value = %d", value);
	ret = camera_attr_set_preview_fps(g_mm_handle->hcam, value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_detail_error_get(ret);
		cam_critical(LOG_MM,"camera_attr_set_preview_fps failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_get_iso(int *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_iso(g_mm_handle->hcam, (camera_attr_iso_e *)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_iso failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_iso(int value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_set_iso(g_mm_handle->hcam, (camera_attr_iso_e)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_iso failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_get_focus_mode(int *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_af_mode(g_mm_handle->hcam, (camera_attr_af_mode_e *)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_af_mode failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_set_focus_mode(int value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_set_af_mode(g_mm_handle->hcam, (camera_attr_af_mode_e)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_af_mode failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_get_zoom_valid_intrange(int *min, int *max)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_zoom_range(g_mm_handle->hcam, min, max);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_zoom_range failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_get_brightless_valid_intrange(int *min, int *max)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_brightness_range(g_mm_handle->hcam, min, max);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_brightness_range failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_get_exposure_valid_intrange(int *min, int *max)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_exposure_range(g_mm_handle->hcam, min, max);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_exposure_range failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_set_af_area(int x, int y, int w, int h)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	cam_secure_debug(LOG_MM,"Touch AF area ![ x,y,width,height: %d,%d,%d,%d ]", x, y, w, h);

	ret = camera_attr_set_af_area(g_mm_handle->hcam, x, y);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_af_area failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_clear_af_area()
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_clear_af_area(g_mm_handle->hcam);
	if (ret != CAMERA_ERROR_NONE) {
		cam_warning(LOG_MM,"camera_attr_clear_af_area failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_get_image_enc_quality(int *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_image_quality(g_mm_handle->hcam, value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_image_quality failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_set_image_enc_quality(int value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_set_image_quality(g_mm_handle->hcam, value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_image_quality failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_get_flash(int *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_flash_mode(g_mm_handle->hcam, (camera_attr_flash_mode_e *)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_flash_mode failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_flash(int value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_set_flash_mode(g_mm_handle->hcam, (camera_attr_flash_mode_e)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_flash_mode failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_enable_auto_contrast(gboolean enable)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_enable_auto_contrast(g_mm_handle->hcam, enable);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_enable_auto_contrast failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_is_enabled_auto_contrast(gboolean *enable)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_is_enabled_auto_contrast(g_mm_handle->hcam, (bool *)enable);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_is_enabled_auto_contrast failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_get_brightness(int *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_brightness(g_mm_handle->hcam, value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_brightness failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;


}

gboolean cam_mm_set_brightness(int value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_set_brightness(g_mm_handle->hcam, value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_brightness failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_get_white_balance(int *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_whitebalance(g_mm_handle->hcam, (camera_attr_whitebalance_e *)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_whitebalance failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_set_white_balance(int value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_set_whitebalance(g_mm_handle->hcam, (camera_attr_whitebalance_e)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_whitebalance failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_get_effect(int *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_effect(g_mm_handle->hcam, (camera_attr_effect_mode_e *)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_effect failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;


}


gboolean cam_mm_set_effect(int value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_set_effect(g_mm_handle->hcam, value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_effect failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;


}

gboolean cam_mm_get_gl_effect(int* value, const char *filter_path)
{
	/*
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_camera_effectbox_genre(g_mm_handle->hcam, (camera_effect_gs_e *)value, (void **)&filter_path);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_camera_effectbox_genre failed - code[%x]", ret);
		return FALSE;
	}*/
	return FALSE;


}

gboolean cam_mm_set_gl_effect(int value, const char *filter_path)
{
//	int ret = 0;
//	g_return_val_if_fail(g_mm_handle, FALSE);
//	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
//
//	ret = camera_attr_set_camera_effectbox_genre(g_mm_handle->hcam, value, (void *)filter_path);
//	if (ret != CAMERA_ERROR_NONE) {
//		cam_critical(LOG_MM,"camera_attr_set_camera_effectbox_genre failed - code[%x]", ret);
//		return FALSE;
//	}
	return TRUE;


}

gboolean cam_mm_get_program_mode(int *value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_get_scene_mode(g_mm_handle->hcam, (camera_attr_scene_mode_e *)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_scene_mode failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;


}

gboolean cam_mm_set_program_mode(int value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_set_scene_mode(g_mm_handle->hcam, value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_scene_mode failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;


}

gboolean cam_mm_set_audio_recording(gboolean b_on)
{
	int ret = 0;

	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);
	if (b_on)
		ret = recorder_attr_set_mute(g_mm_handle->hrec, FALSE);
	else
		ret = recorder_attr_set_mute(g_mm_handle->hrec, TRUE);

	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"set attr failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_get_recommanded_preview_size(int *width, int *height)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_get_recommended_preview_resolution(g_mm_handle->hcam, width, height);

	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_get_recommended_preview_resolution failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;
}


gboolean cam_mm_get_image_size(int *width, int *height)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_get_capture_resolution(g_mm_handle->hcam, width, height);

	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_get_capture_resolution failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_set_image_size(int width, int height)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_set_capture_resolution(g_mm_handle->hcam, width, height);

	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_capture_resolution failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_get_video_size(int *width, int *height)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	ret = recorder_get_video_resolution(g_mm_handle->hrec, width, height);

	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_get_video_resolution failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_set_video_size(int width, int height)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	ret = recorder_set_video_resolution(g_mm_handle->hrec, width, height);

	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_set_video_resolution failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_set_video_encoder_bitrate(int bitrate)
{
	recorder_error_e ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	ret = recorder_attr_set_video_encoder_bitrate(g_mm_handle->hrec, bitrate);
	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_attr_set_video_encoder_bitrate failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_audio_encoder_bitrate(int bitrate)
{
	recorder_error_e ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	ret = recorder_attr_set_audio_encoder_bitrate(g_mm_handle->hrec, bitrate);
	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_attr_set_audio_encoder_bitrate failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_display_rotate(int rotate)
{
	int ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_set_display_rotation(g_mm_handle->hcam, rotate);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_display_rotation failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_get_display_rotate(int *rotate)
{
	int ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_get_display_rotation(g_mm_handle->hcam, (camera_rotation_e *)rotate);

	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_get_display_rotation failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}


gboolean cam_mm_set_camcorder_rotate(int camcorder_rotate)
{
//	int ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

//#ifdef CAMERA_MACHINE_I686
//	ret = recorder_attr_set_recording_orientation(g_mm_handle->hrec, 0);
//#else
//	ret = recorder_attr_set_recording_orientation(g_mm_handle->hrec, (recorder_rotation_e)camcorder_rotate);
//#endif
//	if (ret != CAMERA_ERROR_NONE) {
//		cam_detail_error_get(ret);
//		cam_critical(LOG_MM,"recorder_attr_set_recording_orientation failed - code[%x]", ret);
		return FALSE;
//	}
//	return TRUE;
}

gboolean cam_mm_set_camera_rotate(int camera_rotate)
{
	int ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	#ifdef CAMERA_MACHINE_I686
	ret = camera_attr_set_stream_rotation(g_mm_handle->hcam, 0);
	#else
	ret = camera_attr_set_stream_rotation(g_mm_handle->hcam, camera_rotate);
	#endif
	if (ret != CAMERA_ERROR_NONE) {
		cam_detail_error_get(ret);
		cam_critical(LOG_MM,"camera_attr_set_stream_rotation failed - code[%x]", ret);
		return FALSE;

	}
	return TRUE;

}

gboolean cam_mm_get_display_geometry_method(int *value)
{
	int ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_get_display_mode(g_mm_handle->hcam, (camera_display_mode_e *)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_get_display_mode failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_set_display_geometry_method(int value)
{
	int ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_set_display_mode(g_mm_handle->hcam, (camera_display_mode_e)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_display_mode failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_display_visible(gboolean visible)
{
	int ret = 0;

	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_set_display_visible(g_mm_handle->hcam, visible);
	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_display_visible failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_set_filename(const gchar *filename)
{
	int ret = 0;
	/*char *err_name = NULL;*/

	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	if (filename) {
		ret = recorder_set_filename(g_mm_handle->hrec, filename);
		if (ret != RECORDER_ERROR_NONE) {
			cam_critical(LOG_MM,"recorder_set_filename failed - code[%x]", ret);
			return FALSE;
		}
		return TRUE;

	} else {
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_get_filename(char **filename, gint *size)
{
	int ret = 0;

	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	ret = recorder_get_filename(g_mm_handle->hrec, filename);
	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_get_filename failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_get_max_size(int *value)
{
	recorder_error_e ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);
	ret = recorder_attr_get_time_limit(g_mm_handle->hrec, value);
	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_attr_get_time_limit failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_get_max_time(int *value)
{
	recorder_error_e ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	ret = recorder_attr_get_time_limit(g_mm_handle->hrec, value);
	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_attr_get_time_limit failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_set_max_size(int max_val)
{
	recorder_error_e ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	ret = recorder_attr_set_size_limit(g_mm_handle->hrec, max_val);
	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_attr_set_size_limit failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_max_time(int max_val)
{
	recorder_error_e ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);
	ret = recorder_attr_set_time_limit(g_mm_handle->hrec, max_val);
	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_attr_set_time_limit failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_get_tag_enable(int *value)
{
	int ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_is_enabled_tag(g_mm_handle->hcam, (bool *)value);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_is_enabled_tag failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_set_tag_enable(gboolean bvalue)
{
	int ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_enable_tag(g_mm_handle->hcam, (bool)bvalue);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_enable_tag failed - code[%x]", ret);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_set_tag_img_orient(int orient)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	camera_error_e ret;
#ifdef CAMERA_MACHINE_I686
	ret = camera_attr_set_tag_orientation(g_mm_handle->hcam, CAMERA_ATTR_TAG_ORIENTATION_TOP_LEFT);
#else
	ret = camera_attr_set_tag_orientation(g_mm_handle->hcam, (camera_attr_tag_orientation_e)orient);
#endif

	if (ret != CAMERA_ERROR_NONE) {
		cam_detail_error_get(ret);
		cam_critical(LOG_MM,"camera_attr_set_tag_orientation failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_tag_video_orient(int orient)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);
	recorder_error_e ret;
#ifdef CAMERA_MACHINE_I686
	ret = recorder_attr_set_orientation_tag(g_mm_handle->hrec, RECORDER_ROTATION_NONE);
#else
	ret = recorder_attr_set_orientation_tag(g_mm_handle->hrec, (recorder_rotation_e)orient);
#endif
	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_attr_set_orientation_tag failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_get_tag_video_orient(int *orient)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);
	recorder_error_e ret;
	ret = recorder_attr_get_orientation_tag(g_mm_handle->hrec, (recorder_rotation_e *)orient);
	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_attr_get_orientation_tag failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_file_format(int format)
{
	recorder_error_e ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);
	ret = recorder_set_file_format(g_mm_handle->hrec, format);
	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_set_file_format failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_video_profile(void)
{
	recorder_error_e e;

	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	e = recorder_attr_set_audio_device(g_mm_handle->hrec, RECORDER_AUDIO_DEVICE_MIC);
	if (e != RECORDER_ERROR_NONE) {
		rec_detail_error_get(e);
		cam_critical(LOG_MM,"recorder_attr_set_audio_device failed - code[%x]", e );
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_codec(int audio_codec, int video_codec)
{
	int ret = 0;
	recorder_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	e = recorder_set_audio_encoder(g_mm_handle->hrec, audio_codec);
	if (e != RECORDER_ERROR_NONE) {
		rec_detail_error_get(ret);
		cam_critical(LOG_MM,"recorder_set_audio_encoder failed - code[%x]", e);
		return FALSE;

	}
    cam_critical(LOG_CAM, "cam_mm_set_codec  video_codec = %d " ,video_codec);
	e = recorder_set_video_encoder(g_mm_handle->hrec, video_codec);
	if (e != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_set_video_encoder failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_audio_source(int sample_rate, int channel)
{
	recorder_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);
	e = recorder_attr_set_audio_samplerate(g_mm_handle->hrec, sample_rate);
	if (e != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_attr_set_audio_samplerate failed - code[%x]", e);
		return FALSE;
	}
	e = recorder_attr_set_audio_channel(g_mm_handle->hrec, channel);
	if (e != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_attr_set_audio_channel failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_video_source_format(int format)
{
	if (format <= CAMERA_PIXEL_FORMAT_INVALID || format > CAMERA_PIXEL_FORMAT_JPEG) {
		return FALSE;
	}

	int err;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	err = camera_set_preview_format(g_mm_handle->hcam, format);
	if (err != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_preview_format failed - code[%x]", err);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_get_video_source_format(int *format)
{
	int err;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	err = camera_get_preview_format(g_mm_handle->hcam, format);
	if (err != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_get_preview_format failed - code[%x]", err);
		return FALSE;
	}
	return TRUE;
}

gboolean  cam_mm_get_front_cam_display_rotate_value(int *value, int *rotate)
{
	int err;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	err = camera_attr_get_lens_orientation(g_mm_handle->hcam, value);
	switch (*value) {
	case 0:
		*rotate = CAMERA_ROTATION_NONE;
		break;
	case 90:
		*rotate = CAMERA_ROTATION_90;
		break;
	case 180:
		*rotate = CAMERA_ROTATION_180;
		break;
	case 270:
		*rotate = CAMERA_ROTATION_270;
		break;
	default:
		*rotate = CAMERA_ROTATION_NONE;

	}
	if (err != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_lens_orientation failed - code[%x]", err);
		return FALSE;
	}

	return TRUE;

}

gboolean cam_mm_get_scene_mode(camera_attr_scene_mode_e *mode)
{
	int err;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	err = camera_attr_get_scene_mode(g_mm_handle->hcam, mode);
	if (err != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_get_scene_mode failed - code[%x]", err);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_scene_mode(camera_attr_scene_mode_e mode)
{
	int err;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	err = camera_attr_set_scene_mode(g_mm_handle->hcam, mode);
	if (err != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_set_scene_mode failed - code[%x]", err);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_conti_shot_break(gboolean bvalue)
{
	int err;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	cam_warning(LOG_UI, "burst shot stop");

	if (bvalue) {
		err = camera_stop_continuous_capture(g_mm_handle->hcam);
		if (err != CAMERA_ERROR_NONE) {
			cam_critical(LOG_MM,"camera_stop_continuous_capture failed - code[%x]", err);
			return FALSE;
		}
	}
	return TRUE;
}

gboolean cam_mm_set_capture_format(int value)
{
	int err;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	err = camera_set_capture_format(g_mm_handle->hcam, value);
	if (err != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_capture_format failed - code[%x]", err);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_get_capture_format(int *value)
{
	int err;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	err = camera_get_capture_format(g_mm_handle->hcam, (camera_pixel_format_e *)value);
	if (err != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_get_capture_format failed - code[%x]", err);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_set_shutter_sound(int value)
{
//	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
//	e = camera_attr_set_shutter_sound(g_mm_handle->hcam, value);
//	if (e != CAMERA_ERROR_NONE) {
//		cam_critical(LOG_MM,"camera_attr_set_shutter_sound failed - code[%x]", e);
		return FALSE;
//	}
	return TRUE;
}

gboolean cam_mm_disable_shutter_sound(gboolean value)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	e = camera_attr_disable_shutter_sound(g_mm_handle->hcam, value);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_disable_shutter_sound failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_remove_geo_tag(void)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	int ret = 0;
	ret = camera_attr_remove_geotag(g_mm_handle->hcam);
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_attr_remove_geotag failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_enable_geo_tag(gboolean value)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	if (value) {
		ret = camera_attr_set_geotag(g_mm_handle->hcam, -1.0, -1.0, -1.0);
		if (ret != CAMERA_ERROR_NONE) {
			cam_critical(LOG_MM,"camera_attr_set_geotag failed - code[%x]", ret);
			return FALSE;
		}
	} else {
		ret = camera_attr_remove_geotag(g_mm_handle->hcam);
		if (ret != CAMERA_ERROR_NONE) {
			cam_critical(LOG_MM,"camera_attr_remove_geotag failed - code[%x]", ret);
			return FALSE;
		}
	}
	return TRUE;
}

gboolean cam_mm_reset_recording_motion_fps()
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	ret = recorder_attr_set_recording_motion_rate(g_mm_handle->hrec, DEFAULT_REC_MOTION_RATE);
	if (ret != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"RECORDER_ERROR_NONE failed - code[%x]", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_is_preview_started(int mode)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	int mm_state = 0;
	mm_state = cam_mm_get_state();

	if ((CAM_CAMERA_MODE == mode && mm_state < CAMERA_STATE_PREVIEW)
		|| (mode == CAM_CAMCORDER_MODE && mm_state < RECORDER_STATE_READY) ) {
		cam_critical(LOG_MM, "cur_state:%d", mm_state);
		return FALSE;
	} else
		return TRUE;
}

gboolean cam_mm_preview_start(int mode)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	g_return_val_if_fail(ad, FALSE);

	int ret = 0;
	ret = (mode == CAM_CAMERA_MODE) ? CAMERA_ERROR_NONE : RECORDER_ERROR_NONE;
	if (CAM_CAMERA_MODE == mode) {
		ret = camera_start_preview(g_mm_handle->hcam);
		if (ret != CAMERA_ERROR_NONE) {
			cam_critical(LOG_MM, "camera_start_preview failed, error - [%d]", ret);
			ad->fw_error_type = ret;
			return FALSE;
		}
	} else if (CAM_CAMCORDER_MODE == mode) {
		g_return_val_if_fail(g_mm_handle->hrec, FALSE);
		ret = recorder_prepare(g_mm_handle->hrec);
		if (ret != RECORDER_ERROR_NONE) {
			cam_critical(LOG_MM, "camera_start_preview failed, error - [%d]", ret);
			ad->fw_error_type = ret;
			return FALSE;
		}
	}

	return TRUE;
}

gboolean cam_mm_preview_stop(int mode)
{
	int state = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	if (CAM_CAMERA_MODE == mode) {
		state = cam_mm_get_cam_state();
		cam_debug(LOG_MM, " camera state : %d", state);
		/*todo:please consider recorder and camera*/
		if (state < 0)
			return FALSE;

		switch (state) {
		case CAMERA_STATE_NONE:
		case CAMERA_STATE_CAPTURING:
		case CAMERA_STATE_CREATED:
			return FALSE;
		case CAMERA_STATE_PREVIEW:
			CHECK_MM_ERROR(camera_stop_preview(g_mm_handle->hcam));
			break;
		case CAMERA_STATE_CAPTURED:
			break;
		}
		return TRUE;
	}else if (CAM_CAMCORDER_MODE == mode) {

		state = cam_mm_get_rec_state();
		g_return_val_if_fail(g_mm_handle->hrec, FALSE);
		cam_debug(LOG_MM, " camera state : %d", state);
		/*todo:please consider recorder and camera*/
		if (state < 0) {
			return FALSE;
		}

		switch (state) {
		case RECORDER_STATE_NONE:
		case RECORDER_STATE_RECORDING:
		case RECORDER_STATE_PAUSED:
			return FALSE;

		case RECORDER_STATE_READY:
			CHECK_MM_ERROR(recorder_unprepare(g_mm_handle->hrec));
			break;

		case RECORDER_STATE_CREATED:
			break;

		}
		return TRUE;

	}
	return TRUE;
}

gboolean cam_mm_is_created(void)
{
	if (g_mm_handle) {
		return TRUE;
	}
	return FALSE;
}

gboolean cam_mm_create(int camera_type, int mode)
{
	cam_warning(LOG_CAM, "START");
	cam_debug(LOG_CAM, "camera_type = %d mode = %d" ,camera_type, mode);
	if (g_mm_handle) {
		cam_critical(LOG_MM, "The mm handle is already created");
		return FALSE;
	}

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	g_return_val_if_fail(ad, FALSE);

	camera_h hcam;
	camera_error_e e;

	if (!cam_sound_session_create()) {
		cam_critical(LOG_CAM, "cam_sound_session_create failed");
		return FALSE;
	}

	if (!cam_sound_session_set_option(CAM_SESSION_OPT_MIX_WITH_OTHERS)) {
		cam_critical(LOG_CAM, "cam_sound_session_set_option failed");
		return FALSE;
	}

	e = camera_create(camera_type, &hcam);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_CAM, "[ERROR] camera_create - error(%d)", e);
		return FALSE;
	}
	g_return_val_if_fail(hcam, FALSE);
	recorder_h hrec = NULL;
	recorder_error_e re;
	re = recorder_create_videorecorder(hcam, &hrec);
	if (re != RECORDER_ERROR_NONE) {
		cam_critical(LOG_CAM, "[ERROR] camera_create - error(%d)", e);
		recorder_destroy(hrec);
		CHECK_MM_ERROR(camera_destroy(hcam));
		return FALSE;
	}
	//g_return_val_if_fail(hrec, FALSE);

	g_mm_handle = g_new0(CamMMHandle, 1);
	if (g_mm_handle) {
		g_mm_handle->hcam = hcam;
		g_mm_handle->hdev = camera_type;
		g_mm_handle->hrec = hrec;
	} else {
		cam_critical(LOG_CAM, "[ERROR] memory allocation failed", e);
		recorder_destroy(hrec);
		CHECK_MM_ERROR(camera_destroy(hcam));
		return FALSE;
	}

	cam_warning(LOG_CAM, "END");

	return TRUE;
}

gboolean cam_mm_destory(void)
{
	g_return_val_if_fail(g_mm_handle, FALSE);

	cam_warning(LOG_CAM, "START");

	if (!cam_sound_session_destroy()) {
		cam_critical(LOG_MM, "cam_sound_session_destroy failed");
	}

	gboolean recorder_ret = FALSE;
	if( g_mm_handle->hrec != NULL) {
		if (recorder_destroy(g_mm_handle->hrec) == RECORDER_ERROR_NONE) {
			recorder_ret = TRUE;
		} else {
			cam_critical(LOG_MM, "recorder_destroy failed");
		}
		g_mm_handle->hrec = NULL;
	}
	gboolean camera_ret = FALSE;
	if(g_mm_handle->hcam != NULL) {
		if (camera_destroy(g_mm_handle->hcam) == CAMERA_ERROR_NONE) {
			camera_ret = TRUE;
		} else {
			cam_critical(LOG_MM, "camera_destroy failed");
		}
		g_mm_handle->hcam = NULL;
	}

	g_mm_handle->hdev = -1;

	g_free(g_mm_handle);
	g_mm_handle = NULL;

	cam_warning(LOG_CAM, "END");

	return (camera_ret && recorder_ret);
}

gboolean cam_mm_set_display_device(int display_type, void *display_handle)
{
	int ret;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_set_display(g_mm_handle->hcam, display_type, GET_DISPLAY(display_handle));
	if (ret != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM, "[ERROR] camera_set_display - error(%d)", ret);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_continuous_capture_start(int count, int interval, camera_capturing_cb capturing_cb, camera_capture_completed_cb completed_cb , void *user_data)
{
	int state = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	cam_debug(LOG_MM, "");

	state = cam_mm_get_state();
	if (state == CAMERA_STATE_PREVIEW) {
		CHECK_MM_ERROR(camera_start_continuous_capture(g_mm_handle->hcam, count, interval, capturing_cb, completed_cb, user_data));
	} else {
		cam_critical(LOG_MM, "error, camera state is [%d]", state);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_capture_start(camera_capturing_cb capturing_cb , camera_capture_completed_cb completed_cb , void *user_data)
{
	int state = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	struct appdata *ad =  (struct appdata *)user_data;
	g_return_val_if_fail(ad, FALSE);
	CamAppData *camapp = ad->camapp_handle;
	g_return_val_if_fail(camapp, FALSE);

	cam_debug(LOG_MM, "");

	gboolean do_capture = FALSE;

	state = cam_mm_get_state();
	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		if (state == CAMERA_STATE_PREVIEW) {
			do_capture = TRUE;
		}
	} else if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		if (state == RECORDER_STATE_RECORDING
			|| state == RECORDER_STATE_PAUSED) {
			do_capture = TRUE;
		}
	}

	if (do_capture) {
		gError = camera_start_capture(g_mm_handle->hcam, capturing_cb, completed_cb, user_data);
		if (gError != CAMERA_ERROR_NONE) {
			cam_critical(LOG_MM, "camera_start_capture failed, error[%d]", gError);
			return FALSE;
		}
	} else {
		cam_critical(LOG_MM, "capture failed - skipped while state is :%d", state);
		cam_standby_view_shooting_frame_destroy();
		return TRUE;/*NOTE: in this case, we return true, for do not display popup*/
	}
	return TRUE;
}

gboolean cam_mm_rec_start()
{
	int state = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	state = cam_mm_get_state();
	if ((state == RECORDER_STATE_READY)
	    || (state == RECORDER_STATE_PAUSED)) {
		gError = recorder_start(g_mm_handle->hrec);
		if (gError != RECORDER_ERROR_NONE) {
			cam_critical(LOG_UI, "recorder_start failed, error:[%d]", gError);
			return FALSE;
		}
	} else {
		cam_critical(LOG_UI, "operation failed - state:[%d]", state);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_rec_stop(gboolean to_stop)
{
	int state = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	state = cam_mm_get_state();
	if ((state == RECORDER_STATE_RECORDING)
	    || (state == RECORDER_STATE_PAUSED)) {
		if (!to_stop) {
			CHECK_MM_ERROR(recorder_commit(g_mm_handle->hrec));
		} else {
			CHECK_MM_ERROR(recorder_commit(g_mm_handle->hrec));
			CHECK_MM_ERROR(recorder_unprepare(g_mm_handle->hrec));
		}
	} else {
		cam_critical(LOG_MM, "error, camera state is [%d]", state);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_rec_pause()
{
	int state = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	state = cam_mm_get_state();
	if ((state == RECORDER_STATE_RECORDING)) {
		CHECK_MM_ERROR(recorder_pause(g_mm_handle->hrec));
	} else {
		cam_critical(LOG_MM, "error, camera state is [%d]", state);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_rec_cancel()
{
	int state = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	state = cam_mm_get_state();
	if ((state == RECORDER_STATE_RECORDING)
	    || (state == RECORDER_STATE_PAUSED)) {
		CHECK_MM_ERROR(recorder_cancel(g_mm_handle->hrec));
	} else {
		cam_critical(LOG_MM, "error, camera state is [%d]", state);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_create_audio_in(void)
{
	cam_warning(LOG_UI, "create autio recorder");

	if (g_mm_handle && g_mm_handle->haudio) {
		cam_warning(LOG_UI, "audio handle is already created");
		return TRUE;
	}

	audio_in_h haudio = NULL;
	audio_io_error_e e = AUDIO_IO_ERROR_NONE;

	e = audio_in_create(44100, AUDIO_CHANNEL_MONO, AUDIO_SAMPLE_TYPE_S16_LE, &haudio);
	if (e != AUDIO_IO_ERROR_NONE) {
		cam_critical(LOG_UI, "audio_in_create failed, error[%d]", e);
		return FALSE;
	}

	if (g_mm_handle) {
		g_mm_handle->haudio = haudio;
	} else {
		cam_critical(LOG_UI, "camera handle is NULL");
		audio_in_destroy(haudio);
		return FALSE;
	}

	e = audio_in_prepare(g_mm_handle->haudio);
	if (e != AUDIO_IO_ERROR_NONE) {
		cam_critical(LOG_UI, "audio_in_prepare failed, error[%d]", e);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_destroy_audio_in(void)
{
	g_return_val_if_fail(g_mm_handle, FALSE);

	audio_io_error_e e = AUDIO_IO_ERROR_NONE;
	if( g_mm_handle->haudio != NULL) {
		e = audio_in_unprepare(g_mm_handle->haudio);
		if (e != AUDIO_IO_ERROR_NONE) {
			cam_critical(LOG_UI, "audio_in_unprepare failed, error[%d]", e);
			return FALSE;
		}

		e = audio_in_destroy(g_mm_handle->haudio);
		if (e != AUDIO_IO_ERROR_NONE) {
			cam_critical(LOG_UI, "audio_in_destroy failed - error[%d]", e);
			return FALSE;
		}

		g_mm_handle->haudio = NULL;
	}

	return TRUE;
}

gboolean cam_mm_audio_in_get_buffer_size(int *size)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->haudio, FALSE);

	audio_io_error_e e = AUDIO_IO_ERROR_NONE;
	e = audio_in_get_buffer_size(g_mm_handle->haudio, size);
	if (e != AUDIO_IO_ERROR_NONE) {
		cam_critical(LOG_UI, "audio_in_get_buffer_size failed, error[%d]", e);
		return FALSE;
	}

	return TRUE;
}

int cam_mm_audio_in_read(void *buffer, unsigned int length)
{
	g_return_val_if_fail(g_mm_handle, -1);
	g_return_val_if_fail(g_mm_handle->haudio, -1);

	int ret = audio_in_read(g_mm_handle->haudio, buffer, length);
	if (ret <= 0) {
		cam_critical(LOG_UI, "audio_in_read failed, error[%d]", ret);
		return -1;
	}

	return ret;
}

gboolean cam_mm_start_focusing(gint af_mode)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	struct appdata *ad =  (struct appdata *)cam_appdata_get();
	g_return_val_if_fail(ad, FALSE);
	CamAppData *camapp = ad->camapp_handle;
	g_return_val_if_fail(camapp, FALSE);

	cam_debug(LOG_UI, "");

	int ret = 0;
	int state = cam_mm_get_state();

	if (camapp->camera_mode == CAM_CAMERA_MODE) {
		if (state == CAMERA_STATE_PREVIEW
			|| state == CAMERA_STATE_CREATED
			|| state == CAMERA_STATE_CAPTURED) {
			if ((CamAppFocusMode)af_mode == CAM_FOCUS_MODE_CONTINUOUS) {
				cam_debug(LOG_UI, "continuous");
				ret = camera_start_focusing(g_mm_handle->hcam, TRUE);
				if (ret != CAMERA_ERROR_NONE) {
					cam_warning(LOG_MM, "camera_start_focusing failed [%d]", ret);
					return FALSE;
				}
			} else {
				cam_debug(LOG_UI, "touchAF");
				ret = camera_start_focusing(g_mm_handle->hcam, FALSE);
				if (ret != CAMERA_ERROR_NONE) {
					cam_warning(LOG_MM, "camera_start_focusing failed [%d]", ret);
					return FALSE;
				}
				if (camapp->focus_mode != 1)
					ev_load_edje(ad);
			}
		} else {
			cam_debug(LOG_CAM, "Start focus operation failed in camera mode - invalid state:%d ", state);
			return FALSE;
		}
	} else if (camapp->camera_mode == CAM_CAMCORDER_MODE) {
		if (state == RECORDER_STATE_CREATED
			|| state == RECORDER_STATE_READY
			|| state == RECORDER_STATE_RECORDING
			|| state == RECORDER_STATE_PAUSED) {
			if ((CamAppFocusMode)af_mode == CAM_FOCUS_MODE_CONTINUOUS) {
				ret = camera_start_focusing(g_mm_handle->hcam, TRUE);
				if (ret != CAMERA_ERROR_NONE) {
					cam_warning(LOG_MM, "camera_start_focusing failed [%d]", ret);
					return FALSE;
				}
			} else {
				ret = camera_start_focusing(g_mm_handle->hcam, FALSE);
				if (ret != CAMERA_ERROR_NONE) {
					cam_warning(LOG_MM, "camera_start_focusing failed [%d]", ret);
					return FALSE;
				}
			}
		} else {
			cam_debug(LOG_CAM, "Start focus operation failed in camcorder mode- invalid state:%d ", state);
			return FALSE;
		}
	}

	return TRUE;
}

gboolean cam_mm_stop_focusing()
{
	int state = 0;

	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	cam_debug(LOG_UI, "");

	state = cam_mm_get_state();
	if (state == CAMERA_STATE_PREVIEW
		|| state == CAMERA_STATE_CREATED
		|| state == RECORDER_STATE_RECORDING) {
		/*TODO:please think rec mod, but now the rec and cam mode state value is same*/
		CHECK_MM_ERROR(camera_cancel_focusing(g_mm_handle->hcam));
	} else {
		cam_warning(LOG_MM, "error, camera state is [%d]", state);
		return FALSE;
	}

	return TRUE;
}

gboolean cam_mm_set_error_cb(camera_error_cb error_cb, void *data)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	e = camera_set_error_cb(g_mm_handle->hcam, error_cb, data);
		if (e != CAMERA_ERROR_NONE) {
			cam_critical(LOG_MM,"camera_set_error_cb failed - code[%x]", e);
			return FALSE;
		}
		return TRUE;

}

gboolean cam_mm_unset_error_cb(void)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	e = camera_unset_error_cb(g_mm_handle->hcam);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_unset_error_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_set_state_changed_cb(camera_state_changed_cb state_cb, void *data)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_set_state_changed_cb(g_mm_handle->hcam, state_cb, data);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_state_changed_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_unset_state_changed_cb(void)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_unset_state_changed_cb(g_mm_handle->hcam);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_unset_state_changed_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}


gboolean cam_mm_set_focus_changed_cb(camera_focus_changed_cb focus_cb, void *data)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_set_focus_changed_cb(g_mm_handle->hcam, focus_cb, data);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_focus_changed_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_unset_focus_changed_cb(void)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_unset_focus_changed_cb(g_mm_handle->hcam);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_unset_focus_changed_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_preview_cb(camera_preview_cb preview_cb, void *data)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_set_preview_cb(g_mm_handle->hcam, preview_cb, data);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_preview_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_unset_preview_cb(void)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_unset_preview_cb(g_mm_handle->hcam);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_unset_preview_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_set_camera_interrupted_cb(camera_interrupted_cb callback, void *data) {

	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_set_interrupted_cb(g_mm_handle->hcam, callback, data);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_interrupted_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_recorder_set_state_changed_cb(recorder_state_changed_cb callback, void* user_data)
{
	recorder_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	e = recorder_set_state_changed_cb(g_mm_handle->hrec, callback, user_data);
	if (e != RECORDER_ERROR_NONE) {
		rec_detail_error_get(e);
		cam_critical(LOG_MM,"recorder_set_state_changed_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
   }

gboolean cam_mm_unset_camera_interrupted_cb(void) {

	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_unset_interrupted_cb(g_mm_handle->hcam);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"cam_mm_unset_camera_interrupted_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_recorder_interrupted_cb(recorder_interrupted_cb callback, void *data) {

	recorder_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	e = recorder_set_interrupted_cb(g_mm_handle->hrec, callback, data);
	if (e != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_set_interrupted_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_unset_recorder_interrupted_cb(void)
{
	recorder_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	e = recorder_unset_interrupted_cb(g_mm_handle->hrec);
	if (e != RECORDER_ERROR_NONE) {
		rec_detail_error_get(e);
		cam_critical(LOG_MM,"cam_mm_unset_recorder_interrupted_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_recorder_unset_state_changed_cb(void)
{
	recorder_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	e = recorder_unset_state_changed_cb(g_mm_handle->hrec);
	if (e != RECORDER_ERROR_NONE) {
		rec_detail_error_get(e);
		cam_critical(LOG_MM,"recorder_unset_state_changed_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_recorder_set_recording_status_cb(recorder_recording_status_cb callback, void* user_data)
{
	recorder_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	e = recorder_set_recording_status_cb(g_mm_handle->hrec, callback, user_data);
	if (e != RECORDER_ERROR_NONE) {
		rec_detail_error_get(e);
		cam_critical(LOG_MM,"recorder_set_recording_status_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_recorder_unset_recording_status_cb(void)
{
	recorder_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	e = recorder_unset_recording_status_cb(g_mm_handle->hrec);
	if (e != RECORDER_ERROR_NONE) {
		rec_detail_error_get(e);
		cam_critical(LOG_MM,"recorder_unset_recording_status_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_recorder_set_recording_limit_reached_cb(recorder_recording_limit_reached_cb callback, void* user_data)
{
	recorder_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	e = recorder_set_recording_limit_reached_cb(g_mm_handle->hrec, callback, user_data);
	if (e != RECORDER_ERROR_NONE) {
		rec_detail_error_get(e);
		cam_critical(LOG_MM,"recorder_set_recording_limit_reached_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_recorder_unset_recording_limit_reached_cb(void)
{
	recorder_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	e = recorder_unset_recording_limit_reached_cb(g_mm_handle->hrec);
	if (e != RECORDER_ERROR_NONE) {
		rec_detail_error_get(e);
		cam_critical(LOG_MM,"recorder_unset_recording_limit_reached_cb failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;

}

gboolean cam_mm_set_recording_motion(double rate)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	recorder_error_e err;
	err = recorder_attr_set_recording_motion_rate(g_mm_handle->hrec, rate);
	if (err != RECORDER_ERROR_NONE) {
		cam_critical(LOG_MM,"recorder_attr_set_recording_motion_rate failed - code[%x]", err);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_start_camera_face_detection(camera_face_detected_cb callback, void *data)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_start_face_detection(g_mm_handle->hcam, callback, data);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_start_face_detection failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_stop_camera_face_detection(void)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_stop_face_detection(g_mm_handle->hcam);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_stop_face_detection failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_is_supported_face_detection(void)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	return camera_is_supported_face_detection(g_mm_handle->hcam);

}

gboolean cam_mm_set_camera_face_zoom(int face_id)
{
//	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

//	e = camera_face_zoom(g_mm_handle->hcam, face_id);
//	if (e != CAMERA_ERROR_NONE) {
//		cam_critical(LOG_MM,"camera_face_zoom failed - code[%x]", e);
		return FALSE;
//	}
//	return TRUE;
}

gboolean cam_mm_camera_cancel_face_zoom(void)
{
//	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

//	e = camera_cancel_face_zoom(g_mm_handle->hcam);
//	if (e != CAMERA_ERROR_NONE) {
//		cam_critical(LOG_MM,"camera_cancel_face_zoom failed - code[%x]", e);
		return FALSE;
//	}
//	return TRUE;

}

gboolean cam_mm_get_recording_flip(void)
{
 	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

//	camera_error_e e;
//	//recorder_flip_e flip = RECORDER_FLIP_NONE;
//	e = recorder_attr_get_recording_flip(g_mm_handle->hrec, &flip);
//
//	if (e != CAMERA_ERROR_NONE) {
//		cam_critical(LOG_MM,"recorder_attr_get_recording_flip failed - code[%x]", e);
	return FALSE;
//	}

//	if (flip == RECORDER_FLIP_HORIZONTAL
//		|| flip == RECORDER_FLIP_VERTICAL)
//		return TRUE;
//	else
//		return FALSE;
}

gboolean cam_mm_set_recording_flip(gboolean value)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

	if (value) {
		//e = recorder_attr_set_recording_flip(g_mm_handle->hrec, RECORDER_FLIP_HORIZONTAL);

		if (g_mm_handle->hdev == (camera_device_e)CAM_DEVICE_REAR) {
			e = camera_set_display_flip(g_mm_handle->hcam, CAMERA_FLIP_VERTICAL);
		} else {
			e = camera_set_display_flip(g_mm_handle->hcam, CAMERA_FLIP_NONE);
		}
	} else {
		//e = recorder_attr_set_recording_flip(g_mm_handle->hrec, RECORDER_FLIP_NONE);

		if (g_mm_handle->hdev == (camera_device_e)CAM_DEVICE_REAR) {
			e = camera_set_display_flip(g_mm_handle->hcam, CAMERA_FLIP_NONE);
		} else {
			e = camera_set_display_flip(g_mm_handle->hcam, CAMERA_FLIP_VERTICAL);
		}
	}

	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_display_flip failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_set_image_flip(gboolean value)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	if (value) {
		e = camera_attr_set_stream_flip(g_mm_handle->hcam, CAMERA_FLIP_HORIZONTAL);

		if (g_mm_handle->hdev == (camera_device_e)CAM_DEVICE_REAR) {
			e = camera_set_display_flip(g_mm_handle->hcam, CAMERA_FLIP_VERTICAL);
		} else {
			e = camera_set_display_flip(g_mm_handle->hcam, CAMERA_FLIP_NONE);
		}
	} else {
		e = camera_attr_set_stream_flip(g_mm_handle->hcam, CAMERA_FLIP_NONE);

		if (g_mm_handle->hdev == (camera_device_e)CAM_DEVICE_REAR) {
			e = camera_set_display_flip(g_mm_handle->hcam, CAMERA_FLIP_NONE);
		} else {
			e = camera_set_display_flip(g_mm_handle->hcam, CAMERA_FLIP_VERTICAL);
		}
	}

	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_display_flip failed - code[%x]", e);
		return FALSE;
	}
	return TRUE;
}

gboolean cam_mm_get_caps_minmax(unsigned int type, int *min, int *max)
{
	if (!cam_mm_is_created()) {
		cam_debug(LOG_CAM, "cam_mm_is_created() false");
		return FALSE;
	}

	gboolean ret = TRUE;
	int tempmin, tempmax = 0;

	switch (type) {
	case CAM_CP_FUNC_EXPOSURE:
		{
			 if (camera_attr_get_exposure_range(g_mm_handle->hcam, &tempmin, &tempmax) != CAMERA_ERROR_NONE) {
				cam_debug(LOG_CAM, "camera_attr_get_exposure_range() is false");
				ret = FALSE;
			 }
		}
		break;
	case CAM_CP_FUNC_BRIGHTNESS:
		{
			 if (camera_attr_get_brightness_range(g_mm_handle->hcam, &tempmin, &tempmax) != CAMERA_ERROR_NONE) {
				cam_debug(LOG_CAM, "camera_attr_get_brightness_range() is false");
				ret = FALSE;
			 }
		}
		break;
	case CAM_CP_FUNC_ZOOM:
		{
			if (camera_attr_get_zoom_range(g_mm_handle->hcam, &tempmin, &tempmax) != CAMERA_ERROR_NONE) {
				cam_debug(LOG_CAM, "camera_attr_get_zoom_range() is false");
				ret = FALSE;
			}

			if (tempmin == tempmax) {
				cam_debug(LOG_CAM, "zoom is not supported");
				ret = FALSE;
			}
		}
		break;
	default:
		{
			cam_debug(LOG_CAM, "not support get_minmax() about this type[%d]", type);
			ret = FALSE;
		}
		break;
	}

	if (!ret) {
		tempmin = 0;
		tempmax = 0;
	}

	*min = tempmin;
	*max = tempmax;

	return ret;
}

gboolean cam_mm_get_fps_by_resolution(int width, int height, void *user_data)
{
	gboolean ret = TRUE;
	if (camera_attr_foreach_supported_fps(g_mm_handle->hcam,
		(camera_attr_supported_fps_cb)__get_fps_by_resolution_cb, user_data) != CAMERA_ERROR_NONE) {
		cam_debug(LOG_CAM, "camera_attr_foreach_supported_fps() is fail");
		ret = FALSE;
	}
	return ret;
}

gboolean cam_mm_get_caps_range(unsigned int type, unsigned int *caps, void *user_data)
{
	if (!cam_mm_is_created()) {
		cam_debug(LOG_CAM, "cam_mm_is_created() false");
		return FALSE;
	}

	g_caps = 0;
	g_caps_cb_cnt = 0;
	gboolean ret = TRUE;

	switch (type) {
	case CAM_CP_FUNC_FLASH_MODE:
		{
			if (camera_attr_foreach_supported_flash_mode(g_mm_handle->hcam,
				(camera_attr_supported_flash_mode_cb)__get_flash_cb, user_data) != CAMERA_ERROR_NONE ) {
				cam_debug(LOG_CAM, "camera_attr_foreach_supported_flash_mode() is fail");
				ret = FALSE;
			}
		}
		break;
	case CAM_CP_FUNC_ISO:
		{
			if (camera_attr_foreach_supported_iso(g_mm_handle->hcam,
				(camera_attr_supported_iso_cb)__get_iso_cb, user_data) != CAMERA_ERROR_NONE ) {
				cam_debug(LOG_CAM, "camera_attr_foreach_supported_iso() is fail");
				ret = FALSE;
			}
		}
		break;
	case CAM_CP_FUNC_SCENE_MODE:
		{
			if (camera_attr_foreach_supported_scene_mode(g_mm_handle->hcam,
				(camera_attr_supported_scene_mode_cb)__get_scene_cb, user_data) != CAMERA_ERROR_NONE ) {
				cam_debug(LOG_CAM, "camera_attr_foreach_supported_scene_mode() is fail");
				ret = FALSE;
			}
		}
		break;
	case CAM_CP_FUNC_METERING:
		{
			if (camera_attr_foreach_supported_exposure_mode(g_mm_handle->hcam,
				(camera_attr_supported_exposure_mode_cb)__get_metering_cb, user_data) != CAMERA_ERROR_NONE ) {
				cam_debug(LOG_CAM, "camera_attr_foreach_supported_exposure_mode() is fail");
				ret = FALSE;
			}
		}
		break;

	case CAM_CP_FUNC_WHITE_BALANCE:
		{
			if (camera_attr_foreach_supported_whitebalance(g_mm_handle->hcam,
				(camera_attr_supported_whitebalance_cb)__get_wb_cb, user_data) != CAMERA_ERROR_NONE) {
				cam_debug(LOG_CAM, "camera_attr_foreach_supported_whitebalance() is fail");
				ret = FALSE;
			}
		}
		break;
	case CAM_CP_FUNC_FOCUS_MODE:
		{
			if (camera_attr_foreach_supported_af_mode(g_mm_handle->hcam,
				(camera_attr_supported_af_mode_cb)__get_focus_cb, user_data) != CAMERA_ERROR_NONE) {
				cam_debug(LOG_CAM, "camera_attr_foreach_supported_exposure_mode() is fail");
				ret = FALSE;
			}
		}
		break;
	case CAM_CP_FUNC_FPS:
		{
			if (camera_attr_foreach_supported_fps(g_mm_handle->hcam,
				(camera_attr_supported_fps_cb)__get_fps_cb, user_data) != CAMERA_ERROR_NONE) {
				cam_debug(LOG_CAM, "camera_attr_foreach_supported_fps() is fail");
				ret = FALSE;
			}
		}
		break;
	case CAM_CP_FUNC_CAM_RESOLUTION:
		{
			if(camera_foreach_supported_capture_resolution(g_mm_handle->hcam,
				(camera_supported_capture_resolution_cb)__get_capture_res_cb, user_data) != CAMERA_ERROR_NONE) {
				cam_debug(LOG_CAM, "camera_foreach_supported_capture_resolution() is fail");
				ret = FALSE;
			}
		}
		break;
	case CAM_CP_FUNC_REC_RESOLUTION:
		{
			if(recorder_foreach_supported_video_resolution(g_mm_handle->hrec,
				(recorder_supported_video_resolution_cb)__get_recording_res_cb, user_data) != CAMERA_ERROR_NONE) {
				cam_debug(LOG_CAM, "recorder_foreach_supported_video_resolution() is fail");
				ret = FALSE;
			}
		}
		break;
	default:
		{
			cam_debug(LOG_CAM, "not support get_range() about this type[%d]", type);
			ret = FALSE;
		}
		break;
	}

	*caps = g_caps;
	return ret;
}

int cam_mm_get_caps_cb_cnt()
{
	return g_caps_cb_cnt;
}

static bool __get_iso_cb(camera_attr_iso_e iso, void *user_data)
{
	cam_retvm_if(iso > CAMERA_ATTR_ISO_3200, false, "input is invalid");

	unsigned int uRet = cam_iso_dev_convert_caps(iso);
	if (uRet != 0) {
		g_caps |= uRet;
		g_caps_cb_cnt++;
	}
	return true;
}

/*static bool __get_effect_cb(camera_effect_gs_e effect, void *user_data)
{
	cam_retvm_if(effect > CAMERA_EFFECT_GS_DOWNLOAD, false, "input is invalid");

	unsigned int uRet = cam_effect_dev_convert_caps(effect);
	if (uRet != 0 ) {
		g_caps |= uRet;
		g_caps_cb_cnt++;
	}
	return true;
}*/

static bool __get_fps_by_resolution_cb(camera_attr_fps_e fps, void *user_data)
{
	cam_retvm_if(fps > CAMERA_ATTR_FPS_120, false, "input is invalid");

	struct appdata *ad =  (struct appdata *)user_data;
	g_return_val_if_fail(ad, FALSE);
	CamAppData *camapp = ad->camapp_handle;
	g_return_val_if_fail(camapp, FALSE);

	cam_critical(LOG_UI,"fps value fps = %d", fps);
	camapp->fps_by_resolution = fps;

	return true;
}

static bool __get_fps_cb(camera_attr_fps_e fps, void *user_data)
{
	cam_retvm_if(fps > CAMERA_ATTR_FPS_120, false, "input is invalid");

	unsigned int uRet = cam_fps_dev_convert_caps(fps);
	if (uRet != 0) {
		g_caps |= uRet;
		g_caps_cb_cnt++;
	}
	return true;
}

static bool __get_wb_cb(camera_attr_whitebalance_e wb, void *user_data)
{
	cam_retvm_if(wb > CAMERA_ATTR_WHITE_BALANCE_CUSTOM, false, "input is invalid");

	unsigned int uRet = cam_wb_dev_convert_caps(wb);
	if (uRet != 0) {
		g_caps |= uRet;
		g_caps_cb_cnt++;
	}
	return true;
}

static bool __get_focus_cb(camera_attr_af_mode_e focus, void *user_data)
{
	cam_retvm_if(focus > CAMERA_ATTR_AF_FULL, false, "input is invalid");

	unsigned int uRet = cam_focus_dev_convert_caps(focus);
	if (uRet != 0) {
		g_caps |= uRet;
		g_caps_cb_cnt++;
	}
	return true;
}

static bool __get_metering_cb(camera_attr_exposure_mode_e metering, void *user_data)
{
	cam_retvm_if(metering > CAMERA_ATTR_EXPOSURE_MODE_CUSTOM, false, "input is invalid");

	unsigned int uRet = cam_metering_dev_convert_caps(metering);
	if (uRet != 0) {
		g_caps |= uRet;
		g_caps_cb_cnt++;
	}
	return true;
}

static bool __get_scene_cb(camera_attr_scene_mode_e scene, void *user_data)
{
	cam_retvm_if(scene > CAMERA_ATTR_SCENE_MODE_AQUA, false, "input is invalid");

	unsigned int uRet = cam_scene_dev_convert_caps(scene);
	if (uRet != 0) {
		g_caps |= uRet;
		g_caps_cb_cnt++;
	}
	return true;
}

static bool __get_flash_cb(camera_attr_flash_mode_e flash, void *user_data)
{
	cam_retvm_if(flash > CAMERA_ATTR_FLASH_MODE_PERMANENT, false, "input is invalid");

	unsigned int uRet = cam_flash_dev_convert_caps(flash);
	if (uRet != 0 ) {
		g_caps |= uRet;
		g_caps_cb_cnt++;
	}
	return true;
}

static bool __get_capture_res_cb(int width, int height, void *user_data)
{
	unsigned int uRet = cam_resolution_cam_convert_caps((unsigned int)CAM_RESOLUTION(width, height));
	if (uRet != 0) {
		g_caps |= uRet;
		g_caps_cb_cnt++;
	}
	return true;
}

static bool __get_recording_res_cb(int width, int height, void *user_data)
{
	unsigned int uRet = cam_resolution_cam_convert_caps((unsigned int)CAM_RESOLUTION(width, height));
	if (uRet != 0) {
		g_caps |= uRet;
		g_caps_cb_cnt++;
	}
	return true;
}

gboolean cam_mm_set_hybrid_mode(gboolean value)
{
	/*g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	camera_error_e e;
	camera_hybrid_mode_e hybrid_mode;

	if (value) {
		hybrid_mode = CAMERA_HYBRID_MODE_ENABLE;
	} else {
		hybrid_mode = CAMERA_HYBRID_MODE_DISABLE;
	}

	e = camera_set_hybrid_mode(g_mm_handle->hcam, hybrid_mode);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM, "camera_set_hybrid_mode() error(%d)", e);
		return FALSE;
	}
	return TRUE;*/
	return FALSE;
}

/*gboolean cam_mm_set_light_state_changed_cb(camera_low_light_state_changed_cb light_cb, void *data)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_set_low_light_state_changed_cb(g_mm_handle->hcam, light_cb, data);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM, "camera_set_low_light_state_changed_cb() error(%d)", e);
		return FALSE;
	}
	return TRUE;
}*/

gboolean cam_mm_unset_light_state_changed_cb(void)
{
//	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

//	e = camera_unset_low_light_state_changed_cb(g_mm_handle->hcam);
//	if (e != CAMERA_ERROR_NONE) {
//		cam_critical(LOG_MM, "camera_unset_low_light_state_changed_cb() error(%d)", e);
		return FALSE;
//	}
//	return TRUE;
}

int cam_mm_get_error(void)
{
	return gError;
}

gboolean cam_mm_set_cameraplex_inset_window_device(int camera_type)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

//	camera_error_e e;

//	e = camera_attr_set_cameraplex_inset_window_device(g_mm_handle->hcam, camera_type);
//
//	if (e != CAMERA_ERROR_NONE) {
//		cam_critical(LOG_MM, "camera_attr_set_cameraplex_inset_window_device() error(%d)", e);
		return FALSE;
//	}
//	return TRUE;
}

gboolean cam_mm_set_cameraplex_style(int value)
{
	/*int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_set_cameraplex_style(g_mm_handle->hcam, (camera_cameraplex_filter_e)value);
	if (ret != CAMERA_ERROR_NONE) {

		return FALSE;
	}
	return TRUE;*/
	return FALSE;

}

gboolean cam_mm_get_video_display_size(void *data, CamVideoRectangle *video, CamVideoRectangle *preview)
{
	CamVideoRectangle screen = {0,};
	CamVideoRectangle video_tmp = {0,};
	CamVideoRectangle preview_tmp = {0,};
	int ret = 0;
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	cam_retvm_if(video == NULL, FALSE, "video is NULL");
	cam_retvm_if(preview == NULL, FALSE, "preview is NULL");

	ret = cam_mm_get_preview_size(&(video_tmp.w), &(video_tmp.h));
	if(ret == TRUE) {
		screen.w = ad->win_width;
		screen.h = ad->win_height;
		cam_app_preview_start_coordinate(video_tmp, screen, &preview_tmp);
	} else {
		preview_tmp.x = ad->preview_offset_x;
		preview_tmp.y = ad->preview_offset_y;
		preview_tmp.w = ad->preview_w;
		preview_tmp.h = ad->preview_h;
		video_tmp.w = ad->camfw_video_width;
		video_tmp.h = ad->camfw_video_height;
		cam_critical(LOG_CAM, "cam_mm_get_preview_size error");
	}

	*video = video_tmp;
	*preview = preview_tmp;

	return TRUE;

}

/*change the screen rect xywh to cameraplex rect xywh,
screen rect xy00 is change when rotate, it is at the left top corner of screen
cameraplex xywh is fix and the same as screen rect landscape*/
gboolean cam_mm_convert_screen_rect_to_cameraplex_window(void *data, CamRectangle *video, CamRectangle *rect)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	cam_retvm_if(rect == NULL, FALSE, "rect is NULL");

	double scale_w = 0;
	double scale_h = 0;
	CamRectangle rect_step1 = {0,};
	CamRectangle rect_step2 = {0,};
	CamVideoRectangle video_tmp = {0,};
	CamVideoRectangle preview = {0,};
	CamVideoRectangle screen = {0,};

	if(video) {
		/*use new video wh get new preview wh*/
		video_tmp.w = video->width;
		video_tmp.h = video->height;
		screen.w = ad->win_width;
		screen.h = ad->win_height;
		cam_app_preview_start_coordinate(video_tmp, screen, &preview);
		cam_debug(LOG_CAM, "new video(%d,%d)", video_tmp.w, video_tmp.h);
	} else {
		/*use default video wh*/
		preview.x = ad->preview_offset_x;
		preview.y = ad->preview_offset_y;
		preview.w = ad->preview_w;
		preview.h = ad->preview_h;
		video_tmp.w = ad->camfw_video_width;
		video_tmp.h = ad->camfw_video_height;
	}

	/*step1: change xy00*/
	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		rect_step1.x = rect->x - preview.x;
		rect_step1.y = rect->y - preview.y;
		rect_step1.width = rect->width;
		rect_step1.height = rect->height;
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		rect_step1.x = ad->win_width - rect->x - rect->width - preview.x;
		rect_step1.y = ad->win_height - rect->y - rect->height - preview.y;
		rect_step1.width = rect->width;
		rect_step1.height = rect->height;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		rect_step1.x = rect->y - preview.x;
		rect_step1.y = ad->win_height - rect->x - rect->width - preview.y;
		rect_step1.width = rect->height;
		rect_step1.height = rect->width;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		rect_step1.x = ad->win_width - rect->y - rect->height - preview.x;
		rect_step1.y = rect->x - preview.y;
		rect_step1.width = rect->height;
		rect_step1.height = rect->width;
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction %d", ad->target_direction);
		return FALSE;
		break;
	}

	/*step2: change according to bg ratio, default is 1280*720*/
	rect_step2 = rect_step1;
	if (preview.w != 0)
		scale_w = (double)video_tmp.w / (double)preview.w;
	if (preview.h != 0)
		scale_h = (double)video_tmp.h / (double)preview.h;

	rect_step2.x = rect_step1.x * scale_w;
	rect_step2.width = rect_step1.width * scale_w;
	rect_step2.y = rect_step1.y * scale_h;
	rect_step2.height = rect_step1.height * scale_h;

	*rect = rect_step2;

	return TRUE;
}

gboolean cam_mm_convert_cameraplex_window_to_screen_rect(void *data, CamRectangle *rect)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	cam_retvm_if(rect == NULL, FALSE, "rect is NULL");

	double scale_w = 0;
	double scale_h = 0;
	CamRectangle rect_step1 = {0,};
	CamRectangle rect_step2 = {0,};

	/*step1: change according to bg ratio, screen is 1280*720*/
	rect_step1 = *rect;
	scale_w = (double)ad->preview_w / (double)ad->camfw_video_width;
	scale_h = (double)ad->preview_h / (double)ad->camfw_video_height;

	rect_step1.x = rect_step1.x * scale_w;
	rect_step1.width = rect_step1.width * scale_w;
	rect_step1.y = rect_step1.y * scale_h;
	rect_step1.height = rect_step1.height * scale_h;

	/*step2: change xy00*/
	rect_step2 = rect_step1;
	switch (ad->target_direction) {
	case CAM_TARGET_DIRECTION_LANDSCAPE:
		rect_step2.x = rect_step1.x + ad->preview_offset_x;
		rect_step2.y = rect_step1.y + ad->preview_offset_y;
		rect_step2.width = rect_step1.width;
		rect_step2.height = rect_step1.height;
		break;
	case CAM_TARGET_DIRECTION_LANDSCAPE_INVERSE:
		rect_step2.x = ad->win_width - rect_step1.x - rect_step1.width - ad->preview_offset_x;
		rect_step2.y = ad->win_height - rect_step1.y - rect_step1.height - ad->preview_offset_y;
		rect_step2.width = rect_step1.width;
		rect_step2.height = rect_step1.height;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT:
		rect_step2.x = ad->win_height - rect_step1.y - rect_step1.height - ad->preview_offset_y;
		rect_step2.y = rect_step1.x + ad->preview_offset_x;
		rect_step2.width = rect_step1.height;
		rect_step2.height = rect_step1.width;
		break;
	case CAM_TARGET_DIRECTION_PORTRAIT_INVERSE:
		rect_step2.x = rect_step1.y + ad->preview_offset_y;
		rect_step2.y = ad->win_width - rect_step1.x - rect_step1.width - ad->preview_offset_x;
		rect_step2.width = rect_step1.height;
		rect_step2.height = rect_step1.width;
		break;
	default:
		cam_critical(LOG_UI, "invalid target direction %d", ad->target_direction);
		break;
	}

	*rect = rect_step2;

	return TRUE;
}

/*cameraplex_window xy00 is fixed*/
gboolean cam_mm_set_cameraplex_window(CamRectangle rect)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	//ret = camera_attr_set_cameraplex_inset_window_layout(g_mm_handle->hcam, rect.x, rect.y, rect.width, rect.height);
	cam_retvm_if(ret != CAMERA_ERROR_NONE, FALSE, "set cameraplex window error %x", ret);

	cam_secure_debug(LOG_CAM, "set_cameraplex_window(%d,%d,%d,%d)",
		rect.x, rect.y, rect.width, rect.height);

	return TRUE;
}

/*screen rect*/
gboolean cam_mm_set_cameraplex_window_layout(void *data, CamRectangle *rect)
{
	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	cam_retvm_if(rect == NULL, FALSE,"rect is NULL");

	CamRectangle rect_tmp = {0,};

	rect_tmp = *rect;
	cam_mm_convert_screen_rect_to_cameraplex_window(data, NULL, &rect_tmp);

	ret = cam_mm_set_cameraplex_window(rect_tmp);
	cam_retvm_if(ret != TRUE, FALSE, "set cameraplex layout error %x", ret);

	return TRUE;

}

/*cameraplex_window xy00 is fixed*/
gboolean cam_mm_get_cameraplex_window(CamRectangle *rect)
{
//	int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);
	cam_retvm_if(rect == NULL, FALSE,"rect is NULL");

//	CamRectangle rect_tmp = {0,};

//	ret = camera_attr_get_cameraplex_inset_window_layout(g_mm_handle->hcam, &rect_tmp.x, &rect_tmp.y, &rect_tmp.width, &rect_tmp.height);
//	cam_retvm_if(ret != CAMERA_ERROR_NONE, FALSE, "get cameraplex error %x", ret);
//
//	*rect = rect_tmp;
//	return TRUE;
    return FALSE;
}

/*screen rect*/
gboolean cam_mm_get_cameraplex_window_layout(void *data, CamRectangle *rect)
{
	int ret = 0;
	cam_retvm_if(rect == NULL, FALSE,"rect is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");

	CamRectangle rect_tmp = {0,};

	ret = cam_mm_get_cameraplex_window(&rect_tmp);
	cam_retvm_if(ret != TRUE, FALSE, "get cameraplex error %x", ret);

	cam_mm_convert_cameraplex_window_to_screen_rect(data, &rect_tmp);

	*rect = rect_tmp;

	return TRUE;

}

gboolean cam_mm_set_cameraplex_inset_window_orientation(int orientation)
{
	/*int ret = 0;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	ret = camera_attr_set_cameraplex_inset_window_orientation(g_mm_handle->hcam, (camera_inset_window_orientation_e)orientation);
	if (ret != CAMERA_ERROR_NONE) {
*/
		return FALSE;
	/*}
	return TRUE;*/
}

/*gboolean cam_mm_set_shutter_sound_cb(camera_shutter_sound_cb callback, void *data)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_set_shutter_sound_cb(g_mm_handle->hcam, callback, data);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_shutter_sound_cb failed - error(%d)", e);
		return FALSE;
	}
	return TRUE;
}*/

gboolean cam_mm_unset_shutter_sound_cb(void)
{
//	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

//	e = camera_unset_shutter_sound_cb(g_mm_handle->hcam);
//	if (e != CAMERA_ERROR_NONE) {
//		cam_critical(LOG_MM, "camera_unset_shutter_sound_cb failed - error(%d)", e);
		return FALSE;
//	}
//	return TRUE;
}

/*gboolean cam_mm_set_shutter_sound_completed_cb(camera_shutter_sound_completed_cb callback, void *data)
{
	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

	e = camera_set_shutter_sound_completed_cb(g_mm_handle->hcam, callback, data);
	if (e != CAMERA_ERROR_NONE) {
		cam_critical(LOG_MM,"camera_set_shutter_sound_completed_cb failed - error(%d)", e);
		return FALSE;
	}
	return TRUE;
}*/

gboolean cam_mm_unset_shutter_sound_completed_cb(void)
{
//	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

//	e = camera_unset_shutter_sound_completed_cb(g_mm_handle->hcam);
//	if (e != CAMERA_ERROR_NONE) {
//		cam_critical(LOG_MM, "camera_unset_shutter_sound_completed_cb failed - error(%d)", e);
		return FALSE;
//	}
//	return TRUE;
}

gboolean cam_mm_set_auto_exposure_lock(gboolean value)
{
//	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

//	e = camera_attr_set_auto_exposure_lock(g_mm_handle->hcam, (bool)value);
//	if (e != CAMERA_ERROR_NONE) {
//		cam_warning(LOG_MM, "camera_attr_set_auto_exposure_lock failed - error(%d)", e);
		return FALSE;
//	}
//	return TRUE;
}

gboolean cam_mm_set_auto_white_balance_lock(gboolean value)
{
//	camera_error_e e;
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hcam, FALSE);

//	e = camera_attr_set_auto_white_balance_lock(g_mm_handle->hcam, (bool)value);
//	if (e != CAMERA_ERROR_NONE) {
//		cam_warning(LOG_MM, "camera_attr_set_auto_white_balance_lock failed - error(%d)", e);
		return FALSE;
//	}
//	return TRUE;
}

gboolean cam_mm_set_audio_tuning(int tuning)
{
	g_return_val_if_fail(g_mm_handle, FALSE);
	g_return_val_if_fail(g_mm_handle->hrec, FALSE);

//	recorder_error_e err;
//	err = recorder_attr_set_audio_tuning(g_mm_handle->hrec, tuning);
//	if (err != RECORDER_ERROR_NONE) {
//		cam_critical(LOG_MM,"recorder_attr_set_audio_tuning failed - code[%x]", err);
		return FALSE;
//	}
//
//	return TRUE;
}

//end of file
