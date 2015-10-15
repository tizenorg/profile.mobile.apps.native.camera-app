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

#include <app.h>
#include "cam_menu_composer.h"
#include "cam_debug.h"
#include "cam_capacity_type.h"
#include "cam_device_capacity.h"
#include "cam_typeconverter.h"
#include "cam_menu_item.h"
#include <storage.h>

const CAM_MENU_ITEM setting_rear[] = { CAM_MENU_PHOTO_RESOLUTION,
					CAM_MENU_VIDEO_RESOLUTION,
					CAM_MENU_TAP_SHOT,
					CAM_MENU_GPS_TAG,
					CAM_MENU_STORAGE,
					CAM_MENU_VOLUME_KEY,
					CAM_MENU_RESET
				};

const CAM_MENU_ITEM setting_front[] = {	CAM_MENU_PHOTO_RESOLUTION,
					CAM_MENU_VIDEO_RESOLUTION,
				};

const CAM_MENU_ITEM scene_mode[] = {CAM_MENU_SCENE_AUTO		, CAM_MENU_SCENE_PORTRAIT	, CAM_MENU_SCENE_LANDSCAPE	, CAM_MENU_SCENE_NIGHT	, CAM_MENU_SCENE_SPORTS
								, CAM_MENU_SCENE_PARTY		, CAM_MENU_SCENE_BEACHSNOW	, CAM_MENU_SCENE_SUNSET		, CAM_MENU_SCENE_DUSKDAWN, CAM_MENU_SCENE_FALL
								, CAM_MENU_SCENE_FIREWORK	, CAM_MENU_SCENE_TEXT		, CAM_MENU_SCENE_CANDLELIGHT, CAM_MENU_SCENE_BACKLIGHT, CAM_MENU_SCENE_SHOW_WINDOW};

const CAM_MENU_ITEM shooting_mode[] = { CAM_MENU_SHOOTING_SINGLE,
										CAM_MENU_SHOOTING_SOUND,
										CAM_MENU_SHOOTING_PX,			/*10*/
										CAM_MENU_SHOOTING_AUTO_PORTRAIT,
										CAM_MENU_SHOOTING_SPORTS,
										CAM_MENU_SHOOTING_AUTO,		/*15*/
										CAM_MENU_SHOOTING_SELF_SINGLE,
										CAM_MENU_SHOOTING_SELF_AUTO,
										CAM_MENU_SHOOTING_SELF_SOUND,
										};

const CAM_MENU_ITEM recording_mode[] = {CAM_MENU_RECORDING_MODE_NORMAL,			CAM_MENU_RECORDING_MODE_LIMIT,		CAM_MENU_RECORDING_MODE_SLOW,	CAM_MENU_RECORDING_MODE_FAST,
										CAM_MENU_RECORDING_MODE_SELF_NORMAL,	CAM_MENU_RECORDING_MODE_SELF_LIMIT};

const CAM_MENU_ITEM photo_resolution[] = {
					CAM_MENU_PHOTO_RESOLUTION_3264x2448, /*0*/
					CAM_MENU_PHOTO_RESOLUTION_3264x1836,
					CAM_MENU_PHOTO_RESOLUTION_2560x1920,
					CAM_MENU_PHOTO_RESOLUTION_2560x1440,
					CAM_MENU_PHOTO_RESOLUTION_2448x2448,
					CAM_MENU_PHOTO_RESOLUTION_2048x1536, /*5*/
					CAM_MENU_PHOTO_RESOLUTION_2048x1232,
					CAM_MENU_PHOTO_RESOLUTION_2048x1152,
					CAM_MENU_PHOTO_RESOLUTION_1920x1080,
					CAM_MENU_PHOTO_RESOLUTION_1600x1200,
					CAM_MENU_PHOTO_RESOLUTION_1600x960	,/*10*/
					CAM_MENU_PHOTO_RESOLUTION_1392x1392,
					CAM_MENU_PHOTO_RESOLUTION_1280x960,
					CAM_MENU_PHOTO_RESOLUTION_1280x720,
					CAM_MENU_PHOTO_RESOLUTION_1056x1056,
					CAM_MENU_PHOTO_RESOLUTION_720x480,  /*15*/
					CAM_MENU_PHOTO_RESOLUTION_640x480,
					CAM_MENU_PHOTO_RESOLUTION_800x450,
					CAM_MENU_PHOTO_RESOLUTION_800x480};

const CAM_MENU_ITEM video_resolution[] = {CAM_MENU_VIDEO_RESOLUTION_FULLHD, /*0*/
										CAM_MENU_VIDEO_RESOLUTION_1440x1080,
										CAM_MENU_VIDEO_RESOLUTION_1056x1056,
										CAM_MENU_VIDEO_RESOLUTION_1080x1080,
										CAM_MENU_VIDEO_RESOLUTION_HD,
										CAM_MENU_VIDEO_RESOLUTION_WVGA, /*5*/
										CAM_MENU_VIDEO_RESOLUTION_VGA,
										CAM_MENU_VIDEO_RESOLUTION_QVGA,
										CAM_MENU_VIDEO_RESOLUTION_QCIF};

const CAM_MENU_ITEM wb[] = {CAM_MENU_WB_AWB	, CAM_MENU_WB_DAYLIGHT	, CAM_MENU_WB_CLOUDY	, CAM_MENU_WB_INCANDESCENT	, CAM_MENU_WB_FLUORESCENT};

const CAM_MENU_ITEM effect[] = {CAM_MENU_EFFECT_NONE,
								CAM_MENU_EFFECT_GREY,
								CAM_MENU_EFFECT_SEPIA,
								CAM_MENU_EFFECT_NEGATIVE};

const CAM_MENU_ITEM focus_mode[] = {CAM_MENU_FOCUS_AUTO	,	CAM_MENU_FOCUS_MACRO	,	CAM_MENU_FOCUS_FACE_DETECTION	};

const CAM_MENU_ITEM flash[] = {CAM_MENU_FLASH_OFF	,	CAM_MENU_FLASH_ON	,	CAM_MENU_FLASH_AUTO	};

const CAM_MENU_ITEM metering[] = {CAM_MENU_METERING_CENTER_WEIGHTED	, CAM_MENU_METERING_MATRIX	, CAM_MENU_METERING_SPOT	};

