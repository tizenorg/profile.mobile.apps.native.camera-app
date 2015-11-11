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


#include "cam_menu_item.h"
#include "cam_menu_composer.h"
#include "edc_image_name.h"
#include "cam_config.h"

int cam_convert_menu_item_to_setting_value(int mode)
{
	switch (mode) {
	case CAM_MENU_SCENE_AUTO:
		return 	CAM_SCENE_NONE;
	case CAM_MENU_SCENE_PORTRAIT:
		return 	CAM_SCENE_PORTRAIT;
	case CAM_MENU_SCENE_LANDSCAPE:
		return 	CAM_SCENE_LANDSCAPE;
	case CAM_MENU_SCENE_NIGHT:
		return CAM_SCENE_NIGHT;
	case CAM_MENU_SCENE_SPORTS:
		return CAM_SCENE_SPORTS;
	case CAM_MENU_SCENE_PARTY:
		return CAM_SCENE_PARTY;
	case CAM_MENU_SCENE_BEACHSNOW:
		return CAM_SCENE_BEACHSNOW;
	case CAM_MENU_SCENE_SUNSET:
		return CAM_SCENE_SUNSET;
	case CAM_MENU_SCENE_DUSKDAWN:
		return CAM_SCENE_DUSKDAWN;
	case CAM_MENU_SCENE_FALL:
		return CAM_SCENE_FALL;
	case CAM_MENU_SCENE_FIREWORK:
		return CAM_SCENE_FIREWORK;
	case CAM_MENU_SCENE_TEXT:
		return CAM_SCENE_TEXT;
	case CAM_MENU_SHARE_BUDDY_PHOTO:
		return CAM_SHARE_BUDDY_PHOTO;
	case CAM_MENU_SCENE_CANDLELIGHT:
		return CAM_SCENE_CANDLELIGHT;
	case CAM_MENU_SCENE_BACKLIGHT:
		return CAM_SCENE_BACKLIGHT;
	case CAM_MENU_PHOTO_RESOLUTION_3264x2448:
		return CAM_RESOLUTION_3264x2448;
	case CAM_MENU_PHOTO_RESOLUTION_3264x1836:
		return CAM_RESOLUTION_3264x1836;
	case CAM_MENU_PHOTO_RESOLUTION_2560x1920:
		return CAM_RESOLUTION_2560x1920;
	case CAM_MENU_PHOTO_RESOLUTION_2560x1440:
		return CAM_RESOLUTION_2560x1440;
	case CAM_MENU_PHOTO_RESOLUTION_2448x2448:
		return CAM_RESOLUTION_2448x2448;
	case CAM_MENU_PHOTO_RESOLUTION_2048x1536:
		return CAM_RESOLUTION_2048x1536;
	case CAM_MENU_PHOTO_RESOLUTION_2048x1232:
		return CAM_RESOLUTION_2048x1232;
	case CAM_MENU_PHOTO_RESOLUTION_2048x1152:
		return CAM_RESOLUTION_2048x1152;
	case CAM_MENU_PHOTO_RESOLUTION_1920x1080:
		return CAM_RESOLUTION_1920x1080;
	case CAM_MENU_PHOTO_RESOLUTION_1600x1200:
		return CAM_RESOLUTION_1600x1200;
	case CAM_MENU_PHOTO_RESOLUTION_1600x960:
		return CAM_RESOLUTION_1600x960;
	case CAM_MENU_PHOTO_RESOLUTION_1440x1080:
		return CAM_RESOLUTION_1440x1080;
	case CAM_MENU_PHOTO_RESOLUTION_1392x1392:
		return CAM_RESOLUTION_1392x1392;
	case CAM_MENU_PHOTO_RESOLUTION_1280x960:
		return CAM_RESOLUTION_1280x960;
	case CAM_MENU_PHOTO_RESOLUTION_1280x720:
		return CAM_RESOLUTION_1280x720;
	case CAM_MENU_PHOTO_RESOLUTION_1056x1056:
		return CAM_RESOLUTION_1056x1056;
	case CAM_MENU_PHOTO_RESOLUTION_800x480:
		return CAM_RESOLUTION_800x480;
	case CAM_MENU_PHOTO_RESOLUTION_800x450:
		return CAM_RESOLUTION_800x450;
	case CAM_MENU_PHOTO_RESOLUTION_720x480:
		return CAM_RESOLUTION_WVGA2;
	case CAM_MENU_PHOTO_RESOLUTION_640x480:
		return CAM_RESOLUTION_VGA;
	case CAM_MENU_FAST_MOTION_X2:
		return CAM_FAST_MOTION_X2;
	case CAM_MENU_FAST_MOTION_X4:
		return CAM_FAST_MOTION_X4;
	case CAM_MENU_FAST_MOTION_X8:
		return CAM_FAST_MOTION_X8;
	case CAM_MENU_SLOW_MOTION_X2:
		return CAM_SLOW_MOTION_X2;
	case CAM_MENU_SLOW_MOTION_X4:
		return CAM_SLOW_MOTION_X4;
	case CAM_MENU_SLOW_MOTION_X8:
		return CAM_SLOW_MOTION_X8;
	case CAM_MENU_TIMER_OFF:
		return CAM_SETTINGS_TIMER_OFF;
	case CAM_MENU_TIMER_3SEC:
		return CAM_SETTINGS_TIMER_3SEC;
	case CAM_MENU_TIMER_10SEC:
		return CAM_SETTINGS_TIMER_10SEC;
	case CAM_MENU_VIDEO_RESOLUTION_FULLHD:
		return CAM_RESOLUTION_FULLHD;
	case CAM_MENU_VIDEO_RESOLUTION_HD:
		return CAM_RESOLUTION_HD;
	case CAM_MENU_VIDEO_RESOLUTION_WVGA:
		return CAM_RESOLUTION_WVGA;
	case CAM_MENU_VIDEO_RESOLUTION_VGA:
		return CAM_RESOLUTION_VGA;
	case CAM_MENU_VIDEO_RESOLUTION_QVGA:
		return CAM_RESOLUTION_QVGA;
	case CAM_MENU_VIDEO_RESOLUTION_QCIF:
		return CAM_RESOLUTION_QCIF;
	case CAM_MENU_VIDEO_RESOLUTION_1440x1080:
		return CAM_RESOLUTION_1440x1080;
	case CAM_MENU_VIDEO_RESOLUTION_1080x1080:
		return CAM_RESOLUTION_1080x1080;
	case CAM_MENU_VIDEO_RESOLUTION_1056x1056:
		return CAM_RESOLUTION_1056x1056;
	case CAM_MENU_WB_AWB:
		return CAM_SETTINGS_WB_AWB;
	case CAM_MENU_WB_INCANDESCENT:
		return CAM_SETTINGS_WB_INCANDESCENT;
	case CAM_MENU_WB_FLUORESCENT:
		return CAM_SETTINGS_WB_FLUORESCENT;
	case CAM_MENU_WB_DAYLIGHT:
		return CAM_SETTINGS_WB_DAYLIGHT;
	case CAM_MENU_WB_CLOUDY:
		return CAM_SETTINGS_WB_CLOUDY;
	case CAM_MENU_EFFECT_NONE:
		return CAM_SETTINGS_EFFECTS_NOR;
	case CAM_MENU_EFFECT_SEPIA:
		return CAM_SETTINGS_EFFECTS_SEPIA;
	case CAM_MENU_EFFECT_GREY:
		return CAM_SETTINGS_EFFECTS_BLACKWHITE;
	case CAM_MENU_EFFECT_NEGATIVE:
		return CAM_SETTINGS_EFFECTS_NEGATIVE;
	case CAM_MENU_FOCUS_AUTO:
		return CAM_FOCUS_AUTO;
	case CAM_MENU_FOCUS_MACRO:
		return CAM_FOCUS_MACRO;
	case CAM_MENU_FOCUS_FACE_DETECTION:
		return CAM_FOCUS_FACE;
	case CAM_MENU_RECORDING_MODE_NORMAL:
		return CAM_RECORD_NORMAL;
	case CAM_MENU_RECORDING_MODE_LIMIT:
		return CAM_RECORD_MMS;
	case CAM_MENU_RECORDING_MODE_SLOW:
		return CAM_RECORD_SLOW;
	case CAM_MENU_RECORDING_MODE_FAST:
		return CAM_RECORD_FAST;
	case CAM_MENU_RECORDING_MODE_SELF_NORMAL:
		return CAM_RECORD_SELF;
	case CAM_MENU_RECORDING_MODE_SELF_LIMIT:
		return CAM_RECORD_SELF_MMS;
	case CAM_MENU_FLASH_OFF:
		return CAM_FLASH_OFF;
	case CAM_MENU_FLASH_ON:
		return CAM_FLASH_ON;
	case CAM_MENU_FLASH_AUTO:
		return CAM_FLASH_AUTO;
	case CAM_MENU_METERING_MATRIX:
		return CAM_METERING_MATRIX;
	case CAM_MENU_METERING_CENTER_WEIGHTED:
		return CAM_METERING_CENTER_WEIGHTED;
	case CAM_MENU_METERING_SPOT:
		return CAM_METERING_SPOT;
	case CAM_MENU_ISO_AUTO:
		return CAM_ISO_AUTO;
	case CAM_MENU_ISO_50:
		return CAM_ISO_50;
	case CAM_MENU_ISO_100:
		return CAM_ISO_100;
	case CAM_MENU_ISO_200:
		return CAM_ISO_200;
	case CAM_MENU_ISO_400:
		return CAM_ISO_400;
	case CAM_MENU_ISO_80O:
		return CAM_ISO_800;
	case CAM_MENU_ISO_1600:
		return CAM_ISO_1600;
	case CAM_MENU_ISO_320O:
		return CAM_ISO_3200;
	case CAM_MENU_VOLUME_KEY_ZOOM:
		return CAM_VOLUME_KEY_ZOOM;
	case CAM_MENU_VOLUME_KEY_CAMERA:
		return CAM_VOLUME_KEY_CAMERA;
	case CAM_MENU_VOLUME_KEY_RECORD:
		return CAM_VOLUME_KEY_RECORD;
	case CAM_MENU_FACE_DETECTION_ON:
		return CAM_FACE_DETECTION_ON;
	case CAM_MENU_FACE_DETECTION_OFF:
		return CAM_FACE_DETECTION_OFF;
	case CAM_MENU_TAP_SHOT_ON:
		return TRUE;
	case CAM_MENU_TAP_SHOT_OFF:
		return FALSE;
	case CAM_MENU_GPS_TAG_ON:
		return TRUE;
	case CAM_MENU_GPS_TAG_OFF:
		return FALSE;
	default:
		return -1;
	}

	return -1;
}

