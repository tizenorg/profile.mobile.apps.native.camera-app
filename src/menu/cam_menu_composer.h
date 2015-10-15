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


#ifndef _CAM_MENU_COMPOSER_H_
#define _CAM_MENU_COMPOSER_H_

#include "cam_property.h"
#include "cam.h"
#include "cam_setting_view.h"

void cam_compose_list_menu(void *data, int menu_type, cam_menu_composer* composer);
void cam_compose_menu_shooting_mode(void *data, cam_menu_composer* composer);
void cam_compose_menu_scene_mode(void *data, cam_menu_composer* composer);
void cam_compose_menu_photo_resolution(void *data, cam_menu_composer* composer);
void cam_compose_menu_video_resolution(void *data, cam_menu_composer* composer);
void cam_compose_menu_effect(void *data, cam_menu_composer* composer);
void cam_compose_menu_wb(void *data, cam_menu_composer* composer);
void cam_compose_menu_focus_mode(void *data, cam_menu_composer* composer);
void cam_compose_menu_iso(void *data, cam_menu_composer* composer);
void cam_compose_menu_metering(void *data, cam_menu_composer* composer);
void cam_compose_menu_flash(void *data, cam_menu_composer* composer);
void cam_compose_menu_recording_mode(void *data, cam_menu_composer* composer);
void cam_compose_menu_storage(void* data, cam_menu_composer* composer);
void cam_compose_menu_timer_mode(void* data, cam_menu_composer* composer);
void cam_compose_menu_volume_key(void* data, cam_menu_composer* composer);
void cam_compose_menu_share(void* data, cam_menu_composer* composer);
void cam_compose_menu_face_detection(void* data, cam_menu_composer* composer);

void cam_compose_free(cam_menu_composer* composer);
gboolean cam_is_enabled_menu(void *data, const CAM_MENU_ITEM item);
void cam_compose_list_get(CamSettingTabType tab_type, CAM_MENU_ITEM** list, int* list_num);
void cam_compose_menu_common(void *data, cam_menu_composer* composer, CAM_MENU_ITEM *menu_list, int list_cnt);
void cam_compose_menu_fast_motion(void* data, cam_menu_composer* composer);
void cam_compose_menu_slow_motion(void* data, cam_menu_composer* composer);
void cam_compose_menu_shutter_sound(void* data, cam_menu_composer* composer);


#endif