const CAM_MENU_ITEM iso[] = {CAM_MENU_ISO_AUTO	,	CAM_MENU_ISO_50		,CAM_MENU_ISO_100, 	CAM_MENU_ISO_200
						,CAM_MENU_ISO_400	,	CAM_MENU_ISO_80O,	CAM_MENU_ISO_1600,	CAM_MENU_ISO_320O};

const CAM_MENU_ITEM shutter_sound[] = {CAM_MENU_SHUTTER_SOUND_OFF,	CAM_MENU_SHUTTER_SOUND_ON};

const CAM_MENU_ITEM storage[] = {CAM_MENU_STORAGE_PHONE, CAM_MENU_STORAGE_MMC};
const CAM_MENU_ITEM timer[] = {CAM_MENU_TIMER_OFF, CAM_MENU_TIMER_3SEC, CAM_MENU_TIMER_10SEC};
const CAM_MENU_ITEM volume_key[] = {CAM_MENU_VOLUME_KEY_ZOOM, CAM_MENU_VOLUME_KEY_CAMERA, CAM_MENU_VOLUME_KEY_RECORD};
const CAM_MENU_ITEM gps[] = {CAM_MENU_GPS_TAG_OFF, CAM_MENU_GPS_TAG_ON};

const CAM_MENU_ITEM anti_shake[] = {CAM_MENU_ANTI_SHAKE_OFF, CAM_MENU_ANTI_SHAKE_ON};

const CAM_MENU_ITEM video_stabilization[] = {CAM_MENU_VIDEO_STABILIZATION_OFF, CAM_MENU_VIDEO_STABILIZATION_ON};

const CAM_MENU_ITEM auto_contrast[] = {CAM_MENU_AUTO_CONTRAST_OFF, CAM_MENU_AUTO_CONTRAST_ON};

const CAM_MENU_ITEM save_as_flip[] = {CAM_MENU_SAVE_AS_FLIP_OFF, CAM_MENU_SAVE_AS_FLIP_ON};

const CAM_MENU_ITEM tap_shot[] = {CAM_MENU_TAP_SHOT_OFF, CAM_MENU_TAP_SHOT_ON};

const CAM_MENU_ITEM review[] = {CAM_MENU_REVIEW_OFF, CAM_MENU_REVIEW_ON};

const CAM_MENU_ITEM share[] = {CAM_MENU_SHARE_OFF, CAM_MENU_SHARE_BUDDY_PHOTO};

const CAM_MENU_ITEM face_detection[] = {CAM_MENU_FACE_DETECTION_OFF, CAM_MENU_FACE_DETECTION_ON};

const CAM_MENU_ITEM fast_motion[] = {CAM_MENU_FAST_MOTION_X2, CAM_MENU_FAST_MOTION_X4, CAM_MENU_FAST_MOTION_X8};

const CAM_MENU_ITEM slow_motion[] = {CAM_MENU_SLOW_MOTION_X2, CAM_MENU_SLOW_MOTION_X4, CAM_MENU_SLOW_MOTION_X8};

static gboolean __is_enabled_flash(void* data);
static gboolean __is_enabled_shooting_mode(void* data);
static gboolean __is_enabled_recording_mode(void* data);
static gboolean __is_enabled_scene_mode(void* data);
static gboolean __is_enabled_exposure_value(void* data);
static gboolean __is_enabled_focus_mode(void* data);
static gboolean __is_enabled_timer(void* data);
static gboolean __is_enabled_effects(void* data);
static gboolean __is_enabled_photo_resolution(void* data);
static gboolean __is_enabled_video_resolution(void* data);
static gboolean __is_enabled_white_balance(void* data);
static gboolean __is_enabled_iso(void* data);
static gboolean __is_enabled_metering(void* data);
static gboolean __is_enabled_anti_shake(void* data);
static gboolean __is_enabled_video_stabilization(void* data);
static gboolean __is_enabled_auto_contrast(void* data);
static gboolean __is_enabled_shutter_sound(void* data);
static gboolean __is_enabled_gps_tag(void* data);
static gboolean __is_enabled_storage(void* data);
static gboolean __is_enabled_self_mode(void* data);
static gboolean __is_enabled_save_as_flip(void *data);
static gboolean __is_enabled_review(void* data);
static gboolean __is_enabled_buddy_photo_share(void* data);
static gboolean __is_enabled_face_detection(void* data);
static gboolean __is_enabled_volume_key(void* data);
static gboolean __is_enabled_reset(void* data);
static gboolean __is_enabled_tap_shot_shot(void* data);
static void __cam_push_mode_into_array(int mode, cam_menu_composer* composer, int capacity);


unsigned int __get_capacity_mask_by_menu_type(CAM_MENU_ITEM menu_item)
{
	unsigned int capacity_mask = CAM_CP_FUNC_NOT_SUPPORT;
	switch (menu_item)
	{
		case CAM_MENU_SELF_PORTRAIT:
			capacity_mask = CAM_CP_FUNC_SELF_MODE;
			break;
		case CAM_MENU_FLASH:
			capacity_mask = CAM_CP_FUNC_FLASH_MODE;
			break;
		case CAM_MENU_SHOOTING_MODE:
			capacity_mask = CAM_CP_FUNC_SHOT_MODE;
			break;
		case CAM_MENU_RECORDING_MODE:
			capacity_mask = CAM_CP_FUNC_REC_MODE;
			break;
		case CAM_MENU_SCENE_MODE:
			capacity_mask = CAM_CP_FUNC_SCENE_MODE;
			break;
		case CAM_MENU_EXPOSURE_VALUE:
			capacity_mask = CAM_CP_FUNC_EXPOSURE;
			break;
		case CAM_MENU_FOCUS_MODE:
			capacity_mask = CAM_CP_FUNC_FOCUS_MODE;
			break;
		case CAM_MENU_EFFECTS:
			capacity_mask = CAM_CP_FUNC_EFFECT_MODE;
			break;
		case CAM_MENU_PHOTO_RESOLUTION:
			capacity_mask = CAM_CP_FUNC_CAM_RESOLUTION;
			break;
		case CAM_MENU_VIDEO_RESOLUTION:
			capacity_mask = CAM_CP_FUNC_REC_RESOLUTION;
			break;
		case CAM_MENU_WHITE_BALANCE:
			capacity_mask = CAM_CP_FUNC_WHITE_BALANCE;
			break;
		case CAM_MENU_ISO:
			capacity_mask = CAM_CP_FUNC_ISO;
			break;
		case CAM_MENU_METERING:
			capacity_mask = CAM_CP_FUNC_METERING;
			break;
		case CAM_MENU_ANTI_SHAKE:
			capacity_mask = CAM_CP_FUNC_CAM_ANS;
			break;
		case CAM_MENU_VIDEO_STABILIZATION:
			capacity_mask = CAM_CP_FUNC_REC_ANS;
			break;
		case CAM_MENU_AUTO_CONTRAST:
			capacity_mask = CAM_CP_FUNC_AUTO_CONTRAST;
			break;
		case CAM_MENU_TAP_SHOT:
			capacity_mask = CAM_CP_FUNC_TAP_SHOT;
			break;
		case CAM_MENU_GPS_TAG:
			capacity_mask = CAM_CP_FUNC_SUPPORT;
			break;
		case CAM_MENU_STORAGE:
			capacity_mask = CAM_CP_FUNC_SUPPORT;
			break;
		case CAM_MENU_FACE_DETECTION:
			capacity_mask = CAM_CP_FUNC_FACE_DETECTION;
			break;
		case CAM_MENU_SHUTTER_SOUND:
			capacity_mask = CAM_CP_FUNC_NOT_SUPPORT;
			break;
		case CAM_MENU_SHORTCUTS:
		case CAM_MENU_VOLUME_KEY:
		case CAM_MENU_TIMER:
		case CAM_MENU_SAVE_AS_FLIP:
		case CAM_MENU_RESET:
		case CAM_MENU_REVIEW:
		case CAM_MENU_MORE_HELP:
			/* is support, special mask */
			capacity_mask = CAM_CP_FUNC_SUPPORT;
			break;
		default:
			/* not support, special mask */
			cam_critical(LOG_CAM, "could not find menu_item %d", menu_item);
			capacity_mask = CAM_CP_FUNC_NOT_SUPPORT;
			break;
	}
	return capacity_mask;
}