int cam_convert_menu_shooting_mode_to_setting_value(int mode)
{
	int shot_mode = CAM_SHOT_MODE_MIN + 1;
	CAM_COMMON_DATA *mode_data = NULL;
	for (shot_mode = (CAM_SHOT_MODE_MIN + 1); shot_mode < CAM_SHOT_MODE_NUM; shot_mode ++) {
		mode_data = cam_get_shooting_mode(shot_mode);
		if (mode_data != NULL && mode_data->shot_property.property_index == mode) {
			return mode_data->shot_index;
		}
	}
	return -1;
}

int cam_convert_shot_mode_to_menu_item(int mode)
{
	CAM_COMMON_PROPERTY * mode_property = NULL;
	mode_property = cam_get_shooting_mode_property(mode);
	if (mode_property != NULL) {
		return mode_property->property_index;
	} else {
		return CAM_MENU_EMPTY;
	}
}

int cam_convert_flash_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_FLASH_OFF:
		return CAM_MENU_FLASH_OFF;
	case CAM_FLASH_ON:
		return CAM_MENU_FLASH_ON;
	case CAM_FLASH_AUTO:
		return CAM_MENU_FLASH_AUTO;
	default:
		return CAM_MENU_FLASH_OFF;
	}
}

int cam_convert_video_stabilization_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_VIDEO_STABILIZATION_OFF:
		return CAM_MENU_VIDEO_STABILIZATION_OFF;
	case CAM_VIDEO_STABILIZATION_ON:
		return CAM_MENU_VIDEO_STABILIZATION_ON;
	default:
		return CAM_MENU_VIDEO_STABILIZATION_OFF;
	}
}

int cam_convert_recording_mode_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_RECORD_NORMAL:
		return CAM_MENU_RECORDING_MODE_NORMAL;
	case CAM_RECORD_SELF:
		return CAM_MENU_RECORDING_MODE_SELF_NORMAL;
	case CAM_RECORD_MMS:
		return CAM_MENU_RECORDING_MODE_LIMIT;
	case CAM_RECORD_SELF_MMS:
		return CAM_MENU_RECORDING_MODE_SELF_LIMIT;
	case CAM_RECORD_SLOW:
		return CAM_MENU_RECORDING_MODE_SLOW;
	case CAM_RECORD_FAST:
		return CAM_MENU_RECORDING_MODE_FAST;
	default:
		return CAM_MENU_RECORDING_MODE_NORMAL;
	}
}

int cam_convert_video_resolution_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_RESOLUTION_FULLHD:
		return CAM_MENU_VIDEO_RESOLUTION_FULLHD;
	case CAM_RESOLUTION_HD:
		return CAM_MENU_VIDEO_RESOLUTION_HD;
	case CAM_RESOLUTION_WVGA:
		return CAM_MENU_VIDEO_RESOLUTION_WVGA;
	case CAM_RESOLUTION_VGA:
		return CAM_MENU_VIDEO_RESOLUTION_VGA;
	case CAM_RESOLUTION_QVGA:
		return CAM_MENU_VIDEO_RESOLUTION_QVGA;
	case CAM_RESOLUTION_QCIF:
		return CAM_MENU_VIDEO_RESOLUTION_QCIF;
	case CAM_RESOLUTION_1440x1080:
		return CAM_MENU_VIDEO_RESOLUTION_1440x1080;
	case CAM_RESOLUTION_1056x1056:
		return CAM_MENU_VIDEO_RESOLUTION_1056x1056;
	case CAM_RESOLUTION_1080x1080:
		return CAM_MENU_VIDEO_RESOLUTION_1080x1080;
	case CAM_RESOLUTION_800x450:
		return CAM_MENU_VIDEO_RESOLUTION_800x450;
	default:
		return CAM_MENU_VIDEO_RESOLUTION_QCIF;
	}
}

int cam_convert_photo_resolution_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_RESOLUTION_3264x2448:
		return CAM_MENU_PHOTO_RESOLUTION_3264x2448;
	case CAM_RESOLUTION_3264x1836:
		return CAM_MENU_PHOTO_RESOLUTION_3264x1836;
	case CAM_RESOLUTION_2560x1440:
		return CAM_MENU_PHOTO_RESOLUTION_2560x1440;
	case CAM_RESOLUTION_2448x2448:
		return CAM_MENU_PHOTO_RESOLUTION_2448x2448;
	case CAM_RESOLUTION_2048x1536:
		return CAM_MENU_PHOTO_RESOLUTION_2048x1536;
	case CAM_RESOLUTION_2048x1232:
		return CAM_MENU_PHOTO_RESOLUTION_2048x1232;
	case CAM_RESOLUTION_2048x1152:
		return CAM_MENU_PHOTO_RESOLUTION_2048x1152;
	case CAM_RESOLUTION_1920x1080:
		return CAM_MENU_PHOTO_RESOLUTION_1920x1080;
	case CAM_RESOLUTION_1440x1080:
		return CAM_MENU_PHOTO_RESOLUTION_1440x1080;
	case CAM_RESOLUTION_1600x1200:
		return CAM_MENU_PHOTO_RESOLUTION_1600x1200;
	case CAM_RESOLUTION_1600x960:
		return CAM_MENU_PHOTO_RESOLUTION_1600x960;
	case CAM_RESOLUTION_1392x1392:
		return CAM_MENU_PHOTO_RESOLUTION_1392x1392;
	case CAM_RESOLUTION_1280x960:
		return CAM_MENU_PHOTO_RESOLUTION_1280x960;
	case CAM_RESOLUTION_1280x720:
		return CAM_MENU_PHOTO_RESOLUTION_1280x720;
	case CAM_RESOLUTION_1056x1056:
		return CAM_MENU_PHOTO_RESOLUTION_1056x1056;
	case CAM_RESOLUTION_800x480:
		return CAM_MENU_PHOTO_RESOLUTION_800x480;
	case CAM_RESOLUTION_800x450:
		return CAM_MENU_PHOTO_RESOLUTION_800x450;
	case CAM_RESOLUTION_WVGA2:
		return CAM_MENU_PHOTO_RESOLUTION_720x480;
	case CAM_RESOLUTION_VGA:
		return CAM_MENU_PHOTO_RESOLUTION_640x480;
	default:
		return CAM_MENU_PHOTO_RESOLUTION_640x480;
	}
}

int cam_convert_iso_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_ISO_AUTO:
		return CAM_MENU_ISO_AUTO;
	case CAM_ISO_50:
		return CAM_MENU_ISO_50;
	case CAM_ISO_100:
		return CAM_MENU_ISO_100;
	case CAM_ISO_200:
		return CAM_MENU_ISO_200;
	case CAM_ISO_400:
		return CAM_MENU_ISO_400;
	case CAM_ISO_800:
		return CAM_MENU_ISO_80O;
	case CAM_ISO_1600:
		return CAM_MENU_ISO_1600;
	case CAM_ISO_3200:
		return CAM_MENU_ISO_320O;
	default:
		return CAM_MENU_ISO_AUTO;
	}
}

int cam_convert_focus_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_FOCUS_AUTO:
		return CAM_MENU_FOCUS_AUTO;
	case CAM_FOCUS_MACRO:
		return CAM_MENU_FOCUS_MACRO;
	case CAM_FOCUS_FACE:
		return CAM_MENU_FOCUS_FACE_DETECTION;
	default:
		return CAM_MENU_FOCUS_AUTO;
	}
}

int cam_convert_timer_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_SETTINGS_TIMER_OFF:
		return CAM_MENU_TIMER_OFF;
	case CAM_SETTINGS_TIMER_3SEC:
		return CAM_MENU_TIMER_3SEC;
	case CAM_SETTINGS_TIMER_10SEC:
		return CAM_MENU_TIMER_10SEC;
	default:
		return CAM_MENU_TIMER_OFF;
	}
}

