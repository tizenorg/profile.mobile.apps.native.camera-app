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


#include <image_util.h>
#include <malloc.h>
#include <ctype.h>
#include <errno.h>
#include <storage.h>
#include "cam.h"
#include "cam_config.h"
#include "edc_image_name.h"
#include "cam_shot.h"
#include "cam_app.h"
#include "cam_property.h"
#include "cam_selfie_alarm_shot.h"
#include "cam_selfie_alarm_layout.h"
#include "cam_popup.h"
#include "cam_capacity_type.h"

static CAM_COMMON_DATA cam_shoot_mode[CAM_SHOT_MODE_NUM] = {{0}};

static gboolean __cam_init_preload_mode_by_index(int mode);
static void __cam_init_single_mode(void *data);
static void __cam_init_px_mode(void *data);
static void __cam_init_selfie_alarm_mode(void *data);
static void __cam_init_self_single_mode(void *data);
static void __cam_set_default_property(void *data, int mode);
static void __cam_init_px_mode(void *data);
static void __cam_init_selfie_alarm_mode(void *data);


static void __cam_set_default_property(void *data, int mode)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL,  "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL,  "camapp is NULL");

	if (camapp->self_portrait == TRUE) {
		cam_shoot_mode[mode].shot_property.photo_resolution.data = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_SELF_PHOTO_RESOLUTION_NAME,  SELF_IMAGE_RESOLUTION_DEFAULT);
	} else {
		cam_shoot_mode[mode].shot_property.photo_resolution.data = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_PHOTO_RESOLUTION_NAME,  IMAGE_RESOLUTION_DEFAULT);
	}
	cam_shoot_mode[mode].shot_property.photo_resolution.enable = TRUE;
	cam_shoot_mode[mode].shot_property.video_resolution.data = -1;
	cam_shoot_mode[mode].shot_property.video_resolution.enable = TRUE;
	cam_shoot_mode[mode].shot_property.shot_format = CAMERA_PIXEL_FORMAT_JPEG;
	cam_shoot_mode[mode].shot_property.shot_set_format_f = NULL;
	cam_shoot_mode[mode].shot_property.tray_enable = FALSE;

	cam_shoot_mode[mode].shot_property.flash.data = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_FLASH_NAME, FLASH_DEFAULT);
	cam_shoot_mode[mode].shot_property.flash.enable = TRUE;
	cam_shoot_mode[mode].shot_property.wb.data = cam_config_get_int(CAM_CONFIG_TYPE_RESERVE, PROP_WB_NAME, WB_DEFAULT);
	cam_shoot_mode[mode].shot_property.wb.enable = TRUE;
	cam_shoot_mode[mode].shot_property.effect.data = cam_config_get_int(CAM_CONFIG_TYPE_RESERVE, PROP_EFFECT_NAME, EFFECT_DEFAULT);
	cam_shoot_mode[mode].shot_property.effect.enable = TRUE;
	cam_shoot_mode[mode].shot_property.iso.data = cam_config_get_int(CAM_CONFIG_TYPE_RESERVE, PROP_ISO_NAME, ISO_DEFAULT);
	cam_shoot_mode[mode].shot_property.iso.enable = TRUE;
	cam_shoot_mode[mode].shot_property.exposure_value.data = cam_config_get_int(CAM_CONFIG_TYPE_RESERVE, PROP_EXPOSURE_VALUE_NAME, camapp->brightness_default);
	cam_shoot_mode[mode].shot_property.exposure_value.enable = TRUE;
	cam_shoot_mode[mode].shot_property.metering.data = cam_config_get_int(CAM_CONFIG_TYPE_RESERVE, PROP_METERING_NAME, AE_DEFAULT);
	cam_shoot_mode[mode].shot_property.metering.enable = TRUE;
	cam_shoot_mode[mode].shot_property.share.data = cam_config_get_int(CAM_CONFIG_TYPE_RESERVE, PROP_SHARE_NAME, SHARE_DEFAULT);
	cam_shoot_mode[mode].shot_property.share.enable = TRUE;
	cam_shoot_mode[mode].shot_property.face_detection.data = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_FACE_DETECTION_NAME, FACE_DETECTION_DEFAULT);
	cam_shoot_mode[mode].shot_property.face_detection.enable = TRUE;
	cam_shoot_mode[mode].shot_property.timer.data = cam_config_get_int(CAM_CONFIG_TYPE_RESERVE, PROP_TIMER_NAME, TIMER_DEFAULT);
	cam_shoot_mode[mode].shot_property.timer.enable = TRUE;
	cam_shoot_mode[mode].shot_property.storage.data = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_STORAGE_NAME, STORAGE_DEFAULT);
	cam_shoot_mode[mode].shot_property.storage.enable = TRUE;
	cam_shoot_mode[mode].shot_property.scene.data = CAM_SCENE_NONE;
	cam_shoot_mode[mode].shot_property.scene.enable = TRUE;

	cam_shoot_mode[mode].shot_property.fps.data = FPS_AUTO;
	cam_shoot_mode[mode].shot_property.fps.enable = TRUE;
	cam_shoot_mode[mode].shot_property.volume_key.data = cam_config_get_int(CAM_CONFIG_TYPE_COMMON, PROP_VOLUME_KEY_NAME, VOLUME_KEY_DEFAULT);;
	cam_shoot_mode[mode].shot_property.volume_key.enable = TRUE;

	cam_shoot_mode[mode].shot_property.ahs.data = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_AHS_NAME, AHS_DEFAULT);
	cam_shoot_mode[mode].shot_property.ahs.enable = TRUE;
	cam_shoot_mode[mode].shot_property.video_stabilization.data = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_VIDEO_STABILIZATION_NAME, VIDEO_STABILIZATION_DEFAULT);
	cam_shoot_mode[mode].shot_property.video_stabilization.enable = TRUE;
	cam_shoot_mode[mode].shot_property.review.data =  cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_REVIEW_NAME, REVIEW_DEFAULT);
	cam_shoot_mode[mode].shot_property.review.enable = TRUE;
	cam_shoot_mode[mode].shot_property.tap_shot.data = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_TAP_SHOT_NAME, TAP_SHOT_DEFAULT);
	cam_shoot_mode[mode].shot_property.tap_shot.enable = TRUE;
	cam_shoot_mode[mode].shot_property.save_as_flip.data = FALSE;
	cam_shoot_mode[mode].shot_property.save_as_flip.enable = TRUE;
	cam_shoot_mode[mode].shot_property.gps.data = cam_config_get_boolean(CAM_CONFIG_TYPE_COMMON, PROP_GPS_NAME, GPS_DEFAULT);/*GPS HERE*/
	cam_shoot_mode[mode].shot_property.gps.enable = TRUE;
	cam_shoot_mode[mode].shot_property.shot_set_format_f = NULL;
}

