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


extern "C" {

#include "cam_property.h"
#include "CamTypeConverter.h"
#include "cam_typeconverter.h"


unsigned int cam_iso_dev_convert_caps(camera_attr_iso_e iso)
{
	return CCamTypeConverter::CamIsoDevConvertCaps(iso);
}

 unsigned int cam_iso_cam_convert_caps(CamIso iso)
{
	return CCamTypeConverter::CamIsoCamConvertCaps(iso);
}

/*unsigned int cam_effect_dev_convert_caps(camera_effect_gs_e effect)
{
	return 	CCamTypeConverter::CamEffectDevConvertCaps(effect);
}*/

unsigned int cam_effect_cam_convert_caps(CamSettingsEffects effect)
{
	return 	CCamTypeConverter::CamEffectCamConvertCaps(effect);
}

unsigned int cam_fps_dev_convert_caps(camera_attr_fps_e fps)
{
	return 	CCamTypeConverter::CamFpsDevConvertCaps(fps);
}

unsigned int cam_wb_dev_convert_caps(camera_attr_whitebalance_e wb)
{
	return 	CCamTypeConverter::CamWbDevConvertCaps(wb);
}

unsigned int cam_wb_cam_convert_caps(CamSettingsWB wb)
{
	return 	CCamTypeConverter::CamWbCamConvertCaps(wb);
}


unsigned int cam_focus_dev_convert_caps(camera_attr_af_mode_e focus)
{
	return 	CCamTypeConverter::CamFocusDevConvertCaps(focus);
}

unsigned int cam_focus_cam_convert_caps(CamFocusMode focus)
{
	return 	CCamTypeConverter::CamFocusCamConvertCaps(focus);
}

unsigned int cam_metering_dev_convert_caps(camera_attr_exposure_mode_e metering)
{
	return 	CCamTypeConverter::CamMeteringDevConvertCaps(metering);
}

unsigned int cam_metering_cam_convert_caps(CamMetering type)
{
	return 	CCamTypeConverter::CamMeteringCamConvertCaps(type);
}

unsigned int cam_scene_dev_convert_caps(camera_attr_scene_mode_e scene)
{
	return 	CCamTypeConverter::CamSceneDevConvertCaps(scene);
}

unsigned int cam_scene_cam_convert_caps(CamSceneMode scene)
{
	return 	CCamTypeConverter::CamSceneCamConvertCaps(scene);
}

unsigned int cam_flash_dev_convert_caps(camera_attr_flash_mode_e flash)
{
	return 	CCamTypeConverter::CamFlashDevConvertCaps(flash);
}

unsigned int cam_flash_cam_convert_caps(CamFlashMode flash)
{
	return 	CCamTypeConverter::CamFlashCamConvertCaps(flash);
}

unsigned int cam_resolution_cam_convert_caps(unsigned int res)
{
	return 	CCamTypeConverter::CamResolutionCamConvertCaps(res);
}

}