int cam_convert_scene_mode_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_SCENE_NONE:
		return CAM_MENU_SCENE_AUTO;
	case CAM_SCENE_PORTRAIT:
		return CAM_MENU_SCENE_PORTRAIT;
	case CAM_SCENE_LANDSCAPE:
		return CAM_MENU_SCENE_LANDSCAPE;
	case CAM_SCENE_NIGHT:
		return CAM_MENU_SCENE_NIGHT;
	case CAM_SCENE_SPORTS:
		return CAM_MENU_SCENE_SPORTS;
	case CAM_SCENE_PARTY:
		return CAM_MENU_SCENE_PARTY;
	case CAM_SCENE_BEACHSNOW:
		return CAM_MENU_SCENE_BEACHSNOW;
	case CAM_SCENE_SUNSET:
		return CAM_MENU_SCENE_SUNSET;
	case CAM_SCENE_DUSKDAWN:
		return CAM_MENU_SCENE_DUSKDAWN;
	case CAM_SCENE_FALL:
		return CAM_MENU_SCENE_FALL;
	case CAM_SCENE_FIREWORK:
		return CAM_MENU_SCENE_FIREWORK;
	case CAM_SCENE_TEXT:
		return CAM_MENU_SCENE_TEXT;
	case CAM_SCENE_CANDLELIGHT:
		return CAM_MENU_SCENE_CANDLELIGHT;
	case CAM_SCENE_BACKLIGHT:
		return CAM_MENU_SCENE_BACKLIGHT;
	default:
		return CAM_MENU_SCENE_AUTO;
	}
}

int cam_convert_wb_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_SETTINGS_WB_AWB:
		return CAM_MENU_WB_AWB;
	case CAM_SETTINGS_WB_INCANDESCENT:
		return CAM_MENU_WB_INCANDESCENT;
	case CAM_SETTINGS_WB_FLUORESCENT:
		return CAM_MENU_WB_FLUORESCENT;
	case CAM_SETTINGS_WB_DAYLIGHT:
		return CAM_MENU_WB_DAYLIGHT;
	case CAM_SETTINGS_WB_CLOUDY:
		return CAM_MENU_WB_CLOUDY;
	case CAM_SETTINGS_WB_CUSTOM:
		return CAM_MENU_WB_CLOUDY;
	default:
		return CAM_MENU_WB_AWB;
	}
}

int cam_convert_metering_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_METERING_MATRIX:
		return CAM_MENU_METERING_MATRIX;
	case CAM_METERING_CENTER_WEIGHTED:
		return CAM_MENU_METERING_CENTER_WEIGHTED;
	case CAM_METERING_SPOT:
		return CAM_MENU_METERING_SPOT;
	default:
		return CAM_MENU_METERING_MATRIX;
	}
}

int cam_convert_shutter_sound_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_SHUTTER_SOUND_OFF:
		return CAM_MENU_SHUTTER_SOUND_OFF;
	case CAM_SHUTTER_SOUND_ON:
		return CAM_MENU_SHUTTER_SOUND_ON;
	default:
		return CAM_MENU_SHUTTER_SOUND_ON;
	}
}

int cam_convert_storage_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_STORAGE_INTERNAL:
		return CAM_MENU_STORAGE_PHONE;
	case CAM_STORAGE_EXTERNAL:
		return CAM_MENU_STORAGE_MMC;
	default:
		return CAM_MENU_STORAGE_PHONE;
	}
}

int cam_convert_volume_key_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_VOLUME_KEY_ZOOM:
		return CAM_MENU_VOLUME_KEY_ZOOM;
	case CAM_VOLUME_KEY_CAMERA:
		return CAM_MENU_VOLUME_KEY_CAMERA;
	case CAM_VOLUME_KEY_RECORD:
		return CAM_MENU_VOLUME_KEY_RECORD;
	default:
		return CAM_MENU_VOLUME_KEY_CAMERA;
	}
}

int cam_convert_share_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_SHARE_OFF:
		return CAM_MENU_SHARE_OFF;
	case CAM_SHARE_BUDDY_PHOTO:
		return CAM_MENU_SHARE_BUDDY_PHOTO;
	default:
		return CAM_MENU_SHARE_OFF;
	}
}

int cam_convert_fast_motion_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_FAST_MOTION_X2:
		return CAM_MENU_FAST_MOTION_X2;
	case CAM_FAST_MOTION_X4:
		return CAM_MENU_FAST_MOTION_X4;
	case CAM_FAST_MOTION_X8:
		return CAM_MENU_FAST_MOTION_X8;
	default:
		return CAM_MENU_FAST_MOTION_X2;
	}
}

int cam_convert_slow_motion_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_SLOW_MOTION_X2:
		return CAM_MENU_SLOW_MOTION_X2;
	case CAM_SLOW_MOTION_X4:
		return CAM_MENU_SLOW_MOTION_X4;
	case CAM_SLOW_MOTION_X8:
		return CAM_MENU_SLOW_MOTION_X8;
	default:
		return CAM_MENU_SLOW_MOTION_X2;
	}
}

int cam_convert_effect_to_menu_item(int mode)
{
	switch (mode) {
	case CAM_SETTINGS_EFFECTS_NOR:
		return CAM_MENU_EFFECT_NONE;
	case CAM_SETTINGS_EFFECTS_SEPIA:
		return CAM_MENU_EFFECT_SEPIA;
	case CAM_SETTINGS_EFFECTS_BLACKWHITE:
		return CAM_MENU_EFFECT_GREY;
	case CAM_SETTINGS_EFFECTS_NEGATIVE:
		return CAM_MENU_EFFECT_NEGATIVE;
	default:
		return CAM_MENU_MAX_NUMS;
	}
}

int cam_convert_face_detection_to_menu_item(int mode)
{
	switch (mode) {
	case 1:
		return CAM_MENU_FACE_DETECTION_ON;
	default:
		return CAM_MENU_FACE_DETECTION_OFF;
	}
}

int cam_convert_anti_shake_to_menu_item(int mode)
{
	switch (mode) {
	case 1:
		return CAM_MENU_ANTI_SHAKE_ON;
	default:
		return CAM_MENU_ANTI_SHAKE_OFF;
	}
}

int cam_convert_tap_shot_to_menu_item(int mode)
{
	switch (mode) {
	case 1:
		return CAM_MENU_TAP_SHOT_ON;
	default:
		return CAM_MENU_TAP_SHOT_OFF;
	}
}

int cam_convert_review_to_menu_item(int mode)
{
	switch (mode) {
	case 1:
		return CAM_MENU_REVIEW_ON;
	default:
		return CAM_MENU_REVIEW_OFF;
	}
}

int cam_convert_save_as_flipped_to_menu_item(int mode)
{
	switch (mode) {
	case 1:
		return CAM_MENU_SAVE_AS_FLIP_ON;
	default:
		return CAM_MENU_SAVE_AS_FLIP_OFF;
	}
}

int cam_convert_setting_value_to_menu_index(void* data, int menu_type, int value)
{
	struct appdata *ad = (struct appdata *)data;
	cam_retvm_if(ad == NULL, INVALID_INDEX, "appdata is NULL");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, INVALID_INDEX, "camapp_handle is NULL");

	cam_menu_composer* menu_composer = NULL;
	menu_composer = (cam_menu_composer *)CAM_CALLOC(1, sizeof(cam_menu_composer));
	cam_retvm_if(menu_composer == NULL, INVALID_INDEX, "menu_composer is NULL");

	int item = 0;

	switch (menu_type) {
	case CAM_MENU_SHOOTING_MODE:
		cam_compose_menu_shooting_mode(data, menu_composer);
		item = cam_convert_shot_mode_to_menu_item(value);
		break;

	case CAM_MENU_FLASH:
		cam_compose_menu_flash(data, menu_composer);
		item = cam_convert_flash_to_menu_item(value);
		break;

	case CAM_MENU_RECORDING_MODE:
		cam_compose_menu_recording_mode(data, menu_composer);
		item = cam_convert_recording_mode_to_menu_item(value);
		break;

	case CAM_MENU_PHOTO_RESOLUTION:
		cam_compose_menu_photo_resolution(data, menu_composer);
		item = cam_convert_photo_resolution_to_menu_item(value);
		break;

	case CAM_MENU_VIDEO_RESOLUTION:
		cam_compose_menu_video_resolution(data, menu_composer);
		item = cam_convert_video_resolution_to_menu_item(value);
		break;

	case CAM_MENU_ISO:
		cam_compose_menu_iso(data, menu_composer);
		item = cam_convert_iso_to_menu_item(value);
		break;

	case CAM_MENU_FOCUS_MODE:
		cam_compose_menu_focus_mode(data, menu_composer);
		item = cam_convert_focus_to_menu_item(value);
		break;

	case CAM_MENU_TIMER:
		cam_compose_menu_timer_mode(data, menu_composer);
		item = cam_convert_timer_to_menu_item(value);
		break;

	case CAM_MENU_SCENE_MODE:
		cam_compose_menu_scene_mode(data, menu_composer);
		item = cam_convert_scene_mode_to_menu_item(value);
		break;

	case CAM_MENU_WHITE_BALANCE:
		cam_compose_menu_wb(data, menu_composer);
		item = cam_convert_wb_to_menu_item(value);
		break;

	case CAM_MENU_METERING:
		cam_compose_menu_metering(data, menu_composer);
		item = cam_convert_metering_to_menu_item(value);
		break;

	case CAM_MENU_SHUTTER_SOUND:
		cam_compose_menu_shutter_sound(data, menu_composer);
		item = cam_convert_shutter_sound_to_menu_item(value);
		break;

	case CAM_MENU_STORAGE:
		cam_compose_menu_storage(data, menu_composer);
		item = cam_convert_storage_to_menu_item(value);
		break;

	case CAM_MENU_VOLUME_KEY:
		cam_compose_menu_volume_key(data, menu_composer);
		item = cam_convert_volume_key_to_menu_item(value);
		break;

	case CAM_MENU_SHARE:
		cam_compose_menu_share(data, menu_composer);
		item = cam_convert_share_to_menu_item(value);
		break;

	case CAM_MENU_FAST_MOTION:
		cam_compose_menu_fast_motion(data, menu_composer);
		item = cam_convert_fast_motion_to_menu_item(value);
		break;

	case CAM_MENU_SLOW_MOTION:
		cam_compose_menu_slow_motion(data, menu_composer);
		item = cam_convert_slow_motion_to_menu_item(value);
		break;

	case CAM_MENU_EFFECTS:
		cam_compose_menu_effect(data, menu_composer);
		item = cam_convert_effect_to_menu_item(value);
		break;
	default:
		break;
	}

	CAM_MENU_ITEM* menu_item  = NULL;
	int size = eina_array_count(menu_composer->array);

	int index = 0;
	for (index = 0; index < size; index++) {
		menu_item = (CAM_MENU_ITEM*)eina_array_data_get(menu_composer->array, index);

		if ((CAM_MENU_ITEM)item == *menu_item) {
			cam_compose_free(menu_composer);
			return index;
		}
	}

	cam_compose_free(menu_composer);
	return INVALID_INDEX;
}