/*
 * init property of single mode
 */

static void __cam_init_single_mode(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL,  "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL,  "camapp is NULL");

	/*basic information*/
	int mode = CAM_SINGLE_MODE;
	IF_FREE(cam_shoot_mode[mode].shot_name);
	IF_FREE(cam_shoot_mode[mode].shot_mode_display);
	cam_shoot_mode[mode].shot_name = CAM_STRDUP("CAM_SINGLE_MODE");
	cam_shoot_mode[mode].b_preload = TRUE;
	cam_shoot_mode[mode].shot_index = CAM_SINGLE_MODE;
	cam_shoot_mode[mode].shot_cp_value = CAM_CP_SHOT_MODE_AUTO;
	cam_shoot_mode[mode].shot_mode_display = CAM_STRDUP("IDS_CAM_BODY_AUTO");
	cam_shoot_mode[mode].shot_mode_icon_path = SHOTTING_MODE_AUTO_ICON;
	cam_shoot_mode[mode].shot_mode_description = dgettext(PACKAGE, "IDS_CAM_POP_AUTOMATICALLY_ADJUSTS_THE_EXPOSURE_TO_OPTIMISE_THE_COLOUR_AND_BRIGHTNESS_OF_PICTURES_YOU_CAN_TAKE_SINGLE_OR_BURST_SHOTS");

	/*set founction callback*/
	cam_shoot_mode[mode].shot_create_f = NULL;
	cam_shoot_mode[mode].shot_capture_start = NULL;
	cam_shoot_mode[mode].shot_capture_complete = NULL;
	cam_shoot_mode[mode].shot_destroy_f = NULL;
	cam_shoot_mode[mode].shot_capture_stop = NULL;
	cam_shoot_mode[mode].shot_is_capturing = NULL;

	cam_shoot_mode[mode].shot_capture_cb = cam_shot_capture_cb;
	cam_shoot_mode[mode].shot_capture_completed_cb = cam_shot_capture_completed_cb;
	cam_shoot_mode[mode].shot_preview_cb = NULL;
	/*cam_shoot_mode[mode].shot_shutter_sound_cb = cam_shot_shutter_sound_cb;*/
	/*cam_shoot_mode[mode].shutter_sound_completed_cb = NULL;*/
	cam_shoot_mode[mode].shot_set_cb_f = NULL;

	/*set mode porperty  -1 means do not set it*/
	cam_shoot_mode[mode].shot_property.property_index = CAM_MENU_SHOOTING_SINGLE;
	__cam_set_default_property(ad, mode);
	cam_shoot_mode[mode].shot_property.review.data = (ad->launching_mode == CAM_LAUNCHING_MODE_NORMAL) ? cam_shoot_mode[mode].shot_property.review.data : TRUE;
	cam_shoot_mode[mode].shot_property.tray_enable = TRUE;
	cam_shoot_mode[mode].shot_property.shot_format = CAMERA_PIXEL_FORMAT_JPEG;
	cam_shoot_mode[mode].shot_property.shot_set_format_f = NULL;
}