void cam_compose_menu_common(void *data, cam_menu_composer* composer, CAM_MENU_ITEM *menu_list, int list_cnt)
{
	struct appdata *ad = NULL;
	CamAppData *camapp = NULL;
	unsigned int capacity = 0;
	unsigned int capacity_mask = CAM_CP_FUNC_NOT_SUPPORT;
	ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_retm_if(composer == NULL, "composer is NULL");

	cam_retm_if(list_cnt <= 0, "menu_cnt is invalid");

	int list_index = 0;

	composer->array = eina_array_new(list_cnt);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}

	eina_array_step_set(composer->array, sizeof(*(composer->array)), 5);

	GetCamDevFuncCaps(&capacity, data);

	for (list_index = 0; list_index < list_cnt; list_index++) {
		capacity_mask = __get_capacity_mask_by_menu_type(menu_list[list_index]);

		if((CAM_CP_FUNC_SUPPORT == capacity_mask)
			|| (capacity & capacity_mask)) {
			eina_array_push(composer->array, (void*)&menu_list[list_index]);
		}
	}

	return;
}

void cam_compose_menu_without_getcaps(void *data, cam_menu_composer* composer, CAM_MENU_ITEM *menu_list, int list_cnt)
{
	struct appdata *ad = NULL;
	CamAppData *camapp = NULL;


	ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_retm_if(composer == NULL, "composer is NULL");

	cam_retm_if(list_cnt <= 0, "menu_cnt is invalid");

	int list_index = 0;

	composer->array = eina_array_new(list_cnt);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}

	eina_array_step_set(composer->array, sizeof(*(composer->array)), 5);

	for (list_index = 0; list_index < list_cnt; list_index++)
	{
		eina_array_push(composer->array, (void*)&menu_list[list_index]);
		cam_debug(LOG_CAM, "composer type %d", menu_list[list_index]);
	}

	return;
}

void cam_compose_list_menu(void *data, int menu_type, cam_menu_composer* composer)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_retm_if(composer == NULL, "composer is NULL");

	switch ((CAM_MENU_ITEM)menu_type)
	{
		case CAM_MENU_VOLUME_KEY:
			cam_compose_menu_volume_key(data, composer);
			break;
		case CAM_MENU_SHOOTING_MODE:
			cam_compose_menu_shooting_mode(data, composer);
			break;
		case CAM_MENU_FLASH:
			cam_compose_menu_flash(data, composer);
			break;
		case CAM_MENU_RECORDING_MODE:
			cam_compose_menu_recording_mode(data, composer);
			break;
		case CAM_MENU_PHOTO_RESOLUTION:
			cam_compose_menu_photo_resolution(data, composer);
			break;
		case CAM_MENU_VIDEO_RESOLUTION:
			cam_compose_menu_video_resolution(data, composer);
			break;
		case CAM_MENU_ISO:
			cam_compose_menu_iso(data, composer);
			break;
		case CAM_MENU_FOCUS_MODE:
			cam_compose_menu_focus_mode(data, composer);
			break;
		case CAM_MENU_TIMER:
			cam_compose_menu_timer_mode(data, composer);
			break;
		case CAM_MENU_SCENE_MODE:
			cam_compose_menu_scene_mode(data, composer);
			break;
		case CAM_MENU_WHITE_BALANCE:
			cam_compose_menu_wb(data, composer);
			break;
		case CAM_MENU_EFFECTS:
			cam_compose_menu_effect(data, composer);
			break;
		case CAM_MENU_METERING:
			cam_compose_menu_metering(data, composer);
			break;
		case CAM_MENU_SHUTTER_SOUND:
			cam_compose_menu_shutter_sound(data, composer);
			break;
		case CAM_MENU_STORAGE:
			cam_compose_menu_storage(data, composer);
			break;
		case CAM_MENU_ANTI_SHAKE:
			cam_compose_menu_without_getcaps(data, composer,
				(CAM_MENU_ITEM *)anti_shake, sizeof(anti_shake)/sizeof(CAM_MENU_ITEM));
			break;
		case CAM_MENU_VIDEO_STABILIZATION:
			cam_compose_menu_without_getcaps(data, composer,
				(CAM_MENU_ITEM *)video_stabilization, sizeof(video_stabilization)/sizeof(CAM_MENU_ITEM));
			break;
		case CAM_MENU_AUTO_CONTRAST:
			cam_compose_menu_without_getcaps(data, composer,
				(CAM_MENU_ITEM *)auto_contrast, sizeof(auto_contrast)/sizeof(CAM_MENU_ITEM));
			break;
		case CAM_MENU_SAVE_AS_FLIP:
			cam_compose_menu_without_getcaps(data, composer,
				(CAM_MENU_ITEM *)save_as_flip, sizeof(save_as_flip)/sizeof(CAM_MENU_ITEM));
			break;
		case CAM_MENU_TAP_SHOT:
			cam_compose_menu_without_getcaps(data, composer,
				(CAM_MENU_ITEM *)tap_shot, sizeof(tap_shot)/sizeof(CAM_MENU_ITEM));
			break;
		case CAM_MENU_GPS_TAG:
			cam_compose_menu_without_getcaps(data, composer,
				(CAM_MENU_ITEM *)gps, sizeof(gps)/sizeof(CAM_MENU_ITEM));
			break;
		case CAM_MENU_REVIEW:
			cam_compose_menu_without_getcaps(data, composer,
				(CAM_MENU_ITEM *)review, sizeof(review)/sizeof(CAM_MENU_ITEM));
			break;
		case CAM_MENU_SHARE:
			cam_compose_menu_share(data, composer);
			break;
		case CAM_MENU_FACE_DETECTION:
			cam_compose_menu_face_detection(data, composer);
			break;
		case CAM_MENU_FAST_MOTION:
			cam_compose_menu_fast_motion(data, composer);
			break;
		case CAM_MENU_SLOW_MOTION:
			cam_compose_menu_slow_motion(data, composer);
			break;
		default:
			cam_debug(LOG_UI, "invalid menu type");
			break;
	}
}

