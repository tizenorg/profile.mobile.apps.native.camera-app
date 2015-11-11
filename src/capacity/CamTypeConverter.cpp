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

#include "CamTypeConverter.h"
#include "cam_capacity_type.h"
#include "cam_debug.h"


/*static g_cam_single_effects when MMFW support  camera_attr_foreach_supported_effect*/
CamSingleEffects g_cam_single_effects[] = {
	{CAM_SETTINGS_EFFECTS_NOR, 					CAM_CP_EFFECT_NONE},
	{CAM_SETTINGS_EFFECTS_SEPIA, 				CAM_CP_EFFECT_SEPIA},
	{CAM_SETTINGS_EFFECTS_BLACKWHITE, 			CAM_CP_EFFECT_BLACKWHITE},
	{CAM_SETTINGS_EFFECTS_NEGATIVE, 			CAM_CP_EFFECT_NEGATIVE},
};

unsigned int
CCamTypeConverter::CamIsoDevConvertCaps(camera_attr_iso_e iso)
{
	unsigned int ret = 0;

	switch (iso) {
	case CAMERA_ATTR_ISO_AUTO:
		ret = CAM_CP_ISO_AUTO;
		break;
	case CAMERA_ATTR_ISO_50:
		ret = CAM_CP_ISO_50;
		break;
	case CAMERA_ATTR_ISO_100:
		ret = CAM_CP_ISO_100;
		break;
	case CAMERA_ATTR_ISO_200:
		ret = CAM_CP_ISO_200;
		break;
	case CAMERA_ATTR_ISO_400:
		ret = CAM_CP_ISO_400;
		break;
	case CAMERA_ATTR_ISO_800:
		ret = CAM_CP_ISO_800;
		break;
	case CAMERA_ATTR_ISO_1600:
		ret = CAM_CP_ISO_1600;
		break;
	case CAMERA_ATTR_ISO_3200:
		ret = CAM_CP_ISO_3200;
		break;
	default:
		break;
	}

	return ret;

}

unsigned int
CCamTypeConverter::CamIsoCamConvertCaps(CamIso iso)
{
	unsigned int ret = 0;

	switch (iso) {
	case CAM_ISO_AUTO:
		ret = CAM_CP_ISO_AUTO;
		break;
	case CAM_ISO_50:
		ret = CAM_CP_ISO_50;
		break;
	case CAM_ISO_100:
		ret = CAM_CP_ISO_100;
		break;
	case CAM_ISO_200:
		ret = CAM_CP_ISO_200;
		break;
	case CAM_ISO_400:
		ret = CAM_CP_ISO_400;
		break;
	case CAM_ISO_800:
		ret = CAM_CP_ISO_800;
		break;
	case CAM_ISO_1600:
		ret = CAM_CP_ISO_1600;
		break;
	case CAM_ISO_3200:
		ret = CAM_CP_ISO_3200;
		break;
	default:
		break;
	}

	return ret;
}

/*unsigned int
CCamTypeConverter::CamEffectDevConvertCaps(camera_effect_gs_e effect)
{
	unsigned int loop = 0;
	for (loop = 0; loop < sizeof(g_cam_single_effects)/sizeof(CamSingleEffects); loop++) {
		if (effect == (camera_effect_gs_e)g_cam_single_effects[loop].effect_type) {
			return g_cam_single_effects[loop].effect_cap;
		}
	}

	return 0;
}*/

unsigned int
CCamTypeConverter::CamEffectCamConvertCaps(CamSettingsEffects effect)
{
	unsigned int loop = 0;
	for (loop = 0; loop < sizeof(g_cam_single_effects) / sizeof(CamSingleEffects); loop++) {
		if (effect == g_cam_single_effects[loop].effect_type) {
			return g_cam_single_effects[loop].effect_cap;
		}
	}

	return 0;
}