const char* cam_get_menu_item_image(CAM_MENU_ITEM item, CamMenuState state)
{
	char *icon_name = NULL;

	switch (item) {
	case CAM_MENU_SHORTCUTS:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_SETTING_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_SETTING_DIM;
		} else {
			icon_name = QUICK_SETTING_SETTING_NORMAL;
		}
		break;
	case CAM_MENU_METERING_CENTER_WEIGHTED:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_METERING_CENTER_WEIGHTED_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_METERING_CENTER_WEIGHTED_DIM;
		} else {
			icon_name = QUICK_SETTING_METERING_CENTER_WEIGHTED_NORMAL;
		}
		break;
	case CAM_MENU_METERING_MATRIX:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_METERING_MULTI_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_METERING_MULTI_DIM;
		} else {
			icon_name = QUICK_SETTING_METERING_MULTI_NORMAL;
		}
		break;
	case CAM_MENU_METERING_SPOT:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_METERING_SPOT_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_METERING_SPOT_DIM;
		} else {
			icon_name = QUICK_SETTING_METERING_SPOT_NORMAL;
		}
		break;
	case CAM_MENU_EXPOSURE_VALUE:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_EXPOSURE_VALUE_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_EXPOSURE_VALUE_DIM;
		} else {
			icon_name = QUICK_SETTING_EXPOSURE_VALUE_NORMAL;
		}
		break;
	case CAM_MENU_WB_AWB:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_WHITE_BALANCE_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_WHITE_BALANCE_DIM;
		} else {
			icon_name = QUICK_SETTING_WHITE_BALANCE_NORMAL;
		}
		break;
	case CAM_MENU_WB_CLOUDY:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_WHITE_BALANCE_CLOUDY_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_WHITE_BALANCE_CLOUDY_DIM;
		} else {
			icon_name = QUICK_SETTING_WHITE_BALANCE_CLOUDY_NORMAL;
		}
		break;
	case CAM_MENU_WB_DAYLIGHT:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_WHITE_BALANCE_DAYLIGHT_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_WHITE_BALANCE_DAYLIGHT_DIM;
		} else {
			icon_name = QUICK_SETTING_WHITE_BALANCE_DAYLIGHT_NORMAL;
		}
		break;
	case CAM_MENU_WB_FLUORESCENT:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_WHITE_BALANCE_FLUORESCENT_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_WHITE_BALANCE_FLUORESCENT_DIM;
		} else {
			icon_name = QUICK_SETTING_WHITE_BALANCE_FLUORESCENT_NORMAL;
		}
		break;
	case CAM_MENU_WB_INCANDESCENT:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_WHITE_BALANCE_INCANDESCENT_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_WHITE_BALANCE_INCANDESCENT_DIM;
		} else {
			icon_name = QUICK_SETTING_WHITE_BALANCE_INCANDESCENT_NORMAL;
		}
		break;
	case CAM_MENU_FLASH_OFF:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_FLASH_OFF_PRESS ;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_FLASH_OFF_DIM;
		} else {
			icon_name = QUICK_SETTING_FLASH_OFF_NORMAL;
		}
		break;
	case CAM_MENU_FLASH_ON:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_FLASH_ON_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_FLASH_ON_DIM;
		} else {
			icon_name = QUICK_SETTING_FLASH_ON_NORMAL;
		}
		break;
	case CAM_MENU_FLASH_AUTO:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_FLASH_AUTO_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_FLASH_AUTO_DIM;
		} else {
			icon_name = QUICK_SETTING_FLASH_AUTO_NORMAL;
		}
		break;
	case CAM_MENU_RECORDING_MODE_NORMAL:
	case CAM_MENU_RECORDING_MODE_SELF_NORMAL:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_RECORDING_NORMAL_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_RECORDING_NORMAL_DIM;
		} else {
			icon_name = QUICK_SETTING_RECORDING_NORMAL_NORMAL;
		}
		break;
	case CAM_MENU_RECORDING_MODE_LIMIT:
	case CAM_MENU_RECORDING_MODE_SELF_LIMIT:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_RECORDING_MMS_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_RECORDING_MMS_DIM;
		} else {
			icon_name = QUICK_SETTING_RECORDING_MMS_NORMAL;
		}
		break;
	case CAM_MENU_ISO_AUTO:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_ISO_AUTO_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_ISO_AUTO_DIM;
		} else {
			icon_name = QUICK_SETTING_ISO_AUTO_NORMAL;
		}
		break;
	case CAM_MENU_ISO_100:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_ISO_100_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_ISO_100_DIM;
		} else {
			icon_name = QUICK_SETTING_ISO_100_NORMAL;
		}
		break;
	case CAM_MENU_ISO_200:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_ISO_200_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_ISO_200_DIM;
		} else {
			icon_name = QUICK_SETTING_ISO_200_NORMAL;
		}
		break;
	case CAM_MENU_ISO_400:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_ISO_400_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_ISO_400_DIM;
		} else {
			icon_name = QUICK_SETTING_ISO_400_NORMAL;
		}
		break;
	case CAM_MENU_ISO_80O:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_ISO_800_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_ISO_800_DIM;
		} else {
			icon_name = QUICK_SETTING_ISO_800_NORMAL;
		}
		break;
	case CAM_MENU_TIMER_OFF:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_TIMER_OFF_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_TIMER_OFF_DIM;
		} else {
			icon_name = QUICK_SETTING_TIMER_OFF_NORMAL;
		}
		break;
	case CAM_MENU_TIMER_3SEC:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_TIMER_3SEC_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_TIMER_3SEC_DIM;
		} else {
			icon_name = QUICK_SETTING_TIMER_3SEC_NORMAL;
		}
		break;
	case CAM_MENU_TIMER_10SEC:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_TIMER_10SEC_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_TIMER_10SEC_DIM;
		} else {
			icon_name = QUICK_SETTING_TIMER_10SEC_NORMAL;
		}
		break;
	case CAM_MENU_SHUTTER_SOUND_ON:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_SHUTTER_SOUND_ON_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_SHUTTER_SOUND_ON_DIM;
		} else {
			icon_name = QUICK_SETTING_SHUTTER_SOUND_ON_NORMAL;
		}
		break;
	case CAM_MENU_SHUTTER_SOUND_OFF:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_SHUTTER_SOUND_OFF_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_SHUTTER_SOUND_OFF_DIM;
		} else {
			icon_name = QUICK_SETTING_SHUTTER_SOUND_OFF_NORMAL;
		}
		break;
	case CAM_MENU_STORAGE_PHONE:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_STORAGE_MYDEVICE_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_STORAGE_MYDEVICE_DIM;
		} else {
			icon_name = QUICK_SETTING_STORAGE_MYDEVICE_NORMAL;
		}
		break;
	case CAM_MENU_STORAGE_MMC:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_STORAGE_CARD_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_STORAGE_CARD_DIM;
		} else {
			icon_name = QUICK_SETTING_STORAGE_CARD_NORMAL;
		}
		break;
	case CAM_MENU_VOLUME_KEY_ZOOM:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_VOLUME_KEY_AS_ZOOM_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_VOLUME_KEY_AS_ZOOM_DIM;
		} else {
			icon_name = QUICK_SETTING_VOLUME_KEY_AS_ZOOM_NORMAL;
		}
		break;
	case CAM_MENU_VOLUME_KEY_CAMERA:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_VOLUME_KEY_AS_CAMERA_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_VOLUME_KEY_AS_CAMERA_DIM;
		} else {
			icon_name = QUICK_SETTING_VOLUME_KEY_AS_CAMERA_NORMAL;
		}
		break;
	case CAM_MENU_VOLUME_KEY_RECORD:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_VOLUME_KEY_AS_RECORD_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_VOLUME_KEY_AS_RECORD_DIM;
		} else {
			icon_name = QUICK_SETTING_VOLUME_KEY_AS_RECORD_NORMAL;
		}
		break;
	case CAM_MENU_TAP_SHOT_ON:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = SETTING_TAP_SHOT_ON_PRESS_IMAGE;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = SETTING_TAP_SHOT_ON_DIM_IMAGE;
		} else {
			icon_name = SETTING_TAP_SHOT_ON_IMAGE;
		}
		break;
	case CAM_MENU_TAP_SHOT_OFF:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = SETTING_TAP_SHOT_OFF_PRESS_IMAGE;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = SETTING_TAP_SHOT_OFF_DIM_IMAGE;
		} else {
			icon_name = SETTING_TAP_SHOT_OFF_IMAGE;
		}
		break;
	case CAM_MENU_GPS_TAG_ON:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_GPS_ON_NORMAL;//QUICK_SETTING_GPS_ON_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_GPS_ON_NORMAL;//QUICK_SETTING_GPS_ON_DIM;
		} else {
			icon_name = QUICK_SETTING_GPS_ON_NORMAL;
		}
		break;
	case CAM_MENU_GPS_TAG_OFF:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_GPS_OFF_NORMAL;//QUICK_SETTING_GPS_OFF_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_GPS_OFF_NORMAL;//QUICK_SETTING_GPS_OFF_DIM;
		} else {
			icon_name = QUICK_SETTING_GPS_OFF_NORMAL;
		}
		break;
	case CAM_MENU_REVIEW_ON:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_REVIEW_ON_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_REVIEW_ON_DIM;
		} else {
			icon_name = QUICK_SETTING_REVIEW_ON_NORMAL;
		}
		break;
	case CAM_MENU_REVIEW_OFF:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_REVIEW_OFF_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_REVIEW_OFF_DIM;
		} else {
			icon_name = QUICK_SETTING_REVIEW_OFF_NORMAL;
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_2048x1536:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = CAM_RESOLUTION_3_1M_PRESS_ICON;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = CAM_RESOLUTION_3_1M_DIM_ICON;
		} else {
			icon_name = CAM_RESOLUTION_3_1M_NORMAL_ICON;
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_2048x1232:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = CAM_RESOLUTION_W2_5M_PRESS_ICON;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = CAM_RESOLUTION_W2_5M_DIM_ICON;
		} else {
			icon_name = CAM_RESOLUTION_W2_5M_NORMAL_ICON;
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_1600x1200:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = CAM_RESOLUTION_1_9M_PRESS_ICON;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = CAM_RESOLUTION_1_9M_DIM_ICON;
		} else {
			icon_name = CAM_RESOLUTION_1_9M_NORMAL_ICON;
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_1600x960:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = CAM_RESOLUTION_W1_5M_PRESS_ICON;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = CAM_RESOLUTION_W1_5M_DIM_ICON;
		} else {
			icon_name = CAM_RESOLUTION_W1_5M_NORMAL_ICON;
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_800x480:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = CAM_RESOLUTION_0_4M_PRESS_ICON;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = CAM_RESOLUTION_0_4M_DIM_ICON;
		} else {
			icon_name = CAM_RESOLUTION_0_4M_NORMAL_ICON;
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_640x480:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = CAM_RESOLUTION_0_3M_PRESS_ICON;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = CAM_RESOLUTION_0_3M_DIM_ICON;
		} else {
			icon_name = CAM_RESOLUTION_0_3M_NORMAL_ICON;
		}
		break;
	case CAM_MENU_VIDEO_RESOLUTION_VGA:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = REC_RESOLUTION_VGA_PRESS_ICON;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = REC_RESOLUTION_VGA_DIM_ICON;
		} else {
			icon_name = REC_RESOLUTION_VGA_NORMAL_ICON;
		}
		break;
	case CAM_MENU_VIDEO_RESOLUTION_QVGA:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = REC_RESOLUTION_QVGA_PRESS_ICON;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = REC_RESOLUTION_QVGA_DIM_ICON;
		} else {
			icon_name = REC_RESOLUTION_QVGA_NORMAL_ICON;
		}
		break;
	case CAM_MENU_VIDEO_RESOLUTION_QCIF:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = REC_RESOLUTION_QCIF_PRESS_ICON;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = REC_RESOLUTION_QCIF_DIM_ICON;
		} else {
			icon_name = REC_RESOLUTION_QCIF_NORMAL_ICON;
		}
		break;
	case CAM_MENU_FACE_DETECTION_ON:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = FACE_DETECTION_ON_PRESS_ICON;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = FACE_DETECTION_ON_DIM_ICON;
		} else {
			icon_name = FACE_DETECTION_ON_NORMAL_ICON;
		}
		break;
	case CAM_MENU_FACE_DETECTION_OFF:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = FACE_DETECTION_OFF_PRESS_ICON;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = FACE_DETECTION_OFF_DIM_ICON;
		} else {
			icon_name = FACE_DETECTION_OFF_NORMAL_ICON;
		}
		break;
	case CAM_MENU_RESET:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = SHOTCUTS_EDIT_RESET_PRESS_IMAGE;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = SHOTCUTS_EDIT_RESET_DIM_IMAGE;
		} else {
			icon_name = SHOTCUTS_EDIT_RESET_IMAGE;
		}
		break;
	case CAM_MENU_SELF_PORTRAIT_ON:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = SELF_PORTRAIT_FRONT_PRESS_IMAGE;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = SELF_PORTRAIT_FRONT_DIM_IMAGE;
		} else {
			icon_name = SELF_PORTRAIT_FRONT_NORMAL_IMAGE;
		}
		break;
	case CAM_MENU_SELF_PORTRAIT_OFF:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = SELF_PORTRAIT_FRONT_PRESS_IMAGE;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = SELF_PORTRAIT_FRONT_DIM_IMAGE;
		} else {
			icon_name = SELF_PORTRAIT_FRONT_NORMAL_IMAGE;
		}
		break;
	case CAM_MENU_SETTING:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_SETTING_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_SETTING_DIM;
		} else {
			icon_name = QUICK_SETTING_SETTING_NORMAL;
		}
		break;
	case CAM_MENU_EFFECTS:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_EFFECT_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_EFFECT_DIM;
		} else {
			icon_name = QUICK_SETTING_EFFECT_NORMAL;
		}
		break;
	case CAM_MENU_EFFECT_NONE:
		icon_name = MANAGE_EFFECT_ORIGINAL;
		break;
	case CAM_MENU_EFFECT_GREY:
		icon_name = MANAGE_EFFECT_GREYSCALE;
		break;
	case CAM_MENU_EFFECT_SEPIA:
		icon_name = MANAGE_EFFECT_SEPIA;
		break;
	case CAM_MENU_EFFECT_NEGATIVE:
		icon_name = MANAGE_EFFECT_NEGATIVE;
		break;
	case CAM_MENU_SAVE_AS_FLIP_ON:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_SAVE_AS_FLIP_ON_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_SAVE_AS_FLIP_ON_DIM;
		} else {
			icon_name = QUICK_SETTING_SAVE_AS_FLIP_ON_NORMAL;
		}
		break;
	case CAM_MENU_SAVE_AS_FLIP_OFF:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_SAVE_AS_FLIP_OFF_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_SAVE_AS_FLIP_OFF_DIM;
		} else {
			icon_name = QUICK_SETTING_SAVE_AS_FLIP_OFF_NORMAL;
		}
		break;
	case CAM_MENU_MORE:
		if (state == CAM_MENU_STATE_PRESS) {
			icon_name = QUICK_SETTING_MORE_PRESS;
		} else if (state == CAM_MENU_STATE_DIM) {
			icon_name = QUICK_SETTING_MORE_DIM;
		} else {
			icon_name = QUICK_SETTING_MORE_NORMAL;
		}
		break;
	default:
		cam_critical(LOG_UI, "invalid menu item [%d]", item);
		icon_name = NULL;
	}

	return icon_name;
}