static void __cam_push_mode_into_array(int mode, cam_menu_composer* composer, int capacity)
{
	CAM_COMMON_PROPERTY *mode_property = NULL;
	CAM_COMMON_DATA *mode_data = cam_get_shooting_mode(mode);
	if (mode_data != NULL) {
		mode_property = &(mode_data->shot_property);
		if (mode_property != NULL && (capacity & mode_data->shot_cp_value)) {
			cam_debug(LOG_UI, "__cam_push_mode_into_array pushes %d now", mode_property->property_index);
			eina_array_push(composer->array, (void*)&(mode_property->property_index));
		}
	}
}

void cam_compose_menu_shooting_mode(void *data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	composer->array = eina_array_new(5);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 5);

	unsigned int capacity = 0;
	unsigned int scene_capacity = 0;
	GetCamDevShotModeCaps(&capacity, data);
	GetCamDevSceneModeCaps(&scene_capacity, data);

	if (camapp->self_portrait == FALSE) {
		int mode = CAM_SHOT_NORMAL_MODE_BEGIN;
		for (mode = CAM_SHOT_NORMAL_MODE_BEGIN; mode < CAM_SHOT_NORMAL_MODE_NUM; mode++) {
			if (ad->launching_mode != CAM_LAUNCHING_MODE_NORMAL &&
				(mode == CAM_SINGLE_MODE)) {
				__cam_push_mode_into_array(mode, composer, capacity);
			} else if (ad->launching_mode == CAM_LAUNCHING_MODE_NORMAL) {
				__cam_push_mode_into_array(mode, composer, capacity);
			}
		}

	} else {
		int mode = CAM_SHOT_SELF_MODE_BEGIN;
		for (mode = CAM_SHOT_SELF_MODE_BEGIN; mode < CAM_SHOT_SELF_MODE_END; mode++) {
			if (ad->launching_mode != CAM_LAUNCHING_MODE_NORMAL &&
				(mode == CAM_SELF_SINGLE_MODE)) {
				__cam_push_mode_into_array(mode, composer, capacity);
			} else if (ad->launching_mode == CAM_LAUNCHING_MODE_NORMAL) {
				__cam_push_mode_into_array(mode, composer, capacity);
			}
		}
	}
}

void cam_compose_menu_scene_mode(void *data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(15);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}

	eina_array_step_set(composer->array, sizeof(*(composer->array)), 20);

	unsigned int capacity = 0;
	GetCamDevSceneModeCaps(&capacity, data);

	if (capacity & CAM_CP_SCENE_MODE_AUTO) {
		eina_array_push(composer->array, (void*)&scene_mode[0]);
	}

	if (capacity & CAM_CP_SCENE_MODE_PORTRAIT) {
		eina_array_push(composer->array, (void*)&scene_mode[1]);
	}

	if (capacity & CAM_CP_SCENE_MODE_LANDSCAPE) {
		eina_array_push(composer->array, (void*)&scene_mode[2]);
	}

	if (capacity & CAM_CP_SCENE_MODE_NIGHTSHOT) {
		eina_array_push(composer->array, (void*)&scene_mode[3]);
	}

	if (capacity & CAM_CP_SCENE_MODE_SPORTS) {
		eina_array_push(composer->array, (void*)&scene_mode[4]);
	}

	if (capacity & CAM_CP_SCENE_MODE_PARTY_AND_INDOOR) {
		eina_array_push(composer->array, (void*)&scene_mode[5]);
	}

	if (capacity & CAM_CP_SCENE_MODE_BEACH_AND_SNOW) {
		eina_array_push(composer->array, (void*)&scene_mode[6]);
	}

	if (capacity & CAM_CP_SCENE_MODE_SUNSET) {
		eina_array_push(composer->array, (void*)&scene_mode[7]);
	}

	if (capacity & CAM_CP_SCENE_MODE_DUSK_AND_DAWN) {
		eina_array_push(composer->array, (void*)&scene_mode[8]);
	}

	if (capacity & CAM_CP_SCENE_MODE_FALLCOLOR) {
		eina_array_push(composer->array, (void*)&scene_mode[9]);
	}

	if (capacity & CAM_CP_SCENE_MODE_FIREWORK) {
		eina_array_push(composer->array, (void*)&scene_mode[10]);
	}

	if (capacity & CAM_CP_SCENE_MODE_TEXT) {
		eina_array_push(composer->array, (void*)&scene_mode[11]);
	}

	if (capacity & CAM_CP_SCENE_MODE_CANDLE_LIGHT) {
		eina_array_push(composer->array, (void*)&scene_mode[12]);
	}

	if (capacity & CAM_CP_SCENE_MODE_BACK_LIGHT) {
		eina_array_push(composer->array, (void*)&scene_mode[13]);
	}

	if (capacity & CAM_CP_SCENE_MODE_SHOW_WINDOW) {
		eina_array_push(composer->array, (void*)&scene_mode[14]);
	}
}