unsigned int
CCamTypeConverter::CamFpsDevConvertCaps(camera_attr_fps_e fps)
{
	unsigned int ret = 0;

	switch (fps) {
	case CAMERA_ATTR_FPS_AUTO:
		ret = CAM_CP_FPS_AUTO;
		break;
	case CAMERA_ATTR_FPS_8:
		ret = CAM_CP_FPS_8;
		break;
	case CAMERA_ATTR_FPS_15:
		ret = CAM_CP_FPS_15;
		break;
	case CAMERA_ATTR_FPS_24:
		ret = CAM_CP_FPS_24;
		break;
	case CAMERA_ATTR_FPS_25:
		ret = CAM_CP_FPS_25;
		break;
	case CAMERA_ATTR_FPS_30:
		ret = CAM_CP_FPS_30;
		break;
	case CAMERA_ATTR_FPS_60:
		ret = CAM_CP_FPS_60;
		break;
	case CAMERA_ATTR_FPS_120:
		ret = CAM_CP_FPS_120;
		break;
	default:
		break;
	}

	return ret;

}

unsigned int
CCamTypeConverter::CamWbDevConvertCaps(camera_attr_whitebalance_e wb)
{
	unsigned int ret = 0;

	switch (wb) {
	case CAMERA_ATTR_WHITE_BALANCE_NONE:
		break;
	case CAMERA_ATTR_WHITE_BALANCE_AUTOMATIC:
		ret = CAM_CP_WB_AUTO;
		break;
	case CAMERA_ATTR_WHITE_BALANCE_DAYLIGHT:
		ret = CAM_CP_WB_DAYLIGHT;
		break;
	case CAMERA_ATTR_WHITE_BALANCE_CLOUDY:
		ret = CAM_CP_WB_CLOUDY;
		break;
	case CAMERA_ATTR_WHITE_BALANCE_FLUORESCENT:
		ret = CAM_CP_WB_FLUORESCENT;
		break;
	case CAMERA_ATTR_WHITE_BALANCE_INCANDESCENT:
		ret = CAM_CP_WB_INCANDESCENT;
		break;
	case CAMERA_ATTR_WHITE_BALANCE_SHADE:
		ret = CAM_CP_WB_SHADE;
		break;
	case CAMERA_ATTR_WHITE_BALANCE_HORIZON:
		ret = CAM_CP_WB_HORIZON;
		break;
	case CAMERA_ATTR_WHITE_BALANCE_FLASH:
		ret = CAM_CP_WB_FLASH;
		break;
	case CAMERA_ATTR_WHITE_BALANCE_CUSTOM:
		ret = CAM_CP_WB_CUSTOM;
		break;
	default:
		break;
	}

	return ret;
}

unsigned int
CCamTypeConverter::CamWbCamConvertCaps(CamSettingsWB wb)
{
	unsigned int ret = 0;

	switch (wb) {
	case CAM_SETTINGS_WB_AWB:
		ret = CAM_CP_WB_AUTO;
		break;
	case CAM_SETTINGS_WB_DAYLIGHT:
		ret = CAM_CP_WB_DAYLIGHT;
		break;
	case CAM_SETTINGS_WB_CLOUDY:
		ret = CAM_CP_WB_CLOUDY;
		break;
	case CAM_SETTINGS_WB_FLUORESCENT:
		ret = CAM_CP_WB_FLUORESCENT;
		break;
	case CAM_SETTINGS_WB_INCANDESCENT:
		ret = CAM_CP_WB_INCANDESCENT;
		break;
	case CAM_SETTINGS_WB_SHADE:
		ret = CAM_CP_WB_SHADE;
		break;
	case CAM_SETTINGS_WB_HORIZON:
		ret = CAM_CP_WB_HORIZON;
		break;
	case CAM_SETTINGS_WB_FLASH:
		ret = CAM_CP_WB_FLASH;
		break;
	case CAM_SETTINGS_WB_CUSTOM:
		ret = CAM_CP_WB_CUSTOM;
		break;
	default:
		break;
	}

	return ret;
}


unsigned int
CCamTypeConverter::CamFocusDevConvertCaps(camera_attr_af_mode_e focus)
{
	unsigned int ret = 0;

	switch (focus) {
	case CAMERA_ATTR_AF_NONE:
		ret = CAM_CP_FOCUS_NONE;
		break;
	case CAMERA_ATTR_AF_NORMAL:
		ret = CAM_CP_FOCUS_AUTO_FOCUS;
		break;
	case CAMERA_ATTR_AF_MACRO:
		ret = CAM_CP_FOCUS_MACRO;
		break;
	case CAMERA_ATTR_AF_FULL:
		ret = CAM_CP_FOCUS_CAF;
		break;
	default:
		break;
	}

	return ret;
}

