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


#ifndef __DEF_CAM_SELFIE_ALARM_LAYOUT_H_
#define __DEF_CAM_SELFIE_ALARM_LAYOUT_H_

#include "cam_property.h"
#define USE_SELFIE_FRAME					1

#if USE_SELFIE_FRAME
Evas_Object* cam_selfie_alarm_layout_create(Evas_Object *parent);
void cam_selfie_alarm_layout_destroy();
gboolean cam_selfie_alarm_layout_exist(void *data);
void cam_selfie_alarm_layout_mouse_move_cb(void *data, Evas * evas, Evas_Object *obj, void *event_info);
void cam_selfie_alarm_layout_mouse_down_cb(void *data, Evas * evas, Evas_Object *obj, void *event_info);
void cam_selfie_alarm_layout_mouse_up_cb(void *data, Evas * evas, Evas_Object *obj, void *event_info);
void selfie_alarm_camera_save_rect();
#endif
void cam_selfie_alarm_layout_check_face_area(camera_preview_data_s *preview_frame, void *user_data);
void cam_selfie_alarm_layout_stop_timer();
gboolean cam_selfie_alarm_layout_capturing();

#if USE_SELFIE_FRAME
gboolean cam_selfie_alarm_layout_is_moving();
gboolean cam_selfie_alarm_check_faces();
void cam_selfie_alarm_layout_set_status(gboolean status);
void cam_selfie_alarm_layout_reset_rect();
void cam_selfie_alarm_adjust_resolution();
#endif

#endif	/* __DEF_CAM_SELFIE_ALARM_LAYOUT_H_ */