void cam_compose_menu_photo_resolution(void *data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	composer->array = eina_array_new(6);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 6);

	unsigned int capacity = 0;
	GetCamDevCamResolutionCaps(&capacity, data);

	if (camapp->self_portrait == FALSE) {
/*		if (capacity & CAM_CP_SIZE_3264X2448) {
			eina_array_push(composer->array, (void*)&photo_resolution[0]);
		}

		if (capacity & CAM_CP_SIZE_3264X1836) {
			eina_array_push(composer->array, (void*)&photo_resolution[1]);
		}

		if (capacity & CAM_CP_SIZE_2448X2448) {
			eina_array_push(composer->array, (void*)&photo_resolution[4]);
		}
*/
		if (capacity & CAM_CP_SIZE_2048X1536) {
			eina_array_push(composer->array, (void*)&photo_resolution[5]);
		}
/*
		if (capacity & CAM_CP_SIZE_2048X1232) {
			eina_array_push(composer->array, (void*)&photo_resolution[6]);
		}

		if (capacity & CAM_CP_SIZE_2048X1152) {
			eina_array_push(composer->array, (void*)&photo_resolution[7]);
		}

		if (capacity & CAM_CP_SIZE_1600X1200) {
			eina_array_push(composer->array, (void*)&photo_resolution[9]);
		}
*/
		if (capacity & CAM_CP_SIZE_640X480) {
			eina_array_push(composer->array, (void*)&photo_resolution[16]);
		}
	}  else {
		if (capacity & CAM_CP_SIZE_640X480) {
			eina_array_push(composer->array, (void*)&photo_resolution[16]);
		}
	}

	int size = eina_array_count(composer->array);
	cam_debug(LOG_MM, "size %d capacity 0x%x",size, capacity);
}

void cam_compose_menu_video_resolution(void *data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	composer->array = eina_array_new(5);
	if (!(composer->array))	{
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 5);

	unsigned int capacity = 0;
	GetCamDevRecResolutionCaps(&capacity, data);

	if (camapp->self_portrait == FALSE) {
/*		if (capacity & CAM_CP_SIZE_1920X1080) {
			eina_array_push(composer->array, (void*)&video_resolution[0]);
		}

		if (capacity & CAM_CP_SIZE_1440X1080) {
			eina_array_push(composer->array, (void*)&video_resolution[1]);
		}

		if (capacity & CAM_CP_SIZE_1080X1080) {
			eina_array_push(composer->array, (void*)&video_resolution[3]);
		}

		if (capacity & CAM_CP_SIZE_1280X720) {
			eina_array_push(composer->array, (void*)&video_resolution[4]);
		}

		if (capacity & CAM_CP_SIZE_800X480) {
			eina_array_push(composer->array, (void*)&video_resolution[5]);
		}
*/
		if (capacity & CAM_CP_SIZE_640X480) {
			eina_array_push(composer->array, (void*)&video_resolution[6]);
		}

		if (capacity & CAM_CP_SIZE_320X240) {
			eina_array_push(composer->array, (void*)&video_resolution[7]);
		}
	} else {
/*		if (capacity & CAM_CP_SIZE_1920X1080) {
			eina_array_push(composer->array, (void*)&video_resolution[0]);
		}

		if (capacity & CAM_CP_SIZE_1440X1080) {
			eina_array_push(composer->array, (void*)&video_resolution[1]);
		}

		if (capacity & CAM_CP_SIZE_1080X1080) {
			eina_array_push(composer->array, (void*)&video_resolution[2]);
		}

		if (capacity & CAM_CP_SIZE_1280X720) {
			eina_array_push(composer->array, (void*)&video_resolution[4]);
		}
*/
		if (capacity & CAM_CP_SIZE_640X480) {
			eina_array_push(composer->array, (void*)&video_resolution[6]);
		}

		if (capacity & CAM_CP_SIZE_320X240) {
			eina_array_push(composer->array, (void*)&video_resolution[7]);
		}
	}

	int size = eina_array_count(composer->array);
	cam_warning(LOG_MM, "size %d ",size );
}

void cam_compose_menu_effect(void *data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");
	unsigned int loop = 0;
	/*the array effect's size may change in the future, so calculate a dynamic value.*/
	composer->array = eina_array_new(sizeof(effect)/sizeof(CAM_MENU_ITEM));
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	/*set Eina_array full enlarge size a little bigger. from the old value 5 to the new value 15.*/
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 15);


	unsigned int capacity = 1;
//	unsigned int effect_mask = 0;
//	GetCamDevEffectCaps(&capacity, data);

	for (loop = 0; loop < sizeof(effect)/sizeof(CAM_MENU_ITEM); loop++)
	{
//		effect_mask = cam_effect_cam_convert_caps(cam_convert_menu_item_to_setting_value(effect[loop]));

#ifdef CAMERA_MACHINE_I686
		capacity = 0;
#endif
		if (capacity) {
			eina_array_push(composer->array, (void*)&effect[loop]);
		}
	}

	cam_debug(LOG_CAM, "capacity 0x%x", capacity);

	return;
}

void cam_compose_menu_wb(void *data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(5);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 10);

	unsigned int capacity = 0;
	GetCamDevWBCaps(&capacity, data);

	if (capacity & CAM_CP_WB_AUTO) {
		eina_array_push(composer->array, (void*)&wb[0]);
	}

	if (capacity & CAM_CP_WB_DAYLIGHT) {
		eina_array_push(composer->array, (void*)&wb[1]);
	}

	if (capacity & CAM_CP_WB_CLOUDY) {
		eina_array_push(composer->array, (void*)&wb[2]);
	}

	if (capacity & CAM_CP_WB_INCANDESCENT) {
		eina_array_push(composer->array, (void*)&wb[3]);
	}

	if (capacity & CAM_CP_WB_FLUORESCENT) {
		eina_array_push(composer->array, (void*)&wb[4]);
	}
}

void cam_compose_menu_focus_mode(void *data, cam_menu_composer* composer)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(5);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 10);

	unsigned int capacity = 0;
	GetCamDevFocusCaps(&capacity, data);

	if (capacity & CAM_CP_FOCUS_AUTO_FOCUS) {
		eina_array_push(composer->array, (void*)&focus_mode[0]);
	}

	if (capacity & CAM_CP_FOCUS_MACRO) {
		eina_array_push(composer->array, (void*)&focus_mode[1]);
	}

	if (!(ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL
		&& ad->exe_args->cam_mode == CAM_CAMCORDER_MODE)) {
		if (capacity & CAM_CP_FOCUS_FACE_DETECTION) {
			eina_array_push(composer->array, (void*)&focus_mode[2]);
		}
	}
}