unsigned int
CCamTypeConverter::CamFocusCamConvertCaps(CamFocusMode focus)
{
	unsigned int ret = 0;

	switch (focus) {
	case CAM_FOCUS_AUTO:
		ret = CAM_CP_FOCUS_AUTO_FOCUS;
		break;
	case CAM_FOCUS_MACRO:
		ret = CAM_CP_FOCUS_MACRO;
		break;
	case CAM_FOCUS_FACE:
		ret = CAM_CP_FOCUS_FACE_DETECTION;
		break;
	case CAM_FOCUS_CAF:
		ret = CAM_CP_FOCUS_CAF;
		break;
	default:
		break;
	}

	return ret;
}

unsigned int
CCamTypeConverter::CamMeteringDevConvertCaps(camera_attr_exposure_mode_e metering)
{
	unsigned int ret = 0;

	switch (metering) {
	case CAMERA_ATTR_EXPOSURE_MODE_CENTER:
		ret = CAM_CP_METERING_CENTERWEIGHT;
		break;
	case CAMERA_ATTR_EXPOSURE_MODE_SPOT:
		ret = CAM_CP_METERING_SPOT;
		break;
	case CAMERA_ATTR_EXPOSURE_MODE_ALL:
		ret = CAM_CP_METERING_MATRIX;
		break;
	default:
		break;
	}

	return ret;
}

unsigned int
CCamTypeConverter::CamMeteringCamConvertCaps(CamMetering type)
{
	unsigned int ret = 0;

	switch (type) {
	case CAM_METERING_MATRIX:
		ret = CAM_CP_METERING_MATRIX;
		break;
	case CAM_METERING_CENTER_WEIGHTED:
		ret = CAM_CP_METERING_CENTERWEIGHT;
		break;
	case CAM_METERING_SPOT:
		ret = CAM_CP_METERING_SPOT;
		break;
	default:
		break;
	}

	return ret;
}

unsigned int
CCamTypeConverter::CamSceneDevConvertCaps(camera_attr_scene_mode_e scene)
{
	unsigned int ret = 0;
	switch (scene) {
	case CAMERA_ATTR_SCENE_MODE_NORMAL:
		ret = CAM_CP_SCENE_MODE_AUTO;
		break;
	case CAMERA_ATTR_SCENE_MODE_PORTRAIT:
		ret = CAM_CP_SCENE_MODE_PORTRAIT;
		break;
	case CAMERA_ATTR_SCENE_MODE_LANDSCAPE:
		ret = CAM_CP_SCENE_MODE_LANDSCAPE;
		break;
	case CAMERA_ATTR_SCENE_MODE_SPORTS:
		ret = CAM_CP_SCENE_MODE_SPORTS;
		break;
	case CAMERA_ATTR_SCENE_MODE_PARTY_N_INDOOR:
		ret = CAM_CP_SCENE_MODE_PARTY_AND_INDOOR;
		break;
	case CAMERA_ATTR_SCENE_MODE_BEACH_N_INDOOR:
		ret = CAM_CP_SCENE_MODE_BEACH_AND_SNOW;
		break;
	case CAMERA_ATTR_SCENE_MODE_SUNSET:
		ret = CAM_CP_SCENE_MODE_SUNSET;
		break;
	case CAMERA_ATTR_SCENE_MODE_DUSK_N_DAWN:
		ret = CAM_CP_SCENE_MODE_DUSK_AND_DAWN;
		break;
	case CAMERA_ATTR_SCENE_MODE_FALL_COLOR:
		ret = CAM_CP_SCENE_MODE_FALLCOLOR;
		break;
	case CAMERA_ATTR_SCENE_MODE_NIGHT_SCENE:
		ret = CAM_CP_SCENE_MODE_NIGHTSHOT;
		break;
	case CAMERA_ATTR_SCENE_MODE_FIREWORK:
		ret = CAM_CP_SCENE_MODE_FIREWORK;
		break;
	case CAMERA_ATTR_SCENE_MODE_TEXT:
		ret = CAM_CP_SCENE_MODE_TEXT;
		break;
	case CAMERA_ATTR_SCENE_MODE_CANDLE_LIGHT:
		ret = CAM_CP_SCENE_MODE_CANDLE_LIGHT;
		break;
	case CAMERA_ATTR_SCENE_MODE_BACKLIGHT:
		ret = CAM_CP_SCENE_MODE_BACK_LIGHT;
		break;
	case CAMERA_ATTR_SCENE_MODE_SHOW_WINDOW:
		ret = CAM_CP_SCENE_MODE_SHOW_WINDOW;
		break;
	default:
		break;
	}

	return ret;
}


