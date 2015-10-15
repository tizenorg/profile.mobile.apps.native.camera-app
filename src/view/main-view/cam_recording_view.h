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


#ifndef __DEF_CAM_RECORDING_VIEW_H_
#define __DEF_CAM_RECORDING_VIEW_H_

typedef struct __Cam_Recording_View
{
	Evas_Object *parent;
	Evas_Object *layout;

	Evas_Object *rec_stop_button;
	Evas_Object *rec_pause_button;
	Evas_Object *rec_resume_button;
	Evas_Object *recording_icon;
	Evas_Object *progressbar;
	Evas_Object *progressbar_layout;
	Evas_Object *snapshot_button;

	guint64 display_remain_time;
	guint64 max_record_time;
	bool is_memory_limited;
	char *sec_text;

	char *recording_view_edj;

	struct appdata *ad;
} Cam_Recording_View;

gboolean cam_recording_view_create(Evas_Object *parent, struct appdata *ad);
void cam_recording_view_destroy();
void cam_recording_view_rotate(Evas_Object *parent, struct appdata *ad);

void cam_recording_view_rec_stop_button_cb(void *data, Evas_Object *obj, void *event_info);
void cam_recording_view_rec_stop_button_press(void *data, Evas_Object *obj, void *event_info);
void cam_recording_view_rec_stop_button_unpress(void *data, Evas_Object *obj, void *event_info);

void cam_recording_view_add_rotate_object(Elm_Transit *transit);
void cam_recording_view_update_time();

Cam_Recording_View *cam_recording_view_create_instance();
void cam_recording_view_destroy_instance();
Cam_Recording_View *cam_recording_view_get_instance();

gboolean cam_recording_view_create_progress_bar(struct appdata *ad);
void cam_recording_view_display_max_record_time_with_rec_time_and_blink(char *time_text, unsigned long long left_time);
bool cam_recording_view_check_memory_and_set_max_recording_time(void *data, Cam_Recording_View *recording_view);




#endif	/* __DEF_CAM_RECORDING_VIEW_H_ */