void cam_compose_menu_iso(void *data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(10);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 15);

	unsigned int capacity = 0;
	GetCamDevISOCaps(&capacity, data);

	if (capacity & CAM_CP_ISO_AUTO) {
		eina_array_push(composer->array, (void*)&iso[0]);
	}

	if (capacity & CAM_CP_ISO_50) {
		eina_array_push(composer->array, (void*)&iso[1]);
	}

	if (capacity & CAM_CP_ISO_100) {
		eina_array_push(composer->array, (void*)&iso[2]);
	}

	if (capacity & CAM_CP_ISO_200) {
		eina_array_push(composer->array, (void*)&iso[3]);
	}

	if (capacity & CAM_CP_ISO_400) {
		eina_array_push(composer->array, (void*)&iso[4]);
	}

	if (capacity & CAM_CP_ISO_800) {
		eina_array_push(composer->array, (void*)&iso[5]);
	}

}

void cam_compose_menu_metering(void *data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(5);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 10);

	unsigned int capacity = 0;
	GetCamDevMeteringCaps(&capacity, data);

	if (capacity & CAM_CP_METERING_CENTERWEIGHT) {
		eina_array_push(composer->array, (void*)&metering[0]);
	}

	if (capacity & CAM_CP_METERING_MATRIX) {
		eina_array_push(composer->array, (void*)&metering[1]);
	}

	if (capacity & CAM_CP_METERING_SPOT) {
		eina_array_push(composer->array, (void*)&metering[2]);
	}

}

void cam_compose_menu_recording_mode(void *data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	composer->array = eina_array_new(5);
	if (!(composer->array))	{
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 10);

	unsigned int capacity = 0;
	GetCamDevRecModeCaps(&capacity, NULL);

	if (camapp->self_portrait == FALSE) {
		if (capacity & CAM_CP_REC_MODE_NORMAL) {
			eina_array_push(composer->array, (void*)&recording_mode[0]);
		}

		if (capacity & CAM_CP_REC_MODE_MMS) {
			eina_array_push(composer->array, (void*)&recording_mode[1]);
		}
	} else {
		if (capacity & CAM_CP_REC_MODE_NORMAL) {
			eina_array_push(composer->array, (void*)&recording_mode[4]);
		}

		if (capacity & CAM_CP_REC_MODE_MMS) {
			eina_array_push(composer->array, (void*)&recording_mode[5]);
		}
	}
}


void cam_compose_menu_flash(void *data, cam_menu_composer* composer)
{
	struct appdata *ad = (struct appdata *)data;
	cam_ret_if(ad == NULL);

	CamAppData *camapp = NULL;
	camapp = ad->camapp_handle;
	cam_retm_if(camapp == NULL, "camapp_handle is NULL");

	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(5);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 10);

	unsigned int capacity = 0;
	GetCamDevFlashCaps(&capacity, data);

	if (capacity & CAM_CP_FLASH_OFF) {
		eina_array_push(composer->array, (void*)&flash[0]);
	}

	if (capacity & CAM_CP_FLASH_ON) {
		eina_array_push(composer->array, (void*)&flash[1]);
	}

	if(camapp->camera_mode == CAM_CAMERA_MODE) {
		if (capacity & CAM_CP_FLASH_AUTO) {
			eina_array_push(composer->array, (void*)&flash[2]);
		}
	}

}

void cam_compose_menu_timer_mode(void *data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(4);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 8);

	eina_array_push(composer->array, (void*)&timer[0]);
	eina_array_push(composer->array, (void*)&timer[1]);
	eina_array_push(composer->array, (void*)&timer[2]);
}

void cam_compose_menu_shutter_sound(void* data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(2);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 4);

	eina_array_push(composer->array, (void*)&shutter_sound[0]);
	eina_array_push(composer->array, (void*)&shutter_sound[1]);
}

void cam_compose_menu_storage(void* data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(2);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 4);

	eina_array_push(composer->array, (void*)&storage[0]);
	eina_array_push(composer->array, (void*)&storage[1]);
}

void cam_compose_menu_volume_key(void* data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");
	struct appdata *ad = (struct appdata *)data;
	cam_retm_if(ad == NULL, "appdata is NULL");

	composer->array = eina_array_new(3);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 2);

	eina_array_push(composer->array, (void*)&volume_key[0]);

	if (ad->launching_mode == CAM_LAUNCHING_MODE_NORMAL) {
		eina_array_push(composer->array, (void*)&volume_key[1]);
		eina_array_push(composer->array, (void*)&volume_key[2]);
	} else {
		if (ad->exe_args->allow_switch == TRUE) {
			eina_array_push(composer->array, (void*)&volume_key[1]);
			eina_array_push(composer->array, (void*)&volume_key[2]);
		} else {
			if (ad->exe_args->cam_mode == CAM_CAMCORDER_MODE) {
				eina_array_push(composer->array, (void*)&volume_key[2]);
			} else if (ad->exe_args->cam_mode == CAM_CAMERA_MODE) {
				eina_array_push(composer->array, (void*)&volume_key[1]);
			}
		}
	}
}

void cam_compose_menu_share(void* data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(2);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 2);

	eina_array_push(composer->array, (void*)&share[0]);
	eina_array_push(composer->array, (void*)&share[1]);
}

void cam_compose_menu_face_detection(void* data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(2);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 2);

	eina_array_push(composer->array, (void*)&face_detection[0]);
	eina_array_push(composer->array, (void*)&face_detection[1]);
}

void cam_compose_menu_fast_motion(void* data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(3);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 2);

	eina_array_push(composer->array, (void*)&fast_motion[0]);
	eina_array_push(composer->array, (void*)&fast_motion[1]);
	eina_array_push(composer->array, (void*)&fast_motion[2]);
}

void cam_compose_menu_slow_motion(void* data, cam_menu_composer* composer)
{
	cam_retm_if(composer == NULL, "composer is NULL");

	composer->array = eina_array_new(3);
	if (!(composer->array)) {
		cam_critical(LOG_CAM, "could not create new eina array");
		return;
	}
	eina_array_step_set(composer->array, sizeof(*(composer->array)), 2);

	eina_array_push(composer->array, (void*)&slow_motion[0]);
	eina_array_push(composer->array, (void*)&slow_motion[1]);
	eina_array_push(composer->array, (void*)&slow_motion[2]);
}