unsigned int
CCamTypeConverter::CamSceneCamConvertCaps(CamSceneMode scene)
{
	unsigned int ret = 0;

	switch (scene) {
	case CAM_SCENE_NONE:
		ret = CAM_CP_SCENE_MODE_AUTO;
		break;
	case CAM_SCENE_PORTRAIT:
		ret = CAM_CP_SCENE_MODE_PORTRAIT;
		break;
	case CAM_SCENE_LANDSCAPE:
		ret = CAM_CP_SCENE_MODE_LANDSCAPE;
		break;
	case CAM_SCENE_SPORTS:
		ret = CAM_CP_SCENE_MODE_SPORTS;
		break;
	case CAM_SCENE_PARTY:
		ret = CAM_CP_SCENE_MODE_PARTY_AND_INDOOR;
		break;
	case CAM_SCENE_BEACHSNOW:
		ret = CAM_CP_SCENE_MODE_BEACH_AND_SNOW;
		break;
	case CAM_SCENE_SUNSET:
		ret = CAM_CP_SCENE_MODE_SUNSET;
		break;
	case CAM_SCENE_DUSKDAWN:
		ret = CAM_CP_SCENE_MODE_DUSK_AND_DAWN;
		break;
	case CAM_SCENE_FALL:
		ret = CAM_CP_SCENE_MODE_FALLCOLOR;
		break;
	case CAM_SCENE_NIGHT:
		ret = CAM_CP_SCENE_MODE_NIGHTSHOT;
		break;
	case CAM_SCENE_FIREWORK:
		ret = CAM_CP_SCENE_MODE_FIREWORK;
		break;
	case CAM_SCENE_TEXT:
		ret = CAM_CP_SCENE_MODE_TEXT;
		break;
	case CAM_SCENE_CANDLELIGHT:
		ret = CAM_CP_SCENE_MODE_CANDLE_LIGHT;
		break;
	case CAM_SCENE_BACKLIGHT:
		ret = CAM_CP_SCENE_MODE_BACK_LIGHT;
		break;
	case CAM_SCENE_SHOW_WINDOW:
		ret = CAM_CP_SCENE_MODE_SHOW_WINDOW;
		break;
	default:
		break;
	}

	return ret;
}

unsigned int
CCamTypeConverter::CamFlashDevConvertCaps(camera_attr_flash_mode_e flash)
{
	unsigned int ret = 0;

	switch (flash) {
	case CAMERA_ATTR_FLASH_MODE_OFF:
		ret = CAM_CP_FLASH_OFF;
		break;
	case CAMERA_ATTR_FLASH_MODE_ON:
		ret = CAM_CP_FLASH_ON;
		break;
	case CAMERA_ATTR_FLASH_MODE_AUTO:
		ret = CAM_CP_FLASH_AUTO;
		break;
	case CAMERA_ATTR_FLASH_MODE_REDEYE_REDUCTION:
		ret = CAM_CP_FLASH_REDEYE;
		break;
	case CAMERA_ATTR_FLASH_MODE_SLOW_SYNC:
		ret = CAM_CP_FLASH_SLOW_SYNC;
		break;
	case CAMERA_ATTR_FLASH_MODE_FRONT_CURTAIN:
		ret = CAM_CP_FLASH_FRONT_CURTAIN;
		break;
	case CAMERA_ATTR_FLASH_MODE_REAR_CURTAIN:
		ret = CAM_CP_FLASH_REAR_CURTAIN;
		break;
	case CAMERA_ATTR_FLASH_MODE_PERMANENT:
		ret = CAM_CP_FLASH_PERMANENT;
		break;
	default:
		break;
	}

	return ret;
}

