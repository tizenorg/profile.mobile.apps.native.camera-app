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


#ifndef _CAM_MENU_ITEM_H_
#define _CAM_MENU_ITEM_H_

#include "cam_property.h"
#include "cam_config.h"

int cam_convert_menu_item_to_setting_value(int mode);
int cam_convert_shot_mode_to_menu_item(int mode);
int cam_convert_flash_to_menu_item(int mode);
int cam_convert_video_stabilization_to_menu_item(int mode);
int cam_convert_recording_mode_to_menu_item(int mode);
int cam_convert_video_resolution_to_menu_item(int mode);
int cam_convert_photo_resolution_to_menu_item(int mode);
int cam_convert_iso_to_menu_item(int mode);
int cam_convert_focus_to_menu_item(int mode);
int cam_convert_timer_to_menu_item(int mode);
int cam_convert_scene_mode_to_menu_item(int mode);
int cam_convert_wb_to_menu_item(int mode);
int cam_convert_metering_to_menu_item(int mode);
int cam_convert_storage_to_menu_item(int mode);
int cam_convert_volume_key_to_menu_item(int mode);
int cam_convert_share_to_menu_item(int mode);
int cam_convert_tap_shot_to_menu_item(int mode);
int cam_convert_setting_value_to_menu_index(void* data, int menu_type, int value);
int cam_convert_face_detection_to_menu_item(int mode);
int cam_convert_anti_shake_to_menu_item(int mode);
int cam_convert_review_to_menu_item(int mode);
int cam_convert_capture_voice_to_menu_item(int mode);
int cam_convert_save_as_flipped_to_menu_item(int mode);
int cam_convert_effect_to_menu_item(int mode);


const char* cam_get_menu_item_image(CAM_MENU_ITEM item, CamMenuState state);
void cam_get_menu_item_text(CAM_MENU_ITEM item, char *description, gboolean without_proportion);
void cam_get_menu_item_description(CAM_MENU_ITEM item, char *description);
int cam_convert_fast_motion_to_menu_item(int mode);
int cam_convert_slow_motion_to_menu_item(int mode);
int cam_convert_menu_shooting_mode_to_setting_value(int mode);
CAM_MENU_ITEM cam_convert_setting_value_to_menu_item(CAM_MENU_ITEM setting_type);
CamAppProperty cam_convert_menu_item_to_property(CAM_MENU_ITEM menu_item);
char *cam_convert_menu_item_to_config_key_name(CAM_MENU_ITEM menu_item);
CamConfigKeyType cam_convert_menu_item_to_key_type(CAM_MENU_ITEM menu_item);
CamConfigType cam_convert_menu_item_to_config_type(CAM_MENU_ITEM menu_item);

#endif