void cam_compose_free(cam_menu_composer* composer)
{
	cam_ret_if(composer == NULL);

	if (composer->array) {
		eina_array_free(composer->array);
	}

	IF_FREE(composer);
}

gboolean cam_is_enabled_menu(void *data, const CAM_MENU_ITEM item)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, FALSE);

	switch (item) {
		case CAM_MENU_SHORTCUTS:
			return TRUE;
		case CAM_MENU_SELF_PORTRAIT:
			return __is_enabled_self_mode((void*)ad);
		case CAM_MENU_FLASH:
			return __is_enabled_flash((void*)ad);
		case CAM_MENU_TAP_SHOT:
			return __is_enabled_tap_shot_shot((void*)ad);
		case CAM_MENU_SHOOTING_MODE:
			return __is_enabled_shooting_mode((void*)ad);
		case CAM_MENU_RECORDING_MODE:
			return __is_enabled_recording_mode((void*)ad);
		case CAM_MENU_SCENE_MODE:
			return __is_enabled_scene_mode((void*)ad);
		case CAM_MENU_EXPOSURE_VALUE:
			return __is_enabled_exposure_value((void*)ad);
		case CAM_MENU_FOCUS_MODE:
			return __is_enabled_focus_mode((void*)ad);
		case CAM_MENU_TIMER:
			return __is_enabled_timer((void*)ad);
		case CAM_MENU_EFFECTS:
			return __is_enabled_effects((void*)ad);
		case CAM_MENU_PHOTO_RESOLUTION:
			return __is_enabled_photo_resolution((void*)ad);
		case CAM_MENU_VIDEO_RESOLUTION:
			return __is_enabled_video_resolution((void*)ad);
		case CAM_MENU_WHITE_BALANCE:
			return __is_enabled_white_balance((void*)ad);
		case CAM_MENU_ISO:
			return __is_enabled_iso((void*)ad);
		case CAM_MENU_METERING:
			return __is_enabled_metering((void*)ad);
		case CAM_MENU_ANTI_SHAKE:
			return __is_enabled_anti_shake((void*)ad);
		case CAM_MENU_VIDEO_STABILIZATION:
			return __is_enabled_video_stabilization((void*)ad);
		case CAM_MENU_AUTO_CONTRAST:
			return __is_enabled_auto_contrast((void*)ad);
		case CAM_MENU_SAVE_AS_FLIP:
			return __is_enabled_save_as_flip((void*)ad);
		case CAM_MENU_GPS_TAG:
			return __is_enabled_gps_tag((void*)ad);
		case CAM_MENU_SHUTTER_SOUND:
			return __is_enabled_shutter_sound((void*)ad);
		case CAM_MENU_STORAGE:
			return __is_enabled_storage((void*)ad);
		case CAM_MENU_SHOTS:
			return TRUE;
		case CAM_MENU_RESET:
			return __is_enabled_reset((void*)ad);
		case CAM_MENU_REVIEW:
			return __is_enabled_review((void*)ad);
		case CAM_MENU_SHARE:
			return __is_enabled_buddy_photo_share((void*)ad);
		case CAM_MENU_SHARE_BUDDY_PHOTO:
			return __is_enabled_buddy_photo_share((void*)ad);
		case CAM_MENU_FACE_DETECTION:
			return __is_enabled_face_detection((void*)ad);
		case CAM_MENU_VOLUME_KEY:
			return __is_enabled_volume_key((void*)ad);
		default:
			cam_debug(LOG_UI, "invalid item [%d]", item);
			break;
		}

	return TRUE;

}

static gboolean __is_enabled_flash(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, ad);

	if (!(capacity & CAM_CP_FUNC_FLASH_MODE)) {
		return FALSE;
	}

	if (!(camapp->scene_mode == CAM_SCENE_NONE
		|| camapp->scene_mode == CAM_SCENE_PORTRAIT
		|| camapp->scene_mode == CAM_SCENE_BACKLIGHT
		|| camapp->scene_mode == CAM_SCENE_PARTY
		|| camapp->scene_mode == CAM_SCENE_TEXT) ) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->flash.enable) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_shooting_mode(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		return FALSE;
	}

	if (camapp->share == CAM_SHARE_BUDDY_PHOTO) {
		return FALSE;
	}

	if (ad->secure_mode == TRUE) {
		return FALSE;
	}

	if (camapp->self_portrait) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_recording_mode(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL
		&& ad->exe_args->allow_switch == FALSE) {
		return FALSE;
	}

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL
		&& ad->exe_args->resolution == CAM_RESOLUTION_QCIF) {
		return FALSE;
	}

	if (camapp->effect != CAM_SETTINGS_EFFECTS_NOR) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_scene_mode(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if (!(capacity & CAM_CP_FUNC_SCENE_MODE)) {
		return FALSE;
	}

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL
		&& ad->exe_args->allow_switch == FALSE
		&& ad->exe_args->cam_mode == CAM_CAMCORDER_MODE) {
		return FALSE;
	}

	if (camapp->camera_mode == CAM_CAMERA_MODE &&
		(camapp->shooting_mode == CAM_PX_MODE)) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_exposure_value(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if (!(capacity & CAM_CP_FUNC_EXPOSURE)) {
		return FALSE;
	}

	if (camapp->scene_mode != CAM_SCENE_NONE) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->exposure_value.enable) {
		return FALSE;
	}

	if (camapp->auto_contrast == TRUE) {
		return FALSE;
	}

	if (camapp->effect != CAM_SETTINGS_EFFECTS_NOR) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_focus_mode(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if(!(capacity & CAM_CP_FUNC_FOCUS_MODE)) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_timer(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->timer.enable) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_effects(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

#ifdef CAMERA_MACHINE_I686
		capacity = 0;
#endif

	if (!(capacity & CAM_CP_FUNC_SUPPORT)) {
		return FALSE;
	}

	if (camapp->scene_mode != CAM_SCENE_NONE) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->effect.enable) {
		return FALSE;
	}

	if (camapp->recording_mode != CAM_RECORD_NORMAL
			&& camapp->recording_mode != CAM_RECORD_SELF) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_photo_resolution(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->photo_resolution.enable) {
		return FALSE;
	}

	if(ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL
		&& ad->exe_args->allow_switch == FALSE
		&& ad->exe_args->cam_mode == CAM_CAMCORDER_MODE) {
		return FALSE;
	}

	if (camapp->self_portrait == TRUE) {
		return TRUE;
	}

	return TRUE;
}