unsigned int
CCamTypeConverter::CamFlashCamConvertCaps(CamFlashMode flash)
{
	unsigned int ret = 0;

	switch (flash) {
	case CAM_FLASH_OFF:
		ret = CAM_CP_FLASH_OFF;
		break;
	case CAM_FLASH_ON:
		ret = CAM_CP_FLASH_ON;
		break;
	case CAM_FLASH_AUTO:
		ret = CAM_CP_FLASH_AUTO;
		break;
	case CAM_FLASH_MOVIE_ON:
		ret = CAM_CP_FLASH_PERMANENT;
		break;
	default:
		break;
	}

	return ret;
}

unsigned int
CCamTypeConverter::CamResolutionCamConvertCaps(unsigned int res)
{
	unsigned int ret = 0;

	switch (res) {
	case CAM_RESOLUTION_3264x2448:
		ret = CAM_CP_SIZE_3264X2448;
		break;
	case CAM_RESOLUTION_3264x1836:
		ret = CAM_CP_SIZE_3264X1836;
		break;
	case CAM_RESOLUTION_2560x1920:
		ret = CAM_CP_SIZE_2560X1920;
		break;
	case CAM_RESOLUTION_2560x1536:
		ret = CAM_CP_SIZE_2560X1536;
		break;
	case CAM_RESOLUTION_2560x1440:
		ret = CAM_CP_SIZE_2560X1440;
		break;
	case CAM_RESOLUTION_2448x2448:
		ret = CAM_CP_SIZE_2448X2448;
		break;
	case CAM_RESOLUTION_2048x1536:
		ret = CAM_CP_SIZE_2048X1536;
		break;
	case CAM_RESOLUTION_2048x1232:
		ret = CAM_CP_SIZE_2048X1232;
		break;
	case CAM_RESOLUTION_2048x1152:
		ret = CAM_CP_SIZE_2048X1152;
		break;
	case CAM_RESOLUTION_1920x1080:
		ret = CAM_CP_SIZE_1920X1080;
		break;
	case CAM_RESOLUTION_1600x1200:
		ret = CAM_CP_SIZE_1600X1200;
		break;
	case CAM_RESOLUTION_1440x1080:
		ret = CAM_CP_SIZE_1440X1080;
		break;
	case CAM_RESOLUTION_1392x1392:
		ret = CAM_CP_SIZE_1392X1392;
		break;
	case CAM_RESOLUTION_1280x960:
		ret = CAM_CP_SIZE_1280X960;
		break;
	case CAM_RESOLUTION_1280x720:
		ret = CAM_CP_SIZE_1280X720;
		break;
	case CAM_RESOLUTION_1080x1080:
		ret = CAM_CP_SIZE_1080X1080;
		break;
	case CAM_RESOLUTION_1056x1056:
		ret = CAM_CP_SIZE_1080X1080;
		break;
	case CAM_RESOLUTION_SVGA:
		ret = CAM_CP_SIZE_800X600;
		break;
	case CAM_RESOLUTION_WVGA:
		ret = CAM_CP_SIZE_800X480;
		break;
	case CAM_RESOLUTION_WVGA2:
		ret = CAM_CP_SIZE_720X480;
		break;
	case CAM_RESOLUTION_VGA:
		ret = CAM_CP_SIZE_640X480;
		break;
	case CAM_RESOLUTION_WQVGA:
		ret = CAM_CP_SIZE_400X240;
		break;
	case CAM_RESOLUTION_QVGA:
		ret = CAM_CP_SIZE_320X240;
		break;
	case CAM_RESOLUTION_QCIF:
		ret = CAM_CP_SIZE_176X144;
		break;
	default:
		break;
	}

	return ret;
}


