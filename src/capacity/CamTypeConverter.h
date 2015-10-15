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

#ifndef CAM_TYPECONVERTER_H_
#define CAM_TYPECONVERTER_H_

#include "camera.h"
#include "cam_property.h"

class CCamTypeConverter {

public:
static unsigned int CamIsoDevConvertCaps(camera_attr_iso_e iso);
//static unsigned int CamEffectDevConvertCaps(camera_effect_gs_e effect);
static unsigned int CamFpsDevConvertCaps(camera_attr_fps_e fps);
static unsigned int CamWbDevConvertCaps(camera_attr_whitebalance_e wb);
static unsigned int CamMeteringDevConvertCaps(camera_attr_exposure_mode_e metering);
static unsigned int CamSceneDevConvertCaps(camera_attr_scene_mode_e scene);
static unsigned int CamFlashDevConvertCaps(camera_attr_flash_mode_e flash);
static unsigned int CamFocusDevConvertCaps(camera_attr_af_mode_e focus);

static unsigned int CamIsoCamConvertCaps(CamIso iso);
static unsigned int CamEffectCamConvertCaps(CamSettingsEffects effect);
static unsigned int CamWbCamConvertCaps(CamSettingsWB wb);
static unsigned int CamMeteringCamConvertCaps(CamMetering metering);
static unsigned int CamSceneCamConvertCaps(CamSceneMode scene);
static unsigned int CamFlashCamConvertCaps(CamFlashMode flash);
static unsigned int CamFocusCamConvertCaps(CamFocusMode focus);
static unsigned int CamResolutionCamConvertCaps(unsigned int res);
};

#endif				/*  CAM_TYPECONVERTER_H_ */