static gboolean __is_enabled_video_resolution(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if(camapp->recording_mode == CAM_RECORD_MMS
		|| camapp->recording_mode == CAM_RECORD_SELF_MMS
		|| camapp->recording_mode == CAM_RECORD_SLOW) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->video_resolution.enable) {
		return FALSE;
	}


	if(ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL
		&& ad->exe_args->allow_switch == FALSE
		&& ad->exe_args->cam_mode == CAM_CAMERA_MODE) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_white_balance(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if(!(capacity & CAM_CP_FUNC_WHITE_BALANCE)) {
		return FALSE;
	}

	if(camapp->scene_mode != CAM_SCENE_NONE) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->wb.enable) {
		return FALSE;
	}

	if (camapp->effect != CAM_SETTINGS_EFFECTS_NOR) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_iso(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if (!(capacity & CAM_CP_FUNC_ISO)) {
		return FALSE;
	}

	if (camapp->scene_mode != CAM_SCENE_NONE) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->iso.enable) {
		return FALSE;
	}

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL
		&& ad->exe_args->cam_mode == CAM_CAMCORDER_MODE) {
		return FALSE;
	}

	if (camapp->effect != CAM_SETTINGS_EFFECTS_NOR) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_metering(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if(!(capacity & CAM_CP_FUNC_METERING)) {
		return FALSE;
	}

	if(camapp->scene_mode != CAM_SCENE_NONE) {
		return FALSE;
	}

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL
		&& ad->exe_args->cam_mode == CAM_CAMCORDER_MODE) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->metering.enable) {
		return FALSE;
	}

	if (camapp->effect != CAM_SETTINGS_EFFECTS_NOR) {
		return FALSE;
	}

	return TRUE;

}

static gboolean __is_enabled_anti_shake(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if (!(capacity & CAM_CP_FUNC_CAM_ANS)) {
		return FALSE;
	}

	if (camapp->scene_mode != CAM_SCENE_NONE) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->ahs.enable) {
		return FALSE;
	}

	if (camapp->iso != CAM_ISO_AUTO) {
		return FALSE;
	}

	if (camapp->effect != CAM_SETTINGS_EFFECTS_NOR) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_video_stabilization(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		return FALSE;
	}

	if (!(capacity & CAM_CP_FUNC_REC_ANS)) {
		return FALSE;
	}

	if (camapp->recording_mode != CAM_RECORD_NORMAL) {
		return FALSE;
	}

	if (camapp->video_resolution < CAM_RESOLUTION_HD) {
		return FALSE;
	}

	if (camapp->effect != CAM_SETTINGS_EFFECTS_NOR) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_auto_contrast(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if(!(capacity & CAM_CP_FUNC_AUTO_CONTRAST)) {
		return FALSE;
	}

	if(camapp->scene_mode != CAM_SCENE_NONE) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_self_mode(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, FALSE);
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if (!(capacity & CAM_CP_FUNC_SELF_MODE)) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_tap_shot_shot(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, FALSE);
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if (!(capacity & CAM_CP_FUNC_TAP_SHOT)) {
		return FALSE;
	}

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL
		&& ad->exe_args->allow_switch == FALSE
		&& ad->exe_args->cam_mode == CAM_CAMCORDER_MODE) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->tap_shot.enable) {
		return FALSE;
	}

	return TRUE;
}


static gboolean __is_enabled_save_as_flip(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, FALSE);
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if (!(capacity & CAM_CP_FUNC_SAVE_AS_FLIP)) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->save_as_flip.enable) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_shutter_sound(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, FALSE);
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	return TRUE;
}

static gboolean __is_enabled_gps_tag(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, FALSE);
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	unsigned int capacity;
	GetCamDevFuncCaps(&capacity, (void*)ad);

	if (!(capacity & CAM_CP_FUNC_GPS)) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->gps.enable) {
		return FALSE;
	}

	if (ad->secure_mode == TRUE) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_storage(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retv_if(ad == NULL, FALSE);
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->storage.enable) {
		return FALSE;
	}

	storage_state_e mmc_state;
	int error_code = storage_get_state(ad->externalstorageId, &mmc_state);
	if (error_code != STORAGE_ERROR_NONE) {
		cam_debug(LOG_CAM, "Get storage state failed");
		return FALSE;
	} else {
		if (mmc_state == STORAGE_STATE_MOUNTED) {
			return TRUE;
		}
	}

	return FALSE;
}

static gboolean __is_enabled_review(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		return FALSE;
	}

	if (camapp->share == CAM_SHARE_BUDDY_PHOTO) {
		return FALSE;
	}

	if (camapp->effect != CAM_SETTINGS_EFFECTS_NOR) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->review.enable) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_buddy_photo_share(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		return FALSE;
	}

	if (!(camapp->camera_mode == CAM_CAMERA_MODE &&
		(camapp->shooting_mode == CAM_SINGLE_MODE
		|| camapp->shooting_mode == CAM_SELF_SINGLE_MODE))) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_face_detection(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (camapp->effect != CAM_SETTINGS_EFFECTS_NOR) {
		return FALSE;
	}

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->face_detection.enable) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_volume_key(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	CAM_COMMON_PROPERTY *mode_property = cam_get_shooting_mode_property(camapp->shooting_mode);
	if (mode_property != NULL && !mode_property->volume_key.enable) {
		return FALSE;
	}

	return TRUE;
}

static gboolean __is_enabled_reset(void* data)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, FALSE, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, FALSE, "camapp_handle is NULL");

	if (ad->launching_mode == CAM_LAUNCHING_MODE_EXTERNAL) {
		return FALSE;
	}

	return TRUE;
}

void cam_compose_list_get(CamSettingTabType tab_type, CAM_MENU_ITEM** list, int* list_num)
{
	CAM_MENU_ITEM* ret_list = NULL;
	int list_cnt = 0;

	switch (tab_type) {
	case CAM_SETTING_REAR:
		ret_list = (CAM_MENU_ITEM*)setting_rear;
		list_cnt = sizeof(setting_rear)/sizeof(CAM_MENU_ITEM);
		break;
	case CAM_SETTING_FRONT:
		ret_list = (CAM_MENU_ITEM*)setting_front;
		list_cnt = sizeof(setting_front)/sizeof(CAM_MENU_ITEM);
		break;
	default:
		ret_list = NULL;
		list_cnt = 0;
		cam_debug(LOG_CAM, "Get tab_type %d is failed", tab_type);
		break;
	}

	*list = ret_list;
	*list_num = list_cnt;

	return;
}