/*
 * init property of px mode
 */

static void __cam_init_px_mode(void *data)
{

}

/*
 * init property of selfie mode
 */

static void __cam_init_selfie_alarm_mode(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL,  "ad is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL,  "camapp is NULL");

	/*basic information*/
	int mode = CAM_SELFIE_ALARM_MODE;
	IF_FREE(cam_shoot_mode[mode].shot_name);
	IF_FREE(cam_shoot_mode[mode].shot_mode_display);
	cam_shoot_mode[mode].shot_name = CAM_STRDUP("CAM_SELFIE_ALARM_MODE");
	cam_shoot_mode[mode].b_preload = TRUE;
	cam_shoot_mode[mode].shot_index = CAM_SELFIE_ALARM_MODE;
	cam_shoot_mode[mode].shot_cp_value = CAM_CP_SHOT_MODE_SELFIE_ALARM;
	cam_shoot_mode[mode].shot_mode_display = CAM_STRDUP("IDS_CAM_HEADER_SELFIE_ALARM");
	cam_shoot_mode[mode].shot_mode_icon_path = SHOTTING_MODE_SELFIE_ALARM_ICON;
	cam_shoot_mode[mode].shot_mode_description = dgettext(PACKAGE, "IDS_CAM_POP_THE_REAR_CAMERA_WILL_AUTOMATICALLY_DETECT_AND_FOCUS_ON_YOUR_FACE_WHEN_YOU_TAKE_SELF_PORTRAIT_PICTURES");

	/*set founction callback*/
	cam_shoot_mode[mode].shot_create_f = cam_selfie_alarm_shot_init;
	cam_shoot_mode[mode].shot_destroy_f = cam_selfie_alarm_shot_deinit;
	cam_shoot_mode[mode].shot_capture_start = cam_selfie_alarm_shot_capture_start;
	cam_shoot_mode[mode].shot_capture_stop = cam_selfie_alarm_photo_shot_stop;
	cam_shoot_mode[mode].shot_capture_complete = cam_selfie_alarm_shot_complete;
	cam_shoot_mode[mode].shot_is_capturing = cam_selfie_alarm_shot_is_capturing;

	cam_shoot_mode[mode].shot_capture_cb = cam_selfie_alarm_shot_capture_callback;
	cam_shoot_mode[mode].shot_capture_completed_cb = cam_shot_capture_completed_cb;
	cam_shoot_mode[mode].shot_preview_cb = cam_selfie_alarm_layout_check_face_area;
	/*cam_shoot_mode[mode].shot_hdr_processing_cb = NULL;
	cam_shoot_mode[mode].shot_shutter_sound_cb = NULL;
	cam_shoot_mode[mode].shutter_sound_completed_cb = NULL;*/

	/*set mode porperty  -1 means do not set it*/
	cam_shoot_mode[mode].shot_property.property_index = CAM_MENU_SHOOTING_SELFIE_ALARM;
	__cam_set_default_property(ad, mode);
	cam_shoot_mode[mode].shot_property.shot_format = CAMERA_PIXEL_FORMAT_JPEG;
	cam_shoot_mode[mode].shot_property.face_detection.data = CAM_FACE_DETECTION_OFF;
	cam_shoot_mode[mode].shot_property.metering.data = CAM_METERING_CENTER_WEIGHTED;
	cam_shoot_mode[mode].shot_property.ahs.data = FALSE;
	cam_shoot_mode[mode].shot_property.exposure_value.data = camapp->brightness_default;
	cam_shoot_mode[mode].shot_property.review.data = FALSE;
	cam_shoot_mode[mode].shot_property.timer.data = CAM_SETTINGS_TIMER_OFF;
	cam_shoot_mode[mode].shot_property.video_stabilization.data = FALSE;
	cam_shoot_mode[mode].shot_property.wb.data = CAM_SETTINGS_WB_AWB;
	cam_shoot_mode[mode].shot_property.flash.data = CAM_FLASH_OFF;
	cam_shoot_mode[mode].shot_property.effect.data = CAM_SETTINGS_EFFECTS_NOR;
	cam_shoot_mode[mode].shot_property.volume_key.data = CAM_VOLUME_KEY_CAMERA;

	cam_shoot_mode[mode].shot_property.flash.enable = FALSE;
	cam_shoot_mode[mode].shot_property.exposure_value.enable = FALSE;
	cam_shoot_mode[mode].shot_property.timer.enable = FALSE;
	cam_shoot_mode[mode].shot_property.effect.enable = FALSE;
	cam_shoot_mode[mode].shot_property.wb.enable = FALSE;
	cam_shoot_mode[mode].shot_property.review.enable = FALSE;
	cam_shoot_mode[mode].shot_property.metering.enable = FALSE;
	cam_shoot_mode[mode].shot_property.face_detection.enable = FALSE;
	cam_shoot_mode[mode].shot_property.volume_key.enable = FALSE;
}

