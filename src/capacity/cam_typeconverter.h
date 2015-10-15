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


#ifdef _cplusplus
extern "C" {
#endif


#include "camera.h"
#include "cam_property.h"


unsigned int cam_iso_dev_convert_caps(camera_attr_iso_e iso);
//unsigned int cam_effect_dev_convert_caps(camera_effect_gs_e effect);
unsigned int cam_fps_dev_convert_caps(camera_attr_fps_e fps);
unsigned int cam_wb_dev_convert_caps(camera_attr_whitebalance_e wb);
unsigned int cam_metering_dev_convert_caps(camera_attr_exposure_mode_e metering);
unsigned int cam_scene_dev_convert_caps(camera_attr_scene_mode_e scene);
unsigned int cam_flash_dev_convert_caps(camera_attr_flash_mode_e flash);
unsigned int cam_focus_dev_convert_caps(camera_attr_af_mode_e focus);

unsigned int cam_iso_cam_convert_caps(CamIso iso);
unsigned int cam_effect_cam_convert_caps(CamSettingsEffects effect);
unsigned int cam_wb_cam_convert_caps(CamSettingsWB wb);
unsigned int cam_metering_cam_convert_caps(CamMetering metering);
unsigned int cam_scene_cam_convert_caps(CamSceneMode scene);
unsigned int cam_flash_cam_convert_caps(CamFlashMode flash);
unsigned int cam_focus_cam_convert_caps(CamFocusMode focus);
unsigned int cam_resolution_cam_convert_caps(unsigned int res);

#ifdef _cplusplus
}
#endif



#endif				/*  CAM_TYPECONVERTER_H_ */
