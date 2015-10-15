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

#ifndef _CAM_SHOT_H_
#define _CAM_SHOT_H_

#include "cam.h"

gboolean cam_shot_create(void* data);
gboolean cam_shot_destroy(void* data);
gboolean cam_shot_capture(void* data);
void cam_shot_init_callback(void* data);
Eina_Bool cam_shot_capture_complete(void *data);
void cam_shot_stop_capture(void* data);
gboolean  cam_shot_is_capturing(void *data);
void cam_shot_start_image_viwer_thread(void *data);
void cam_shot_shutter_sound_cb(void *data);
void cam_shot_capture_completed_cb(void *user_data);
void cam_shot_capture_cb(camera_image_data_s* image, camera_image_data_s* postview, camera_image_data_s* thumbnail, void *user_data);
void cam_common_shot_set_property(void *data);
Eina_Bool cam_shot_stop_preview_after_caputre_timer(void *data);
void cam_shot_push_multi_shots(void *data);
gboolean cam_shot_update_thumbnail_using_thumbnail_data(struct appdata *ad);
void cam_shot_update_progress_vaule(void* data);
void cam_shot_restore_sound_session(void *data);

#endif