/*
 * init property of self single mode
 */

static void __cam_init_self_single_mode(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL,  "ad is NULL");

	int mode = CAM_SELF_SINGLE_MODE;
	IF_FREE(cam_shoot_mode[mode].shot_name);
	IF_FREE(cam_shoot_mode[mode].shot_mode_display);
	cam_shoot_mode[mode].shot_name = CAM_STRDUP("CAM_SELF_SINGLE_MODE");
	cam_shoot_mode[mode].b_preload = TRUE;
	cam_shoot_mode[mode].shot_index = CAM_SELF_SINGLE_MODE;
	cam_shoot_mode[mode].shot_cp_value = CAM_CP_SHOT_MODE_AUTO;
	cam_shoot_mode[mode].shot_mode_display = CAM_STRDUP("IDS_CAM_OPT_SELFIE_ABB");
	/*cam_shoot_mode[mode].shot_mode_display = CAM_STRDUP(cam_shoot_mode[CAM_SINGLE_MODE].shot_mode_display);*/
	cam_shoot_mode[mode].shot_mode_icon_path = cam_shoot_mode[CAM_SINGLE_MODE].shot_mode_icon_path;
	cam_shoot_mode[mode].shot_mode_description = cam_shoot_mode[CAM_SINGLE_MODE].shot_mode_description;

	/*set founction callback*/
	cam_shoot_mode[mode].shot_create_f = NULL;
	cam_shoot_mode[mode].shot_capture_start = NULL;
	cam_shoot_mode[mode].shot_capture_complete = NULL;
	cam_shoot_mode[mode].shot_destroy_f = NULL;
	cam_shoot_mode[mode].shot_capture_stop = NULL;
	cam_shoot_mode[mode].shot_is_capturing = NULL;

	cam_shoot_mode[mode].shot_capture_cb = cam_shot_capture_cb;
	cam_shoot_mode[mode].shot_capture_completed_cb = cam_shot_capture_completed_cb;
	cam_shoot_mode[mode].shot_preview_cb = NULL;
	/*cam_shoot_mode[mode].shot_shutter_sound_cb = cam_shot_shutter_sound_cb;*/
	/*cam_shoot_mode[mode].shutter_sound_completed_cb = NULL;*/
	cam_shoot_mode[mode].shot_set_cb_f = NULL;

	/*set mode porperty  -1 means do not set it*/
	cam_shoot_mode[mode].shot_property.property_index = CAM_MENU_SHOOTING_SELF_SINGLE;
	__cam_set_default_property(ad, mode);
	cam_shoot_mode[mode].shot_property.review.data = (ad->launching_mode == CAM_LAUNCHING_MODE_NORMAL) ? cam_shoot_mode[mode].shot_property.review.data : TRUE;
	cam_shoot_mode[mode].shot_property.tray_enable = TRUE;
	cam_shoot_mode[mode].shot_property.shot_format = CAMERA_PIXEL_FORMAT_JPEG;
	cam_shoot_mode[mode].shot_property.metering.data = CAM_METERING_CENTER_WEIGHTED;
	cam_shoot_mode[mode].shot_property.ahs.data = FALSE;
	cam_shoot_mode[mode].shot_property.video_stabilization.data = FALSE;
	cam_shoot_mode[mode].shot_property.flash.data = CAM_FLASH_OFF;

	cam_shoot_mode[mode].shot_property.shot_set_format_f = NULL;
}

