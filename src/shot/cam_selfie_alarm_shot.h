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


#ifndef __DEF_CAM_SELFIE_ALARM_SHOT_H_
#define __DEF_CAM_SELFIE_ALARM_SHOT_H_

#include "cam.h"

void cam_selfie_alarm_photo_shot_data_free(void *data);
gboolean cam_selfie_alarm_shot_capture_start(void *data);
gboolean cam_selfie_alarm_photo_shot_stop(void* data);
void cam_selfie_alarm_shot_capture_callback(camera_image_data_s* image,
								camera_image_data_s* postview,
								camera_image_data_s* thumbnail,
								void *user_data);
void cam_selfie_alarm_create_best_photo_detect_thread(void *data);
gboolean cam_selfie_alarm_shot_init(void* data);
gboolean cam_selfie_alarm_shot_deinit(void *data);
gboolean cam_selfie_alarm_shot_complete(void* data);
gboolean cam_selfie_alarm_shot_is_capturing();
gboolean cam_selfie_alarm_shot_face_detection(camera_preview_data_s *preview_frame, RECT *face_rect, int *face_num);
char* cam_selfie_alarm_shot_get_images(int index);
char* cam_selfie_alarm_shot_get_thumbnail_images(int index);
void cam_selfie_alarm_update_progress_value(void *data);
void cam_selfie_alarm_update_progress_value_reset(void *data);

#endif /*__DEF_CAM_SELFIE_ALARM_SHOT_H_*/