void cam_get_menu_item_description(CAM_MENU_ITEM item, char *description)
{
	cam_retm_if(description == NULL, "description is NULL");
	char *strID = NULL;
	switch (item) {
	case CAM_MENU_RECORDING_MODE_NORMAL:
	case CAM_MENU_RECORDING_MODE_SELF_NORMAL:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_RECORD_FILES_USING_YOUR_CUSTOM_SIZE_SETTING_M_VIDEO");
		if (strID) {
			if (strlen(strID) + 1 <= 512) {
				strncpy(description, strID, strlen(strID) + 1);
			}
		}
		break;
	case CAM_MENU_RECORDING_MODE_LIMIT:
	case CAM_MENU_RECORDING_MODE_SELF_LIMIT:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_THE_FILE_SIZE_OF_THE_RECORDING_WILL_BE_LIMITED_FOR_SHARING_VIA_MESSAGES_M_VIDEO");
		if (strID) {
			if (strlen(strID) + 1 <= 512) {
				strncpy(description, strID, strlen(strID) + 1);
			}
		}
		break;
	case CAM_MENU_RECORDING_MODE_SLOW: {
		char *tmp_str = dgettext(PACKAGE, "IDS_CAM_BODY_RECORD_FILES_AT_PD_FPS_FOR_VIEWING_IN_SLOW_MOTION_M_VIDEO");
		if (tmp_str != NULL) {
			snprintf(description, strlen(tmp_str) + 1, tmp_str, 120);
		}
	}
	break;
	case CAM_MENU_RECORDING_MODE_FAST:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_RECORD_FILES_FOR_VIEWING_IN_FAST_MOTION_M_VIDEO");
		if (strID) {
			if (strlen(strID) + 1 <= 512) {
				strncpy(description, strID, strlen(strID) + 1);
			}
		}
		break;
	case CAM_MENU_SHARE_BUDDY_PHOTO:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_SHARE_PICTURES_WITH_YOUR_FRIENDS_AND_CONTACTS_WHOSE_FACES_HAVE_BEEN_DETECTED");
		if (strID) {
			if (strlen(strID) + 1 <= 512) {
				strncpy(description, strID, strlen(strID) + 1);
			}
		}
		break;
	case CAM_MENU_ANTI_SHAKE:
		strID = dgettext(PACKAGE, "IDS_CAM_POP_ANTI_SHAKE_PREVENTS_BLURRING_BY_SPEEDING_UP_THE_SHUTTER");
		if (strID) {
			if (strlen(strID) + 1 <= 512) {
				strncpy(description, strID, strlen(strID) + 1);
			}
		}
		break;
	case CAM_MENU_REVIEW:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_DISPLAYS_PICTURES_AND_VIDEOS_IMMEDIATELY_AFTER_YOU_TAKE_OR_RECORD_THEM");
		if (strID) {
			if (strlen(strID) + 1 <= 512) {
				strncpy(description, strID, strlen(strID) + 1);
			}
		}
		break;
	case CAM_MENU_GPS_TAG:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_THIS_FUNCTION_WILL_ATTACH_EMBED_AND_STORE_GEOGRAPHICAL_LOCATION_DATA_WITHIN_EACH_PICTURE_THAT_YOU_TAKE_MSG");
		if (strID) {
			if (strlen(strID) + 1 <= 512) {
				strncpy(description, strID, strlen(strID) + 1);
			}
		}
		break;
	case CAM_MENU_VIDEO_STABILIZATION:
		strID = dgettext(PACKAGE, "IDS_CAM_BODY_STABILISES_THE_DISPLAY_WHEN_YOU_RECORD_VIDEOS");
		if (strID) {
			if (strlen(strID) + 1 <= 512) {
				strncpy(description, strID, strlen(strID) + 1);
			}
		}
		break;
	default:
		strID = dgettext(PACKAGE, "test");
		if (strID) {
			if (strlen(strID) + 1 <= 512) {
				strncpy(description, strID, strlen(strID) + 1);
			}
		}
		cam_critical(LOG_UI, "error type [%d], please check!", item);
		break;
	}

}