static gboolean __cam_init_preload_mode_by_index(int mode)
{
	cam_debug(LOGUI, "start");
	if (mode <= CAM_SHOT_MODE_MIN || mode >= CAM_SHOT_MODE_NUM) {
		cam_critical(LOG_UI, "mode is %d out of bounds");
		return FALSE;
	}

	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, FALSE, "ad is NULL");

	switch (mode) {
		case CAM_SINGLE_MODE:
			__cam_init_single_mode(ad);
			break;
		case CAM_PX_MODE:
			__cam_init_px_mode(ad);
			break;
		case CAM_SELFIE_ALARM_MODE:
			__cam_init_selfie_alarm_mode(ad);
			break;
		case CAM_SELF_SINGLE_MODE:
			__cam_init_self_single_mode(ad);
			break;
		default:
			break;
	}
	return TRUE;
}

static gboolean __is_mode_data_valid(int mode)
{
	if (mode <= CAM_SHOT_MODE_MIN || mode >= CAM_SHOT_MODE_NUM) {
		cam_critical(LOG_UI, "mode is %d out of bounds");
		return FALSE;
	}

	if (cam_shoot_mode[mode].shot_name == NULL) {
		return FALSE;
	}

	return TRUE;
}

void cam_init_shooting_mode()
{
	cam_debug(LOG_UI, "cam_init_shooting_mode");

	int mode_index = 0;
	for (mode_index = (CAM_SHOT_MODE_MIN + 1); mode_index < CAM_SHOT_MODE_NUM; mode_index++) {
		__cam_init_preload_mode_by_index(mode_index);
	}
}

CAM_COMMON_DATA *cam_get_shooting_mode(int mode)
{
	if (!__is_mode_data_valid(mode)) {
		cam_warning(LOG_UI, "date for mode=%d is invalid", mode);
		return NULL;
	}

	return &cam_shoot_mode[mode];
}

CAM_COMMON_PROPERTY *cam_get_shooting_mode_property(int mode)
{
	if (!__is_mode_data_valid(mode)) {
		return NULL;
	}

	return &cam_shoot_mode[mode].shot_property;
}

void cam_shooting_mode_reset_property(int mode)
{
	if (!__is_mode_data_valid(mode)) {
		return ;
	}

	__cam_init_preload_mode_by_index(mode);
}

void cam_destroy_shooting_mode()
{
	cam_debug(LOG_UI, "cam_destroy_shooting_mode");

	int mode_index = 0;
	for (mode_index = (CAM_SHOT_MODE_MIN + 1); mode_index < CAM_SHOT_MODE_NUM; mode_index++) {
		IF_FREE(cam_shoot_mode[mode_index].shot_name);
		IF_FREE(cam_shoot_mode[mode_index].shot_mode_display);
		memset(&cam_shoot_mode[mode_index], 0x00, sizeof(CAM_COMMON_DATA));
	}
}