void cam_get_menu_item_text(CAM_MENU_ITEM item, char *description, gboolean without_proportion)
{
	cam_retm_if(description == NULL, "description is NULL");
	char *strID = NULL;
	switch (item) {
	case CAM_MENU_SCENE_AUTO:
		strID = "IDS_CAM_BODY_NONE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_PORTRAIT:
		strID = "IDS_CAM_OPT_SCENE_PORTRAIT";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_LANDSCAPE:
		strID = "IDS_CAM_OPT_SCENE_LANDSCAPE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_NIGHT:
		strID = "IDS_CAM_OPT_SCENEMODE_NIGHT";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_SPORTS:
		strID = "IDS_CAM_OPT_SCENE_SPORTS";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_PARTY:
		strID = "IDS_CAM_OPT_SCENEMODE_PARTY_INDOOR";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_BEACHSNOW:
		strID = "IDS_CAM_OPT_BEACH_SNOW";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_SUNSET:
		strID = "IDS_CAM_OPT_SCENEMODE_SUNSET";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_DUSKDAWN:
		strID = "IDS_CAM_OPT_SCENE_DAWN";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_FALL:
		strID = "IDS_CAM_OPT_AUTUMN_COLOUR";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_FIREWORK:
		strID = "IDS_CAM_BODY_SCENEMODE_FIREWORK";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_TEXT:
		strID = "IDS_CAM_BODY_SCENEMODE_TEXT";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_CANDLELIGHT:
		strID = "IDS_CAM_OPT_SCENEMODE_CANDLELIGHT";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_BACKLIGHT:
		strID = "IDS_CAM_OPT_SCENE_BACKLIGHT";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_3264x2448:
		if (without_proportion) {
			strID = "3264x2448";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "3264x2448 (4:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_3264x1836:
		if (without_proportion) {
			strID = "3264x1836";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "3264x1836 (16:9)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_2560x1920:
		if (without_proportion) {
			strID = "2560x1920";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "2560x1920 (4:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_2560x1440:
		if (without_proportion) {
			strID = "2560x1440";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "2560x1440 (16:9)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_2448x2448:
		if (without_proportion) {
			strID = "2448x2448";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "2448x2448 (1:1)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_2048x1536:
		if (without_proportion) {
			strID = "2048x1536";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "2048x1536 (4:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_2048x1232:
		if (without_proportion) {
			strID = "2048x1232";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "2048x1232 (5:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_2048x1152:
		if (without_proportion) {
			strID = "2048x1152";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "2048x1152 (16:9)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_1920x1080:
		if (without_proportion) {
			strID = "1920x1080";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "1920x1080 (16:9)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_1600x1200:
		if (without_proportion) {
			strID = "1600x1200";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "1600x1200 (4:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_1600x960:
		if (without_proportion) {
			strID = "1600x960";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "1600x960 (5:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_1440x1080:
		if (without_proportion) {
			strID = "1440x1080";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "1440x1080 (4:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_1392x1392:
		if (without_proportion) {
			strID = "1392x1392";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "1392x1392 (1:1)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_1280x960:
		if (without_proportion) {
			strID = "1280x960";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "1280x960 (4:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_1280x720:
		if (without_proportion) {
			strID = "1280x720";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "1280x720 (16:9)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_1056x1056:
		if (without_proportion) {
			strID = "1080x1080";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "1080x1080 (1:1)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_800x480:
		if (without_proportion) {
			strID = "800x480";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "800x480 (5:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_800x450:
		if (without_proportion) {
			strID = "800x450";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "800x450 (16:9)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_720x480:
		if (without_proportion) {
			strID = "720x480";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "720x480 (16:9)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION_640x480:
		if (without_proportion) {
			strID = "640x480";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "640x480 (4:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_VIDEO_RESOLUTION_FULLHD:
		if (without_proportion) {
			strID = "1920x1080";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "1920x1080 (16:9)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_VIDEO_RESOLUTION_HD:
		if (without_proportion) {
			strID = "1280x720";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "1280x720 (16:9)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_VIDEO_RESOLUTION_WVGA:
		if (without_proportion) {
			strID = "800x480";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "800x480 (5:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
		break;
	case CAM_MENU_VIDEO_RESOLUTION_VGA:
		if (without_proportion) {
			strID = "640x480";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "640x480 (4:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_VIDEO_RESOLUTION_QVGA:
		if (without_proportion) {
			strID = "320x240";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "320x240 (4:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_VIDEO_RESOLUTION_QCIF:
		strID = "176x144";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_VIDEO_RESOLUTION_1440x1080:
		if (without_proportion) {
			strID = "1440x1080";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "1440x1080 (4:3)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_VIDEO_RESOLUTION_1056x1056:
	case CAM_MENU_VIDEO_RESOLUTION_1080x1080:
		if (without_proportion) {
			strID = "1080x1080";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		} else {
			strID = "1080x1080 (1:1)";
			if (strID) {
				if (strlen(strID) + 1 <= 128) {
					strncpy(description, strID, 128);
				}
			}
		}
		break;
	case CAM_MENU_VIDEO_RESOLUTION_800x450:
		strID = "800x450";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_WB_AWB:
		strID = "IDS_CAM_BODY_AUTO";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_WB_INCANDESCENT:
		strID = "IDS_CAM_BODY_WHITEBALANCE_INCANDESCENT";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_WB_FLUORESCENT:
		strID = "IDS_CAM_BODY_FLUORESCENT";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_WB_DAYLIGHT:
		strID = "IDS_CAM_BODY_DAYLIGHT";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_WB_CLOUDY:
		strID = "IDS_CAM_BODY_CLOUDY";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_FOCUS_AUTO:
		strID = "IDS_CAM_OPT_AUTO_FOCUS";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_FOCUS_MACRO:
		strID = "IDS_CAM_BODY_MACRO";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_FOCUS_FACE_DETECTION:
		strID = "IDS_CAM_OPT_FACE_DETECTION";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_FACE_DETECTION:
		strID = "IDS_CAM_OPT_FACE_DETECTION";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_RECORDING_MODE_NORMAL:
		strID = "IDS_CAM_BODY_NORMAL_M_OPTION";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_RECORDING_MODE_LIMIT:
		strID = "IDS_CAM_BUTTON_LIMIT_FOR_MMS_ABB";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_RECORDING_MODE_SLOW:
	case CAM_MENU_SLOW_MOTION:
		strID = "IDS_CAM_OPT_SLOW_MOTION";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_RECORDING_MODE_FAST:
	case CAM_MENU_FAST_MOTION:
		strID = "IDS_CAM_BODY_FAST_MOTION";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_RECORDING_MODE_SELF_NORMAL:
		strID = "IDS_CAM_BODY_NORMAL_M_OPTION";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_RECORDING_MODE_SELF_LIMIT:
		strID = "IDS_CAM_BUTTON_LIMIT_FOR_MMS_ABB";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_FLASH_OFF:
		strID = "IDS_CAM_BODY_OFF";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_FLASH_ON:
		strID = "IDS_CAM_BODY_ON";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_FLASH_AUTO:
		strID = "IDS_CAM_BODY_AUTO";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_METERING_MATRIX:
		strID = "IDS_CAM_BODY_MATRIX";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_METERING_CENTER_WEIGHTED:
		strID = "IDS_CAM_BODY_CENTER_WEIGHTED";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_METERING_SPOT:
		strID = "IDS_CAM_BODY_SPOT";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_ISO_AUTO:
		strID = "IDS_CAM_BODY_AUTO";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_ISO_50:
		strID = "50";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_ISO_100:
		strID = "100";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_ISO_200:
		strID = "200";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_ISO_400:
		strID = "400";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_ISO_80O:
		strID = "800";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_ISO_1600:
		strID = "1600";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_ISO_320O:
		strID = "3200";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_STORAGE_PHONE:
		strID = "IDS_CAM_BODY_DEVICE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_STORAGE_MMC:
		strID = "IDS_CAM_OPT_SD_CARD_ABB";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_TIMER_OFF:
		strID = "IDS_CAM_BODY_OFF";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_TIMER_3SEC:
		strID = "3";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_TIMER_10SEC:
		strID = "10";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_VOLUME_KEY_ZOOM:
		strID = "IDS_CAM_OPT_ZOOM";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_VOLUME_KEY_CAMERA:
		strID = "IDS_CAM_OPT_TAKE_PICTURE_ABB";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_VOLUME_KEY_RECORD:
		strID = "IDS_CAM_BODY_RECORD_VIDEO_ABB";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_ANTI_SHAKE_OFF:
	case CAM_MENU_VIDEO_STABILIZATION_OFF:
	case CAM_MENU_AUTO_CONTRAST_OFF:
	case CAM_MENU_SAVE_AS_FLIP_OFF:
	case CAM_MENU_TAP_SHOT_OFF:
	case CAM_MENU_REVIEW_OFF:
	case CAM_MENU_FACE_DETECTION_OFF:
	case CAM_MENU_SELF_PORTRAIT_OFF:
	case CAM_MENU_GPS_TAG_OFF:
	case CAM_MENU_SHUTTER_SOUND_OFF:
		strID = "IDS_CAM_BODY_OFF";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_ANTI_SHAKE_ON:
	case CAM_MENU_VIDEO_STABILIZATION_ON:
	case CAM_MENU_AUTO_CONTRAST_ON:
	case CAM_MENU_SAVE_AS_FLIP_ON:
	case CAM_MENU_TAP_SHOT_ON:
	case CAM_MENU_REVIEW_ON:
	case CAM_MENU_FACE_DETECTION_ON:
	case CAM_MENU_SELF_PORTRAIT_ON:
	case CAM_MENU_SHUTTER_SOUND_ON:
	case CAM_MENU_GPS_TAG_ON:
		strID = "IDS_CAM_BODY_ON";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SHORTCUTS:
		strID = "IDS_CAM_OPT_EDIT_SHORTCUTS";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_VOLUME_KEY:
		strID = "IDS_CAM_BUTTON2_VOLUME_KEY_ABB2";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SELF_PORTRAIT:
		strID = "IDS_CAM_BODY_SELF_SHOT";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_FLASH:
		strID = "IDS_CAM_BODY_FLASH";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_TAP_SHOT:
		strID = "IDS_CAM_OPT_TAP_TO_TAKE_PICS_ABB";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_GPS_TAG:
		strID = "IDS_CAM_MBODY_LOCATION_TAGS";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SHOOTING_MODE:
		strID = "IDS_CAM_OPT_SHOOTING_MODE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_RECORDING_MODE:
		strID = "IDS_CAM_OPT_RECORDING_MODE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SCENE_MODE:
		strID = "IDS_CAM_BODY_SCENE_MODE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_EXPOSURE_VALUE:
		strID = "IDS_CAM_OPT_EXPOSURE_VALUE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_FOCUS_MODE:
		strID = "IDS_CAM_BODY_FOCUS_MODE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_TIMER:
		strID = "IDS_CAM_BODY_TIMER";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_PHOTO_RESOLUTION:
		strID = "IDS_CAM_HEADER_PICTURE_SIZE_ABB2";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_VIDEO_RESOLUTION:
		strID = "IDS_CAM_MBODY_VIDEO_SIZE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_WHITE_BALANCE:
		strID = "IDS_CAM_OPT_WHITE_BALANCE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_ISO:
		strID = "IDS_CAM_BODY_ISO";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_METERING:
		strID = "IDS_CAM_HEADER_METERING_MODES_ABB";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_ANTI_SHAKE:
		strID = "IDS_CAM_BODY_ANTI_SHAKE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_VIDEO_STABILIZATION:
		strID = "IDS_CAM_BODY_VIDEO_STABILISATION";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_AUTO_CONTRAST:
		strID = "IDS_CAM_BODY_AUTO_CONTRAST";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SAVE_AS_FLIP:
		strID = "IDS_CAM_BODY_SAVE_AS_FLIPPED";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SHUTTER_SOUND:
		strID = "IDS_CAM_BODY_SETTINGS_SHUTTER_SOUND";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_STORAGE:
		strID = "IDS_CAM_MBODY_STORAGE_LOCATION";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_RESET:
		strID = "IDS_CAM_BODY_RESET_SETTINGS_RESET";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_REVIEW:
		strID = "IDS_CAM_HEADER_REVIEW_PICS_VIDEOS_ABB";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SHARE:
		strID = "IDS_CAM_BODY_SHARE_ABB";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SHARE_OFF:
		strID = "IDS_CAM_BODY_OFF";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SHARE_BUDDY_PHOTO:
		strID = "IDS_CAM_OPT_BUDDY_PHOTO_SHARE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_FAST_MOTION_X2:
		strID = "x2";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_FAST_MOTION_X4:
		strID = "x4";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_FAST_MOTION_X8:
		strID = "x8";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SLOW_MOTION_X2:
		strID = "x1/2";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SLOW_MOTION_X4:
		strID = "x1/4";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SLOW_MOTION_X8:
		strID = "x1/8";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_MORE_QUICK_SETTING:
		strID = "IDS_CAM_OPT_EDIT_QUICK_SETTINGS";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_SETTING:
	case CAM_MENU_MORE_SETTING:
		strID = "IDS_CAM_HEADER_MODE_SETTINGS";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_MORE_HELP:
		strID = "IDS_CAM_BODY_HELP_ABB";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_EFFECTS:
		strID = "IDS_CAM_BODY_EFFECTS";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_EFFECT_NONE:
		strID = "IDS_CAM_BODY_NO_EFFECT_ABB";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_EFFECT_GREY:
		strID = "IDS_CAM_BODY_GREYSCALE";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_EFFECT_NEGATIVE:
		strID = "IDS_CAM_BODY_NEGATIVE_M_EFFECT";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_EFFECT_SEPIA:
		strID = "IDS_CAM_BODY_SEPIA";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_MORE:
		strID = "IDS_CAM_BODY_CAMERA_SETTINGS";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_EMPTY:
		strID = " ";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		break;
	case CAM_MENU_MAX_NUMS:
		cam_critical(LOG_CAM, "your array out, please check!", 128);
		break;
	default:
		strID = " ";
		if (strID) {
			if (strlen(strID) + 1 <= 128) {
				strncpy(description, strID, 128);
			}
		}
		cam_critical(LOG_UI, "error type [%d], please check!", item);
		break;
	}

}

CAM_MENU_ITEM cam_convert_setting_value_to_menu_item(CAM_MENU_ITEM setting_type)
{
	struct appdata *ad = (struct appdata *)cam_appdata_get();
	cam_retvm_if(ad == NULL, CAM_MENU_MAX_NUMS, "appdata is null");
	CamAppData *camapp = ad->camapp_handle;
	cam_retvm_if(camapp == NULL, CAM_MENU_MAX_NUMS, "camapp is null");

	CAM_MENU_ITEM item = CAM_MENU_MAX_NUMS;

	switch (setting_type) {
	case CAM_MENU_SHORTCUTS:
	case CAM_MENU_MORE_HELP:
	case CAM_MENU_EFFECTS:
	case CAM_MENU_RESET:
	case CAM_MENU_EXPOSURE_VALUE:
	case CAM_MENU_MORE_SETTING:
	case CAM_MENU_EMPTY:
	case CAM_MENU_SETTING:
	case CAM_MENU_MORE:
		return setting_type;
		break;

	case CAM_MENU_VOLUME_KEY: {
		item = cam_convert_volume_key_to_menu_item(camapp->volume_key);
	}
	break;
	case CAM_MENU_SELF_PORTRAIT: {
		if (camapp->self_portrait == FALSE) {
			item = CAM_MENU_SELF_PORTRAIT_OFF;
		} else {
			item = CAM_MENU_SELF_PORTRAIT_ON;
		}
	}
	break;
	case CAM_MENU_FLASH: {
		item = cam_convert_flash_to_menu_item(camapp->flash) ;
	}
	break;
	case CAM_MENU_SHOOTING_MODE: {
		item = cam_convert_shot_mode_to_menu_item(camapp->shooting_mode) ;
	}
	break;
	case CAM_MENU_RECORDING_MODE: {
		item = cam_convert_recording_mode_to_menu_item(camapp->recording_mode);
	}
	break;
	case CAM_MENU_SCENE_MODE: {
		item = cam_convert_scene_mode_to_menu_item(camapp->scene_mode);
	}
	break;
	case CAM_MENU_TIMER: {
		item = cam_convert_timer_to_menu_item(camapp->timer);
	}
	break;
	case CAM_MENU_PHOTO_RESOLUTION: {
		item = cam_convert_photo_resolution_to_menu_item(camapp->photo_resolution);
	}
	break;
	case CAM_MENU_VIDEO_RESOLUTION: {
		item = cam_convert_video_resolution_to_menu_item(camapp->video_resolution);
	}
	break;
	case CAM_MENU_WHITE_BALANCE: {
		item = cam_convert_wb_to_menu_item(camapp->white_balance);
	}
	break;
	case CAM_MENU_ISO: {
		item = cam_convert_iso_to_menu_item(camapp->iso);
	}
	break;
	case CAM_MENU_METERING: {
		item = cam_convert_metering_to_menu_item(camapp->metering);
	}
	break;
	case CAM_MENU_ANTI_SHAKE:
		if (ad->camapp_handle->anti_shake) {
			item = CAM_MENU_ANTI_SHAKE_ON;
		} else {
			item = CAM_MENU_ANTI_SHAKE_OFF;
		}
		break;
	case CAM_MENU_VIDEO_STABILIZATION:
		if (ad->camapp_handle->video_stabilization) {
			item = CAM_MENU_VIDEO_STABILIZATION_ON;
		} else {
			item = CAM_MENU_VIDEO_STABILIZATION_OFF;
		}
		break;
	case CAM_MENU_AUTO_CONTRAST:
		if (ad->camapp_handle->auto_contrast) {
			item = CAM_MENU_AUTO_CONTRAST_ON;
		} else {
			item = CAM_MENU_AUTO_CONTRAST_OFF;
		}
		break;
	case CAM_MENU_TAP_SHOT:
		if (camapp->tap_shot) {
			item = CAM_MENU_TAP_SHOT_ON;
		} else {
			item = CAM_MENU_TAP_SHOT_OFF;
		}
		break;
	case CAM_MENU_GPS_TAG:
		if (camapp->gps) {
			item = CAM_MENU_GPS_TAG_ON;
		} else {
			item = CAM_MENU_GPS_TAG_OFF;
		}
		break;
	case CAM_MENU_SAVE_AS_FLIP:
		if (ad->camapp_handle->save_as_flip) {
			item = CAM_MENU_SAVE_AS_FLIP_ON;
		} else {
			item = CAM_MENU_SAVE_AS_FLIP_OFF;
		}
		break;
	case CAM_MENU_SHUTTER_SOUND:
		if (ad->camapp_handle->need_shutter_sound) {
			item = CAM_MENU_SHUTTER_SOUND_ON;
		} else {
			item = CAM_MENU_SHUTTER_SOUND_OFF;
		}
		break;
	case CAM_MENU_STORAGE: {
		item = cam_convert_storage_to_menu_item(camapp->storage) ;
	}
	break;
	case CAM_MENU_REVIEW: {
		if (ad->camapp_handle->review || ad->camapp_handle->share == CAM_SHARE_BUDDY_PHOTO) {
			item = CAM_MENU_REVIEW_ON;
		} else {
			item = CAM_MENU_REVIEW_OFF;
		}
	}
	break;
	case CAM_MENU_FACE_DETECTION: {
		if (ad->camapp_handle->face_detection == CAM_FACE_DETECTION_ON) {
			item = CAM_MENU_FACE_DETECTION_ON;
		} else {
			item = CAM_MENU_FACE_DETECTION_OFF;
		}
	}
	break;
	case CAM_MENU_MAX_NUMS:
		cam_critical(LOG_CAM, "your array out, please check!");
		item = CAM_MENU_MAX_NUMS;
		break;
	default:
		cam_critical(LOG_CAM, "Error occur!");
	}

	return item;
}

CamAppProperty cam_convert_menu_item_to_property(CAM_MENU_ITEM menu_item)
{
	switch (menu_item) {
	case CAM_MENU_PHOTO_RESOLUTION:
		return PROP_PHOTO_RESOLUTION;
	case CAM_MENU_VIDEO_RESOLUTION:
		return PROP_VIDEO_RESOLUTION;
	case CAM_MENU_METERING:
		return PROP_METERING;
	case CAM_MENU_FACE_DETECTION:
		return PROP_FACE_DETECTION;
	case CAM_MENU_FLASH:
		return PROP_FLASH;
	case CAM_MENU_VOLUME_KEY:
		return PROP_VOLUME_KEY;
	case CAM_MENU_ANTI_SHAKE:
		return PROP_AHS;
	case CAM_MENU_VIDEO_STABILIZATION:
		return PROP_VIDEO_STABILIZATION;
	case CAM_MENU_REVIEW:
		return PROP_REVIEW;
	case CAM_MENU_SAVE_AS_FLIP:
		return PROP_SAVE_AS_FLIP;
	case CAM_MENU_TAP_SHOT:
		return PROP_TAP_SHOT;
	case CAM_MENU_GPS_TAG:
		return PROP_GPS;
	case CAM_MENU_SHUTTER_SOUND:
		return PROP_SHUTTER_SOUND;
	default:
		cam_critical(LOG_UI, "error menu_item [%d], please check!", menu_item);
		break;
	}

	return PROP_NUMS;
}

char *cam_convert_menu_item_to_config_key_name(CAM_MENU_ITEM menu_item)
{
	switch (menu_item) {
	case CAM_MENU_PHOTO_RESOLUTION:
		return PROP_PHOTO_RESOLUTION_NAME;
	case CAM_MENU_VIDEO_RESOLUTION:
		return PROP_VIDEO_RESOLUTION_NAME;
	case CAM_MENU_METERING:
		return PROP_METERING_NAME;
	case CAM_MENU_FACE_DETECTION:
		return PROP_FACE_DETECTION_NAME;
	case CAM_MENU_FLASH:
		return PROP_FLASH_NAME;
	case CAM_MENU_VOLUME_KEY:
		return PROP_VOLUME_KEY_NAME;
	case CAM_MENU_ANTI_SHAKE:
		return PROP_AHS_NAME;
	case CAM_MENU_VIDEO_STABILIZATION:
		return PROP_VIDEO_STABILIZATION_NAME;
	case CAM_MENU_REVIEW:
		return PROP_REVIEW_NAME;
	case CAM_MENU_SAVE_AS_FLIP:
		return PROP_SAVE_AS_FLIP_NAME;
	case CAM_MENU_TAP_SHOT:
		return PROP_TAP_SHOT_NAME;
	case CAM_MENU_GPS_TAG:
		return PROP_GPS_NAME;
	case CAM_MENU_SHUTTER_SOUND:
		return PROP_SHUTTER_SOUND_NAME;
	default:
		cam_critical(LOG_UI, "error menu_item [%d], please check!", menu_item);
		break;
	}

	return NULL;
}

CamConfigKeyType cam_convert_menu_item_to_key_type(CAM_MENU_ITEM menu_item)
{
	CamConfigKeyType type = CAM_CONFIG_KEY_TYPE_MAX;

	switch (menu_item) {
	case CAM_MENU_PHOTO_RESOLUTION:
	case CAM_MENU_VIDEO_RESOLUTION:
	case CAM_MENU_METERING:
	case CAM_MENU_FACE_DETECTION:
	case CAM_MENU_FLASH:
	case CAM_MENU_VOLUME_KEY:
		type = CAM_CONFIG_KEY_TYPE_INT;
		break;
	case CAM_MENU_ANTI_SHAKE:
	case CAM_MENU_VIDEO_STABILIZATION:
	case CAM_MENU_REVIEW:
	case CAM_MENU_SAVE_AS_FLIP:
	case CAM_MENU_TAP_SHOT:
	case CAM_MENU_GPS_TAG:
	case CAM_MENU_SHUTTER_SOUND:
		type = CAM_CONFIG_KEY_TYPE_BOOL;
		break;
	default:
		type = CAM_CONFIG_KEY_TYPE_MAX;
		cam_critical(LOG_UI, "error menu_item [%d], please check!", menu_item);
		break;
	}

	return type;
}

CamConfigType cam_convert_menu_item_to_config_type(CAM_MENU_ITEM menu_item)
{
	CamConfigType type = CAM_CONFIG_MAX;

	switch (menu_item) {
	case CAM_MENU_PHOTO_RESOLUTION:
	case CAM_MENU_VIDEO_RESOLUTION:
	case CAM_MENU_METERING:
	case CAM_MENU_FACE_DETECTION:
	case CAM_MENU_FLASH:
	case CAM_MENU_VOLUME_KEY:
	case CAM_MENU_ANTI_SHAKE:
	case CAM_MENU_VIDEO_STABILIZATION:
	case CAM_MENU_REVIEW:
	case CAM_MENU_SAVE_AS_FLIP:
	case CAM_MENU_TAP_SHOT:
	case CAM_MENU_GPS_TAG:
	case CAM_MENU_SHUTTER_SOUND:
		type = CAM_CONFIG_TYPE_COMMON;
		break;
	default:
		type = CAM_CONFIG_MAX;
		cam_critical(LOG_UI, "error menu_item [%d], please check!", menu_item);
		break;
	}

	return type;
}


//end file
